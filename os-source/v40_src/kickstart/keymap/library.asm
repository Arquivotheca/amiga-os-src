**
**	$Id: library.asm,v 36.3 90/04/13 11:27:56 kodiak Exp $
**
**      init keymap library and resource
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**
	SECTION	keymap

**	Included Files

	INCLUDE	"kldata.i"

	INCLUDE	"exec/initializers.i"

**	Exported Names

	XDEF	KLInitTable


**	Imported Names

	XREF	KLName
	XREF	KLID
	XREF	KRName
	XREF	REVISION
	XREF	VERSION

	XREF	SetKeyMapDefault
	XREF	AskKeyMapDefault
	XREF	MapRawKey
	XREF	MapANSI

	XREF	USAName
	XREF	USA1Name
	XREF	USALowMapType
	XREF	USALowMap
	XREF	USALCapsable
	XREF	USALRepeatable
	XREF	USAHighMapType
	XREF	USAHighMap
	XREF	USAHCapsable
	XREF	USAHRepeatable


	XLVO	AddResource


klFuncInit:
		dc.w	-1
		dc.w	open-klFuncInit
		dc.w	close-klFuncInit
		dc.w	expunge-klFuncInit
		dc.w	extFunc-klFuncInit
		dc.w	SetKeyMapDefault+(*-klFuncInit)
		dc.w	AskKeyMapDefault+(*-klFuncInit)
		dc.w	MapRawKey+(*-klFuncInit)
		dc.w	MapANSI+(*-klFuncInit)
		dc.w	-1

klStructInit:
	    INITBYTE	LN_TYPE,NT_LIBRARY
	    INITLONG	LN_NAME,KLName
	    INITBYTE	LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
	    INITWORD	LIB_REVISION,REVISION
	    INITWORD	LIB_VERSION,VERSION
	    INITWORD	LIB_OPENCNT,1	; never closes
	    INITBYTE	kl_R+LN_TYPE,NT_RESOURCE
	    INITLONG	kl_R+LN_NAME,KRName
	    INITSTRUCT	0,kl_USA+LN_NAME,,9-1
		dc.l	USAName
		dc.l	USALowMapType
		dc.l	USALowMap
		dc.l	USALCapsable
		dc.l	USALRepeatable
		dc.l	USAHighMapType
		dc.l	USAHighMap
		dc.l	USAHCapsable
		dc.l	USAHRepeatable
	    INITSTRUCT	0,kl_USA1+LN_NAME,,9-1
		dc.l	USA1Name
		dc.l	USALowMapType
		dc.l	USALowMap
		dc.l	USALCapsable
		dc.l	USALRepeatable
		dc.l	USAHighMapType
		dc.l	USAHighMap
		dc.l	USAHCapsable
		dc.l	USAHRepeatable

		dc.w	0

KLInitTable:
		dc.l	kl_SIZEOF
		dc.l	klFuncInit
		dc.l	klStructInit
		dc.l	init

;------	init
;
;	d0	library base
;
init:
		move.l	a5,-(a7)
		move.l	d0,a5

		lea	kl_USA1+kn_KeyMap(a5),a0
		move.l	a0,kl_KMDefault(a5)

		;-- kr_List -> kl_USA -> kl_USA1 -> kr_List
		lea	kl_R+kr_List(a5),a0
		move.l	a0,kl_USA+LN_PRED(a5)
		addq	#4,a0
		move.l	a0,kl_USA1+LN_SUCC(a5)
		lea	kl_USA(a5),a0
		move.l	a0,kl_R+kr_List+LH_HEAD(a5)
		move.l	a0,kl_USA1+LN_PRED(a5)
		lea	kl_USA1(a5),a0
		move.l	a0,kl_USA+LN_SUCC(a5)
		move.l	a0,kl_R+kr_List+LH_TAILPRED(a5)

		lea	kl_R(a5),a1
		CALLLVO	AddResource

		move.l	a5,d0
		move.l	(a7)+,a5
		rts


;------	open
;
;	a6	library base
;
open:
		move.l	a6,d0
		rts


;------	close
;------	expunge
;------	extFunc
close:
expunge:
extFunc:
		moveq	#0,d0
		rts

	END
