
* *** sfuncs3.asm *************************************************************
* 
* Services Functions Implementation Algorithms and Routines
* =Robert J. Mical=
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name           Description
* ---------  -------------  ---------------------------------------------------
* 15-Jul-88  -RJ            Changed all files to work with new includes
* 19-Feb-88  =RJ Mical=     Created this file!
* 
* *****************************************************************************


   INCLUDE "assembly.i"


   NOLIST
   INCLUDE "exec/types.i"

   INCLUDE "janus/janusbase.i"
   INCLUDE "janus/memory.i"
   INCLUDE "janus/services.i"

    INCLUDE "serviceint.i"
   LIST

   INCLUDE "asmsupp.i"


   XLIB   AllocJanusMem
   XLIB   AllocMem
   XLIB   FreeJanusMem
   XLIB   FreeMem
   XLIB   JanusMemBase
   XLIB   JanusMemToOffset
   XLIB   JanusMemType
;NOSEMS   XLIB   ObtainSemaphore
;NOSEMS   XLIB   ReleaseSemaphore
   XLIB   SendJanusInt


   XDEF   MakeBytePtr
   XDEF   MakeWordPtr
   XDEF   JanusOffsetToMem
   XDEF   TranslateJanusPtr

   XLIB   MakeBytePtr
   XLIB   MakeWordPtr
   XLIB   JanusOffsetToMem
   XLIB   TranslateJanusPtr



* =============================================================================
* === IMPLEMENTATION ALGORITHMS ===============================================
* =============================================================================
* 
* This section is for Torsten and RJ.  These notes are *not* to be released 
* to the real world, please.




JanusOffsetToMem:
******* janus.library/JanusOffsetToMem **************************************
*
*   NAME   
*     JanusOffsetToMem -- Create a pointer from a Janus offset and type
*
*   SYNOPSIS
*     MemPtr =  JanusOffsetToMem(Offset, Type)
*     D0                         D0:0-15 D1:0-15
*
*     APTR JanusOffsetToMem(USHORT,USHORT);
* 
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     Turns a Janus memory offset and a type specifier into a normal 
*     memory pointer.  
*
*   INPUTS
*     Offset = 16-bit Janus memory offset
*     Type   = Janus memory type specifier, which includes information such
*              as whether the pointer should point to buffer or parameter
*              memory,whether you want word-access or byte-access address
*              space, and others
*
*   RESULT
*     MemPtr = the desired 68000 memory pointer
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     JanusMemType(), JanusMemToOffset(), GetParamOffset()
*
*****************************************************************************
*
*/


      MOVEM.L   D2,-(SP)
      MOVE.W   D0,D2
      MOVE.L   #$0000FFFF,D0
      AND.L   D0,D1
      AND.L   D0,D2

   IFGE   INFOLEVEL-2
   MOVEM.L   D1,-(SP)
   MOVEM.L   D2,-(SP)
   PUTMSG   2,<'%s/JanusOffsetToMem( $%04lx $%04lx )'>
   LEA   2*4(SP),SP
   ENDC


;==============================================================================
; if this is an AT with the memory set to $D000 then all offsets should be 
; moved forward by $4000 because the PC will be taking all offsets off a segment
; base address of $D400.  SHIT !!!!
;==============================================================================
      BTST.L   #MEMB_BUFFER,D1      ; see if it's buffer mem
      BEQ.S   10$         ; nope, then do nothing special
; this is that funny buffer mem, see if it's at segment offset $D400
      CMPI.W   #$D400,jb_ATOffset(A6)   ; are we in the funny bank ?
      BNE.S   10$         ; no, leave everything alone
      ADDI.L   #$4000,D2      ; yes, back it up by 4000
10$

      MOVE.L   D1,D0
      CALLSYS JanusMemBase
      ADD.L   D2,D0

      MOVEA.L   D0,A0

      MOVEM.L   (SP)+,D2
      RTS



TranslateJanusPtr:
******* janus.library/TranslateJanusPtr *************************************
*
*   NAME   
*     TranslateJanusPtr -- Translate a Janus memory pointer to a new type
*
*   SYNOPSIS
*     MemPtr = TranslateJanusPtr(OldPtr, Type);
*     D0,A0                      A0      D0:0-15
*
*     APTR TranslateJanusPtr(APTR,USHORT);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     Translates the specified Janus memory pointer to point to the 
*     memory-access address space specified by the type argument.  
*     Ignores all portions of the type argument except for the 
*     memory-access specification, which is to say that if pointer 
*     points to parameter memory, for instance, then it will still 
*     point to parameter memory when finished, except that the pointer 
*     will be adjusted for the desired memory access address space.  
*     This is one of the few Janus memory routines which doesn't require
*     a completely well-formed type argument.  
*
*   INPUTS
*     OldPtr = the Janus memory pointer that you want to translate
*     Type   = the specifier for the memory-access address space 
*              to which you want the pointer translated 
*
*   RESULT
*     MemPtr = the desired memory pointer
*
*   EXAMPLE
*     UBYTE  *byteptr;
*     USHORT *wordptr;
*     if (byteptr = (UBYTE *)AllocJanusMem(100, MEMF_BUFFER | MEM_BYTEACCESS))
*     {
*        /* Note that neither MEMF_BUFFER nor MEMF_PARAMETER needs to be
*         * supplied in the type argument in the following call.
*         */
*        wordptr = TranslateJanusPtr(byteptr, MEM_WORDACCESS);
*     }
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     MakeBytePtr(), MakeWordPtr()
*
*****************************************************************************
*
*/


TA_REGS      REG   D2/A2

      MOVEM.L   TA_REGS,-(SP)
      ANDI.L   #$0000FFFF,D0

   IFGE   INFOLEVEL-2
   MOVEM.L   D0,-(SP)
   MOVEM.L   A0,-(SP)
   PUTMSG   2,<'%s/TranslateJanusPtr( $%lx $%04lx )'>
   LEA   2*4(SP),SP
   ENDC

      MOVE.L   A0,A2      ; Grab local copies 
      MOVE.L   D0,D2      ; of the arguments

      MOVE.L   D0,D1
      MOVE.L   A0,D0
      CALLSYS   JanusMemType   ; Get the memory type of the ptr
      ANDI.L   #~MEM_ACCESSMASK,D0
      ANDI.L   #MEM_ACCESSMASK,D2
      OR.L   D0,D2

      MOVE.L   A2,D0      ; Put ptr back in D0
      CALLSYS   JanusMemToOffset ; and get the offset of it

      MOVE.L   D2,D1      ; With offset in D0 and type in D1
      CALLSYS   JanusOffsetToMem ; create the desired pointer

      MOVEM.L   (SP)+,TA_REGS
      RTS



MakeBytePtr:
******* janus.library/MakeBytePtr *******************************************
*
*   NAME   
*     MakeBytePtr -- Make any Janus memory pointer into a byte-access ptr
*
*   SYNOPSIS
*     BytePtr = MakeBytePtr(OldPtr);
*     D0                    A0
*
*     UBYTE *MakeBytePtr(APTR);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     This routine accepts a valid Janus memory pointer of any type 
*     and translates it into a Janus byte-access address space memory 
*     pointer.  If you have a pointer and you don't know its type, 
*     or if you know the type isn't byte-access and what you want is 
*     byte-access, then this is the routine for you.  
*
*   INPUTS
*     OldPtr = the Janus memory pointer that you want to translate
*
*   RESULT
*     BytePtr = the desired byte-access memory pointer
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     MakeWordPtr(), TranslateJanusPtr()
*
*****************************************************************************
*
*/

      MOVE.L   #MEM_BYTEACCESS,D0
      CALLSYS   TranslateJanusPtr
      RTS



MakeWordPtr:
******* janus.library/MakeWordPtr *******************************************
*
*   NAME   
*     MakeWordPtr -- Make any Janus memory pointer into a word-access ptr
*
*   SYNOPSIS
*     WordPtr = MakeWordPtr(OldPtr);
*     D0                    A0
*
*     USHORT *MakeWordPtr(APTR);
*
*     From assembly:  A6 has pointer to JanusBase
*
*   FUNCTION
*     This routine accepts a valid Janus memory pointer of any type 
*     and translates it into a Janus word-access address space memory 
*     pointer.  If you have a pointer and you don't know its type, 
*     or if you know the type isn't word-access and what you want is 
*     word-access, then this is the routine for you.  
*
*   INPUTS
*     OldPtr = the Janus memory pointer that you want to translate
*
*   RESULT
*     WordPtr = the desired word-access memory pointer
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     MakeBytePtr(), TranslateJanusPtr()
*
*****************************************************************************
*
*/

      MOVE.L   #MEM_WORDACCESS,D0
      CALLSYS   TranslateJanusPtr
      RTS



      END


