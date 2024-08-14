
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"

;---------------------------------------------------------------------------

	XREF	_oldAllocMem
	XREF	_oldAvailMem

	XDEF	@NoFastAllocMem
	XDEF	@NoFastAvailMem

;---------------------------------------------------------------------------

@NoFastAllocMem:
	bset	#MEMB_CHIP,d1
	bclr    #MEMB_FAST,d1
	move.l	_oldAllocMem,a0
	jmp	(a0)

;---------------------------------------------------------------------------

@NoFastAvailMem:
	bset	#MEMB_CHIP,d1
	move.l	_oldAvailMem,a0
	jmp	(a0)

;---------------------------------------------------------------------------

	END
