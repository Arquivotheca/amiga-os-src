/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: cia8520.h,v 37.0 91/01/07 15:13:12 spence Exp $
*
*	$Locker:  $
*
*	$Log:	cia8520.h,v $
*   Revision 37.0  91/01/07  15:13:12  spence
*   initial switchover from V36
*   
*   Revision 33.3  90/07/27  16:25:54  bart
*   *** empty log message ***
*   
*   Revision 33.2  90/03/28  09:36:52  bart
*   *** empty log message ***
*   
*   Revision 33.1  89/05/02  09:28:53  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  14:58:04  bart
*   added to rcs for updating
*   
*
******************************************************************************/


/* cia.h -- definitions for the registers and bits in the Complex Interface
 * Adapter (CIA8520) chip
 */

/*
 * $Id: cia8520.h,v 37.0 91/01/07 15:13:12 spence Exp $
 *
 * $Locker:  $
 *
 */

#ifndef HARDWARE_CIA8520_H
#define HARDWARE_CIA8520_H

/*
 * ciaa is on an ODD address (e.g. the low byte) -- $bfe001
 * ciab is on an EVEN address (e.g. the high byte) -- $bfd000
 *
 * do this to get the definitions:
 *    extern struct CIA8520 ciaa, ciab;
 */


struct CIA8520 {
    UBYTE   pra;
    UBYTE   pad0[0xff];
    UBYTE   prb;
    UBYTE   pad1[0xff];
    UBYTE   ddra;
    UBYTE   pad2[0xff];
    UBYTE   ddrb;
    UBYTE   pad3[0xff];
    UBYTE   talo;
    UBYTE   pad4[0xff];
    UBYTE   tahi;
    UBYTE   pad5[0xff];
    UBYTE   tblo;
    UBYTE   pad6[0xff];
    UBYTE   tbhi;
    UBYTE   pad7[0xff];
    UBYTE   todlow;
    UBYTE   pad8[0xff];
    UBYTE   todmid;
    UBYTE   pad9[0xff];
    UBYTE   todhi;
    UBYTE   pad10[0xff];
    UBYTE   unused;
    UBYTE   pad11[0xff];
    UBYTE   sdr;
    UBYTE   pad12[0xff];
    UBYTE   icr;
    UBYTE   pad13[0xff];
    UBYTE   cra;
    UBYTE   pad14[0xff];
    UBYTE   crb;
};


/* interrupt control register bit numbers */
#define CIA8520ICRB_TA	0
#define CIA8520ICRB_TB	1
#define CIA8520ICRB_ALRM	2
#define CIA8520ICRB_SP	3
#define CIA8520ICRB_FLG	4
#define CIA8520ICRB_IR	7
#define CIA8520ICRB_SETCLR	7

/* control register A bit numbers */
#define CIA8520CRAB_START	0
#define CIA8520CRAB_PBON	1
#define CIA8520CRAB_OUTMODE 2
#define CIA8520CRAB_RUNMODE 3
#define CIA8520CRAB_LOAD	4
#define CIA8520CRAB_INMODE	5
#define CIA8520CRAB_SPMODE	6
#define CIA8520CRAB_TODIN	7

/* control register B bit numbers */
#define CIA8520CRBB_START	0
#define CIA8520CRBB_PBON	1
#define CIA8520CRBB_OUTMODE 2
#define CIA8520CRBB_RUNMODE 3
#define CIA8520CRBB_LOAD	4
#define CIA8520CRBB_INMODE0 5
#define CIA8520CRBB_INMODE1 6
#define CIA8520CRBB_ALARM	7
 
/* interrupt control register masks */
#define CIA8520ICRF_TA	(1<<CIA8520ICRB_TA)
#define CIA8520ICRF_TB	(1<<CIA8520ICRB_TB)
#define CIA8520ICRF_ALRM	(1<<CIA8520ICRB_ALRM)
#define CIA8520ICRF_SP	(1<<CIA8520ICRB_SP)
#define CIA8520ICRF_FLG	(1<<CIA8520ICRB_FLG)
#define CIA8520ICRF_IR	(1<<CIA8520ICRB_IR)
#define CIA8520ICRF_SETCLR	(1<<CIA8520ICRB_SETCLR)

/* control register A register masks */
#define CIA8520CRAF_START	(1<<CIA8520CRAB_START)
#define CIA8520CRAF_PBON	(1<<CIA8520CRAB_PBON)
#define CIA8520CRAF_OUTMODE (1<<CIA8520CRAB_OUTMODE)
#define CIA8520CRAF_RUNMODE (1<<CIA8520CRAB_RUNMODE)
#define CIA8520CRAF_LOAD	(1<<CIA8520CRAB_LOAD)
#define CIA8520CRAF_INMODE	(1<<CIA8520CRAB_INMODE)
#define CIA8520CRAF_SPMODE	(1<<CIA8520CRAB_SPMODE)
#define CIA8520CRAF_TODIN	(1<<CIA8520CRAB_TODIN)
 
/* control register B register masks */
#define CIA8520CRBF_START	(1<<CIA8520CRBB_START)
#define CIA8520CRBF_PBON	(1<<CIA8520CRBB_PBON)
#define CIA8520CRBF_OUTMODE (1<<CIA8520CRBB_OUTMODE)
#define CIA8520CRBF_RUNMODE (1<<CIA8520CRBB_RUNMODE)
#define CIA8520CRBF_LOAD	(1<<CIA8520CRBB_LOAD)
#define CIA8520CRBF_INMODE0 (1<<CIA8520CRBB_INMODE0)
#define CIA8520CRBF_INMODE1 (1<<CIA8520CRBB_INMODE1)
#define CIA8520CRBF_ALARM	(1<<CIA8520CRBB_ALARM)

/* control register B INMODE masks */
#define CIA8520CRBF_IN_PHI2 0
#define CIA8520CRBF_IN_CNT	(CIA8520CRBF_INMODE0)
#define CIA8520CRBF_IN_TA	(CIA8520CRBF_INMODE1)
#define CIA8520CRBF_IN_CNT_TA	(CIA8520CRBF_INMODE0|CIA8520CRBF_INMODE1)

/*
 * Port definitions -- what each bit in a cia peripheral register is tied to
 */

/* ciaa port A (0xbfe001) */
#define CIA8520B_GAMEPORT1	(7)	/* gameport 1, pin 6 (fire button*) */
#define CIA8520B_GAMEPORT0	(6)	/* gameport 0, pin 6 (fire button*) */
#define CIA8520B_DSKRDY	(5)	/* disk ready* */
#define CIA8520B_DSKTRACK0	(4)	/* disk on track 00* */
#define CIA8520B_DSKPROT	(3)	/* disk write protect* */
#define CIA8520B_DSKCHANGE	(2)	/* disk change* */
#define CIA8520B_LED	(1)	/* led light control (0==>bright) */
#define CIA8520B_OVERLAY	(0)	/* memory overlay bit */

/* ciaa port B (0xbfe101) -- parallel port */

/* ciab port A (0xbfd000) -- serial and printer control */
#define CIA8520B_COMDTR	(7)	/* serial Data Terminal Ready* */
#define CIA8520B_COMRTS	(6)	/* serial Request to Send* */
#define CIA8520B_COMCD	(5)	/* serial Carrier Detect* */
#define CIA8520B_COMCTS	(4)	/* serial Clear to Send* */
#define CIA8520B_COMDSR	(3)	/* serial Data Set Ready* */
#define CIA8520B_PRTRSEL	(2)	/* printer SELECT */
#define CIA8520B_PRTRPOUT	(1)	/* printer paper out */
#define CIA8520B_PRTRBUSY	(0)	/* printer busy */

/* ciab port B (0xbfd100) -- disk control */
#define CIA8520B_DSKMOTOR	(7)	/* disk motorr* */
#define CIA8520B_DSKSEL3	(6)	/* disk select unit 3* */
#define CIA8520B_DSKSEL2	(5)	/* disk select unit 2* */
#define CIA8520B_DSKSEL1	(4)	/* disk select unit 1* */
#define CIA8520B_DSKSEL0	(3)	/* disk select unit 0* */
#define CIA8520B_DSKSIDE	(2)	/* disk side select* */
#define CIA8520B_DSKDIREC	(1)	/* disk direction of seek* */
#define CIA8520B_DSKSTEP	(0)	/* disk step heads* */

/* ciaa port A (0xbfe001) */
#define CIA8520F_GAMEPORT1	(1<<7)
#define CIA8520F_GAMEPORT0	(1<<6)
#define CIA8520F_DSKRDY	(1<<5)
#define CIA8520F_DSKTRACK0	(1<<4)
#define CIA8520F_DSKPROT	(1<<3)
#define CIA8520F_DSKCHANGE	(1<<2)
#define CIA8520F_LED	(1<<1)
#define CIA8520F_OVERLAY	(1<<0)

/* ciaa port B (0xbfe101) -- parallel port */

/* ciab port A (0xbfd000) -- serial and printer control */
#define CIA8520F_COMDTR	(1<<7)
#define CIA8520F_COMRTS	(1<<6)
#define CIA8520F_COMCD	(1<<5)
#define CIA8520F_COMCTS	(1<<4)
#define CIA8520F_COMDSR	(1<<3)
#define CIA8520F_PRTRSEL	(1<<2)
#define CIA8520F_PRTRPOUT	(1<<1)
#define CIA8520F_PRTRBUSY	(1<<0)

/* ciab port B (0xbfd100) -- disk control */
#define CIA8520F_DSKMOTOR	(1<<7)
#define CIA8520F_DSKSEL3	(1<<6)
#define CIA8520F_DSKSEL2	(1<<5)
#define CIA8520F_DSKSEL1	(1<<4)
#define CIA8520F_DSKSEL0	(1<<3)
#define CIA8520F_DSKSIDE	(1<<2)
#define CIA8520F_DSKDIREC	(1<<1)
#define CIA8520F_DSKSTEP	(1<<0)

#endif HARDWARE_CIA8520_H
