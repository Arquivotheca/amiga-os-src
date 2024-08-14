
* *** serviceint.i **********************************************************
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
* ************************************************************************* 



	IFND	EXEC_SEMAPHORES_I
	INCLUDE "exec/semaphores.i"
	ENDC	EXEC_SEMAPHORES_I



* === ===================================================================== 
* === PRIVATE STRUCTURES ================================================== 
* === ===================================================================== 
* For Your Eyes Only
* Not for Public Consumption




* === ServiceParam Structure ============================================== 
* This structure exists in parameter memory to be used by both the PC and 
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


SERVICE_IO_FIELDS	EQU	4	; The number of channel IO fields 

 STRUCTURE ServiceParam,0

	; A janus-style lock for non-autolocking accessing of
	; these parameters
	UBYTE 	spm_Lock

	UBYTE 	spm_ReservedPad0


 IFD	CHANNELING
	; The ChannelMask bytes have a bit for each channel in the system.
	; There are 32 bytes, which total 256 bits, which, amazingly, is
	; exactly how many channels we have available.
	; If a bit is set, the channel is already allocated.  If a bit is
	; clear, the channel is available.
	; You must lock the ServiceParam before accessing these bytes.
	STRUCT	spm_ChannelMasks,32


	; These fields are used by the PC and the Amiga to inform one
	; another of channel requests.
	; There are multiple fields in case one side has multiple services
	; to send before the other can get around to receiving them.
	; You don't need to lock before accessing these fields.
	STRUCT 	spm_PCToAmiga,SERVICE_IO_FIELDS	; Set by PC, cleared by Amiga 
	STRUCT 	spm_AmigaToPC,SERVICE_IO_FIELDS	; Set by Amiga, cleared by PC 


	; The AddsService bytes are used to communicate channel numbers
	; when either side adds a service.  The side adding the service
	; writes the service number here (waiting for the zero-byte protocol
	; of course), writes an ADD_SERVICE channel number to the
	; normal channel number array above, then interrupts the other side.
	; You don't need to lock before accessing these fields.
	UBYTE 	spm_PCAddsService
	UBYTE 	spm_AmigaAddsService
 ENDC	; of IFD CHANNELING

 IFND	CHANNELING
	; These fields are used by the PC and the Amiga to inform one
	; another of channel requests.
	; There are multiple fields in case one side has multiple services
	; to send before the other can get around to receiving them.
	; You don't need to lock before accessing these fields.
	STRUCT 	spm_PCToAmiga,SERVICE_IO_FIELDS*2 ; Set by PC, unset by Amiga 
	STRUCT 	spm_AmigaToPC,SERVICE_IO_FIELDS*2 ; Set by Amiga, unset by PC 


	; The AddsService bytes are used to communicate channel numbers
	; when either side adds a service.  The side adding the service
	; writes the service number here (waiting for the unset-field protocol
	; of course), then interrupts the other side.
	; You don't need to lock before accessing these fields.
	STRUCT	spm_PCAddsService,2*2
	STRUCT	spm_PCDeletesService,2*2
	STRUCT	spm_AmigaAddsService,2*2
	STRUCT	spm_AmigaDeletesService,2*2
 ENDC	; of IFND CHANNELING



	; Each service in the system has a ServiceData structure in janus
	; memory.  The ServiceData structures are linked together in
	; ascending numerical order according to channel number.
	; This field points to the first of the ServiceData structures.
	; You must lock the ServiceParam before traversing the list.
	RPTR	spm_FirstServiceData


	STRUCT	spm_CyclopsReserved,4*4

    LABEL	ServiceParam_SIZEOF


 IFD	CHANNELING
* These are the channel number definitions.
* The first group defines the special channel numbers which are used by the 
* two processors to communicate their service requests to one another.  
* The final definition describes the first channel for user allocations.

NO_CHANNEL		EQU	0	
ADD_SERVICE		EQU	1	
DELETE_SERVICE		EQU	2	
FIRST_USER_CHANNEL	EQU	16	; User channels start with this one 
 ENDC	; of IFD CHANNELING



* === ServiceBase Structure =============================================== 
* This structure is the main controlling structure on the Amiga side.  

 STRUCTURE ServiceBase,0

*	STRUCT	sb_ServiceSemaphore,SignalSemaphore_SIZEOF
	STRUCT	sb_ServiceSemaphore,SS_SIZE


	; This points to the service interrupt parameter block 
	APTR	sb_ServiceParam


	; The ServiceTask allocates a signal and stores it here 
	LONG 	sb_TaskSigNum


	; The ServiceTask calls SetupSig() to attach itself to
	; the JSERV_AMIGASERVICE interrupt.  The pointer to the
	; setupsig structure that the calls returns is stashed here.
	APTR	sb_SetupSig


	; When the janus library opens, it creates the service task, which
	; monitors the service interrupt signal from the PC and shares
	; dispatch responsibilities with the service library calls.
	; Either of these guys must lock the semaphore of this structure
	; before they use it.
	APTR	sb_ServiceTask


	; This address has the pointer to the GfxBase library structure 
	APTR	sb_GfxBase


	STRUCT	sb_SiameseReserved,4*4

    LABEL	ServiceBase_SIZEOF



;OLD:* === ServiceCustomer Structure =========================================== 
;OLD:* A ServiceCustomer structure is created for each "customer" of a given 
;OLD:* channel
;OLD:
;OLD: STRUCTURE ServiceCustomer,0
;OLD:
;OLD:	APTR	sc_NextCustomer
;OLD:
;OLD:	USHORT 	sc_Flags
;OLD:
;OLD:	APTR	sc_Task		; This points to the task of the customer 
;OLD:	ULONG 	sc_SignalBit	; Signal the customer with this bit 
;OLD:
;OLD:	STRUCT 	sc_JazzReserved,4*4
;OLD:
;OLD:    LABEL	ServiceCustomer_SIZEOF



