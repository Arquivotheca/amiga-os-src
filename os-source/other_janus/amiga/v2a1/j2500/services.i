
 IFND   JANUS_SERVICES_I   
JANUS_SERVICES_I   EQU   1

* *** services.i ************************************************************
*
* services.i -- Service Definitions and Data Structures
*
* Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc., All rights reserved
* 
* HISTORY
* Date       name               Description
* ---------  -----------------  ---------------------------------------------
* 22-Feb-88  -RJ Mical-         Added service data structures
* early 86   Burns/Katin clone  Created this file
*
* *************************************************************************


*
* this is the table of hard coded services.  Other services may exist
* that are dynamically allocated.
*


* service numbers constrained by hardware 
JSERV_MINT   EQU      0   ; monochrome display written to 
JSERV_GINT   EQU      1   ; color display written to 
JSERV_CRT1INT   EQU      2   ; mono display's control registers changed 
JSERV_CRT2INT   EQU      3   ; color display's control registers changed 
JSERV_ENBKB   EQU      4   ; keyboard ready for next character 
JSERV_LPT1INT   EQU      5   ; parallel control register 
JSERV_COM2INT   EQU      6   ; serial control register 

* hard coded service numbers 
JSERV_PCBOOTED   EQU      7   ; PC is ready to service soft interrupts 

JSERV_SCROLL   EQU      8   ; PC is scrolling its screen 
JSERV_HARDDISK   EQU      9   ; Amiga reading PC hard disk 
JSERV_READAMIGA   EQU      10   ; PC reading Amiga mem 
JSERV_READPC   EQU      11   ; Amiga reading PC mem 
JSERV_AMIGACALL   EQU      12   ; PC causing Amiga function call 
JSERV_PCCALL   EQU      13   ; Amiga causing PC interrupt 
JSERV_AMIGASERVICE   EQU   14   ; PC initiating Amiga side of a service 
JSERV_PCSERVICE   EQU      15   ; Amiga initiating PC side of a service 



* === ServiceData Structure =============================================== 
* The ServiceData structure is used to share data among all callers of 
* all of the Service routines.  One of these is allocated in janus memory 
* for each service.  
* STRUCTURE PREFIX sd_

 STRUCTURE ServiceData,0

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
   USHORT    sd_LocalID


   ; The flag bits are defined below.  Some of these are set by the
   ; application programs which use the service, and some are set
   ; by the system.
   USHORT    sd_Flags


   ; This field is initialized by the system for you, and then 
   ; is never touched by the system again.  Users of the 
   ; service can agree by convention that they have to obtain 
   ; this lock before using the service.  
   ; If you are the AddService() caller and you want this lock 
   ; to be locked before the service is linked into the system, 
   ; set the AddService() ADDS_LOCKDATA argument flag.  
   UBYTE   sd_ServiceDataLock


   ; This tracks the number of users currently connected 
   ; to this service.
   UBYTE   sd_UserCount


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
   USHORT    sd_MemSize
   USHORT    sd_MemType
   RPTR   sd_MemOffset
   APTR    sd_AmigaMemPtr
   APTR    sd_PCMemPtr


   ; This offset is used as the key for calls to AllocServiceMem() 
   ; and FreeServiceMem().  This key can be used by any one 
   ; who's learned about this service via either AddService() 
   ; or GetService().  The system makes no memory allocations 
   ; using this key, so it's completely under application control.
   ; Any memory attached to this key by calls to AllocServiceMem() 
   ; will be freed automatically after the service has been 
   ; deleted and all users of the service have released the service.  
   RPTR   sd_JRememberKey


   ; These pointers are for the system-maintained lists of
   ; structures.  If you disturb any of these pointers, you will be
   ; tickling the guru's nose, and when the guru sneezes ...
   RPTR   sd_NextServiceData
   APTR    sd_FirstPCCustomer
   APTR    sd_FirstAmigaCustomer


   ; These fields are reserved for future use 
   STRUCT   sd_ZaphodReserved,4*4

    LABEL   ServiceData_SIZEOF


* === Flag Definitions === 
SERVICE_DELETED      EQU   $0001   ; Owner of this service deleted it 
SERVICE_DELETEDn   EQU   0
EXPUNGE_SERVICE      EQU   $0002   ; Owner of service should delete 
EXPUNGE_SERVICEn   EQU   1
SERVICE_AMIGASIDE   EQU   $0004   ; Set if Amiga created the service 
SERVICE_AMIGASIDEn   EQU   2

AMIGA_MEMPTR      EQU   $0100   ; This is a system flag 
AMIGA_MEMPTRn      EQU   8   ; This is a system flag 
PC_MEMPTR      EQU   $0200   ; This is a system flag 
PC_MEMPTRn      EQU   9   ; This is a system flag 



; === AddService() Flags ==================================================
; These are the definitions of the flag arguments that can be passed to the 
; AddService() function.
ADDS_EXCLUSIVE      EQU $0001 ; You want to be the *only* Amiga customer
ADDS_EXCLUSIVEn      EQU 0
ADDS_TOPC_ONLY      EQU $0002 ; You want to send signals only to the PC
ADDS_TOPC_ONLYn      EQU 1
ADDS_FROMPC_ONLY   EQU $0004 ; You want to get signals only from the PC
ADDS_FROMPC_ONLYn   EQU 2
ADDS_TOAMIGA_ONLY   EQU $0008 ; You want to send signals only to the Amiga
ADDS_TOAMIGA_ONLYn   EQU 3
ADDS_FROMAMIGA_ONLY   EQU $0010 ; You want to get signals only from the Amiga 
ADDS_FROMAMIGA_ONLYn   EQU 4
ADDS_LOCKDATA      EQU $0020 ; S'DataLock locked before linking to system
ADDS_LOCKDATAn      EQU 5

* These are the system's AddService() Flags 
SD_CREATED      EQU $0100
SD_CREATEDn      EQU 8



; === GetService() Flags ================================================== 
; These are the definitions of the flag arguments that can be passed to the 
; GetService() function.
GETS_WAIT      EQU $0001 ; If service not yet available, you'll wait 
GETS_WAITn      EQU 0
GETS_TOPC_ONLY      EQU $0002 ; You want to send signals only to the PC
GETS_TOPC_ONLYn      EQU 1
GETS_FROMPC_ONLY   EQU $0004 ; You want to get signals only from the PC
GETS_FROMPC_ONLYn   EQU 2
GETS_TOAMIGA_ONLY   EQU $0008 ; You want to send signals only to the Amiga
GETS_TOAMIGA_ONLYn   EQU 3
GETS_FROMAMIGA_ONLY   EQU $0010 ; You want to get signals only from the Amiga 
GETS_FROMAMIGA_ONLYn   EQU 4
GETS_EXCLUSIVE      EQU $0020 ; You want to be the *only* Amiga customer 
GETS_EXCLUSIVEn      EQU 5
GETS_ALOAD_A            EQU $0040 ; Autoload the service on the Amiga side 
GETS_ALOAD_An           EQU 6
GETS_ALOAD_PC           EQU $0080 ; Autoload the service on the PC side 
GETS_ALOAD_PCn          EQU 7



; === AddService() and GetService() Result Codes ========================== 
; These are the result codes that may be returned by a call to AddService() 
; or GetService()
JSERV_OK         EQU 0  ; All is well 
JSERV_NOJANUSMEM EQU 1  ; We ran out of Janus memory 
JSERV_NOAMIGAMEM EQU 2  ; On the Amiga side we ran out of Amiga memory 
JSERV_NOPCMEM    EQU 3  ; On the PC side we ran out of PC memory 
JSERV_NOSERVICE  EQU 4  ; Tried to get a service that doesn't exist 
JSERV_DUPSERVICE EQU 5  ; Tried to add a service that already existed 



 ENDC   !JANUS_SERVICES_I


