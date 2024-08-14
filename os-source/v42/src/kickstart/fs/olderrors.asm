		SECTION	filesystem,CODE

		NOLIST
		include	"exec/types.i"
		include	"exec/alerts.i"
		include	"libraries/dosextens.i"
		include	"intuition/intuition.i"
		include	"globals.i"
		include	"printf.mac"
		LIST

;		DEBUGENABLE

		XDEF	Request

		XREF	_LVOAlert,_LVOOpenLibrary,_LVOCloseLibrary,_LVOFreeMem
		XREF	GetPubMem,_LVOAutoRequest,_AbsExecBase

STACKSIZE EQU 2048
NEGSTACK  EQU 4-2048

;==============================================================================
; result = Request( line1,line2,line3 )
;   d0		     a0    a1    a2
;
; builds a system requester with the given three lines of text and puts it up
; in the current processes window.  Waits for user response and returns TRUE
; if user clicked on retry or false if user clicked on cancel.
;==============================================================================
Request		movem.l	d2-d3/d6-d7/a2-a4,-(sp)
		movea.l	a0,a3			save lines 2 and 3
		movea.l	a1,a4
		moveq.l	#FALSE,d6		assume false return

		movea.l	ThisTask(a5),a0		test window pointer
		tst.l	pr_WindowPtr(a0)
		bmi	RequestDone		no window, return false

		move.l	#STACKSIZE,d0		get stack space
		bsr	GetPubMem
		tst.l	d0
		beq	RequestDone		no memory, return false
		exg.l	d0,sp			switch to new stack
		lea.l	STACKSIZE(sp),sp
		move.l	d0,-(sp)		and save old stack pointer
		lea.l	IntName(pc),a1		open intuition
		moveq.l	#33,d0			this version
		jsr	_LVOOpenLibrary(a6)
		tst.l	d0
		beq	RequestFailed		didn't get intuition
		movea.l	d0,a6			using intuition lib now

; convert the 3 supplied strings to a list of IntuiText structures on the stack
		move.l	sp,d7			save the current stack pointer
		lea.l	-it_SIZEOF(sp),sp	get an intuitext struct
		movea.l	sp,a0			where struct is
		movea.l	a3,a1			first string
		moveq.l	#15,d0			initial leftedge
		moveq.l	#5,d1			initial topedge
		bsr	InitText		initialise the text

		clr.l	it_NextText(sp)		assume no more text
		cmpa.w	#0,a4			is there a second line ?
		beq.s	10$			nope, look for third line
		lea.l	-it_SIZEOF(sp),a0	get next struct
		move.l	a0,it_NextText(sp)	link to previous
		movea.l	a0,sp			and get stack space
		movea.l	a4,a1			this string
		moveq.l	#15,d0			leftedge
		moveq.l	#15,d1			topedge
		bsr	InitText

10$		clr.l	it_NextText(sp)		assume no more text
		cmpa.w	#0,a2			is there a third line ?
		beq.s	20$			nope, go do gadget text
		lea.l	-it_SIZEOF(sp),a0	get next struct
		move.l	a0,it_NextText(sp)	link to previous
		movea.l	a0,sp			and get stack space
		movea.l	a2,a1			this string
		moveq.l	#15,d0			leftedge
		moveq.l	#25,d1			topedge
		bsr	InitText
		clr.l	it_NextText(sp)		last in bodytext list

20$		lea.l	-it_SIZEOF(sp),sp
		movea.l	sp,a2			this will be PositiveText
		movea.l	sp,a0
		lea.l	RetryMess(pc),a1	text
		moveq.l	#AUTOLEFTEDGE,d0
		moveq.l	#AUTOTOPEDGE,d1
		bsr	InitText
		clr.l	it_NextText(sp)

		lea.l	-it_SIZEOF(sp),sp
		movea.l	sp,a3			this will be NegativeText
		movea.l	sp,a0
		lea.l	CancelMess(pc),a1	text
		moveq.l	#AUTOLEFTEDGE,d0
		moveq.l	#AUTOTOPEDGE,d1
		bsr.s	InitText
		clr.l	it_NextText(sp)

		move.l	#DISKINSERTED,d0	positive flags
		moveq.l	#0,d1			negative flags
		move.l	#320,d2			width
		moveq.l	#72,d3			height

		movea.l	ThisTask(a5),a0		get the window pointer
		movea.l	pr_WindowPtr(a0),a0
		movea.l	d7,a1			point to body text
		lea.l	-it_SIZEOF(a1),a1	first entry in body text

		jsr	_LVOAutoRequest(a6)	display requester
		move.l	d0,d6			stash result
		movea.l	a6,a1			close down intuition
		movea.l	_AbsExecBase,a6
		jsr	_LVOCloseLibrary(a6)
		move.l	d7,sp			reclaim stack space used

RequestFailed	lea.l	NEGSTACK(sp),a1		point to beginning of stack
		move.l	(sp)+,sp		back to previous stack
		move.l	#STACKSIZE,d0
		jsr	_LVOFreeMem(a6)		free up the new stack

RequestDone	move.l	d6,d0			retrieve return code
		movem.l	(sp)+,d2-d3/d6-d7/a2-a4
		rts

RetryMess	DC.B	'Retry',0
		CNOP	0,2
CancelMess	DC.B	'Cancel',0
		CNOP	0,2
IntName		DC.B	'intuition.library',0
		CNOP	0,2

;==============================================================================
; InitText( itext, text, left, top )
;	     a0     a1    d0    d1
;
; Initialises an intuitext structure with the given text and position.
;==============================================================================
InitText	move.b	#AUTOFRONTPEN,it_FrontPen(a0)
		move.b	#AUTOBACKPEN,it_BackPen(a0)
		move.b	#AUTODRAWMODE,it_DrawMode(a0)
		move.w	d0,it_LeftEdge(a0)
		move.w	d1,it_TopEdge(a0)
		move.l	#AUTOITEXTFONT,it_ITextFont(a0)
		move.l	a1,it_IText(a0)
		rts

		END
