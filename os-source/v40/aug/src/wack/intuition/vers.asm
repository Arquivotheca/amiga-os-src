* Amiga Grand Wack
*
* vers.asm -- Storage for the actual version and revision.
*
* $Id: vers.asm,v 40.1 93/09/10 11:11:26 peter Exp $

	SECTION __MERGED,data
	INCLUDE	"intuition.wack_rev.i"

	xdef	_VERSION
	xdef	_REVISION

	VERSTAG

		cnop	0,4
_VERSION	dc.w	VERSION
_REVISION	dc.w	REVISION
	end
