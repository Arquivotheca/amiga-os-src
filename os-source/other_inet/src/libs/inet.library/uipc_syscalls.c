/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)uipc_syscalls.c	7.5 (Berkeley) 6/29/88
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <socket.h>

#include "proto.h"

/*
 * System call interface to the socket abstraction.
 */

#define VALIDSO ('S'<<24 | 'O'<<16 | 'C'<<8 | 'K')
#define VALIDATESO(so) (((struct socket *)so)->so_stamp == VALIDSO)
#define VALIDATE(AP, so) if((((long)AP)&1) || AP==0 || !VALIDATESO(so)) return;

#ifdef LATTICE
#define ARGLIST register __d1 caddr_t AP
#define ARGDEFS 
#define FUNCDEF	void __asm __saveds
#include <proto/exec.h>
#else
#define ARGLIST AP
#define ARGDEFS caddr_t AP;
#define __asm
#define FUNCDEF void
#endif


/* LOCAL FUNCTIONS */
static void sendit(register struct socket *so,register struct msghdr *mp,int flags, short *errno, long *rval);
static sockargs(struct mbuf **aname, caddr_t name, int namelen, int type);
static void recvit(register struct socket *, register struct msghdr *, int , short *, caddr_t , short * , long *);


FUNCDEF
_socket(ARGLIST)
	ARGDEFS
{
	register struct sockargs *uap = (struct sockargs *)AP;
	extern struct Task *FindTask();
	struct socket *so;

#ifdef DEBUG_IT
	kprintf("Creating socket with type=%ld protocol=%ld domain=%ld\n",uap->type,uap->protocol,uap->domain);
#endif
	uap->rval = 0l;
	uap->errno = socreate(uap->domain, &so, uap->type, uap->protocol);
	if (uap->errno){
		return;
	}

	so->so_pgrp = FindTask(0L);
	so->so_sigio = uap->sigio;
	so->so_sigurg = uap->sigurg;
	so->so_stamp = VALIDSO;
	uap->rval = so;

	DEBUG("Finished socket\n");
	return;
}

/*
 * inherit - used when inetd passes off open socket to new daemon.  Resets
 *	     signal bits and task pntr in socket struct.
 */
FUNCDEF
_inherit(ARGLIST)
	ARGDEFS
{
	register struct inheritargs *uap = (struct inheritargs *)AP;
	register struct socket *so;
DEBUG("INHERIT\n");
	VALIDATE(AP, uap->fp);
	so = (struct socket *)uap->fp;
	so->so_pgrp = FindTask(0L);
	so->so_sigurg = uap->sigurg;
	so->so_sigio = uap->sigio;
	so->so_rcv.sb_sel = so->so_snd.sb_sel = 0;
	so->so_rcv.sb_flags &= ~SB_SEL;
	so->so_snd.sb_flags &= ~SB_SEL;
DEBUG("finished INHERIT\n");
}

FUNCDEF
_networkclose(ARGLIST)
	ARGDEFS
{
	register struct closeargs *uap = (struct closeargs *)AP;
DEBUG("NETWORK CLOSE\n");
	VALIDATE(AP, uap->fp);
	((struct socket *)(uap->fp))->so_pgrp = 0;
	((struct socket *)(uap->fp))->so_stamp = -1;
	uap->errno = soclose(((struct socket *)(uap->fp)));
DEBUG("Finished closing\n");
}

FUNCDEF
_bind(ARGLIST)
	ARGDEFS
{
	register struct bindargs *uap = (struct bindargs *)AP;
	struct mbuf *nam;

DEBUG("bind\n");
	VALIDATE(AP, uap->fp);
	uap->errno = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (uap->errno){
		return;
	}

	uap->errno = sobind((struct socket *)uap->fp, nam);
	m_freem(nam);
DEBUG("finished bind\n");
}

FUNCDEF
_listen(ARGLIST)
	ARGDEFS
{
	register struct listenargs *uap = (struct listenargs *)AP;

DEBUG("listen\n");
	VALIDATE(AP, uap->fp);
	uap->errno = solisten((struct socket *)uap->fp, uap->backlog);
DEBUG("finished listen\n");
}

FUNCDEF
_accept(ARGLIST)
	ARGDEFS
{
	register struct acceptargs *uap = (struct acceptargs *)AP;
	register struct socket *so, *aso;
	struct mbuf *nam;
	int s;
DEBUG("accept\n");
	VALIDATE(AP, uap->fp);
	if (uap->name == 0){
		goto noname;
	}

noname:
	s = splnet();
	so = (struct socket *)uap->fp;
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		uap->errno = EINVAL;
		splx(s);
		return;
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		uap->errno = EWOULDBLOCK;
		splx(s);
		return;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		if(sleep((ulong)&so->so_timeo, PZERO+1)) {
			so->so_error = EINTR;
			break;
		}
	}
	if (so->so_error) {
		uap->errno = so->so_error;
		so->so_error = 0;
		splx(s);
		return;
	}

	aso = so->so_q;
  	if (soqremque(aso, 1) == 0){
		panic("accept");
	}

	uap->rval = aso;
	aso->so_stamp = VALIDSO;
	nam = m_get(M_WAIT, MT_SONAME);
	(void) soaccept(aso, nam);
	if (uap->name) {
		if (uap->namelen > nam->m_len){
			uap->namelen = nam->m_len;
		}
		bcopy(mtod(nam, caddr_t), (caddr_t)uap->name,
				    (u_int)uap->namelen);
	}
	m_freem(nam);
	splx(s);
DEBUG("finished accept\n");
}

FUNCDEF
_connect(ARGLIST)
	ARGDEFS
{
	register struct connectargs *uap = (struct connectargs*)AP;
	register struct socket *so;
	struct mbuf *nam;

	DEBUG("connect\n");
	VALIDATE(AP, uap->fp);
	DEBUG("CONNECT 1\n");
	so = (struct socket *)uap->fp;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		uap->errno = EALREADY;
		DEBUG("CONNECT: ERROR 1\n");
		return;
	}
	uap->errno = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
DEBUG("Back from sockargs\n");
	if (uap->errno){
		DEBUG("CONNECT: ERROR 2\n");
		return;
	}

	uap->errno = soconnect(so, nam);
DEBUG("Back from soconnect\n");
	if (uap->errno){
		DEBUG("CONNECT: ERROR 3\n");
		goto bad;
	}
	if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
		uap->errno = EINPROGRESS;
		DEBUG("CONNECT: ERROR 4\n");
		m_freem(nam);
		return;
	}
	Forbid();	/* s = splnet(); */
// kprintf("so =%lx so->so_state=%lx so->timeo=%lx\n",so,so->so_state,so->so_timeo);
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0){
		if(sleep((ulong)&so->so_timeo, PZERO+1))
			so->so_error = EINTR;
	}
DEBUG("Back from sleep\n");
	uap->errno = so->so_error;
	so->so_error = 0;
bad2:
	Permit();	/* splx(s); */
bad:
	so->so_state &= ~SS_ISCONNECTING;
	m_freem(nam);
	DEBUG("leaving connect\n");
}

FUNCDEF
_sendto(ARGLIST)
	ARGDEFS
{
	register struct sendtoargs *uap = (struct sendtoargs *)AP;
	struct msghdr msg;
	struct iovec aiov;

//	DEBUG("sendto\n");

	VALIDATE(AP, uap->fp);
	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->fp, &msg, uap->flags, &uap->errno, &uap->rval);

//	DEBUG("leaving sendto\n");
}

FUNCDEF
_send(ARGLIST)
	ARGDEFS
{
	register struct sendargs *uap = (struct sendargs *)AP;
	struct msghdr msg;
	struct iovec aiov;
	DEBUG("send\n");

	VALIDATE(AP, uap->fp);
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->fp, &msg, uap->flags, &uap->errno, &uap->rval);

	DEBUG("leaving send\n");

}

FUNCDEF
_sendmsg(ARGLIST)
	ARGDEFS
{
	register struct sendmsgargs *uap = (struct sendmsgargs *)AP;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];
DEBUG("sendmsg\n");
	VALIDATE(AP, uap->fp);
	bcopy(uap->msg, (caddr_t)&msg, sizeof (msg));
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		uap->errno = EMSGSIZE;
		return;
	}
	bcopy((caddr_t)msg.msg_iov, (caddr_t)aiov,
			(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	msg.msg_iov = aiov;
	sendit(uap->fp, &msg, uap->flags, &uap->errno, &uap->rval);
DEBUG("finished sendmsg\n");
}

FUNCDEF
_recvfrom(ARGLIST)
	ARGDEFS
{
	register struct recvfromargs *uap = (struct recvfromargs *)AP;
	struct msghdr msg;
	struct iovec aiov;
	short len;

// DEBUG("recvfrom\n");
	VALIDATE(AP, uap->fp);
	msg.msg_name = uap->from;
	len = msg.msg_namelen = uap->fromlen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->fp, &msg, uap->flags, &len, (caddr_t)0, &uap->errno, &uap->rval);

	uap->fromlen = len;
// DEBUG("finished recvfrom\n");
}

FUNCDEF
_recv(ARGLIST)
	ARGDEFS
{
	register struct recvargs *uap = (struct recvargs *)AP;
	struct msghdr msg;
	struct iovec aiov;
DEBUG("recv\n");
	VALIDATE(AP, uap->fp);
	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->fp, &msg, uap->flags, (caddr_t)0, (caddr_t)0,
			&uap->errno, &uap->rval);
DEBUG("finished recv\n");
}

FUNCDEF
_recvmsg(ARGLIST)
	ARGDEFS
{
	register struct recvmsgargs *uap = (struct recvmsgargs *)AP;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];
DEBUG("recvmsg\n");
	VALIDATE(AP, uap->fp);
	bcopy((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg));
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		uap->errno = EMSGSIZE;
		return;
	}
	bcopy((caddr_t)msg.msg_iov, (caddr_t)aiov,
				(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	msg.msg_iov = aiov;
	recvit(uap->fp, &msg, uap->flags, (short *)&uap->msg->msg_namelen, (caddr_t)&uap->msg->msg_accrightslen,
				&uap->errno, &uap->rval);
DEBUG("finished recvmsg\n");
}

FUNCDEF
_shutdown(ARGLIST)
	ARGDEFS
{
	register struct shutdownargs *uap = (struct shutdownargs *)AP;
DEBUG("shutdown\n");
	VALIDATE(AP, uap->fp);
	uap->errno = soshutdown((struct socket *)uap->fp, uap->how);
DEBUG("finished shutdown\n");
}

FUNCDEF
_setsockopt(ARGLIST)
	ARGDEFS
{
	register struct setsockoptargs *uap = (struct setsockoptargs *)AP;
	struct mbuf *m = NULL;

	DEBUG("setsockopt\n");
	VALIDATE(AP, uap->fp);
	if (uap->valsize > MLEN) {
		uap->errno = EINVAL;
		return;
	}
	if (uap->val) {
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			uap->errno = ENOBUFS;
			return;
		}
		bcopy(uap->val, mtod(m, caddr_t), (u_int)uap->valsize);
		m->m_len = uap->valsize;
	}
	uap->errno = sosetopt((struct socket *)uap->fp, 
		    			uap->level, uap->name, m);
	DEBUG("leaving setsockopt\n");

}

FUNCDEF
_getsockopt(ARGLIST)
	ARGDEFS
{
	register struct getsockoptargs *uap = (struct getsockoptargs *)AP;
	struct mbuf *m = NULL;

	DEBUG("getsockopt\n");

	VALIDATE(AP, uap->fp);
	uap->errno = sogetopt((struct socket *)uap->fp, 
	    				uap->level, uap->name, &m);
	if (uap->errno){
		goto bad;
	}
	if (uap->val && uap->valsize && m != NULL) {
		if (uap->valsize > m->m_len){
			uap->valsize = m->m_len;
		}
		bcopy(mtod(m, caddr_t), uap->val, (u_int)uap->valsize);
	}

bad:	if (m != NULL){
		(void) m_free(m);
	}

	DEBUG("leaving getsockopt\n");

}

/*
 * Get socket name.
 */
FUNCDEF
_getsockname(ARGLIST)
	ARGDEFS
{
	register struct getsocknameargs *uap = (struct getsocknameargs *)AP;
	register struct socket *so;
	struct mbuf *m;
	int len;

	VALIDATE(AP, uap->fp);
	len = uap->len;
	so = (struct socket *)uap->fp;
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		uap->errno = ENOBUFS;
		return;
	}
	uap->errno = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0);
	if (uap->errno){
		goto bad;
	}
	if (len > m->m_len){
		len = m->m_len;
	}
	bcopy(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	uap->len = len;

bad:	m_freem(m);
}

/*
 * Get name of peer for connected socket.
 */
FUNCDEF
_getpeername(ARGLIST)
	ARGDEFS
{
	register struct getpeernameargs *uap = (struct getpeernameargs *)AP;
	register struct socket *so;
	struct mbuf *m;
DEBUG("getpeername\n");
	VALIDATE(AP, uap->fp);
	so = (struct socket *)uap->fp;
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		uap->errno = ENOTCONN;
		return;
	}
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		uap->errno = ENOBUFS;
		return;
	}
	uap->errno = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0);
	if (uap->errno){
		goto bad;
	}
	if (uap->len > m->m_len){
		uap->len = m->m_len;
	}
	bcopy(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)uap->len);

bad:	m_freem(m);
DEBUG("finished getpeername\n");
}

FUNCDEF
_select(ARGLIST)
	ARGDEFS
{
	register struct selectargs *uap = (struct selectargs *)AP;
	register int fd, off, bit;
	register struct socket *so;
DEBUG("select\n");	
	Forbid();	/* s = splnet(); */
	uap->rval = 0;
	bit = 1;
	for(off = fd = 0; fd < uap->numfds; fd++, bit <<= 1){
		so = uap->socks[fd];

		if(bit == (1<<NFDBITS)){
			off++;
			bit = 1;
		}
#define SEL_ISSET(set) (set->fds_bits[off] & bit)
#define SEL_CLR(set)   (set->fds_bits[off] &= ~bit)

		if(uap->rd_set && SEL_ISSET(uap->rd_set)){
			if(!VALIDATESO(so)){
				uap->errno = EBADF;
				break;
			}
			if(!soreadable(so)){
				SEL_CLR(uap->rd_set);
				so->so_rcv.sb_sel = uap->task;
				so->so_rcv.sb_sigbit = uap->sigbit;
			} else {
				/* leave bit set */
				uap->rval++;
			}
		}

		if(uap->wr_set && SEL_ISSET(uap->wr_set)){
			if(!VALIDATESO(so)){
				uap->errno = EBADF;
				break;
			}
			if(!sowriteable(so)){
				SEL_CLR(uap->wr_set);
				so->so_snd.sb_sel = uap->task;
				so->so_snd.sb_sigbit = uap->sigbit;
			} else {
				/* leave bit set */
				uap->rval++;
			}
		}
	
		if(uap->ex_set && SEL_ISSET(uap->ex_set)){
			if(!VALIDATESO(so)){
				uap->errno = EBADF;
				break;
			}
			if(!(so->so_oobmark || (so->so_state & SS_RCVATMARK))){
				SEL_CLR(uap->ex_set);
				so->so_snd.sb_sel = uap->task;
				so->so_snd.sb_sigbit = uap->sigbit;
			} else {
				/* leave bit set */
				uap->rval++;
			}
		}
#undef SEL_ISSET
#undef SEL_CLR
	}
	Permit();	/* splx(s); */
DEBUG("finished select\n");
}

FUNCDEF
_ioctl(ARGLIST)
	ARGDEFS
{
	struct ioctlargs *uap = (struct ioctlargs *)AP;
	DEBUG("ioctl\n");
	VALIDATE(AP, uap->fp);
	uap->errno = soo_ioctl(uap->fp, uap->cmd, uap->data);	
	DEBUG("leaving ioctl\n");

}

/*
** Local functions.
*/
static void
recvit(so, mp, flags, namelenp, rightslenp, errno, rval)
	register struct socket *so;
	register struct msghdr *mp;
	int flags;
	caddr_t rightslenp;
	short *errno, *namelenp;
	long *rval;
{
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *from, *rights;
	int len;
	
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			*errno = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		auio.uio_resid += iov->iov_len;
	}
	len = auio.uio_resid;
	*errno = soreceive(so, &from, &auio, flags, &rights);
	*rval = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0){
			len = 0;
		} else {
			if (len > from->m_len){
				len = from->m_len;
			}
			bcopy((caddr_t)mtod(from, caddr_t),
				    (caddr_t)mp->msg_name, (unsigned)len);
		}
		*namelenp = len;
	}
	if (mp->msg_accrights) {
		len = mp->msg_accrightslen;
		if (len <= 0 || rights == 0){
			len = 0;
		} else {
			if (len > rights->m_len){
				len = rights->m_len;
			}
			bcopy((caddr_t)mtod(rights, caddr_t),
				    (caddr_t)mp->msg_accrights, (unsigned)len);
		}
		bcopy((caddr_t)&len, rightslenp, sizeof (int));
	}
	if (rights){
		m_freem(rights);
	}
	if (from){
		m_freem(from);
	}
}

static
sockargs(aname, name, namelen, type)
	struct mbuf **aname;
	caddr_t name;
	int namelen, type;
{
	register struct mbuf *m;

	if ((u_int)namelen > MLEN){
		return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if (m == NULL){
		return (ENOBUFS);
	}
	m->m_len = namelen;
	bcopy(name, mtod(m, caddr_t), (u_int)namelen);
	*aname = m;
	return (0);
}

static void
sendit(so, mp, flags, errno, rval)
	register struct socket *so;
	register struct msghdr *mp;
	int flags;
	short *errno;
	long *rval;
{
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *rights;
	int len;
	
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			*errno = EINVAL;
			return;
		}
		if (iov->iov_len == 0){
			continue;
		}
		auio.uio_resid += iov->iov_len;
	}
	if (mp->msg_name) {
		*errno =
		    sockargs(&to, mp->msg_name, mp->msg_namelen, MT_SONAME);
		if (*errno)
			return;
	} else
		to = 0;
	if (mp->msg_accrights) {
		*errno =
		    sockargs(&rights, mp->msg_accrights, mp->msg_accrightslen,
		    MT_RIGHTS);
		if (*errno){
			goto bad;
		}
	} else {
		rights = 0;
	}
	len = auio.uio_resid;
	*errno =
	    sosend(so, to, &auio, flags, rights);
	*rval = len - auio.uio_resid;
	if (rights){
		m_freem(rights);
	}
bad:
	if (to){
		m_freem(to);
	}
}

