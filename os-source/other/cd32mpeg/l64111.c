/*
** $Header: HOG:Other/cd32mpeg/RCS/l64111.c,v 40.4 94/01/26 11:56:05 kcd Exp $
**
** Amiga MPEG Device Driver
**
** L64111 Specific Code
**
*/

#include <exec/types.h>
#include "mpeg_device.h"
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#ifndef _GENPROTO
#include "l64111_protos.h"
#endif

VOID InitL64111(struct MPEGUnit *mpegUnit)
{
    L64111Regs *l64111Regs = mpegUnit->mu_L64111;

    WrL64111Register(Control_1, ANC_DATA_FIFO_RST | BUF_RST | SOFT_RST);
    {
         int i;
         volatile UBYTE delay;

         for(i = 0; i < 10000; i++)
            delay = *(volatile UBYTE *) 0xbfe001;
    }
    WrL64111Register(Control_1, 0);
    {
         int i;
         volatile UBYTE delay;

         for(i = 0; i < 10000; i++)
            delay = *(volatile UBYTE *) 0xbfe001;
    }
    WrL64111Register(Control_2, SEL_16 | PCM48FS);
    SetL64111AudioID(mpegUnit);
    WrL64111Register(Status_Int_1, 0);
    WrL64111Register(Status_Int_2, 0);
    WrL64111Register(Timer_CountDown, 4);

    /* The remainder values need to be zero for oversampling DAC. */

    WrL64111Register(Timer_OffsetHi, 0);
    WrL64111Register(Timer_OffsetLo, 0);
    return;
}

VOID SetL64111StatusInt1(UWORD intbits, struct MPEGUnit *mpegUnit)
{
    mpegUnit->mu_L64111Int1Bits = intbits;
    WriteL64111Register(Status_Int_1, intbits);
    return;
}

VOID SetL64111StatusInt2(UWORD intbits, struct MPEGUnit *mpegUnit)
{
    mpegUnit->mu_L64111Int2Bits = intbits;
    WriteL64111Register(Status_Int_2, intbits);
    return;
}

VOID SetL64111AudioID(struct MPEGUnit *mpegUnit)
{
    L64111Regs *l64111Regs = mpegUnit->mu_L64111;

    if(mpegUnit->mu_AudioID == 0xffff)
        WrL64111Register(Control_3, 0x80);
    else
    	WrL64111Register(Control_3, mpegUnit->mu_AudioID & 0x1f);
}

VOID PlayL64111(UWORD streamType, struct MPEGUnit *mpegUnit)
{
    L64111Regs *l64111Regs = mpegUnit->mu_L64111;

    if(streamType == MPEGSTREAM_AUDIO)
    {
        WrL64111Register(Control_2, SEL_16 | AUDIO_ONLY | PCM64FS);
    }
    else
    {
      WrL64111Register(Control_2, SEL_16 | PCM64FS);
    }
    WrL64111Register(Control_1, DEC_STRT | RdL64111Register(Control_1));
    return;
}

VOID PauseL64111(struct MPEGUnit *mpegUnit)
{
    WriteL64111Register(Control_1, ReadL64111Register(Control_1) & ~DEC_STRT);
    return;
}
