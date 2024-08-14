
	include	janus\janusvar.inc
	include	janus\services.inc

;	Dual Port RAM Buffer area definitions

AMouseBufMem	struc

	AmigaPCX	dw	0		; X Increment from Amiga
	AmigaPCY	dw	0		; Y Increment from Amiga
	AmigaPCLeftP	dw	0		; Left Pressed count f. A.
	AmigaPCRightP	dw	0		; Right Pressed count f. A.
	AmigaPCLeftR	dw	0		; Left Released count f. A.
	AmigaPCRightR	dw	0		; Right Released count f. A.
	AmigaPCStatus	dw	0		; Buttons Status f.A.

AMouseBufMem	ends

DPSize			equ	8	; words

;	Dual Port RAM Parameter area definitions

AMouseParamMem	struc

	BufMemOffset	dw	0		; Buffer Memory offset
	PCParam		dw	0		; Parameter for AMiga
	WriteLock	db	0		; lock byte as seen by PC

AMouseParamMem	ends

;	Janus definitions

JIntNum			equ	JSERV_AMOUSE


;	Card type definitions

VGAType		equ	1
EGAType		equ	2
CGAType		equ	4
MDAType		equ	8
HerType		equ	16

;	EGA Lib Table

ELTab	struc

	EGA_Port	dw	0		; Address of port
	EGA_Values	dw	0		; Addr. of values for pointer
						; Actual value for single
	EGA_Defs	dw	0		; Defaults (as Values)
	EGA_Count	db	0		; number of registers at port
	EGA_Status	db	0		; status byte

ELTab	ends

