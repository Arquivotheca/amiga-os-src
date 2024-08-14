;************************************************************************
;* (PC side file)
;*
;* PCDisk.inc - PCDisk specific data structures
;*
;************************************************************************


PCDISK_BUFFER_SIZE   =   8704

; Disk request structure for higher level Amiga file request from 8086

AmigaDiskReq   struc

   adr_Fnctn        dw 0      ; Function code (see below)
   adr_File         dw 0      ; File number (filehandle for PC side)
   adr_Offset_h     dw 0      ; High byte of offset into file
   adr_Offset_l     dw 0      ; Low  byte of offset into file
   adr_Count_h      dw 0      ; High byte of # of bytes to transfer
                              ; Actual number transfered on return
   adr_Count_l      dw 0      ; Low byte as above
   adr_BufferOffset dw 0      ; Offset into MEMF_BUFFER memory
   adr_Err          dw 0      ; Return code (see below) 0 if all OK

AmigaDiskReq   ends

; Function codes for AmigaDiskReq adr_Fnctn word

ADR_FNCTN_INIT     = 0     ; Currently not used
ADR_FNCTN_READ     = 1     ; Given file, offset, count, buffer
ADR_FNCTN_WRITE    = 2     ; Given file, offset, count, buffer
ADR_FNCTN_SEEK     = 3     ; Given file, offset
ADR_FNCTN_INFO     = 4     ; Currently not used
ADR_FNCTN_OPEN_OLD = 5     ; Given ASCIIZ filespec in buffer
ADR_FNCTN_OPEN_NEW = 6     ; Given ASCIIZ filespec in buffer
ADR_FNCTN_CLOSE    = 7     ; Given file
ADR_FNCTN_DELETE   = 8     ; Given ASCIIZ filespec in buffer

; Error codes for adr_Err, returned in low byte

ADR_ERR_OK         = 0     ; No error
ADR_ERR_OFFSET     = 1     ; Not used
ADR_ERR_COUNT      = 2     ; Not used
ADR_ERR_FILE       = 3     ; File does not exist
ADR_ERR_FNCTN      = 4     ; Illegal function code
ADR_ERR_EOF        = 5     ; Offset past end of file
ADR_ERR_MULPL      = 6     ; Not used
ADR_ERR_FILE_COUNT = 7     ; Too many open files
ADR_ERR_SEEK       = 8     ; Seek error
ADR_ERR_READ       = 9     ; Read went wrong
ADR_ERR_WRITE      = 10    ; Write error
ADR_ERR_LOCKED     = 11    ; File is locked

