;***********************************************************************
; (PC Side File)
;
; mouseser.inc - MouseServ specific data structures
;
; 11-06-90 - rsd - created
;************************************************************************/

; our magic numbers
MOUSESERV_APPLICATION_ID	equ	1
MOUSESERV_LOCAL_ID		equ	2

; mouse "request" structure to get amiga mouse info
MouseServReq	struc
	AmigaPCX	dw	0	; mouse delta X in "mickeys"
	AmigaPCY	dw	0	; mouse delta Y in "mickeys"
	AmigaPCLeftP	dw	0	; delta count of left button presses
	AmigaPCRightP	dw	0	; delta count of right button presses
	AmigaPCLeftR	dw	0	; delta count of left button releases
	AmigaPCRightR	dw	0	; delta count of right button releases
	AmigaPCStatus	dw	0	; button status (?)
	WriteLock	db	0	; the lock
	ChangeCount	db	0	; change count
MouseServReq	ends

; 7 words excluding the lock & change count
DPSize		equ	7

; card types
VGAType		equ	1
EGAType		equ	2
CGAType		equ	4
MDAType		equ	8
HerType		equ	16

; EGA Lib Table
ELTab struc
	EGA_Port	dw	0	; address of port
	EGA_Values	dw	0	; addr. of values for pointer
	EGA_Defs	dw	0	; defaults (as values)
	EGA_Count	db	0	; number of registers at port
	EGA_Status	db	0	; status byte
ELTab ends

