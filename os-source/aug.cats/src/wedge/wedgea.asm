
*   wedgea.asm  --- assembler interface for Wedge.c
*                 Carolyn Scheppner --- CBM  08/87

   INCLUDE   "exec/types.i"
   INCLUDE   "libraries/dos.i"

	SECTION CODE

ABSEXECBASE	EQU	4

   XREF   _RealFunc
   XREF   _MyFunc
   XREF   _MyFuncRet
   XREF   _Occupants
   XREF   _DebugAgain
   XREF   _RealForbid
   XREF   _RealPermit
   XREF   _RealWait
   XREF   _RealDebug
   XREF    KPutStr
   XREF    KPutChar
   XREF    KGetChar
   XREF    KMayGetChar

   XDEF   _myFunc
   XDEF   _myForbid
   XDEF   _myPermit
   XDEF	  _myWait
   XDEF   _myDebug



_myFunc:
   addq.l  #1,_Occupants       ;Must count entries and exits
   move.l  d0,-(a7)            ;save a space on the stack
   movem.l d0-d7/a0-a7,-(a7)   ;push registers
   jsr     _MyFunc             ;call C wedge
   move.l  d0,64(a7)           ;push returned id into saved spot

   btst.l  #16,d0              ;Need result report ?
   beq     _noRet              ;  if no, go to No Return code

   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's regs (Id still on stack)
   pea.l   _myFuncRet          ;push _myFuncRet so function returns there
   move.l  _RealFunc,-(a7)     ;go to real function
   rts


_noRet:
   btst.l  #17,d0              ;Need to call Debug ?
   beq     _nrSkip1            ;  if no, skip Debug

   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers
   addq.l  #4,a7               ;get rid of Id on stack
   jsr     _doDebug            ;Do debug
   bra     _nrSkip2

_nrSkip1:
   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers
   addq.l  #4,a7               ;get rid of Id on stack
_nrSkip2:
   move.l  _RealFunc,-(a7)     ;continue to real rtn
   subq.l  #1,_Occupants       ;not coming back so decrement Occupants
   rts




_myFuncRet:

   movem.l d0-d7/a0-a7,-(a7)   ;push registers

   jsr      _MyFuncRet         ;have C code output return value

   move.l  64(a7),d0
   btst.l  #17,d0              ;Check id we stashed. Need to call Debug ?
   beq     _rSkip1             ;  if no, skip Debug

   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers
   addq.l  #4,a7               ;get rid of Id on stack
   jsr     _doDebug            ;Do debug
   bra     _rSkip2

_rSkip1:
   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers
   addq.l  #4,a7               ;get rid of Id on stack
_rSkip2:
   subq.l  #1,_Occupants       ;decrement Occupants
   rts


_myForbid:
   move.l  a6,-(a7)            ;push register(s)
   move.l  ABSEXECBASE,a6
   pea.l   _myFRet
   move.l  _RealForbid,-(a7)
   rts
_myFRet:
   move.l  (a7)+,a6            ;restore caller's register(s)
   rts

_myPermit:
   move.l  a6,-(a7)            ;push register(s)
   move.l  ABSEXECBASE,a6
   pea.l   _myPRet
   move.l  _RealPermit,-(a7)
   rts
_myPRet:
   move.l  (a7)+,a6            ;restore caller's register(s)
   rts

_myWait:
   move.l  a6,-(a7)            ;push register(s)
   move.l  ABSEXECBASE,a6
   pea.l   _myWRet
   move.l  _RealWait,-(a7)
   rts
_myWRet:
   move.l  (a7)+,a6            ;restore caller's register(s)
   rts

_myDebug:
   move.l  a6,-(a7)            ;push register(s)
   move.l  ABSEXECBASE,a6
   pea.l   _myDRet
   move.l  _RealDebug,-(a7)
   rts
_myDRet:
   move.l  (a7)+,a6            ;restore register(s)
   rts


_doDebug:
   movem.l d0-d7/a0-a7,-(a7)   ;push caller's registers
   jsr     _myForbid

clbuf0
   jsr     KMayGetChar         ;clear buffer
   cmpi.l  #-1,d0
   bne     clbuf0
   lea.l   _help,a0            ;kprint help
   jsr     KPutStr

   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers

   jsr     _myDebug            ;call Debug

   movem.l d0-d7/a0-a7,-(a7)   ;push caller's registers

askdeb:
clbuf1
   jsr     KMayGetChar         ;clear buffer
   cmpi.l  #-1,d0
   bne     clbuf1

   lea.l   _prompt,a0          ;kprint prompt
   jsr     KPutStr

   jsr     KGetChar            ;get answer
   move.l  d0,_answer
   jsr     KPutChar            ;echo
   move.l  #10,d0
   jsr     KPutChar            ;LF

   move.l  _answer,d0          ;tolower
   ori     #$20,d0
   cmpi    #$77,d0             ;'w' ? (wants to wait the application)
   bne     checky              ;no

   move.l  ABSEXECBASE,a6
   move.l  #SIGBREAKF_CTRL_C,d0
   jsr	   _myWait	       ; wait for CTRL_C
   bra	   askdeb	       ; then ask again

checky
   cmpi    #$79,d0             ;'y' ?
   bne     checkn              ; not y
   ori.w   #-1,_DebugAgain     ; yes, DebugAgain
   bra     clbuf2

checkn
   cmpi	   #$6e,d0	       ; 'n' ?
   bne     askdeb              ; no, ask again

clbuf2
   jsr     KMayGetChar         ; clear buffer
   cmpi.l  #-1,d0
   bne     clbuf2

   jsr     _myPermit
   movem.l (a7)+,d0-d7/a0-a7   ;restore caller's registers
   rts

   SECTION DATA

   CNOP 0,2

_answer  DC.L 0
_prompt  DC.B 10,'Debug next <y, n, or w=wait app for CTRL-C> ?  ',0
_help    DC.B 10,'Note: resume exits romwack',10,0
   END


  
