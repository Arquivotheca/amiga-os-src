;
; Communication Structures used
; by IOCTL
;
; This block is returned after an IOCTL Read command:
;
Ident	STRUC			; IOCTL Identify
I_Length	DB      ?       ; Length of this reply, = SIZE Ident
I_Marker	DW      ?	; Marker, must be "JD"
I_Version       DB      ?	; driver version
I_BaseDrv	DB	?	; base drive #
I_Units		DB	?	; units we now
I_FTbl		DW	?	; Current Handle for each Unit
I_CS		DW	?	; our Code Segment
I_Init		DB	?	; Driver has been used if 1
Ident   ENDS
;
; Identy, returned in I_Marker
;
JD	equ	256*'D'+'J'	; Identy marker
;
; This Structure is used to link or unlink a file:
;
ILink	STRUC
IL_Command	db	?	; Command on call
IL_Status	db	?	; Return Status
IL_Flag		db	?	; flags
IL_Handle	dw	?	; handle
;
IL_JBuffer	dd	?	; Janus Buffer to use
IL_JParam	dd	?	; Janus Parm Block to use
;
; This is always the last area:
;
IL_Name		db	128 DUP (?) ; filename
ILink	ENDS
;
; Commands given in IL_Command
;
IL_Link		equ	0	; Link Command
IL_UnLink	equ	1	; Unlink
;
; IL_Flags:
; 
ILF_RO		equ	1 shl 0	; Read Only
;
; IL Error, returned in IL_Status
;
ILE_OK		equ	0	; nothing wrong
ILE_COMERR	equ	1	; unknown command
ILE_LINKED	equ	2	; drive is linked
ILE_CLOSE	equ	3	; error while closing vdrive
ILE_NOT_LINKED  equ	4	; Nothing linked
;
;
	
