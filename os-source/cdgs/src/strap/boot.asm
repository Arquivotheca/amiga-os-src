**
**      $Id: boot.asm,v 1.1 92/08/20 13:45:48 jerryh Exp Locker: jerryh $
**
**      boot module initialization and main loop
**
**      (C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**          All Rights Reserved
**
        SECTION strap

**      Includes

        INCLUDE "strap.i"

        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/alerts.i"
                                           
        INCLUDE "devices/trackdisk.i"
 
        INCLUDE "libraries/expansionbase.i"
        INCLUDE "libraries/configvars.i"

        INCLUDE "bootblock.i"

        INCLUDE "internal/card.i"
        INCLUDE "internal/librarytags.i"


**      Exports

        XDEF    RMInit
        XDEF    SMInit
        XDEF    SMAlert


**      Imports

        XLVO    AddHead                 ; Exec
        XLVO    AddTail                 ;
        XLVO    Alert                   ;
        XLVO    AllocEntry              ;
        XLVO    CloseDevice             ;
        XLVO    CloseLibrary            ;
        XLVO    DoIO                    ;
        XLVO    InitResident            ;
        XLVO    Insert                  ;
        XLVO    OpenDevice              ;
        XLVO    OpenLibrary             ;
        XLVO    Remove                  ;
        XLVO    CacheClearU             ;
        XLVO    OpenResource            ;
        XLVO    TaggedOpenLibrary       ;

        XLVO    ObtainConfigBinding     ; Expansion
        XLVO    ReleaseConfigBinding    ;
        XLVO    SetCurrentBinding       ;

        XLVO    UMult32                 ; Utility

        IFND    _LVOOwnCard
        XLVO    OwnCard                 ; Card
        ENDC

        IFND    _LVOReleaseCard
        XLVO    ReleaseCard
        ENDC

        IFND    _LVOIfAmigaXIP
        XLVO    IfAmigaXIP
        ENDC

        XREF    SMDisplayOn
        XREF    SMDisplayOff
        XREF    SMDisplayTick

        XREF    smName

**      Locals

 STRUCTURE      BootBlockEntry,MLN_SIZE
    CPTR    bbe_Entry                   ; address of entry in eb_MountList
    CPTR    bbe_FSSM                    ; CPTR to FSSM in entry
    CPTR    bbe_Data                    ; BootBlocks data
    ULONG   bbe_Length                  ; size of BootBlocks data
    ULONG   bbe_Offset                  ; origin of partition
    STRUCT  bbe_IOR,IOSTD_SIZE          ; IO request for device
    ULONG   bbe_DiskChangeCnt           ; disk change count for device
    LABEL   BootBlockEntry_SIZEOF

 STRUCTURE      BootLocals,0
    STRUCT  bbl_NonAmigaDOS,MLH_SIZE    ; list of non-AmigaDOS entries
    STRUCT  bbl_Port,MP_SIZE            ; request reply port
    STRUCT  bbl_BBEList,MLH_SIZE        ; list of BootBlockEntries
    STRUCT  bbl_MLBody,ML_SIZE+(ME_SIZE*2)-LN_SIZE
                                        ; used to allocate BootBlockEntries
    APTR    bbl_CardResource            ; pointer to card.resource
    APTR    bbl_CardHandle              ; point to a card handle
    ULONG   bbl_CardCount               ; Card change count
    LABEL   BootLocals_SIZEOF

bbl_MemList     EQU     bbl_MLBody-LN_SIZE ; don't use node for allocations


**      Assumptions

        IFNE    de_TableSize
        FAIL    "de_TableSize not zero, recode"
        ENDC
        IFNE    DAC_CONFIGTIME-$10
        FAIL    "DAC_CONFIGTIME not bit 4, recode"
        ENDC
        IFNE    da_Config
        FAIL    "da_Config not zero, recode"
        ENDC
        


RMInit:
                movem.l d2/a2-a6,-(a7)

                moveq   #0,d0
                move.l  d0,-(a7)        ; cb_ToolTypes
                move.l  d0,-(a7)        ; cb_ProductString
                move.l  d0,-(a7)        ; cb_FileName

                add.w   #12,a7
                movem.l (a7)+,d2/a2-a6
                rts


**********************************************************************
;
;       1.  remove the non-AmigaDOS eb_MountList entries from the
;           list and store on a second list
;       2.  step thru both lists in priority order and try booting
;           from each entry.  Cache BootBlock style entries in the
;           process.
;       3.  put up the "insert workbench" graphic and poll for a
;           disk change in a BootBlock entry.  Try booting off the
;           changed disk.  repeat until successful.
;
;       Notes:
;       -   expansion library is not closed if boot is from BootCode
;           style boot.  this should be fixed
;

;
;       d2      eb_MountList previous entry
;       d3      eb_MountList next entry, or zero if using bll_NonAmigaDOS
;       d4      short duration temporary: bbe entry, Envec, SysBase,
;               then bbl_BBEList next entry in prompt loop
;       d5      ExpansionBase or StrapDisplayLocals (see a5)
;
;       a2      current entry (from eb_Mountlist or bbl_NonAmigaDOS),
;               then from bbl_BBEList
;       a3      eb_MountList current entry
;       a4      BootLocals
;       a5      ExpansionBase or StrapDisplayLocals (see d5)
;       a6      SysBase
;

SMInit:
                ;-- get expansion library
                moveq   #OLTAG_EXPANSION,d0
                CALLLVO TaggedOpenLibrary
                tst.l   d0
                bne.s   sExpLibOK

                move.l  #AN_BootStrap!AG_OpenLib!AO_ExpansionLib,d0
                bsr     SMAlert
                rts

                ;-- initialize environment
sExpLibOK:
                movem.l d2-d5/a2-a6,-(a7)

                lea     -BootLocals_SIZEOF(a7),a7
                move.l  a7,a4

                move.l  d0,a5                   ; save ExpansionBase
                moveq   #0,d5                   ; Display off

                ;--     initialize list of non-AmigaDOS devices
                lea     bbl_NonAmigaDOS(a4),a0
                NEWLIST a0

                ;--     initialize local reply port
                clr.b   bbl_Port+MP_FLAGS(a4)
                move.b  #SIGB_SINGLE,bbl_Port+MP_SIGBIT(a4)
                move.l  ThisTask(a6),bbl_Port+MP_SIGTASK(a4)
                lea     bbl_Port+MP_MSGLIST(a4),a0
                NEWLIST a0

                ;--     initialize boot block list
                lea     bbl_BBEList(a4),a0
                NEWLIST a0

                ;--     initialize memlist constants
                move.w  #2,bbl_MemList+ML_NUMENTRIES(a4)
                move.l  #MEMF_PUBLIC!MEMF_CLEAR,bbl_MemList+ML_ME+ME_REQS(a4)
                move.l  #BootBlockEntry_SIZEOF,bbl_MemList+ML_ME+ME_LENGTH(a4)


                ;-- trim eb_MountList of bad and non-AmigaDOS nodes
                lea     eb_MountList(a5),a0     ; "previous node"
                move.l  a0,d2
                move.l  (a0),a3                 ; current node

sTrimEMList:
                move.l  (a3),d3                 ; check for end of mount list
                beq.s   sScanForBoot

                ;--     validate node has device entry
                move.l  bn_DeviceNode(a3),d0
                bne.s   sValidEMNode

                ;--     REMOVE node, and loop
sRemoveEMNode:
                move.l  d3,a3                   ; next
                move.l  d2,a0                   ; prev
                move.l  a3,(a0)                 ; cache next in prev succ
                move.l  a0,LN_PRED(a3)          ; cache prev in next pred
                bra.s   sTrimEMList

sValidEMNode:
                ;--     determine if AmigaDOS node
                move.l  d0,a0
                tst.b   dn_Handler(a0)          ; test MS bit of handler long
                bpl.s   sTrimEMNext

                ;--     put non-AmigaDOS boot bode on seperate list
                lea     bbl_NonAmigaDOS(a4),a0
                move.l  a3,a1
                CALLLVO AddTail
                bra.s   sRemoveEMNode

                ;--     inspect next list entry
sTrimEMNext:
                move.l  a3,d2
                move.l  d3,a3
                bra.s   sTrimEMList


                ;-- step thru eb_MountList and bbl_NonAmigaDOS for boot
sScanForBoot:
                lea     eb_MountList(a5),a0     ; "previous node"
                move.l  a0,d2
                move.l  (a0),a3                 ; current node

sScanList:
                ;--     get eb_MountList and bbl_NonAmigaDOS candidates
                move.l  bbl_NonAmigaDOS(a4),a0
                move.l  a3,a2
                move.l  (a2),d3                 ; check for end of mount list
                beq.s   sEBLoser
                tst.l   (a0)                    ; check for end of non' list
                beq.s   sEBWinner
                move.b  LN_PRI(a2),d0
                cmp.b   LN_PRI(a0),d0
                bge.s   sEBWinner               ; (AmigaDOS wins ties :-)

sNADWinner:
                moveq   #0,d3                   ; show not eb_MountList entry
sEBLoser:
                ;--     forward remhead non-AmigaDOS winner from head of list
                move.l  (a0),bbl_NonAmigaDOS(a4)
                beq     sPromptDisk             ; this list is empty, too
                move.l  a0,a2

sEBWinner:
                ;--     verify that this is a boot node
                cmp.b   #NT_BOOTNODE,LN_TYPE(a2)
                bne     sScanNext
                move.l  bn_DeviceNode(a2),a0
                ;--     determine if BootBlocks or BootPoint
                move.l  dn_Startup(a0),d0       ; get FileSystemStartup
                moveq   #$40,d1                 ; validate it
                cmp.l   d1,d0                   ;   (0x0-0x3f are invalid)
                bcs     sChkBootPoint
                lsl.l   #2,d0
                move.l  d0,a0
                move.l  fssm_Environ(a0),d0     ; get Envec
                beq     sChkBootPoint
                lsl.l   #2,d0
                move.l  d0,a0
                ;--         look for valid non-zero de_BootBlocks
                cmp.l   #DE_BOOTBLOCKS,(a0)     ; de_TableSize(a0)
                blt     sChkBootPoint
                move.l  de_BootBlocks(a0),d0
                beq     sChkBootPoint

                ;--         generate BootBlockEntry
                move.l  de_BufMemType(a0),bbl_MemList+ML_ME+ME_SIZE+ME_REQS(a4)
                mulu    de_SizeBlock+2(a0),d0   ; block always < 64K bytes
                lsl.l   #2,d0                   ; size in bytes
                move.l  d0,bbl_MemList+ML_ME+ME_SIZE+ME_LENGTH(a4)
                lea     bbl_MemList(a4),a0
                CALLLVO AllocEntry
                tst.l   d0
                bpl.s   sInitBBE

                ;--     memory allocation failure               
                move.l  #AN_BootStrap!AG_NoMemory,d0
                bsr     SMAlert
                bra     sScanNext


sInitBBE:
                move.l  d0,a1
                ;--         initialize entries in bbe
                move.l  ML_ME+ME_ADDR(a1),a0    ; bbe
                tst.l   d3
                beq.s   sCacheBufPtr
                move.l  a2,bbe_Entry(a0)        ; cache entry pointer
sCacheBufPtr:
                move.l  ML_ME+ME_SIZE+ME_ADDR(a1),bbe_Data(a0)
                move.l  ML_ME+ME_SIZE+ME_LENGTH(a1),bbe_Length(a0)

                ;--         add MemEntry to task list
                move.l  a0,d4                   ; save bbe
                move.l  ThisTask(a6),a0
                lea     TC_MEMENTRY(a0),a0                
                CALLLVO AddTail

                move.l  bn_DeviceNode(a2),a1
;
;   a2  bbe entry
;
                move.l  d4,a2
                move.l  dn_Startup(a1),d4
                lsl.l   #2,d4
                move.l  d4,bbe_FSSM(a2)

                ;--         calculate bbe_Offset
                moveq   #OLTAG_UTILITY,d0
                CALLLVO TaggedOpenLibrary
                tst.l   d0
                bne.s   sUtilLibOK

                move.l  #AN_BootStrap!AG_OpenLib!AO_UtilityLib,d0
                bsr     SMAlert
                bra.s   sScanNext

sUtilLibOK:
                move.l  a3,-(a7)
                move.l  d4,a3                   ; get Envec
                move.l  fssm_Environ(a3),d1     ;
                lsl.l   #2,d1                   ;
                move.l  d1,a3                   ;
                move.l  a6,d4                   ; save SysBase
                move.l  d0,a6                   ; activate Utility
                move.l  de_LowCyl(a3),d0
                move.l  de_Surfaces(a3),d1
                CALLLVO UMult32
                move.l  de_BlocksPerTrack(a3),d1
                CALLLVO UMult32
                move.l  de_SizeBlock(a3),d1
                CALLLVO UMult32
                lsl.l   #2,d0
                move.l  d0,bbe_Offset(a2)
                move.l  (a7)+,a3
                move.l  a6,a1
                move.l  d4,a6                   ; activate SysBase
                CALLLVO CloseLibrary

                ;--         IO request initialization
                lea     bbl_Port(a4),a0
                move.l  a0,bbe_IOR+MN_REPLYPORT(a2)

                ;--         add bbe to local list
                lea     bbl_BBEList(a4),a0
                move.l  a2,a1
                CALLLVO AddTail

                bsr.s   SMBootBlocks            ; try booting from this
                beq.s   sBootA0

                bra.s   sScanNext

                ;--         validate da_BootPoint
sChkBootPoint:
                move.l  LN_NAME(a2),d0          ; ConfigDev pointer
                beq.s   sScanNext
                move.l  d0,a0
                btst.b  #ERTB_DIAGVALID,cd_Rom+er_Type(a0)
                beq.s   sScanNext
                move.l  cd_Rom+er_Reserved0c(a0),d1 ; DiagArea pointer
                beq.s   sScanNext
                move.l  d1,a0
                btst.b  #4,(a0)                 ; DAC_CONFIGTIME,da_Config(a0)
                beq.s   sScanNext

                bsr     SMBootPoint

sScanNext:
                tst.l   d3                      ; eb_MountList scan?
                beq     sScanList
                move.l  a3,d2                   ; save ln_Pred of next
                move.l  d3,a3                   ; get next
                bra     sScanList


                ;-- prompt for boot disk till successful boot
sPromptDisk:
                move.l  bbl_BBEList(a4),a2
                ;--     !!! Assumes at least one BBEList entry !!!
spdLoop:
                move.l  (a2),d4
                beq.s   sPromptDisk

                bsr.s   SMBootBlocks
                beq.s   sBootA0

                move.l  d4,a2
                bra.s   spdLoop


sBootA0:
                move.l  a0,d4           ; save floppy boot entry point
                bsr     SMCloseIORs

                move.l  bbe_Entry(a2),d3
                beq.s   sCloseEL

                ;--     Put this AmigaDOS entry at head of eb_MountList
                move.l  d3,a3
                move.l  LN_PRED(a3),d2
                bsr     SMSwizzleBefore

                ;--     close expansion library
sCloseEL:
                move.l  a5,a1           ; ExpansionBase
                CALLLVO CloseLibrary

                ;--     restore stack, registers & call boot module
                move.l  d4,a0
                lea     BootLocals_SIZEOF(a7),a7
                movem.l (a7)+,d2-d5/a2-a6
                jmp     (a0)            ; w/ SysBase in A6!



;------ SMBootBlocks -------------------------------------------------
;
;       a2      BootBlockEntry
;       a4      BootLocals
;       a5      ExpansionBase
;       a6      ExecBase
;
;       d2      disk check flag
;       a3      IOR
;
SMBootBlocks:
                movem.l d2/a3,-(a7)
                moveq   #0,d2
                lea     bbe_IOR(a2),a3  ; get IOR

                tst.l   IO_DEVICE(a3)
                bne.s   smbbHaveDevice

                ;-- open device
                moveq   #1,d2           ; show need to check disk
                move.l  bbe_FSSM(a2),a0
                move.l  fssm_Unit(a0),d0
                move.l  fssm_Flags(a0),d1
                move.l  fssm_Device(a0),a0
                add.l   a0,a0           ; convert BSTR to CSTR
                add.l   a0,a0           ;
                addq.l  #1,a0           ;
                move.l  a3,a1
                CALLLVO OpenDevice
                tst.l   d0
                beq.s   smbbHaveDevice

                clr.l   IO_DEVICE(a3)
                bne     smbbNoDisk

smbbHaveDevice:
                ;-- we now have a drive.  Get the change number
                move.l  a3,a1
                move.w  #TD_CHANGENUM,IO_COMMAND(a1)    
                CALLLVO DoIO
                tst.l   d0
                bne     smbbIOError

                ;--     compare the change number
                move.l  IO_ACTUAL(a3),d0
                cmp.l   bbe_DiskChangeCnt(a2),d0
                beq.s   smbbCacheChangeCnt
                moveq   #1,d2                   ; show need to check disk

smbbCacheChangeCnt:
                move.l  d0,bbe_DiskChangeCnt(a2)

                tst     d2
                beq     smbbNoDisk

                ;-- check for disk in drive
                move.l  a3,a1
                move.w  #TD_CHANGESTATE,IO_COMMAND(a1)
                CALLLVO DoIO
                tst.l   d0
                bne     smbbIOError

                tst.l   IO_ACTUAL(a3)
                bne     smbbNoDisk

                ;-- clear out any stale buffers
                move.l  a3,a1
                move.w  #CMD_CLEAR,IO_COMMAND(a1)       
                CALLLVO DoIO
                tst.l   d0
                bne     smbbIOError

                ;-- read from device
                move.l  a3,a1
                move.w  #CMD_READ,IO_COMMAND(a1)
                move.l  bbe_Length(a2),IO_LENGTH(a1)
                move.l  bbe_Data(a2),IO_DATA(a1)
                move.l  bbe_Offset(a2),IO_OFFSET(a1)
                CALLLVO DoIO
                tst.l   d0
                bne.s   smbbFail

                ;------ see if there is a legal header
                move.l  bbe_Data(a2),a0
                move.l  (a0),d0         ; check for valid cookie
                cmp.l   #$424f4f55,d0   ; 'BOOT' ?
                beq.s   smbbSum
                clr.b   d0              ; 'DOS?' ?  (low byte ignored)
                cmp.l   #BBNAME_DOS,d0
                bne.s   smbbFail

                ;------ validate the checksum (longword w/ carry wraparound)
smbbSum:
                move.l  bbe_Length(a2),d1
                lsr.l   #2,d1
                moveq   #0,d0
                bra.s   smbbSumDBF

smbbSumLoop:
                add.l   (a0)+,d0
                bcc.s   smbbSumDBF
                addq.l  #1,d0
smbbSumDBF:
                dbf     d1,smbbSumLoop

                not.l   d0
                bne.s   smbbFail

                ;--     Might have set SILENTSTART in expansion base
                ;--     while trying BootPoints

                bclr    #EBB_SILENTSTART,eb_Flags(a5)

                ;--     and clear cache before calling loaded code

                CALLLVO CacheClearU

                ;------ jump to the code with a1/a6 set up
                move.l  a3,a1
                move.l  bbe_Length(a2),IO_LENGTH(a1)
                move.l  bbe_Data(a2),a0
                move.l  a0,IO_DATA(a1)
                move.l  bbe_Offset(a2),IO_OFFSET(a1)

                ;-- save all registers for robust system operations.
                ;-- in truth its implied that register be saved by
                ;-- the boot block code, but after talking with
                ;-- a developer, its become clear that this wasn't
                ;-- well understood.  Since register dependencies
                ;-- changed since 1.x days, save all non-scratchable
                ;-- registers (overkill)

                movem.l d2-d7/a2-a6,-(sp)
                jsr     BB_ENTRY(a0)
                movem.l (sp)+,d2-d7/a2-a6

                ;------ check the return code: zero means a0 contains boot addr
                tst.l   d0
                beq.s   smbbSucceed

                move.l  #AN_BootError,d0

smbbAlert:
                bsr     SMAlert

smbbFail:
                ;------ turn the motor off
                move.l  a3,a1
                move.w  #TD_MOTOR,IO_COMMAND(a1)        
                clr.l   IO_LENGTH(a1)
                CALLLVO DoIO

smbbNoDisk:
                moveq   #-1,d0

smbbSucceed:
                movem.l (a7)+,d2/a3
                rts

smbbIOError:
                cmp.b   #TDERR_DiskChanged,d0
                beq     smbbHaveDevice

                move.l  #AN_BootStrap!AG_IOError,d0
                bra.s   smbbAlert

;------ SMCloseIORs --------------------------------------------------
;
;       a4      BootLocals
;       a5      ExpansionBase
;       a6      ExecBase
;
;       d2      next bbe
;       a2      current bbe
;
SMCloseIORs:
                movem.l d2/a2,-(a7)
                move.l  bbl_BBEList(a4),a2
smciLoop:
                move.l  (a2),d2
                beq.s   smciDone

                tst.l   bbe_IOR+IO_DEVICE(a2)
                beq.s   smciNext

                lea     bbe_IOR(a2),a1
                CALLLVO CloseDevice
                clr.l   bbe_IOR+IO_DEVICE(a2)

smciNext:
                move.l  d2,a2
                bra.s   smciLoop

smciDone:
                movem.l (a7)+,d2/a2
                rts


;------ SMSwizzleBefore ----------------------------------------------
;
;       d3      non-zero for eb_MountList entry
;       a3      if d3, eb_MountList current entry
;       a5      ExpansionBase
;       a6      ExecBase
;
SMSwizzleBefore:
                tst.l   d3                      ; check if eb_MountList entry
                beq.s   smsbDone                ;   no, no swizzle

                ;-- temporarily swizzle current node to head of eb_MountList
                move.l  a3,a1
                CALLLVO Remove

                lea     eb_MountList(a5),a0
                move.l  a3,a1
                CALLLVO AddHead

smsbDone:
                rts


;------ SMSwizzleAfter -----------------------------------------------
;
;       d2      if d3, eb_MountList previous entry
;       d3      non-zero for eb_MountList entry
;       a3      if d3, eb_MountList current entry
;       a5      ExpansionBase
;       a6      ExecBase
;
SMSwizzleAfter:
                tst.l   d3
                beq.s   smsaDone

                ;-- put current node back in place
                move.l  a3,a1
                CALLLVO Remove

                lea     eb_MountList(a5),a0     ; (even though not used)
                move.l  a3,a1
                exg     d2,a2
                CALLLVO Insert
                exg     d2,a2

smsaDone:
                rts


;------ SMBootPoint --------------------------------------------------
;
;       d0      ConfigDev
;       d2      if d3, eb_MountList previous entry
;       d3      non-zero for eb_MountList entry
;       a0      DiagArea
;       a2      current entry
;       a3      if d3, eb_MountList current entry
;       a4      BootLocals
;       a5      ExpansionBase
;       a6      ExecBase
;
SMBootPoint:
                ;-- invoke boot function
                ;--     prepare boot function
                moveq   #0,d1
                move.w  da_BootPoint(a0),d1
                add.l   d1,a0
                move.l  d0,-(a7)                ; ConfigDev arg on stack
                move.l  a0,-(a7)                ; boot entry point on stack

                bsr.s   SMSwizzleBefore
                bsr.s   SMCloseIORs

                ;--     call boot function
                ;       1.  ConfigDev is on the stack
                ;       2.  BootNode is in a2
                ;       3.  SysBase is in a6

                ;--     For BootPoint, silent startup is assumed to
                ;--     to be TRUE!

                bset    #EBB_SILENTSTART,eb_Flags(a5)

                move.l  (a7)+,a0
                jsr     (a0)
                addq.l  #4,a7

                ;-- return implies failure to boot
                bsr.s   SMSwizzleAfter

ebNoBoot:
                rts


;------ SMAlert ------------------------------------------------------
SMAlert:
                movem.l d7/a5/a6,-(a7)
                move.l  d0,d7
                moveq   #0,d0
                move.l  d0,a5
                move.l  ABSEXECBASE,a6
                CALLLVO Alert
                movem.l (a7)+,d7/a5/a6
iRts            rts


        END
