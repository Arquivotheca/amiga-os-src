	INCLUDE	"exec/types.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/libraries.i"

	INCLUDE	"keymap.i"

 STRUCTURE	KeyMapLibrary,LIB_SIZE
    ULONG   kl_KMDefault		; default key map
    STRUCT  kl_R,kr_SIZEOF
    STRUCT  kl_USA,kn_SIZEOF
    STRUCT  kl_USA1,kn_SIZEOF
    LABEL   kl_SIZEOF


ABSEXECBASE	EQU	4


XLVO	MACRO
	XREF	_LVO\1
	ENDM

CALLLVO	MACRO
		jsr	_LVO\1(a6)
	ENDM
