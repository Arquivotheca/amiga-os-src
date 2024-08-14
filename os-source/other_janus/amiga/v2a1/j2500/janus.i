
 IFND   JANUS_JANUS_I      
JANUS_JANUS_I   EQU   1

* *** janus.i ****************************************************************
* 
* janus.i  --  Primary include file for janus.library
* 
* Copyright (c) 1986, 1987, 1988, Commodore Amiga Inc.
* All rights reserved
* 
* Date       Name               Description
* ---------  ---------------    ----------------------------------------------
* 12-Feb-88  -RJ Mical-         Added JanusRemember structure
* Early `86  Katin/Burns clone  Created this file!
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


* As a coding convenience, we assume a maximum of 32 handlers.
* People should avoid using this in their code, because we want
* to be able to relax this constraint in the future.  All the
* standard commands' syntactically support any number of interrupts,
* but the internals are limited to 32.


MAXHANDLER   EQU   32      


* === ===================================================================== 
* === JanusAmiga Structure ================================================ 
* === ===================================================================== 
* JanusAmiga -- the main janus.library data structure.  
* This is the structure that you must declare a pointer to:
*   struct JanusAmiga *JanusBase = NULL;
* and initialize by opening janus.library:
*   JanusBase = OpenLibrary(JANUSNAME, myJANUSVERSION);
*  before using any of the Janus routines.  

; STRUCTURE JanusAmiga,0
;   STRUCT   ja_LibNode,Library_SIZEOF   
 STRUCTURE JanusAmiga,LIB_SIZE

   ULONG    ja_IntReq   ; software copy of outstanding requests 
   ULONG    ja_IntEna   ; software copy of enabled interrupts 
   APTR   ja_ParamMem   ; ptr to (byte arranged) param mem 
   APTR   ja_IoBase   ; ptr to base of io register region 
   APTR   ja_ExpanBase   ; ptr to start of shared memory 
   APTR    ja_ExecBase   ; ptr to exec library 
   APTR    ja_DOSBase   ; ptr to DOS library 
   APTR    ja_SegList   ; ptr to loaded code 
   APTR   ja_IntHandlers   ; base of array of int handler ptrs 
   STRUCT   ja_IntServer,IS_SIZE   ; INTB_PORTS server 
   STRUCT   ja_ReadHandler,IS_SIZE   ; JSERV_READAMIGA handler 

   UWORD    ja_KeyboardRegisterOffset   ; exactly that 
   UWORD    ja_ATFlag   ; 1 if this is an AT 
   UWORD    ja_ATOffset   ; offset to the AT ROM bank 

   APTR   ja_ServiceBase   ; Amiga Services data structure 

    LABEL   JanusAmiga_SIZEOF



* === ===================================================================== 
* === JanusRemember Structure ============================================= 
* === ===================================================================== 
* This is the structure used for the JRemember memory management routines 
* Assembly STRUCTURE PREFIX jr_

 STRUCTURE JanusRemember,0

   RPTR    jr_NextRemember   ; Pointer to the next JanusRemember struct 
   RPTR    jr_Offset   ; Janus offset to this memory allocation 
   USHORT    jr_Size      ; Size of this memory allocation 
   USHORT    jr_Type      ; Type of this memory allocation 

    LABEL   JanusRemember_SIZEOF



* === ===================================================================== 
* === Miscellaneous ======================================================= 
* === ===================================================================== 
* hide a byte field in the lib_pad field 
ja_SpurriousMask   EQU   LIB_pad      

* magic constants for memory allocation 
MEM_TYPEMASK   EQU   $00ff      ; 8 memory areas 
MEMB_PARAMETER   EQU   (0)      ; parameter memory 
MEMB_BUFFER   EQU   (1)      ; buffer memory 

MEMF_PARAMETER   EQU   (1<<0)      ; parameter memory 
MEMF_BUFFER   EQU   (1<<1)      ; buffer memory 

MEM_ACCESSMASK   EQU   $3000      ; bits that participate in access types 
MEM_BYTEACCESS   EQU   $0000      ; pointer to byte access address space 
MEM_WORDACCESS   EQU   $1000      ; pointer to word access address space 
MEM_GRAPHICACCESS EQU   $2000      ; ptr to graphic access address space 
MEM_IOACCESS   EQU   $3000      ; pointer to i/o access address space 

TYPEACCESSTOADDR   EQU   5      ; # of bits to turn access mask to addr 



* === ===================================================================== 
* === Miscellaneous ======================================================= 
* === ===================================================================== 
* This stuff is not declared in janus.h

; JanusResource -- an entity which keeps track of the reset state of the 8088
;   if this resource does not exist, it is assumed the 8088 can be reset
 STRUCTURE   JanusResource,LN_SIZE
   APTR   jr_BoardAddress      ; Address of Janus board
   UBYTE   jr_Reset      ; non-zero indicates 8088 is held reset
   UBYTE   jr_Pad0
    LABEL   JanusResource_SIZEOF


; macro to lock access to janus data structures from PC side
LOCK      MACRO   ; ( \1 -- effective address of lock byte )
begin\@:
      tas   \1
      bpl.s   exit\@
      nop
      nop
      bra.s   begin\@
exit\@:
      endm

; macro to try once to lock access to janus data structures from PC side
; changes D0 to 1 if lock gotten, 0 if lock not gotten
LOCKTRY      MACRO   ; ( \1 -- effective address of lock byte )
begin\@:
      MOVEQ.L   #1,D0
      TAS   \1
      BPL.S   exit\@
      MOVEQ.L   #0,D0
exit\@:
      ENDM

UNLOCK      MACRO   ; ( \1 -- effective address of lock byte )
      move.b   #$7F,\1
      ENDM

JANUSNAME   MACRO
      dc.b   'janus.library',0
      ENDM



;???CHANNELING   EQU   1



 ENDC   ; of IFND JANUS_JANUS_I


