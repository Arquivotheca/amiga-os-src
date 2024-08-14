	section mathieeesingtrans

;	xdef	MIIEEESPSin
	xdef	MIIEEESPCos
;	xdef	MIIEEESPTan
	xdef	MIIEEESPSincos
	xdef	MIIEEESPSinh
	xdef	MIIEEESPCosh
	xdef	MIIEEESPTanh
;	xdef	MIIEEESPExp
	xdef	MIIEEESPLog
	xdef	MIIEEESPPow
;	xdef	MIIEEESPSqrt
;	xdef	MIIEEESPAsin
;	xdef	MIIEEESPAcos
	xdef	MIIEEESPLog10

	xref	_rsin
	xref	_rcos
	xref	_rtan
	xref	_rsincos
	xref	_rsinh
	xref	_rcosh
	xref	_rtanh
;	xref	_rexp
	xref	_rlog
	xref	_rpow
	xref	_rsqrt
	xref	_rtieee
	xref	_rfieee
;	xref	_rasin
;	xref	_racos
	xref	_rlog10
	
;MIIEEESPSin:
;	move.l	d0,-(sp)
;	bsr	_rsin
;	addq.l	#4,sp
;	rts
MIIEEESPCos:
	move.l	d0,-(sp)
	bsr	_rcos
	addq.l	#4,sp
	rts
MIIEEESPTan:
	move.l	d0,-(sp)
;	bsr	_rtan
	addq.l	#4,sp
	rts
MIIEEESPSincos:
	move.l	a0,-(sp)
	move.l	d0,-(sp)
	bsr	_rsincos
	addq.l	#8,sp
	rts
MIIEEESPSinh:
	move.l	d0,-(sp)
	bsr	_rsinh
	addq.l	#4,sp
	rts
MIIEEESPCosh:
	move.l	d0,-(sp)
	bsr	_rcosh
	addq.l	#4,sp
	rts
MIIEEESPTanh:
	move.l	d0,-(sp)
	bsr	_rtanh
	addq.l	#4,sp
	rts
;MIIEEESPExp:
;	move.l	d0,-(sp)
;	bsr	_rexp
;	addq.l	#4,sp
;	rts
MIIEEESPLog:
	move.l	d0,-(sp)
	bsr	_rlog
	addq.l	#4,sp
	rts
MIIEEESPPow:
	move.l	d1,-(sp)
	move.l	d0,-(sp)
	bsr	_rpow
	addq.l	#8,sp
	rts
;MIIEEESPSqrt:
;	move.l	d0,-(sp)
;	bsr	_rsqrt
;	addq.l	#4,sp
;	rts
;MIIEEESPAsin:
;	move.l	d0,-(sp)
;	bsr	_rasin
;	addq.l	#4,sp
;	rts
;MIIEEESPAcos:
;	move.l	d0,-(sp)
;	bsr	_racos
;	addq.l	#4,sp
;	rts
;	xref	_ratan
;	xdef	MIIEEESPAtan
;MIIEEESPAtan:
;	move.l	d0,-(sp)
;	bsr	_ratan
;	addq.l	#4,sp
;	rts
MIIEEESPLog10:
	move.l	d0,-(sp)
	bsr	_rlog10
	addq.l	#4,sp
	rts


	end
