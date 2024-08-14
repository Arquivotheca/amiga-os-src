
;*************************************************************************************

	INCLUDE "exec/types.i"

;*************************************************************************************

	XDEF	_nAllocMem
	XDEF	_nFreeMem

	XDEF	alloc0
	XDEF	alloc1

	XDEF	corrupt
	XDEF	badsum

	XDEF	free
	XDEF	free0
	XDEF	free1
	XDEF	free2
	XDEF	free3
	XDEF	free4
	XDEF	free5
	XDEF	free6
	XDEF	free7
	XDEF	free8
	XDEF	free9

;*************************************************************************************

	XREF	_LVOForbid
	XREF	_LVOPermit
	XREF	_newAllocMem
	XREF	_newFreeMem

;*************************************************************************************
; void *ASM nAllocMem (REG (d0) ULONG size, REG (d1) ULONG attrs, REG (a6) struct Library *);
;*************************************************************************************
_nAllocMem:
	jsr	_LVOForbid(a6)
	movem.l	a5,-(a7)
	lea	4(a7),a5
	jsr	_newAllocMem
	movem.l	(a7)+,a5
	jmp	_LVOPermit(a6)

;*************************************************************************************
; void ASM nFreeMem (REG (a1) UBYTE * memb, REG (d0) ULONG size, REG (a6) struct Library *);
;*************************************************************************************
_nFreeMem:
	jsr	_LVOForbid(a6)
	movem.l	a2,-(a7)
	lea	4(a7),a2
	jsr	_newFreeMem
	movem.l	(a7)+,a2
	jmp	_LVOPermit(a6)

;*************************************************************************************

alloc0	dc.b	'Allc: Attempt to alloc zero bytes.',13,10,0
alloc1	dc.b	'Allc: Failed AllocMem(%ld, %08lx)',13,10,0
corrupt	dc.b	'Lower: %08lx  Upper: %08lx  Size: %ld : !! Memory List Corrupt !!',0
badsum	dc.b	' : Sum=%ld',13,10,0

free	dc.b	'Free: (%08lx,%ld) ',0
free0	dc.b	'Attempt to free NULL pointer.',13,10,0
free1	dc.b	'Attempt to free zero bytes.',13,10,0
free2	dc.b	'Invalid memory pointer.',13,10,0
free3	dc.b	'Misaligned pointer.',13,10,0
free4	dc.b	'Cookie checksum failure.',13,10,0
free5	dc.b	'Old cookie.',13,10,0
free6	dc.b	'Attempt to free %ld bytes when allocated %ld.',13,10,0
free7	dc.b	'Pre-wall contains %ld bytes of damage.',13,10,0
free8	dc.b	'Post-wall contains %ld bytes of damage.',13,10,0
free9	dc.b	'Corrupt memory list.',13,10,0
