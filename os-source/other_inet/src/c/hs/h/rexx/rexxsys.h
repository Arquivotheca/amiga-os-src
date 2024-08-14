/*  $Header: rexxsys_lib.fd,v 0.3 88/08/04 joe stivaletta Exp $*/
/**/
/* The library entry point offsets*/
/**/
/*pragma libcall RexxSysBase Rexx 1e 0*/
/*pragma libcall RexxSysBase rxParse 24 0*/
/*pragma libcall RexxSysBase rxInstruct 2a 0*/
/*pragma libcall RexxSysBase rxSuspend 30 0*/
/*pragma libcall RexxSysBase EvalOp 36 0*/
/**/
/*pragma libcall RexxSysBase AssignValue 3c 0*/
/*pragma libcall RexxSysBase EnterSymbol 42 0*/
/*pragma libcall RexxSysBase FetchValue 48 0*/
/*pragma libcall RexxSysBase LookUpValue 4e 0*/
/*pragma libcall RexxSysBase SetValue 54 0*/
/*pragma libcall RexxSysBase SymExpand 5a 0*/
/**/
/*pragma libcall RexxSysBase ErrorMsg 60 0*/
#pragma libcall RexxSysBase IsSymbol 66 801
#pragma libcall RexxSysBase CurrentEnv 6c 801
#pragma libcall RexxSysBase GetSpace 72 802
#pragma libcall RexxSysBase FreeSpace 78 9803
/**/
#pragma libcall RexxSysBase CreateArgstring 7e 802
#pragma libcall RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxSysBase LengthArgstring 8a 801
#pragma libcall RexxSysBase CreateRexxMsg 90 9803
#pragma libcall RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxSysBase ClearRexxMsg 9c 802
#pragma libcall RexxSysBase FillRexxMsg a2 10803
#pragma libcall RexxSysBase IsRexxMsg a8 801
/**/
#pragma libcall RexxSysBase AddRsrcNode ae 9803
#pragma libcall RexxSysBase FindRsrcNode b4 9803
#pragma libcall RexxSysBase RemRsrcList ba 801
#pragma libcall RexxSysBase RemRsrcNode c0 801
#pragma libcall RexxSysBase OpenPublicPort c6 9802
#pragma libcall RexxSysBase ClosePublicPort cc 801
#pragma libcall RexxSysBase ListNames d2 802
/**/
#pragma libcall RexxSysBase ClearMem d8 802
#pragma libcall RexxSysBase InitList de 801
#pragma libcall RexxSysBase InitPort e4 9802
#pragma libcall RexxSysBase FreePort ea 801
/**/
#pragma libcall RexxSysBase CmpString f0 9802
/*pragma libcall RexxSysBase StcToken f6 0*/
#pragma libcall RexxSysBase StrcmpN fc 9803
#pragma libcall RexxSysBase StrcmpU 102 9803
#pragma libcall RexxSysBase StrcpyA 108 9803
#pragma libcall RexxSysBase StrcpyN 10e 9803
#pragma libcall RexxSysBase StrcpyU 114 9803
#pragma libcall RexxSysBase StrflipN 11a 802
#pragma libcall RexxSysBase Strlen 120 801
#pragma libcall RexxSysBase ToUpper 126 1
/**/
/*pragma libcall RexxSysBase CVa2i 12c 801*/
#pragma libcall RexxSysBase CVi2a 132 10803
#pragma libcall RexxSysBase CVi2arg 138 1
#pragma libcall RexxSysBase CVi2az 13e 10803
#pragma libcall RexxSysBase CVc2x 144 109804
#pragma libcall RexxSysBase CVx2c 14a 109804
/**/
#pragma libcall RexxSysBase OpenF 150 109804
#pragma libcall RexxSysBase CloseF 156 801
#pragma libcall RexxSysBase ReadStr 15c 9803
#pragma libcall RexxSysBase ReadF 162 9803
#pragma libcall RexxSysBase WriteF 168 9803
#pragma libcall RexxSysBase SeekF 16e 10803
#pragma libcall RexxSysBase QueueF 174 9803
#pragma libcall RexxSysBase StackF 17a 9803
#pragma libcall RexxSysBase ExistF 180 801
/**/
#pragma libcall RexxSysBase DOSCommand 186 9802
#pragma libcall RexxSysBase DOSRead 18c 9803
#pragma libcall RexxSysBase DOSWrite 192 9803
#pragma libcall RexxSysBase CreateDOSPkt 198 0
#pragma libcall RexxSysBase DeleteDOSPkt 19e 801
/*pragma libcall RexxSysBase SendDOSPkt 1a4 0*/
/*pragma libcall RexxSysBase WaitDOSPkt 1aa 0*/
#pragma libcall RexxSysBase FindDevice 1b0 802
/**/
#pragma libcall RexxSysBase AddClipNode 1b6 109804
#pragma libcall RexxSysBase RemClipNode 1bc 801
#pragma libcall RexxSysBase LockRexxBase 1c2 1
#pragma libcall RexxSysBase UnlockRexxBase 1c8 1
