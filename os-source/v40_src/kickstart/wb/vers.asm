*
* $Id: vers.asm,v 38.1 91/06/24 11:39:15 mks Exp $
*
	SECTION section

	XDEF	VERNUM
	XDEF	REVNUM
	XDEF	_VERSTRING

	INCLUDE	'wb_rev.i'

beginning:
	moveq	#0,d0	; prevent anyone from running us
	rts

_VERSTRING:	VSTRING
VERNUM		EQU	VERSION
REVNUM		EQU	REVISION

		END


