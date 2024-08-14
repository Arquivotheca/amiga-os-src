
	    XREF    _LVOLoadView
	    XREF    _LVOMakeVPort

JMPLIB	MACRO
	XREF _LVO\1
	jmp  _LVO\1(a6)
	ENDM
JSRLIB	MACRO
	XREF _LVO\1
	jsr  _LVO\1(a6)
	ENDM
BLINK	MACRO
	bchg.b #1,$bfe001
	ENDM
	LIST

	    bra     main
;----------------------------------------------------------------------------
;	a2-View
;	a3-ViewPort
;   Write LoadView with:::::::::::::::::::::
;
;		 if (d->LOFCprList == 0)
;		 {
;			 goto no_cpr_list;
;		 }
;		 GB->Modes = d->Modes;
;		 GB->LOFlist = d->LOFCprList->start;
;		 if (d->SHFCprList) GB->SHFlist = d->SHFCprList->start;
;		 update_top_color();
;
MyLoadView: movem.l a2/a3,-(sp)
	    move.l  a1,a2	;View in A2
	    move.l  a1,a3	;Start of ViewPort chain
	    bra.s   ilk_enter

ilk_loop:   move.l  a2,a0	;view
	    move.l  a3,a1	;viewport
	    JSRLIB  MakeVPort
ilk_enter:  move.l  (a3),a3     ;Next viewport
	    move.l  a3,d0
	    bne.s   ilk_loop

	    move.l  a2,a1	;restore View to expected register
	    movem.l (sp)+,a2/a3
	    move.l  OldLoadView(pc),-(a7)
	    rts
;----------------------------------------------------------------------------



main:		move.l	4,a6

		lea.l  GfxName(pc),a1
		JSRLIB OldOpenLibrary
		move.l d0,GfxBase	;no error check
		lea.l	DOSName(pc),a1
		JSRLIB OldOpenLibrary
		move.l d0,DOSBase	;no error check

		JSRLIB	Forbid	;Protect all but Wait() and printf()
		lea.l	NewTaskName(pc),a1
		JSRLIB	FindTask	;See if we are already installed
		tst.l	d0
		beq.s	All_Clear

		move.l	d0,a1
		move.l	#1<<12,d0	;CTRL-C
		JSRLIB	Signal		;Kill the offender!
		JSRLIB	Permit
		lea.l	FailMessage(pc),a0
		move.l	#FailMessagee-FailMessage,d0
		bsr	PutMessage
		moveq	#5,d0		;RETURN_WARN
		bra	CloseEm

All_Clear:	suba.l	a1,a1
		JSRLIB	FindTask	;Find US
		move.l	d0,a0
		move.l	10(a0),OldTaskName
		lea.l	NewTaskName(pc),a1
		move.l	a1,10(a0)       ;Set our task's name! (LN_NAME)

;Patch vectors
		move.l GfxBase(pc),a1
		lea.l  MyLoadView(pc),a0
		move.l a0,d0
		move.w #_LVOLoadView,a0
		JSRLIB SetFunction
		move.l d0,OldLoadView

		lea.l  OkMessage(pc),a0
		move.l #OkMessagee-OkMessage,d0
		bsr    PutMessage

;Wait for CTRL-C to be hit
		move.l #1<<12,d0	;SIGBREAKF_CTRL_C
		JSRLIB Wait

;Unpatch vectors
		move.l OldLoadView(pc),d0
		move.w #_LVOLoadView,a0
		move.l GfxBase(pc),a1
		JSRLIB SetFunction

		suba.l a1,a1
		JSRLIB FindTask 	;Find US
		move.l d0,a0
		move.l OldTaskName(pc),10(a0)
		JSRLIB Permit
		moveq  #0,d0

;Close up libraries
CloseEm:
		move.l d0,-(a7)
		move.l GfxBase(pc),a1
		JSRLIB CloseLibrary
		move.l DOSBase(pc),a6
		moveq  #50,d1
		JSRLIB Delay	; Let things settle down
		move.l DOSBase(pc),a1
		move.l 4,a6
		JSRLIB CloseLibrary

		move.l	(a7)+,d0
		rts

;a0-address
;d0-size
PutMessage	movem.l a6/d2/d3,-(a7)
		move.l	a0,d2
		move.l	d0,d3
		move.l	DOSBase(pc),a6
		JSRLIB	Output
		move.l	d0,d1
		beq.s	no_handle
		JSRLIB	Write
no_handle	movem.l (a7)+,a6/d2/d3
		rts

OldTaskName	dc.l 0
OldLoadView	dc.l 0
GfxBase 	dc.l 0
DOSBase 	dc.l 0

GfxName 	dc.b 'graphics.library',0
DOSName 	dc.b 'dos.library',0

FailMessage
 dc.b 10,'The bylaws of the 23rd anual intergalactic bug basher',$27,'s',10
 dc.b 'ball specifically forbid having more than one fixlace active.',10
 dc.b 'That other fixlace has been KILLED.  Moo-Hahahahahah!',10
FailMessagee
OkMessage
 dc.b 10,'You have run '
NewTaskName
 dc.b 'Bryce',$27,'s Deadly Interlace Fixer ("fixlace")',0,10,10
 dc.b 'Use CTRL-C, fixlace, or the BREAK command to kill me.',10
OkMessagee
	    CNOP    0,4


	    END
