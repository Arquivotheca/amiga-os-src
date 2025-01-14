**
**	$Id: flush.asm,v 1.1 93/02/01 12:18:49 darren Exp $
**
**      EUC text close/flush routines
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**

	SECTION		eucdriver

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/alerts.i"
	include		"exec/execbase.i"
	include		"exec/ables.i"

	INCLUDE		"graphics/text.i"

	INCLUDE		"graphics/gfx.i"
	INCLUDE		"graphics/clip.i"
	INCLUDE		"graphics/rastport.i"
	INCLUDE		"graphics/gfxbase.i"

	INCLUDE		"hardware/blit.i"

	INCLUDE		"driver.i"
	INCLUDE		"debug.i"

*------ Imported Names -----------------------------------------------

*------ Exported Names -----------------------------------------------

*------ Equates ------------------------------------------------------


_LVOExpungeDiskFont	EQU	-$48

*------ Exported Globals ---------------------------------------------

	XDEF		_EUCCloseFont
	XDEF		_EUCLowMemory

	TASK_ABLES

DEBUG_DETAIL	SET	0

*****i* eucdriver.library/EUCLowMemory *******************************
*
*   NAME
*	EUCLowMemory -- Close all open private fonts
*
*   SYNOPSIS
*	EUCLowMemory(execbase, driverbase, MemHandlerData)
*	             A6        A1          A0	   
*
*	void EUCLowMemory(struct Library *,struct EUCDriver *, struct MemHandlerData *);
*
*   FUNCTION
*	Closes all private fonts, and returns.  Diskfont.library actually
*	expunges the private fonts for us if needed for the memory allocation.
*
*   INPUTS
*	execbase - passed by exec in A6
*
*	driverbase - our IS_DATA field, from which we can get at the list
*	of EUC fonts, and from that, the private fonts.
*
*	MemHandlerData - Memory request passed in by exec.  Ignored
*	because all we can do is close any private fonts, and return.
*
*   NOTES
*	Called in Forbid() / Must not break Forbid()
*
*   BUGS
**********************************************************************
_EUCLowMemory:
	PRINTF	DBG_ENTRY,<'EUC--EUCLowMemory($%lx,$%lx,$%lx)'>,A6,A1,A0

		movem.l	d2/d3/a2/a3/a5/a6,-(sp)

		moveq	#MEM_DID_NOTHING,D3		;assume no expunge

		move.l	a1,a5
		move.l	edvr_EUCFontList(a5),a2
		move.l	edvr_GfxBase(a5),a6

eclm_closeprivates:
		tst.l	(a2)
		BEQ_S	eclm_nofonts

	PRINTF	DBG_FLOW,<'ELM--[EUC Font on list $%lx]'>,A2

		moveq	#(EUC_HICHAR-EUC_LOCHAR),d2
		lea	efh_FontLookup(a2),a3

eclm_closefont:
	;
	; If locked, then do not bother trying to close this
	; private font - means loader process is trying to
	; open this font at this time.

		tst.w	efe_Lock(a3)
		bne.s	eclm_lockedfont

	; clear retry count regardless, meaning in low mem conditions
	; we might then later try to reopen a font which could not be
	; opened earlier

		clr.w	efe_Retry(a3)

	; If font is not opened, nothing we can do, try next

		move.l	efe_TextFont(a3),d0
		beq.s	eclm_lockedfont

		move.l	d0,a1

	; CloseFont(), and mark this slot as not pre-opened

		moveq	#00,d0
		move.w	d0,efe_Retry(a3)
		move.l	d0,efe_TextFont(a3)

	PRINTF	DBG_OSCALL,<'ELM--CloseFont($%lx)'>,A1

	; Graphics.library CloseFont() does not break Forbid()
	; CloseFont() must not break Forbid() -- may need mods if
	; if ever does in the future

		JSRLIB	CloseFont

eclm_lockedfont:
		addq.w	#EUCFontElement_SIZEOF,a3
		dbf	d2,eclm_closefont

		move.l	(a2),a2
		BRA_S	eclm_closeprivates

eclm_nofonts:

	; check if there are any fonts to expunge on list

		move.l	edvr_EUCFontList(a5),a2
		moveq	#00,d2				;close library count

eclm_expungepublic:

		tst.l	(a2)
		BEQ_S	eclm_ifopenfonts

		move.l	a2,a3				;current handle
		move.l	(a2),a2				;ready with next

	PRINTF	DBG_FLOW,<'ELM--[Expunge? font handle $%lx - Next $%lx]'>,A3,A2


	; Remove from list, and close this font if tf_Accessors is
	; equal to 1 (meaning the driver is the only opener)

		move.l	efh_TextFont(a3),a1

	PUSHWORD	tf_Accessors(a0)
	PRINTF	DBG_FLOW,<'ELM--[Font open count $%lx]'>
	POPLONG		1

		cmp.w	#1,tf_Accessors(a1)
		BNE_S	eclm_expungepublic

	; font is ready for expunge

		subq.w	#1,tf_Accessors(a1)
		move.l	edvr_DiskFontBase(a5),a6

	PRINTF	DBG_OSCALL,<'ELM--ExpungeDiskFont($%lx)'>,A1

		jsr	_LVOExpungeDiskFont(a6)

	; remove handle from EUC list

		move.l	a3,a1
		REMOVE

	; expunge font handle

		move.l	edvr_SysBase(a5),a6
		move.l	a3,a1

	PRINTF	DBG_OSCALL,<'ELM--FreeVec($%lx)'>,A1

		JSRLIB	FreeVec

	; add to library base count decrement - indicate something freed

		addq.l	#1,d2
		moveq	#MEM_ALL_DONE,d3

		BRA_S	eclm_expungepublic


	; close library once per fully expunged font

eclm_libclose:
		move.l	edvr_SysBase(a5),a6
		move.l	a5,a1

	PRINTF	DBG_OSCALL,<'ELM--CloseLibrary($%lx)'>,A1

		JSRLIB	CloseLibrary

	; code is accessed post final close of library, but in FORBID
	; so is safe from expunge - infact libbase expunge does not
	; occur until ramlib, which is its own memory handler after
	; this one.

eclm_ifopenfonts:
		dbf	d2,eclm_libclose

		move.l	d3,d0				;return (CC set if
		movem.l	(sp)+,d2/d3/a2/a3/a5/a6		;required)
		rts


*****i* eucdriver.library/EUCCloseFont *******************************
*
*   NAME
*	EUCCloseFont -- Close an EUC vectored font
*
*   SYNOPSIS
*	EUCCloseFont(handle, tf, func)
*	             A6      A1  A5
*
*	void EUCCloseFont(struct EUCFontHandle *,struct TextFont *, APTR);
*
*   FUNCTION
*	See graphics/CloseFont()
*
*   INPUTS
*	handle - Pointer to TextFont handle
*
*	tf     - TextFont structure pointer
*
*	APTR   - Pointer to original CloseFont() function
*
*   NOTES
*	MUST NOT BREAK FORBID!!
*
*   BUGS
**********************************************************************
_EUCCloseFont:
	PRINTF	DBG_ENTRY,<'EUC--CloseFont($%lx)'>,A1

	PUSHWORD	tf_Accessors(a1)
	PRINTF	DBG_FLOW,<'EUC--[Font accessors = %ld]'>
	POPLONG		1

		subq.w	#1,tf_Accessors(a1)
		rts


	END
