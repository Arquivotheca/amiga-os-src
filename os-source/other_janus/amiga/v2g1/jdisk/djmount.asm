;*********************************************************************
;
; djmount.asm -- mount command for jdisk device
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
; Steve.  Added 2 cylinder reservation for bad block mapping. jan/87
;*********************************************************************

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i" 
        INCLUDE "exec/io.i"
        INCLUDE "exec/memory.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "janus/janus.i"
        LIST

        INCLUDE "djmount.i" 
        INCLUDE "asmsupp.i"
        INCLUDE "printf.mac"

        XLVO    AllocMem
        XLVO    FreeMem
        XLVO    AllocSignal
        XLVO    CloseDevice
        XLVO    CloseLibrary
        XLVO    Forbid
        XLVO    FreeSignal
        XLVO    OpenDevice
        XLVO    OpenLibrary
        XLVO    Permit
        XLVO    Wait

        XLVO    DeviceProc
        XLVO    Open
        XLVO    Close
        XLVO    Delay

        XLVO    AllocJanusMem
        XLVO    CleanupJanusSig
        XLVO    FreeJanusMem
        XLVO    JanusMemToOffset
        XLVO    SendJanusInt
        XLVO    SetupJanusSig

        XREF    _SysBase
        XREF    _DOSBase

        XDEF    _main

BOOT_TIMEOUT    EQU     40      ; seconds to wait for PC handler booted

DEBUG_CODE EQU 1

;---------------------------------------------------------------------
        SECTION section,data
Info            DS.L    1
_JanusBase      DS.L    1

JH0             EQU     $034a4830           ; BCPL string "JH0"
;---------------------------------------------------------------------
        SECTION section,code

_main:          DEBUGINIT
                movem.l d2-d7/a2-a6,-(a7)
                ;------ check that JH0 does not already exist
                move.l  _DOSBase,a0         ; get DOSBase
                move.l  dl_Root(a0),a0      ; get RootNode
                move.l  rn_Info(a0),a0      ; get BPTR Info
                add.l   a0,a0               ; adjust to APTR
                add.l   a0,a0               ;
                move.l  a0,Info             ; save Info structure location
                move.l  di_DevInfo(a0),d0   ; get BPTR DevInfo
loopDevInfos:
                beq.s   endOfDevInfos
                lsl.l   #2,d0               ; adjust to APTR   
                move.l  d0,a0
                move.l  jmd_Name(a0),a1     ; get BPTR to name
                add.l   a1,a1               ; adjust to APTR
                add.l   a1,a1               ;
                cmp.l   #JH0,(a1)           ; check if "JH0"
                beq.s   endOfDevInfos
                move.l  (a0),d0             ; get BPTR jmd_Next
                bra.s   loopDevInfos

endOfDevInfos:
                ;------ if d0 is zero here, JH0 was not found
                tst.l   d0
                bne     errMain1
        printf  <'JH0: not already linked in\n'>
                ;------ get janus.library                
                lea     jlName(pc),a1
                moveq   #0,d0
                move.l  _SysBase,a6
                CALLLVO OpenLibrary
                move.l  d0,_JanusBase   
                beq     errMain1            ; library not found

;============== before starting up, wait for the PC's handler to be running
                movea.l _JanusBase,A0           ; wait for $42
                movea.l jb_ParamMem(A0),A2
                moveq.l #BOOT_TIMEOUT,d4        ; only wait for a while
                movea.l _DOSBase,a6
PCReadyLoop     cmpi.b  #$42,ja_ParamMem+jmh_pad0(A2)
                beq     PCGoing                 ; yes, PC up and running
        printf  <'.'>
                moveq.l #50,d1                  ; delay 1 second
                CALLLVO Delay
                dbra    d4,PCReadyLoop          ; and try again maybe
;==== PC didn't boot it's handler within timeout time so error exit

                movea.l _JanusBase,a1
                movea.l _SysBase,a6
                CALLLVO CloseLibrary
                bra     errMain1

                ;------ setup signal handshake for this initialization code
PCGoing         printf  <'\nPC running\n'>
                movea.l _SysBase,a6
                moveq   #-1,D0
                CALLLVO AllocSignal
                move.l  d0,d1
                move.l  d0,d4               ; save signal to free later
                moveq   #JSERV_HARDDISK,D0
                moveq   #HardDskReq_SIZEOF,D2
                move.l  #MEMF_PARAMETER!MEM_WORDACCESS,d3

                move.l  _JanusBase,a6
                CALLLVO SetupJanusSig
                tst.l   d0
                beq     errMain2          
                move.l  d0,a3               ; save signal structure

                ;------ inquire the hard disk
                move.l  ss_ParamPtr(a3),a4
        printf  <'Mein Paramptr ist @ $%lx\n'>,a4
                move.w  #HDR_FNCTN_INIT,hdr_Fnctn(a4)
                moveq   #JSERV_HARDDISK,D0
                CALLLVO SendJanusInt
                move.l  ss_SigMask(a3),d0
                move.l  _SysBase,a6
                CALLLVO Wait                 

                ;------ now inquire for each partition on the hard disk
                move.w  hdr_Part(a4),d2     ; number of partitions
        printf  <'Wir haben %ld partition\n'>,d2
        tst.w   d2
                beq     errMain3

                moveq   #HDskPartition_SIZEOF,d0
                move.l  #MEMF_BUFFER!MEM_WORDACCESS,d1
                move.l  _JanusBase,a6
                CALLLVO AllocJanusMem
                tst.l   d0
                beq     errMain3
        printf  <'Buffer mem at %lx\n'>,d0
                move.l  d0,a5
                CALLLVO JanusMemToOffset
        printf  <'Buffer mem offset = %x\n'>,d0
                move.w  d0,hdr_BufferAddr(a4)
                move.w  #HDR_FNCTN_INFO,hdr_Fnctn(A4)
                moveq   #0,d3               ; current partition

                ;------ loop for each partition
partitionLoop:
                move.l  #HDskPartition_SIZEOF,hdr_Count(a4)
                move.w  d3,hdr_Part(a4)
                moveq   #JSERV_HARDDISK,d0
                CALLLVO SendJanusInt
                move.l  ss_SigMask(a3),d0
                move.l  _SysBase,a6
                CALLLVO Wait                 
                cmp.l   #HDskPartition_SIZEOF,hdr_Count(a4)
                blt     errMain4

                ;---------- build a device node for this partition
                move.l  #jma_SIZEOF,d0
                move.l  #MEMF_PUBLIC+MEMF_CLEAR,d1
                CALLLVO AllocMem
                tst.l   d0
                beq     errMain4
                move.l  d0,a2

                ;---------- fill the device node
                move.l  #$400,jma_jmd+jmd_StackSize(a2)
                move.l  #10,jma_jmd+jmd_Priority(a2)
                lea     jma_jms(a2),a0
                move.l  a0,d0
                lsr.l   #2,d0
                move.l  d0,jma_jmd+jmd_Startup(a2)
                move.l  _DOSBase,a6
                move.l  #df0Name,d1
                CALLLVO DeviceProc
                tst.l   d0
                beq     errMain4
                move.l  d0,a0
                move.l  pr_SegList-pr_MsgPort(a0),a0
                add.l   a0,a0
                add.l   a0,a0
                move.l  12(a0),jma_jmd+jmd_SegList(a2)

                ;---------- fill the environment vector
                move.l  #(jme_SIZEOF/4)-1,jma_jme+jme_StructSize(a2)
                move.l  #1,jma_jme+jme_NSectsPerBlk(a2)
                move.l  #128,jma_jme+jme_BlockSize(a2)   ;longwords/block
                move.l  #5,jma_jme+jme_NBuffers(a2)      
                moveq   #0,d0
                move.w  hdp_NumHeads(a5),d0
        printf  <'NumHeads = %d\n'>,d0
                move.l  d0,jma_jme+jme_NSurfaces(a2)
                move.w  hdp_NumSecs(a5),d0
        printf  <'NumSecs = %d\n'>,d0
                move.l  d0,jma_jme+jme_NBlksPerTrack(a2)

*********** fixes a VERY old bug that ate the boot block when disk gets full
                move.l  #2,jma_jme+jme_NReservedBlks(a2)
*********** steve. 1/2/87 **************************************************
        move.w  hdp_BaseCyl(a5),d0
        printf  <'BaseCyl=%d '>,d0
                move.w  hdp_EndCyl(a5),d0
        printf  <'EndCyl = %d\n'>,d0
                sub.w   hdp_BaseCyl(a5),d0
                subq.w  #2,d0                           ;reserve 2 cylinders
                move.l  d0,jma_jme+jme_HighCyl(a2)

        
;============================================================================
; Patch added since bad block handling was installed.  If the unit has less
; than 3 cylinders, then we ignore it because 2 cyls are used for bad blocks
;============================================================================
                cmpi.l  #3,d0                   ; enough cylinders ?
                bge.s   GoodUnit                ; yep, carry on then
                movea.l _SysBase,a6             ; free memory used
                move.l  #jma_SIZEOF,d0          ; unit too small !!
                movea.l a2,a1
                CALLLVO FreeMem
                bra     IgnoredUnit

                ;---------- fill the startup descriptor
GoodUnit        move.l  d3,jma_jms+jms_Unit(a2)
                lea     jma_ExecName(a2),a0
                move.l  a0,d0
                lsr.l   #2,d0
                move.l  d0,jma_jms+jms_DevName(a2)
                lea     jdbName(pc),a1
                clr.w   d0
                move.b  (a1),d0
1$              move.b  (a1)+,(a0)+
                dbf     d0,1$
                lea     jma_jme(a2),a0
                move.l  a0,d0
                lsr.l   #2,d0
                move.l  d0,jma_jms+jms_Envir(a2)

                ;---------- initialize and link in the device name
                lea     jma_DosName(a2),a0
                move.l  a0,d0
                lsr.l   #2,d0
                move.l  d0,jma_jmd+jmd_Name(a2)
                move.l  #JH0,d0
                add.w   d3,d0                   ; convert to jh0: jh1: etc.
                move.l  d0,jma_DosName(a2)
                clr.l   jma_DosNameTerm(a2)     ; needs null terminator still

                ;---------- link in this device node into the system  
                move.l  _SysBase,a6
                CALLLVO Forbid
                move.l  Info,a0
                move.l  di_DevInfo(a0),jma_jmd+jmd_Next(a2)
                move.l  a2,d0
                lsr.l   #2,d0
                move.l  d0,di_DevInfo(a0)
                CALLLVO Permit

                ;---------- loop for more partitions
IgnoredUnit     move.l  _JanusBase,a6
                addq.w  #1,d3               ; increment current partition
                cmp.w   d2,d3                   ; and see if there's more
                blt     partitionLoop

                ;------ free the janus items used here
                move.l  a5,a1
                moveq   #HDskPartition_SIZEOF,d0
                CALLLVO FreeJanusMem

                move.l  a3,a0
                CALLLVO CleanupJanusSig

                move.l  _SysBase,a6
                move.l  d4,d0
                CALLLVO FreeSignal

                move.l  _JanusBase,a1
                CALLLVO CloseLibrary

; D3 = number of partitions on the disk, try to open that many dummy files
; so that the disk icon for each partition appears on the workbench.
; DOS won't load the handler until it's been accessed.

                subq.w  #1,d3
DummyLoop       movea.l _DOSBase,a6
                move.w  d3,d0                   ; get address of name
                asl.w   #3,d0                   ; 8 bytes per name
                lea.l   dummyname(pc,d0.w),a0
                move.l  a0,d1
                move.l  #MODE_OLDFILE,d2
                CALLLVO Open                    ; don't care if it opens
                move.l  d0,d1                   ; save filehandle
                beq     NoNeedToClose           ; nothing to close
                CALLLVO Close                   ; thats it, we're done
NoNeedToClose   dbra    d3,DummyLoop            ; go back for next
                moveq   #0,d0

endMain:
                movem.l (a7)+,d2-d7/a2-a6
                rts
          
errMain4:
                move.l  a5,a1
                moveq   #HDskPartition_SIZEOF,d0
                move.l  _JanusBase,a6
                CALLLVO FreeJanusMem

errMain3:
                move.l  a3,a0
                move.l  _JanusBase,a6
                CALLLVO CleanupJanusSig
errMain2:
                move.l  _SysBase,a6
                move.l  d4,d0
                bmi.s   1$
                CALLLVO FreeSignal
1$
                move.l  _JanusBase,a1
                CALLLVO CloseLibrary
errMain1:
                moveq   #20,d0
                bra     endMain

jdbName:        dc.b    12,'jdisk.device',0
                CNOP    0,4
df0Name:        dc.b    'DF0:',0
                CNOP    0,4

; allow for 8 units because there can be up to 2 hard disks installed on
; the PC side with a maximum of 4 partitions per disk.

dummyname:      dc.b    'JH0:Ste',0
                dc.b    'JH1:ve ',0
                dc.b    'JH2:Bea',0
                dc.b    'JH3:ts ',0
                dc.b    'JH4:lik',0
                dc.b    'JH5:es ',0
                dc.b    'JH6:Ami',0
                dc.b    'JH7:ga ',0
                CNOP    0,2
jlName:         JANUSNAME
           
        END


