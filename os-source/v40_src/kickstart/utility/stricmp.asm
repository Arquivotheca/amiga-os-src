        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"

	LIST

;---------------------------------------------------------------------------

	XDEF	Stricmp
	XDEF	Strnicmp
	XDEF	ToUpper
	XDEF	ToLower

;---------------------------------------------------------------------------

Stricmp:
	move.l	#$7fffffff,d0		; infinite length
	;-- fall through!


;---------------------------------------------------------------------------

	;-- note: stricmp falls through to here
Strnicmp:
	movem.l	d2/d3/d4,-(a7)	; d4 for check_sub use
	move.l	d0,d3		; save length
	beq.s	exit		; if length == 0, return SAME
	moveq	#0,d0		; clear high 3 bytes, set equal if len=0

caseloop:
	subq.l	#1,d3		; done yet?
	bmi.s	make_res	; if done, make result
	move.b	(a0)+,d1	; Deal with either string ending first
	move.b	(a1)+,d2	; must fetch both for comparison, even if d0==0
	beq.s	make_res	; exit if *s2 == 0
	move.b	d1,d0
	beq.s	make_res	; exit if *s1 == 0

	bsr.s	check_sub_toupper
	beq.s	1$
	sub.b	#'a'-'A',d1
1$:
	move.b	d2,d0
	bsr.s	check_sub_toupper
	beq.s	2$
	sub.b	#'a'-'A',d2

2$:
	cmp.b	d2,d1		; do the comparison (must
	beq.s	caseloop	; check next character (after checking size)

make_res:
	move.b	d1,d0
	sub.b	d2,d0		; generate return value
exit:
	ext.w	d0
	ext.l	d0		; return LONG!!!
	movem.l	(a7)+,d2/d3/d4
	rts

*
* returns d0 = 0 if no sub needed, = 1 if needed (cc's set!!!!)
* can't touch d1/d2/d3, d4 is scratch, d0 has character/return
*
check_sub_tolower:
	move.l	#'^@ZA',d4	; load up 4 characters for compares
	bra.s	check_common	; @ <- à, ^ <- þ
check_sub_toupper:
	move.l	#'~`za',d4	; load up 4 characters for compares
check_common:
	cmp.b	#$f7,d0		; these are special
	beq.s	no
				; Z/z and A/a in low 16 bits
	bclr.b	#7,d0		; clear high bit
	beq.s	normal_char
	swap	d4		; Þ/þ and À/à are the highest in the upper area
				; (~ = þ, ' = à minus bit 7)
normal_char:
	cmp.b	d4,d0		; a or à(`)  (or A or À)
	blt.s	no
	lsr.l	#8,d4		; get upper bound
	cmp.b	d4,d0		; z or þ(~)  (or Z or Þ)
	bgt.s	no
	moveq	#1,d0
	rts
no:	moveq	#0,d0
	rts

;---------------------------------------------------------------------------

ToUpper:
	movem.l	d2-d4,-(sp)
	move.b	#'A'-'a',d3	; amount to add (negative in this case)
	lea	check_sub_toupper(pc),a0

;---------------------------------------------------------------------------

	;-- common routine
toxxx:
	move.l	d0,d2		; save character
	jsr	(a0)		; check_sub_xxxx
	beq.s	1$		; returns cc's, hits d4
	add.b	d3,d2		; + or - ('a'-'A')
1$:
	moveq	#0,d0
	move.b	d2,d0		; make sure the return is good
	movem.l	(sp)+,d2-d4
	rts

;---------------------------------------------------------------------------

ToLower:
	movem.l	d2-d4,-(sp)
	lea	check_sub_tolower(pc),a0
	move.b	#'a'-'A',d3			; amount to add
	bra.s	toxxx
