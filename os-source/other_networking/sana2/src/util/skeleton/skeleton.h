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
BOOL __asm CopyToBuff(register __a0 VOID *in, register __a1 VOID *out, register __d0 ULONG n);
BOOL __asm CopyFromBuff(register __a0 VOID *in, register __a1 VOID *out, register __d0 ULONG n);

/*
**  To make life easy on non-sending/receiving utilities
*/
#ifdef NOBUFFS
BOOL __asm CopyToBuff(register __a0 void *foo, register __a1 void *bar, register __d0 ULONG n);
BOOL __asm CopyFromBuff(register __a0 void *foo, register __a1 void *bar, register __d0 ULONG n);

BOOL __asm CopyToBuff(register __a0 void *foo, register __a1 void *bar, register __d0 ULONG n)
{
    return(FALSE);
}
BOOL __asm CopyFromBuff(register __a0 void *foo, register __a1 void *bar, register __d0 ULONG n)
{
    return(FALSE);
}
#endif
