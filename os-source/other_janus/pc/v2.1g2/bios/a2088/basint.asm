;
; changed --------------------------------------------------------------
;
;**********************************************
;
; FRCINT   ( ENTRY )
;
;*********************************************
;  LOCATION MUST BE 0F000:0C003H       ; ..ADDRESS FOR IBM COMPATIBILITY.
;
;  which maps from 0F000:4003h in the IBM ROM BASIC
;  where original entries are located.
;
;  So we trap them, since our BIOS ROM is not fully decoded.
;
FRCINT	PROC	FAR
	CLC				; ..INDICATE THIS ROUTINE
	INT	0C0h			; ..INTO OUR BASICA
	RET
FRCINT	ENDP
;*********************************************
;
; MAKINT   ( ENTRY )
;
;********************************************
MAKINT	PROC	FAR
	STC				; INDICATE THIS ROUTINE
	INT	0C0h			; ..OUR BASICA HANDLES THE ROUTINE
	RET
MAKINT	ENDP
;
;---------------------------------------------------------------------------
;