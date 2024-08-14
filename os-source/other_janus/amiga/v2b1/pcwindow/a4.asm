
	INCLUDE	"zaphod.i"

	IFD	AZTEC
	DSEG
	EVEN
	ENDC

a4save	dc.l    0

	IFD	AZTEC
	CSEG
        PUBLIC  _SaveA4
        PUBLIC  _GetA4
	ENDC
	IFND	AZTEC
        XDEF    _SaveA4
        XDEF    _GetA4
	ENDC

_SaveA4:
        lea     a4save,a0
        move.l  a4,(a0)
        rts

_GetA4:
        move.l  a4save,a4
        rts










	END

