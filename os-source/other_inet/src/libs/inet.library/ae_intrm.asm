;
; Lance interrupt stub.
;

	public _ae_intr,_aeintrC,__H1_org
	far	code
	far	data

_ae_intr:
	movem.l	d7/a4-a6,-(sp)		; save interrupted state 
;	lea	-2(sp),sp		; longword align
	lea	__H1_org+32766,a4	; for MANX
	jsr	_aeintrC		; call C int handler
	movem.l	(sp)+,d7/a4-a6		; restore interrupted state
;	lea	2(sp),sp
;	clr.l	d0
	rts
