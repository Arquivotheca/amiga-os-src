
 IFND   JANUS_JANUSBASE_I      
JANUS_JANUSBASE_I   SET 1

* *** JanusBase.i *********************************************************
* (Amiga side file)
* 
* JanusBase.i  --  Primary include file for janus.library
* 
* This file contains all AMIGA SPECIFIC definitions and does not contain
* any definitions required on the PC
*
* Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc.
* All rights reserved
* 
* Date       Name                Description
* --------   ---------------     ------------------------------------------
* Early 86 - Katin/Burns clone - Created this file!
* 02-12-88 - RJ Mical          - Added JanusRemember structure
* 07-15-88 - Bill Koester      - Modified for self inclusion of required files
* 07-25-88 - Bill Koester      - Added jb_Reserved to JanusBase
* 
* *************************************************************************


 IFND   EXEC_TYPES_I      
   INCLUDE   "exec/types.i"   
 ENDC   EXEC_TYPES_I

 IFND   EXEC_LIBRARIES_I      
   INCLUDE   "exec/libraries.i"   
 ENDC   EXEC_LIBRARIES_I

 IFND   EXEC_INTERRUPTS_I      
   INCLUDE   "exec/interrupts.i"   
 ENDC   EXEC_INTERRUPTS_I




* === ===================================================================== 
* === JanusBase Structure =================================================
* === ===================================================================== 
* JanusBase -- the main janus.library data structure.
* This is the structure that you must declare a pointer to:
*
*   struct JanusBase *JanusBase = NULL;
*
* and initialize by opening janus.library:
*
*   JanusBase = OpenLibrary(JANUSNAME, myJANUSVERSION);
*
*  before using any of the Janus routines.  

 STRUCTURE  JanusBase,LIB_SIZE

   ULONG    jb_IntReq                  ; software copy of outstanding requests
   ULONG    jb_IntEna                  ; software copy of enabled interrupts
   APTR     jb_ParamMem                ; ptr to (byte arranged) param mem
   APTR     jb_IoBase                  ; ptr to base of io register region
   APTR     jb_ExpanBase               ; ptr to start of shared memory
   APTR     jb_ExecBase                ; ptr to exec library
   APTR     jb_DOSBase                 ; ptr to DOS library
   APTR     jb_SegList                 ; ptr to loaded code
   APTR     jb_IntHandlers             ; base of array of int handler ptrs
   STRUCT   jb_IntServer,IS_SIZE       ; INTB_PORTS server
   STRUCT   jb_ReadHandler,IS_SIZE     ; JSERV_READAMIGA handler

   UWORD    jb_KeyboardRegisterOffset  ; exactly that
   UWORD    jb_ATFlag                  ; 1 if this is an AT
   UWORD    jb_ATOffset                ; offset to the AT ROM bank

   APTR     jb_ServiceBase             ; Amiga Services data structure

   STRUCT   jb_Reserved,4*4

   LABEL    JanusBase_SIZEOF




* === ===================================================================== 
* === Miscellaneous ======================================================= 
* === ===================================================================== 
* hide a byte field in the lib_pad field 

jb_SpurriousMask   EQU   LIB_pad


* === ===================================================================== 
* === Miscellaneous ======================================================= 
* === ===================================================================== 

;*************************************************************************
;
; data structure for SetupJanusSig() routine
;
;*************************************************************************

 STRUCTURE  SetupSig,IS_SIZE

   APTR     ss_TaskPtr
   ULONG    ss_SigMask
   APTR     ss_ParamPtr
   ULONG    ss_ParamSize
   UWORD    ss_JanusIntNum

   LABEL    SetupSig_SIZEOF


; JanusResource -- an entity which keeps track of the reset state of the 8088
; if this resource does not exist, it is assumed the 8088 can be reset

 STRUCTURE  JanusResource,LN_SIZE

   APTR     jr_BoardAddress         ; Address of Janus board
   UBYTE    jr_Reset                ; non-zero indicates 8088 is held reset
   UBYTE    jr_Pad0

   LABEL    JanusResource_SIZEOF


; macro to lock access to janus data structures from PC side

LOCK     MACRO    ; ( \1 -- effective address of lock byte )
begin\@:
      tas      \1
      bpl.s    exit\@
      nop
      nop
      bra.s    begin\@
exit\@:
      endm

; macro to try once to lock access to janus data structures from PC side
; changes D0 to 1 if lock gotten, 0 if lock not gotten

LOCKTRY     MACRO    ; ( \1 -- effective address of lock byte )
begin\@:
      MOVEQ.L  #1,D0
      TAS      \1
      BPL.S    exit\@
      MOVEQ.L  #0,D0
exit\@:
      ENDM

UNLOCK      MACRO    ; ( \1 -- effective address of lock byte )
      move.b   #$7F,\1
      ENDM

JANUSNAME   MACRO
      dc.b  'janus.library',0
      ENDM



 ENDC    ; of IFND JANUS_JANUSBASE_I


