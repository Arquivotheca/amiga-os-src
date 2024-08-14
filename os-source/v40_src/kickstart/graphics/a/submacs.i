*******************************************************************************
*
*	$Id: submacs.i,v 39.0 91/08/21 17:29:40 chrisg Exp $
*
*******************************************************************************


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
			tst.b   dmaconr(\1) * kludge for xmem blitter problem
			btst	#14-8,dmaconr(\1)
		endc
		ifeq	NARG
			xref	_dmaconr
			tst.b   _dmaconr    * kludge for xmem blitter problem
			btst	#14-8,_dmaconr
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
    jsr _Disable
    endm

enable_interrupts   macro
	xref	_Enable
    jsr _Enable
    endm
