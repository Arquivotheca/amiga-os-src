
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/execbase.h>
#include <hardware/cia.h>
#include <resources/cia.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/cia_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/cia_pragmas.h>

#include "realtimebase.h"
#include "timerinit.h"


/*****************************************************************************/

/* Here's the poop on how the CIA timer is used by realtime.library.
 *
 * There are two possibilities:
 *
 * 1- realtime.library has a CIA handler which increments the current time
 *    counter, and then invokes a softint to process players waiting on
 *    time changes
 *
 * 2- realtime.library has a CIA handler which increments the current time
 *    counter, and processes players itself.
 *
 * Whether case 1 or 2 is used depends on which CPU is available, and
 * which CIA timer is available:
 *
 *  - if running on a 68000, approach #1 is always used
 *
 *  - if running on >= 68020 and a CIA timer of int priority 2 is available,
 *    then approach #2 is used
 *
 *  - if running on >= 68020 and only a CIA timer of int priority 6 is
 *    available, then approach #1 is used.
 *
 * This setup is to maximize response time, to minimize system interrupt
 * latency, and to attempt to avoid overruns in system interrupt handling,
 * by needlessly blocking interrupts for too long.
 *
 * Now, depending on which CPU is being used, the CIA is run at different
 * actual frequencies. On a 68000, the frequency is 300Hz, on a 68020
 * it's 600Hz, and on anything above it's 1200Hz (currently). Applications
 * expect a fixed frequency of 1200Hz. In order to accomodate this, the
 * tick counter is incremented by various amounts. On a 68000, it is
 * incremented by 4 for each CIA interrupt, 2 on a 68020, and 1 on
 * anything above. This has the effect of giving a constant clock of
 * 1200 ticks per second for the user. The only difference is a reduced
 * accuracy on slower processors.
 */


/*****************************************************************************/


/* nanosecond error from ideal tick length */
#define TICKERR(freq,base) (UWORD)((((long)CIAMod(freq,(base)) * (base) - (freq)) * (1000000000L/(base)) + (freq)/2) / (freq))

#define CIAMod(freq,base)  (UWORD)(((freq) + (base)/2) / (base))      /* CIA timer modulo */

/* softint.asm */
extern void SoftIntHandler();
extern void CIAHandler68020();
extern void CIAHandler68030();

/* hardware */
extern struct CIA far ciaa;
extern struct CIA far ciab;


/*****************************************************************************/


static BOOL AttemptCIA(STRPTR name, struct CIA *cia, UWORD mod, struct RealTimeLib *RealTimeBase)
{
    if (CiaBase = OpenResource(name))
    {
        mod--;       /* CIA count register appears to be N-1 for N states */

        if (!AddICRVector(CiaBase, CIAICRB_TA, &RealTimeBase->rtl_CIAInt))
        {
            RealTimeBase->rtl_CIAChannel = CIAICRB_TA;
            RealTimeBase->rtl_CIACtrlReg = &cia->ciacra;

                                              /* stop timer    PB6 normal     pulse output      continuous run    phase 2 clock */
            *RealTimeBase->rtl_CIACtrlReg &= ~(CIACRAF_START | CIACRAF_PBON | CIACRAF_OUTMODE | CIACRAF_RUNMODE | CIACRAF_INMODE);

            cia->ciatalo = (UBYTE)mod;
            cia->ciatahi = mod >> 8;

            return(TRUE);
        }

        if (!AddICRVector(CiaBase, CIAICRB_TB, &RealTimeBase->rtl_CIAInt))
        {
            RealTimeBase->rtl_CIAChannel = CIAICRB_TB;
            RealTimeBase->rtl_CIACtrlReg = &cia->ciacrb;

                                                                             /* stop timer    PB7 normal     pulse output      continuous run              (clear in bits)              phase 2 clock */
            *RealTimeBase->rtl_CIACtrlReg = *RealTimeBase->rtl_CIACtrlReg & ~(CIACRBF_START | CIACRBF_PBON | CIACRBF_OUTMODE | CIACRBF_RUNMODE | (CIACRBF_INMODE0 | CIACRBF_INMODE1)) | CIACRBF_IN_PHI2;

            cia->ciatblo = (UBYTE)mod;
            cia->ciatbhi = mod >> 8;

            return(TRUE);
        }
    }

    return(FALSE);
}


/*****************************************************************************/


LONG OpenTimer(struct RealTimeLib *RealTimeBase)
{
UWORD mod;
ULONG freq;

    if (!RealTimeBase->rtl_CIACtrlReg)
    {
        /* The CIA interrupt handler that's used in the combo cia/softint
         * setup is built up on the fly. This allows for faster code which
         * can use constants instead of variables stored in RAM. Kinda neat,
         * somewhat of a pain to update or change though. Anyway, the routine
         * essentially looks like:
         *
         *      movea.l     (a1)+,a6        ; load SysBase, a1 now points to rtb_TimerSoftInt
         *      addq.l      #X,Y(a5)        ; increases rtb_Time by X ticks
         *      jmp         _LVOCause(a6)   ; cause the timer soft int
         *
         * This is a total of 46 cycles on a 68000. On entry to the code, A1
         * points to &RealTimeBase->rtl_SysBase, after which must come
         * rtl_TimerSoftInt. Also on entry, A5 points to the code segment
         * itself. Since the code is actually in the library base, then
         * -Y(a5) of the code, gives a pointer to rtl_Time if Y is adjusted
         * appropriately.
         *
         * The binary equivalent of this code is copied into the CIAHandlerCode
         * array, and the values of X and Y are poked in.
         */

        RealTimeBase->rtl_CIAHandlerCode[0] = 0x2c59;  /* movea.l (a1)+,a6  */
        RealTimeBase->rtl_CIAHandlerCode[2] = (ULONG)&RealTimeBase->rtl_Time - (ULONG)&RealTimeBase->rtl_CIAHandlerCode[0];
        RealTimeBase->rtl_CIAHandlerCode[3] = 0x4eee;  /* jmp _LVOCause(a6) */
        RealTimeBase->rtl_CIAHandlerCode[4] = 0xff4c;

        if (SysBase->AttnFlags & AFF_68030)
        {
            freq = TICK_FREQ;
            mod  = CIAMod(ECLOCKFREQ(),freq);
            RealTimeBase->rtl_CIAInt.is_Data    = RealTimeBase;
            RealTimeBase->rtl_CIAInt.is_Code    = CIAHandler68030;
            RealTimeBase->rtl_CIAHandlerCode[1] = 0x52ad;  /* addq.l #1,Y(a5)   */
        }
        else if (SysBase->AttnFlags & AFF_68020)
        {
            freq = TICK_FREQ / 2;
            mod  = CIAMod(ECLOCKFREQ(),freq);
            RealTimeBase->rtl_CIAInt.is_Data    = RealTimeBase;
            RealTimeBase->rtl_CIAInt.is_Code    = CIAHandler68020;
            RealTimeBase->rtl_CIAHandlerCode[1] = 0x54ad;  /* addq.l #2,Y(a5)   */
        }
        else
        {
            freq = TICK_FREQ / 4;
            mod  = CIAMod(ECLOCKFREQ(),freq);
            RealTimeBase->rtl_CIAInt.is_Data    = &RealTimeBase->rtl_SysBase;
            RealTimeBase->rtl_CIAInt.is_Code    = RealTimeBase->rtl_CIAHandlerCode;
            RealTimeBase->rtl_CIAHandlerCode[1] = 0x58ad;  /* addq.l #4,Y(a5)   */
        }

        /* make sure the code we generated reaches memory */
        CacheClearU();

        RealTimeBase->rtl_TimeFrac                     = 0;         /* always 0 for now */
        RealTimeBase->rtl_TimeFreq                     = TICK_FREQ; /* must not change for compatibility */
        RealTimeBase->rtl_TickErr                      = TICKERR(ECLOCKFREQ(),freq);
        RealTimeBase->rtl_Message.pmt_Method           = PM_TICK;
        RealTimeBase->rtl_CIAInt.is_Node.ln_Type       = NT_INTERRUPT;
        RealTimeBase->rtl_CIAInt.is_Node.ln_Name       = RealTimeBase->rtl_Public.rtb_LibNode.lib_Node.ln_Name;
        RealTimeBase->rtl_TimerSoftInt.is_Node.ln_Type = NT_INTERRUPT;
        RealTimeBase->rtl_TimerSoftInt.is_Node.ln_Pri  = 32;
        RealTimeBase->rtl_TimerSoftInt.is_Node.ln_Name = RealTimeBase->rtl_Public.rtb_LibNode.lib_Node.ln_Name;
        RealTimeBase->rtl_TimerSoftInt.is_Data         = RealTimeBase;
        RealTimeBase->rtl_TimerSoftInt.is_Code         = SoftIntHandler;

        /* attempt to get a CIA timer channel */

        /* first, try a CIA interrupt level 2 timer, that's always better */
        if (AttemptCIA(CIAANAME,&ciaa,mod,RealTimeBase))
        {
        }
        else
        {
            RealTimeBase->rtl_CIAInt.is_Data = &RealTimeBase->rtl_SysBase;
            RealTimeBase->rtl_CIAInt.is_Code = RealTimeBase->rtl_CIAHandlerCode;

            /* so then, let's try for a level 6 timer */
            if (!AttemptCIA(CIABNAME,&ciab,mod,RealTimeBase))
                return(RTE_NOTIMER);
        }

        /* clear this after getting CIA because interrupt may have already gotten called */
        RealTimeBase->rtl_Time = 0;

        /* now start up the timer for real */
        *RealTimeBase->rtl_CIACtrlReg |= CIACRAF_LOAD | CIACRAF_START;   /* load and start timer */
    }

    return 0;
}


/*****************************************************************************/


VOID CloseTimer(struct RealTimeLib *RealTimeBase)
{
    if (RealTimeBase->rtl_CIACtrlReg)
    {
	*RealTimeBase->rtl_CIACtrlReg &= ~CIACRAF_START;  /* stop timer */
	RemICRVector(RealTimeBase->rtl_CIAResource, (WORD)RealTimeBase->rtl_CIAChannel, &RealTimeBase->rtl_CIAInt);
	RealTimeBase->rtl_CIACtrlReg = NULL;
    }

    RealTimeBase->rtl_Time = 0;
}
