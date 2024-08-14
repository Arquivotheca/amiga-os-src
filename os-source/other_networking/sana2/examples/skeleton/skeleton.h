/*
**  $Id: skeleton.h,v 1.1 91/10/29 11:51:20 dlarson Exp $
**
**  SANA-II driver utility skeleton header.
**
**  Copyright 1991 Commodore-Amiga, Inc.
**
**  This code may be modified and used freely on Amiga computers.
**
*/

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <devices/sana2.h>

/*
**  globals
*/
extern struct Library *SysBase;
extern struct MsgPort *DevPort;
extern struct IOSana2Req *IOB1;
extern int    DeviceOpen;
extern long   DevBits;
/*
**  prototypes
*/
int initdev(void);
void closedev(void);
BOOL CopyToBuff(VOID *in, VOID *out, LONG n);
BOOL CopyFromBuff(VOID *in, VOID *out, LONG n);

/*
**  To make life easy on non-sending/receiving utilities
*/
#ifdef NOBUFFS
BOOL CopyToBuff(void *foo, void *bar, long n){return(FALSE);}
BOOL CopyFromBuff(void *foo, void *bar, long n){return(FALSE);}
#endif
