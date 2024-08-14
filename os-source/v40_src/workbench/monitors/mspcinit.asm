
	include "exec/types.i"
	include "graphics/gfxnodes.i"
	include "graphics/monitor.i"

	xdef    _MonitorSpecInit

_MonitorSpecInit:

	movem.l a6,-(sp)        * save
	move.l  8(sp),a6        * libbase
	move.l  12(sp),a0       * mspc
	move.l  16(sp),d0       * flags
	pea.l	init_rts(pc)
	move.l	XLN_INIT(a0),-(sp)
	rts						* doit
init_rts:
	movem.l (sp)+,a6        * restore
	rts

	end
