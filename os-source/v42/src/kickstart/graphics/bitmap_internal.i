	include	'exec/types.i'

	BITDEF IBM,INTERLEAVED,0
	BITDEF IBM,FAST,1
	BITDEF IBM,CHUNKY,2

ALWAYS_USE_CPU	set	1		; if set, then always use fast memory versions of
							; routines.