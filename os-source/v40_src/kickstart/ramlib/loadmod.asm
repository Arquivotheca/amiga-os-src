**
**	$Id: loadmod.asm,v 36.6 91/03/14 17:12:30 darren Exp $
**
**      ramlib module loader
**
**      (C) Copyright 1985,1989 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION	ramlib,code

**	Includes

	INCLUDE	"ramlib.i"


**	Exports

	XDEF	EndModule
	XDEF	FindOnList
	XDEF	LoadModule


**	Imports

	XLVO	FindName			; Exec
	XLVO	FindResident			;
	XLVO	InitResident			;

	XLVO	LoadSeg				; Dos
	XLVO	UnLoadSeg			;

	TASK_ABLES


**	Assumptions

	IFNE	rl_DosBase
	FAIL	"rl_DosBase not zero: recode references below"
	ENDC


**********************************************************************
*
*   LoadModule will perform the following:
*
*   5.	Search in the rom resident tags for the module.  If module is
*	not found, skip to 6.  Otherwise, InitResident() the module and
*	return.
*
*   6.	Create the file name for the module -- if no ':' exists in the
*	name already, prepend DEVS: or LIBS: to the name.  LoadSeg the
*	module.  Look for a resident tag in the loaded image -- if not
*	found, initialize with a jump to the first executable
*	instruction and return.
*
*   7.	Check the version and the library name in the resident tag and
*	if sufficient and a name match, InitResident the image and
*	return.  UnLoadSeg a useless image and return.
*

;------ LoadModule ---------------------------------------------------
;
;   a4	OpenParms
;   a5	RamLibPrivate
;   a6	SysBase
;
LoadModule:
		movem.l	a2/a6,-(a7)

		;-- scan the exec list one more time for this module
		move.l	op_ExecList(a4),a0 
		bsr	FindOnList 
		bne	lmDone

		clr.l	op_Segment(a4)

		;-- see if we can find a resident tag
		move.l	op_OpenName(a4),a1
		CALLLVO	FindResident
		move.l	d0,a1		; set up for InitResident
		tst.l	d0
		bne.s	lmInitResident


		;-- try and load module in from disk relative to DEVS:/LIBS:
		move.l	op_LoadName(a4),d1
		move.l	a6,-(a7)
		move.l	(a5),a6		; rl_DosBase(a5)
		CALLLVO	LoadSeg

		move.l	d0,op_Segment(a4)
		bne.s	lmLoadedModule

		;-- try and load module in from disk w/ original name
		move.l	op_ReqName(a4),d1
		cmp.l	op_LoadName(a4),d1
		beq	lmPopDone

		move.l	op_CurrentDir(a4),d0
		beq.s	nocurrentdir	; dont bother if current dir is
					; NULL.

		CALLLVO	LoadSeg
nocurrentdir:
		move.l	d0,op_Segment(a4)
		beq.s	lmPopDone

lmLoadedModule:
		move.l	(a7)+,a6

		;-- well we loaded in something.  try and find a romtag
lmNextSegment:
		lsl.l	#2,d0		; convert from BPTR to APTR
		beq.s	lmNoRamTag	; no more segments to check
		move.l	d0,a0
		move.l	-4(a0),d1	; length of segment (in bytes)
		move.l	(a0)+,d0	; cache next segment BPTR
		sub.l	#6+RT_SIZE,d1	; subtract segment header
					; + sizeof resident tag
		bls.s	lmNextSegment

lmRamTagSearch:
		subq.l	#2,d1
		bls.s	lmNextSegment

		cmp.w	#RTC_MATCHWORD,(a0)+
		bne.s	lmRamTagSearch

		lea	-RT_MATCHTAG(a0),a1
		cmp.l	(a0),a1
		bne.s	lmRamTagSearch

		;-- we found a ram tag

lmInitResident:
		;-- is this OpenLibrary or OpenDevice
		tst.b	op_OpenLibFlag(a4)
		beq.s	lmDevice

		;-- check library version
		move.l	OpenParms_SIZEOF+or_D0(a4),d0
		cmp.b	RT_VERSION(a1),d0
		bgt.s	lmFailUnload

		;-- check dev/lib name
lmDevice:
		move.l	op_OpenName(a4),a0

		move.l	RT_NAME(a1),a2
lmNameCheck:
		move.b	(a0)+,d0
		beq.s	lmNameNull
		cmp.b	(a2)+,d0
		bne.s	lmNameEnd
		bra.s	lmNameCheck
lmNameNull:
		tst.b	(a2)
lmNameEnd:
		bne.s	lmFailUnload

		;-- initialize resident tag at a1
		move.l	op_Segment(a4),d1
		CALLLVO	InitResident

lmCheckLoad:
		move.l	op_ExecList(a4),a0
		bsr.s	FindOnList
		beq.s	lmFailUnload


lmDone:
		movem.l	(a7)+,a2/a6
		rts


		;-- module image inappropriate
lmFailUnload:
		move.l	op_Segment(a4),d1
		beq.s	lmDone
		move.l	a6,-(a7)
		move.l	(a5),a6			; rl_DosBase
		CALLLVO	UnLoadSeg
lmPopDone:
		move.l	(a7)+,a6
		bra.s	lmDone


		;-- no ram tag found
lmNoRamTag:
		move.l	op_Segment(a4),a0
		move.l	a0,d0
		lsl.l	#2,d0
		move.l	d0,a1
		jsr	4(a1)			; w/ segment in a0
		bra.s	lmCheckLoad


;------	FindOnList ---------------------------------------------------
;  a0	list
;  a4	OpenParms
;  a6	SysBase
FindOnList:
		move.l	op_OpenName(a4),a1
		CALLLVO	FindName

		;-- get the condition codes set right
		tst.l	d0
		rts

EndModule:

	END
