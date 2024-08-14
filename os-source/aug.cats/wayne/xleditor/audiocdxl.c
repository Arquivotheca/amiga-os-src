/*************

    AudioCDXL.c

    W.D.L 930330

**************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */


// Tab size is 8!

#include <exec/types.h>
#include <graphics/gfxbase.h>

#include <devices/audio.h>
#include "devices/cd.h"

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>

#include <string.h>	// for setmem()

#include "cdxl/cdxlob.h"
#include "cdxl/runcdxl.h"
#include "cdxl/debugsoff.h"

/**/	// Uncomment to get debug output turned on
#define KPRINTF
//#include "cdxl/debugson.h"



IMPORT struct Custom far custom;

STATIC UBYTE		* Name0 = "CDXLAUD0";
STATIC struct Interrupt	* Aud0PriorInterrupt;
STATIC struct Interrupt	  AUD0Interrupt;
STATIC BOOL		  Aud0PriorEnable;
STATIC BOOL		  Aud1PriorEnable;
STATIC BOOL		  Aud0PriorSet;
STATIC BOOL		  Aud1PriorSet;
STATIC BOOL		  aud0_installed;


STATIC struct MsgPort	* AudioPort[4];
STATIC struct IOAudio	  IOAudio[4];
STATIC UBYTE		  Channels[] = {1,2,4,8};
STATIC BOOL		  DeviceOpen[4];

STATIC UWORD	Period;		// Global Period

IMPORT	CDXLOB * CDXL_OB;	// Global CDXLOB
IMPORT	LONG	XLSignal;
IMPORT	struct Process	* parent;
IMPORT  struct Task	* CDXLTask;

IMPORT	struct GfxBase	* GfxBase;


IMPORT	ULONG	Count;
ULONG		SaveCount,AudioCount;
WORD		padjust;

/*
 * The AUD0 interrupt code. This routine gets called immediately after
 * the audio DMA channel has read the location and length registers and
 * stored their values in the back up registers. The philospy here is to
 * rewrite the location registers to point to the other audio buffer in
 * preparation for the next time the audio DMA reads them. It seems that
 * the audio period that was specified is sometimes off by less than 1.
 * For long CDXL files, this will result in the audio getting out of
 * sync with the video. What I do here is add or subtract one from the
 * period to compensate for this. A better way to do this could probably 
 * be figured out.
 */
#ifndef	OUTT	// [

VOID __interrupt __saveds
AUD0Handler( VOID )
{
    // clear AUD0 interrupt
    custom.intreq = INTF_AUD0;

    ++AudioCount;

    if( !(CDXL_OB->flags & CDXL_DOSXL) ) {

	padjust = AudioCount - Count;
	custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;

	if ( (SaveCount == Count) ) {
	    CDXL_OB->curAudio ^= 1;
	    AudioCount--;
	} else if ( (Count - SaveCount) > 1) {
	    AudioCount++;
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	} else {
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	}

	D(PRINTF("%ld-%ld ",CDXL_OB->curAudio,padjust);)

	// Point the location registers to the buffer that is currently being filled
	// by the CDXL. If we are timed right, when the CDXL is done reading, the
	// audio DMA should be ready to reread these location registers.
	custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_OB->audio[CDXL_OB->curAudio];

    } else {
	// Point the location registers to the buffer that is currently being filled
	// by the CDXL. If we are timed right, when the CDXL is done reading, the
	// audio DMA should be ready to reread these location registers.
	padjust = (AudioCount - Count);
	custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;
	custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_OB->audio[CDXL_OB->curAudio];


	if ( (SaveCount == Count) ) {
	    CDXL_OB->curAudio ^= 1;
	    AudioCount--;
	} else if ( (Count - SaveCount) > 1) {
	    AudioCount++;
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	} else {
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	}

//	custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_OB->audio[CDXL_OB->curAudio];

	Signal( CDXLTask, XLSignal );
    }

    SaveCount = Count;

} // AUD0Handler()

#else		// ][
VOID __interrupt __saveds
AUD0Handler( VOID )
{
    // clear AUD0 interrupt
    custom.intreq = INTF_AUD0;

    if( !(CDXL_OB->flags & CDXL_DOSXL) ) {
	// Adjust Period if we are getting ahead or behind the CDXL transfer
	if ( SaveCount == Count ) {
	    // We hit this interrupt twice before we got the CDXL one, slow it down.
	    if ( padjust < 4 )
		padjust++;

	    custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;//Period + 1; //++Period;
	    CDXL_OB->curAudio ^= 1;
	} else if ( (Count - SaveCount) > 1) {
	    // We got the CDXL interrupt at least twice before the last time, speed
	    // it up and get the curAudio from curVideo.
	    if ( padjust > -4 )
		padjust--;

	    custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;//Period - 1; //--Period;
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	} else {
	    // Every thing seems to be in sync. No need to adjust the period.
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	}

	SaveCount = Count;

	// Point the location registers to the buffer that is currently being filled
	// by the CDXL. If we are timed right, when the CDXL is done reading, the
	// audio DMA should be ready to reread these location registers.
	custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_OB->audio[CDXL_OB->curAudio];

    } else {

	// Point the location registers to the buffer that is currently being filled
	// by the CDXL. If we are timed right, when the CDXL is done reading, the
	// audio DMA should be ready to reread these location registers.
	custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_OB->audio[CDXL_OB->curAudio];

	// Adjust Period if we are getting ahead or behind the CDXL transfer
	if ( SaveCount == Count ) {
	    // We hit this interrupt twice before we got the CDXL one, slow it down.
	    if ( padjust < 4 )
		padjust++;

	    custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;//Period + 1; //++Period;
	    CDXL_OB->curAudio ^= 1;
	} else if ( (Count - SaveCount) > 1) {
	    // We got the CDXL interrupt at least twice before the last time, speed
	    // it up and get the curAudio from curVideo.
	    if ( padjust > -4 )
		padjust--;

	    custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = Period + padjust;//Period - 1; //--Period;
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	} else {
	    // Every thing seems to be in sync. No need to adjust the period.
	    CDXL_OB->curAudio = CDXL_OB->curVideo;
	}

	SaveCount = Count;

	Signal( (struct Task *)parent, XLSignal );
    }

} // AUD0Handler()
#endif		// ]

/*
 * Free the audio channels.
 */
STATIC VOID
FreeAudio( VOID )
{
    int i;

    for ( i = 0; i < 4; i++ ) {
	if ( DeviceOpen[i] ) {
	    CloseDevice( (struct IORequest *)&IOAudio[i] );
	    DeviceOpen[i] = NULL;
	    D(PRINTF("Closing channel %ld\n",i);)
	} else {
	    D(PRINTF("NOT Closing channel %ld\n",i);)
	}

	if ( AudioPort[i] ) {
	    DeleteMsgPort( AudioPort[i] );
	    AudioPort[i] = NULL;
	    D(PRINTF("Deleting Port %ld\n",i);)
	} else {
	    D(PRINTF("NOT Deleting Port %ld\n",i);)
	}
    }

} // FreeAudio()


/*
 * Allocate the audio channels. Probably don't need to allocate all 4 but...
 */
STATIC
AllocAudio( VOID )
{
    int	i,ret = RC_OK;

    D(PRINTF("AllocAudio() 1\n");)

    for ( i = 0; i < 4; i++ ) {
	setmem( &IOAudio[i], sizeof ( struct IOAudio ), 0 );

	D(PRINTF("AllocAudio() 2\n");)

	if ( AudioPort[i] = CreateMsgPort() ) {
	    IOAudio[i].ioa_Request.io_Message.mn_ReplyPort = AudioPort[i];
	    IOAudio[i].ioa_Request.io_Message.mn_Node.ln_Pri = 127;
	    IOAudio[i].ioa_AllocKey = 0;
	    IOAudio[i].ioa_Data = &Channels[i];
	    IOAudio[i].ioa_Length = 1;
	    if ( !OpenDevice("audio.device",0,(struct IORequest *)&IOAudio[i],0) ) {
		DeviceOpen[i] = TRUE;
		D(PRINTF("Got channel %ld\n",i);)
	    } else {
		DeviceOpen[i] = FALSE;
		D(PRINTF("Did NOT get channel %ld\n",i);)
		ret = RC_NO_AUDIODEVICE;
		break;
	    }
	} else {
	    ret = RC_NO_MEM;
	    break;
	}
    }

    D(PRINTF("AllocAudio() 3\n");)

    if ( ret )
	FreeAudio();

    D(PRINTF("AllocAudio() END ret= %ld\n",ret);)

    return( ret );

} // AllocAudio()


/*
 * Disable AUD0 & AUD1 DMA as well as the AUD0 interrupt.
 */
VOID
StopAudio( VOID )
{
    // disable AUD0 DMA
    custom.dmacon = DMAF_AUD0;

    // disable AUD1 DMA
    custom.dmacon = DMAF_AUD1;

    // disable AUD0 interrupt
    custom.intena = INTF_AUD0;

    // clear AUD0 interrupt
    custom.intreq = INTF_AUD0;

} // StopAudio()


/*
 * Enable AUD0 & AUD1 DMA as well as the AUD0 interrupt.
 */
VOID
StartAudio( VOID )
{
    // clear AUD0 interrupt
    custom.intreq = INTF_AUD0;

    // enable AUD0 DMA
    custom.dmacon = DMAF_SETCLR|DMAF_MASTER|DMAF_AUD0;

    // enable AUD1 DMA
    custom.dmacon = DMAF_SETCLR|DMAF_MASTER|DMAF_AUD1;

    // enable AUD0 interrupt
    custom.intena = INTF_SETCLR|INTF_AUD0;

    AudioCount = 0;

} // StartAudio()


/*
 * Restore the audio system back to how we found it.
 */
VOID
QuitAudio( VOID )
{
    if ( aud0_installed ) {
	aud0_installed = FALSE;

	StopAudio();

	/*
	 * Remove the interrupt we installed and replace it
	 * with the old one (if there was one).
	 */
	SetIntVector(INTB_AUD0, Aud0PriorInterrupt);

	if(Aud0PriorEnable) {
	    custom.intena = INTF_SETCLR|INTF_AUD0;
	    Aud0PriorEnable = FALSE;
	}

	if(Aud1PriorEnable) {
	    custom.intena = INTF_SETCLR|INTF_AUD1;
	    Aud1PriorEnable = FALSE;
	}

	if ( Aud0PriorSet ) {
	    custom.intreq = INTF_AUD0;
	    Aud0PriorSet = FALSE;
	}

	if ( Aud1PriorSet ) {
	    custom.intreq = INTF_AUD1;
	    Aud1PriorSet = FALSE;
	}
    }

    // Free the audio channels we allocated.
    FreeAudio();

} // QuitAudio()


/*
 * Install an AUD0 interrupt
 */
AddAudioInterrupt( VOID )
{
    setmem( &AUD0Interrupt, sizeof ( AUD0Interrupt ), 0 );

    /* Initialize the Interrupt node */
    AUD0Interrupt.is_Node.ln_Type = NT_INTERRUPT;
    AUD0Interrupt.is_Node.ln_Pri  = 0;
    AUD0Interrupt.is_Node.ln_Name = Name0;

    AUD0Interrupt.is_Data = (APTR)NULL;
    AUD0Interrupt.is_Code = AUD0Handler;

    custom.intena = INTF_AUD0|INTF_AUD1;
    custom.intreq = INTF_AUD0|INTF_AUD1;

    // Install the new interrupt handler
    Aud0PriorInterrupt = SetIntVector( INTB_AUD0, &AUD0Interrupt );

    if (Aud0PriorInterrupt) {
	D(PRINTF("Replaced the %ls AUD0 interrupt handler\n",
	    Aud0PriorInterrupt->is_Node.ln_Name);)
    }

    return( (int)(aud0_installed = TRUE) );

} // AddAudioInterrupt()


/*
 * Set up the audio system.
 */
InitAudio( CDXLOB * CDXL_ob )
{
    ULONG	freq;
    LONG	rate;
    UWORD	period;
    int		ret;

    D(PRINTF("InitAudio() 1\n");)

    // If this CDXL has no audio, there is no need.
    if ( !CDXL_ob->AudioSize )
	return( RC_OK );

    D(PRINTF("InitAudio() 2\n");)

    if ( ret = AllocAudio() )
	return( ret );

    D(PRINTF("InitAudio() 3\n");)

    // Save state of AUD0 & AUD1 interrupts
    Aud0PriorEnable = custom.intenar & INTF_AUD0 ? TRUE : FALSE;
    Aud1PriorEnable = custom.intenar & INTF_AUD1 ? TRUE : FALSE;
    Aud0PriorSet = custom.intreqr & INTF_AUD0 ? TRUE : FALSE;
    Aud1PriorSet = custom.intreqr & INTF_AUD1 ? TRUE : FALSE;

    D(PRINTF("InitAudio() 4\n");)

    // Add the Audio interrupt
    AddAudioInterrupt();

    D(PRINTF("InitAudio() 5\n");)

    // Calculate period
    freq = ( SYSTEM_PAL ? PAL_FREQ : NTSC_FREQ );

    D(PRINTF("InitAudio() 4.1 freq= %ld, CDXL_ob->AudioSize= %ld, CDXL_ob->FrameSize= %ld\n",
	freq,CDXL_ob->AudioSize,CDXL_ob->FrameSize);)

    if ( CDXL_ob->flags & CDXL_DOSXL) {
	rate = INTDIV( (( CDXL_ob->ReadXLSpeed * DEFAULT_SECTOR_SIZE) * CDXL_ob->AudioSize ), CDXL_ob->FrameSize );
    } else {
	rate = INTDIV( ( DATA_TRANS_RATE * CDXL_ob->AudioSize ), CDXL_ob->FrameSize );
    }

    D(PRINTF("InitAudio() 4.2 rate= %ld\n",rate);)

    period = (UWORD) INTDIV( freq, rate );

    Period = period;	// Set Global pointer
    padjust = 0;	// Global period adjuster.

    CDXL_ob->curAudio = 0;

    // Set up the volume,period and length
    custom.aud[ 0 ].ac_vol = custom.aud[ 1 ].ac_vol = CDXL_ob->Volume;
    custom.aud[ 0 ].ac_per = custom.aud[ 1 ].ac_per = period;
    custom.aud[ 0 ].ac_ptr = custom.aud[ 1 ].ac_ptr = (USHORT *)CDXL_ob->audio[CDXL_ob->curAudio];
    custom.aud[ 0 ].ac_len = custom.aud[ 1 ].ac_len = ( CDXL_ob->AudioSize / sizeof( UWORD) );

    D(PRINTF("InitAudio() END\n");)

    return( RC_OK );

} // InitAudio()
