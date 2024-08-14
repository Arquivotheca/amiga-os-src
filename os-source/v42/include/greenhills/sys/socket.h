/*
** Generic socket defines.
*/

#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H

#include <sys/types.h>
/*#include <sys/uio.h>*/

/*
** Address families for socket call.
*/

#define        AF_UNSPEC       0               /* unspecified */

#define AF_INET		1	/* Internet protocols		*/
#define AF_UNIX		2	/* Local `pipe' services	*/
#define AF_AMIGA 	3	/* Synonym for above		*/
#define AF_DECNET 	4	/* DECNET protocols		*/
#define AF_802		5	/* ISO protocols		*/
#define AF_ARP		6	/* Access to addr res. protocol	*/
#define AF_RAW		7	/* Direct access to device layer*/

/*
** Types of service for socket call.
*/

#define SOCK_STREAM	1
#define SOCK_DGRAM	2
#define SOCK_RAW	3
#define SOCK_SEQPACKET	4
#define SOCK_RDM	5
#define SOCK_PHYS	6

/*
** Options for [get|set]sockopt.
*/

#define SO_DEBUG	(1<<0)
#define SO_REUSEADDR	(1<<1)
#define SO_KEEPALIVE	(1<<2)
#define SO_DONTROUTE	(1<<3)
#define SO_LINGER	(1<<4)
#define SO_DONTLINGER	(1<<5)

#define SOL_SOCKET	0xffff	/* socket lvl options	*/

/*
** Flags for use by send*&recv* calls.
*/

#define	MSG_PEEK	0x01	/* Peek at msg. Not implemented	*/
#define MSG_OOB		0x02	/* Send/recv out-of-band data	*/

/*
** struct msg_hdr is used in sendmsg/recvmsg to specify gather/scatter
** message.
*/
struct msg_hdr {
	caddr_t	msg_name;	/* address, if non zero		*/
	long	msg_namelen;	/* length of address		*/
	struct iovec *msg_iov;	/* message vector		*/
	long	msg_iovlen;	/* length of message vector	*/
	caddr_t	msg_accrights;	/* access rights, ignored	*/
	long	msg_accrightslen; /* length of accessrights	*/
};

/*
** Generic socket template.
*/
struct sockaddr {
	u_short	sa_family;
	u_char	sa_data[14];
};


#endif SYS_SOCKET_H

