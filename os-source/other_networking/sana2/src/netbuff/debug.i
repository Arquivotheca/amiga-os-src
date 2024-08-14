	XREF	KPutStr
	XREF	KPutFmt


ERRMSG:	MACRO	* msg
;
;	movem.l	a0/a1/d0/d1,-(sp)
;	DISABLE	a0
;	lea	msg\@,a0
;	jsr	KPutStr
;	ENABLE	a0
;	movem.l (sp)+,d0/d1/a0/a1
;	bra.s	end\@
; 
;msg\@	dc.b	\1
;	dc.b	10
;	dc.b	0
;	ds.w	0
;end\@
	ENDM


DPRINTF:	MACRO	* msg
;
;	movem.l	a0/a1/d0/d1,-(sp)
;	lea	dmsg\@,a0
;	lea	4*4(sp),a1
;	jsr	KPutFmt
;	movem.l (sp)+,d0/d1/a0/a1
;	addq.l	#4,sp
;	bra.s	dend\@
; 
;dmsg\@	dc.b	<\1>
;	dc.b	10
;	dc.b	0
;	ds.w	0
;dend\@
	ENDM
