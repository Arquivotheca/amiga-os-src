   IFND  JANUS_DOSSERV_I
JANUS_DOSSERV_I  SET   1

;************************************************************************
;* (Amiga side file)
;*
;* DOSServ.h - DOSServ specific data structures
;*
;* 11-19-90 - Bill Koester - Created this file
;************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

DOSSERV_APPLICATION_ID EQU $00000001
DOSSERV_LOCAL_ID       EQU $0003

;*************************
;* DOS request structure *
;*************************

 STRUCTURE DOSServReq,0

   UWORD dsr_Function
   UBYTE dsr_Lock
   UBYTE dsr_Pad
   UWORD dsr_Buffer_Seg
   UWORD dsr_Buffer_Off
   UWORD dsr_Buffer_Size
   UWORD dsr_Err          ;Return code (see below) 0 if all OK 
   UWORD dsr_Arg1_h
   UWORD dsr_Arg1_l
   UWORD dsr_Arg2_h
   UWORD dsr_Arg2_l
   UWORD dsr_Arg3_h
   UWORD dsr_Arg3_l

   LABEL  DOSServReq_SIZEOF

;***********************************
;* Function codes for dsr_Function *
;***********************************

DSR_FUNC_OPEN_OLD		 EQU	1
DSR_FUNC_OPEN_NEW		 EQU	2
DSR_FUNC_OPEN_READ_WRITE EQU	3
DSR_FUNC_CLOSE			 EQU	4
DSR_FUNC_READ			 EQU	5
DSR_FUNC_WRITE			 EQU	6
DSR_FUNC_SEEK_BEGINING	 EQU	7
DSR_FUNC_SEEK_END		 EQU	8
DSR_FUNC_SEEK_CURRENT	 EQU	9
DSR_FUNC_SEEK_EXTEND	 EQU   10	
DSR_FUNC_CREATE_DIR		 EQU   11
DSR_FUNC_LOCK			 EQU   12
DSR_FUNC_UNLOCK			 EQU   13
DSR_FUNC_EXAMINE		 EQU   14

;***************************
;* Error codes for dsr_Err *
;***************************

DSR_ERR_OK         			EQU	0
DSR_ERR_UNKNOWN_FUNCTION 	EQU	1
DSR_ERR_TOO_MANY_FILES 		EQU	2
DSR_ERR_OPEN_ERROR			EQU	3
DSR_ERR_FILE_NOT_OPEN		EQU	4
DSR_ERR_SEEK_ERROR	  	    EQU	5
DSR_ERR_TOO_MANY_LOCKS 		EQU	6

   ENDC  ; End of JANUS_DOSSERV_I conditional assembly
