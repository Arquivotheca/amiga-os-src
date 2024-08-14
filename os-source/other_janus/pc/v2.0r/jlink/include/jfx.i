;
; Disk request structure for higher level amiga file request
; from 8086
;
AmigaDskReq	STRUC
adr_Fnctn	DW	?	; function code (see below)
adr_File	DW	?	; file number
adr_Offset_h	DW	?	; byte offset into file high
adr_Offset_l	DW	?	; byte offset into file low
adr_Count_h	DW	?	; number of bytes to transfer high
adr_Count_l	DW	?	; number of bytes to transfer low
adr_BufferAddr	DW	?	; offset into MEMF_BUFFER memory for buffer
adr_Err		DW	?	; return code, 0 if all OK
AmigaDskReq	ENDS
;
; function codes for AmigaDskReq adr_Fnctn word
;
ADR_FNCTN_INIT		EQU	0	; currently not used
ADR_FNCTN_READ		EQU	1	; given file, offset, count, buffer
ADR_FNCTN_WRITE		EQU	2	; given file, offset, count, buffer
ADR_FNCTN_SEEK		EQU	3	; given file, offset
ADR_FNCTN_INFO		EQU	4	; currently not used
ADR_FNCTN_OPEN_OLD	EQU	5	; given ASCIIZ pathname in buffer
ADR_FNCTN_OPEN_NEW	EQU	6	; given ASCIIZ pathname in buffer
ADR_FNCTN_CLOSE		EQU	7	; given file
ADR_FNCTN_DELETE	EQU	8	; given ASCIIZ pathname in buffer
;
; error codes for adr_Err, returned in low byte
;
ADR_ERR_OK		EQU	0	; no error
ADR_ERR_OFFSET		EQU	1	; not used
ADR_ERR_COUNT		EQU	2	; not used
ADR_ERR_FILE		EQU	3	; file does not exist
ADR_ERR_FNCT		EQU	4	; illegal function code
ADR_ERR_EOF		EQU	5	; offset past end of file
ADR_ERR_MULPL		EQU	6	; not used
ADR_ERR_FILE_COUNT	EQU	7	; too many open files 
ADR_ERR_SEEK		EQU	8	; seek error
ADR_ERR_READ		EQU     9	; read went wrong
ADR_ERR_WRITE		EQU    10	; write error
ADR_ERR_LOCKED		EQU    11	; file is locked
;


