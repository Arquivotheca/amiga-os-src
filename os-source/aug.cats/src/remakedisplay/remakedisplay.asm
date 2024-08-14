*
* remakedisplay.asm - remakes display to get copperlists as low
*   as possible in memory.
*
* blink from remakedisplay.o to remakedisplay library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/macros.i"

startme:

	movem.l	d1-d7/a0-a6,-(sp)

	move.l	4,a6			;exec
	lea	iname(pc),a1		;intuition.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq	badopen

	move.l	d0,a6			;IntuitionBase
	JSRLIB	RemakeDisplay

	movea.l	a6,a1			;IntuitionBase
	move.l	4,a6
	JSRLIB	CloseLibrary
badopen:
	moveq	#00,d0			;return OK
	movem.l	(sp)+,d1-d7/a0-a6
	rts

iname	DC.B	'intuition.library',0
