*************************************************************************
*                                                                       *
*   ulib.asm -- utility.library romtag
*                                                                       *
*   Copyright (C) 1985, 1989 Commodore Amiga Inc.  All rights reserved. *
*                                                                       *
*   $Id: ulib.asm,v 36.2 90/11/05 18:55:01 peter Exp $
*************************************************************************

   SECTION   section

   NOLIST
   INCLUDE "exec/types.i"
   INCLUDE "exec/libraries.i"
   INCLUDE "exec/lists.i"
   INCLUDE "exec/alerts.i"
   INCLUDE "exec/initializers.i"
   INCLUDE "exec/resident.i"
   INCLUDE "libraries/dos.i"

   INCLUDE "asmsupp.i"
   INCLUDE "ubase.i"
   INCLUDE "utility_rev.i"

   LIST

   XDEF		Open
   XDEF		Close
   XDEF		Expunge
   XDEF		Null

; I want to duck linking with amiga.lib, and I don't
; can't find an lvos.i around.  These are from CAPE's allsyms.i
;   XLIB   FreeMem
;   XLIB   Remove
_LVOFreeMem			EQU	$FFFFFF2E
_LVORemove			EQU	$FFFFFF04

   XREF	uFuncTable
   XREF	_HardwareOverride


Start:
   MOVEQ   #-1,d0
   rts

; ---- library definition ----
utilityName:	DC.B   'utility.library',0
MYPRI   	EQU   103
idString	VSTRING		; macro from automatic utility_rev.i

   ; force word alignment
   ds.w   0

initDDescrip:
               ;STRUCTURE RT,0
     DC.W    RTC_MATCHWORD      ; UWORD RT_MATCHWORD
     DC.L    initDDescrip       ; APTR  RT_MATCHTAG
     DC.L    EndCode            ; APTR  RT_ENDSKIP
     DC.B    RTF_AUTOINIT!RTF_COLDSTART       ; UBYTE RT_FLAGS
     DC.B    VERSION            ; UBYTE RT_VERSION
     DC.B    NT_LIBRARY         ; UBYTE RT_TYPE
     DC.B    MYPRI              ; BYTE  RT_PRI
     DC.L    utilityName         ; APTR  RT_NAME
     DC.L    idString           ; APTR  RT_IDSTRING
     DC.L    Init               ; APTR  RT_INIT

   ; force word alignment
   ds.w   0

Init:
   DC.L   UtilityBase_SIZEOF ; size of library base data space
   DC.L   uFuncTable         ; pointer to function initializers
   DC.L   dataTable         ; pointer to data initializers
   DC.L   initRoutine       ; routine to run


dataTable:
   INITBYTE   LN_TYPE,NT_LIBRARY
   INITLONG   LN_NAME,utilityName
   INITBYTE   LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
   INITWORD   LIB_VERSION,VERSION
   INITWORD   LIB_REVISION,REVISION
   INITLONG   LIB_IDSTRING,idString
   DC.L   0


   ; This routine gets called after the library has been allocated.
   ; The library pointer is in D0.  The segment list is in A0.
   ; If it returns non-zero then the library will be linked into
   ; the library list.
initRoutine:

   move.l   a5,-(sp)
   move.l   d0,a5

   ;------ save a pointer to exec
   move.l   a6,ub_SysLib(a5)

   ;------ save a pointer to our loaded code
   move.l   a0,ub_SegList(a5)

   ; further initialization goes here
   ; ZZZZZZZZZZZZZZZZZZZZ:
   move.l d0,-(sp)
   jsr	_HardwareOverride
   addq.l #4,sp

   move.l   a5,d0
   move.l   (sp)+,a5
   rts


Open:      ; ( libptr:a6, version:d0 )

   addq.w   #1,LIB_OPENCNT(a6)
   bclr   #LIBB_DELEXP,ub_Flags(a6)

   ;--- other open processing here
   ; ZZZ:

   move.l   a6,d0
   rts

Close:      ; ( libptr:a6 )
   CLEAR   d0
   subq.w   #1,LIB_OPENCNT(a6)
   bne.s   1$
   btst   #LIBB_DELEXP,ub_Flags(a6)
   beq.s   1$
   bsr   Expunge
1$:
   rts


Expunge:   ; ( libptr: a6 )
   movem.l   d2/a5/a6,-(sp)
   move.l   a6,a5
   move.l   ub_SysLib(a5),a6
   tst.w   LIB_OPENCNT(a5)
   beq   1$
   ;------ still open.  set the delayed expunge flag
   bset   #LIBB_DELEXP,ub_Flags(a5)
   CLEAR   d0
   bra.s   Expunge_End

1$:
   move.l   ub_SegList(a5),d2
   ;------ unlink from library list
   move.l   a5,a1
   CALLSYS   Remove
   
   ; library specific closings here...
   ; ZZZZ:

   ;------ free our memory
   CLEAR   d0
   move.l   a5,a1
   move.w   LIB_NEGSIZE(a5),d0

   sub.l   d0,a1
   add.w   LIB_POSSIZE(a5),d0

   CALLSYS   FreeMem
   move.l   d2,d0

Expunge_End:
   movem.l   (sp)+,d2/a5/a6
   rts

Null:
   CLEAR   d0
   rts

EndCode:

   END
