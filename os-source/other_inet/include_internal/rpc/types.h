/* @(#)types.h	1.2 87/11/07 3.9 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/*      @(#)types.h 1.18 87/07/24 SMI      */

/*
 * Rpc additions to <sys/types.h>
 */
#ifndef __TYPES_RPC_HEADER__
#define __TYPES_RPC_HEADER__

#include <ss/socket.h>
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
extern struct ExecBase *SysBase;

#define	bool_t	int
#define	enum_t	int

#ifndef FALSE
#define	FALSE	(0)
#endif

#ifndef TRUE
#define	TRUE	(1)
#endif

#define __dontcare__	-1
#ifndef NULL
#	define NULL 0
#endif

#define index(s,c)	strchr(s,c)
#define getegid()	getgid()
#define geteuid()	getuid()
#define ioctl(a,b,c)	s_ioctl(a,b,c)
#define close(sock)		s_close(sock)
#define malloc(a)		AllocVec(a,MEMF_PUBLIC)
#define calloc(a,b)		AllocVec(a*b,MEMF_PUBLIC|MEMF_CLEAR)
#define free(a)			FreeVec(a)

extern int MAXSOCKS;
#define _rpc_dtablesize()	MAXSOCKS

#define getpid()				(ulong)FindTask(0L)

#define mem_alloc(bsize)	AllocVec(bsize,MEMF_PUBLIC)
#define mem_free(ptr, bsize)	FreeVec(ptr)

#ifndef makedev /* ie, we haven't already included it */
#include <sys/types.h>
#endif
/*#include <sys/time.h>*/

#endif /* __TYPES_RPC_HEADER__ */
