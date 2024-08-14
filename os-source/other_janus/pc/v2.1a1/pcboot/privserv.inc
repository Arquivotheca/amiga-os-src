;*****************************************************************************
;
; PrivServ.inc contains private structures and debug switches for service1+2 
;
;******************************************************************************
.xlist
;
; Main Service structure (private for RJ and me)
;
ServiceParam		struc

ServiceLock		db	0	; lock this byte before walking 
ReservePad1		db	0	;  through ServiceData

PCToAmiga		dw   4 dup (0)	; channel information area set by PC
AmigaToPC		dw   4 dup (0)	; channel information area set by Amiga

PCAddService		dw   2 dup (0) 	; transfer added channel number here 
PCDeleteService		dw   2 dup (0) 	; transfer deleted channel number here

AmigaAddService		dw   2 dup (0)	; transfer added channel number here 			
AmigaDeleteService	dw   2 dup (0) 	; transfer deleted channel number here

FirstServiceData 	dw	0	; points to 1. ServiceData structure
					;  -1: no valid structure available
CyclopsReserved		dd   4 dup (0)	; Amiga	reserved

ServiceParam		ends 
;
.list
