**
**      $Filename nofastmem.asm $
**      $Release: 1.0 $
**      $Revision: 1.7 $
**      $Date: 94/03/22 10:21:24 $
**
**      nofastmem resource module
**
**      (C) Copyright 1994 Commodore-Amiga, Inc.
**          All Rights Reserved
**
**      $Id: nofastmem.asm,v 1.7 94/03/22 10:21:24 jerryh Exp Locker: jerryh $

        SECTION nofastmem,code

**      Included Files

        INCLUDE         "exec/types.i"
        INCLUDE         "exec/nodes.i"
        INCLUDE         "exec/lists.i"
        INCLUDE         "exec/execbase.i"
        INCLUDE         "exec/libraries.i"
        INCLUDE         "exec/resident.i"
        INCLUDE         "exec/initializers.i"
        INCLUDE         "exec/memory.i"

        INCLUDE         "nofastmem_rev.i"


**      Local Macros

XLVO    MACRO
        XREF    _LVO\1
        ENDM

**      Imported

        XLVO    AvailMem
        XLVO    AllocMem

************************************************************************
***
***  Standard Macros
***
************************************************************************

clear       macro   data-register
        moveq.l #0,\1
        endm

save        macro   (regs)
        movem.l \1,-(sp)
        endm

restore     macro   (regs)
        movem.l (sp)+,\1
        endm

exec        macro   function
        move.l  a6,-(sp)
        move.l  4,a6
        jsr LVO.\1(A6)
        move.l  (sp)+,a6
        endm

************************************************************************
***
***  Library Vector Offsets (LVO's)
***
************************************************************************

FUNCDEF     macro
LVO.\1      equ LVOFF
LVOFF       set LVOFF-6
        endm
LVOFF       set -5*6
    include "include:exec/exec_lib.i"


**********************************************************************

residentnfmr:
                dc.w    RTC_MATCHWORD
                dc.l    residentnfmr
                dc.l    nfmrEndModule
                dc.b    RTF_AUTOINIT!RTF_COLDSTART
                dc.b    VERSION
                dc.b    NT_RESOURCE
                dc.b    100
                dc.l    nfmrName
                dc.l    nfmrID
                dc.l    nfmrInitTable

nfmrName:
                dc.b    'nofastmem.resource',0

nfmrID:
                VSTRING


nfmrInitTable:
                dc.l    LIB_SIZE
                dc.l    nfmrFuncInit
                dc.l    nfmrStructInit
                dc.l    nfmrInit



RESDEF MACRO
                dc.w    nfmr\1-nfmrFuncInit
        ENDM

nfmrFuncInit:
                dc.w    -1
                RESDEF RemoveFastMem
                RESDEF RestoreFastMem
                dc.w    -1

nfmrStructInit:
                INITBYTE    LN_TYPE,NT_RESOURCE
                INITLONG    LN_NAME,nfmrName
                INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
                dc.w        0


;------ nofastmem.resource/Init ------------------------------------------
;
;
nfmrInit:
                rts



**********************************************************************
*
* Patch code
*
**********************************************************************

nfmr_Patch:

oldAvailMem:    dc.l    0
oldAllocMem:    dc.l    0

NoFastAvailMem:
    bset    #MEMB_CHIP,d1
    move.l  oldAvailMem(pc),a0
    jmp     (a0)

NoFastAllocMem:
    bset    #MEMB_CHIP,d1
    bclr    #MEMB_FAST,d1
    move.l  oldAllocMem(pc),a0
    jmp     (a0)

nfmr_EndPatch:



******* nofastmem.resource/RemoveFastMem *****************************
*
*   NAME
*       RemoveFastMem -- Remove fast memory from system
*
*   SYNOPSIS
*       RemoveFastMem()
*
*       void RemoveFastMem(void);
*
*   FUNCTION
*       Removes fast memory from system.
*
*   INPUTS
*
*   RESULTS
*       Fast memory is removed.
*
**********************************************************************

nfmrRemoveFastMem:

        exec    Forbid                                                  ; Disable task switching

        move.l  $4,a0                                                   ; Already patched? (this is a questionable test)
        move.l  _LVOAllocMem+2(a0),d0
        sub.l   _LVOAvailMem+2(a0),d0
        cmp.l   #NoFastAllocMem-NoFastAvailMem,d0
        beq     99$

        move.l  #nfmr_EndPatch-nfmr_Patch,d0                            ; Allocate patch memory
        move.l  #MEMF_PUBLIC,d1
        exec    AllocVec
        tst.l   d0
        beq     99$
        move.l  d0,a1

        save    a1                                                      ; Copy patch to memory
        lea     nfmr_Patch(pc),a0
        move.l  #nfmr_EndPatch-nfmr_Patch,d0
        exec    CopyMem
        restore a1

        move.l  $4,a0                                                   ; Prepare and implement patch
        move.l  _LVOAvailMem+2(a0),0(a1)
        move.l  _LVOAllocMem+2(a0),4(a1)
        move.l  #NoFastAvailMem-nfmr_Patch,d0
        add.l   a1,d0
        move.l  d0,_LVOAvailMem+2(a0)
        move.l  #NoFastAllocMem-nfmr_Patch,d0
        add.l   a1,d0
        move.l  d0,_LVOAllocMem+2(a0)
99$
        exec    Permit                                                  ; Reenable task switching
        rts



******* nofastmem.resource/RestoreFastMem ****************************
*
*   NAME
*       RestoreFastMem -- Restore fast memory to system
*
*   SYNOPSIS
*       RestoreFastMem()
*
*       void RestoreFastMem(void);
*
*   FUNCTION
*       Restores fast memory to system.
*
*   INPUTS
*
*   RESULTS
*       Fast memory is restored.
*
**********************************************************************

nfmrRestoreFastMem:

        exec    Forbid                                                  ; Disable task switching

        move.l  $4,a0                                                   ; Patch installed? (this is a questionable test)
        move.l  _LVOAllocMem+2(a0),d0
        sub.l   _LVOAvailMem+2(a0),d0
        cmp.l   #NoFastAllocMem-NoFastAvailMem,d0
        bne     99$

        move.l  _LVOAvailMem+2(a0),a1                                   ; Get patch copy base
        sub.l   #8,a1

        move.l  0(a1),_LVOAvailMem+2(a0)                                ; Restore original patch
        move.l  4(a1),_LVOAllocMem+2(a0)

        exec    FreeVec                                                 ; Free patch memory
99$
        exec    Permit                                                  ; Reenable task switching
        rts



nfmrEndModule:

        END
