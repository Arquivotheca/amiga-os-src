* == rexxlib.asm =======================================================
*
* Copyright (c) 1986-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Production version of the REXX systems library.

         IDNT     RexxSysLib
         SECTION  RexxSysLib,CODE

PRODUCT  EQU      1                    ; production version?
*MEMCHECK EQU      1                    ; memory testing?
*BETATEST EQU      1                    ; beta-test version?

         OPTIMON                       ; optimization flag

         NOLIST
         INCLUDE  "storage.i"
         INCLUDE  "rxslib.i"
         INCLUDE  "errors.i"
         INCLUDE  "rexxio.i"

         INCLUDE  "version.i"
         INCLUDE  "rexx.i"
         INCLUDE  "rexxenv.i"
         INCLUDE  "instruct.i"
         INCLUDE  "oper.i"
         INCLUDE  "btree.i"
         INCLUDE  "rexxmath.i"

         INCLUDE  "exec/tasks.i"
         INCLUDE  "exec/initializers.i"
         INCLUDE  "exec/resident.i"

         INCLUDE  "utility/tagitem.i"

         INCLUDE  "dos/dos.i"
         INCLUDE  "dos/dosextens.i"
         INCLUDE  "dos/dostags.i"

;         INCLUDE  "libdefs.i"   ; system library definitions

         ; Macro to call a library function (library pointer in A6)

CALLSYS  MACRO    * FunctionName
         CALLLIB  _LVO\1
         ENDM

* Macro to define an external library entry point (offset)
XLIB     MACRO    * FunctionName
         XREF     _LVO\1
         ENDM

_AbsExecBase EQU  4

         ; DOS library entry points

         IFND     DOS_DOS_LIB_I
         XLIB     Close
         XLIB     CurrentDir
         XLIB     DateStamp
         XLIB     DupLock
         XLIB     Examine
         XLIB     Execute
         XLIB     IoErr
         XLIB     Lock
         XLIB     Open
         XLIB     ParentDir
         XLIB     Read
         XLIB     Seek
         XLIB     UnLock
         XLIB     WaitForChar
         XLIB     Write
_LVOSystem EQU    -6*(4+97)            ; 2.0 System() call
;         XLIB     System
         ENDC

         ; EXEC library entry points

         XLIB     AddPort
         XLIB     AddTail
         XLIB     AllocMem
         XLIB     AllocSignal
         XLIB     AvailMem
         XLIB     CloseLibrary
         XLIB     Disable
         XLIB     Enable
         XLIB     FindName
         XLIB     FindPort
         XLIB     Forbid
         XLIB     FreeMem
         XLIB     FreeSignal
         XLIB     GetMsg
         XLIB     OpenLibrary
         XLIB     Permit
         XLIB     PutMsg
         XLIB     RawDoFmt
         XLIB     Remove
         XLIB     RemPort
         XLIB     RemTail
         XLIB     ReplyMsg
         XLIB     SetSignal
         XLIB     SetTaskPri
         XLIB     Signal
         XLIB     Wait
         XLIB     WaitPort

         XLIB     IEEEDPAdd
         XLIB     IEEEDPCmp
         XLIB     IEEEDPDiv
         XLIB     IEEEDPMul
         XLIB     IEEEDPSub

         IFND     EXEC_EXECBASE_I
ThisTask          EQU   276
TDNestCnt         EQU   295
AttnFlags         EQU   296
LibList           EQU   378
PortList          EQU   392
VBlankFrequency   EQU   530

AFB_68010         EQU   0
AFB_68020         EQU   1
AFB_68881         EQU   4
         ENDC

         IFND GRAPHICS_GFXBASE_I
gb_DisplayFlags EQU $CE                ; offset to DisplayFlags
         ENDC

         XDEF     AddClipNode
         XDEF     AddHead
         XDEF     AddRsrcNode
         XDEF     AddString
         XDEF     AddTail
         XDEF     AddToken
         XDEF     AssignSpecial
         XDEF     AssignValue
         XDEF     BIFabs               ; ** temp **
         XDEF     BIFdatatype          ; ** temp **
         XDEF     BIFsourceln          ; ** temp **
         XDEF     BIFunc
         XDEF     CacheClause          ; ** temp **
         XDEF     CALLCode
         XDEF     ClearMem
         XDEF     ClearRexxMsg
         XDEF     CloseF
         XDEF     ClosePublicPort
         XDEF     CmpString
         XDEF     CmpStringLen
         XDEF     CopyClause
         XDEF     CopyString
         XDEF     CreateArgstring
         XDEF     CreateCLI
         XDEF     CreateDOSPkt
         XDEF     CreateRexxMsg
         XDEF     CTable
         XDEF     CurrentEnv
         XDEF     CVa2d
         XDEF     CVa2i
         XDEF     CVarg2s
         XDEF     CVc2x
         XDEF     CVd2a
         XDEF     CVd2s
         XDEF     CVFalse
         XDEF     CVi2a
         XDEF     CVi2arg
         XDEF     CVi2az
         XDEF     CVi2s
         XDEF     CVs2bool
         XDEF     CVs2d
         XDEF     CVs2i
         XDEF     CVx2c
         XDEF     DeleteArgstring
         XDEF     DeleteCLI
         XDEF     DeleteDOSPkt
         XDEF     DeleteRexxMsg
         XDEF     DOCode
         XDEF     DOSCommand
         XDEF     DOSExamine
         XDEF     DOSParent
         XDEF     DOSRead
         XDEF     DOSWaitChar
         XDEF     DOSWrite
         XDEF     DoubleWorkBuffer
         XDEF     DPExp                ; ** temp **
         XDEF     DPF1                 ; IEEE DP 1.0
         XDEF     DPFix
         XDEF     DPModf
         XDEF     DPNorm               ; ** temp **
         XDEF     DPPow
         XDEF     DPPow10              ; ** temp **
         XDEF     DPPower              ; ** temp **
         XDEF     DPTst
         XDEF     EndCode
         XDEF     EndToken
         XDEF     EnterSymbol
         XDEF     ErrClause
         XDEF     ErrLabel
         XDEF     ErrorMsg
         XDEF     EvalExpr             ; ** temp **
         XDEF     EvalOp
         XDEF     ExistF
         XDEF     ExprTree             ; ** temp **
         XDEF     ExtFunction
         XDEF     FetchValue
         XDEF     FillRexxMsg
         XDEF     FindCachedClause
         XDEF     FindCachedToken
         XDEF     FindConsoleHandler
         XDEF     FindDevice
         XDEF     FindLogical
         XDEF     FindQuickOne
         XDEF     FindQuickThree
         XDEF     FindQuickTwo
         XDEF     FindRsrcNode
         XDEF     FindSlowly           ; ** temp **
         XDEF     FindSpace
         XDEF     FindTrace
         XDEF     FlushClause
         XDEF     Forbid
         XDEF     FormatString
         XDEF     FreeBTNode
         XDEF     FreeBTree
         XDEF     FreeClause
         XDEF     FreeEnv              ; ** temp **
         XDEF     FreeKeepStr
         XDEF     FreePort
         XDEF     FreeQuickOne
         XDEF     FreeQuickThree
         XDEF     FreeQuickTwo
         XDEF     FreeRange
         XDEF     FreeSeg
         XDEF     FreeSlowly           ; ** temp **
         XDEF     FreeSpace
         XDEF     FreeSrcSeg
         XDEF     FreeString
         XDEF     FreeSymTable
         XDEF     FreeToken
         XDEF     GetBTNode
         XDEF     GetClause
         XDEF     GetEnv
         XDEF     GetRange
         XDEF     GetSpace
         XDEF     GetString
         XDEF     GetSymTable
         XDEF     GetValue
         XDEF     GetWorkBuffer
         XDEF     gnCOMMAND
         XDEF     gnFALSE
         XDEF     gnNULL
         XDEF     gnREXX
         XDEF     gnSTDERR
         XDEF     gnSTDIN
         XDEF     gnSTDOUT
         XDEF     gnTRUE
         XDEF     gtRESULT
         XDEF     HostCommand          ; ** temp **
         XDEF     InitGlobal
         XDEF     InitList
         XDEF     InitPort
         XDEF     IsOper
         XDEF     IsRexxMsg
         XDEF     IsString
         XDEF     IsSymbol
         XDEF     KeepString
         XDEF     KeyTable
         XDEF     Ldivu
         XDEF     LengthArgstring
         XDEF     LibClose
         XDEF     LibExpunge
         XDEF     LibInit
         XDEF     LibOpen
         XDEF     LibReserved
         XDEF     LinkBTNode
         XDEF     ListNames
         XDEF     LocateBTNode
         XDEF     LocateFile
         XDEF     LockF
         XDEF     LockRexxBase
         XDEF     LookUpValue
         XDEF     MaxLen
         XDEF     MayCopyString
         XDEF     NewClause
         XDEF     NextToken            ; ** temp **
         XDEF     OpenF
         XDEF     OpenPublicPort
         XDEF     ParseString
         XDEF     PathName
         XDEF     PatMatch             ; ** temp **
         XDEF     pENG
         XDEF     Permit
         XDEF     PopStack
         XDEF     PopTail
         XDEF     PRIMask
         XDEF     pSCI
         XDEF     PullString
         XDEF     PushStack
         XDEF     PushTail
         XDEF     QFreeToken
         XDEF     QMark
         XDEF     QueueF
         XDEF     ReadClause           ; ** temp **
         XDEF     ReadF
         XDEF     ReadSource
         XDEF     ReadStr
         XDEF     Relink
         XDEF     RemClipNode
         XDEF     RemHead
         XDEF     Remove
         XDEF     RemRsrcList
         XDEF     RemRsrcNode
         XDEF     RemTail
         XDEF     Rexx
         XDEF     RoundDigits          ; ** temp **
         XDEF     rxInsDO              ; ** temp **
         XDEF     rxInsEND             ; ** temp **
         XDEF     rxInsIF              ; ** temp **
         XDEF     rxInsPARSE           ; ** temp **
         XDEF     rxInsRETURN          ; ** temp **
         XDEF     rxInsTRACE           ; ** temp **
         XDEF     rxInstruct
         XDEF     rxLocate
         XDEF     rxParse
         XDEF     rxRange              ; ** temp **
         XDEF     rxSegList
         XDEF     rxsID
         XDEF     rxSignal
         XDEF     rxsName
         XDEF     rxSuspend
         XDEF     rxTrace
         XDEF     ScanClause           ; ** temp **
         XDEF     ScanDigits
         XDEF     ScanDO               ; ** temp **
         XDEF     ScanExpr             ; ** temp **
         XDEF     SDFinal              ; ** temp **
         XDEF     SecCode
         XDEF     SeekF
         XDEF     SendDOSPkt
         XDEF     SendRexxMsg
         XDEF     SetCachePosition
         XDEF     SetDirectory
         XDEF     SetDirName
         XDEF     SetPriority
         XDEF     SetStack
         XDEF     SetTrace
         XDEF     SetValue
         XDEF     SkipWord             ; ** temp **
         XDEF     SPCTable
         XDEF     SrcPosition
         XDEF     SrcSegment
         XDEF     StackF
         XDEF     StcToken
         XDEF     StrcmpN
         XDEF     StrcmpU
         XDEF     Strcopy
         XDEF     StrcpyA
         XDEF     StrcpyN
         XDEF     StrcpyU
         XDEF     StrflipN
         XDEF     Strlen
         XDEF     SymExpand
         XDEF     SysConfig
         XDEF     SysTime
         XDEF     ToUpper
         XDEF     TrimLeading
         XDEF     Unlink
         XDEF     UnlockF
         XDEF     UnlockRexxBase
         XDEF     UpperString
         XDEF     WaitDOSPkt
         XDEF     WriteF

         ; The source files

         LIST
         INCLUDE "rxslib.asm"

         INCLUDE "stacks.asm"
         INCLUDE "findspace.asm"
         INCLUDE "lex.asm"
         INCLUDE "strings.asm"
         INCLUDE "errormsg.asm"
         INCLUDE "keytable.asm"
         INCLUDE "ctable.asm"

         INCLUDE "struct.asm"
         INCLUDE "cvs2d.asm"
         INCLUDE "cvs2i.asm"
         INCLUDE "convert.asm"
         INCLUDE "dpmodf.asm"
         INCLUDE "readsource.asm"

         INCLUDE "rxtrace.asm"
         INCLUDE "rxlocate.asm"
         INCLUDE "assignvalue.asm"
         INCLUDE "symtable.asm"
         INCLUDE "evalop.asm"

         INCLUDE "rexx.asm"
         INCLUDE "rxparse.asm"
         INCLUDE "environ.asm"
         INCLUDE "rxdebug.asm"
         INCLUDE "readclause.asm"
         INCLUDE "scanclause.asm"
         INCLUDE "scanexpr.asm"
         INCLUDE "exprtree.asm"

         INCLUDE "rxinstruct.asm"
         INCLUDE "rxinsaddress.asm"
         INCLUDE "rxinsdo.asm"
         INCLUDE "rxinsdrop.asm"
         INCLUDE "rxinsif.asm"
         INCLUDE "rxinsnumeric.asm"
         INCLUDE "rxinsparse.asm"
         INCLUDE "rxinsreturn.asm"
         INCLUDE "rxinstrace.asm"

         INCLUDE "library.asm"
         INCLUDE "rxstartup.asm"
         INCLUDE "extfunction.asm"
         INCLUDE "packets.asm"
         INCLUDE "io.asm"
         INCLUDE "rxutility.asm"

         INCLUDE "bifunc.asm"
         INCLUDE "biflist.asm"
         INCLUDE "bif100.asm"
         INCLUDE "bif200.asm"
         INCLUDE "bif300.asm"
         INCLUDE "bif400.asm"
         INCLUDE "bif500.asm"
         INCLUDE "bif600.asm"
         INCLUDE "bif700.asm"
         INCLUDE "bif800.asm"
         INCLUDE "bif900.asm"

EndCode  EQU      *                       ; end of source

         END
