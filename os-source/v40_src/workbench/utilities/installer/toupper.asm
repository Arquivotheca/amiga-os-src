*****************************************************
*		Parsec Soft Systems string functions		*
*****************************************************

;---------------------------------------------
;	touppper(c) - converts character to uppercase including foreign
;				also callable from assembly

		section	toupper.asm,code

		include 'macros.i'

		DECLAREA	toupper

		move.l	4(sp),d0
toupper
		TOUPPER	d0		; use Talin's macro
		rts

		end
