;************************************************************************
;* (PC side file)
;*
;* TimeServ.inc - TimeServ specific data structures
;*
;* 4-19-88 - Bill Koester - Created this file
;************************************************************************


TIMESERV_APPLICATION_ID = 000000001h
TIMESERV_APPLICATION_ID_H	=	00000h
TIMESERV_APPLICATION_ID_L	=	00001h
TIMESERV_LOCAL_ID       = 00001h

; Time request structure for Amiga Date/Time request from 8086

TimeServReq struc

   tsr_Year      dw 0
   tsr_Month     db 0
   tsr_Day       db 0
   tsr_Hour      db 0
   tsr_Minutes   db 0
   tsr_Seconds   db 0
   tsr_String    db 27 dup (0)
   tsr_Err       db 0         ; Return code (see below) 0 if all OK

TimeServReq ends

; Error codes for adr_Err, returned in low byte

TSR_ERR_OK         = 0     ; No error
TSR_ERR_NOT_SET    = 1     ; Time not set on Amiga

