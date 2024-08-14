* === rxslib.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* The System library for the REXX interpreter.

         IDNT     RexxSysLib
         SECTION  RexxSysLib,CODE

         INCLUDE  "storage.i"
         INCLUDE  "rxslib.i"
         INCLUDE  "rexx.i"

         INCLUDE  "exec/initializers.i"
         INCLUDE  "exec/resident.i"

         ; Static data definitions

         XDEF     ROMTag

         ; External routines called

         IFND     PRODUCT
         XREF     AddClipNode
         XREF     AddRsrcNode
         XREF     AssignValue
         XREF     ClearRexxMsg
         XREF     ClearMem
         XREF     CloseF
         XREF     ClosePublicPort
         XREF     CmpString
         XREF     CreateArgstring
         XREF     CreateCLI
         XREF     CreateDOSPkt
         XREF     CreateRexxMsg
         XREF     CurrentEnv
         XREF     CVa2i
         XREF     CVi2a
         XREF     CVi2arg
         XREF     CVi2az
         XREF     CVc2x
         XREF     CVs2i
         XREF     CVx2c
         XREF     DeleteArgstring
         XREF     DeleteCLI
         XREF     DeleteDOSPkt
         XREF     DeleteRexxMsg
         XREF     DOSCommand
         XREF     DOSRead
         XREF     DOSWrite
         XREF     EndCode
         XREF     EnterSymbol
         XREF     ErrorMsg
         XREF     EvalOp
         XREF     ExistF
         XREF     FetchValue
         XREF     FillRexxMsg
         XREF     FindDevice
         XREF     FindRsrcNode
         XREF     FindSpace
         XREF     FreePort
         XREF     FreeSpace
         XREF     GetSpace
         XREF     InitList
         XREF     InitPort
         XREF     IsRexxMsg
         XREF     IsSymbol
         XREF     LengthArgstring
         XREF     LibInit
         XREF     LibOpen
         XREF     LibClose
         XREF     LibExpunge
         XREF     LibReserved
         XREF     ListNames
         XREF     LocateFile
         XREF     LockRexxBase
         XREF     LookUpValue
         XREF     OpenF
         XREF     OpenPublicPort
         XREF     QueueF
         XREF     ReadF
         XREF     ReadStr
         XREF     RemClipNode
         XREF     RemRsrcList
         XREF     RemRsrcNode
         XREF     Rexx
         XREF     rxInstruct
         XREF     rxParse
         XREF     rxSegList
         XREF     rxSuspend
         XREF     SeekF
         XREF     SendDOSPkt
         XREF     SetValue
         XREF     StackF
         XREF     StcToken
         XREF     StrcmpN
         XREF     StrcmpU
         XREF     StrcpyA
         XREF     StrcpyN
         XREF     StrcpyU
         XREF     StrflipN
         XREF     Strlen
         XREF     SymExpand
         XREF     SysConfig
         XREF     ToUpper
         XREF     UnlockRexxBase
         XREF     WriteF
         XREF     WaitDOSPkt

         ; External data referenced

         XREF     rxsID
         XREF     rxsName
         XREF     EndCode
         ENDC

         IFEQ     REVISION&1
         FAIL     'Commodore shipped ARexx must be odd revision'
         ENDC

         ; First executable location (not a valid entry point)

Start    moveq    #0,d0
         rts

         ; The "romtag" structure follows

ROMTag   dc.w   RTC_MATCHWORD          ; the magic word
         dc.l   ROMTag                 ; insurance
         dc.l   EndCode                ; end of checksum
         dc.b   RTF_AUTOINIT           ; initializer table supplied
         dc.b   RXS_LIB_VERSION        ; version number
         dc.b   NT_LIBRARY             ; node type
         dc.b   0                      ; initialization priority
         dc.l   rxsName                ; library name
         dc.l   rxsID                  ; library ID
         dc.l   Init                   ; initialization table

         CNOP     0,4
Init     dc.l     rl_SIZEOF            ; size of library node
         dc.l     FuncTable            ; entry points
         dc.l     DataTable            ; initialization table
         dc.l     LibInit              ; initialization function

         ; Table of entry points in jump table order.

FuncTable
         IFND     PRODUCT
         dc.l     LibOpen              ; required entry point
         dc.l     LibClose             ; required
         dc.l     LibExpunge           ; required
         dc.l     LibReserved          ; reserved

         dc.l     Rexx                 ; Main entry point
         dc.l     rxParse              ; (private)
         dc.l     rxInstruct           ; (private)
         dc.l     rxSuspend            ; (private)
         dc.l     EvalOp               ; (private)

         dc.l     AssignValue          ; (private)
         dc.l     EnterSymbol          ; (private)
         dc.l     FetchValue           ; (private)
         dc.l     LookUpValue          ; (private)
         dc.l     SetValue             ; (private)
         dc.l     SymExpand            ; (private)

         dc.l     ErrorMsg
         dc.l     IsSymbol
         dc.l     CurrentEnv
         dc.l     GetSpace
         dc.l     FreeSpace

         dc.l     CreateArgstring
         dc.l     DeleteArgstring
         dc.l     LengthArgstring
         dc.l     CreateRexxMsg
         dc.l     DeleteRexxMsg
         dc.l     ClearRexxMsg
         dc.l     FillRexxMsg
         dc.l     IsRexxMsg

         dc.l     AddRsrcNode
         dc.l     FindRsrcNode
         dc.l     RemRsrcList
         dc.l     RemRsrcNode
         dc.l     OpenPublicPort
         dc.l     ClosePublicPort
         dc.l     ListNames

         dc.l     ClearMem
         dc.l     InitList
         dc.l     InitPort
         dc.l     FreePort

         dc.l     CmpString
         dc.l     StcToken
         dc.l     StrcmpN
         dc.l     StrcmpU
         dc.l     StrcpyA              ; obsolete
         dc.l     StrcpyN
         dc.l     StrcpyU
         dc.l     StrflipN
         dc.l     Strlen
         dc.l     ToUpper

         dc.l     CVa2i
         dc.l     CVi2a
         dc.l     CVi2arg
         dc.l     CVi2az
         dc.l     CVc2x
         dc.l     CVx2c

         dc.l     OpenF
         dc.l     CloseF
         dc.l     ReadStr
         dc.l     ReadF
         dc.l     WriteF
         dc.l     SeekF
         dc.l     QueueF
         dc.l     StackF
         dc.l     ExistF

         dc.l     DOSCommand
         dc.l     DOSRead
         dc.l     DOSWrite
         dc.l     CreateDOSPkt         ; obsolete
         dc.l     DeleteDOSPkt         ; obsolete
         dc.l     SendDOSPkt           ; (private)
         dc.l     WaitDOSPkt           ; (private)
         dc.l     FindDevice

         dc.l     AddClipNode
         dc.l     RemClipNode
         dc.l     LockRexxBase
         dc.l     UnlockRexxBase
         dc.l     CreateCLI
         dc.l     DeleteCLI

         dc.l     CVs2i
         dc.l     -1                   ; table end marker
         ENDC

         IFD      PRODUCT
         dc.w     -1
         dc.w     LibOpen-FuncTable    ; required entry point
         dc.w     LibClose-FuncTable   ; required
         dc.w     LibExpunge-FuncTable ; required
         dc.w     LibReserved-FuncTable; reserved

         dc.w     Rexx-FuncTable       ; Main entry point
         dc.w     rxParse-FuncTable    ; (private)
         dc.w     rxInstruct-FuncTable ; (private)
         dc.w     rxSuspend-FuncTable  ; (private)
         dc.w     EvalOp-FuncTable     ; (private)

         dc.w     AssignValue-FuncTable; (private)
         dc.w     EnterSymbol-FuncTable; (private)
         dc.w     FetchValue-FuncTable ; (private)
         dc.w     LookUpValue-FuncTable; (private)
         dc.w     SetValue-FuncTable   ; (private)
         dc.w     SymExpand-FuncTable  ; (private)

         dc.w     ErrorMsg-FuncTable
         dc.w     IsSymbol-FuncTable
         dc.w     CurrentEnv-FuncTable
         dc.w     GetSpace-FuncTable
         dc.w     FreeSpace-FuncTable

         dc.w     CreateArgstring-FuncTable
         dc.w     DeleteArgstring-FuncTable
         dc.w     LengthArgstring-FuncTable
         dc.w     CreateRexxMsg-FuncTable
         dc.w     DeleteRexxMsg-FuncTable
         dc.w     ClearRexxMsg-FuncTable
         dc.w     FillRexxMsg-FuncTable
         dc.w     IsRexxMsg-FuncTable

         dc.w     AddRsrcNode-FuncTable
         dc.w     FindRsrcNode-FuncTable
         dc.w     RemRsrcList-FuncTable
         dc.w     RemRsrcNode-FuncTable
         dc.w     OpenPublicPort-FuncTable
         dc.w     ClosePublicPort-FuncTable
         dc.w     ListNames-FuncTable

         dc.w     ClearMem-FuncTable
         dc.w     InitList-FuncTable
         dc.w     InitPort-FuncTable
         dc.w     FreePort-FuncTable

         dc.w     CmpString-FuncTable
         dc.w     StcToken-FuncTable
         dc.w     StrcmpN-FuncTable
         dc.w     StrcmpU-FuncTable
         dc.w     StrcpyA-FuncTable       ; obsolete
         dc.w     StrcpyN-FuncTable
         dc.w     StrcpyU-FuncTable
         dc.w     StrflipN-FuncTable
         dc.w     Strlen-FuncTable
         dc.w     ToUpper-FuncTable

         dc.w     CVa2i-FuncTable
         dc.w     CVi2a-FuncTable
         dc.w     CVi2arg-FuncTable
         dc.w     CVi2az-FuncTable
         dc.w     CVc2x-FuncTable
         dc.w     CVx2c-FuncTable

         dc.w     OpenF-FuncTable
         dc.w     CloseF-FuncTable
         dc.w     ReadStr-FuncTable
         dc.w     ReadF-FuncTable
         dc.w     WriteF-FuncTable
         dc.w     SeekF-FuncTable
         dc.w     QueueF-FuncTable
         dc.w     StackF-FuncTable
         dc.w     ExistF-FuncTable

         dc.w     DOSCommand-FuncTable
         dc.w     DOSRead-FuncTable
         dc.w     DOSWrite-FuncTable
         dc.w     CreateDOSPkt-FuncTable  ; obsolete
         dc.w     DeleteDOSPkt-FuncTable  ; obsolete
         dc.w     SendDOSPkt-FuncTable    ; (private)
         dc.w     WaitDOSPkt-FuncTable    ; (private)
         dc.w     FindDevice-FuncTable

         dc.w     AddClipNode-FuncTable
         dc.w     RemClipNode-FuncTable
         dc.w     LockRexxBase-FuncTable
         dc.w     UnlockRexxBase-FuncTable
         dc.w     CreateCLI-FuncTable
         dc.w     DeleteCLI-FuncTable

         dc.w     CVs2i-FuncTable
         dc.w     -1
         ENDC

         ; Initializers for the allocated library structure.

DataTable
         INITBYTE LN_TYPE,NT_LIBRARY
         INITBYTE LIB_FLAGS,(LIBF_SUMUSED!LIBF_CHANGED)
         INITWORD LIB_VERSION,RXS_LIB_VERSION
         INITWORD LIB_REVISION,RXS_LIB_REVISION
         dc.l     0

