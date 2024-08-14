		SECTION	mainprog,CODE
		INCLUDE	"exec/types.i"
		INCLUDE	"libraries/dos.i"
		INCLUDE	"intuition/intuition.i"
		INCLUDE	"../janus/janusvar.i"
		INCLUDE	"../janus/janusreg.i"
		INCLUDE	"../janus/janus.i"
		INCLUDE	"mydata.i"

		XDEF	MainProgram

		XREF	_AbsExecBase,_LVOWait,_LVOGetMsg,_LVOReplyMsg
		XREF	_LVOModifyIDCMP,_LVOPrintIText
		XREF	_LVOBeginRefresh,_LVOEndRefresh
		XREF	_LVOSetAPen,_LVOMove,_LVODraw

		XREF	A000Gadget,MonoGadget,OffText,OnText
		XREF	ColorGadget,SerialGadget

;==========================================================================
; Called from the initialisation section with A5 pointing to the data area.
;==========================================================================

MainProgram	move.l	#-1,FinishedFlag(a5)	non zero = not finished
		bsr	RefreshWindow		draw non gadget stuff

WaitLoop	movea.l	_AbsExecBase,a6		exec library pointer
		move.l	a6,FinishedFlag(a5)	non zero in finished flag
		move.l	MSigs(a5),d0		signal to wait for
		jsr	_LVOWait(a6)		go to sleep

; got woken up by an intuition message, can only be refresh or gadget msgs

MsgLoop		movea.l	MPort(a5),a0		message port that woke us
		jsr	_LVOGetMsg(a6)		fetch the message
		tst.l	d0			did we get one ?
		beq.s	WaitLoop		no message, no work!!
		movea.l	d0,a2			stash the message ptr
		move.l	im_Class(a2),d0		see what the message was
		cmpi.l	#REFRESHWINDOW,d0
		bne.s	10$
		movea.l	Ilib(a5),a6		set up layers
		movea.l	PrefWindow(a5),a0
		jsr	_LVOBeginRefresh(a6)
		bsr	RefreshWindow		do a refresh
		movea.l	PrefWindow(a5),a0
		jsr	_LVOEndRefresh(a6)	undo layers setup
		bra.s	ReplyToMessage		and reply it
10$		cmpi.l	#GADGETUP,d0
		bne.s	20$
		bsr	HandleGadgetUp		handle gadget up message
		bra.s	ReplyToMessage		and reply to it
20$		cmpi.l	#GADGETDOWN,d0
		bne.s	ReplyToMessage		not a message we recognise
		bsr	HandleGadgetDown	handle gadget down message

; Message has been received and dispatched, so now return it to intuition

ReplyToMessage	movea.l	_AbsExecBase,a6
		movea.l	a2,a1
		jsr	_LVOReplyMsg(a6)
		tst.l	FinishedFlag(a5)	did we finish yet ?
		bne.s	MsgLoop			no, process more info

; The last function called set the finished flag to 0 so it's time to quit

		movea.l	Ilib(a5),a6		intuition library
		movea.l	PrefWindow(a5),a0	stop further msgs arriving
		moveq.l	#0,d0
		jsr	_LVOModifyIDCMP(a6)
		movea.l	_AbsExecBase,a6		and strip existing msgs
StripMessages	movea.l	MPort(a5),a0
		jsr	_LVOGetMsg(a6)
		tst.l	d0
		beq.s	MainExit		no more messages
		movea.l	d0,a1
		jsr	_LVOReplyMsg(a6)
		bra.s	StripMessages
MainExit	rts

;=========================================================================
; Handles REFRESHWINDOW event from the intuition IDCMP (A2 = message ptr)
;=========================================================================

RefreshWindow	movem.l	a2/a6,-(sp)
		movea.l	Glib(a5),a6		get graphics lib ptr
		movea.l	PrefWindow(a5),a2	get windows RastPort
		movea.l	wd_RPort(a2),a2
		moveq.l	#1,d0			set pen to white
		movea.l	a2,a1
		jsr	_LVOSetAPen(a6)
		movea.l	a2,a1			move to left edge
		moveq.l	#0,d0			below top row of gadgets
		moveq.l	#30,d1
		jsr	_LVOMove(a6)
		movea.l	a2,a1			draw along to right edge
		move.l	#300,d0
		moveq.l	#30,d1
		jsr	_LVODraw(a6)
		movea.l	a2,a1
		moveq.l	#104,d0			draw down side of RAM...
		moveq.l	#30,d1			...gadgets
		jsr	_LVOMove(a6)
		movea.l	a2,a1
		moveq.l	#104,d0
		moveq.l	#100,d1
		jsr	_LVODraw(a6)
		movea.l	a2,a1			make it double thickness
		moveq.l	#105,d0
		moveq.l	#30,d1
		jsr	_LVOMove(a6)
		movea.l	a2,a1
		moveq.l	#105,d0
		moveq.l	#100,d1
		jsr	_LVODraw(a6)
		bsr	RefreshGadgets		highlight correct gadget
		bsr	RefreshSwitches		do all the on/off gadgets
		movem.l	(sp)+,a2/a6
		rts

;=========================================================================
; Handle Gadget down events
;=========================================================================

HandleGadgetDown:
		lea.l	A000Gadget,a0		deselect other gadgets
		movea.l	im_IAddress(a2),a1	that this one excludes
		moveq.l	#3-1,d0			checking 3 gadgets
10$		ori.w	#SELECTED,gg_Flags(a0)	select this one
		cmpa.l	a0,a1			is this the selected one
		bne.s	20$			no, so deselect it
		move.w	gg_GadgetID(a0),d1	save ram state flag
		move.b	d1,RamBank(a5)		for later save
		bra.s	30$			and go check the next
20$		andi.w	#~SELECTED,gg_Flags(a0)	
30$		movea.l	gg_NextGadget(a0),a0	point to next gadget
		dbra	d0,10$			drop through to refresh

; Render a box around the currently selected gadget (Intuition screws up!)

RefreshGadgets	movem.l	d2/a2/a6,-(sp)
		movea.l	Glib(a5),a6		set pen to erase
		moveq.l	#0,d0
		movea.l	PrefWindow(a5),a0
		movea.l	wd_RPort(a0),a1
		jsr	_LVOSetAPen(a6)

		lea.l	A000Gadget,a2		first erase non selected
		moveq.l	#3-1,d2
10$		move.w	#SELECTED,d0
		and.w	gg_Flags(a2),d0		is this one selected
		bne.s	20$			yes, don't erase it
		movea.l	a2,a0
		bsr.s	GadgetHighlight
20$		movea.l	gg_NextGadget(a2),a2
		dbra	d2,10$

		moveq.l	#3,d0			now set pen to orange
		movea.l	PrefWindow(a5),a0
		movea.l	wd_RPort(a0),a1
		jsr	_LVOSetAPen(a6)

		lea.l	A000Gadget,a2
		moveq.l	#3-1,d2
30$		move.w	#SELECTED,d0		now highlight selected
		and.w	gg_Flags(a2),d0
		beq.s	40$			not this one
		movea.l	a2,a0
		bsr.s	GadgetHighlight
40$		movea.l	gg_NextGadget(a2),a2
		dbra	d2,30$

		movem.l	(sp)+,d2/a2/a6
		rts

;=========================================================================
; Handles the highlight draw/erase of an individual gadget.  Pen should
; already be set to the required color ( 0 or 3 ) and a0 points to gadget
;=========================================================================

GadgetHighlight	movem.l	d2-d5/a2/a6,-(sp)
		movea.l	a0,a2			save gadget address

; First draw two horizontal bars above the gadgets hit box

		move.w	gg_LeftEdge(a2),d2	get gadget sizes
		move.w	d2,d4
		add.w	gg_Width(a2),d4
		subq.w	#4,d2			drawing above gadget
		addq.w	#3,d4
		move.w	gg_TopEdge(a2),d3
		subq.w	#1,d3
		move.w	d3,d5
		bsr	DrawLine		draw line in d2-d5
		subq.w	#1,d3			and draw a line above it
		subq.w	#1,d5
		bsr	DrawLine

; Now draw two horizontal bars below the gadgets hit box

		move.w	gg_TopEdge(a2),d3
		add.w	gg_Height(a2),d3
		move.w	d3,d5
		bsr	DrawLine		widths stay the same
		addq.w	#1,d3
		addq.w	#1,d5
		bsr	DrawLine

; Draw four vertical bars against the left edge of the gadget

		move.w	gg_TopEdge(a2),d3
		move.w	d3,d5
		add.w	gg_Height(a2),d5
		move.w	gg_LeftEdge(a2),d2
		subq.w	#1,d2
		move.w	d2,d4
		bsr	DrawLine
		subq.w	#1,d2
		subq.w	#1,d4
		bsr	DrawLine
		subq.w	#1,d2
		subq.w	#1,d4
		bsr	DrawLine
		subq.w	#1,d2
		subq.w	#1,d4
		bsr	DrawLine

; Draw four vertical bars against the right edge of the gadget

		move.w	gg_LeftEdge(a2),d2
		add.w	gg_Width(a2),d2
		move.w	d2,d4
		bsr	DrawLine
		addq.w	#1,d2
		addq.w	#1,d4
		bsr	DrawLine
		addq.w	#1,d2
		addq.w	#1,d4
		bsr	DrawLine
		addq.w	#1,d2
		addq.w	#1,d4
		bsr	DrawLine
		movem.l	(sp)+,d2-d5/a2/a6
		rts

; This is a support routine for GadgetHighlight it trashes A6 so should
; not be called from anywhere else.  Draws the line D2,D3 to D4,D5.

DrawLine	movea.l	Glib(a5),a6
		movea.l	PrefWindow(a5),a0
		move.l	wd_RPort(a0),-(sp)	stash rastport address
		move.w	d2,d0
		move.w	d3,d1
		movea.l	(sp),a1
		jsr	_LVOMove(a6)
		move.w	d4,d0
		move.w	d5,d1
		movea.l	(sp)+,a1
		jmp	_LVODraw(a6)		JMP/RTS

; Handles gadgetup events from the boolean switches and the exit switches

HandleGadgetUp	movea.l	im_IAddress(a2),a0
		move.w	gg_GadgetID(a0),d0
		cmpi.w	#2,d0
		bgt.s	10$
		clr.l	FinishedFlag(a5)
		move.w	d0,FinishCode(a5)	set up code to quit with
		bra.s	20$

10$		clr.b	SerialState(a5)		clear out bool flags
		clr.b	MonoState(a5)
		clr.b	ColorState(a5)
		bsr.s	RefreshSwitches
		lea.l	MonoGadget,a0		set up bool flags now
		move.w	#SELECTED,d0
		and.w	gg_Flags(a0),d0
		beq.s	11$			mono is off
		move.b	#1,MonoState(a5)	mono is on
11$		lea.l	ColorGadget,a0
		move.w	#SELECTED,d0
		and.w	gg_Flags(a0),d0
		beq.s	12$			color is off
		move.b	#1,ColorState(a5)	color is on
12$		lea.l	SerialGadget,a0
		move.w	#SELECTED,d0
		and.w	gg_Flags(a0),d0
		beq.s	20$			serial is off
		move.b	#1,SerialState(a5)	serial is on
20$		rts

; Refreshes the gadgets that have on or off printed into them depending on
; thier current state.  Uses intuitext to render into the gadget box

RefreshSwitches	movem.l	a2/a3/a6,-(sp)
		lea.l	MonoGadget,a2		point to first switch
		movea.l	PrefWindow(a5),a0
		movea.l	wd_RPort(a0),a3		save rastport ptr
		movea.l	Ilib(a5),a6		and intuition ptr
10$		lea.l	OffText,a1		assume off
		move.w	#SELECTED,d0
		and.w	gg_Flags(a2),d0
		beq.s	20$			it is off
		lea.l	OnText,a1		nope, it's on
20$		movea.l	a3,a0			fetch rastport
		clr.l	d0
		clr.l	d1
		move.w	gg_LeftEdge(a2),d0
		move.w	gg_TopEdge(a2),d1
		jsr	_LVOPrintIText(a6)	display the text
		tst.l	gg_NextGadget(a2)
		beq.s	30$			no more gadgets
		movea.l	gg_NextGadget(a2),a2
		bra.s	10$			go for the next
30$		movem.l	(sp)+,a2/a3/a6
		rts

	 	END
