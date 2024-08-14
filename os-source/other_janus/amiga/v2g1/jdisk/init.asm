;********************************************************************
;
; init.asm
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;********************************************************************


        INCLUDE "assembly.i"

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/memory.i"
        INCLUDE "../janus/i86block.i"
        INCLUDE "../janus/janus.i"
        INCLUDE "../janus/setupsig.i"
        INCLUDE "../janus/services.i"
        LIST

        INCLUDE "jddata.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XDEF    InitTable
                
        XLVO    AllocSignal
        XLVO    CloseLibrary
        XLVO    Disable
        XLVO    Enable
        XLVO    FreeMem
        XLVO    FreeSignal
        XLVO    OpenLibrary 
        XLVO    Wait
        XLVO    AllocMem

        XLVO    AllocJanusMem
        XLVO    CleanupJanusSig
        XLVO    FreeJanusMem
        XLVO    GetParamOffset
        XLVO    JanusMemToOffset
        XLVO    SendJanusInt
        XLVO    SetJanusHandler
        XLVO    SetJanusEnable
        XLVO    SetParamOffset
        XLVO    SetupJanusSig


        XREF    JDOpen
        XREF    JDClose
        XREF    JDExpunge
        XREF    JDNull
        XREF    JDBeginIO
        XREF    JDAbortIO

        XREF    JDiskName
        XREF    VERNUM
        XREF    REVNUM

DEBUG_CODE EQU 1

InitTable:
                dc.l    JDiskDevice_SIZEOF
                dc.l    funcTable
                dc.l    initStructData
                dc.l    init

funcTable:
                dc.l    JDOpen
                dc.l    JDClose
                dc.l    JDExpunge
                dc.l    JDNull
                dc.l    JDBeginIO
                dc.l    JDAbortIO   

                dc.l    -1


init:           ; ( d0: lib node, a0: seg list )
                movem.l d2-d4/a2-a6,-(sp)
        DEBUGINIT
                ;------ do some low level initialization
                move.l  d0,a5
                move.l  a0,jd_Segment(a5)       ; stash segment for DOS
                move.l  a6,jd_ExecBase(a5)      ; stash Exec for us
                lea     JDiskName(pc),a0        ; give cmd port a name
                move.l  a0,LN_NAME(a5)
                move.l  a0,jd_CmdTerm+LN_NAME(a5)
                lea     jd_CmdQueue(a5),a0
                NEWLIST a0
                
                ;------ get janus.library                
                lea     jlName(pc),a1
                moveq   #0,d0
                CALLLVO OpenLibrary
                move.l  d0,jd_JanusBase(a5)
                beq     initErr                 ; library not found

;------ allocate the parameter memory and track buffer for jdisk.device ----
                moveq   #AmigaDskReq_SIZEOF,d0  ; get a diskrequest
                move.l  #MEMF_PARAMETER!MEM_WORDACCESS,d1
                move.l  jd_JanusBase(a5),a6
                CALLLVO AllocJanusMem
                move.l  d0,jd_AmigaDskReq(a5)   ; save in jdisk dev struct
                beq     initErr1                ; didn't get it
                move.l  d0,a2                   ; stash for later

                move.l  #TB_SIZE,d0             ; allocate track buffer
                move.l  #MEMF_BUFFER!MEM_BYTEACCESS,d1
                CALLLVO AllocJanusMem           ; in byte access memory
                move.l  d0,jd_TrackBuffer(a5)   ; and save the pointer
                beq     initErr2                ; didn't get it
                CALLLVO JanusMemToOffset        ; convert physical to offset
                move.w  d0,adr_BufferAddr(a2)   ; set up diskreq buffer

;------ setup signal handshake for this initialization code ----------------
                movea.l jd_ExecBase(a5),a6
                moveq   #-1,D0
                CALLLVO AllocSignal
                move.l  d0,d1
                move.l  d0,d4                   ; save signal to free later
                moveq   #JSERV_HARDDISK,D0
                moveq   #AmigaDskReq_SIZEOF,D2
                move.l  #MEMF_PARAMETER!MEM_WORDACCESS,d3
                move.l  jd_JanusBase(a5),a6
                CALLLVO SetupJanusSig
                tst.l   d0
                beq     initErr3          
                move.l  d0,a3                   ; save signal structure

;------ find out how many partitions there are on the PC hard disk(s) ------
                move.l  ss_ParamPtr(a3),a4      ; get parameter block ptr
                move.w  #ADR_FNCTN_INIT,adr_Fnctn(a4)
                moveq   #JSERV_HARDDISK,D0
                CALLLVO SendJanusInt            ; OK, do this please
                move.l  ss_SigMask(a3),d0
                move.l  jd_ExecBase(a5),a6
                CALLLVO Wait                    ; wait for the reply
                tst.w   adr_Err(a4)
                bne     initErr4                ; this should never happen !

;------ now get info for each partition on the hard disk(s) ----------------
                move.w  adr_Part(a4),d2         ; number of partitions
                subq.w  #1,d2                   ; change to 0 origin
                blt     initErr4                ; no partitions for us
                
                moveq   #DskPartition_SIZEOF,d0 ; get a partition struct
                move.l  #MEMF_BUFFER!MEM_WORDACCESS,d1
                move.l  jd_JanusBase(a5),a6
                CALLLVO AllocJanusMem
                tst.l   d0
                beq     initErr4                ; damn! didn't get it
                move.l  d0,a2                   ; a2 points to disk request
                CALLLVO JanusMemToOffset
                move.w  d0,adr_BufferAddr(a4)   ; link into disk part struct
                move.w  #ADR_FNCTN_INFO,adr_Fnctn(A4)
                clr.l   jd_Units(a5)            ; nothing found yet

;------ loop for each partition (d2 holds the number of partitions - 1) -----
SizeLoop        move.l  #DskPartition_SIZEOF,adr_Count(a4)
                move.w  d2,adr_Part(a4)         ; get info on this partition
                moveq   #JSERV_HARDDISK,d0
                CALLLVO SendJanusInt
                move.l  ss_SigMask(a3),d0
                move.l  jd_ExecBase(a5),a6
                CALLLVO Wait                    ; wait for PC to reply
                tst.w   adr_Err(a4)             ; did it work OK ?
                bne     initErr5                ; nope, free up and exit

;------ allocate memory for this units bad block maps -----------------------
                move.l  #BB_SIZEOF,d0           ; allocate the mem required
                move.l  #MEMF_PUBLIC!MEMF_CLEAR,d1
                jsr     _LVOAllocMem(a6)
                tst.l   d0                      ; did we get the memory ?
                beq     initErr5                ; nope, clean up and exit
                movea.l jd_Units(a5),a0         ; link into device
                cmpa.w  #0,a0
                bne.s   10$                     ; not the first entry
                move.l  d0,jd_Units(a5)         ; this is the first
                bra.s   30$
10$             tst.l   BB_Next(a0)             ; any more units
                beq.s   20$                     ; nope, link in here
                movea.l BB_Next(a0),a0          ; move to the next
                bra.s   10$                     ; and keep on truckin'
20$             move.l  d0,BB_Next(a0)          ; link to prev unit struct

;------ fill in important variables for this unit --------------------------
30$             movea.l d0,a0
                move.l  #-1,BB_DummyEntry(a0)   ; last entry always -1
                move.w  d2,BB_Unit(a0)          ; it's for this unit #
                move.w  dp_EndCyl(a2),d0        ; calculate where ...
                sub.w   dp_BaseCyl(a2),d0       ; ... bad block map may be
                subq.w  #1,d0                   ; it takes up 2 cylinders
                move.w  dp_NumSecs(a2),d1       ; get offset to bad blocks
                mulu.w  dp_NumHeads(a2),d1      ; d0 contains #cylinders
                mulu.w  d1,d0                   ; d0=base blk for bad blks
                asl.l   #8,d0                   ; calculate byte offset
                asl.l   #1,d0                   ; 512 bytes/block
                move.l  d0,BB_BadMapOffset(a0)
                move.l  d0,BB_RealMapOffset(a0) ; where first block really is

                move.w  dp_NumSecs(a2),d1       ; how many blocks in bad area
                mulu.w  dp_NumHeads(a2),d1
                asl.w   #1,d1                   ; number of blocks
                cmpi.w  #MAXMAPPED,d1           ; can we fit this many...
                ble.s   31$                     ; in our BBM struct ?
                move.w  #MAXMAPPED,d1           ; no, fixup size of ...
31$             move.w  d1,BB_TotalBlocks(a0)   ; ... the bad block map

;------ go back for the next partition -------------------------------------
                move.l  jd_JanusBase(a5),a6     ; get back janus lib ptr
                dbra    d2,SizeLoop             ; and keep going

;------ we don't need partition info any more ------------------------------
                movea.l a2,a1                   ; get partition pointer
                moveq.l #DskPartition_SIZEOF,d0
                CALLLVO FreeJanusMem            ; free up the memory used

;------ get bad block info from each partition (if it's there) -------------
                move.l  jd_TrackBuffer(a5),d0   ; read to the track buffer
                CALLLVO JanusMemToOffset        ; convert physical to offset
                move.w  d0,adr_BufferAddr(a4)   ; set up diskreq buffer
                movea.l jd_Units(a5),a2         ; follow list of units

;------ try to read the bad block map.  If it's not at the current real
;------ offset, then bump by one sector and try 16 more times.  If it's
;------ still not found then it isn't there and format will use the
;------ tentative offset to write it out when it is called and update
;------ the real offset accordingly if the first attempt(s) fail.
GetBadBlocks    move.w  BB_Unit(a2),adr_Part(a4) ; this partition
                moveq.l #16-1,d2                ; retry count
GetUnitBad      move.w  #ADR_FNCTN_READ,adr_Fnctn(a4)
                move.l  #1024,adr_Count(a4)     ; read 2 sectors
                move.l  BB_RealMapOffset(a2),adr_Offset(a4)
                moveq.l #JSERV_HARDDISK,d0
                move.l  jd_JanusBase(a5),a6
                CALLLVO SendJanusInt            ; do the read
                move.l  ss_SigMask(a3),d0       ; get signal mask
                move.l  jd_ExecBase(a5),a6
                CALLLVO Wait                    ; wait for PC to reply
                movea.l jd_TrackBuffer(a5),a0   ; see if this is valid
                cmpi.l  #$DEFACED,(a0)          ; actually BB_Valid
                beq     GotUnit                 ; OK, this unit set up
                addi.l  #512,BB_RealMapOffset(a2)
                dbra    d2,GetUnitBad           ; try again
                bra     NextUnit                ; not here, move to next

;----- got the bad block map, copy it to the unit buffer -------------------
GotUnit         lea.l   BB_Valid(a2),a1         ; place to copy to
                move.w  #1024/4-1,d0            ; 2 sectors worth of data
10$             move.l  (a0)+,(a1)+             ; copy the data
                dbra    d0,10$
 printf <'bad at %ld mapped to %ld\n'>,BB_BlockMaps(a2),BB_BlockMaps+4(a2)
NextUnit        movea.l BB_Next(a2),a2          ; point to next unit
                cmpa.w  #0,a2                   ; did we reach the end
                bne     GetBadBlocks            ; no, another unit to test

;------ units initialised, free up everything we used temporarily ----------
                move.l  a3,a0                   ; the janus sig we used
                movea.l jd_JanusBase(a5),a6
                CALLLVO CleanupJanusSig
                move.l  jd_ExecBase(a5),a6
                move.l  d4,d0                   ; and the signal bit
                CALLLVO FreeSignal

;------ attach track buffer and parameter block to a janus interrupt -------
                CALLLVO Disable

                move.l  #JSERV_HARDDISK,d0
                move.l  jd_JanusBase(a5),a6
                CALLLVO GetParamOffset          ; get offset to param bits
                cmp.w   #$ffff,d0               ; is it free for us to use
                bne.s   initErr6                ; nope, we better quit now

                move.l  jd_AmigaDskReq(a5),d0
                CALLLVO JanusMemToOffset        ; set parameter offset ...
                move.l  d0,d1
                moveq   #JSERV_HARDDISK,d0      ; ... for this interrupt
                CALLLVO SetParamOffset

                moveq   #JSERV_HARDDISK,d0      ; add it as a handler
                lea     jd_CmdTerm(a5),a1
                CALLLVO SetJanusHandler

                moveq   #JSERV_HARDDISK,d0      ; and turn the sucker on
                moveq   #1,d1
                CALLLVO SetJanusEnable

                move.l  jd_ExecBase(a5),a6
                CALLLVO Enable

;------ set up return code as pointer to the device base ------------------
                move.l  a5,d0
initEnd         movem.l (sp)+,d2-d4/a2-a6
                rts

;------ various exit points when things get screwed up --------------------
initErr6        movea.l jd_ExecBase(a5),a6
                CALLLVO Enable                  ; DISABLEd when called here
                bra.s   initErr4                ; but disk partition gone

initErr5        movea.l jd_JanusBase(a5),a6     ; free up partition struct
                movea.l a2,a1
                moveq.l #DskPartition_SIZEOF,d0
                CALLLVO FreeJanusMem

initErr4        movea.l a3,a0                   ; free janus signal struct
                movea.l jd_JanusBase(a5),a6
                CALLLVO CleanupJanusSig

initErr3        movea.l jd_JanusBase(a5),a6     ; free the track buffer
                move.l  #TB_SIZE,d0
                movea.l jd_TrackBuffer(a5),a1
                CALLLVO FreeJanusMem
                move.l  jd_ExecBase(a5),a6      ; and the signal we got
                move.l  d4,d0
                bmi.s   initErr2 
                CALLLVO FreeSignal

initErr2        movea.l jd_JanusBase(a5),a6     ; free the disk request
                moveq.l #AmigaDskReq_SIZEOF,d0
                movea.l jd_AmigaDskReq(a5),a1
                CALLLVO FreeJanusMem

initErr1        movea.l jd_ExecBase(a5),a6      ; close janus library
                move.l  jd_JanusBase(a5),a1
                CALLLVO CloseLibrary

initErr         move.l  jd_ExecBase(a5),a6
                move.l  a3,-(sp)                ; free up unit patches
                movea.l jd_Units(a5),a3         ; free up unit nodes
UnitFree        cmpa.w  #0,a3                   ; last unit ?
                beq.s   UnitFreeDone            ; yep, all memory freed
                movea.l a3,a1                   ; freeing this unit
                movea.l BB_Next(a3),a3          ; move to next first
                move.l  #BB_SIZEOF,d0
                CALLLVO FreeMem
                bra.s   UnitFree                ; go do the next
UnitFreeDone    move.l  (sp)+,a3

                move.l  a5,a1                   ; get rid of the device
                CLEAR   d0
                move.w  LIB_NEGSIZE(a5),d0
                sub.w   d0,a1
                add.w   LIB_POSSIZE(a5),d0
                CALLLVO FreeMem

                moveq.l #0,d0                   ; return an error status
                bra     initEnd
                                  

;----------------------------------------------------------------------
initStructData:
                ;------ initialize the device library structure
                INITBYTE        LN_TYPE,NT_DEVICE
                INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
                INITWORD        LIB_VERSION,VERNUM
                INITWORD        LIB_REVISION,REVNUM

                DC.W    0

jlName:         JANUSNAME

        END


