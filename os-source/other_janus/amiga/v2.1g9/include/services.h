#ifndef JANUS_SERVICES_H
#define JANUS_SERVICES_H

/************************************************************************
 * (Amiga side file)
 *
 * services.h -- Service Definitions and Data Structures
 *
 * Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc., All rights reserved
 * 
 * HISTORY
 * Date       name                Description
 * --------   -----------------   ---------------------------------------------
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
 *                              - Self inclusion of exec/semaphores.h
 * 11-08-88 - Bill Koester      - Added AUTOWAIT flags
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef JANUS_MEMORY_H        /* include for RPTR def from memory.h     */
#include <janus/memory.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

/*
 * As a coding convenience, we assume a maximum of 32 handlers.
 * People should avoid using this in their code, because we want
 * to be able to relax this constraint in the future.  All the
 * standard commands' syntactically support any number of interrupts,
 * but the internals are limited to 32.
 */

#define MAXHANDLER 32

/*
 *
 * this is the table of hard coded services.  Other services may exist
 * that are dynamically allocated.
 *
 */


/* service numbers constrained by hardware                              */

#define JSERV_MINT       0  /* monochrome display written to            */
#define JSERV_GINT       1  /* color display written to                 */
#define JSERV_CRT1INT    2  /* mono display's control registers changed */
#define JSERV_CRT2INT    3  /* color display's control registers changed*/
#define JSERV_ENBKB      4  /* keyboard ready for next character        */
#define JSERV_LPT1INT    5  /* parallel control register                */
#define JSERV_COM2INT    6  /* serial control register                  */

/* hard coded service numbers                                           */

#define JSERV_PCBOOTED      7  /* PC is ready to service soft interrupts*/
#define JSERV_SCROLL        8  /* PC is scrolling its screen            */
#define JSERV_HARDDISK      9  /* Amiga reading PC hard disk            */
#define JSERV_READAMIGA     10 /* PC reading Amiga mem                  */
#define JSERV_READPC        11 /* Amiga reading PC mem                  */
#define JSERV_AMIGACALL     12 /* PC causing Amiga function call        */
#define JSERV_PCCALL        13 /* Amiga causing PC interrupt            */
#define JSERV_AMIGASERVICE  14 /* PC initiating Amiga side of a service */
#define JSERV_PCSERVICE     15 /* Amiga initiating PC side of a service */
#define JSERV_PCDISK        16 /* PC using AmigaDos files               */
#define JSERV_AMOUSE        17 /* AMouse Communications                 */
                  
/*--- JANUS PC Function calls -----------
 *
 * This is the table of function codes. These functions allow controlling
 * of dynamically allocated services (dyn-service).
 *
 * 1.Generation:   (befor Mai'88)
 */
#define JFUNC_GETSERVICE1 0 /* not supported any more        */
#define JFUNC_GETBASE     1 /* report segments, offset of janus mem */
#define JFUNC_ALLOCMEM    2 /* allocate janus memory */
#define JFUNC_FREEMEM     3 /* free janus memory */
#define JFUNC_SETPARAM    4 /* set pointer to service parameter */
#define JFUNC_SETSERVICE  5 /* not supported any more        */
#define JFUNC_STOPSERVICE 6 /* not supported any more        */
#define JFUNC_CALLAMIGA   7 /* call service on Amiga side */
#define JFUNC_WAITAMIGA   8 /* wait for service becomes ready */
#define JFUNC_CHECKAMIGA  9 /* check service status */
/*
 * 2.Generation:
 */
#define JFUNC_ADDSERVICE        10 /* add a dyn-service */
#define JFUNC_GETSERVICE        11 /* link to a dyn-service*/
#define JFUNC_CALLSERVICE       12 /* call a dyn-service*/
#define JFUNC_RELEASESERVICE    13 /* unlink from a dyn-service*/
#define JFUNC_DELETESERVICE     14 /* delete a dyn-service*/
#define JFUNC_LOCKSERVICEDATA   15 /* lock private mem of a dyn-service*/
#define JFUNC_UNLOCKSERVICEDATA 16 /* unlock private mem of a dyn-service*/
#define JFUNC_INITLOCK          17
#define JFUNC_LOCKATTEMPT       18
#define JFUNC_LOCK              19
#define JFUNC_UNLOCK            20
#define JFUNC_ALLOCJREMEMBER    21
#define JFUNC_ATTACHJREMEMBER   22
#define JFUNC_FREEJREMEMBER     23
#define JFUNC_ALLOCSERVICEMEM   24
#define JFUNC_FREESERVICEMEM    25
 
#define JFUNC_MAX      25    /* Last function (for range check only) */

#define JFUNC_JINT   0x0b


/*
 * === ServiceData Structure =============================================== 
 * The ServiceData structure is used to share data among all callers of 
 * all of the Service routines.  One of these is allocated in janus memory 
 * for each service.  
 */

struct ServiceData {

   /*
    * The ServiceData ID numbers are used to uniquely identify
    * application-specific services.  There are two ID numbers:
    * the global ApplicationID and the application's local LocalID.
    *
    * The ApplicationID is a 32-bit number which *must* be assigned to
    * an application designer by Commodore-Amiga.
    * Once a service ApplicationID is assigned to an application
    * designer, that designer "owns" that ID number forever.
    * Note that this will provide unique ServiceData identification
    * numbers only for the first 4.3 billion ServiceData designers
    * after that, there's some risk of a collision.
    *
    * The LocalID, defined by the application designer, is a local
    * subcategory of the global ApplicationID.  These can mean anything
    * at all.  There are 65,536 of these local ID's.
    */
   ULONG    sd_ApplicationID;
   USHORT   sd_LocalID;

   /*
    * The flag bits are defined below.  Some of these are set by the
    * application programs which use the service, and some are set
    * by the system.
    */
   USHORT   sd_Flags;

   /*
    * This field is initialized by the system for you, and then
    * is never touched by the system again.  Users of the
    * service can agree by convention that they have to obtain
    * this lock before using the service.
    * If you are the AddService() caller and you want this lock
    * to be locked before the service is linked into the system,
    * set the AddService() ADDS_LOCKDATA argument flag.
    */
   UBYTE    sd_ServiceDataLock;

   /*
    * This tracks the number of users currently connected
    * to this service.
    */
   UBYTE    sd_AmigaUserCount;
   UBYTE    sd_PCUserCount;
   UBYTE    sd_ReservedByte;

   /*
    * These are the standard janus memory descriptions, which describe
    * the parameter memory associated with this service.  This memory
    * (if any) will be allocated automatically by the system when the
    * service if first added.  The creator of the service
    * (the one who calls AddService()) supplies the MemSize and
    * MemType values * after the service is added the MemPtr field
    * will point to the parameter memory.  GetService() callers, after
    * the service comes available, will find all of these fields
    * filled in with the appropriate values.
    * The AmigaMemPtr and PCMemPtr both point to the same location
    * of Janus memory; an Amiga program should use the AmigaMemPtr,
    * and a PC program should use the PCMemPtr
    */
   USHORT   sd_MemSize;
   USHORT   sd_MemType;
   RPTR     sd_MemOffset;
   APTR     sd_AmigaMemPtr;
   APTR     sd_PCMemPtr;

   /*
    * This offset is used as the key for calls to AllocServiceMem()
    * and FreeServiceMem().  This key can be used by any one
    * who's learned about this service via either AddService()
    * or GetService().  The system makes no memory allocations
    * using this key, so it's completely under application control.
    * Any memory attached to this key by calls to AllocServiceMem()
    * will be freed automatically after the service has been
    * deleted and all users of the service have released the service.
    */
   RPTR     sd_JRememberKey;

   /*
    * These pointers are for the system-maintained lists of
    * structures.  If you disturb any of these pointers, you will be
    * tickling the guru's nose, and when the guru sneezes ...
    */
   RPTR     sd_NextServiceData;
   APTR     sd_FirstPCCustomer;
   APTR     sd_FirstAmigaCustomer;

   /*
    * Semaphore structure pointer for services that allow multiple customers
    */
   struct SignalSemaphore *sd_Semaphore;

   /*
    * These fields are reserved for future use
    */
   ULONG    sd_ZaphodReserved[4];
   };




/*
 * === Flag Definitions === 
 */

#define SERVICE_DELETED    0x0001   /* Owner of this service deleted it */
#define SERVICE_DELETEDn   0
#define EXPUNGE_SERVICE    0x0002   /* Owner of service should delete   */
#define EXPUNGE_SERVICEn   1
#define SERVICE_AMIGASIDE  0x0004   /* Set if Amiga created the service */
#define SERVICE_AMIGASIDEn 2
#define SERVICE_PCWAIT     0x0008   /* Set when PC calls a service      */
#define SERVICE_PCWAITn    3        /* Cleared when service replys      */
#define AMIGA_EXCLUSIVE    0x0010   /* Only one Amiga customer allowed  */
#define AMIGA_EXCLUSIVEn   4
#define PC_EXCLUSIVE       0x0020   /* Only one PC customer allowed     */
#define PC_EXCLUSIVEn      5
#define SERVICE_ADDED      0x0040   /* Set when service is added        */
#define SERVICE_ADDEDn     6


/* === ServiceCustomer Structure =========================================== 
 * A ServiceCustomer structure is created for each "customer" of a given 
 * channel
 */

 struct ServiceCustomer {

   APTR     scs_NextCustomer;

   USHORT   scs_Flags;

   APTR     scs_Task;          /* This points to the task of the customer */
   ULONG    scs_SignalBit;     /* Signal the customer with this bit       */

   ULONG    scs_JazzReserved[4];
   };

/* === Flag Definitions === */
/* These flags are set/cleared by the system */
#define CALL_TOPC_ONLY      0x0100
#define CALL_TOPC_ONLYn      8
#define CALL_FROMPC_ONLY   0x0200
#define CALL_FROMPC_ONLYn   9
#define CALL_TOAMIGA_ONLY   0x0400
#define CALL_TOAMIGA_ONLYn   10
#define CALL_FROMAMIGA_ONLY   0x0800
#define CALL_FROMAMIGA_ONLYn   11




/*
 * === AddService() Flags ==================================================
 * These are the definitions of the flag arguments that can be passed to the
 * AddService() function.
 */

#define ADDS_EXCLUSIVE       0x0001 /* want to be only Amiga customer     */
#define ADDS_EXCLUSIVEn      0
#define ADDS_TOPC_ONLY       0x0002 /* want to send signals only to PC    */
#define ADDS_TOPC_ONLYn      1
#define ADDS_FROMPC_ONLY     0x0004 /* want to get signals only from PC   */
#define ADDS_FROMPC_ONLYn    2
#define ADDS_TOAMIGA_ONLY    0x0008 /* want to send signals only to Amiga */
#define ADDS_TOAMIGA_ONLYn   3
#define ADDS_FROMAMIGA_ONLY  0x0010 /* want to get signals only from Amiga*/
#define ADDS_FROMAMIGA_ONLYn 4
#define ADDS_LOCKDATA        0x0020 /* S'DataLock locked before linking to*/
#define ADDS_LOCKDATAn       5      /* system                             */

/*
 * These are the system's AddService() Flags 
 */
#define SD_CREATED         0x0100
#define SD_CREATEDn        8



/*
 * === GetService() Flags ==================================================
 * These are the definitions of the flag arguments that can be passed to the
 * GetService() function.
 */

#define GETS_WAIT            0x0001 /* wait till service is available     */
#define GETS_WAITn           0
#define GETS_TOPC_ONLY       0x0002 /* want to send signals only to PC    */
#define GETS_TOPC_ONLYn      1
#define GETS_FROMPC_ONLY     0x0004 /* want to get signals only from PC   */
#define GETS_FROMPC_ONLYn    2
#define GETS_TOAMIGA_ONLY    0x0008 /* want to send signals only to Amiga */
#define GETS_TOAMIGA_ONLYn   3
#define GETS_FROMAMIGA_ONLY  0x0010 /* want to get signals only from Amiga*/
#define GETS_FROMAMIGA_ONLYn 4
#define GETS_EXCLUSIVE       0x0020 /* want to be only Amiga customer     */
#define GETS_EXCLUSIVEn      5
#define GETS_ALOAD_A         0x0040 /* Autoload the Amiga service         */
#define GETS_ALOAD_An        6
#define GETS_ALOAD_PC        0x0080 /* Autoload the PC service            */
#define GETS_ALOAD_PCn       7
#define GETS_WAITmask        0x0300
#define GETS_WAIT_15         0x0100 /* Wait up to 15 seconds for a service*/
#define GETS_WAIT_30         0x0200 /*      "     30          "           */
#define GETS_WAIT_120        0x0300 /*      "    120          "           */



/*
 * === AddService() and GetService() Result Codes ==========================
 * These are the result codes that may be returned by a call to AddService()
 * or GetService()
 */

#define JSERV_NOFUNCTION     -1 /* Tried to call a not supported function */
#define JSERV_OK             0  /* All is well */
#define JSERV_PENDING        0  /* Called service still pending on Amiga side */
#define JSERV_FINISHED       1  /* Called service is finished on Amiga side */
#define JSERV_NOJANUSBASE    2  /* ServiceBase structure not defined */
#define JSERV_NOJANUSMEM     3  /* We ran out of Janus memory */
#define JSERV_NOAMIGAMEM     4  /* On the Amiga side we ran out of Amiga memory */
#define JSERV_NOPCMEM        5  /* On the PC side we ran out of PC memory */
#define JSERV_NOSERVICE      6  /* Tried to get a service that doesn't exist */
#define JSERV_DUPSERVICE     7  /* Tried to add a service that already existed */
#define JSERV_ILLFUNCTION    8  /* Tried to call an illegal function */
#define JSERV_NOTEXCLUSIVE   9  /* Wanted to but couldn't be exclusive user */
#define JSERV_BADAUTOLOAD    10 /* Wanted to autoload but couldn't */


#endif /* End of JANUS_SERVICES_H conditional compilation                 */
