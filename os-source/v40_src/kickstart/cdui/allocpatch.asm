        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	_PatchStart
	XDEF	_patchSize

;---------------------------------------------------------------------------

_PatchStart:
	DC.L	0
	or.l	#MEMF_REVERSE,d1
	move.l	_PatchStart(pc),a0
	jmp	(a0)

_PatchEnd:

_patchSize DC.L (_PatchEnd-_PatchStart)


;---------------------------------------------------------------------------

	END
