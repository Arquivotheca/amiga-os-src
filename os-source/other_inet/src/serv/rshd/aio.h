/* -----------------------------------------------------------------------
 * aio.h   (rshd) 
 *
 * $Locker:  $
 *
 * $Id: aio.h,v 1.2 92/07/10 17:40:55 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	aio.h,v $
 * Revision 1.2  92/07/10  17:40:55  bj
 * Added RCS header.
 * 
 *
 * $Header: AS225:src/serv/rshd/RCS/aio.h,v 1.2 92/07/10 17:40:55 bj Exp $
 *
 *------------------------------------------------------------------------
 */
#ifndef AIO_H
#define AIO_H

/*
**	non-blocking, asynchronous I/O functions
**
**	Martin Hunt	Mar 1992	
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>


struct afh {
	struct FileHandle 		*fh;
	struct StandardPacket	pk;
	struct MsgPort			*rp;
	long	pending;
};

typedef struct afh AFH;

/* prototypes for functions */

AFH *AOpen( char *name, LONG mode);
AFH *COpen( void );
void ARead( AFH *af, void *buf, LONG size);
void AWrite( AFH *af, void *buf, LONG size);
void AClose(AFH *af);
void CClose(AFH *af);

#define ASignal(a)	(a->rp->mp_SigBit)
#define APort(a)	(a->rp)
#define ARes1(a)	(a->pk.sp_Pkt.dp_Res1)
#define ARes2(a)	(a->pk.sp_Pkt.dp_Res2)

#endif	/* AIO_H */
