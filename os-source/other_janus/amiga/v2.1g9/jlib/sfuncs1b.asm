* *** sfuncs1b.asm **********************************************************
* 
* GetService() Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  -------------------------------------------------
* 18 Oct 88  -RJ            Cut GetService() out of sfuncs1.asm
* 7-Oct-88   -RJ            Added lots of little details regarding adding 
*                           EXCLUSIVE and ONLY stuff
* 27-Jul-88  -RJ            In AddService() now calculate and save PCMemPtr
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 19-Feb-88  =RJ Mical=     Created this file!
* 25-Aug-89  -BK            Fixed GetService() to actually wait if GETS_WAIT
*                           is set.
* 
* ***************************************************************************


      INCLUDE  "assembly.i"

      NOLIST
      INCLUDE  "exec/types.i"

      INCLUDE  "janus/janusbase.i"
      INCLUDE  "janus/janusvar.i"
      INCLUDE  "janus/memory.i"
      INCLUDE  "janus/services.i"

      INCLUDE  "serviceint.i"
      LIST

      INCLUDE  "asmsupp.i"
    
      XLIB  AllocJanusMem
      XLIB  AllocMem
      XLIB  Delay
      XLIB  FreeJanusMem
      XLIB  FreeMem
      XLIB  JanusMemBase
      XLIB  JanusMemToOffset
      XLIB  LockServiceData
      XLIB  ObtainSemaphore
      XLIB  ReleaseSemaphore
      XLIB  SendJanusInt
      XLIB  UnlockServiceData

      XREF  AllocServiceCustomer
      XREF  FreeServiceCustomer
      XREF  FreeSDSemaphore
      XREF  InformNewCustomers
      XREF  MakeSDSemaphore
      XREF  MakeServiceData
      XREF  UnmakeServiceData
      XREF  WaitServiceParam
      XREF  WaitForPC

      XREF  _GetServiceInternal
      XDEF  GetService

      XLIB  CallService
      XLIB  MakeBytePtr
      XLIB  MakeWordPtr
      XLIB  JanusOffsetToMem
      XLIB  ReleaseService
      XLIB  TranslateJanusPtr
      XLIB  JanusUnlock
      XLIB  JanusLock

GetService:
******* janus.library/GetService ********************************************
*
*   NAME   
*     GetService -- Gets a Janus Service
*
*   SYNOPSIS
*     resultcode = GetService(
*     D0:0-15
*              ServiceData, AppID, LocalID, SignalNumber, Flags)
*              A0           D0     D1:0-15  D2:0-15       D3:0-15
* 
*
*     SHORT GetService(struct ServiceData *,ULONG,USHORT,SHORT,USHORT);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     This routine gets a Janus service from the system.  Please refer 
*     to the Janus Services reference documentation for a complete 
*     description of services and getting a service from the system.  
*     
*     The ServiceData argument you supply must be an address of a pointer 
*     to a ServiceData structure.  If the service is successfully acquired, 
*     when this call returns your pointer will point to the ServiceData 
*     structure of your newly-acquired service.  If the call fails your 
*     pointer will be equal to NULL.  The address, if non-NULL, will 
*     be an address in Janus memory's word-access address space.  
*     
*     Your ServiceData is set equal to NULL if GetService() fails. This
*     provides you with a convenient way to test whether or not you have 
*     to call ReleaseService() during your clean-up routine.  For example:
*
*     struct ServiceData *SData = NULL;
*     result = GetService(&SData, ...);
*     myExit();
*     myExit()
*     {
*        if (SData) ReleaseService(SData);
*           exit(0);
*     }
*     
*     The AppID, or Application ID, and LocalID that you provide are used 
*     to rendevous with a service that is added by some other program.  
*     You must learn of the Application ID and Local ID of the service from 
*     the creator of the service before you can call GetService().  
*     
*     SignalNumber is the number of the signal bit that you will use 
*     to Wait() for CallService() signals.  Typically you will get 
*     your SignalNumber from the Exec function AllocSignal().  
* 
*     The Flags argument lets you specify details about the type of service 
*     you want to get.  Please refer to the Janus Services reference 
*     documentation and the services.[hi] include file for information 
*     regarding the arguments that may be provided to the GetService() 
*     call via the Flags argument.  
*     
*     This routine returns a code which is either JSERV_OK or some
*     error-return code.  The codes that this routine may return are
*     defined in services.[hi].
*     
*     If you call GetService() successfully, you are obliged to call 
*     ReleaseService() sooner or later.  
*
*   INPUTS
*     ServiceData  = address of a pointer to a ServiceData structure.  
*                    After this call, the pointer will be set to either
*                    the word-access address of the newly-acquired service's
*                    ServiceData structure, or NULL if the call to this
*                    routine failed
*     AppID        = a 32-bit Application ID number for the service you want
*                    to find
*     LocalID      = a 16-bit Local ID number of the service you want to
*                    find
*     SignalNumber = the number of the signal bit that you will Wait() 
*                    on when waiting to receive CallService() signals. The
*                    SignalNumber is usually gotten from a call to Exec's
*                    AllocSignal()
*     Flags        = flags describing any special treatment you want during
*                    the processing of your GetService() call. See
*                    services.[hi] and the Janus Services reference
*                    document for details and definitions
*
*   RESULT
*     resultcode = a code describing the result of your call to GetService().  
*                  This will be JSERV_OK if all went well, or some
*                  error-return code if the call failed for any reason. The
*                  codes that this routine may return are defined in
*                  services.[hi].
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     AddService(), CallService(), DeleteService(), ReleaseService()
*
*****************************************************************************
*
*/


GETS_REGS      REG   D0/D2-D7/A0-A6
DREGS          EQU   7
G_D0_OFFSET    EQU   0*4
G_A0_OFFSET    EQU   DREGS*4
G_A1_OFFSET    EQU   G_A0_OFFSET+4   ; used for scratch 
GETS_GOTSD     EQU   $0001
GETS_GOTSDn    EQU   0
GETS_SENTLOAD  EQU   $0002
GETS_SENTLOADn EQU   1
GETS_SBSEM     EQU   $0004
GETS_SBSEMn    EQU   2
GETS_SPLOCK    EQU   $0008
GETS_SPLOCKn   EQU   3

      MOVEM.L  GETS_REGS,-(SP)

      IFGE     INFOLEVEL-1
      MOVEM.L  D0-D3,-(SP)
      MOVE.L   A0,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService( $%lx $%08lx $%04lx %ld $%04lx )'>
      LEA.L    5*4(SP),SP
      ENDC


      ;------  Before anything, call this completely
      ;------  non-register-destructive routine which waits until
      ;------  the ServiceTask has allocated and initialized
      ;------  the ServiceParam data structure

      JSR      WaitServiceParam

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: After WaitServiceParam'>

      ;------  Before anything, call this completely
      ;------  non-register-destructive routine which waits until
      ;------  the PC is booted and ready to receive service
      ;------  interrupts
      JSR      WaitForPC

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: After WaitForPC'>

;      JSR      _GetServiceInternal

      ;------  Throughout this routine, except for local changes (which
      ;------  must be repaired afterwards), these registers will be
      ;------  constant:
      ;------  D2 = SignalNumber
      ;------  D3 = GetService() Flags
      ;------  D4 = ApplicationID
      ;------  D5 = LocalID
      ;------  D6 = internal flags
      ;------  A2 = ExecBase
      ;------  A3 = if A5 has a ServiceData.WA, A3 has address of
      ;------  ServiceData.BA
      ;------  A4 = ServiceCustomer struct.WA, if one is allocated, NULL
      ;------  until
      ;------  A5 = Address of ServiceData.WA, NULL until one is found
      ;------  (if ever)
      ;------  A6 = JanusBase, for janus library
      MOVE.L   D0,D4
      MOVE.L   D1,D5
      MOVEQ.L  #0,D6
      MOVE.L   #$0000FFFF,D0
      AND.L    D0,D2
      AND.L    D0,D3
      AND.L    D0,D5
      MOVEA.L  jb_ExecBase(A6),A2
      MOVEA.L  #0,A3
      MOVEA.L  A3,A4
      MOVEA.L  A4,A5


      ;------  Set the D0 return value register to the OK return.  If
      ;------  the AddService() call fails, the D0 on the stack will
      ;------  be changed to reflect the reason for failure.  Later,
      ;------  when the registers are restored at the end of the
      ;------  call, D0 will correctly have the return code
      MOVE.L   #JSERV_OK,G_D0_OFFSET(SP)


      ;------  if we're the Amiga lock the ServiceBase semaphore
      MOVEA.L  jb_ServiceBase(A6),A3
      LEA.L    sb_ServiceSemaphore(A3),A0
      EXG.L    A6,A2
      CALLSYS  ObtainSemaphore
      EXG.L    A6,A2
      BSET     #GETS_SBSEMn,D6

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: After obtain sb_ServiceSemaphore'>

      ;------  lock the ServiceParam lock
      MOVEA.L  sb_ServiceParam(A3),A3
      MOVEA.L  A3,A0
      CALLSYS  MakeBytePtr
	move.l	d0,a0
      move.l   a0,-(sp)
      lea.l    spm_Lock(A0),A0
      CALLSYS  JanusLock
      move.l   (sp)+,a0
      BSET     #GETS_SPLOCKn,D6

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: After Lock Service Param'>

      ;------  try to find the service:  traverse the ServiceData list,
      ;------  checking that the ServiceData's SERVICE_DELETED flag isn't
      ;------  set and if it isn't then check for a match with
      ;------  ApplicationID and LocalID
      MOVE.W   spm_FirstServiceData(A3),A3
      BRA      GETCHECK_END

GETCHECK_EXIST:
      ;------  Turn offset into pointer to next ServiceData struct
      MOVE.W   A3,D0
      MOVE.L   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS  JanusOffsetToMem
      MOVEA.L  D0,A3

      ;------  If this service was already deleted, don't check it
      MOVE.W   sd_Flags(A3),D0
      BTST     #SERVICE_DELETEDn,D0
      BNE      GETCHECK_NEXT

      ;------  Check if the user-specified ID fields match the
      ;------  values of this service, and react if so
      CMP.L    sd_ApplicationID(A3),D4   ; Does this ID match?
      BNE      GETCHECK_NEXT      ; If not, check next
      CMP.W    sd_LocalID(A3),D5   ; Does LocalID match too?
      BNE      GETCHECK_NEXT      ; If not, try again
      MOVEA.L  A3,A5         ; else we have a ServiceData
      MOVEA.L  A3,A0
      CALLSYS  MakeBytePtr
	move.l	d0,a0
      MOVEA.L  A0,A3
      BRA      GET_RESOLVED      ; so let's go process it

GETCHECK_NEXT:
      MOVE.W   sd_NextServiceData(A3),A3
GETCHECK_END:
      CMP.W    #-1,A3         ; If the offset to next isn't -1
      BNE      GETCHECK_EXIST      ; then go examine next struct


GET_RESOLVED:
      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GET_RESOLVED sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC
      ;------  ON ENTRY TO GET_RESOLVED, either A5 is NULL or A5 is a
      ;------  word-access ptr to a ServiceData and A3 is a byte-access
      ;------  ptr to the ServiceData
      ;------  if match found
      ;------  if we're the Amiga and the ServiceData has no semaphore
      ;------  if MakeSDSemaphore() == NULL goto GETSERVICE_ERROR
      ;------  with a JSERV_NOAMIGAMEM error
      CMPA.L   #0,A5
      BEQ      GET_RESalpha
      TST.L    sd_Semaphore(A5)
      BNE      10$
      MOVEA.L  A5,A0
      JSR      MakeSDSemaphore
      TST.L    D0
      BNE      10$
      MOVE.L   #JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR
10$
      ;------  LockServiceData()
      MOVEA.L  A5,A0
      CALLSYS  LockServiceData
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GET_RESOLVED after lock sd'>

GET_RESalpha:

      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GET_RESalpha sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC
      ;------  if match found, the service already is defined
      ;------  if we're the Amiga and the ServiceData's AMIGA_EXCLUSIVE
      ;------  flag is set or we're the PC and the ServiceData's
      ;------  PC_EXCLUSIVE flag is set then we're about to violate
      ;------  someone else's exclusivity so goto GETSERVICE_ERROR with a
      ;------  NOTEXCLUSIVE error
      CMPA.L   #0,A5
      BEQ      GET_RESb
      MOVE.W   sd_Flags(A5),D0
      BTST     #AMIGA_EXCLUSIVEn,D0
      BNE      GS_notExclusive

      ;------  if this processor's ServiceCustomer field is non-null
      ;------  and the caller has requested GETS_EXCLUSIVE, then some
      ;------  task or handler is already attached with this service so
      ;------  the exclusivity request fails, so goto GETSERVICE_ERROR with a
      ;------  NOTEXCLUSIVE error
      TST.L    sd_FirstAmigaCustomer(A5)  ; Got any customers yet?
      BEQ      GET_RESb         ; If not, we're cool
      BTST     #GETS_EXCLUSIVEn,D3      ; Want it exclusively?
      BEQ      GET_RESb         ; If not, we're cool

GS_notExclusive:
      MOVE.L   #JSERV_NOTEXCLUSIVE,G_D0_OFFSET(SP) ; else oops
      BRA      GETSERVICE_ERROR

GET_RESb:

      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GET_RESb sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC
      ;------  if match found and AmigaUserCount > 0 or PCUserCount > 0,
      ;------  the service is available
      CMPA.L   #0,A5
      BEQ      GET_RES2
      TST.B    sd_AmigaUserCount(A3)
      BNE      10$
      TST.B    sd_PCUserCount(A3)
      BEQ      GET_RES2   ; Branch if Amiga and PC have no users
10$

      ;------  allocate and initialize a ServiceCustomer structure in
      ;------  non-janus mem if not able to allocate
      ;------  if Amiga, goto GETSERVICE_ERROR with a
      ;------  NOAMIGAMEM error
      ;------  if PC, goto GETSERVICE_ERROR with a
      ;------  NOPCMEM error
      MOVE.W   D2,D0
      MOVE.W   D3,A0      ; Send GetService() Flags
      MOVEQ.L  #0,D1      ; Designate call from GetService()
      JSR      AllocServiceCustomer
      TST.L    D0
      BNE      111$
      MOVE.L   #JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR
111$  MOVEA.L  D0,A4

      ;------  add ServiceCustomer to ServiceData list of customers
      MOVE.L   sd_FirstAmigaCustomer(A5),scs_NextCustomer(A4)
      MOVE.L   A4,sd_FirstAmigaCustomer(A5)

      ;------  if we're the Amiga increment ServiceData AmigaUserCount
      ;------  if we're the PC increment ServiceData PCUserCount
      ADDQ.B   #1,sd_AmigaUserCount(A3)

      ;------  goto GETSERVICE_DONE
      BRA      GETSERVICE_DONE


GET_RES2: 
      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GET_RES2 sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC
      ;------  if we get here we know that the service hasn't been added yet
      ;------  though it may already be requested (as described by whether
      ;------  or not a ServiceData was found).  Either no ServiceData was
      ;------  found, or a ServiceData was found but both UserCounts were
      ;------  zero if the caller hasn't specified GETS_WAIT, then the
      ;------  caller is unwilling to wait around for the service so goto
      ;------  GETSERVICE_ERROR to return the error JSERV_NOSERVICE

      BTST     #GETS_WAITn,D3
      BNE      20$
      MOVE.L   D3,D0
      ANDI.W   #GETS_WAITmask,D0
      BNE      20$


      ;------  service not added yet and caller isn't
      ;------  isn't interested in waiting around, so error
      MOVE.L   #JSERV_NOSERVICE,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR

20$   ;------  GETS_WAIT set
      ;------  else since the caller has specified GETS_WAIT, the caller is
      ;------  willing to wait around for a pending service
      ;------  if ServiceData not found, create a ServiceData structure at
      ;------  this time ServiceData = MakeServiceData()
      ;------  if ServiceData == NULL goto GETSERVICE_ERROR with
      ;------  a NOJANUSMEM error
      CMP.L    #0,A5
      BNE      SD_Set

      JSR      MakeServiceData
      TST.L    D0
      BNE      100$
      MOVE.L   #JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR
100$
      BSET     #GETS_GOTSDn,D6
      MOVEA.L  A0,A5
      CALLSYS  MakeBytePtr
	move.l	d0,a0
      MOVEA.L  A0,A3

      ;------  if we're the Amiga
      ;------  if MakeSDSemaphore(ServiceData) == NULL goto
      ;------  GETSERVICE_ERROR with a NOAMIGAMEM error
      MOVEA.L  A5,A0
      JSR      MakeSDSemaphore
      TST.L    D0
      BNE      105$
      MOVE.L   #JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR
105$

      ;------  for simplicity, lock the ServiceData here which will allow
      ;------  us to always unlock it below whether error return or not
      ;------  LockServiceData()
      MOVEA.L  A5,A0
      CALLSYS  LockServiceData

SD_Set:

      ;------  allocate and a ServiceCustomer structure in non-janus mem
      ;------  if not able to allocate
      ;------  if Amiga, goto GETSERVICE_ERROR with a
      ;------  NOAMIGAMEM error
      ;------  if PC, goto GETSERVICE_ERROR with a
      ;------  NOPCMEM error
      MOVE.W   D2,D0
      MOVE.W   D3,A0   ; send GetService() flags
      MOVEQ.L  #0,D1   ; designate that we're not calling from AddS()
      JSR      AllocServiceCustomer
      TST.L    D0
      BNE      10$
      MOVE.L   #JSERV_NOAMIGAMEM,G_D0_OFFSET(SP)
      BRA      GETSERVICE_ERROR
10$   MOVEA.L  D0,A4

      ;------  add ServiceCustomer to ServiceData list of customers
      MOVE.L   sd_FirstAmigaCustomer(A5),scs_NextCustomer(A4)
      MOVE.L   A4,sd_FirstAmigaCustomer(A5)

      ;------  goto GETSERVICE_DONE
      BRA      GETSERVICE_DONE


GETSERVICE_ERROR:
      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC

      ;------  if we allocated a ServiceCustomer, free it
      CMPA.L   #0,A4
      BEQ      nocust
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR Free customer'>
      MOVEA.L  A4,A0
      JSR      FreeServiceCustomer
      MOVEA.L  #0,A4
nocust:

      ;------  if we're the Amiga and we have a non-zero ServiceData
      ;------  pointer and there are no other customers and the semaphore
      ;------  field is non-NULL then FreeSDSemaphore()
      CMPA.L   #0,A5
      BEQ      nofreesem
      TST.L    sd_FirstAmigaCustomer(A5)
      BNE      nofreesem
      TST.L    sd_Semaphore(A5)
      BEQ      nofreesem
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR Free Semaphore'>
      MOVEA.L  A5,A0
      JSR      FreeSDSemaphore
nofreesem:

      ;------  if we created a ServiceData structure with MakeServiceData(),
      ;------  get rid of it now with UnmakeServiceData()
      BTST     #GETS_GOTSDn,D6
      BEQ      nounmake
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR Unmake Sericedata'>
      MOVEA.L  A5,A0
      JSR      UnmakeServiceData
      BCLR     #GETS_GOTSDn,D6
      BRA      ERROR_END
nounmake:

      ;------  else if we found a pre-existing ServiceData
      CMPA.L   #0,A5
      BEQ      ERROR_END

      ;------  unlock ServiceData lock
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR Ulock Sericedata'>
      move.l   a0,-(sp)
      lea.l    sd_ServiceDataLock(A3),A0
      CALLSYS  JanusUnlock
      move.l   (sp)+,a0

      ;------  if we're the Amiga
      ;------  if there are no more Amiga customers free semaphore
      TST.L    sd_Semaphore(A5)
      BEQ      semDone

      TST.L    sd_FirstAmigaCustomer(A5)
      BNE      releaseSem   ; Branch if there are others

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR free Sericedata semaphore'>
      MOVEA.L  A5,A0      ; Else free the thing up
      JSR      FreeSDSemaphore
      BRA      semDone

releaseSem:
      ;------  else unlock ServiceData semaphore
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_ERROR release Sericedata semaphore'>
      MOVEA.L  sd_Semaphore(A5),A0
      EXG.L    A6,A2
      CALLSYS  ReleaseSemaphore
      EXG.L    A6,A2

semDone:

ERROR_END:
      ;------  forget we ever knew anything about any ServiceData structure
      ;------  (set the pointer to NULL, or whatever.  it's important below)
      MOVEA.L  #0,A5

      ;------  and fall into GETSERVICE_DONE



GETSERVICE_DONE:
      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_DONE sdata = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC

      ;------  if we're returning successfully with a valid ServiceData
      ;------  pointer
      CMPA.L   #0,A5
      BEQ      nullSD

      ;------  if the caller's GETS_EXCLUSIVE flag is set
      ;------  if we're the Amiga, set the ServiceData's AMIGA_EXCLUSIVE
      ;------  flag if we're the PC, set the ServiceData's PC_EXCLUSIVE
      ;------  flag
      BTST     #GETS_EXCLUSIVEn,D3
      BEQ      notExclusive
      MOVE.W   sd_Flags(A5),D0
      BSET     #AMIGA_EXCLUSIVEn,D0
      MOVE.W   D0,sd_Flags(A5)
notExclusive:

      ;------  UnlockServiceData()
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_DONE unlock sdata'>
      MOVEA.L  A5,A0
      CALLSYS  UnlockServiceData

nullSD:


      ;------  unlock the ServiceParam lock
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_DONE unlock sparam'>
      MOVEA.L  jb_ServiceBase(A6),A0
      MOVEA.L  sb_ServiceParam(A0),A0
      CALLSYS  MakeBytePtr
	move.l	d0,a0
      lea.l    spm_Lock(A0),A0
      CALLSYS  JanusUnlock

      ;------  if we're the Amiga, unlock the ServiceBase semaphore
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_DONE release sb sem'>
      BTST     #GETS_SBSEMn,D6
      BEQ      33$
      MOVEA.L  jb_ServiceBase(A6),A0
      LEA.L    sb_ServiceSemaphore(A0),A0
      EXG.L    A6,A2
      CALLSYS  ReleaseSemaphore
      EXG.L    A6,A2
      BCLR     #GETS_SBSEMn,D6
33$

      ;------  if we're returning successfully with a ServiceData that has
      ;------  its SERVICE_ADDED flag set go triumphantly to GETSERVICE_EXIT
      CMPA.L   #0,A5      ; Do we have a service data?
      BEQ      678$      ; Branch if not
      MOVE.W   sd_Flags(A5),D0   ; Is the SERVICE_ADDED flag set?
      BTST     #SERVICE_ADDEDn,D0
      BNE      GETSERVICE_EXIT   ; Branch if set
      BRA      oksdata
678$
      ;------  We have no service data at all so set error and return
      ;------  Note we could come here from just about anywhere that
      ;------  calls GETSERVICE_ERROR.
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: GETSERVICE_DONE NOSERVICE'>
      MOVE.L   #JSERV_NOSERVICE,G_D0_OFFSET(SP)
      BRA      GETSERVICE_EXIT
oksdata:

      ;------  else we didn't find an existing service.  we found either a
      ;------  pending service (hasn't been added yet) and we are a
      ;------  customer, or we have no ServiceData at all.  let's see if
      ;------  the caller wants us to take some extraordinary action
      ;------  Stash A5, our current SD, in case of emergency
      MOVE.L   A5,G_A1_OFFSET(SP)

      ;------  Throughout this routine, except for local changes (which
      ;------  must be repaired afterwards), these registers will be
      ;------  constant:
      ;------  D2 = SignalNumber
      ;------  D3 = GetService() Flags
      ;------  D4 = ApplicationID
      ;------  D5 = LocalID
      ;------  D6 = internal flags
      ;------  D7 = the TimeOut value
      ;------  A2 = ExecBase
      ;------  A3 = if A5 has a ServiceData.WA, A3 has address of
      ;------  ServiceData.BA
      ;------  A5 = Address of ServiceData.WA, NULL until one is found
      ;------  (if ever)
      ;------  A6 = JanusBase, for janus library

      ;------  set a TimeOut value based on the AUTO_WAIT flags. every time
      ;------  we end up waiting for something to happen in the code below,
      ;------  we will wait for a second and decrement the Timeout value.
      ;------  if Timeout hits zero, we give up

      MOVE.W   D3,D0
      BTST     #GETS_WAITn,D0           ; wait forever
      BEQ      5$
      MOVE.L   #$ff,D7
      BRA      40$
5$    ANDI.W   #GETS_WAITmask,D0
      BNE      10$
      MOVEQ.L  #0,D7
      BRA      40$
10$   CMP.W    #GETS_WAIT_15,D0
      BNE      20$
      MOVE.W   #15,D7
      BRA      40$
20$   CMP.W    #GETS_WAIT_30,D0
      BNE      30$
      MOVE.W   #30,D7
      BRA      40$
30$   MOVE.W   #120,D7
40$

      ;------  if the caller has requested amiga autoload
      ;------  (GETS_ALOAD_A flag) then let's try to ask the autoloader to
      ;------  do the job for us. we start by trying to GetService() the
      ;------  autoloader
      BTST     #GETS_ALOAD_An,D3
      BEQ      aloadDone

      ;------  while TimeOut is greater than or equal to 0
      ;------  and service not gotten
      ;------  try to GetService() the autoloader, with
      ;------  Application ID of AUTOLOAD_APPID, LocalID of
      ;------  AUTOLOAD_LOCALID, and with flags of NULL

aloadGet:

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aload Get'>

      ;------  resultcode = GetService(ServiceData, AppID, LocalID,
      ;------                                          SignalNumber, Flags)
      ;------  D0:0-15                 A0           D0     D1:0-15
      ;------                                          D2:0-15     D3:0-15
      MOVE.L   D3,-(SP)
      LEA.L    -4(SP),SP
      MOVEA.L  SP,A0
      MOVE.L   #123,D0
      MOVEQ.L  #2,D1
      MOVEQ.L  #0,D3
      JSR      GetService
      MOVE.L   (SP)+,A5
      MOVE.L   (SP)+,D3
      CMP.W    #JSERV_OK,D0
      BEQ      aloadGotService
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadNotGotService'>

      ;------  if service not gotten
      ;------  decrement TimeOut
      ;------  delay for one second
      ;------  if service not gotten then we timed out while trying so
      ;------  goto GETSERVICE_EXIT with what ServiceData we managed
      ;------  to get, if any
      ;------  Still some time left?
;      SUBQ.W   #1,D7
;      BGT      123$   ; Branch if so
      ;------  Else restore our ServiceData and leave
      MOVE.L   G_A1_OFFSET(SP),A5
      BRA      GETSERVICE_EXIT
;123$
;      MOVE.L   A6,-(SP)
;      MOVE.L   jb_DOSBase(A6),A6
;      MOVEQ.L  #50,D1
;      CALLSYS  Delay
;      MOVE.L   (SP)+,A6
;      BRA      aloadGet

aloadGotService:

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadGotService'>

      ;------  alright, we got the autoloader!  next we try to send
      ;------  it our autoload request

aloadSend:
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadsend'>
      ;------  while TimeOut is greater than or equal to 0
      ;------  and we haven't been able to find a free autoloader buffer yet
      ;------   - LockServiceData(autoloader)
      MOVEA.L  A5,A0
      CALLSYS  LockServiceData

      ;------  try to find an autoloader AppID field that's zero
      MOVEA.L  sd_AmigaMemPtr(A5),A0
      MOVE.W   (A0)+,D0
      SUBQ.W   #1,D0   ; -1 for DBRA
10$   TST.L    (A0)
      BEQ      20$
      ADD.L    #6,A0
      DBRA     D0,10$
      BRA      30$
20$
      ;------  if one found write our ID's into the fields
      BSET     #GETS_SENTLOADn,D6
      MOVE.L   D4,(A0)+
      MOVE.W   D5,(A0)
30$

      ;------  UnlockServiceData(autoloader)
      MOVEA.L  A5,A0
      CALLSYS  UnlockServiceData

      ;------  if we didn't find a free autoloader field
      ;------    - decrement TimeOut
      ;------    - delay for one second
      BTST     #GETS_SENTLOADn,D6
      BNE      aloadSent   ; Branch if we sent one
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: not sent'>

      SUBQ.W   #1,D7      ; Any time left?
      BLE      aloadRelease   ; Branch if not

      ;------  Kill some time
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: kill time'>
      MOVE.L   A6,-(SP)
      MOVE.L   jb_DOSBase(A6),A6
      MOVEQ.L  #50,D1
      CALLSYS  Delay
      MOVE.L   (SP)+,A6
      BRA      aloadSend

aloadSent:

      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadSent'>

      ;------  if we were able to write our ID's into the autoloader
      ;------  buffer, then CallService() the autoloader
      IFGE     INFOLEVEL-1
      MOVE.L   A5,-(SP)
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: ServiceData $%lx '>
      LEA.L    1*4(SP),SP
      ENDC
      MOVEA.L  A5,A0
      CALLSYS  CallService
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadcalled'>

aloadRelease:
      ;------  ReleaseService() the autoloader
      MOVEA.L  A5,A0
      CALLSYS  ReleaseService
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloadreleased'>

      ;------  Restore the ServiceData
      MOVE.L   G_A1_OFFSET(SP),A5
      CMPA.L   #0,A5
      BEQ      55$
      MOVEA.L  A5,A0
      CALLSYS  MakeBytePtr
	move.l	d0,a0
      MOVEA.L  A0,A3
55$

*       - if we weren't able to find a free autoloader buffer before 
*         we timed out then just goto GETSERVICE_EXIT with the 
*         ServiceData, if any, that we've got
      BTST     #GETS_SENTLOADn,D6   ; Did we send?
      BEQ      GETSERVICE_EXIT      ; Branch if we failed to send

*       - else we managed to get the autoloader and send an 
*         autoload request, so proceed


aloadDone:
      PUTMSG   1,<'%s/sfuncs1b.asm: GetService: aloaddone'>
*    - if we haven't gotten a ServiceData, that's because either the caller 
*      didn't designate GETS_WAIT or we ran out of memory.  in either 
*      case, there's no point in hanging around any longer so if there's 
*      no ServiceData, goto GETSERVICE_EXIT now
      CMPA.L   #0,A5
      BEQ      GETSERVICE_EXIT

*    - however, if we've gotten a ServiceData, we know at this point 
*      that it's a ServiceData for a pending service (one that's not 
*      added to the system yet), so let's wait around for the 
*      remainder of the timeout period (if any) for the 
*      ServiceData's SERVICE_ADDED flag to get set 
*       - while TimeOut is greater than zero and the SERVICE_ADDED 
*         flag isn't set
*          - decrement TimeOut
*          - delay for one second

aloadWait:

      PUTMSG   3,<'%s/sfuncs1b.asm: GetService: aload wait'>

      IFGE     INFOLEVEL-3
      MOVEM.L  D7,-(SP)
      PUTMSG   3,<'%s/sfuncs1b.asm: Delay = $%lx'>
      LEA.L    1*4(SP),SP
      ENDC


      CMP.L    #$ff,D7     ; if GETS_WAIT then wait forever
      BEQ      4$
      SUBQ.W   #1,D7      ; Any time left
      BLE      GETSERVICE_ERROR   ; Branch if not
4$
      ;------  Wait for a second
      MOVE.L   A6,-(SP)
      MOVE.L   jb_DOSBase(A6),A6
      MOVEQ.L  #50,D1
      CALLSYS  Delay
      MOVE.L   (SP)+,A6

      MOVE.W   sd_Flags(A5),D0
      BTST     #SERVICE_ADDEDn,D0   ; Is the added flag set?
      BEQ      aloadWait      ; Branch if not set


GETSERVICE_EXIT:
      PUTMSG   1,<'%s/sfuncs1b.asm: GETSERVICE_EXIT:'>

      ;------  write the address of the ServiceData structure, or NULL,
      ;------  as appropriate, return result code
      MOVEA.L  G_A0_OFFSET(SP),A0   ; Get the address of the ptr
      MOVE.L   A5,(A0)

      MOVEM.L  (SP)+,GETS_REGS
      RTS

      END


