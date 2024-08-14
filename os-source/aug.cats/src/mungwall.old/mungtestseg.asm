* mungtestseg.asm
* Asm -ih:include/ -omungtestseg.o mungtestseg.asm
* Blink FROM mungtestseg.o TO mungtestseg LIBRARY LIB:amiga.lib

******* Included Files *************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"


******* Macros *********************************************************


callsys	MACRO
	XREF	_LVO\1
	JSR	_LVO\1(a6)
	ENDM

******* Imported *******************************************************

ABSEXECBASE	EQU	4
ALLOCSIZE	EQU	500000000

	section code

* called with a1=memory, d0=size
start:
		movem.l	d0-d1/a0-a1/a6,-(sp)
		move.l	ABSEXECBASE,a6
		subq.l	#1,d0
		callsys	FreeMem
		movem.l	(sp)+,d0-d1/a0-a1/a6
		rts

	END
