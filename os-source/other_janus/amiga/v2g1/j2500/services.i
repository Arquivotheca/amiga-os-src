   IFND  JANUS_SERVICES_I
JANUS_SERVICES_I  SET   1

* *** services.i ************************************************************
* (Amiga side file)
*
* services.i -- Service Definitions and Data Structures
*
* Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc., All rights reserved
* 
* HISTORY
* Date       name                Description
* --------   -----------------   ------------------------------------------
* early 86 - Burns/Katin clone - Created this file
* 02-22-88 - RJ Mical          - Added service data structures
* 07-15-88 - Bill Koester      - Mod for self inclusion of required files
* 07-25-88 - Bill Koester      - Added ServiceCustomer structure
* 07-26-88 - Bill Koester      - Added sd_PCUserCount to ServiceData
*                                Changed sd_UserCount to sd_AmigaUserCount
*                                Added sd_ReservedByte to ServiceData
* 10-05-88 - Bill Koester      - Added SERVICE_PCWAIT flag definitions
* 10-09-88 - Bill Koester      - Added PC/AMIGA_EXCLUSVIE & SERVICE_ADDED
*                                flag definitions.
*                              - Added sd_Semaphore field to ServiceData
* 11-08-88 - Bill Koester      - Added AUTOWAIT flags
* *************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE "exec/types.i"
   ENDC

* As a coding convenience, we assume a maximum of 32 handlers.
* People should avoid using this in their code, because we want
* to be able to relax this constraint in the future.  All the
* standard commands' syntactically support any number of interrupts,
* but the internals are limited to 32.


MAXHANDLER   EQU   32      


*
* this is the table of hard coded services.  Other services may exist
* that are dynamically allocated.
*


* service numbers constrained by hardware 

JSERV_MINT           EQU      0  ; monochrome display written to
JSERV_GINT           EQU      1  ; color display written to
JSERV_CRT1INT        EQU      2  ; mono display's control registers changed
JSERV_CRT2INT        EQU      3  ; color display's control registers changed
JSERV_ENBKB          EQU      4  ; keyboard ready for next character
JSERV_LPT1INT        EQU      5  ; parallel control register
JSERV_COM2INT        EQU      6  ; serial control register

* hard coded service numbers 
JSERV_PCBOOTED       EQU      7  ; PC is ready to service soft interrupts
JSERV_SCROLL         EQU      8  ; PC is scrolling its screen
JSERV_HARDDISK       EQU      9  ; Amiga reading PC hard disk
JSERV_READAMIGA      EQU      10 ; PC reading Amiga mem
JSERV_READPC         EQU      11 ; Amiga reading PC mem
JSERV_AMIGACALL      EQU      12 ; PC causing Amiga function call
JSERV_PCCALL         EQU      13 ; Amiga causing PC interrupt
JSERV_AMIGASERVICE   EQU      14 ; PC initiating Amiga side of a service
JSERV_PCSERVICE      EQU      15 ; Amiga initiating PC side of a service
JSERV_PCDISK         EQU      16 ; PC using AmigaDos files
JSERV_AMOUSE         EQU      17 ; AMouse Communications
                  
;--- JANUS PC Function calls -----------
;
; This is the table of function codes. These functions allow controlling
; of dynamically allocated services (dyn-service).
; 
; 1.Generation:   (befor Mai'88)
;
JFUNC_GETSERVICE1 EQU 0 ; not supported any more
JFUNC_GETBASE     EQU 1 ; report segments, offset of janus mem
JFUNC_ALLOCMEM    EQU 2 ; allocate janus memory
JFUNC_FREEMEM     EQU 3 ; free janus memory
JFUNC_SETPARAM    EQU 4 ; set pointer to service parameter
JFUNC_SETSERVICE  EQU 5 ; not supported any more
JFUNC_STOPSERVICE EQU 6 ; not supported any more
JFUNC_CALLAMIGA   EQU 7 ; call service on Amiga side
JFUNC_WAITAMIGA   EQU 8 ; wait for service becomes ready
JFUNC_CHECKAMIGA  EQU 9 ; check service status
;               
; 2.Generation:
;
JFUNC_ADDSERVICE        EQU 10 ; add a dyn-service
JFUNC_GETSERVICE        EQU 11 ; link to a dyn-service
JFUNC_CALLSERVICE       EQU 12 ; call a dyn-service
JFUNC_RELEASESERVICE    EQU 13 ; unlink from a dyn-service
JFUNC_DELETESERVICE     EQU 14 ; delete a dyn-service
JFUNC_LOCKSERVICEDATA   EQU 15 ; lock private mem of a dyn-service
JFUNC_UNLOCKSERVICEDATA EQU 16 ; unlock private mem of a dyn-service
 
JFUNC_MAX      EQU 16     Last function (for range check only)

JFUNC_JINT   EQU $0b


* === ServiceData Structure =============================================== 
* The ServiceData structure is used to share data among all callers of 
* all of the Service routines.  One of these is allocated in janus memory 
* for each service.  

 STRUCTURE  ServiceData,0

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
   ULONG    sd_ApplicationID
   USHORT   sd_LocalID


   ; The flag bits are defined below.  Some of these are set by the
   ; application programs which use the service, and some are set
   ; by the system.
   USHORT   sd_Flags


   ; This field is initialized by the system for you, and then 
   ; is never touched by the system again.  Users of the 
   ; service can agree by convention that they have to obtain 
   ; this lock before using the service.  
   ; If you are the AddService() caller and you want this lock 
   ; to be locked before the service is linked into the system, 
   ; set the AddService() ADDS_LOCKDATA argument flag.  
   UBYTE    sd_ServiceDataLock


   ; This tracks the number of users currently connected 
   ; to this service.
   UBYTE    sd_AmigaUserCount
   UBYTE    sd_PCUserCount
   UBYTE    sd_ReservedByte


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
   USHORT   sd_MemSize
   USHORT   sd_MemType
   RPTR     sd_MemOffset
   APTR     sd_AmigaMemPtr
   APTR     sd_PCMemPtr


   ; This offset is used as the key for calls to AllocServiceMem() 
   ; and FreeServiceMem().  This key can be used by any one 
   ; who's learned about this service via either AddService() 
   ; or GetService().  The system makes no memory allocations 
   ; using this key, so it's completely under application control.
   ; Any memory attached to this key by calls to AllocServiceMem() 
   ; will be freed automatically after the service has been 
   ; deleted and all users of the service have released the service.  
   RPTR     sd_JRememberKey


   ; These pointers are for the system-maintained lists of
   ; structures.  If you disturb any of these pointers, you will be
   ; tickling the guru's nose, and when the guru sneezes ...
   RPTR     sd_NextServiceData
   APTR     sd_FirstPCCustomer
   APTR     sd_FirstAmigaCustomer

   ;
   ; Semaphore structure pointer for services that allow multiple customers
   ;
   APTR     sd_Semaphore


   ; These fields are reserved for future use 
   STRUCT   sd_ZaphodReserved,4*4

    LABEL   ServiceData_SIZEOF



* === Flag Definitions === 

SERVICE_DELETED      EQU   $0001    ; Owner of this service deleted it 
SERVICE_DELETEDn     EQU   0
EXPUNGE_SERVICE      EQU   $0002    ; Owner of service should delete 
EXPUNGE_SERVICEn     EQU   1
SERVICE_AMIGASIDE    EQU   $0004    ; Set if Amiga created the service 
SERVICE_AMIGASIDEn   EQU   2
SERVICE_PCWAIT       EQU   $0008    ; Set when PC calls a service
SERVICE_PCWAITn      EQU   3        ; Cleared when service replys
AMIGA_EXCLUSIVE      EQU   $0010    ; Only one Amiga customer allowed
AMIGA_EXCLUSIVEn     EQU   4
PC_EXCLUSIVE         EQU   $0020    ; Only one PC customer allowed
PC_EXCLUSIVEn        EQU   5
SERVICE_ADDED        EQU   $0040    ; Set when service is added
SERVICE_ADDEDn       EQU   6



* === ServiceCustomer Structure =========================================== 
* A ServiceCustomer structure is created for each "customer" of a given 
* channel

 STRUCTURE ServiceCustomer,0

   APTR     scs_NextCustomer

   USHORT   scs_Flags

   APTR     scs_Task        ; This points to the task of the customer
   ULONG    scs_SignalBit   ; Signal the customer with this bit

   STRUCT   scs_JazzReserved,4*4

   LABEL    ServiceCustomer_SIZEOF

* === Flag Definitions === *
* These flags are set/cleared by the system
CALL_TOPC_ONLY      EQU   $0100
CALL_TOPC_ONLYn      EQU   8
CALL_FROMPC_ONLY   EQU   $0200
CALL_FROMPC_ONLYn   EQU   9
CALL_TOAMIGA_ONLY   EQU   $0400
CALL_TOAMIGA_ONLYn   EQU   10
CALL_FROMAMIGA_ONLY   EQU   $0800
CALL_FROMAMIGA_ONLYn   EQU   11

; === AddService() Flags ==================================================
; These are the definitions of the flag arguments that can be passed to the 
; AddService() function.

ADDS_EXCLUSIVE       EQU $0001 ; You want to be the *only* Amiga customer
ADDS_EXCLUSIVEn      EQU 0
ADDS_TOPC_ONLY       EQU $0002 ; You want to send signals only to the PC
ADDS_TOPC_ONLYn      EQU 1
ADDS_FROMPC_ONLY     EQU $0004 ; You want to get signals only from the PC
ADDS_FROMPC_ONLYn    EQU 2
ADDS_TOAMIGA_ONLY    EQU $0008 ; You want to send signals only to the Amiga
ADDS_TOAMIGA_ONLYn   EQU 3
ADDS_FROMAMIGA_ONLY  EQU $0010 ; You want to get signals only from the Amiga
ADDS_FROMAMIGA_ONLYn EQU 4
ADDS_LOCKDATA        EQU $0020 ; S'DataLock locked before linking to system
ADDS_LOCKDATAn       EQU 5

* These are the system's AddService() Flags 
SD_CREATED           EQU $0100
SD_CREATEDn          EQU 8



; === GetService() Flags ================================================== 
; These are the definitions of the flag arguments that can be passed to the 
; GetService() function.

GETS_WAIT            EQU $0001 ; If service not yet available, you'll wait 
GETS_WAITn           EQU 0
GETS_TOPC_ONLY       EQU $0002 ; You want to send signals only to the PC
GETS_TOPC_ONLYn      EQU 1
GETS_FROMPC_ONLY     EQU $0004 ; You want to get signals only from the PC
GETS_FROMPC_ONLYn    EQU 2
GETS_TOAMIGA_ONLY    EQU $0008 ; You want to send signals only to the Amiga
GETS_TOAMIGA_ONLYn   EQU 3
GETS_FROMAMIGA_ONLY  EQU $0010 ; You want to get signals only from the Amiga
GETS_FROMAMIGA_ONLYn EQU 4
GETS_EXCLUSIVE       EQU $0020 ; You want to be the *only* Amiga customer 
GETS_EXCLUSIVEn      EQU 5
GETS_ALOAD_A         EQU $0040 ; Autoload the service on the Amiga side
GETS_ALOAD_An        EQU 6
GETS_ALOAD_PC        EQU $0080 ; Autoload the service on the PC side
GETS_ALOAD_PCn       EQU 7
GETS_AUTOWAITmask    EQU $0300
GETS_AUTOWAIT_0      EQU $0000
GETS_AUTOWAIT_15     EQU $0100
GETS_AUTOWAIT_30     EQU $0200
GETS_AUTOWAIT_120    EQU $0300


; === AddService() and GetService() Result Codes ========================== 
; These are the result codes that may be returned by a call to AddService() 
; or GetService()

JSERV_NOFUNCTION   EQU -1 ; Tried to call a not supported function
JSERV_OK           EQU  0 ; All is well
JSERV_PENDING      EQU  0 ; Called service still pending on Amiga side
JSERV_FINISHED     EQU  1 ; Called service is finished on Amiga side
JSERV_NOJANUSBASE  EQU  2 ; ServiceBase structure not defined
JSERV_NOJANUSMEM   EQU  3 ; We ran out of Janus memory
JSERV_NOAMIGAMEM   EQU  4 ; On the Amiga side we ran out of Amiga memory
JSERV_NOPCMEM      EQU  5 ; On the PC side we ran out of PC memory
JSERV_NOSERVICE    EQU  6 ; Tried to get a service that doesn't exist
JSERV_DUPSERVICE   EQU  7 ; Tried to add a service that already existed
JSERV_ILLFUNCTION  EQU  8 ; Tried to call an illegal function
JSERV_NOTEXCLUSIVE EQU  9  ; Wanted to but couldn't be exclusive user
JSERV_BADAUTOLOAD  EQU  10 ; Wanted to autoload but couldn't


   ENDC  ;End of JANUS_SERVICES_I conditional assembly
