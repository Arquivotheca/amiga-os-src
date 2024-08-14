   IFND  JANUS_PCDISK_I
JANUS_PCDISK_I SET   1

;************************************************************************
;* (Amiga side file)
;*
;* PCDisk.i - PCDisk specific data structures
;*
;* 7-15-88 - Bill Koester - Modified for self inclusion of required files
;************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

PCDISK_BUFFER_SIZE   EQU   8704

; Disk request structure for higher level Amiga file request from 8086

 STRUCTURE AmigaDiskReq,0

   UWORD adr_Fnctn              ; Function code (see below)
   UWORD adr_File               ; File number (filehandle for PC side)
   UWORD adr_Offset_h           ; High byte of offset into file
   UWORD adr_Offset_l           ; Low  byte of offset into file
   UWORD adr_Count_h            ; High byte of # of bytes to transfer
                                ; Actual number transfered on return
   UWORD adr_Count_l            ; Low byte as above
   UWORD adr_BufferOffset       ; Offset into MEMF_BUFFER memory
   UWORD adr_Err                ; Return code (see below) 0 if all OK

   LABEL AmigaDiskReq_SIZEOF

; Function codes for AmigaDiskReq adr_Fnctn word

ADR_FNCTN_INIT     EQU 0     ; Currently not used
ADR_FNCTN_READ     EQU 1     ; Given file, offset, count, buffer
ADR_FNCTN_WRITE    EQU 2     ; Given file, offset, count, buffer
ADR_FNCTN_SEEK     EQU 3     ; Given file, offset
ADR_FNCTN_INFO     EQU 4     ; Currently not used
ADR_FNCTN_OPEN_OLD EQU 5     ; Given ASCIIZ filespec in buffer
ADR_FNCTN_OPEN_NEW EQU 6     ; Given ASCIIZ filespec in buffer
ADR_FNCTN_CLOSE    EQU 7     ; Given file
ADR_FNCTN_DELETE   EQU 8     ; Given ASCIIZ filespec in buffer

; Error codes for adr_Err, returned in low byte

ADR_ERR_OK         EQU 0     ; No error
ADR_ERR_OFFSET     EQU 1     ; Not used
ADR_ERR_COUNT      EQU 2     ; Not used
ADR_ERR_FILE       EQU 3     ; File does not exist
ADR_ERR_FNCTN      EQU 4     ; Illegal function code
ADR_ERR_EOF        EQU 5     ; Offset past end of file
ADR_ERR_MULPL      EQU 6     ; Not used
ADR_ERR_FILE_COUNT EQU 7     ; Too many open files
ADR_ERR_SEEK       EQU 8     ; Seek error
ADR_ERR_READ       EQU 9     ; Read went wrong
ADR_ERR_WRITE      EQU 10    ; Write error
ADR_ERR_LOCKED     EQU 11    ; File is locked

   ENDC  ; End of JANUS_PCDISK_I conditional assembly
