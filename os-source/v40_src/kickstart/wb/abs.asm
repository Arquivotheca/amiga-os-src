*
* $Id: abs.asm,v 38.24 93/05/24 15:32:37 mks Exp $
*
* $Log:	abs.asm,v $
* Revision 38.24  93/05/24  15:32:37  mks
* Slight cleanup
* 
* Revision 38.23  92/12/15  15:56:08  mks
* Fixed some minor quoting
*
* Revision 38.22  92/11/13  11:15:10  mks
* Updated the text again...
*
* Revision 38.21  92/11/09  18:31:15  mks
* Changed the special text
*
* Revision 38.20  92/08/25  12:39:45  mks
* Saved some space by removing extra init code...
*
* Revision 38.19  92/08/20  10:00:17  mks
* Now uses the ROM copyright from the cached version
*
* Revision 38.18  92/08/20  08:59:17  mks
* Removed the beta version string from the requester and
* made it use the special EXEC version string for that.
*
* Revision 38.17  92/08/04  13:20:46  mks
* Some changes to the About requester
*
* Revision 38.16  92/07/16  15:19:23  mks
* Added Lauren to the others group...
*
* Revision 38.15  92/07/13  18:34:59  mks
* Added config locking to the async tools...
*
* Revision 38.14  92/06/29  09:55:08  mks
* Names have been changed to protect the uninitiated ;^)
*
* Revision 38.13  92/06/22  14:53:09  mks
* Updated some minor strings
*
* Revision 38.12  92/05/30  18:16:47  mks
* Added the WBInfo LVO and stubs
*
* Revision 38.11  92/04/16  10:27:49  mks
* Added the beta version string for beta releases
*
* Revision 38.10  92/04/16  10:02:41  mks
* Changed to use EXEC's strings
*
* Revision 38.9  92/04/14  11:24:37  mks
* Changed to use TaggedOpenLibrary...
*
* Revision 38.8  92/02/18  12:30:36  mks
* Changed part of the 8to6 database
*
* Revision 38.7  92/02/18  12:13:30  mks
* New encoding for 8to6 data...
* New QuickFilter...
*
* Revision 38.6  92/01/07  14:03:53  mks
* Major rework of how resources are allocated.  Also no longer opens
* diskfont.library since the new configuration code will do that.
*
* Revision 38.5  91/12/02  19:43:59  mks
* Fixed "backwards" bug...  Wrong branch on Open of DiskFont library
*
* Revision 38.4  91/11/13  10:58:25  mks
* Now does all of the InitFunc here.  (Silly to do it in C)
*
* Revision 38.3  91/11/12  18:40:54  mks
* Now does thw open and close of the libraries in here.
*
* Revision 38.2  91/11/12  10:07:56  mks
* Small change to some comments/constants
*
* Revision 38.1  91/06/28  15:44:00  mks
* Initial V38 tree checkin - Log file stripped
*

	SECTION	section

;	Included Files

	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE 'exec/io.i'

	INCLUDE	'exec/execbase.i'
	INCLUDE	'devices/inputevent.i'
	INCLUDE	'intuition/intuition.i'

	INCLUDE	'internal/librarytags.i'

	INCLUDE	'workbench.i'
	INCLUDE	'workbenchbase.i'

;	Imported Names

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

	XREF	EndCode
	XREF	Startup
	XREF	_wbmain
	XREF	_InitFunc
	XREF	_LoadTool
	XREF	_DosCopy
	XREF	_DosCopyW
	XREF	_StartInformation
	XREF	_SystemWorkbenchName

	XREF	VERNUM
	XREF	REVNUM
	XREF	_VERSTRING

	XREF	_LVOAlert
	XREF	_LVOAllocMem
	XREF	_LVOOpenLibrary
	XREF	_LVOTaggedOpenLibrary
	XREF	_LVORemTask
	XREF	_LVOInitResident
	XREF	_LVOPutMsg
	XREF	_AbsExecBase
	XREF	_LVORemove
	XREF	_LVOFreeMem
	XREF	_LVOCloseLibrary
	XREF	_LVOQuoteWorkbench
	XREF	_LVOEasyRequestArgs

;	Exported Names

*------ Functions ----------------------------------------------------

	XDEF	_exit

*------ Data --------------------------------------------------------
	XDEF	_subsysName
	XDEF	_WorkbenchName

	xdef	_DOSBaseOffset
	xdef	_GfxBaseOffset
	xdef	_IconBaseOffset
	xdef	_IntuitionBaseOffset
	xdef	_LayersBaseOffset
	xdef	_SysBaseOffset
	xdef	_TimerBaseOffset
	xdef	_GadToolsBaseOffset
        xdef    _LoadToolSegment
	xdef	_DosCopySegment
	xdef	_DosCopySegmentW
	xdef	_AboutSegment
	xdef	_InfoSegment

	XDEF	_VersionFormat
	XDEF	_CopyrightFormat
	XDEF	_KickstartName

;	Local Definitions

initLDescrip:
				;STRUCTURE RT,0
	  DC.W    RTC_MATCHWORD ; UWORD RT_MATCHWORD
	  DC.L    initLDescrip  ; APTR  RT_MATCHTAG
	  DC.L    EndCode	; APTR  RT_ENDSKIP
	  DC.B    RTF_AUTOINIT	; UBYTE RT_FLAGS
	  DC.B    VERNUM        ; UBYTE RT_VERSION
	  DC.B    NT_LIBRARY	; UBYTE RT_TYPE
	  DC.B    -120		; BYTE  RT_PRI
	  DC.L    _subsysName   ; APTR  RT_NAME
	  DC.L    _VERSTRING    ; APTR  RT_IDSTRING
	  DC.L    inittable	; APTR  RT_INIT
				; LABEL RT_SIZE
inittable:
	    DC.L WorkbenchBase_SIZEOF
	    DC.L wbvectors
	    DC.L 0
	    DC.L initFunc

_KickstartName:		dc.b	'Kickstart',0
;
; The following string is used in the About... requester
;
****** VersionFormat: *******
*                           *
* Kickstart version 37.23   *		The "version" is localized
* Workbench version 37.10   *		(same here)
*                           *
* Copyright © 1985-1992     *		Copyright message is below...
* Commodore-Amiga, Inc.     *		(as three parts for the %s)
* All Rights Reserved.      *
*                           *
*           OK              *		Note that OK is a gadget definition
*                           *
*****************************

_VersionFormat:		dc.b	'%s %s %ld.%ld',10,'%s %s %ld.%ld',10,10,'%s',10,'%s',10,'%s',0
;
; The following format is for the initial titlebar display of the copyright
;
_CopyrightFormat:	dc.b	'%s%s%s',0
*
_WorkbenchName:
_subsysName:	dc.b 	'workbench.library',0
verlib:		dc.b	'version.library',0
		CNOP	0,2
*
LibErrors:	dc.w	OLTAG_GRAPHICS,AO_GraphicsLib
		dc.w	OLTAG_INTUITION,AO_Intuition
		dc.w	OLTAG_ICON,AO_IconLib
		dc.w	OLTAG_DOS,AO_DOSLib
		dc.w	OLTAG_LAYERS,AO_LayersLib
		dc.w	OLTAG_GADTOOLS,AO_GadTools
		dc.w	OLTAG_UTILITY,AO_UtilityLib
		dc.w	0
*
*******************************************************************************
*
	;------ this is the entry point for the new, improved, wb task
		xdef	_WbSegment
_WbSegment:	move.l	_AbsExecBase,a6		; Get execbase...
		moveq.l	#OLTAG_WORKBENCH,d0	; Get workbench tag...
		jsr	_LVOTaggedOpenLibrary(a6)	;We know this will work
		move.l	d0,a6			; And into a6 (workbench base)
		jmp	_wbmain			; Start the main routine...
*
*******************************************************************************
*
_exit:

; entryreserved is the required library reserved entry...
entryreserved:
;
; Reserved for future use...
entrywbReserved90:
entrywbReserved96:
entrywbReserved102:
entrywbReserved108:
entrywbReserved114:
	moveq	#0,d0
	rts

; 'LoadToolSegment' and 'DosCopySegment' are fake seglist ptrs that
; get CreateProc'd by workbench to add asynchronicity to launching
; a program and copying files.  They need workbenchbase in a6 in
; order to call the routines inside workbench.  One way to get it is
; to open the library.  This also makes sure that workbench can not
; go away while they are active...

	;------- this is the entry point for the "Information" requester
	CNOP	0,4		; Long word align...
	DC.L	0		; size (none)
_InfoSegment:
	DC.L	0		; bptr to next seg (none)
	move.l	#_StartInformation,a5
	bra.s	ToolSegmentCommon

	;------- this is the entry point for the "About Workbench" requester
	CNOP	0,4		; longword align
	DC.L	0		; size (unknown)
_AboutSegment:
	DC.L	0		; bptr to next seg (none)
	lea	AboutWorkbench(pc),a5	; Point at real code...
	bra.s	ToolSegmentCommon

	;------- this is the entry point for the "async tool loader"
	CNOP	0,4		; longword align
	DC.L	0		; size (unknown)
_LoadToolSegment:
	DC.L	0		; bptr to next seg (none)
	move.l	#_LoadTool,a5	; Point at real code.
	bra.s	ToolSegmentCommon

	;------ this is the entry point for the async copy loader
	CNOP	0,4		; longword align
	DC.L	0		; size (unknown)
_DosCopySegment:
	DC.L	0		; bptr to next seg
	move.l	#_DosCopy,a5	; Point at real code...
	bra.s	ToolSegmentCommon

	;------ this is the entry point for the async copy loader
	CNOP	0,4		; longword align
	DC.L	0		; size (unknown)
_DosCopySegmentW:
	DC.L	0		; bptr to next seg
	move.l	#_DosCopyW,a5	; Point at real code...
;	bra.s	ToolSegmentCommon

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

ToolSegmentCommon:		; a5 == address of code to start running...
	;------ open workbench library
	moveq.l	#OLTAG_WORKBENCH,d0	; Workbench tag...
	move.l	_AbsExecBase,a6
	jsr	_LVOTaggedOpenLibrary(a6)	; Open workbench.library...
;				; We know we are open...
;
	move.l	a6,-(sp)	; Save a6
	move.l	d0,-(sp)	;   and the library pointer...
	move.l	d0,a6		; Set library pointer into a6...
	addq.l	#1,wb_ConfigLock(a6)	; Lock the config...
	jsr	(a5)		; Call the code...
	subq.l	#1,wb_ConfigLock(a6)	; Unlock the config...
	move.l	(sp)+,a1	; Get library pointer ready for CloseLibrary
	move.l	(sp)+,a6	; Get execbase again...
;
	jmp	_LVOCloseLibrary(a6)	; Close workbench.library...
					; CloseLibrary does the RTS for us...

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
initFunc:
	move.l	d0,a1			; Set up workbench base pointer...
	move.l	a0,wb_SegList(a1)	; Save seglist
	move.w	#REVNUM,LIB_REVISION(a1)
;
; Now, make the hooks point at the right stuff...
;
	xref	CoolHook
	lea	CoolHook,a0
	move.l	a0,h_Entry+wb_WBHook(a1)	; Code to run...
	move.l	a0,h_Entry+wb_WinHook(a1)
	move.l	a1,h_SubEntry+wb_WBHook(a1)	; WorkbenchBase
	move.l	a1,h_SubEntry+wb_WinHook(a1)
	clr.l	h_Data+wb_WBHook(a1)		; Pattern pointer
	clr.l	h_Data+wb_WinHook(a1)
;
	rts

entryopen:
	btst	#WB2B_Quit,wb_Flags2(a6) ; is wb quiting?
	beq.s	10$			; no
	moveq	#0,d0			; return failure
	rts

10$	tst.w	LIB_OPENCNT(a6)		; Check if we are running...
	bne.s	OpenIt

	; Ok, now open all libraries I will needed...
	movem.l	a2/a3/a6,-(sp)
	lea	wb_SysBase(a6),a2	; Point at first one...
	move.l	_AbsExecBase,a6		; Get execbase...

	move.l	a6,(a2)+		; Set up SysBase...
	lea	LibErrors(pc),a3	; Get library tags and error codes
DoLoop:	moveq.l	#0,d0			; Clear d0
	move.w	(a3)+,d0		; Get library tag
	beq.s	OpenAll_Done		; If 0, we be done...
	jsr	_LVOTaggedOpenLibrary(a6)	; Open it...
	move.w	(a3)+,d1		; Get error number
	move.l	d0,(a2)+		; Store library base...
	bne.s	DoLoop			; If no error, do next
*
* We should never get here!  But if we do, we just fail out nicely...
*
	move.l	d7,-(sp)
	move.l	#AN_Workbench!AG_OpenLib,d7	; Set other bits...
	or.w	d1,d7				; Add in specific bits
	jsr	_LVOAlert(a6)
	move.l	(sp)+,d7
	movem.l	(sp)+,a2/a3/a6		; Restore...
	bra.s	alreadyClosed

OpenAll_Done:
	lea	wb_Copyright1-wb_TimerBase(a2),a2	; Point at wb_Copyright1
	moveq.l	#OLTAG_COPYRIGHT2,d0		; Copyright ROM2
	jsr	_LVOTaggedOpenLibrary(a6)	; Get pointer
	move.l	d0,(a2)+			; Store it...
	moveq.l	#OLTAG_COPYRIGHT3,d0		; Copyright ROM3
	jsr	_LVOTaggedOpenLibrary(a6)	; Get pointer
	move.l	d0,(a2)+			; Store it...
	moveq.l	#OLTAG_COPYRIGHT4,d0		; Copyright ROM4
	jsr	_LVOTaggedOpenLibrary(a6)	; Get pointer
	move.l	d0,(a2)+			; Store it...
	move.l	#OLTAG_COPYRIGHT5,d0		; Copyright ROM5
	jsr	_LVOTaggedOpenLibrary(a6)	; Get pointer
	move.l	d0,(a2)+			; Store it...
	movem.l	(sp)+,a2/a3/a6		; Restore...

OpenIt:	addq.w	#1,LIB_OPENCNT(a6)	; inc lib open count
	move.l	a6,d0			; return wbbase
	rts

entryclose:
	subq.w	#1,LIB_OPENCNT(a6)	; dec lib open count
	bne.s	StillOpen		; If not 0, we skip...
*
alreadyClosed:
	movem.l	a2/a3/a6,-(sp)		; Save
	lea	wb_SysBase(a6),a2	; Point at sysbase...
	lea	wb_TimerBase(a6),a3	; End point...
	move.l	(a2)+,a6		; Get ExecBase
CloseLoop:
	move.l	(a2),a1			; Get library
	clr.l	(a2)+			; Clear pointer
	jsr	_LVOCloseLibrary(a6)	; Close it
	cmp.l	a2,a3			; Finished?
	bne.s	CloseLoop		; Do some more
	movem.l	(sp)+,a2/a3/a6		; Restore
*
StillOpen:
	moveq	#0,d0			; return 0
	rts

entryexpunge:
	movem.l	d2/a5/a6,-(sp)		; save regs used

	move.l	a6,a5			; wbbase in a5
	move.l	_AbsExecBase,a6		; execbase in a6
	move.l	wb_SegList(a5),d2	; save seglist in d2

	tst.w	LIB_OPENCNT(a5)		; anyone have us open?
	beq.s	10$			; no, so expunge us
	moveq	#0,d0			; setup return value
	bra.s	20$			; we're outa here!

10$:	move.l	a5,a1			; lib base in a1 (for Remove)
	jsr	_LVORemove(a6)		; unlink from lib list

	; calculate mem used and then free it
	moveq	#0,d0
	move.l	a5,a1
	move.w	LIB_NEGSIZE(a5),d0
	sub.l	d0,a1
	add.w	LIB_POSSIZE(a5),d0
	jsr	_LVOFreeMem(a6)		; free our memory

	move.l	d2,d0			; setup return value

20$:	movem.l	(sp)+,d2/a5/a6		; restore regs used
	rts				; bye!


;
; These are the library entry points.
;
	XREF	GetQuote	; External from here...
entryQuoteWorkbench:
	jmp	GetQuote

	xref	_StartWorkbench
entryStartWorkbench:	;(flags,ptr)(D0/D1)
	movem.l	d0/d1,-(sp)
	jsr	_StartWorkbench
	addq.l	#8,sp
	rts

	xref	_AddAppWindow
entryAddAppWindowA:	;(id,userdata,window,msgport,userdata,id)(D0/D1/A0/A1)
	movem.l	d0/d1/a0/a1,-(sp)
	jsr	_AddAppWindow
	lea	16(sp),sp
	rts

	xref	_RemoveAppWindow
entryRemoveAppWindow:	;(aw)(A0)
	move.l	A0,-(sp)
	jsr	_RemoveAppWindow
	addq.l	#4,sp
	rts

	xref	_AddAppIcon
entryAddAppIconA:	;(id,userdata,name,msgport,lock,dobj)(D0/D1/A0/A1/A2/A3)
	movem.l	d0/d1/a0/a1/a2/a3,-(sp)
	jsr	_AddAppIcon
	lea	24(sp),sp
	rts

	xref	_RemoveAppIcon
entryRemoveAppIcon:	;(ai)(A0)
	move.l	a0,-(sp)
	jsr	_RemoveAppIcon
	addq.l	#4,sp
	rts

	xref	_AddAppMenuItem
entryAddAppMenuItemA:	;(id,userdata,text,msgport)(D0/D1/A0/A1)
	movem.l	d0/d1/a0/a1,-(sp)
	jsr	_AddAppMenuItem
	lea	16(sp),sp
	rts

	xref	_RemoveAppMenuItem
entryRemoveAppMenuItem:	;(am)(A0)
	move.l	A0,-(sp)
	jsr	_RemoveAppMenuItem
	addq.l	#4,sp
	rts

	xref	_UpdateWorkbench
entryUpdateWorkbench:	;(name,lock,flag)(A0,A1,D0);
	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp);
	jsr	_UpdateWorkbench
	lea	12(sp),sp
	rts

	xref	_WBConfig
entryWBConfig:	; (tag,pointer)(d0,d1)
	jmp	_WBConfig

	xref	_WBInformation
entryWBInfo:	; (lock,name,screen) (a0,a1,a2)
	movem.l	a0-a2,-(sp)
	jsr	_WBInformation
	lea	12(sp),sp
	rts

FUNCDEF	MACRO
	DC.W	entry\1-wbvectors
	ENDM

wbvectors:
	DC.W	-1
	FUNCDEF	open
	FUNCDEF	close
	FUNCDEF	expunge
	FUNCDEF	reserved
*
	FUNCDEF	UpdateWorkbench
	FUNCDEF	QuoteWorkbench
	FUNCDEF	StartWorkbench
	FUNCDEF	AddAppWindowA
	FUNCDEF	RemoveAppWindow
	FUNCDEF	AddAppIconA
	FUNCDEF	RemoveAppIcon
	FUNCDEF	AddAppMenuItemA
	FUNCDEF	RemoveAppMenuItem
*
* New for V39
*
	FUNCDEF	WBConfig
	FUNCDEF	WBInfo
*
	FUNCDEF	wbReserved90
	FUNCDEF	wbReserved96
	FUNCDEF	wbReserved102
	FUNCDEF	wbReserved108
	FUNCDEF	wbReserved114
*
	DC.W	-1

; Entry point to call to get strings within workbench...
;******************************************************************************
;
; This is the extern C-language entry point...
;
; char * = Quote(ULONG num)
;
		XDEF	_Quote
_Quote:		move.l	4(sp),d0	; Get the quote number...
		jmp	_LVOQuoteWorkbench(a6)	; Jump to the private LVO...
;
;******************************************************************************
;
; This is the extern C-language entry point...
;
; BOOL = WB_Info(BPTR lock, char *name,struct Screen *screen)
;
_LVOWBInfo	equ	-$5a
		XDEF	_WB_Info
_WB_Info:	move.l	a2,-(sp)	; Save a2
		movem.l	8(sp),a0-a2	; Get values...
		jsr	_LVOWBInfo(a6)	; Call it...
		move.l	(sp)+,a2	; Restore
		rts
;
;******************************************************************************
;
;
_DOSBaseOffset		EQU	wb_DOSBase
_GfxBaseOffset		EQU	wb_GfxBase
_IconBaseOffset		EQU	wb_IconBase
_IntuitionBaseOffset	EQU	wb_IntuitionBase
_LayersBaseOffset	EQU	wb_LayersBase
_SysBaseOffset		EQU	wb_SysBase
_TimerBaseOffset	EQU	wb_TimerBase
_GadToolsBaseOffset	EQU	wb_GadToolsBase
*
*
*******************************************************************************
*
* Taking control of the alternate menus will shift you into enlightened state
*
*******************************************************************************
*
* Qualifier check mask : These are the qualifiers that we check...
*
Q_C1	equ	IEQUALIFIER_LSHIFT!IEQUALIFIER_RSHIFT!IEQUALIFIER_CONTROL
Q_C2	equ	IEQUALIFIER_LALT!IEQUALIFIER_RALT
Q_C3	equ	IEQUALIFIER_LCOMMAND!IEQUALIFIER_RCOMMAND
Q_CHECK	equ	Q_C1!Q_C2!Q_C3
*
* Qualifier must mask : These are the qualifiers that must match from the above
*
Q_MUST	equ	Q_C1!Q_C2
*
*******************************************************************************
*
* void QuickFilter(struct IntuiMessage *)
*
* This routine takes an intuition message and does some checks to see if it
* is "valid"  If not, it will change the IntuiMessage CODE field into
* a MENUNULL and thus clearing the menu event.
*
		XDEF	_QuickFilter
_QuickFilter:	move.l	4(sp),a0		; Get the message from the stack...
		cmp.l	#MENUPICK,im_Class(a0)	; A menu message?
		bne.s	NotForMe		; If not for me...
*
* Now we check for qualifier match...
*
		move.w	im_Qualifier(a0),d0	; Get qualifiers...
		and.w	#Q_CHECK,d0		; Mask the important ones
		move.w	d0,wb_LastFlags(a6)	; Store them...
NotForMe:	rts				; A short branch away...
*
*******************************************************************************
*
* This routine does the About() box...
*
* Entry:  a6=WorkbenchBase
* All registers are available...
*
AboutWorkbench:	move.l	sp,a4			; Save Stack somewhere...
		moveq.l	#16,d0			; We need 16 opens...
		cmp.w	LIB_OPENCNT(a6),d0	; Check the open count...
		bne.s	DoAbout
		move.w	wb_LastFlags(a6),d0	; Get flags
		cmp.w	#Q_MUST,d0		; Check them...
		beq	ForMe			; If set, we go...
*
* Get the string "version" from the quotes catalog
*
DoAbout:	moveq.l	#27,d0			; The string number
		jsr	_LVOQuoteWorkbench(a6)	; Get the quote
		move.l	d0,d4			; Save this string...
*
* Now, get the disk version (if possible)
*
		move.l	a6,a5			; Save WBBase somewhere...
		move.l	wb_SysBase(a5),a6	; Get execbase...
		moveq.l	#-1,d2			; Version
		lea	verlib(pc),a1		; Get Version
		moveq.l	#0,d0			; Any version
		jsr	_LVOOpenLibrary(a6)	; Open it?
		move.l	d0,a1			; Get it back...
		move.l	d0,d3			; Check it...
		beq.s	no_ver			; No DiskVersion if NULL
		moveq.l	#0,d2			; Clear d2
		move.w	LIB_VERSION(a1),d2	; Get version
		moveq.l	#0,d3			; Clear d3
		move.w	LIB_REVISION(a1),d3	; Get revision
no_ver:		jsr	_LVOCloseLibrary(a6)	; Close it again...
		exg	a5,a6			; Restore WB to a6...
*
* Start by setting up the copyright strings...
*
		move.l	wb_Copyright3(a6),-(sp)	; 3rd Copyright line
		move.l	wb_Copyright2(a6),-(sp)	; 2nd Copyright line
		move.l	wb_Copyright1(a6),-(sp)	; 1st Copyright line
		move.l	d3,-(sp)		; Revision (Disk)
		move.l	d2,-(sp)		; Version (Disk)
		move.l	d4,-(sp)		; the "version" string
		pea	_SystemWorkbenchName	; "Workbench"
		move.w	SoftVer(a5),d3		; Get Revision of ROM
		move.l	d3,-(sp)		; Revision (ROM)
		move.w	LIB_VERSION(a5),d3	; Get Version of ROM
		move.l	d3,-(sp)		; Version (ROM)
		move.l	d4,-(sp)		; the "version" string
		pea	_KickstartName(pc)	; "Kickstart"
		move.l	sp,a3			; Save a pointer to this
		lea	_VersionFormat(pc),a2	; Point at format string...
*
* Now build the EZ structure...
*
		moveq.l	#30,d0			; The "OK" string
		jsr	_LVOQuoteWorkbench(a6)	; Get the real string
make_ez:	move.l	d0,-(sp)		; es_GadgetFormat
		move.l	a2,-(sp)		; es_TextFormat
		move.l	wb_Copyright4(a6),-(sp)	; es_Title
		clr.l	-(sp)			; es_Flags
		moveq.l	#20,d0			; size of es structure...
		move.l	d0,-(sp)		; es_StructSize
		move.l	sp,a1			; Pointer to es_structure...
		sub.l	a2,a2			; Clear a2
		move.l	wb_BackWindow(a6),a0	; Base window pointer
*
* Now call intuition...
*
		move.l	wb_IntuitionBase(a6),a5	; Get IntuitionBase
		exg	a5,a6			; Put it into a6...
		jsr	_LVOEasyRequestArgs(a6)	; Call it...
		exg	a5,a6			; Restore...
		move.l	a4,sp			; Release stack
		rts
*
*******************************************************************************
*
* Ok, so we have a message that is for me, what do we do about it?
*
ForMe:		move.l	#LineSize,d0		; Get Line0 size...
		sub.l	d0,sp			; Get the space on the stack...
		move.l	sp,a1			; Point at it...
		lea	Line(pc),a2		; Point at code...
		sub.l	a3,a3			; Clear a3...
*
*******************************************************************************
*
* Do the line decode into the buffer...  (a1==buffer, a2==line)
*
DoLine:		moveq.l	#4,d1			; Set character count.
		move.l	(a2)+,d0		; Get next long-word
		rol.l	#2,d0			; Preshift...
DoLine2:	rol.l	#6,d0			; Rotate in next 6-bit byte
		moveq.l	#$3F,d2			; 6 bits of mask
		and.l	d0,d2			; Mask into d2...
		beq.s	SkipChecks		; Skip to character checks
		subq.l	#1,d2			; Check if 1
		bne.s	TokenNot1
		moveq.l	#':',d2			; A 1 token is ': '
		move.b	d2,(a1)+		; Store it
		bra.s	DoLineSpace		; Do a space
TokenNot1:	subq.l	#1,d2			; Check if 2
		bne.s	TokenNot2
		moveq.l	#',',d2			; A 2 token is ', '
		move.b	d2,(a1)+		; Store it
		bra.s	DoLineSpace		; Do a space
TokenNot2:	subq.l	#1,d2			; Check if 3
		bne.s	TokenNot3
DoLineSpace:	moveq.l	#-29,d2			; A 3 token is ' '
TokenNot3:	subq.l	#1,d2			; Check if 4
		bne.s	TokenNot4		; Skip it not..
		moveq.l	#-16,d2			; A 4 token is '.'
TokenNot4:	subq.l	#1,d2			; Check if 5
		bne.s	TokenNot5		; Skip it not
		moveq.l	#-30,d2			; A 5 token is '!'
TokenNot5:	subq.l	#1,d2			; Check if 6
		bne.s	TokenNot6		; Skip if not
		moveq.l	#-54,d2			; A 6 token is a NewLine
TokenNot6:	sub.b	#26,d2			; Subtract z+1 token
		bmi.s	NotLower		; If not negative, skip...
		addq.l	#6,d2			; Shift to lower...
NotLower:	add.b	#'@'+26,d2		; If not a token, convert to
SkipChecks:	move.b	d2,(a1)+		; character and store
		beq.s	LineDone		; Check for NULL
		dbra	d1,DoLine2		; Loop back for bits...
		bra.s	DoLine			; Loop for more line...
LineDone:	move.l	a3,d0			; Get d0 set up...
		move.l	a1,a3			; Get new a3 setting...
		beq.s	DoLine			; If d0 is NULL, loop back...
		move.l	sp,a2			; Get a2 set up...
		bra	make_ez			; Make the requester

*
*******************************************************************************
*
Line:
	; We made it...++
	; OS Group: M.Sinz, D.Greenwald, R.Jesup, J.Horanoff+
	; GFX: S.Shanson, C.Green, J.Barkley+
	; GUI: P.Cherna, M.Taillefer, D.Junod, K.Kuwata+
	; NET: B.Jackson, G.Miller, K.Dyke+
	; SP: E.Cotton, P.Pawlik+
	; Others: Porter, Bryce, Lauren+
	; Others: A.Havemose, Ned, Carolyn, CATS
	;
 dc.l $1D943B61,$24943A74,$04104186,$15643372,$2FD70053,$04669BBA,$02284372
 dc.l $2596EDE1,$2C902604,$10973D70,$024043AF,$3286EBE6,$2618D31E,$01644668
 dc.l $21BB3BEE,$02244372,$2596E090,$04221CAB,$2C97918D,$1B3C1584,$09A25CAE
 dc.l $2109311A,$21A6CB25,$2697208A,$04435BAF,$24091111,$35DE1D21,$0650B681
 dc.l $08110863,$2BCEFB82,$0D113A6C,$2C972091,$042B9AE5,$0665604B,$0426FD34
 dc.l $2FB82584,$16877B29,$2B195D28,$25CB3056,$2FCB4972,$02232E63,$25092875
 dc.l $3296E195,$34A25CB3,$011C43A1,$3696DBF3,$25094964,$02261CAF,$2CE6E089
 dc.l $07699000
	; Better than ever!
	;
 dc.l $08974D25,$320F4A21,$2E0E5DA5,$32140000

LineEnd:
LineSize:	EQU	(LineEnd-Line)*2
*
*******************************************************************************
*
	END
