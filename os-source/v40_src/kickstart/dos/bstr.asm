*
* bstr.asm
*
* BCPL strings that must be long-aligned (mainly for cli_init.b)
*
	INCLUDE "exec/types.i"
	INCLUDE	"exec/memory.i"

	SECTION bstr,DATA

	XDEF	_b_prompt
	XDEF	_b_porthandler

	CNOP	0,4
_b_prompt:
	dc.b	$4,'%N> '

	CNOP	0,4
_b_porthandler:
	dc.b	$e,'L:port-handler',$0

	END
