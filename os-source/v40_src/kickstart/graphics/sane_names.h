/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: sane_names.h,v 39.0 91/08/21 17:12:34 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	sane_names.h,v $
*   Revision 39.0  91/08/21  17:12:34  chrisg
*   Bumped
*   
*   Revision 37.1  91/03/08  18:06:04  spence
*   audhard
*   
*   Revision 37.0  91/01/07  15:15:09  spence
*   initial switchover from V36
*   
*   Revision 33.6  90/07/27  16:32:01  bart
*   *** empty log message ***
*   
*   Revision 33.5  90/03/28  09:37:00  bart
*   *** empty log message ***
*   
*   Revision 33.4  89/05/02  09:30:34  bart
*   copyright 1989
*   
*   Revision 33.3  88/12/13  12:27:40  bart
*   *** empty log message ***
*   
*   Revision 33.2  88/06/23  18:39:40  dale
*   cp
*   
*   Revision 33.1  87/10/30  10:03:26  dale
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  14:58:22  bart
*   added to rcs for updating
*   
*
******************************************************************************/

/*#include "audhard.h"*/

#ifndef _SPRITEDEF
#define _SPRITEDEF
struct hsprdef
{
    short pos;
    short ctl;
    short dataa;
    short datab;
};
#endif

struct AudHard {
    WORD *start;    /* ptr to start of waveform data */
    UWORD length;   /* length of waveform in words */
    UWORD period;   /* sample period */
    WORD volume;    /* volume */
    WORD sample;    /* sample pair */
    LONG dummy;     /* unused */
};

struct AmigaRegs
{
	USHORT	ddata;
	USHORT	dmaconr;
	USHORT	vposr,vhposr;
	USHORT	dskdatr;
	USHORT	joy0dat,joy1dat;
	USHORT	clxdat;
	USHORT	adkconr;	
	USHORT	pot0dat,pot1dat;
	USHORT	potinp;
	USHORT	serdatr;
	USHORT	dskbytr;
	USHORT	intenar,intreqr;

	USHORT	*dskpt;
	USHORT	dsklen;
	USHORT	dskdat;
	USHORT	refpt;
	USHORT	vposw,vhposw;
	USHORT	copcon;
	USHORT	serdat;
	USHORT	serper;
	USHORT	potgo;
	USHORT	joytest;
	USHORT	ioskip1[4];

	USHORT	bltcon0,bltcon1,fwmask,lwmask;
	UBYTE	*bltptc,*bltptb,*bltpta,*bltptd;
	USHORT	bltsize;
	USHORT	bltcon0l;
	USHORT	bltsizv,bltsizh;
	SHORT	bltmdc,bltmdb,bltmda,bltmdd;
	SHORT	ioskip3[4];
	USHORT	cdata,bdata,adata;
	SHORT	ioskip4[3];

    UWORD deniseid;   /* 1e4 */
	USHORT	dsksync;
	USHORT	*cop1ptr,*cop2ptr;
	USHORT	cop1jmp,cop2jmp;
	USHORT	copins;
	USHORT	diwstrt,diwstop;
	USHORT	ddfstrt,ddfstop;
	USHORT	dmaconw;
	USHORT	clxconw;
	USHORT	intenaw,intreqw;
	USHORT	adkconw;

	struct	AudHard	audregs[4];
	UWORD	*bplpt[8];
	UWORD	bplcon0,bplcon1,bplcon2;
	UWORD	ioskip5;
	UWORD	bpl1mod,bpl2mod;
	UWORD	bplhmod;
	UWORD	ioskip6[1];
	UWORD	bpldat[8];
	UWORD	*sprpt[8];
	struct	hsprdef	hsprtdef[8];
	UWORD	color00[32];
    UWORD htotal;
    UWORD hsstop;
    UWORD hbstrt;
    UWORD hbstop;
    UWORD vtotal;
    UWORD vsstop;
    UWORD vbstrt;
    UWORD vbstop;
    UWORD sprhstrt;
    UWORD sprhstop;
    UWORD bplhstrt;
    UWORD bplhstop;
    UWORD hhposw;
    UWORD hhposr;
    UWORD beamcon0;
    UWORD hsstrt;
    UWORD vsstrt;
    UWORD hcenter;
	USHORT	diwhigh;
};
#ifdef	HARDWARE_INTBITS_H
#define INTF_MASTER	INTF_INTEN
#endif
