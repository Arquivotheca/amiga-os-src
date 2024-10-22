        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_UnlinkRomTag

;---------------------------------------------------------------------------


   STRUCTURE KickTag,0
	APTR kt_TagAddr
	APTR kt_NextArray
   LABEL KickTag_SIZEOF


;---------------------------------------------------------------------------


; This function removes references to the rom tag passed in A0 from
; Exec's KickTag list and ResModules list.

_UnlinkRomTag:
; a0 - Rom tag pointer

	movem.l	a2/a6,-(sp)

	move.l	4,a6		  ; load exec base pointer

	move.l	KickTagPtr(a6),d0 ; d0 now points to array of rom tag pointers
	beq.s	doResModules	  ; if the pointer is NULL, leave
	move.l	d0,a1		  ; a1 now points to array of rom tag pointers

	move.l	(a1)+,d0	  ; d0 has first ROM Tag
	cmp.l	d0,a0		  ; is this us?
	bne.s	scanKickArray	  ; not first one, so scan array

        move.l	(a1),d0		  ; d0 has pointer to next array
        bclr	#31,d0		  ; clear upper bit
        move.l	d0,KickTagPtr(a6) ; store pointer to next array in ExecBase
        bra.s	doResModules

scanKickArray:
	tst.l	d0		  ; set condition codes
	bmi.s	skaDoLink	  ; was this first rom tag a link pointer?

skaLoop:
	move.l	(a1)+,d0	  ; d0 has next rom tag
	beq.s	doResModules	  ; end of list, didn't find it...
	bge.s	skaCmp		  ; if bit 31 is set, then d0 holds pointer to next array of pointers

skaDoLink:
	bclr	#31,d0		  ; clear magical high bit
	move.l	a1,a2		  ; save address of "next array" pointer
	move.l	d0,a1		  ; a1 has address of next array
	bra.s	skaLoop           ; so try again

skaCmp:
        cmp.l	d0,a0		  ; is this our lucky winner?
 	bne.s	skaLoop    	  ; nope, so try again

	move.l	(a1),-(a2)	  ; our next into the previous' next pointer



doResModules:
	move.l	ResModules(a6),d0 ; d0 now points to array of rom tag pointers
	beq.s	exit		  ; if the pointer is NULL, leave
	move.l	d0,a1		  ; a1 now points to array of rom tag pointers

1$:	move.l	(a1)+,d0	  ; d0 has next rom tag
	beq.s	exit        	  ; end of list, didn't find it...
	bge.s	2$		  ; if bit 31 is set, then d0 holds pointer to next array of pointers
	bclr	#31,d0		  ; clear magical high bit
	move.l	d0,a1		  ; a1 has address of next array
	bra.s	1$		  ; so try again

2$:     cmp.l	d0,a0		  ; is this our lucky winner?
 	bne.s	1$		  ; nope, so try again

3$:	move.l	a1,d0		  ; now make our rom tag pointer a link
	bset	#31,d0		  ; pointer instead, and point it to the
	move.l	d0,-(a1)	  ; next slot in the array

exit: 	movem.l	(sp)+,a2/a6
	rts

;---------------------------------------------------------------------------

	END
