;************************************************************************
;* (PC side file)
;*
;* LockServ.inc - LockServ specific data structures
;*
;* 10-09-90 - Bill Koester - Created this file
;************************************************************************


LOCKSERV_APPLICATION_ID 	= 000000001h
LOCKSERV_APPLICATION_ID_H	= 00000h
LOCKSERV_APPLICATION_ID_L	= 00001h
LOCKSERV_LOCAL_ID       	= 00002h

; Lock request structure 

LockServReq struc

   lsr_b1	db 0
   lsr_b2       db 0

LockServReq ends
