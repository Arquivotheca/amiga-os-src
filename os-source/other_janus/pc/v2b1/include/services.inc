;* *** services.inc ********************************************************
;* (PC side file)
;*
;* services.inc -- Service Definitions and Data Structures
;*
;* Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc., All rights reserved
;* 
;* HISTORY
;* Date       name               Description
;* ---------  -----------------  -------------------------------------------
;* early 86 - Burns/Katin clone - Created this file
;* 02-22-88 - RJ Mical          - Added service data structures
;* 07-25-88 - Bill Koester      - Added ServiceCustomer structure
;* 07-26-88 - Bill Koester      - Added sd_PCUserCount to ServiceData
;*                                Changed sd_UserCount to sd_AmigaUserCount
;*                                Added sd_ReservedByte to ServiceData
;* *************************************************************************


;* As a coding convenience, we assume a maximum of 32 handlers.
;* People should avoid using this in their code, because we want
;* to be able to relax this constraint in the future.  All the
;* standard commands' syntactically support any number of interrupts,
;* but the internals are limited to 32.


MAXHANDLER   =   32


;*
;* this is the table of hard coded services.  Other services may exist
;* that are dynamically allocated.
;*


;* service numbers constrained by hardware 

JSERV_MINT           =      0  ; monochrome display written to
JSERV_GINT           =      1  ; color display written to
JSERV_CRT1INT        =      2  ; mono display's control registers changed
JSERV_CRT2INT        =      3  ; color display's control registers changed
JSERV_ENBKB          =      4  ; keyboard ready for next character
JSERV_LPT1INT        =      5  ; parallel control register
JSERV_COM2INT        =      6  ; serial control register

;* hard coded service numbers 

JSERV_PCBOOTED       =      7  ; PC is ready to service soft interrupts
JSERV_SCROLL         =      8  ; PC is scrolling its screen
JSERV_HARDDISK       =      9  ; Amiga reading PC hard disk
JSERV_READAMIGA      =      10 ; PC reading Amiga mem
JSERV_READPC         =      11 ; Amiga reading PC mem
JSERV_AMIGACALL      =      12 ; PC causing Amiga function call
JSERV_PCCALL         =      13 ; Amiga causing PC interrupt
JSERV_AMIGASERVICE   =      14 ; PC initiating Amiga side of a service
JSERV_PCSERVICE      =      15 ; Amiga initiating PC side of a service
JSERV_PCDISK         =      16 ; PC using AmigaDos files
JSERV_AMOUSE         =      17 ; AMouse Communications


;--- JANUS PC Function calls -----------
;
; This is the table of function codes. These functions allow controlling
; of dynamically allocated services (dyn-service).
; 
; 1.Generation:	(befor Mai'88)
;
JFUNC_GETSERVICE1	=	 0	; not supported any more    	
JFUNC_GETBASE		=	 1	; report segments, offset of janus mem
JFUNC_ALLOCMEM		=	 2	; allocate janus memory
JFUNC_FREEMEM		=	 3	; free janus memory
JFUNC_SETPARAM		=	 4	; set pointer to service parameter
JFUNC_SETSERVICE	=	 5	; not supported any more    	
JFUNC_STOPSERVICE	=	 6	; not supported any more    	
JFUNC_CALLAMIGA		=	 7	; call service on Amiga side
JFUNC_WAITAMIGA		=	 8 	; wait for service becomes ready
JFUNC_CHECKAMIGA	=	 9	; check service status
;					
; 2.Generation:
;
JFUNC_ADDSERVICE	=	10	; add a dyn-service 
JFUNC_GETSERVICE	=	11	; link to a dyn-service
JFUNC_CALLSERVICE	=	12	; call a dyn-service
JFUNC_RELEASESERVICE	=	13	; unlink from a dyn-service
JFUNC_DELETESERVICE	=	14	; delete a dyn-service
JFUNC_LOCKSERVICEDATA	=	15	; lock private mem of a dyn-service
JFUNC_UNLOCKSERVICEDATA	=	16	; unlock private mem of a dyn-service

JFUNC_MAX		=	16 	; Last function (for range check only)

JFUNC_JINT		=	0bh

;* === ServiceData Structure =============================================== 
;* The ServiceData structure is used to share data among all callers of 
;* all of the Service routines.  One of these is allocated in janus memory 
;* for each service.  

ServiceData struc

   ; The ServiceData ID numbers are used to uniquely identify
   ; application-specific services.  There are two ID numbers:
   ; the global ApplicationID and the application's local LocalID.
   ;
   ; The ApplicationID is a 32-bit number which *must* be assigned to
   ; an application designer by Commodore-Amiga.
   ; Once a service ApplicationID is assigned to an application
   ; designer, that designer "owns" that ID number forever.
   ; Note that this will provide unique ServiceData identification
   ; numbers only for the first 4.3 billion ServiceData designers
   ; after that, there's some risk of a collision.
   ;
   ; The LocalID, defined by the application designer, is a local
   ; subcategory of the global ApplicationID.  These can mean anything
   ; at all.  There are 65,536 of these local ID's.
   sd_ApplicationID  dd 0
   sd_LocalID        dw 0


   ; The flag bits are defined below.  Some of these are set by the
   ; application programs which use the service, and some are set
   ; by the system.
   sd_Flags    dw 0


   ; This field is initialized by the system for you, and then 
   ; is never touched by the system again.  Users of the 
   ; service can agree by convention that they have to obtain 
   ; this lock before using the service.  
   ; If you are the AddService() caller and you want this lock 
   ; to be locked before the service is linked into the system, 
   ; set the AddService() ADDS_LOCKDATA argument flag.  
   sd_ServiceDataLock   db 0


   ; This tracks the number of users currently connected 
   ; to this service.
   sd_AmigaUserCount   db 0
   sd_PCUserCount      db 0
   sd_ReservedByte     db 0

   ; These are the standard janus memory descriptions, which describe
   ; the parameter memory associated with this service.  This memory
   ; (if any) will be allocated automatically by the system when the
   ; service if first added.  The creator of the service
   ; (the one who calls AddService()) supplies the MemSize and
   ; MemType values; after the service is added the MemPtr field
   ; will point to the parameter memory.  GetService() callers, after
   ; the service comes available, will find all of these fields
   ; filled in with the appropriate values.
   ; The AmigaMemPtr and PCMemPtr both point to the same location
   ; of Janus memory; an Amiga program should use the AmigaMemPtr,
   ; and a PC program should use the PCMemPtr
   sd_MemSize        dw 0
   sd_MemType        dw 0
   sd_MemOffset      dw 0
   sd_AmigaMemPtr    dd 0
   sd_PCMemPtr       dd 0


   ; This offset is used as the key for calls to AllocServiceMem() 
   ; and FreeServiceMem().  This key can be used by any one 
   ; who's learned about this service via either AddService() 
   ; or GetService().  The system makes no memory allocations 
   ; using this key, so it's completely under application control.
   ; Any memory attached to this key by calls to AllocServiceMem() 
   ; will be freed automatically after the service has been 
   ; deleted and all users of the service have released the service.  
   sd_JRememberKey   dw 0


   ; These pointers are for the system-maintained lists of
   ; structures.  If you disturb any of these pointers, you will be
   ; tickling the guru's nose, and when the guru sneezes ...
   sd_NextServiceData      dw 0
   sd_FirstPCCustomer      dd 0
   sd_FirstAmigaCustomer   dd 0


   ; These fields are reserved for future use 
   sd_ZaphodReserved    dd 4 dup (0)

ServiceData ends


;* === Flag Definitions === 

SERVICE_DELETED      =   00001h    ; Owner of this service deleted it
SERVICE_DELETEDn     =   0
EXPUNGE_SERVICE      =   00002h    ; Owner of service should delete
EXPUNGE_SERVICEn     =   1
SERVICE_AMIGASIDE    =   00004h    ; Set if Amiga created the service
SERVICE_AMIGASIDEn   =   2


;* === ServiceCustomer Structure =========================================== 
;* A ServiceCustomer structure is created for each "customer" of a given 
;* channel

ServiceCustomer	struc

	scs_NextCustomer 	dd 0

	scs_Flags		dw 0

	scs_Task			dd 0 ;This points to the task of the customer 
	scs_SignalBit		dd 0 ;Signal the customer with this bit 

	scs_JazzReserved  	dd 4 dup (0)

ServiceCustomer	ends

;* === Flag Definitions === *
;* These flags are set/cleared by the system
CALL_TOPC_ONLY       =   0100h
CALL_TOPC_ONLYn      =   8
CALL_FROMPC_ONLY     =   0200h
CALL_FROMPC_ONLYn    =   9
CALL_TOAMIGA_ONLY    =   0400h
CALL_TOAMIGA_ONLYn   =   10
CALL_FROMAMIGA_ONLY  =   0800h
CALL_FROMAMIGA_ONLYn =   11

; === AddService() Flags ==================================================
; These are the definitions of the flag arguments that can be passed to the 
; AddService() function.

ADDS_EXCLUSIVE       = 00001h ; You want to be the *only* Amiga customer
ADDS_EXCLUSIVEn      = 0
ADDS_TOPC_ONLY       = 00002h ; You want to send signals only to the PC
ADDS_TOPC_ONLYn      = 1
ADDS_FROMPC_ONLY     = 00004h ; You want to get signals only from the PC
ADDS_FROMPC_ONLYn    = 2
ADDS_TOAMIGA_ONLY    = 00008h ; You want to send signals only to the Amiga
ADDS_TOAMIGA_ONLYn   = 3
ADDS_FROMAMIGA_ONLY  = 00010h ; You want to get signals only from the Amiga
ADDS_FROMAMIGA_ONLYn = 4
ADDS_LOCKDATA        = 00020h ; S'DataLock locked before linking to system
ADDS_LOCKDATAn       = 5

;* These are the system's AddService() Flags 
SD_CREATED           = 00100h
SD_CREATEDn          = 8



; === GetService() Flags ================================================== 
; These are the definitions of the flag arguments that can be passed to the 
; GetService() function.

GETS_WAIT            = 00001h ; If service not yet available, you'll wait
GETS_WAITn           = 0
GETS_TOPC_ONLY       = 00002h ; You want to send signals only to the PC
GETS_TOPC_ONLYn      = 1
GETS_FROMPC_ONLY     = 00004h ; You want to get signals only from the PC
GETS_FROMPC_ONLYn    = 2
GETS_TOAMIGA_ONLY    = 00008h ; You want to send signals only to the Amiga
GETS_TOAMIGA_ONLYn   = 3
GETS_FROMAMIGA_ONLY  = 00010h ; You want to get signals only from the Amiga
GETS_FROMAMIGA_ONLYn = 4
GETS_EXCLUSIVE       = 00020h ; You want to be the *only* Amiga customer
GETS_EXCLUSIVEn      = 5
GETS_ALOAD_A         = 00040h ; Autoload the service on the Amiga side
GETS_ALOAD_An        = 6
GETS_ALOAD_PC        = 00080h ; Autoload the service on the PC side
GETS_ALOAD_PCn       = 7



; === Service() Result Codes ========================================== 
; These are the result codes that may be returned by a call to a Service 
;
JSERV_NOFUNCTION	= -1 	; Tried to call a not supported function
JSERV_OK         	=  0  	; All is well
JSERV_PENDING	 	=  0  	; Called service still pending on Amiga side
JSERV_FINISHED	 	=  1  	; Called service is finished on Amiga side
JSERV_NOJANUSBASE	=  2  	; ServiceBase structure not defined
JSERV_NOJANUSMEM 	=  3  	; We ran out of Janus memory
JSERV_NOAMIGAMEM 	=  4  	; On the Amiga side we ran out of Amiga memory
JSERV_NOPCMEM    	=  5  	; On the PC side we ran out of PC memory
JSERV_NOSERVICE  	=  6  	; Tried to get a service that doesn't exist
JSERV_DUPSERVICE 	=  7  	; Tried to add a service that already existed
JSERV_ILLFUNCTION	=  8	; Tried to call an illegal function
JSERV_NOTEXCLUSIVE      =  9  ; Wanted to but couldn't be exclusive user 
JSERV_BADAUTOLOAD       =  10 ; Wanted to autoload but couldn't




