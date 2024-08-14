	OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/macros.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/resident.i"

	INCLUDE "versionbase.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	_FindResidentNC
	XDEF	_FindNameNC

	XREF	_LVOStricmp

;---------------------------------------------------------------------------

; a6 has VersionBase, a0 has name pointer
_FindResidentNC:
	    movem.l a2-a4/a6,-(sp)		; save these
	    move.l  vl_SysBase(a6),a1		; get ExecBase
	    move.l  ResModules(a1),a2		; get resident module array
	    move.l  a0,a3			; put name
	    move.l  vl_UtilityBase(a6),a6	; load UtilityBase for Stricmp

*	    ------- fetch next pointer:
ft_next:
	    move.l  (a2)+,d0
	    beq.s   ft_exit
	    bgt.s   ft_noextend
	    bclr    #31,d0		* extended table pointer
	    move.l  d0,a2
	    bra.s   ft_next

*	    ------- compare names
ft_noextend:
	    move.l  d0,a4
	    move.l  d0,a1
	    move.l  a3,a0
	    move.l  RT_NAME(a1),a1
	    CALL    Stricmp
	    tst.l   d0
	    bne.s   ft_next
	    move.l  a4,d0
ft_exit:
	    movem.l (sp)+,a2-a4/a6
	    rts

;---------------------------------------------------------------------------

_FindNameNC
; A0 - list pointer
; A1 - name pointer
; RETURNS:
; D0 - node pointer, or NULL
	movem.l	d2/a2/a3/a6,-(sp)	; save these guys
	move.l	vl_UtilityBase(a6),a6	; load UtilityBase in A6
	move.l	a1,a3			; a3 - name of node
	move.l	LN_SUCC(a0),d2		; d2 - address of first node

1$:	move.l	d2,a2			; load address of next node
	move.l	LN_SUCC(a2),d2		; load pointer to next node
	beq.s	3$			; if at end of list, then exit

	move.l	a3,a0			; name being searched for
	move.l	LN_NAME(a2),a1		; name of current node
	CALL	Stricmp			; compare them
	tst.l	d0			; test the result
	bne.s	1$			; if not equal, then we didn't find it

	move.l	a2,d0			; found it, return the pointer
	movem.l	(sp)+,d2/a2/a3/a6	; restore these
	rts

3$:	moveq.l	#0,d0			; didn't find it
	movem.l	(sp)+,d2/a2/a3/a6	; restore these
	rts

;---------------------------------------------------------------------------

	END
