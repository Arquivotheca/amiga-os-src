**
**	$Id: loader.asm,v 1.2 93/04/23 14:04:44 darren Exp $
**
**      EUC process loader for fonts
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
**


	SECTION		eucdriver

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/execbase.i"
	INCLUDE		"exec/alerts.i"
	INCLUDE		"exec/ables.i"

	INCLUDE		"graphics/text.i"
	INCLUDE		"graphics/gfxbase.i"

	INCLUDE		"diskfont/diskfont.i"

	INCLUDE		"dos/dos.i"
	INCLUDE		"dos/dosextens.i"

	TASK_ABLES
*------ Macros -------------------------------------------------------


		IFND	PRINTF

PRINTF		MACRO	; level,<string>,...
		IFGE	DEBUG_DETAIL-\1

		IFNE	ROM_MODULE
			XREF	kprint_macro
		ENDC

PUSHCOUNT	SET	0

		IFNC	'\9',''
		move.l	\9,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\8',''
		move.l	\8,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\7',''
		move.l	\7,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\6',''
		move.l	\6,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\5',''
		move.l	\5,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\4',''
		move.l	\4,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		IFNC	'\3',''
		move.l	\3,-(sp)
PUSHCOUNT	SET	PUSHCOUNT+4
		ENDC

		movem.l a0/a1,-(sp)
		lea.l	PSS\@(pc),A0
		lea.l	4*2(SP),A1
		BSR	kprint_macro
		movem.l (sp)+,a0/a1
		bra.s	PSE\@

PSS\@		dc.b	\2
		IFEQ	(\1&1)  ;If even, add CR/LF par...
		   dc.b 13,10
		ENDC
		dc.b	0
		ds.w	0
PSE\@
		lea.l	PUSHCOUNT(sp),sp
		ENDC	;IFGE	DEBUG_DETAIL-\1
		ENDM	;PRINTF	MACRO
		ENDC	;IFND	PRINTF


	INCLUDE		"driver.i"
	INCLUDE		"debug.i"

*------ Imported Globals ---------------------------------------------

	XREF	DFName

*------ Imported Functions -------------------------------------------

	XREF	_LVOAlert

*------ Exported Functions -------------------------------------------

	XDEF	_OpenEUCFont

*------ Exported Globals -------------------------------------------

	XDEF	_EUCLoaderSeg
	XDEF	_EUCLoaderName
	XDEF	_EUC_STARTSEGMENT
	XDEF	_EUC_ENDSEGMENT
	XDEF	_EUCName
	XDEF	EUC_PROCESS_SIZE

*------ Equates ------------------------------------------------------

EUC_PROCESS_SIZE	EQU	_EUC_ENDSEGMENT-_EUC_STARTSEGMENT

_LVONewOpenDiskFontA	EQU	-$3c

*------ Assumptions --------------------------------------------------


DEBUG_DETAIL	SET	0
**
** ROM or RAMable code ... must be safe to signal, and bail out
** without having to worry about breaking Forbid() in CloseLibrary()
** which lets us do the next step ... call CloseLibrary() on the context
** of CloseFont() without breaking Forbid().
**


EXECBASE	EQU	4

		CNOP	0,4

_EUC_STARTSEGMENT:

		dc.l	16	;Segment length (faked)
_EUCLoaderSeg:
		dc.l	0	;Pointer to next segment

		movem.l	d2-d5/a2-a6,-(sp)

	PRINTF	DBG_ENTRY,<'EUL--Begin'>

		move.l	EXECBASE,a6
		lea	_EUCName(pc),a1
		JSRLIB	FindPort

	PRINTF	DBG_OSCALL,<'EUL--$%lx=FindPort()'>,D0

		tst.l	d0
		beq	els_Alert

		move.l	d0,a4	;library base message port
		move.l	elp_LibBase(a4),a5

	IFEQ	ROM_MODULE

	; link memory for this task

		move.l	ThisTask(a6),a0
		lea	TC_MEMENTRY(a0),a0
		move.l	edvr_MemList(a5),a1

	PRINTF	DBG_OSCALL,<'EUL--AddTail($%lx,$%lx) to TC_MEMENTRY'>,A0,A1

		JSRLIB	AddTail

	ENDC
	; create port for loader

		moveq	#-1,d0
		JSRLIB	AllocSignal

		cmp.b	#-1,d0
		beq	els_Alert	;??what??

	PUSHBYTE	D0
	PRINTF	DBG_OSCALL,<'EUL--$%lx=AllocSignal()'>
	POPLONG		1

		move.b	d0,edvr_LoaderPort+MP_SIGBIT(a5)

	; cache signal bit for exit code

		move.b	d0,d3

		moveq	#01,d2
		lsl.l	d0,d2		;make mask for signals
		or.l	#SIGBREAKF_CTRL_C,d2

		lea	_EUCLoaderName(pc),a0
		move.l	a0,edvr_LoaderPort+LN_NAME(a5)
		move.b	#NT_MSGPORT,edvr_LoaderPort+LN_TYPE(a5)
		move.b	#PA_SIGNAL,edvr_LoaderPort+MP_FLAGS(a5)
		move.l	ThisTask(a6),edvr_LoaderPort+MP_SIGTASK(a5)
		lea	edvr_LoaderPort+MP_MSGLIST(a5),a1
		NEWLIST	a1

		moveq	#00,d0
		move.l	d2,d1
		JSRLIB	SetSignal	;clear any pending (safety)

	; and tell OpenLibrary() we are ready for messages

		move.l	MP_SIGTASK(a4),a1
		moveq	#01,d0
		moveq	#00,d1
		move.b	MP_SIGBIT(a4),d1
		lsl.l	d1,d0

	PRINTF	DBG_OSCALL,<'EUL--Signal($%lx,$%lx)'>,A1,D0

		JSRLIB	Signal

	; disable DOS requesters so that we do not dead-lock
	; with Intuition calling Text().

		move.l	ThisTask(a6),a0
		move.l	pr_WindowPtr(a0),-(sp)
		moveq	#-1,d0
		move.l	d0,pr_WindowPtr(a0)

	;;; This is the main loader process loop

els_LoaderLoop:
		move.l	d2,d0

	PRINTF	DBG_OSCALL,<'EUL--Wait($%lx)'>,D0

		JSRLIB	Wait

	PRINTF	DBG_OSCALL,<'EUL--$%lx=Wait()'>,D0

	; check for signal to bail out - code must NOT access
	; any library base memory if signaled to EXIT

		move.l	d0,d1
		and.l	#SIGBREAKF_CTRL_C,d1
		bne	rtn_exit

	; must be a message to open a private font

els_nextmessage:
		lea	edvr_LoaderPort(a5),a0
		JSRLIB	GetMsg

	PRINTF	DBG_OSCALL,<'EUL--$%lx=GetMsg()'>,D0

		tst.l	d0
		BEQ_S	els_LoaderLoop

		move.l	d0,a3

	; If diskfont has been opened, use diskfont, else try to
	; open this font via OpenFont()/WeighTAMatch() first.
	;
	; o If diskfont already opened, then just let diskfont
	;   call OpenFont()/WeighTAMatch().
	;
	; o If diskfont has not been opened yet, assume we are
	;   running from ROM only (diskfont not available), and
	;   try to open the font before walking the library list
	;   via OpenLibrary().
	;
	; o If font not found, then try to open diskfont.library.
	;   In most cases we will use diskfont's code, or our
	;   own open code (because the font is in ROM).


		move.l	edvr_DiskFontBase(a5),d0
		move.l	d0,a6
		BNE_S	els_usediskfont


	; Try to open file part of name

		move.l	eof_TTextAttr+tta_Name(a3),d1
		move.l	d1,d5		;cache
		move.l	edvr_DOSBase(a5),a6
		JSRLIB	FilePart
		move.l	d0,eof_TTextAttr+tta_Name(a3)	;replace

		move.l	edvr_GfxBase(a5),a6
		lea	eof_TTextAttr(a3),a0

	PRINTF	DBG_OSCALL,<'EUL--OpenFont(%s)'>,LN_NAME(A0)

		JSRLIB	OpenFont

	PRINTF	DBG_OSCALL,<'EUL--$%lx=OpenFont(%s)'>,D0

		tst.l	d0
		beq.s	els_trydiskfont

	; Check if this is an exact match

		move.l	d0,d4				;cache
		lea	eof_TTextAttr(a3),a0
		move.l	d0,a1
		move.l	tf_Extension(a1),a2		;tags of font we opened
		move.l	tfe_Tags(a3),a2			;tags 
		lea	tf_YSize-ta_YSize(a1),a1	;TTextAttr in font we opened
		JSRLIB	WeighTAMatch			;only YSize, flags, styles are important

		move.w	d0,d1
		move.l	d4,d0

		cmp.w	#MAXFONTMATCHWEIGHT,d1
		BEQ_S	els_returnfont			;perfect!!! return success

	;
	; CloseFont(), and try via diskfont
	;

		move.l	d4,a1

	PRINTF	DBG_OSCALL,<'EUL--CloseFont($%lx)'>,A1

		JSRLIB	CloseFont

	;
	; Not able to OpenFont() or WeighTAMatch() perfect, so try to open
	; via diskfont.library, but if no diskfont, return an error
	;

els_trydiskfont:
		move.l	d5,eof_TTextAttr+tta_Name(a3)	;restore full path
		move.l	edvr_SysBase(a5),a6
		lea	DFName(pc),a1
		moveq	#40,d0				;V40 diskfont required
		JSRLIB	OpenLibrary
		move.l	d0,edvr_DiskFontBase(a5)
		BEQ_S	els_returnfont			;return failure if no library
							;else try to open via diskfont


	PRINTF	DBG_FLOW,<'EUL--diskfont.library opened'>

els_usediskfont:

		lea	eof_TTextAttr(a3),a0
		lea	els_privatetag(pc),a1

	PRINTF	DBG_OSCALL,<'EUL--NewOpenDiskFontA($%lx,$%lx)'>,A0,A1

		jsr	_LVONewOpenDiskFontA(a6)

	PRINTF	DBG_OSCALL,<'EUL--$%lx=OpenDiskFont()'>,D0

els_returnfont:

		move.l	d0,eof_TextFont(a3)
		move.l	edvr_SysBase(a5),a6

		move.l	a3,a1

	PRINTF	DBG_OSCALL,<'EUL--ReplyMsg($%lx)'>,A1

		JSRLIB	ReplyMsg

		bra	els_nextmessage

rtn_exit:
	; code must NOT access any library base memory if signaled to
	; exit

	PRINTF	DBG_FLOW,<'EUL--[Exit EUC Driver process]'>

	; restore pr_WindowPtr from stack

		move.l	ThisTask(a6),a0
		move.l	(sp)+,pr_WindowPtr(a0)

	; not really necessary to free signal, but what the heck

		moveq	#00,d0		;not really necessary - UBYTE is ok
		move.b	d3,d0
		JSRLIB	FreeSignal
		
rtn_alert:
		movem.l	(sp)+,d2-d5/a2-a6
		moveq	#00,d0
		rts

** wow, something is really really wrong, and we can't even signal
** the task which is holding a semaphore, and trying to start this
** code!!

els_Alert:
	DEADALERT	(AN_Unknown!AG_OpenLib!AO_Unknown)
	bra	rtn_alert		;??what??


**
** Local debugging - conditional compile if not a ROM module
**


	IFEQ	ROM_MODULE

		IFNE	DEBUG_DETAIL
		IFND	KPrintF
		XREF	KPrintF
		ENDC

kprint_macro:
		movem.l	d0-d1/a6,-(sp)
		jsr	KPrintF
		movem.l	(sp)+,d0-d1/a6
		rts

		ENDC
	ENDC

els_privatetag:
		dc.l	NODF_Private
		dc.l	1			;TRUE
		dc.l	TAG_DONE

_EUCName:
		dc.b	'eucdriver.library',0

_EUCLoaderName:
		dc.b	'EUCFont.loader',0

		CNOP	0,4
_EUC_ENDSEGMENT:


*------ OpenEUCFont --------------------------------------------------
*
*   NAME
*	OpenEUCFont -- Open a font via process
*
*   SYNOPSIS
*	font = OpenEUCFont( fonthandle, key )
*	d0		     a6         d0
*					ULONG
*   FUNCTION
*
*	Returns pointer to private EUC font if already available,
*	else attempts to load it from disk via the driver process.
*
* 	Use of process is single threaded, which is no loss since
*	OpenDiskFont() is also single threaded via a semaphore.
*	Does not call OpenFont() directly because OpenFont() will
*	return a match which may not be perfect - important
*	for using aspect ratio scaled fonts.
*
*	Regardless of the base font, Japanese private font data
*	is assumed to be 1:1 aspect ratio, non-proportional data.
*	For best results, data should be a square matrix (e.g.,
*	14x14, 16x16, 24x24).  A DPI tag is always provided so
*	that we ask for a Japanese font the width of which is
*	2x that of the X width of the base font.  For a proportional
*	base font this means that we obtain Japanese font data which
*	is 2x the nominal width of the basefont.
*
*	For white-space enhanced fonts, private fonts are opened
*	from non-white-space enhanced fonts, and size adjusted.
*
*	Fonts returned by this function can be closed with CloseFont()
*
*    INPUTS
*
*	fonthandle - Pointer to EUCFontHandle structure.
*
*	key - $A0 through $FF tells us which private font to
*	open, and adds it to this font handle's list of fonts
*	open.
*
*    NOTES
*
*	PRESERVES ALL REGISTERS!
*
*	All fonts managed by the eucdriver library are stored
*	on a list, and within each handle on the list, there
*	is an array of open private fonts (each with a lock, and
*	retry count associated with it).  
*
*	Fonts which are return always have an accessor count of
*	at least 2, and will be decremented to 1 when closed by
*	the caller.  The base count of 1 is held until expunged
*	during low memory conditions, at which time the driver
*	will be responsible for closing all private fonts
*
*	During a low memory condition, expunge code needs to go
*	through the list of handles, and search for any fonts
*	within a handle which are not locked, and which have
*	an accessor count of 1.  For each of these, the
*	private font is closed, and removed from the handles font
*	list.  The font must be reopened again via OpenDiskFont().
*	Because diskfont will be smarter (not expunge all closed
*	fonts during low memory conditions), it may be that many
*	of these expunged fonts can be reopened quickly from memory.
*
*---------------------------------------------------------------------
_OpenEUCFont:
		

	; perform quick look-up first - lock entry on list from
	; expunge; do quick lookup to determine if pointer to TextFont
	; is TRUE; if so, increment font accessor count, release lock,
	; and return pointer to TextFont structure

		movem.l	d2/a0-a1,-(sp)

		move.l	d0,d2

		lea	efh_FontLookup(a6),a0

	IFNE	EUCFontElement_SIZEOF-8
	FAIL	"EUCFontElement_SIZEOF not equal to 8, recode!"
	ENDC

		sub.b	#EUC_LOCHAR,d0
		lsl.w	#3,d0			;* sizeof(struct EUCFontElement)
		adda.w	d0,a0

		addq.w	#1,efe_Lock(a0)		;atomic lock - disable expunge
		move.l	efe_TextFont(a0),d0
		BEQ_S	oef_OpenFont

		move.l	d0,a1			;bump TextFont count
		addq.w	#1,tf_Accessors(a1)

	PRINTF	DBG_FLOW,<'EUC--[Immediate open of font @ $%lx]'>,D0

		subq.w	#1,efe_Lock(a0)		;release lock

	; swizzle forward on public list if already opened -
	; this causes frequently used fonts to bubble to the head
	; of the public font list, but only relative to each others
	; priority.

		movem.l	d0-d1/a2/a6,-(sp)

		move.l	efh_GfxBase(a6),a0
		move.l	efh_SysBase(a6),a6
		lea	gb_TextFonts(a0),a0
		move.l	a0,d0			;cache for restore after REMOVE

		FORBID

	; if not head of list

		move.l	LN_PRED(a1),a2

		cmpa.l	a2,a0
		beq.s	oef_athead

	; if previous is same priority

		move.b	LN_PRI(a2),d1
		cmp.b	LN_PRI(a1),d1
		bne.s	oef_athead

	; Remove previous

	PRINTF	DBG_FLOW,<'EUC--[Swizzle $%lx ahead of $%lx]'>,A1,A2

		exg	a2,a1			;remove PRED/cache TextFont Node
		move.l	a1,d1			;cache for restore after REMOVE
		REMOVE

		move.l	d0,a0			;restore list pointer
		move.l	d1,a1			;node to insert
		JSRLIB	Insert

	; insert previous after textfont node
oef_athead:

		PERMIT

		movem.l	(sp)+,d0-d1/a2/a6
		movem.l	(sp)+,d2/a0-a1
		rts


	; The slot is still locked from expunge, but the font is
	; not already open, so obtain semaphore for this handle,
	; and then try to open the font.  Someone else may
	; come in behind me, and find out that the font is
	; still not available, in which case that task also
	; increments the lock for the font slot, and has to
	; wait for the semaphore to be released.  If the font
	; is available, it will only be so because I updated
	; the slot after it is safe to do so.

oef_OpenFont:

	PRINTF	DBG_FLOW,<'EUC--[oef_OpenFont]'>,D0

		movem.l	d1/d3-d5/a2-a6,-(sp)

		move.l	a0,a2			;cache

		move.l	a6,a4
		move.l	efh_DriverBase(a4),a5
		move.l	efh_SysBase(a4),a6

		lea	efh_HandleSemaphore(a4),a0

	PRINTF	DBG_OSCALL,<'EUC-- ObtainSemaphore($%lx)'>

		JSRLIB	ObtainSemaphore

	PRINTF	DBG_FLOW,<'EUC--[Handle Semaphore obtained]'>,D0

	; test slot one more time, just in case this is a second
	; task which got in here after we already updated the slot
	; indicating the font was opened

		move.l	efe_TextFont(a2),d0
		bne	oef_havefont

	PRINTF	DBG_FLOW,<'EUC--[Font still not open]'>,D0

	;	moveq	#00,d0

		move.w	efe_Retry(a2),d1
		cmp.w	#RETRY_FONT,d1
		bhi	oef_nofont

		addq.w	#1,d1
		move.w	d1,efe_Retry(a2)

	PUSHWORD	D1
	PRINTF	DBG_FLOW,<'EUC--[Font Retry count = %ld]'>
	POPLONG		1

	; create process message from TextFont structure

		move.l	#256,d3				;for AddPart()
		sub.l	d3,sp

		lea	efh_Message(a4),a1

	; initialize DPI tag for request

		lea	eof_DPITag(a1),a0
		move.l	a0,eof_TTextAttr+tta_Tags(a1)	;link
		move.l	#TA_DeviceDPI,eof_DPITag+ti_Tag(a1)

	; copy YSize, and relevant style bits

		move.l	efh_TextFont(a4),a0

		move.w	tf_YSize(a0),eof_TTextAttr+tta_YSize(a1)

		move.b	tf_Style(a0),d0
		or.b	#FSF_TAGGED,d0			;always tagged
		move.b	d0,eof_TTextAttr+tta_Style(a1)

	; Do not request ROMFONT, DISKFONT, TALLDOT, or WIDEDOT.
	; Even if the base font is proportional, do not request 
	; PROPORTIONAL for kanji font data.  Do not request 
	; DESIGNED - scaling is always allowed since we request
	; via DPI

		move.b	#00,eof_TTextAttr+tta_Flags(a1)

	;
	; Make a DPI tag which results will result in a font of twice the
	; width of our base font, or at least twice the width of the
	; base fonts optimal X size.
	;
	; For algorithmic white space enhanced fonts, size adjusted
	; so that we ask for a font which is YSize-2 (Y1), and
	; ((XSize * 2)-2) (X1).
	;
	; We assume base font data is 1:1 aspect ratio, so by providing
	; a request for YSize of N, and an aspect ratio of X1/Y1, we
	; guarantee X size will match our expectation.
	;
	; For a simple font, say 16x8 base, this means we would request
	; 16 * (16/16 DPI), resulting in the normal 1:1 disk font.
	;
	; For a scaled font when the base font is 19 high by 9 wide,
	; we request 19 * (18/19 DPI) which results in a font 19
	; high, and 18 pixels wide.
	;


		move.w	tf_XSize(a0),d0
		add.w	d0,d0				;require X size * 2

		btst.b	#FFB_WHITESPACED,efh_FontFlags+1(a4)
		beq.s	oef_maketags

	PRINTF	DBG_FLOW,<'EUC--[White Space Enhanced font TRUE]'>

		subq.w	#2,eof_TTextAttr+tta_YSize(a1)
		bgt.s	oef_decxsize			; signed > 0?

	; too small to render, don't even bother

		moveq	#00,d0
		bra	oef_nofont

oef_decxsize:
		subq.w	#2,d0
		bgt.s	oef_maketags

	; too small to render, don't even bother

		moveq	#00,d0
		bra	oef_nofont

oef_maketags:
		move.w	d0,d4				;cache X size request
		move.w	eof_TTextAttr+tta_YSize(a1),d5	;cache Y size request

		swap	d0
		move.w	d5,d0
		
		move.l	d0,eof_DPITag+ti_Data(a1)

	PRINTF	DBG_FLOW,<'EUC--[DPI Request = $%lx]'>,D0


	;
	; create font name from name, underscore, and key
	;
	; example -
	;
	; path is fonts:coral
	;
	; font name is coral.font
	;
	; key is $A1
	;
	; open fonts:coral/A1_coral.font
	;


		lea	efh_FontLoadName(a4),A1

	PRINTF	DBG_FLOW,<'EUC--[Master Font Name: %s]'>,A1

		move.l	sp,a3
		lea	nibbletoascii(pc),a0

		move.w	d2,d0
		lsr.b	#4,d0				;get upper nibble
		move.b	0(a0,d0.w),(a3)+

		move.w	d2,d0
		and.b	#$0F,d0				;get lower nibble
		move.b	0(a0,d0.w),(a3)+

		move.b	#'_',(a3)+

oef_copyfontname:
		move.b	(a1)+,(a3)+			;from TextFont struct name
		bne.s	oef_copyfontname		;to stack space

	; add font name to path name

		lea	efh_ExtensionPath(a4),a0
		lea	efh_PFontPath(a4),a3
		move.l	a3,d1				;for AddPart()

	PRINTF	DBG_FLOW,<'EUC--[Extension Path: %s]'>,A0

oef_copypath:
		move.b	(a0)+,(a3)+
		bne.s	oef_copypath
		
		move.l	sp,d2				;for AddPart()

	PRINTF	DBG_FLOW,<'EUC--[Add FileName: %s]'>,D2

	PRINTF	DBG_OSCALL,<'EUC-- AddPart($%lx,$%lx,%ld)'>,D1,D2,D3

		move.l	edvr_DOSBase(a5),a6
		JSRLIB	AddPart

	; finish off TTextAttr by setting pointer to font name (yet
	; to be created)

		add.l	d3,sp				;free stack space

		move.l	edvr_SysBase(a5),a6		;restore

		tst.l	d0				;check AddPart() success
		BEQ_S	oef_nofont

	PRINTF	DBG_FLOW,<'EUC--[AddPart succeeded]'>

	; create reply port for message / clear signal bit

		moveq	#00,d0
		moveq	#SIGF_SINGLE,d1
		JSRLIB	SetSignal

		lea	efh_ReplyPort(a4),a0
		move.l	ThisTask(a6),MP_SIGTASK(a0)

	; - init message

		lea	efh_Message(a4),a1

		move.b	#NT_MESSAGE,LN_TYPE(a1)
		move.w	#EUCOpenFont_SIZEOF,MN_LENGTH(a1)
		move.l	a0,MN_REPLYPORT(a1)

		lea	efh_PFontPath(a4),a0
		move.l	a0,eof_TTextAttr+tta_Name(a1)	;add to TTextAttr

	PRINTF	DBG_FLOW,<'EUC--[Font Path: %s]'>,A0

		lea	edvr_LoaderPort(a5),a0

	PRINTF	DBG_OSCALL,<'EUC-- PutMsg($%lx,$%lx)'>,A0,A1

		JSRLIB	PutMsg

		moveq	#SIGF_SINGLE,d0

	PRINTF	DBG_OSCALL,<'EUC-- Wait($%lx)'>,D0

		JSRLIB	Wait

		lea	efh_ReplyPort(a4),a0

	PRINTF	DBG_OSCALL,<'EUC-- GetMsg($%lx)'>,A0

		JSRLIB	GetMsg

		lea	efh_Message(a4),a0
		move.l	eof_TextFont(a0),d0
		move.l	d0,a1
		BEQ_S	oef_nofont

	PRINTF	DBG_FLOW,<'EUC--[Font returned @ $%lx]'>,D0

	; if X/Y size of returned font does not match request, fail and
	; close font.

	PUSHWORD	tf_XSize(a1)
	PRINTF	DBG_FLOW,<'EUC--[X size = %ld]'>
	POPLONG		1

	PUSHWORD	D4
	PRINTF	DBG_FLOW,<'EUC--[Expected X size = %ld]'>
	POPLONG		1

		cmp.w	tf_XSize(a1),d4
		BNE_S	oef_closefont

	PUSHWORD	tf_YSize(a1)
	PRINTF	DBG_FLOW,<'EUC--[Y size = %ld]'>
	POPLONG		1

	PUSHWORD	D5
	PRINTF	DBG_FLOW,<'EUC--[Expected Y size = %ld]'>
	POPLONG		1

		cmp.w	tf_YSize(a1),d5
		BNE_S	oef_closefont

		move.l	d0,efe_TextFont(a2)		;stash
		clr.w	efe_Retry(a2)			;reset retry

oef_havefont:
		move.l	d0,a0				;increment accessor
		addq.w	#1,tf_Accessors(a0)

	; Release process semaphore; D0 preserved by function

oef_nofont:

		lea	efh_HandleSemaphore(a4),a0

	PRINTF	DBG_OSCALL,<'EUC-- ReleaseSemaphore($%lx)'>

		JSRLIB	ReleaseSemaphore

		move.l	a2,a0				;restore element pointer
		movem.l	(sp)+,d1/d3-d5/a2-a6

	; We can now release the lock because the font accessor count
	; has been bumped.

	PRINTF	DBG_FLOW,<'EUC--[Return TextFont $%lx]'>,D0

		subq.w	#1,efe_Lock(a0)
		movem.l	(sp)+,d2/a0-a1
		rts

	;--
	;-- Close this font because diskfont returned something whose
	;-- X/Y size does not match our request.  Retry count is bumped
	;--

oef_closefont:
		move.l	edvr_GfxBase(a5),a6

	PRINTF	DBG_OSCALL,<'EUC-- CloseFont($%lx)'>,A1

		JSRLIB	CloseFont
		move.l	edvr_SysBase(a5),a6
		moveq	#00,d0
		BRA_S	oef_nofont

nibbletoascii:
		dc.b	'0123456789abcdef'

