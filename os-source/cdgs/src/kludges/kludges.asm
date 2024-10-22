************************************************************************
****************                                        ****************
****************     -=CDTV-CR SYSTEM SUPPORT =-        ****************
****************          "Kludges" module              ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***  Further modified for CDGS - 1993.				     ***
***                                                                  ***
***  Modified by Darren Greenwald 1992.  CDTV-CR patches are now     ***
***  applied only if booting off of CD, and then in some cases       ***
***  only applied based on the title.                                ***
***                                                                  ***
***   Written by Bryce Nesbitt for Commodore-Amiga, Inc.             ***
***   Additional patches by Andy Finkel for Commodore-Amiga, Inc.    ***
***   Copyright (C) 1991 Commodore-Amiga, Inc. All rights reserved.  ***
***                                  ***
************************************************************************

    NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/devices.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/exec.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/macros.i"
	INCLUDE "hardware/cia.i"
	INCLUDE "hardware/custom.i"
	INCLUDE "hardware/intbits.i"
	INCLUDE "exec/ables.i"
	INCLUDE "cd/cd.i"
	INCLUDE "kludges_rev.i"
	INCLUDE "debug.i"

	XREF    _intena         ;from amiga.lib
	XREF    _intenar
	XREF    _intreqr
	XREF    _custom
	XREF	_ciaa

	TASK_ABLES          ;for PERMIT macro

	; SilentStartup Patch

	INCLUDE "libraries/expansionbase.i"
	XREF    _LVOInitResident

	;Xiphias patch
	INCLUDE "intuition/screens.i"
	XREF    _LVOOpenScreen

	; Mouse queue limit patch
	XREF    _LVOOpenWindow
	XREF    _LVOSetMouseQueue

	; classic board games

	INCLUDE "intuition/intuitionbase.i"
	INCLUDE "devices/input.i"
	INCLUDE "devices/inputevent.i"

	; Music Maker
	XREF    _LVOAllocMem
	XREF    _LVOAvailMem

	; Battlestorm system-configuration patch
	XREF    _LVOSetPrefs

	; Mouse patch
	XREF	_LVOWritePotgo

	;;INCLUDE	"game.i"
	INCLUDE	"resources/potgo.i"

_LVOInstallJoyMouse	EQU	-$108

	;slow cpu patch
	XREF	_LVOAddTask
	OPT P=68020

	;LoadSeg patches
	XREF	_LVOLoadSeg

    LIST

SetpatchROMTag:
	DC.W    RTC_MATCHWORD
	DC.L    SetpatchROMTag
	DC.L    SetpatchEndMarker
	DC.B    RTF_COLDSTART
	DC.B    VERSION
	DC.B    NT_UNKNOWN
*	DC.B    -34     ; after CDStrap
	DC.B    -59     ; after CDStrap (CDGS)
	DC.L    nameString
	DC.L    idString
	DC.L    SetpatchTagEntry


***********************************************************************
** Two tables - one of names, the other of patch bits we will apply
** which currently limits us to 32, but should be enough to fix
** all 118 or so CDTV titles that exist.  Names are NULL terminated
** to save a bit of space, and the string compare code is assembly,
** so we can optimize easily enough
***********************************************************************

    BITDEF  PATCH,SILENTSTARTUP,0
    BITDEF  PATCH,POINTERPOS,1
    BITDEF  PATCH,BOGUSPATH,2
    BITDEF  PATCH,NOFASTMEM,3
    BITDEF  PATCH,NOSETPREFS,4
    BITDEF  PATCH,MOUSESTICK1,5
    BITDEF  PATCH,JOYMOUSE,6
    BITDEF  PATCH,MAKEBBUTTON,7
    BITDEF  PATCH,MOUSESTICKVBR,8
    BITDEF  PATCH,LOADSEG,9
    BITDEF  PATCH,NOCACHE,10

namesat:

	dc.l    title1			; WorldVistaAtlas
	dc.l    title2			; ClassicBoardGames
	dc.l    title3			; Context
	dc.l    title4			; Team_Yankee
	dc.l    title5			; WORLD_VISTA (yes, they changed the name)
	dc.l    title6			; Music Maker
	dc.l    title7			; Independent_Soccer
	dc.l    title8			; BattleStorm
	dc.l    title9			; Scary Poems (Discis)
	dc.l    title10			; SimCity
	dc.l	title11			; The Tale of Peter Rabbit (Discis)
	dc.l	title12			; The Paper Bag Princess (Discis)
	dc.l	title13			; The Tale of Benjamin Bunny (Discis)
	dc.l	title14			; Cinderella (Discis)
	dc.l	title15			; Moving gives me a ... (Discis)
	dc.l	title16			; Mud Puddle (Discis)
	dc.l	title17			; Long Hard Day at the Ranch (Discis)
	dc.l	title18			; Thomas' Snowsuit (Discis)
	dc.l	title19			; Lemmings
	dc.l	title20			; Heather hits her first ... (Discis)
	dc.l	title21			; Hutchinsons Dictionary
	dc.l	title22			; Le Monde

; terminate namesat table, and make first entry of patchtypes 0

patchtypes:
	dc.l    0

	dc.l    PATCHF_SILENTSTARTUP			;WorldVistaAtlas
	dc.l    PATCHF_POINTERPOS!PATCHF_JOYMOUSE	;ClassicBoardGames
	dc.l    PATCHF_BOGUSPATH			;Context
	dc.l    PATCHF_SILENTSTARTUP			;Team_Yankee
	dc.l    PATCHF_SILENTSTARTUP			;World_Vista
	dc.l    PATCHF_NOFASTMEM!PATCHF_SILENTSTARTUP   ;Music Maker
	dc.l    PATCHF_SILENTSTARTUP			;Independent_Soccer
	dc.l    PATCHF_NOSETPREFS			;BattleStorm
	dc.l	PATCHF_JOYMOUSE				;Scary Poems
	dc.l	PATCHF_MAKEBBUTTON!PATCHF_NOCACHE	;SimCity
	dc.l	PATCHF_JOYMOUSE				;Peter Rabbit
	dc.l	PATCHF_JOYMOUSE				;Paper Bag Princess
	dc.l	PATCHF_JOYMOUSE				;Benjamin Bunny
	dc.l	PATCHF_JOYMOUSE				;Cinderella
	dc.l	PATCHF_JOYMOUSE				;Moving gives me..
	dc.l	PATCHF_JOYMOUSE				;Mud Puddle
	dc.l	PATCHF_JOYMOUSE				;Long Hard Day...
	dc.l	PATCHF_JOYMOUSE				;Thomas Snowsuit
	dc.l	PATCHF_MOUSESTICKVBR!PATCHF_LOADSEG	;Lemmings
	dc.l	PATCHF_JOYMOUSE				;Heather hits her first home..
	dc.l	PATCHF_JOYMOUSE				;Hutchinsons Dictionary
	dc.l	PATCHF_MOUSESTICK1			;LeMonde

;
; names may be no longer than 33 bytes (Yes, its a magic number based
; on the names being padded with spaces to be a total of 34 bytes long.
; The padding is not needed here - the code below will check for the
; padding)
;

title1:
	dc.b    'WorldVistaAtlas',0
title2:
	dc.b    'ClassicBoardGames',0
title3:
	dc.b    'Context',0         ;Ultimate BasketBall
title4:
	dc.b    'TEAM_YANKEE',0
title5:
	dc.b    'WORLD_VISTA',0
title6:
	dc.b    'MM1F',0
title7:
	dc.b    'Independent_Soccer',0
title8:
	dc.b    'BAT',0
title9:
	dc.b	'SCARY_POEMS',0
title10:
	dc.b	'SIM_CITY',0
title11:
	dc.b	'PETER_RABBIT',0
title12:
	dc.b	'PAPER_BAG_PRINCESS',0
title13:
	dc.b	'BENJAMIN_BUNNY',0
title14:
	dc.b	'CINDERELLA',0
title15:
	dc.b	'MOVING',0
title16:
	dc.b	'MUD_PUDDLE',0
title17:
	dc.b	'RANCH',0
title18:
	dc.b	'THOMAS_SNOWSUIT',0
title19:
	dc.b	'LEMMINGS_PLANETSIDE',0
title20:
	dc.b	'HEATHER',0
title21:
	dc.b	'ATTICA_03',0
title22:
	dc.b	'Chrono',0

    CNOP    0,4

**=====================================================================
**=====================================================================
**
** Patch pointer table - be careful to add entries for every patch bit
** defined above!!   Must have one pointer, and one piece of code
** for every selective patch we may need to apply.
**
**=====================================================================
**=====================================================================

patchcode_table:

    dc.l    patch_silentstartup
    dc.l    patch_pointerpos
    dc.l    patch_badpath
    dc.l    patch_nofastmem
    dc.l    patch_nosetprefs
    dc.l    patch_mousestick
    dc.l    patch_joymouse
    dc.l    patch_makebbutton
    dc.l    patch_mousestickvbr
    dc.l    patch_loadseg
    dc.l    patch_nocache

MATCHLEN    EQU 34

***********************************************************************
** Find public port indicating we are booting off of CDTV title
**
** If not found, exit
**
** If found, free it, and find name of disk.
** Apply standard patches (patches for ALL CDTV-CR titles)
** Apply selective patches (patches for known CDTV titles based on name)
**
***********************************************************************

SetpatchTagEntry:

        movem.l a2/a3,-(sp)

    ; forbid is not needed - the port really cannot go away

        lea cdbootport(pc),a1

        FORBID

        JSRLIB  FindPort
        
        PERMIT

        tst.l   d0
        beq notbootingCD

    PRINTF  DBG_FLOW,<'BOOTING CDTV TITLE'>

    ; remove/free port left hanging around by CDStrap

        move.l  d0,a3

        move.l  a3,a1
        JSRLIB  RemPort

        move.l  a3,a1
        moveq   #MP_SIZE,d0
        JSRLIB  FreeMem

    ; check execbase rev just for safety (should never fail), but
    ; we don't want to crash under 1.3 ROM

        cmp.w   #37,LIB_VERSION(a6)
        BCS_S   notbootingCD        ;blt unsigned


    ; apply standard patches (AllocMem()'s but no memory
    ; fragmentation problems at this point)

        bsr StandardPatches

    ; setup for obtaining disk name, and searching table

        JSRLIB  CreateMsgPort
        tst.l   d0
        BEQ_S   nomsgport

        move.l  d0,a2           ;cache
        move.l  d0,a0
        moveq   #IOSTD_SIZE,d0

        JSRLIB  CreateIORequest

        tst.l   d0
        BEQ_S   noiorequest

        move.l  d0,a3           ;use IO request

        lea cddevname(pc),a0
        move.l  a3,a1
        moveq   #00,d0
        moveq   #00,d1

        JSRLIB  OpenDevice
        tst.l   d0
        BNE_S   nocddevice

    ;
    ; apply selective patches (may leave a small memory fragment
    ; for MsgPort, and IORequest above - not really a problem -
    ; the whole is small enough it should be filled in anyway)
    ;
        move.w  #CD_READ,IO_COMMAND(a3)
        move.l  #32808,IO_OFFSET(a3)    ;magic constant
        move.l  #(MATCHLEN+8),IO_LENGTH(a3)

        sub.l   #(MATCHLEN+8),sp
	clr.l	MATCHLEN(sp)

        move.l  sp,IO_DATA(a3)

        move.l  a3,a1

        JSRLIB  DoIO
        tst.l   d0
        bne.s   freenamebuf
        move.l  sp,IO_DATA(a3)      ;refresh IO_DATA

    PRINTF	DBG_FLOW,<'Title Name:%s'>,IO_DATA(A3)

        bsr SelectivePatches



    ; may apply custom selective patches now (if ever needed)
    ; title number is in D0
    ;
    ;   tst.l   d0
    ;   beq.s   freenamebuf     ;no match found
    ;
    ;   ...etc...



freenamebuf:

        lea (MATCHLEN+8)(sp),sp     ;restore stack

        move.l  a3,a1
        JSRLIB  CloseDevice

nocddevice:
        move.l  a3,a0
        JSRLIB  DeleteIORequest

noiorequest:
        move.l  a2,a0
        JSRLIB  DeleteMsgPort

nomsgport:


notbootingCD:
        movem.l (sp)+,a2/a3

    PRINTF  DBG_EXIT,<'Exiting Kludges'>
        rts


***********************************************************************
** Searches table for a CDTV title match (title name).  Returns
** title number in D0, or 0 meaning not found.
**
***********************************************************************

SelectivePatches:   ;(struct IOStdReq *)
            ;        A3

    PRINTF  DBG_ENTRY,<'SelectivePatches($%lx)'>,A3

        movem.l d2-d4/a2-a3,-(sp)

        move.l  IO_DATA(a3),d1      ;fetch once only

        moveq   #00,d2          ;title # (0 means NOT FOUND)
        lea namesat(pc),a2

        bra.s   nexttitle

trytitle:
        addq.w  #1,d2           ;add to title counter

        movea.l d1,a0           ;restore
        movea.l d0,a1

        moveq   #(MATCHLEN-1),d0

    ; this is a fast search since string compares tend to fail
    ; early

maymatch:
        cmp.b   (a0)+,(a1)+
        dbne    d0,maymatch

        tst.b   -(a1)           ;did we match up to the NULL byte?
        bne.s   nexttitle

; assumes cdtitle name in table is not max length
;
;       tst.w   d0
;       bmi.s   nexttitle

matchpad:

        cmp.b   #' ',(a0)+
        dbne    d0,matchpad         

        tst.w   d0
        bpl.s   foundtitle

nexttitle:

        move.l  (a2)+,d0        ;next string pointer
        bne.s   trytitle        ;return 0 (no match found)

patched:
        movem.l (sp)+,d2-d4/a2-a3

    PRINTF  DBG_EXIT,<'%ld=SelectivePatches()'>,D0

        rts

**
** Name Match!!  Get patches to apply
**

foundtitle:
        move.l  d2,d1
        lsl.w   #2,d1
        lea patchtypes(pc),a0

        move.l  0(a0,d1.w),d3

    PRINTF  DBG_FLOW,<' TITLE #%ld PATCHES=$%lx'>,D2,D3

        moveq   #-1,d4          ;first time roll from -1 -> 0

nextpatch:

        addq.l  #1,d4           ;modify all 32 bits

        move.l  d2,d0           ;return title number of done

        tst.l   d3          ;terminate loop early if done
        BEQ_S   patched

    ; d4 == 0 through 31, and then wraps to an effective 0 by which
    ; time d3 must also be 0, and the loop terminates

        bclr    d4,d3
        beq.s   nextpatch

    ; use bit # as index into table of patches to apply

        move.l  d4,d0           ;high word is 0
        lsl.w   #2,d0

        lea patchcode_table(pc),a0
        move.l  0(a0,d0.w),a0
        jsr (a0)            ;code may use A2

        bra.s   nextpatch

***********************************************************************
** Apply patches which will be added regardless of CDTV we are booting
** from.
**
***********************************************************************

StandardPatches:

    PRINTF  DBG_ENTRY,<'StandardPatches()'>


        movem.l a2-a3,-(sp)
        lea LibList(a6),a0
        lea IntuiName(pc),a1

        FORBID
        JSRLIB  FindName        ; do not OpenLibrary()
                        ; Intuition!

        move.l  d0,a1           ;check later
        move.l  d0,a3           ;cache
        move.w  #(_LVOOpenScreen),a0
        lea	patch_xiphias(pc),a2
        bsr.s   AddPatch

        move.l  a3,a1
        move.w  #(_LVOOpenWindow),a0
        lea	patch_mqueue(pc),a2
        bsr.s   AddPatch

        PERMIT

        movem.l (sp)+,a2-a3     

    PRINTF  DBG_EXIT,<'VOID=StandardPatches()'>

        rts

**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Add patch entry (newcode, liboffset, libptr)
**          A2       a0         a1
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OFFSET_OLDJMP   equ 10
OFFSET_NEWJMP   equ 6

AddPatch:
    
    ; make sure we have a library base, and we only patch 2.0 or greater
    ; let caller deal with other exceptions

    move.l  a1,d0
    BEQ_S   libbase_invalid
    cmp.w   #37,LIB_VERSION(a1)
    bcs.s   libbase_invalid

    movem.l a5-a6,-(sp)
    movem.l a0-a1,-(sp)

    moveq   #(patch_templateE-patch_template),d0

    move.l  #MEMF_PUBLIC,d1
    move.l  $4,a6
    JSRLIB  AllocMem
    tst.l   d0
    beq.s   patch_lowmem        ;???

        move.l  d0,a5
        lea patch_template(pc),a0
        move.l  a5,a1
        moveq   #(patch_templateE-patch_template),d0
        JSRLIB  CopyMem

    ; patch JMP instruction
    
        move.l  a2,OFFSET_NEWJMP(a5)

    ; SetFunction

        movem.l (sp)+,a0-a1
        subq.l  #8,sp       ;munge stack for exit code

        move.l  a5,d0

        FORBID

    ; SetFunction() also clears cache

        JSRLIB  SetFunction

    ; CacheClearU() not needed - writing to data, not instruction

        move.l  d0,OFFSET_OLDJMP(a5)
        PERMIT

patch_lowmem:

    movem.l (sp)+,a0-a1
    movem.l (sp)+,a5-a6

libbase_invalid:

    rts

;;
;;
;; Patch entry template
;;
;;

patch_template:
        move.l  patch_temp_old(pc),-(sp)
        jmp $12345678
patch_temp_old:
        dc.l    $12345678
patch_templateE:


**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Add an AFTERDOS ROMTAG entry (code)
**                        A2
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 STRUCTURE DOSROMTAG,0
    APTR    DRT_CODE
    APTR    DRT_OLD
    APTR    DRT_NEXT
    STRUCT  DRT_ROMTAG,RT_SIZE
    LABEL   DOSROMTAG_SIZEOF

AddDOSPatch:
        movem.l a5-a6,-(sp)

        move.l  $4,a6

        moveq   #DOSROMTAG_SIZEOF,d0
        move.l  #(MEMF_PUBLIC!MEMF_CLEAR),d1
        JSRLIB  AllocMem
    
        tst.l   d0
        BEQ_S   AddDOSPatch_Fail

        move.l  d0,a5

        move.l  a2,DRT_ROMTAG+RT_INIT(a5)

        move.l  #DOSPatchName,DRT_ROMTAG+RT_NAME(a5)
        move.l  #DOSPatchId,DRT_ROMTAG+RT_IDSTRING(a5)

    ; TYPE/VERSION -- implied 0

        move.b  #RTF_AFTERDOS,DRT_ROMTAG+RT_FLAGS(a5)

    ; Before ramlib - due to a bug in ramlib's init code which
    ; trashes d2-d4 -- ACKK!!!! to be fixed in V39 KS

        move.b  #-90,d1
        move.b  d1,DRT_ROMTAG+RT_PRI(a5)

    ; endskip - doesn't matter - added after ROMTAG search

    ; finally matchword, and backpointer (doesnt really matter either)

        move.w  #RTC_MATCHWORD,DRT_ROMTAG+RT_MATCHWORD(a5)
        lea DRT_ROMTAG(a5),a0
        move.l  a0,DRT_ROMTAG+RT_MATCHTAG(a5)

    ; link -- add another item to the ResModule chain

        move.l  a0,DRT_CODE(a5)

    ; add to exec's resident tag list

        move.l  ResModules(a6),a0

        FORBID
add_walkrlist:
        move.l  (a0)+,d0
        beq.s   add_rend        ;???what???
        bmi.s   add_contrlist

        move.l  d0,a1
        cmp.b   RT_PRI(a1),d1
        blt.s   add_walkrlist       ;signed compare
        bra.s   add_rlist

add_contrlist:
        bclr    #31,d0          ;continue list from
        move.l  d0,a0
        bra.s   add_walkrlist

add_rlist:
    ; link me in

        move.l  a0,d0
        ori.l   #$80000000,d0
        move.l  d0,DRT_NEXT(a5)     ;backlink
add_rend:
        move.l  -(a0),DRT_OLD(a5)   ;save on my list

        move.l  a5,d0
        or.l    #$80000000,d0       ;set extension bit
        move.l  d0,(a0)         ;continue chain in my space

    PRINTF  DBG_FLOW,<'  Add AFTERDOS ROM-TAG $%lx to $%lx'>,A5,A0

        PERMIT

        move.l  a5,d0               ;return pointer

AddDOSPatch_Fail:
        movem.l (sp)+,a5-a6
        rts

**---------------------------------------------------------------------
**
** Patch silent startup (disable silent-startup)
**
**---------------------------------------------------------------------

patch_silentstartup:

    PRINTF  DBG_ENTRY,<'  Patch SilentStartup'>

        lea pss_code(pc),a2
        move.w  #(_LVOInitResident),a0
        move.l  $4,a1               ;execbase
    
            bra AddPatch            ;and RTS

* Patch InitResident -
*
* This patch clears the silent-startup bit during InitResident of ramlib,
* hence cleared after STRAP, but before DOS checks the magic silent
* startup bit.
*
* This causes WB to open immediately, and fix bug in programs which have
* trouble with WB opening late, or never.
*
* Examples:
*
*   WorldVistaMaps - Outputs to stdout when bring up map, causing
*   a change in window activation.  They rely on their own screen's
*   window being active and behind their gfx View for mouse position
*   tracking.  Without this patch the Shell window delays opening,
*   and activating until they output, changing their window activation.
*

pss_code:
        movem.l d0-d1/a0-a1,-(sp)   ;save d1,a1

        lea RamLibName(pc),a0
        move.l  RT_NAME(a1),a1

        moveq   #((RamLibNameE-RamLibName)-1),d0

pss_dosonly:
        cmp.b   (a0)+,(a1)+
        dbne    d0,pss_dosonly

        tst.w   d0
        bpl.s   nomatchdos

        lea.l   LibList(a6),a0
        lea.l   ExpName(pc),a1

        FORBID

        JSRLIB  FindName
        move.l  d0,a0
        tst.l   d0
        beq.s   noexplib    ;"impossible" situation
        bclr    #EBB_SILENTSTART,eb_Flags(a0)       

    PRINTF  DBG_FLOW,<'!!SilentStartup OFF'>

noexplib:
        PERMIT
nomatchdos:
        movem.l (sp)+,d0-d1/a0-a1 ;restore d1/a1
        rts

**---------------------------------------------------------------------
**
** Patch Intuition pointer position - not great, but hopefully usable
**
** GACK!!  This is a bug in ClassicBoardGames -- pokes
** ibase MouseX/Y which use to actually move the mouse under 1.3.
** They did this because they have 3 screens, and an interface for
** which a mouse isn't desirable -- hence they force it stay in the
** main window (though its not visible).
**
**---------------------------------------------------------------------

 STRUCTURE  POINTERPOS,0
    STRUCT  PP_MP,IOSTD_SIZE
    STRUCT  PP_INTSTRUCT,IS_SIZE
    APTR    PP_IBASE
    APTR    PP_IBASECODE
    ULONG   PP_IBASEDATA
    UWORD   PP_LASTY
    UWORD   PP_LASTX
    STRUCT  PP_IEVENT,ie_SIZEOF
    LABEL   POINTERPOS_SIZEOF

patch_pointerpos:

    PRINTF  DBG_ENTRY,<'  Patch PointerPos Pokers'>

        movem.l a2/a3/a6,-(sp)

        move.l  $4,a6
        JSRLIB  CreateMsgPort
        move.l  d0,a3

        tst.l   d0
        beq pss_pplowmem
        
    ; this memory will hang around until reset

        moveq   #POINTERPOS_SIZEOF,d0
        move.l  #MEMF_PUBLIC!MEMF_CLEAR,d1
        JSRLIB  AllocMem

        move.l  d0,a2

        tst.l   d0
        beq pss_nopointerpos
    
        lea IntuiName(pc),a1
        moveq   #37,d0
        JSRLIB  OpenLibrary     ;start intuition handler

        move.l  d0,PP_IBASE(a2)
        beq pss_noidevice       ;???? 

        move.b  #IECLASS_POINTERPOS,PP_IEVENT+ie_Class(a2)

        move.b  #NT_REPLYMSG,LN_TYPE(a2)
        move.l  a3,MN_REPLYPORT(a2)

        move.l  a2,a1
        lea InputName(pc),a0
        moveq   #00,d0
        moveq   #00,d1
        JSRLIB  OpenDevice

        tst.l   d0
        bne.s   pss_noidevice

        ;; before intuition

        move.l  a2,PP_INTSTRUCT+IS_DATA(a2)
        move.l  #pss_fixxy,PP_INTSTRUCT+IS_CODE(a2)
        move.b  #51,PP_INTSTRUCT+LN_PRI(a2)
        
        ;; then add handler which may modify mouse position

        lea PP_INTSTRUCT(a2),a0
        move.l  a0,IO_DATA(a2)
        move.w  #IND_ADDHANDLER,IO_COMMAND(a2)

        move.l  a2,a1
        JSRLIB  DoIO

        tst.l   d0
        bne.s   pss_noidevice           ;???

        ;; hook in after Intuition, regardless of what Intuition
        ;; drops through the input chain

        DISABLE

        lea PP_INTSTRUCT(a2),a0

pss_findibasehandler:

        move.l  LN_SUCC(a0),d0
        beq.s   pss_noibasehandler      ;???

        move.l  d0,a0

        cmp.b   #50,LN_PRI(a0)          ;string compare
        bne.s   pss_findibasehandler        ;not really needed

        move.l  IS_CODE(a0),PP_IBASECODE(a2)
        move.l  IS_DATA(a0),PP_IBASEDATA(a2)
        move.l  a2,IS_DATA(a0)
        move.l  #pss_cachexy,IS_CODE(a0)

pss_noibasehandler:

        ENABLE

        bra.s   pss_nopointerpos


pss_noidevice:
        move.l  a2,a1
        moveq   #POINTERPOS_SIZEOF,d0
        JSRLIB  FreeMem
        
pss_nopointerpos:

    ; must free signal bit, and no longer needed

        move.l  a3,a0
        JSRLIB  DeleteMsgPort
pss_pplowmem:
        movem.l (sp)+,a2/a3/a6
        rts

    ;;
    ;; cache last MouseY/X in ibase -- call back Intuition, then
    ;; cache y/x, then return to input.device task
    ;;

pss_cachexy:

        movem.l a2-a3,-(sp)
        move.l  a1,a3

        move.l  PP_IBASECODE(a3),a2
        move.l  PP_IBASEDATA(a3),a1

        jsr (a2)            ;call Intuition handler

    ;; no touch D0 - has event list, or NULL from Intuition

        move.l  PP_IBASE(a3),a0     ;cache mouse y/x
        move.l  ib_MouseY(a0),PP_LASTY(a3)
        movem.l (sp)+,a2-a3
        rts

    ;;
    ;; check if ibase mouse x/y poked; if so force mouse to absolute
    ;; position
    ;;

pss_fixxy:

        move.l  a0,d0

        move.l  PP_IBASE(a1),a0
        move.l  ib_MouseY(a0),d1

    ; sniff out poke of ibase

        cmp.l   PP_LASTY(a1),d1
        beq.s   pss_notpoked

        swap    d1

        lea PP_IEVENT(a1),a0
        move.l  d1,ie_X(a0)

        move.l  d0,ie_NextEvent(a0)
        move.l  a0,d0           ;AddHead()

pss_notpoked:
        rts

**---------------------------------------------------------------------
**
** Disable internal PATH command.
** 
** Ultimate BasketBall tries to add PATH sys:prefs, which doesn't
** exist.  Since PATH is now internal, it is used, and it returns
** an error code of 20, unlike their old PATH command.
**
**---------------------------------------------------------------------
patch_badpath:

    PRINTF  DBG_ENTRY,<'  Patch PATH command'>

        lea pss_badpath(pc),a2
        bra AddDOSPatch     ;and RTS

pss_badpath:

    PRINTF  DBG_ENTRY,<'!!Disable internal PATH command'>

        movem.l d2-d3/a6,-(sp)
        move.l  $4,a6
        lea DosName(pc),a1
        moveq   #37,d0
        JSRLIB  OpenLibrary

        tst.l   d0
        beq.s   pss_bpathnodos

        move.l  d0,a6
        move.l  #NoInternalPath,d1
        moveq   #00,d2          ;no I/O handles
        moveq   #00,d3

        JSRLIB  Execute

        move.l  a6,a1
        move.l  $4,a6
        JSRLIB  CloseLibrary

pss_bpathnodos:
        movem.l (sp)+,d2-d3/a6
        rts

**---------------------------------------------------------------------
**
** Patch LoadSeg for the purpose of patching executable code
** 
** Patch Lemmings so that it does not poke POTGO
**
**---------------------------------------------------------------------
patch_loadseg:

    PRINTF  DBG_ENTRY,<'  Patch LoadSeg()'>

        lea pss_loadseg(pc),a2
        bra AddDOSPatch     ;and RTS

pss_loadseg:
    PRINTF  DBG_ENTRY,<'  LoadSeg() patch invoked'>

	movem.l	d2-d3/a2/a6,-(sp)
	move.l	$4,a6
	lea	DosName(pc),a1
	moveq	#37,d0
	JSRLIB	OpenLibrary

	tst.l	d0
	beq.s	pss_loadsegfailed

        lea 	pss_newloadseg(pc),a2
        move.w  #(_LVOLoadSeg),a0
        move.l  d0,a1               ;execbase
        bsr AddPatch
	
pss_loadsegfailed:
	movem.l	(sp)+,d2-d3/a2/a6
	rts

pss_newloadseg:
        	move.l  (sp)+,a0
		movem.l	a2/a3/a6,-(sp)
		move.l	d1,a2

	PUSHLONG	D1
	PRINTF	DBG_FLOW,<'LoadSeg(%s)'>
	POPLONG		1

        	jsr (a0)

		move.l	d0,-(sp)	;cache segment base
		beq	pss_nlsfailed
		
		lea	loadseg_titles(pc),a1
		tst.l	(a1)		;end of list?
		beq.s	pss_nlsfailed

		move.l	(a1)+,a0	;name of title
		move.l	(a1)+,a6	;code for title
		move.l	a2,a3
patch_findtitle:
		cmp.b	(a0)+,(a3)+
		bne.s	pss_nlsfailed
		tst.b	-1(a0)		;end of title name?
		bne.s	patch_findtitle

patch_title:
		jmp	(a6)

pss_clearcache:
		move.l	$4,a6
		JSRLIB	CacheClearU

pss_nlsfailed:
		move.l	(sp)+,d0	;return segment pointer
		movem.l	(sp)+,a2/a3/a6
		rts


LemmingsPatch:
	PRINTF	DBG_FLOW,<'Lemmings LoadSeg() patch'>

lemmings_nextseg
		lsl.l	#2,d0		;BPTR->APTR
		beq.s	pss_nlsfailed

		move.l	d0,a0
		move.l	-4(a0),d1	;length of segment in bytes
		move.l	(a0)+,d0
		sub.l	#(6+6),d1	;subtract segment header
					;+size of match of string
		bls.s	lemmings_nextseg

lemmings_scanseg:
		subq.l	#2,d1
		bls.s	lemmings_nextseg

		cmp.w	#$3D7C,(a0)+	;find move.w #$CC00,(0034,A6)
		bne.s	lemmings_scanseg

		cmp.w	#$CC00,(a0)
		bne.s	lemmings_scanseg

		cmp.w	#$0034,2(a0)
		bne.s	lemmings_scanseg

	PRINTF	DBG_FLOW,<'Lemmings code found'>

		subq.w	#2,a0

		move.l	#$4E714E71,(a0)+
		move.w	#$4E71,(a0)	;fill with 3 NOP's

		BRA_S	pss_clearcache
		

loadseg_titles:
		dc.l	LemmingsName
		dc.l	LemmingsPatch
		dc.l	0		;terminate table

LemmingsName:
		dc.b	'Lemmings',0
		ds.w	0

**---------------------------------------------------------------------
**
** Xiphias patch
**
**---------------------------------------------------------------------

patch_xiphias:
        move.w  ns_Depth(a0),d0
        bne.s   pch_goold       ;Not zero bitplanes...
        move.w  ns_Type(a0),d0
        and.w   #CUSTOMBITMAP!SCREENQUIET!CUSTOMSCREEN,d0
        cmp.w   #CUSTOMBITMAP!SCREENQUIET!CUSTOMSCREEN,d0
        bne.s   pch_goold
        move.l  ns_CustomBitMap(a0),d0
        beq.s   pch_goold       ;No custom bitmap
        move.l  d0,a1
        move.b  bm_Depth(a1),d0
        bne.s   pch_goold       ;Bitmap not zero planes...

    PRINTF  DBG_FLOW,<'!!Xiphias title detected'>

        ;
        ; Xiphias title detected.  They open as zero bitplanes
        ; to work around a bug in 1.3; even with SCREENQUIET
        ; the area under the title bar is cleared.  The 2.04
        ; vector image creation pukes on zero bitplanes.
        ;
        ; To fix, we set the depth to 1 for the duration of
        ; OpenScreen.  Xiphias requires the depth be left at
        ; zero so they can open a window without disturbing
        ; the bitmap.
        ;
        move.b  #1,bm_Depth(a1)
            move.l  (sp)+,a1
            jsr (a1)
        tst.l   d0
        beq.s   pch_exit
            move.l  d0,a0
            clr.b   sc_BitMap+bm_Depth(a0)
pch_goold:
pch_exit:   rts

**---------------------------------------------------------------------
**
** MouseQueue limit patch -- many CDTV titles do not like mouse queue
** limits - bring back 1.3 behavior by making it big.  Problem is titles
** which used goofy code to throw away extra mice events (e.g., by
** throwing away a 'tuned' number of intuition messages after each event).
**
**---------------------------------------------------------------------

patch_mqueue:

        movem.l a0/a1,-(sp)
        move.l  8(sp),a1    ; get the address of openwindow
        jsr (a1)
        movem.l (sp)+,a0/a1 ;restore a0/a1
        addq.l  #4,sp   ; throw away the address of OpenWindow

        tst.l   d0
        beq.s   patch_mqfail    ;openwindow failed, skip the set

        movem.l d0/a0,-(sp) ; save return, and A0 for register
                    ; dependency in some titles

        move.l  d0,a0
        move.l  #999,d0     ;lots!
        jsr _LVOSetMouseQueue(a6)

        movem.l (sp)+,d0/a0
patch_mqfail:

        rts


**---------------------------------------------------------------------
**
** NoFatMem patch.  Amazingly enough in this day and age, some titles
** don't work properly with fast ram.  I know, boggles the mind.
** This patch was put in for MusicMaker, which loses its graphics
** if fast ram is in place.
**
**---------------------------------------------------------------------


patch_nofastmem:
        lea pss_nofastmem(pc),a2
        move.w  #(_LVOAllocMem),a0
        move.l  $4,a1               ;execbase
        bsr AddPatch

        lea pss_nofastmem(pc),a2
        move.w  #(_LVOAvailMem),a0
        move.l  $4,a1               ;execbase
        bra AddPatch            ;and RTS

; same patch works for both allocmem and availmem

pss_nofastmem:
	bset    #MEMB_CHIP,d1
	bclr    #MEMB_FAST,d1
	rts


**---------------------------------------------------------------------
** Patch for a bad SetPrefs
** (Battlestorm has a bad system-configuration file)
**---------------------------------------------------------------------
patch_nosetprefs:
        PRINTF  DBG_ENTRY,<'  Patch SetPrefs'>

        lea LibList(a6),a0
        lea IntuiName(pc),a1

        FORBID
        JSRLIB  FindName        ; do not OpenLibrary()
                        ; Intuition!

        move.l  d0,a1           ; OK, got IBase
                            ;(addpatch checks for null)

        lea pss_setprefs(pc),a2
        move.w  #(_LVOSetPrefs),a0
            bra AddPatch            ; and return

pss_setprefs:
        clr.l   d0          ; length of 0, nothing happens
        rts

		
**---------------------------------------------------------------------
** Patch to turn off caches
** (SimCity gets garbage in status maps if cache enabled)
**---------------------------------------------------------------------
patch_nocache:
        PRINTF  DBG_ENTRY,<'  Disable CPU caches'>

	moveq	#00,d0
	move.l	#(CACRF_EnableI!CACRF_EnableD!CACRF_CopyBack),d1
	JSRLIB	CacheControl
        rts

		
**---------------------------------------------------------------------
**
** Mouse emulating patch
** (Many CDTV titles expect a mouse in port 0)
**
** This one uses InstallJoyMouse() to add an input handler
** - for those titles that use Intuition events, this patch
** is less drastic than the hardware mouse emulation
**
**---------------------------------------------------------------------
patch_joymouse:
	PRINTF	DBG_ENTRY,<'  InstallJoyMouse() patch'>

		move.l	a6,-(sp)
		lea	playerprefsname(pc),a1
		moveq	#00,d0
		JSRLIB	OpenLibrary

		tst.l	d0
		beq.s	pss_nojoymouse

		move.l	d0,a6

	PRINTF	DBG_OSCALL,<'    InstallJoyMouse()'>

		jsr	_LVOInstallJoyMouse(a6)

	PRINTF	DBG_OSCALL,<'    $%lx=InstallJoyMouse()'>,D0

pss_nojoymouse:
		move.l	(sp)+,a6
		rts
		
**---------------------------------------------------------------------
** Mouse emulation patch
** (Many CDTV titles expect a mouse in port 0)
**
** This one actually tries to emulate what a mouse looks like
** at the hardware level by using joytest to change the
** mouse counters, setting the fire button to an output and
** toggling the bit on/off, and emulating the B button by
** wedging into WritePotgo() such that we set the corresponding
** B button on port 0 as an input, or output.
**
** The emulation is not exact, and will be a little slow on
** a PAL unit (since it polls for joystick direction at VBLANK
** time), but the idea is to make these titles usable with
** a game controller only.  Actually the patch waits for
** a mouse at port 0, or a game controller + a down button
** at port 1.  If a mouse is seen first, it does nothing
** else forever.  If a game controller + button is seen,
** it goes into emulation mode forever.
**
**---------------------------------------------------------------------

 STRUCTURE  MOUSESTICK,0
    STRUCT  MSTK_INTERRUPT,IS_SIZE		;VBLANK interrupt
    APTR    MSTK_POTGORES
    UWORD   MSTK_POTGOBITS
    UWORD   MSTK_VBRPATCH
    UWORD   MSTK_MODE				;emulation mode
    UBYTE   MSTK_FLAGS
    UBYTE   MSTK_SKIPVB
    UBYTE   MSTK_SKIPCOUNT
    UBYTE   MSTK_PAD
    LABEL   MOUSESTICK_VARIABLES
    LABEL   MOUSESTICK_SIZEOF			;code is relative to patch memory

	BITDEF	DATRY,POT,14
	BITDEF	OUTLY,POT,11
	BITDEF	DATLY,POT,10
	BITDEF	START,POT,0

	BITDEF	IS,MOUSE,0
	BITDEF	IS,GAMEC,1
        BITDEF	IS,BDOWN,2
        BITDEF	IS,IRSKIP,3

	BITDEF  JOYDAT,X0,0
	BITDEF  JOYDAT,X1,1
	BITDEF  JOYDAT,Y0,8
	BITDEF  JOYDAT,Y1,9

VBDATAPTR	EQU	1024			;end of allocated vector space
LV3VECTOR	EQU	1028
LV2VECTOR	EQU	1032

	; Do a full hardware mouse emulation, except
	; also moves the interrupts so that we can sniff out
	; applications which play with the vertical blank interrupt
	; vector

patch_mousestickvbr:
		move.l	#$00010001,a2
		bra.s	patch_stick

	; make B Button only makes B button hardware events

patch_makebbutton:
		suba.l	a2,a2			;flag
		bra.s	patch_stick
patch_mousestick:
		suba.l	a2,a2
		addq.w	#1,a2
		
patch_stick:
        PRINTF  DBG_ENTRY,<'  Patch Mouse emulation'>

		move.l	a5,-(sp)

		moveq	#MOUSESTICK_SIZEOF,d0
		add.l	#pss_patchpotgoend-pss_patchpotgo,d0

		move.l	#MEMF_PUBLIC!MEMF_CLEAR!MEMF_REVERSE,d1
		JSRLIB	AllocMem
		tst.l	d0
		beq	patch_mstick_failed

		move.l	d0,a0
		move.l	a2,MSTK_VBRPATCH(a0)	;and MSTK_MODE
		movea.l	a0,a2

	; set VBLANK skip rate for IR pulse emulation

		move.b	#(3-1),MSTK_SKIPCOUNT(a2)	;at 60Hz, 20 IR pulses/sec * 8 steps per pulse


	; if VBR replacement needed, try to do this for
	; if 68020 or better

		tst.w	MSTK_VBRPATCH(a2)
		beq.s	patch_notvbr

		move.w	AttnFlags(a6),d0
		btst	#AFB_68010,d0
		beq	patch_mstick_nolib

		lea	pss_getvbr(pc),a5
		JSRLIB	Supervisor
		tst.l	d0			;if already moved, no point in patching
		bne	patch_mstick_nolib	;the title is already too broken to fix

		move.l	#(1024+12),d0
		move.l	#MEMF_PUBLIC!MEMF_REVERSE,d1
		JSRLIB	AllocMem
		
		tst.l	d0
		beq	patch_mstick_nolib

	; copy vectors, and assume they are not going to change suddenly

		move.l	d0,a1
		move.l	d0,a5
		suba.l	a0,a0			;VBR is known to be 0
		move.w	#(256-1),d0

patch_copyvectors:
		move.l	(a0)+,(a1)+
		dbf	d0,patch_copyvectors
				
	; this is a kludge, so use some of the vector space for
	; private data

		move.l	$68,LV2VECTOR(a5)	;cache level 2 vector
		move.l	$6c,LV3VECTOR(a5)	;cache level 3 vector
		move.l	a2,VBDATAPTR(a5)	;and pointer to data

		lea	pss_makestick(pc),a0
		move.l	a0,$6c(a5)

		lea	pss_mousel2(pc),a0	;and catch other important game interrupts
		move.l	a0,$68(a5)

		lea	pss_mousel4(pc),a0
		move.l	a0,$70(a5)

		lea	pss_mousel6(pc),a0
		move.l	a0,$78(a5)

		move.l	a5,d0
		lea	pss_setvbr(pc),a5
		JSRLIB	Supervisor

patch_notvbr:
	; copy potgo patch code so that we can get at our
	; data relative to the PC of the code

		lea	MOUSESTICK_VARIABLES(a2),a1
		lea	pss_patchpotgo(pc),a0
		moveq	#((pss_patchpotgoend-pss_patchpotgo)-1),d0
patch_forpotgo:
		move.b	(a0)+,(a1)+
		dbf	d0,patch_forpotgo

	; clear all caches before attempting to use this moved code

		JSRLIB	CacheClearU

	; now install code to protect potgo bits from being modified
	; by other callers, and to possibly force the B button on/off

		lea	potgoname(pc),a1
		JSRLIB	OpenResource

		move.l	d0,MSTK_POTGORES(a2)
		beq	patch_mstick_nolib

	        move.l  d0,a1

		move.l	a4,-(sp)
		move.l	a2,a4

		lea	MOUSESTICK_VARIABLES(a4),a2
        	move.w  #_LVOWritePotgo,a0
		bsr	AddPatch

	; Low priority interrupt server; typically after most
	; game servers would run.  Changes we make to joy0dat will
	; be picked up the next time through the server chain, or
	; by a task running between interrupts.  We also choose
	; low priority to minimize latency for other vertical blank
	; code


		move.b	#NT_INTERRUPT,LN_TYPE(a4)
		move.b	#-124,LN_PRI(a4)
		move.l	#nameString,LN_NAME(a4)
		move.l	a4,IS_DATA(a4)
		lea	pss_mousestick(pc),a0
		move.l	a0,IS_CODE(a4)
		move.l	a4,a1
		moveq	#INTB_VERTB,d0
		JSRLIB	AddIntServer

		move.l	(sp)+,a4

	PRINTF	DBG_FLOW,<'   Mouse Emulation installed'>

		bra.s	patch_mstick_failed

patch_mstick_nolib:

		move.l	a2,a1
		moveq	#MOUSESTICK_SIZEOF,d0
		add.l	#pss_patchpotgoend-pss_patchpotgo,d0
		JSRLIB	FreeMem

patch_mstick_failed:
		move.l	(sp)+,a5
		rts

pss_patchpotgo:
		lea	pss_patchpotgo(pc),a0
		suba.w	#MOUSESTICK_VARIABLES,a0
		btst	#ISB_GAMEC,MSTK_FLAGS(a0)
		beq.s	pss_notpotgo

		movem.l	d0-d1,-(sp)
		or.w	d1,d0
		btst	#STARTB_POT,d0		;if start bit set, do nothing special
		movem.l	(sp)+,d0-d1
		bne.s	pss_notpotgo

		or.w	#(OUTLYF_POT!DATLYF_POT),d1

		and.w	#(~(OUTLYF_POT!DATLYF_POT)),d0
		or.w	MSTK_POTGOBITS(a0),d0

pss_notpotgo:
		rts

pss_patchpotgoend:
		nop
		nop


pss_getvbr:
		movec.l	VBR,d0
		rte

pss_setvbr:
		movec.l	d0,VBR
		rte


	; replace level 2 interrupt
pss_mousel2:
		move.l	$68,-(sp)
		rts

	; replace level 4 interrupt
pss_mousel4:
		move.l	$70,-(sp)
		rts

	; replace level 6 interrupt
pss_mousel6:
		move.l	$78,-(sp)
		rts


	; replace level 3 interrupt

pss_makestick:
		movem.l	d0/d1/a0/a1/a5/a6,-(sp)

		movec.l	VBR,D0

		move.l	d0,a1
		move.l	LV3VECTOR(a1),d0
		cmp.l	$6c,d0			;has this been munged?
		beq.s	pss_samevector

		move.l	VBDATAPTR(a1),a1

	;; do not bother if not game controller mode

		btst	#ISB_GAMEC,MSTK_FLAGS(a1)
		beq.s	pss_samevector

		lea	_custom,a0

		move.w	intenar(a0),d0		;intercept only valid vertical blank

	;; who cares about spurious - the games don't!
	;;	btst	#INTB_INTEN,d0
	;;	beq.s	pss_samevector

	; for lemmings, check if copper interrupts enabled but not VBLANK
	; only call my code if its a game controller

		btst	#INTB_VERTB,d0
		beq.s	pss_vbdisabled

		and.w	intreqr(a0),d0
		btst	#INTB_VERTB,d0
		beq.s	pss_samevector


pss_emulatemouse:
		bsr	pss_mousestick

pss_samevector:
		movem.l	(sp)+,d0/d1/a0/a1/a5/a6
		move.l	$6c,-(sp)		;jmp to original code
		rts
				
	; must be copper or blitter interrupt

pss_vbdisabled:

	; verify this is a VBLANK PENDING so we get (almost) consistent timing

		move.w	intreqr(a0),d0
		and.w	#INTF_VERTB,d0
		beq.s	pss_samevector
		move.w	#INTF_VERTB,intreq(a0)
		bra.s	pss_emulatemouse


pss_mousestick:

		movem.l	d2-d3/a5-a6,-(sp)

		move.l	a1,a5

	; do not set flag until int server is added, and all code
	; is running

		bset	#ISB_GAMEC,MSTK_FLAGS(a5)
		bra.s	pss_isstick

pss_unknown:
		lea	_custom,a0

		movem.l	(sp)+,d2-d3/a5-a6
		moveq	#00,d0
		rts

pss_isstick:
	; emulation of B button only?

		tst.w	MSTK_MODE(a5)
		beq	pss_setBButton

		lea	_custom,a0

	; skip mouse checking this vblank?

		subq.b	#1,MSTK_SKIPVB(a5)
		bpl.s	pss_skipIRpulse
		move.b	MSTK_SKIPCOUNT(a5),MSTK_SKIPVB(a5)

		move.w	joy1dat(a0),d1
		move.w	d1,d0
		
	; calculate up/down/left/right
		move.w	d0,d1
		and.w	#JOYDATF_Y1!JOYDATF_X1,d1
		ror.w	#1,d1		;rotate bits 9->8 & 1->0
		ror.b	#1,d1		;rotate bit 0 to bit 7
		ror.w	#7,d1		;rotate to bits 0 & 1

		move.w	d1,d3

		move.w	d0,d1
		and.w	#JOYDATF_Y1!JOYDATF_X1,d1	;mask LF and RT bits
		and.w	#JOYDATF_Y0!JOYDATF_X0,d0	;mask UP and DOWN bits
		ror.w	#1,d1				;match positions for XOR
		eor.w	d1,d0
		ror.b	#1,d0				;rotate bit 0 to 7
		ror.w	#5,d0				;rotate to bits 2 and 3
		or.b	d3,d0

		move.w	joy0dat(a0),d2
		move.w	d2,d3
		and.b	#%11111111,d2	;left & right
		lsr.w	#8,d3		;up & down

		moveq	#08,d1

		btst	#1,d0		;left
		beq.s	pss_notleft

		sub.b	d1,d2
pss_notleft:
		btst	#0,d0		;right
		beq.s	pss_notright

		add.b	d1,d2

pss_notright:
		btst	#3,d0		;up
		beq.s	pss_notup

		sub.b	d1,d3

pss_notup:
		btst	#2,d0		;down
		beq.s	pss_notdown

		add.b	d1,d3
pss_notdown:
		lsl.w	#8,d3
		or.b	d2,d3

		move.w	d3,joytest(a0)

	; emulate mouse buttons
pss_skipIRpulse:

		lea	_ciaa,a0

		btst	#CIAB_GAMEPORT1,ciapra(a0)
		beq.s	pss_fireON

		bclr	#CIAB_GAMEPORT0,ciaddra(a0)	;set to input

		BRA_S	pss_setBButton

pss_fireON:
		bset	#CIAB_GAMEPORT0,ciaddra(a0)	;set to output
		bclr	#CIAB_GAMEPORT0,ciapra(a0)	;set to 0

	; emulate B button - do this in a couple of ways.
	; Toggle Right mouse B button on/off - also force
	; pot0dat to wiggle for Lemmings

pss_setBButton:
		lea	_custom,a0

		btst	#(DATRYB_POT-8),potinp(a0)
		beq.s	pss_downBButton

		move.w	#(OUTLYF_POT!DATLYF_POT),MSTK_POTGOBITS(a5)

		bclr	#ISB_BDOWN,MSTK_FLAGS(a5)
		beq	pss_unknown

pss_newpotgo:

		move.l	MSTK_POTGORES(a5),a6
		moveq	#00,d0
		moveq	#00,d1

		JSRLIB	WritePotgo			;enable during my vector
		bra	pss_unknown

pss_downBButton

		move.w	#(OUTLYF_POT),MSTK_POTGOBITS(a5)

		bset	#ISB_BDOWN,MSTK_FLAGS(a5)
		bne	pss_unknown

		bra.s	pss_newpotgo


**;;;;;;;
**
** Patch constants
**
**;;;;;;;

idString:
    VSTRING

nameString:
    dc.b    'kludges',0

cdbootport:
    dc.b    'CDBOOT',0

cddevname:
    dc.b    'cd.device',0

ExpName:
    dc.b    'expansion.library',0

IntuiName:
    dc.b    'intuition.library',0

playerprefsname:
    dc.b    'playerprefs.library',0

potgoname:
	POTGONAME

RamLibName:
    dc.b    'ramlib',0
RamLibNameE:

DosName:
    dc.b    'dos.library',0

GfxName:
    dc.b    'graphics.library',0

InputName:
    dc.b    'input.device',0

NoInternalPath:
    dc.b    'RESIDENT PATH REMOVE',0

DOSPatchId:
    dc.b    'DOSPATCH '
    dc.b    '('
    DATE
    dc.b    ')',13,10,0

DOSPatchName:
    dc.b    'DOSPATCH',0

    CNOP    0,2

SetpatchEndMarker:


    END             ;END assembly

