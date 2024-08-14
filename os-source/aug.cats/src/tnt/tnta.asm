
*
* tnta.asm - C.Scheppner 09/90, 2/93
*    Position independent code part of tnt debugging trap code
*
* mods - 09/90 - come up on NULL window
*        03/91 - stack dump
*	 02/93 - SERIAL option
*	 03/93 - SegTracker support

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"intuition/intuition.i"
	INCLUDE	"intuition/intuitionbase.i"

	XREF	_LVORawDoFmt
	XREF	_LVOWait
	XREF	_LVOOpenLibrary
	XREF	_LVOCloseLibrary
	XREF	_LVOAutoRequest
	XREF	_LVOAlert
	
	XDEF	_startcode
	XDEF	_endcode
	XDEF	_useCountT
	XDEF	_useCountW
	XDEF	_ourAddTaskCode
	XDEF	_realAddTaskAddr
	XDEF	_execbase
	XDEF	_Serial
	XDEF	_func_segfind

_LVORawPutChar          EQU     -516

* actually 4 higher until I subtract 4 from a5 (SSP)
EXNUM	EQU	60
UPC00	EQU	66

  STRUCTURE StkVars,0
    LONG alertnum
    LONG upc
    STRUCT ITextB1,it_SIZEOF
    STRUCT ITextB2,it_SIZEOF
    STRUCT ITextB3,it_SIZEOF
    STRUCT ITextB4,it_SIZEOF
    STRUCT ITextB5,it_SIZEOF
    STRUCT ITextB6,it_SIZEOF
    STRUCT ITextB7,it_SIZEOF
    STRUCT ITextPG,it_SIZEOF
    STRUCT ITextNG,it_SIZEOF
    STRUCT TAttr,ta_SIZEOF
    LABEL sv_SIZEOF 

    SECTION CODE


_startcode:
					;exception code is on super stack
	movem.l	d0-d7/a0-a6,-(sp)	;save registers on super stack
	move.l	USP,a4			;save user stack pointer in a4
	move.l	sp,a5			;save super stack pointer in a5 
	andi.w	#(~$2000),SR		;into user mode

	suba.l	#sv_SIZEOF,sp		;get my stack
	lea.l	_useCountT(PC),a0	;increment trap useCount
	addq.l	#1,(a0)

	movea.l	_execbase(PC),a6
	
*----- Need Exception number, Task address, PC, SP, SSP
	lea.l	nbuf(PC),a1		; a1 = nbuf
	move.l	a4,12(a1)		; get USP from where I stashed it


	move.l	EXNUM(a5),a0		; get exception number
	move.l	a0,0(a1)		; show exception number
	andi.l	#$00FF,0(a1)
	move.l	a0,alertnum(sp)		; save as deadend for possible alert
	ori.l	#$80000000,alertnum(sp)

	move.l	ThisTask(a6),a0		; Task to a0
	move.l	a0,4(a1)		; show task address
	move.l	#UPC00,d0		;offset on superstack to user PC
	cmp.l	#3,EXNUM(a5)		;unless exception 3
	bne.s	not3			;

	btst.b	#AFB_68010,AttnFlags+1(a6)     ;on a plain 68000
	bne.s	not3
	add.l	#8,d0			;in which case it is 8 bytes further
not3:
	move.l	0(a5,d0.l),8(a1)	; PC
	move.l	8(a1),upc(sp)		; also save for possible alert
	move.l	a5,16(a1)		; SSP
	lea.l	tstr(PC),a0
	lea.l	sbuf1(PC),a3
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;sbuf1 = line1

	move.l	a5,a1			; D registers at 0 SSP offset
	lea.l	dstr(PC),a0
	lea.l	sbuf2(PC),a3
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;sbuf2 = line2

	lea.l	32(a5),a1		;A registers next, except A7
	lea.l	astr(PC),a0		;format string
	lea.l	sbuf3(PC),a3		;output buffer
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;format all but a7, sbuf3 = line3

	move.l	a4,a1			;stack dump 1
	lea.l	sstr(PC),a0		;format string
	lea.l	sbuf4(PC),a3		;output buffer
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;sbuf4 = line4

	move.l	a4,a1			;stack dump 2
	adda.l	#32,a1
	lea.l	sstr(PC),a0		;format string
	lea.l	sbuf5(PC),a3		;output buffer
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;sbuf5 = line5

*----- Now put task/command name in left gadget
	movea.l	ThisTask(a6),a1		;this task
	cmpi.b	#NT_PROCESS,LN_TYPE(a1)	;process ?
	bne.s	justtask		;no
	move.l	pr_CLI(a1),d0		;cli process ?
	beq.s	justproc		;no
	lsl.l	#2,d0			;bptr to ptr
	move.l	d0,a1			;cli ptr in a1
	move.l	cli_CommandName(a1),d0	;is a command running ?
	beq.s	justtask		;no

	lsl.l	#2,d0			;bptr to ptr
	movea.l	d0,a1			;ptr command name bstr in a1
	moveq.l	#0,d0			;clear out d0
	move.b	(a1)+,d0		;len to d0 while bump a1 to 1st char
	clr.b	0(a1,d0.l)		;null terminate
	lea.l	cnstr(PC),a0		;command name format string
	bra.s	doname
justtask:
	lea.l	tnstr(PC),a0		;task name format string
	bra.s	tpname
justproc:
	lea.l	pnstr(PC),a0		;process name format string
tpname:
	movea.l	LN_NAME(a1),a1		;show task/process name
doname:
*----- Store name ptr in nbuf and point a1 there for RawDoFmt stream
*----- format string already in a0
	lea.l	nbuf(PC),a3		;temp buffer for args
	move.l	a1,0(a3)		;ptr to task/proc/command name
	movea.l	a3,a1			;arg pointer for format string
	lea.l	sbuf6(PC),a3		;output buffer
	lea.l	putchproc(PC),a2
	jsr	_LVORawDoFmt(a6)	;sbuf6 is taskname/command line


doseg:
	lea.l	noststr(PC),a0		;no segtracker = format string
	move.l	_func_segfind(PC),d0
	beq.s	sprsegt			;sprint no segtracker

dosegst:
	lea.l	nosegstr(PC),a0		;no segment found
	* check PC
	move.l	upc(sp),d0		;PC to d0
	bsr	checkseg		;in a seglist ?
	tst.l	d0
	beq.s	notpcseg		;no
	lea.l	pcsegstr(PC),a0		;yes
	bra.s	sprsegt

notpcseg:
	* check addresses on stack
	move.l	0,d2
stloop:
	movea.l	0(a4,d2.w),a0		;a longword on the user stack
	move.l	a0,d0
	bsr	checkseg
	tst.l	d0
	bne.s	gotstseg
	add.l	#4,d2
	cmp.l	#72,d2
	ble.s	stloop
	bra.s	noseg

gotstseg:
	lea.l	stsegstr(PC),a0
	bra.s	sprsegt

noseg:
	lea.l	nosegstr(PC),a0

* a0 = format string
* d0 = name, if any
sprsegt:
	lea.l	nbuf(PC),a3		;temp buffer for args
	move.l	segaddr(PC),0(a3)	;store arg stream for sprint 
	move.l	d0,4(a3)
	move.l	hunk(PC),8(a3)
	move.l	offset(PC),12(a3)
	movea.l	a3,a1			;args pointer for format string
	lea.l	sbuf7(PC),a3		;output buffer
	lea.l	putchproc(PC),a2	;will trash a3
	jsr	_LVORawDoFmt(a6)	;sbuf6 is taskname/command line

dorequest:

*----- Set Up TextAttr, IntuiTexts
	lea.l	TAttr(sp),a1
	lea.l	topazname(pc),a2
	move.l	a2,ta_Name(a1)
	move.w	#8,ta_YSize(a1)
	move.b	#FS_NORMAL,ta_Style(a1)
	move.b	#FPF_ROMFONT,ta_Flags(a1)

	lea.l	ITextB1(sp),a1			;body text 1
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #2,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf1(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB2(sp),a2
	move.l	a2,it_NextText(a1)

	lea.l	ITextB2(sp),a1			;body text 2
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #12,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf2(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB3(sp),a2
	move.l	a2,it_NextText(a1)

	lea.l	ITextB3(sp),a1			;body text 3
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #22,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf3(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB4(sp),a2
	move.l	a2,it_NextText(a1)


	lea.l	ITextB4(sp),a1			;body text 4
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #32,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf4(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB5(sp),a2
	move.l	a2,it_NextText(a1)

	lea.l	ITextB5(sp),a1			;body text 5
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #42,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf5(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB6(sp),a2
	move.l	a2,it_NextText(a1)

	lea.l	ITextB6(sp),a1			;body text 6
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #52,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf6(pc),a2
	move.l	a2,it_IText(a1)
	lea.l	ITextB7(sp),a2
	move.l	a2,it_NextText(a1)

	lea.l	ITextB7(sp),a1			;body text 7
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #62,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sbuf7(pc),a2
	move.l	a2,it_IText(a1)
	move.l	#0,it_NextText(a1)

	lea.l	ITextPG(sp),a1			;pos gad text
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #AUTOTOPEDGE,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	sustr(pc),a2
	move.l	a2,it_IText(a1)
	move.l	#0,it_NextText(a1)

	lea.l	ITextNG(sp),a1			;neg gad text
	move.b	#AUTOFRONTPEN,it_FrontPen(a1)
	move.b	#AUTOBACKPEN,it_BackPen(a1)
	move.b	#AUTODRAWMODE,it_DrawMode(a1)
	move.b	#0,it_KludgeFill00(a1)
	move.w	#AUTOLEFTEDGE,it_LeftEdge(a1)
	move.w  #AUTOTOPEDGE,it_TopEdge(a1)
	lea.l	TAttr(sp),a2
	move.l	a2,it_ITextFont(a1)
	lea.l	rebstr(pc),a2
	move.l	a2,it_IText(a1)
	move.l	#0,it_NextText(a1)

hmmm:
	tst.w	_Serial(PC)
	beq.s	noserial
	movem.l d0-d7/a0-a6,-(sp)
	lea.l	sbuf1(pc),a2			;body text 1
	bsr	kputs
	lea.l	sbuf2(pc),a2			;body text 2
	bsr	kputs
	lea.l	sbuf3(pc),a2			;body text 3
	bsr	kputs
	lea.l	sbuf4(pc),a2			;body text 4
	bsr	kputs
	lea.l	sbuf5(pc),a2			;body text 5
	bsr	kputs
	lea.l	sbuf6(pc),a2			;task/command name
	bsr	kputs
	lea.l	sbuf7(pc),a2			;segtracker
	bsr	kputs

	movem.l (sp)+,d0-d7/a0-a6
noserial:
	moveq	#0,d0
	lea.l	IName(pc),a1
	jsr	_LVOOpenLibrary(a6)
	tst.l	d0
	beq	barf

	move.l	d0,a6
	movea.l	#0,a0
	lea.l	ITextB1(sp),a1
	lea.l	ITextPG(sp),a2
	lea.l	ITextNG(sp),a3
	moveq	#0,d0
	moveq	#0,d1
	move.l	#640,d2
	move.l	#126,d3
	jsr	_LVOAutoRequest(a6)
	move.l	d0,-(sp)		;save return value
	move.l	a6,a1
	movea.l	_execbase(PC),a6
	jsr	_LVOCloseLibrary(a6)	;Close Intuition
	move.l	(sp)+,d0		;return value from AutoRequest

comeback:
	lea.l	_useCountT(PC),a0	;decrement trap useCount
	subq.l	#1,(a0)

	move.l	alertnum(sp),d7		;set up for alert just in case
	move.l	upc(sp),a5
	movea.l	_execbase(PC),a6	;need exec either way

	adda.l	#sv_SIZEOF,sp		;restore the stack pointer

	tst.l	d0			;buggin - did they want to crash ?
	beq.s	barf			;they want to crash machine

	moveq	#0,d0			;they want to just hang the task
	jsr	_LVOWait(a6)		;wait forever

barf:
	jsr	_LVOAlert(a6)
	rts				;should never get here


* passed string in a2 and execbase in a6
kputs:
	move.b	0(a2),d0
	beq.s	kputsdone
	jsr	_LVORawPutChar(a6)
	adda.l	#1,a2
	bra.s	kputs
kputsdone
	moveq	#10,d0
	jsr	_LVORawPutChar(a6)
	rts

checkseg
	movem.l a0-a3,-(sp)
	lea.l	segaddr(PC),a0
	move.l	d0,(a0)			;store address passed in d0
	move.l	d0,a0			;pass address
	lea.l	hunk(PC),a1		;to hold hunk
	lea.l	offset(PC),a2		;to hold offset
	movea.l	_func_segfind(PC),a3	
	jsr	(a3)			;return name in d0 if found
	movem.l	(sp)+,a0-a3
	rts

_ourAddTaskCode:
	lea.l	_useCountW(PC),a0	;increment wedge useCount
	addq.l	#1,(a0)
	move.l	a1,-(sp)		;stash the passed task pointer
	pea.l	backwedge(pc)	
	move.l	_realAddTaskAddr(pc),-(sp)
	rts				;go to real addtask
backwedge:
	move.l	(sp)+,a0		;retrieve the task pointer
	lea.l	_startcode(pc),a1	;get addr of our trap code
	move.l	a1,TC_TRAPCODE(a0)	; an install in the new task
	lea.l	_useCountW(PC),a0	;decrement wedge useCount
	subq.l	#1,(a0)
	rts				;return to caller

putchproc:
   move.b  d0,(a3)+
   rts

   CNOP 0,4

sbuf1   DS.B 80
sbuf2   DS.B 80
sbuf3   DS.B 80
sbuf4   DS.B 80
sbuf5   DS.B 80
sbuf6   DS.B 80
sbuf7   DS.B 80
nbuf	DS.B 80

tstr  DC.B 'Exception=%lx  Task=%08lx  PC=%08lx  USP=%08lx  SSP=%08lx',0
dstr  DC.B 'd: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx',0
astr  DC.B 'a: %08lx %08lx %08lx %08lx %08lx %08lx %08lx --------',0
sstr  DC.B 's: %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx',0

cnstr DC.B  'Command: %s',0
pnstr DC.B  'Process: %s',0
tnstr DC.B  'Task: %s',0

pcsegstr DC.B  'ST: PC $%lx in "%s" Hunk $%04lx Offset $%08lx',0
stsegstr DC.B  'ST: On stack $%lx in "%s" Hunk $%04lx Offset $%08lx',0
nosegstr DC.B  'ST: No seglist found',0
noststr  DC.B  'ST: Segtracker not running',0



topazname	DC.B	'topaz.font',0

sustr    DC.B 'SUSPEND',0
debstr   DC.B 'DEBUG',0
rebstr   DC.B 'REBOOT',0

IName	DC.B	'intuition.library',0

   CNOP 0,4


hunk			DC.L	0
offset			DC.L	0
segaddr			DC.L	0

_useCountT		DC.L	0	;no removal if UseCount != 0
_useCountW		DC.L	0	;no removal if UseCount != 0

_realAddTaskAddr	DC.L	0	;for pointer to real AddTask

_execbase		DC.L	0	;set up by tnt.c

_func_segfind		DC.L	0	;set up by tnt.c

_Serial			DC.W	0	;for serial output option

			CNOP 0,4
_endcode:
* other data placed here by c portion

   END

