
*   slimea.asm  --- assembler interface for slime.c
*                 Carolyn Scheppner --- CBM  04/91

   INCLUDE   'exec/types.i'
   INCLUDE   'exec/execbase.i'
   INCLUDE   'exec/tasks.i'
   INCLUDE   'hardware/custom.i'

todlo   equ     $BFE801
todmid  equ     $BFE901
todhi   equ     $BFEA01

	SECTION CODE

   XREF   _AbsExecBase

   XREF	  _stricmp

   XREF   _MySwitch
   XREF   _MyLaunch

   XDEF   _myAddTask
   XDEF   _mySwitch
   XDEF   _myLaunch
   XDEF   _vbget

   XDEF   _RealAddTask
   XDEF	  _vbcnt
   XDEF   _usecnt


_myLaunch:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)            ;save registers
   bsr	   _vbget

   move.l  ThisTask(A6),-(sp)           ;push ptr to ThisTask
   jsr     _MyLaunch
   addq.l  #4,a7
   
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   sub.l   #1,_usecnt
   rts


_mySwitch:
   add.l   #1,_usecnt
   movem.l d0-d1/a0-a1,-(sp)            ;save registers
   bsr     _vbget

   move.l  ThisTask(A6),-(sp)           ;push ptr to ThisTask
   jsr     _MySwitch
   addq.l  #4,a7
   
   movem.l (sp)+,d0-d1/a0-a1   		;restore registers
   sub.l   #1,_usecnt
   rts


_myAddTask:
   	add.l   #1,_usecnt
	move.l	a1,-(sp)		;stash the passed task pointer
	pea.l	backwedge(pc)	
	move.l	_RealAddTask(pc),-(sp)
	rts				;go to real addtask
backwedge:
	move.l	(sp)+,a0		;retrieve the task pointer
	btst.b	#TB_LAUNCH,TC_FLAGS(a0) ;already has its own Launch code ?
	bne.s	hasL			; yes
	lea.l	_myLaunch(pc),a1	;get addr of our Launch code
	move.l	a1,TC_LAUNCH(a0)	; and install in the new task	
	bset.b	#TB_LAUNCH,TC_FLAGS(a0) ; and flag it
hasL:
	btst.b	#TB_SWITCH,TC_FLAGS(a0) ;already has its own Switch code ?
	bne.s	hasS			; yes
	lea.l	_mySwitch(pc),a1	;get addr of our Switch code
	move.l	a1,TC_SWITCH(a0)	; and install in the new task	
	bset.b	#TB_SWITCH,TC_FLAGS(a0) ; and flag it
hasS:
   	sub.l   #1,_usecnt
	rts				;return to caller


* get current 50/60 Hz event counter
_vbget:
	move.l  d0,-(sp)
        moveq   #0,d0
        move.b  todhi,d0
        lsl     #8,d0
        move.b  todmid,d0
        lsl     #8,d0
        move.b  todlo,d0
        move.l  d0,_vbcnt(PC)
	move.l	(sp)+,d0
	rts


_vbcnt		DC.L	0

_RealAddTask:	DC.L	0

_usecnt		DC.L	0



  

	END

