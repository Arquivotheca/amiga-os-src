*
* noopenwb.asm - kills the OpenWorkbench function
* 	systemwide and non-reversible
*
* slink from noopenwb.o to noopenwb library lib:amiga.lib
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/macros.i"

	XREF	_LVOOpenWorkBench

startme:

	movem.l	d1-d7/a0-a6,-(sp)

	move.l	4,a6			;exec
	lea	iname(pc),a1		;intuition.library
	moveq	#00,d0			;any version

	JSRLIB	OpenLibrary
	tst.l	d0
	beq	badopen
	move.l	d0,a5			;IntuitionBase (a5)

	move.l	#cancele-cancel,d0
	move.l	#0,d1
	JSRLIB	AllocMem
	move.l	d0,d2			;dest for code (d2)
	beq.s	outahere

	lea.l	cancel,a0		;source
	movea.l	d0,a1			;dest
	move.l	#cancele-cancel,d0	;size
	JSRLIB	CopyMem			;copy code to alloc'd mem


	movea.l	a5,a1			;library to setfunction (intuition)
	movea.l #_LVOOpenWorkBench,a0	;offset
	move.l	d2,d0			;new code
	JSRLIB	SetFunction

outahere:
	movea.l	a5,a1			;IntuitionBase
	move.l	4,a6
	JSRLIB	CloseLibrary
badopen:
	moveq	#00,d0			;return OK
	movem.l	(sp)+,d1-d7/a0-a6
	rts

* Code to be copied to allocated RAM
cancel:
	moveq.l	#0,d0
	rts
cancele:

	CNOP	0,2

verstag	DC.B    '$VER: noopenwb 37.1 (3.11.93)',0

iname	DC.B	'intuition.library',0
