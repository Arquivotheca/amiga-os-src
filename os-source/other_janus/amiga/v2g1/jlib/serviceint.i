
* *** serviceint.i **********************************************************
* 
* Internal Kanus Service Data Structures
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



* === ServiceParam Structure ============================================== 
* This structure exists in parameter memory to be used by both the PC and 
* the Amiga.  It is allocated and initialized by the call to SetupJanusSig() 
* on the Amiga side.  


SERVICE_IO_FIELDS	EQU	4	; The number of channel IO fields 

 STRUCTURE ServiceParam,0

	; A janus-style lock for non-autolocking accessing of
	; these parameters
	UBYTE 	spm_Lock

	UBYTE 	spm_ReservedPad0

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

	; Each service in the system has a ServiceData structure in janus
	; memory.  The ServiceData structures are linked together in
	; ascending numerical order according to channel number.
	; This field points to the first of the ServiceData structures.
	; You must lock the ServiceParam before traversing the list.
	RPTR	spm_FirstServiceData

	STRUCT	spm_CyclopsReserved,4*4

    LABEL	ServiceParam_SIZEOF


IOFIELD_COUNT EQU (SERVICE_IO_FIELDS+SERVICE_IO_FIELDS+2+2+2+2)



* === ServiceBase Structure =============================================== 
* This structure is the main controlling structure on the Amiga side.  

 STRUCTURE ServiceBase,0

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



