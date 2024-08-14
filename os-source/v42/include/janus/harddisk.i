   IFND  JANUS_HARDDISK_I
JANUS_HARDDISK_I  SET   1

;*************************************************************************
; (Amiga side file)
;
; HardDisk.i -- Structures for using the PC hard disk with JSERV_HARDISK
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
; 7-15-88 - Bill Koester - Modified for self inclusion of required files
;*************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

; disk request structure for raw amiga access to 8086's disk
; goes directly to PC BOIS (via PC int 13 scheduler)

 STRUCTURE  HDskAbsReq,0

   UWORD    har_FktCode    ; bios function code (see ibm tech ref)
   UWORD    har_Count      ; sector count
   UWORD    har_Track      ; cylinder #
   UWORD    har_Sector     ; sector #
   UWORD    har_Drive      ; drive
   UWORD    har_Head       ; head
   UWORD    har_Offset     ; offset of buffer in MEMF_BUFFER memory
   UWORD    har_Status     ; return status

    LABEL   HDskAbsReq_SIZEOF


; definition of an AMIGA disk partition.  returned by info function

 STRUCTURE  HDskPartition,0

   UWORD    hdp_Next        ; 8088 ptr to next part.  0 -> end of list
   UWORD    hdp_BaseCyl     ; cyl # where partition starts
   UWORD    hdp_EndCyl      ; last cyclinder # of this partition
   UWORD    hdp_DrvNum      ; DOS drive number (80H, 81H, ...)
   UWORD    hdp_NumHeads    ; number of heads for this drive
   UWORD    hdp_NumSecs     ; number of sectors per track for this drive

   LABEL    HDskPartition_SIZEOF


; disk request structure for higher level amiga disk request to 8086

 STRUCTURE  HardDskReq,0

   UWORD    hdr_Fnctn      ; function code (see below)
   UWORD    hdr_Part       ; partition number (0 is first partition)
   ULONG    hdr_Offset     ; byte offset into partition
   ULONG    hdr_Count      ; number of bytes to transfer
   UWORD    hdr_BufferAddr ; offset into MEMF_BUFFER memory for buffer
   UWORD    hdr_Err        ; return code, 0 if all OK

   LABEL   HardDskReq_SIZEOF


; function codes for HardDskReq hdr_Fnctn word

HDR_FNCTN_INIT       EQU   0  ; given nothings, sets adr_Part to # partitions
HDR_FNCTN_READ       EQU   1  ; given partition, offset, count, buffer
HDR_FNCTN_WRITE      EQU   2  ; given partition, offset, count, buffer
HDR_FNCTN_SEEK       EQU   3  ; given partition, offset
HDR_FNCTN_INFO       EQU   4  ; given part, buff adr, cnt, copys in a
                              ;   DskPartition structure. cnt set to actual
                              ;   number of bytes copied.


; error codes for hdr_Err, returned in low byte

HDR_ERR_OK           EQU   0  ; no error
HDR_ERR_OFFSET       EQU   1  ; offset not on sector boundary
HDR_ERR_COUNT        EQU   2  ; dsk_count not a multiple of sector size
HDR_ERR_PART         EQU   3  ; partition does not exist
HDR_ERR_FNCT         EQU   4  ; illegal function code
HDR_ERR_EOF          EQU   5  ; offset past end of partition
HDR_ERR_MULPL        EQU   6  ; multiple calls while pending service
HDR_ERR_FILE_COUNT   EQU   7  ; too many open files

; error condition from IBM-PC BIOS, returned in high byte

HDR_ERR_SENSE_FAIL      EQU   $ff
HDR_ERR_UNDEF_ERR       EQU   $bb
HDR_ERR_TIME_OUT        EQU   $80
HDR_ERR_BAD_SEEK        EQU   $40
HDR_ERR_BAD_CNTRLR      EQU   $20
HDR_ERR_DATA_CORRECTED  EQU   $11   ; data corrected
HDR_ERR_BAD_ECC         EQU   $10
HDR_ERR_BAD_TRACK       EQU   $0b
HDR_ERR_DMA_BOUNDARY    EQU   $09
HDR_ERR_INIT_FAIL       EQU   $07
HDR_ERR_BAD_RESET       EQU   $05
HDR_ERR_RECRD_NOT_FOUND EQU   $04
HDR_ERR_BAD_ADDR_MARK   EQU   $02
HDR_ERR_BAD_CMD         EQU   $01


   ENDC  ;End of JANUS_HARDDISK_I conditional assembly
