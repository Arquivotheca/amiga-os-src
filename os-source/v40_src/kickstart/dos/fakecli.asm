        PLEN 55
        TTL    "*** cli global vector definitions ***"

	INCLUDE "libhdr.i"
	INCLUDE "calldos.i"

	XDEF	FAKECLI
	XDEF	FAKECLI_SEG
	XDEF	_bcpl_cli_init

*	XREF	_start
	XREF	@cli_init
	XREF	CALLDOS
	
FAKECLI_SEG
	DC.L	0			; no next segment
FAKECLI
	DC.L	(FAKECLIEND-FAKECLI)/4

*
* start() in cli.c is a C routine, and is called from the global vector
* as a bcpl routine.
*
*start
*	lea	_start,a3
common
	lea	BPTRCPTR+(CALLDEND<<ARG2),a4
	bra	CALLDOS

*
* cli_init is called by cli as a BCPL routine
*
_bcpl_cli_init:
cli_init:
	lea	@cli_init,a3
	bra.s	common

	CNOP	0,4

	DC.L	0
	DC.L	G_START/4,-1			; FIX!!! ? (start-FAKECLI)
	DC.L	G_CLISTART/4,(cli_init-FAKECLI)

	DC.L	G_GLOBMAX/4
FAKECLIEND
	END
