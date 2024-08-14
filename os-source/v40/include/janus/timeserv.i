   IFND  JANUS_TIMESERV_I
JANUS_TIMESERV_I  SET   1

;************************************************************************
;* (Amiga side file)
;*
;* TimeServ.i - TimeServ specific data structures
;*
;* 4-19-88 - Bill Koester - Created this file
;* 7-15-88 - Bill Koester - Modified for self inclusion of required files
;************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

TIMESERV_APPLICATION_ID EQU $00000001
TIMESERV_LOCAL_ID       EQU $0001

; Time request structure for Amiga Date/Time request from 8086

 STRUCTURE TimeServReq,0

   UWORD  tsr_Year
   UBYTE  tsr_Month
   UBYTE  tsr_Day
   UBYTE  tsr_Hour
   UBYTE  tsr_Minutes
   UBYTE  tsr_Seconds
   STRUCTURE tsr_String,27
   UBYTE  tsr_Err          ; Return code (see below) 0 if all OK

   LABEL  TimeServReq_SIZEOF

; Error codes for adr_Err, returned in low byte

TSR_ERR_OK         EQU 0     ; No error
TSR_ERR_NOT_SET    EQU 1     ; Time not set on Amiga

   ENDC  ; End of JANUS_TIMESERV_I conditional assembly
