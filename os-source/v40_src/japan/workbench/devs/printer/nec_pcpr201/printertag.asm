**
**  printertag.asm
**

	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"NEC_PCPR201_rev.i"

	INCLUDE		"devices/prtbase.i"


*------ Imported Names -----------------------------------------------

	XREF		_Init
	XREF		_Expunge
	XREF		_Open
	XREF		_Close
	XREF		_CommandTable
	XREF		_PrinterSegmentData
	XREF		_DoSpecial
	XREF		_Render
;	XREF		_ExtendedCharTable
	XREF		_ConvFunc

*------ Exported Names -----------------------------------------------

	XDEF		_PEDData

**********************************************************************

	MOVEQ	#0,D0              ; show error for OpenLibrary()
	RTS
	DC.W	VERSION
	DC.W	REVISION
_PEDData:
	DC.L	printerName
	DC.L	_Init
	DC.L	_Expunge
	DC.L	_Open
	DC.L	_Close
	DC.B	PPC_COLORGFX       ; PrinterClass
	DC.B	PCC_YMCB           ; ColorClass
;	DC.B	PPC_BWGFX          ; PrinterClass
;	DC.B	PCC_BW             ; ColorClass
	DC.B	136                ; MaxColumns
	DC.B	0                  ; NumCharSets
	DC.W	24                 ; NumRows
	DC.L	2176               ; MaxXDots
	DC.L	0                  ; MaxYDots
	DC.W	160                ; XDotsInch
	DC.W	160                ; YDotsInch
	DC.L	_CommandTable      ; Commands
	DC.L	_DoSpecial         ; Command code function
	DC.L	_Render            ; Graphics render function
	DC.L	30                 ; Timeout
;	DC.L	_ExtendedCharTable ; 8BitChars
	DC.L	0                  ; 8BitChars
	DS.L	1                  ; PrintMode (reserve space)
	DC.L	_ConvFunc          ; ptr to char conversion function

printerName:
	DC.B	"NEC_PCPR201R"
	DC.B	0

		END
