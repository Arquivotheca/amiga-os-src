
#ifndef JANUS_SERVICEINT_H
#define JANUS_SERVICEINT_H

/* *** serviceint.h **********************************************************
 * 
 * Private (Internal Only) Service Data Structures
 * 
 * CONFIDENTIAL and PROPRIETARY
 * Copyright (C) 1987, 1988, Commodore-Amiga Inc.
 * 
 * HISTORY       Name            Description
 * ------------  --------------  ----------------------------------------------
 * 18 Feb 1988   -RJ Mical-      Created this file!
 *
 * ************************************************************************* */



/* === ===================================================================== 
 * === PRIVATE STRUCTURES ================================================== 
 * === ===================================================================== 
 * For Your Eyes Only
 * Not for Public Consumption
 */



#include <exec/semaphores.h>	



/* === ServiceParam Structure ============================================== */
/* This structure exists in parameter memory to be used by both the PC and 
 * the Amiga.  It is allocated and initialized by the call to SetupJanusSig() 
 * on the Amiga side.  
 * The "channel fields" in this parameter memory can be safely read or 
 * written without locking by either the Amiga or PC because of 
 * two things:  first, only one processor can access a byte at a time; 
 * second, the agreed-upon protocol is that for any given field 
 * one processor will write only non-zero values and the other will 
 * write only zeroes, which provides an implicit lock in this sense:
 *    - Consider a PCToAmiga field which is initialized to zero
 *    - While the Amiga wants to read from this field and sees it's zero, 
 *      the Amiga doesn't read
 *    - The PC wants to write to this field, sees it's zero, writes
 *    - If the PC wants to write to the field and sees it's non-zero, 
 *      its previous write hasn't been read yet by the Amiga so it waits
 *    - If the Amiga wants to read from this field and sees it's non-zero, 
 *      there's a valid value there which won't change until the Amiga 
 *      sets the field to zero so the Amiga reads the value and safely 
 *      zeroes the field, signalling the PC that it's safe to write a new 
 *      non-zero value
 * This protocol obviates the need for a Lock for channel passing.
 * 
 * Assembly STRUCTURE PREFIX sp_
 */

#define SERVICE_IO_FIELDS	4	/* The number of channel IO fields */

struct ServiceParam
	{
	/* A janus-style lock for non-autolocking accessing of 
	 * these parameters
	 */
	UBYTE	Lock;

	UBYTE	ReservedPad0;


#ifdef	CHANNELING
	/* The ChannelMask bytes have a bit for each channel in the system.  
	 * There are 32 bytes, which total 256 bits, which, amazingly, is 
	 * exactly how many channels we have available.  
	 * If a bit is set, the channel is already allocated.  If a bit is 
	 * clear, the channel is available.  
	 * You must lock the ServiceParam before accessing these bytes.
	 */
	UBYTE	ChannelMasks[32];


	/* These fields are used by the PC and the Amiga to inform one 
	 * another of channel requests.
	 * There are multiple fields in case one side has multiple services 
	 * to send before the other can get around to receiving them.
	 * You don't need to lock before accessing these fields.
	 */
	UBYTE	PCToAmiga[SERVICE_IO_FIELDS]; /* Set by PC, cleared by Amiga */
	UBYTE	AmigaToPC[SERVICE_IO_FIELDS]; /* Set by Amiga, cleared by PC */


	/* The AddsService bytes are used to communicate channel numbers 
	 * when either side adds a service.  The side adding the service 
	 * writes the service number here (waiting for the zero-byte protocol
	 * of course), writes an ADD_SERVICE channel number to the 
	 * normal channel number array above, then interrupts the other side.  
	 * You don't need to lock before accessing these fields.
	 */
	UBYTE	PCAddsService;
	UBYTE	AmigaAddsService;
#endif	/* of ifdef CHANNELING */
#ifndef	CHANNELING
	/* These fields are used by the PC and the Amiga to inform one 
	 * another of channel requests.
	 * There are multiple fields in case one side has multiple services 
	 * to send before the other can get around to receiving them.
	 * You don't need to lock before accessing these fields.
	 */
	USHORT	PCToAmiga[SERVICE_IO_FIELDS]; /* Set by PC, unset by Amiga */
	USHORT	AmigaToPC[SERVICE_IO_FIELDS]; /* Set by Amiga, unset by PC */


	/* The AddsService bytes are used to communicate channel numbers 
	 * when either side adds a service.  The side adding the service 
	 * writes the service number here (waiting for the unset-field protocol
	 * of course), then interrupts the other side.  
	 * You don't need to lock before accessing these fields.
	 */
	USHORT	PCAddsService[2];
	USHORT	PCDeletesService[2];
	USHORT	AmigaAddsService[2];
	USHORT	AmigaDeletesService[2];
#endif	/* of ifndef CHANNELING */


	/* Each service in the system has a ServiceData structure in janus 
	 * memory.  The ServiceData structures are linked together in 
	 * ascending numerical order according to channel number.  
	 * This field points to the first of the ServiceData structures.
	 * You must lock the ServiceParam before traversing the list.
	 */
	RPTR	FirstServiceData;


	ULONG	CyclopsReserved[4];
	};


#ifdef	CHANNELING
/* These are the channel number definitions.
 * The first group defines the special channel numbers which are used by the 
 * two processors to communicate their service requests to one another.  
 * The final definition describes the first channel for user allocations.
 */
#define NO_CHANNEL		0
#define ADD_SERVICE		1
#define DELETE_SERVICE		2
#define FIRST_USER_CHANNEL	16   /* User channels start with this one */
#endif	/* of ifdef CHANNELING */



/* === ServiceBase Structure =============================================== */
/* This structure is the main controlling structure on the Amiga side.  
 * 
 * Assembly STRUCTURE PREFIX sb_
 */
struct ServiceBase
	{
	/* NOTE that size in assembly language of the SignalSemaphore structure 
	 * defined below should be declared using the less than clear
	 * STRUCT  sb_ServiceSemaphore,SS_SIZE
	 */
	struct SignalSemaphore ServiceSemaphore;


	/* This is a word-access oriented Amiga pointer 
	 * to the service interrupt parameter block
	 */
	struct	ServiceParam *ServiceParam;


	/* The ServiceTask allocates a signal and stores it here */
	LONG	TaskSigNum;


	/* The ServiceTask calls SetupSig() to attach itself to 
	 * the JSERV_AMIGASERVICE interrupt.  The pointer to the 
	 * setupsig structure that the calls returns is stashed here.
	 */
	struct	SetupSig *SetupSig;


	/* When the janus library opens, it creates the service task, which 
	 * monitors the service interrupt signal from the PC and shares 
	 * dispatch responsibilities with the service library calls.  
	 * Either of these guys must lock the semaphore of this structure 
	 * before they use it.
	 */
	struct	Task *ServiceTask;


	/* This address has the pointer to the GfxBase library structure */
	struct	GfxBase *GfxBase;


	ULONG	SiameseReserved[4];
	};



/* === ServiceCustomer Structure =========================================== */
/* A ServiceCustomer structure is created for each "customer" of a given 
 * channel
 * Assembly STRUCTURE PREFIX sc_
 */
struct ServiceCustomer
	{
	struct	ServiceCustomer *NextCustomer;

	USHORT	Flags;

	struct	Task *Task;	/* This points to the task of the customer */
	ULONG	SignalBit;	/* Signal the customer with this bit */

	ULONG	JazzReserved[4];
	};



#endif !JANUS_SERVICEINT_H


