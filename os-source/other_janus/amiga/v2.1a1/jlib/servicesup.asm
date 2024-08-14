
* *****************************************************************************
* 
* Service Support Routines  --  Janus Library
* 
* Copyright (C) 1988, Commodore-Amiga, Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 26-apr-87  -BK            Fixed argument to FreeServiceMem in ExpungeServ
* 7-Oct-88   -RJ            Added *lots* of little details regarding adding 
*                           EXCLUSIVE and ONLY stuff
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 8-Feb-88   =RJ Mical=     Created this file!
* 
* *****************************************************************************


   INCLUDE "assembly.i"

   NOLIST
   INCLUDE "exec/types.i"
    INCLUDE "exec/memory.i"

   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/janusvar.i"
   INCLUDE "janus/memory.i"
   INCLUDE "janus/services.i"

    INCLUDE "serviceint.i"
   LIST

   INCLUDE "asmsupp.i"


   XLIB   AllocMem
   XLIB   Delay
   XLIB   FindTask
   XLIB   Forbid
   XLIB   FreeMem
   XLIB   InitSemaphore
   XLIB   ObtainSemaphore
   XLIB   ReleaseSemaphore
   XLIB   Permit
   XLIB   Signal

   XLIB   WaitTOF

   XLIB   AllocJanusMem
   XLIB   FreeJanusMem
   XLIB   JanusMemToOffset
   XLIB   JanusOffsetToMem


   XLIB   MakeBytePtr
   XLIB   MakeWordPtr
   XLIB   FreeServiceMem
   XLIB   JanusInitLock


   XDEF   AllocServiceCustomer
   XDEF   ExpungeService
   XDEF   FindPCAdds
   XDEF   FindPCDeletes
   XDEF   FindPCToA
   XDEF   FindUnusedAToPC
   XDEF   FreeServiceCustomer
   XDEF   FreeSDSemaphore
   XDEF   InformDeletedService
   XDEF   InformNewCustomers
   XDEF   KillServiceCustomer
   XDEF   MakeServiceData
   XDEF   MakeSDSemaphore
   XDEF   SignalCustomers
   XDEF   UnmakeServiceData
   XDEF   WaitServiceParam
   XDEF   WaitForPC
   XDEF   validateServiceData



MakeServiceData:
* === MakeServiceData() =======================================================
* 
* struct ServiceData *MakeServiceData(ApplicationID, LocalID)
* D0,A0                               D4             D5:0-15
* Assembly language expects JanusBase in A6
* 
* This routine returns a pointer to an allocated and initialized ServiceData 
* structure, or NULL if anything went wrong.  The initializations include 
* the allocation of the structure, setting of the 
* ID fields, and linking the ServiceData structure into the ServiceParam list
* 
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked
*
* ENTRY:
*    D4 = ApplicationID
*    D5 = LocalID
*    A6 = Address of JanusBase

MSD_REGS   REG   A3-A5/D6-D7

      MOVEM.L  MSD_REGS,-(SP)

      ;------  Initialize our constant registers
      MOVEQ.L  #0,D7         ; D7 has our channel bit, if any
      MOVEA.L  D7,A4         ; A4 has ServiceData, if any
      MOVEA.L  jb_ServiceBase(A6),A5   ; A5 points to ServiceBase
   

* - allocate ServiceData structure in janus parameter memory
*    - if allocation not successful goto end to return a NULL
      MOVE.L   #ServiceData_SIZEOF,D0
      MOVE.L   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS  AllocJanusMem
      TST.L    D0
      BEQ      MSD_DONE
      MOVEA.L  D0,A4
      CALLSYS  JanusMemToOffset
      MOVE.W   D0,D6      ; Save the offset for later use

      ;------  Get a temporary byte-access pointer to ServiceData.BA
      MOVEA.L  A4,A0
      CALLSYS  MakeBytePtr
      MOVE.L   A0,A3      ; Stash ServiceData.BA in A3

* - zero the ServiceData structure
      MOVE.W   #ServiceData_SIZEOF-1,D0
10$   MOVE.B   #0,(A0)+
      DBRA     D0,10$

* - set the ServiceData's ApplicationID and LocalID
      MOVE.L   D4,sd_ApplicationID(A4)
      MOVE.W   D5,sd_LocalID(A4)

* - initialize the ServiceDataLock
      move.l   a0,-(sp)
      lea      sd_ServiceDataLock(a3),a0
      CALLSYS  JanusInitLock
      move.l   (sp)+,a0

* - initialize the sd_FirstPCCustomer to $FFFFFFFF
      MOVE.L   #$FFFFFFFF,sd_FirstPCCustomer(A4)

* - link the ServiceData structure into the ServiceParam list 
      MOVEA.L  sb_ServiceParam(A5),A0
      MOVE.W   spm_FirstServiceData(A0),sd_NextServiceData(A4)
      MOVE.W   D6,spm_FirstServiceData(A0)


* - initialize the ServiceData's JRememberKey to -1
;      MOVE.W   #-1,sd_JRememberKey(A4)


* - if we're the Amiga set ServiceData Flag SERVICE_AMIGASIDE
      MOVE.W   sd_Flags(A4),D0
      BSET     #SERVICE_AMIGASIDEn,D0
      MOVE.W   D0,sd_Flags(A4)

* - goto MSD_DONE
      BRA      MSD_DONE

MSD_DONE:
* - return address of ServiceData structure, or NULL
      MOVE.L   A4,D0
      MOVEA.L  A4,A0

      MOVEM.L  (SP)+,MSD_REGS
      RTS



UnmakeServiceData:
* === UnmakeServiceData() =====================================================
* VOID UnmakeServiceData(ServiceData.WA)
*                        A0
* Assembly language expects JanusBase in A6
* Expects ServiceData arg to be in word-access memory space
* 
* Unmakes everything that MakeServiceData() makes.
* 
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked
* 

USD_REGS   REG   D2/A2-A3

   IFGE   INFOLEVEL-2
   MOVE.L   A0,-(SP)
   PUTMSG   2,<'%s/UnmakeServiceData( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

      MOVEM.L  USD_REGS,-(SP)

      MOVE.L   A0,A2         ; Stash a copy

      MOVE.L   A0,D0         ; Get the offset
      CALLSYS  JanusMemToOffset
      MOVE.W   D0,D2         ; Stash a copy

      MOVEA.L  jb_ServiceBase(A6),A3   ; Get the pointer to
      MOVEA.L  sb_ServiceParam(A3),A3   ; the ServiceParam structure

* - unlink ServiceData from system
      MOVE.W   spm_FirstServiceData(A3),D0 ; Perchance is ours first?
      CMP.W    D0,D2         ; Perchance is ours first?
      BNE      FIND_OFFSET      ;  branch if not
      MOVE.W   sd_NextServiceData(A2),spm_FirstServiceData(A3)
      BRA      SD_UNLINKED

FIND_OFFSET:   ;------   Get the address of the ServiceData structure referred 
      ;------   to by the current offset in D0, which data structure 
      ;------   is known to be not the one we're looking for, and 
      ;------   get the offset to the next structure which might be us
      CMP.W    #-1,D0         ; What the heck, 
      BEQ      SD_UNLINKED      ; for safety's sake
      MOVE.L   #MEM_WORDACCESS+MEMF_PARAMETER,D1
      CALLSYS  JanusOffsetToMem
      MOVEA.L  D0,A0
      MOVE.W   sd_NextServiceData(A0),D0 ; Perchance is next ours?
      CMP.W    D0,D2
      BNE      FIND_OFFSET      ;  branch if not
      ;------  There we are!  We found us.  Now, put our Next pointer
      ;------  in the previous fellow's Next field, amd we're unlinked.
      MOVE.W   sd_NextServiceData(A2),sd_NextServiceData(A0)

SD_UNLINKED:

* - free memory of ServiceData structure 
      MOVEA.L  A2,A1
      MOVE.L   #ServiceData_SIZEOF,D0
      CALLSYS  FreeJanusMem

      MOVEM.L  (SP)+,USD_REGS
      RTS



ExpungeService:
* === ExpungeService() ========================================================
* VOID ExpungeService(ServiceData.WA)
*                     A0
* Assembly language expects JanusBase in A6
* Expects ServiceData arg to be in word-access memory space
* 
* All users have abandoned a deleted service, so finally expunge it and 
* erase all record of it from the system.  
* 
* This is our "deadbolt" routine.  We will use this routine to make sure 
* that there are no lingering, as-yet-unreceived notifications about 
* this service.  Here's how we'll do it:  when a service is expunged, 
* whoever does the expunging will also 
* search all of the processor-to-processor IO fields in ServiceParam 
* and reset any references to this service.  Bang!  This presumes, 
* of course, that the ServiceParam is locked.  
* 
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked
* 

      MOVE.L   A2,-(SP)

      MOVEA.L  A0,A2

   IFGE   INFOLEVEL-2
   MOVE.L   A0,-(SP)
   PUTMSG   2,<'%s/ExpungeService( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

* - if ServiceData's MemSize is non-zero, free MemPtr memory 
*   by calling FreeJanusMem()
      MOVEQ.L  #0,D0
      MOVE.W   sd_MemSize(A2),D0
      BEQ      140$
      MOVE.L   sd_AmigaMemPtr(A2),A1
      CALLSYS  FreeJanusMem
140$

* - call FreeServiceMem() to auto-free any service memory
      MOVEA.L  A2,A0
*      MOVEQ.L #0,D0  Bill fix 4-26-89 Arg should be in A1
      MOVEQ.L  #0,D0
      MOVEA.L  D0,A1
      CALLSYS  FreeServiceMem


* - UnmakeServiceData(ServiceData)
      MOVEA.L  A2,A0
      JSR      UnmakeServiceData


* - remove all final traces of this service:  
*   starting from spm_PCToAmiga, the first of the ServiceParam's 
*   Amiga/PC Service IO fields, check IOFIELD_COUNT fields for a match 
*   with the offset number of this ServiceData and wherever match found 
*   set the field to -1
      MOVE.L   A2,D0         ; Get the offset of the
      CALLSYS  JanusMemToOffset   ; ServiceData
      MOVEA.L  jb_ServiceBase(A6),A0   ; Get the pointer to
      MOVEA.L  sb_ServiceParam(A0),A0   ; the ServiceParam structure
      LEA.L    spm_PCToAmiga(A0),A0   ; Get address of first field
      MOVE.W   #IOFIELD_COUNT-1,D1   ; -1 for DBRA
5$    CMP.W    (A0)+,D0
      BNE      10$
      MOVE.W   #-1,-2(A0)
10$   DBRA     D1,5$

   PUTMSG   2,<'%s/ExpungeService done'>
      MOVEA.L  (SP)+,A2
      RTS



InformNewCustomers:
* === InformNewCustomers() ====================================================
* customercount = InformNewCustomers(ServiceData.WA, OneIfAddService, From)
*                                    A0              D0.B      D1.B
* Assembly language expects JanusBase in A6
* 
* 
* This algorithm is for Amiga only, unless you implement customers on the PC
* 
* A new service has been added, so inform any customers who have been waiting 
* The From argument is the one expected by the SignalCustomers() routine 
* Returns the number of customers signalled as returned by SignalCustomer() 
* plus one if the OneIfAddedService value is non-zero, which will be 
* the case if this routine is called from AddService() (which won't be 
* the case if this routine is called because the other processor 
* added the service).  Why?  because if this routine is called from 
* AddService() then when this routine calls SignalCustomers() the current 
* task or handler wouldn't be signalled and so SignalCustomers() returns 
* a customer count one less than the true customer count.  So to get 
* the true customer count if called from AddService() then bump the 
* customer count by 1.  If you still don't understand, change careers.  
* 
* This routine expects that the ServiceData lock is already locked, and 
* if this is the Amiga then that the ServiceData semaphore is already locked
* 
* - customer count = SignalCustomers() + OneIfAddService
      MOVEM.L  A2/D2,-(SP)      ; Stash work registers
      MOVE.L   A0,A2         ; Copy our 
      MOVE.B   D0,D2         ; arguments
      MOVE.B   D1,D0         ; Copy in From arg
      JSR      SignalCustomers      ; Signal our customers
* - if we're the Amiga, AmigaUserCount = customer count
* - if we're the PC, PCUserCount = customer count
      ADD.B    D0,D2         ; Increase customer count if Add
      MOVE.L   A2,A0         ; Restore ServiceData pointer
      CALLSYS  MakeBytePtr      ; Create byte-access pointer
      ADD.B    D2,sd_AmigaUserCount(A0) ; Add customer count
      MOVEM.L  (SP)+,A2/D2      ; restore
      RTS            ; and return or crash



InformDeletedService:
* === InformDeletedService() ==================================================
* VOID InformDeletedService(ServiceData.WA)
*                           A0
* 
* This algorithm is for the Amiga side, but the idea should be much the same 
* on the PC
* 
* A service has been deleted, so signal all customers to take notice.  
* 
* This routine expects that the ServiceData lock is already locked, and 
* if this is the Amiga then that the ServiceData semaphore is already locked
* 
* - if the ServiceData's FirstCustomer field is non-zero, traverse the 
*   list and signal each task or handler, which wakes them up at which time 
*   they are supposed (by protocol) to have a look at the structure and 
*   notice that the SERVICE_DELETED flag is now set

   IFGE   INFOLEVEL-2
   MOVE.L   A0,-(SP)
   PUTMSG   2,<'%s/InformDeletedService( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

      ;------   We want to tell SignalCustomers() to ignore the 
      ;------   source of the call and just signal all customers
      MOVEQ.L  #0,D0
      JSR      SignalCustomers

      RTS



AllocServiceCustomer:
* === AllocServiceCustomer() ==================================================
*
* struct ServiceCustomer = AllocServiceCustomer(SignalNumber, Flags, FromAddS);
* D0                                            D0            A0.W   D1.W
* 
* Flags are the flags provided to AddService() or GetService()
* FromAddS is non-zero is this routine was called from AddService(), else 0
* Assembly language expects JanusBase in A6
* 

      MOVEM.L   D2-D4/A2/A6,-(SP)

   IFGE   INFOLEVEL-2
   MOVE.L   D1,-(SP)
   MOVE.L   A0,-(SP)
   MOVE.L   D0,-(SP)
   PUTMSG   2,<'%s/AllocServiceCustomer( $%lx $%lx $%lx )'>
   LEA.L   3*4(SP),SP
   ENDC   ; of IFGE   INFOLEVEL-2

      MOVEQ.L  #0,D2
      BSET     D0,D2      ; D2 gets SignalBit
      MOVE.W   D1,D3      ; D3 gets FromAddS flag
      MOVE.W   A0,D4      ; D4 gets Flags

      MOVEA.L  jb_ExecBase(A6),A6

* - if we can allocate a ServiceCustomer structure
      ;------  Try to allocate the ServiceCustomer data structure
      MOVE.L   #ServiceCustomer_SIZEOF,D0
      MOVE.L   #MEMF_CLEAR,D1
      CALLSYS  AllocMem
      TST.L    D0
      BEQ      ASC_DONE   ; Branch if allocation unsuccessful
      MOVE.L   D0,A2      ; A2 gets ServiceCustomer

      ;------   Initialize the ServiceCustomer structure
      MOVE.L   D2,scs_SignalBit(A2)   ; Copy the SignalBit 
      MOVE.L   #0,A1         ; Find this task's address
      CALLSYS  FindTask
      MOVE.L   D0,scs_Task(A2)

*    - set the customer's flags
*       - if FromAddS is non-zero, we were called from AddService() 
*         so regard the Flags argument as ADDS_ flags when 
*         setting the customer's flags
*       - else we were called from GetService() 
*         so translate the Flags argument as GETS_ flags when 
*         setting the customer's flags
      ;------   Set the customer's flags according to the arg
      TST.W    D3
      BNE      AddSFlags

GetSFlags:
      BTST     #GETS_TOPC_ONLYn,D4
      BEQ      getSF10
      BSET     #CALL_TOPC_ONLYn,D3
getSF10
      BTST     #GETS_FROMPC_ONLYn,D4
      BEQ      getSF20
      BSET     #CALL_FROMPC_ONLYn,D3
getSF20
      BTST     #GETS_TOAMIGA_ONLYn,D4
      BEQ      getSF30
      BSET     #CALL_TOAMIGA_ONLYn,D3
getSF30
      BTST     #GETS_FROMAMIGA_ONLYn,D4
      BEQ      getSF40
      BSET     #CALL_FROMAMIGA_ONLYn,D3
getSF40
      BRA      flagsDone

AddSFlags:
      MOVEQ.L  #0,D3

      BTST     #ADDS_TOPC_ONLYn,D4
      BEQ      addSF10
      BSET     #CALL_TOPC_ONLYn,D3
addSF10
      BTST     #ADDS_FROMPC_ONLYn,D4
      BEQ      addSF20
      BSET     #CALL_FROMPC_ONLYn,D3
addSF20
      BTST     #ADDS_TOAMIGA_ONLYn,D4
      BEQ      addSF30
      BSET     #CALL_TOAMIGA_ONLYn,D3
addSF30
      BTST     #ADDS_FROMAMIGA_ONLYn,D4
      BEQ      addSF40
      BSET     #CALL_FROMAMIGA_ONLYn,D3
addSF40

flagsDone:
      MOVE.W   D3,scs_Flags(A2)   ; Save the flags
      MOVE.L   A2,D0         ; Restore D0 with the address

ASC_DONE:
      MOVEA.L  D0,A0

      MOVEM.L  (SP)+,D2-D4/A2/A6
      RTS



FreeServiceCustomer:
* === FreeServiceCustomer() ===================================================
*
* VOID FreeServiceCustomer(customer);
*                          A0
* Assembly language expects JanusBase in A6
* 

      MOVEM.L  A6,-(SP)

* - free the memory of the ServiceCustomer structure 

      MOVE.L   #ServiceCustomer_SIZEOF,D0
      MOVE.L   A0,A1
      MOVEA.L  jb_ExecBase(A6),A6
      CALLSYS  FreeMem

      MOVEM.L  (SP)+,A6
      RTS



SignalCustomers:
* === SignalCustomers =========================================================
* count = SignalCustomers(ServiceData.WA, From);
* D0:0-7                  A0              D0.B
* Assembly expects JanusBase in A6
* 
* Expects the ServiceData argument to point into word-access memory space.
* 
* The From arg can be:
*    <= 0 : ignore From, signal all customers
*    == 1 : called from the other processor
*    >= 2 : called from this processor
* 
* This routine expects the ServiceData lock to be locked, and if we're 
* the Amiga the ServiceData semaphore locked too.  
* 
* Sends a signal to all ServiceData customers (if any) except the customer 
* who caused this routine to be called (if any; this routine might be called 
* in response to a signal from the other processor, not because of 
* a function call by a program on this side of the Bridge)
* 
* Returns the number of customers attached to the service less one for 
* the customer who called this routine (if this routine was called 
* by a customer)
* 

SC_REGS      REG   D2-D3/A2-A3/A6

      MOVEM.L  SC_REGS,-(SP)

   IFGE   INFOLEVEL-2
   MOVEM.L   A0/A6,-(SP)
   PUTMSG   2,<'%s/SignalCustomers( $%lx ) with $%lx'>
   LEA.L   2*4(SP),SP
   ENDC   ; of IFGE   INFOLEVEL-2

      MOVEA.L  sd_FirstAmigaCustomer(A0),A3
      MOVE.B   D0,D3

      MOVEA.L  jb_ExecBase(A6),A6

   IFGE   INFOLEVEL-3
   MOVEM.L   A3/A6,-(SP)
   PUTMSG   3,<'%s/SignalCustomers Customer=$%lx ExecBase=$%lx'>
   LEA.L   2*4(SP),SP
   ENDC

      MOVEA.L  #0,A1
      CALLSYS  FindTask
      MOVEA.L  D0,A2
      MOVEQ.L  #0,D2

      SUBQ.B   #1,D3      ; If From arg <= 0
      BMI      SC_LOOP      ; do the simple version of the loop
      SUBQ.B   #1,D3      ; else if From arg is 1
      BMI      SC_PC_LOOP   ; do the PC version of the loop
      ;------  else do the Amiga version of the loop

SC_AMIGA_LOOP:
* - if From argument equals 2 this routine was entered because of a call 
*   that started on this side of the Bridge 
*    - check each customer and if the customer's CALL_FROMother_ONLY 
*     flag is set then don't signal, else go ahead and signal the customer
      CMPA.L   #0,A3
      BEQ      SC_DONE

      ;------  Consider signalling all tasks except the current one
      ;------  (which task *might* be the ServiceTask)
      MOVEA.L  scs_Task(A3),A1
      CMPA.L   A1,A2
      BEQ      AMIGA_CUSTDONE

      ;------  The Amiga is the source of this SignalCustomers()
      ;------  If this customer wants signals only from the PC
      ;------  then don't signal but count anyway
      MOVE.W   scs_Flags(A3),D0
      BTST     #CALL_FROMPC_ONLYn,D0
      BNE      AMIGA_CUSTCOUNT

      ;------  OK, signal away
      MOVE.L   scs_SignalBit(A3),D0
      CALLSYS  Signal

   IFGE   INFOLEVEL-3
   MOVE.L   A1,-(SP)
   PUTMSG   3,<'%s/Signalling task number $%lx'>
   LEA   1*4(SP),SP
   ENDC

AMIGA_CUSTCOUNT:
      ADDQ.B   #1,D2
AMIGA_CUSTDONE:
      MOVEA.L  scs_NextCustomer(A3),A3
      BRA      SC_AMIGA_LOOP


SC_PC_LOOP:
* - if From argument equals 1 this routine was entered because of a call 
*   that started on the other side of the Bridge
*    - check each customer and if the customer's CALL_FROMus_ONLY 
*      flag is set then don't signal, else signal the customer
      CMPA.L   #0,A3
      BEQ      SC_DONE

      ;------   Consider signalling all tasks except the current one
      MOVEA.L  scs_Task(A3),A1
      CMPA.L   A1,A2
      BEQ      PC_CUSTDONE

      ;------   The PC is the source of this SignalCustomers()
      ;------   If this customer wants signals only from the Amiga
      ;------   then don't signal but count anyway
      MOVE.W   scs_Flags(A3),D0
      BTST     #CALL_FROMAMIGA_ONLYn,D0
      BNE      PC_CUSTCOUNT

      ;------   OK, signal away
      MOVE.L   scs_SignalBit(A3),D0
      CALLSYS  Signal

   IFGE   INFOLEVEL-3
   MOVE.L   A1,-(SP)
   PUTMSG   3,<'%s/Signalling task number $%lx'>
   LEA   1*4(SP),SP
   ENDC

PC_CUSTCOUNT:
      ADDQ.B   #1,D2
PC_CUSTDONE:
      MOVEA.L  scs_NextCustomer(A3),A3
      BRA      SC_PC_LOOP


SC_LOOP:
* - else send a signal to *all* customers
      CMPA.L   #0,A3
      BEQ      SC_DONE

      ;------   Consider signalling all tasks except the current one
      ;------   (which task *might* be the ServiceTask)
      MOVEA.L  scs_Task(A3),A1
      CMPA.L   A1,A2
      BEQ      SC_CUSTDONE
      MOVE.L   scs_SignalBit(A3),D0
      CALLSYS  Signal

   IFGE   INFOLEVEL-3
   MOVE.L   A1,-(SP)
   PUTMSG   3,<'%s/Signalling task number $%lx'>
   LEA   1*4(SP),SP
   ENDC

SC_CUSTCOUNT:  ADDQ.B   #1,D2
SC_CUSTDONE:   MOVEA.L  scs_NextCustomer(A3),A3
      BRA      SC_LOOP

SC_DONE:
      MOVE.L   D2,D0
      MOVEM.L  (SP)+,SC_REGS
      RTS



WaitServiceParam:
* *****************************************************************************
* 
* VOID WaitServiceParam();
* Expects JanusBase in A6
* 
* Waits politely until the ServiceTask has initialized the ServiceParam 
* structure.
* 
* This routine is advertised as being completely non-register-destructive,
* so do it or die!
* 

WSP_REGS   REG   D0-D1/A0-A1/A5-A6

      MOVEM.L  WSP_REGS,-(SP)

      MOVE.L   jb_ServiceBase(A6),A5
      MOVE.L   sb_GfxBase(A5),A6

5$    TST.L    sb_ServiceParam(A5)
      BNE      10$
      ;------   Note that if you are in Forbid() when the following 
      ;------   call to WaitTOF is made, the Forbid() is broken 
      ;------   until control is returned to you, at which time the 
      ;------   Forbid() is already reestablished
      CALLSYS  WaitTOF
      BRA      5$
10$

      MOVEM.L  (SP)+,WSP_REGS
      RTS


      
FindUnusedAToPC:
* === FindUnusedAToPC() ============================================================
*
* UBYTE *FindUnusedAToPC()
* D0,A0
* From assembly:  JanusBase in A6
* 
* This routine tries and tries, waiting politely between tries, until it finds 
* an unused field of AmigaToPC[].
* 
* You *must* have the ServiceBase semaphore 
* locked when entering, though bear in mind that this semaphore will 
* be unlocked if no field is available and then, after a delay, 
* the semaphore will be locked again before looking again for 
* an unused field.  What this buys is this:  once you've found a free 
* AmigaToPC[] field, no other Amiga task will get in and find the same 
* field free.  Meanwhile, if some hoser has something that the PC needs 
* to proceed, by doing a ReleaseSemaphore()/Delay()/ObtainSemaphore cycle 
* you give someone else a chance to get in there and satisfy the 
* much-convoluted needs of everyone!  
* 
* You cannot have any locks locked when you enter this routine.  
* 
* Returns the word-access pointer to the available AmigaToPC[] field in 
* both A0 and D0.
* 

      MOVEM.L  A2-A4/A6,-(SP)

* A0 = address of first ServiceParam AmigaToPC[] field
* A2 = GfxBase
* A3 = address of ServiceBase semaphore 
* A4 = ExecBase
      MOVEA.L  jb_ExecBase(A6),A4
      MOVEA.L  jb_ServiceBase(A6),A3
      MOVEA.L  sb_GfxBase(A3),A2
      MOVEA.L  sb_ServiceParam(A3),A0
      LEA.L    sb_ServiceSemaphore(A3),A3
      ;------   Don't do anything, A0 already has the right kind of ptr
      LEA.L    spm_AmigaToPC(A0),A0

FINDZERO:   MOVEQ.L   #SERVICE_IO_FIELDS-1,D0
      MOVEQ.L  #SERVICE_IO_FIELDS*2,D1

FINDZERO_LOOP:   
      SUBQ.W   #2,D1
      CMP.W    #-1,0(A0,D1.W)
      DBEQ     D0,FINDZERO_LOOP
      BEQ      FINDFOUND

   PUTMSG   3,<'%s/still waiting to find unused AmigaToPC[] field...'>
      MOVE.L   A0,-(SP)

   PUTMSG   2,<'%s/FindUnusedAToPC() ReleaseSemaphore(sb_ServiceSemaphore)'>
      MOVEA.L  A3,A0
      MOVEA.L  A4,A6
      CALLSYS  ReleaseSemaphore

      ;------   Note that if you are in Forbid() when the following 
      ;------   call to WaitTOF is made, the Forbid() is broken 
      ;------   until control is returned to you, at which time the 
      ;------   Forbid() is already reestablished
      MOVEA.L  A2,A6
      CALLSYS  WaitTOF

      MOVEA.L  A3,A0
      MOVEA.L  A4,A6
      CALLSYS  ObtainSemaphore

      MOVE.L   (SP)+,A0

      BRA      FINDZERO

FINDFOUND:
      ADD.W    D1,A0
      MOVE.L   A0,D0

      MOVEM.L  (SP)+,A2-A4/A6

      RTS



FindPCToA:
* === FindPCToA() =========================================================
*
* offset = FindPCToA()
* D0:0-15
* From assembly:  JanusBase in A6
* 
* This routine tries once to find an offset in any field of PCToAmiga[].  
* If an offset is found, this routine checks all fields in PCToAmiga[] 
* and all that match the offset are set to -1.  
* 
* Returns the offset found in the used PCToAmiga[] field in D0.W.  
* If no offset was found, returns -1.  
* 

      MOVE.L   D2,-(SP)

      ;------   Get a byte-access pointer to the AmigaToPC block
      MOVEA.L  jb_ServiceBase(A6),A0
      MOVEA.L  sb_ServiceParam(A0),A0
      ;------   Don't do anything, A0 already has the right kind of ptr
      LEA.L    spm_PCToAmiga(A0),A1

      MOVEA.L  A1,A0
      MOVEQ.L  #SERVICE_IO_FIELDS-1,D0
      MOVEQ.L  #SERVICE_IO_FIELDS*2,D1

FINDNZERO_LOOP:   

      SUBQ.W   #2,D1
      CMP.W    #-1,0(A0,D1.W)
      DBNE     D0,FINDNZERO_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ      retError
      ADD.W    D1,A0

      MOVE.W   (A0),D0

      MOVEA.L  A1,A0
      MOVEQ.L  #SERVICE_IO_FIELDS-1,D1

FINDMATCH_LOOP:   

      MOVE.W   (A0),D2
      CMP.W    D0,D2
      BNE      10$
      MOVE.W   #-1,(A0)
10$   ADDQ.L   #2,A0
      DBRA     D1,FINDMATCH_LOOP

      BRA      retDone

retError:   MOVE.W   #-1,D0

retDone:

      MOVE.L   (SP)+,D2

      RTS


FindPCAdds:
* === FindPCAdds() =========================================================
*
* UBYTE *FindPCAdds()
* D0,A0
* From assembly:  JanusBase in A6
* 
* This routine tries once to find a used field of PCAddsService[].
* 
* Returns the word-access pointer to the used PCAddsService[] field in 
* both A0 and D0.
* 

      ;------   Get a word-access pointer to the AmigaToPC block
      MOVEA.L  jb_ServiceBase(A6),A0
      MOVEA.L  sb_ServiceParam(A0),A0
      LEA      spm_PCAddsService(A0),A0

findAdds:   MOVEQ.L   #2-1,D0
      MOVEQ.L  #2*2,D1

findAdds_LOOP:   
      SUBQ.W   #2,D1
      CMP.W    #-1,0(A0,D1.W)
      DBNE     D0,findAdds_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ      10$      ; Branch if all were -1
      ADD.W    D1,A0
      BRA      20$
10$   MOVE.L   #0,A0
20$
      MOVE.L   A0,D0
      RTS



FindPCDeletes:
* === FindPCDeletes() =========================================================
*
* UBYTE *FindPCDeletes()
* D0,A0
* From assembly:  JanusBase in A6
* 
* This routine tries once to find a used field of PCDeletesService[].
* 
* Returns the word-access pointer to the used PCDeletesService[] field in 
* both A0 and D0.
* 

      ;------   Get a word-access pointer to the AmigaToPC block
      MOVEA.L  jb_ServiceBase(A6),A0
      MOVEA.L  sb_ServiceParam(A0),A0
      LEA      spm_PCDeletesService(A0),A0

findDeletes:   MOVEQ.L   #2-1,D0
      MOVEQ.L  #2*2,D1

findDeletes_LOOP:   
      SUBQ.W   #2,D1
      CMP.W    #-1,0(A0,D1.W)
      DBNE     D0,findDeletes_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ      10$
      ADD.W    D1,A0
      BRA      20$
10$   MOVE.L   #0,A0
20$
      MOVE.L   A0,D0
      RTS




WaitForPC:
* === WaitForPC() ===========================================================
* Waits until the PC is ready to process service interrupts
* 
* Expects JanusBase in A6
* 
* This routine is advertised as being completely non-register-destructive,
* so do it or die!
* 

WFPC_REGS   REG   A0-A3/D0-D1

      MOVEM.L  WFPC_REGS,-(SP)

   PUTMSG   2,<'%s/Do we need to WaitForPC()?'>

* - if the PC_READY flag isn't set yet

      MOVE.L   jb_ParamMem(A6),A0   ; Get the address of JanusAmiga
      CALLSYS  MakeWordPtr      ; Use word access
      MOVEA.L  A0,A2
      MOVEA.L  jb_ExecBase(A6),A3

      MOVE.W   ja_AmigaState(A2),D0
      BTST     #AMIGA_PC_READYn,D0
      BNE      WFPC_DONE

   PUTMSG   3,<'%s/PC not ready yet, so enter wait logic'>

*    - is the PC handler loaded yet?
WFPC_WAITLOAD:
      TST.W    ja_HandlerLoaded(A2)
      BNE      WFPC_LOADED

   PUTMSG   4,<'%s/PC not loaded yet, so wait 1 second'>

*       - if not, wait a second and then try again
      MOVE.L   #50,D1
      JSR      DelayWait
      BRA      WFPC_WAITLOAD

WFPC_LOADED:
   PUTMSG   3,<'%s/PC handler loaded, so wait 5 second'>

*    - PC handler is loaded, so wait 5 seconds
      MOVE.L   #5*50,D1
      JSR      DelayWait

   PUTMSG   3,<'%s/Sure hope PC is ready!  Set AMIGA_PC_READY flag'>

*    - Since we only have to lock out other Amiga programs, Forbid();
      EXG.L    A6,A3
      CALLSYS  Forbid

*    - set the PC_READY flag
      MOVE.W   ja_AmigaState(A2),D0
      BSET     #AMIGA_PC_READYn,D0
      MOVE.W   D0,ja_AmigaState(A2)

*   - Permit();
      CALLSYS  Permit
      EXG.L    A6,A3

WFPC_DONE:
   PUTMSG   2,<'%s/Seems that PC is ready...'>

      MOVEM.L  (SP)+,WFPC_REGS
      RTS


DelayWait:
* === DelayWait(ticks) ======================================================
* DelayWait(ticks)
*           D1
* Expects JanusBase in A6
* 
      MOVE.L   A6,-(SP)
      MOVEA.L  jb_DOSBase(A6),A6
      CALLSYS  Delay
      MOVE.L   (SP)+,A6
      RTS


MakeSDSemaphore:
* === MakeSDSemaphore() =====================================================
* semaphore = MakeSDSemaphore(ServiceData.WA)
* D0,A0                       A0
* Expects JanusBase in A6
* 
* If the ServiceData's sd_Semaphore field is NULL, 
* allocates and initializes a SignalSemaphore and if 
* successful attaches it to the ServiceData
* 
* Returns the address of the semaphore or NULL
* 

MSDS_REGS   REG   A2/A6

      MOVEM.L  MSDS_REGS,-(SP)

      MOVE.L   sd_Semaphore(A0),D0
      BNE      MSDS_DONE

      MOVEA.L  A0,A2
      MOVEA.L  jb_ExecBase(A6),A6

* - semaphore = AllocMem()
* - if semaphore == NULL goto MSDS_DONE
      MOVE.L   #SS_SIZE,D0
      MOVEQ.L  #0,D1
      CALLSYS  AllocMem
      TST.L    D0
      BEQ      MSDS_DONE

* - ServiceData->sd_Semaphore = semaphore
* - InitSemaphore(semaphore)
      MOVE.L   D0,sd_Semaphore(A2)
      MOVEA.L  D0,A0
      CALLSYS  InitSemaphore
      MOVE.L   sd_Semaphore(A2),D0

MSDS_DONE:
      MOVEM.L  (SP)+,MSDS_REGS
      RTS



KillServiceCustomer:
* === KillServiceCustomer() =================================================
* KillServiceCustomer(ServiceData.WA)
*                       A0
* Expects ExecBase in A2
* Expects JanusBase in A6
* 
* This routine expects the ServiceData lock to be locked, and if we're 
* the Amiga the ServiceData semaphore locked too.  
* 
* If we can find a customer that matches the caller, unlink the 
* customer from the ServiceData and free up the ServiceCustomer memory
* 

DSC_REGS   REG   A3-A4

      MOVEM.L  DSC_REGS,-(SP)

      MOVEA.L  A0,A4

* - if we can find a ServiceCustomer structure that matches the caller
      MOVEA.L  #0,A1      ; Get the address of this task into D0
      EXG      A2,A6
      CALLSYS  FindTask
      EXG      A2,A6
      MOVEA.L  #0,A3      ; No struct has been examined yet
      MOVEA.L  sd_FirstAmigaCustomer(A4),A1

DELTASK_LOOP:  CMPA.L   #0,A1      ; Did we reach the end of the Customers?
      BEQ      DELCUST_DONE   ; branch if so
      CMP.L    scs_Task(A1),D0   ; Compare customer's task with caller's
      BEQ      DELTASK_FOUND   ; If equal, this is the one we want!
      MOVEA.L  A1,A3      ; else save this struct address
      MOVEA.L  scs_NextCustomer(A1),A1 ; get the next struct
      BRA      DELTASK_LOOP   ; and go try again

DELTASK_FOUND:
* Here, A1 has the address of this task's ServiceCustomer structure, and A3 
* has the address of the previous structure examined or NULL if no 
* structure was previously examined
*    - unlink the ServiceCustomer structure
      CMPA.L   #0,A3      ; Did we see some other structure?
      BNE      10$      ; Branch if yes
      ;------   else stuff ptr to next into ServiceData's first ptr
      MOVE.L   scs_NextCustomer(A1),sd_FirstAmigaCustomer(A4)
      BRA      20$
10$   MOVE.L   scs_NextCustomer(A1),scs_NextCustomer(A3)
20$

*    - free the memory of the ServiceCustomer structure
      MOVEA.L  A1,A0
      JSR      FreeServiceCustomer

DELCUST_DONE:

      MOVEM.L  (SP)+,DSC_REGS
      RTS



FreeSDSemaphore:
* Frees the ServiceData's semaphore, sets the sd_Semaphore field to NULL 
* A0 = ServiceData.WA
* A2 = ExecBase
* 
      MOVEA.L  sd_Semaphore(A0),A1
      MOVE.L   #0,sd_Semaphore(A0)
      MOVEQ.L  #SS_SIZE,D0
      EXG      A2,A6
      CALLSYS  FreeMem
      EXG      A2,A6
      RTS



validateServiceData:
* validate the service data in A0

   IFGE  INFOLEVEL-1   ; this includes the body of validate routine

      MOVEM.L  D0-D7/A0-A6,-(SP)

      MOVE.L   A0,-(SP)
      PUTMSG   2,<'%s/validating ServiceData at %lx'>
      LEA.L    4(SP),SP

      MOVEQ.L  #0,D2
      MOVE.L   A0,A2
      MOVE.L   jb_ServiceBase(A6),A3
      MOVEA.L  sb_ServiceParam(A3),A4
      MOVE.W   spm_FirstServiceData(A4),D0

validLoop:
      CMP.W    #-1,D0
      BEQ      validLoopDone
      MOVE.W   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS  JanusOffsetToMem
      CMPA.L   A2,A0
      BEQ      validSD
      MOVE.W   sd_NextServiceData(A0),D0
      BRA      validLoop

validLoopDone:
      BRA      validSDDone

validSD:
      MOVEQ.L  #1,D2

validSDDone:
      TST.B    D2
      BNE      validDone2
      MOVEM.L  A2,-(SP)
      PUTMSG   3,<'%s/ServiceData $%lx not valid, will probably crash very soon!'>
      LEA      1*4(SP),SP

validDone2
      MOVEM.L  (SP)+,D0-D7/A0-A6

   ENDC   ; of IFGE INFOLEVEL-1 ; includes the body of validate routine

      RTS


      END



