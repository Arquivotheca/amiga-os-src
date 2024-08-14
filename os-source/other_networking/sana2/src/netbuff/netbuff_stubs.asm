**********************************************************************
*
* NetBuff_stubs.asm
*
* Copyright 1991 Raymond S. Brand, All rights reserved.
*
* 19910212
*
* $Id$
*
**********************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"


	SECTION	CSTUB

	CODE

	XDEF	_IntAllocSegments
	XDEF	_AllocSegments
	XDEF	_IntFreeSegments
	XDEF	_FreeSegments
	XDEF	_SplitNetBuff
	XDEF	_TrimNetBuff
	XDEF	_CopyToNetBuff
	XDEF	_CopyFromNetBuff
	XDEF	_CopyNetBuff
	XDEF	_CompactNetBuff
	XDEF	_ReadyNetBuff
	XDEF	_IsContiguous
	XDEF	_NetBuffAppend
	XDEF	_PrependNetBuff
	XDEF	_ReclaimSegments

	XREF	_NetBuffBase

	XREF	_LVOIntAllocSegments
	XREF	_LVOAllocSegments
	XREF	_LVOIntFreeSegments
	XREF	_LVOFreeSegments
	XREF	_LVOSplitNetBuff
	XREF	_LVOTrimNetBuff
	XREF	_LVOCopyToNetBuff
	XREF	_LVOCopyFromNetBuff
	XREF	_LVOCopyNetBuff
	XREF	_LVOCompactNetBuff
	XREF	_LVOReadyNetBuff
	XREF	_LVOIsContiguous
	XREF	_LVONetBuffAppend
	XREF	_LVOPrependNetBuff
	XREF	_LVOReclaimSegments


_IntAllocSegments:
;	IntAllocSegments( Count, Segment_List )
;	                  D0     A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),d0
	move.l	12(sp),a0
	jsr	_LVOIntAllocSegments(a6)
	move.l	(sp)+,a6
	rts


_AllocSegments:
;	AllocSegments( Count, Segment_List )
;	               D0     A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),d0
	move.l	12(sp),a0
	jsr	_LVOAllocSegments(a6)
	move.l	(sp)+,a6
	rts


_IntFreeSegments:
;	IntFreeSegments( Segment_list )
;	                 A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	jsr	_LVOIntFreeSegments(a6)
	move.l	(sp)+,a6
	rts


_FreeSegments:
;	FreeSegments( Segment_list )
;	              A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	jsr	_LVOFreeSegments(a6)
	move.l	(sp)+,a6
	rts


_SplitNetBuff:
;	error = SplitNetBuff( Netbuff0, Offset, Netbuff1 )
;	D0                    A0        D0      A1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	move.l	16(sp),a1
	jsr	_LVOSplitNetBuff(a6)
	move.l	(sp)+,a6
	rts


_TrimNetBuff:
;	error = TrimNetBuff( Netbuff, Count )
;	D0                   A0        D0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	jsr	_LVOTrimNetBuff(a6)
	move.l	(sp)+,a6
	rts


_CopyToNetBuff:
;	error = CopyToNetBuff( Netbuff, Offset, Data, Count )
;	D0                     A0       D0      A1    D1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	move.l	16(sp),a1
	move.l	20(sp),d1
	jsr	_LVOCopyToNetBuff(a6)
	move.l	(sp)+,a6
	rts


_CopyFromNetBuff:
;	error = CopyFromNetBuff( Netbuff, Offset, Data, Count )
;	D0                       A0       D0      A1    D1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	move.l	16(sp),a1
	move.l	20(sp),d1
	jsr	_LVOCopyFromNetBuff(a6)
	move.l	(sp)+,a6
	rts


_CopyNetBuff:
;	error = CopyNetBuff( Netbuff0, Netbuff1 )
;	D0                   A0        A1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),a1
	jsr	_LVOCopyNetBuff(a6)
	move.l	(sp)+,a6
	rts


_CompactNetBuff:
;	error = CompactNetBuff( Netbuff )
;	D0                      A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	jsr	_LVOCompactNetBuff(a6)
	move.l	(sp)+,a6
	rts


_ReadyNetBuff:
;	error = ReadyNetBuff( Netbuff, Count )
;	D0                    A0       D0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	jsr	_LVOReadyNetBuff(a6)
	move.l	(sp)+,a6
	rts


_IsContiguous:
;	result = IsContiguous( Netbuff, Offset, Count )
;	D0                     A0       D0      D1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),d0
	move.l	16(sp),d1
	jsr	_LVOIsContiguous(a6)
	move.l	(sp)+,a6
	rts


_NetBuffAppend:
;	error = NetBuffAppend( Netbuff0, Netbuff1 )
;	D0                     A0        A1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),a1
	jsr	_LVONetBuffAppend(a6)
	move.l	(sp)+,a6
	rts


_PrependNetBuff:
;	error = PrependNetBuff( Netbuff0, Netbuff1 )
;	D0                      A0        A1
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	move.l	12(sp),a1
	jsr	_LVOPrependNetBuff(a6)
	move.l	(sp)+,a6
	rts


_ReclaimSegments:
;	error = ReclaimSegments( Netbuff )
;	D0                       A0
	move.l	a6,-(sp)
	move.l	_NetBuffBase,a6
	move.l	8(sp),a0
	jsr	_LVOReclaimSegments(a6)
	move.l	(sp)+,a6
	rts


	END
