*		Sprintf(buffer,format,args)

		section	sprintf.asm,code

		include 'macros.i'

sput_char
		move.b	d0,(a3)+
		rts

		DECLAREA VSprintf				; VSprintf(buffer,format,args)

		move.l	12(sp),a1				; get args start
		bra.s	merge

		DECLAREL Sprintf				; Sprintf(buffer,format,args)

		lea		12(sp),a1				; get args start
merge
		movem.l	a2/a3/a6,-(sp)
		move.l	16(sp),a3				; store buffer location
		move.l	20(sp),a0				; get format string address
		lea		sput_char,a2			; routine address

		CallEx	RawDoFmt				; format it

		movem.l	(sp)+,a2/a3/a6
		move.l	4(sp),d0				; buffer start is return value
		rts

VSprintf								; (d0,a0,a1)
		movem.l	d0/a2/a3/a6,-(sp)
		move.l	d0,a3					; store buffer location
		lea		sput_char,a2			; routine address

		CallEx	RawDoFmt				; format it

		movem.l	(sp)+,d0/a2/a3/a6
		rts

		end
