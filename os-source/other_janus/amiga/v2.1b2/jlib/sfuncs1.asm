
* *** sfuncs1.asm *************************************************************
* 
* Services Functions Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 08-Mar-91  rsd	    make local (xxx$) symbols <= 3 digits for cape
* 20-Apr-89  -BK            Fixed AddService to Allow adding a service
*                           with no data memory.
* 7-Oct-88   -RJ            Added lots of little details regarding adding 
*                           EXCLUSIVE and ONLY stuff
* 27-Jul-88  -RJ            In AddService() now calculate and save PCMemPtr
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 19-Feb-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


   INCLUDE "assembly.i"

   NOLIST
   INCLUDE "exec/types.i"

   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/janusvar.i"
   INCLUDE "janus/memory.i"
   INCLUDE "janus/services.i"

    INCLUDE "serviceint.i"
   LIST

   INCLUDE "asmsupp.i"


   XLIB   AllocJanusMem
   XLIB   AllocMem
   XLIB   Forbid
   XLIB   FreeJanusMem
   XLIB   FreeMem
   XLIB   JanusMemBase
   XLIB   JanusMemToOffset
   XLIB   JanusMemType
;SEMS
   XLIB   ObtainSemaphore
   XLIB   Permit
   XLIB   ReleaseSemaphore
;SEMS
   XLIB   SendJanusInt
   XLIB   WaitTOF

   XREF   AllocServiceCustomer
   XREF   FreeServiceCustomer
   XREF   FreeSDSemaphore
   XREF   InformNewCustomers
   XREF   MakeSDSemaphore
   XREF   MakeServiceData
   XREF   UnmakeServiceData
   XREF   WaitServiceParam
   XREF   WaitForPC

   XDEF   AddService

   XLIB   MakeBytePtr
   XLIB   MakeWordPtr
   XLIB   JanusOffsetToMem
   XLIB   TranslateJanusPtr
   XLIB   JanusUnlock
   XLIB   JanusLock



* =============================================================================
* === IMPLEMENTATION ALGORITHMS ===============================================
* =============================================================================
* 
* This section is for Torsten and RJ.  These notes are *not* to be released 
* to the real world, please.




* - Regarding reading and writing the communication fields 
*   that we share in ServiceBase, here's the rules that we must follow:  
*    - When you're working with a "To" field (where the data is 
*      coming to you, for example you're the PC and you're working 
*      with the AmigaToPC[] or the AmigaAddsService[] fields) then:
*       - Lock the ServiceParam lock 
*       - Read any or all of the "To" fields, setting back to -1 
*       - as needed 
*       - Unlock the ServiceParam lock
*    - When you're working with one of the fields where the data 
*      is going from you to the other processor, you must follow 
*      the protocol described by the following loop:
*       - If you are looking for a field that contains -1 
*         (which means that the field is available) then 
*         you don't have to lock the ServiceParam lock.  
*         But if you are looking for a field that's not -1 
*         (for instance, you're trying to match a given service 
*         offset number) then you must lock the ServiceParam lock
*       - Examine and use the fields of interest just once.  
*         Do not loop here!  Look at the fields just once and 
*         then proceed
*       - Unlock the ServiceParam lock if you locked it
*       - If you did not succeed in what you were doing, 
*         then start again at the top of this loop
* - Regarding locking both ServiceParam and ServiceData locks, remember
*   that when you are going to be locking them both you must *always* 
*   lock ServiceParam first, ServiceData second.  If one of us ever does it 
*   the other way around, we can deadlock waiting for one another 



AddService:
******* janus.library/AddService ********************************************
*
*   NAME   
*     AddService -- Adds a Janus Service to the system
*
*   SYNOPSIS
*     resultcode = AddService(
*     D0:0-15
*         ServiceData, AppID, LocalID, MemSize, MemType, SignalNum, Flags)
*         A0           D0     D1:0-15  D2:0-15  D3:0-15  D4:0-15    D5:0-15
*
*
*     SHORT AddService(struct ServiceData **,ULONG,USHORT,USHORT,USHORT,
*                                   SHORT,USHORT);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     This routine adds a Janus service to the system.  Please refer 
*     to the Janus Services reference documentation for a description of 
*     services and adding a service to the system.  
*     
*     The ServiceData argument you supply must be an address of a pointer 
*     to a ServiceData structure.  If the service is successfully added, 
*     when this call returns your pointer will point to the ServiceData 
*     structure of your newly-added service.  If the call fails your 
*     pointer will be equal to NULL.  The address, if non-NULL, will 
*     be an address in Janus memory's word-access address space.  
*     
*     Your ServiceData is set equal to NULL if AddService() fails. This
*     provides you with a convenient way to test whether or not you have 
*     to call DeleteService() during your clean-up routine.  For example:
*
*        struct ServiceData *SData = NULL;
*        result = AddService(&SData, ...);
*        myExit();
*        myExit()
*        {
*           if (SData) DeleteService(SData);
*           exit(0);
*        }
*     
*     The Application ID, and LocalID that you provide are used
*     by other programs to rendevous with your service.  The Application ID 
*     that you use should be gotten from Commodore-Amiga, Inc.  
*     Do not use an Application ID of your own making, for you risk a 
*     collision with someone else's use of the same ID number.
*     The LocalID is defined by you to mean anything you want it to mean.  
*     Anyone who wants to rendevous with your service needs to know both 
*     your Application ID and your Local ID.  
*     
*     SignalNumber is the number of the signal bit that you will use 
*     to Wait() for CallService() signals.  Typically you will get 
*     your SignalNumber from the Exec function AllocSignal().  
* 
*     The Flags argument lets you specify details about the type of service 
*     you want to add.  Please refer to the Janus Services reference 
*     documentation and the services.[hi] include file for information 
*     regarding the arguments that may be provided to the AddService() 
*     call via the Flags argument.  
*     
*     This routine returns a code which is either JSERV_OK or some
*     error-return code. The codes that this routine may return are defined
*     in services.[hi].
*     
*     If you call AddService() successfully, you are obliged to call 
*     DeleteService() sooner or later.  
*
*   INPUTS
*     ServiceData  = address of a pointer to a ServiceData structure.  
*                    After this call, the pointer will be set to either the 
*                    word-access address of the newly-added service's
*                    ServiceData structure, or NULL if the call to this
*                    routine failed
*     AppID        = a 32-bit Application ID number for the service you want
*                    to add. This number is assigned to you by
*                    Commodore-Amiga, Inc.
*                    REMINDER: do not use a number of your own making, as
*                              other software may then invalidly rendevous
*                              with your service.
*     LocalID      = a 16-bit Local ID number of your own choosing
*     MemSize      = the size of the memory block to be allocated for this 
*                     service, if any. Set this arg to zero if you want no
*                     memory allocated for this service
*     MemType      = the type of memory to be allocated for this service, 
*                    if any. If the MemSize arg above is zero, this argument
*                    is ignored
*     SignalNumber = the number of the signal bit that you will Wait() 
*                    on when waiting to receive CallService() signals.
*                    The SignalNumber is usually gotten from a call to
*                    Exec's AllocSignal() Flags = flags describing any
*                    special treatment you want during the processing of
*                    your AddService() call. See services.[hi] and the
*                    Janus Services reference document for details and
*                    definitions
*
*   RESULT
*     resultcode = a code describing the result of your call to AddService().  
*                  This will be JSERV_OK if all went well, or some
*                  error-return code if the call failed for any reason.
*                  The codes that this routine may return are defined in
*                  services.[hi].
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     CallService(), DeleteService(), GetService(), ReleaseService()
*
*****************************************************************************
*
*/


ADDS_REGS   REG   D0/D2-D7/A0/A2-A6
A_D0_OFFSET   EQU   0*4
A_A0_OFFSET   EQU   7*4

      MOVEM.L   ADDS_REGS,-(SP)

   IFGE   INFOLEVEL-1
   MOVEM.L   D0-D5,-(SP)
   MOVE.L   A0,-(SP)
   PUTMSG   1,<'%s/AddService( $%lx $%08lx $%04lx %ld $%04lx %ld $%04lx)'>
   LEA   7*4(SP),SP
   ENDC

      ;------   Before anything, call this completely 
      ;------   non-register-destructive routine which waits until 
      ;------   the ServiceTask has allocated and initialized 
      ;------   the ServiceParam data structure
      JSR   WaitServiceParam


      ;------   Before anything, call this completely 
      ;------   non-register-destructive routine which waits until 
      ;------   the PC is booted and ready to receive service 
      ;------   interrupts
      JSR   WaitForPC


* Throughout this routine, except for local changes (which must be repaired
* afterwards), these registers will be constant:
*    D2 = MemSize
*    D3 = MemType
*    D4 = ApplicationID
*    D5 = LocalID
*    D6 = AddService() Flags
*    D7 = SignalNumber
*    A2 = ExecBase
*    A4 = ServiceData.WA structure (if one is allocated, else A4 == NULL)
*    A5 = Service Janus memory.WA (if allocated, else A5 == NULL)
*    A6 = JanusBase, for janus library
      MOVE.W   D4,D7
      MOVE.W   D5,D6
      MOVE.L   D0,D4
      MOVE.L   D1,D5
      MOVEA.L   jb_ExecBase(A6),A2
      MOVEA.L   #0,A4
      MOVEA.L   A4,A5

      ;------   Next, to avoid compliations between the fact that 
      ;------   some of our args are word-sized to make PC Services 
      ;------   easier while janus.library expects these same args 
      ;------   to be longs, let's turn them into long words with 
      ;------   only words of significant information.
      MOVE.L   #$0000FFFF,D0
      AND.L   D0,D2
      AND.L   D0,D3
      AND.L   D0,D5
      AND.L   D0,D6
      AND.L   D0,D7


      ;------   Set the D0 return value register to the OK return.  If
      ;------   the AddService() call fails, the D0 on the stack will 
      ;------   be changed to reflect the reason for failure.  Later, 
      ;------   when the registers are restored at the end of the 
      ;------   call, D0 will correctly have the return code
      MOVE.L   #JSERV_OK,A_D0_OFFSET(SP)


* - if we're the Amiga lock the ServiceBase semaphore
      MOVEA.L   jb_ServiceBase(A6),A3
      LEA.L   sb_ServiceSemaphore(A3),A0
      EXG.L   A6,A2
      CALLSYS   ObtainSemaphore
;SEMS      CALLSYS   Forbid
      EXG.L   A6,A2


* - lock the ServiceParam lock
      MOVEA.L   sb_ServiceParam(A3),A3
      MOVEA.L   A3,A0
      CALLSYS   MakeBytePtr
      move.l   a0,-(sp)
      lea.l    spm_Lock(A0),A0
      CALLSYS  JanusLock
      move.l   (sp)+,a0


* - check to see if the service already exists:  traverse the ServiceData list, 
*   checking first that the ServiceData's SERVICE_DELETED flag isn't set and 
*   if it isn't then checking for a match with ApplicationID and LocalID
      MOVE.W   spm_FirstServiceData(A3),A3
      BRA   CHECK_END

CHECK_EXIST:
      ;------   Turn offset into pointer to next ServiceData struct
      MOVE.W   A3,D0
      MOVE.L   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS   JanusOffsetToMem
      MOVEA.L   D0,A3

      ;------   If this service was already deleted, don't check it
      MOVE.W   sd_Flags(A3),D0
      BTST   #SERVICE_DELETEDn,D0
      BNE   CHECK_NEXT

      ;------   Check if the user-specified ID fields match the 
      ;------   values of this service, and react if so
      CMP.L   sd_ApplicationID(A3),D4   ; Is this ID already used?
      BNE   CHECK_NEXT      ; If not, check next
      CMP.W   sd_LocalID(A3),D5   ; Is LocalID used too?
      BEQ   EXISTS         ; If so, service already exists

CHECK_NEXT:
      ;------   Get the offset to the next ServiceData structure
      MOVE.W   sd_NextServiceData(A3),A3

CHECK_END:
      CMPA.W   #-1,A3         ; If the offset to next is -1
      BEQ   NOT_EXISTS      ; then we're at end of list
      BRA   CHECK_EXIST      ; else go examine next struct


EXISTS:
*    - if match found, the service is already present (though not
*     necessarily added to the system yet) 
   PUTMSG   59,<'%s/Existing service found'>

*       - if AmigaUserCount != 0 or PCUserCount != 0, 
*         this is an AddService() request 
*         for a service that has already been added, so goto 
*        ADDSERVICE_ERROR with an error of JSERV_DUPSERVICE
      MOVEA.L   A3,A0
      CALLSYS   MakeBytePtr
      TST.B   sd_AmigaUserCount(A0)
      BNE   dupService
      TST.B   sd_PCUserCount(A0)
      BEQ   Exists3

dupService:
   PUTMSG   59,<'%s/ ERROR:  duplicate service'>
      MOVE.L   #JSERV_DUPSERVICE,A_D0_OFFSET(SP)
      BRA   ADDSERVICE_ERROR
Exists3:
   PUTMSG   59,<'%s/ at least not duplicate'>

*       - if we're the Amiga and the ServiceData's AMIGA_EXCLUSIVE 
*        flag is set or we're the PC and the ServiceData's
*        PC_EXCLUSIVE flag is set then we're about to violate 
*        someone else's exclusivity so goto ADDSERVICE_ERROR 
*        with a NOTEXCLUSIVE error
      MOVE.W   sd_Flags(A3),D0
      BTST   #AMIGA_EXCLUSIVEn,D0
      BNE   notExclusive

*       - if this processor's ServiceCustomer field is non-null 
*         and the caller has requested ADDS_EXCLUSIVE, then some 
*         task or handler is already attached with this service so 
*         the exclusivity request fails, so goto ADDSERVICE_ERROR 
*        with a NOTEXCLUSIVE error
      TST.L   sd_FirstAmigaCustomer(A3)  ; Got any customers yet?
      BEQ   Exists2            ; If not, we're cool
      BTST   #ADDS_EXCLUSIVEn,D6      ; Want it exclusively?
      BEQ   Exists2            ; If not, we're cool

notExclusive:
   PUTMSG   59,<'%s/ ERROR:  not exclusive'>
      MOVE.L   #JSERV_NOTEXCLUSIVE,A_D0_OFFSET(SP) ; else oops
      BRA   ADDSERVICE_ERROR

Exists2:
   PUTMSG   59,<'%s/ at least not exclusive'>

*       - else the service has been requested but not yet added
*        which is cool, so proceed
      MOVE.L   A3,A4      ; Copy the new ServiceData ptr
      BRA   NEW_SERVICE_USER

NOT_EXISTS:
*    - else as a match was not found, this is a virgin service 
*      so start it from scratch
*       - ServiceData = MakeServiceData()
*       - if ServiceData == NULL goto ADDSERVICE_ERROR
*        with a NOJANUSMEM error
      JSR   MakeServiceData
      TST.L   D0
      BNE   10$
      MOVE.L   #JSERV_NOJANUSMEM,A_D0_OFFSET(SP)
      BRA   ADDSERVICE_ERROR
10$      MOVEA.L   D0,A4      ; Copy our new ServiceData ptr
      BSET   #SD_CREATEDn,D6   ; Mark the flag that we allocated it

NEW_SERVICE_USER:
   PUTMSG   59,<'%s/new service user'>
* - if the caller's ADDS_EXCLUSIVE flag is set
*    - to get here, either we just created the ServiceData or we are 
*     the only customer on this side of the Bridge
*    - if we're the Amiga, set the ServiceData's AMIGA_EXCLUSIVE flag
*    - if we're the PC, set the ServiceData's PC_EXCLUSIVE flag
      BTST   #ADDS_EXCLUSIVEn,D6
      BEQ   500$
      MOVE.W   sd_Flags(A4),D0
      BSET   #AMIGA_EXCLUSIVEn,D0
      MOVE.W   D0,sd_Flags(A4)
500$

* - allocate janus memory of MemType and MemSize
*    - if allocation not successful, goto ADDSERVICE_ERROR with 
*      a NOJANUSMEM error
      MOVE.L   D2,D0
      TST.L    D0                ;DO WE NEED MEMORY?
      BEQ      9$                ;NO!
      MOVE.L   D3,D1
      CALLSYS   AllocJanusMem
      TST.L   D0
      BNE   10$
      MOVE.L   #JSERV_NOJANUSMEM,A_D0_OFFSET(SP)
      BRA   ADDSERVICE_ERROR
9$    MOVEA.L D0,A5                 ;Kludge, to avoid a total rewrite
      MOVE.W   D2,sd_MemSize(A4)    ;we duplicate some code here
      MOVE.W   D3,sd_MemType(A4)
      MOVE.W   D0,sd_MemOffset(A4)
      MOVE.L   A5,sd_AmigaMemPtr(A4)
      MOVE.L   D0,sd_PCMemPtr(A4)   ; Save the address
      BRA      400$

10$   MOVEA.L   D0,A5


* - clear the new memory buffer
      MOVEA.L   A5,A0
      CALLSYS   MakeBytePtr
      MOVE.W   D2,D1
      SUBQ.W   #1,D1
      MOVEQ.L   #0,D0
100$      MOVE.B   D0,(A0)+
      DBRA   D1,100$


* - set ServiceData's MemType, MemSize and MemOffset
      MOVE.W   D2,sd_MemSize(A4)
      MOVE.W   D3,sd_MemType(A4)
      MOVE.L   A5,D0
      CALLSYS   JanusMemToOffset
      MOVE.W   D0,sd_MemOffset(A4)


* - if we're the Amiga
*    - save the MemPtr in the ServiceData's AmigaMemPtr field
*    - calculate and set the PCMemPtr
* - else if we're the PC
*    - save the MemPtr in the ServiceData's PCMemPtr field
*    - calculate and set the AmigaMemPtr
      MOVE.L   A5,sd_AmigaMemPtr(A4)

      ;------   Calculate the PC address of the memory too.
      ;------   First get the 16 bits of Offset as the high 16 bits.  
      ;------   Then get PC's segment of the memory and use this 
      ;------   as the low 16 bits of the address.  
      ;------   This is the PC address.
      MOVE.L   jb_ParamMem(A6),A0   ; Get the address of JanusAmiga
      CALLSYS   MakeWordPtr      ; Use word access
      BTST   #MEMB_PARAMETER,D3   ; Is memory type param?
      BEQ   320$         ; If not try next type
      LEA   ja_ParamMem(A0),A0   ; Use param mem 
      BRA   321$
320$      LEA   ja_BufferMem(A0),A0   ; No other type, so use buffer
321$
      MOVE.W   sd_MemOffset(A4),D0   ; Get the offset into low 16
      SWAP   D0         ; Put offset into high 16
      MOVE.W   jmh_8088Segment(A0),D0   ; Fetch the segment
      MOVE.L   D0,sd_PCMemPtr(A4)   ; Save the address

400$
* - if we're the Amiga
*    - if MakeSDSemaphore(ServiceData) == NULL goto
*      ADDSERVICE_ERROR with a NOAMIGAMEM error
      MOVEA.L   A4,A0
      JSR   MakeSDSemaphore
      TST.L   D0
      BNE   432$
      MOVE.L   #JSERV_NOAMIGAMEM,A_D0_OFFSET(SP)
      BRA   ADDSERVICE_ERROR
432$

* - allocate a ServiceCustomer structure in non-janus memory
*    - if can't allocate, goto ADDSERVICE_ERROR with 
*      a NOAMIGAMEM or NOPCMEM error
      MOVE.W   D7,D0
      MOVE.W   D6,A0   ; pass AddService() Flags
      MOVEQ.L   #1,D1   ; We're calling from AddService()
      JSR   AllocServiceCustomer
      TST.L   D0
      BNE   20$
      MOVE.L   #JSERV_NOAMIGAMEM,A_D0_OFFSET(SP)
      BRA   ADDSERVICE_ERROR
20$

   IFGE   INFOLEVEL-59
   MOVEM.L   D0,-(SP)
   PUTMSG   59,<'%s/AllocServiceCustomer=0x%lx'>
   MOVEM.L   (SP)+,D0
   ENDC

* - NOTE:  FROM HERE TO THE END there can be no error exit, because 
*   we are about to tell the customers (if any) that the service has 
*   become available, so it would not be nice to fail after this; 
*   also, we're going to link in the ServiceCustomer structure and 
*   lock the ServiceData now if the caller wants it 
*   which are actions we currently don't undo in the error handler below 

* - initialize and add the new ServiceCustomer to the ServiceData list
      MOVE.L   sd_FirstAmigaCustomer(A4),scs_NextCustomer(A0)
      MOVE.L   A0,sd_FirstAmigaCustomer(A4)


* - if user's ADDS_LOCKDATA flag is set
      BTST   #ADDS_LOCKDATAn,D6
      BEQ   lockDone

*    - if we're the Amiga, lock the ServiceData semaphore now
      MOVEA.L   sd_Semaphore(A4),A0
      EXG   A2,A6
      CALLSYS   ObtainSemaphore
      EXG   A2,A6

*    - lock the ServiceDataLock now
      MOVEA.L   A4,A0
      CALLSYS   MakeBytePtr
      move.l   a0,-(sp)
      lea.l    sd_ServiceDataLock(A0),A0
      CALLSYS  JanusLock
      move.l   (sp)+,a0

lockDone:

* - set the ServiceData's SERVICE_ADDED flag
      MOVE.W   sd_Flags(A4),D0
      BSET   #SERVICE_ADDEDn,D0
      MOVE.W   D0,sd_Flags(A4)

* - InformNewCustomers()
      MOVE.L   D0,-(SP)   ; Stash ptr to new ServiceCustomer
      MOVE.L   A4,A0
      MOVEQ.L   #1,D0      ; We are calling from AddService()
      MOVEQ.L   #2,D1      ; We're calling from the Amiga
      JSR   InformNewCustomers
      MOVEA.L   (SP)+,A0   ; Restore ptr to new ServiceCustomer

* - unlock the ServiceParam lock
      MOVEA.L   jb_ServiceBase(A6),A3
      MOVEA.L   sb_ServiceParam(A3),A0
      CALLSYS   MakeBytePtr
      lea.l    spm_Lock(A0),a0
      CALLSYS  JanusUnlock

* - if we're the Amiga unlock the ServiceBase semaphore
;SEMS
      LEA.L   sb_ServiceSemaphore(A3),A0
      EXG.L   A6,A2
      CALLSYS   ReleaseSemaphore
;SEMS      CALLSYS   Permit
      EXG.L   A6,A2

* - signal the other processor with an AddsService
;???      MOVEA.L   jb_ServiceBase(A6),A3
      MOVE.L   sb_ServiceParam(A3),A3

   EXG.L   A6,A2
   CALLSYS   Forbid
   EXG.L   A6,A2

ADDS_wait:
      CMP.W   #-1,spm_AmigaAddsService(A3)
      BEQ   ADDS_add0
      CMP.W   #-1,spm_AmigaAddsService+2(A3)
      BEQ   ADDS_add1
   PUTMSG   2,<'%s/still waiting to ADD_SERVICE...'>
      ;------   Call WaitTOF() to let the system run while we're 
      ;------   waiting to check again
      MOVE.L   A3,-(SP)   ; Stash A3
      MOVEA.L   jb_ServiceBase(A6),A3
      MOVEA.L   sb_GfxBase(A3),A3
      EXG.L   A3,A6
      ;------   Note that if you are in Forbid() when the following 
      ;------   call to WaitTOF is made, the Forbid() is broken 
      ;------   until control is returned to you, at which time the 
      ;------   Forbid() is already reestablished
      CALLSYS   WaitTOF
      MOVE.L   A3,A6
      MOVE.L   (SP)+,A3
      BRA   ADDS_wait
ADDS_add1:
      ;------   We're going to transmit using the second element of the 
      ;------   16-bit array, so offset the structure pointer by two
      ADD.W   #2,A3
ADDS_add0:
      ;------   Here, A3 points to either the first or second word 
      ;------   of the ServiceParam structure, so that offsetting with
      ;------   spm_AmigaAddsService will result in the address of 
      ;------   either the first or second element of the AddsService
      ;------   array
      MOVE.L   A4,D0
      CALLSYS   JanusMemToOffset
      MOVE.W   D0,spm_AmigaAddsService(A3)

   EXG.L   A6,A2
   CALLSYS   Permit
   EXG.L   A6,A2

      MOVE.L   #JSERV_PCSERVICE,D0   ; Amiga initiates service 
      CALLSYS   SendJanusInt

* - goto ADDSERVICE_DONE
      BRA   ADDSERVICE_DONE


ADDSERVICE_ERROR:
   PUTMSG   2,<'AddService error'>

* - if we're the Amiga and we have a non-zero ServiceData pointer and 
*   there are no other customers and the semaphore field is non-NULL 
*   then FreeSDSemaphore().  Note that this presumes that we never linked 
*   a customer structure for us into the ServiceData
      CMPA.L   #0,A4
      BEQ   15$
      TST.L   sd_FirstAmigaCustomer(A4)
      BNE   15$
      TST.L   sd_Semaphore(A4)
      BEQ   15$
      MOVEA.L   A4,A0
      JSR   FreeSDSemaphore
15$

* - if we created a ServiceData structure with MakeServiceData(), 
*   get rid of it now with UnmakeServiceData()
      CMPA.L   #0,A4      ; Is there a ServiceData pointer?
      BEQ   ADDS_FREE1   ;   branch if not
      BTST   #SD_CREATEDn,D6   ; Did we allocate this structure?
      BEQ   ADDS_FREE1   ;   branch if not
      MOVEA.L   A4,A0
      JSR   UnmakeServiceData
      MOVE.L   #0,A4
ADDS_FREE1:

* - if janus memory allocated, free
      CMPA.L   #0,A5      ; Was buffer memory allocated?
      BEQ   ADDS_FREE2   ;   branch if not
      MOVEA.L   A5,A1
      MOVE.L   D2,D0
      CALLSYS   FreeJanusMem
      MOVE.L   #0,A5
ADDS_FREE2:

* - unlock the ServiceParam lock
      MOVEA.L   jb_ServiceBase(A6),A3
      MOVEA.L   sb_ServiceParam(A3),A0
      CALLSYS   MakeBytePtr
      lea.l    spm_Lock(A0),A0
      CALLSYS  JanusUnlock

* - if we're the Amiga unlock the ServiceBase semaphore
;SEMS
;???      MOVEA.L   jb_ServiceBase(A6),A3
      LEA.L   sb_ServiceSemaphore(A3),A0
      EXG.L   A6,A2
      CALLSYS   ReleaseSemaphore
;SEMS      CALLSYS   Permit
      EXG.L   A6,A2

* - and fall into ADDSERVICE_DONE

ADDSERVICE_DONE:
   PUTMSG   59,<'%s/AddService done'>

* - write the address of the ServiceData structure, or NULL, as appropriate
* - return result code
      MOVEA.L   A_A0_OFFSET(SP),A0   ; Get the address of the ptr
      MOVE.L   A4,(A0)

      MOVEM.L   (SP)+,ADDS_REGS
      RTS



      END


