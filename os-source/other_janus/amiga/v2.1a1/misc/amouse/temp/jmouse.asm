   INCLUDE  "exec/types.i"
   INCLUDE  "janus/janus.i"
   INCLUDE  "asmsupp.i"

   XREF  _LVOSendJanusInt
   XREF  _LVOSetupJanusSig
   XREF  _LVOAllocJanusMem
   XREF  _LVOJanusMemToOffset
   XREF  _LVOFreeJanusMem
   XREF  _LVOCleanupJanusSig
   XREF  _LVOJanusLock
   XREF  _LVOJanusUnlock

   XREF  _LVOOpenLibrary
   XREF  _LVOCloseLibrary
   XREF  _LVOAllocSignal
   XREF  _LVOFreeSignal

   XDEF   _SetUpJanus
   XDEF   _CleanUpJanus
   XDEF   _PCSignal
   XDEF   _PCMessage
   XDEF   _GetMouseXY
   XDEF   _SetMouseXY
   XDEF   _SetMousePressed

   XREF   _SetVideoMode
   XREF   _SetMouseImage
   XREF   _NewMouseImage
   XREF   _MoveMouseImage
   XREF   _DualPortParams

   include   "myjanus.i"

;************************************************************************
;*
;* _PCMessage
;*
;************************************************************************
_PCMessage
   link     a5,#-0
   movea.l  _DualPortParams,a0
   move.w   PCParam(a0),d0
   move.w   #-1,PCParam(a0)

; -----------------------------  if only  ---------------------------
   move.w   d0,-(a7)      ; save request number
   moveq    #JIntNum,d0
   movea.l  JanusBase,a6
   jsr      _LVOSendJanusInt(a6)   ; acknowledge
   move.w   (a7)+,d0      ; get request number back
; ----------------------------- oh!, well ---------------------------

;   move.b  #$a5,PCParam(a0)      ; tell PC it can use it now

   bmi      PCMExit         ; negative not allowed
   cmpi.w   #5,d0
   bgt      PCMExit         ; greater than max
   asl.w    #2,d0
   move.l   JTable(pc,d0.w),d0
   jsr      JTable(pc,d0.w)      ; switch( what to do )
PCMExit
   unlk     a5
   rts

JTable
   dc.l     VideoMode-JTable   ;   0
   dc.l     MouseImage-JTable   ;   1
   dc.l     MouseOn-JTable      ;   2
   dc.l     MouseOff-JTable      ;   3
   dc.l     MouseMove-JTable   ;   4
   dc.l     StandardImage-JTable   ;   5

StandardImage
   move.l   #0,-(a7)
   bra      DoImage
MouseImage
   movea.l  JanusMem,a0
   pea      32(a0)         ; mouse image data
DoImage
   bsr      _NewMouseImage
   addq.w   #4,a7
   rts

MouseOn
   move.l   #-1,-(a7)
MouseOnOff
   bsr      _SetMouseImage
   addq.w   #4,a7
   rts

MouseOff
   move.l   #0,-(a7)
   bra      MouseOnOff

VideoMode
   movea.l  JanusMem,a0
   clr.l    d0
   move.w   PCVideoMode(a0),d0
   move.l   d0,-(a7)
   bsr      _SetVideoMode
   addq.w   #4,a7
   rts

MouseMove
   jsr      _GetMouseXY
   clr.l    d1
   move.w   d0,d1
   move.l   d1,-(a7)
   moveq    #16,d1
   asr.l    d1,d0
   move.l   d0,-(a7)
   jsr      _MoveMouseImage
   addq.w   #8,a7
   rts

;************************************************************************
;*
;* _SetUpJanus
;*
;************************************************************************
_SetUpJanus
   link     A5,#-0
   MOVEA.L  4,A6
   LEA      Janus,A1
   MOVEQ    #0,D0
   JSR      _LVOOpenLibrary(A6)      ; OpenLibrary("janus.library,0)
   MOVE.L   D0,JanusBase
   bne      2$         ; jump if OK
   bra      1$
2$
   MOVEQ    #-1,D0
   JSR      _LVOAllocSignal(A6)      ; AllocSignal(-1)
   MOVE.W   D0,Signum      ; store in Signum
   BMI      CU4         ; jump if failed (-1)
   MOVEQ    #1,D1
   LSL.L    D0,D1
   MOVE.L   D1,_PCSignal      ; store 1 << Signum
   MOVE.L   D0,D1
   MOVEQ    #JIntNum,D0
   MOVEQ    #ParamSize,D2
   MOVE.L   #WordParam,D3
   MOVEA.L  JanusBase,A6
   JSR      _LVOSetupJanusSig(A6)   ; SetupJanusSig(
               ;   JintNum = JIntNum,
               ;   Signum,
               ;   Size = 0,
               ;   MEM_WORDACCESS+MEMF_PARAMETER)
   MOVE.L   D0,SigPtr      ; store setupSig ptr
   BEQ.S    CU5         ; jump if failed
   move.l   #DataSize,d0
   move.l   #WordBuff,d1
   jsr      _LVOAllocJanusMem(A6)
   move.l   d0,JanusMem
   beq.l    CU1
   moveq    #(DataSize>>2)-1,d1
   move.l   d0,a0
9$
   clr.l    (a0)+         ; zeroise the area
   dbf      d1,9$

   jsr      _LVOJanusMemToOffset(a6)
   movea.l  SigPtr,a0
   movea.l  $1e(a0),a0      ; Parameter area
   move.w   d0,(a0)+      ; store offset for PC to read
   clr.l    (a0)         ; zeroise 4 bytes - inc. Lock
   move.l   SigPtr,d0
1$
   unlk     A5         ; finished - restore stack
   rts

;************************************************************************
;*
;* _CleanUpJanus
;*
;************************************************************************
_CleanUpJanus
   link     A5,#-0
   MOVEA.L  JanusBase,A6      ; JanusBase
   move.l   #DataSize,d0
   movea.l  JanusMem,a1
   jsr      _LVOFreeJanusMem(a6)


CU1
   MOVEA.L  SigPtr,A0      ; SetupSig ptr
   JSR      _LVOCleanupJanusSig(A6)   ; CleanupJanusSig(ptr)


CU5
   MOVEA.L  4,A6         ; ExecBase
   MOVE.W   Signum,D0      ; Signum
   JSR      _LVOFreeSignal(A6)      ; FreeSignal(Signum)


CU4
   MOVEA.L  JanusBase,A1      ; JanusBase
   JSR      _LVOCloseLibrary(A6)      ; CloseLibrary(JanusBase)
   moveq    #0,d0
   unlk     A5         ; finished - restore stack
   RTS



   
;************************************************************************
;*
;* _SetMouseXY
;*
;************************************************************************
;   routine to set Mouse X and Y
;
;   parameters

MouseX      equ   10
MouseY      equ   14


_SetMouseXY
   link     A5,#-0

   movea.l  _DualPortParams,a0
   lea      WriteLock(a0),a0
   movea.l  JanusBase,a6
   move.w   MouseX(A5),D0      ; what if JanusLock ..
   move.w   MouseY(A5),D1      ; .. should destroy ?
   move.l   A0,-(sp)
   jsr      _LVOJanusLock(a6)

   movea.l  JanusMem,A0
   add.w    D0,AmigaPCX(A0)
   add.w    D1,AmigaPCY(A0)

   move.l   (sp)+,A0      ; lock address

;   movea.l _DualPortParams,a0
;   lea     WriteLock(a0),a0
;   movea.l JanusBase,a6

   jsr      _LVOJanusUnlock(a6)

   unlk     A5         ; finished - restore stack
   rts            ;            and return
   
;************************************************************************
;*
;* _GetMouseXY
;*
;************************************************************************
;   routine to get Mouse X and Y as seen by the PC
;

_GetMouseXY
   link     A5,#-0
   movea.l  JanusMem,A0
   move.l   PCAmigaX(A0),D0      ; get X and Y as seen by the PC
   unlk     A5         ; finished - restore stack
   rts            ;            and return

;************************************************************************
;*
;* _SetMousePressed
;*
;************************************************************************
;   routine to set Mouse buttons pressed/released
;
;   parameters

LeftP       equ   10   ; pressed
RightP      equ   14
LeftR       equ   18   ; released
RightR      equ   22
Status      equ   26

_SetMousePressed
   link     A5,#-0

   movea.l  _DualPortParams,a0
   lea      WriteLock(a0),a0
   movea.l  JanusBase,a6
   jsr      _LVOJanusLock(a6)

   movea.l  JanusMem,A0
   move.w   LeftP(A5),D0
   add.w    D0,AmigaPCLeftP(A0)
   move.w   RightP(A5),D0
   add.w    D0,AmigaPCRightP(A0)
   move.w   LeftR(A5),D0
   add.w    D0,AmigaPCLeftR(A0)
   move.w   RightR(A5),D0
   add.w    D0,AmigaPCRightR(A0)
   move.w   Status(A5),AmigaPCStatus(A0)

   movea.l  _DualPortParams,a0
   lea      WriteLock(a0),a0
   movea.l  JanusBase,a6
   jsr      _LVOJanusUnlock(a6)

   unlk     A5         ; finished - restore stack
   rts            ;            and return

;************************************************************************
;*
;* _myHandlerStub
;*
;************************************************************************
        XREF  _myHandler
        XDEF  _myHandlerStub

_myHandlerStub:

   movem.l  D2/D3/A4,-(sp)
   MOVEM.L  A0/A1,-(sp)
   JSR      _myHandler
   MOVEM.L  (sp)+,A0/A1
   movem.l  (sp)+,D2/D3/A4
   RTS


_XScaleD
   ds.w   0
   dc.w   2
_XScaleM
   ds.w   0
   dc.w   2
_YScaleD
   ds.w   0
   dc.w   2
_YScaleM
   ds.w   0
   dc.w   2
Signum
   ds.w   1
_PCSignal
   ds.l   1
SigPtr
   ds.l   1
JanusMem
   ds.l   1
JanusBase
   ds.l   1
Janus
   dc.b   'janus.library',0
