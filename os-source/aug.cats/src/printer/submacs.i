******************************************************************************
*
*	Source Control
*	--------------
*	$Id: submacs.i,v 1.2 90/07/27 02:21:17 bryce Exp $
*
*	$Locker:  $
*
*	$Log:	submacs.i,v $
*   Revision 1.2  90/07/27  02:21:17  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:25:42  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:33:02  daveb
*   added to rcs
*   
*	Revision 1.1  87/08/19  10:40:22  daveb
*	Initial revision
*
*   Revision 34.3  87/05/24  12:32:18  bart
*   checkpoint
*   
*   Revision 34.2  87/05/24  11:32:27  bart
*   working
*   
*   Revision 34.1  87/05/20  19:16:35  bart
*   checkpoint 05.20.87
*   
*   Revision 34.0  87/05/18  17:45:50  bart
*   added to rcs for updating
*   
*
******************************************************************************

GENMINTERMS		MACRO						* Requires a1=*r
		bsr		GenMinTerms					* genminterms(r);
				ENDM

LOCKLAYER		MACRO						* Requires *cw in register a4
		xref	_LVOLockLayerRom
		jsr		_LVOLockLayerRom(a6)		* lock_layer(cw);
				ENDM

UNLOCKLAYER		MACRO						* Requires *cw in register a4
		xref	_LVOUnlockLayerRom
		jsr		_LVOUnlockLayerRom(a6)		* unlock_layer(cw);
				ENDM

OWNBLITTER		MACRO						* Requires *GB in register a6
		xref	OwnBlitmacroentry
		addq.w	#1,gb_BlitLock(a6)
		beq.s	\@ob
		bsr		OwnBlitmacroentry
\@ob:
				ENDM

DISOWNBLITTER	MACRO						* Requires *GB in register a6
		xref	DisownBlitmacroen
		subq.w	#1,gb_BlitLock(a6)
		blt.s	\@db
		jsr		DisownBlitmacroen	* disownblitter(GB);
\@db:
				ENDM

WAITBLITDONE	MACRO
		ifeq	NARG-1
			btst	#14-8,dmaconr(\1)
			btst	#14-8,dmaconr(\1) * temp kludge for xmem blitter problem
		endc
		ifeq	NARG
			xref	_dmaconr
			btst	#14-8,_dmaconr
			btst	#14-8,_dmaconr * temp kludge for xmem blitter problem
		endc
		beq.s	\@wbd
			bsr		waitblitdone				* waitblitdone();
\@wbd:
				ENDM

BLITBYTES		MACRO						* Requires x0 in d0 and x1 in d1
		bsr		BLTBYTES					* Make the BLTBYTES call
				ENDM

GETCODE			MACRO						* Requires ????
		bsr		getcode						* getcode(arg1, arg2, arg3);
				ENDM

disable_interrupts  macro
	xref	_Disable
    movem.l d0/d1/a0/a1,-(sp)
    jsr _Disable
    movem.l (sp)+,d0/d1/a0/a1
    endm

enable_interrupts   macro
	xref	_Enable
    movem.l d0/d1/a0/a1,-(sp)
    jsr _Enable
    movem.l (sp)+,d0/d1/a0/a1
    endm
