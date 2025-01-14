; IMATCH - a macro that quickly does a case-insensitive character test
;	note this function destroys the two registers passed to it.

;			IMATCH		reg1,reg1,matchlabel,nomatchmabel

			section		TEXT,CODE

*			xdef		imatch,_imatch
*			xdef		str_mch,_str_mch
			xdef		str_mchi,_str_mchi
*			xdef		str_nmch,_str_nmch
*			xdef		str_nmchi,_str_nmchi

IMATCH:		macro

			cmp.b		\1,\2					; if characters match
			beq.s		\3
			eor.b		\1,\2					; get exclusive or of characters
			cmp.b		#32,\2					; if different only by bit 32
			bne.s		\4						; then
			bset		#5,\1					; force to upper case
			cmp.b		#$61,\1					; if less than lower case 'a'
			blo.s		\4						; then no match
			cmp.b		#$7a,\1					; if less that or equal to lower 'z'
			bls.s		\3						; then match
			cmp.b		#$E0,\1					; if less than lower case '�'
			blo.s		\4						; then no match
			bra.s		\3						; then match

			endm

*_imatch:	move.b		4+3(sp),d0
*			move.b		8+3(sp),d0
imatch:		IMATCH		d0,d1,1$,2$
1$			moveq		#-1,d0
			rts
2$			moveq		#0,d0
			rts

; match a null-terminated string against a (possibly unterminated) buffer.
; if the string does not match the first part of the buffer return failure;
; else returns a pointer to (buffer + length of 1st string).
; Uses case-sensitive comparison.

			IFGT	0
_str_mch:	move.l	4(sp),a0
			move.l	8(sp),a1
str_mch:				; (a0=terminated string ,a1=unterminated string)
1$			move.b	(a0)+,d1					; get char from string 1
			beq.s	2$							; if end string 1 then succeed
			cmp.b	(a1)+,d1					; compare with char from str2
			beq.s	1$							; if equal, then continue
			sub.l	a1,a1						; else fail (a1 = 0)
2$			move.l	a1,d0						; return next parse point
			rts
			ENDC

; match a null-terminated string against a (possibly unterminated) buffer.
; if the string does not match the first part of the buffer return failure;
; else returns a pointer to (buffer + length of 1st string).
; Uses case-insensitive comparison.

_str_mchi:	move.l	4(sp),a0
			move.l	8(sp),a1
str_mchi:				; (a0=terminated string ,a1=unterminated string)
1$			move.b	(a0)+,d0					; get char from string 1
			beq.s	2$							; if end string 1, then succeed
			move.b	(a1)+,d1					; get char from string 2
			jsr		imatch						; compare them
			bne.s	1$							; if equal, then continue
			move	d0,a1						; return 0
2$			move.l	a1,d0						; return pointer to last match loc
			rts

; match a null-terminated string against a (possibly unterminated) buffer.
; if the string does not match the first part of the buffer, or if the string
; length is greater than bufsize, return failure; else returns a pointer to
; (buffer + length of 1st string). Uses case-sensitive comparison.

			IFGT	0
_str_nmch:	move.l	4(sp),a0
			move.l	8(sp),a1
			move.l	12(sp),d0
str_nmch:				; (a0=terminated string ,a1=unterminated string,d0=bufsize)
			move.l	d2,-(sp)					; character count
			move.l	d0,d2						; get count
			bra.s	2$
1$			move.b	(a0)+,d1					; get char from string 1
			beq.s	3$							; if end string 1 then succeed
			addq.l	#1,d0						; add 1 to length of string1
			cmp.b	(a1)+,d1					; compare with char from str2
2$			dbne	d2,1$						; if equal, then continue
			sub.l	a1,a1						; else fail (a1 = 0)
3$			move.l	(sp)+,d2
			move.l	a1,d0						; return next parse point
			rts
			ENDC

; match a null-terminated string against a (possibly unterminated) buffer.
; if the string does not match the first part of the buffer, or if the string
; length is greater than bufsize, return failure; else returns a pointer to
; (buffer + length of 1st string). Uses case-insensitive comparison.

			IFGT	0
_str_nmchi:	move.l	4(sp),a0
			move.l	8(sp),a1
			move.l	12(sp),d0
str_nmchi:				; (a0=terminated string ,a1=unterminated string,d0=bufsize)
			move.l	d2,-(sp)					; character count
			move.l	d0,d2						; get count in d2
			bra.s	2$
1$			move.b	(a0)+,d0					; get char from string 1
			beq.s	3$							; if end string 1, then succeed
			move.b	(a1)+,d1					; get char from string 2
			jsr		imatch						; compare them
2$			dbeq	d2,1$						; if equal, then continue
			move	d0,a1						; return 0
3$			move.l	(sp)+,d2
			move.l	a1,d0						; return pointer to last match loc
			rts
			ENDC

			end
