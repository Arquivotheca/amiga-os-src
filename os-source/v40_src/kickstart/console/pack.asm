**
**	$Id: pack.asm,v 1.4 92/04/03 13:13:44 darren Exp $
**
**      Downcode of pack.c (pack, unpack character map - see pack.c
**	code below following END.  Downcode for speed, and size! )
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	INCLUDE	"exec/types.i"
	INCLUDE	"conmap.i"
	INCLUDE "debug.i"

DEBUG_DETAIL	SET	0

EXTP		EQUR	A0
LOCP		EQUR	A1
CM		EQUR	A2
BUFA		EQUR	A3
BUFC		EQUR	A4
DISPC		EQUR	A5
DISPA		EQUR	A6

DISPLAYYL	EQUR	D2
DISPLAYXL	EQUR	D3
LIMITY		EQUR	D4
Y		EQUR	D5

* reserve D6-D7 for temps

    STRUCTURE	PackLocals,0
	UWORD	saveAttr
	UWORD	limitX
	UWORD	limitCPX
	UWORD	implicitNL
	LABEL	PackLocals_SIZEOF

		XDEF	_packSorted
		XDEF	_unpackSortedLine

*********************************************************************
* void packSorted(cm, displayXL, displayYL, cpX, cpY, lastAttr)
* struct ConsoleMap *cm;
* ULONG displayXL, displayYL;
* ULONG cpX, cpY;
* ULONG lastAttr;
* {

lastAttr	SET	24
cpY		SET	20
cpX		SET	16
displayYL	SET	12
displayXL	SET	8
cm		SET	4

_packSorted:

	PRINTF	DBG_ENTRY,<'_packSorted()'>

		move.l	sp,EXTP		;passed in

		sub.l	#PackLocals_SIZEOF,sp
		move.l	sp,LOCP		;locals

		movem.l	d2-d7/a2-a6,-(sp)

		movea.l	cm(EXTP),CM			;registerize
		move.l	displayXL(EXTP),DISPLAYXL
		move.l	displayYL(EXTP),DISPLAYYL

* saveAttr = lastAttr & CMAM_BGPEN

		move.w	lastAttr+2(EXTP),saveAttr(LOCP)
		andi.w	#CMAM_BGPEN,saveAttr(LOCP)
		
	PRINTF	DBG_FLOW,<'cm=$%lx displayXL=%ld displayYL=%ld'>,CM,DISPLAYXL,DISPLAYYL

	PRINTF	DBG_FLOW,<'lastAttr=$%lx'>,lastAttr(EXTP)

* limitY = (displayYL > cpY) ? displayYL : cpY

		move.l	DISPLAYYL,LIMITY

		move.l	cpY(EXTP),d0

		cmp.l	d0,DISPLAYYL
		bhi.s	dyGTcy

		move.l	d0,LIMITY
dyGTcy:

	PRINTF	DBG_FLOW,<'cpY=%ld cpX=%ld limitY=%ld'>,cpY(EXTP),cpX(EXTP),LIMITY

* for (y=0; y <= limitY; y++)

		moveq	#00,Y
		bra	termYloop
nextYloop:

	PRINTF	DBG_FLOW,<'y=%ld'>,Y

* find destination
*  bufC = (UBYTE *) (cm->cm_BufferStart +
*       (cm->cm_BufferWidth * cm->cm_BufferYL) + cm->cm_BufferXL);
*  bufA = (WORD *) (((LONG) bufC) << 1);
*  bufC += cm->cm_AttrToChar;

		movea.l	cm_BufferStart(CM),BUFC
		move.w	cm_BufferWidth(CM),d0
		mulu.w	cm_BufferYL(CM),d0
		adda.l	d0,BUFC
		adda.w	cm_BufferXL(CM),BUFC

		move.l	BUFC,d0
		add.l	d0,d0				;<<1
		movea.l	d0,BUFA

		move.l	cm_AttrToChar(CM),d1
		adda.l	d1,BUFC				;bufC += cm_AttrToChar

	PRINTF	DBG_FLOW,<'  bufC=$%lx bufA=$%lx'>,BUFC,BUFA

* if( y <= displayYL)

		cmp.w	DISPLAYYL,Y
		bhi	yGTdy


* find source
*  	dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y));
*	dispA = (WORD *) (((LONG) dispC) << 1);
*	dispC += cm->cm_AttrToChar;
*	implicitNL = *dispA & CMAF_IMPLICITNL;
*

		move.l	cm_DisplayStart(CM),DISPC
		move.w	cm_DisplayWidth(CM),d0
		mulu	Y,d0
		adda.l	d0,DISPC

		move.l	DISPC,d0
		add.l	d0,d0				;<<1
		move.l	d0,DISPA

		adda.l	d1,DISPC			;+ cm_AttrToChar

		move.w	(DISPA),d0
		andi.w	#CMAF_IMPLICITNL,d0		;implicitNL = *dispA &..
		move.w	d0,implicitNL(LOCP)

	PRINTF	DBG_FLOW,<'  dispC=$%lx dispA=$%lx'>,DISPC,DISPA

	PUSHWORD	D0
	PRINTF	DBG_FLOW,<'  implicitNL=$%lx'>
	POPLONG		1

*	    /* ensure no overlap with destination */
*	    if (((long) bufC) + cm->cm_BufferWidth > ((long) dispC)) {

		moveq	#00,d0
		move.w	cm_BufferWidth(CM),d0
		add.l	BUFC,d0

		cmp.l	DISPC,d0
		BLS_S	notoverlap

	PRINTF	DBG_FLOW,<'  Slow Scroll'>
	PRINTF	DBG_FLOW,<'  bufC+BufferWidth $%lx dispC $%lx'>,D0,DISPC

*		/* scroll buffer (painfully) */
*		sdC = (UBYTE *) cm->cm_BufferStart;
*		sdA = (WORD *) (((LONG) sdC) << 1);
*		sdC += cm->cm_AttrToChar;
*		ssA = sdA + cm->cm_BufferWidth;
*		ssC = sdC + cm->cm_BufferWidth;

		movem.l	BUFA/BUFC/DISPC/DISPA,-(sp)

		move.l	cm_BufferStart(CM),d0
		movea.l	d0,a3			;sdC
		add.l	d0,d0
		movea.l	d0,a4			;sdA
		adda.l	d1,a3			;sdC

		move.w	cm_BufferWidth(CM),d0

		movea.l	a4,a6			;ssA
		adda.w	d0,a6			;+BufferWidth
		adda.w	d0,a6			;(UWORD *)

		movea.l	a3,a5			;ssC
		adda.w	d0,a5			;+BufferWidth

*		for (sy = cm->cm_BufferYL; sy > 0; sy--) {
*		    for (sx = cm->cm_BufferWidth; sx > 0; sx--) {
*			*sdA++ = *ssA++;
*			*sdC++ = *ssC++;
*		    }
*		}
		
		move.w	cm_BufferYL(CM),d6

	PRINTF	DBG_FLOW,<'  sdC=$%lx sdA-$%lx ssC=$%lx ssA=$%lx'>,A3,A4,A5,A6

	PUSHWORD	D6
	PRINTF	DBG_FLOW,<'  sy=$%lx'>
	POPLONG		1

	PUSHWORD	D0
	PRINTF	DBG_FLOW,<'  sx=$%lx'>
	POPLONG		1

		bra.s	termslowloopo		;check sy first (outer loop)

slowloopo:

		move.w	cm_BufferWidth(CM),d0
		bra.s	termslowloopi

slowloopi:

		move.w	(a6)+,(a4)+
		move.b	(a5)+,(a3)+
termslowloopi:
		dbf	d0,slowloopi		;if sx = 0, terminate, else decrement

termslowloopo:
		dbf	d6,slowloopo		;if sy = 0, terminate, else decrement

	PRINTF	DBG_FLOW,<'Slow scroll memory moved'>

		movem.l	(sp)+,BUFA/BUFC/DISPC/DISPA

*		bufA -= cm->cm_BufferWidth;
*		bufC -= cm->cm_BufferWidth;
*		cm->cm_BufferYL--;

		move.w	cm_BufferWidth(CM),d0
		suba.w	d0,BUFA			;UWORD *
		suba.w	d0,BUFA
		suba.w	d0,BUFC			;UBYTE *
		subq.w	#1,cm_BufferYL(CM)

	PRINTF	DBG_FLOW,<'  Scrolled bufA=$%lx bufC=$%lx'>,BUFA,BUFC

*	    }

notoverlap:

*	    /* find source width */
*	    if (y == displayYL)
*		limitX = displayXL;
*	    else
*		limitX = cm->cm_DisplayWidth;

		move.w	DISPLAYXL,limitX(LOCP)
		cmp.w	DISPLAYYL,Y
		beq.s	yEQdy

		move.w	cm_DisplayWidth(CM),limitX(LOCP)
yEQdy:

	PUSHWORD	limitX(LOCP)
	PRINTF	DBG_FLOW,<'  limitX=%ld'>
	POPLONG		1

		bra.s	yLSdy

yGTdy:
*	else {
*	    limitX = 0;
*	    implicitNL = 0;
*	}

		clr.w	limitX(LOCP)
		clr.w	implicitNL(LOCP)

	PRINTF	DBG_FLOW,<' limitX & implicitNL cleared'>

yLSdy:

*	/* check for empty source or explicit newline */
*	if (((y > displayYL) || (!implicitNL)) &&
*		/* but not empty buffer */
*		((cm->cm_BufferYL != 0) || (cm->cm_BufferXL != 0))) {
*	    /* clear the rest of this line in the buffer */


		tst.w	implicitNL(LOCP)
		beq.s	maybeempty

		cmp.w	DISPLAYYL,Y
		BLS_S	notempty

maybeempty:

	PRINTF	DBG_FLOW,<'  y GT displayYL OR implicitNL==0'>

		tst.l	cm_BufferXL(CM)		;and cm_BufferYL
		BEQ_S	notempty

*	    for (x = cm->cm_BufferXL; x < cm->cm_BufferWidth; x++) {
*		*bufA++ = 0;
*		*bufC++;
*	    }
		move.w	cm_BufferXL(CM),d0
		move.w	cm_BufferWidth(CM),d1

	PRINTF	DBG_FLOW,<'  Clear the rest of line in buffer'>

	PUSHWORD	D0
	PRINTF	(DBG_FLOW-1),<'  x=%ld while x <'>
	POPLONG		1

	PUSHWORD	D1
	PRINTF	DBG_FLOW,<' BufferWidth=%ld'>
	POPLONG		1

		bra.s	clearloope
clearloopn:
		clr.w	(BUFA)+
		addq.l	#1,BUFC
		addq.w	#1,d0			;x++
clearloope:
		cmp.w	d1,d0
		bcs.s	clearloopn		;blt unsigned

*	    cm->cm_BufferXL = 0;
*	    cm->cm_BufferYL++;
*	}
		clr.w	cm_BufferXL(CM)
		addq.w	#1,cm_BufferYL(CM)


notempty:

*	/* calculate limit with CP in mind */
*	limitCPX = limitX;
*	if ((y == cpY) && (limitX <= cpX))
*	    limitCPX = cpX + 1;
*
		move.w	limitX(LOCP),d0

		move.w	d0,limitCPX(LOCP)

		cmp.l	cpY(EXTP),Y
		bne.s	yNEcpY

		move.l	cpX(EXTP),d1

		cmp.w	d1,d0
		bhi.s	yNEcpY			;limitX <= cpX?

		addq.w	#1,d1
		move.w	d1,limitCPX(LOCP)	;limitCPX = cpX+1

		move.w	d1,d0			;use below
yNEcpY:

	PUSHWORD	D0
	PRINTF	DBG_FLOW,<'  limitCPX=%ld'>
	POPLONG		1

*	/* copy source to destination */
*	for (x = 0; x < limitCPX; x++) {

		moveq	#00,d1			;x
		bra	copysdend
copysdloop:

*	    if (cm->cm_BufferXL == cm->cm_BufferWidth) {

		move.w	cm_BufferWidth(CM),d6
		cmp.w	cm_BufferXL(CM),d6
		bne.s	bXLNEbw


*		/* buffer line wrap has occurred */
*		implicitNL = CMAF_IMPLICITNL;
*		cm->cm_BufferXL = 0;
*		cm->cm_BufferYL++;

		move.w	#CMAF_IMPLICITNL,implicitNL(LOCP)
		clr.w	cm_BufferXL(CM)
		addq.w	#1,cm_BufferYL(CM)

	PUSHWORD	cm_BufferYL(CM)
	PRINTF	DBG_FLOW,<'  Buffer line wrap  BufferYL=%ld'>
	POPLONG		1


bXLNEbw:

*	    if (x < limitX) {
*		attr = *dispA++;
*		character = *dispC++;
*	    }
*	    else
*		attr = 0;

		moveq	#00,d6			;attr = 0;

		cmp.w	limitX(LOCP),d1
		bcc.s	xGElimitX		;bge unsigned

		move.b	(DISPC)+,d7		;character
		move.w	(DISPA)+,d6		;attr
		bmi.s	crendered
		
xGElimitX:

*	    if (attr >= 0) {

		;;; implied from code above

*		if ((y != cpY) || (x > cpX))
*		    /* no more rendered on this line */
*		    break;

		cmp.l	cpY(EXTP),Y
		BNE_S	break1

		cmp.l	cpX(EXTP),d1
		BHI_S	break1			;x > cpX??

*		attr = lastAttr;
*		character = ' ';

		move.l	lastAttr(EXTP),d6
		moveq	#' ',d7
		bra.s	specialcheck
*	    }
*	    else
*		lastAttr = attr;


crendered:
		move.l	d6,lastAttr(EXTP)	;lastAttr = attr

specialcheck:


	; preset attr for use below (mask out highlight, and select bits -
	; implicitly cleared on new size, and or in implicitNL bit for now)

		andi.w	#(~(CMAF_HIGHLIGHT!CMAF_SELECTED)),d6
		or.w	implicitNL(LOCP),d6

*	    if ((y == cpY) && (x == cpX))
*	    {
*		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
*			| implicitNL | CMAF_CURSOR;

		cmp.l	cpY(EXTP),Y
		bne.s	xNEcpX

		cmp.l	cpX(EXTP),d1
		bne.s	xNEcpX

	; mark that cursor is stored here in off screen map

		ori.w	#CMAF_CURSOR,d6

	PRINTF	DBG_FLOW,<'Cursor @ X%ld Y%ld'>,D1,Y

		move.w	d6,(BUFA)+

*
*		/* special case - cursor at col 0, last line of display 
*		   such as a NEWLINE(s) but no text printed yet.
*
*		   - if we
*		   don't do this the previous attr can have the IMPLICITNL bit
*		   which will be bogus when unpacking the map.
*
*		   Also some code to fix a special case when the colors of the
*		   last character gets carried forward as bogus.
*
*
*		*/
*
*
*		if(y==limitY)
*		{
*			if(x==(limitCPX - 1))	/* outside of text - explicit NL */
*			{
*				if(x == limitX)
*				{
*

		cmp.w	LIMITY,Y
		bne.s	yNElimitY

		move.w	limitCPX(LOCP),d6	;done with attr (reuse d6)
		subq.w	#1,d6

		cmp.w	d6,d1
		bne.s	yNElimitY

		cmp.w	limitX(LOCP),d1
		bne.s	yNElimitY

*					*(bufA - 1) &= ~(CMAM_FGPEN|CMAM_BGPEN);
*					*(bufA - 1) |= (saveAttr | (saveAttr >> CMAS_BGPEN));

	; clear pens; set to last attr pens

		andi.w	#(~(CMAM_FGPEN!CMAM_BGPEN)),-(BUFA)
		move.w	saveAttr(LOCP),d6
		lsr.w	#CMAS_BGPEN,d6
		or.w	saveAttr(LOCP),d6
		or.w	d6,(BUFA)+
*					if(x==0)
*					{
*						*(bufA - 1) &= ~CMAF_IMPLICITNL;
*					}

		tst.w	d1
		bne.s	yNElimitY

		andi.w	#(~(CMAF_IMPLICITNL)),-2(BUFA)
		bra.s	yNElimitY

xNEcpX:

*	    else
*		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
*			| implicitNL;

		move.w	d6,(BUFA)+

yNElimitY:
*	    *bufC++ = character;
*	    cm->cm_BufferXL++;
*	    implicitNL = CMAF_IMPLICITNL;

		move.b	d7,(BUFC)+
		addq.w	#1,cm_BufferXL(CM)
		move.w	#CMAF_IMPLICITNL,implicitNL(LOCP)

		addq.w	#1,d1			;x++
copysdend:
		cmp.w	d0,d1
		bcs	copysdloop		;blt unsigned


break1:
incYloop:
		addq.w	#1,Y
termYloop:
		cmp.w	LIMITY,Y
		bls	nextYloop

	PUSHWORD cm_BufferXL(CM)
	PRINTF	DBG_EXIT,<'  BufferXL=%ld'>
	POPLONG 1

	PUSHWORD cm_BufferYL(CM)
	PRINTF	DBG_EXIT,<'  BufferYL=%ld'>
	POPLONG 1

		movem.l	(sp)+,d2-d7/a2-a6

		add.l	#PackLocals_SIZEOF,sp	;assembler optimizes as addq
		rts

*********************************************************************
*
*int
*unpackSortedLine(cm, y, displayXL, displayYL, cursorMask, cpX, cpY)
*struct ConsoleMap *cm;
*ULONG y;
*UWORD *displayXL, *displayYL;
*UWORD *cursorMask;
*UWORD *cpX, *cpY;

cpY		SET	28
cpX		SET	24
cursorMask	SET	20
displayYL	SET	16
displayXL	SET	12
y		SET	8
cm		SET	4

TEMP		EQUR	A1

BUFOFFSET	EQUR	D2
IREG		EQUR	D3
BUFWIDTH	EQUR	D4
SIZEX		EQUR	D5

_unpackSortedLine:

	PRINTF	DBG_ENTRY,<'unpackSortedLine'>

		move.l	sp,EXTP		;passed in

		movem.l	d2-d7/a2-a6,-(sp)

		movea.l	cm(EXTP),CM			;registerize

*    /* test for degenerate case */
*    if ((cm->cm_BufferYL == 0) && (cm->cm_BufferXL == 0))
*	return(1);

	;
	; actually the degenerate case returns 1, which is added
	; to a counter of the # of lines not filled - this function
	; is called once per display line, and whatever is left
	; over after the off screen buffer is empty is scrolled
	; up by code in scroll.asm.
	;
		moveq	#01,d0				;return(1)

		tst.l	cm_BufferXL(CM)
		beq	unpackdegenerate

*    /* find source */
*    bufC = (UBYTE *) (cm->cm_BufferStart +
*	    (cm->cm_BufferWidth * cm->cm_BufferYL));
*    bufA = (WORD *) (((LONG) bufC) << 1);
*    bufC += cm->cm_AttrToChar;
*    bufOffset = cm->cm_BufferXL;


		moveq	#00,IREG
		moveq	#00,BUFWIDTH

		movea.l	cm_BufferStart(CM),BUFC
		move.w	cm_BufferWidth(CM),BUFWIDTH
		move.w	cm_BufferYL(CM),IREG
		move.l	BUFWIDTH,d0
		mulu.w	IREG,d0
		adda.l	d0,BUFC

		move.l	BUFC,d0
		add.l	d0,d0				;<<1
		movea.l	d0,BUFA

		adda.l	cm_AttrToChar(CM),BUFC		;bufC += cm_AttrToChar

		moveq	#00,BUFOFFSET
		move.w	cm_BufferXL(CM),BUFOFFSET

*    i = cm->cm_BufferYL; (done above)

*    while ((i > 0) && ((*bufA & (CMAF_RENDERED | CMAF_IMPLICITNL)) ==
*	    (CMAF_RENDERED | CMAF_IMPLICITNL))) {
*	/* roll back to non-implicit beginning of buffer */
*	bufOffset += cm->cm_BufferWidth;
*	bufA -= cm->cm_BufferWidth;
*	bufC -= cm->cm_BufferWidth;
*	i--;
*    }

		tst.l	IREG
		beq.s	rolledback

	; get ready with mask
		move.w	#(CMAF_RENDERED!CMAF_IMPLICITNL),d1


rollback:

		move.w	(BUFA),d0

		and.w	d1,d0
		cmp.w	d1,d0
		bne.s	rolledback

		add.l	BUFWIDTH,BUFOFFSET
		suba.w	BUFWIDTH,BUFC

		suba.w	BUFWIDTH,BUFA			;- N * sizeof(WORD)
		suba.w	BUFWIDTH,BUFA
		
		subq.l	#1,IREG
		bne.s	rollback

rolledback:	

*    /* recover source end to be used for this display line */
*    if (bufOffset != 0) {
*	sizeX = bufOffset % cm->cm_DisplayWidth;
*	if (sizeX == 0)
*	    sizeX = cm->cm_DisplayWidth;
*    }
*    else
*	sizeX = 0;

		moveq	#00,SIZEX			;sizeX = 0

		tst.l	BUFOFFSET
		beq.s	offset0

		move.l	BUFOFFSET,d0
slowMOD:
		divu.w	cm_DisplayWidth(CM),d0
		bvc.s	fastMOD

	; ack, overflow (rare case) (subtract 64K*divisor from destination
	; and try again)
		

		moveq	#00,d1
		move.w	cm_DisplayWidth(CM),d1
		swap	d1				;DisplayWidth*64K
		sub.l	d1,d0				;subtract from BufOffset
		bra.s	slowMOD
fastMOD:
		swap	d0				;remainder
		move.w	d0,SIZEX
		bne.s	offset0				;if(sizeX == 0)

		move.w	cm_DisplayWidth(CM),SIZEX

offset0:
*    bufA += bufOffset;
*    bufC += bufOffset;

		adda.w	BUFOFFSET,BUFC
		adda.w	BUFOFFSET,BUFA
		adda.w	BUFOFFSET,BUFA

*    /* find destination end */
*    dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y) + 
*	    cm->cm_DisplayWidth);
*    dispA = (WORD *) (((LONG) dispC) << 1);
*    dispC += cm->cm_AttrToChar;

		move.l	cm_DisplayStart(CM),DISPC
		move.l	y(EXTP),d0
		move.w	cm_DisplayWidth(CM),d1
		mulu.w	d1,d0				;y * cm_DisplayWidth
		adda.l	d0,DISPC			;DisplayStart +
		adda.w	d1,DISPC			;+ DisplayWidth (Ax extended)

		move.l	DISPC,d0
		add.l	d0,d0
		move.l	d0,DISPA

		adda.l	cm_AttrToChar(CM),DISPC

*    /* clear unused destination end */
*    for (x = cm->cm_DisplayWidth; x > sizeX; x--) {
*	*--dispA = 0;
*	--dispC;
*    }
	;;;	move.w	cm_DisplayWidth(CM),d1		;x
		moveq	#00,d0
		bra.s	termclearloop
nextclearloop:
		move.w	d0,-(DISPA)
		subq.l	#1,DISPC
		subq.w	#1,d1
termclearloop:
		cmp.w	SIZEX,d1
		bhi.s	nextclearloop

*    if (y == *displayYL)
*	*displayXL = x;

		move.l	displayYL(EXTP),a1
		move.l	y(EXTP),d0
		cmp.w	(a1),d0				;if(y==*displayYL)
		bne.s	yNEdisplayYL

		move.l	displayXL(EXTP),a1
		move.w	d1,(a1)				;*displayXL = x
yNEdisplayYL:

*    /* copy from buffer */
*    for (; x > 0; x--) {
*	attr = *--bufA;
*	if (attr & *cursorMask) {
*	    *cpX = x - 1;
*	    *cpY = y;
*	    attr &= ~CMAF_CURSOR;
*	    *cursorMask = 0;
*	    /* bufA[1] = attr; */
*	}
*	*--dispA = attr;
*	*--dispC = *--bufC;
*    }

		tst.w	d1
		beq.s	bufcopied

		move.l	cursorMask(EXTP),a1
		move.w	(a1),d6				;D6 = *cursorMask

copybuf:
		move.w	-(BUFA),d0			;attr = *--bufA
		move.w	d0,d7
		and.w	d6,d7				if(attr & *cursorMask)
		beq.s	notcursor

	PRINTF	DBG_FLOW,<'Cursor %ld'>,D1

	; rare case - not time critical

		move.l	cpX(EXTP),a1
		move.w	d1,d7
		subq.w	#1,d7
		move.w	d7,(a1)				;*cpX = x - 1;

		move.l	cpY(EXTP),a1
		move.l	y(EXTP),d7
		move.w	d7,(a1)				;*cpY = y;

		andi.w	#(~(CMAF_CURSOR)),d0		;attr &= ~CMAF_CURSOR;

		clr.w	d6				;*cursorMask = 0;
		move.l	cursorMask(EXTP),a1
		move.w	d6,(a1)				;clear for all future invocations
notcursor:
		move.w	d0,-(DISPA)			;*--dispA = attr
		move.b	-(BUFC),-(DISPC)		;*--dispC = *--bufC
		subq.w	#1,d1
		bne.s	copybuf

bufcopied:

*    /* find new BufferX */
*    cm->cm_BufferXL -= sizeX;
*    if (cm->cm_BufferXL == 0) {
*	if (cm->cm_BufferYL != 0) {
*	    /* look for cm_BufferX start on prior line */
*	    cm->cm_BufferXL = cm->cm_BufferWidth;
*	    while ((((WORD) cm->cm_BufferXL) > 0) && (*--bufA >= 0))
*		cm->cm_BufferXL--;
*	    cm->cm_BufferYL -= 1;
*	}
*    }

		move.w	cm_BufferXL(CM),d0
		sub.w	SIZEX,d0

		bne.s	XLnotempty

		tst.w	cm_BufferYL(CM)
		beq.s	unpackok

		move.w	cm_BufferWidth(CM),d0		;BufferXL = BufferWidth
		beq.s	backline			;??? (safety net)

scanbackXL:
		tst.w	-(BUFA)
		bmi.s	backline

		subq.w	#1,d0
		bne.s	scanbackXL		
		bra.s	backline
XLnotempty:

*    else if (((WORD) cm->cm_BufferXL) < 0) {
*	/* this can only happen when grabbing buffer data thru implicit NL */
*	cm->cm_BufferXL += cm->cm_BufferWidth;
*	cm->cm_BufferYL -= 1;
*    }

		bpl.s	unpackok
		add.w	cm_BufferWidth(CM),d0
backline:
		subq.w	#1,cm_BufferYL(CM)		;skip back 1 line
unpackok:
		move.w	d0,cm_BufferXL(CM)		;save for next time
		moveq	#00,d0


unpackdegenerate:

	PUSHWORD cm_BufferXL(CM)
	PRINTF	DBG_EXIT,<'  BufferXL=%ld'>
	POPLONG	1

	PUSHWORD cm_BufferYL(CM)
	PRINTF	DBG_EXIT,<'  BufferYL=%ld'>
	POPLONG 1

		movem.l	(sp)+,d2-d7/a2-a6

		rts

		END

*****************************************************************************
*****************************************************************************
*****************************************************************************
*#include	"exec/types.h"
*#include	"conmap.h"
*
*#define PDEBUG	0	/* pack debug   */
*#define PCDEBUG 0	/* pack debug - character info */
*#define UDEBUG	0	/* unpack debug */
*
*void packSorted(struct ConsoleMap *,ULONG,ULONG,ULONG,ULONG,ULONG);
*unpackSortedLine(struct ConsoleMap *,UWORD,UWORD *,UWORD *,UWORD *,UWORD *,UWORD *);
*
*void packSorted(cm, displayXL, displayYL, cpX, cpY, lastAttr)
*struct ConsoleMap *cm;
*ULONG displayXL, displayYL;
*ULONG cpX, cpY;
*ULONG lastAttr;
*{
*    UBYTE character, *dispC, *bufC, *ssC, *sdC;
*    WORD attr, *dispA, *bufA, *ssA, *sdA, implicitNL;
*    short limitX, limitCPX, limitY, sx, sy, x, y;
*
*    UWORD saveAttr;
*    saveAttr=(lastAttr & CMAM_BGPEN);
*
*    limitY = (displayYL > cpY) ? displayYL : cpY;
*
*#if PDEBUG
*	kprintf("packSorted routine\n");
*
*	kprintf("limitY=%ld displayYL=%ld cpY=%ld\n",
*		(long)limitY,(long)displayYL,(long)cpY);
*#endif
*
*    for (y = 0; y <= limitY; y++) {
*
*#if PDEBUG
*	kprintf("y=%ld\n",(long)y);
*#endif
*
*	/* find destination */
*	bufC = (UBYTE *) (cm->cm_BufferStart +
*		(cm->cm_BufferWidth * cm->cm_BufferYL) + cm->cm_BufferXL);
*	bufA = (WORD *) (((LONG) bufC) << 1);
*	bufC += cm->cm_AttrToChar;
*
*#if PDEBUG
*	kprintf("   bufA=%lx  bufC=%lx\n",(long)bufA,(long)bufC);
*#endif
*
*	if (y <= displayYL) {
*	    /* find source */
*	    dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y));
*	    dispA = (WORD *) (((LONG) dispC) << 1);
*	    dispC += cm->cm_AttrToChar;
*	    implicitNL = *dispA & CMAF_IMPLICITNL;
*
*#if PDEBUG
*	kprintf("      dispA=%lx dispC=%lx implicitNL=%lx\n",
*		(long)dispA,(long)dispC,(long)implicitNL);
*#endif
*
*	    /* ensure no overlap with destination */
*	    if (((long) bufC) + cm->cm_BufferWidth > ((long) dispC)) {
*
*#if PDEBUG
*	kprintf("         Slow scroll required\n");
*#endif
*
*		/* scroll buffer (painfully) */
*		sdC = (UBYTE *) cm->cm_BufferStart;
*		sdA = (WORD *) (((LONG) sdC) << 1);
*		sdC += cm->cm_AttrToChar;
*		ssA = sdA + cm->cm_BufferWidth;
*		ssC = sdC + cm->cm_BufferWidth;
*		for (sy = cm->cm_BufferYL; sy > 0; sy--) {
*		    for (sx = cm->cm_BufferWidth; sx > 0; sx--) {
*			*sdA++ = *ssA++;
*			*sdC++ = *ssC++;
*		    }
*		}
*		bufA -= cm->cm_BufferWidth;
*		bufC -= cm->cm_BufferWidth;
*		cm->cm_BufferYL--;
*	    }
*
*	    /* find source width */
*	    if (y == displayYL)
*		limitX = displayXL;
*	    else
*		limitX = cm->cm_DisplayWidth;
*	}
*	else {
*	    limitX = 0;
*	    implicitNL = 0;
*	}
*
*#if PDEBUG
*	kprintf("   limitX=%ld implicitNL=%lx\n",
*		(long)limitX,(long)implicitNL);
*
*	kprintf("   y=%ld displayYL=%ld BufferYL=%ld BufferXL=%ld\n",
*		(long)y,(long)displayYL,
*		(long)cm->cm_BufferYL,(long)cm->cm_BufferXL);
*
*#endif
*
*	/* check for empty source or explicit newline */
*	if (((y > displayYL) || (!implicitNL)) &&
*		/* but not empty buffer */
*		((cm->cm_BufferYL != 0) || (cm->cm_BufferXL != 0))) {
*	    /* clear the rest of this line in the buffer */
*
*#if PDEBUG
*	kprintf("      Clear the rest of line in buffer\n");
*#endif
*
*	    for (x = cm->cm_BufferXL; x < cm->cm_BufferWidth; x++) {
*		*bufA++ = 0;
*		*bufC++;
*	    }
*	    cm->cm_BufferXL = 0;
*	    cm->cm_BufferYL++;
*	}
*
*	/* calculate limit with CP in mind */
*	limitCPX = limitX;
*	if ((y == cpY) && (limitX <= cpX))
*	    limitCPX = cpX + 1;
*
*#if PDEBUG
*	kprintf("   limitCPX=%ld\n",(long)limitCPX);
*#endif
*
*	/* copy source to destination */
*	for (x = 0; x < limitCPX; x++) {
*	    if (cm->cm_BufferXL == cm->cm_BufferWidth) {
*#if PDEBUG
*	kprintf("      Buffer line wrapped\n");
*#endif
*		/* buffer line wrap has occurred */
*		implicitNL = CMAF_IMPLICITNL;
*		cm->cm_BufferXL = 0;
*		cm->cm_BufferYL++;
*	    }
*	    if (x < limitX) {
*		attr = *dispA++;
*		character = *dispC++;
*	    }
*	    else
*		attr = 0;
*	    if (attr >= 0) {
*		if ((y != cpY) || (x > cpX))
*		    /* no more rendered on this line */
*		    break;
*		attr = lastAttr;
*		character = ' ';
*	    }
*	    else
*		lastAttr = attr;
*
*#if PCDEBUG
*	kprintf("   character=%ld attr=%ld lastAttr=%ld\n",
*		(long)character,(long)attr,(long)lastAttr);
*
*	kprintf("   y=%ld cpY=%ld x=%ld cpX=%ld\n",
*		(long)y,(long)cpY,(long)x,(long)cpX);
*#endif
*
*	    if ((y == cpY) && (x == cpX))
*	    {
*		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
*			| implicitNL | CMAF_CURSOR;
*
*		/* special case - cursor at col 0, last line of display 
*		   such as a NEWLINE(s) but no text printed yet.
*
*		   - if we
*		   don't do this the previous attr can have the IMPLICITNL bit
*		   which will be bogus when unpacking the map.
*
*		   Also some code to fix a special case when the colors of the
*		   last character gets carried forward as bogus.
*
*
*		*/
*
*		if(y==limitY)
*		{
*			if(x==(limitCPX - 1))	/* outside of text - explicit NL */
*			{
*				if(x == limitX)
*				{
*					*(bufA - 1) &= ~(CMAM_FGPEN|CMAM_BGPEN);
*					*(bufA - 1) |= (saveAttr | (saveAttr >> CMAS_BGPEN));
G
*					if(x==0)
*					{
*						*(bufA - 1) &= ~CMAF_IMPLICITNL;
*					}
*				}
*			}
*		}
*	    }
*	    else
*		*bufA++ = (attr & ~(CMAF_HIGHLIGHT | CMAF_SELECTED))
*			| implicitNL;
*	    *bufC++ = character;
*	    cm->cm_BufferXL++;
*	    implicitNL = CMAF_IMPLICITNL;
*	}
*    }
*}
*
*
*int
*unpackSortedLine(cm, y, displayXL, displayYL, cursorMask, cpX, cpY)
*struct ConsoleMap *cm;
*UWORD y;
*UWORD *displayXL, *displayYL;
*UWORD *cursorMask;
*UWORD *cpX, *cpY;
*{
*    UBYTE *dispC, *bufC;
*    WORD attr, *dispA, *bufA;
*    short sizeX, x, i;
*    long bufOffset;
*
*#if UDEBUG
*	kprintf("unpackSortedLine routine\n");
*	kprintf("BufferYL=%ld BufferXL=%ld\n",
*		(long)cm->cm_BufferYL,(long)cm->cm_BufferXL);
*#endif
*
*    /* test for degenerate case */
*    if ((cm->cm_BufferYL == 0) && (cm->cm_BufferXL == 0))
*	return(1);
*
*
*    /* find source */
*    bufC = (UBYTE *) (cm->cm_BufferStart +
*	    (cm->cm_BufferWidth * cm->cm_BufferYL));
*    bufA = (WORD *) (((LONG) bufC) << 1);
*    bufC += cm->cm_AttrToChar;
*    bufOffset = cm->cm_BufferXL;
*
*    i = cm->cm_BufferYL;
*
*#if UDEBUG
*	kprintf("   bufA=%lx bufC=%lx bufOffset=%lx i=%ld\n",
*		(long)bufA,(long)bufC,(long)bufOffset,(long)i);
*#endif
*
*    while ((i > 0) && ((*bufA & (CMAF_RENDERED | CMAF_IMPLICITNL)) ==
*	    (CMAF_RENDERED | CMAF_IMPLICITNL))) {
*	/* roll back to non-implicit beginning of buffer */
*	bufOffset += cm->cm_BufferWidth;
*	bufA -= cm->cm_BufferWidth;
*	bufC -= cm->cm_BufferWidth;
*	i--;
*    }
*
*    /* recover source end to be used for this display line */
*    if (bufOffset != 0) {
*	sizeX = bufOffset % cm->cm_DisplayWidth;
*	if (sizeX == 0)
*	    sizeX = cm->cm_DisplayWidth;
*#if UDEBUG
*	kprintf("bufOffset ! 0  sizeX=%ld\n",(long)sizeX);
*#endif
*
*    }
*    else
*	sizeX = 0;
*    bufA += bufOffset;
*    bufC += bufOffset;
*
*    /* find destination end */
*    dispC = (UBYTE *) (cm->cm_DisplayStart + (cm->cm_DisplayWidth * y) + 
*	    cm->cm_DisplayWidth);
*    dispA = (WORD *) (((LONG) dispC) << 1);
*    dispC += cm->cm_AttrToChar;
*
*#if UDEBUG
*	kprintf("   sizeX=%ld bufA=%lx bufC=%lx dispA=%lx dispC=%lx\n",
*		(long)sizeX,(long)bufA,(long)bufC,(long)dispA,(long)dispC);
*
*#endif
*
*    /* clear unused destination end */
*    for (x = cm->cm_DisplayWidth; x > sizeX; x--) {
*	*--dispA = 0;
*	--dispC;
*    }
*
*    if (y == *displayYL)
*	*displayXL = x;
*
*    /* copy from buffer */
*    for (; x > 0; x--) {
*	attr = *--bufA;
*	if (attr & *cursorMask) {
*	    *cpX = x - 1;
*	    *cpY = y;
*	    attr &= ~CMAF_CURSOR;
*	    *cursorMask = 0;
*	    /* bufA[1] = attr; */
*	}
*	*--dispA = attr;
*	*--dispC = *--bufC;
*    }
*
*#if UDEBUG
*	kprintf("   BufferXL=%ld BufferYL=%ld sizeX=%ld\n",
*		(long)cm->cm_BufferXL,(long)cm->cm_BufferYL,(long)sizeX);
*#endif
*
*    /* find new BufferX */
*    cm->cm_BufferXL -= sizeX;
*    if (cm->cm_BufferXL == 0) {
*	if (cm->cm_BufferYL != 0) {
*	    /* look for cm_BufferX start on prior line */
*	    cm->cm_BufferXL = cm->cm_BufferWidth;
*	    while ((((WORD) cm->cm_BufferXL) > 0) && (*--bufA >= 0))
*		cm->cm_BufferXL--;
*	    cm->cm_BufferYL -= 1;
*	}
*    }
*    else if (((WORD) cm->cm_BufferXL) < 0) {
*	/* this can only happen when grabbing buffer data thru implicit NL */
*	cm->cm_BufferXL += cm->cm_BufferWidth;
*	cm->cm_BufferYL -= 1;
*    }
*
*#if UDEBUG
*	kprintf("   BufferXL=%ld BufferYL=%ld\n",
*		(long)cm->cm_BufferXL,(long)cm->cm_BufferYL);
*#endif
*    return(0);
*}
*
*
