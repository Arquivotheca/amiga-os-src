*
* $Id: loadwb_patches.asm,v 38.7 92/03/30 08:41:16 mks Exp $
*
* $Log:	loadwb_patches.asm,v $
* Revision 38.7  92/03/30  08:41:16  mks
* Removed (conditionally) the BETA string patch...
* 
* Revision 38.6  92/03/10  10:33:02  mks
* Added check of the returned string to make sure that if it is
* a NULL string that it really returns NULL...
*
* Revision 38.5  92/03/06  09:54:10  mks
* Beta version...  (special BETA text hack)
*
* Revision 38.4  91/07/19  14:45:35  mks
* Now opens V37 of locale.library (the "quicky" locale)
*
* Revision 38.3  91/07/15  15:03:24  mks
* Added code for CleanUp option hack.
* Changed locale library usage to not open a locale and
* to open sys/workbench.catalog rather than just workbench.catalog
*
* Revision 38.2  91/06/24  18:15:46  mks
* Saved two bytes...  (and made adding of the hack work...)
*
* Revision 38.1  91/06/24  11:41:48  mks
* Initial V38 tree checkin - Log file stripped
*
*******************************************************************************
*
* This file contains the patches that will be used in LoadWB.
* The trick is that the patches must remove themselves when Workbench
* wishes to exit.  Also, note that this is for V37 workbench only.
* The patches will *not* install themselves in any other version of
* workbench.  (Or revision)
*
*******************************************************************************
*
		include	"exec/types.i"
		include	"exec/memory.i"
		include	"exec/macros.i"
		include	"exec/libraries.i"
		include	"exec/ables.i"
		include	"utility/tagitem.i"
		xref	_AbsExecBase
*
* These are here just until the amiga.lib and includes are updated for V38
* and locale.library
*
_LVOCloseCatalog	EQU	-$24
_LVOGetCatalogStr	EQU	-$48
_LVOOpenCatalogA	EQU	-$96
*
*******************************************************************************
*
* The following is the first section of workbenchbase in V37.132
* It is copied here such that this code will continue to compile
* in the future.
*
 STRUCTURE WorkbenchBase,0

    STRUCT	wb_Library,LIB_SIZE

    BYTE	wb_UpdateNestCnt
    BYTE	wb_DiskIONestCnt
    ; now longword aligned

    ; ptr to wb's seglist
    APTR	wb_SegList

    ; the workbench task
    ULONG	wb_Task

    ; the work bench status flags
    UBYTE	wb_Flags1
    UBYTE	wb_Flags2
    UBYTE	wb_Flags3
    UBYTE	wb_Flags4
*
* The flag bytes above are used as described here
*
* Note that we will be using the unused_0 flag to signal
* that the patches have been installed.
*
    BITDEF	WB1,Patched,7		; Workbench has been patched
					; Fits in place of unused_0
    ; flags in wb_Flags1
    BITDEF	WB1,unused_0,7		; Unused bit...
    BITDEF	WB1,Dragging,6		; on when dragging icon(s)
    BITDEF	WB1,DoubleClick,5	; we just double clicked
    BITDEF	WB1,SpecialGadget,4	; last gadget was special
    BITDEF	WB1,InputTrashed,3	; Don't believe SELECTUP
    BITDEF	WB1,ErrorDisplayed,2	; ErrorTitle is up
    BITDEF	WB1,NameChanged,1	; NameChange win is up
    BITDEF	WB1,Closed,0		; all windows closed

    ; flags in wb_Flags2
    BITDEF	WB2,KPrintfOK,7		; OK to send kprintfs to serial port
    BITDEF	WB2,unused_1,6		; Unused...
    BITDEF	WB2,CreditOn,5		; title needs refreshing
    BITDEF	WB2,DragSelect,4	; drag selecting
    BITDEF	WB2,Quit,3		; quit WB
    BITDEF	WB2,Backdrop,2		; backdrop disk window is enabled
    BITDEF	WB2,StartWorkbench,1	; workbench has been started
    BITDEF	WB2,unused_2,0		; Unused...
*
*******************************************************************************
*
CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		CALLLIB	_LVO\1		; Call the EXEC definied macro
		ENDM
*
*******************************************************************************
*
* PatchWorkbench(struct Library *WorkbenchBase)
*			a0
*
* This function will determin if Workbench needs to be patched and do
* the patching as needed...  A0 should be the pointer to WorkbenchBase
* which was just opened by LoadWB.  (Before the call to StartWorkbench())
*
_PatchWorkbench:	xdef	_PatchWorkbench
		movem.l	a2/a3/a6,-(sp)		; Save...
		move.l	a0,a2			; Put WorkbenchBase into a2
		move.l	_AbsExecBase,a6		; Get ExecBase
		CALLSYS	Forbid			; We must not task switch
		cmp.w	#37,LIB_VERSION(a2)	; Check the version
		bne.s	Return			; first...
		cmp.w	#132,LIB_REVISION(a2)	; then check the
		beq.s	DoPatchWork		; revision...
*
* If the version and revision don't match, we exit...
*
Return:		CALLSYS	Permit			; Start task switch again
		movem.l	(sp)+,a2/a3/a6		; Restore..
		rts				; No match...
*
* Ok, we have a match on the version and revision.  So, we need to
* do some more checks...
*
* For example:  We do not install the patch if workbench is already running.
* Since we know that this is the exact version we want, there is no problem
* checking this "private" WorkbenchBase value.
*
DoPatchWork:	btst.b	#WB2B_StartWorkbench,wb_Flags2(a2)
		bne.s	Return			; Already started, so no patch
		btst.b	#WB1B_Patched,wb_Flags1(a2)
		bne.s	Return			; Already patched, so no patch
*
* At this point, we know we want to patch this, so lets try to get the memory.
*
		move.l	#PatchSize,d0		; Get size of patch area
		move.l	#MEMF_PUBLIC,d1		; Get memory type
		CALLSYS	AllocVec		; Allocate the memory...
		move.l	d0,a3			; Save the pointer...
		lsr.l	#2,d0			; Shift into a BPTR
		beq.s	Return			; We did not get the memory...
*
* Now, copy up the code
*
		move.l	a3,a0			; Point at the memory...
		lea	StartOfPatch(pc),a1	; Get patch area
		move.l	#(PatchSize/4)-1,d1	; Get number of words...
1$		move.l	(a1)+,(a0)+		; Copy...
		dbra.s	d1,1$			; Loop for all of it...
*
* Now, a sneaky trick:  Link this into the seglist of the library...
* Note that d0 is still the BPTR...
*
		move.l	wb_SegList(a2),(a3)	; Set up the BPTR to nextseg...
		move.l	d0,wb_SegList(a2)	; Save this segment.
*
* Ok, we now need to CacheClearU() such that the code is put into
* real memory
*
		CALLSYS	CacheClearU		; Push caches to memory...
*
* Now, to SetFunction() the QuoteWorkbench() function
*
		lea	NewQuotePatch(a3),a0	; The patch entry point...
		move.l	a0,d0			; To the right register
		move.l	a2,a1			; Workbench base...
		move.w	#_LVOQuoteWorkbench,a0	; LVO offset...
		CALLSYS	SetFunction		; Do the dirty work...
		move.l	d0,OldQuotePatch(a3)	; Store old routine...
		lea	NewClosePatch(a3),a0	; The patch entry point...
		move.l	a0,d0			; To the right register
		move.l	a2,a1			; Workbench base...
		move.w	#LIB_CLOSE,a0		; Get Close vector offset...
		CALLSYS	SetFunction		; Do the dirty work...
		move.l	d0,OldClosePatch(a3)	; Save old entry point...
*
*******************************************************************************
*
* We are done patching the code, so set the Pached flag...
*
		bset.b	#WB1B_Patched,wb_Flags1(a2)	; Set flag for patch
*
*******************************************************************************
*
* Now to try to do the locale patches...
*
		lea	localeName(pc),a1	; Get library name
		moveq.l	#37,d0			; Get minimum version
		CALLSYS	OpenLibrary		; So, we should have Locale
		move.l	d0,LocaleBasePatch(a3)	; ... in LocaleBase...
		beq	Return			; If NULL, exit...
*
* Note that we must return execbase into a6 before we return...
*
		move.l	d0,a6			; Set up LocaleBase
		sub.l	a2,a2			; Clear tag pointer...
		lea	catalogName(pc),a1	; Catalog name
		sub.l	a0,a0			; Default local...
		CALLSYS	OpenCatalogA		; Open it...
		move.l	d0,CatalogPatch(a3)	; Store catalog
*
* Restore ExecBase before we return...
*
		move.l	_AbsExecBase,a6		; Get ExecBase
		bra	Return			; Exit...
*
*******************************************************************************
*
* This is the patch code.  It will be copied to an allocated space for
* the system to be able to use...
*
StartOfPatch:	dc.l	0	; This is the "NEXT" pointer in the seglist.
*
* The patch code starts here.  It *MUST* be position independant.
*
*******************************************************************************
*
* This patch entry point is for the kludge to fix the rename error
* staying in the title bar problem.
*
* This is a rather interesting bug fix.  The bug is that a state value
* is left on when it should not be.  The situation where the fix is done
* is to notice that just before this happens, workbench has to ask for the
* Q_DO_NOT_USE_IN_NAMES=8 string.  So, when workbench asks for this string,
* we just find and clear the state bit.  No real problem, just a rather
* interesting way to fix something that is not external.  Note that this
* can only fix this bug if the workbench version/revision matches exactly.
* Thus, it is only an issue for the 37.132 release of workbench.library
* Future library releases will not have this problem.
*
NewQuote:	cmp.l	#8,d0		; Check if we have the problem string
		bne.s	NoKludge	; If not, skip kludge
*
* We got the string that this happens with.  Now clear the state bit...
*
		bclr.b	#WB1B_NameChanged,wb_Flags1(a6)	;Ugh!
NoKludge:	; Drop down into the Locale patch...
*
*******************************************************************************
*
* ** BETA **  This section of code should only be installed if we have a beta
* release workbench disk set!
*
	IFNE	0
		cmp.l	#27,d0		; Check for "version"
		bne.s	BETA_skip
		lea	BETA_text(pc),a0	; Get text "BETA"
		move.l	a0,d0		; and put into d0
		rts			; exit
BETA_text:	dc.b	'beta',0,0
BETA_skip:
	ENDC
*
*******************************************************************************
*
* This is the local patch...
*
		move.l	d2,-(sp)		; Save...
		move.l	d0,d2			; Save string number
		move.l	OldQuote(pc),a0		; Get old vector
		jsr	(a0)			; Get initial string
*
* Hack to add a key for cleanup...
*
		cmp.l	#100,d2			; Check if CleanUpKey
		bne.s	notCleanUp		; If not it, we skip...
		lea	_CleanUpKey(pc),a0	; Get CleanUpKey
		move.b	(a0),d0			; Check byte...
		beq.s	notCleanUp		; If not right, skip...
		move.l	a0,d0			; Store it...
notCleanUp:
*
		move.l	Catalog(pc),d1		; Get catalog
		beq.s	NoCatalog		; If not catalog, skip...
*
		move.l	d1,a0			; Put catalog into a0
		move.l	d0,a1			; Move string into a1
		move.l	d2,d0			; Get string number into d0
		move.l	LocaleBase(pc),d2	; Get locale base
		exg.l	d2,a6			; Put it into a6/save a6
		CALLSYS	GetCatalogStr		; Do local catalog
		tst.l	d0			; Check if we are NULL
		beq.s	NoString		; If so, we skip the next test
		move.l	d0,a0			; Now, we need to test if we
		move.b	(a0),d1			; got a NULL string
		bne.s	NoString		; If not, we are done
		moveq.l	#0,d0			; If NULL string we return NULL
NoString:	exg.l	d2,a6			; Restore a6
*
NoCatalog:	move.l	(sp)+,d2		; restore...
		rts
*
*******************************************************************************
*
* This the the Close patch such that on the last close, the local is closed
*
NewClose:	move.l	OldClose(pc),-(sp)	; Set up for the RTS at end
		cmp.w	#1,LIB_OPENCNT(a6)	; Check my open count
		bne.s	NoClose			; Not the last close if not 1
		move.l	LocaleBase(pc),d0	; Get localebase...
		beq.s	NoClose			; No close if no base...
		move.l	a6,-(sp)		; Save WBbase
		move.l	d0,a6			; Set up LocaleBase
		move.l	Catalog(pc),a0		; Get catalog value...
		CALLSYS	CloseCatalog		; Close the catalog
		move.l	a6,a1			; Get localebase into a1
		move.l	_AbsExecBase,a6		; Get execbase...
		CALLSYS	CloseLibrary		; Close it...
		moveq.l	#0,d0			; Clear d0
		lea	LocaleBase(pc),a0	; Get pointer to LocaleBase
		move.l	d0,(a0)+		; Clear LocaleBase
		move.l	d0,(a0)+		; Clear Locale
		move.l	d0,(a0)			; Clear Catalog
		move.l	(sp)+,a6		; Restore WBbase
NoClose:	rts
*
*******************************************************************************
*
OldQuote:	dc.l	0	; Store the old QuoteWorkbench address here...
OldClose:	dc.l	0	; Store the old Close vector in Workbench...
*
* This must remain in the same order...
*
LocaleBase:	dc.l	0	; Storage for these...
Catalog:	dc.l	0	; ...values handles...
*
*******************************************************************************
*
* This is the end of the patch area.
*
_CleanUpKey:	xdef	_CleanUpKey
		dc.b	0,0,0,0	; Just a place holder (plus hack...)
EndOfPatch:
*
*******************************************************************************
*
* Now, for a few extra little features...
*
PatchSize		equ	EndOfPatch-StartOfPatch
NewQuotePatch		equ	NewQuote-StartOfPatch
OldQuotePatch		equ	OldQuote-StartOfPatch
NewClosePatch		equ	NewClose-StartOfPatch
OldClosePatch		equ	OldClose-StartOfPatch
LocaleBasePatch		equ	LocaleBase-StartOfPatch
CatalogPatch		equ	Catalog-StartOfPatch
*
* Since this is a private LVO it is not in the amiga.lib...
*
_LVOQuoteWorkbench	equ	-$24
*
*******************************************************************************
*
* This is some locale data...  Not used in the patch code itself...
*
localeName:	dc.b	"locale.library",0
catalogName:	dc.b	"sys/workbench.catalog",0
*
*******************************************************************************
		END
