**
**
**  Decigel020	- The functionality of the famous "decigel" program, but
**  working on the 68020/68030 processors.
**
**  The old Decigel would correctly patch the instruction on the 68020,
**  but chances are the old (bad) instruction was still in the instruction
**  cache.  This code flushes the cache after modifying memory.
**
**  This code may not function under future revisions of the operating
**  system.  This code is safe on the 68000/68010/68020 and 68030.
**  This code is not expected to function on the 68040.  This code may
**  be called from the CLI only.
**
**
**  Written Tuesday 03-Apr-90 21:21:47 -Bryce Nesbitt
**
**
		INCLUDE "exec/types.i"
		INCLUDE "exec/memory.i"
		INCLUDE "exec/ables.i"
		INCLUDE "exec/execbase.i"
		INCLUDE "libraries/dosextens.i"

		INT_ABLES

		XREF	_LVOFindTask
		XREF	_LVOSupervisor

ABSEXECBASE	EQU 4
PrivTrapVector	EQU $20




;-------------- install patch then detach -----------------------------------

		move.l	ABSEXECBASE,a6


		;
		;   Contents of the old vector are used to self-modify our
		;   code.  The new vector replaces the old.
		;
		DISABLE
		move.l	PrivTrapVector,ModifyCode+2
		bsr.s	FlushCache
		move.l	#NewPrivTrap,PrivTrapVector
		ENABLE


		;
		;   Detach our code from the CLI
		;
		suba.l	a1,a1
		jsr	_LVOFindTask(a6)
		move.l	d0,a0
		move.l	pr_CLI(a0),a0
		add.l	a0,a0
		add.l	a0,a0
		move.l	a0,d0
		beq.s	not_cli
		clr.l	cli_Module(a0)
not_cli:	moveq	#0,d0
		rts




*
*   Flush the instruction cache
*
FlushCache:	movem.l a5/a6,-(sp)
		move.l	ABSEXECBASE,a6
		btst.b	#AFB_68020,AttnFlags+1(a6)  ;>=68020 includes cache
		beq.s	fc_nocache

		lea.l	FlushTrap(pc),a5
		jsr	_LVOSupervisor(a6)

fc_nocache:	movem.l (sp)+,a5/a6
		rts
;
;
FlushTrap:	dc.w	$4e7a,$0002 ;movec.l CACR,d0
		bset	#3,d0	    ;Set "Clear instruction cache" bit
		dc.w	$4e7b,$0002 ;movec.l d0,CACR
		rte





*****************************************************************************
**									   **
**									   **
**  The trap handler wedged into the privilege violation vector.	   **
**									   **
**  If the instruction was MOVE SR,<ea> it is converted to MOVE CCR,<ea>.  **
**  The instruction cache is flushed, then the instruction is re-executed. **
**									   **
**									   **
*****************************************************************************

STKOFFSET	EQU	4*3

;
;   New privilege violation vector
;
NewPrivTrap:	movem.l d0/a0/a6,-(sp)
		move.l	STKOFFSET+2(sp),a0
		move.w	(a0),d0             ; Examine opcode
		andi.w	#~%111111,d0	    ; Mask out EA field
		cmpi.w	#$40C0,d0	    ; A MOVE SR,<ea>?
		beq.s	GotOne
		movem.l (sp)+,d0/a0/a6
ModifyCode:	jmp	$01234567	    ; To previous handler... (exit)

;
;   Code executed if the instruction was MOVE SR,<ea>
;
GotOne: 	move.l	ABSEXECBASE,a6


		DISABLE
		bset	#1,(a0)             ; Convert to MOVE CCR,<ea>
		btst.b	#AFB_68020,AttnFlags+1(a6)  ;>=68020 includes cache
		beq.s	no_cache

		dc.w	$4e7b,$8802 ; movec.l a0,CAAR
		dc.w	$4e7a,$0002 ; movec.l CACR,d0
		bset	#2,d0	    ; Set "Clear entry in instruction cache"
		dc.w	$4e7b,$0002 ; movec.l d0,CACR
no_cache:	ENABLE


		movem.l (sp)+,d0/a0/a6
		rte			    ; Rerun new opcode... (exit)


		END
