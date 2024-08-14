* === rexxglue.asm =====================================================
*
* Copyright (c) 1986, 1987, 1988 by William S. Hawes (All Rights Reserved)
*
* ======================================================================
* "Glue" routines for calling functions in the ARexx Systems Library.
* All calls assume that the external _RexxSysBase has been set to the
* ARexx SYstems library base by a call to OpenLibrary.

         INCLUDE  "rexx/storage.i"
         INCLUDE  "rexx/rxslib.i"

         XREF     _RexxSysBase

* First calling convention:
* 1, 2, or 3 parameters in (A0,A1,D0), return value in D0.

         ; node = AddRsrcNode(list,name,size);

         XDEF     _AddRsrcNode
_AddRsrcNode
         move.w   #_LVOAddRsrcNode,d1
         bra.s    CallSeq1

         ; CloseF(iobptr);

         XDEF     _CloseF
_CloseF  move.w   #_LVOCloseF,d1
         bra.s    CallSeq1

         ; test = CmpString(string1,string2);

         XDEF     _CmpString
_CmpString:
         move.w   #_LVOCmpString,d1
         bra.s    CallSeq1

         ; envptr = CurrentEnv(rxtptr);

         XDEF     _CurrentEnv
_CurrentEnv:
         moveq    #_LVOCurrentEnv,d1
         bra.s    CallSeq1

         ; ClosePublicPort(node);

         XDEF     _ClosePublicPort
_ClosePublicPort
         move.w   #_LVOClosePublicPort,d1
         bra.s    CallSeq1

         ; spptr = CreateDOSPkt();

         XDEF     _CreateDOSPkt
_CreateDOSPkt
         move.w   #_LVOCreateDOSPkt,d1
         bra.s    CallSeq1

         ; msgptr = CreateRexxMsg(replyport,fileext,hostname);

         XDEF     _CreateRexxMsg
_CreateRexxMsg
         move.w   #_LVOCreateRexxMsg,d1
         bra.s    CallSeq1

         ; DeleteArgstring(argptr);

         XDEF     _DeleteArgstring
_DeleteArgstring
         move.w   #_LVODeleteArgstring,d1
         bra.s    CallSeq1

         ; DeleteRexxMsg(msgptr);

         XDEF     _DeleteRexxMsg
_DeleteRexxMsg
         move.w   #_LVODeleteRexxMsg,d1
         bra.s    CallSeq1

         ; code = DOSCommand(string,filehandle);

         XDEF     _DOSCommand
_DOSCommand
         move.w   #_LVODOSCommand,d1
         bra.s    CallSeq1

         ; count = DOSRead(filehandle,buffer,length);

         XDEF     _DOSRead
_DOSRead
         move.w   #_LVODOSRead,d1
         bra.s    CallSeq1

         ; count = DOSWrite(filehandle,buffer,length);

         XDEF     _DOSWrite
_DOSWrite
         move.w   #_LVODOSWrite,d1
         bra.s    CallSeq1

         ; DeleteDOSPkt(stdpkt);

         XDEF     _DeleteDOSPkt
_DeleteDOSPkt:
         move.w   #_LVODeleteDOSPkt,d1
         bra.s    CallSeq1

         ; count = ExistF(filename);

         XDEF     _ExistF
_ExistF:
         move.w   #_LVOExistF,d1
         bra.s    CallSeq1

         ; nodeptr = FindRsrcNode(list,name,type);

         XDEF     _FindRsrcNode
_FindRsrcNode
         move.w   #_LVOFindRsrcNode,d1
         bra.s    CallSeq1

         ; FreeSpace(envptr,memptr,length);

         XDEF     _FreeSpace
_FreeSpace
         moveq    #_LVOFreeSpace,d1
         bra.s    CallSeq1

         ; FreePort(msgport);

         XDEF     _FreePort
_FreePort
         move.w   #_LVOFreePort,d1
         bra.s    CallSeq1

         ; type = IsSymbol(string);

         XDEF     _IsSymbol
_IsSymbol
         moveq    #_LVOIsSymbol,d1
         bra.s    CallSeq1

         ; InitList(list);

         XDEF     _InitList
_InitList
         move.w   #_LVOInitList,d1
         bra.s    CallSeq1

         ; signal = InitPort(msgport);

         XDEF     _InitPort
_InitPort
         move.w   #_LVOInitPort,d1
         bra.s    CallSeq1

         ; boolean = IsRexxMsg(msgptr);

         XDEF     _IsRexxMsg
_IsRexxMsg
         move.w   #_LVOIsRexxMsg,d1

         ; Load three arguments into (A0,A1,D0)

CallSeq1 movem.l  4(sp),a0/a1
         move.l   12(sp),d0

         ; Call the library function

CallFunc move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6
         jsr      0(a6,d1.w)
         movea.l  (sp)+,a6
         rts

         ; length = LengthArgstring(argptr);

         XDEF     _LengthArgstring
_LengthArgstring
         move.w   #_LVOLengthArgstring,d1
         bra.s    CallSeq1

         ; nodeptr = OpenPublicPort(list,name);

         XDEF     _OpenPublicPort
_OpenPublicPort:
         move.w   #_LVOOpenPublicPort,d1
         bra.s    CallSeq1

         ; count = QueueF(iobptr,buffer,length);

         XDEF     _QueueF
_QueueF  move.w   #_LVOQueueF,d1
         bra.s    CallSeq1

         ; count = ReadF(iobptr,buffer,length);

         XDEF     _ReadF
_ReadF   move.w   #_LVOReadF,d1
         bra.s    CallSeq1

         ; count = ReadStr(iobptr,buffer,length);

         XDEF     _ReadStr
_ReadStr move.w   #_LVOReadStr,d1
         bra.s    CallSeq1

         ; RemRsrcList(list);

         XDEF     _RemRsrcList
_RemRsrcList
         move.w   #_LVORemRsrcList,d1
         bra.s    CallSeq1

         ; RemRsrcNode(node);

         XDEF     _RemRsrcNode
_RemRsrcNode
         move.w   #_LVORemRsrcNode,d1
         bra.s    CallSeq1

         ; RemClipNode(node);

         XDEF     _RemClipNode
_RemClipNode
         move.w   #_LVORemClipNode,d1
         bra.s    CallSeq1

         ; test = StrcmpN(string1,string2,length);

         XDEF     _StrcmpN
_StrcmpN move.w   #_LVOStrcmpN,d1
         bra.s    CallSeq1

         ; test = StrcmpU(string1,string2,length);

         XDEF     _StrcmpU
_StrcmpU move.w   #_LVOStrcmpU,d1
         bra.s    CallSeq1

         ; test = StrcpyA(string1,string2,length);

         XDEF     _StrcpyA
_StrcpyA move.w   #_LVOStrcpyA,d1
         bra.s    CallSeq1

         ; test = StrcpyN(string1,string2,length);

         XDEF     _StrcpyN
_StrcpyN move.w   #_LVOStrcpyN,d1
         bra.s    CallSeq1

         ; test = StrcpyU(string1,string2,length);

         XDEF     _StrcpyU
_StrcpyU move.w   #_LVOStrcpyU,d1
         bra.s    CallSeq1

         ; length = Strlen(string);

         XDEF     _Strlen
_Strlen  move.w   #_LVOStrlen,d1
         bra.s    CallSeq1

         ; count = StackF(iobptr,buffer,length)

         XDEF     _StackF
_StackF  move.w   #_LVOStackF,d1
         bra.s    CallSeq1

         ; count = WriteF(iobptr,buffer,length)

         XDEF     _WriteF
_WriteF  move.w   #_LVOWriteF,d1
         bra.s    CallSeq1

* Second calling convention:  2 parameters in (A0,D0), return value in D0.

         ; argptr = CreateArgstring(string,length);

         XDEF     _CreateArgstring
_CreateArgstring:
         moveq    #_LVOCreateArgstring,d1
         bra.s    CallSeq2

         ; ClearMem(address,length);

         XDEF     _ClearMem
_ClearMem
         move.w   #_LVOClearMem,d1
         bra.s    CallSeq2

         ; count = ClearRexxMsg(msgptr,count);

         XDEF     _ClearRexxMsg
_ClearRexxMsg
         move.w   #_LVOClearRexxMsg,d1
         bra.s    CallSeq2

         ; devptr = FindDevice(name,type);

         XDEF     _FindDevice
_FindDevice
         move.w   #_LVOFindDevice,d1
         bra.s    CallSeq2

         ; block = GetSpace(envptr,size);

         XDEF     _GetSpace
_GetSpace
         moveq    #_LVOGetSpace,d1
         bra.s    CallSeq2

         ; argptr = ListNames(list,separator);

         XDEF     _ListNames
_ListNames
         move.w   #_LVOListNames,d1
         bra.s    CallSeq2

         ; test = StrFlipN(string1,length);

         XDEF     _StrflipN
_StrflipN
         move.w   #_LVOStrflipN,d1

         ; Load two arguments (A0,D0)

CallSeq2 movea.l  4(sp),a0
         move.l   8(sp),d0
         bra      CallFunc

* Third calling convention:  1 parameter in D0, return value in D0.

         ; argptr = CVi2arg(value);

         XDEF     _CVi2arg
_CVi2arg move.w   #_LVOCVi2arg,d1
         bra.s    CallSeq3

         ; LockRexxBase(resource);

         XDEF     _LockRexxBase
_LockRexxBase
         move.w   #_LVOLockRexxBase,d1
         bra.s    CallSeq3

         ; char = ToUpper(char);

         XDEF     _ToUpper
_ToUpper move.w   #_LVOToUpper,d1
         bra.s    CallSeq3

         ; UnlockRexxBase(resource);

         XDEF     _UnlockRexxBase
_UnlockRexxBase
         move.w   #_LVOUnlockRexxBase,d1

         ; Load one argument in D0

CallSeq3 move.l   4(sp),d0
         bra      CallFunc

* Fourth calling sequence:  3 parameters in (A0,D0,D1), return value in D0.

         ; length = CVi2a(buffer,value,digits);

         XDEF     _CVi2a
_CVi2a   move.w   #_LVOCVi2a,a1
         bra.s    CallSeq4

         ; length = CVi2az(buffer,value,digits);

         XDEF     _CVi2az
_CVi2az  move.w   #_LVOCVi2az,a1
         bra.s    CallSeq4

         ; boolean = FillRexxMsg(msgptr,count,mask);

         XDEF     _FillRexxMsg
_FillRexxMsg:
         move.w   #_LVOFillRexxMsg,a1
         bra.s    CallSeq4

         ; offset = SeekF(iobptr,offset,anchor);

         XDEF     _SeekF
_SeekF   move.w   #_LVOSeekF,a1

CallSeq4 movea.l  4(sp),a0
         movem.l  8(sp),d0/d1

         move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6      ; library base
         jsr      0(a6,a1.w)           ; call function
         movea.l  (sp)+,a6
         rts

* Special calling sequences

         ; node = AddClipNode(list,name,value,length);

         XDEF     _AddClipNode
_AddClipNode
         movem.l  4(sp),d0/d1/a0/a1
         exg      d0,a0
         exg      d1,a1
         move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6      ; library base
         jsr      _LVOAddClipNode(a6)  ; D0=node
         movea.l  (sp)+,a6
         rts

         ; error = CVc2x(buffer,string,length,mode);

         XDEF     _CVc2x
_CVc2x   movem.l  4(sp),d0/d1/a0/a1
         exg      d0,a0
         exg      d1,a1
         move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6      ; library base
         jsr      _LVOCVc2x(a6)        ; D0=error D1=length
         movea.l  (sp)+,a6
         rts

         ; error = CVx2c(buffer,string,length,mode);

         XDEF     _CVx2c
_CVx2c   movem.l  4(sp),d0/d1/a0/a1
         exg      d0,a0
         exg      d1,a1
         move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6      ; library base
         jsr      _LVOCVx2c(a6)        ; D0=error D1=length
         movea.l  (sp)+,a6
         rts

         ; iobptr = OpenF(list,filename,length,logical);

         XDEF     _OpenF
_OpenF   movem.l  4(sp),d0/d1/a0/a1
         exg      d0,a0
         exg      d1,a1
         move.l   a6,-(sp)
         movea.l  _RexxSysBase,a6
         jsr      _LVOOpenF(a6)        ; D0=IoBuff pointer
         movea.l  (sp)+,a6
         rts

         ; value = CVa2i(string,&digits);

         XDEF     _CVa2i
_CVa2i   movea.l  4(sp),a0             ; scan pointer
         move.w   #_LVOCVa2i,d1
         bsr      CallFunc             ; D0=value D1=digits
         movea.l  8(sp),a1             ; return pointer
         move.l   d1,(a1)              ; digits scanned
         rts

         ; boolean = ErrorMsg(code,&ssptr);

         XDEF     _ErrorMsg
_ErrorMsg
         move.l   4(sp),d0             ; error code
         moveq    #_LVOErrorMsg,d1
         bsr      CallFunc             ; D0=boolean  A0=string structure
         movea.l  8(sp),a1             ; return pointer
         move.l   a0,(a1)              ; string structure
         rts

         END
