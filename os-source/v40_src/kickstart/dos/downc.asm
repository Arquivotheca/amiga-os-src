
	IFND KLUDGE
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "dos/dosextens.i"

	INCLUDE	"libhdr.i"
	ENDC

*
* External defs
*
	XDEF	@CtoB
	XDEF	_CtoB
	XDEF	@BtoCstr
	XDEF	_BtoCstr
	XDEF	@clearvec
	XDEF	_clearvec
	XDEF	BCPLCtoB
	XDEF	BCPLBtoC
	XDEF	CLEARVEC
	XDEF	COPYSTRING
	XDEF	COPYVEC
	XDEF	UNPACKSTRING
	XDEF	PACKSTRING

	IFND KLUDGE
FUNCDEF     MACRO   * function
_LVO\1	    EQU	    FUNC_CNT
FUNC_CNT    SET     FUNC_CNT-6
            ENDM

FUNC_CNT    SET     LIB_NONSTD

	INCLUDE	"exec/exec_lib.i"
	INCLUDE	"dos/dos_lib.i"
	ENDC

*LET CtoB(v1,v2) = valof
*$(  let l = 0
*    while l<254 do
*    $( if 0%(v1+l)=0 break
*       l := l+1
*    $)
*    for i=l to 1 by -1 DO
*       v2%i := 0%(v1+i-1)
*    v2%0 := l
*    resultis v2
*$)
*
* first param is CPTR, second is BPTR
* UGLY routine to avoid copying more than 255 chars
*
BCPLCtoB
	movem.l	a0/a1,-(sp)
	asl.l	#2,d2
	jsr	@CtoB
	movem.l (sp)+,a0/a1
	jmp	(a6)

@CtoB
_CtoB
	move.l	d2,-(sp)
	moveq	#0,d0		Zero counter (number of chars in string)
	movea.l	d1,a0		Copy of source
1$	tst.b	(a0)+		See if null at end
	beq.s	2$		J if so	
	addq.b	#1,d0		Bump counter
	bne.s	1$		J unless overflow

2$	subq.l	#1,a0		Avoid final null (a0 now ptr to null byte)
	move.l	d2,d1		return second arg (as BPTR)
	lsr.l	#2,d1		To bptr
	movea.l	d2,a1		destination ptr
	move.w	d0,d2		save length
	lea.l   1(a1,d0.w),a1	adjust to end
	bra.s	4$
3$	move.b	-(a0),-(a1)	copy bytes
4$	dbra	d0,3$
	move.b	d2,-(a1)	insert length
	move.l	d1,d0		return value for c also (bptr)
	move.l	(sp)+,d2
	rts


*// Convert BCPL string v (Bptr) to C string in place.
*LET BtoC(v) BE
*$( let len=v%0
*   for i=1 to len do v%(i-1) := v%i
*   v%len := 0
*$)

BCPLBtoC
	asl.l	#2,d1
	jsr	@BtoCstr
	sub.l	a0,a0		fix Z reg
	jmp	(a6)
	
@BtoCstr:
_BtoCstr:
	movea.l	d1,a0		Into addr reg
	moveq	#0,d0		Clear counter
	move.b	(a0)+,d0	Get length
	bra.s	2$
1$	move.b	(a0)+,-2(a0)	Replace in place	EVIL!!!!!!!
2$	dbra	d0,1$
	clr.b	-1(a0)		Insert zero byte
	rts

*LET clearvec( v, upb ) BE
*   FOR i=0 to upb DO v!i := 0
* This version limited to 32K
*
* clears 1 too many LWs!!!!! Has always done so!!!!! REJ
*
CLEARVEC:
	move.l	a6,-(sp)
@clearvec:
_clearvec:
	move.l	d2,d0		We sometimes get called with...
	bmi.s	3$		...an upb of -1 !!
	lsl.l	#2,d1		Convert to mc ptr
	exg	d1,a3		Into addr reg (save old value)
1$	clr.l	(a3)+		Clear area
	dbra	d0,1$		Loop until done
	exg	d1,a3		restore a3
3$	rts

*LET copystring(source, dest) BE
*   FOR i=0 to source%0 DO dest%i := source%i

COPYSTRING:
	asl.l	#2,d1		source to mc ptr
	movea.l	d1,a3
	asl.l	#2,d2		dest to mc ptr
	movea.l	d2,a4
	moveq	#0,d0		zero counter
	move.b	(a3),d0		get length
1$	move.b	(a3)+,(a4)+	copy bytes (we copy length+1 to include length)
	dbra	d0,1$
	jmp	(a6)

*LET copyvec(source,dest,upb) BE
*   FOR i=0 to upb DO dest!i := source!i
* unchanged - REJ
COPYVEC:
	tst.l	d3		We sometimes get called with...
	bmi.s	2$		...an upb of -1 !!
	asl.l	#2,d1		Convert to mc ptr
	movea.l	d1,a3		Into addr reg
	asl.l	#2,d2		Convert to mc ptr
	movea.l	d2,a4		Into addr reg
1$	move.l	(a3)+,(a4)+	Copy vector
	dbra	d3,1$
2$	jmp	(a6)

*AND unpackstring(s,v) BE 
*   FOR i = s%0 TO 0 BY -1 DO v!i := s%i
UNPACKSTRING:
	asl.l	#2,d1		source to mc ptr
	movea.l	d1,a3
	asl.l	#2,d2		dest to mc ptr
	movea.l	d2,a4
	moveq	#0,d0		zero counter
	moveq	#0,d1		zero transfer reg
	move.b	(a3),d0		get length
1$	move.b	(a3)+,d1	copy byte out to tfr reg
	move.l	d1,(a4)+	copy long into vector
	dbra	d0,1$
	jmp	(a6)
	

*AND packstring(v,s) = VALOF
*$( LET n = v!0 & #XFF
*   FOR i = 0 TO n DO s%i := v!i
*   RESULTIS n >> 2
*$)
PACKSTRING:
	asl.l	#2,d1		source to mc ptr
	movea.l	d1,a3
	asl.l	#2,d2		dest to mc ptr
	movea.l	d2,a4
	moveq	#0,d0		zero counter
	move.b	3(a3),d0	get length from lsb of !v
	move.l	d0,d1		setup result
	lsr.l	#2,d1
1$	move.l	(a3)+,d2	get word into tfr reg
	move.b	d2,(a4)+	copy byte into dest
	dbra	d0,1$
	jmp	(a6)

*AND capitalch(ch) = 'a' <= ch <= 'z' -> ch + 'A' - 'a', ch
*
CAPITALCH
	cmp.b	#'a',d1
	blt.s	ret
	cmp.b	#'z',d1
	bgt.s	ret
	sub.b	#'a'-'A',d1
ret	jmp	(a6)

*AND compch(ch1, ch2) = capitalch(ch1) - capitalch(ch2)
*
* Evil and ugly, but small
* makes assumptions about capitalch
*
COMPCH
	move.l	a6,-(sp)
	lea	conv_sec,a6
	bra.s	CAPITALCH	; ch1
conv_sec
	exg	d1,d2
	lea	subthem,a6
	bra.s	CAPITALCH	; ch2
subthem
	sub.b	d1,d2		; d1 = ch2, d2 = ch1
	move.b	d2,d1
	ext.w	d1
	ext.l	d1		; return signed long
	move.l	(sp)+,a6
	jmp	(a6)

* AND compstring(s1, s2) = VALOF
*     $(
*     LET lens1, lens2 = s1%0, s2%0
*     LET smaller = lens1 < lens2 -> s1, s2
* 
*     FOR i = 1 TO smaller%0
*     DO
*         $(
*         LET res = compch(s1%i, s2%i)
* 
*         UNLESS res = 0 RESULTIS res
*         $)
* 
*      IF lens1 = lens2 RESULTIS 0
* 
*     RESULTIS smaller = s1 -> -1, 1
*     $)
*
* makes assumptions about capitalch
* takes two BSTR's as arguements - don't want to convert them!!!
*
COMPSTR
	move.l	a6,-(sp)

	asl.l	#2,d1		; inputs are BPTRs
	asl.l	#2,d2
	move.l  d1,a3		; s1
	move.l  d2,a4		; s2
	moveq	#0,d0
	move.b	(a3)+,d6	; lens1
	move.b	(a4)+,d7	; lens2
	cmp.b	d6,d7		; is lens2 - lens1
	blt.s	1$		;   < 0?
	move.l	d6,d5		; smaller%0
	bra.s	2$
1$	move.l	d7,d5		; d5 = min(len(s1),len(s2))
2$
	moveq	#0,d4		; d5 has a byte in it
	move.b	d5,d4		; save d5
	bra.s	end		; to dbra

caseloop:
	move.b	(a4)+,d1	; s2%i

	lea	check2,a6
	bra	CAPITALCH	; capitalize
check2:
	move.l	d1,d2		; returns in d1 - move to d2
	move.b	(a3)+,d1	; s1%i

	lea	cmpit,a6
	bra	CAPITALCH	; capitalize
cmpit:
	sub.b	d2,d1		; capitalch(s1%i) - capitalch(s2%i)
	bne.s	exit		; different - end of comparison

end:	dbra	d4,caseloop	; min of the two lengths
				; equal for length of shorter
	cmp.b	d6,d7		; are lengths the same?
	bne.s	noteq
	moveq	#0,d1		; yes - return same
	bra.s	exit
noteq:
	moveq	#-1,d1		; if smaller == s1, return -1 else 1
	cmp.b	d5,d6		; is d5 (smaller%0) == s1%0?
	beq.s	exit
	neg.b	d1		; no: -(-1) == 1
exit:
	ext.w	d1		; d1 may be byte value
	ext.l	d1		; a bit slower than optimal, i don't care
	move.l	(sp)+,a6
	jmp	(a6)


	IFND KLUDGE
	end
	ENDC
