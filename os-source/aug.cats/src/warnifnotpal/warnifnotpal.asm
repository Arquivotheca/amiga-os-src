*
* warnifnotpal.asm - 
* returns WARN if not PAL
*
* blink from warnifnotpal.o to warnifnotpal library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/macros.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "dos/dos.i"

startme:
	movem.l	d2-d7/a2-a6,-(sp)

	move.l	4,a6			;exec
	lea	gname(pc),a1		;graphics.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	bne.s	okopen
	moveq.l	#RETURN_FAIL,d2
	bra.s	badopen
okopen:
	move.l	d0,a1			;GfxBase (a0)
	move.w	gb_DisplayFlags(a1),d0
	andi.w	#PAL,d0
	beq.s	notpal
	moveq.l	#RETURN_OK,d2		;is pal
	bra.s	outahere
notpal:
	moveq.l	#RETURN_WARN,d2		;not pal
outahere:
	move.l	4,a6			;GfxBase still in a1
	JSRLIB	CloseLibrary
badopen:
	move.l	d2,d0			;move return code to d0
	movem.l	(sp)+,d2-d7/a2-a6
	rts


	CNOP	0,2

verstag	DC.B    '$VER: warnifnotpal 40.1 (6.7.93)',0

gname	DC.B	'graphics.library',0

