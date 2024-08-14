;************************************************************
; File: dskstruc.inc
;
; Structures for pc disk access from amiga
;
; All new code: 25-Feb-86     DP
;
;************************************************************
;Absolute Disk acccess - goes directly to PC BIOS (via PC
;			 int 13 scheduler)
;
;************************************************************
dsk_abs_req	    struc
dsk_abs_fkt_code    dw	 0    ; PC bios function code
dsk_abs_count	    dw	 0    ; PC sector count
dsk_abs_track	    dw	 0    ; PC cylinder #
dsk_abs_sector	    dw	 0    ; PC sector #
dsk_abs_drive	    dw	 0    ; PC drive #
dsk_abs_head	    dw	 0    ; PC head # 
dsk_abs_offset	    dw	 0    ; buffer offset in buffer segment
dsk_abs_status	    dw	 0    ; return status
dsk_abs_req	    ends
;**************************************************************
; Definiton of an AMIGA disk partiton
; to be used locally by PC code
;**************************************************************
dsk_partition  struc	      ; defines an amiga disk partition
part_next      dw   0	      ; pointer to next partition. 0 -> no next part.
part_base_cyl  dw   0	      ; cylinder #  where partition starts
part_end_cyl   dw   0	      ; last cylinder 3 of this partition
part_drvnum    dw   0	      ; DOS drive number (80H,81H)
part_numheads  dw   0	      ; number of heads for this drive
part_numsecs   dw   0	      ; sectors per track for this drive
dsk_partition ends
;**************************************************************
; Sidecar HD Partition structure	
;**************************************************************
HDPart0 	struc
BootFlag	db	?
HeadStart	db	?
SecStart	db	?
CylStart	db	?
SysFlag 	db	?
HeadEnd 	db	?
SecEnd		db	?
CylEnd		db	?
SecOffLow	dw	?
SecOffHigh	dw	?
SecNumLow	dw	?
SecNumHigh	dw	?
HDPart0 	ends
;*************************************************************
;
; Standard AMIGA disk I/O request to a partition of a PC disk
;
; Note that double words are accessed by 8088 in reverse order,
; so most significant word comes at lower address. So 68000 may
; use the word area also for long words.
;
;*************************************************************
amiga_dsk_req  struc	      ; amiga disk I/O request
dsk_fnctn      dw   0	      ; function code: see table below 
dsk_part       dw   0	      ; partition number (0 is 1st #)
dsk_offset     dd   0	      ; byte offset for operation
dsk_count      dd   0	      ; byte count
dsk_buffer_adr dw   0	      ; address of BUFFER RAM for operation
dsk_err        dw   0	      ; return status, 0 if everything ok
amiga_dsk_req  ends
;*************************************************************
; Function codes specified in amiga_dsk_req.dsk_function
;************************************************************* 
dsk_fnctn_init	    =	 0    ; given nothing, returns # of partitions
			      ; in dsk_part (0: no partition).
			      ; never returns an error.
dsk_fnctn_read	    =	 1    ; given part,offset,count,buffer
dsk_fnctn_write     =	 2    ; given part,offset,count,buffer
dsk_fnctn_seek	    =	 3    ; given part,offset
dsk_fnktn_info	    =	 4    ; given part, dsk_buffer_adr,dsk_count
			      ; If partition exists, returns a
			      ; dsk_partition structure @ dsk_buffer_adr,
			      ; dsk_count adjusted.
			      ; if partition does not exist,
			      ; returns a dsk_err_part.
;*************************************************************
; Error code definitions in low byte of amiga_dsk_req.dsk_err
;*************************************************************
dsk_err_ok	  = 0		 ; no error found
dsk_err_offset	  = 1		 ; dsk_offset not on sector boundary
dsk_err_count	  = 2		 ; dsk_count not a multiple of sector size
dsk_err_part	  = 3		 ; partition does not exist
dsk_err_fnct	  = 4		 ; illegal function code specified
dsk_err_dsk_full  = 5		 ; illegal offset specified   
dsk_err_multpl	  = 6		 ; mutliple calls while pending service
;*************************************************************
; Error conditions from PC BIOS, returned in high byte
; of amiga_dsk_req.dsk_err;
;*************************************************************
dsk_err_sense_fail     = 0ffh
dsk_err_undef_err      = 0bbh
dsk_err_time_out       = 080h
dsk_err_bad_seek       = 040h
dsk_err_bad_cntrlr     = 020h
dsk_err_data_corrected = 011h
dsk_err_bad_ecc        = 010h
dsk_err_bad_track      = 00bh
dsk_err_dma_boundary   = 009h
dsk_err_init_fail      = 007h
dsk_err_bad_reset      = 005h
dsk_err_recrd_not_fnd  = 004h
dsk_err_bad_addr_mark  = 002h
dsk_err_bad_cmd        = 001h
;
;******************************************************************************
