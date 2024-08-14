
* *** init.asm ****************************************************************
* 
* init.asm -- initialization code for janus library
* 
* Copyright (C) 1986, 1987, 1988, Commodore Amiga Inc.
* All rights reserved
* 
* Date       Name               Description
* ---------  -----------        -----------------------------------------------
* 04-oct-89  -BK                Now set the lib_Version and lib_Revision
* 27-apr-89  -BK                Adjust the init of JanusMemBase for buffer
*                               to start at an offset of $4000 before
*                               calling Init_MemInit
* 27-apr-89  -BK                Adjusted return value from ReBoot8088 to
*                               be a size in bytes rather than the offset
*                               from the begining of the d000 segment.
*                               should have been offset from d400 seg
* 27-apr-89  -BK                Added test for AT to Buffer mem call to
*                               Init_MemInit, AT is now correctly set to
*                               48K instead of 64K for jmh_Max
* 6-Oct-88   -RJ                Initialize ja_JVerNum and ja_JRevNum fields
* 27-Jul-88  -RJ                Set NUMLOCK_RESET flag in AmigaState field
* 26-Jul-88  -RJ                Changed final init value of ja_8088Go 
*                               from $7F to $00
* 21-Jul-88  -RJ                Changed the text 'at.boot' to 'pc.boot'
* 15-Jul-88  -RJ                Changed all files to work with new includes
* 17 May 88  Wolf Schmidt       Made the FreeMem fix described under 
*                               WOLF_FIX_1 below
* 29-Mar-88  Bill Koester       Added code to store the 8088Segment for 
*                               parm mem into the ParamMem MemHead structure 
*                               in janus base
* 28-Mar-88  Bill Koester       Fixed Init_MemInit to use word access when 
*                               initializing the first free mem chunks - 
*                               - memory allocation was broken!
* Feb 88     -RJ Mical          Added call to services initialization
* early 86   Katin/Burns clone  Created this file!
* 
* *****************************************************************************



   INCLUDE "assembly.i"

   NOLIST
   INCLUDE "exec/types.i"
   INCLUDE "exec/nodes.i"
   INCLUDE "exec/lists.i"
   INCLUDE "exec/ports.i"
   INCLUDE "exec/memory.i"
   INCLUDE "exec/libraries.i"
   INCLUDE "libraries/configvars.i"
   INCLUDE "libraries/dos.i"

   INCLUDE "hardware/intbits.i"

   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/janusreg.i"
   INCLUDE "janus/janusvar.i"
   INCLUDE "janus/services.i"
   LIST

   INCLUDE "asmsupp.i"

   XDEF   InitTable
   XDEF   Init

   XLIB   Forbid
   XLIB   Permit
   XLIB   OpenLibrary
   XLIB   CloseLibrary
   XLIB   AllocMem
   XLIB   FreeMem
   XLIB   AddIntServer

   XLIB   Open
   XLIB   Read
   XLIB   Close

   XLIB   GetCurrentBinding

   XLIB   SetJanusHandler
   XLIB   SetJanusEnable
   XLIB   JanusUnlock

   XREF   _AbsExecBase

   XREF   Open
   XREF   Close
   XREF   Expunge
   XREF   Null
   XREF   subsysName
   XREF   JanusIntCode
   XREF   AmigaRW

   XREF   InitService

   XREF   VERNUM
   XREF   REVNUM


InitTable:
      dc.l   JanusBase_SIZEOF+4*MAXHANDLER
      dc.l   funcTable
      dc.l   0
      dc.l   Init


Init:
      ;------- Janus.Library Initialization section
      ;------- Here we set up Amiga and PC shared structures and
      ;------- actually start the PC
      ;------- Called with ( d0: lib node, a0: seg list )

      movem.l  d2-d5/a2-a3/a6,-(sp)

      IFGE     INFOLEVEL-1
       MOVE.L   #REVNUM,-(SP)
       MOVE.L   #VERNUM,-(SP)
       INFOMSG   1,<'Init.asm: janus.library %ld.%ld'>
       LEA.L    2*4(SP),SP
      ENDC

      ;------- do some low level initialization
      move.l   d0,a2                            ; JanusBase -> a2
      move.l   a0,jb_SegList(a2)                ; Save SegList

      move.w   #REVNUM,LIB_REVISION(a2)
      move.w   #VERNUM,LIB_VERSION(a2)

      lea      subsysName(pc),a0                ; "janus.library"
      move.l   a0,LN_NAME(a2)                   ; 
      move.l   a0,jb_IntServer+LN_NAME(a2)      ;
      move.l   a0,jb_ReadHandler+LN_NAME(a2)    ;

      move.l   a6,jb_ExecBase(a2)               ; save ExecBase

      ;------- open the DOS
      lea      dosLibName(pc),a1                ; "dos.library"
      moveq    #0,d0                            ; any version
      CALLSYS  OpenLibrary                      ; Open dos.library
      move.l   d0,jb_DOSBase(a2)                ; Save DOSBase
      beq      errInit1

      ;------- go out and find the board.
      lea      elName(pc),a1                    ; "expansion.library"
      moveq    #0,d0                            ; any version
      CALLSYS  OpenLibrary                      ; Open expansion.library

      tst.l    d0
      beq      errInit2

      move.l   d0,a6                            ; ExpansionBase
      link     a5,#-CurrentBinding_SIZEOF
      move.l   a7,a0
      CALLSYS  GetCurrentBinding

      ;------- only pay attention to the first board
      move.l   cb_ConfigDev(a7),a0
      bclr     #CDB_CONFIGME,cd_Flags(a0)
      move.l   a2,cd_Driver(a0)
      move.l   cd_BoardAddr(a0),d2

      unlk     a5

      ;------- done with the expansion.library
      move.l   a6,a1
      move.l   _AbsExecBase,a6
      CALLSYS  CloseLibrary                     ; Close expansion.library

gotBaseAddress:
      IFGE     INFOLEVEL-1
       move.l   d2,-(a7)
       INFOMSG  1,<'Init.asm: BridgeBoard @ 0x%06lx'>
       addq.l   #4,a7
      ENDC
      bsr      GetPrefs                         ; get configuration byte

      INFOMSG  1,<'Init.asm: release 8088 interrupt'>
      ;------- set up the various base pointers
      move.l   d2,a0
      add.l    #ParameterOffset,a0
      move.l   d2,a1
      add.l    #IoRegOffset+IoAccessOffset,a1
;==============================================================================
; Set up offset to the keyboard data register depending on which machine we
; are running in.  If the PCConfiguration register can be written and read
; then we are running on a PC emulator board in the A2000.  If not, then we
; are running on an Amiga 1000 with a Sidecar.  jb_KeyboardRegisterOffset
; will contain the signed word offset from IOBase when we are done.
;==============================================================================
      move.b   #$ff,jio_Control(a1)                ; ****FRANK'S FIX****
      move.w   d0,-(sp)                  save config value and assume sidecar
      move.w   #jio_1000KeyboardData,jb_KeyboardRegisterOffset(a2)
      move.b   #$7f,jio_PCconfiguration(a1)
      move.b   jio_PCconfiguration(a1),d0
      andi.b   #$7f,d0
      cmpi.b   #$7f,d0
      bne.s    WasA1000                             it's a sidecar
      move.b   #$00,jio_PCconfiguration(a1)
      move.b   jio_PCconfiguration(a1),d0
      andi.b   #$7f,d0
      cmpi.b   #$00,d0
      bne.s    WasA1000                             it's a sidecar
      move.w   #jio_2000KeyboardData,jb_KeyboardRegisterOffset(a2)

      ;------- set PC configuration (D0 set from GetPrefs)
      ;------- won't hurt to do this in a sidecar anyway
WasA1000
      move.w   (sp)+,d0                         get back config byte
      move.b   d0,jio_PCconfiguration(a1)
;==============================================================================
; now we have to set up the offset to the pc boot code so we can tell the PC
; where it is.  This value is set by the pcprefs program and can be any of
; A000, D000 or E000.  If the value is D000 and we are running an AT then this
; should be set to $D400 and the boot code loaded $4000 bytes further into the
; buffer memory.  We read the value back in case this is a Sidecar in which
; case the mem config will have been set by dip switches.
;==============================================================================
      move.b   jio_PCconfiguration(a1),d0
      rol.b    #3,d0                           get mem bits to bits 0 and 1
      andi.w   #3,d0
      asl.w    #1,d0                          make index for getting offset
      move.w   MemOffs(pc,d0.w),jb_ATOffset(a2)
      bra.s    Check4AT                         check for special d000 offset

MemOffs      DC.W   0,$a000,$d000,$e000

Check4AT
      btst.b   #7,jio_PCconfiguration(a1)        is this an AT
      bne      ATChecked                          no, config bit is high
      cmpi.w   #$d000,jb_ATOffset(a2)               yes, check for d000
      bne      ATChecked                          nope, mem is at A000
      move.w   #$d400,jb_ATOffset(a2)              AT can't see d000
ATChecked:

      ;------- clear 8088 interrupts
      move.b   #$ff&~JCNTRLF_CLRPCINT,jio_Control(a1)
      ;------- wait for the 8088 to indicate memory is being refreshed
      ;------- This must be performed within refresh decay time from
      ;------- the write to the lock function being performed in the  
      ;------- loop
      move.w   #2000,d0
10$   move.b   #$ff,ja_Lock(a0)                    ; initiate refresh
      move.b   #$ff,ja_8088Go(a0)                  ; hold 8088 from going
      dbra     d0,10$

      move.b   #$ff,ja_Lock(a0)                   ; write the data
      move.b   #$ff,ja_8088Go(a0)                 ; hold 8088 from going
      move.l   #2000000,d0                        ; timeout loop
      INFOMSG  1,<'Init.asm: config/8088 release reset'>

checkRefresh:
;      tas     ja_Lock(a0)
      cmp.b    #0,ja_Lock(a0)
      beq.s    refreshGoing
      move.b   #$ff,ja_8088Go(a0)                 ; hold 8088 from going
      tst.b    jio_ReleasePcReset(a1)
      subq.l   #1,d0
      ble      errInit2.1                         ; it's taken too long!
      bra.s    checkRefresh

refreshGoing:
      INFOMSG  1,<'Init.asm: OK, AT/XT has woken up at last!'>
      ;------- set up the various base pointers
ReallyGoing
      move.l   d2,jb_ExpanBase(a2)
      beq      errInit2                           ; board not found
      move.l   d2,a3
      add.l    #ParameterOffset,a3
      move.l   a3,jb_ParamMem(a2)

      move.l   d2,a0
      add.l    #IoRegOffset+IoAccessOffset,a0
      move.l   a0,jb_IoBase(a2)

      ;------- a bit of magic -- our library is sized to have
      ;------- the interrupt tables at the end
      lea      JanusBase_SIZEOF(a2),a0
      move.l   a0,jb_IntHandlers(a2)

      ;------- clear out parameter and buffer memory
      CLEAR    d1

MustBeClear
      clr.l    (a3)                         ; check refresh is really going
      tst.l    (a3)                         ; before clearing memory
      bne.s    MustBeClear

      move.l   a3,a0
      move.w   #(ParameterSize>>2)-1,d0
1$
      move.l   d1,(a0)+
      dbra     d0,1$

      move.l   jb_ExpanBase(a2),a0
      move.w   #(BufferSize>>2)-1,d0
2$
      move.l   d1,(a0)+
      dbra     d0,2$

      ;------- now set up the parameter memory
      add.l    #WordAccessOffset,a3
      move.w   #JanusAmiga_SIZEOF,ja_Interrupts(a3)
      move.w   #JanusAmiga_SIZEOF+2*MAXHANDLER,ja_Parameters(a3)
      move.w   #MAXHANDLER,ja_NumInterrupts(a3)

      ;------- set the NUMLOCK flag in the AmigaState field
      ;------- to designate to both sides that the default PC
      ;------- NUMLOCK condition is NUMLOCK set.
      MOVE.W   ja_AmigaState(A3),D0
      BSET     #AMIGA_NUMLOCK_RESETn,D0
      MOVE.W   D0,ja_AmigaState(A3)

      ;------- initialize the parameter area and the softint array
      moveq    #-1,d1
      move.w   #MAXHANDLER-1,d0
      lea      JanusAmiga_SIZEOF(a3),a0
3$
      move.l   d1,(a0)+
      dbra     d0,3$

      ;------- Set the janus.library version and revision nums
      MOVE.W   #VERNUM,ja_JLibVer(A3)
      MOVE.W   #REVNUM,ja_JLibRev(A3)

      ;------- Init the software master lock
      MOVE.B   #0,ja_PCFlag(a3)
      MOVE.B   #0,ja_AmigaFlag(a3)
      MOVE.B   #0,ja_Turn(a3)

      ;------- load in the 8088 boot data file, if it exists
      move.l   a2,a6
      bsr      ReBoot8088

      ;------- set up the memory list heads
      move.l   jb_ExpanBase(a2),a0
      tst.w    jb_ATFlag(a2)           ;Billfix 4-27-89
      beq      15$                      ;If AT then base is
      adda.l   #$4000,a0               ;$4000 bytes from start
15$
      lea      ja_BufferMem(a3),a1
      ;------- D0 contains boot size from above
      move.w   #BufferSize-1,d1
      tst.w    jb_ATFlag(a2)           ;Bill fix 4-27-89
      beq      4$                      ;Max bytes is less for AT
      subi.w   #$4000,d1               ;
4$                                     ;
      bsr      Init_MemInit

; need to set up the segment for the boot file so PC knows where it is
      lea.l    ja_BufferMem(a3),a0
      move.w   jb_ATOffset(a2),jmh_8088Segment(a0)

      ;------- Setup jmh_8088segment to reflect param mem seg

; START OF BILL'S 29-Mar-88 FIX
      movem.l  a0-a4,-(sp)
      move.l   jb_IoBase(a2),a0
      move.l   jb_ExpanBase(a2),a3
      add.l    #ParameterOffset,a3
      add.l    #WordAccessOffset,a3
      lea.l    ja_ParamMem(a3),a4
      btst.b   #7,jio_PCconfiguration(a0)
      bne.s    5$
      move.w   #$d000,jmh_8088Segment(a4) ; It's an AT
      bra.s    6$
5$    move.w   #$f000,jmh_8088Segment(a4) ; It's not an AT
6$    movem.l  (sp)+,a0-a4
; END OF BILL'S 29-Mar-88 FIX

      move.l   a3,a0
      sub.l    #WordAccessOffset,a0
      lea      ja_ParamMem(a3),a1
      move.w   #JanusAmiga_SIZEOF+4*MAXHANDLER,d0
      move.w   #ParameterSize-1,d1
      bsr      Init_MemInit

      ;------- set up the interrupt handlers
      move.b   #$bf,jb_SpurriousMask(a2)   ; mask out LPT1 & COM2
      lea      jb_IntServer(a2),a1
      move.l   a2,IS_DATA(a1)
      move.l   #JanusIntCode,IS_CODE(a1)
      move.b   #10,LN_PRI(a1)                   ; !!! testing only
      moveq    #INTB_PORTS,d0
      move.l   jb_ExecBase(a2),a6
      CALLSYS  AddIntServer

      ;------- initialize amiga read/write 
      lea      jb_ReadHandler(a2),a1
      move.l   a2,IS_DATA(a1)
      move.l   #AmigaRW,IS_CODE(a1)
      moveq    #JSERV_READAMIGA,d0
      exg      a2,a6
      CALLSYS  SetJanusHandler

      moveq    #JSERV_READAMIGA,d0
      moveq    #1,d1
      CALLSYS  SetJanusEnable
      exg      a2,a6


      ;------- and let the pc run!
      move.l   jb_IoBase(a2),a0
      sub.l    #WordAccessOffset,a3
      MOVE.B   #0,ja_8088Go(a3)                ; RJ's fix, 26 July 88
      move.l   a0,-(sp)
      lea.l    ja_Lock(a3),a0
      exg      a2,a6
      CALLSYS  JanusUnlock
      exg      a2,a6
*      UNLOCK   ja_Lock(a3)
      move.l   (sp)+,a0

      ;------- enable interrupts from the pc
      move.b   #$ff&~JCNTRLF_ENABLEINT,jio_Control(a0)


;==============================================================================
; Here we insert the new Services initialization stuff, which will set up 
; necessary the data structures and mint the services task.
; RJM:  7 Feb 1988
;==============================================================================
      JSR      InitService
;==============================================================================
; End of new code added by RJM.
;==============================================================================


      ;------- set up return code
      move.l   a2,d0
endInit: 

      INFOMSG   1,<'Init.asm: Init: ending'>

      movem.l (sp)+,d2-d5/a2-a3/a6
      rts

errInit3:
   PUTMSG   1,<'%s/errInit3: called'>
      move.l   d4,d1
      CALLSYS Close

errInit2.1:
   PUTMSG   1,<'%s/errInit2.1: called'>

errInit2:
   PUTMSG   1,<'%s/errInit2: called'>
      move.l   jb_DOSBase(a2),a1
      CALLSYS CloseLibrary

errInit1:

   PUTMSG   1,<'errInit1: called'>

      ;------- free up library node
      move.l   a2,a1
      CLEAR    d0
      move.w   LIB_NEGSIZE(a2),d0
      sub.w    d0,a1
      add.w    LIB_POSSIZE(a2),d0

; WOLF_FIX_1
; This fix was put in this file by Wolf sometime in the distant past.  
; It seems that the following call to LINKEXE FreeMem was an erroneous 
; call to make.  I, DoctorRJ, have checked and it's easily possible 
; for no FreeMem args to be in place at all, so I agree with Wolf.  
; 
;      LINKEXE FreeMem

      moveq    #0,d0
      bra      endInit 


; initialize a JanusMemHead structure
Init_MemInit   ; (a0: byte 68k base, a1: word memhead, d0: first, d1: max)

      move.l   d1,-(sp)
      move.l   d0,-(sp)
      move.l   a1,-(sp)
      move.l   a0,-(sp)
      INFOMSG   1,<'Init.asm: Init_MemInit(base 0x%lx memhead 0x%lx first 0x%lx max 0x%lx)'>
      add.l    #(4*4),sp

      move.l   d2,-(sp)
      move.l   a0,jmh_68000Base(a1)   ; set base address

      addq.w   #3,d0
      and.w    #$fffc,d0
      move.w   d0,jmh_First(a1)
      move.w   d1,jmh_Max(a1)

      move.w   d1,d2
      sub.w    d0,d2
      move.w   d2,jmh_Free(a1)

      ;------- now set up the first mem chunk
      CLEAR    d1
      move.w   d0,d1

      move.l   d1,-(sp)
      move.l   a0,-(sp)
      INFOMSG  1,<'Init.asm: Address = 0x%lx Offset = 0x%lx'>
      addq.l   #8,sp

; START OF BILL'S 28-Mar-88 FIX
      move.l   a0,-(sp)                   ; bill's fix for word
      adda.l   #WordAccessOffset,a0       ; access!
      move.w   #-1,jmc_Next(a0,d1.l)      ; -1 means no next chunk
      move.w   d2,jmc_Size(a0,d1.l)       ; size is # free bytes -1
      move.l   (sp)+,a0

; Bill's goodly fix above replaces these two very naughty lines
;      move.w   #-1,jmc_Next(a0,d1.l)   ; -1 means no next chunk
;      move.w   d2,jmc_Size(a0,d1.l)   ; size is # of free bytes -1
; END OF BILL'S 28-Mar-88 FIX

      move.l   (sp)+,d2
      rts


FUNCDEF    MACRO
      XREF   \1
      dc.l   \1
      ENDM

      LIBINIT

funcTable:
      dc.l   Open
      dc.l   Close
      dc.l   Expunge
      dc.l   Null

      INCLUDE "jfuncs.i"
      INCLUDE "jfuncs2.i"

      dc.l   -1

;------ ReBoot8088 ----------------------------------------------------------
;------ load in the 8088 boot data file, if it exists.  Returns bytes loaded
;----------------------------------------------------------------------------
ReBoot8088:
      INFOMSG  1,<'Init.asm: Trying to boot pc.boot'>
      movem.l  d2-d5/a2/a6,-(a7)
      clr.l    d5                               ; count of bytes loaded
      move.l   a6,a2                            ; janus library base
      lea      BootName(pc),a0                  ; get filename
      move.l   a0,d1
      clr.w    jb_ATFlag(a6)                    ; assume a PC

; fix for PC/AT.  Load a different boot file if it's an AT

      movea.l  jb_IoBase(a2),a0                 ; XT or AT board
      btst.b   #7,jio_PCconfiguration(a0)
      bne.s    LoadXTBoot                       ; bootfile OK, it's an XT
      move.w   #1,jb_ATFlag(a6)                 ; it's an AT
      lea.l    BootName(pc),a0
      move.l   a0,d1                            ; do the boot for an AT
      cmpi.w   #$d400,jb_ATOffset(a6)           ; maybe we need offset
      bne.s    LoadXTBoot                       ; nope, mem is at a000
      move.l   #$4000,d5                        ; AT can't see lower 16K

LoadXTBoot
      move.l   #MODE_OLDFILE,d2
      move.l   jb_DOSBase(a2),a6                ; pointer to DOS library
      CALLSYS  Open
      move.l   d0,d4                            ; save open handle
      beq.s    readNothing                      ; no file to read
      INFOMSG  1,<'Init.asm: File opened OK'>

      move.l   d4,d1                            ; get file handle    
      move.l   jb_ExpanBase(a2),d2              ; and buffer to read into

; fix for PC/AT.  Load at an extra offset of $4000 for the AT

      tst.l    d5                               ; if d5 <> $4000
      beq.s    ItsAnXT                          ; offset OK, it's an XT
      addi.l   #$4000,d2                        ; else increment buffer ptr

ItsAnXT
      move.l   #100000,d3                       ; allow for maximum length
      IFGE     INFOLEVEL-10
       move.l   d2,-(sp)
       INFOMSG  1,<'Init.asm: pc.boot loaded at %lx'>
       addq.l   #4,sp
      ENDC
      CALLSYS  Read
      tst.l    d0                               ; did we get the file in ?
      bmi.s    readError                        ; nope, some kind of error
      add.l    d0,d5                            ; stash number of bytes
readError:
      move.l   d4,d1                            ; OK, close the file
      CALLSYS  Close
readNothing:
      tst.w    jb_ATFlag(a2)                    ; 4-27-89 bill fix
      beq      1$                               ; if AT Adjust d5 to be
      subi.l   #$4000,d5                        ; size not offset since
1$                                              ; seg is at d400
      move.l   d5,d0                            ; return # bytes loaded
      IFGE     INFOLEVEL-1
       move.l   d5,-(sp)
       INFOMSG  1,<'Init.asm: Number of bytes loaded: %ld'>
       addq.l   #4,sp
       move.l   jb_ExpanBase(a2),-(sp)
       INFOMSG  1,<'Init.asm: Bytes loaded to address: $%lx'>
       addq.l   #4,sp
      ENDC
      movem.l  (a7)+,d2-d5/a2/a6
      rts


;===========================================================================
; Fetch PC configuration byte from the preferences file in sys:sidecar.
;===========================================================================

GetPrefs
      movem.l  d1-d4/a0-a1/a6,-(sp)             ; can't clobber anything
      INFOMSG  1,<'Init.asm: Getting preferences'>
      move.w   #$fe00,-(sp)                     ; put default value for XT
                                                ; on the stack

      movea.l  jb_IoBase(a2),a0                 ; XT or AT board
      btst.b   #7,jio_PCconfiguration(a0)
      bne.s    XTPrefs                          ; prefs OK, it's an XT
      move.w   #$de00,(sp)                      ; AT, use AT default

XTPrefs
      movea.l  jb_DOSBase(a2),a6                ; get DOS library
      lea.l    PrefsName(pc),a0
      move.l   a0,d1
      move.l   #MODE_OLDFILE,d2
      CALLSYS  Open
      move.l   d0,d4                            ; did we get the file ?
      beq.s    UseDefaults                      ; no, use default values
      move.l   sp,d2                            ; read to stack
      moveq.l  #1,d3                            ; only 1 byte to read
      move.l   d4,d1
      CALLSYS  Read
      move.l   d4,d1                            ; close the file
      CALLSYS  Close
      INFOMSG  1,<'Init.asm: Opened preferences file OK'>
UseDefaults
      clr.l    d0
      move.b   (sp),d0                          ; return correct value
      MOVE.L   d0,-(SP)
      INFOMSG  1,<'Init.asm: value stored = %ld'>
      ADDQ.L   #4,sp
      addq.l   #2,sp                            ; scrap one on stack
      movem.l  (sp)+,d1-d4/a0-a1/a6
      rts

BootName:      dc.b  'sys:pc/system/pc.boot',0
      CNOP     0,2
elName         dc.b   'expansion.library',0
      CNOP     0,2
PrefsName      dc.b   'sys:pc/system/2500prefs',0
      CNOP     0,2
dosLibName:    DOSNAME
      CNOP     0,2
      END


