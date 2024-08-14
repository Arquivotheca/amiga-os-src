
*
* bumpbpra.asm
*    Position independent code part of a wedge
*
* AllocBitMap(sizex,sizey,depth,flags,friend_bitmap)(d0/d1/d2/d3/a0)
*

   INCLUDE   "exec/types.i"
   INCLUDE   "exec/execbase.i"
   INCLUDE   "exec/tasks.i"
   INCLUDE   "graphics/gfx.i"

ABSEXECBASE EQU 4

   XDEF   _startFix
   XDEF   _endFix
   XDEF   _useCount
   XDEF   _testcnt
   XDEF	  _rounder

    CODE

* Position Critical on these first items

_startFix:

_oldVector	DC.L   0      	;init'd by .c portion

_newCode:         		;position critical
   	move.l  a1,-(sp)		;save a1
   	lea.l   _useCount(PC),a1       	;increment useCount
   	addq.l  #1,(a1)
	move.l  (sp)+,a1		;restore a1

	btst	#BMB_DISPLAYABLE,d3
	beq.s	toreal

	; round up d0 sizex to a multiple of rounder
	move.l	d4,-(sp)		;save d4
	move.l	_rounder(PC),d4		;power of 2 like 0x00000040
	sub.l	#1,d4			;           like 0x0000003F
	add.l	d4,d0
	eor.l	#$FFFFFFFF,d4		;           like 0xFFFFFFC0
	and.l	d4,d0
	move.l	(sp)+,d4		;restore d4

toreal:
   	move.l  a1,-(sp)		;save a1
	lea.l    _useCount(PC),a1       ;decrement useCount
   	subq.l   #1,(a1)
	move.l  (sp)+,a1		;restore a1

	move.l   _oldVector(PC),-(sp)   ;where real function is
	rts                 		;jsr to real function

		CNOP 0,4
_useCount    	DC.L   0      ;no removal if UseCount != 0
                CNOP 0,4
_testcnt	DC.L   0
_rounder	DC.L   64
		CNOP 0,4      ;end be longword aligned
_endFix:

   END

