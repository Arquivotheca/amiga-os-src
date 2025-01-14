
*******************************************************************************
*
*	$Id: interface.asm,v 42.0 93/06/16 11:17:05 chrisg Exp $
*
*******************************************************************************

	include "exec/types.i"
	include "graphics/gfxnodes.i"
	include "graphics/monitor.i"

	section graphics
	code

	xdef    _mspc_init

_mspc_init:

	movem.l a6,-(sp)        * save
	move.l  8(sp),a6        * libbase
	move.l  12(sp),a0       * mspc
	move.l  16(sp),d0       * flags
	pea.l	init_rts(pc)
	move.l	XLN_INIT(a0),-(sp)
	rts			* doit
init_rts:
	movem.l (sp)+,a6        * restore
	rts

	xdef    _mspc_transform

_mspc_transform:

	movem.l	a2/a6,-(sp)	* save
	move.l	12(sp),a6  	* libbase
	move.l	16(sp),a0 	* node
	move.l	20(sp),a1 	* src
	move.l	24(sp),d0 	* type
	move.l	28(sp),a2 	* dst
	pea.l	transform_rts(pc)
	move.l	ms_transform(a0),-(sp)
	rts			* doit
transform_rts:
	movem.l	(sp)+,a2/a6	* restore
	rts

	xdef	_mspc_translate

_mspc_translate:

	movem.l	a2/a6,-(sp)	* save
	move.l	12(sp),a6  	* libbase
	move.l	16(sp),a0 	* node
	move.l	20(sp),a1 	* src
	move.l	24(sp),d0 	* type
	move.l	28(sp),a2 	* dst
	pea.l	translate_rts(pc)
	move.l	ms_translate(a0),-(sp)
	rts			* doit
translate_rts:
	movem.l	(sp)+,a2/a6	* restore
	rts

	xdef	_mspc_scale

_mspc_scale:

	movem.l	a2/a6,-(sp)	* save
	move.l	12(sp),a6  	* libbase
	move.l	16(sp),a0 	* node
	move.l	20(sp),a1 	* src
	move.l	24(sp),d0 	* type
	move.l	28(sp),a2 	* dst
	pea.l	scale_rts(pc)
	move.l	ms_scale(a0),-(sp)
	rts			* doit
scale_rts:
	movem.l	(sp)+,a2/a6	* restore
	rts

	xdef	_mspc_oscan

_mspc_oscan:

	movem.l	a6,-(sp)	* save
	move.l	8(sp),a6  	* libbase
	move.l	12(sp),a0 	* node
	move.l	16(sp),a1 	* rect
	move.l	20(sp),d0 	* type
	pea.l	oscan_rts(pc)
	move.l	ms_maxoscan(a0),-(sp)
	rts			* doit
oscan_rts:
	movem.l	(sp)+,a6	* restore
	rts

	xdef _gfxnode_init
	xref _gfxinit

*	gfxinit(	gfxnode	),	GB
*			a0		a6

_gfxnode_init:

	move.l	a0,-(sp)
	jsr     _gfxinit
	addq.l	#4,sp
	rts

*	flags = init_monitorspec(	mspc,	type	),	GB
*	d0				a0	d0		a6

	xdef _monitor_init
	xref _init_monitorspec

*	flags = init_monitorspec(	mspc,	type	),	GB
*	d0				a0	d0		a6

_monitor_init:

	move.l	d0,-(sp)
	move.l	a0,-(sp)
	jsr     _init_monitorspec
	addq.l	#8,sp
	rts

	xdef _monitor_transform
	xref _transform_monitorspec

*	transform_monitorspec(	mspc,	src,	type,	dst	), 	GB
*				a0,	a1,	d0,	a2    		a6

_monitor_transform:

	move.l	a2,-(sp)
	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	jsr	_transform_monitorspec
	lea.l	16(sp),sp
	rts

	xdef _monitor_translate
	xref _translate_monitorspec

*	translate_monitorspec(	mspc,	src,	type,	dst	), 	GB
*				a0,	a1,	d0,	a2    		a6

_monitor_translate:

	move.l	a2,-(sp)
	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	jsr	_translate_monitorspec
	lea.l	16(sp),sp
	rts

	xdef _monitor_scale
	xref _scale_monitorspec

*	scale_monitorspec(	mspc,	src,	type,	dst	), 	GB
*				a0,	a1,	d0,	a2    		a6

_monitor_scale:

	move.l	a2,-(sp)
	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	jsr	_scale_monitorspec
	lea.l	16(sp),sp
	rts

	xdef _unity_scale

_unity_scale:

	move.l	(a1),(a2)	* copy src point to dest
	rts		  	* that's all folks! :-)

	xdef _monitor_bounds
	xref _maxoscan

*	maxoscan(	mspc,	rect,	mode	), 	GB
*			a0,	a1,	d0		a6

_monitor_bounds:

	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	jsr	_maxoscan
	lea.l	12(sp),sp
	rts

	xdef _video_bounds
	xref _videoscan

*	videoscan(	mspc,	rect,	mode	), 	GB
*			a0,	a1,	d0		a6

_video_bounds:

	move.l	d0,-(sp)
	move.l	a1,-(sp)
	move.l	a0,-(sp)
	jsr	_videoscan
	lea.l	12(sp),sp
	rts

	xdef    _stuf_base

_stuf_base:

	move.l	4(sp),a6
	rts

	end
