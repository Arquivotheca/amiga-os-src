******************************************************************************
*
*       $Id: start.asm,v 40.19 93/04/07 10:14:40 brummer Exp Locker: vertex $
*
******************************************************************************
*
*       $Log:   start.asm,v $
* Revision 40.19  93/04/07  10:14:40  brummer
* Fix the open/close routine to zero out library pointers in nvbase, and
* call the diskinit routine whenever the nvlock pointer is 0.  This helped
* finding the disk more consistantly on opens.
* 
* Revision 40.18  93/03/31  17:58:14  brummer
* Add call to releasedisk during closelibs
*
* Revision 40.17  93/03/31  14:27:06  brummer
* Fix to close opened libraries at close time not expunge time.
*
* Revision 40.16  93/03/26  16:23:37  brummer
* Fix to prevent multiple opens of lowlevel and utility libraries.
*
* Revision 40.15  93/03/23  13:07:41  brummer
* Fix to error paths when lowlevel.library is not opened correctly.
* If error, it was trying to close lowlevel and bombing.  This was
* found when generating the DISK based version of nonvolatile.library
*
* Revision 40.14  93/03/12  10:04:39  brummer
* Moved Version string definition to immediatly following the
* ROMTagName definition per Jerry's request.
*
* Revision 40.13  93/03/11  14:24:44  brummer
* Open lowlevel.library on opens and close it on closes.  This is fix
* related to nonvolatilebase.i 40.5
*
* Revision 40.12  93/03/11  10:33:02  brummer
* Fix writing library revision during init.  It was writing a long value
* and should be a word.
* Fix no open of utility.library if dos not opened before.  NVRAM
* routines use utility.
* Fix close/expunge code to get rid of C code and use code duped from
* locale.library.
*
* Revision 40.11  93/03/10  14:34:56  brummer
* Fix to open code so that if DOS has not been opened, the
* open of nonvolatile.library will still be successful.
*
* Revision 40.10  93/03/09  14:05:37  brummer
* Change 1 long jump to short.
*
* Revision 40.9  93/03/08  18:03:05  brummer
* Add Martin's requested changes - short intvectors, DiskInit semaphore,
* open of DOS failure left UTIL open...
*
* Revision 40.8  93/03/05  12:51:23  brummer
* Move InitNVRAM call to init instead of open.
* Remove origonal method of disabling requesters and use the parameterized method instead.
* If the parameter is TRUE, a open of lowlevel.library will be done
* and a call will be made to KillReq() and RestoreReq()
*
* Revision 40.7  93/03/03  10:49:26  brummer
* Move call to InitNVRAM to library open instead of library init.
* (see associated fix in nvram/nvramtree.asm rev 40.7)
* Add open failure test to open of utility.library.
*
* Revision 40.6  93/03/01  15:07:14  brummer
* utility.library is opened and added to nonvolatile library base
* on opens.  A signal semiphore for NVRAM device access is
* initialized on opens as well.
*
* Revision 40.5  93/02/25  09:02:33  Jim2
* StoreNV nolonger requires the leading underscore.
*
* Revision 40.4  93/02/19  15:34:47  Jim2
*
* Revision 40.3  93/02/19  15:21:13  Jim2
* DOSBase will be opened iff it has not yet been opened and it
* exists on the libraries list.
* DOSBase will only be closed in the expunge if it has been opened.
*
* Revision 40.2  93/02/18  11:07:55  Jim2
* Added SetNVProtection to the list of LVOs.
*
* Revision 40.1  93/02/16  13:48:54  Jim2
* Changed the interface to NVRAM subcode to be identical with
* Disk subcode.
*
* Revision 40.0  93/02/10  17:25:00  Jim2
* Kills requestors before calling InitDisk.
*
* Revision 39.1  93/02/03  13:48:49  Jim2
* Corrected field names for nonvolatilebase record.  Also
* removed an line dealing with the deleted semaphore.
*
* Revision 39.0  93/02/03  11:34:17  Jim2
* Rewrote downcoding more of the standard functions into assembler.
*
*
*       (C) Copyright 1992,1993 Commodore-Amiga, Inc.
*           All Rights Reserved
*
******************************************************************************

        INCLUDE 'dos/dosextens.i'

        INCLUDE 'exec/macros.i'
        INCLUDE "exec/types.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/execbase.i"

        INCLUDE "nonvolatilebase.i"
        INCLUDE 'nonvolatile.i'
        INCLUDE "nonvolatile_rev.i"

;
; externals in oldmain.c :
;
        XREF    _GetCopyNV
        XREF    _FreeNVData
        XREF    _DeleteNV
;
; externals in main.asm :
;
        XREF    GetNVInfo
        XREF    GetNVList
        XREF    SetNVProtection
        XREF    StoreNV
;
; externals in EndCode.asm :
;
        XREF    __EndCode
;
; externals in NVRAM/nvramtree.asm :
;
        XREF    InitNVRAM
        XREF    ReleaseNVRAM
;
; externals in Disk/DiskInformation.asm :
;
        XREF    InitDisk
        XREF    ReleaseDisk
;
; public definitions :
;
        XDEF    subsysName
        XDEF    ROMTagName

        SECTION NonVolatileLibrary
;
; Prevent direct execution :
;
        moveq.l #-1,d0
        rts


ROMTAG:
        dc.w    RTC_MATCHWORD   ;RT_MATCHWORD
        dc.l    ROMTAG          ;RT_MATCHTAG
        dc.l    __EndCode       ;RT_ENDSKIP
        dc.b    RTF_COLDSTART|RTF_AUTOINIT   ;RT_FLAGS
        dc.b    VERSION         ;RT_VERSION
        dc.b    NT_LIBRARY      ;RT_TYPE
        dc.b    0               ;RT_PRI
        dc.l    subsysName      ;RT_NAME
        dc.l    VERSTR          ;RT_IDSTRING
        dc.l    InitTable       ;RT_INIT

InitTable:      dc.l    NVBASESIZE
                dc.l    initFunc
                dc.l    0                       ; initStruct
                dc.l    InitModule

*------ Functions Offsets -------------------------------------

V_DEF           MACRO
                DC.W    \1+(*-initFunc)
                ENDM

initFunc:
        dc.w    -1
        V_DEF   LibOpen
        V_DEF   LibClose
        V_DEF   LibExpunge
        V_DEF   Reserved
        V_DEF   _GetCopyNV
        V_DEF   _FreeNVData
        V_DEF   StoreNV
        V_DEF   _DeleteNV
        V_DEF   GetNVInfo
        V_DEF   GetNVList
        V_DEF   SetNVProtection
        dc.w    -1


*------ Initializaton Table -----------------------------------

; initStruct:
;       INITLONG        LIB_IDSTRING,VERSTR
;        INITLONG        LN_NAME,subsysName
;        INITWORD        LIB_VERSION,VERSION
;        INITWORD        LIB_REVISION,REVISION
;        INITBYTE        LN_TYPE,NT_LIBRARY
;        INITBYTE        LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
;
;        DC.L            0



*****l* Start.asm/InitModule *************************************************
*
*   NAME
*       InitModule - Initialization function for nonvolatile.device.
*
*   SYNOPSIS
*       InitLib = InitModule (LibBase, SegList, ExecBase)
*       d0                    d0       a0       a6
*
*   FUNCTION
*       If this is successful the library is added to the system.
*
*   INPUTS
*       LibBase - Pointer to the base for this library.
*       SegList - Pointer to the code for this library.
*       ExecBase - Pointer to the base of exec.library.
*
*   RESULT
*       InitLib - Pointer to the base for this library if successful.  NULL
*                 if there is a problem in intialization.
*
******************************************************************************
InitModule:
                move.l  a5,-(sp)                        ; save a5
                move.l  d0,a5                           ; a5 gets lib base
;
; Init library base value not autoinited :
;
                move.l  a0,nv_SegList(a5)               ; init library seglist
                move.l  a6,nv_ExecBase(a5)              ; init library execbase
                move.w  #REVISION,LIB_REVISION(a5)      ; init library revision
;
; Init semaphores :
;
                lea     nv_DiskInitSema(a5),a0          ; init DiskInit semaphore
                JSRLIB  InitSemaphore                   ;
                lea     nv_Semaphore(a5),a0             ; init NVRAM device semaphore
                JSRLIB  InitSemaphore                   ;
;
; Call to init NVRAM device interface :
;
                bsr     InitNVRAM                       ; call to init NVRAM

                move.l  a5,d0                           ; d0 gets ptr to lib base
                move.l  (sp)+,a5
                rts

*****l* start.asm/LibOpen ****************************************************
*
*   NAME
*       LibOpen - open routine for nonvolatile.device.
*
*   SYNOPSIS
*       NVBase = LibOpen (NVBase)
*       d0                a6
*
*   FUNCTION
*       This routine will open the libraries required for NVRAM functions.
*
*       The nonvolatile.library may be opened before DOS has been opened and
*       booted.  This is to get the language settings out of the NVRAM device.
*
*       If DOS has not been opened, this library will open all libraries
*       required for NVRAM device operations.  The Disk operations require
*       the DOS library pointer in the base to be nonzero to execute.  If DOS
*       fails to open, all other libraries will be closed and the open will
*       not be successful.
*
*
*   INPUTS
*       NVBase - Pointer to the base of nonvolatile.library.
*
*   RESULT
*       Either returns the library base if sucessful, or NULL if cannot open
*       the others.
*
******************************************************************************
LibOpen:
                movem.l a5/a6,-(sp)
                move.l  a6,a5                           ; a5 gets nonvolatile base
                move.l  nv_ExecBase(a5),a6              ; a6 gets exec base
;
; Open libraries needed even if DOS is unavailable :
;
                tst.w   LIB_OPENCNT(a5)                 ;
                bne.s   lo_01                           ;

                lea     UTILName(pc),a1                 ; open utility.library
                move.l  #37,d0                          ; version 37
                JSRLIB  OpenLibrary                     ;
                move.l  d0,nv_UTILBase(a5)              ;
                beq.s   lo_Exit                         ; if error, j to exit

                lea     LLName(pc),a1                   ; open lowlevel.library
                moveq.l #0,d0                           ; any version
                JSRLIB  OpenLibrary                     ;
                move.l  d0,nv_LowLevelBase(a5)          ;
                bne.s   lo_01                           ; if OK, j to continue

                bsr     CloseLibs3                      ; call to close libraries
                bra.s   lo_03                           ; j to exit
;
; Determine if we have already opened DOS library :
;
lo_01:          move.l  nv_DOSBase(a5),d0               ; is DOS lib ptr in our base ?
                bne.s   lo_04                           ; if yes, j to bumpcount
;
; Determine if anyone has opened DOS library :
;
                lea     DOSName(pc),a1                  ; a1 gets ptr to string
                lea     LibList(a6),a0                  ; a0 gets list header
                JSRLIB  FindName                        ; is DOS in Exec Lib list ?
                tst.l   d0                              ;
                beq.s   lo_06                           ; if no, j to return OK
;
; DOS has already been opened, so we can open it :
;
                lea     DOSName(pc),a1                  ; open dos.library
                move.l  #37,d0                          ; version 37
                JSRLIB  OpenLibrary                     ;
                move.l  d0,nv_DOSBase(a5)               ;
                bne.s   lo_04                           ; if OK, j to continue

lo_02:          bsr     CloseLibs2                      ; call to close libraries
lo_03:          moveq.l #0,d0                           ;
                bra.s   lo_Exit                         ; j to exit
;
; Call InitDisk :
;
lo_04:          tst.l   nv_DiskLock(a5)
                bne.s   lo_06
                bsr     InitDisk                        ; call for disk init
;
; Bump open count and return :
;
lo_06:          addq.w  #1,LIB_OPENCNT(a5)
                bclr    #LIBB_DELEXP,LIB_FLAGS(a5)
                move.l  a5,d0                           ; return base in d0
lo_Exit:        movem.l (sp)+,a5/a6                     ; restore registers
                rts






;-----------------------------------------------------------------------
; Library close entry point called every CloseLibrary()
; On entry, A6 has LocaleBase, task switching is disabled
; Returns 0 normally, or the library seglist when lib should be expunged
;   due to delayed expunge bit being set
LibClose:
        movem.l a5/a6,-(sp)                     ; save registers
        move.l  a6,a5                           ; a5 gets NVBase
        move.l  nv_ExecBase(a5),a6              ; a6 gets ExecBase
        subq.w  #1,LIB_OPENCNT(a5)              ; decrement lib open count
        bne.s   lc_04                           ; if not zero, j to return
        bsr     ReleaseDisk                     ; call to release disk
        bsr.s   CloseLibs                       ; call to close libraries opened at open
        bclr    #LIBB_DELEXP,LIB_FLAGS(a5)      ; is delayed expunge set ?
        beq.s   lc_04                           ; if no, j to return
;
; Delayed expunge :
;
        tst.w   LIB_OPENCNT(a5)                 ; is library in use ?
        bne.s   lc_04                           ; if yes, j to return
        bsr.s   DoExpunge                       ; call to expunge

lc_04:  moveq   #0,d0                           ; d0 gets return code
        movem.l (sp)+,a5/a6                     ; restore registers
        rts

;-----------------------------------------------------------------------
; Library expunge entry point called whenever system memory is lacking memory
; On entry, A6 has LocaleBase, task switching is disabled
; Returns the library seglist if the library open count is 0, returns 0
; otherwise and sets the delayed expunge bit.
LibExpunge:
        tst.w   LIB_OPENCNT(a6)
        beq.s   DoExpunge
        bset    #LIBB_DELEXP,LIB_FLAGS(a6)
        moveq   #0,d0
        rts

DoExpunge:
        movem.l d2/a5/a6,-(sp)
        move.l  a6,a5
        move.l  nv_ExecBase(a5),a6
        move.l  nv_SegList(a5),d2

        move.l  a5,a1
        JSRLIB  Remove

        move.l  a5,a1
        moveq   #0,d0
        move.w  LIB_NEGSIZE(a5),d0
        sub.l   d0,a1
        add.w   LIB_POSSIZE(a5),d0
        JSRLIB  FreeMem

        move.l  d2,d0
        movem.l (sp)+,d2/a5/a6
        rts


;-----------------------------------------------------------------------
CloseLibs:
        move.l  nv_DOSBase(a5),a1
        JSRLIB  CloseLibrary
        move.l  #0,nv_DOSBase(a5)
CloseLibs2:
        move.l  nv_LowLevelBase(a5),a1
        JSRLIB  CloseLibrary
        move.l  #0,nv_LowLevelBase(a5)
CloseLibs3:
        move.l  nv_UTILBase(a5),a1
        JSRLIB  CloseLibrary
        move.l  #0,nv_UTILBase(a5)
        rts


;-----------------------------------------------------------------------
Reserved:
        moveq   #0,d0
        rts





ROMTagName:     dc.b    'ad_'                   ; this must be followed by VERSTRING
VERSTR:         VSTRING
subsysName:     NONVOLATILELIBRARYNAME
DOSName:        dc.b    'dos.library',0
UTILName:       dc.b    'utility.library',0
LLName:         dc.b    'lowlevel.library',0


        section __MERGED,data

        end


