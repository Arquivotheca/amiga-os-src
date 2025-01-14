***********************************************************************
* Printf.asm - like standard printf, but uses rom RawDoFmt()          *
* Written March 1988 by Talin                                         *
* Also Fprintf.asm - written Oct 1988 by JPearce                      *
***********************************************************************

		section	printf.asm,code

		include	'lat_macros.i'
		xref	_DOSBase

MAXLINE		equ		128
FHPOS		equ		0
COUNTPOS	equ		4
DOSPOS		equ		6
TOTAL		equ		10
BUFPOS		equ		12
LOCSPACE	equ		14

put_char				; d0 = character, a3 = data
		tst.b	d0				; do not write NUL byte
		beq.s	1$

		moveq	#0,d1
		move.w	COUNTPOS(a3),d1		; get count
		move.b	d0,BUFPOS(a3,d1.w)	; put byte in buffer
		addq.w	#1,TOTAL(a3)		; increment printed count
		addq.w	#1,d1				; increment count
		cmp.w	#MAXLINE,d1
		bmi.s	2$

		tst.l	(a3)				; check file handle
		beq.s	4$					; if file handle NULL, skip writing

		SaveM	d2/d3/a0/a6			; save regs
		move.l	d1,d3				; length
		lea		BUFPOS(a3),a0		; get buffer address
		move.l	a0,d2
		move.l	(a3),d1				; file handle
		move.l	DOSPOS(a3),a6		; cached DOSBase pointer (not small code)
		Call	Write
		RestoreM	d2/d3/a0/a6
		tst.l	d0
		bmi.s	3$					; if error, handle it below
4$		moveq	#0,d1
2$		move.w	d1,COUNTPOS(a3)
1$		rts

3$		clr.l	(a3)				; zero cached file handle
		bra.s	4$

		DECLAREA VPrintf			; VPrintf(string,args)

		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		beq.s	bye

		move.l	4+4(sp),a0			; format
		move.l	8+4(sp),a1			; args
		bra.s	Fprintf

VPrintf
		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		beq.s	bye

		DECLAREL Printf				; Printf(string,args)

		move.l	a6,-(sp)
		CallDos	Output				; get standard output
		tst.l	d0
		bne.s	ok
bye		move.l	(sp)+,a6
		rts

ok		move.l	4+4(sp),a0			; format
		lea		8+4(sp),a1			; args
		bra.s	Fprintf

		DECLAREA	VFprintf		; VFprintf(file,string,args)

		move.l	12(sp),a1			; args
		bra.s	merge

		DECLAREL	Fprintf			; Fprintf(file,string,args)

		lea		12(sp),a1			; args
merge
		move.l	4(sp),d0			; file
		move.l	8(sp),a0			; format
VFprintf
		move.l	a6,-(sp)
Fprintf
		movem.l	a2/a3,-(sp)
		sub.w	#MAXLINE+LOCSPACE,sp		; get local storage
		move.l	sp,a3
		move.l	d0,(a3)				; handle
		lea		put_char,a2			; format function
		clr.w	COUNTPOS(a3)		; no characters in buffer
		clr.w	TOTAL(a3)			; no characters printed
		move.l	_DOSBase,DOSPOS(a3)	; DOSBase

		CallEx	RawDoFmt			; format it

		moveq	#0,d1
		move.w	COUNTPOS(a3),d1		; get count
		beq.s	1$

		tst.l	(a3)				; check file handle
		beq.s	2$

		SaveM	d2/d3/a0			; save regs
		move.l	d1,d3				; length
		lea		BUFPOS(a3),a0		; get buffer address
		move.l	a0,d2
		move.l	(a3),d1				; file handle
		CallDos	Write
		RestoreM	d2/d3/a0
		tst.l	d0
		bmi.s	2$					; if error, handle below

1$		move.w	TOTAL(a3),d0		; return count printed
		ext.l	d0
		add.w	#MAXLINE+LOCSPACE,sp
		movem.l	(sp)+,a2/a3
		move.l	(sp)+,a6
		rts

2$		neg.w	TOTAL(a3)			
		bra.s	1$

		end

#define MAXLINE		128

static unsigned short	wprint_size;
static char				*wbuf = (void *)0L;

put_char(c) char c;
{	if (!c) return;
	wbuf[wprint_size++] = c;
	if (wprint_size >= MAXLINE)
	{	Write(Output(),wbuf,wprint_size);
		wprint_size = 0;
	}
}

Printf(string,args) char *string; long args;
{
	if (!Output()) return;							/* can't print if on WB */
	wbuf = buf; wprint_size = 0;					/* start at zero */
	Format(put_char,string,&args);					/* format the string */
	if (wprint_size) Write(Output(),wbuf,wprint_size);
}
