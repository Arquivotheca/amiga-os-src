* Amiga Grand Wack
*
* vers.asm -- Storage for the actual version and revision.
*
* $Id: vers.asm,v 39.1 92/11/02 12:42:19 peter Exp $

	SECTION __MERGED,data
	INCLUDE	"wack_rev.i"

	xdef	_VERSION
	xdef	_REVISION

	VERSTAG

		cnop	0,4
_VERSION	dc.w	VERSION
_REVISION	dc.w	REVISION
	end
