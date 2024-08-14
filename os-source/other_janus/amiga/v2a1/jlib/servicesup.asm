
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
* 8-Feb-88   =RJ Mical=     Created this file!
* 
* *****************************************************************************


   INCLUDE "assembly.i"


   NOLIST
   INCLUDE "exec/types.i"
;-----    INCLUDE "exec/nodes.i"
;-----    INCLUDE "exec/lists.i"
;-----    INCLUDE "exec/ports.i"
    INCLUDE "exec/memory.i"
;-----    INCLUDE "libraries/configvars.i"
;-----    INCLUDE "libraries/dos.i"
;----- 
;-----    INCLUDE "hardware/intbits.i"
;----- 
   INCLUDE "janus/janus.i"
;-----   INCLUDE "janusreg.i"
;-----   INCLUDE "janusvar.i"
   INCLUDE "janus/services.i"
   INCLUDE "serviceint.i"
;-----   INCLUDE "setupsig.i"
   LIST

   INCLUDE "asmsupp.i"


   XLIB   AllocMem
   XLIB   FindTask
   XLIB   FreeMem
   XLIB   Signal

   XLIB   WaitTOF

   XLIB   AllocJanusMem
   XLIB   FreeJanusMem
   XLIB   JanusMemToOffset
   XLIB   JanusOffsetToMem


   XLIB   MakeBytePtr
   XLIB   MakeWordPtr
;???   XLIB   OffsetToMem
   XLIB   FreeServiceMem


   XDEF   AllocServiceCustomer
 IFD   CHANNELING
   XDEF   ChannelMatch
 ENDC   ; of CHANNELING
   XDEF   ExpungeService
 IFND   CHANNELING
   XDEF   FindPCAdds
   XDEF   FindPCDeletes
 ENDC   ; of IFND   CHANNELING
   XDEF   FindPCToA
   XDEF   FindUnusedAToPC
   XDEF   FreeServiceCustomer
   XDEF   InformDeletedService
   XDEF   InformNewCustomers
   XDEF   MakeServiceData
   XDEF   SignalCustomers
   XDEF   UnmakeServiceData
   XDEF   WaitServiceParam



MakeServiceData:
* === MakeServiceData() =======================================================
* 
* struct MakeServiceData(ApplicationID, LocalID)
* D0,A0                  D4             D5:0-15
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

      MOVEM.L   MSD_REGS,-(SP)

      ;------   Initialize our constant registers
      MOVEQ.L   #0,D7         ; D7 has our channel bit, if any
      MOVEA.L   D7,A4         ; A4 has ServiceData, if any
      MOVEA.L   ja_ServiceBase(A6),A5   ; A5 points to ServiceBase
   

* - allocate ServiceData structure in janus parameter memory
*    - if allocation not successful goto error
      MOVE.L   #ServiceData_SIZEOF,D0
      MOVE.L   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS   AllocJanusMem
      TST.L   D0
      BEQ   MSD_ERROR
      MOVEA.L   D0,A4
      CALLSYS   JanusMemToOffset
      MOVE.W   D0,D6         ; Save the offset for later use


 IFD   CHANNELING
* - allocate a free channel bit
*    - if none found, goto error
      MOVEA.L   A5,A0
      JSR   AllocChannel
      MOVE.B   D0,D7
      BEQ   MSD_ERROR
 ENDC   ; of IFD CHANNELING


      ;------   Get a temporary byte-access pointer to ServiceData.BA
      MOVEA.L   A4,A0
      CALLSYS   MakeBytePtr
      MOVE.L   A0,A3      ; Stash ServiceData.BA in A3


* - zero the ServiceData structure
      MOVE.W   #ServiceData_SIZEOF-1,D0
10$      MOVE.B   #0,(A0)+
      DBRA   D0,10$

 IFD   CHANNELING
* - set channel bit in ServiceData structure
      MOVE.B   D7,sd_ChannelNumber(A3)
 ENDC   ; of CHANNELING


* - set the ServiceData's ApplicationID and LocalID
      MOVE.L   D4,sd_ApplicationID(A4)
      MOVE.W   D5,sd_LocalID(A4)


* - initialize the ServiceDataLock
      MOVE.B   #$7F,sd_ServiceDataLock(A3)

* - link the ServiceData structure into the ServiceParam list 
      MOVEA.L   sb_ServiceParam(A5),A0
      MOVE.W   sp_FirstServiceData(A0),sd_NextServiceData(A4)
      MOVE.W   D6,sp_FirstServiceData(A0)


* - initialize the ServiceData's JRememberKey to -1
      MOVE.W   #-1,sd_JRememberKey(A4)


* - if we're the Amiga
*    - set ServiceData Flag SERVICE_AMIGASIDE
      MOVE.W   sd_Flags(A4),D0
      BSET   #SERVICE_AMIGASIDEn,D0
      MOVE.W   D0,sd_Flags(A4)

      BRA   MSD_DONE

MSD_ERROR:
* - if ServiceData allocated, free
      CMPA.L   #0,A4
      BEQ   100$
      MOVE.L   A4,A1
      MOVE.L   #ServiceData_SIZEOF,D0
      CALLSYS   FreeJanusMem
      MOVEA.L   #0,A4
100$

 IFD   CHANNELING
* - if channel bit allocated, free
      TST.B   D7
      BEQ   200$
      MOVE.B   D7,D0
      JSR   FreeChannel
      MOVEQ.L   #0,D7
200$
 ENDC   ; of CHANNELING

MSD_DONE:
* - return address of ServiceData structure, or NULL
      MOVE.L   A4,D0
      MOVEA.L   A4,A0

      MOVEM.L   (SP)+,MSD_REGS
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

   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/UnmakeServiceData( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

      MOVEM.L   USD_REGS,-(SP)

      MOVE.L   A0,A2         ; Stash a copy

      MOVE.L   A0,D0         ; Get the offset
      CALLSYS   JanusMemToOffset
      MOVE.W   D0,D2         ; Stash a copy

      MOVEA.L   ja_ServiceBase(A6),A3   ; Get the pointer to 
      MOVEA.L   sb_ServiceParam(A3),A3   ; the ServiceParam structure

* - unlink ServiceData from system
      MOVE.W   sp_FirstServiceData(A3),D0 ; Perchance is ours first?
      CMP.W   D0,D2         ; Perchance is ours first?
      BNE   FIND_OFFSET      ;  branch if not
      MOVE.W   sd_NextServiceData(A2),sp_FirstServiceData(A3)
      BRA   SD_UNLINKED

FIND_OFFSET:   ;------   Get the address of the ServiceData structure referred 
      ;------   to by the current offset in D0, which data structure 
      ;------   is known to be not the one we're looking for, and 
      ;------   get the offset to the next structure which might be us
      CMP.W   #-1,D0         ; What the heck, 
      BEQ   SD_UNLINKED      ; for safety's sake
      MOVE.L   #MEM_WORDACCESS+MEMF_PARAMETER,D1
      CALLSYS   JanusOffsetToMem
      MOVEA.L   D0,A0
      MOVE.W   sd_NextServiceData(A0),D0 ; Perchance is next ours?
      CMP.W   D0,D2
      BNE   FIND_OFFSET      ;  branch if not
      ;------   There we are!  We found us.  Now, put our Next pointer 
      ;------   in the previous fellow's Next field, amd we're unlinked.
      MOVE.W   sd_NextServiceData(A2),sd_NextServiceData(A0)

SD_UNLINKED:


 IFD   CHANNELING
* - if ServiceData channel number is non-zero, free the bit
      MOVEA.L   A2,A0
      CALLSYS   MakeBytePtr
      MOVE.B   sd_ChannelNumber(A0),D0
      BEQ   10$
      JSR   FreeChannel
10$
 ENDC   ; of CHANNELING


* - free memory of ServiceData structure 
      MOVEA.L   A2,A1
      MOVE.L   #ServiceData_SIZEOF,D0
      CALLSYS   FreeJanusMem

      MOVEM.L   (SP)+,USD_REGS
      RTS



ExpungeService:
* === ExpungeService() ========================================================
* VOID ExpungeService(ServiceData.WA)
*                     A0
* Assembly language expects JanusBase in A6
* Expects ServiceData arg to be in word-access memory space
* 
* All users have abandoned a deleted service, so finally expunge it
* 
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked

   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/ExpungeService( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

* - if ServiceData memory is non-zero, free ServiceData memory 
      MOVEQ.L   #0,D0
      MOVE.W   sd_MemSize(A0),D0
      BEQ   10$
      MOVE.L   sd_AmigaMemPtr(A0),A1
      MOVE.L   A0,-(SP)
      CALLSYS   FreeJanusMem
      MOVE.L   (SP)+,A0
10$

* - call FreeServiceMem() to free any memory
      MOVE.L   A0,-(SP)
      MOVEQ.L   #0,D0
      CALLSYS   FreeServiceMem
      MOVE.L   (SP)+,A0


* - UnmakeServiceData(ServiceData)
      JSR   UnmakeServiceData


      RTS



InformNewCustomers:
* === InformNewCustomers() ====================================================
* VOID InformNewCustomers(ServiceData.WA)
*                         A0
* Assembly language expects JanusBase in A6
* 
* 
* This algorithm is for Amiga only, unless you implement customers on the PC
* 
* A new service has been added, so inform any customers who have been waiting 
* 
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked
* 
* - if the ServiceData's FirstCustomer field is non-zero, traverse the 
*   list and 
*    - signal the task
*    - increment UserCount

      MOVE.L   A0,-(SP)      ; Stash our word-access ptr
      JSR   SignalCustomers      ; Signal our customers
      ADDQ.B   #1,D0         ; Increase customer count by us
      MOVEA.L   (SP)+,A0      ; Restore word-access ptr
      MOVE.B   D0,-(SP)      ; Stash customer count
      CALLSYS   MakeBytePtr      ; Create byte-access pointer
      MOVE.B   (SP)+,D0      ; Restore customer count
       ADD.B   D0,sd_UserCount(A0)   ; Add customer count
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
* This routine expects that the ServiceParam lock is already locked, and 
* if this is the Amiga then that the ServiceBase semaphore is already locked
* 
* - if the ServiceData's FirstCustomer field is non-zero, traverse the 
*   list and signal each task, which wakes them up at which time they 
*   should have a look at the structure and notice that the SERVICE_DELETED
*   flag is now set

   IFGE   INFOLEVEL-59
   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/InformDeletedService( $%lx )'>
   LEA   1*4(SP),SP
   ENDC

      JSR   SignalCustomers

      RTS



AllocServiceCustomer:
* === AllocServiceCustomer() ==================================================
*
* struct ServiceCustomer = AllocServiceCustomer(SignalNumber);
* D0                                            D0
* Assembly language expects JanusBase in A6

      MOVEM.L   D2/A2/A6,-(SP)

      MOVEQ.L   #0,D2
      BSET   D0,D2

      MOVEA.L   ja_ExecBase(A6),A6

      ;------   Try to allocate the ServiceCustomer data structure
      MOVE.L   #ServiceCustomer_SIZEOF,D0
      MOVE.L   #MEMF_CLEAR,D1
      CALLSYS   AllocMem
      TST.L   D0
      BEQ   10$      ; Branch if allocation unsuccessful
      MOVE.L   D0,A2

      ;------   Initialize the ServiceCustomer structure
      MOVE.L   D2,sc_SignalBit(A2)   ; Copy the SignalBit 
      MOVE.L   #0,A1         ; Find this task's address
      CALLSYS   FindTask
      MOVE.L   D0,sc_Task(A2)
      MOVE.L   A2,D0         ; Restore D0 with the address
10$

      MOVEM.L   (SP)+,D2/A2/A6
      RTS



FreeServiceCustomer:
* === FreeServiceCustomer() ===================================================
*
* VOID FreeServiceCustomer(customer);
*                          A1
* Assembly language expects JanusBase in A6

      MOVEM.L   A6,-(SP)

      MOVEA.L   ja_ExecBase(A6),A6

      MOVE.L   #ServiceCustomer_SIZEOF,D0
      CALLSYS   FreeMem

      MOVEM.L   (SP)+,A6
      RTS



SignalCustomers:
* === SignalCustomers =========================================================
* count = SignalCustomers(ServiceData.WA);
* D0:0-7                  A0
* Assembly expects JanusBase in A6
* 
* Expects the ServiceData argument to point into word-access memory space.
* 
* Sends a signal to all ServiceData customers (if any)
* Returns the number of customers signalled

SC_REGS      REG   D2/A2-A3/A6

      MOVEM.L   SC_REGS,-(SP)

   IFGE   INFOLEVEL-2
   MOVEM.L   A0/A6,-(SP)
   PUTMSG   2,<'%s/SignalCustomers( $%lx ) with $%lx'>
   LEA.L   2*4(SP),SP
   ENDC   ; of IFGE   INFOLEVEL-2

      MOVEA.L   sd_FirstAmigaCustomer(A0),A3

      MOVEA.L   ja_ExecBase(A6),A6

   IFGE   INFOLEVEL-59
   MOVEM.L   A3/A6,-(SP)
   PUTMSG   59,<'%s/SignalCustomers Customer=$%lx ExecBase=$%lx'>
   LEA.L   2*4(SP),SP
   ENDC   ; of IFGE   INFOLEVEL-59

      MOVEA.L   #0,A1
      CALLSYS   FindTask
      MOVEA.L   D0,A2
      MOVEQ.L   #0,D2

SC_LOOP:   CMPA.L   #0,A3
      BEQ   SC_DONE

      ;------   Signal all tasks except the current one
      MOVEA.L   sc_Task(A3),A1
      CMPA.L   A1,A2
      BEQ   SC_SIGNALLED
      MOVE.L   sc_SignalBit(A3),D0
      CALLSYS   Signal
   IFGE   INFOLEVEL-59
   MOVE.L   A1,-(SP)
   PUTMSG   59,<'%s/Signalling task number $%lx'>
   LEA   1*4(SP),SP
   ENDC

      ADDQ.B   #1,D2
SC_SIGNALLED:   MOVEA.L   sc_NextCustomer(A3),A3
      BRA   SC_LOOP

SC_DONE:
      MOVE.L   D2,D0
      MOVEM.L   (SP)+,SC_REGS
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

WSP_REGS   REG   D0-D1/A0-A1/A5-A6

      MOVEM.L   WSP_REGS,-(SP)

      MOVE.L   ja_ServiceBase(A6),A5
      MOVE.L   sb_GfxBase(A5),A6

5$      TST.L   sb_ServiceParam(A5)
      BNE   10$
      ;------   Note that if you are in Forbid() when the following 
      ;------   call to WaitTOF is made, the Forbid() is broken 
      ;------   until control is returned to you, at which time the 
      ;------   Forbid() is already reestablished
      CALLSYS   WaitTOF
      BRA   5$
10$

      MOVEM.L   (SP)+,WSP_REGS
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
* IFD CHANNELING
* Returns the byte-access pointer to the available AmigaToPC[] field in 
* both A0 and D0.
* ENDC ; of IFD CHANNELING
* IFND CHANNELING
* Returns the word-access pointer to the available AmigaToPC[] field in 
* both A0 and D0.
* ENDC ; of IFND CHANNELING

      MOVEM.L   A2/A6,-(SP)

      ;------   Get a byte-access pointer to the AmigaToPC block
      MOVEA.L   ja_ServiceBase(A6),A0
      MOVEA.L   sb_GfxBase(A0),A2
      MOVEA.L   sb_ServiceParam(A0),A0
 IFD   CHANNELING
      CALLSYS   MakeBytePtr
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      ;------   Don't do anything, A0 already has the right kind of ptr
 ENDC   ; of IFND CHANNELING
      LEA   sp_AmigaToPC(A0),A0

FINDZERO:   MOVEQ.L   #SERVICE_IO_FIELDS-1,D0
 IFND   CHANNELING
      MOVEQ.L   #SERVICE_IO_FIELDS*2,D1
 ENDC   ; of IFND CHANNELING

FINDZERO_LOOP:   
 IFD   CHANNELING
      TST.B   0(A0,D0.W)
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      SUBQ.W   #2,D1
      CMP.W   #-1,0(A0,D1.W)
 ENDC   ; of IFND CHANNELING
      DBEQ   D0,FINDZERO_LOOP
      BEQ   FINDFOUND

   PUTMSG   2,<'%s/still waiting to find unused AmigaToPC[] field...'>
      MOVE.L   A0,-(SP)
      ;------   Note that if you are in Forbid() when the following 
      ;------   call to WaitTOF is made, the Forbid() is broken 
      ;------   until control is returned to you, at which time the 
      ;------   Forbid() is already reestablished
      EXG.L   A2,A6
      CALLSYS   WaitTOF
      EXG.L   A2,A6
      MOVE.L   (SP)+,A0

      BRA   FINDZERO

FINDFOUND:
 IFD   CHANNELING
      ADD.W   D0,A0
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      ADD.W   D1,A0
 ENDC   ; of IFND CHANNELING
      MOVE.L   A0,D0

      MOVEM.L   (SP)+,A2/A6

      RTS



FindPCToA:
* === FindPCToA() =========================================================
*
* UBYTE *FindPCToA()
* D0,A0
* From assembly:  JanusBase in A6
* 
* This routine tries once to find a used field of PCToAmiga[].
* 
* IFD CHANNELING
* Returns the byte-access pointer to the used PCToAmiga[] field in 
* both A0 and D0.
* ENDC ; of IFD CHANNELING
* IFND CHANNELING
* Returns the word-access pointer to the used PCToAmiga[] field in 
* both A0 and D0.
* ENDC ; of IFND CHANNELING

      ;------   Get a byte-access pointer to the AmigaToPC block
      MOVEA.L   ja_ServiceBase(A6),A0
      MOVEA.L   sb_ServiceParam(A0),A0
 IFD   CHANNELING
      CALLSYS   MakeBytePtr
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      ;------   Don't do anything, A0 already has the right kind of ptr
 ENDC   ; of IFND CHANNELING
      LEA   sp_PCToAmiga(A0),A0

FINDNZERO:   MOVEQ.L   #SERVICE_IO_FIELDS-1,D0
 IFND   CHANNELING
      MOVEQ.L   #SERVICE_IO_FIELDS*2,D1
 ENDC   ; of IFND CHANNELING

FINDNZERO_LOOP:   
 IFD   CHANNELING
      TST.B   0(A0,D0.W)
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      SUBQ.W   #2,D1
      CMP.W   #-1,0(A0,D1.W)
 ENDC   ; of IFND CHANNELING
      DBNE   D0,FINDNZERO_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ   retNULL
 IFD   CHANNELING
      ADD.W   D0,A0
 ENDC   ; of IFD CHANNELING
 IFND   CHANNELING
      ADD.W   D1,A0
 ENDC   ; of IFND CHANNELING
      BRA   retDone
retNULL:   MOVE.L   #0,A0
retDone:

      MOVE.L   A0,D0
      RTS



 IFD   CHANNELING
* === =========================================================================
* ===  Here are the Channel-Specific  Functions ===============================
* === =========================================================================

AllocChannel:
* === AllocChannel(ServiceBase) ===============================================
*
* channel = AllocChannel(ServiceBase)
* D0:0-7                 A0
* Assembly language expects JanusBase in A6
*
* This routine attempts to allocate a channel bit from ServiceParam.  
* 
* If it succeeds, it returns the channel number as a byte in D0.  
* If it fails, it returns a byte of 0 in D0
* 
* ENTRY:
*    - A0 has address of ServiceBase
* EXIT:
*    - D0 has 8 bits of a non-zero channel number, or 0 if failure


      MOVE.L   D2,-(SP)

      MOVE.L   sb_ServiceParam(A0),A0   ; Get address of ServiceParam
      ;------   sb_ServiceParam is a 
      ;------   word-access pointer, but 
      ;------   we want bytes below, so switch
      CALLSYS   MakeBytePtr
      LEA   sp_ChannelMasks(A0),A0   ; Get address of channel bytes

      MOVE.W   #31,D0         ; Prepare to check 32 bytes

10$      MOVE.B   0(A0,D0.W),D1      ; Get the next channel mask byte
      NOT.B   D1         ; Invert to test if any are free
      DBNE   D0,10$         ; If equal (all set) try next
      BEQ   NO_CHANNELS      ; Counted down D0, no channels!


      ;------   Some bit is available, so now we find it.
      ;------   We know that one of the bits was set, so the only way to 
      ;------ fall out of the below loop is on finding the set bit.
      ;------   Note that with the DBNE below, the condition is 
      ;------   tested and only if not satisfied is the register
      ;------   decremented.
      MOVE.W   #7,D2         ; Prepare to test 8 bits
20$      BTST   D2,D1         ; Test the next bit
      DBNE   D2,20$         ; Dand B until bit isn't equal

      ;------   Set the bit to designate that this channel is taken
      NOT.B   D1         ; Invert back to normal
      BSET   D2,D1         ; Set the bit
      MOVE.B   D1,0(A0,D0.W)      ; Save the new channel mask byte

* Here, D0 has the number of the byte and D2 has the number of the bit.
      LSL.W   #3,D0         ; Now D0 has the byte number * 8
      ADD.W   D2,D0         ; Now D0 has the channel number
      BRA   RETURN

NO_CHANNELS:
      MOVEQ.L   #0,D0

RETURN:
      MOVE.L   (SP)+,D2
      RTS



FreeChannel:
* === FreeChannel() ===========================================================
* 
* VOID FreeChannel(Channel)
*                  D0:0-7
* Assembly language expects JanusBase in A6
* 

      MOVE.L   D2,-(SP)

      MOVE.L   D0,D2         ; Stash the channel number

      MOVEA.L   ja_ServiceBase(A6),A0   ; A0 points to ServiceBase
      MOVE.L   sb_ServiceParam(A0),A0   ; Get address of ServiceParam
      ;------   sb_ServiceParam is a 
      ;------   word-access pointer, but 
      ;------   we want bytes below, so switch
      CALLSYS   MakeBytePtr
      LEA   sp_ChannelMasks(A0),A0   ; Get address of channel bytes

      MOVEQ.L   #0,D1         ; Clear offset register
      MOVE.B   D2,D1         ; Set 8 bits of offset
      LSR.W   #3,D1         ; Turn into a byte offset
; Don't have to do the next line, as BCLR operations to memory destinations 
; are modulo 8, sayeth the 68000 manual.  All hail 68000.   
; ------------   ANDI.W   #$0007,D2      ; Mask to make bit number
      BCLR   D2,0(A0,D1.W)      ; Clear that bit. Channel free!

      MOVE.L   (SP)+,D2

      RTS



ChannelMatch:
* ENTRY:
*     D2 = Channel number to match
*     A5 = ServicePara.WA
* 
* EXIT:
*     A3 = Word-access pointer to ServiceData of channel that matches, 
*          or NULL if no match
*     A0 = Byte-access pointer, if A3 is non-null, else undefined
* 
* USES:
*     D0, D1, A0, A1

* - find the ServiceData with this ChannelNumber:  
*   traverse the ServiceData list, 
*   checking first that the ServiceData's SERVICE_DELETED flag isn't set and 
*   if it isn't then checking for a match with ChannelNumber

      MOVE.W   sp_FirstServiceData(A5),A3
      BRA   CHANNELCHECK_END

CHANNELCHECK_EXIST:
      ;------   Turn offset into pointer to next ServiceData struct
      MOVE.W   A3,D0
      MOVE.W   #MEMF_PARAMETER+MEM_WORDACCESS,D1
      CALLSYS   JanusOffsetToMem
      MOVEA.L   D0,A3

      ;------   If this service was already deleted, don't check it
      MOVE.W   sd_Flags(A3),D0
      BTST   #SERVICE_DELETEDn,D0
      BNE   CHANNELCHECK_NEXT

      ;------   Check if the user-specified channel number matches
      ;------   this service, and react if so
      MOVEA.L   A3,A0
      CALLSYS   MakeBytePtr
      CMP.B   sd_ChannelNumber(A0),D2   ; Is this our channel number
      BEQ   CHANNEL_FOUND      ; Branch if we found it


CHANNELCHECK_NEXT:
      MOVE.W   sd_NextServiceData(A3),A3
CHANNELCHECK_END:
      CMPA.W   #-1,A3         ; If the offset to next is -1
      BEQ   CHANNEL_ERROR      ; then we're at end of list
      BRA   CHANNELCHECK_EXIST      ; else go examine next struct


CHANNEL_FOUND:
   PUTMSG   2,<'%s/Channel found'>

* Here, we know that A3 has the word-access and A0 has the byte-access pointers
* to the ServiceData of the channel
      BRA   CHANNEL_DONE


CHANNEL_ERROR:
   PUTMSG   2,<'%s/Channel not found'>

      MOVE.L   #0,A3
      MOVEA.L   A3,A0

CHANNEL_DONE:
      RTS

* === =========================================================================
* ===  End of the Channel-Specific  Functions =================================
* === =========================================================================
 ENDC   ; of IFD CHANNELING



 IFND   CHANNELING
* === =========================================================================
* ===  Here are the Non-Channel-Specific  Functions ===========================
* === =========================================================================

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

      ;------   Get a word-access pointer to the AmigaToPC block
      MOVEA.L   ja_ServiceBase(A6),A0
      MOVEA.L   sb_ServiceParam(A0),A0
      LEA   sp_PCAddsService(A0),A0

findAdds:   MOVEQ.L   #2-1,D0
      MOVEQ.L   #2*2,D1

findAdds_LOOP:   
      SUBQ.W   #2,D1
      CMP.W   #-1,0(A0,D1.W)
      DBNE   D0,findAdds_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ   10$
      ADD.W   D1,A0
      BRA   20$
10$      MOVE.L   #0,A0
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

      ;------   Get a word-access pointer to the AmigaToPC block
      MOVEA.L   ja_ServiceBase(A6),A0
      MOVEA.L   sb_ServiceParam(A0),A0
      LEA   sp_PCDeletesService(A0),A0

findDeletes:   MOVEQ.L   #2-1,D0
      MOVEQ.L   #2*2,D1

findDeletes_LOOP:   
      SUBQ.W   #2,D1
      CMP.W   #-1,0(A0,D1.W)
      DBNE   D0,findDeletes_LOOP

      ;------   If we fall out of the loop, either we found a not-equal
      ;------   byte or we found no not-equal bytes.  If none found, 
      ;-----   return NULL now
      BEQ   10$
      ADD.W   D1,A0
      BRA   20$
10$      MOVE.L   #0,A0
20$
      MOVE.L   A0,D0
      RTS



* === =========================================================================
* ===  End of the Non-Channel-Specific  Functions =============================
* === =========================================================================
 ENDC   ; of IFND CHANNELING




   IFGE   INFOLEVEL-1
   XDEF   validateServiceData
validateServiceData:
* validate the service data in A0
   MOVEM.L   D0-D7/A0-A6,-(SP)

   MOVE.L   A0,-(SP)
   PUTMSG   59,<'%s/validating ServiceData at %lx'>
   LEA.L   4(SP),SP

   MOVEQ.L   #0,D2
   MOVE.L   A0,A2
   MOVE.L   ja_ServiceBase(A6),A3
   MOVEA.L   sb_ServiceParam(A3),A4
   MOVE.W   sp_FirstServiceData(A4),D0

validLoop:
   CMP.W   #-1,D0
   BEQ   validLoopDone
   MOVE.W   #MEMF_PARAMETER+MEM_WORDACCESS,D1
   CALLSYS   JanusOffsetToMem
   CMPA.L   A2,A0
   BEQ   validSD
   MOVE.W   sd_NextServiceData(A0),D0
   BRA   validLoop

validLoopDone:
   BRA   validSDDone

validSD:
   MOVEQ.L   #1,D2

validSDDone:
   TST.B   D2
   BNE   validDone2
   MOVEM.L   A2,-(SP)
   PUTMSG   2,<'%s/ServiceData $%lx not valid, will probably crash very soon!'>
   LEA   1*4(SP),SP

validDone2
      MOVEM.L   (SP)+,D0-D7/A0-A6
      RTS
   ENDC   ; of IFGE INFOLEVEL-1


      END

