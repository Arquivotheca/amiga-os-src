SAS AMIGA 680x0OBJ Module Disassembler V6.01
Copyright © 1992 SAS Institute, Inc.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_client 0000-00    _FixBString 0774-00    _SaveFHName 07CA-00    
_SaveName 081A-00    _KeepLock 086A-00    _KeepFH 08B4-00    _NukeLock 08FE-00
_NukeFH 094E-00    _FixFH 099E-00    _FixLock 0B10-00    _Reconnect 0C24-00    
_StartFIND 0ED0-00    _EndFind 0F90-00    _StartEND 1048-00    _EndEnd 10D4-00
_EndSeek 10FC-00    _StartSeek 114E-00    _StartRead 11A2-00    
_StartWrite 12EE-00    _EndRead 1452-00    _EndWrite 14DE-00    
_StartRename 1530-00    _StartSetProtect 16AE-00    _StartSetComment 1786-00
_StartFreeLock 18BC-00    _EndFreeLock 1918-00    _StartSetDate 198A-00    
_StartParent 1A9C-00    _StartParentFH 1AF4-00    _EndParent 1B3E-00    
_StartDeleteObject 1C3A-00    _StartLocateObject 1D0A-00    
_StartExamine 1E96-00    _EndExamine 1F60-00    _StartSameLock 1FBC-00    
_StartInfo 204E-00    _StartChangeMode 20A8-00    _EndInfo 2134-00    
_StartFlush 21BA-00    _StartDiskInfo 21C4-00    _StartToID 21DC-00    
_StartToInfo 2214-00    _EndToInfo 223A-00    _EndDiskInfo 229E-00    
_StartOpenFromLock 2324-00    _StartLockFromFH 237C-00    
_StartReadLink 23C6-00    _DoStart 23E6-00    _DoEnd 2714-00    
_NetDownRequest 2888-00    _MountFailedRequest 2902-00    
_CantConnectRequest 29DC-00    _GPRequest 2A46-00    
_TimeOutEasyRequest 2AB0-00    _rev 0016-01    _revision 0036-01    
_SysBase 0000-02    _DOSBase 0004-02    _NIPCBase 0008-02    
_ServicesBase 000C-02    _IntuitionBase 0010-02

SECTION 00 "client.c" 00002BF0 BYTES
;   1: 
;   2: /*
;   3:  * Envoy Filesystem Client V0.0
;   4:  * $id$
;   5:  *
;   6:  *
;   7:  * BUGS:
;   8:  *
;   9:  * If on reconnect, the seek of a FH fails to locate the correct position, set the file
;  10:  * size TO at least the last known position.
;  11:  *
;  12:  * I ought to be doing something akin to gp = (UBYTE *) ((ULONG) (gp+3L) & ~3L) instead of the
;  13:  * slow little while() I have in there everywhere.  FIXME.
;  14:  *
;  15:  */
;  16: 
;  17: /* REQUESTERTIMEOUT is the # of secs EZ requests are brought up for */
;  18: #define REQUESTERTIMEOUT  20
;  19: #define FILTERWBFILES 3
;  20: 
;  21: #include <exec/types.h>
;  22: #include <exec/memory.h>
;  23: #include <dos/dos.h>
;  24: #include <dos/dosextens.h>
;  25: #include <dos/filehandler.h>
;  26: #include <intuition/intuition.h>
;  27: #include "/fs.h"
;  28: #include "efs_rev.h"
;  29: 
;  30: #include <pragmas/exec_pragmas.h>
;  31: #include <clib/exec_protos.h>
;  32: #include <pragmas/nipc_pragmas.h>
;  33: #include <clib/nipc_protos.h>
;  34: #include <pragmas/services_pragmas.h>
;  35: #include <clib/services_protos.h>
;  36: #include <dos.h>
;  37: 
;  38: struct Entity *Reconnect();
;  39: void SaveName();
;  40: BOOL FixLock();
;  41: void KeepLock();
;  42: void NukeLock();
;  43: void KeepFH();
;  44: void NukeFH();
;  45: BOOL FixFH();
;  46: void SaveFHName();
;  47: BOOL DoStart();
;  48: BOOL NetDownRequest(UBYTE *machine, UBYTE *volume);
;  49: BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err);
;  50: BOOL CantConnectRequest(UBYTE *machine);
;  51: BOOL GPRequest(ULONG num);
;  52: void DoEnd();
;  53: extern UBYTE *clpcpy(UBYTE *d, UBYTE *s, ULONG l);
;  54: LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout);
;  55: 
;  56: 
;  57: char *rev=VSTRING;
;  58: char *revision = VERSTAG;
;  59: 
;  60: #define DEBUGMSG kprintf
;  61: #define MAXSIZE 65536
;  62: #define PACKETTIMEOUT 10
;  63: #define MIN(x, y) ((x) < (y) ? (x):(y))
;  64: 
;  65: extern struct Library * AbsExecBase;
;  66: //#define SysBase  AbsExecBase
;  67: 
;  68: #define ERROR_NETWORK_FAILED ERROR_SEEK_ERROR
;  69: 
;  70: BOOL FixBString();
;  71: 
;  72: 
;  73:     struct Library *SysBase;
;  74:     struct Library *DOSBase;
;  75:     struct Library *NIPCBase;
;  76:     struct Library *ServicesBase;
;  77:     struct Library *IntuitionBase;
;  78: 
;  79: 
;  80: void client()
;  81: {
       | 0000  4E55 FD3C                      LINK        A5,#FD3C
       | 0004  48E7 2732                      MOVEM.L     D2/D5-D7/A2-A3/A6,-(A7)
;  82: 
;  83:     struct MsgPort *localport;
;  84:     struct StandardPacket *sp;
;  85:     struct DosPacket *dp;
;  86:     struct DeviceNode *dn;
;  87:     BSTR   ourname;
;  88: 
;  89:     /* NIPC/Services Mgr stuff */
;  90:     struct Entity *e, *re;
;  91:     struct DosList *vol;
;  92:     struct Transaction *t;
;  93:     struct TPacket *tp;
;  94:     ULONG sigbit=0;
       | 0008  7000                           MOVEQ       #00,D0
       | 000A  2B40 FFD4                      MOVE.L      D0,FFD4(A5)
;  95:     ULONG cetags[3]={ENT_AllocSignal,0,TAG_DONE};
       | 000E  41F9  0000 003A-01             LEA         01.0000003A,A0
       | 0014  43ED FFC8                      LEA         FFC8(A5),A1
       | 0018  22D8                           MOVE.L      (A0)+,(A1)+
       | 001A  22D8                           MOVE.L      (A0)+,(A1)+
       | 001C  22D8                           MOVE.L      (A0)+,(A1)+
;  96:     ULONG attags[3]={TRN_AllocReqBuffer,512,TAG_DONE};
       | 001E  41F9  0000 0046-01             LEA         01.00000046,A0
       | 0024  43ED FFBC                      LEA         FFBC(A5),A1
       | 0028  22D8                           MOVE.L      (A0)+,(A1)+
       | 002A  22D8                           MOVE.L      (A0)+,(A1)+
       | 002C  22D8                           MOVE.L      (A0)+,(A1)+
;  97: //    ULONG notags[1]={TAG_DONE};
;  98:     ULONG retrycount=0;
       | 002E  2C00                           MOVE.L      D0,D6
;  99:     UBYTE *startstring;
; 100: 
; 101:     UBYTE machine[80],rname[256],lname[80],user[80],password[80], flags[17];
; 102: 
; 103:     BOOL networkdown=FALSE;                 /* True if the network appears to be down at the moment */
       | 0030  7A00                           MOVEQ       #00,D5
; 104:     BOOL startupstatus=FALSE;
       | 0032  426D FD5E                      CLR.W       FD5E(A5)
; 105: //    ULONG cnum=0L;                          /* current connection #; merely distinguishes old data vs. new */
; 106:     ULONG mflags = 0L;
; 107:     struct MountedFSInfoClient *m;          /* General global structure describing this beast */
; 108:     struct FileSysStartupMsg *fssm;
; 109: 
; 110:     SysBase = (struct Library *) (*(void **)4L);
       | 0036  307C 0004                      MOVEA.W     #0004,A0
       | 003A  2250                           MOVEA.L     (A0),A1
       | 003C  23C9  0000 0000-02             MOVE.L      A1,02.00000000
; 111: 
; 112:     cetags[1] = (ULONG) &sigbit;
       | 0042  41ED FFD4                      LEA         FFD4(A5),A0
       | 0046  2B48 FFCC                      MOVE.L      A0,FFCC(A5)
; 113: 
; 114: 
; 115: 
; 116: /* Now, attempt to get everything allocated and initialized */
; 117:     DOSBase = (struct Library *) OpenLibrary("dos.library",0L);
       | 004A  2B40 FD5A                      MOVE.L      D0,FD5A(A5)
       | 004E  2C49                           MOVEA.L     A1,A6
       | 0050  43F9  0000 0052-01             LEA         01.00000052,A1
       | 0056  4EAE FDD8                      JSR         FDD8(A6)
       | 005A  23C0  0000 0004-02             MOVE.L      D0,02.00000004
; 118:     if (DOSBase)
       | 0060  6700 070A                      BEQ.W       076C
; 119:     {
; 120:         BOOL startupreturned=FALSE;
       | 0064  426D FD50                      CLR.W       FD50(A5)
; 121: 
; 122:     /* Get startup packet */
; 123:         localport = &((struct Process *)FindTask(0L))->pr_MsgPort;
       | 0068  93C9                           SUBA.L      A1,A1
       | 006A  4EAE FEDA                      JSR         FEDA(A6)
       | 006E  2040                           MOVEA.L     D0,A0
       | 0070  43E8 005C                      LEA         005C(A0),A1
       | 0074  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
; 124:         while (!(sp=(struct StandardPacket *)GetMsg(localport)))
       | 0078  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 007C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0082  4EAE FE8C                      JSR         FE8C(A6)
       | 0086  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 008A  4A80                           TST.L       D0
       | 008C  660A                           BNE.B       0098
; 125:             WaitPort(localport);
       | 008E  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0092  4EAE FE80                      JSR         FE80(A6)
       | 0096  60E0                           BRA.B       0078
; 126:         dp = (struct DosPacket *) sp->sp_Msg.mn_Node.ln_Name;
       | 0098  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 009C  2668 000A                      MOVEA.L     000A(A0),A3
; 127:         ourname = (BSTR) dp->dp_Arg1;
       | 00A0  2E2B 0014                      MOVE.L      0014(A3),D7
; 128:         dn = (struct DeviceNode *) BADDR(dp->dp_Arg3);
       | 00A4  202B 001C                      MOVE.L      001C(A3),D0
       | 00A8  E580                           ASL.L       #2,D0
       | 00AA  2440                           MOVEA.L     D0,A2
; 129:         fssm = (struct FileSysStartupMsg *) BADDR(dn->dn_Startup);
       | 00AC  202A 001C                      MOVE.L      001C(A2),D0
       | 00B0  E580                           ASL.L       #2,D0
; 130:         dp->dp_Res1 = DOSFALSE; /* Default to handler not coming up at all */
       | 00B2  42AB 000C                      CLR.L       000C(A3)
; 131: 
; 132:         startstring = (UBYTE *) fssm->fssm_Unit;
       | 00B6  2240                           MOVEA.L     D0,A1
       | 00B8  2051                           MOVEA.L     (A1),A0
; 133:         if (startstring=clpcpy(machine,startstring,80))
       | 00BA  4878 0050                      PEA         0050
       | 00BE  2F08                           MOVE.L      A0,-(A7)
       | 00C0  486D FF64                      PEA         FF64(A5)
       | 00C4  2B40 FD52                      MOVE.L      D0,FD52(A5)
       | 00C8  2B48 FFB4                      MOVE.L      A0,FFB4(A5)
       | 00CC  4EBA  0000-XX.1                JSR         _clpcpy(PC)
       | 00D0  4FEF 000C                      LEA         000C(A7),A7
       | 00D4  2B40 FFB4                      MOVE.L      D0,FFB4(A5)
       | 00D8  6700 00FE                      BEQ.W       01D8
; 134:         {
; 135:             if (startstring=clpcpy(rname,startstring,80))
       | 00DC  4878 0050                      PEA         0050
       | 00E0  2F00                           MOVE.L      D0,-(A7)
       | 00E2  486D FE64                      PEA         FE64(A5)
       | 00E6  4EBA  0000-XX.1                JSR         _clpcpy(PC)
       | 00EA  4FEF 000C                      LEA         000C(A7),A7
       | 00EE  2B40 FFB4                      MOVE.L      D0,FFB4(A5)
       | 00F2  6700 00E4                      BEQ.W       01D8
; 136:             {
; 137:                 if (startstring=clpcpy(user,startstring,80))
       | 00F6  4878 0050                      PEA         0050
       | 00FA  2F00                           MOVE.L      D0,-(A7)
       | 00FC  486D FDC4                      PEA         FDC4(A5)
       | 0100  4EBA  0000-XX.1                JSR         _clpcpy(PC)
       | 0104  4FEF 000C                      LEA         000C(A7),A7
       | 0108  2B40 FFB4                      MOVE.L      D0,FFB4(A5)
       | 010C  6700 00CA                      BEQ.W       01D8
; 138:                 {
; 139:                     if (startstring=clpcpy(password,startstring,80))
       | 0110  4878 0050                      PEA         0050
       | 0114  2F00                           MOVE.L      D0,-(A7)
       | 0116  486D FD74                      PEA         FD74(A5)
       | 011A  4EBA  0000-XX.1                JSR         _clpcpy(PC)
       | 011E  4FEF 000C                      LEA         000C(A7),A7
       | 0122  2B40 FFB4                      MOVE.L      D0,FFB4(A5)
       | 0126  6700 00B0                      BEQ.W       01D8
; 140:                     {
; 141:                         UBYTE *gptmp;
; 142:                         int x;
; 143:                         gptmp = (UBYTE *) BADDR(dn->dn_Name);
       | 012A  202A 0028                      MOVE.L      0028(A2),D0
       | 012E  E580                           ASL.L       #2,D0
; 144:                         if (gptmp[0] < 80)
       | 0130  48ED 0001 FD4C                 MOVEM.L     D0,FD4C(A5)
       | 0136  2040                           MOVEA.L     D0,A0
       | 0138  1210                           MOVE.B      (A0),D1
       | 013A  7450                           MOVEQ       #50,D2
       | 013C  B202                           CMP.B       D2,D1
       | 013E  642E                           BCC.B       016E
; 145:                         {
; 146:                             memcpy(lname,&gptmp[1],gptmp[0]);
       | 0140  43E8 0001                      LEA         0001(A0),A1
       | 0144  7000                           MOVEQ       #00,D0
       | 0146  1001                           MOVE.B      D1,D0
       | 0148  2F00                           MOVE.L      D0,-(A7)
       | 014A  2F09                           MOVE.L      A1,-(A7)
       | 014C  486D FE14                      PEA         FE14(A5)
       | 0150  4EBA  0000-XX.1                JSR         _memcpy(PC)
       | 0154  4FEF 000C                      LEA         000C(A7),A7
; 147:                             lname[gptmp[0]]=0;
       | 0158  7000                           MOVEQ       #00,D0
       | 015A  206D FD4C                      MOVEA.L     FD4C(A5),A0
       | 015E  1010                           MOVE.B      (A0),D0
       | 0160  41ED FE14                      LEA         FE14(A5),A0
       | 0164  D0C0                           ADDA.W      D0,A0
       | 0166  4210                           CLR.B       (A0)
; 148:                             startupstatus = TRUE;
       | 0168  3B7C 0001 FD5E                 MOVE.W      #0001,FD5E(A5)
; 149:                         }
; 150: 
; 151:                         startstring=clpcpy(flags,startstring,16);
       | 016E  4878 0010                      PEA         0010
       | 0172  2F2D FFB4                      MOVE.L      FFB4(A5),-(A7)
       | 0176  486D FD63                      PEA         FD63(A5)
       | 017A  4EBA  0000-XX.1                JSR         _clpcpy(PC)
       | 017E  4FEF 000C                      LEA         000C(A7),A7
; 152:                         for (x=0; x < strlen(flags); x++)
       | 0182  42AD FD48                      CLR.L       FD48(A5)
       | 0186  2B40 FFB4                      MOVE.L      D0,FFB4(A5)
       | 018A  486D FD63                      PEA         FD63(A5)
       | 018E  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0192  584F                           ADDQ.W      #4,A7
       | 0194  222D FD48                      MOVE.L      FD48(A5),D1
       | 0198  B280                           CMP.L       D0,D1
       | 019A  6C3C                           BGE.B       01D8
; 153:                         {
; 154:                             if ((flags[x] == 'B') || (flags[x] == 'b'))
       | 019C  41ED FD63                      LEA         FD63(A5),A0
       | 01A0  2248                           MOVEA.L     A0,A1
       | 01A2  D3C1                           ADDA.L      D1,A1
       | 01A4  7042                           MOVEQ       #42,D0
       | 01A6  B011                           CMP.B       (A1),D0
       | 01A8  670A                           BEQ.B       01B4
       | 01AA  2248                           MOVEA.L     A0,A1
       | 01AC  D3C1                           ADDA.L      D1,A1
       | 01AE  7062                           MOVEQ       #62,D0
       | 01B0  B011                           CMP.B       (A1),D0
       | 01B2  6606                           BNE.B       01BA
; 155:                                 mflags |= MFSIF_NOBACKDROP;
       | 01B4  08ED 0000 FD5D                 BSET        #0000,FD5D(A5)
; 156:                             if ((flags[x] == 'D') || (flags[x] == 'd'))
       | 01BA  2248                           MOVEA.L     A0,A1
       | 01BC  D3C1                           ADDA.L      D1,A1
       | 01BE  7044                           MOVEQ       #44,D0
       | 01C0  B011                           CMP.B       (A1),D0
       | 01C2  6708                           BEQ.B       01CC
       | 01C4  D1C1                           ADDA.L      D1,A0
       | 01C6  7064                           MOVEQ       #64,D0
       | 01C8  B010                           CMP.B       (A0),D0
       | 01CA  6606                           BNE.B       01D2
; 157:                                 mflags |= MFSIF_NODISKINFO;
       | 01CC  08ED 0001 FD5D                 BSET        #0001,FD5D(A5)
       | 01D2  52AD FD48                      ADDQ.L      #1,FD48(A5)
       | 01D6  60B2                           BRA.B       018A
; 158:                         }
; 159:                     }
; 160:                 }
; 161:             }
; 162:         }
; 163: 
; 164:         NIPCBase = (struct Library *) OpenLibrary("nipc.library",0L);
       | 01D8  43F9  0000 005E-01             LEA         01.0000005E,A1
       | 01DE  7000                           MOVEQ       #00,D0
       | 01E0  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 01E6  4EAE FDD8                      JSR         FDD8(A6)
       | 01EA  23C0  0000 0008-02             MOVE.L      D0,02.00000008
; 165:         if ((NIPCBase) && (startupstatus))
       | 01F0  6700 0526                      BEQ.W       0718
       | 01F4  4A6D FD5E                      TST.W       FD5E(A5)
       | 01F8  6700 051E                      BEQ.W       0718
; 166:         {
; 167:             ServicesBase = (struct Library *) OpenLibrary("services.library",0L);
       | 01FC  43F9  0000 006C-01             LEA         01.0000006C,A1
       | 0202  7000                           MOVEQ       #00,D0
       | 0204  4EAE FDD8                      JSR         FDD8(A6)
       | 0208  23C0  0000 000C-02             MOVE.L      D0,02.0000000C
; 168:             if (ServicesBase)
       | 020E  6700 04F8                      BEQ.W       0708
; 169:             {
; 170:                 e = (struct Entity *) CreateEntityA((struct TagItem *) &cetags);
       | 0212  41ED FFC8                      LEA         FFC8(A5),A0
       | 0216  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 021C  4EAE FF82                      JSR         FF82(A6)
; 171:                 if (e)
       | 0220  2B40 FFE8                      MOVE.L      D0,FFE8(A5)
       | 0224  6700 04D2                      BEQ.W       06F8
; 172:                 {
; 173:                     t = (struct Transaction *) AllocTransactionA((struct TagItem *)&attags);
       | 0228  41ED FFBC                      LEA         FFBC(A5),A0
       | 022C  4EAE FF8E                      JSR         FF8E(A6)
; 174:                     if (t)
       | 0230  2B40 FFDC                      MOVE.L      D0,FFDC(A5)
       | 0234  6700 04A0                      BEQ.W       06D6
; 175:                     {
; 176:                         m = (struct MountedFSInfoClient *) AllocMem(sizeof(struct MountedFSInfoClient),MEMF_PUBLIC|MEMF_CLEAR);
       | 0238  704C                           MOVEQ       #4C,D0
       | 023A  D080                           ADD.L       D0,D0
       | 023C  223C 0001 0001                 MOVE.L      #00010001,D1
       | 0242  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0248  4EAE FF3A                      JSR         FF3A(A6)
; 177:                         if (m)
       | 024C  2B40 FD56                      MOVE.L      D0,FD56(A5)
       | 0250  4A80                           TST.L       D0
       | 0252  6700 0474                      BEQ.W       06C8
; 178:                         {
; 179:                             NewList( (struct List *) &m->mfsi_Locks);
       | 0256  2040                           MOVEA.L     D0,A0
       | 0258  D0FC 000C                      ADDA.W      #000C,A0
       | 025C  2F08                           MOVE.L      A0,-(A7)
       | 025E  4EBA  0000-XX.1                JSR         _NewList(PC)
; 180:                             NewList( (struct List *) &m->mfsi_FileHandles);
       | 0262  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0266  D0FC 0018                      ADDA.W      #0018,A0
       | 026A  2E88                           MOVE.L      A0,(A7)
       | 026C  4EBA  0000-XX.1                JSR         _NewList(PC)
; 181: 
; 182:                             m->mfsi_Source = e;
       | 0270  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0274  216D FFE8 0038                 MOVE.L      FFE8(A5),0038(A0)
; 183:                             m->mfsi_RemotePath = rname;
       | 027A  43ED FE64                      LEA         FE64(A5),A1
       | 027E  2149 0048                      MOVE.L      A1,0048(A0)
; 184:                             m->mfsi_LocalPath = lname;
       | 0282  43ED FE14                      LEA         FE14(A5),A1
       | 0286  2149 0044                      MOVE.L      A1,0044(A0)
; 185:                             m->mfsi_Machine = machine;
       | 028A  43ED FF64                      LEA         FF64(A5),A1
       | 028E  2149 0040                      MOVE.L      A1,0040(A0)
; 186:                             m->mfsi_UserName = user;
       | 0292  43ED FDC4                      LEA         FDC4(A5),A1
       | 0296  2149 004C                      MOVE.L      A1,004C(A0)
; 187:                             m->mfsi_Password = password;
       | 029A  43ED FD74                      LEA         FD74(A5),A1
       | 029E  2149 0050                      MOVE.L      A1,0050(A0)
; 188:                             m->mfsi_LocalPort = localport;
       | 02A2  216D FFFC 0030                 MOVE.L      FFFC(A5),0030(A0)
; 189:                             m->mfsi_Flags = mflags;
       | 02A8  216D FD5A 0054                 MOVE.L      FD5A(A5),0054(A0)
; 190: 
; 191: 
; 192:                             t->trans_ResponseData = t->trans_RequestData;
       | 02AE  226D FFDC                      MOVEA.L     FFDC(A5),A1
       | 02B2  2369 002A 0036                 MOVE.L      002A(A1),0036(A1)
; 193:                             tp = (struct TPacket *) t->trans_RequestData;
       | 02B8  2B69 002A FFD8                 MOVE.L      002A(A1),FFD8(A5)
; 194:                             t->trans_RespDataLength = t->trans_ReqDataLength;
       | 02BE  2369 002E 003A                 MOVE.L      002E(A1),003A(A1)
; 195:                             networkdown = TRUE;
       | 02C4  7A01                           MOVEQ       #01,D5
; 196: 
; 197:                             m->mfsi_Destination = Reconnect(t,m,1,0L);
       | 02C6  4297                           CLR.L       (A7)
       | 02C8  4878 0001                      PEA         0001
       | 02CC  2F08                           MOVE.L      A0,-(A7)
       | 02CE  2F09                           MOVE.L      A1,-(A7)
       | 02D0  6100 0952                      BSR.W       0C24
       | 02D4  4FEF 0010                      LEA         0010(A7),A7
       | 02D8  206D FD56                      MOVEA.L     FD56(A5),A0
       | 02DC  2140 003C                      MOVE.L      D0,003C(A0)
; 198:                             re = m->mfsi_Destination;
; 199:                             if (m->mfsi_Destination)
       | 02E0  48ED 0001 FFE4                 MOVEM.L     D0,FFE4(A5)
       | 02E6  6700 03C6                      BEQ.W       06AE
; 200:                             {
; 201:                                 re = m->mfsi_Destination;
; 202: 
; 203:                                 Forbid();
       | 02EA  2B40 FFE4                      MOVE.L      D0,FFE4(A5)
       | 02EE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 02F4  4EAE FF7C                      JSR         FF7C(A6)
; 204:                                 if (!dn->dn_Task)
       | 02F8  4AAA 0008                      TST.L       0008(A2)
       | 02FC  6600 038E                      BNE.W       068C
; 205:                                 {
; 206:                                     dn->dn_Task = localport;
       | 0300  256D FFFC 0008                 MOVE.L      FFFC(A5),0008(A2)
; 207:                                     Permit();
       | 0306  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 030C  4EAE FF76                      JSR         FF76(A6)
; 208:                                     ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
       | 0310  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 0314  4878 FFFF                      PEA         FFFF
       | 0318  2F0B                           MOVE.L      A3,-(A7)
       | 031A  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 209:                                     startupreturned = TRUE;
       | 031E  3B7C 0001 FD50                 MOVE.W      #0001,FD50(A5)
; 210: 
; 211:                                     if (!strlen(&m->mfsi_VolumeName))           /* If no vol name given, use default */
       | 0324  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0328  D0FC 0058                      ADDA.W      #0058,A0
       | 032C  2E88                           MOVE.L      A0,(A7)
       | 032E  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0332  4FEF 000C                      LEA         000C(A7),A7
       | 0336  4A80                           TST.L       D0
       | 0338  6614                           BNE.B       034E
; 212:                                         strcpy(&m->mfsi_VolumeName,lname);
       | 033A  206D FD56                      MOVEA.L     FD56(A5),A0
       | 033E  D0FC 0058                      ADDA.W      #0058,A0
       | 0342  486D FE14                      PEA         FE14(A5)
       | 0346  2F08                           MOVE.L      A0,-(A7)
       | 0348  4EBA  0000-XX.1                JSR         _strcpy(PC)
       | 034C  504F                           ADDQ.W      #8,A7
; 213: 
; 214:                                     vol = (struct DosList *) MakeDosEntry(&m->mfsi_VolumeName,DLT_VOLUME);
       | 034E  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0352  D0FC 0058                      ADDA.W      #0058,A0
       | 0356  4878 0002                      PEA         0002
       | 035A  2F08                           MOVE.L      A0,-(A7)
       | 035C  4EBA  0000-XX.1                JSR         _MakeDosEntry(PC)
       | 0360  504F                           ADDQ.W      #8,A7
; 215:                                     if (vol)
       | 0362  2B40 FFE0                      MOVE.L      D0,FFE0(A5)
       | 0366  4A80                           TST.L       D0
       | 0368  6700 0344                      BEQ.W       06AE
; 216:                                     {
; 217:                                         ((struct DeviceList *)vol)->dl_Task = localport;
       | 036C  2040                           MOVEA.L     D0,A0
       | 036E  216D FFFC 0008                 MOVE.L      FFFC(A5),0008(A0)
; 218:                                         ((struct DeviceList *)vol)->dl_DiskType = ((struct DosEnvec *)BADDR(fssm->fssm_Environ))->de_DosType;
       | 0374  226D FD52                      MOVEA.L     FD52(A5),A1
       | 0378  2229 0008                      MOVE.L      0008(A1),D1
       | 037C  E581                           ASL.L       #2,D1
       | 037E  2241                           MOVEA.L     D1,A1
       | 0380  2169 0040 0020                 MOVE.L      0040(A1),0020(A0)
; 219: 
; 220:                                         m->mfsi_Volume = vol;
       | 0386  226D FD56                      MOVEA.L     FD56(A5),A1
       | 038A  2340 0034                      MOVE.L      D0,0034(A1)
; 221: 
; 222: 
; 223:                                         AddDosEntry(vol);
       | 038E  2F00                           MOVE.L      D0,-(A7)
       | 0390  4EBA  0000-XX.1                JSR         _AddDosEntry(PC)
       | 0394  584F                           ADDQ.W      #4,A7
; 224: 
; 225:                                         while(TRUE)
; 226:                                         {
; 227:                                             struct Transaction *rt;
; 228:                                             ULONG failcode=0;
       | 0396  42AD FD48                      CLR.L       FD48(A5)
; 229: 
; 230:                                             /* DosPackets */
; 231:                                             sp = (struct StandardPacket *)GetMsg(localport);
       | 039A  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 039E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 03A4  4EAE FE8C                      JSR         FE8C(A6)
; 232:                                             if (sp)
       | 03A8  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 03AC  4A80                           TST.L       D0
       | 03AE  6700 020E                      BEQ.W       05BE
; 233:                                             {
; 234:                                                 struct Transaction *ot;
; 235:                                                 dp = (struct DosPacket *) sp->sp_Msg.mn_Node.ln_Name;
       | 03B2  2040                           MOVEA.L     D0,A0
       | 03B4  2668 000A                      MOVEA.L     000A(A0),A3
; 236:                                                 dp->dp_Link = &(sp->sp_Msg); /* In case they don't do it */
       | 03B8  2680                           MOVE.L      D0,(A3)
; 237:                                                 /*
; 238:                                                  * If the network is down, attempt to reconnect.  If this attempt
; 239:                                                  * fails, return the packet as failed.
; 240:                                                  */
; 241:                                                 if ((networkdown) && (dp->dp_Type != ACTION_IS_FILESYSTEM))      /* To keep Workbench happy */
       | 03BA  4A45                           TST.W       D5
       | 03BC  6700 00DE                      BEQ.W       049C
       | 03C0  0CAB 0000 0403 0008            CMPI.L      #00000403,0008(A3)
       | 03C8  6700 00D2                      BEQ.W       049C
; 242:                                                 {
; 243:                                                     if (retrycount > 1)
       | 03CC  7201                           MOVEQ       #01,D1
       | 03CE  BC81                           CMP.L       D1,D6
       | 03D0  6360                           BLS.B       0432
; 244:                                                     {
; 245:                                                         if (!NetDownRequest(m->mfsi_Machine,m->mfsi_LocalPath))
       | 03D2  226D FD56                      MOVEA.L     FD56(A5),A1
       | 03D6  2F29 0044                      MOVE.L      0044(A1),-(A7)
       | 03DA  2F29 0040                      MOVE.L      0040(A1),-(A7)
       | 03DE  6100 24A8                      BSR.W       2888
       | 03E2  504F                           ADDQ.W      #8,A7
       | 03E4  4A40                           TST.W       D0
       | 03E6  664A                           BNE.B       0432
; 246:                                                         {
; 247:                                                             ULONG failure,failresp;
; 248:                                                             failure = ERROR_NETWORK_FAILED;
       | 03E8  2B7C 0000 00DB FD40            MOVE.L      #000000DB,FD40(A5)
; 249:                                                             if (failcode == ENVOYERR_NORESOURCES)
       | 03F0  0CAD 0000 01F5 FD48            CMPI.L      #000001F5,FD48(A5)
       | 03F8  6606                           BNE.B       0400
; 250:                                                                 failure = ERROR_NO_FREE_STORE;
       | 03FA  7067                           MOVEQ       #67,D0
       | 03FC  2B40 FD40                      MOVE.L      D0,FD40(A5)
; 251:                                                             failresp = DOSFALSE;
       | 0400  42AD FD3C                      CLR.L       FD3C(A5)
; 252:                                                             if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE))
       | 0404  202B 0008                      MOVE.L      0008(A3),D0
       | 0408  7252                           MOVEQ       #52,D1
       | 040A  B081                           CMP.L       D1,D0
       | 040C  6706                           BEQ.B       0414
       | 040E  7257                           MOVEQ       #57,D1
       | 0410  B081                           CMP.L       D1,D0
       | 0412  6606                           BNE.B       041A
; 253:                                                                 failresp = DOSTRUE;
       | 0414  70FF                           MOVEQ       #FF,D0
       | 0416  2B40 FD3C                      MOVE.L      D0,FD3C(A5)
; 254:                                                             ReplyPkt(dp,failresp,failure);
       | 041A  2F2D FD40                      MOVE.L      FD40(A5),-(A7)
       | 041E  2F2D FD3C                      MOVE.L      FD3C(A5),-(A7)
       | 0422  2F0B                           MOVE.L      A3,-(A7)
       | 0424  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 0428  4FEF 000C                      LEA         000C(A7),A7
; 255:                                                             retrycount = 0L;
       | 042C  7C00                           MOVEQ       #00,D6
; 256:                                                             continue;
       | 042E  6000 FF66                      BRA.W       0396
; 257:                                                         }
; 258:                                                     }
; 259:                                                     if (!(m->mfsi_Destination = Reconnect(t,m,(m->mfsi_Destination != 0),&failcode)))
       | 0432  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0436  4AA8 003C                      TST.L       003C(A0)
       | 043A  56C0                           SNE         D0
       | 043C  4400                           NEG.B       D0
       | 043E  4880                           EXT.W       D0
       | 0440  48C0                           EXT.L       D0
       | 0442  486D FD48                      PEA         FD48(A5)
       | 0446  2F00                           MOVE.L      D0,-(A7)
       | 0448  2F08                           MOVE.L      A0,-(A7)
       | 044A  2F2D FFDC                      MOVE.L      FFDC(A5),-(A7)
       | 044E  6100 07D4                      BSR.W       0C24
       | 0452  4FEF 0010                      LEA         0010(A7),A7
       | 0456  206D FD56                      MOVEA.L     FD56(A5),A0
       | 045A  2140 003C                      MOVE.L      D0,003C(A0)
       | 045E  662A                           BNE.B       048A
; 260:                                                     {
; 261:                                                         re = m->mfsi_Destination;
; 262:                                                         networkdown = TRUE;
       | 0460  7A01                           MOVEQ       #01,D5
; 263:                                                         /* put it at the head of the msgport */
; 264:                                                         Disable();
       | 0462  2B40 FFE4                      MOVE.L      D0,FFE4(A5)
       | 0466  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 046C  4EAE FF88                      JSR         FF88(A6)
; 265:                                                         AddHead(&(localport->mp_MsgList),(struct Node *)sp);
       | 0470  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0474  D0FC 0014                      ADDA.W      #0014,A0
       | 0478  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 047C  4EAE FF10                      JSR         FF10(A6)
; 266:                                                         Enable();
       | 0480  4EAE FF82                      JSR         FF82(A6)
; 267:                                                         retrycount++;
       | 0484  5286                           ADDQ.L      #1,D6
; 268:                                                         continue;
       | 0486  6000 FF0E                      BRA.W       0396
; 269:                                                     }
; 270:                                                     else
; 271:                                                     {
; 272:                                                         failcode = 0;
       | 048A  7000                           MOVEQ       #00,D0
       | 048C  2B40 FD48                      MOVE.L      D0,FD48(A5)
; 273:                                                         re = m->mfsi_Destination;
       | 0490  2068 003C                      MOVEA.L     003C(A0),A0
; 274:                                                         retrycount = 0L;
       | 0494  2C00                           MOVE.L      D0,D6
; 275:                                                         networkdown = FALSE;
       | 0496  7A00                           MOVEQ       #00,D5
; 276:                                                         re = m->mfsi_Destination;
; 277:                                                     }
       | 0498  2B48 FFE4                      MOVE.L      A0,FFE4(A5)
; 278:                                                 }
; 279: 
; 280:                                                 ot = (struct Transaction *) AllocTransactionA((struct TagItem *)&attags);
       | 049C  41ED FFBC                      LEA         FFBC(A5),A0
       | 04A0  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 04A6  4EAE FF8E                      JSR         FF8E(A6)
; 281:                                                 if (!ot)
       | 04AA  2B40 FD44                      MOVE.L      D0,FD44(A5)
       | 04AE  6616                           BNE.B       04C6
; 282:                                                 {
; 283:                                                     ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
       | 04B0  4878 0067                      PEA         0067
       | 04B4  42A7                           CLR.L       -(A7)
       | 04B6  2F0B                           MOVE.L      A3,-(A7)
       | 04B8  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 04BC  4FEF 000C                      LEA         000C(A7),A7
; 284:                                                     retrycount =0L;
       | 04C0  7C00                           MOVEQ       #00,D6
; 285:                                                 }
; 286:                                                 else
       | 04C2  6000 00FA                      BRA.W       05BE
; 287:                                                 {
; 288:                                                     struct TPacket *otp;
; 289:                                                     otp = (struct TPacket *) ot->trans_RequestData;
       | 04C6  2240                           MOVEA.L     D0,A1
       | 04C8  2069 002A                      MOVEA.L     002A(A1),A0
; 290:                                                     otp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 04CC  2C6D FD56                      MOVEA.L     FD56(A5),A6
       | 04D0  20AE 0028                      MOVE.L      0028(A6),(A0)
; 291:                                                     otp->tp_DosPacket = dp;
       | 04D4  214B 0004                      MOVE.L      A3,0004(A0)
; 292:                                                     otp->tp_Type = dp->dp_Type;
       | 04D8  222B 0008                      MOVE.L      0008(A3),D1
       | 04DC  2141 0008                      MOVE.L      D1,0008(A0)
; 293:                                                     ot->trans_ResponseData = ot->trans_RequestData;
       | 04E0  2369 002A 0036                 MOVE.L      002A(A1),0036(A1)
; 294:                                                     ot->trans_RespDataLength = ot->trans_ReqDataLength;
       | 04E6  2229 002E                      MOVE.L      002E(A1),D1
       | 04EA  2341 003A                      MOVE.L      D1,003A(A1)
; 295:                                                     ot->trans_ReqDataActual = ot->trans_ReqDataLength;
       | 04EE  2369 002E 0032                 MOVE.L      002E(A1),0032(A1)
; 296:                                                     ot->trans_Timeout = PACKETTIMEOUT;
       | 04F4  337C 000A 0042                 MOVE.W      #000A,0042(A1)
; 297:                                                     ot->trans_Command = FSCMD_DOSPACKET;
       | 04FA  137C 0003 001C                 MOVE.B      #03,001C(A1)
; 298:                                                     /* Some Special case shit right here */
; 299:                                                     if ((dp->dp_Type == ACTION_READ) || (dp->dp_Type == ACTION_WRITE))
       | 0500  2B48 FD40                      MOVE.L      A0,FD40(A5)
       | 0504  202B 0008                      MOVE.L      0008(A3),D0
       | 0508  7252                           MOVEQ       #52,D1
       | 050A  B081                           CMP.L       D1,D0
       | 050C  6706                           BEQ.B       0514
       | 050E  7257                           MOVEQ       #57,D1
       | 0510  B081                           CMP.L       D1,D0
       | 0512  6654                           BNE.B       0568
; 300:                                                     {
; 301:                                                     /* Action_Read and Action_Write */
; 302:                                                         if (!DoStart(dp->dp_Type,ot,otp,m))
       | 0514  2F0E                           MOVE.L      A6,-(A7)
       | 0516  2F08                           MOVE.L      A0,-(A7)
       | 0518  2F09                           MOVE.L      A1,-(A7)
       | 051A  2F00                           MOVE.L      D0,-(A7)
       | 051C  6100 1EC8                      BSR.W       23E6
       | 0520  4FEF 0010                      LEA         0010(A7),A7
       | 0524  4A40                           TST.W       D0
       | 0526  6624                           BNE.B       054C
; 303:                                                         {
; 304:                                                             FreeTransaction(ot);
       | 0528  226D FD44                      MOVEA.L     FD44(A5),A1
       | 052C  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0532  4EAE FF88                      JSR         FF88(A6)
; 305:                                                             ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
       | 0536  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 053A  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 053E  2F0B                           MOVE.L      A3,-(A7)
       | 0540  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 0544  4FEF 000C                      LEA         000C(A7),A7
; 306:                                                             retrycount =0L;
       | 0548  7C00                           MOVEQ       #00,D6
; 307:                                                         }
; 308:                                                         else
       | 054A  600E                           BRA.B       055A
; 309:                                                         {
; 310:                                                             FreeTransaction(ot);
       | 054C  226D FD44                      MOVEA.L     FD44(A5),A1
       | 0550  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0556  4EAE FF88                      JSR         FF88(A6)
; 311:                                                         }
; 312:                                                         if (!m->mfsi_Destination)
       | 055A  206D FD56                      MOVEA.L     FD56(A5),A0
       | 055E  4AA8 003C                      TST.L       003C(A0)
       | 0562  665A                           BNE.B       05BE
; 313:                                                             networkdown = TRUE;
       | 0564  7A01                           MOVEQ       #01,D5
; 314:                                                     }
; 315:                                                     else
       | 0566  6056                           BRA.B       05BE
; 316:                                                     {
; 317:                                                         /* Everybody else */
; 318:                                                         if (DoStart(dp->dp_Type,ot,otp,m))
       | 0568  2F0E                           MOVE.L      A6,-(A7)
       | 056A  2F08                           MOVE.L      A0,-(A7)
       | 056C  2F09                           MOVE.L      A1,-(A7)
       | 056E  2F00                           MOVE.L      D0,-(A7)
       | 0570  6100 1E74                      BSR.W       23E6
       | 0574  4FEF 0010                      LEA         0010(A7),A7
       | 0578  4A40                           TST.W       D0
       | 057A  6720                           BEQ.B       059C
; 319:                                                         {
; 320:                                                             BeginTransaction(m->mfsi_Destination,e,ot);
       | 057C  2F0A                           MOVE.L      A2,-(A7)
       | 057E  206D FD56                      MOVEA.L     FD56(A5),A0
       | 0582  2068 003C                      MOVEA.L     003C(A0),A0
       | 0586  226D FFE8                      MOVEA.L     FFE8(A5),A1
       | 058A  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0590  246D FD44                      MOVEA.L     FD44(A5),A2
       | 0594  4EAE FF64                      JSR         FF64(A6)
       | 0598  245F                           MOVEA.L     (A7)+,A2
; 321:                                                         }
; 322:                                                         else
       | 059A  6022                           BRA.B       05BE
; 323:                                                         {
; 324:                                                             FreeTransaction(ot);
       | 059C  226D FD44                      MOVEA.L     FD44(A5),A1
       | 05A0  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 05A6  4EAE FF88                      JSR         FF88(A6)
; 325:                                                             ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
       | 05AA  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 05AE  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 05B2  2F0B                           MOVE.L      A3,-(A7)
       | 05B4  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 05B8  4FEF 000C                      LEA         000C(A7),A7
; 326:                                                             retrycount = 0L;
       | 05BC  7C00                           MOVEQ       #00,D6
; 327:                                                         }
; 328:                                                     }
; 329:                                                 }
; 330:                                             }
; 331: 
; 332:                                             /* Returning Transactions */
; 333:                                             rt = GetTransaction(e);
       | 05BE  206D FFE8                      MOVEA.L     FFE8(A5),A0
       | 05C2  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 05C8  4EAE FF5E                      JSR         FF5E(A6)
; 334:                                             if (rt)
       | 05CC  2B40 FD4C                      MOVE.L      D0,FD4C(A5)
       | 05D0  6700 0082                      BEQ.W       0654
; 335:                                             {
; 336:                                                 struct TPacket *rtp;
; 337:                                                 rtp = (struct TPacket *) rt->trans_RequestData;
       | 05D4  2240                           MOVEA.L     D0,A1
       | 05D6  2069 002A                      MOVEA.L     002A(A1),A0
; 338: 
; 339:                                                 if (rt->trans_Error)
       | 05DA  2B48 FD44                      MOVE.L      A0,FD44(A5)
       | 05DE  4AA9 001E                      TST.L       001E(A1)
       | 05E2  6750                           BEQ.B       0634
; 340:                                                 {
; 341:                                                     struct StandardPacket *ssp;
; 342:                                                     rtp = (struct TPacket *) rt->trans_RequestData;
; 343:                                                     ssp = (struct StandardPacket *) ((struct DosPacket *)rtp->tp_DosPacket)->dp_Link;
       | 05E4  2268 0004                      MOVEA.L     0004(A0),A1
       | 05E8  2B51 FD40                      MOVE.L      (A1),FD40(A5)
; 344:                                                     networkdown = TRUE;
       | 05EC  7A01                           MOVEQ       #01,D5
; 345:                                                     if  ((((rtp->tp_Type == ACTION_READ) || (rtp->tp_Type == ACTION_WRITE)) && (!rtp->tp_Arg4)) || ((rtp->tp_Type != ACTION_READ) && (rtp->tp_Type != ACTION_WRITE)) )
       | 05EE  2B48 FD44                      MOVE.L      A0,FD44(A5)
       | 05F2  2028 0008                      MOVE.L      0008(A0),D0
       | 05F6  7252                           MOVEQ       #52,D1
       | 05F8  B081                           CMP.L       D1,D0
       | 05FA  6706                           BEQ.B       0602
       | 05FC  7457                           MOVEQ       #57,D2
       | 05FE  B082                           CMP.L       D2,D0
       | 0600  6606                           BNE.B       0608
       | 0602  4AA8 0020                      TST.L       0020(A0)
       | 0606  670A                           BEQ.B       0612
       | 0608  B081                           CMP.L       D1,D0
       | 060A  673A                           BEQ.B       0646
       | 060C  7257                           MOVEQ       #57,D1
       | 060E  B081                           CMP.L       D1,D0
       | 0610  6734                           BEQ.B       0646
; 346:                                                     {
; 347:                                                         /* put it at the head of the msgport, yes - I know this is not a nice thing to do */
; 348:                                                         Disable();
       | 0612  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0618  4EAE FF88                      JSR         FF88(A6)
; 349:                                                         AddHead(&(localport->mp_MsgList),(struct Node *)ssp);
       | 061C  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0620  D0FC 0014                      ADDA.W      #0014,A0
       | 0624  226D FD40                      MOVEA.L     FD40(A5),A1
       | 0628  4EAE FF10                      JSR         FF10(A6)
; 350:                                                         Enable();
       | 062C  4EAE FF82                      JSR         FF82(A6)
; 351:                                                         retrycount++;
       | 0630  5286                           ADDQ.L      #1,D6
; 352:                                                     }
; 353:                                                 }
; 354:                                                 else
       | 0632  6012                           BRA.B       0646
; 355:                                                 {
; 356:                                                     DoEnd(rtp->tp_Type,rt,m);
       | 0634  2F2D FD56                      MOVE.L      FD56(A5),-(A7)
       | 0638  2F00                           MOVE.L      D0,-(A7)
       | 063A  2F28 0008                      MOVE.L      0008(A0),-(A7)
       | 063E  6100 20D4                      BSR.W       2714
       | 0642  4FEF 000C                      LEA         000C(A7),A7
; 357:                                                 }
; 358:                                                 FreeTransaction(rt);
       | 0646  226D FD4C                      MOVEA.L     FD4C(A5),A1
       | 064A  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0650  4EAE FF88                      JSR         FF88(A6)
; 359:                                             }
; 360: 
; 361:                                             if ((!sp) && (!rt))
       | 0654  4AAD FFF8                      TST.L       FFF8(A5)
       | 0658  6600 FD3C                      BNE.W       0396
       | 065C  4AAD FD4C                      TST.L       FD4C(A5)
       | 0660  6600 FD34                      BNE.W       0396
; 362:                                                 Wait( (1 << sigbit) | (1 << (localport->mp_SigBit)) );
       | 0664  7001                           MOVEQ       #01,D0
       | 0666  2200                           MOVE.L      D0,D1
       | 0668  242D FFD4                      MOVE.L      FFD4(A5),D2
       | 066C  E5A1                           ASL.L       D2,D1
       | 066E  7400                           MOVEQ       #00,D2
       | 0670  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0674  1428 000F                      MOVE.B      000F(A0),D2
       | 0678  E5A0                           ASL.L       D2,D0
       | 067A  8280                           OR.L        D0,D1
       | 067C  2001                           MOVE.L      D1,D0
       | 067E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0684  4EAE FEC2                      JSR         FEC2(A6)
       | 0688  6000 FD0C                      BRA.W       0396
; 363:                                         }
; 364:                                         RemDosEntry(vol);
       | 068C  3B7C 0001 FD50                 MOVE.W      #0001,FD50(A5)
       | 0692  2F2B 0010                      MOVE.L      0010(A3),-(A7)
; 365:                                         FreeDosEntry(vol);
       | 0696  4878 FFFF                      PEA         FFFF
       | 069A  2F0B                           MOVE.L      A3,-(A7)
       | 069C  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 366:                                     }
; 367:                                 }
; 368:                                 else
       | 06A0  4FEF 000C                      LEA         000C(A7),A7
; 369:                                 {
; 370:                                     startupreturned = TRUE;
; 371:                                     ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
; 372:                                     Permit();
       | 06A4  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 06AA  4EAE FF76                      JSR         FF76(A6)
; 373:                                 }
; 374:                             }
; 375:                             re = m->mfsi_Destination;
       | 06AE  206D FD56                      MOVEA.L     FD56(A5),A0
       | 06B2  2B68 003C FFE4                 MOVE.L      003C(A0),FFE4(A5)
; 376:                             FreeMem(m,sizeof(struct MountedFSInfoClient));
       | 06B8  2248                           MOVEA.L     A0,A1
       | 06BA  704C                           MOVEQ       #4C,D0
       | 06BC  D080                           ADD.L       D0,D0
       | 06BE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 06C4  4EAE FF2E                      JSR         FF2E(A6)
; 377:                         }
; 378:                         FreeTransaction(t);
       | 06C8  226D FFDC                      MOVEA.L     FFDC(A5),A1
       | 06CC  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 06D2  4EAE FF88                      JSR         FF88(A6)
; 379:                     }
; 380:                     if (re)
       | 06D6  4AAD FFE4                      TST.L       FFE4(A5)
       | 06DA  670E                           BEQ.B       06EA
; 381:                         LoseService(re);
       | 06DC  206D FFE4                      MOVEA.L     FFE4(A5),A0
       | 06E0  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 06E6  4EAE FFDC                      JSR         FFDC(A6)
; 382: 
; 383:                     DeleteEntity(e);
       | 06EA  206D FFE8                      MOVEA.L     FFE8(A5),A0
       | 06EE  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 06F4  4EAE FF7C                      JSR         FF7C(A6)
; 384:                 }
; 385:                 CloseLibrary(ServicesBase);
       | 06F8  2279  0000 000C-02             MOVEA.L     02.0000000C,A1
       | 06FE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0704  4EAE FE62                      JSR         FE62(A6)
; 386:             }
; 387:             CloseLibrary(NIPCBase);
       | 0708  2279  0000 0008-02             MOVEA.L     02.00000008,A1
       | 070E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0714  4EAE FE62                      JSR         FE62(A6)
; 388:         }
; 389:         if (!startupreturned)
       | 0718  4A6D FD50                      TST.W       FD50(A5)
       | 071C  663E                           BNE.B       075C
; 390:         {
; 391:             Forbid();
       | 071E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0724  4EAE FF7C                      JSR         FF7C(A6)
; 392:             if (dn->dn_Task)
       | 0728  4AAA 0008                      TST.L       0008(A2)
       | 072C  6714                           BEQ.B       0742
; 393:             {
; 394:                 ReplyPkt(dp,DOSTRUE,dp->dp_Res2);
       | 072E  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 0732  4878 FFFF                      PEA         FFFF
       | 0736  2F0B                           MOVE.L      A3,-(A7)
       | 0738  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 073C  4FEF 000C                      LEA         000C(A7),A7
; 395:             }
; 396:             else
       | 0740  6010                           BRA.B       0752
; 397:             {
; 398:                 ReplyPkt(dp,DOSFALSE,dp->dp_Res2);
       | 0742  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 0746  42A7                           CLR.L       -(A7)
       | 0748  2F0B                           MOVE.L      A3,-(A7)
       | 074A  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 074E  4FEF 000C                      LEA         000C(A7),A7
; 399:             }
; 400:             Permit();
       | 0752  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0758  4EAE FF76                      JSR         FF76(A6)
; 401:         }
; 402:         CloseLibrary(DOSBase);
       | 075C  2279  0000 0004-02             MOVEA.L     02.00000004,A1
       | 0762  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0768  4EAE FE62                      JSR         FE62(A6)
; 403:     }
; 404: }
       | 076C  4CDF 4CE4                      MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A6
       | 0770  4E5D                           UNLK        A5
       | 0772  4E75                           RTS
; 405: 
; 406: BOOL FixBString(UBYTE *cp)
; 407: {
       | 0774  4E55 FFFC                      LINK        A5,#FFFC
       | 0778  48E7 2322                      MOVEM.L     D2/D6-D7/A2/A6,-(A7)
; 408:     BOOL retval=FALSE;
       | 077C  7E00                           MOVEQ       #00,D7
; 409:     UBYTE x;
; 410:     for (x = 1; x <= cp[0]; x++)
       | 077E  7C01                           MOVEQ       #01,D6
       | 0780  206D 0008                      MOVEA.L     0008(A5),A0
       | 0784  1010                           MOVE.B      (A0),D0
       | 0786  BC00                           CMP.B       D0,D6
       | 0788  6236                           BHI.B       07C0
; 411:         if (cp[x]==':')
       | 078A  7200                           MOVEQ       #00,D1
       | 078C  1206                           MOVE.B      D6,D1
       | 078E  743A                           MOVEQ       #3A,D2
       | 0790  B430 1000                      CMP.B       00(A0,D1.W),D2
       | 0794  6626                           BNE.B       07BC
; 412:         {
; 413:             cp[0] -= x;
       | 0796  9D10                           SUB.B       D6,(A0)
; 414:             CopyMem(&cp[x+1],&cp[1],cp[0]);
       | 0798  7000                           MOVEQ       #00,D0
       | 079A  1006                           MOVE.B      D6,D0
       | 079C  2248                           MOVEA.L     A0,A1
       | 079E  D3C0                           ADDA.L      D0,A1
       | 07A0  45E9 0001                      LEA         0001(A1),A2
       | 07A4  43E8 0001                      LEA         0001(A0),A1
       | 07A8  7000                           MOVEQ       #00,D0
       | 07AA  1010                           MOVE.B      (A0),D0
       | 07AC  204A                           MOVEA.L     A2,A0
       | 07AE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 07B4  4EAE FD90                      JSR         FD90(A6)
; 415:             x = 0;
       | 07B8  7C00                           MOVEQ       #00,D6
; 416:             retval = TRUE;
       | 07BA  7E01                           MOVEQ       #01,D7
; 417:         }
       | 07BC  5206                           ADDQ.B      #1,D6
       | 07BE  60C0                           BRA.B       0780
; 418:     return(retval);
       | 07C0  2007                           MOVE.L      D7,D0
; 419: }
       | 07C2  4CDF 44C4                      MOVEM.L     (A7)+,D2/D6-D7/A2/A6
       | 07C6  4E5D                           UNLK        A5
       | 07C8  4E75                           RTS
; 420: 
; 421: void SaveFHName(struct CFH *fh,struct TPacket *tp)
; 422: {
       | 07CA  4E55 FFF4                      LINK        A5,#FFF4
       | 07CE  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 07D2  266D 0008                      MOVEA.L     0008(A5),A3
; 423:     UBYTE *n, *s;
; 424:     int l;
; 425:     n = &(((UBYTE *)tp)[sizeof(struct TPacket)]);
       | 07D6  206D 000C                      MOVEA.L     000C(A5),A0
       | 07DA  D0FC 0034                      ADDA.W      #0034,A0
       | 07DE  2448                           MOVEA.L     A0,A2
; 426:     l = strlen(n)+1;
       | 07E0  2F0A                           MOVE.L      A2,-(A7)
       | 07E2  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 07E6  584F                           ADDQ.W      #4,A7
       | 07E8  2E00                           MOVE.L      D0,D7
       | 07EA  5287                           ADDQ.L      #1,D7
; 427: 
; 428:     s = (UBYTE *) AllocMem(l,MEMF_CLEAR|MEMF_PUBLIC);
       | 07EC  2007                           MOVE.L      D7,D0
       | 07EE  223C 0001 0001                 MOVE.L      #00010001,D1
       | 07F4  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 07FA  4EAE FF3A                      JSR         FF3A(A6)
; 429:     if (s)
       | 07FE  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 0802  670E                           BEQ.B       0812
; 430:     {
; 431:         fh->CFH_FullServerPath = s;
       | 0804  2740 0014                      MOVE.L      D0,0014(A3)
; 432:         strcpy(s,n);
       | 0808  2F0A                           MOVE.L      A2,-(A7)
       | 080A  2F00                           MOVE.L      D0,-(A7)
       | 080C  4EBA  0000-XX.1                JSR         _strcpy(PC)
       | 0810  504F                           ADDQ.W      #8,A7
; 433:     }
; 434: }
       | 0812  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 0816  4E5D                           UNLK        A5
       | 0818  4E75                           RTS
; 435: 
; 436: void SaveName(struct CLock *cl,struct TPacket *tp)
; 437: {
       | 081A  4E55 FFF4                      LINK        A5,#FFF4
       | 081E  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 0822  266D 0008                      MOVEA.L     0008(A5),A3
; 438:     UBYTE *n, *s;
; 439:     int l;
; 440:     n = &(((UBYTE *)tp)[sizeof(struct TPacket)]);
       | 0826  206D 000C                      MOVEA.L     000C(A5),A0
       | 082A  D0FC 0034                      ADDA.W      #0034,A0
       | 082E  2448                           MOVEA.L     A0,A2
; 441:     l = strlen(n)+1;
       | 0830  2F0A                           MOVE.L      A2,-(A7)
       | 0832  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0836  584F                           ADDQ.W      #4,A7
       | 0838  2E00                           MOVE.L      D0,D7
       | 083A  5287                           ADDQ.L      #1,D7
; 442: 
; 443:     s = (UBYTE *) AllocMem(l,MEMF_CLEAR|MEMF_PUBLIC);
       | 083C  2007                           MOVE.L      D7,D0
       | 083E  223C 0001 0001                 MOVE.L      #00010001,D1
       | 0844  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 084A  4EAE FF3A                      JSR         FF3A(A6)
; 444:     if (s)
       | 084E  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 0852  670E                           BEQ.B       0862
; 445:     {
; 446:         cl->CLock_FullServerPath = s;
       | 0854  2740 001C                      MOVE.L      D0,001C(A3)
; 447:         strcpy(s,n);
       | 0858  2F0A                           MOVE.L      A2,-(A7)
       | 085A  2F00                           MOVE.L      D0,-(A7)
       | 085C  4EBA  0000-XX.1                JSR         _strcpy(PC)
       | 0860  504F                           ADDQ.W      #8,A7
; 448:     }
; 449: }
       | 0862  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 0866  4E5D                           UNLK        A5
       | 0868  4E75                           RTS
; 450: 
; 451: void KeepLock(APTR thelock,struct MountedFSInfoClient *m)
; 452: {
       | 086A  4E55 FFFC                      LINK        A5,#FFFC
       | 086E  48E7 0012                      MOVEM.L     A3/A6,-(A7)
       | 0872  266D 0008                      MOVEA.L     0008(A5),A3
; 453: 
; 454:     struct ResourcesUsed *ru;
; 455: 
; 456:     ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
       | 0876  7012                           MOVEQ       #12,D0
       | 0878  223C 0001 0001                 MOVE.L      #00010001,D1
       | 087E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0884  4EAE FF3A                      JSR         FF3A(A6)
; 457:     if (!ru)
       | 0888  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 088C  4A80                           TST.L       D0
       | 088E  671C                           BEQ.B       08AC
; 458:         return;
; 459:     ru->ru_Resource = thelock;
       | 0890  2040                           MOVEA.L     D0,A0
       | 0892  214B 000E                      MOVE.L      A3,000E(A0)
; 460:     AddTail((struct List *)&m->mfsi_Locks,(struct Node *)&ru->ru_link);
       | 0896  226D 000C                      MOVEA.L     000C(A5),A1
       | 089A  D2FC 000C                      ADDA.W      #000C,A1
       | 089E  2049                           MOVEA.L     A1,A0
       | 08A0  2240                           MOVEA.L     D0,A1
       | 08A2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 08A8  4EAE FF0A                      JSR         FF0A(A6)
; 461: }
       | 08AC  4CDF 4800                      MOVEM.L     (A7)+,A3/A6
       | 08B0  4E5D                           UNLK        A5
       | 08B2  4E75                           RTS
; 462: 
; 463: 
; 464: void KeepFH(APTR thefh,struct MountedFSInfoClient *m)
; 465: {
       | 08B4  4E55 FFFC                      LINK        A5,#FFFC
       | 08B8  48E7 0012                      MOVEM.L     A3/A6,-(A7)
       | 08BC  266D 0008                      MOVEA.L     0008(A5),A3
; 466: 
; 467:     struct ResourcesUsed *ru;
; 468: 
; 469:     ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
       | 08C0  7012                           MOVEQ       #12,D0
       | 08C2  223C 0001 0001                 MOVE.L      #00010001,D1
       | 08C8  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 08CE  4EAE FF3A                      JSR         FF3A(A6)
; 470:     if (!ru)
       | 08D2  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 08D6  4A80                           TST.L       D0
       | 08D8  671C                           BEQ.B       08F6
; 471:         return;
; 472:     ru->ru_Resource = thefh;
       | 08DA  2040                           MOVEA.L     D0,A0
       | 08DC  214B 000E                      MOVE.L      A3,000E(A0)
; 473:     AddTail((struct List *)&m->mfsi_FileHandles,(struct Node *)&ru->ru_link);
       | 08E0  226D 000C                      MOVEA.L     000C(A5),A1
       | 08E4  D2FC 0018                      ADDA.W      #0018,A1
       | 08E8  2049                           MOVEA.L     A1,A0
       | 08EA  2240                           MOVEA.L     D0,A1
       | 08EC  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 08F2  4EAE FF0A                      JSR         FF0A(A6)
; 474: }
       | 08F6  4CDF 4800                      MOVEM.L     (A7)+,A3/A6
       | 08FA  4E5D                           UNLK        A5
       | 08FC  4E75                           RTS
; 475: 
; 476: void NukeLock(APTR thelock,struct MountedFSInfoClient *m)
; 477: {
       | 08FE  4E55 FFFC                      LINK        A5,#FFFC
       | 0902  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 0906  266D 0008                      MOVEA.L     0008(A5),A3
       | 090A  246D 000C                      MOVEA.L     000C(A5),A2
; 478: 
; 479:     struct ResourcesUsed *r;
; 480:     r = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
       | 090E  2B6A 000C FFFC                 MOVE.L      000C(A2),FFFC(A5)
; 481:     while (r->ru_link.ln_Succ)
       | 0914  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0918  4A90                           TST.L       (A0)
       | 091A  672A                           BEQ.B       0946
; 482:     {
; 483:         if (r->ru_Resource == (APTR) thelock)
       | 091C  2268 000E                      MOVEA.L     000E(A0),A1
       | 0920  B3CB                           CMPA.L      A3,A1
       | 0922  6618                           BNE.B       093C
; 484:         {
; 485:             Remove((struct Node *)r);
       | 0924  2248                           MOVEA.L     A0,A1
       | 0926  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 092C  4EAE FF04                      JSR         FF04(A6)
; 486:             FreeMem(r,sizeof(struct ResourcesUsed));
       | 0930  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 0934  7012                           MOVEQ       #12,D0
       | 0936  4EAE FF2E                      JSR         FF2E(A6)
; 487:             return;
       | 093A  600A                           BRA.B       0946
; 488:         }
; 489:         r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
       | 093C  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0940  2B50 FFFC                      MOVE.L      (A0),FFFC(A5)
       | 0944  60CE                           BRA.B       0914
; 490:     }
; 491: }
       | 0946  4CDF 4C00                      MOVEM.L     (A7)+,A2-A3/A6
       | 094A  4E5D                           UNLK        A5
       | 094C  4E75                           RTS
; 492: 
; 493: void NukeFH(APTR thefh,struct MountedFSInfoClient *m)
; 494: {
       | 094E  4E55 FFFC                      LINK        A5,#FFFC
       | 0952  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 0956  266D 0008                      MOVEA.L     0008(A5),A3
       | 095A  246D 000C                      MOVEA.L     000C(A5),A2
; 495:     struct ResourcesUsed *r;
; 496: 
; 497:     r = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
       | 095E  2B6A 0018 FFFC                 MOVE.L      0018(A2),FFFC(A5)
; 498:     while (r->ru_link.ln_Succ)
       | 0964  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0968  4A90                           TST.L       (A0)
       | 096A  672A                           BEQ.B       0996
; 499:     {
; 500:         if (r->ru_Resource == (APTR) thefh)
       | 096C  2268 000E                      MOVEA.L     000E(A0),A1
       | 0970  B3CB                           CMPA.L      A3,A1
       | 0972  6618                           BNE.B       098C
; 501:         {
; 502:             Remove((struct Node *)r);
       | 0974  2248                           MOVEA.L     A0,A1
       | 0976  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 097C  4EAE FF04                      JSR         FF04(A6)
; 503:             FreeMem(r,sizeof(struct ResourcesUsed));
       | 0980  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 0984  7012                           MOVEQ       #12,D0
       | 0986  4EAE FF2E                      JSR         FF2E(A6)
; 504:             return;
       | 098A  600A                           BRA.B       0996
; 505:         }
; 506:         r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
       | 098C  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0990  2B50 FFFC                      MOVE.L      (A0),FFFC(A5)
       | 0994  60CE                           BRA.B       0964
; 507:     }
; 508: }
       | 0996  4CDF 4C00                      MOVEM.L     (A7)+,A2-A3/A6
       | 099A  4E5D                           UNLK        A5
       | 099C  4E75                           RTS
; 509: 
; 510: BOOL FixFH(struct Entity *re, struct Entity *e,struct CFH *c,struct Transaction *t,
; 511:            struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
; 512: {
       | 099E  4E55 FFF4                      LINK        A5,#FFF4
       | 09A2  48E7 2732                      MOVEM.L     D2/D5-D7/A2-A3/A6,-(A7)
       | 09A6  266D 0008                      MOVEA.L     0008(A5),A3
       | 09AA  246D 000C                      MOVEA.L     000C(A5),A2
       | 09AE  2E2D 001C                      MOVE.L      001C(A5),D7
; 513: 
; 514:  UBYTE *datax;
; 515:  int x;
; 516:  LONG oldtype;
; 517: 
; 518:     oldtype = tp->tp_Type;
       | 09B2  206D 0018                      MOVEA.L     0018(A5),A0
       | 09B6  2A28 0008                      MOVE.L      0008(A0),D5
; 519:    /* This FH is known to be created on a previous connection */
; 520: 
; 521:     datax = &((UBYTE *)tp)[sizeof(struct TPacket)];
       | 09BA  43E8 0034                      LEA         0034(A0),A1
       | 09BE  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
; 522: 
; 523:     while ( (ULONG) datax & 3)
       | 09C2  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 09C6  2200                           MOVE.L      D0,D1
       | 09C8  7403                           MOVEQ       #03,D2
       | 09CA  C282                           AND.L       D2,D1
       | 09CC  6706                           BEQ.B       09D4
; 524:         datax ++;
       | 09CE  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 09D2  60EE                           BRA.B       09C2
; 525: 
; 526:     t->trans_Command = FSCMD_DOSPACKET;
       | 09D4  206D 0014                      MOVEA.L     0014(A5),A0
       | 09D8  117C 0003 001C                 MOVE.B      #03,001C(A0)
; 527:     t->trans_Timeout = PACKETTIMEOUT;
       | 09DE  317C 000A 0042                 MOVE.W      #000A,0042(A0)
; 528:     if (c->CFH_Access == ACTION_FINDINPUT)
       | 09E4  226D 0010                      MOVEA.L     0010(A5),A1
       | 09E8  0CA9 0000 03ED 000C            CMPI.L      #000003ED,000C(A1)
       | 09F0  660E                           BNE.B       0A00
; 529:         tp->tp_Type = ACTION_FINDINPUT;
       | 09F2  226D 0018                      MOVEA.L     0018(A5),A1
       | 09F6  237C 0000 03ED 0008            MOVE.L      #000003ED,0008(A1)
; 530:     else
       | 09FE  600C                           BRA.B       0A0C
; 531:         tp->tp_Type = ACTION_FINDUPDATE;
       | 0A00  226D 0018                      MOVEA.L     0018(A5),A1
       | 0A04  237C 0000 03EC 0008            MOVE.L      #000003EC,0008(A1)
; 532:     tp->tp_Arg1 = 0L;
       | 0A0C  42A9 0014                      CLR.L       0014(A1)
; 533:     tp->tp_Arg2 = -1L;
       | 0A10  70FF                           MOVEQ       #FF,D0
       | 0A12  2340 0018                      MOVE.L      D0,0018(A1)
; 534:     tp->tp_Arg3= ((ULONG) datax - (ULONG)tp);
       | 0A16  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 0A1A  2209                           MOVE.L      A1,D1
       | 0A1C  9081                           SUB.L       D1,D0
       | 0A1E  2340 001C                      MOVE.L      D0,001C(A1)
; 535:     tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
       | 0A22  7003                           MOVEQ       #03,D0
       | 0A24  2C6D 0020                      MOVEA.L     0020(A5),A6
       | 0A28  C0AE 0054                      AND.L       0054(A6),D0
       | 0A2C  2340 0020                      MOVE.L      D0,0020(A1)
; 536:     tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 0A30  22AE 0028                      MOVE.L      0028(A6),(A1)
; 537:     x = strlen(c->CFH_FullServerPath);
       | 0A34  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 0A38  2F2E 0014                      MOVE.L      0014(A6),-(A7)
       | 0A3C  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0A40  2C00                           MOVE.L      D0,D6
; 538:     datax[0]=x;
       | 0A42  2006                           MOVE.L      D6,D0
       | 0A44  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0A48  1080                           MOVE.B      D0,(A0)
; 539:     CopyMem(c->CFH_FullServerPath,&datax[1],x);
       | 0A4A  43E8 0001                      LEA         0001(A0),A1
       | 0A4E  206D 0010                      MOVEA.L     0010(A5),A0
       | 0A52  2068 0014                      MOVEA.L     0014(A0),A0
       | 0A56  2006                           MOVE.L      D6,D0
       | 0A58  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0A5E  4EAE FD90                      JSR         FD90(A6)
; 540:     FixBString(datax);
       | 0A62  2EAD FFFC                      MOVE.L      FFFC(A5),(A7)
       | 0A66  6100 FD0C                      BSR.W       0774
       | 0A6A  584F                           ADDQ.W      #4,A7
; 541: 
; 542:     t->trans_ReqDataActual = t->trans_ReqDataLength;
       | 0A6C  206D 0014                      MOVEA.L     0014(A5),A0
       | 0A70  2168 002E 0032                 MOVE.L      002E(A0),0032(A0)
; 543:     DoTransaction(re,e,t);
       | 0A76  2F0A                           MOVE.L      A2,-(A7)
       | 0A78  224A                           MOVEA.L     A2,A1
       | 0A7A  2448                           MOVEA.L     A0,A2
       | 0A7C  204B                           MOVEA.L     A3,A0
       | 0A7E  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0A84  4EAE FF6A                      JSR         FF6A(A6)
       | 0A88  245F                           MOVEA.L     (A7)+,A2
; 544:     t->trans_Timeout = PACKETTIMEOUT;
       | 0A8A  700A                           MOVEQ       #0A,D0
       | 0A8C  206D 0014                      MOVEA.L     0014(A5),A0
       | 0A90  3140 0042                      MOVE.W      D0,0042(A0)
; 545:     tp->tp_Type = oldtype;
       | 0A94  226D 0018                      MOVEA.L     0018(A5),A1
       | 0A98  2345 0008                      MOVE.L      D5,0008(A1)
; 546:     if (!t->trans_Error)
       | 0A9C  4AA8 001E                      TST.L       001E(A0)
       | 0AA0  6660                           BNE.B       0B02
; 547:     {
; 548:         if (tp->tp_Res1)
       | 0AA2  2229 000C                      MOVE.L      000C(A1),D1
       | 0AA6  675E                           BEQ.B       0B06
; 549:         {
; 550:             c->CFH_ConnectionNumber = cnum;
       | 0AA8  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 0AAC  2D47 0010                      MOVE.L      D7,0010(A6)
; 551:             c->CFH_ServerFH = (struct FileLock *) tp->tp_Res1;
       | 0AB0  2C69 000C                      MOVEA.L     000C(A1),A6
       | 0AB4  226D 0010                      MOVEA.L     0010(A5),A1
       | 0AB8  234E 0004                      MOVE.L      A6,0004(A1)
; 552:             t->trans_Timeout = PACKETTIMEOUT;
       | 0ABC  3140 0042                      MOVE.W      D0,0042(A0)
; 553:             tp->tp_Type = ACTION_SEEK;
       | 0AC0  206D 0018                      MOVEA.L     0018(A5),A0
       | 0AC4  217C 0000 03F0 0008            MOVE.L      #000003F0,0008(A0)
; 554:             tp->tp_Arg1 = (LONG) c->CFH_ServerFH;
       | 0ACC  2169 0004 0014                 MOVE.L      0004(A1),0014(A0)
; 555:             tp->tp_Arg2 = c->CFH_Pos;
       | 0AD2  2169 0008 0018                 MOVE.L      0008(A1),0018(A0)
; 556:             tp->tp_Arg3 = OFFSET_BEGINNING;
       | 0AD8  70FF                           MOVEQ       #FF,D0
       | 0ADA  2140 001C                      MOVE.L      D0,001C(A0)
; 557:             tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 0ADE  206D 0020                      MOVEA.L     0020(A5),A0
       | 0AE2  226D 0018                      MOVEA.L     0018(A5),A1
       | 0AE6  22A8 0028                      MOVE.L      0028(A0),(A1)
; 558:             DoTransaction(re,e,t);
       | 0AEA  2F0A                           MOVE.L      A2,-(A7)
       | 0AEC  224A                           MOVEA.L     A2,A1
       | 0AEE  204B                           MOVEA.L     A3,A0
       | 0AF0  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0AF6  246D 0014                      MOVEA.L     0014(A5),A2
       | 0AFA  4EAE FF6A                      JSR         FF6A(A6)
       | 0AFE  245F                           MOVEA.L     (A7)+,A2
; 559:         }
; 560:     }
; 561:     else
       | 0B00  6004                           BRA.B       0B06
; 562:         return(FALSE);
       | 0B02  7000                           MOVEQ       #00,D0
       | 0B04  6002                           BRA.B       0B08
; 563:     return(TRUE);
       | 0B06  7001                           MOVEQ       #01,D0
; 564: 
; 565: }
       | 0B08  4CDF 4CE4                      MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A6
       | 0B0C  4E5D                           UNLK        A5
       | 0B0E  4E75                           RTS
; 566: 
; 567: 
; 568: BOOL FixLock(struct Entity *re, struct Entity *e,struct CLock *c,struct Transaction *t,
; 569:              struct TPacket *tp, ULONG cnum, struct MountedFSInfoClient *m)
; 570: {
       | 0B10  4E55 FFF4                      LINK        A5,#FFF4
       | 0B14  48E7 2732                      MOVEM.L     D2/D5-D7/A2-A3/A6,-(A7)
       | 0B18  266D 0008                      MOVEA.L     0008(A5),A3
       | 0B1C  246D 000C                      MOVEA.L     000C(A5),A2
       | 0B20  2E2D 001C                      MOVE.L      001C(A5),D7
; 571:     UBYTE *datax;
; 572:     int x;
; 573:     LONG oldtype;
; 574: 
; 575:     oldtype = tp->tp_Type;
       | 0B24  206D 0018                      MOVEA.L     0018(A5),A0
       | 0B28  2A28 0008                      MOVE.L      0008(A0),D5
; 576: 
; 577: /* This lock is known to be created on a previous connection */
; 578: 
; 579:     datax = &((UBYTE *)tp)[sizeof(struct TPacket)];
       | 0B2C  43E8 0034                      LEA         0034(A0),A1
       | 0B30  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
; 580: 
; 581:     while ( (ULONG) datax & 3)
       | 0B34  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 0B38  2200                           MOVE.L      D0,D1
       | 0B3A  7403                           MOVEQ       #03,D2
       | 0B3C  C282                           AND.L       D2,D1
       | 0B3E  6706                           BEQ.B       0B46
; 582:         datax++;
       | 0B40  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 0B44  60EE                           BRA.B       0B34
; 583: 
; 584:     t->trans_Command = FSCMD_DOSPACKET;
       | 0B46  206D 0014                      MOVEA.L     0014(A5),A0
       | 0B4A  117C 0003 001C                 MOVE.B      #03,001C(A0)
; 585:     tp->tp_Type = ACTION_LOCATE_OBJECT;
       | 0B50  7008                           MOVEQ       #08,D0
       | 0B52  226D 0018                      MOVEA.L     0018(A5),A1
       | 0B56  2340 0008                      MOVE.L      D0,0008(A1)
; 586: //    tp->tp_Arg1 = -1L;
; 587:     tp->tp_Arg1 = 0L;
       | 0B5A  42A9 0014                      CLR.L       0014(A1)
; 588:     tp->tp_Arg2 = ((ULONG)datax-(ULONG)tp);
       | 0B5E  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 0B62  2209                           MOVE.L      A1,D1
       | 0B64  9081                           SUB.L       D1,D0
       | 0B66  2340 0018                      MOVE.L      D0,0018(A1)
; 589:     tp->tp_Arg3 = c->CLock_FileLock.fl_Access;
       | 0B6A  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 0B6E  236E 0008 001C                 MOVE.L      0008(A6),001C(A1)
; 590:     tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
       | 0B74  7003                           MOVEQ       #03,D0
       | 0B76  2C6D 0020                      MOVEA.L     0020(A5),A6
       | 0B7A  C0AE 0054                      AND.L       0054(A6),D0
       | 0B7E  2340 0020                      MOVE.L      D0,0020(A1)
; 591:     tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 0B82  22AE 0028                      MOVE.L      0028(A6),(A1)
; 592:     x = strlen(c->CLock_FullServerPath);
       | 0B86  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 0B8A  2F2E 001C                      MOVE.L      001C(A6),-(A7)
       | 0B8E  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0B92  2C00                           MOVE.L      D0,D6
; 593:     datax[0]=x;
       | 0B94  2006                           MOVE.L      D6,D0
       | 0B96  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0B9A  1080                           MOVE.B      D0,(A0)
; 594:     CopyMem(c->CLock_FullServerPath,&datax[1],x);
       | 0B9C  43E8 0001                      LEA         0001(A0),A1
       | 0BA0  206D 0010                      MOVEA.L     0010(A5),A0
       | 0BA4  2068 001C                      MOVEA.L     001C(A0),A0
       | 0BA8  2006                           MOVE.L      D6,D0
       | 0BAA  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0BB0  4EAE FD90                      JSR         FD90(A6)
; 595:     FixBString(datax);
       | 0BB4  2EAD FFFC                      MOVE.L      FFFC(A5),(A7)
       | 0BB8  6100 FBBA                      BSR.W       0774
       | 0BBC  584F                           ADDQ.W      #4,A7
; 596: 
; 597:     t->trans_ReqDataActual = t->trans_ReqDataLength;
       | 0BBE  206D 0014                      MOVEA.L     0014(A5),A0
       | 0BC2  2168 002E 0032                 MOVE.L      002E(A0),0032(A0)
; 598:     DoTransaction(re,e,t);
       | 0BC8  2F0A                           MOVE.L      A2,-(A7)
       | 0BCA  224A                           MOVEA.L     A2,A1
       | 0BCC  2448                           MOVEA.L     A0,A2
       | 0BCE  204B                           MOVEA.L     A3,A0
       | 0BD0  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0BD6  4EAE FF6A                      JSR         FF6A(A6)
       | 0BDA  245F                           MOVEA.L     (A7)+,A2
; 599:     t->trans_Timeout = PACKETTIMEOUT;
       | 0BDC  206D 0014                      MOVEA.L     0014(A5),A0
       | 0BE0  317C 000A 0042                 MOVE.W      #000A,0042(A0)
; 600:     tp->tp_Type = oldtype;
       | 0BE6  226D 0018                      MOVEA.L     0018(A5),A1
       | 0BEA  2345 0008                      MOVE.L      D5,0008(A1)
; 601:     if (!t->trans_Error)
       | 0BEE  4AA8 001E                      TST.L       001E(A0)
       | 0BF2  6622                           BNE.B       0C16
; 602:     {
; 603:         if (tp->tp_Res1)
       | 0BF4  2029 000C                      MOVE.L      000C(A1),D0
       | 0BF8  6720                           BEQ.B       0C1A
; 604:         {
; 605:             c->CLock_ConnectionNumber = cnum;
       | 0BFA  206D 0010                      MOVEA.L     0010(A5),A0
       | 0BFE  2147 0018                      MOVE.L      D7,0018(A0)
; 606:             c->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;
       | 0C02  2069 000C                      MOVEA.L     000C(A1),A0
       | 0C06  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 0C0A  2D48 0014                      MOVE.L      A0,0014(A6)
; 607:             c->CLock_FileLock.fl_Key = tp->tp_Res1;
       | 0C0E  2D69 000C 0004                 MOVE.L      000C(A1),0004(A6)
; 608:         }
; 609:     }
; 610:     else
       | 0C14  6004                           BRA.B       0C1A
; 611:     {
; 612:         return(FALSE);
       | 0C16  7000                           MOVEQ       #00,D0
       | 0C18  6002                           BRA.B       0C1C
; 613:     }
; 614:     return(TRUE);
       | 0C1A  7001                           MOVEQ       #01,D0
; 615: }
       | 0C1C  4CDF 4CE4                      MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A6
       | 0C20  4E5D                           UNLK        A5
       | 0C22  4E75                           RTS
; 616: 
; 617: struct Entity *Reconnect(struct Transaction *t, struct MountedFSInfoClient *m, ULONG announce, ULONG *failcode)
; 618: {
       | 0C24  4E55 FFD4                      LINK        A5,#FFD4
       | 0C28  48E7 0332                      MOVEM.L     D6-D7/A2-A3/A6,-(A7)
       | 0C2C  266D 0008                      MOVEA.L     0008(A5),A3
       | 0C30  2E2D 0010                      MOVE.L      0010(A5),D7
       | 0C34  246D 0014                      MOVEA.L     0014(A5),A2
; 619:     ULONG notags[3]={FSVC_Error,0,TAG_DONE};
       | 0C38  41F9  0000 007E-01             LEA         01.0000007E,A0
       | 0C3E  43ED FFF4                      LEA         FFF4(A5),A1
       | 0C42  22D8                           MOVE.L      (A0)+,(A1)+
       | 0C44  22D8                           MOVE.L      (A0)+,(A1)+
       | 0C46  22D8                           MOVE.L      (A0)+,(A1)+
; 620:     struct TPacket *tp=t->trans_RequestData;
       | 0C48  2B6B 002A FFF0                 MOVE.L      002A(A3),FFF0(A5)
; 621:     struct Entity *e=m->mfsi_Source;
       | 0C4E  206D 000C                      MOVEA.L     000C(A5),A0
       | 0C52  2B68 0038 FFEC                 MOVE.L      0038(A0),FFEC(A5)
; 622:     struct Entity *re=m->mfsi_Destination;
       | 0C58  2068 003C                      MOVEA.L     003C(A0),A0
; 623:     UBYTE *gp;
; 624:     LONG old;
; 625:     struct ResourcesUsed *rr;
; 626:     ULONG ecode=0L;
       | 0C5C  42AD FFD8                      CLR.L       FFD8(A5)
; 627:     notags[1] = (ULONG) &ecode;
       | 0C60  43ED FFD8                      LEA         FFD8(A5),A1
       | 0C64  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
; 628: 
; 629:     old = tp->tp_Type;
       | 0C68  226D FFF0                      MOVEA.L     FFF0(A5),A1
       | 0C6C  2C29 0008                      MOVE.L      0008(A1),D6
; 630:     if (re)
       | 0C70  2B48 FFE8                      MOVE.L      A0,FFE8(A5)
       | 0C74  2008                           MOVE.L      A0,D0
       | 0C76  670A                           BEQ.B       0C82
; 631:     {
; 632:         LoseService(re);
       | 0C78  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 0C7E  4EAE FFDC                      JSR         FFDC(A6)
; 633:     }
; 634: 
; 635: retry:
; 636:     re = (struct Entity *) FindServiceA(m->mfsi_Machine,"Filesystem",e,(struct TagItem *)&notags);
       | 0C82  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 0C86  206D 000C                      MOVEA.L     000C(A5),A0
       | 0C8A  2068 0040                      MOVEA.L     0040(A0),A0
       | 0C8E  43F9  0000 008A-01             LEA         01.0000008A,A1
       | 0C94  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 0C9A  246D FFEC                      MOVEA.L     FFEC(A5),A2
       | 0C9E  47ED FFF4                      LEA         FFF4(A5),A3
       | 0CA2  4EAE FFE2                      JSR         FFE2(A6)
       | 0CA6  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
; 637:     if (!re)
       | 0CAA  2B40 FFE8                      MOVE.L      D0,FFE8(A5)
       | 0CAE  4A80                           TST.L       D0
       | 0CB0  662C                           BNE.B       0CDE
; 638:     {
; 639:         if (failcode)
       | 0CB2  220A                           MOVE.L      A2,D1
       | 0CB4  6704                           BEQ.B       0CBA
; 640:             *failcode = ecode;
       | 0CB6  24AD FFD8                      MOVE.L      FFD8(A5),(A2)
; 641: 
; 642:         if ((announce) && (announce+1))
       | 0CBA  4A87                           TST.L       D7
       | 0CBC  671A                           BEQ.B       0CD8
       | 0CBE  2207                           MOVE.L      D7,D1
       | 0CC0  5281                           ADDQ.L      #1,D1
       | 0CC2  4A81                           TST.L       D1
       | 0CC4  6712                           BEQ.B       0CD8
; 643:             if (CantConnectRequest(m->mfsi_Machine))
       | 0CC6  206D 000C                      MOVEA.L     000C(A5),A0
       | 0CCA  2F28 0040                      MOVE.L      0040(A0),-(A7)
       | 0CCE  6100 1D0C                      BSR.W       29DC
       | 0CD2  584F                           ADDQ.W      #4,A7
       | 0CD4  4A40                           TST.W       D0
       | 0CD6  66AA                           BNE.B       0C82
; 644:                 goto retry; /* Sorry.  I wasn't about to mess the design up just to rid myself of a 'goto' */
; 645:         return(0L);
       | 0CD8  7000                           MOVEQ       #00,D0
       | 0CDA  6000 01EC                      BRA.W       0EC8
; 646:     }
; 647:     else
; 648:     {
; 649:         /* Send a MOUNT packet to the remote server */
; 650:         t->trans_Command = FSCMD_MOUNT;
       | 0CDE  177C 0001 001C                 MOVE.B      #01,001C(A3)
; 651: 
; 652:         gp = (UBYTE *) t->trans_RequestData;
       | 0CE4  206B 002A                      MOVEA.L     002A(A3),A0
; 653:         strcpy(gp,m->mfsi_RemotePath);
       | 0CE8  226D 000C                      MOVEA.L     000C(A5),A1
       | 0CEC  2F29 0048                      MOVE.L      0048(A1),-(A7)
       | 0CF0  2F08                           MOVE.L      A0,-(A7)
       | 0CF2  2B48 FFE4                      MOVE.L      A0,FFE4(A5)
       | 0CF6  4EBA  0000-XX.1                JSR         _strcpy(PC)
; 654:         gp += strlen(m->mfsi_RemotePath)+1;
       | 0CFA  206D 000C                      MOVEA.L     000C(A5),A0
       | 0CFE  2EA8 0048                      MOVE.L      0048(A0),(A7)
       | 0D02  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0D06  5280                           ADDQ.L      #1,D0
       | 0D08  D1AD FFE4                      ADD.L       D0,FFE4(A5)
; 655:         strcpy(gp,m->mfsi_LocalPath);
       | 0D0C  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D10  2EA8 0044                      MOVE.L      0044(A0),(A7)
       | 0D14  2F2D FFE4                      MOVE.L      FFE4(A5),-(A7)
       | 0D18  4EBA  0000-XX.1                JSR         _strcpy(PC)
; 656:         gp += strlen(m->mfsi_LocalPath)+1;
       | 0D1C  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D20  2EA8 0044                      MOVE.L      0044(A0),(A7)
       | 0D24  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0D28  5280                           ADDQ.L      #1,D0
       | 0D2A  D1AD FFE4                      ADD.L       D0,FFE4(A5)
; 657:         strcpy(gp,m->mfsi_UserName);
       | 0D2E  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D32  2EA8 004C                      MOVE.L      004C(A0),(A7)
       | 0D36  2F2D FFE4                      MOVE.L      FFE4(A5),-(A7)
       | 0D3A  4EBA  0000-XX.1                JSR         _strcpy(PC)
; 658:         gp += strlen(m->mfsi_UserName)+1;
       | 0D3E  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D42  2EA8 004C                      MOVE.L      004C(A0),(A7)
       | 0D46  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0D4A  5280                           ADDQ.L      #1,D0
       | 0D4C  D1AD FFE4                      ADD.L       D0,FFE4(A5)
; 659:         strcpy(gp,m->mfsi_Password);
       | 0D50  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D54  2EA8 0050                      MOVE.L      0050(A0),(A7)
       | 0D58  2F2D FFE4                      MOVE.L      FFE4(A5),-(A7)
       | 0D5C  4EBA  0000-XX.1                JSR         _strcpy(PC)
; 660:         gp += strlen(m->mfsi_Password)+1;
       | 0D60  206D 000C                      MOVEA.L     000C(A5),A0
       | 0D64  2EA8 0050                      MOVE.L      0050(A0),(A7)
       | 0D68  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0D6C  5280                           ADDQ.L      #1,D0
       | 0D6E  D1AD FFE4                      ADD.L       D0,FFE4(A5)
; 661:         strcpy(gp,"NEW");
       | 0D72  4879  0000 0096-01             PEA         01.00000096
       | 0D78  2F2D FFE4                      MOVE.L      FFE4(A5),-(A7)
       | 0D7C  4EBA  0000-XX.1                JSR         _strcpy(PC)
; 662:         gp += strlen("NEW")+1;
       | 0D80  4879  0000 009A-01             PEA         01.0000009A
       | 0D86  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0D8A  4FEF 0020                      LEA         0020(A7),A7
       | 0D8E  5280                           ADDQ.L      #1,D0
       | 0D90  D1AD FFE4                      ADD.L       D0,FFE4(A5)
; 663: 
; 664:         t->trans_Timeout = PACKETTIMEOUT;
       | 0D94  377C 000A 0042                 MOVE.W      #000A,0042(A3)
; 665:         t->trans_ReqDataActual = t->trans_ReqDataLength;
       | 0D9A  202B 002E                      MOVE.L      002E(A3),D0
       | 0D9E  2740 0032                      MOVE.L      D0,0032(A3)
; 666:         t->trans_RespDataLength = t->trans_ReqDataLength;
       | 0DA2  276B 002E 003A                 MOVE.L      002E(A3),003A(A3)
; 667:         DoTransaction(re,e,t);
       | 0DA8  2F0A                           MOVE.L      A2,-(A7)
       | 0DAA  206D FFE8                      MOVEA.L     FFE8(A5),A0
       | 0DAE  226D FFEC                      MOVEA.L     FFEC(A5),A1
       | 0DB2  244B                           MOVEA.L     A3,A2
       | 0DB4  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0DBA  4EAE FF6A                      JSR         FF6A(A6)
       | 0DBE  245F                           MOVEA.L     (A7)+,A2
; 668:         t->trans_Command = FSCMD_DOSPACKET;
       | 0DC0  177C 0003 001C                 MOVE.B      #03,001C(A3)
; 669:         t->trans_Timeout = PACKETTIMEOUT;
       | 0DC6  377C 000A 0042                 MOVE.W      #000A,0042(A3)
; 670: 
; 671:         if (t->trans_Error)
       | 0DCC  202B 001E                      MOVE.L      001E(A3),D0
       | 0DD0  6736                           BEQ.B       0E08
; 672:         {
; 673:             if (announce)
       | 0DD2  4A87                           TST.L       D7
       | 0DD4  6716                           BEQ.B       0DEC
; 674:                 MountFailedRequest(m->mfsi_Machine, m->mfsi_LocalPath, t->trans_Error);
       | 0DD6  2F00                           MOVE.L      D0,-(A7)
       | 0DD8  206D 000C                      MOVEA.L     000C(A5),A0
       | 0DDC  2F28 0044                      MOVE.L      0044(A0),-(A7)
       | 0DE0  2F28 0040                      MOVE.L      0040(A0),-(A7)
       | 0DE4  6100 1B1C                      BSR.W       2902
       | 0DE8  4FEF 000C                      LEA         000C(A7),A7
; 675:             tp->tp_Type = old;
       | 0DEC  206D FFF0                      MOVEA.L     FFF0(A5),A0
       | 0DF0  2146 0008                      MOVE.L      D6,0008(A0)
; 676:             LoseService(re);
       | 0DF4  206D FFE8                      MOVEA.L     FFE8(A5),A0
       | 0DF8  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 0DFE  4EAE FFDC                      JSR         FFDC(A6)
; 677:             return(0L);
       | 0E02  7000                           MOVEQ       #00,D0
       | 0E04  6000 00C2                      BRA.W       0EC8
; 678:         }
; 679:         else
; 680:         {
; 681:             UBYTE *r;
; 682: 
; 683:             r = (UBYTE *) t->trans_ResponseData;
       | 0E08  206B 0036                      MOVEA.L     0036(A3),A0
; 684:             r = &r[4];
       | 0E0C  2B48 FFD4                      MOVE.L      A0,FFD4(A5)
       | 0E10  5888                           ADDQ.L      #4,A0
; 685:             strcpy(&m->mfsi_VolumeName[0],r);
       | 0E12  226D 000C                      MOVEA.L     000C(A5),A1
       | 0E16  D2FC 0058                      ADDA.W      #0058,A1
       | 0E1A  2F08                           MOVE.L      A0,-(A7)
       | 0E1C  2F09                           MOVE.L      A1,-(A7)
       | 0E1E  2B48 FFD4                      MOVE.L      A0,FFD4(A5)
       | 0E22  4EBA  0000-XX.1                JSR         _strcpy(PC)
       | 0E26  504F                           ADDQ.W      #8,A7
; 686:             tp->tp_Type = old;
       | 0E28  206D FFF0                      MOVEA.L     FFF0(A5),A0
       | 0E2C  2146 0008                      MOVE.L      D6,0008(A0)
; 687: 
; 688:             m->mfsi_ServerMFSI = (APTR) (((ULONG *)t->trans_ResponseData)[0]);
       | 0E30  206B 0036                      MOVEA.L     0036(A3),A0
       | 0E34  226D 000C                      MOVEA.L     000C(A5),A1
       | 0E38  2350 0028                      MOVE.L      (A0),0028(A1)
; 689: 
; 690:             m->mfsi_ConnectionNumber++;
       | 0E3C  52A9 002C                      ADDQ.L      #1,002C(A1)
; 691: 
; 692:             /* Re-establish all known FH's and Locks.  Luckily, the code already exists for
; 693:              * doing exactly this.  Just go through the list!
; 694:              */
; 695: 
; 696:             rr = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
       | 0E40  2B69 000C FFDC                 MOVE.L      000C(A1),FFDC(A5)
; 697:             while (rr->ru_link.ln_Succ)
       | 0E46  206D FFDC                      MOVEA.L     FFDC(A5),A0
       | 0E4A  4A90                           TST.L       (A0)
       | 0E4C  6734                           BEQ.B       0E82
; 698:             {
; 699:                 FixLock(re,m->mfsi_Source,(struct CLock *) BADDR(rr->ru_Resource),
       | 0E4E  2028 000E                      MOVE.L      000E(A0),D0
       | 0E52  E580                           ASL.L       #2,D0
; 700:                         t,tp,m->mfsi_ConnectionNumber,m);
       | 0E54  2F2D 000C                      MOVE.L      000C(A5),-(A7)
       | 0E58  226D 000C                      MOVEA.L     000C(A5),A1
       | 0E5C  2F29 002C                      MOVE.L      002C(A1),-(A7)
       | 0E60  2F2D FFF0                      MOVE.L      FFF0(A5),-(A7)
       | 0E64  2F0B                           MOVE.L      A3,-(A7)
       | 0E66  2F00                           MOVE.L      D0,-(A7)
       | 0E68  2F29 0038                      MOVE.L      0038(A1),-(A7)
       | 0E6C  2F2D FFE8                      MOVE.L      FFE8(A5),-(A7)
       | 0E70  6100 FC9E                      BSR.W       0B10
       | 0E74  4FEF 001C                      LEA         001C(A7),A7
; 701:                 rr = (struct ResourcesUsed *) rr->ru_link.ln_Succ;
       | 0E78  206D FFDC                      MOVEA.L     FFDC(A5),A0
       | 0E7C  2B50 FFDC                      MOVE.L      (A0),FFDC(A5)
       | 0E80  60C4                           BRA.B       0E46
; 702:             }
; 703: 
; 704:             rr = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
       | 0E82  206D 000C                      MOVEA.L     000C(A5),A0
       | 0E86  2B68 0018 FFDC                 MOVE.L      0018(A0),FFDC(A5)
; 705:             while (rr->ru_link.ln_Succ)
       | 0E8C  206D FFDC                      MOVEA.L     FFDC(A5),A0
       | 0E90  4A90                           TST.L       (A0)
       | 0E92  6730                           BEQ.B       0EC4
; 706:             {
; 707: 
; 708:                 FixFH(re,m->mfsi_Source,(struct CFH *) rr->ru_Resource,
; 709:                       t,tp,m->mfsi_ConnectionNumber,m);
       | 0E94  2F2D 000C                      MOVE.L      000C(A5),-(A7)
       | 0E98  226D 000C                      MOVEA.L     000C(A5),A1
       | 0E9C  2F29 002C                      MOVE.L      002C(A1),-(A7)
       | 0EA0  2F2D FFF0                      MOVE.L      FFF0(A5),-(A7)
       | 0EA4  2F0B                           MOVE.L      A3,-(A7)
       | 0EA6  2F28 000E                      MOVE.L      000E(A0),-(A7)
       | 0EAA  2F29 0038                      MOVE.L      0038(A1),-(A7)
       | 0EAE  2F2D FFE8                      MOVE.L      FFE8(A5),-(A7)
       | 0EB2  6100 FAEA                      BSR.W       099E
       | 0EB6  4FEF 001C                      LEA         001C(A7),A7
; 710:                 rr = (struct ResourcesUsed *) rr->ru_link.ln_Succ;
       | 0EBA  206D FFDC                      MOVEA.L     FFDC(A5),A0
       | 0EBE  2B50 FFDC                      MOVE.L      (A0),FFDC(A5)
       | 0EC2  60C8                           BRA.B       0E8C
; 711:             }
; 712: 
; 713:             return(re);
       | 0EC4  202D FFE8                      MOVE.L      FFE8(A5),D0
; 714:         }
; 715:     }
; 716: }
       | 0EC8  4CDF 4CC0                      MOVEM.L     (A7)+,D6-D7/A2-A3/A6
       | 0ECC  4E5D                           UNLK        A5
       | 0ECE  4E75                           RTS
; 717: 
; 718: 
; 719: 
; 720: /*
; 721:  * StartFind()
; 722:  *
; 723:  * The 'start' routine for all FINDxxxx packets; FINDINPUT, FINDOUTPUT, FINDUPDATE.
; 724:  *
; 725:  */
; 726: BOOL StartFIND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
; 727:                struct MountedFSInfoClient *m)
; 728: {
       | 0ED0  4E55 FFF8                      LINK        A5,#FFF8
       | 0ED4  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 0ED8  266D 0008                      MOVEA.L     0008(A5),A3
       | 0EDC  246D 000C                      MOVEA.L     000C(A5),A2
; 729:     struct CLock *oldlock;
; 730:     UBYTE *gp;
; 731: 
; 732:     /* There's a possibility that this FIND is depending on a Lock that was
; 733:      * created on a previous connection -- one that we may well not have
; 734:      * been rebuilt.  Check for this, and fail if true.
; 735:      * if the address of the lock is NULL, we're dealing with the special case of
; 736:      * the lock referencing the root of the mount.  These are always valid.
; 737:      */
; 738: 
; 739:     oldlock = BADDR(dp->dp_Arg2);
       | 0EE0  206D 0010                      MOVEA.L     0010(A5),A0
       | 0EE4  2028 0018                      MOVE.L      0018(A0),D0
       | 0EE8  E580                           ASL.L       #2,D0
; 740:     if (oldlock)
       | 0EEA  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 0EEE  672E                           BEQ.B       0F1E
; 741:     {
; 742:         if (oldlock->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 0EF0  2040                           MOVEA.L     D0,A0
       | 0EF2  2228 0018                      MOVE.L      0018(A0),D1
       | 0EF6  226D 0014                      MOVEA.L     0014(A5),A1
       | 0EFA  B2A9 002C                      CMP.L       002C(A1),D1
       | 0EFE  6416                           BCC.B       0F16
; 743:         {
; 744:             dp->dp_Res1 = DOSFALSE;
       | 0F00  7200                           MOVEQ       #00,D1
       | 0F02  226D 0010                      MOVEA.L     0010(A5),A1
       | 0F06  2341 000C                      MOVE.L      D1,000C(A1)
; 745:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 0F0A  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
; 746:             return(FALSE);
       | 0F12  7000                           MOVEQ       #00,D0
       | 0F14  6070                           BRA.B       0F86
; 747:         }
; 748:         tp->tp_Arg2 = (LONG) oldlock->CLock_ServerLock;
       | 0F16  2568 0014 0018                 MOVE.L      0014(A0),0018(A2)
; 749:     }
; 750:     else
       | 0F1C  6004                           BRA.B       0F22
; 751:         tp->tp_Arg2 = 0L;
       | 0F1E  42AA 0018                      CLR.L       0018(A2)
; 752: 
; 753:     gp = &((UBYTE *) (t->trans_RequestData))[sizeof(struct TPacket)];
       | 0F22  206B 002A                      MOVEA.L     002A(A3),A0
       | 0F26  D0FC 0034                      ADDA.W      #0034,A0
       | 0F2A  2B48 FFF8                      MOVE.L      A0,FFF8(A5)
; 754: 
; 755:     /* Bump the general purpose pointer up to longword alignment */
; 756:     while ( (ULONG) gp & 3L)
       | 0F2E  202D FFF8                      MOVE.L      FFF8(A5),D0
       | 0F32  2200                           MOVE.L      D0,D1
       | 0F34  7403                           MOVEQ       #03,D2
       | 0F36  C282                           AND.L       D2,D1
       | 0F38  6706                           BEQ.B       0F40
; 757:         gp++;
       | 0F3A  52AD FFF8                      ADDQ.L      #1,FFF8(A5)
       | 0F3E  60EE                           BRA.B       0F2E
; 758: 
; 759:     /* Tell the server the offset to get to the filename, from the start of the
; 760:      * TPacket */
; 761: 
; 762:     tp->tp_Arg3 = ((ULONG)gp - (ULONG) tp);
       | 0F40  202D FFF8                      MOVE.L      FFF8(A5),D0
       | 0F44  220A                           MOVE.L      A2,D1
       | 0F46  2400                           MOVE.L      D0,D2
       | 0F48  9481                           SUB.L       D1,D2
       | 0F4A  2542 001C                      MOVE.L      D2,001C(A2)
; 763: 
; 764:     tp->tp_Arg4 = (m->mfsi_Flags & FLAGSMASK);
       | 0F4E  7203                           MOVEQ       #03,D1
       | 0F50  206D 0014                      MOVEA.L     0014(A5),A0
       | 0F54  C2A8 0054                      AND.L       0054(A0),D1
       | 0F58  2541 0020                      MOVE.L      D1,0020(A2)
; 765: 
; 766:     /* Copy the BSTR over */
; 767:     CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
       | 0F5C  206D 0010                      MOVEA.L     0010(A5),A0
       | 0F60  2228 001C                      MOVE.L      001C(A0),D1
       | 0F64  E581                           ASL.L       #2,D1
       | 0F66  7400                           MOVEQ       #00,D2
       | 0F68  2041                           MOVEA.L     D1,A0
       | 0F6A  1410                           MOVE.B      (A0),D2
       | 0F6C  5282                           ADDQ.L      #1,D2
       | 0F6E  2240                           MOVEA.L     D0,A1
       | 0F70  2002                           MOVE.L      D2,D0
       | 0F72  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0F78  4EAE FD90                      JSR         FD90(A6)
; 768: 
; 769:     /* Fix the BSTR; yank the stuff up to the colon */
; 770:     FixBString(gp);
       | 0F7C  2F2D FFF8                      MOVE.L      FFF8(A5),-(A7)
       | 0F80  6100 F7F2                      BSR.W       0774
; 771: 
; 772:     return(TRUE);
       | 0F84  7001                           MOVEQ       #01,D0
; 773: }
       | 0F86  4CED 4C04 FFE8                 MOVEM.L     FFE8(A5),D2/A2-A3/A6
       | 0F8C  4E5D                           UNLK        A5
       | 0F8E  4E75                           RTS
; 774: 
; 775: 
; 776: /*
; 777:  * EndFind()
; 778:  *
; 779:  * The 'end' routine for all FINDxxxx packets; FINDINPUT, FINDOUTPUT, FINDUPDATE.
; 780:  * All Endxxxx routines are required to return the dospacket, one way or another.
; 781:  *
; 782:  */
; 783: void EndFind(struct TPacket *tp, struct MountedFSInfoClient *m)
; 784: {
       | 0F90  4E55 FFF8                      LINK        A5,#FFF8
       | 0F94  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 0F98  266D 0008                      MOVEA.L     0008(A5),A3
       | 0F9C  246D 000C                      MOVEA.L     000C(A5),A2
; 785:     struct CFH *cfile;
; 786:     struct DosPacket *dp=tp->tp_DosPacket;
       | 0FA0  2B6B 0004 FFF8                 MOVE.L      0004(A3),FFF8(A5)
; 787: 
; 788: 
; 789:     /* If successful, create a local filehandle */
; 790:     if (tp->tp_Res1)
       | 0FA6  4AAB 000C                      TST.L       000C(A3)
       | 0FAA  6700 0082                      BEQ.W       102E
; 791:     {
; 792:         cfile = (struct CFH *) AllocMem(sizeof(struct CFH),MEMF_PUBLIC);
       | 0FAE  7018                           MOVEQ       #18,D0
       | 0FB0  7201                           MOVEQ       #01,D1
       | 0FB2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0FB8  4EAE FF3A                      JSR         FF3A(A6)
; 793:         if (!cfile)
       | 0FBC  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 0FC0  4A80                           TST.L       D0
       | 0FC2  6610                           BNE.B       0FD4
; 794:         {
; 795:             ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
       | 0FC4  4878 0067                      PEA         0067
       | 0FC8  42A7                           CLR.L       -(A7)
       | 0FCA  2F2D FFF8                      MOVE.L      FFF8(A5),-(A7)
       | 0FCE  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 796:             return;
       | 0FD2  606A                           BRA.B       103E
; 797:         }
; 798:         SaveFHName(cfile,tp);       /* Grab the full pathname of this file */
       | 0FD4  2F0B                           MOVE.L      A3,-(A7)
       | 0FD6  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 0FDA  6100 F7EE                      BSR.W       07CA
; 799: 
; 800:         cfile->CFH_FH = (struct FileHandle *) BADDR(dp->dp_Arg1);
       | 0FDE  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 0FE2  2028 0014                      MOVE.L      0014(A0),D0
       | 0FE6  E580                           ASL.L       #2,D0
       | 0FE8  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0FEC  2080                           MOVE.L      D0,(A0)
; 801:         cfile->CFH_ServerFH = (APTR) tp->tp_Res1;
       | 0FEE  216B 000C 0004                 MOVE.L      000C(A3),0004(A0)
; 802:         cfile->CFH_Pos = 0L;
       | 0FF4  42A8 0008                      CLR.L       0008(A0)
; 803:         cfile->CFH_Access = dp->dp_Type;
       | 0FF8  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 0FFC  2169 0008 000C                 MOVE.L      0008(A1),000C(A0)
; 804:         cfile->CFH_ConnectionNumber = m->mfsi_ConnectionNumber;
       | 1002  216A 002C 0010                 MOVE.L      002C(A2),0010(A0)
; 805: 
; 806:         KeepFH(cfile,m);
       | 1008  2E8A                           MOVE.L      A2,(A7)
       | 100A  2F08                           MOVE.L      A0,-(A7)
       | 100C  6100 F8A6                      BSR.W       08B4
       | 1010  4FEF 000C                      LEA         000C(A7),A7
; 807: 
; 808:         ((struct FileHandle *)BADDR(dp->dp_Arg1))->fh_Type = m->mfsi_LocalPort;
       | 1014  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 1018  2028 0014                      MOVE.L      0014(A0),D0
       | 101C  E580                           ASL.L       #2,D0
       | 101E  2040                           MOVEA.L     D0,A0
       | 1020  216A 0030 0008                 MOVE.L      0030(A2),0008(A0)
; 809:         ((struct FileHandle *)BADDR(dp->dp_Arg1))->fh_Arg1 = (LONG) cfile;
       | 1026  2040                           MOVEA.L     D0,A0
       | 1028  216D FFFC 0024                 MOVE.L      FFFC(A5),0024(A0)
; 810:     }
; 811: 
; 812:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 102E  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 1032  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 1036  2F2D FFF8                      MOVE.L      FFF8(A5),-(A7)
       | 103A  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 813: 
; 814: }
       | 103E  4CED 4C00 FFEC                 MOVEM.L     FFEC(A5),A2-A3/A6
       | 1044  4E5D                           UNLK        A5
       | 1046  4E75                           RTS
; 815: 
; 816: 
; 817: /*
; 818:  * StartEND
; 819:  *
; 820:  */
; 821: BOOL StartEND(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
; 822:                struct MountedFSInfoClient *m)
; 823: {
       | 1048  4E55 FFF4                      LINK        A5,#FFF4
       | 104C  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 1050  266D 0008                      MOVEA.L     0008(A5),A3
       | 1054  246D 000C                      MOVEA.L     000C(A5),A2
; 824: 
; 825:     struct CFH *cf;
; 826:     UBYTE *l, *gp;
; 827: 
; 828:     /* The possibility that the filehandle referenced by this existed on a
; 829:      * previous connection, and couldn't be reestablished exists.
; 830:      * Thus, we check.
; 831:      */
; 832: 
; 833: 
; 834:     cf = (struct CFH *) dp->dp_Arg1;
       | 1058  206D 0010                      MOVEA.L     0010(A5),A0
       | 105C  2B68 0014 FFFC                 MOVE.L      0014(A0),FFFC(A5)
; 835: 
; 836:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1062  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1066  2028 0010                      MOVE.L      0010(A0),D0
       | 106A  226D 0014                      MOVEA.L     0014(A5),A1
       | 106E  B0A9 002C                      CMP.L       002C(A1),D0
       | 1072  6414                           BCC.B       1088
; 837:     {
; 838:         dp->dp_Res1 = DOSFALSE;
       | 1074  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 1078  42AE 000C                      CLR.L       000C(A6)
; 839:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 107C  2D7C 0000 00CD 0010            MOVE.L      #000000CD,0010(A6)
; 840:         return(FALSE);
       | 1084  7000                           MOVEQ       #00,D0
       | 1086  6042                           BRA.B       10CA
; 841:     }
; 842: 
; 843:     /* Free up the path string */
; 844:     l = (UBYTE *) cf->CFH_FullServerPath;
       | 1088  2C68 0014                      MOVEA.L     0014(A0),A6
; 845:     if (l)
       | 108C  2B4E FFF8                      MOVE.L      A6,FFF8(A5)
       | 1090  200E                           MOVE.L      A6,D0
       | 1092  6718                           BEQ.B       10AC
; 846:     {
; 847:         FreeMem(l,strlen(l)+1);
       | 1094  2F0E                           MOVE.L      A6,-(A7)
       | 1096  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 109A  584F                           ADDQ.W      #4,A7
       | 109C  5280                           ADDQ.L      #1,D0
       | 109E  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 10A2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 10A8  4EAE FF2E                      JSR         FF2E(A6)
; 848:     }
; 849: 
; 850:     /* Remove this fh from our lists */
; 851:     NukeFH(cf,m);
       | 10AC  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 10B0  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 10B4  6100 F898                      BSR.W       094E
; 852: 
; 853:     /* make the general purpose pointer point to directly past the TPacket */
; 854:     gp = &((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket *)];
       | 10B8  206B 002A                      MOVEA.L     002A(A3),A0
       | 10BC  5888                           ADDQ.L      #4,A0
; 855: 
; 856:     tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;
       | 10BE  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 10C2  2569 0004 0014                 MOVE.L      0004(A1),0014(A2)
; 857: 
; 858:     return(TRUE);
       | 10C8  7001                           MOVEQ       #01,D0
; 859: 
; 860: }
       | 10CA  4CED 4C00 FFE8                 MOVEM.L     FFE8(A5),A2-A3/A6
       | 10D0  4E5D                           UNLK        A5
       | 10D2  4E75                           RTS
; 861: 
; 862: /*
; 863:  * EndEnd, EndSeek, EndRename, EndSetProtect, EndSetDate, EndDeleteObject, EndSetComment
; 864:  *
; 865:  *  I love names like that.
; 866:  *
; 867:  */
; 868: 
; 869: void EndEnd(struct TPacket *tp, struct MountedFSInfoClient *m)
; 870: {
       | 10D4  4E55 FFFC                      LINK        A5,#FFFC
       | 10D8  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 10DC  266D 0008                      MOVEA.L     0008(A5),A3
; 871:     struct DosPacket *dp=tp->tp_DosPacket;
       | 10E0  246B 0004                      MOVEA.L     0004(A3),A2
; 872: 
; 873:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 10E4  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 10E8  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 10EC  2F0A                           MOVE.L      A2,-(A7)
       | 10EE  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 874: }
       | 10F2  4CED 0C00 FFF4                 MOVEM.L     FFF4(A5),A2-A3
       | 10F8  4E5D                           UNLK        A5
       | 10FA  4E75                           RTS
; 875: 
; 876: 
; 877: /*
; 878:  * EndSeek
; 879:  *
; 880:  *
; 881:  */
; 882: 
; 883: void EndSeek(struct TPacket *tp, struct MountedFSInfoClient *m)
; 884: {
       | 10FC  4E55 FFFC                      LINK        A5,#FFFC
       | 1100  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 1104  266D 0008                      MOVEA.L     0008(A5),A3
; 885:     struct DosPacket *dp=tp->tp_DosPacket;
       | 1108  246B 0004                      MOVEA.L     0004(A3),A2
; 886: 
; 887:     switch (dp->dp_Arg3)
       | 110C  202A 001C                      MOVE.L      001C(A2),D0
       | 1110  5280                           ADDQ.L      #1,D0
       | 1112  670A                           BEQ.B       111E
       | 1114  5380                           SUBQ.L      #1,D0
       | 1116  6712                           BEQ.B       112A
       | 1118  5380                           SUBQ.L      #1,D0
       | 111A  671A                           BEQ.B       1136
       | 111C  6018                           BRA.B       1136
; 888:     {
; 889:         case OFFSET_BEGINNING:
; 890:             ((struct CFH *) dp->dp_Arg1)->CFH_Pos = dp->dp_Arg2;
       | 111E  206A 0014                      MOVEA.L     0014(A2),A0
       | 1122  216A 0018 0008                 MOVE.L      0018(A2),0008(A0)
; 891:             break;
       | 1128  600C                           BRA.B       1136
; 892: 
; 893:         case OFFSET_CURRENT:
; 894:             ((struct CFH *) dp->dp_Arg1)->CFH_Pos += dp->dp_Arg2;
       | 112A  206A 0014                      MOVEA.L     0014(A2),A0
       | 112E  202A 0018                      MOVE.L      0018(A2),D0
       | 1132  D1A8 0008                      ADD.L       D0,0008(A0)
; 895:             break;
; 896: 
; 897:         case OFFSET_END:
; 898:             /* FIXME!  CUrrently broken.  Needs to insert an ACTION_SEEK from
; 899:              * OFFSET_BEGINNING w/ offset=0 (specially flagged) in here to find
; 900:              * out where the position is after something like this.
; 901:              */
; 902:             break;
; 903: 
; 904:     }
; 905: 
; 906:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 1136  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 113A  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 113E  2F0A                           MOVE.L      A2,-(A7)
       | 1140  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
; 907: }
       | 1144  4CED 0C00 FFF4                 MOVEM.L     FFF4(A5),A2-A3
       | 114A  4E5D                           UNLK        A5
       | 114C  4E75                           RTS
; 908: 
; 909: 
; 910: 
; 911: 
; 912: /* StartSeek
; 913:  *
; 914:  * ACTION_SEEK
; 915:  *
; 916:  */
; 917: BOOL StartSeek(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
; 918:                struct MountedFSInfoClient *m)
; 919: {
       | 114E  4E55 FFFC                      LINK        A5,#FFFC
       | 1152  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 1156  266D 000C                      MOVEA.L     000C(A5),A3
       | 115A  246D 0010                      MOVEA.L     0010(A5),A2
; 920: 
; 921:     struct CFH *cf;
; 922: 
; 923:     cf = (struct CFH *) dp->dp_Arg1;
       | 115E  2B6A 0014 FFFC                 MOVE.L      0014(A2),FFFC(A5)
; 924: 
; 925:     /* The possibility of an old, unreincarnated fh exists. */
; 926: 
; 927:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1164  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1168  2028 0010                      MOVE.L      0010(A0),D0
       | 116C  226D 0014                      MOVEA.L     0014(A5),A1
       | 1170  B0A9 002C                      CMP.L       002C(A1),D0
       | 1174  6410                           BCC.B       1186
; 928:     {
; 929:         dp->dp_Res1 = DOSFALSE;
       | 1176  42AA 000C                      CLR.L       000C(A2)
; 930:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 117A  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
; 931:         return(FALSE);
       | 1182  7000                           MOVEQ       #00,D0
       | 1184  6014                           BRA.B       119A
; 932:     }
; 933: 
; 934:     tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;
       | 1186  2768 0004 0014                 MOVE.L      0004(A0),0014(A3)
; 935:     tp->tp_Arg2 = dp->dp_Arg2;
       | 118C  276A 0018 0018                 MOVE.L      0018(A2),0018(A3)
; 936:     tp->tp_Arg3 = dp->dp_Arg3;
       | 1192  276A 001C 001C                 MOVE.L      001C(A2),001C(A3)
; 937: 
; 938:     return(TRUE);
       | 1198  7001                           MOVEQ       #01,D0
; 939: }
       | 119A  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 119E  4E5D                           UNLK        A5
       | 11A0  4E75                           RTS
; 940: 
; 941: 
; 942: /* StartRead
; 943:  *
; 944:  * ACTION_READ
; 945:  *
; 946:  */
; 947: BOOL StartRead(struct Transaction *x, struct TPacket *xp, struct DosPacket *dp,
; 948:                struct MountedFSInfoClient *m)
; 949: {
       | 11A2  4E55 FFD8                      LINK        A5,#FFD8
       | 11A6  48E7 0332                      MOVEM.L     D6-D7/A2-A3/A6,-(A7)
       | 11AA  266D 0008                      MOVEA.L     0008(A5),A3
       | 11AE  246D 0010                      MOVEA.L     0010(A5),A2
; 950: 
; 951:     struct CFH *cf;
; 952:     ULONG remaining, thispacketsize;
; 953: 
; 954:     cf = (struct CFH *) dp->dp_Arg1;
       | 11B2  2B6A 0014 FFFC                 MOVE.L      0014(A2),FFFC(A5)
; 955: 
; 956:     /* The possibility of an old, unreincarnated fh exists. */
; 957: 
; 958:     if (!dp->dp_Arg3)
       | 11B8  202A 001C                      MOVE.L      001C(A2),D0
       | 11BC  6610                           BNE.B       11CE
; 959:     {
; 960:         dp->dp_Res1 = 0L;
       | 11BE  7200                           MOVEQ       #00,D1
       | 11C0  2541 000C                      MOVE.L      D1,000C(A2)
; 961:         dp->dp_Res2 = 0L;
       | 11C4  2541 0010                      MOVE.L      D1,0010(A2)
; 962:         return(FALSE);
       | 11C8  7000                           MOVEQ       #00,D0
       | 11CA  6000 0118                      BRA.W       12E4
; 963:     }
; 964: 
; 965:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 11CE  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 11D2  2028 0010                      MOVE.L      0010(A0),D0
       | 11D6  206D 0014                      MOVEA.L     0014(A5),A0
       | 11DA  B0A8 002C                      CMP.L       002C(A0),D0
       | 11DE  6414                           BCC.B       11F4
; 966:     {
; 967:         dp->dp_Res1 = DOSTRUE;
       | 11E0  70FF                           MOVEQ       #FF,D0
       | 11E2  2540 000C                      MOVE.L      D0,000C(A2)
; 968:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 11E6  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
; 969:         return(FALSE);
       | 11EE  7000                           MOVEQ       #00,D0
       | 11F0  6000 00F2                      BRA.W       12E4
; 970:     }
; 971: 
; 972:     /* Find the total amount of data to read */
; 973:     dp->dp_Res1 = 0L;           /* Set this to 0 */
       | 11F4  7000                           MOVEQ       #00,D0
       | 11F6  2540 000C                      MOVE.L      D0,000C(A2)
; 974:     remaining = dp->dp_Arg3;
       | 11FA  2E2A 001C                      MOVE.L      001C(A2),D7
; 975:     if (!remaining)
       | 11FE  4A87                           TST.L       D7
       | 1200  660A                           BNE.B       120C
; 976:     {
; 977:         dp->dp_Res1 = DOSFALSE;
       | 1202  2540 000C                      MOVE.L      D0,000C(A2)
; 978:         return(FALSE);
       | 1206  7000                           MOVEQ       #00,D0
       | 1208  6000 00DA                      BRA.W       12E4
; 979:     }
; 980: 
; 981:     while (remaining)
       | 120C  4A87                           TST.L       D7
       | 120E  6700 00D2                      BEQ.W       12E2
; 982:     {
; 983:         ULONG rttags[5]={TRN_AllocReqBuffer,sizeof(struct TPacket),TRN_AllocRespBuffer,0,TAG_DONE};
       | 1212  41F9  0000 009E-01             LEA         01.0000009E,A0
       | 1218  43ED FFE0                      LEA         FFE0(A5),A1
       | 121C  7004                           MOVEQ       #04,D0
       | 121E  22D8                           MOVE.L      (A0)+,(A1)+
       | 1220  51C8 FFFC                      DBF         D0,121E
; 984:         struct Transaction *t;
; 985: 
; 986:         thispacketsize = MIN(remaining,MAXSIZE);        /* Find out how big this packet will be */
       | 1224  0C87 0001 0000                 CMPI.L      #00010000,D7
       | 122A  6404                           BCC.B       1230
       | 122C  2007                           MOVE.L      D7,D0
       | 122E  6004                           BRA.B       1234
       | 1230  7001                           MOVEQ       #01,D0
       | 1232  4840                           SWAP        D0
       | 1234  2C00                           MOVE.L      D0,D6
; 987:         remaining -= thispacketsize;
       | 1236  9E86                           SUB.L       D6,D7
; 988:         rttags[3]=sizeof(struct TPacket) + thispacketsize;
       | 1238  2006                           MOVE.L      D6,D0
       | 123A  7234                           MOVEQ       #34,D1
       | 123C  D081                           ADD.L       D1,D0
       | 123E  2B40 FFEC                      MOVE.L      D0,FFEC(A5)
; 989: 
; 990:         t = AllocTransactionA((struct TagItem *)&rttags);
       | 1242  41ED FFE0                      LEA         FFE0(A5),A0
       | 1246  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 124C  4EAE FF8E                      JSR         FF8E(A6)
; 991:         if (t)
       | 1250  2B40 FFDC                      MOVE.L      D0,FFDC(A5)
       | 1254  6764                           BEQ.B       12BA
; 992:         {
; 993:             struct TPacket *tp;
; 994:             tp = (struct TPacket *) t->trans_RequestData;
       | 1256  2040                           MOVEA.L     D0,A0
       | 1258  2B68 002A FFD8                 MOVE.L      002A(A0),FFD8(A5)
; 995:             tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 125E  226D 0014                      MOVEA.L     0014(A5),A1
       | 1262  2C6D FFD8                      MOVEA.L     FFD8(A5),A6
       | 1266  2CA9 0028                      MOVE.L      0028(A1),(A6)
; 996:             tp->tp_DosPacket = dp;
       | 126A  2D4A 0004                      MOVE.L      A2,0004(A6)
; 997:             tp->tp_Type = dp->dp_Type;
       | 126E  2D6A 0008 0008                 MOVE.L      0008(A2),0008(A6)
; 998: 
; 999:             t->trans_ReqDataActual = t->trans_ReqDataLength;
       | 1274  2168 002E 0032                 MOVE.L      002E(A0),0032(A0)
;1000:             t->trans_Timeout = PACKETTIMEOUT;
       | 127A  317C 000A 0042                 MOVE.W      #000A,0042(A0)
;1001:             t->trans_Command = FSCMD_DOSPACKET;
       | 1280  117C 0003 001C                 MOVE.B      #03,001C(A0)
;1002: 
;1003:             tp->tp_Arg1 = (LONG)((struct CFH *)dp->dp_Arg1)->CFH_ServerFH;
       | 1286  206A 0014                      MOVEA.L     0014(A2),A0
       | 128A  2D68 0004 0014                 MOVE.L      0004(A0),0014(A6)
;1004:             tp->tp_Arg2 = sizeof(struct TPacket);
       | 1290  7234                           MOVEQ       #34,D1
       | 1292  2D41 0018                      MOVE.L      D1,0018(A6)
;1005:             tp->tp_Arg3 = thispacketsize;
       | 1296  2D46 001C                      MOVE.L      D6,001C(A6)
;1006:             /* When the packets return, I can match up the 'last' one to the DosPacket
;1007:              * by looking at tp_Arg4.  If 0, that tpacket is the last one -- and the
;1008:              * associated dospacket can be returned.
;1009:              */
;1010:             tp->tp_Arg4 = remaining;
       | 129A  2D47 0020                      MOVE.L      D7,0020(A6)
;1011:             BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t);
       | 129E  2F0A                           MOVE.L      A2,-(A7)
       | 12A0  2440                           MOVEA.L     D0,A2
       | 12A2  2069 003C                      MOVEA.L     003C(A1),A0
       | 12A6  2269 0038                      MOVEA.L     0038(A1),A1
       | 12AA  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 12B0  4EAE FF64                      JSR         FF64(A6)
       | 12B4  245F                           MOVEA.L     (A7)+,A2
;1012:         }
;1013:         else
       | 12B6  6000 FF54                      BRA.W       120C
;1014:         {
;1015:             m->mfsi_Destination = Reconnect(x,m,TRUE,0);
       | 12BA  42A7                           CLR.L       -(A7)
       | 12BC  4878 0001                      PEA         0001
       | 12C0  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 12C4  2F0B                           MOVE.L      A3,-(A7)
       | 12C6  6100 F95C                      BSR.W       0C24
       | 12CA  206D 0014                      MOVEA.L     0014(A5),A0
       | 12CE  2140 003C                      MOVE.L      D0,003C(A0)
;1016:             dp->dp_Res1 = DOSTRUE;
       | 12D2  70FF                           MOVEQ       #FF,D0
       | 12D4  2540 000C                      MOVE.L      D0,000C(A2)
;1017:             dp->dp_Res2 = ERROR_NO_FREE_STORE;
       | 12D8  7067                           MOVEQ       #67,D0
       | 12DA  2540 0010                      MOVE.L      D0,0010(A2)
;1018:             return(FALSE);
       | 12DE  7000                           MOVEQ       #00,D0
       | 12E0  6002                           BRA.B       12E4
;1019:         }
;1020:     }
;1021:     return(TRUE);
       | 12E2  7001                           MOVEQ       #01,D0
;1022: }
       | 12E4  4CED 4CC0 FFC4                 MOVEM.L     FFC4(A5),D6-D7/A2-A3/A6
       | 12EA  4E5D                           UNLK        A5
       | 12EC  4E75                           RTS
;1023: 
;1024: 
;1025: /* StartWrite
;1026:  *
;1027:  * ACTION_WRITE
;1028:  *
;1029:  */
;1030: BOOL StartWrite(struct Transaction *x, struct TPacket *xp, struct DosPacket *dp,
;1031:                 struct MountedFSInfoClient *m)
;1032: {
       | 12EE  4E55 FFDC                      LINK        A5,#FFDC
       | 12F2  48E7 0332                      MOVEM.L     D6-D7/A2-A3/A6,-(A7)
       | 12F6  266D 0008                      MOVEA.L     0008(A5),A3
       | 12FA  246D 0010                      MOVEA.L     0010(A5),A2
;1033: 
;1034:     struct CFH *cf;
;1035:     ULONG remaining, thispacketsize;
;1036: 
;1037:     cf = (struct CFH *) dp->dp_Arg1;
       | 12FE  2B6A 0014 FFFC                 MOVE.L      0014(A2),FFFC(A5)
;1038: 
;1039:     /* The possibility of an old, unreincarnated fh exists. */
;1040: 
;1041:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1304  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1308  2028 0010                      MOVE.L      0010(A0),D0
       | 130C  206D 0014                      MOVEA.L     0014(A5),A0
       | 1310  B0A8 002C                      CMP.L       002C(A0),D0
       | 1314  6414                           BCC.B       132A
;1042:     {
;1043:         dp->dp_Res1 = DOSTRUE;
       | 1316  70FF                           MOVEQ       #FF,D0
       | 1318  2540 000C                      MOVE.L      D0,000C(A2)
;1044:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 131C  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
;1045:         return(FALSE);
       | 1324  7000                           MOVEQ       #00,D0
       | 1326  6000 0120                      BRA.W       1448
;1046:     }
;1047: 
;1048:     /* Find the total amount of data to read */
;1049:     remaining = dp->dp_Arg3;
       | 132A  2E2A 001C                      MOVE.L      001C(A2),D7
;1050:     dp->dp_Res1 = 0L;           /* Set this to 0 */
       | 132E  42AA 000C                      CLR.L       000C(A2)
;1051:     while (remaining)
       | 1332  4A87                           TST.L       D7
       | 1334  6700 0110                      BEQ.W       1446
;1052:     {
;1053:         ULONG rttags[3]={TRN_AllocReqBuffer,0,TAG_DONE};
       | 1338  41F9  0000 00B2-01             LEA         01.000000B2,A0
       | 133E  43ED FFE8                      LEA         FFE8(A5),A1
       | 1342  22D8                           MOVE.L      (A0)+,(A1)+
       | 1344  22D8                           MOVE.L      (A0)+,(A1)+
       | 1346  22D8                           MOVE.L      (A0)+,(A1)+
;1054:         struct Transaction *t;
;1055:         UBYTE *dataptr;
;1056: 
;1057: 
;1058:         thispacketsize = MIN(remaining,MAXSIZE);        /* Find out how big this packet will be */
       | 1348  0C87 0001 0000                 CMPI.L      #00010000,D7
       | 134E  6404                           BCC.B       1354
       | 1350  2007                           MOVE.L      D7,D0
       | 1352  6004                           BRA.B       1358
       | 1354  7001                           MOVEQ       #01,D0
       | 1356  4840                           SWAP        D0
       | 1358  2C00                           MOVE.L      D0,D6
;1059:         rttags[1]=sizeof(struct TPacket) + thispacketsize;
       | 135A  2006                           MOVE.L      D6,D0
       | 135C  7234                           MOVEQ       #34,D1
       | 135E  D081                           ADD.L       D1,D0
       | 1360  2B40 FFEC                      MOVE.L      D0,FFEC(A5)
;1060: 
;1061:         dataptr = &(((UBYTE *)dp->dp_Arg2)[dp->dp_Arg3-remaining]);
       | 1364  206A 0018                      MOVEA.L     0018(A2),A0
       | 1368  202A 001C                      MOVE.L      001C(A2),D0
       | 136C  9087                           SUB.L       D7,D0
       | 136E  D1C0                           ADDA.L      D0,A0
;1062:         remaining -= thispacketsize;
       | 1370  9E86                           SUB.L       D6,D7
;1063: 
;1064:         t = AllocTransactionA((struct TagItem *)&rttags);
       | 1372  2B48 FFE0                      MOVE.L      A0,FFE0(A5)
       | 1376  41ED FFE8                      LEA         FFE8(A5),A0
       | 137A  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 1380  4EAE FF8E                      JSR         FF8E(A6)
;1065:         if (t)
       | 1384  2B40 FFE4                      MOVE.L      D0,FFE4(A5)
       | 1388  6700 0094                      BEQ.W       141E
;1066:         {
;1067:             struct TPacket *tp;
;1068:             tp = (struct TPacket *) t->trans_RequestData;
       | 138C  2040                           MOVEA.L     D0,A0
       | 138E  2B68 002A FFDC                 MOVE.L      002A(A0),FFDC(A5)
;1069:             tp->tp_ServerMFSI = m->mfsi_ServerMFSI;
       | 1394  226D 0014                      MOVEA.L     0014(A5),A1
       | 1398  2C6D FFDC                      MOVEA.L     FFDC(A5),A6
       | 139C  2CA9 0028                      MOVE.L      0028(A1),(A6)
;1070:             tp->tp_DosPacket = dp;
       | 13A0  2D4A 0004                      MOVE.L      A2,0004(A6)
;1071:             tp->tp_Type = dp->dp_Type;
       | 13A4  2D6A 0008 0008                 MOVE.L      0008(A2),0008(A6)
;1072: 
;1073:             t->trans_ReqDataActual = t->trans_ReqDataLength;
       | 13AA  2228 002E                      MOVE.L      002E(A0),D1
       | 13AE  2141 0032                      MOVE.L      D1,0032(A0)
;1074:             t->trans_Timeout = PACKETTIMEOUT;
       | 13B2  317C 000A 0042                 MOVE.W      #000A,0042(A0)
;1075:             t->trans_Command = FSCMD_DOSPACKET;
       | 13B8  117C 0003 001C                 MOVE.B      #03,001C(A0)
;1076:             t->trans_ResponseData = t->trans_RequestData;
       | 13BE  2168 002A 0036                 MOVE.L      002A(A0),0036(A0)
;1077:             t->trans_RespDataLength = t->trans_ReqDataLength;
       | 13C4  2168 002E 003A                 MOVE.L      002E(A0),003A(A0)
;1078: 
;1079:             tp->tp_Arg1 = (LONG)((struct CFH *)dp->dp_Arg1)->CFH_ServerFH;
       | 13CA  206A 0014                      MOVEA.L     0014(A2),A0
       | 13CE  2D68 0004 0014                 MOVE.L      0004(A0),0014(A6)
;1080:             tp->tp_Arg2 = sizeof(struct TPacket);
       | 13D4  7034                           MOVEQ       #34,D0
       | 13D6  2D40 0018                      MOVE.L      D0,0018(A6)
;1081:             tp->tp_Arg3 = thispacketsize;
       | 13DA  2D46 001C                      MOVE.L      D6,001C(A6)
;1082: 
;1083:             /* When the packets return, I can match up the 'last' one to the DosPacket
;1084:              * by looking at tp_Arg4.  If 0, that tpacket is the last one -- and the
;1085:              * associated dospacket can be returned.
;1086:              */
;1087:             tp->tp_Arg4 = remaining;
       | 13DE  2D47 0020                      MOVE.L      D7,0020(A6)
;1088:             CopyMem(dataptr,&((UBYTE *)tp)[sizeof(struct TPacket)],thispacketsize);
       | 13E2  41EE 0034                      LEA         0034(A6),A0
       | 13E6  2248                           MOVEA.L     A0,A1
       | 13E8  206D FFE0                      MOVEA.L     FFE0(A5),A0
       | 13EC  2006                           MOVE.L      D6,D0
       | 13EE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 13F4  4EAE FD90                      JSR         FD90(A6)
;1089:             BeginTransaction(m->mfsi_Destination,m->mfsi_Source,t);
       | 13F8  2F0A                           MOVE.L      A2,-(A7)
       | 13FA  206D 0014                      MOVEA.L     0014(A5),A0
       | 13FE  2068 003C                      MOVEA.L     003C(A0),A0
       | 1402  226D 0014                      MOVEA.L     0014(A5),A1
       | 1406  2269 0038                      MOVEA.L     0038(A1),A1
       | 140A  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 1410  246D FFE4                      MOVEA.L     FFE4(A5),A2
       | 1414  4EAE FF64                      JSR         FF64(A6)
       | 1418  245F                           MOVEA.L     (A7)+,A2
;1090:         }
;1091:         else
       | 141A  6000 FF16                      BRA.W       1332
;1092:         {
;1093:             m->mfsi_Destination = Reconnect(x,m,TRUE,0);
       | 141E  42A7                           CLR.L       -(A7)
       | 1420  4878 0001                      PEA         0001
       | 1424  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 1428  2F0B                           MOVE.L      A3,-(A7)
       | 142A  6100 F7F8                      BSR.W       0C24
       | 142E  206D 0014                      MOVEA.L     0014(A5),A0
       | 1432  2140 003C                      MOVE.L      D0,003C(A0)
;1094:             dp->dp_Res1 = DOSTRUE;
       | 1436  70FF                           MOVEQ       #FF,D0
       | 1438  2540 000C                      MOVE.L      D0,000C(A2)
;1095:             dp->dp_Res2 = ERROR_NO_FREE_STORE;
       | 143C  7067                           MOVEQ       #67,D0
       | 143E  2540 0010                      MOVE.L      D0,0010(A2)
;1096:             return(FALSE);
       | 1442  7000                           MOVEQ       #00,D0
       | 1444  6002                           BRA.B       1448
;1097:         }
;1098: /* Because of the potential problems of not being able to get a transaction at this point,
;1099:  * I feel the adequate solution is to return FAILED at this point, and also force a
;1100:  * reconnect.  I have yet to code this, though.  FIXME!  IMPORTANT! */
;1101:     }
;1102: 
;1103:     return(TRUE);
       | 1446  7001                           MOVEQ       #01,D0
;1104: }
       | 1448  4CED 4CC0 FFC8                 MOVEM.L     FFC8(A5),D6-D7/A2-A3/A6
       | 144E  4E5D                           UNLK        A5
       | 1450  4E75                           RTS
;1105: 
;1106: 
;1107: /*
;1108:  *  EndRead
;1109:  *
;1110:  *  Finishes off each individual ACTION_READ . . .
;1111:  *
;1112:  */
;1113: 
;1114: void EndRead(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
;1115: {
       | 1452  4E55 FFF4                      LINK        A5,#FFF4
       | 1456  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
;1116:     struct DosPacket *dp=tp->tp_DosPacket;
       | 145A  206D 000C                      MOVEA.L     000C(A5),A0
       | 145E  2668 0004                      MOVEA.L     0004(A0),A3
;1117: 
;1118:     /*
;1119:      * If successful, find out where the return data should go,
;1120:      * copy the data there, and be done with it.
;1121:      */
;1122: 
;1123:     if ((tp->tp_Res1) && (tp->tp_Res1 != -1))
       | 1462  2028 000C                      MOVE.L      000C(A0),D0
       | 1466  674C                           BEQ.B       14B4
       | 1468  72FF                           MOVEQ       #FF,D1
       | 146A  B081                           CMP.L       D1,D0
       | 146C  6746                           BEQ.B       14B4
;1124:     {
;1125:         /* Some data was returned */
;1126:         ULONG offset;
;1127:         UBYTE *destptr;
;1128:         /* the Offset that this block lies at can be calculated by:
;1129:          * TotalAskedFor - RemainingAfterThisBlock - SizeRequestedOfThisBlock
;1130:          */
;1131:         ((struct CFH *)dp->dp_Arg1)->CFH_Pos += tp->tp_Res1;
       | 146E  226B 0014                      MOVEA.L     0014(A3),A1
       | 1472  D1A9 0008                      ADD.L       D0,0008(A1)
;1132:         offset = dp->dp_Arg3 - tp->tp_Arg4 - tp->tp_Arg3;
       | 1476  202B 001C                      MOVE.L      001C(A3),D0
       | 147A  90A8 0020                      SUB.L       0020(A0),D0
       | 147E  90A8 001C                      SUB.L       001C(A0),D0
       | 1482  2E00                           MOVE.L      D0,D7
;1133:         destptr = (UBYTE *) &((UBYTE *) dp->dp_Arg2)[offset];
       | 1484  226B 0018                      MOVEA.L     0018(A3),A1
       | 1488  D3C7                           ADDA.L      D7,A1
       | 148A  2449                           MOVEA.L     A1,A2
;1134:         CopyMem(&((UBYTE *)tp)[sizeof(struct TPacket)],destptr,tp->tp_Res1);
       | 148C  43E8 0034                      LEA         0034(A0),A1
       | 1490  2049                           MOVEA.L     A1,A0
       | 1492  224A                           MOVEA.L     A2,A1
       | 1494  2C6D 000C                      MOVEA.L     000C(A5),A6
       | 1498  202E 000C                      MOVE.L      000C(A6),D0
       | 149C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 14A2  4EAE FD90                      JSR         FD90(A6)
;1135:         dp->dp_Res1 += tp->tp_Res1;
       | 14A6  206D 000C                      MOVEA.L     000C(A5),A0
       | 14AA  2028 000C                      MOVE.L      000C(A0),D0
       | 14AE  D1AB 000C                      ADD.L       D0,000C(A3)
;1136:     }
;1137:     else
       | 14B2  600A                           BRA.B       14BE
;1138:     {
;1139:         if (tp->tp_Res1 == -1)
       | 14B4  72FF                           MOVEQ       #FF,D1
       | 14B6  B081                           CMP.L       D1,D0
       | 14B8  6604                           BNE.B       14BE
;1140:             dp->dp_Res1 = tp->tp_Res1;
       | 14BA  2740 000C                      MOVE.L      D0,000C(A3)
;1141:     }
;1142: 
;1143:     if (!tp->tp_Arg4)                               /* If the last packet, return the dp! */
       | 14BE  4AA8 0020                      TST.L       0020(A0)
       | 14C2  6612                           BNE.B       14D6
;1144:         ReplyPkt(dp,dp->dp_Res1,tp->tp_Res2);
       | 14C4  2F28 0010                      MOVE.L      0010(A0),-(A7)
       | 14C8  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 14CC  2F0B                           MOVE.L      A3,-(A7)
       | 14CE  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 14D2  4FEF 000C                      LEA         000C(A7),A7
;1145: 
;1146: }
       | 14D6  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 14DA  4E5D                           UNLK        A5
       | 14DC  4E75                           RTS
;1147: 
;1148: 
;1149: /*
;1150:  *  EndWrite
;1151:  *
;1152:  *  Finishes off each individual ACTION_WRITE . . .
;1153:  *
;1154:  */
;1155: 
;1156: void EndWrite(struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
;1157: {
       | 14DE  4E55 FFFC                      LINK        A5,#FFFC
       | 14E2  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 14E6  266D 000C                      MOVEA.L     000C(A5),A3
;1158:     struct DosPacket *dp=tp->tp_DosPacket;
       | 14EA  246B 0004                      MOVEA.L     0004(A3),A2
;1159: 
;1160:     /*
;1161:      * If successful, increment the Res1 counter
;1162:      * copy the data there, and be done with it.
;1163:      */
;1164: 
;1165:     if ( (tp->tp_Res1) && (tp->tp_Res1 != -1) ) /* If a good response */
       | 14EE  202B 000C                      MOVE.L      000C(A3),D0
       | 14F2  6718                           BEQ.B       150C
       | 14F4  72FF                           MOVEQ       #FF,D1
       | 14F6  B081                           CMP.L       D1,D0
       | 14F8  6712                           BEQ.B       150C
;1166:     {
;1167:         dp->dp_Res1 += tp->tp_Res1;
       | 14FA  D1AA 000C                      ADD.L       D0,000C(A2)
;1168:         ((struct CFH *)dp->dp_Arg1)->CFH_Pos += tp->tp_Res1;
       | 14FE  206A 0014                      MOVEA.L     0014(A2),A0
       | 1502  202B 000C                      MOVE.L      000C(A3),D0
       | 1506  D1A8 0008                      ADD.L       D0,0008(A0)
;1169: 
;1170:     }
;1171:     else
       | 150A  6004                           BRA.B       1510
;1172:     {
;1173:         dp->dp_Res1 = tp->tp_Res1;
       | 150C  2540 000C                      MOVE.L      D0,000C(A2)
;1174:     }
;1175: 
;1176:     if (!tp->tp_Arg4)                               /* If the last packet, return the dp! */
       | 1510  4AAB 0020                      TST.L       0020(A3)
       | 1514  6612                           BNE.B       1528
;1177:         ReplyPkt(dp,dp->dp_Res1,tp->tp_Res2);
       | 1516  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 151A  2F2A 000C                      MOVE.L      000C(A2),-(A7)
       | 151E  2F0A                           MOVE.L      A2,-(A7)
       | 1520  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 1524  4FEF 000C                      LEA         000C(A7),A7
;1178: 
;1179: }
       | 1528  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 152C  4E5D                           UNLK        A5
       | 152E  4E75                           RTS
;1180: 
;1181: 
;1182: /* StartRename
;1183:  *
;1184:  * ACTION_RENAME_OBJECT
;1185:  *
;1186:  */
;1187: BOOL StartRename(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1188:                  struct MountedFSInfoClient *m)
;1189: {
       | 1530  4E55 FFF8                      LINK        A5,#FFF8
       | 1534  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 1538  266D 0008                      MOVEA.L     0008(A5),A3
       | 153C  246D 000C                      MOVEA.L     000C(A5),A2
;1190: 
;1191:     UBYTE *gp;
;1192: 
;1193:     /* Either of the two given locks may be old, and invalid.  Check.  */
;1194:     /* If not a root lock, fill in the TPacket version */
;1195: 
;1196:     /* Src lock */
;1197:     if (dp->dp_Arg1)
       | 1540  206D 0010                      MOVEA.L     0010(A5),A0
       | 1544  2028 0014                      MOVE.L      0014(A0),D0
       | 1548  6738                           BEQ.B       1582
;1198:     {
;1199:         struct CLock *ol;
;1200:         ol = (struct CLock *) BADDR(dp->dp_Arg1);
       | 154A  E580                           ASL.L       #2,D0
;1201: 
;1202:         if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 154C  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 1550  206D 0014                      MOVEA.L     0014(A5),A0
       | 1554  2228 002C                      MOVE.L      002C(A0),D1
       | 1558  2040                           MOVEA.L     D0,A0
       | 155A  2428 0018                      MOVE.L      0018(A0),D2
       | 155E  B481                           CMP.L       D1,D2
       | 1560  6418                           BCC.B       157A
;1203:         {
;1204:             dp->dp_Res1 = DOSFALSE;
       | 1562  7400                           MOVEQ       #00,D2
       | 1564  226D 0010                      MOVEA.L     0010(A5),A1
       | 1568  2342 000C                      MOVE.L      D2,000C(A1)
;1205:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 156C  722C                           MOVEQ       #2C,D1
       | 156E  4601                           NOT.B       D1
       | 1570  2341 0010                      MOVE.L      D1,0010(A1)
;1206:             return (FALSE);
       | 1574  7000                           MOVEQ       #00,D0
       | 1576  6000 012C                      BRA.W       16A4
;1207:         }
;1208: 
;1209:         tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
       | 157A  2568 0014 0014                 MOVE.L      0014(A0),0014(A2)
;1210:     }
;1211:     else
       | 1580  6006                           BRA.B       1588
;1212:         tp->tp_Arg1 = 0L;
       | 1582  7000                           MOVEQ       #00,D0
       | 1584  2540 0014                      MOVE.L      D0,0014(A2)
;1213: 
;1214: 
;1215:     /* Dst Lock */
;1216:     if (dp->dp_Arg3)
       | 1588  206D 0010                      MOVEA.L     0010(A5),A0
       | 158C  2028 001C                      MOVE.L      001C(A0),D0
       | 1590  6736                           BEQ.B       15C8
;1217:     {
;1218:         struct CLock *ol;
;1219:         ol = (struct CLock *) BADDR(dp->dp_Arg3);
       | 1592  E580                           ASL.L       #2,D0
;1220:         if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1594  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 1598  2040                           MOVEA.L     D0,A0
       | 159A  2228 0018                      MOVE.L      0018(A0),D1
       | 159E  226D 0014                      MOVEA.L     0014(A5),A1
       | 15A2  B2A9 002C                      CMP.L       002C(A1),D1
       | 15A6  6418                           BCC.B       15C0
;1221:         {
;1222:             dp->dp_Res1 = DOSFALSE;
       | 15A8  7200                           MOVEQ       #00,D1
       | 15AA  226D 0010                      MOVEA.L     0010(A5),A1
       | 15AE  2341 000C                      MOVE.L      D1,000C(A1)
;1223:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 15B2  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1224:             return (FALSE);
       | 15BA  7000                           MOVEQ       #00,D0
       | 15BC  6000 00E6                      BRA.W       16A4
;1225:         }
;1226:         tp->tp_Arg3 = (LONG) ol->CLock_ServerLock;
       | 15C0  2568 0014 001C                 MOVE.L      0014(A0),001C(A2)
;1227:     }
;1228:     else
       | 15C6  6004                           BRA.B       15CC
;1229:         tp->tp_Arg3 = 0L;
       | 15C8  42AA 001C                      CLR.L       001C(A2)
;1230: 
;1231: 
;1232:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 15CC  206B 002A                      MOVEA.L     002A(A3),A0
       | 15D0  D0FC 0034                      ADDA.W      #0034,A0
       | 15D4  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1233: 
;1234:     /* Fill in the "from" filepath */
;1235:     /* bump gp up to longword boundries */
;1236:     while ((ULONG) gp & 3L)
       | 15D8  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 15DC  2200                           MOVE.L      D0,D1
       | 15DE  7403                           MOVEQ       #03,D2
       | 15E0  C282                           AND.L       D2,D1
       | 15E2  6706                           BEQ.B       15EA
;1237:         gp++;
       | 15E4  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 15E8  60EE                           BRA.B       15D8
;1238: 
;1239:     tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
       | 15EA  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 15EE  220A                           MOVE.L      A2,D1
       | 15F0  2400                           MOVE.L      D0,D2
       | 15F2  9481                           SUB.L       D1,D2
       | 15F4  2542 0018                      MOVE.L      D2,0018(A2)
;1240: 
;1241:     CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
       | 15F8  206D 0010                      MOVEA.L     0010(A5),A0
       | 15FC  2228 0018                      MOVE.L      0018(A0),D1
       | 1600  E581                           ASL.L       #2,D1
       | 1602  7400                           MOVEQ       #00,D2
       | 1604  2041                           MOVEA.L     D1,A0
       | 1606  1410                           MOVE.B      (A0),D2
       | 1608  5282                           ADDQ.L      #1,D2
       | 160A  2240                           MOVEA.L     D0,A1
       | 160C  2002                           MOVE.L      D2,D0
       | 160E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1614  4EAE FD90                      JSR         FD90(A6)
;1242:     FixBString(gp);
       | 1618  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 161C  6100 F156                      BSR.W       0774
       | 1620  584F                           ADDQ.W      #4,A7
;1243:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];
       | 1622  206D 0010                      MOVEA.L     0010(A5),A0
       | 1626  2028 0018                      MOVE.L      0018(A0),D0
       | 162A  E580                           ASL.L       #2,D0
       | 162C  7200                           MOVEQ       #00,D1
       | 162E  2040                           MOVEA.L     D0,A0
       | 1630  1210                           MOVE.B      (A0),D1
       | 1632  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1636  D1C1                           ADDA.L      D1,A0
       | 1638  43E8 0001                      LEA         0001(A0),A1
       | 163C  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
;1244: 
;1245:     /* Fill in the "to" filepath */
;1246:     /* bump gp up to longword boundries */
;1247:     while ((ULONG) gp & 3L)
       | 1640  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1644  2200                           MOVE.L      D0,D1
       | 1646  7403                           MOVEQ       #03,D2
       | 1648  C282                           AND.L       D2,D1
       | 164A  6706                           BEQ.B       1652
;1248:         gp++;
       | 164C  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 1650  60EE                           BRA.B       1640
;1249: 
;1250:     tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);
       | 1652  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1656  220A                           MOVE.L      A2,D1
       | 1658  2400                           MOVE.L      D0,D2
       | 165A  9481                           SUB.L       D1,D2
       | 165C  2542 0020                      MOVE.L      D2,0020(A2)
;1251: 
;1252:     CopyMem(BADDR(dp->dp_Arg4),gp,((UBYTE *)BADDR(dp->dp_Arg4))[0]+1);
       | 1660  206D 0010                      MOVEA.L     0010(A5),A0
       | 1664  2228 0020                      MOVE.L      0020(A0),D1
       | 1668  E581                           ASL.L       #2,D1
       | 166A  7400                           MOVEQ       #00,D2
       | 166C  2041                           MOVEA.L     D1,A0
       | 166E  1410                           MOVE.B      (A0),D2
       | 1670  5282                           ADDQ.L      #1,D2
       | 1672  2240                           MOVEA.L     D0,A1
       | 1674  2002                           MOVE.L      D2,D0
       | 1676  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 167C  4EAE FD90                      JSR         FD90(A6)
;1253:     FixBString(gp);
       | 1680  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1684  6100 F0EE                      BSR.W       0774
;1254:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg4))[0]+1];
       | 1688  206D 0010                      MOVEA.L     0010(A5),A0
       | 168C  2028 0020                      MOVE.L      0020(A0),D0
       | 1690  E580                           ASL.L       #2,D0
       | 1692  7200                           MOVEQ       #00,D1
       | 1694  2040                           MOVEA.L     D0,A0
       | 1696  1210                           MOVE.B      (A0),D1
       | 1698  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 169C  D1C1                           ADDA.L      D1,A0
       | 169E  43E8 0001                      LEA         0001(A0),A1
;1255: 
;1256:     return(TRUE);
       | 16A2  7001                           MOVEQ       #01,D0
;1257: 
;1258: }
       | 16A4  4CED 4C04 FFE8                 MOVEM.L     FFE8(A5),D2/A2-A3/A6
       | 16AA  4E5D                           UNLK        A5
       | 16AC  4E75                           RTS
;1259: 
;1260: 
;1261: /* StartSetProtect
;1262:  *
;1263:  * ACTION_SET_PROTECT
;1264:  *
;1265:  */
;1266: BOOL StartSetProtect(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1267:                      struct MountedFSInfoClient *m)
;1268: {
       | 16AE  4E55 FFF8                      LINK        A5,#FFF8
       | 16B2  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 16B6  266D 0008                      MOVEA.L     0008(A5),A3
       | 16BA  246D 000C                      MOVEA.L     000C(A5),A2
;1269: 
;1270:     UBYTE *gp;
;1271:     struct CLock *cl;
;1272: 
;1273: /* The given lock may be old */
;1274:     if (dp->dp_Arg2)
       | 16BE  206D 0010                      MOVEA.L     0010(A5),A0
       | 16C2  2028 0018                      MOVE.L      0018(A0),D0
       | 16C6  6736                           BEQ.B       16FE
;1275:     {
;1276:         cl = (struct CLock *) BADDR(dp->dp_Arg2);
       | 16C8  E580                           ASL.L       #2,D0
;1277:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 16CA  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 16CE  2040                           MOVEA.L     D0,A0
       | 16D0  2228 0018                      MOVE.L      0018(A0),D1
       | 16D4  226D 0014                      MOVEA.L     0014(A5),A1
       | 16D8  B2A9 002C                      CMP.L       002C(A1),D1
       | 16DC  6418                           BCC.B       16F6
;1278:         {
;1279:             dp->dp_Res1 = DOSFALSE;
       | 16DE  7200                           MOVEQ       #00,D1
       | 16E0  226D 0010                      MOVEA.L     0010(A5),A1
       | 16E4  2341 000C                      MOVE.L      D1,000C(A1)
;1280:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 16E8  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1281:             return(FALSE);
       | 16F0  7000                           MOVEQ       #00,D0
       | 16F2  6000 0088                      BRA.W       177C
;1282:         }
;1283:         tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
       | 16F6  2568 0014 0018                 MOVE.L      0014(A0),0018(A2)
;1284:     }
;1285:     else
       | 16FC  6004                           BRA.B       1702
;1286:         tp->tp_Arg2 = 0L;
       | 16FE  42AA 0018                      CLR.L       0018(A2)
;1287: 
;1288:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 1702  206B 002A                      MOVEA.L     002A(A3),A0
       | 1706  D0FC 0034                      ADDA.W      #0034,A0
       | 170A  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1289: 
;1290:     while ((ULONG) gp & 3L)
       | 170E  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1712  2200                           MOVE.L      D0,D1
       | 1714  7403                           MOVEQ       #03,D2
       | 1716  C282                           AND.L       D2,D1
       | 1718  6706                           BEQ.B       1720
;1291:         gp++;
       | 171A  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 171E  60EE                           BRA.B       170E
;1292: 
;1293:     tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);
       | 1720  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1724  220A                           MOVE.L      A2,D1
       | 1726  2400                           MOVE.L      D0,D2
       | 1728  9481                           SUB.L       D1,D2
       | 172A  2542 001C                      MOVE.L      D2,001C(A2)
;1294: 
;1295:     CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
       | 172E  206D 0010                      MOVEA.L     0010(A5),A0
       | 1732  2228 001C                      MOVE.L      001C(A0),D1
       | 1736  E581                           ASL.L       #2,D1
       | 1738  7400                           MOVEQ       #00,D2
       | 173A  2041                           MOVEA.L     D1,A0
       | 173C  1410                           MOVE.B      (A0),D2
       | 173E  5282                           ADDQ.L      #1,D2
       | 1740  2240                           MOVEA.L     D0,A1
       | 1742  2002                           MOVE.L      D2,D0
       | 1744  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 174A  4EAE FD90                      JSR         FD90(A6)
;1296:     FixBString(gp);
       | 174E  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1752  6100 F020                      BSR.W       0774
;1297:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];
       | 1756  206D 0010                      MOVEA.L     0010(A5),A0
       | 175A  2028 001C                      MOVE.L      001C(A0),D0
       | 175E  E580                           ASL.L       #2,D0
       | 1760  7200                           MOVEQ       #00,D1
       | 1762  2040                           MOVEA.L     D0,A0
       | 1764  1210                           MOVE.B      (A0),D1
       | 1766  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 176A  D1C1                           ADDA.L      D1,A0
       | 176C  43E8 0001                      LEA         0001(A0),A1
;1298: 
;1299:     tp->tp_Arg4 = dp->dp_Arg4;
       | 1770  206D 0010                      MOVEA.L     0010(A5),A0
       | 1774  2568 0020 0020                 MOVE.L      0020(A0),0020(A2)
;1300: 
;1301:     return(TRUE);
       | 177A  7001                           MOVEQ       #01,D0
;1302: 
;1303: }
       | 177C  4CED 4C04 FFE8                 MOVEM.L     FFE8(A5),D2/A2-A3/A6
       | 1782  4E5D                           UNLK        A5
       | 1784  4E75                           RTS
;1304: 
;1305: /* StartSetComment
;1306:  *
;1307:  * ACTION_SET_COMMENT
;1308:  *
;1309:  */
;1310: BOOL StartSetComment(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1311:                      struct MountedFSInfoClient *m)
;1312: {
       | 1786  4E55 FFF8                      LINK        A5,#FFF8
       | 178A  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 178E  266D 0008                      MOVEA.L     0008(A5),A3
       | 1792  246D 000C                      MOVEA.L     000C(A5),A2
;1313: 
;1314:     UBYTE *gp;
;1315:     struct CLock *cl;
;1316: 
;1317: /* The given lock may be old */
;1318:     if (dp->dp_Arg2)
       | 1796  206D 0010                      MOVEA.L     0010(A5),A0
       | 179A  2028 0018                      MOVE.L      0018(A0),D0
       | 179E  6736                           BEQ.B       17D6
;1319:     {
;1320:         cl = (struct CLock *) BADDR(dp->dp_Arg2);
       | 17A0  E580                           ASL.L       #2,D0
;1321:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 17A2  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 17A6  2040                           MOVEA.L     D0,A0
       | 17A8  2228 0018                      MOVE.L      0018(A0),D1
       | 17AC  226D 0014                      MOVEA.L     0014(A5),A1
       | 17B0  B2A9 002C                      CMP.L       002C(A1),D1
       | 17B4  6418                           BCC.B       17CE
;1322:         {
;1323:             dp->dp_Res1 = DOSFALSE;
       | 17B6  7200                           MOVEQ       #00,D1
       | 17B8  226D 0010                      MOVEA.L     0010(A5),A1
       | 17BC  2341 000C                      MOVE.L      D1,000C(A1)
;1324:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 17C0  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1325:             return(FALSE);
       | 17C8  7000                           MOVEQ       #00,D0
       | 17CA  6000 00E6                      BRA.W       18B2
;1326:         }
;1327:         tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
       | 17CE  2568 0014 0018                 MOVE.L      0014(A0),0018(A2)
;1328:     }
;1329:     else
       | 17D4  6004                           BRA.B       17DA
;1330:         tp->tp_Arg2 = 0L;
       | 17D6  42AA 0018                      CLR.L       0018(A2)
;1331: 
;1332:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 17DA  206B 002A                      MOVEA.L     002A(A3),A0
       | 17DE  D0FC 0034                      ADDA.W      #0034,A0
       | 17E2  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1333: 
;1334:     while ((ULONG) gp & 3L)
       | 17E6  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 17EA  2200                           MOVE.L      D0,D1
       | 17EC  7403                           MOVEQ       #03,D2
       | 17EE  C282                           AND.L       D2,D1
       | 17F0  6706                           BEQ.B       17F8
;1335:         gp++;
       | 17F2  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 17F6  60EE                           BRA.B       17E6
;1336: 
;1337:     tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);
       | 17F8  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 17FC  220A                           MOVE.L      A2,D1
       | 17FE  2400                           MOVE.L      D0,D2
       | 1800  9481                           SUB.L       D1,D2
       | 1802  2542 001C                      MOVE.L      D2,001C(A2)
;1338: 
;1339:     CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
       | 1806  206D 0010                      MOVEA.L     0010(A5),A0
       | 180A  2228 001C                      MOVE.L      001C(A0),D1
       | 180E  E581                           ASL.L       #2,D1
       | 1810  7400                           MOVEQ       #00,D2
       | 1812  2041                           MOVEA.L     D1,A0
       | 1814  1410                           MOVE.B      (A0),D2
       | 1816  5282                           ADDQ.L      #1,D2
       | 1818  2240                           MOVEA.L     D0,A1
       | 181A  2002                           MOVE.L      D2,D0
       | 181C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1822  4EAE FD90                      JSR         FD90(A6)
;1340:     FixBString(gp);
       | 1826  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 182A  6100 EF48                      BSR.W       0774
       | 182E  584F                           ADDQ.W      #4,A7
;1341:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];
       | 1830  206D 0010                      MOVEA.L     0010(A5),A0
       | 1834  2028 001C                      MOVE.L      001C(A0),D0
       | 1838  E580                           ASL.L       #2,D0
       | 183A  7200                           MOVEQ       #00,D1
       | 183C  2040                           MOVEA.L     D0,A0
       | 183E  1210                           MOVE.B      (A0),D1
       | 1840  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1844  D1C1                           ADDA.L      D1,A0
       | 1846  43E8 0001                      LEA         0001(A0),A1
       | 184A  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
;1342: 
;1343:     while ((ULONG) gp & 3L)
       | 184E  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1852  2200                           MOVE.L      D0,D1
       | 1854  7403                           MOVEQ       #03,D2
       | 1856  C282                           AND.L       D2,D1
       | 1858  6706                           BEQ.B       1860
;1344:         gp++;
       | 185A  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 185E  60EE                           BRA.B       184E
;1345: 
;1346:     tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);
       | 1860  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1864  220A                           MOVE.L      A2,D1
       | 1866  2400                           MOVE.L      D0,D2
       | 1868  9481                           SUB.L       D1,D2
       | 186A  2542 0020                      MOVE.L      D2,0020(A2)
;1347: 
;1348:     CopyMem(BADDR(dp->dp_Arg4),gp,((UBYTE *)BADDR(dp->dp_Arg4))[0]+1);
       | 186E  206D 0010                      MOVEA.L     0010(A5),A0
       | 1872  2228 0020                      MOVE.L      0020(A0),D1
       | 1876  E581                           ASL.L       #2,D1
       | 1878  7400                           MOVEQ       #00,D2
       | 187A  2041                           MOVEA.L     D1,A0
       | 187C  1410                           MOVE.B      (A0),D2
       | 187E  5282                           ADDQ.L      #1,D2
       | 1880  2240                           MOVEA.L     D0,A1
       | 1882  2002                           MOVE.L      D2,D0
       | 1884  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 188A  4EAE FD90                      JSR         FD90(A6)
;1349:     FixBString(gp);
       | 188E  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1892  6100 EEE0                      BSR.W       0774
;1350:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg4))[0]+1];
       | 1896  206D 0010                      MOVEA.L     0010(A5),A0
       | 189A  2028 0020                      MOVE.L      0020(A0),D0
       | 189E  E580                           ASL.L       #2,D0
       | 18A0  7200                           MOVEQ       #00,D1
       | 18A2  2040                           MOVEA.L     D0,A0
       | 18A4  1210                           MOVE.B      (A0),D1
       | 18A6  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 18AA  D1C1                           ADDA.L      D1,A0
       | 18AC  43E8 0001                      LEA         0001(A0),A1
;1351: 
;1352:     return(TRUE);
       | 18B0  7001                           MOVEQ       #01,D0
;1353: 
;1354: }
       | 18B2  4CED 4C04 FFE8                 MOVEM.L     FFE8(A5),D2/A2-A3/A6
       | 18B8  4E5D                           UNLK        A5
       | 18BA  4E75                           RTS
;1355: 
;1356: /* StartFreeLock
;1357:  *
;1358:  * ACTION_FREE_LOCK
;1359:  *
;1360:  */
;1361: BOOL StartFreeLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1362:                    struct MountedFSInfoClient *m)
;1363: {
       | 18BC  4E55 FFFC                      LINK        A5,#FFFC
       | 18C0  48E7 2030                      MOVEM.L     D2/A2-A3,-(A7)
       | 18C4  266D 000C                      MOVEA.L     000C(A5),A3
       | 18C8  246D 0010                      MOVEA.L     0010(A5),A2
;1364: 
;1365:     struct CLock *cl;
;1366: 
;1367: /* The given lock may be old */
;1368:     if (dp->dp_Arg1)
       | 18CC  202A 0014                      MOVE.L      0014(A2),D0
       | 18D0  6732                           BEQ.B       1904
;1369:     {
;1370:         cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 18D2  2200                           MOVE.L      D0,D1
       | 18D4  E581                           ASL.L       #2,D1
;1371:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 18D6  2B41 FFFC                      MOVE.L      D1,FFFC(A5)
       | 18DA  2041                           MOVEA.L     D1,A0
       | 18DC  2428 0018                      MOVE.L      0018(A0),D2
       | 18E0  226D 0014                      MOVEA.L     0014(A5),A1
       | 18E4  B4A9 002C                      CMP.L       002C(A1),D2
       | 18E8  6412                           BCC.B       18FC
;1372:         {
;1373:             dp->dp_Res1 = DOSFALSE;
       | 18EA  7400                           MOVEQ       #00,D2
       | 18EC  2542 000C                      MOVE.L      D2,000C(A2)
;1374:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 18F0  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;1375:             return(FALSE);
       | 18F8  7000                           MOVEQ       #00,D0
       | 18FA  6014                           BRA.B       1910
;1376:         }
;1377:         tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
       | 18FC  2768 0014 0014                 MOVE.L      0014(A0),0014(A3)
;1378:     }
;1379:     else
       | 1902  6004                           BRA.B       1908
;1380:         tp->tp_Arg1 = 0L;
       | 1904  42AB 0014                      CLR.L       0014(A3)
;1381: 
;1382:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1908  202A 0014                      MOVE.L      0014(A2),D0
       | 190C  E580                           ASL.L       #2,D0
;1383: 
;1384:     return(TRUE);
       | 190E  7001                           MOVEQ       #01,D0
;1385: 
;1386: }
       | 1910  4CDF 0C04                      MOVEM.L     (A7)+,D2/A2-A3
       | 1914  4E5D                           UNLK        A5
       | 1916  4E75                           RTS
;1387: 
;1388: /*
;1389:  *  EndFreeLock
;1390:  *
;1391:  */
;1392: 
;1393: void EndFreeLock(struct TPacket *tp, struct MountedFSInfoClient *m)
;1394: {
       | 1918  4E55 FFF8                      LINK        A5,#FFF8
       | 191C  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 1920  266D 0008                      MOVEA.L     0008(A5),A3
       | 1924  246D 000C                      MOVEA.L     000C(A5),A2
;1395:     struct DosPacket *dp=tp->tp_DosPacket;
       | 1928  2B6B 0004 FFFC                 MOVE.L      0004(A3),FFFC(A5)
;1396:     struct CLock *cl;
;1397:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 192E  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1932  2028 0014                      MOVE.L      0014(A0),D0
       | 1936  E580                           ASL.L       #2,D0
;1398: 
;1399: /*  Free the path string */
;1400:     NukeLock((APTR)MKBADDR(cl),m);
       | 1938  2200                           MOVE.L      D0,D1
       | 193A  E481                           ASR.L       #2,D1
       | 193C  2F0A                           MOVE.L      A2,-(A7)
       | 193E  2F01                           MOVE.L      D1,-(A7)
       | 1940  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 1944  6100 EFB8                      BSR.W       08FE
       | 1948  504F                           ADDQ.W      #8,A7
;1401:     if (cl->CLock_FullServerPath)
       | 194A  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 194E  2028 001C                      MOVE.L      001C(A0),D0
       | 1952  671C                           BEQ.B       1970
;1402:         FreeMem(cl->CLock_FullServerPath,strlen(cl->CLock_FullServerPath)+1);
       | 1954  2F00                           MOVE.L      D0,-(A7)
       | 1956  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 195A  584F                           ADDQ.W      #4,A7
       | 195C  5280                           ADDQ.L      #1,D0
       | 195E  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 1962  2269 001C                      MOVEA.L     001C(A1),A1
       | 1966  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 196C  4EAE FF2E                      JSR         FF2E(A6)
;1403: 
;1404:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 1970  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 1974  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 1978  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 197C  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1405: }
       | 1980  4CED 4C00 FFEC                 MOVEM.L     FFEC(A5),A2-A3/A6
       | 1986  4E5D                           UNLK        A5
       | 1988  4E75                           RTS
;1406: 
;1407: 
;1408: 
;1409: /* StartSetDate
;1410:  *
;1411:  * ACTION_SET_DATE
;1412:  *
;1413:  */
;1414: BOOL StartSetDate(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1415:                   struct MountedFSInfoClient *m)
;1416: {
       | 198A  4E55 FFF8                      LINK        A5,#FFF8
       | 198E  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 1992  266D 0008                      MOVEA.L     0008(A5),A3
       | 1996  246D 000C                      MOVEA.L     000C(A5),A2
;1417:     UBYTE *gp;
;1418:     struct CLock *cl;
;1419: 
;1420: /* The given lock may be old */
;1421:     cl = (struct CLock *) BADDR(dp->dp_Arg2);
       | 199A  206D 0010                      MOVEA.L     0010(A5),A0
       | 199E  2028 0018                      MOVE.L      0018(A0),D0
       | 19A2  2200                           MOVE.L      D0,D1
       | 19A4  E581                           ASL.L       #2,D1
;1422: 
;1423:     if (dp->dp_Arg2)
       | 19A6  2B41 FFF8                      MOVE.L      D1,FFF8(A5)
       | 19AA  4A80                           TST.L       D0
       | 19AC  6730                           BEQ.B       19DE
;1424:     {
;1425:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 19AE  2041                           MOVEA.L     D1,A0
       | 19B0  2028 0018                      MOVE.L      0018(A0),D0
       | 19B4  226D 0014                      MOVEA.L     0014(A5),A1
       | 19B8  B0A9 002C                      CMP.L       002C(A1),D0
       | 19BC  6418                           BCC.B       19D6
;1426:         {
;1427:             dp->dp_Res1 = DOSFALSE;
       | 19BE  7000                           MOVEQ       #00,D0
       | 19C0  226D 0010                      MOVEA.L     0010(A5),A1
       | 19C4  2340 000C                      MOVE.L      D0,000C(A1)
;1428:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 19C8  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1429:             return(FALSE);
       | 19D0  7000                           MOVEQ       #00,D0
       | 19D2  6000 00C0                      BRA.W       1A94
;1430:         }
;1431:         tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
       | 19D6  2568 0014 0018                 MOVE.L      0014(A0),0018(A2)
;1432:     }
;1433:     else
       | 19DC  6004                           BRA.B       19E2
;1434:         tp->tp_Arg2 = 0L;
       | 19DE  42AA 0018                      CLR.L       0018(A2)
;1435: 
;1436:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 19E2  206B 002A                      MOVEA.L     002A(A3),A0
       | 19E6  D0FC 0034                      ADDA.W      #0034,A0
       | 19EA  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1437: 
;1438:     while ((ULONG) gp & 3L)
       | 19EE  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 19F2  2200                           MOVE.L      D0,D1
       | 19F4  7403                           MOVEQ       #03,D2
       | 19F6  C282                           AND.L       D2,D1
       | 19F8  6706                           BEQ.B       1A00
;1439:         gp++;
       | 19FA  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 19FE  60EE                           BRA.B       19EE
;1440: 
;1441:     tp->tp_Arg3 = ((ULONG) gp - (ULONG) tp);
       | 1A00  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1A04  220A                           MOVE.L      A2,D1
       | 1A06  2400                           MOVE.L      D0,D2
       | 1A08  9481                           SUB.L       D1,D2
       | 1A0A  2542 001C                      MOVE.L      D2,001C(A2)
;1442: 
;1443:     CopyMem(BADDR(dp->dp_Arg3),gp,((UBYTE *)BADDR(dp->dp_Arg3))[0]+1);
       | 1A0E  206D 0010                      MOVEA.L     0010(A5),A0
       | 1A12  2228 001C                      MOVE.L      001C(A0),D1
       | 1A16  E581                           ASL.L       #2,D1
       | 1A18  7400                           MOVEQ       #00,D2
       | 1A1A  2041                           MOVEA.L     D1,A0
       | 1A1C  1410                           MOVE.B      (A0),D2
       | 1A1E  5282                           ADDQ.L      #1,D2
       | 1A20  2240                           MOVEA.L     D0,A1
       | 1A22  2002                           MOVE.L      D2,D0
       | 1A24  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1A2A  4EAE FD90                      JSR         FD90(A6)
;1444:     FixBString(gp);
       | 1A2E  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1A32  6100 ED40                      BSR.W       0774
       | 1A36  584F                           ADDQ.W      #4,A7
;1445:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg3))[0]+1];
       | 1A38  206D 0010                      MOVEA.L     0010(A5),A0
       | 1A3C  2028 001C                      MOVE.L      001C(A0),D0
       | 1A40  E580                           ASL.L       #2,D0
       | 1A42  7200                           MOVEQ       #00,D1
       | 1A44  2040                           MOVEA.L     D0,A0
       | 1A46  1210                           MOVE.B      (A0),D1
       | 1A48  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1A4C  D1C1                           ADDA.L      D1,A0
       | 1A4E  43E8 0001                      LEA         0001(A0),A1
       | 1A52  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
;1446: 
;1447:     while ((ULONG) gp & 3L)
       | 1A56  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1A5A  2200                           MOVE.L      D0,D1
       | 1A5C  7403                           MOVEQ       #03,D2
       | 1A5E  C282                           AND.L       D2,D1
       | 1A60  6706                           BEQ.B       1A68
;1448:         gp++;
       | 1A62  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 1A66  60EE                           BRA.B       1A56
;1449: 
;1450:     tp->tp_Arg4 = ((ULONG) gp - (ULONG) tp);
       | 1A68  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1A6C  220A                           MOVE.L      A2,D1
       | 1A6E  2400                           MOVE.L      D0,D2
       | 1A70  9481                           SUB.L       D1,D2
       | 1A72  2542 0020                      MOVE.L      D2,0020(A2)
;1451:     CopyMem((APTR)dp->dp_Arg4,gp,sizeof(struct DateStamp));
       | 1A76  226D 0010                      MOVEA.L     0010(A5),A1
       | 1A7A  2069 0020                      MOVEA.L     0020(A1),A0
       | 1A7E  2240                           MOVEA.L     D0,A1
       | 1A80  700C                           MOVEQ       #0C,D0
       | 1A82  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1A88  4EAE FD90                      JSR         FD90(A6)
;1452: 
;1453:     gp = &gp[sizeof(struct DateStamp)];
       | 1A8C  700C                           MOVEQ       #0C,D0
       | 1A8E  D1AD FFFC                      ADD.L       D0,FFFC(A5)
;1454: 
;1455:     return(TRUE);
       | 1A92  7001                           MOVEQ       #01,D0
;1456: 
;1457: }
       | 1A94  4CDF 4C04                      MOVEM.L     (A7)+,D2/A2-A3/A6
       | 1A98  4E5D                           UNLK        A5
       | 1A9A  4E75                           RTS
;1458: 
;1459: 
;1460: /* StartParent, StartCopyDir
;1461:  *
;1462:  * ACTION_PARENT
;1463:  *
;1464:  */
;1465: BOOL StartParent(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1466:                   struct MountedFSInfoClient *m)
;1467: {
       | 1A9C  4E55 FFFC                      LINK        A5,#FFFC
       | 1AA0  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 1AA4  266D 000C                      MOVEA.L     000C(A5),A3
       | 1AA8  246D 0010                      MOVEA.L     0010(A5),A2
;1468:     struct CLock *cl;
;1469: 
;1470: /* The given lock may be old */
;1471:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1AAC  202A 0014                      MOVE.L      0014(A2),D0
       | 1AB0  2200                           MOVE.L      D0,D1
       | 1AB2  E581                           ASL.L       #2,D1
;1472: 
;1473:     if (dp->dp_Arg1)
       | 1AB4  2B41 FFFC                      MOVE.L      D1,FFFC(A5)
       | 1AB8  4A80                           TST.L       D0
       | 1ABA  672A                           BEQ.B       1AE6
;1474:     {
;1475:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1ABC  2041                           MOVEA.L     D1,A0
       | 1ABE  2028 0018                      MOVE.L      0018(A0),D0
       | 1AC2  226D 0014                      MOVEA.L     0014(A5),A1
       | 1AC6  B0A9 002C                      CMP.L       002C(A1),D0
       | 1ACA  6412                           BCC.B       1ADE
;1476:         {
;1477:             dp->dp_Res1 = DOSFALSE;
       | 1ACC  7000                           MOVEQ       #00,D0
       | 1ACE  2540 000C                      MOVE.L      D0,000C(A2)
;1478:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 1AD2  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;1479:             return(FALSE);
       | 1ADA  7000                           MOVEQ       #00,D0
       | 1ADC  600E                           BRA.B       1AEC
;1480:         }
;1481:         tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
       | 1ADE  2768 0014 0014                 MOVE.L      0014(A0),0014(A3)
;1482:     }
;1483:     else
       | 1AE4  6004                           BRA.B       1AEA
;1484:         tp->tp_Arg1 = 0L;
       | 1AE6  42AB 0014                      CLR.L       0014(A3)
;1485: 
;1486:     return(TRUE);
       | 1AEA  7001                           MOVEQ       #01,D0
;1487: 
;1488: }
       | 1AEC  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 1AF0  4E5D                           UNLK        A5
       | 1AF2  4E75                           RTS
;1489: 
;1490: /* StartParentFH
;1491:  *
;1492:  * ACTION_PARENT_FH
;1493:  *
;1494:  */
;1495: BOOL StartParentFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1496:                    struct MountedFSInfoClient *m)
;1497: {
       | 1AF4  4E55 FFFC                      LINK        A5,#FFFC
       | 1AF8  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 1AFC  266D 000C                      MOVEA.L     000C(A5),A3
       | 1B00  246D 0010                      MOVEA.L     0010(A5),A2
;1498:     struct CFH *cf;
;1499: 
;1500:     cf = (struct CFH *) dp->dp_Arg1;
       | 1B04  2B6A 0014 FFFC                 MOVE.L      0014(A2),FFFC(A5)
;1501: 
;1502:     /* The possibility of an old, unreincarnated fh exists. */
;1503: 
;1504:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1B0A  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1B0E  2028 0010                      MOVE.L      0010(A0),D0
       | 1B12  226D 0014                      MOVEA.L     0014(A5),A1
       | 1B16  B0A9 002C                      CMP.L       002C(A1),D0
       | 1B1A  6412                           BCC.B       1B2E
;1505:     {
;1506:         dp->dp_Res1 = DOSTRUE;
       | 1B1C  70FF                           MOVEQ       #FF,D0
       | 1B1E  2540 000C                      MOVE.L      D0,000C(A2)
;1507:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 1B22  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
;1508:         return(FALSE);
       | 1B2A  7000                           MOVEQ       #00,D0
       | 1B2C  6008                           BRA.B       1B36
;1509:     }
;1510: 
;1511:     tp->tp_Arg1 = (LONG)cf->CFH_ServerFH;
       | 1B2E  2768 0004 0014                 MOVE.L      0004(A0),0014(A3)
;1512: 
;1513:     return(TRUE);
       | 1B34  7001                           MOVEQ       #01,D0
;1514: 
;1515: }
       | 1B36  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 1B3A  4E5D                           UNLK        A5
       | 1B3C  4E75                           RTS
;1516: 
;1517: 
;1518: /*
;1519:  * EndParent, EndCopyDir, EndLocateObject, EndCreateDir
;1520:  *
;1521:  */
;1522: 
;1523: void EndParent(struct TPacket *tp, struct MountedFSInfoClient *m)
;1524: {
       | 1B3E  4E55 FFF8                      LINK        A5,#FFF8
       | 1B42  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 1B46  266D 0008                      MOVEA.L     0008(A5),A3
       | 1B4A  246D 000C                      MOVEA.L     000C(A5),A2
;1525:     struct DosPacket *dp=tp->tp_DosPacket;
       | 1B4E  2B6B 0004 FFFC                 MOVE.L      0004(A3),FFFC(A5)
;1526:     struct CLock *cl;
;1527: 
;1528:     if (tp->tp_Res1)        /* Successful! */
       | 1B54  4AAB 000C                      TST.L       000C(A3)
       | 1B58  6700 00C2                      BEQ.W       1C1C
;1529:     {
;1530:         cl = (struct CLock *) AllocMem(sizeof(struct CLock),MEMF_CLEAR|MEMF_PUBLIC);
       | 1B5C  7020                           MOVEQ       #20,D0
       | 1B5E  223C 0001 0001                 MOVE.L      #00010001,D1
       | 1B64  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1B6A  4EAE FF3A                      JSR         FF3A(A6)
;1531:         if (!cl)
       | 1B6E  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 1B72  6612                           BNE.B       1B86
;1532:         {
;1533:             ReplyPkt(dp,DOSFALSE,ERROR_NO_FREE_STORE);
       | 1B74  4878 0067                      PEA         0067
       | 1B78  42A7                           CLR.L       -(A7)
       | 1B7A  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1B7E  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1534:             return;
       | 1B82  6000 00AC                      BRA.W       1C30
;1535:         }
;1536:         cl->CLock_ConnectionNumber = m->mfsi_ConnectionNumber;
       | 1B86  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 1B8A  216A 002C 0018                 MOVE.L      002C(A2),0018(A0)
;1537:         cl->CLock_ServerLock = (struct FileLock *) tp->tp_Res1;
       | 1B90  226B 000C                      MOVEA.L     000C(A3),A1
       | 1B94  2149 0014                      MOVE.L      A1,0014(A0)
;1538:         cl->CLock_FileLock.fl_Key = tp->tp_Res1;
       | 1B98  216B 000C 0004                 MOVE.L      000C(A3),0004(A0)
;1539:         if (dp->dp_Type == ACTION_LOCATE_OBJECT)
       | 1B9E  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 1BA2  2029 0008                      MOVE.L      0008(A1),D0
       | 1BA6  7208                           MOVEQ       #08,D1
       | 1BA8  B081                           CMP.L       D1,D0
       | 1BAA  6608                           BNE.B       1BB4
;1540:             cl->CLock_FileLock.fl_Access = dp->dp_Arg3;
       | 1BAC  2169 001C 0008                 MOVE.L      001C(A1),0008(A0)
;1541:         else
       | 1BB2  6020                           BRA.B       1BD4
;1542:         {
;1543:             if ( (dp->dp_Arg1) && (dp->dp_Type != ACTION_COPY_DIR_FH) )
       | 1BB4  2229 0014                      MOVE.L      0014(A1),D1
       | 1BB8  6714                           BEQ.B       1BCE
       | 1BBA  0C80 0000 0406                 CMPI.L      #00000406,D0
       | 1BC0  670C                           BEQ.B       1BCE
;1544:             {
;1545:                 cl->CLock_FileLock.fl_Access = ((struct CLock *) BADDR(dp->dp_Arg1))->CLock_FileLock.fl_Access;
       | 1BC2  E581                           ASL.L       #2,D1
       | 1BC4  2241                           MOVEA.L     D1,A1
       | 1BC6  2169 0008 0008                 MOVE.L      0008(A1),0008(A0)
;1546:             }
;1547:             else
       | 1BCC  6006                           BRA.B       1BD4
;1548:                 cl->CLock_FileLock.fl_Access = SHARED_LOCK;
       | 1BCE  70FE                           MOVEQ       #FE,D0
       | 1BD0  2140 0008                      MOVE.L      D0,0008(A0)
;1549:         }
;1550:         cl->CLock_FileLock.fl_Volume = MKBADDR(m->mfsi_Volume);
       | 1BD4  202A 0034                      MOVE.L      0034(A2),D0
       | 1BD8  E480                           ASR.L       #2,D0
       | 1BDA  2140 0010                      MOVE.L      D0,0010(A0)
;1551:         cl->CLock_FileLock.fl_Task = m->mfsi_LocalPort;
       | 1BDE  216A 0030 000C                 MOVE.L      0030(A2),000C(A0)
;1552:         dp->dp_Res1 = (LONG) MKBADDR(cl);
       | 1BE4  2008                           MOVE.L      A0,D0
       | 1BE6  E480                           ASR.L       #2,D0
       | 1BE8  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 1BEC  2340 000C                      MOVE.L      D0,000C(A1)
;1553:         SaveName(cl,tp);
       | 1BF0  2F0B                           MOVE.L      A3,-(A7)
       | 1BF2  2F08                           MOVE.L      A0,-(A7)
       | 1BF4  6100 EC24                      BSR.W       081A
;1554:         KeepLock((APTR)dp->dp_Res1,m);
       | 1BF8  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 1BFC  2069 000C                      MOVEA.L     000C(A1),A0
       | 1C00  2E8A                           MOVE.L      A2,(A7)
       | 1C02  2F08                           MOVE.L      A0,-(A7)
       | 1C04  6100 EC64                      BSR.W       086A
;1555: 
;1556:         ReplyPkt(dp,dp->dp_Res1,dp->dp_Res2);
       | 1C08  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1C0C  2EA8 0010                      MOVE.L      0010(A0),(A7)
       | 1C10  2F28 000C                      MOVE.L      000C(A0),-(A7)
       | 1C14  2F08                           MOVE.L      A0,-(A7)
       | 1C16  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1557:         return;
       | 1C1A  6014                           BRA.B       1C30
;1558:     }
;1559:     else                    /* Not successful, or Parent reached root */
;1560:     {
;1561:         ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 1C1C  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 1C20  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 1C24  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1C28  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
       | 1C2C  4FEF 000C                      LEA         000C(A7),A7
;1562:     }
;1563: }
       | 1C30  4CED 4C00 FFEC                 MOVEM.L     FFEC(A5),A2-A3/A6
       | 1C36  4E5D                           UNLK        A5
       | 1C38  4E75                           RTS
;1564: 
;1565: 
;1566: /* StartDeleteObject
;1567:  *
;1568:  * ACTION_DELETE_OBJECT
;1569:  *
;1570:  */
;1571: BOOL StartDeleteObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1572:                        struct MountedFSInfoClient *m)
;1573: {
       | 1C3A  4E55 FFF8                      LINK        A5,#FFF8
       | 1C3E  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 1C42  266D 0008                      MOVEA.L     0008(A5),A3
       | 1C46  246D 000C                      MOVEA.L     000C(A5),A2
;1574:     UBYTE *gp;
;1575:     struct CLock *cl;
;1576: 
;1577: /* The given lock may be old */
;1578:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1C4A  206D 0010                      MOVEA.L     0010(A5),A0
       | 1C4E  2028 0014                      MOVE.L      0014(A0),D0
       | 1C52  2200                           MOVE.L      D0,D1
       | 1C54  E581                           ASL.L       #2,D1
;1579: 
;1580:     if (dp->dp_Arg1)
       | 1C56  2B41 FFF8                      MOVE.L      D1,FFF8(A5)
       | 1C5A  4A80                           TST.L       D0
       | 1C5C  672E                           BEQ.B       1C8C
;1581:     {
;1582:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1C5E  2041                           MOVEA.L     D1,A0
       | 1C60  2028 0018                      MOVE.L      0018(A0),D0
       | 1C64  226D 0014                      MOVEA.L     0014(A5),A1
       | 1C68  B0A9 002C                      CMP.L       002C(A1),D0
       | 1C6C  6416                           BCC.B       1C84
;1583:         {
;1584:             dp->dp_Res1 = DOSFALSE;
       | 1C6E  7000                           MOVEQ       #00,D0
       | 1C70  226D 0010                      MOVEA.L     0010(A5),A1
       | 1C74  2340 000C                      MOVE.L      D0,000C(A1)
;1585:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 1C78  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1586:             return(FALSE);
       | 1C80  7000                           MOVEQ       #00,D0
       | 1C82  607C                           BRA.B       1D00
;1587:         }
;1588:         tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
       | 1C84  2568 0014 0014                 MOVE.L      0014(A0),0014(A2)
;1589:     }
;1590:     else
       | 1C8A  6004                           BRA.B       1C90
;1591:         tp->tp_Arg1 = 0L;
       | 1C8C  42AA 0014                      CLR.L       0014(A2)
;1592: 
;1593:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 1C90  206B 002A                      MOVEA.L     002A(A3),A0
       | 1C94  D0FC 0034                      ADDA.W      #0034,A0
       | 1C98  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1594: 
;1595:     while ((ULONG) gp & 3L)
       | 1C9C  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1CA0  2200                           MOVE.L      D0,D1
       | 1CA2  7403                           MOVEQ       #03,D2
       | 1CA4  C282                           AND.L       D2,D1
       | 1CA6  6706                           BEQ.B       1CAE
;1596:         gp++;
       | 1CA8  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 1CAC  60EE                           BRA.B       1C9C
;1597: 
;1598:     tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
       | 1CAE  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1CB2  220A                           MOVE.L      A2,D1
       | 1CB4  2400                           MOVE.L      D0,D2
       | 1CB6  9481                           SUB.L       D1,D2
       | 1CB8  2542 0018                      MOVE.L      D2,0018(A2)
;1599: 
;1600:     CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
       | 1CBC  206D 0010                      MOVEA.L     0010(A5),A0
       | 1CC0  2228 0018                      MOVE.L      0018(A0),D1
       | 1CC4  E581                           ASL.L       #2,D1
       | 1CC6  7400                           MOVEQ       #00,D2
       | 1CC8  2041                           MOVEA.L     D1,A0
       | 1CCA  1410                           MOVE.B      (A0),D2
       | 1CCC  5282                           ADDQ.L      #1,D2
       | 1CCE  2240                           MOVEA.L     D0,A1
       | 1CD0  2002                           MOVE.L      D2,D0
       | 1CD2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1CD8  4EAE FD90                      JSR         FD90(A6)
;1601:     FixBString(gp);
       | 1CDC  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1CE0  6100 EA92                      BSR.W       0774
;1602:     gp = &gp[((UBYTE *)BADDR(dp->dp_Arg2))[0]+1];
       | 1CE4  206D 0010                      MOVEA.L     0010(A5),A0
       | 1CE8  2028 0018                      MOVE.L      0018(A0),D0
       | 1CEC  E580                           ASL.L       #2,D0
       | 1CEE  7200                           MOVEQ       #00,D1
       | 1CF0  2040                           MOVEA.L     D0,A0
       | 1CF2  1210                           MOVE.B      (A0),D1
       | 1CF4  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1CF8  D1C1                           ADDA.L      D1,A0
       | 1CFA  43E8 0001                      LEA         0001(A0),A1
;1603: 
;1604:     return(TRUE);
       | 1CFE  7001                           MOVEQ       #01,D0
;1605: 
;1606: }
       | 1D00  4CED 4C04 FFE8                 MOVEM.L     FFE8(A5),D2/A2-A3/A6
       | 1D06  4E5D                           UNLK        A5
       | 1D08  4E75                           RTS
;1607: 
;1608: 
;1609: 
;1610: /* StartLocateObject
;1611:  *
;1612:  * ACTION_LOCATE_OBJECT
;1613:  *
;1614:  */
;1615: BOOL StartLocateObject(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1616:                        struct MountedFSInfoClient *m)
;1617: {
       | 1D0A  4E55 FFF0                      LINK        A5,#FFF0
       | 1D0E  48E7 2132                      MOVEM.L     D2/D7/A2-A3/A6,-(A7)
       | 1D12  266D 0008                      MOVEA.L     0008(A5),A3
       | 1D16  246D 000C                      MOVEA.L     000C(A5),A2
;1618:     UBYTE *gp;
;1619:     struct CLock *cl;
;1620:     int slen;
;1621: 
;1622: /* The given lock may be old */
;1623:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1D1A  206D 0010                      MOVEA.L     0010(A5),A0
       | 1D1E  2028 0014                      MOVE.L      0014(A0),D0
       | 1D22  2200                           MOVE.L      D0,D1
       | 1D24  E581                           ASL.L       #2,D1
;1624: 
;1625:     if (dp->dp_Arg1)
       | 1D26  2B41 FFF8                      MOVE.L      D1,FFF8(A5)
       | 1D2A  4A80                           TST.L       D0
       | 1D2C  6730                           BEQ.B       1D5E
;1626:     {
;1627:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1D2E  2041                           MOVEA.L     D1,A0
       | 1D30  2028 0018                      MOVE.L      0018(A0),D0
       | 1D34  226D 0014                      MOVEA.L     0014(A5),A1
       | 1D38  B0A9 002C                      CMP.L       002C(A1),D0
       | 1D3C  6418                           BCC.B       1D56
;1628:         {
;1629:             dp->dp_Res1 = DOSFALSE;
       | 1D3E  7000                           MOVEQ       #00,D0
       | 1D40  226D 0010                      MOVEA.L     0010(A5),A1
       | 1D44  2340 000C                      MOVE.L      D0,000C(A1)
;1630:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 1D48  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1631:             return(FALSE);
       | 1D50  7000                           MOVEQ       #00,D0
       | 1D52  6000 0138                      BRA.W       1E8C
;1632:         }
;1633:         tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
       | 1D56  2568 0014 0014                 MOVE.L      0014(A0),0014(A2)
;1634:     }
;1635:     else
       | 1D5C  6004                           BRA.B       1D62
;1636:         tp->tp_Arg1 = 0L;
       | 1D5E  42AA 0014                      CLR.L       0014(A2)
;1637: 
;1638:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 1D62  206B 002A                      MOVEA.L     002A(A3),A0
       | 1D66  D0FC 0034                      ADDA.W      #0034,A0
       | 1D6A  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1639: 
;1640:     while ((ULONG) gp & 3L)
       | 1D6E  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1D72  2200                           MOVE.L      D0,D1
       | 1D74  7403                           MOVEQ       #03,D2
       | 1D76  C282                           AND.L       D2,D1
       | 1D78  6706                           BEQ.B       1D80
;1641:         gp++;
       | 1D7A  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 1D7E  60EE                           BRA.B       1D6E
;1642: 
;1643:     tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
       | 1D80  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1D84  220A                           MOVE.L      A2,D1
       | 1D86  2400                           MOVE.L      D0,D2
       | 1D88  9481                           SUB.L       D1,D2
       | 1D8A  2542 0018                      MOVE.L      D2,0018(A2)
;1644: 
;1645:     CopyMem(BADDR(dp->dp_Arg2),gp,((UBYTE *)BADDR(dp->dp_Arg2))[0]+1);
       | 1D8E  206D 0010                      MOVEA.L     0010(A5),A0
       | 1D92  2228 0018                      MOVE.L      0018(A0),D1
       | 1D96  E581                           ASL.L       #2,D1
       | 1D98  7400                           MOVEQ       #00,D2
       | 1D9A  2041                           MOVEA.L     D1,A0
       | 1D9C  1410                           MOVE.B      (A0),D2
       | 1D9E  5282                           ADDQ.L      #1,D2
       | 1DA0  2240                           MOVEA.L     D0,A1
       | 1DA2  2002                           MOVE.L      D2,D0
       | 1DA4  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1DAA  4EAE FD90                      JSR         FD90(A6)
;1646:     FixBString(gp);
       | 1DAE  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 1DB2  6100 E9C0                      BSR.W       0774
       | 1DB6  584F                           ADDQ.W      #4,A7
;1647:     slen = gp[0];
       | 1DB8  7E00                           MOVEQ       #00,D7
       | 1DBA  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1DBE  1E10                           MOVE.B      (A0),D7
;1648:     gp += gp[0]+1;
       | 1DC0  7000                           MOVEQ       #00,D0
       | 1DC2  1010                           MOVE.B      (A0),D0
       | 1DC4  5280                           ADDQ.L      #1,D0
       | 1DC6  D1AD FFFC                      ADD.L       D0,FFFC(A5)
;1649: 
;1650:     tp->tp_Arg3 = dp->dp_Arg3;
       | 1DCA  206D 0010                      MOVEA.L     0010(A5),A0
       | 1DCE  2568 001C 001C                 MOVE.L      001C(A0),001C(A2)
;1651:     tp->tp_Arg4 = m->mfsi_Flags & FLAGSMASK;
       | 1DD4  7003                           MOVEQ       #03,D0
       | 1DD6  206D 0014                      MOVEA.L     0014(A5),A0
       | 1DDA  C0A8 0054                      AND.L       0054(A0),D0
       | 1DDE  2540 0020                      MOVE.L      D0,0020(A2)
;1652: 
;1653:     t->trans_ReqDataActual = ((ULONG)gp - (ULONG)tp);
       | 1DE2  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1DE6  220A                           MOVE.L      A2,D1
       | 1DE8  9081                           SUB.L       D1,D0
       | 1DEA  2740 0032                      MOVE.L      D0,0032(A3)
;1654: 
;1655: /* Special casing for Mike Sinz */
;1656: /* If the caller is attempting to get a lock on the root of a
;1657:  * volume, indicated by a null string length (after the : and
;1658:  * previous is removed), and null reference lock, create a
;1659:  * lock structure for them immediately, and don't bother to
;1660:  * contact the server.
;1661:  */
;1662:     if ((!cl) && (!slen))
       | 1DEE  4AAD FFF8                      TST.L       FFF8(A5)
       | 1DF2  6600 0096                      BNE.W       1E8A
       | 1DF6  4A87                           TST.L       D7
       | 1DF8  6600 0090                      BNE.W       1E8A
;1663:     {
;1664:         struct CLock *ncl;
;1665:         ncl = (struct CLock *) AllocMem(sizeof(struct CLock),MEMF_CLEAR|MEMF_PUBLIC);
       | 1DFC  7020                           MOVEQ       #20,D0
       | 1DFE  223C 0001 0001                 MOVE.L      #00010001,D1
       | 1E04  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1E0A  4EAE FF3A                      JSR         FF3A(A6)
;1666:         if (ncl)
       | 1E0E  2B40 FFF0                      MOVE.L      D0,FFF0(A5)
       | 1E12  4A80                           TST.L       D0
       | 1E14  6774                           BEQ.B       1E8A
;1667:         {
;1668:             ncl->CLock_ConnectionNumber = m->mfsi_ConnectionNumber;
       | 1E16  206D 0014                      MOVEA.L     0014(A5),A0
       | 1E1A  2240                           MOVEA.L     D0,A1
       | 1E1C  2368 002C 0018                 MOVE.L      002C(A0),0018(A1)
;1669:             ncl->CLock_ServerLock = 0L;
       | 1E22  42A9 0014                      CLR.L       0014(A1)
;1670:             ncl->CLock_FileLock.fl_Access = SHARED_LOCK;
       | 1E26  72FE                           MOVEQ       #FE,D1
       | 1E28  2341 0008                      MOVE.L      D1,0008(A1)
;1671: 
;1672:             ncl->CLock_FileLock.fl_Volume = MKBADDR(m->mfsi_Volume);
       | 1E2C  2228 0034                      MOVE.L      0034(A0),D1
       | 1E30  E481                           ASR.L       #2,D1
       | 1E32  2341 0010                      MOVE.L      D1,0010(A1)
;1673:             ncl->CLock_FileLock.fl_Task = m->mfsi_LocalPort;
       | 1E36  2368 0030 000C                 MOVE.L      0030(A0),000C(A1)
;1674:             dp->dp_Res1 = (LONG) MKBADDR(ncl);
       | 1E3C  2200                           MOVE.L      D0,D1
       | 1E3E  E481                           ASR.L       #2,D1
       | 1E40  2C6D 0010                      MOVEA.L     0010(A5),A6
       | 1E44  2D41 000C                      MOVE.L      D1,000C(A6)
;1675:             ncl->CLock_FullServerPath = AllocMem(1,MEMF_PUBLIC);
       | 1E48  7001                           MOVEQ       #01,D0
       | 1E4A  2200                           MOVE.L      D0,D1
       | 1E4C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1E52  4EAE FF3A                      JSR         FF3A(A6)
       | 1E56  206D FFF0                      MOVEA.L     FFF0(A5),A0
       | 1E5A  2140 001C                      MOVE.L      D0,001C(A0)
;1676:             if (!ncl->CLock_FullServerPath)
       | 1E5E  6610                           BNE.B       1E70
;1677:             {
;1678:                 FreeMem(ncl,sizeof(struct CLock));
       | 1E60  2248                           MOVEA.L     A0,A1
       | 1E62  7020                           MOVEQ       #20,D0
       | 1E64  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1E6A  4EAE FF2E                      JSR         FF2E(A6)
;1679:             }
;1680:             else
       | 1E6E  601A                           BRA.B       1E8A
;1681:             {
;1682:                 ((STRPTR)ncl->CLock_FullServerPath)[0]=0;
       | 1E70  2040                           MOVEA.L     D0,A0
       | 1E72  4210                           CLR.B       (A0)
;1683:                 KeepLock((APTR)dp->dp_Res1,m);
       | 1E74  226D 0010                      MOVEA.L     0010(A5),A1
       | 1E78  2069 000C                      MOVEA.L     000C(A1),A0
       | 1E7C  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 1E80  2F08                           MOVE.L      A0,-(A7)
       | 1E82  6100 E9E6                      BSR.W       086A
;1684: 
;1685:                 return(FALSE);
       | 1E86  7000                           MOVEQ       #00,D0
       | 1E88  6002                           BRA.B       1E8C
;1686:             }
;1687:         }
;1688:     }
;1689: 
;1690:     return(TRUE);
       | 1E8A  7001                           MOVEQ       #01,D0
;1691: }
       | 1E8C  4CED 4C84 FFDC                 MOVEM.L     FFDC(A5),D2/D7/A2-A3/A6
       | 1E92  4E5D                           UNLK        A5
       | 1E94  4E75                           RTS
;1692: 
;1693: 
;1694: /* StartExamine
;1695:  *
;1696:  * ACTION_Examine, ExNext
;1697:  *
;1698:  */
;1699: BOOL StartExamine(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1700:                   struct MountedFSInfoClient *m)
;1701: {
       | 1E96  4E55 FFF8                      LINK        A5,#FFF8
       | 1E9A  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 1E9E  266D 0008                      MOVEA.L     0008(A5),A3
       | 1EA2  246D 000C                      MOVEA.L     000C(A5),A2
;1702:     UBYTE *gp;
;1703:     struct CLock *cl;
;1704: 
;1705: /* The given lock may be old */
;1706:     cl = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1EA6  206D 0010                      MOVEA.L     0010(A5),A0
       | 1EAA  2028 0014                      MOVE.L      0014(A0),D0
       | 1EAE  2200                           MOVE.L      D0,D1
       | 1EB0  E581                           ASL.L       #2,D1
;1707: 
;1708:     if (dp->dp_Arg1)
       | 1EB2  2B41 FFF8                      MOVE.L      D1,FFF8(A5)
       | 1EB6  4A80                           TST.L       D0
       | 1EB8  672E                           BEQ.B       1EE8
;1709:     {
;1710:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1EBA  2041                           MOVEA.L     D1,A0
       | 1EBC  2028 0018                      MOVE.L      0018(A0),D0
       | 1EC0  226D 0014                      MOVEA.L     0014(A5),A1
       | 1EC4  B0A9 002C                      CMP.L       002C(A1),D0
       | 1EC8  6416                           BCC.B       1EE0
;1711:         {
;1712:             dp->dp_Res1 = DOSFALSE;
       | 1ECA  7000                           MOVEQ       #00,D0
       | 1ECC  226D 0010                      MOVEA.L     0010(A5),A1
       | 1ED0  2340 000C                      MOVE.L      D0,000C(A1)
;1713:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 1ED4  237C 0000 00D3 0010            MOVE.L      #000000D3,0010(A1)
;1714:             return(FALSE);
       | 1EDC  7000                           MOVEQ       #00,D0
       | 1EDE  6078                           BRA.B       1F58
;1715:         }
;1716:         tp->tp_Arg1 = (LONG) cl->CLock_ServerLock;
       | 1EE0  2568 0014 0014                 MOVE.L      0014(A0),0014(A2)
;1717:     }
;1718:     else
       | 1EE6  6004                           BRA.B       1EEC
;1719:         tp->tp_Arg1 = 0L;
       | 1EE8  42AA 0014                      CLR.L       0014(A2)
;1720: 
;1721:     gp = &(((UBYTE *) t->trans_RequestData)[sizeof(struct TPacket)]);
       | 1EEC  206B 002A                      MOVEA.L     002A(A3),A0
       | 1EF0  D0FC 0034                      ADDA.W      #0034,A0
       | 1EF4  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
;1722: 
;1723:     while ((ULONG) gp & 3L)
       | 1EF8  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1EFC  2200                           MOVE.L      D0,D1
       | 1EFE  7403                           MOVEQ       #03,D2
       | 1F00  C282                           AND.L       D2,D1
       | 1F02  6706                           BEQ.B       1F0A
;1724:         gp++;
       | 1F04  52AD FFFC                      ADDQ.L      #1,FFFC(A5)
       | 1F08  60EE                           BRA.B       1EF8
;1725: 
;1726:     tp->tp_Arg2 = ((ULONG) gp - (ULONG) tp);
       | 1F0A  202D FFFC                      MOVE.L      FFFC(A5),D0
       | 1F0E  220A                           MOVE.L      A2,D1
       | 1F10  2400                           MOVE.L      D0,D2
       | 1F12  9481                           SUB.L       D1,D2
       | 1F14  2542 0018                      MOVE.L      D2,0018(A2)
;1727:     tp->tp_Arg4 = (m->mfsi_Flags & FLAGSMASK);
       | 1F18  7203                           MOVEQ       #03,D1
       | 1F1A  206D 0014                      MOVEA.L     0014(A5),A0
       | 1F1E  C2A8 0054                      AND.L       0054(A0),D1
       | 1F22  2541 0020                      MOVE.L      D1,0020(A2)
;1728: 
;1729:     CopyMem(BADDR(dp->dp_Arg2),gp,sizeof(struct FileInfoBlock));
       | 1F26  206D 0010                      MOVEA.L     0010(A5),A0
       | 1F2A  2228 0018                      MOVE.L      0018(A0),D1
       | 1F2E  E581                           ASL.L       #2,D1
       | 1F30  2041                           MOVEA.L     D1,A0
       | 1F32  2240                           MOVEA.L     D0,A1
       | 1F34  203C 0000 0104                 MOVE.L      #00000104,D0
       | 1F3A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1F40  4EAE FD90                      JSR         FD90(A6)
;1730: 
;1731:     t->trans_ReqDataActual =  (ULONG)(&gp[sizeof(struct FileInfoBlock)]) - (ULONG) tp;
       | 1F44  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 1F48  D0FC 0104                      ADDA.W      #0104,A0
       | 1F4C  200A                           MOVE.L      A2,D0
       | 1F4E  2208                           MOVE.L      A0,D1
       | 1F50  9280                           SUB.L       D0,D1
       | 1F52  2741 0032                      MOVE.L      D1,0032(A3)
;1732: 
;1733:     return(TRUE);
       | 1F56  7001                           MOVEQ       #01,D0
;1734: }
       | 1F58  4CDF 4C04                      MOVEM.L     (A7)+,D2/A2-A3/A6
       | 1F5C  4E5D                           UNLK        A5
       | 1F5E  4E75                           RTS
;1735: 
;1736: 
;1737: /*
;1738:  *  EndExamine
;1739:  *
;1740:  */
;1741: 
;1742: void EndExamine(struct TPacket *tp, struct MountedFSInfoClient *m)
;1743: {
       | 1F60  4E55 FFF8                      LINK        A5,#FFF8
       | 1F64  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
;1744:     struct DosPacket *dp=tp->tp_DosPacket;
       | 1F68  206D 0008                      MOVEA.L     0008(A5),A0
       | 1F6C  2668 0004                      MOVEA.L     0004(A0),A3
;1745: 
;1746:     if (tp->tp_Res1)                /* If sucessful */
       | 1F70  4AA8 000C                      TST.L       000C(A0)
       | 1F74  672A                           BEQ.B       1FA0
;1747:     {
;1748:         UBYTE *efrom;
;1749:         efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
       | 1F76  45E8 0034                      LEA         0034(A0),A2
;1750:         while ((ULONG) efrom & 3L)
       | 1F7A  200A                           MOVE.L      A2,D0
       | 1F7C  7203                           MOVEQ       #03,D1
       | 1F7E  C081                           AND.L       D1,D0
       | 1F80  6704                           BEQ.B       1F86
;1751:             efrom++;
       | 1F82  528A                           ADDQ.L      #1,A2
       | 1F84  60F4                           BRA.B       1F7A
;1752: 
;1753:         CopyMem(efrom,BADDR(dp->dp_Arg2),sizeof(struct FileInfoBlock));
       | 1F86  202B 0018                      MOVE.L      0018(A3),D0
       | 1F8A  E580                           ASL.L       #2,D0
       | 1F8C  2240                           MOVEA.L     D0,A1
       | 1F8E  204A                           MOVEA.L     A2,A0
       | 1F90  203C 0000 0104                 MOVE.L      #00000104,D0
       | 1F96  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 1F9C  4EAE FD90                      JSR         FD90(A6)
;1754:     }
;1755: 
;1756:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 1FA0  206D 0008                      MOVEA.L     0008(A5),A0
       | 1FA4  2F28 0010                      MOVE.L      0010(A0),-(A7)
       | 1FA8  2F28 000C                      MOVE.L      000C(A0),-(A7)
       | 1FAC  2F0B                           MOVE.L      A3,-(A7)
       | 1FAE  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1757: }
       | 1FB2  4CED 4C00 FFEC                 MOVEM.L     FFEC(A5),A2-A3/A6
       | 1FB8  4E5D                           UNLK        A5
       | 1FBA  4E75                           RTS
;1758: 
;1759: 
;1760: /* StartSameLock
;1761:  *
;1762:  * ACTION_SAME_LOCK
;1763:  *
;1764:  */
;1765: BOOL StartSameLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1766:                    struct MountedFSInfoClient *m)
;1767: {
       | 1FBC  4E55 FFFC                      LINK        A5,#FFFC
       | 1FC0  48E7 2030                      MOVEM.L     D2/A2-A3,-(A7)
       | 1FC4  266D 000C                      MOVEA.L     000C(A5),A3
       | 1FC8  246D 0010                      MOVEA.L     0010(A5),A2
;1768: 
;1769:     /* Either of the two given locks may be old, and invalid.  Check.  */
;1770:     /* If not a root lock, fill in the TPacket version */
;1771: 
;1772:     /* Src lock */
;1773:     if (dp->dp_Arg1)
       | 1FCC  202A 0014                      MOVE.L      0014(A2),D0
       | 1FD0  6732                           BEQ.B       2004
;1774:     {
;1775:         struct CLock *ol;
;1776:         ol = (struct CLock *) BADDR(dp->dp_Arg1);
       | 1FD2  E580                           ASL.L       #2,D0
;1777:         if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 1FD4  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 1FD8  206D 0014                      MOVEA.L     0014(A5),A0
       | 1FDC  2228 002C                      MOVE.L      002C(A0),D1
       | 1FE0  2040                           MOVEA.L     D0,A0
       | 1FE2  2428 0018                      MOVE.L      0018(A0),D2
       | 1FE6  B481                           CMP.L       D1,D2
       | 1FE8  6412                           BCC.B       1FFC
;1778:         {
;1779:             dp->dp_Res1 = DOSFALSE;
       | 1FEA  7400                           MOVEQ       #00,D2
       | 1FEC  2542 000C                      MOVE.L      D2,000C(A2)
;1780:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 1FF0  722C                           MOVEQ       #2C,D1
       | 1FF2  4601                           NOT.B       D1
       | 1FF4  2541 0010                      MOVE.L      D1,0010(A2)
;1781:             return (FALSE);
       | 1FF8  7000                           MOVEQ       #00,D0
       | 1FFA  604A                           BRA.B       2046
;1782:         }
;1783:         tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
       | 1FFC  2768 0014 0014                 MOVE.L      0014(A0),0014(A3)
;1784:     }
;1785:     else
       | 2002  6006                           BRA.B       200A
;1786:         tp->tp_Arg1 = 0L;
       | 2004  7000                           MOVEQ       #00,D0
       | 2006  2740 0014                      MOVE.L      D0,0014(A3)
;1787: 
;1788: 
;1789:     /* Dst Lock */
;1790:     if (dp->dp_Arg2)
       | 200A  202A 0018                      MOVE.L      0018(A2),D0
       | 200E  6730                           BEQ.B       2040
;1791:     {
;1792:         struct CLock *ol;
;1793:         ol = (struct CLock *) BADDR(dp->dp_Arg2);
       | 2010  E580                           ASL.L       #2,D0
;1794:         if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 2012  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 2016  2040                           MOVEA.L     D0,A0
       | 2018  2228 0018                      MOVE.L      0018(A0),D1
       | 201C  226D 0014                      MOVEA.L     0014(A5),A1
       | 2020  B2A9 002C                      CMP.L       002C(A1),D1
       | 2024  6412                           BCC.B       2038
;1795:         {
;1796:             dp->dp_Res1 = DOSFALSE;
       | 2026  7200                           MOVEQ       #00,D1
       | 2028  2541 000C                      MOVE.L      D1,000C(A2)
;1797:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 202C  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;1798:             return (FALSE);
       | 2034  7000                           MOVEQ       #00,D0
       | 2036  600E                           BRA.B       2046
;1799:         }
;1800:         tp->tp_Arg2 = (LONG) ol->CLock_ServerLock;
       | 2038  2768 0014 0018                 MOVE.L      0014(A0),0018(A3)
;1801:     }
;1802:     else
       | 203E  6004                           BRA.B       2044
;1803:         tp->tp_Arg2 = 0L;
       | 2040  42AB 0018                      CLR.L       0018(A3)
;1804: 
;1805:     return(TRUE);
       | 2044  7001                           MOVEQ       #01,D0
;1806: }
       | 2046  4CDF 0C04                      MOVEM.L     (A7)+,D2/A2-A3
       | 204A  4E5D                           UNLK        A5
       | 204C  4E75                           RTS
;1807: 
;1808: 
;1809: /* StartInfo
;1810:  *
;1811:  * ACTION_INFO
;1812:  *
;1813:  */
;1814: BOOL StartInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1815:                    struct MountedFSInfoClient *m)
;1816: {
       | 204E  4E55 FFFC                      LINK        A5,#FFFC
       | 2052  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 2056  266D 000C                      MOVEA.L     000C(A5),A3
       | 205A  246D 0010                      MOVEA.L     0010(A5),A2
;1817: 
;1818:     /* Either of the two given locks may be old, and invalid.  Check.  */
;1819:     /* If not a root lock, fill in the TPacket version */
;1820: 
;1821:     /* Src lock */
;1822:     if (dp->dp_Arg1)
       | 205E  202A 0014                      MOVE.L      0014(A2),D0
       | 2062  6730                           BEQ.B       2094
;1823:     {
;1824:         struct CLock *ol;
;1825:         ol = (struct CLock *) BADDR(dp->dp_Arg1);
       | 2064  E580                           ASL.L       #2,D0
;1826:         if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 2066  2B40 FFFC                      MOVE.L      D0,FFFC(A5)
       | 206A  2040                           MOVEA.L     D0,A0
       | 206C  2228 0018                      MOVE.L      0018(A0),D1
       | 2070  226D 0014                      MOVEA.L     0014(A5),A1
       | 2074  B2A9 002C                      CMP.L       002C(A1),D1
       | 2078  6412                           BCC.B       208C
;1827:         {
;1828:             dp->dp_Res1 = DOSFALSE;
       | 207A  7200                           MOVEQ       #00,D1
       | 207C  2541 000C                      MOVE.L      D1,000C(A2)
;1829:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 2080  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;1830:             return (FALSE);
       | 2088  7000                           MOVEQ       #00,D0
       | 208A  6014                           BRA.B       20A0
;1831:         }
;1832:         tp->tp_Arg1 = (LONG) ol->CLock_ServerLock;
       | 208C  2768 0014 0014                 MOVE.L      0014(A0),0014(A3)
;1833:     }
;1834:     else
       | 2092  6004                           BRA.B       2098
;1835:         tp->tp_Arg1 = 0L;
       | 2094  42AB 0014                      CLR.L       0014(A3)
;1836: 
;1837:     tp->tp_Arg2 = sizeof(struct TPacket);
       | 2098  7034                           MOVEQ       #34,D0
       | 209A  2740 0018                      MOVE.L      D0,0018(A3)
;1838: 
;1839:     return(TRUE);
       | 209E  7001                           MOVEQ       #01,D0
;1840: }
       | 20A0  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 20A4  4E5D                           UNLK        A5
       | 20A6  4E75                           RTS
;1841: 
;1842: /* StartChangeMode
;1843:  *
;1844:  * ACTION_CHANGE_MODE
;1845:  *
;1846:  */
;1847: BOOL StartChangeMode(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1848:                      struct MountedFSInfoClient *m)
;1849: {
       | 20A8  4E55 FFFC                      LINK        A5,#FFFC
       | 20AC  48E7 3830                      MOVEM.L     D2-D4/A2-A3,-(A7)
       | 20B0  266D 000C                      MOVEA.L     000C(A5),A3
       | 20B4  246D 0010                      MOVEA.L     0010(A5),A2
;1850: 
;1851:     /* Either of the two given locks may be old, and invalid.  Check.  */
;1852:     /* If not a root lock, fill in the TPacket version */
;1853: 
;1854:     if (dp->dp_Arg1 == CHANGE_LOCK)
       | 20B8  202A 0014                      MOVE.L      0014(A2),D0
       | 20BC  6638                           BNE.B       20F6
;1855:     {
;1856:         /* Src lock */
;1857:         if (dp->dp_Arg2)
       | 20BE  222A 0018                      MOVE.L      0018(A2),D1
       | 20C2  6766                           BEQ.B       212A
;1858:         {
;1859:             struct CLock *ol;
;1860:             ol = (struct CLock *) BADDR(dp->dp_Arg2);
       | 20C4  2401                           MOVE.L      D1,D2
       | 20C6  E582                           ASL.L       #2,D2
;1861:             if (ol->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 20C8  2B42 FFFC                      MOVE.L      D2,FFFC(A5)
       | 20CC  206D 0014                      MOVEA.L     0014(A5),A0
       | 20D0  2628 002C                      MOVE.L      002C(A0),D3
       | 20D4  2042                           MOVEA.L     D2,A0
       | 20D6  2828 0018                      MOVE.L      0018(A0),D4
       | 20DA  B883                           CMP.L       D3,D4
       | 20DC  6410                           BCC.B       20EE
;1862:             {
;1863:                 dp->dp_Res1 = DOSFALSE;
       | 20DE  42AA 000C                      CLR.L       000C(A2)
;1864:                 dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 20E2  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;1865:                 return (FALSE);
       | 20EA  7000                           MOVEQ       #00,D0
       | 20EC  603E                           BRA.B       212C
;1866:             }
;1867:             tp->tp_Arg2 = (LONG) ol->CLock_ServerLock;
       | 20EE  2768 0014 0018                 MOVE.L      0014(A0),0018(A3)
;1868:         }
;1869:     }
;1870:     else if (dp->dp_Arg1 == CHANGE_FH)
       | 20F4  6034                           BRA.B       212A
       | 20F6  5380                           SUBQ.L      #1,D0
       | 20F8  6630                           BNE.B       212A
;1871:     {
;1872:         struct CFH *cf;
;1873:         cf = (struct CFH *) dp->dp_Arg2;
       | 20FA  2B6A 0018 FFFC                 MOVE.L      0018(A2),FFFC(A5)
;1874: 
;1875:         /* The possibility of an old, unreincarnated fh exists. */
;1876: 
;1877:         if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 2100  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 2104  2028 0010                      MOVE.L      0010(A0),D0
       | 2108  226D 0014                      MOVEA.L     0014(A5),A1
       | 210C  B0A9 002C                      CMP.L       002C(A1),D0
       | 2110  6412                           BCC.B       2124
;1878:         {
;1879:             dp->dp_Res1 = DOSTRUE;
       | 2112  70FF                           MOVEQ       #FF,D0
       | 2114  2540 000C                      MOVE.L      D0,000C(A2)
;1880:             dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 2118  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
;1881:             return(FALSE);
       | 2120  7000                           MOVEQ       #00,D0
       | 2122  6008                           BRA.B       212C
;1882:         }
;1883:         tp->tp_Arg2 = (LONG)cf->CFH_ServerFH;
       | 2124  2768 0004 0018                 MOVE.L      0004(A0),0018(A3)
;1884:     }
;1885: 
;1886:     return(TRUE);
       | 212A  7001                           MOVEQ       #01,D0
;1887: }
       | 212C  4CDF 0C1C                      MOVEM.L     (A7)+,D2-D4/A2-A3
       | 2130  4E5D                           UNLK        A5
       | 2132  4E75                           RTS
;1888: 
;1889: 
;1890: 
;1891: /*
;1892:  *  EndInfo
;1893:  *
;1894:  */
;1895: 
;1896: void EndInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
;1897: {
       | 2134  4E55 FFF4                      LINK        A5,#FFF4
       | 2138  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 213C  266D 000C                      MOVEA.L     000C(A5),A3
;1898:     struct DosPacket *dp=tp->tp_DosPacket;
       | 2140  206D 0008                      MOVEA.L     0008(A5),A0
       | 2144  2468 0004                      MOVEA.L     0004(A0),A2
;1899: 
;1900:     if (tp->tp_Res1)                /* If sucessful */
       | 2148  4AA8 000C                      TST.L       000C(A0)
       | 214C  6750                           BEQ.B       219E
;1901:     {
;1902:         UBYTE *efrom;
;1903:         struct InfoData *i;
;1904:         efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
       | 214E  43E8 0034                      LEA         0034(A0),A1
       | 2152  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
;1905:         while ((ULONG) efrom & 3L)
       | 2156  202D FFF8                      MOVE.L      FFF8(A5),D0
       | 215A  2200                           MOVE.L      D0,D1
       | 215C  7403                           MOVEQ       #03,D2
       | 215E  C282                           AND.L       D2,D1
       | 2160  6706                           BEQ.B       2168
;1906:             efrom++;
       | 2162  52AD FFF8                      ADDQ.L      #1,FFF8(A5)
       | 2166  60EE                           BRA.B       2156
;1907: 
;1908:         CopyMem(efrom,BADDR(dp->dp_Arg2),sizeof(struct InfoData));
       | 2168  202A 0018                      MOVE.L      0018(A2),D0
       | 216C  E580                           ASL.L       #2,D0
       | 216E  2240                           MOVEA.L     D0,A1
       | 2170  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 2174  7024                           MOVEQ       #24,D0
       | 2176  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 217C  4EAE FD90                      JSR         FD90(A6)
;1909:         i = (struct InfoData *) BADDR(dp->dp_Arg2);
       | 2180  202A 0018                      MOVE.L      0018(A2),D0
       | 2184  E580                           ASL.L       #2,D0
;1910:         i->id_DiskType = ID_FFS_DISK;
       | 2186  2040                           MOVEA.L     D0,A0
       | 2188  217C 444F 5301 0018            MOVE.L      #444F5301,0018(A0)
;1911:         i->id_VolumeNode = MKBADDR(m->mfsi_Volume);
       | 2190  222B 0034                      MOVE.L      0034(A3),D1
       | 2194  E481                           ASR.L       #2,D1
       | 2196  2141 001C                      MOVE.L      D1,001C(A0)
;1912:     }
       | 219A  2B40 FFF4                      MOVE.L      D0,FFF4(A5)
;1913: 
;1914:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 219E  206D 0008                      MOVEA.L     0008(A5),A0
       | 21A2  2F28 0010                      MOVE.L      0010(A0),-(A7)
       | 21A6  2F28 000C                      MOVE.L      000C(A0),-(A7)
       | 21AA  2F0A                           MOVE.L      A2,-(A7)
       | 21AC  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1915: }
       | 21B0  4CED 4C04 FFE4                 MOVEM.L     FFE4(A5),D2/A2-A3/A6
       | 21B6  4E5D                           UNLK        A5
       | 21B8  4E75                           RTS
;1916: 
;1917: 
;1918: 
;1919: 
;1920: /* StartFlush
;1921:  *
;1922:  * ACTION_FLUSH
;1923:  *
;1924:  */
;1925: BOOL StartFlush(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1926:                 struct MountedFSInfoClient *m)
;1927: {
       | 21BA  4E55 0000                      LINK        A5,#0000
;1928:     return(TRUE);
       | 21BE  7001                           MOVEQ       #01,D0
;1929: }
       | 21C0  4E5D                           UNLK        A5
       | 21C2  4E75                           RTS
;1930: 
;1931: 
;1932: 
;1933: 
;1934: /* StartDiskInfo
;1935:  *
;1936:  * ACTION_DISK_INFO
;1937:  *
;1938:  */
;1939: BOOL StartDiskInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1940:                    struct MountedFSInfoClient *m)
;1941: {
       | 21C4  4E55 0000                      LINK        A5,#0000
       | 21C8  2F0B                           MOVE.L      A3,-(A7)
       | 21CA  266D 000C                      MOVEA.L     000C(A5),A3
;1942: 
;1943:     tp->tp_Arg1 = sizeof(struct TPacket);
       | 21CE  7034                           MOVEQ       #34,D0
       | 21D0  2740 0014                      MOVE.L      D0,0014(A3)
;1944: 
;1945:     return(TRUE);
       | 21D4  7001                           MOVEQ       #01,D0
;1946: }
       | 21D6  265F                           MOVEA.L     (A7)+,A3
       | 21D8  4E5D                           UNLK        A5
       | 21DA  4E75                           RTS
;1947: 
;1948: 
;1949: BOOL StartToID(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1950:                struct MountedFSInfoClient *m)
;1951: {
       | 21DC  4E55 FFFC                      LINK        A5,#FFFC
       | 21E0  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 21E4  266D 000C                      MOVEA.L     000C(A5),A3
       | 21E8  246D 0010                      MOVEA.L     0010(A5),A2
;1952:     STRPTR w;
;1953:     tp->tp_Arg1 = sizeof(struct TPacket);
       | 21EC  7034                           MOVEQ       #34,D0
       | 21EE  2740 0014                      MOVE.L      D0,0014(A3)
;1954:     w = (STRPTR) ((ULONG)(tp) + sizeof(struct TPacket));
       | 21F2  220B                           MOVE.L      A3,D1
       | 21F4  D280                           ADD.L       D0,D1
;1955:     strncpy(w,dp->dp_Arg1,33);
       | 21F6  4878 0021                      PEA         0021
       | 21FA  2F2A 0014                      MOVE.L      0014(A2),-(A7)
       | 21FE  2F01                           MOVE.L      D1,-(A7)
       | 2200  2B41 FFFC                      MOVE.L      D1,FFFC(A5)
       | 2204  4EBA  0000-XX.1                JSR         _strncpy(PC)
;1956:     return(TRUE);
       | 2208  7001                           MOVEQ       #01,D0
;1957: }
       | 220A  4CED 0C00 FFF4                 MOVEM.L     FFF4(A5),A2-A3
       | 2210  4E5D                           UNLK        A5
       | 2212  4E75                           RTS
;1958: 
;1959: BOOL StartToInfo(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;1960:                  struct MountedFSInfoClient *m)
;1961: {
       | 2214  4E55 0000                      LINK        A5,#0000
       | 2218  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 221C  266D 000C                      MOVEA.L     000C(A5),A3
       | 2220  246D 0010                      MOVEA.L     0010(A5),A2
;1962: 
;1963:     tp->tp_Arg1 = dp->dp_Arg1;
       | 2224  276A 0014 0014                 MOVE.L      0014(A2),0014(A3)
;1964:     tp->tp_Arg2 = sizeof(struct TPacket);
       | 222A  7034                           MOVEQ       #34,D0
       | 222C  2740 0018                      MOVE.L      D0,0018(A3)
;1965:     return(TRUE);
       | 2230  7001                           MOVEQ       #01,D0
;1966: }
       | 2232  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 2236  4E5D                           UNLK        A5
       | 2238  4E75                           RTS
;1967: 
;1968: void EndToInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
;1969: {
       | 223A  4E55 FFF0                      LINK        A5,#FFF0
       | 223E  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 2242  266D 0008                      MOVEA.L     0008(A5),A3
;1970:     struct DosPacket *dp=tp->tp_DosPacket;
       | 2246  246B 0004                      MOVEA.L     0004(A3),A2
;1971: 
;1972:     if (tp->tp_Res1)
       | 224A  4AAB 000C                      TST.L       000C(A3)
       | 224E  6736                           BEQ.B       2286
;1973:     {
;1974:         APTR q;
;1975:         ULONG sz;
;1976:         q = (APTR) ((ULONG) tp + sizeof(struct TPacket));
       | 2250  200B                           MOVE.L      A3,D0
       | 2252  7234                           MOVEQ       #34,D1
       | 2254  D081                           ADD.L       D1,D0
;1977:         if (dp->dp_Type == ACTION_UID_TO_USERINFO)
       | 2256  48ED 0001 FFF8                 MOVEM.L     D0,FFF8(A5)
       | 225C  0CAA 0000 4E22 0008            CMPI.L      #00004E22,0008(A2)
       | 2264  6604                           BNE.B       226A
;1978:             sz = sizeof(struct UserInfo);
       | 2266  7E28                           MOVEQ       #28,D7
;1979:         else
       | 2268  6002                           BRA.B       226C
;1980:             sz = sizeof(struct GroupInfo);
       | 226A  7E22                           MOVEQ       #22,D7
;1981: 
;1982:         CopyMem(q,(APTR)dp->dp_Arg2,sz);
       | 226C  206A 0018                      MOVEA.L     0018(A2),A0
       | 2270  2F48 0010                      MOVE.L      A0,0010(A7)
       | 2274  2040                           MOVEA.L     D0,A0
       | 2276  226F 0010                      MOVEA.L     0010(A7),A1
       | 227A  2007                           MOVE.L      D7,D0
       | 227C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2282  4EAE FD90                      JSR         FD90(A6)
;1983:     }
;1984:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 2286  2F2B 0010                      MOVE.L      0010(A3),-(A7)
       | 228A  2F2B 000C                      MOVE.L      000C(A3),-(A7)
       | 228E  2F0A                           MOVE.L      A2,-(A7)
       | 2290  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;1985: 
;1986: }
       | 2294  4CED 4C80 FFE0                 MOVEM.L     FFE0(A5),D7/A2-A3/A6
       | 229A  4E5D                           UNLK        A5
       | 229C  4E75                           RTS
;1987: 
;1988: 
;1989: /*
;1990:  *  EndDiskInfo
;1991:  *
;1992:  */
;1993: 
;1994: void EndDiskInfo(struct TPacket *tp, struct MountedFSInfoClient *m)
;1995: {
       | 229E  4E55 FFF4                      LINK        A5,#FFF4
       | 22A2  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 22A6  266D 000C                      MOVEA.L     000C(A5),A3
;1996:     struct DosPacket *dp=tp->tp_DosPacket;
       | 22AA  206D 0008                      MOVEA.L     0008(A5),A0
       | 22AE  2468 0004                      MOVEA.L     0004(A0),A2
;1997: 
;1998:     if (tp->tp_Res1)                /* If sucessful */
       | 22B2  4AA8 000C                      TST.L       000C(A0)
       | 22B6  6750                           BEQ.B       2308
;1999:     {
;2000:         UBYTE *efrom;
;2001:         struct InfoData *i;
;2002:         efrom = &((UBYTE *)tp)[sizeof(struct TPacket)];
       | 22B8  43E8 0034                      LEA         0034(A0),A1
       | 22BC  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
;2003:         while ((ULONG) efrom & 3L)
       | 22C0  202D FFF8                      MOVE.L      FFF8(A5),D0
       | 22C4  2200                           MOVE.L      D0,D1
       | 22C6  7403                           MOVEQ       #03,D2
       | 22C8  C282                           AND.L       D2,D1
       | 22CA  6706                           BEQ.B       22D2
;2004:             efrom++;
       | 22CC  52AD FFF8                      ADDQ.L      #1,FFF8(A5)
       | 22D0  60EE                           BRA.B       22C0
;2005: 
;2006:         CopyMem(efrom,BADDR(dp->dp_Arg1),sizeof(struct InfoData));
       | 22D2  202A 0014                      MOVE.L      0014(A2),D0
       | 22D6  E580                           ASL.L       #2,D0
       | 22D8  2240                           MOVEA.L     D0,A1
       | 22DA  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 22DE  7024                           MOVEQ       #24,D0
       | 22E0  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 22E6  4EAE FD90                      JSR         FD90(A6)
;2007:         i = (struct InfoData *) BADDR(dp->dp_Arg1);
       | 22EA  202A 0014                      MOVE.L      0014(A2),D0
       | 22EE  E580                           ASL.L       #2,D0
;2008:         i->id_VolumeNode = MKBADDR(m->mfsi_Volume);
       | 22F0  222B 0034                      MOVE.L      0034(A3),D1
       | 22F4  E481                           ASR.L       #2,D1
       | 22F6  2040                           MOVEA.L     D0,A0
       | 22F8  2141 001C                      MOVE.L      D1,001C(A0)
;2009:         i->id_DiskType = ID_FFS_DISK;
       | 22FC  217C 444F 5301 0018            MOVE.L      #444F5301,0018(A0)
;2010:     }
       | 2304  2B40 FFF4                      MOVE.L      D0,FFF4(A5)
;2011: 
;2012:     ReplyPkt(dp,tp->tp_Res1,tp->tp_Res2);
       | 2308  206D 0008                      MOVEA.L     0008(A5),A0
       | 230C  2F28 0010                      MOVE.L      0010(A0),-(A7)
       | 2310  2F28 000C                      MOVE.L      000C(A0),-(A7)
       | 2314  2F0A                           MOVE.L      A2,-(A7)
       | 2316  4EBA  0000-XX.1                JSR         _ReplyPkt(PC)
;2013: }
       | 231A  4CED 4C04 FFE4                 MOVEM.L     FFE4(A5),D2/A2-A3/A6
       | 2320  4E5D                           UNLK        A5
       | 2322  4E75                           RTS
;2014: 
;2015: /*
;2016:  *  StartOpenFromLock
;2017:  *
;2018:  */
;2019: 
;2020: BOOL StartOpenFromLock(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;2021:                        struct MountedFSInfoClient *m)
;2022: {
       | 2324  4E55 FFFC                      LINK        A5,#FFFC
       | 2328  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 232C  266D 000C                      MOVEA.L     000C(A5),A3
       | 2330  246D 0010                      MOVEA.L     0010(A5),A2
;2023: 
;2024:     struct CLock *cl;
;2025: 
;2026: /* The given lock may be old */
;2027:     cl = (struct CLock *) BADDR(dp->dp_Arg2);
       | 2334  202A 0018                      MOVE.L      0018(A2),D0
       | 2338  2200                           MOVE.L      D0,D1
       | 233A  E581                           ASL.L       #2,D1
;2028: 
;2029:     if (dp->dp_Arg2)
       | 233C  2B41 FFFC                      MOVE.L      D1,FFFC(A5)
       | 2340  4A80                           TST.L       D0
       | 2342  672A                           BEQ.B       236E
;2030:     {
;2031:         if (cl->CLock_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 2344  2041                           MOVEA.L     D1,A0
       | 2346  2028 0018                      MOVE.L      0018(A0),D0
       | 234A  226D 0014                      MOVEA.L     0014(A5),A1
       | 234E  B0A9 002C                      CMP.L       002C(A1),D0
       | 2352  6412                           BCC.B       2366
;2032:         {
;2033:             dp->dp_Res1 = DOSFALSE;
       | 2354  7000                           MOVEQ       #00,D0
       | 2356  2540 000C                      MOVE.L      D0,000C(A2)
;2034:             dp->dp_Res2 = ERROR_INVALID_LOCK;
       | 235A  257C 0000 00D3 0010            MOVE.L      #000000D3,0010(A2)
;2035:             return(FALSE);
       | 2362  7000                           MOVEQ       #00,D0
       | 2364  600E                           BRA.B       2374
;2036:         }
;2037:         tp->tp_Arg2 = (LONG) cl->CLock_ServerLock;
       | 2366  2768 0014 0018                 MOVE.L      0014(A0),0018(A3)
;2038:     }
;2039:     else
       | 236C  6004                           BRA.B       2372
;2040:         tp->tp_Arg2 = 0L;
       | 236E  42AB 0018                      CLR.L       0018(A3)
;2041: 
;2042: 
;2043:     return(TRUE);
       | 2372  7001                           MOVEQ       #01,D0
;2044: }
       | 2374  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 2378  4E5D                           UNLK        A5
       | 237A  4E75                           RTS
;2045: 
;2046: /*
;2047:  * StartLockFromFH
;2048:  *
;2049:  * ACTION_COPY_DIR_FH
;2050:  *
;2051:  */
;2052: BOOL StartLockFromFH(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;2053:                      struct MountedFSInfoClient *m)
;2054: {
       | 237C  4E55 FFFC                      LINK        A5,#FFFC
       | 2380  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 2384  266D 000C                      MOVEA.L     000C(A5),A3
       | 2388  246D 0010                      MOVEA.L     0010(A5),A2
;2055:     struct CFH *cf;
;2056: 
;2057:     cf = (struct CFH *) dp->dp_Arg1;
       | 238C  2B6A 0014 FFFC                 MOVE.L      0014(A2),FFFC(A5)
;2058: 
;2059:     /* The possibility of an old, unreincarnated fh exists. */
;2060: 
;2061:     if (cf->CFH_ConnectionNumber < m->mfsi_ConnectionNumber)
       | 2392  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 2396  2028 0010                      MOVE.L      0010(A0),D0
       | 239A  226D 0014                      MOVEA.L     0014(A5),A1
       | 239E  B0A9 002C                      CMP.L       002C(A1),D0
       | 23A2  6412                           BCC.B       23B6
;2062:     {
;2063:         dp->dp_Res1 = DOSTRUE;
       | 23A4  70FF                           MOVEQ       #FF,D0
       | 23A6  2540 000C                      MOVE.L      D0,000C(A2)
;2064:         dp->dp_Res2 = ERROR_OBJECT_NOT_FOUND;
       | 23AA  257C 0000 00CD 0010            MOVE.L      #000000CD,0010(A2)
;2065:         return(FALSE);
       | 23B2  7000                           MOVEQ       #00,D0
       | 23B4  6008                           BRA.B       23BE
;2066:     }
;2067: 
;2068:     tp->tp_Arg1 = (LONG) cf->CFH_ServerFH;
       | 23B6  2768 0004 0014                 MOVE.L      0004(A0),0014(A3)
;2069: 
;2070:     return(TRUE);
       | 23BC  7001                           MOVEQ       #01,D0
;2071: 
;2072: }
       | 23BE  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 23C2  4E5D                           UNLK        A5
       | 23C4  4E75                           RTS
;2073: 
;2074: 
;2075: /*
;2076:  * StartReadLink
;2077:  *
;2078:  * ACTION_READ_LINK
;2079:  *
;2080:  */
;2081: BOOL StartReadLink(struct Transaction *t, struct TPacket *tp, struct DosPacket *dp,
;2082:                    struct MountedFSInfoClient *m)
;2083: {
       | 23C6  4E55 0000                      LINK        A5,#0000
       | 23CA  2F0B                           MOVE.L      A3,-(A7)
       | 23CC  266D 0010                      MOVEA.L     0010(A5),A3
;2084: //    struct CLock *cl;
;2085: //    UBYTE *gp;
;2086:     dp->dp_Res1 = -1L;
       | 23D0  70FF                           MOVEQ       #FF,D0
       | 23D2  2740 000C                      MOVE.L      D0,000C(A3)
;2087:     dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
       | 23D6  277C 0000 00D1 0010            MOVE.L      #000000D1,0010(A3)
;2088: 
;2089:     return(FALSE);
       | 23DE  7000                           MOVEQ       #00,D0
;2090: }
       | 23E0  265F                           MOVEA.L     (A7)+,A3
       | 23E2  4E5D                           UNLK        A5
       | 23E4  4E75                           RTS
;2091: 
;2092: 
;2093: 
;2094: 
;2095: BOOL DoStart(LONG type,struct Transaction *t, struct TPacket *tp, struct MountedFSInfoClient *m)
;2096: {
       | 23E6  4E55 FFFC                      LINK        A5,#FFFC
       | 23EA  48E7 0130                      MOVEM.L     D7/A2-A3,-(A7)
       | 23EE  2E2D 0008                      MOVE.L      0008(A5),D7
       | 23F2  266D 000C                      MOVEA.L     000C(A5),A3
       | 23F6  246D 0010                      MOVEA.L     0010(A5),A2
;2097:     struct DosPacket *dp;
;2098:     dp = (struct DosPacket *) tp->tp_DosPacket;
       | 23FA  2B6A 0004 FFFC                 MOVE.L      0004(A2),FFFC(A5)
;2099:     switch(type)
       | 2400  2007                           MOVE.L      D7,D0
       | 2402  5F80                           SUBQ.L      #7,D0
       | 2404  6700 02DC                      BEQ.W       26E2
       | 2408  5380                           SUBQ.L      #1,D0
       | 240A  6700 0258                      BEQ.W       2664
       | 240E  5F80                           SUBQ.L      #7,D0
       | 2410  6700 0202                      BEQ.W       2614
       | 2414  5380                           SUBQ.L      #1,D0
       | 2416  6700 0238                      BEQ.W       2650
       | 241A  5380                           SUBQ.L      #1,D0
       | 241C  6700 01BA                      BEQ.W       25D8
       | 2420  5580                           SUBQ.L      #2,D0
       | 2422  6700 0218                      BEQ.W       263C
       | 2426  5580                           SUBQ.L      #2,D0
       | 2428  6700 01C2                      BEQ.W       25EC
       | 242C  5380                           SUBQ.L      #1,D0
       | 242E  6700 0234                      BEQ.W       2664
       | 2432  5380                           SUBQ.L      #1,D0
       | 2434  6700 012A                      BEQ.W       2560
       | 2438  5380                           SUBQ.L      #1,D0
       | 243A  6700 0124                      BEQ.W       2560
       | 243E  5380                           SUBQ.L      #1,D0
       | 2440  6700 00CE                      BEQ.W       2510
       | 2444  5380                           SUBQ.L      #1,D0
       | 2446  6700 00B4                      BEQ.W       24FC
       | 244A  5380                           SUBQ.L      #1,D0
       | 244C  6700 023E                      BEQ.W       268C
       | 2450  5380                           SUBQ.L      #1,D0
       | 2452  6700 01AC                      BEQ.W       2600
       | 2456  5380                           SUBQ.L      #1,D0
       | 2458  6700 01E2                      BEQ.W       263C
       | 245C  5B80                           SUBQ.L      #5,D0
       | 245E  6700 01C8                      BEQ.W       2628
       | 2462  5D80                           SUBQ.L      #6,D0
       | 2464  6700 0212                      BEQ.W       2678
       | 2468  722A                           MOVEQ       #2A,D1
       | 246A  9081                           SUB.L       D1,D0
       | 246C  6700 00B6                      BEQ.W       2524
       | 2470  5B80                           SUBQ.L      #5,D0
       | 2472  6700 00C4                      BEQ.W       2538
       | 2476  0480 0000 0395                 SUBI.L      #00000395,D0
       | 247C  6700 00F6                      BEQ.W       2574
       | 2480  5380                           SUBQ.L      #1,D0
       | 2482  6700 00F0                      BEQ.W       2574
       | 2486  5380                           SUBQ.L      #1,D0
       | 2488  6700 00EA                      BEQ.W       2574
       | 248C  5380                           SUBQ.L      #1,D0
       | 248E  6700 0134                      BEQ.W       25C4
       | 2492  5380                           SUBQ.L      #1,D0
       | 2494  6700 011A                      BEQ.W       25B0
       | 2498  720E                           MOVEQ       #0E,D1
       | 249A  9081                           SUB.L       D1,D0
       | 249C  6700 01EE                      BEQ.W       268C
       | 24A0  5580                           SUBQ.L      #2,D0
       | 24A2  6744                           BEQ.B       24E8
       | 24A4  5580                           SUBQ.L      #2,D0
       | 24A6  6700 00E0                      BEQ.W       2588
       | 24AA  5380                           SUBQ.L      #1,D0
       | 24AC  6700 0226                      BEQ.W       26D4
       | 24B0  5380                           SUBQ.L      #1,D0
       | 24B2  6700 0098                      BEQ.W       254C
       | 24B6  5580                           SUBQ.L      #2,D0
       | 24B8  6700 00E2                      BEQ.W       259C
       | 24BC  5380                           SUBQ.L      #1,D0
       | 24BE  6700 0202                      BEQ.W       26C2
       | 24C2  5B80                           SUBQ.L      #5,D0
       | 24C4  6700 0126                      BEQ.W       25EC
       | 24C8  0480 0000 4A14                 SUBI.L      #00004A14,D0
       | 24CE  6700 01CE                      BEQ.W       269E
       | 24D2  5380                           SUBQ.L      #1,D0
       | 24D4  6700 01C8                      BEQ.W       269E
       | 24D8  5380                           SUBQ.L      #1,D0
       | 24DA  6700 01D4                      BEQ.W       26B0
       | 24DE  5380                           SUBQ.L      #1,D0
       | 24E0  6700 01CE                      BEQ.W       26B0
       | 24E4  6000 0212                      BRA.W       26F8
;2100:     {
;2101: 
;2102:         case ACTION_READ_LINK:
;2103:             return(StartReadLink(t,tp,dp,m));
       | 24E8  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 24EC  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 24F0  2F0A                           MOVE.L      A2,-(A7)
       | 24F2  2F0B                           MOVE.L      A3,-(A7)
       | 24F4  6100 FED0                      BSR.W       23C6
       | 24F8  6000 0210                      BRA.W       270A
;2104: 
;2105:         case ACTION_INFO:
;2106:             return(StartInfo(t,tp,dp,m));
       | 24FC  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2500  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2504  2F0A                           MOVE.L      A2,-(A7)
       | 2506  2F0B                           MOVE.L      A3,-(A7)
       | 2508  6100 FB44                      BSR.W       204E
       | 250C  6000 01FC                      BRA.W       270A
;2107: 
;2108:         case ACTION_DISK_INFO:
;2109:             return(StartDiskInfo(t,tp,dp,m));
       | 2510  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2514  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2518  2F0A                           MOVE.L      A2,-(A7)
       | 251A  2F0B                           MOVE.L      A3,-(A7)
       | 251C  6100 FCA6                      BSR.W       21C4
       | 2520  6000 01E8                      BRA.W       270A
;2110: 
;2111:         case ACTION_READ:
;2112:             return(StartRead(t,tp,dp,m));
       | 2524  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2528  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 252C  2F0A                           MOVE.L      A2,-(A7)
       | 252E  2F0B                           MOVE.L      A3,-(A7)
       | 2530  6100 EC70                      BSR.W       11A2
       | 2534  6000 01D4                      BRA.W       270A
;2113: 
;2114:         case ACTION_WRITE:
;2115:             return(StartWrite(t,tp,dp,m));
       | 2538  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 253C  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2540  2F0A                           MOVE.L      A2,-(A7)
       | 2542  2F0B                           MOVE.L      A3,-(A7)
       | 2544  6100 EDA8                      BSR.W       12EE
       | 2548  6000 01C0                      BRA.W       270A
;2116: 
;2117:         case ACTION_CHANGE_MODE:
;2118:             return(StartChangeMode(t,tp,dp,m));
       | 254C  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2550  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2554  2F0A                           MOVE.L      A2,-(A7)
       | 2556  2F0B                           MOVE.L      A3,-(A7)
       | 2558  6100 FB4E                      BSR.W       20A8
       | 255C  6000 01AC                      BRA.W       270A
;2119: 
;2120:         case ACTION_EXAMINE_NEXT:
;2121:         case ACTION_EXAMINE_OBJECT:
;2122:             return(StartExamine(t,tp,dp,m));
       | 2560  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2564  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2568  2F0A                           MOVE.L      A2,-(A7)
       | 256A  2F0B                           MOVE.L      A3,-(A7)
       | 256C  6100 F928                      BSR.W       1E96
       | 2570  6000 0198                      BRA.W       270A
;2123: 
;2124:         case ACTION_FINDINPUT:
;2125:         case ACTION_FINDOUTPUT:
;2126:         case ACTION_FINDUPDATE:
;2127:             return(StartFIND(t,tp,dp,m));
       | 2574  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2578  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 257C  2F0A                           MOVE.L      A2,-(A7)
       | 257E  2F0B                           MOVE.L      A3,-(A7)
       | 2580  6100 E94E                      BSR.W       0ED0
       | 2584  6000 0184                      BRA.W       270A
;2128: 
;2129:         case ACTION_FH_FROM_LOCK:
;2130:             return(StartOpenFromLock(t,tp,dp,m));
       | 2588  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 258C  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2590  2F0A                           MOVE.L      A2,-(A7)
       | 2592  2F0B                           MOVE.L      A3,-(A7)
       | 2594  6100 FD8E                      BSR.W       2324
       | 2598  6000 0170                      BRA.W       270A
;2131: 
;2132:         case ACTION_COPY_DIR_FH:
;2133:             return(StartLockFromFH(t,tp,dp,m));
       | 259C  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 25A0  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 25A4  2F0A                           MOVE.L      A2,-(A7)
       | 25A6  2F0B                           MOVE.L      A3,-(A7)
       | 25A8  6100 FDD2                      BSR.W       237C
       | 25AC  6000 015C                      BRA.W       270A
;2134: 
;2135:         case ACTION_SEEK:
;2136:             return(StartSeek(t,tp,dp,m));
       | 25B0  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 25B4  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 25B8  2F0A                           MOVE.L      A2,-(A7)
       | 25BA  2F0B                           MOVE.L      A3,-(A7)
       | 25BC  6100 EB90                      BSR.W       114E
       | 25C0  6000 0148                      BRA.W       270A
;2137: 
;2138:         case ACTION_END:
;2139:             return(StartEND(t,tp,dp,m));
       | 25C4  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 25C8  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 25CC  2F0A                           MOVE.L      A2,-(A7)
       | 25CE  2F0B                           MOVE.L      A3,-(A7)
       | 25D0  6100 EA76                      BSR.W       1048
       | 25D4  6000 0134                      BRA.W       270A
;2140: 
;2141:         case ACTION_RENAME_OBJECT:
;2142:             return(StartRename(t,tp,dp,m));
       | 25D8  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 25DC  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 25E0  2F0A                           MOVE.L      A2,-(A7)
       | 25E2  2F0B                           MOVE.L      A3,-(A7)
       | 25E4  6100 EF4A                      BSR.W       1530
       | 25E8  6000 0120                      BRA.W       270A
;2143: 
;2144:         case ACTION_SET_OWNER:
;2145:         case ACTION_SET_PROTECT:
;2146:             return(StartSetProtect(t,tp,dp,m));
       | 25EC  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 25F0  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 25F4  2F0A                           MOVE.L      A2,-(A7)
       | 25F6  2F0B                           MOVE.L      A3,-(A7)
       | 25F8  6100 F0B4                      BSR.W       16AE
       | 25FC  6000 010C                      BRA.W       270A
;2147: 
;2148:         case ACTION_SET_COMMENT:
;2149:             return(StartSetComment(t,tp,dp,m));
       | 2600  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2604  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2608  2F0A                           MOVE.L      A2,-(A7)
       | 260A  2F0B                           MOVE.L      A3,-(A7)
       | 260C  6100 F178                      BSR.W       1786
       | 2610  6000 00F8                      BRA.W       270A
;2150: 
;2151:         case ACTION_FREE_LOCK:
;2152:             return(StartFreeLock(t,tp,dp,m));
       | 2614  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2618  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 261C  2F0A                           MOVE.L      A2,-(A7)
       | 261E  2F0B                           MOVE.L      A3,-(A7)
       | 2620  6100 F29A                      BSR.W       18BC
       | 2624  6000 00E4                      BRA.W       270A
;2153: 
;2154:         case ACTION_SET_DATE:
;2155:             return(StartSetDate(t,tp,dp,m));
       | 2628  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 262C  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2630  2F0A                           MOVE.L      A2,-(A7)
       | 2632  2F0B                           MOVE.L      A3,-(A7)
       | 2634  6100 F354                      BSR.W       198A
       | 2638  6000 00D0                      BRA.W       270A
;2156: 
;2157:         case ACTION_PARENT:
;2158:         case ACTION_COPY_DIR:
;2159:             return(StartParent(t,tp,dp,m));
       | 263C  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2640  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2644  2F0A                           MOVE.L      A2,-(A7)
       | 2646  2F0B                           MOVE.L      A3,-(A7)
       | 2648  6100 F452                      BSR.W       1A9C
       | 264C  6000 00BC                      BRA.W       270A
;2160: 
;2161:         case ACTION_DELETE_OBJECT:
;2162:             return(StartDeleteObject(t,tp,dp,m));
       | 2650  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2654  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2658  2F0A                           MOVE.L      A2,-(A7)
       | 265A  2F0B                           MOVE.L      A3,-(A7)
       | 265C  6100 F5DC                      BSR.W       1C3A
       | 2660  6000 00A8                      BRA.W       270A
;2163: 
;2164:         case ACTION_CREATE_DIR:
;2165:         case ACTION_LOCATE_OBJECT:
;2166:             return(StartLocateObject(t,tp,dp,m));
       | 2664  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2668  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 266C  2F0A                           MOVE.L      A2,-(A7)
       | 266E  2F0B                           MOVE.L      A3,-(A7)
       | 2670  6100 F698                      BSR.W       1D0A
       | 2674  6000 0094                      BRA.W       270A
;2167: 
;2168:         case ACTION_SAME_LOCK:
;2169:             return(StartSameLock(t,tp,dp,m));
       | 2678  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 267C  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2680  2F0A                           MOVE.L      A2,-(A7)
       | 2682  2F0B                           MOVE.L      A3,-(A7)
       | 2684  6100 F936                      BSR.W       1FBC
       | 2688  6000 0080                      BRA.W       270A
;2170: 
;2171:         case ACTION_SET_FILE_SIZE:
;2172:         case ACTION_FLUSH:
;2173:             return(StartFlush(t,tp,dp,m));
       | 268C  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2690  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2694  2F0A                           MOVE.L      A2,-(A7)
       | 2696  2F0B                           MOVE.L      A3,-(A7)
       | 2698  6100 FB20                      BSR.W       21BA
       | 269C  606C                           BRA.B       270A
;2174: 
;2175:         case ACTION_USERNAME_TO_UID:
;2176:         case ACTION_GROUPNAME_TO_GID:
;2177:             return(StartToID(t,tp,dp,m));
       | 269E  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 26A2  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 26A6  2F0A                           MOVE.L      A2,-(A7)
       | 26A8  2F0B                           MOVE.L      A3,-(A7)
       | 26AA  6100 FB30                      BSR.W       21DC
       | 26AE  605A                           BRA.B       270A
;2178: 
;2179:         case ACTION_UID_TO_USERINFO:
;2180:         case ACTION_GID_TO_GROUPINFO:
;2181:             return(StartToInfo(t,tp,dp,m));
       | 26B0  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 26B4  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 26B8  2F0A                           MOVE.L      A2,-(A7)
       | 26BA  2F0B                           MOVE.L      A3,-(A7)
       | 26BC  6100 FB56                      BSR.W       2214
       | 26C0  6048                           BRA.B       270A
;2182: 
;2183:         case ACTION_PARENT_FH:
;2184:             return(StartParentFH(t,tp,dp,m));
       | 26C2  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 26C6  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 26CA  2F0A                           MOVE.L      A2,-(A7)
       | 26CC  2F0B                           MOVE.L      A3,-(A7)
       | 26CE  6100 F424                      BSR.W       1AF4
       | 26D2  6036                           BRA.B       270A
;2185: 
;2186:         case ACTION_IS_FILESYSTEM:
;2187:         {
;2188:             dp->dp_Res1 = DOSTRUE;
       | 26D4  70FF                           MOVEQ       #FF,D0
       | 26D6  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 26DA  2140 000C                      MOVE.L      D0,000C(A0)
;2189:             return(FALSE);
       | 26DE  7000                           MOVEQ       #00,D0
       | 26E0  6028                           BRA.B       270A
;2190:         }
;2191: 
;2192:         case ACTION_CURRENT_VOLUME:
;2193:         {
;2194:             dp->dp_Res1 = MKBADDR(m->mfsi_Volume);
       | 26E2  206D 0014                      MOVEA.L     0014(A5),A0
       | 26E6  2028 0034                      MOVE.L      0034(A0),D0
       | 26EA  E480                           ASR.L       #2,D0
       | 26EC  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 26F0  2140 000C                      MOVE.L      D0,000C(A0)
;2195:             return(FALSE);
       | 26F4  7000                           MOVEQ       #00,D0
       | 26F6  6012                           BRA.B       270A
;2196:         }
;2197: 
;2198:         default:
;2199:         {
;2200:             dp->dp_Res1 = DOSFALSE;
       | 26F8  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 26FC  42A8 000C                      CLR.L       000C(A0)
;2201:             dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
       | 2700  217C 0000 00D1 0010            MOVE.L      #000000D1,0010(A0)
;2202:             return(FALSE);
       | 2708  7000                           MOVEQ       #00,D0
;2203:             break;
;2204:         }
;2205:     }
;2206: }
       | 270A  4CED 0C80 FFF0                 MOVEM.L     FFF0(A5),D7/A2-A3
       | 2710  4E5D                           UNLK        A5
       | 2712  4E75                           RTS
;2207: 
;2208: 
;2209: 
;2210: 
;2211: 
;2212: void DoEnd(LONG type, struct Transaction *t, struct MountedFSInfoClient *m)
;2213: {
       | 2714  4E55 FFFC                      LINK        A5,#FFFC
       | 2718  48E7 0130                      MOVEM.L     D7/A2-A3,-(A7)
       | 271C  2E2D 0008                      MOVE.L      0008(A5),D7
       | 2720  266D 000C                      MOVEA.L     000C(A5),A3
       | 2724  246D 0010                      MOVEA.L     0010(A5),A2
;2214:     struct TPacket *tp;
;2215:     tp = (struct TPacket *) t->trans_ResponseData;
       | 2728  2B6B 0036 FFFC                 MOVE.L      0036(A3),FFFC(A5)
;2216: 
;2217:     switch(type)
       | 272E  2007                           MOVE.L      D7,D0
       | 2730  5180                           SUBQ.L      #8,D0
       | 2732  6700 010C                      BEQ.W       2840
       | 2736  5F80                           SUBQ.L      #7,D0
       | 2738  6700 00FA                      BEQ.W       2834
       | 273C  5380                           SUBQ.L      #1,D0
       | 273E  6700 00E8                      BEQ.W       2828
       | 2742  5380                           SUBQ.L      #1,D0
       | 2744  6700 00E2                      BEQ.W       2828
       | 2748  5580                           SUBQ.L      #2,D0
       | 274A  6700 00F4                      BEQ.W       2840
       | 274E  5580                           SUBQ.L      #2,D0
       | 2750  6700 00D6                      BEQ.W       2828
       | 2754  5380                           SUBQ.L      #1,D0
       | 2756  6700 00E8                      BEQ.W       2840
       | 275A  5380                           SUBQ.L      #1,D0
       | 275C  6700 00A6                      BEQ.W       2804
       | 2760  5380                           SUBQ.L      #1,D0
       | 2762  6700 00A0                      BEQ.W       2804
       | 2766  5380                           SUBQ.L      #1,D0
       | 2768  6700 008E                      BEQ.W       27F8
       | 276C  5380                           SUBQ.L      #1,D0
       | 276E  677A                           BEQ.B       27EA
       | 2770  5380                           SUBQ.L      #1,D0
       | 2772  6700 00E6                      BEQ.W       285A
       | 2776  5380                           SUBQ.L      #1,D0
       | 2778  6700 00AE                      BEQ.W       2828
       | 277C  5380                           SUBQ.L      #1,D0
       | 277E  6700 00C0                      BEQ.W       2840
       | 2782  5B80                           SUBQ.L      #5,D0
       | 2784  6700 00A2                      BEQ.W       2828
       | 2788  5D80                           SUBQ.L      #6,D0
       | 278A  6700 009C                      BEQ.W       2828
       | 278E  722A                           MOVEQ       #2A,D1
       | 2790  9081                           SUB.L       D1,D0
       | 2792  6700 00B8                      BEQ.W       284C
       | 2796  5B80                           SUBQ.L      #5,D0
       | 2798  6700 00CC                      BEQ.W       2866
       | 279C  0480 0000 0395                 SUBI.L      #00000395,D0
       | 27A2  676C                           BEQ.B       2810
       | 27A4  5380                           SUBQ.L      #1,D0
       | 27A6  6768                           BEQ.B       2810
       | 27A8  5380                           SUBQ.L      #1,D0
       | 27AA  6764                           BEQ.B       2810
       | 27AC  5380                           SUBQ.L      #1,D0
       | 27AE  6778                           BEQ.B       2828
       | 27B0  5380                           SUBQ.L      #1,D0
       | 27B2  6768                           BEQ.B       281C
       | 27B4  720E                           MOVEQ       #0E,D1
       | 27B6  9081                           SUB.L       D1,D0
       | 27B8  676E                           BEQ.B       2828
       | 27BA  5980                           SUBQ.L      #4,D0
       | 27BC  6752                           BEQ.B       2810
       | 27BE  5580                           SUBQ.L      #2,D0
       | 27C0  6766                           BEQ.B       2828
       | 27C2  5580                           SUBQ.L      #2,D0
       | 27C4  677A                           BEQ.B       2840
       | 27C6  5380                           SUBQ.L      #1,D0
       | 27C8  6776                           BEQ.B       2840
       | 27CA  5B80                           SUBQ.L      #5,D0
       | 27CC  675A                           BEQ.B       2828
       | 27CE  0480 0000 4A14                 SUBI.L      #00004A14,D0
       | 27D4  6752                           BEQ.B       2828
       | 27D6  5380                           SUBQ.L      #1,D0
       | 27D8  674E                           BEQ.B       2828
       | 27DA  5380                           SUBQ.L      #1,D0
       | 27DC  6700 0096                      BEQ.W       2874
       | 27E0  5380                           SUBQ.L      #1,D0
       | 27E2  6700 0090                      BEQ.W       2874
       | 27E6  6000 0096                      BRA.W       287E
;2218:     {
;2219: 
;2220: //        case ACTION_READ_LINK:
;2221: //            EndReadLink(tp,m);
;2222: //            return;
;2223: 
;2224:         case ACTION_INFO:
;2225:             EndInfo(tp,m);
       | 27EA  2F0A                           MOVE.L      A2,-(A7)
       | 27EC  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 27F0  6100 F942                      BSR.W       2134
;2226:             return;
       | 27F4  6000 0088                      BRA.W       287E
;2227: 
;2228:         case ACTION_DISK_INFO:
;2229:             EndDiskInfo(tp,m);
       | 27F8  2F0A                           MOVE.L      A2,-(A7)
       | 27FA  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 27FE  6100 FA9E                      BSR.W       229E
;2230:             return;
       | 2802  607A                           BRA.B       287E
;2231: 
;2232:         case ACTION_EXAMINE_NEXT:
;2233:         case ACTION_EXAMINE_OBJECT:
;2234:             EndExamine(tp,m);
       | 2804  2F0A                           MOVE.L      A2,-(A7)
       | 2806  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 280A  6100 F754                      BSR.W       1F60
;2235:             return;
       | 280E  606E                           BRA.B       287E
;2236: 
;2237:         case ACTION_FH_FROM_LOCK:
;2238:         case ACTION_FINDINPUT:
;2239:         case ACTION_FINDOUTPUT:
;2240:         case ACTION_FINDUPDATE:
;2241:             EndFind(tp,m);
       | 2810  2F0A                           MOVE.L      A2,-(A7)
       | 2812  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2816  6100 E778                      BSR.W       0F90
;2242:             return;
       | 281A  6062                           BRA.B       287E
;2243: 
;2244:         case ACTION_SEEK:
;2245:             EndSeek(tp,m);
       | 281C  2F0A                           MOVE.L      A2,-(A7)
       | 281E  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2822  6100 E8D8                      BSR.W       10FC
;2246:             return;
       | 2826  6056                           BRA.B       287E
;2247: 
;2248:         case ACTION_END:
;2249:         case ACTION_CHANGE_MODE:
;2250:         case ACTION_USERNAME_TO_UID:
;2251:         case ACTION_GROUPNAME_TO_GID:
;2252:         case ACTION_RENAME_OBJECT:
;2253:         case ACTION_SET_PROTECT:
;2254:         case ACTION_SET_OWNER:
;2255:         case ACTION_SET_DATE:
;2256:         case ACTION_DELETE_OBJECT:
;2257:         case ACTION_SET_COMMENT:
;2258:         case ACTION_SAME_LOCK:
;2259:         case ACTION_SET_FILE_SIZE:
;2260:             EndEnd(tp,m);
       | 2828  2F0A                           MOVE.L      A2,-(A7)
       | 282A  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 282E  6100 E8A4                      BSR.W       10D4
;2261:             return;
       | 2832  604A                           BRA.B       287E
;2262: 
;2263:         case ACTION_FREE_LOCK:
;2264:             EndFreeLock(tp,m);
       | 2834  2F0A                           MOVE.L      A2,-(A7)
       | 2836  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 283A  6100 F0DC                      BSR.W       1918
;2265:             return;
       | 283E  603E                           BRA.B       287E
;2266: 
;2267:         case ACTION_PARENT_FH:
;2268:         case ACTION_PARENT:
;2269:         case ACTION_COPY_DIR:
;2270:         case ACTION_COPY_DIR_FH:
;2271:         case ACTION_CREATE_DIR:
;2272:         case ACTION_LOCATE_OBJECT:
;2273:             EndParent(tp,m);
       | 2840  2F0A                           MOVE.L      A2,-(A7)
       | 2842  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2846  6100 F2F6                      BSR.W       1B3E
;2274:             return;
       | 284A  6032                           BRA.B       287E
;2275: 
;2276:         case ACTION_READ:
;2277:             EndRead(t,tp,m);                /* Special case! */
       | 284C  2F0A                           MOVE.L      A2,-(A7)
       | 284E  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2852  2F0B                           MOVE.L      A3,-(A7)
       | 2854  6100 EBFC                      BSR.W       1452
;2278:             return;
       | 2858  6024                           BRA.B       287E
;2279: 
;2280:         case ACTION_FLUSH:
;2281:             EndEnd(tp,m);
       | 285A  2F0A                           MOVE.L      A2,-(A7)
       | 285C  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 2860  6100 E872                      BSR.W       10D4
;2282:             return;
       | 2864  6018                           BRA.B       287E
;2283: 
;2284:         case ACTION_WRITE:
;2285:             EndWrite(t,tp,m);               /* Special case! */
       | 2866  2F0A                           MOVE.L      A2,-(A7)
       | 2868  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 286C  2F0B                           MOVE.L      A3,-(A7)
       | 286E  6100 EC6E                      BSR.W       14DE
;2286:             return;
       | 2872  600A                           BRA.B       287E
;2287: 
;2288:         case ACTION_UID_TO_USERINFO:
;2289:         case ACTION_GID_TO_GROUPINFO:
;2290:             EndToInfo(tp,m);
       | 2874  2F0A                           MOVE.L      A2,-(A7)
       | 2876  2F2D FFFC                      MOVE.L      FFFC(A5),-(A7)
       | 287A  6100 F9BE                      BSR.W       223A
;2291:             return;
;2292: 
;2293:         default:
;2294:         {
;2295:             break;
;2296:         }
;2297:     }
;2298: }
       | 287E  4CED 0C80 FFF0                 MOVEM.L     FFF0(A5),D7/A2-A3
       | 2884  4E5D                           UNLK        A5
       | 2886  4E75                           RTS
;2299: 
;2300: /*
;2301:  * NetDownRequest()
;2302:  *
;2303:  * Returns TRUE for "retry", and FALSE for "cancel".
;2304:  *
;2305:  */
;2306: 
;2307: BOOL NetDownRequest(UBYTE *machine, UBYTE *volume)
;2308: {
       | 2888  4E55 FFE0                      LINK        A5,#FFE0
       | 288C  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 2890  266D 0008                      MOVEA.L     0008(A5),A3
       | 2894  246D 000C                      MOVEA.L     000C(A5),A2
;2309:     BOOL returnval = FALSE;
       | 2898  7E00                           MOVEQ       #00,D7
;2310:     ULONG args[2];
;2311: 
;2312:     IntuitionBase = OpenLibrary("intuition.library",0L);
       | 289A  43F9  0000 00BE-01             LEA         01.000000BE,A1
       | 28A0  7000                           MOVEQ       #00,D0
       | 28A2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 28A8  4EAE FDD8                      JSR         FDD8(A6)
       | 28AC  23C0  0000 0010-02             MOVE.L      D0,02.00000010
;2313:     if (IntuitionBase)
       | 28B2  6744                           BEQ.B       28F8
;2314:     {
;2315:         struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
;2316:                                 "Host '%s'\ndidn't respond regarding\nvolume '%s:'","Retry|Cancel"};
       | 28B4  41F9  0000 0124-01             LEA         01.00000124,A0
       | 28BA  43ED FFE2                      LEA         FFE2(A5),A1
       | 28BE  7004                           MOVEQ       #04,D0
       | 28C0  22D8                           MOVE.L      (A0)+,(A1)+
       | 28C2  51C8 FFFC                      DBF         D0,28C0
;2317:         args[0] = (ULONG) machine;
       | 28C6  2B4B FFF6                      MOVE.L      A3,FFF6(A5)
;2318:         args[1] = (ULONG) volume;
       | 28CA  2B4A FFFA                      MOVE.L      A2,FFFA(A5)
;2319:         returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,args,REQUESTERTIMEOUT);
       | 28CE  4878 0014                      PEA         0014
       | 28D2  486D FFF6                      PEA         FFF6(A5)
       | 28D6  42A7                           CLR.L       -(A7)
       | 28D8  486D FFE2                      PEA         FFE2(A5)
       | 28DC  42A7                           CLR.L       -(A7)
       | 28DE  6100 01D0                      BSR.W       2AB0
       | 28E2  4FEF 0014                      LEA         0014(A7),A7
       | 28E6  2E00                           MOVE.L      D0,D7
;2320: 
;2321:         CloseLibrary(IntuitionBase);
       | 28E8  2279  0000 0010-02             MOVEA.L     02.00000010,A1
       | 28EE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 28F4  4EAE FE62                      JSR         FE62(A6)
;2322:     }
;2323: 
;2324:     return(returnval);
       | 28F8  2007                           MOVE.L      D7,D0
;2325: }
       | 28FA  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 28FE  4E5D                           UNLK        A5
       | 2900  4E75                           RTS
;2326: 
;2327: 
;2328: 
;2329: /*
;2330:  * MountFailedRequest()
;2331:  *
;2332:  * Returns TRUE for "retry", and FALSE for "cancel".
;2333:  *
;2334:  */
;2335: 
;2336: BOOL MountFailedRequest(UBYTE *machine, UBYTE *volume, ULONG err)
;2337: {
       | 2902  4E55 FFD8                      LINK        A5,#FFD8
       | 2906  48E7 0332                      MOVEM.L     D6-D7/A2-A3/A6,-(A7)
       | 290A  266D 0008                      MOVEA.L     0008(A5),A3
       | 290E  246D 000C                      MOVEA.L     000C(A5),A2
       | 2912  2E2D 0010                      MOVE.L      0010(A5),D7
;2338:     BOOL returnval = FALSE;
       | 2916  7C00                           MOVEQ       #00,D6
;2339: 
;2340:     IntuitionBase = OpenLibrary("intuition.library",0L);
       | 2918  43F9  0000 0138-01             LEA         01.00000138,A1
       | 291E  7000                           MOVEQ       #00,D0
       | 2920  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2926  4EAE FDD8                      JSR         FDD8(A6)
       | 292A  23C0  0000 0010-02             MOVE.L      D0,02.00000010
;2341:     if (IntuitionBase)
       | 2930  6700 00A0                      BEQ.W       29D2
;2342:     {
;2343:         ULONG args[3];
;2344:         UBYTE *reason="<Unknown reason>";
       | 2934  2B7C  0000 014A-01  FFEE       MOVE.L      #01.0000014A,FFEE(A5)
;2345:         struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
;2346:                                 "Host '%s'\nrejected connection attempt for\nvolume '%s:'\n%s","OK"};
       | 293C  41F9  0000 01B0-01             LEA         01.000001B0,A0
       | 2942  43ED FFDA                      LEA         FFDA(A5),A1
       | 2946  7004                           MOVEQ       #04,D0
       | 2948  22D8                           MOVE.L      (A0)+,(A1)+
       | 294A  51C8 FFFC                      DBF         D0,2948
;2347: 
;2348: 
;2349:         switch (err)
       | 294E  2007                           MOVE.L      D7,D0
       | 2950  0480 0000 0213                 SUBI.L      #00000213,D0
       | 2956  6730                           BEQ.B       2988
       | 2958  5780                           SUBQ.L      #3,D0
       | 295A  6722                           BEQ.B       297E
       | 295C  0480 0000 7DEA                 SUBI.L      #00007DEA,D0
       | 2962  6706                           BEQ.B       296A
       | 2964  5380                           SUBQ.L      #1,D0
       | 2966  670C                           BEQ.B       2974
       | 2968  6028                           BRA.B       2992
;2350:         {
;2351:             case FSERR_REJECTED_USER:
;2352:                 reason = "User rejected";
       | 296A  2B7C  0000 01C4-01  FFEE       MOVE.L      #01.000001C4,FFEE(A5)
;2353:                 break;
       | 2972  6026                           BRA.B       299A
;2354:             case FSERR_REJECTED_NOMOUNT:
;2355:                 reason = "Volume does not exist";
       | 2974  2B7C  0000 01D2-01  FFEE       MOVE.L      #01.000001D2,FFEE(A5)
;2356:                 break;
       | 297C  601C                           BRA.B       299A
;2357:             case ENVOYERR_TIMEOUT:
;2358:                 reason = "Timed out trying to obtain a mount";
       | 297E  2B7C  0000 01E8-01  FFEE       MOVE.L      #01.000001E8,FFEE(A5)
;2359:                 break;
       | 2986  6012                           BRA.B       299A
;2360:             case ENVOYERR_CANTDELIVER:
;2361:                 reason = "Server is refusing our data";
       | 2988  2B7C  0000 020C-01  FFEE       MOVE.L      #01.0000020C,FFEE(A5)
;2362:                 break;
       | 2990  6008                           BRA.B       299A
;2363:             default:
;2364:                 reason = "Unknown error!";
       | 2992  2B7C  0000 0228-01  FFEE       MOVE.L      #01.00000228,FFEE(A5)
;2365:                 break;
;2366:         }
;2367:         args[0] = (ULONG) machine;
       | 299A  2B4B FFF2                      MOVE.L      A3,FFF2(A5)
;2368:         args[1] = (ULONG) volume;
       | 299E  2B4A FFF6                      MOVE.L      A2,FFF6(A5)
;2369:         args[2] = (ULONG) reason;
       | 29A2  2B6D FFEE FFFA                 MOVE.L      FFEE(A5),FFFA(A5)
;2370:         returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,args,REQUESTERTIMEOUT);
       | 29A8  4878 0014                      PEA         0014
       | 29AC  486D FFF2                      PEA         FFF2(A5)
       | 29B0  42A7                           CLR.L       -(A7)
       | 29B2  486D FFDA                      PEA         FFDA(A5)
       | 29B6  42A7                           CLR.L       -(A7)
       | 29B8  6100 00F6                      BSR.W       2AB0
       | 29BC  4FEF 0014                      LEA         0014(A7),A7
       | 29C0  2C00                           MOVE.L      D0,D6
;2371: 
;2372:         CloseLibrary(IntuitionBase);
       | 29C2  2279  0000 0010-02             MOVEA.L     02.00000010,A1
       | 29C8  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 29CE  4EAE FE62                      JSR         FE62(A6)
;2373:     }
;2374: 
;2375:     return(returnval);
       | 29D2  2006                           MOVE.L      D6,D0
;2376: }
       | 29D4  4CDF 4CC0                      MOVEM.L     (A7)+,D6-D7/A2-A3/A6
       | 29D8  4E5D                           UNLK        A5
       | 29DA  4E75                           RTS
;2377: 
;2378: /*
;2379:  * CantConnectRequest()
;2380:  *
;2381:  * Returns TRUE for "retry", and FALSE for "cancel".
;2382:  *
;2383:  */
;2384: 
;2385: BOOL CantConnectRequest(UBYTE *machine)
;2386: {
       | 29DC  4E55 FFE8                      LINK        A5,#FFE8
       | 29E0  48E7 0102                      MOVEM.L     D7/A6,-(A7)
;2387:     BOOL returnval = FALSE;
       | 29E4  7E00                           MOVEQ       #00,D7
;2388: 
;2389:     IntuitionBase = OpenLibrary("intuition.library",0L);
       | 29E6  43F9  0000 0238-01             LEA         01.00000238,A1
       | 29EC  7000                           MOVEQ       #00,D0
       | 29EE  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 29F4  4EAE FDD8                      JSR         FDD8(A6)
       | 29F8  23C0  0000 0010-02             MOVE.L      D0,02.00000010
;2390:     if (IntuitionBase)
       | 29FE  673C                           BEQ.B       2A3C
;2391:     {
;2392:         struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
;2393:                                "Cannot connect to\nhost '%s'","Retry|Cancel"};
       | 2A00  41F9  0000 028A-01             LEA         01.0000028A,A0
       | 2A06  43ED FFEA                      LEA         FFEA(A5),A1
       | 2A0A  7004                           MOVEQ       #04,D0
       | 2A0C  22D8                           MOVE.L      (A0)+,(A1)+
       | 2A0E  51C8 FFFC                      DBF         D0,2A0C
;2394:         returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,&machine,REQUESTERTIMEOUT);
       | 2A12  4878 0014                      PEA         0014
       | 2A16  486D 0008                      PEA         0008(A5)
       | 2A1A  42A7                           CLR.L       -(A7)
       | 2A1C  486D FFEA                      PEA         FFEA(A5)
       | 2A20  42A7                           CLR.L       -(A7)
       | 2A22  6100 008C                      BSR.W       2AB0
       | 2A26  4FEF 0014                      LEA         0014(A7),A7
       | 2A2A  2E00                           MOVE.L      D0,D7
;2395: 
;2396:         CloseLibrary(IntuitionBase);
       | 2A2C  2279  0000 0010-02             MOVEA.L     02.00000010,A1
       | 2A32  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2A38  4EAE FE62                      JSR         FE62(A6)
;2397:     }
;2398: 
;2399:     return(returnval);
       | 2A3C  2007                           MOVE.L      D7,D0
;2400: }
       | 2A3E  4CDF 4080                      MOVEM.L     (A7)+,D7/A6
       | 2A42  4E5D                           UNLK        A5
       | 2A44  4E75                           RTS
;2401: 
;2402: 
;2403: 
;2404: /*
;2405:  * GPRequest()
;2406:  *
;2407:  * Returns TRUE for "retry", and FALSE for "cancel".
;2408:  *
;2409:  */
;2410: 
;2411: BOOL GPRequest(ULONG num)
;2412: {
       | 2A46  4E55 FFE8                      LINK        A5,#FFE8
       | 2A4A  48E7 0102                      MOVEM.L     D7/A6,-(A7)
;2413:     BOOL returnval = FALSE;
       | 2A4E  7E00                           MOVEQ       #00,D7
;2414: 
;2415:     IntuitionBase = OpenLibrary("intuition.library",0L);
       | 2A50  43F9  0000 029E-01             LEA         01.0000029E,A1
       | 2A56  7000                           MOVEQ       #00,D0
       | 2A58  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2A5E  4EAE FDD8                      JSR         FDD8(A6)
       | 2A62  23C0  0000 0010-02             MOVE.L      D0,02.00000010
;2416:     if (IntuitionBase)
       | 2A68  673C                           BEQ.B       2AA6
;2417:     {
;2418:         struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
;2419:                                "Error code: %lx","OK"};
       | 2A6A  41F9  0000 02DA-01             LEA         01.000002DA,A0
       | 2A70  43ED FFEA                      LEA         FFEA(A5),A1
       | 2A74  7004                           MOVEQ       #04,D0
       | 2A76  22D8                           MOVE.L      (A0)+,(A1)+
       | 2A78  51C8 FFFC                      DBF         D0,2A76
;2420:         returnval = (BOOL) TimeOutEasyRequest(0L,&ers,0L,&num,REQUESTERTIMEOUT);
       | 2A7C  4878 0014                      PEA         0014
       | 2A80  486D 0008                      PEA         0008(A5)
       | 2A84  42A7                           CLR.L       -(A7)
       | 2A86  486D FFEA                      PEA         FFEA(A5)
       | 2A8A  42A7                           CLR.L       -(A7)
       | 2A8C  6100 0022                      BSR.W       2AB0
       | 2A90  4FEF 0014                      LEA         0014(A7),A7
       | 2A94  2E00                           MOVE.L      D0,D7
;2421: 
;2422:         CloseLibrary(IntuitionBase);
       | 2A96  2279  0000 0010-02             MOVEA.L     02.00000010,A1
       | 2A9C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2AA2  4EAE FE62                      JSR         FE62(A6)
;2423:     }
;2424: 
;2425:     return(returnval);
       | 2AA6  2007                           MOVE.L      D7,D0
;2426: }
       | 2AA8  4CDF 4080                      MOVEM.L     (A7)+,D7/A6
       | 2AAC  4E5D                           UNLK        A5
       | 2AAE  4E75                           RTS
;2427: 
;2428: 
;2429: LONG TimeOutEasyRequest(struct Window *ref, struct EasyStruct *ez, ULONG id, APTR args, ULONG timeout)
;2430: {
       | 2AB0  4E55 FFE8                      LINK        A5,#FFE8
       | 2AB4  48E7 2732                      MOVEM.L     D2/D5-D7/A2-A3/A6,-(A7)
       | 2AB8  266D 0008                      MOVEA.L     0008(A5),A3
       | 2ABC  246D 000C                      MOVEA.L     000C(A5),A2
       | 2AC0  2E2D 0018                      MOVE.L      0018(A5),D7
;2431:     LONG resp;
;2432:     struct Window *reqwin;
;2433:     reqwin = (struct Window *) BuildEasyRequestArgs(ref,ez,id,args);
       | 2AC4  2F2D 0014                      MOVE.L      0014(A5),-(A7)
       | 2AC8  2F2D 0010                      MOVE.L      0010(A5),-(A7)
       | 2ACC  2F0A                           MOVE.L      A2,-(A7)
       | 2ACE  2F0B                           MOVE.L      A3,-(A7)
       | 2AD0  4EBA  0000-XX.1                JSR         _BuildEasyRequestArgs(PC)
       | 2AD4  4FEF 0010                      LEA         0010(A7),A7
;2434:     if (reqwin)
       | 2AD8  2B40 FFF8                      MOVE.L      D0,FFF8(A5)
       | 2ADC  6700 0106                      BEQ.W       2BE4
;2435:     {
;2436:         struct MsgPort *rp;
;2437:         rp = (struct MsgPort *) CreateMsgPort();
       | 2AE0  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2AE6  4EAE FD66                      JSR         FD66(A6)
;2438:         if (rp)
       | 2AEA  2B40 FFF4                      MOVE.L      D0,FFF4(A5)
       | 2AEE  6700 00EA                      BEQ.W       2BDA
;2439:         {
;2440:             struct timerequest *tio;
;2441:             tio = (struct timerequest *) CreateIORequest(rp,sizeof(struct timerequest));
       | 2AF2  2040                           MOVEA.L     D0,A0
       | 2AF4  7028                           MOVEQ       #28,D0
       | 2AF6  4EAE FD72                      JSR         FD72(A6)
;2442:             if (tio)
       | 2AFA  2B40 FFF0                      MOVE.L      D0,FFF0(A5)
       | 2AFE  4A80                           TST.L       D0
       | 2B00  6700 00CA                      BEQ.W       2BCC
;2443:             {
;2444:                 if (!OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *) tio, 0))
       | 2B04  2240                           MOVEA.L     D0,A1
       | 2B06  41F9  0000 02EE-01             LEA         01.000002EE,A0
       | 2B0C  7001                           MOVEQ       #01,D0
       | 2B0E  7200                           MOVEQ       #00,D1
       | 2B10  4EAE FE44                      JSR         FE44(A6)
       | 2B14  4A00                           TST.B       D0
       | 2B16  6600 00A6                      BNE.W       2BBE
;2445:                 {
;2446:                     tio->tr_node.io_Command = TR_ADDREQUEST;
       | 2B1A  206D FFF0                      MOVEA.L     FFF0(A5),A0
       | 2B1E  317C 0009 001C                 MOVE.W      #0009,001C(A0)
;2447:                     tio->tr_node.io_Flags = 0;
       | 2B24  4228 001E                      CLR.B       001E(A0)
;2448:                     tio->tr_time.tv_secs = timeout;
       | 2B28  2147 0020                      MOVE.L      D7,0020(A0)
;2449:                     tio->tr_time.tv_micro = 0;
       | 2B2C  42A8 0024                      CLR.L       0024(A0)
;2450:                     SendIO((struct IORequest *) tio);
       | 2B30  2248                           MOVEA.L     A0,A1
       | 2B32  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2B38  4EAE FE32                      JSR         FE32(A6)
;2451:                     while (TRUE)
;2452:                     {
;2453:                         ULONG waitmask,found;
;2454: 
;2455:                         waitmask = ( (1 << rp->mp_SigBit) | (1 << reqwin->UserPort->mp_SigBit) );
       | 2B3C  7000                           MOVEQ       #00,D0
       | 2B3E  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 2B42  1028 000F                      MOVE.B      000F(A0),D0
       | 2B46  7201                           MOVEQ       #01,D1
       | 2B48  2401                           MOVE.L      D1,D2
       | 2B4A  E1A2                           ASL.L       D0,D2
       | 2B4C  2C6D FFF8                      MOVEA.L     FFF8(A5),A6
       | 2B50  226E 0056                      MOVEA.L     0056(A6),A1
       | 2B54  7000                           MOVEQ       #00,D0
       | 2B56  1029 000F                      MOVE.B      000F(A1),D0
       | 2B5A  E1A1                           ASL.L       D0,D1
       | 2B5C  8481                           OR.L        D1,D2
       | 2B5E  2A02                           MOVE.L      D2,D5
;2456:                         found = Wait(waitmask);
       | 2B60  2005                           MOVE.L      D5,D0
       | 2B62  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2B68  4EAE FEC2                      JSR         FEC2(A6)
;2457:                         if (GetMsg(rp))
       | 2B6C  2B40 FFE8                      MOVE.L      D0,FFE8(A5)
       | 2B70  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 2B74  4EAE FE8C                      JSR         FE8C(A6)
       | 2B78  4A80                           TST.L       D0
       | 2B7A  6704                           BEQ.B       2B80
;2458:                         {
;2459:                             resp = 0; /* negative response on timeout */
       | 2B7C  7C00                           MOVEQ       #00,D6
;2460:                             break;
       | 2B7E  6030                           BRA.B       2BB0
;2461:                         }
;2462:                         else
;2463:                         {
;2464:                             resp = SysReqHandler(reqwin, &id, FALSE);
       | 2B80  42A7                           CLR.L       -(A7)
       | 2B82  486D 0010                      PEA         0010(A5)
       | 2B86  2F2D FFF8                      MOVE.L      FFF8(A5),-(A7)
       | 2B8A  4EBA  0000-XX.1                JSR         _SysReqHandler(PC)
       | 2B8E  4FEF 000C                      LEA         000C(A7),A7
       | 2B92  2C00                           MOVE.L      D0,D6
;2465:                             if ((resp+1) >= 0)
       | 2B94  2006                           MOVE.L      D6,D0
       | 2B96  5280                           ADDQ.L      #1,D0
       | 2B98  6DA2                           BLT.B       2B3C
;2466:                             {
;2467:                                 AbortIO((struct IORequest *) tio);
       | 2B9A  226D FFF0                      MOVEA.L     FFF0(A5),A1
       | 2B9E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2BA4  4EAE FE20                      JSR         FE20(A6)
;2468:                                 WaitIO((struct IORequest *) tio);
       | 2BA8  226D FFF0                      MOVEA.L     FFF0(A5),A1
       | 2BAC  4EAE FE26                      JSR         FE26(A6)
;2469:                                 break;
;2470:                             }
;2471:                         }
;2472:                     }
;2473:                     CloseDevice((struct IORequest *) tio);
       | 2BB0  226D FFF0                      MOVEA.L     FFF0(A5),A1
       | 2BB4  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2BBA  4EAE FE3E                      JSR         FE3E(A6)
;2474:                 }
;2475:                 DeleteIORequest(tio);
       | 2BBE  206D FFF0                      MOVEA.L     FFF0(A5),A0
       | 2BC2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2BC8  4EAE FD6C                      JSR         FD6C(A6)
;2476:             }
;2477:             DeleteMsgPort(rp);
       | 2BCC  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 2BD0  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 2BD6  4EAE FD60                      JSR         FD60(A6)
;2478:         }
;2479:         FreeSysRequest(reqwin);
       | 2BDA  2F2D FFF8                      MOVE.L      FFF8(A5),-(A7)
       | 2BDE  4EBA  0000-XX.1                JSR         _FreeSysRequest(PC)
       | 2BE2  584F                           ADDQ.W      #4,A7
;2480:     }
;2481:     return(resp);
       | 2BE4  2006                           MOVE.L      D6,D0
;2482: }
       | 2BE6  4CDF 4CE4                      MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A6
       | 2BEA  4E5D                           UNLK        A5
       | 2BEC  4E75                           RTS

SECTION 01 " " 000002FC BYTES
0000 65 66 73 20 33 37 2E 38 20 28 31 30 2E 31 32 2E efs 37.8 (10.12.
0010 39 32 29 0A 0D 00 92)...
0016 00000000-01 01.00000000
001A 00 24 56 45 52 3A .$VER:
0020 20 65 66 73 20 33 37 2E 38 20 28 31 30 2E 31 32  efs 37.8 (10.12
0030 2E 39 32 29 00 00 .92)..
0036 0000001A-01 01.0000001A
003A 80 0B 10 04 00 00 ......
0040 00 00 00 00 00 00 80 0B 11 01 00 00 02 00 00 00 ................
0050 00 00 64 6F 73 2E 6C 69 62 72 61 72 79 00 6E 69 ..dos.library.ni
0060 70 63 2E 6C 69 62 72 61 72 79 00 00 73 65 72 76 pc.library..serv
0070 69 63 65 73 2E 6C 69 62 72 61 72 79 00 00 80 00 ices.library....
0080 08 02 00 00 00 00 00 00 00 00 46 69 6C 65 73 79 ..........Filesy
0090 73 74 65 6D 00 00 4E 45 57 00 4E 45 57 00 80 0B stem..NEW.NEW...
00A0 11 01 00 00 00 34 80 0B 11 02 00 00 00 00 00 00 .....4..........
00B0 00 00 80 0B 11 01 00 00 00 00 00 00 00 00 69 6E ..............in
00C0 74 75 69 74 69 6F 6E 2E 6C 69 62 72 61 72 79 00 tuition.library.
00D0 45 6E 76 6F 79 20 4E 65 74 77 6F 72 6B 20 50 72 Envoy Network Pr
00E0 6F 62 6C 65 6D 00 48 6F 73 74 20 27 25 73 27 0A oblem.Host '%s'.
00F0 64 69 64 6E 27 74 20 72 65 73 70 6F 6E 64 20 72 didn't respond r
0100 65 67 61 72 64 69 6E 67 0A 76 6F 6C 75 6D 65 20 egarding.volume 
0110 27 25 73 3A 27 00 52 65 74 72 79 7C 43 61 6E 63 '%s:'.Retry|Canc
0120 65 6C 00 00 00 00 00 14 00 00 00 00 el..........
012C 000000D0-01 01.000000D0
0130 000000E6-01 01.000000E6
0134 00000116-01 01.00000116
0138 69 6E 74 75 69 74 69 6F intuitio
0140 6E 2E 6C 69 62 72 61 72 79 00 3C 55 6E 6B 6E 6F n.library.<Unkno
0150 77 6E 20 72 65 61 73 6F 6E 3E 00 00 45 6E 76 6F wn reason>..Envo
0160 79 20 4E 65 74 77 6F 72 6B 20 50 72 6F 62 6C 65 y Network Proble
0170 6D 00 48 6F 73 74 20 27 25 73 27 0A 72 65 6A 65 m.Host '%s'.reje
0180 63 74 65 64 20 63 6F 6E 6E 65 63 74 69 6F 6E 20 cted connection 
0190 61 74 74 65 6D 70 74 20 66 6F 72 0A 76 6F 6C 75 attempt for.volu
01A0 6D 65 20 27 25 73 3A 27 0A 25 73 00 4F 4B 00 00 me '%s:'.%s.OK..
01B0 00 00 00 14 00 00 00 00 ........
01B8 0000015C-01 01.0000015C
01BC 00000172-01 01.00000172
01C0 000001AC-01 01.000001AC
01C4 55 73 65 72 20 72 65 6A 65 63 74 65 User rejecte
01D0 64 00 56 6F 6C 75 6D 65 20 64 6F 65 73 20 6E 6F d.Volume does no
01E0 74 20 65 78 69 73 74 00 54 69 6D 65 64 20 6F 75 t exist.Timed ou
01F0 74 20 74 72 79 69 6E 67 20 74 6F 20 6F 62 74 61 t trying to obta
0200 69 6E 20 61 20 6D 6F 75 6E 74 00 00 53 65 72 76 in a mount..Serv
0210 65 72 20 69 73 20 72 65 66 75 73 69 6E 67 20 6F er is refusing o
0220 75 72 20 64 61 74 61 00 55 6E 6B 6E 6F 77 6E 20 ur data.Unknown 
0230 65 72 72 6F 72 21 00 00 69 6E 74 75 69 74 69 6F error!..intuitio
0240 6E 2E 6C 69 62 72 61 72 79 00 45 6E 76 6F 79 20 n.library.Envoy 
0250 4E 65 74 77 6F 72 6B 20 50 72 6F 62 6C 65 6D 00 Network Problem.
0260 43 61 6E 6E 6F 74 20 63 6F 6E 6E 65 63 74 20 74 Cannot connect t
0270 6F 0A 68 6F 73 74 20 27 25 73 27 00 52 65 74 72 o.host '%s'.Retr
0280 79 7C 43 61 6E 63 65 6C 00 00 00 00 00 14 00 00 y|Cancel........
OFFSETS 0290 THROUGH 0291 CONTAIN ZERO
0292 0000024A-01 01.0000024A
0296 00000260-01 01.00000260
029A 0000027C-01 01.0000027C
029E 69 6E in
02A0 74 75 69 74 69 6F 6E 2E 6C 69 62 72 61 72 79 00 tuition.library.
02B0 45 6E 76 6F 79 20 4E 65 74 77 6F 72 6B 20 50 72 Envoy Network Pr
02C0 6F 62 6C 65 6D 00 45 72 72 6F 72 20 63 6F 64 65 oblem.Error code
02D0 3A 20 25 6C 78 00 4F 4B 00 00 00 00 00 14 00 00 : %lx.OK........
OFFSETS 02E0 THROUGH 02E1 CONTAIN ZERO
02E2 000002B0-01 01.000002B0
02E6 000002C6-01 01.000002C6
02EA 000002D6-01 01.000002D6
02EE 74 69 ti
02F0 6D 65 72 2E 64 65 76 69 63 65 00 00 mer.device..

SECTION 02 " " 00000014 BYTES
