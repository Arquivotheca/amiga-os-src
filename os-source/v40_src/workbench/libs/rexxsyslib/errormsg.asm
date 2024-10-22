* == errormsg.asm ======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ==========================     ErrorMsg     ==========================
* Returns the message (as a string structure) associated with an error code.
* Registers:      D0 -- error code
* Return:         D0 -- boolean result
*                 A0 -- address of string
ERRF     SET      NSF_STRING!NSF_NOTNUM!NSF_SOURCE
EMTable  dc.w     Err00-EMError
         dc.w     Err01-EMError
         dc.w     Err02-EMError
         dc.w     Err03-EMError
         dc.w     Err04-EMError
         dc.w     Err05-EMError
         dc.w     Err06-EMError
         dc.w     Err07-EMError
         dc.w     Err08-EMError
         dc.w     Err09-EMError
         dc.w     Err10-EMError
         dc.w     Err11-EMError
         dc.w     Err12-EMError
         dc.w     Err13-EMError
         dc.w     Err14-EMError
         dc.w     Err15-EMError
         dc.w     Err16-EMError
         dc.w     Err17-EMError
         dc.w     Err18-EMError
         dc.w     Err19-EMError
         dc.w     Err20-EMError
         dc.w     Err21-EMError
         dc.w     Err22-EMError
         dc.w     Err23-EMError
         dc.w     Err24-EMError
         dc.w     Err25-EMError
         dc.w     Err26-EMError
         dc.w     Err27-EMError
         dc.w     Err28-EMError
         dc.w     Err29-EMError
         dc.w     Err30-EMError
         dc.w     Err31-EMError
         dc.w     Err32-EMError
         dc.w     Err33-EMError
         dc.w     Err34-EMError
         dc.w     Err35-EMError
         dc.w     Err36-EMError
         dc.w     Err37-EMError
         dc.w     Err38-EMError
         dc.w     Err39-EMError
         dc.w     Err40-EMError
         dc.w     Err41-EMError
         dc.w     Err42-EMError
         dc.w     Err43-EMError
         dc.w     Err44-EMError
         dc.w     Err45-EMError
         dc.w     Err46-EMError
         dc.w     Err47-EMError
         dc.w     Err48-EMError
EM_Num   EQU      *-EMTable

ErrorMsg
         move.l   d0,d1                ; error code positive?
         bmi.s    1$                   ; no ...
         moveq    #EM_Num>>1,d0
         cmp.l    d0,d1                ; less than maximum?
         bge.s    1$                   ; no ...

         add.w    d1,d1                ; scale for 2-byte offset
         move.w   EMTable(pc,d1.w),d1  ; get offset
         moveq    #-1,d0               ; TRUE return
         bra.s    2$                   ; yes ...

1$:      moveq    #0,d1                ; default offset
         moveq    #0,d0                ; invalid error code

2$:      lea      EMError-2(pc,d1.w),a0; default error string
         rts

         ; Error messages as pseudo-string structures

EMError
Err00    dc.w     20,26
         dc.b     ERRF,56,'Undiagnosed internal error'
Err01:   dc.w     5,17
         dc.b     ERRF,133,'Program not found',0
Err02:   dc.w     10,16
         dc.b     ERRF,70,'Execution halted'
Err03:   dc.w     20,19
         dc.b     ERRF,160,'Insufficient memory',0
Err04    dc.w     10,0                 ; unused
         dc.b     ERRF,0
Err05:   dc.w     10,15
         dc.b     ERRF,231,'Unmatched quote',0
Err06:   dc.w     10,20
         dc.b     ERRF,3,'Unterminated comment'
Err07    dc.w     10,0                 ; unused
         dc.b     ERRF,0
Err08:   dc.w     10,18
         dc.b     ERRF,46,'Unrecognized token'
Err09:   dc.w     10,34
         dc.b     ERRF,212,'Symbol or string >65535 characters'
Err10:   dc.w     10,22
         dc.b     ERRF,100,'Invalid message packet'
Err11:   dc.w     10,20
         dc.b     ERRF,192,'Command string error'
Err12:   dc.w     10,26
         dc.b     ERRF,36,'Error return from function'
Err13:   dc.w     10,26
         dc.b     ERRF,32,'Host environment not found'
Err14:   dc.w     10,27
         dc.b     ERRF,116,'Requested library not found',0
Err15:   dc.w     10,18
         dc.b     ERRF,243,'Function not found'
Err16:   dc.w     10,29
         dc.b     ERRF,5,'Function did not return value',0
Err17:   dc.w     10,25
         dc.b     ERRF,161,'Wrong number of arguments',0
Err18:   dc.w     10,28
         dc.b     ERRF,211,'Invalid argument to function'
Err19:   dc.w     10,17
         dc.b     ERRF,144,'Invalid PROCEDURE',0
Err20:   dc.w     10,23
         dc.b     ERRF,183,'Unexpected THEN or WHEN',0
Err21:   dc.w     10,28
         dc.b     ERRF,57,'Unexpected ELSE or OTHERWISE'
Err22:   dc.w     10,34
         dc.b     ERRF,174,'Unexpected BREAK, LEAVE or ITERATE'
Err23:   dc.w     10,27
         dc.b     ERRF,147,'Invalid statement in SELECT',0
Err24:   dc.w     10,24
         dc.b     ERRF,182,'Missing or multiple THEN'
Err25:   dc.w     10,17
         dc.b     ERRF,180,'Missing OTHERWISE',0
Err26:   dc.w     10,25
         dc.b     ERRF,39,'Missing or unexpected END',0
Err27:   dc.w     10,15
         dc.b     ERRF,236,'Symbol mismatch',0
Err28:   dc.w     10,17
         dc.b     ERRF,65,'Invalid DO syntax',0
Err29:   dc.w     10,23
         dc.b     ERRF,160,'Incomplete IF or SELECT',0
Err30:   dc.w     10,15
         dc.b     ERRF,141,'Label not found',0
Err31:   dc.w     10,15
         dc.b     ERRF,232,'Symbol expected',0
Err32:   dc.w     10,25
         dc.b     ERRF,160,'Symbol or string expected',0
Err33:   dc.w     10,15
         dc.b     ERRF,236,'Invalid keyword',0
Err34:   dc.w     10,24
         dc.b     ERRF,128,'Required keyword missing'
Err35:   dc.w     10,21
         dc.b     ERRF,110,'Extraneous characters',0
Err36:   dc.w     10,16
         dc.b     ERRF,87,'Keyword conflict'
Err37:   dc.w     10,16
         dc.b     ERRF,67,'Invalid template'
Err38    dc.w     10,0                 ; unused
         dc.b     ERRF,0
Err39:   dc.w     10,22
         dc.b     ERRF,191,'Uninitialized variable'
Err40:   dc.w     10,21
         dc.b     ERRF,238,'Invalid variable name',0
Err41:   dc.w     10,18
         dc.b     ERRF,55,'Invalid expression'
Err42:   dc.w     10,22
         dc.b     ERRF,150,'Unbalanced parentheses'
Err43:   dc.w     10,22
         dc.b     ERRF,11,'Expression nesting >32'
Err44:   dc.w     10,25
         dc.b     ERRF,246,'Invalid expression result',0
Err45:   dc.w     10,19
         dc.b     ERRF,177,'Expression required',0
Err46:   dc.w     10,24
         dc.b     ERRF,16,'Boolean value not 0 or 1'
Err47:   dc.w     10,27
         dc.b     ERRF,186,'Arithmetic conversion error',0
Err48:   dc.w     10,15
         dc.b     ERRF,208,'Invalid operand',0
