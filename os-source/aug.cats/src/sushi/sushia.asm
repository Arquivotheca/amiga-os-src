
*
* sushia.asm --- wedge template
*    Position independent code part of wedge
*
* NOTE - this code may be run in a copy of code in allocated ram.
* Don't use (PC) destinations - assembler may generate code which
*   will reference absolute addresses in the original loadsegged code
*

   INCLUDE   "exec/types.i"
   INCLUDE   "exec/execbase.i"
   INCLUDE   "exec/alerts.i"

MYDEBUG	SET 0

	section code


* The debugging macro DBUG
* Only generates code if MYDEBUG is > 0
*

	IFGT	MYDEBUG

* note - current 2.0 debug.lib has this entry
	XREF	KPrintF

* DBUG macro for format string and two variables
*	 preserves all registers   
*        outputs to serial port   link with amiga.lib,debug.lib
* Usage: pass name of format string,value,value
*        values may be register name, variablename(PC), or #value
*        

DBUG	MACRO	* passed name of format string, with args on stack
	MOVEM.L	d0-d7/a0-a6,-(sp)
	MOVE.L  \3,-(sp)
	MOVE.L  \2,-(sp)
	LEA.L	\1(PC),a0
	MOVEA.L sp,a1
	JSR	KPrintF
	ADDA.L	#8,sp
	MOVEM.L	(sp)+,d0-d7/a0-a6	
	ENDM
	ENDC

	IFEQ	MYDEBUG
DBUG	MACRO
* disabled debug macro
	ENDM
	ENDC

ABSEXECBASE EQU 4

	XDEF	_startpatch
	XDEF	_endpatch

	XDEF	_grab_em_offs
	XDEF	_free_em_offs
	XDEF	_usecount_offs
	XDEF	_appstruct_offs

	XREF	_LVODisable
	XREF	_LVOEnable
	XREF	_LVOSetFunction
	XREF	_LVOSignal
	XREF	_LVOFindPort
	XREF	_LVOAlert
	XREF	_LVOCacheClearU

	XREF	_LVOAllocMem
	XREF	_LVOFreeMem

_LVORawIOInit		EQU	-504
_LVORawMayGetChar	EQU	-510
_LVORawPutChar		EQU	-516


* Note - oldvec must be first in structure
  STRUCTURE WedgeTab,0
    LONG  oldvec
    LONG  lvo
    LONG  ouroff
    LONG  ourvec
    LONG  retvec
    LABEL WedgeTab_SIZEOF 


_startpatch:

* this will get more complex if >1 library
* returns non-zero = success, zero = failure 
_grab_em
	movem.l	d1-d7/a0-a6,-(sp)
	DBUG	grabemstr,#0,#0
	bsr	grab_em_exec
*	tst.l	d0
*	beq.s	nograb
	bsr	grab_enforcer
nograb
	movem.l	(sp)+,d1-d7/a0-a6
	rts

* this will get much more complex if >1 library
* returns non-zero = success, zero = failure 
_free_em
	movem.l	d1-d7/a0-a6,-(sp)
	DBUG	freeemstr,#0,#0
	bsr	free_em_exec
	tst.l	d0
	beq.s	nofree
	bsr	free_enforcer
nofree
	movem.l	(sp)+,d1-d7/a0-a6
	rts




* returns non-zero = success, zero = failure
grab_em_exec:

	move.l	$4,a6			; this library open can't fail
	move.l	#1,d0			; so we just say result is success
	move.l	d0,-(sp)		; stash the success (0 or 1) for later
					; if was not 1, bra to grab_em_exec_x
	jsr	_LVODisable(a6)

	; set a2 = relocated base address of this patch code
	;     a3 = wedge table
	;     a4 = base of library to patch (exec)
	;     a6 = execbase

	lea.l	_startpatch(PC),a2
	lea.l	_execwedges(PC),a3
	move.l	$4,a4

	DBUG	grabexstr,#0,#0
	bsr	dograb_em			; do the setfunctions

	jsr	_LVOEnable(a6)

	; would close library here if opened, success is already on stack

grab_em_exec_x
	move.l	(sp)+,d0			; success or failure

	rts


dograb_em:
1$
	move.l	ouroff(a3),d0		; get a new function offset
	beq.s	2$			; if zero, we're done

	add.l	a2,d0			; else add base address of this code
	move.l	d0,ourvec(a3)		; stash new vector for compare on free
	move.l	lvo(a3),a0		; lvo to a0
	move.l	a4,a1			; libbase to a1
	DBUG	gsetfstr,#0,#0

	jsr	_LVOSetFunction(a6)	; patch it
	move.l	d0,oldvec(a3)		; save old vector for restoration

	adda.l	#WedgeTab_SIZEOF,a3
	bra.s	1$
2$
	rts




* returns non-zero = success, zero = failure 
free_em_exec:

	move.l	$4,a6

	jsr	_LVODisable(a6)

	; set a2 = relocated base address of this patch code
	;     a3 = wedge table
	;     a4 = base of library to unpatch (exec)
	;     a6 = execbase

	lea.l	_startpatch(PC),a2
	lea.l	_execwedges(PC),a3
	move.l	$4,a4

	DBUG	freeexstr,#0,#0
	bsr	dofree_em			; do the unsetfunctions
	move.l	d0,-(sp)			; stash returncode

	jsr	_LVOEnable(a6)
	move.l	(sp)+,d0			; return code

	rts



dofree_em:
	; first put old vecs back (and receive returned previous vecs)
	move.l	a3,d3			; save ptr to wedge tab for later loops
1$
	move.l	oldvec(a3),d0		; get an old function vector
	beq.s	2$			; if zero, we're done

	move.l	lvo(a3),a0		; lvo to patch in a0
	movea.l	a4,a1			; lib base in a1
	DBUG	fsetfstr,#0,#0
	jsr	_LVOSetFunction(a6)
	move.l	d0,retvec(a3)		; stash the returned vector
	adda.l	#WedgeTab_SIZEOF,a3
	bra.s	1$
2$
	; now compare the returned vecs to ourvecs
	move.l	d3,a3			; re-init pointer to wedge table
3$
	move.l	retvec(a3),d0		; get a returned vector
	beq.s	free_em_ok		; when we hit zero, we're done
	cmp.l	ourvec(a3),d0		; compare to the vec we had installed
	bne.s	free_em_no		; if not same, we can't pull out
	adda.l	#WedgeTab_SIZEOF,a3
	bra.s	3$

free_em_no
	move.l	d3,a3			; re-init pointer to wedge table
4$
	move.l	retvec(a3),d0		; the returned vector
	beq.s	5$			; if zero, we're done
	move.l	lvo(a3),a0		; the lvo
	movea.l	a4,a1			; the libbase
	DBUG	fsetfstr,#0,#0
	jsr 	_LVOSetFunction(a6)	; put it back
	adda.l	#WedgeTab_SIZEOF,a3
	bra.s	4$
5$
	moveq	#0,d0			; return failure
	bra.s	free_em_x		; all done

free_em_ok
	moveq	#1,d0			; return success
free_em_x
	rts


grab_enforcer
	movem.l	d0-d1/a0-a1/a6,-(sp)
	movea.l	$4,a6
	jsr	_LVODisable(a6)

*	lea.l	temp1(PC),a1
*	move.l	#$333,(a1)		; DEBUGGING

	lea.l	EnforcerName(PC),a1
	jsr	_LVOFindPort(a6)
	tst.l	d0			; if not 0, we found Enforcer's port
	beq.s	gnopatch

*	lea.l	temp1(PC),a1
*	move.l	d0,(a1)			; DEBUGGING

	movea.l	d0,a0
	movea.l	LN_NAME(a0),a0		; now have pointer to name

*	lea.l	temp2(PC),a1
*	move.l	a0,(a1)			; DEBUGGING

	adda.l	#$147E,a0		; at code place to patch

*	lea.l	temp3(PC),a1
*	move.l	a0,(a1)			; DEBUGGING

	cmpi.l	#$48E7C030,0(a0)	; make sure it's the right Enforcer
	bne.s	gmega			; nope - check mega offest
	cmpi.l	#$323900DF,4(a0)	; make double sure
	bne.s	gmega			; nope - check mega offset
	beq.s	gpatch			; yep - Enforcer 2.8b

gmega
	adda.l	#$238,a0		; extra offset for megastack 2.6f
	cmpi.l	#$48E7C030,0(a0)	; make sure it's the right Enforcer
	bne.s	gnopatch		; nope
	cmpi.l	#$323900DF,4(a0)	; make double sure
	bne.s	gnopatch		; nope

gpatch					; a0 contains where to patch
	lea.l	EnforcerPatch(PC),a1	; address of our enforcer patch
	move.w	#$4EF9,0(a0)		; JMP instruction
	move.l	a1,2(a0)		; to our patch

        cmpi.w  #37,LIB_VERSION(a6)	; flush cache
        blt.s   gnoflush
	jsr	_LVOCacheClearU(a6)
gnoflush

	jsr	_LVOEnable(a6)
	movem.l	(sp)+,d0-d1/a0-a1/a6
	ori.l	#$10,d0			; show this patch worked too
	rts

gnopatch	
	jsr	_LVOEnable(a6)
	movem.l	(sp)+,d0-d1/a0-a1/a6
	rts



free_enforcer
	movem.l	d0-d1/a0-a1/a6,-(sp)
	movea.l	$4,a6
	jsr	_LVODisable(a6)

	lea.l	EnforcerName(PC),a1
	jsr	_LVOFindPort(a6)
	tst.l	d0			; if not 0, Enforcer's port
	beq.s	fnopatch

	movea.l	d0,a0
	movea.l	LN_NAME(a0),a0		; now have pointer to name
	adda.l	#$147E,a0		; at code place to patch

	lea.l	EnforcerPatch(PC),a1	; address of our enforcer patch

	cmpi.w	#$4EF9,0(a0)		; make sure it's the patched Enforcer
	bne.s	fmega			; nope - try mega offset
	cmp.l	2(a0),a1		; make double sure
	bne.s	fmega			; nope - try mega offset
	bra.s	fpatch			; yep
fmega
	adda.l	#$238,a0		; extra offset for megastack 2.6f
	cmpi.w	#$4EF9,0(a0)		; make sure it's the patched Enforcer
	bne.s	fnopatch		; nope
	cmp.l	2(a0),a1		; make double sure
	bne.s	fnopatch		; nope

fpatch
	move.l	#$48E7C030,0(a0)	; replace old code
	move.l	#$323900DF,4(a0)	; replace old code

        cmpi.w  #37,LIB_VERSION(a6)	; flush cache
        blt.s   fnoflush
	jsr	_LVOCacheClearU(a6)
fnoflush

fnopatch	
	jsr	_LVOEnable(a6)
	movem.l	(sp)+,d0-d1/a0-a1/a6
	rts


ourRawIOInit:				;no usecnt - right out'a here
	rts

ourRawMayGetChar:			;no usecnt - right out'a here
	move.l	#'y',d0
	rts


* gets character in D0, ExecBase in A6 (from kprintf call to RawPutChar)
ourRawPutChar:
	pea.l	_usecount(PC)		; increment usecount
	addq.l	#1,(sp)
	addq.l	#4,sp

	movem.l d0-d1/a0-a3,-(SP)

	bsr.s	PSCODE			; store char in buffer

	cmpi.b	#13,d0			; if EOL, signal task
	beq.s	dosig	
	cmpi.b	#10,d0
	beq.s	dosig
	bra.s	skipsig
dosig
	movea.l	sutask(PC),a1
	move.l	susignal(PC),d0
	jsr	_LVOSignal(a6)
skipsig
	movem.l (sp)+,d0-d1/a0-a3
	pea.l	_usecount(PC)		; decrement usecount
	subq.l	#1,(sp)
	addq.l	#4,sp	
	rts				; return


DEMOEXAMPLES	EQU	0
	IFNE	DEMOEXAMPLES
* example function with back-wedge also
ourFunction1:
	pea.l	_usecount(PC)		; increment usecount
	addq.l	#1,(sp)
	addq.l	#4,sp

	DBUG	prestr,d0,d1

	pea.l	ourFunction1Return(PC)
	move.l	oldFunction1(PC),-(sp)
	rts

ourFunction1Return
	DBUG	poststr,d0,#0
	pea.l	_usecount(PC)		; decrement usecount
	subq.l	#1,(sp)
	addq.l	#4,sp	
	rts				; return to caller

* example with no back wedge... just continues on to old vector

ourFunction2:
	pea.l	_usecount(PC)		; increment usecount
	addq.l	#1,(sp)
	addq.l	#4,sp

	DBUG	prestr,a1,d0

	move.l	oldFunction2(PC),-(sp)

	pea.l	_usecount(PC)		; decrement usecount
	subq.l	#1,(sp)
	addq.l	#4,sp	
	rts				; address of real func on stack
	ENDC



;-----------------------Bryce's debugging stuff------------------------------
;------------------special for exception handler (no location 4)-------------
	XREF	_LVORawDoFmt
	;A0-string
	;A1-args
	;A6-SPECIAL EXEC
	;All other registers are preserved

* patch for Enforcer 2.8b
EnforcerPatch:
		movem.l D0/D1/A2/A3,-(SP)
		;
		;   Output string
		;
		lea.l	PSCODE(pc),a2
		suba.l	a3,a3
		jsr	_LVORawDoFmt(a6)

string_ignore:
		movem.l (SP)+,D0/D1/A2/A3
		rts


		;
		;   Per character function for RawDoFmt, RawPutChar
		;
PSCODE: 	tst.b	d0
		beq.s	ignore

		movea.l	subuf(PC),a3		; start of subuf buffer
		move.l  subufi(PC),d1		; buffer index
		adda.l  d1,a3			; where this char goes	
		move.b	d0,0(a3)		; store byte
		addi.l	#1,d1			; add 1 to index
		and.l	wrapmask(PC),d1		; wrap index for circular buffer

		movea.l	subuf(PC),a3		; start of subuf buffer
		adda.l	d1,a3			; where next char goes
		move.b	#'@',(a3)		; store a @ next char

		lea.l	subufi(PC),a3		; update index
		move.l	d1,(a3)

ignore: 	rts

;----------------------------------------------------------------------------

		CNOP 0,4

_appstruct
subuf		DC.L	0
subufsize	DC.L	0
subufi		DC.L	0
subufli		DC.L	0
wrapcnt		DC.L	0
wrapmask	DC.L	0
extsigtask	DC.L	0
extsignum	DC.L	0
extsignal	DC.L	0
sumicros	DC.L	0
sutask		DC.L	0
susignum	DC.L	0
susignal	DC.L	0
temp1		DC.L	4
temp2		DC.L	5
temp3		DC.L	6


_usecount    	DC.L   0      ;no removal if UseCount != 0

_usecount_offs	DC.L	_usecount-_startpatch
_grab_em_offs	DC.L	_grab_em-_startpatch
_free_em_offs	DC.L	_free_em-_startpatch

_appstruct_offs	DC.L	_appstruct-_startpatch
		
* equates defined above
* oldexec  LVO $fe08 -504  RawIOInit()
* oldexec  LVO $fe02 -510  RawMayGetChar()
* oldexec  LVO $fdfc -516  RawPutChar()

_execwedges:
oldRawIOInit	 DC.L	0,_LVORawIOInit,ourRawIOInit-_startpatch,0,0
oldRawMayGetChar DC.L	0,_LVORawMayGetChar,ourRawMayGetChar-_startpatch,0,0
oldRawPutChar	 DC.L	0,_LVORawPutChar,ourRawPutChar-_startpatch,0,0
		 DC.L	0,0,0,0,0

EnforcerName	 DC.B	'_The Enforcer_',0

	IFGT	MYDEBUG
gsetfstr	DC.B	'grab - set one function',10,0
fsetfstr	DC.B	'free - set one function',10,0
grabemstr	DC.B	'In grab_em',10,0
freeemstr	DC.B	'In free_em',10,0
grabexstr	DC.B	'In grab_exec',10,0
freeexstr	DC.B	'In free_exec',10,0
prestr		DC.B	'pre:  val1=$%lx val2=$%lx',10,0
poststr		DC.B	'post: val1=$%lx val2=$%lx',10,0
	ENDC

_endpatch:

   END

