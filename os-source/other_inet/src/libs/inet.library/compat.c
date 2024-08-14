/*
 * Compatibility routines for BSD kernel
 */

#include <sys/param.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/socketvar.h>

#include <stdarg.h>
#include "proto.h"

#ifdef LATTICE
#include <proto/exec.h>
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

struct ifqueue ipintrq;

/*
void microtime(void)
{
	printf("microtime\n");
}
*/

void selwakeup(sb)
	struct sockbuf *sb;
{
	if(sb->sb_sel){
		Signal(sb->sb_sel, 1L<<sb->sb_sigbit);
	}
}

/*
int useracc(void)
{
	return 1;
}


imin(a, b)
	int	a, b;
{
	return  (a < b) ? a:b;
}

void log(int type, char *message, ...)
{
    va_list args;

    va_start(args,message);
    printf("LOG type %d: ", type);
    printf(message,args);
    printf("\n");
    va_end(args);
}
*/

/*
 * Print message and make sure machine is useless
 */
void panic(char *msg)
{
	printf("panic: %s \n", msg);
	Wait(0);
/*	while(1); */          
}

/*
 * overlayed bcopy used in ip_output.  It is probably wrong to use
 * bcopy here.

ovbcopy(from, to, len)
	void	*from, *to;
	int	len;
{
	bcopy(from, to, len);
	return 0;
}
*/

/*
 * Copy UIO to/from linear buffer.
 *
 * uio->uio_resid:	total amount of data in uio structure
 * uio->uio_iovcnt:	total number of IOVs
 * uio->uio_iov:	array[uio->iovcnt] of IOVs
 */
int uiomove(bufp, len, op, uio)
	char *bufp;
	register int len;
	enum uio_op op;
	register struct uio *uio;
{
	register struct iovec *iov;
	register long cnt;

	len = min(len, uio->uio_resid);
	for(iov = uio->uio_iov; len != 0; len -= cnt){
		cnt = min(iov->iov_len, len);
		if(cnt == 0){
			iov = ++uio->uio_iov;
			uio->uio_iovcnt--;
			continue;
		}

		switch(op){

		case UIO_WRITE:
			bcopy(iov->iov_base, bufp, cnt);
			break;

		case UIO_READ:
			bcopy(bufp, iov->iov_base, cnt);
			break;
		}

		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		bufp += cnt;
	}

	return 0;
}

/*
 * signal amiga task that an I/O even has happened
 */
void asignal(so, which)
	struct socket *so;
	int which;
{
	int	sigbit;

	switch (which){
	case SIGIO:
		sigbit = so->so_sigio;
		break;

	case SIGURG:
		sigbit = so->so_sigurg;
		break;

	default:
		sigbit = -1;
	}

	if(sigbit > 0 && so->so_pgrp){
		Signal(so->so_pgrp, (1L << sigbit));
	}
}

/*
copyout(from, to, len)
	void	*from, *to;
	int	len;
{
	bcopy(from, to, len);
	return 0;
}

copyin(from, to, len)
	void	*from, *to;
	int	len;
{
	bcopy(from, to, len);
	return 0;
}
*/


