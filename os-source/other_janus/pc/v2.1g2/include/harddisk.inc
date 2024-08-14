;*************************************************************************
; (PC side file)
;
; HardDisk.inc -- Structures for using the PC hard disk with JSERV_HARDISK
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************


; disk request structure for raw amiga access to 8086's disk
; goes directly to PC BOIS (via PC int 13 scheduler)

HDskAbsReq struc

    har_FktCode dw 0   ; bios function code (see ibm tech ref)
    har_Count   dw 0   ; sector count
    har_Track   dw 0   ; cylinder #
    har_Sector  dw 0   ; sector #
    har_Drive   dw 0   ; drive
    har_Head    dw 0   ; head
    har_Offset  dw 0   ; offset of buffer in MEMF_BUFFER memory
    har_Status  dw 0   ; return status

HDskAbsReq  ends


; definition of an AMIGA disk partition.  returned by info function

HDskPartition  struc

    hdp_Next     dw 0   ; 8088 ptr to next part.  0 -> end of list
    hdp_BaseCyl  dw 0   ; cyl # where partition starts
    hdp_EndCyl   dw 0   ; last cyclinder # of this partition
    hdp_DrvNum   dw 0   ; DOS drive number (80H, 81H, ...)
    hdp_NumHeads dw 0   ; number of heads for this drive
    hdp_NumSecs  dw 0   ; number of sectors per track for this drive

HDskPartition  ends


; disk request structure for higher level amiga disk request to 8086

HardDskReq  struc

    hdr_Fnctn      dw 0 ; function code (see below)
    hdr_Part       dw 0 ; partition number (0 is first partition)
    hdr_Offset     dd 0 ; byte offset into partition
    hdr_Count      dd 0 ; number of bytes to transfer
    hdr_BufferAddr dw 0 ; offset into MEMF_BUFFER memory for buffer
    hdr_Err        dw 0 ; return code, 0 if all OK

HardDskReq  ends


; function codes for HardDskReq hdr_Fnctn word

HDR_FNCTN_INIT       =   0  ; given nothings, sets adr_Part to # partitions
HDR_FNCTN_READ       =   1  ; given partition, offset, count, buffer
HDR_FNCTN_WRITE      =   2  ; given partition, offset, count, buffer
HDR_FNCTN_SEEK       =   3  ; given partition, offset
HDR_FNCTN_INFO       =   4  ; given part, buff adr, cnt, copys in a
                            ;   DskPartition structure. cnt set to actual
                            ;   number of bytes copied.


; error codes for hdr_Err, returned in low byte

HDR_ERR_OK           =   0  ; no error
HDR_ERR_OFFSET       =   1  ; offset not on sector boundary
HDR_ERR_COUNT        =   2  ; dsk_count not a multiple of sector size
HDR_ERR_PART         =   3  ; partition does not exist
HDR_ERR_FNCT         =   4  ; illegal function code
HDR_ERR_EOF          =   5  ; offset past end of partition
HDR_ERR_MULPL        =   6  ; multiple calls while pending service
HDR_ERR_FILE_COUNT   =   7  ; too many open files

; error condition from IBM-PC BIOS, returned in high byte

HDR_ERR_SENSE_FAIL      =   0ffh
HDR_ERR_UNDEF_ERR       =   0bbh
HDR_ERR_TIME_OUT        =   080h
HDR_ERR_BAD_SEEK        =   040h
HDR_ERR_BAD_CNTRLR      =   020h
HDR_ERR_DATA_CORRECTED  =   011h   ; data corrected
HDR_ERR_BAD_ECC         =   010h
HDR_ERR_BAD_TRACK       =   00bh
HDR_ERR_DMA_BOUNDARY    =   009h
HDR_ERR_INIT_FAIL       =   007h
HDR_ERR_BAD_RESET       =   005h
HDR_ERR_RECRD_NOT_FOUND =   004h
HDR_ERR_BAD_ADDR_MARK   =   002h
HDR_ERR_BAD_CMD         =   001h


