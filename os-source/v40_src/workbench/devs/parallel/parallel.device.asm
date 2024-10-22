;Bugs: If RTF_AUTOINIT fails, library base still left in memory.
;	AllocMem without error check.
;	8-9-89 fix IFNE AUTOMOUNT


*************************************************************************
*
*   Copyright (C) 1986,1988 Commodore Amiga Inc.  All rights reserved.
*   Permission granted for non-commercial use.
*
*************************************************************************

   SECTION firstsection

   NOLIST
   include "exec/types.i"
   include "exec/devices.i"
   include "exec/initializers.i"
   include "exec/memory.i"
   include "exec/resident.i"
   include "exec/io.i"
   include "exec/ables.i"
   include "exec/errors.i"
   include "exec/tasks.i"
   include "hardware/intbits.i"

   include "asmsupp.i"  ;standard asmsupp.i, same as used for library
   include "pardev.i"
   include "parallel_rev.i"

   IFNE AUTOMOUNT
   include "libraries/expansion.i"
   include "libraries/configvars.i"
   include "libraries/configregs.i"
   ENDC
   LIST


ABSEXECBASE equ 4   ;Absolute location of the pointer to exec.library base


   ;------ These don't have to be external, but it helps some
   ;------ debuggers to have them globally visible

   XDEF   Init
   XDEF   Open
   XDEF   Close
   XDEF   Expunge
   XDEF   Null
   XDEF   myName
   XDEF   BeginIO
   XDEF   AbortIO

   ;Pull these _LVOs in from amiga.lib

   XLIB   AddIntServer
   XLIB   RemIntServer
   XLIB   Debug
   XLIB   InitStruct
   XLIB   OpenLibrary
   XLIB   CloseLibrary
   XLIB   Alert
   XLIB   FreeMem
   XLIB   Remove
   XLIB   AddPort
   XLIB   AllocMem
   XLIB   AddTask
   XLIB   PutMsg
   XLIB   RemTask
   XLIB   ReplyMsg
   XLIB   Signal
   XLIB   GetMsg
   XLIB   Wait
   XLIB   WaitPort
   XLIB   AllocSignal

	XREF	_EndCode
  	XREF	_MD_Open
	XREF	_MD_Close
	XREF	_MD_AbortIO
	XREF	_MD_BeginIO

   INT_ABLES	    ;Macro from exec/ables.i

;-----------------------------------------------------------------------
; The first executable location.  This should return an error
; in case someone tried to run you as a program (instead of
; loading you as a device).

FirstAddress:
		moveq	#-1,d0
		rts

;-----------------------------------------------------------------------
; A romtag structure.  After your driver is brought in from disk, the
; disk image will be scanned for this structure to discover magic constants
; about you (such as where to start running you from...).
;-----------------------------------------------------------------------

   ; Most people will not need a priority and should leave it at zero.
   ; the RT_PRI field is used for configuring the roms.  Use "mods" from
   ; wack to look at the other romtags in the system

MYPRI	EQU   -120

initDDescrip:
				; STRUCTURE RT,0
     DC.W    RTC_MATCHWORD	;  UWORD RT_MATCHWORD (Magic cookie)
     DC.L    initDDescrip	;  APTR	RT_MATCHTAG  (Back pointer)
     DC.L    _EndCode		;  APTR	RT_ENDSKIP   (To end of this hunk)
     DC.B    RTF_AUTOINIT	;  UBYTE RT_FLAGS    (magic-see "Init:")
     DC.B    VERSION		;  UBYTE RT_VERSION
     DC.B    NT_DEVICE		;  UBYTE RT_TYPE      (must be correct)
     DC.B    MYPRI		;  BYTE	RT_PRI
     DC.L    myName		;  APTR	RT_NAME      (exec name)
     DC.L    idString		;  APTR	RT_IDSTRING  (text string)
     DC.L    Init		;  APTR	RT_INIT
				; LABEL RT_SIZE

   ;This name for debugging use

   IFNE INFO_LEVEL  ;If any debugging enabled at all
subSysName:
    dc.b    "parallel",0
   ENDC

   ; this is the name that the device will have

myName:		MYDEVNAME
idString:	VSTRING

   ; force long-word alignment

	CNOP	0,4

Init:
   DC.L   MyDev_Sizeof	    ; data space size
   DC.L   funcTable	    ; pointer to function initializers
   DC.L   dataTable	    ; pointer to data initializers
   DC.L   initRoutine	    ; routine to run


funcTable:
   ;------ standard system routines
   dc.l   Open
   dc.l   Close
   dc.l   Expunge
   dc.l   Null	    ;Reserved for future use!

   ;------ my device definitions
   dc.l   BeginIO
   dc.l   AbortIO

   dc.l   -1

dataTable:
   INITBYTE   LN_TYPE,NT_DEVICE		; Must be LN_TYPE!
   INITLONG   LN_NAME,myName
   INITBYTE   LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
   INITWORD   LIB_VERSION,VERSION
   INITWORD   LIB_REVISION,REVISION
   INITLONG   LIB_IDSTRING,idString
   DC.W   0   				; Terminate list

;=========================================================================
;
;	d0 -- device pointer
;	a0 -- AmigaDOS segment list
;
;=========================================================================

initRoutine:

   PUTMSG   5,<'%s/Init: called'>	
   movem.l  a5,-(sp)   			; Save this register
   move.l   d0,a5
   move.l   a6,md_SysLib(a5)    	; faster access than move.l 4,a6
   move.l   a0,md_SegList(a5)		; Save pointer to loaded code (Seglist)
   move.l   a5,d0
   movem.l  (sp)+,a5			; Restore this register

   tst.l    d0				; Does d0 still hold the device pointer?
   beq	    initNull			; Branch if not
   PUTMSG   25,<'%s/Not null'>		; Tell us D0 is not null
   BRA	    initEnd

initNull:
   PUTMSG   25,<'%s/Is Null'>		; Tell us that it's null

initEnd:
   rts

;==========================================================================

Open:	   ; ( device:a6, iob:a1, unitnum:d0, flags:d1 )

   addq.w   #1,LIB_OPENCNT(a6)  	; Fake an opener
   PUTMSG   20,<'%s/Open: called'>

   movem.l  a1/a6,-(sp)			; Save these registers
   bsr      _MD_Open			; Call the C routine
   movem.l  (sp)+,a1/a6			; Restore these registers

   tst.l    d0				; Do we have an error here?
   bne.s    Open_Error			; Branch if so

   move.b   #NT_REPLYMSG,LN_TYPE(a1) 	; Mark IORequest as "complete"
   bclr     #LIBB_DELEXP,md_Flags(a6)	; Prevent delayed expunges
   subq.w   #1,LIB_OPENCNT(a6) 		; ** End of expunge protection <|>

   PUTMSG   25,<'%s/Open: succeded'>
   rts

Open_Error:
   move.b   d0,IO_ERROR(a1)		; Error is IOERR_OPENFAIL
   move.l   d0,IO_DEVICE(a1)    	; Trash IO_DEVICE on open failure
   subq.w   #1,LIB_OPENCNT(a6) 		; ** End of expunge protection <|>
   PUTMSG   25,<'%s/Open: failed'>
   rts


;--------------------------------------------------------------------------
; There are two different things that might be returned from the Close
; routine.  If the device wishes to be unloaded, then Close must return
; the segment list (as given to Init).  Otherwise close MUST return NULL.
;--------------------------------------------------------------------------

Close:	    ; ( device:a6, iob:a1 )

   PUTMSG   20,<'%s/Close: called'>

   ;------ IMPORTANT: make sure the IORequest is not used again
   ;------ with a -1 in IO_DEVICE, any BeginIO() attempt will
   ;------ immediatly halt (which is better than a subtle corruption
   ;------ that will lead to hard-to-trace crashes!!!!!!!!!!!!!!!!!!

   movem.l  a1/a6,-(sp)		; Save this register
   bsr      _MD_Close
   movem.l  (sp)+,a1/a6		; Restore this register

   move.l   d0,IO_UNIT(a1)      ;We're closed...
   move.l   d0,IO_DEVICE(a1)    ;customers not welcome at this IORequest

   CLEAR   d0
   tst.l   LIB_OPENCNT(a6)		; Is anyone left with us open?
   bne.s   Close_End			; Branch if so

   btst    #LIBB_DELEXP,md_Flags(a6)	; Do we have a delayed expunge pending?
   beq.s   Close_End

   bsr	   Expunge

Close_End:
   rts				; MUST return either zero or the SegList!!!



;------- Expunge -----------------------------------------------------------
;
; Expunge is called by the memory allocator when the system is low on
; memory.
;
; There are two different things that might be returned from the Expunge
; routine.  If the device is no longer open then Expunge may return the
; segment list (as given to Init).  Otherwise Expunge may set the
; delayed expunge flag and return NULL.
;
; One other important note: because Expunge is called from the memory
; allocator, it may NEVER Wait() or otherwise take long time to complete.
;
;	A6	    - library base (scratch)
;	D0-D1/A0-A1 - scratch

Expunge:   ; ( device: a6 )

   PUTMSG   10,<'%s/Expunge: called'>

   movem.l  d1/d2/a5/a6,-(sp)		; Save ALL modified registers
   move.l   a6,a5
   move.l   md_SysLib(a5),a6

   tst.w   LIB_OPENCNT(a5)		; Does anyone have us open?
   beq     1$				; Branch if not

   bset    #LIBB_DELEXP,md_Flags(a5)	; Set the delayed expunge flag
   CLEAR   d0
   bra.s   Expunge_End

1$:
   move.l   md_SegList(a5),d2		; Store seglist in this register
   move.l    a5,a1
   CALLSYS   Remove			; Remove first (before FreeMem)
					; (unlink from device list)
   ;
   ; device specific closings here...
   ;

   ;------ free our memory (must calculate from LIB_POSSIZE & LIB_NEGSIZE)
   move.l   a5,a1		;Devicebase
   CLEAR    d0
   move.w   LIB_NEGSIZE(a5),d0
   suba.l   d0,a1		;Calculate base of functions
   add.w    LIB_POSSIZE(a5),d0  ;Calculate size of functions + data area
   CALLSYS  FreeMem

   ;------ set up our return value
   move.l   d2,d0

Expunge_End:
   movem.l  (sp)+,d1/d2/a5/a6
   rts


;------- Null ---------------------------------------------------------------

Null:
   PUTMSG  1,<'%s/Null: called'>
   CLEAR   d0
   rts	    ;The "Null" function MUST return NULL.




; IMPORTANT:
;   The exec WaitIO() function uses the IORequest node type (LN_TYPE)
;   as a flag.	If set to NT_MESSAGE, it assumes the request is
;   still pending and will wait.  If set to NT_REPLYMSG, it assumes the
;   request is finished.  It's the responsibility of the device driver
;   to set the node type to NT_MESSAGE before returning to the user.


BeginIO:   ; ( iob: a1, device:a6 )

    IFGE INFO_LEVEL-1
	bchg.b	#1,$bfe001  ;Blink the power LED
    ENDC

    IFGE INFO_LEVEL-3
     clr.l    -(sp)
     move.w   IO_COMMAND(a1),2(sp)  ;Get entire word
     PUTMSG   3,<'%s/BeginIO  --  %ld'>
     addq.l   #4,sp
    ENDC

    move.b  #NT_MESSAGE,LN_TYPE(a1) ;So WaitIO() is guaranteed to work

	movem.l d1-d7/a1-a6,-(sp)	; Save these registers
	bsr	_MD_BeginIO
	movem.l (sp)+,d1-d7/a1-a6	; Restore these registers

	moveq.b	#0,d0			; Put error message in d0

	PUTMSG	5,<'%s/BeginIO: completed'>
    rts


AbortIO:	; ( iob: a1, device:a6 )

	movem.l	a1/a6,-(sp)
	bsr	_MD_AbortIO
	movem.l	(sp)+,a1/a6
	move.b	IO_ERROR(a1),d0
	rts

	END

;----------------------------------------------------------------------
; _EndCode is a marker that shows the end of your code.	Make sure it does not
; span hunks, and is not before the rom tag!  It is ok to put it right after
; the rom tag -- that way you are always safe.	I put it here because it
; happens to be the "right" thing to do, and I know that it is safe in this
; case (this program has only a single code hunk).
;----------------------------------------------------------------------

