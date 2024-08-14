*
* freeanim.asm - open and close freeanim.library
*
* blink from freeanim.o to freeanim library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/macros.i"

startme:

	move.l	4,a6			;exec
	lea	libname(pc),a1		;freeanim.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq.s	nolib
	movea.l	d0,a1
	JSRLIB	CloseLibrary
nolib
	moveq.l	#0,d0
	rts

libname	DC.B	'freeanim.library',0

	END
