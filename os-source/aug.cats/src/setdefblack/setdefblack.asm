*
* setdefblack.asm - set old prefs colors to all black
*
* slink from setdefblack.o to setdefblack library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/macros.i"
	INCLUDE "intuition/preferences.i"

	XREF	_LVOGetPrefs
	XREF	_LVOSetPrefs

startme:

	movem.l	d1-d7/a0-a6,-(sp)

	move.l	4,a6			;exec
	lea	iname(pc),a1		;intuition.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq	badopen
	move.l	d0,a6			;IntuitionBase (a5)

	move.l	#pf_SIZEOF,d0
	lea.l	prefstruct,a0
	jsr	_LVOGetPrefs(a6)
	tst.l	d0
	beq.s	outahere

	move.l	d0,a0
	move.w	#0,pf_color0(a0)
	move.w	#1,pf_color0(a0)
	move.w	#2,pf_color0(a0)
	move.w	#3,pf_color0(a0)
	move.l	#pf_SIZEOF,d0
	move.l	#0,d1
	jsr	_LVOSetPrefs(a6)

outahere:
	movea.l	a6,a1			;IntuitionBase
	move.l	4,a6
	JSRLIB	CloseLibrary
badopen:
	moveq	#00,d0			;return OK
	movem.l	(sp)+,d1-d7/a0-a6
	rts

	CNOP	0,2

prefstruct	DS.B	pf_SIZEOF

verstag	DC.B    '$VER: setdefblack 37.1 (3.11.93)',0

iname	DC.B	'intuition.library',0

