
#ifndef JANUS_SERVICES_H
#define JANUS_SERVICES_H

/* *** services.h ************************************************************
 *
 * services.h -- Service Definitions and Data Structures
 *
 * Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc., All rights reserved
 * 
 * HISTORY
 * Date       name               Description
 * ---------  -----------------  ---------------------------------------------
 * 22-Feb-88  -RJ Mical-         Added service data structures
 * early 86   Burns/Katin clone  Created this file
 *
 * **************************************************************************/


/**/
/* this is the table of hard coded services.  Other services may exist
/* that are dynamically allocated.
/**/


/* service numbers constrained by hardware */
#define JSERV_MINT	0	/* monochrome display written to */
#define JSERV_GINT	1	/* color display written to */
#define JSERV_CRT1INT	2	/* mono display's control registers changed */
#define JSERV_CRT2INT	3	/* color display's control registers changed */
#define JSERV_ENBKB	4	/* keyboard ready for next character */
#define JSERV_LPT1INT	5	/* parallel control register */
#define JSERV_COM2INT	6	/* serial control register */

/* hard coded service numbers */
#define JSERV_PCBOOTED	7	/* PC is ready to service soft interrupts */

#define JSERV_SCROLL	8	/* PC is scrolling its screen */
#define JSERV_HARDDISK	9	/* Amiga reading PC hard disk */
#define JSERV_READAMIGA 10	/* PC reading Amiga mem */
#define JSERV_READPC	11	/* Amiga reading PC mem */
#define JSERV_AMIGACALL 12	/* PC causing Amiga function call */
#define JSERV_PCCALL	13	/* Amiga causing PC interrupt */
#define JSERV_AMIGASERVICE 14	/* PC initiating Amiga side of a service */
#define JSERV_PCSERVICE 15	/* Amiga initiating PC side of a service */



/* === ServiceData Structure =============================================== */
/* The ServiceData structure is used to share data among all callers of 
 * all of the Service routines.  One of these is allocated in janus memory 
 * for each service.  
 * Assembly STRUCTURE PREFIX sd_
 */
struct ServiceData
	{
	/* The ServiceData ID numbers are used to uniquely identify 
	 * application-specific services.  There are two ID numbers:  
	 * the global ApplicationID and the application's local LocalID.
	 * 
	 * The ApplicationID is a 32-bit number which *must* be assigned to 
	 * an application designer by Commodore-Amiga.  
	 * Once a service ApplicationID is assigned to an application 
	 * designer, that designer "owns" that ID number forever.
	 * Note that this will provide unique ServiceData identification 
	 * numbers only for the first 4.3 billion ServiceData designers;
	 * after that, there's some risk of a collision.
	 * 
	 * The LocalID, defined by the application designer, is a local 
	 * subcategory of the global ApplicationID.  These can mean anything 
	 * at all.  There are 65,536 of these local ID's.  
	 */
	ULONG	ApplicationID;
	USHORT	LocalID;


	/* The flag bits are defined below.  Some of these are set by the 
	 * application programs which use the service, and some are set
	 * by the system.
	 */
	USHORT	Flags;


	/* This field is initialized by the system for you, and then 
	 * is never touched by the system again.  Users of the 
	 * service can agree by convention that they have to obtain 
	 * this lock before using the service.  
	 * If you are the AddService() caller and you want this lock 
	 * to be locked before the service is linked into the system, 
	 * set the AddService() ADDS_LOCKDATA argument flag.  
	 */
	UBYTE	ServiceDataLock;


	/* This tracks the number of users currently connected 
	 * to this service.  Max of 256 concurrent users of a service.
	 */
	UBYTE	UserCount;


	/* These are the standard janus memory descriptions, which describe 
	 * the buffer memory associated with this service.  This memory 
	 * (if any) will be allocated automatically by the system when the 
	 * service is first added.  The creator of the service 
	 * (the one who calls AddService()) supplies the MemSize and 
	 * MemType values; after the service is added the MemPtr field 
	 * will point to the parameter memory.  GetService() callers, after 
	 * the service comes available, will find all of these fields 
	 * filled in with the appropriate values.  
	 * The AmigaMemPtr and PCMemPtr both point to the same location 
	 * of Janus memory; an Amiga program should use the AmigaMemPtr, 
	 * and a PC program should use the PCMemPtr.
	 */
	USHORT	MemSize;
	USHORT	MemType;
	RPTR	MemOffset;
	APTR	AmigaMemPtr;
	APTR	PCMemPtr;


	/* This offset is used as the key for calls to AllocServiceMem() 
	 * and FreeServiceMem().  This key can be used by any one 
	 * who's learned about this service via either AddService() 
	 * or GetService().  The system makes no memory allocations 
	 * using this key, so it's completely under application control.
	 * Any memory attached to this key by calls to AllocServiceMem() 
	 * will be freed automatically after the service has been 
	 * deleted and all users of the service have released the service.  
	 */
	RPTR	JRememberKey;


	/* These pointers are for the system-maintained lists of 
	 * structures.  If you disturb any of these pointers, you will be 
	 * tickling the guru's nose, and when the guru sneezes ...
	 */
	RPTR	NextServiceData;
	APTR	FirstPCCustomer;
	APTR	FirstAmigaCustomer;


	/* These fields are reserved for future use */
	ULONG	ZaphodReserved[4];
	};


/* === Flag Definitions === */
#define SERVICE_DELETED		0x0001	/* Owner of this service deleted it */
#define SERVICE_DELETEDn	0
#define EXPUNGE_SERVICE		0x0002	/* Owner of service should delete */
#define EXPUNGE_SERVICEn	1
#define SERVICE_AMIGASIDE	0x0004	/* Set if Amiga created the service */
#define SERVICE_AMIGASIDEn	2

#define AMIGA_MEMPTR		0x0100	/* This is a system flag */
#define AMIGA_MEMPTRn		8	/* This is a system flag */
#define PC_MEMPTR		0x0200	/* This is a system flag */
#define PC_MEMPTRn		9	/* This is a system flag */



/* === AddService() Flags ================================================== */
/* These are the definitions of the flag arguments that can be passed to the 
 * AddService() function.
 */
#define ADDS_EXCLUSIVE      0x0001 /* You want to be the *only* Amiga customer */
#define ADDS_EXCLUSIVEn     0
#define ADDS_TOPC_ONLY      0x0002 /* You want to send signals only to the PC */
#define ADDS_TOPC_ONLYn     1
#define ADDS_FROMPC_ONLY    0x0004 /* You want to get signals only from the PC */
#define ADDS_FROMPC_ONLYn   2
#define ADDS_TOAMIGA_ONLY   0x0008 /* You want to send signals only to the Amiga */
#define ADDS_TOAMIGA_ONLYn  3
#define ADDS_FROMAMIGA_ONLY 0x0010 /* You want to get signals only from the Amiga  */
#define ADDS_FROMAMIGA_ONLYn 4
#define ADDS_LOCKDATA       0x0020 /* S'DataLock locked before linking to system */
#define ADDS_LOCKDATAn      5


/* These are the system's AddService() Flags */
#define SD_CREATED          0x0100
#define SD_CREATEDn         8



/* === GetService() Flags ================================================== */
/* These are the definitions of the flag arguments that can be passed to the 
 * GetService() function.
 */
#define GETS_WAIT           0x0001 /* If service not yet available, you'll wait  */
#define GETS_WAITn          0
#define GETS_TOPC_ONLY      0x0002 /* You want to send signals only to the PC */
#define GETS_TOPC_ONLYn     1
#define GETS_FROMPC_ONLY    0x0004 /* You want to get signals only from the PC */
#define GETS_FROMPC_ONLYn   2
#define GETS_TOAMIGA_ONLY   0x0008 /* You want to send signals only to the Amiga */
#define GETS_TOAMIGA_ONLYn  3
#define GETS_FROMAMIGA_ONLY 0x0010 /* You want to get signals only from the Amiga  */
#define GETS_FROMAMIGA_ONLYn 4
#define GETS_EXCLUSIVE      0x0020 /* You want to be the *only* Amiga customer  */
#define GETS_EXCLUSIVEn     5
#define GETS_ALOAD_A        0x0040 /* Autoload the service on the Amiga side */
#define GETS_ALOAD_An       6
#define GETS_ALOAD_PC       0x0080 /* Autoload the service on the PC side */
#define GETS_ALOAD_PCn      7



/* === AddService() and GetService() Result Codes ========================== */
/* These are the result codes that may be returned by a call to AddService() 
 * or GetService()
 */
#define	JSERV_OK         0  /* All is well */
#define	JSERV_NOJANUSMEM 1  /* We ran out of Janus memory */
#define	JSERV_NOAMIGAMEM 2  /* On the Amiga side we ran out of Amiga memory */
#define	JSERV_NOPCMEM    3  /* On the PC side we ran out of PC memory */
#define	JSERV_NOSERVICE  4  /* Tried to get a service that doesn't exist */
#define	JSERV_DUPSERVICE 5  /* Tried to add a service that already existed */



/* === Function Declarations =============================================== */
/* If you want the Service functions to be typed for you, define 
 * SERVICE_TYPES
 * If you want (and your language supports) automatic type-checking 
 * of arguments, define LINT_ARGS.
 */
#ifdef	LINT_ARGS
extern struct ServiceData *AddService(ULONG,USHORT,USHORT,USHORT,SHORT,USHORT);
extern VOID   CallService(ServiceData *);
extern VOID   DeleteService(ServiceData *);
extern struct ServiceData *GetService(ULONG,USHORT,SHORT,USHORT);
extern APTR   JanusOffsetToMem(USHORT,USHORT);
extern UBYTE  *MakeBytePtr(APTR);
extern USHORT *MakeWordPtr(APTR);
extern VOID   ReleaseService(ServiceData *);
extern APTR   TranslateJanusPtr(APTR,USHORT);
#else
#ifdef SERVICE_TYPES
extern struct ServiceData *AddService();
extern VOID   CallService();
extern VOID   DeleteService();
extern struct ServiceData *GetService();
extern APTR   JanusOffsetToMem();
extern UBYTE  *MakeBytePtr();
extern USHORT *MakeWordPtr();
extern VOID   ReleaseService();
extern APTR   TranslateJanusPtr();
#endif /* of SERVICES_TYPES */
#endif /* of LINT_ARGS */




#endif !JANUS_SERVICES_H


