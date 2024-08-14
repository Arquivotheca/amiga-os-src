* I typed this in because bryce didn't send the online copy
* too bad, I hope I don't screw up.

CLEAR	MACRO
	MOVEQ	#0,\1
	ENDM

CALLSYS	MACRO
	JSR	_LVO\1(A6)
	ENDM

XLIB	MACRO
	XREF	_LVO\1
	ENDM
