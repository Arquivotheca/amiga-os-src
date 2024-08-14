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
