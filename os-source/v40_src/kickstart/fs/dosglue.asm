 		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"libhdr.i"
		LIST

		dc.l	0		; End of global list
		dc.l	G_START/4,4	; include pad for longword size
		dc.l	150
	END

