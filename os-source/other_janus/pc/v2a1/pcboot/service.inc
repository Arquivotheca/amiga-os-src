;*****************************************************************************
;
; service.inc contains equals, flag bits for service1+2 
;
;******************************************************************************
.xlist
;--- Assembly conditions ------------------------------------------------------
;
;
CHANNELING	  	=	0
;
; Service 7,8,9 :
DBG_WaitStatus	  	= 	0
;
; Service 10,11 :
DBG_MemChunk	  	=	0		; Memchunk found
DBG_ServiceSearch 	=	0
DBG_MakeService	  	=	0
DBG_AllocParam	  	=  	0
DBG_InitPointers  	=	0
DBG_ServiceAdded  	=	0
;
; Service 12 :
DBG_CallAmigaService 	=  	0
DBG_UCountMsg	  	=	0
DBG_RemoveMsg	  	=	0
DBG_ServiceDeleted 	=    	0 
DBG_AmigaAddMsg	  	=	0
DBG_AmigaCallMsg    	=	0
DBG_AmigaDeleteMsg 	=	0
;
; Service 13,14 :
DBG_GotServiceInt 	=	0
DBG_ChannelMsg	  	=	0
DBG_HandlerMsg    	=	0
DBG_NextSlotMsg	  	=	0
DBG_ExitSDMsg	  	=	0
;			
;
;--- JANUS_SERVICES --------------------
;
; Hard coded services,
; more services dynamically added via service 14 and 15.
;
; These services are constrained by hardware:
;
JSERV_MINT		=	 0	; write acces to monochrome display
JSERV_GINT		=	 1	; write acces to color display
JSERV_CRT1INT		=	 2	; mono control register changed
JSERV_CRT2INT		=	 3	; color control register changed
JSERV_ENBKB		=	 4	; keyboard ready for next char
JSERV_LPT1INT		=	 5	; parallel port control register
JSERV_COM2INT		=	 6	; serial port control register
;
; Following services are implemented in software:
;
JSERV_PCBOOTED		=	 7	; PC is ready to serve janus ints
JSERV_SCROLL		=	 8	; PC is scrolling
JSERV_HARDDISK		=	 9	; Amiga is reading PC hard disk
JSERV_READAMIGA 	=	10	; PC is reading Amiga memory
JSERV_READPC		=	11	; Amiga is reading PC memory
JSERV_AMIGACALL 	=	12	; PC causing Amiga function call
JSERV_PCCALL		=	13	; Amiga causing PC interrupt
JSERV_AMIGASERV 	=	14	; PC is calling extended Amiga service
JSERV_PCSERV		=	15	; Amiga is calling extended PC service
;
;
;--- JANUS PC Function calls -----------
;
; Known function codes:	(1.Generation)
;
J_GET_SERVICE		=	 0
J_GET_BASE		=	 1
J_ALLOC_MEM		=	 2
J_FREE_MEM		=	 3
J_SET_PARAM		=	 4
J_SET_SERVICE		=	 5
J_STOP_SERVICE		=	 6
J_CALL_AMIGA		=	 7
J_WAIT_AMIGA		=	 8 
J_CHECK_AMIGA		=	 9
;
; 2.Generation:
;
J_ADD_SERVICE		=	10
J_GET_SERVICE		=	11
J_CALL_SERVICE		=	12
J_RELEASE_SERVICE	=	13
J_DELETE_SERVICE	=	14
;
; Status returns:
;
J_NO_SERVICE		=	0ffh	; service not implemented
J_OK			=	0	; general good return
J_GOODBYE		=	0	; memory freed successfully
J_PENDING		=	0
J_FINISHED		=	1
J_NO_MEMORY		=	3	; requested memory not available
J_ILL_FNCTN		=	4	; illegal function call
J_NO_SERVICE_ID		=	5	; no according serviceID found
J_NO_CHANNEL		=	6 	; no free communication channel available
J_NO_BASE		=	7	; undefined ServiceBase structure
;
; Memory types:
;
NULL_TYPE		=	0	; undefined memory (not supported yet)
PARA_TYPE		=	1	; parameter memory
BUFF_TYPE		=	2	; buffer memory 
;
ACCESS_TYPE		=	0	; byte access (fixed)
;
; Flag definitions:
;
SERVICE_DELETED		=	1
EXPUNGE_SERVICE 	=	2
SERVICE_AMIGA		=	4
AMIGA_PTR		=	100h
PC_PTR			=	200h
; 
; Misc:
;
WAIT_FOR_SERVICE 	=	8	; set to wait for service come avail.	
WAIT_FREE_LOOP		=	100	; loop count for waiting on a free
					; slot for passing a channel number 
;
if	CHANNELING	
;
; Channel number definitions:
;
 NO_CHANNEL		=	0
 ADD_SERVICE		=	1
 DELETE_SERVICE		=	2
 FIRST_USER_CHANNEL  	=      16
endif
;
;
; Serviceses structures
;
ServiceParam		struc

ServiceLock		db	0	; lock this byte before walking 
ReservePad1		db	0	;  through ServiceData

if	CHANNELING	
 ChannelMasks		db   32 dup (0)	; This field contains a cleared bit
					;  for each free channel
 PCToAmiga		db   4 dup (0)	; channel information area set by PC
 AmigaToPC		db   4 dup (0) 	; channel information area set by Amiga
 PCAddService		db	0	; transfer added channel number here 
 AmigaAddService	db	0	; transfer added channel number here 			
else
 PCToAmiga		dw   4 dup (0)	; channel information area set by PC
 AmigaToPC		dw   4 dup (0)	; channel information area set by Amiga
 PCAddService		dw   2 dup (0) 	; transfer added channel number here 
 PCDeleteService	dw   2 dup (0) 	; transfer deleted channel number here
 AmigaAddService	dw   2 dup (0)	; transfer added channel number here 			
 AmigaDeleteService	dw   2 dup (0) 	; transfer deleted channel number here
endif

FirstServiceData 	dw	0	; points to 1. ServiceData structure
					; -1: no valid structure available
CyclopsReserved		dd   4 dup (0)	; Amiga	

ServiceParam		ends 
;
;
;
ServiceData		struc

ApplicationID		dd	0  	; Commodore-Amiga will handle these IDs		
ServiceID		dw	0	; customer defined local ID

Flags			dw	0	; as defined above

if	CHANNELING	
 Channel		db	0	; communication channel for this service
else
 CustomerLock		db	0 	; customer can lock structure while
endif					;  accessing janus memory (default=unlocked)
		 	
UserCount		db	0	; 0: not available

ParamSize		dw	0	; memory size of shared janus memory
ParamType		dw	0	; memory type of shared janus memory
MemOffset		dw	0	; offset in Janus memory
A68000_Ptr		dd	0	; Amiga pointer to shared janus memory
PC8088_Ptr		dd	0	; PC pointer to shared janus memory
					; both pointers point to the same location
RememberKey		dw	0	; pointer to autofree memory
NextServiceData 	dw	0	; points to next ServiceData structure
					; -1: we are the last structure

PC_Customer		dd	0	; points to handler for a PC service
FirstCustomer		dd	0	; points to Amiga customers of this service

ZaphodReserved  	dd   4 dup (0) 

ServiceData		ends


.list
