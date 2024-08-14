/*****************************************************************************
 *
 * VT100 Beep Sound
 *
 * Sam Dicker
 * 3 January 1986
 * (created:  19 December 1985)
 *
 * This module makes a beep sound like the CTRL-G on a VT100 terminal.
 *
 * Calling VTBeep(flag) starts the sound which stops when it times out.
 * Calling it again, while it is playing, only restarts the timer.
 * Call KillVTBeep() to kill any beeps in progress before exiting your
 * main program.
 *
 * VTBeep() and KillVTBeep() are re-entrant but cannot be used in interrupt
 * code.
 *
 * Note: the flag for VTBeep signifies if the beep is disabled or not.
 * (ie. if flag then beep else no beep)
 *****************************************************************************/

#include "exec/types.h"
#include "exec/errors.h"
#include "exec/memory.h"
#include "devices/timer.h"
#include "devices/audio.h"
#include "libraries/dos.h"

#include "dostags.h"
#include "tagitem.h"

#define CreateBEEP_IO CreateExtIO
#define DeleteBEEP_IO DeleteExtIO

#define BEEPNAME	"VTBeep"
#define BEEPSIZE	10
#define BEEPFREQ	1000
#define SIGB_BEEP	31
#define SIGF_BEEP	(1 << 31)
#define SIGF_PORT	(1 << replyPort->mp_SigBit)
#define COLORCLOCK	3579545

/* channel allocation map (try left channel; if unavailable try right) */
UBYTE allocationMap[] = { 1, 8, 2, 4 };

struct Seg
    {
	ULONG next;
	UWORD instr;
	ULONG addr;
	UWORD end;
    } *beepSeg;

/* beep sound child task */

beepTask()
{
    struct MsgPort *replyPort;
    struct timerequest *timerIOB;
    struct IOAudio *audioIOB;
    UBYTE *beepWave;
    ULONG signals;
    BYTE error;
    ULONG temp1, temp2;

    FreeMem(beepSeg,16L);

    /* allocate signal used to re-start beep */
    if (AllocSignal(SIGB_BEEP) == SIGB_BEEP) {

	/* create reply port for timer and sound I/O block */
	replyPort = (struct MsgPort *)CreatePort(0,0);

	if (replyPort != NULL) {

	    /* create timer I/O block */
	    timerIOB = (struct timerequest *)
		    CreateBEEP_IO(replyPort, sizeof(*timerIOB));

	    if (timerIOB != NULL) {

		/* open timer device */
		if (OpenDevice(TIMERNAME, UNIT_VBLANK, timerIOB, 0) == 0) {

		    timerIOB->tr_node.io_Command = TR_ADDREQUEST;

		    /* create beep waveform */

		    beepWave = (UBYTE *)AllocMem(BEEPSIZE,
			    MEMF_CHIP | MEMF_CLEAR);
		    if (beepWave != 0) {
			beepWave[0] = 100;


			/* create audio I/O block */
			audioIOB = (struct IOAudio *)
				CreateBEEP_IO(replyPort, sizeof(*audioIOB));/* */
			if (audioIOB != NULL) {

			    /* setup audio I/O block to allocate a channel
			     * when the audio device is opened */

			    audioIOB->ioa_Request.io_Message.mn_Node.ln_Pri =
				    85;
			    audioIOB->ioa_Data = allocationMap;
			    audioIOB->ioa_Length = sizeof(allocationMap);


    /* open the audio device */

    if ( (error=OpenDevice(AUDIONAME, 0, audioIOB, 0)) == (BYTE)0)
	{
	    /* setup the audio I/O block to play sound */

	    audioIOB->ioa_Request.io_Command = CMD_WRITE;
	    audioIOB->ioa_Request.io_Flags = ADIOF_PERVOL;
	    audioIOB->ioa_Data = beepWave;
	    audioIOB->ioa_Length = BEEPSIZE;
	    audioIOB->ioa_Period = COLORCLOCK / (BEEPSIZE * BEEPFREQ);
	    audioIOB->ioa_Volume = 64;

	    /* start the sound */

	    BeginIO(audioIOB);

	    /*  from this point on the task in cannot
		be pre-empted.  This prevents the
		parent task from signaling it to
		restart the timer while it is cleaning
		up */
			    
	    Forbid();

	    do {

		    /* start the timer */

		    timerIOB->tr_time.tv_secs = 0;
		    timerIOB->tr_time.tv_micro = 250000;
		    SendIO(timerIOB);

				    
		    /* wait for:
			. the timer to time out,
			. the audio I/O block to return
			  (indicating an error had occurred),
			. a signal to restart to timer, or
			. a signal to quit */

		    do 	signals = Wait(SIGBREAKF_CTRL_C |
			SIGF_BEEP | SIGF_PORT);
		    while ((signals & SIGF_PORT) != 0 &&
			    CheckIO(timerIOB) == 0 &&
			    CheckIO(audioIOB) == 0);


		    /* if the timer is still going, kill it */

		    if (CheckIO(timerIOB) == 0)
			{
			    AbortIO(timerIOB);
			    WaitIO(timerIOB);
			}


			/* restart the timer if signaled */
		} while ((signals & SIGF_BEEP) != 0);

	    /* clean up */

	    /*  closing the audio device kills the sound
	     *	and frees the channels */

	    CloseDevice(audioIOB);
	}


			    DeleteBEEP_IO(audioIOB);/* */
			}
			FreeMem(beepWave, BEEPSIZE);
		    }
		    CloseDevice(timerIOB);
		}
		DeleteBEEP_IO(timerIOB);/* */
	    }
	    DeletePort(replyPort);
	}
    }
}


	

/* make beep sound */

VTBeep(flag)
UBYTE flag;
{
    struct Task *beepTCB;

    if (!flag) return(0); /* exit if beep disabled */

    /* prevent beep child task, if it already exists, from going away before
     * it is signaled */
    Forbid();

    /* find the task by name */
    beepTCB = (struct Task *)FindTask(BEEPNAME);

    /* check if the task exists */
    if (beepTCB == NULL)
    {
	/* it doesn't exist so create it */

	beepSeg = (struct Seg *)AllocMem(16L,MEMF_PUBLIC);

	beepSeg->next = NULL;
	beepSeg->instr = (UWORD)0x4EF9;
	beepSeg->addr = (ULONG)beepTask;
	beepSeg->end = (UWORD)NULL;

	beepTCB = (struct Task *)CreateProc(BEEPNAME,25,((ULONG)beepSeg)>>2,4000);
    }
    else

	/* it already exist so signal it so restart it's timer */
	Signal(beepTCB, SIGF_BEEP);

    Permit();

    /* return sucess */
    return(beepTCB != NULL);
}

	

/* kill any beep sounds in progress.  This is necessary before exiting the
 * main program; otherwise, if a beep is playing, when the beep times out
 * and the child task wakes up its code segment may be gone */

KillVTBeep()
{
    struct Task *beepTCB;

    do {

	/* prevent beep child task, if it already exists, from going away
	 * before it is signaled */
	Forbid();

	/* find the task by name */
	beepTCB = (struct Task *)FindTask(BEEPNAME);

	/* check if the task exists */
	if (beepTCB != NULL) {

	    /* it already exist so signal it so go away */
	    Signal(beepTCB, SIGBREAKF_CTRL_C);

	    /* give it a chance to wake up, if it is lower priority */
	    Delay(10);

	}
	Permit();

    /* if it existed, kill it again */
    } while (beepTCB != NULL);
}
