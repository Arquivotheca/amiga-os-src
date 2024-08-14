*
* lvwedge.asm for grabber
*
* C part must
*	extern MainTask, WedgedTask, ULONG fcount, and BOOLS DoView, Off
*	set MainTask
*	call WedgeLoadView
*	wait for SAVESIG | SIGBREAKF_CTRL_C
*		save screen or view (based on DoView) if SAVESIG, signal DONESIG, loop back
*		set Quiet_A, Delay(), and exit if CTRL_C
*

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/macros.i"
	INCLUDE "dos/dos.i"

	XDEF	_WedgeLoadView
	XDEF	_UnWedgeLoadView
	XDEF	_MainTask
	XDEF	_WedgedTask
	XDEF	_fcount
* BOOLS
	XDEF	_Off
	XDEF	_DoView

	XREF	_LVOLoadView
	XREF	_LVOSignal
	XREF	_LVOFindTask

	XREF	KGetChar
	XREF	KMayGetChar
	XREF	KPutStr
	XREF	KPutChar
	XREF	KPrintF

SAVESIG	EQU	SIGBREAKF_CTRL_F
DONESIG	EQU	SIGBREAKF_CTRL_E

	CODE

_WedgeLoadView:
	movem.l	d1-d7/a0-a6,-(sp)

	move.l	4,a6			;exec
	lea	libname(pc),a1		;graphics library
	moveq.l	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq	badopen
	move.l	d0,a5			;GfxBase (a5)
	move.l	d0,libbase(PC)

	movea.l	a5,a1			;library to setfunction
	lea.l	wedge(PC),a0		;new code
	move.l	a0,d0
	movea.l #_LVOLoadView,a0	;offset

	JSRLIB	SetFunction
	move.l	d0,oldfunc(PC)		;save to restore later

* Keep open and close on InWedge
*	movea.l	a5,a1			;GfxBase
*	move.l	4,a6
*	JSRLIB	CloseLibrary

	moveq	#1,d0			;return TRUE
badopen:
	movem.l	(sp)+,d1-d7/a0-a6
	rts



_UnWedgeLoadView:
	movem.l	d1-d7/a0-a6,-(sp)

	move.w	#1,_Off(PC)

	move.l	4,a6			;exec
	movea.l	libbase(PC),a5		;GfxBase	(opened above)
	move.l	a5,d0
	beq.s	badclose

	movea.l	a5,a1			;library to setfunction
	movea.l #_LVOLoadView,a0	;offset
	move.l	oldfunc(PC),d0		;old code
	JSRLIB	SetFunction		;unsafe - we don't compare to make sure we weren't wedged

	movea.l	a5,a1			;GfxBase
	JSRLIB	CloseLibrary

	moveq	#1,d0			;return TRUE
badclose:
	movem.l	(sp)+,d1-d7/a0-a6
	rts



* The wedge
wedge:
	pea.l   comeback(pc)		;return here
	move.l	oldfunc(pc),-(sp)	;real function
	rts				;go there

comeback:
	addq.l	#1,_fcount(PC)		;frame count
	tst.w	_Off(PC)
	beq.s	doit
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	4,a6			; execbase
	jsr	KMayGetChar
	cmpi    #'q',d0			; quit ?
	beq	quit			; yes
	cmpi    #'n',d0			; turn back on ?
	bne.s	comebackout
	move.w	#0,_Off(PC)
	bra.s	doit2
comebackout	
	movem.l	(sp)+,d0-d7/a0-a6	; restore all registers
	rts

doit
	movem.l	d0-d7/a0-a6,-(sp)
doit2
	move.l	4,a6			;exec

clbuf1
	jsr	KMayGetChar		;clear buffer
	cmpi.l	#-1,d0			;no - clear buffer
	bne	clbuf1

*	lea.l	prompt,a0		;kprint prompt
*	jsr	KPutStr

	lea.l   prompt(PC),a0		;kprintf prompt with frame count
	lea.l   _fcount(PC),a1
	jsr	KPrintF
		
	jsr	KGetChar		;get answer
	move.l	d0,answer
*	jsr	KPutChar		;echo
	move.l	#10,d0
	jsr	KPutChar		;LF

	move.l  answer,d0		;tolower
	ori     #$20,d0
	cmpi    #'n',d0			;next ?
	bne.s	checks
	move.w	#0,_Off(PC)
	bra	outawedge		;yes - continue
checks
	cmpi	#'s',d0			;screensave ?
	bne.s	checkv			; no
	move.w	#0,_DoView
	beq.s	saveit
checkv
	cmpi	#'v',d0			;viewsave ?
	bne.s	checko			; no
	move.w	#1,_DoView
saveit
	movea.l	#0,a1
	JSRLIB	FindTask
	move.l	d0,_WedgedTask(PC)
	movea.l	_MainTask(PC),a1
	move.l	#SAVESIG,d0
	JSRLIB	Signal
	move.l	#DONESIG,d0
	JSRLIB	Wait			; wait for save to be completed
	bra.s	outawedge

checko
	cmpi	#'o',d0			;off ?
	bne.s	checkq			; no
	move.w	#1,_Off(PC)
	lea.l	offstr,a0		;kprint prompt
	jsr	KPutStr
	bra.s	outawedge

checkq
	cmpi	#'q',d0			;quit ?
	bne	clbuf1			; no

quit
	lea.l	quitstr,a0		;kprint prompt
	jsr	KPutStr

	move.w	#1,_Off(PC)
	movea.l	_MainTask(PC),a1
	move.l	#SIGBREAKF_CTRL_C,d0
	JSRLIB	Signal


outawedge:
	movem.l	(sp)+,d0-d7/a0-a6	; restore all registers
	rts				; exit



	CNOP	0,2

_MainTask	DC.L	0
_WedgedTask	DC.L	0

_fcount	DC.L	0
oldfunc	DC.L	0
libbase DC.L	0
answer	DC.L	0

_Off	DC.W	0
_DoView	DC.W	0

libname	DC.B	'graphics.library',0
prompt	DC.B	'%04ld: N=Next/On O=Off S=Screensave V=Viewsave Q=Quit: ',0
quitstr	DC.B	'Grabber: exiting',10,0
offstr	DC.B	'Grabber: off',10,0

	END
