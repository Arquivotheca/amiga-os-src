
	include "utility/hooks.i"
	include "dos/exall.i"

    xdef        @clpcpy
    xdef	@do_match

*
* UBYTE *clpcpy:
*
* Copy a string delimited by ¦ (s-a-I) characters.
*
* clpcpy(dest,src,maxlen);
*
* dest = ptr to destination (will be null terminated)
* src = ptr to src
*
* returns a ptr to character after last delimiting ¦ read, or null if length wasn't big enough.
*

*
@clpcpy:
;        move.l      4(sp),a0            * destination
;        move.l      8(sp),a1            * source
;        move.l     12(sp),d0            * length
	tst.l	    d0
        beq         8$                  * null length -> exit
        clr.b       (a0)                * null terminate the destination, even if no data gets copied
        subq.l      #1,d0               * If asked for null or 1 char destination, exit.
        ble         8$
1$      move.b      (a1)+,d1
        beq         9$
        cmp.b       #'¦',d1
        beq         9$
        move.b      d1,(a0)+
        clr.b       (a0)
        bra         1$
8$      moveq.l     #0,d0
        rts
9$      move.l      a1,d0
        rts


*
* Assembler routine for exall to call hook function
*
@do_match:
	movem.l	d0/a2/a3,-(a7)	; tricky!
	move.l	a7,a2		; points to data (from d0)
	move.l	eac_MatchFunc(a0),a0	; hook structure
	move.l	h_Entry(a0),a3
	jsr	(a3)		; a0=ptr to hook, a1=pointer to param,
				; a2=ptr to object
	movem.l	(a7)+,d1/a2/a3	; don't hit d0!
	rts
*

	END
