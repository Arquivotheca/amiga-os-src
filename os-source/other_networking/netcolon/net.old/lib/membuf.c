
/*
 *  MEMBUF.C
 */

#include "netcomm.h"
#include "proto.h"
#include <proto/exec.h>

void
ResetPacketBuffer(rp)
struct RPacket *rp;
{
    long *ptr;

    BUG(("*******ResetPacketBuffer called! ************\n"))
    BUG(("*******ResetPacketBuffer called! ************\n"))
    BUG(("*******ResetPacketBuffer called! ************\n"))

    if (ptr = (long *)rp->DataP) {
	if (ptr[-1] == MINBUFSIZE)
	    return;
	FreeMem(ptr - 1, ptr[-1] + 4);
	rp->DataP = NULL;
    }
    SetPacketBuffer(rp, MINBUFSIZE);
}

void
SetPacketBuffer(rp, bytes)
struct RPacket *rp;
long bytes;
{
    long *ptr;

    BUG(("*******SetPacketBuffer called! ************\n"))
    BUG(("*******SetPacketBuffer called! ************\n"))
    BUG(("*******SetPacketBuffer called! ************\n"))

    bytes = (bytes + MINBUFSIZE - 1) & ~(MINBUFSIZE - 1);
    if (bytes < MINBUFSIZE)
	bytes = MINBUFSIZE;

    if (ptr = (long *)rp->DataP) {      /*  buffer already large enough */
	if (bytes <= ptr[-1])
	    return;
	FreeMem(ptr - 1, ptr[-1] + 4);
    }
    ptr = (long *)AllocMem(bytes + 4, MEMF_PUBLIC);
    if (ptr == NULL)
	;		/*  XXX */

    *ptr = bytes;
    rp->DataP = (char *)(ptr + 1);
}

void
ClearPacketBuffer(rp)
struct RPacket *rp;
{
    long *ptr;
    BUG(("*******ClearPacketBuffer called! ************\n"))
    BUG(("*******ClearPacketBuffer called! ************\n"))
    BUG(("*******ClearPacketBuffer called! ************\n"))
#if 0
crashing - removed 10mar90/djw
    if (ptr = (long *)rp->DataP) {
	FreeMem(ptr - 1, ptr[-1] + 4);
	rp->DataP = NULL;
    }
#endif
}
