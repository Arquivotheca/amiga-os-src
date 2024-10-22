
/*
** fd_to_fh - macros to extract Amiga filehandle BPTR from MANX or
**	      LATTICE Unix like fd array.  NOTE THAT AT THIS TIME,
**	      ONLY MANX IS SUPPORTED.
*/

#ifndef FDTOFH_H
#define FDTOFH_H

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

#include <errno.h>

extern fd_set _is_socket;
extern void *_socks[];
extern int num_socket;
extern struct InetBase *InetBase;

#ifdef LATTICE
#include <proto/inet.h>
extern int _nufbs;
#define NUMFD _nufbs
#else

#include <fcntl.h>
#include <libraries/dosextens.h>
#define NUMFD	_numdev

#endif

#define GETSOCK(s, sock) {\
	Chk_Abort();\
	if((s) < 0 || (s) >= NUMFD || !FD_ISSET((s),&_is_socket)){\
		errno = EBADF;\
		return -1;\
	}\
	(sock) = _socks[s];\
}

#define PUTSOCK(s, sock){\
	if((s) < 0 || (s) >= NUMFD){\
		errno = EBADF;\
		return -1;\
	}\
	FD_SET(s, &_is_socket);\
	num_socket++;\
	_socks[s] = sock;\
}

#endif
