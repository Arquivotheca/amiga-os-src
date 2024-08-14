; hooks.asm -- intervene (layers) library vectors
;	:ts=8


; this program copyright 1987, james mackraz.  may not be distributed
; for profit.  copies of the source may be made for not-for-profit
; distribution, but must include this notice.
;
; james mackraz, 4021 Second Street, Palo Alto, CA, 94306

	include 'exec/types.i'

 STRUCTURE	HK,0
 	LONG	HK_SYSFUNC
	LONG	HK_MYFUNC
	LONG	HK_ENTRY
	WORD	HK_TEST
	LONG	HK_LVO
	LABEL	HK_SIZE

	MACRO	UNIQUEHOOK
	public	_entry%1
_entry%1:
	movem.l	regboys,-(sp)
	jsr	_geta4
	; address of hook struct serves as unique id
	lea	_myhooks+(%1*HK_SIZE),a3
	jsr	commonhook
	movem.l	(sp)+,regboys
	rts
	ENDM

regboys	reg	d1/d2/d3/d5/a0/a1/a2/a3/a4/a5/a6

	; uses registers that specific system functions
	; do not use, and will preserve.
	;	d3 --	aztec may trash
	;	d5 --	used in test
	;	a3 --	hook table entry
	;	a4 --	aztec context
	;	a5 --	called through
	;	a6 --	aztec may trash

	dseg
	public	_myhooks

	cseg
	public	_geta4

	;------	unique entry points for each layer vector stolen
	UNIQUEHOOK 0
	UNIQUEHOOK 1
	UNIQUEHOOK 2
	UNIQUEHOOK 3
	UNIQUEHOOK 4
	UNIQUEHOOK 5
	UNIQUEHOOK 6
	UNIQUEHOOK 7

commonhook:

	; ***	test condition (two versions)
	move.l	HK_SYSFUNC(a3),a5

	jsr	(a5)

	; ***	call conditionally, based on test
	;------	save/pass myfunc the sysfunc return value
	move.l	d0,-(sp)		
	move.l	HK_MYFUNC(a3),a5
	jsr	(a5)

	move.l	(sp)+,d0
	rts

	end
