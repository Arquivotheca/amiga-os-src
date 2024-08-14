*****************************************************
*		Parsec Soft Systems string functions		*
*****************************************************

;---------------------------------------------
;	tolower(c) - converts character to lowercase including foreign

		section	code

		include 'macros.i'

		DECLAREL tolower

		move.l	4(sp),d0

		xdef	tolower
tolower
		TOLOWER	d0		; use Talin's macro
		rts

		end
