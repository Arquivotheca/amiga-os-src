
*******************************************************************************
*
*	$Header:
*
*******************************************************************************

	section mathieeesingbas

	xdef	do_trapv

do_trapv:
	ori	#2,ccr
	trapv
	rts

	xdef	do_div0
do_div0:
	move.l	d0,-(sp)
	divu	#0,d0
	move.l	(sp)+,d0
	rts

	end
