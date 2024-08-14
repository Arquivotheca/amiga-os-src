Lattice AMIGA 68000-68020 OBJ Module Disassembler V5.04.039
Copyright © 1988, 1989 Lattice Inc.  All Rights Reserved.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

@manager 0000-00    @LoadServicesInfo 01E2-00    @PrefOpen 02DA-00    
@MakeServices 030E-00    @LocalFindService 03F2-00    @ProcessEntity 0428-00
_svc_entity_tags 0000-01    _VersionTag 0020-01    _DOSBase 0000-02    
_NIPCBase 0004-02    _UtilityBase 0008-02    _IFFParseBase 000C-02    
_ServicesBase 0010-02    _SvcBase 0014-02    _SysBase 0018-02    
_clientHostName 001C-02

SECTION 00 "manager.c" 00000528 BYTES
;   1: /*
;   2:  * * $Id: manager.c,v 37.7 92/06/09 15:16:50 kcd Exp Locker: kcd $ *
;   3:  *
;   4:  * (C) Copyright 1992, Commodore-Amiga, Inc. *
;   5:  *
;   6:  * manager.c - Envoy Services Manager *
;   7:  *
;   8:  */
;   9: 
;  10: #include <exec/types.h>
;  11: #include <exec/memory.h>
;  12: #include <exec/lists.h>
;  13: #include <exec/ports.h>
;  14: 
;  15: #include <dos/dos.h>
;  16: #include <dos/dosextens.h>
;  17: #include <dos/dostags.h>
;  18: #include <dos/rdargs.h>
;  19: 
;  20: #include <libraries/iffparse.h>
;  21: 
;  22: #include <clib/exec_protos.h>
;  23: #include <clib/dos_protos.h>
;  24: #include <clib/nipc_protos.h>
;  25: #include <clib/svc_protos.h>
;  26: #include <clib/iffparse_protos.h>
;  27: 
;  28: extern __stdargs NewList(struct List *list);
;  29: 
;  30: #include <pragmas/exec_pragmas.h>
;  31: #include <pragmas/dos_pragmas.h>
;  32: #include <pragmas/nipc_pragmas.h>
;  33: #include <pragmas/svc_pragmas.h>
;  34: #include <pragmas/iffparse_pragmas.h>
;  35: 
;  36: #define SERVICES_MANAGER oui
;  37: 
;  38: #include "services.h"
;  39: #include "servicesinternal.h"
;  40: #include "servicesbase.h"
;  41: #include "manager_rev.h"
;  42: 
;  43: #include "string.h"
;  44: 
;  45: #define SVCERR_SERVERFAIL   301UL
;  46: #define SVCERR_BADSTARTSERV     302UL
;  47: #define SVCERR_CANTLOADSERV 303UL
;  48: #define SVCERR_NOMEMORY     304UL
;  49: #define SVCERR_BADSERVICE   305UL
;  50: #define SVCERR_BADTRANS     306UL
;  51: 
;  52: /*--------------------------------------------------------------------------*/
;  53: 
;  54: /* Globals */
;  55: 
;  56: struct DosLibrary *DOSBase;
;  57: struct Library *NIPCBase;
;  58: struct Library *UtilityBase;
;  59: struct Library *IFFParseBase;
;  60: struct ServicesLib *ServicesBase;
;  61: struct Library *SvcBase;
;  62: struct ExecBase *SysBase;
;  63: struct TagItem svc_entity_tags[4] =
;  64: {
;  65:     ENT_Public, 0,
;  66:     ENT_Name, (ULONG) MGR_ENTITY_NAME,
;  67:     ENT_AllocSignal, NULL,
;  68:     TAG_DONE, 0
;  69: };
;  70: UBYTE clientHostName[128];
;  71: UBYTE VersionTag[] = VERSTAG;
;  72: 
;  73: /*--------------------------------------------------------------------------*/
;  74: 
;  75: /* Prototypes */
;  76: 
;  77: BOOL LoadServicesInfo(void);
;  78: VOID ProcessEntity(APTR svc_entity);
;  79: struct Service *LocalFindService(STRPTR svc_name);
;  80: LONG manager(void);
;  81: BPTR PrefOpen(void);
;  82: BOOL MakeServices(struct IFFHandle *iff);
;  83: 
;  84: #define ID_ISVC  MAKE_ID('I','S','V','C')
;  85: #define ID_PREF  MAKE_ID('P','R','E','F')
;  86: 
;  87: struct IFFService
;  88: {
;  89:     ULONG   is_Flags;
;  90:     UWORD   is_UID;
;  91:     UWORD   is_GID;
;  92:     UBYTE   is_PathName[256];
;  93:     UBYTE   is_SvcName[64];
;  94: };
;  95: 
;  96: #define SVCFLAGB_ENABLE     0
;  97: #define SVCFLAGF_ENABLE     (1L << SVCFLAGB_ENABLE)
;  98: 
;  99: /*--------------------------------------------------------------------------*/
; 100: 
; 101: /* Main  */
; 102: 
; 103: LONG manager()
; 104: {
       | 0000  4E55 FFEC                      LINK      A5,#FFEC
       | 0004  48E7 0132                      MOVEM.L   D7/A2-A3/A6,-(A7)
; 105:     APTR ServiceEntity;
; 106:     ULONG entitymask, waitmask, sigs;
; 107:     ULONG entitysigbit = 0;
       | 0008  7000                           MOVEQ     #00,D0
       | 000A  2B40 FFF0                      MOVE.L    D0,FFF0(A5)
; 108:     struct Message *wbmessage = NULL;
       | 000E  2640                           MOVEA.L   D0,A3
; 109:     struct Process *myproc;
; 110: 
; 111:     SysBase = (struct ExecBase *)*((void **)4);
       | 0010  307C 0004                      MOVEA.W   #0004,A0
       | 0014  2250                           MOVEA.L   (A0),A1
       | 0016  23C9  0000 0018-02             MOVE.L    A1,02.00000018
; 112:     myproc = (struct Process *)FindTask(0L);
       | 001C  2C49                           MOVEA.L   A1,A6
       | 001E  93C9                           SUBA.L    A1,A1
       | 0020  4EAE FEDA                      JSR       FEDA(A6)
       | 0024  2440                           MOVEA.L   D0,A2
; 113:     if(!myproc->pr_CLI)
       | 0026  4AAA 00AC                      TST.L     00AC(A2)
       | 002A  6616                           BNE.B     0042
; 114:     {
; 115:         WaitPort(&myproc->pr_MsgPort);
       | 002C  41EA 005C                      LEA       005C(A2),A0
       | 0030  2F48 0010                      MOVE.L    A0,0010(A7)
       | 0034  4EAE FE80                      JSR       FE80(A6)
; 116:         wbmessage = GetMsg(&myproc->pr_MsgPort);
       | 0038  206F 0010                      MOVEA.L   0010(A7),A0
       | 003C  4EAE FE8C                      JSR       FE8C(A6)
       | 0040  2640                           MOVEA.L   D0,A3
; 117:     }
; 118:     if (DOSBase = (struct DosLibrary *) OpenLibrary("dos.library", 37L))
       | 0042  43FA 011A                      LEA       011A(PC),A1
       | 0046  7025                           MOVEQ     #25,D0
       | 0048  4EAE FDD8                      JSR       FDD8(A6)
       | 004C  23C0  0000 0000-02             MOVE.L    D0,02.00000000
       | 0052  6700 00DA                      BEQ.W     012E
; 119:     {
; 120:         if (UtilityBase = (struct Library *) OpenLibrary("utility.library", 37L))
       | 0056  43FA 0112                      LEA       0112(PC),A1
       | 005A  7025                           MOVEQ     #25,D0
       | 005C  4EAE FDD8                      JSR       FDD8(A6)
       | 0060  23C0  0000 0008-02             MOVE.L    D0,02.00000008
       | 0066  6700 00B6                      BEQ.W     011E
; 121:         {
; 122:             if (NIPCBase = (struct Library *) OpenLibrary("nipc.library", 0L))
       | 006A  43FA 010E                      LEA       010E(PC),A1
       | 006E  7000                           MOVEQ     #00,D0
       | 0070  4EAE FDD8                      JSR       FDD8(A6)
       | 0074  23C0  0000 0004-02             MOVE.L    D0,02.00000004
       | 007A  6700 0092                      BEQ.W     010E
; 123:             {
; 124:                 if (ServicesBase = (struct ServicesLib *) OpenLibrary("services.library", 37L))
       | 007E  43FA 0108                      LEA       0108(PC),A1
       | 0082  7025                           MOVEQ     #25,D0
       | 0084  4EAE FDD8                      JSR       FDD8(A6)
       | 0088  23C0  0000 0010-02             MOVE.L    D0,02.00000010
       | 008E  676E                           BEQ.B     00FE
; 125:                 {
; 126:                     svc_entity_tags[2].ti_Data = (ULONG) & entitysigbit;
       | 0090  41ED FFF0                      LEA       FFF0(A5),A0
       | 0094  23C8  0000 0014-01             MOVE.L    A0,01.00000014
; 127: 
; 128:                     DBMSG(("services.library opened.\n"));
; 129: 
; 130:                     if (ServiceEntity = CreateEntityA(svc_entity_tags))
       | 009A  41F9  0000 0000-01             LEA       01.00000000,A0
       | 00A0  2C79  0000 0004-02             MOVEA.L   02.00000004,A6
       | 00A6  4EAE FF82                      JSR       FF82(A6)
       | 00AA  2440                           MOVEA.L   D0,A2
       | 00AC  200A                           MOVE.L    A2,D0
       | 00AE  673E                           BEQ.B     00EE
; 131:                     {
; 132:                         if (LoadServicesInfo());
       | 00B0  6100 0130                      BSR.W     01E2
; 133:                         {
; 134:                             DBMSG(("Services Configuration Loaded.\n"));
; 135: 
; 136:                             entitymask = (1 << entitysigbit);
       | 00B4  7001                           MOVEQ     #01,D0
       | 00B6  222D FFF0                      MOVE.L    FFF0(A5),D1
       | 00BA  E3A0                           ASL.L     D1,D0
; 137: 
; 138:                             waitmask = (entitymask | SIGBREAKF_CTRL_F);
       | 00BC  2E00                           MOVE.L    D0,D7
       | 00BE  0047 8000                      ORI.W     #8000,D7
; 139: 
; 140:                             while (TRUE)
; 141:                             {
; 142:                                 sigs = Wait(waitmask);
       | 00C2  2007                           MOVE.L    D7,D0
       | 00C4  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 00CA  4EAE FEC2                      JSR       FEC2(A6)
; 143: 
; 144:                                 DBMSG(("Awake! %lx\n",sigs));
; 145: 
; 146:                                 ProcessEntity(ServiceEntity);
       | 00CE  2F40 0010                      MOVE.L    D0,0010(A7)
       | 00D2  204A                           MOVEA.L   A2,A0
       | 00D4  6100 0352                      BSR.W     0428
; 147: 
; 148:                                 DBMSG(("Sigs after PE(): %lx\n",sigs));
; 149: 
; 150:                                 if (sigs & SIGBREAKF_CTRL_F)
       | 00D8  202F 0010                      MOVE.L    0010(A7),D0
       | 00DC  0800 000F                      BTST      #000F,D0
       | 00E0  67E0                           BEQ.B     00C2
; 151:                                 {
; 152:                                     DBMSG(("***break: CTRL_F\n"));
; 153:                                     break;
; 154:                                 }
; 155: 
; 156:                             }
; 157:                         }
; 158:                         DeleteEntity(ServiceEntity);
       | 00E2  204A                           MOVEA.L   A2,A0
       | 00E4  2C79  0000 0004-02             MOVEA.L   02.00000004,A6
       | 00EA  4EAE FF7C                      JSR       FF7C(A6)
; 159:                     }
; 160:                     CloseLibrary((struct Library *) ServicesBase);
       | 00EE  2279  0000 0010-02             MOVEA.L   02.00000010,A1
       | 00F4  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 00FA  4EAE FE62                      JSR       FE62(A6)
; 161:                 }
; 162:                 CloseLibrary(NIPCBase);
       | 00FE  2279  0000 0004-02             MOVEA.L   02.00000004,A1
       | 0104  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 010A  4EAE FE62                      JSR       FE62(A6)
; 163:             }
; 164:             CloseLibrary(UtilityBase);
       | 010E  2279  0000 0008-02             MOVEA.L   02.00000008,A1
       | 0114  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 011A  4EAE FE62                      JSR       FE62(A6)
; 165:         }
; 166:         CloseLibrary((struct Library *) DOSBase);
       | 011E  2279  0000 0000-02             MOVEA.L   02.00000000,A1
       | 0124  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 012A  4EAE FE62                      JSR       FE62(A6)
; 167:     }
; 168:     if(wbmessage)
       | 012E  200B                           MOVE.L    A3,D0
       | 0130  6710                           BEQ.B     0142
; 169:     {
; 170:         Forbid();
       | 0132  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 0138  4EAE FF7C                      JSR       FF7C(A6)
; 171:         ReplyMsg(wbmessage);
       | 013C  224B                           MOVEA.L   A3,A1
       | 013E  4EAE FE86                      JSR       FE86(A6)
; 172:     }
; 173:     return(0L);
       | 0142  7000                           MOVEQ     #00,D0
; 174: }
       | 0144  4CDF 4C80                      MOVEM.L   (A7)+,D7/A2-A3/A6
       | 0148  4E5D                           UNLK      A5
       | 014A  4E75                           RTS
       | 014C  5365                           SUBQ.W    #1,-(A5)
       | 014E  7276                           MOVEQ     #76,D1
       | 0150  6963                           BVS.B     01B5
       | 0152  6573                           BCS.B     01C7
       | 0154  204D                           MOVEA.L   A5,A0
       | 0156  616E                           BSR.B     01C6
       | 0158  6167                           BSR.B     01C1
       | 015A  6572                           BCS.B     01CE
       | 015C  0000 646F                      ORI.B     #6F,D0
       | 0160  732E                           
       | 0162  6C69                           BGE.B     01CD
       | 0164  6272                           BHI.B     01D8
       | 0166  6172                           BSR.B     01DA
       | 0168  7900                           
       | 016A  7574                           
       | 016C  696C                           BVS.B     01DA
       | 016E  6974                           BVS.B     01E4
       | 0170  792E                           
       | 0172  6C69                           BGE.B     01DD
       | 0174  6272                           BHI.B     01E8
       | 0176  6172                           BSR.B     01EA
       | 0178  7900                           
       | 017A  6E69                           BGT.B     01E5
       | 017C  7063                           MOVEQ     #63,D0
       | 017E  2E6C 6962                      MOVEA.L   6962(A4),A7
       | 0182  7261                           MOVEQ     #61,D1
       | 0184  7279                           MOVEQ     #79,D1
       | 0186  0000 7365                      ORI.B     #65,D0
       | 018A  7276                           MOVEQ     #76,D1
       | 018C  6963                           BVS.B     01F1
       | 018E  6573                           BCS.B     0203
       | 0190  2E6C 6962                      MOVEA.L   6962(A4),A7
       | 0194  7261                           MOVEQ     #61,D1
       | 0196  7279                           MOVEQ     #79,D1
       | 0198  0000 6966                      ORI.B     #66,D0
       | 019C  6670                           BNE.B     020E
       | 019E  6172                           BSR.B     0212
       | 01A0  7365                           
       | 01A2  2E6C 6962                      MOVEA.L   6962(A4),A7
       | 01A6  7261                           MOVEQ     #61,D1
       | 01A8  7279                           MOVEQ     #79,D1
       | 01AA  0000 454E                      ORI.B     #4E,D0
       | 01AE  563A 656E                      ADDQ.B    #3,656E(PC)
       | 01B2  766F                           MOVEQ     #6F,D3
       | 01B4  792F                           
       | 01B6  7365                           
       | 01B8  7276                           MOVEQ     #76,D1
       | 01BA  6963                           BVS.B     021F
       | 01BC  6573                           BCS.B     0231
       | 01BE  2E70 7265                      MOVEA.L   65(A0,D7.W*2),A7
       | 01C2  6673                           BNE.B     0237
       | 01C4  0000 454E                      ORI.B     #4E,D0
       | 01C8  5641                           ADDQ.W    #3,D1
       | 01CA  5243                           ADDQ.W    #1,D3
       | 01CC  3A65                           MOVEA.W   -(A5),A5
       | 01CE  6E76                           BGT.B     0246
       | 01D0  6F79                           BLE.B     024B
       | 01D2  2F73 6572 7669 6365 732E 7072  MOVE.L    ([76696365,A3],732E),7072(A7)
       | 01DE  6566                           BCS.B     0246
       | 01E0  7300                           
; 175: 
; 176: /*--------------------------------------------------------------------------*/
; 177: 
; 178: /* LoadServicesInfo  */
; 179: 
; 180: BOOL LoadServicesInfo()
; 181: {
       | 01E2  4E55 FFE8                      LINK      A5,#FFE8
       | 01E6  48E7 0712                      MOVEM.L   D5-D7/A3/A6,-(A7)
; 182:     struct ContextNode *top;
; 183:     ULONG error;
; 184:     BPTR IFFStream;
; 185:     struct IFFHandle *iff;
; 186:     BOOL status = FALSE, cont = TRUE;
       | 01EA  7A00                           MOVEQ     #00,D5
; 187: 
; 188:     LONG props[]={ID_PREF,ID_ISVC};
       | 01EC  41F9  0000 003E-01             LEA       01.0000003E,A0
       | 01F2  43ED FFEA                      LEA       FFEA(A5),A1
       | 01F6  22D8                           MOVE.L    (A0)+,(A1)+
       | 01F8  22D8                           MOVE.L    (A0)+,(A1)+
; 189: 
; 190:     if(IFFParseBase = OpenLibrary("iffparse.library",37))
       | 01FA  43FA FF9E                      LEA       FF9E(PC),A1
       | 01FE  7025                           MOVEQ     #25,D0
       | 0200  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 0206  4EAE FDD8                      JSR       FDD8(A6)
       | 020A  23C0  0000 000C-02             MOVE.L    D0,02.0000000C
       | 0210  6700 00BE                      BEQ.W     02D0
; 191:     {
; 192:         if(IFFStream = PrefOpen())
       | 0214  6100 00C4                      BSR.W     02DA
       | 0218  2E00                           MOVE.L    D0,D7
       | 021A  4A87                           TST.L     D7
       | 021C  6700 00A2                      BEQ.W     02C0
; 193:         {
; 194:             if(iff = AllocIFF())
       | 0220  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 0226  4EAE FFE2                      JSR       FFE2(A6)
       | 022A  2640                           MOVEA.L   D0,A3
       | 022C  200B                           MOVE.L    A3,D0
       | 022E  6700 0084                      BEQ.W     02B4
; 195:             {
; 196:                 InitIFFasDOS(iff);
       | 0232  204B                           MOVEA.L   A3,A0
       | 0234  4EAE FF16                      JSR       FF16(A6)
; 197: 
; 198:                 if(!OpenIFF(iff,IFFF_READ))
       | 0238  204B                           MOVEA.L   A3,A0
       | 023A  7000                           MOVEQ     #00,D0
       | 023C  4EAE FFDC                      JSR       FFDC(A6)
       | 0240  4A80                           TST.L     D0
       | 0242  6664                           BNE.B     02A8
; 199:                 {
; 200:                     iff->iff_Stream = (ULONG) IFFStream;
       | 0244  2687                           MOVE.L    D7,(A3)
; 201:                     if(!CollectionChunks(iff,props,1))
       | 0246  204B                           MOVEA.L   A3,A0
       | 0248  43ED FFEA                      LEA       FFEA(A5),A1
       | 024C  7001                           MOVEQ     #01,D0
       | 024E  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 0254  4EAE FF70                      JSR       FF70(A6)
       | 0258  4A80                           TST.L     D0
       | 025A  6640                           BNE.B     029C
; 202:                     {
; 203:                         while(cont)
; 204:                         {
; 205:                             error = ParseIFF(iff,IFFPARSE_STEP);
       | 025C  204B                           MOVEA.L   A3,A0
       | 025E  7001                           MOVEQ     #01,D0
       | 0260  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 0266  4EAE FFD6                      JSR       FFD6(A6)
       | 026A  2C00                           MOVE.L    D0,D6
; 206:                             if(!error)
       | 026C  4A86                           TST.L     D6
       | 026E  67EC                           BEQ.B     025C
; 207:                                 continue;
; 208:                             if(error != IFFERR_EOC)
       | 0270  70FE                           MOVEQ     #FE,D0
       | 0272  BC80                           CMP.L     D0,D6
       | 0274  6626                           BNE.B     029C
; 209:                                 break;
; 210: 
; 211:                             top = CurrentChunk(iff);
       | 0276  204B                           MOVEA.L   A3,A0
       | 0278  4EAE FF52                      JSR       FF52(A6)
; 212:                             if((top->cn_Type == ID_PREF) && (top->cn_ID == ID_FORM))
       | 027C  2040                           MOVEA.L   D0,A0
       | 027E  0CA8 5052 4546 000C            CMPI.L    #50524546,000C(A0)
       | 0286  66D4                           BNE.B     025C
       | 0288  0CA8 464F 524D 0008            CMPI.L    #464F524D,0008(A0)
       | 0290  66CA                           BNE.B     025C
; 213:                                 status = MakeServices(iff);
       | 0292  204B                           MOVEA.L   A3,A0
       | 0294  6100 0078                      BSR.W     030E
       | 0298  2A00                           MOVE.L    D0,D5
; 214:                         }
       | 029A  60C0                           BRA.B     025C
; 215:                     }
; 216:                     CloseIFF(iff);
       | 029C  204B                           MOVEA.L   A3,A0
       | 029E  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 02A4  4EAE FFD0                      JSR       FFD0(A6)
; 217:                 }
; 218:                 FreeIFF(iff);
       | 02A8  204B                           MOVEA.L   A3,A0
       | 02AA  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 02B0  4EAE FFCA                      JSR       FFCA(A6)
; 219:             }
; 220:             Close(IFFStream);
       | 02B4  2207                           MOVE.L    D7,D1
       | 02B6  2C79  0000 0000-02             MOVEA.L   02.00000000,A6
       | 02BC  4EAE FFDC                      JSR       FFDC(A6)
; 221:         }
; 222:         CloseLibrary(IFFParseBase);
       | 02C0  2279  0000 000C-02             MOVEA.L   02.0000000C,A1
       | 02C6  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 02CC  4EAE FE62                      JSR       FE62(A6)
; 223:     }
; 224:     return status;
       | 02D0  2005                           MOVE.L    D5,D0
; 225: }
       | 02D2  4CDF 48E0                      MOVEM.L   (A7)+,D5-D7/A3/A6
       | 02D6  4E5D                           UNLK      A5
       | 02D8  4E75                           RTS
; 226: 
; 227: BPTR PrefOpen(void)
; 228: {
       | 02DA  48E7 2002                      MOVEM.L   D2/A6,-(A7)
; 229:     BPTR stream;
; 230: 
; 231:     if(stream = Open("ENV:envoy/services.prefs",MODE_OLDFILE))
       | 02DE  41FA FECC                      LEA       FECC(PC),A0
       | 02E2  2208                           MOVE.L    A0,D1
       | 02E4  243C 0000 03ED                 MOVE.L    #000003ED,D2
       | 02EA  2C79  0000 0000-02             MOVEA.L   02.00000000,A6
       | 02F0  4EAE FFE2                      JSR       FFE2(A6)
       | 02F4  4A80                           TST.L     D0
       | 02F6  6610                           BNE.B     0308
; 232:         return stream;
; 233:     else if(stream = Open("ENVARC:envoy/services.prefs",MODE_OLDFILE))
       | 02F8  41FA FECC                      LEA       FECC(PC),A0
       | 02FC  2208                           MOVE.L    A0,D1
       | 02FE  4EAE FFE2                      JSR       FFE2(A6)
       | 0302  4A80                           TST.L     D0
       | 0304  6602                           BNE.B     0308
; 234:         return stream;
; 235:     else
; 236:         return (BPTR)0L;
       | 0306  7000                           MOVEQ     #00,D0
; 237: }
       | 0308  4CDF 4004                      MOVEM.L   (A7)+,D2/A6
       | 030C  4E75                           RTS
; 238: 
; 239: BOOL MakeServices(struct IFFHandle *iff)
; 240: {
       | 030E  4E55 FFF4                      LINK      A5,#FFF4
       | 0312  48E7 0032                      MOVEM.L   A2-A3/A6,-(A7)
       | 0316  2648                           MOVEA.L   A0,A3
; 241:     struct Service *new_svc, *old_svc;
; 242:     struct IFFService *is;
; 243:     struct CollectionItem *ci;
; 244: 
; 245:     if(ci = FindCollection(iff,ID_PREF,ID_ISVC))
       | 0318  204B                           MOVEA.L   A3,A0
       | 031A  203C 5052 4546                 MOVE.L    #50524546,D0
       | 0320  223C 4953 5643                 MOVE.L    #49535643,D1
       | 0326  2C79  0000 000C-02             MOVEA.L   02.0000000C,A6
       | 032C  4EAE FF5E                      JSR       FF5E(A6)
       | 0330  2640                           MOVEA.L   D0,A3
       | 0332  200B                           MOVE.L    A3,D0
       | 0334  6700 00B2                      BEQ.W     03E8
; 246:     {
; 247:         old_svc = (struct Service *) ServicesBase->SVCS_Services.mlh_Head;
       | 0338  2079  0000 0010-02             MOVEA.L   02.00000010,A0
       | 033E  2468 0060                      MOVEA.L   0060(A0),A2
; 248:         while (old_svc->svc_Node.ln_Succ)
       | 0342  6008                           BRA.B     034C
; 249:         {
; 250:             old_svc->svc_Flags |= SVCF_EXPUNGE;
       | 0344  08EA 0001 0151                 BSET      #0001,0151(A2)
; 251:             old_svc = (struct Service *) old_svc->svc_Node.ln_Succ;
       | 034A  2452                           MOVEA.L   (A2),A2
; 252:         }
       | 034C  4A92                           TST.L     (A2)
       | 034E  66F4                           BNE.B     0344
       | 0350  6000 0088                      BRA.W     03DA
; 253: 
; 254:         while(ci)
; 255:         {
; 256:             is = (struct IFFService *)ci->ci_Data;
       | 0354  246B 0008                      MOVEA.L   0008(A3),A2
; 257: 
; 258:             if (old_svc = LocalFindService(is->is_SvcName))
       | 0358  47EA 0108                      LEA       0108(A2),A3
       | 035C  204B                           MOVEA.L   A3,A0
       | 035E  6100 0092                      BSR.W     03F2
       | 0362  4A80                           TST.L     D0
       | 0364  6716                           BEQ.B     037C
; 259:             {
; 260:                 DBMSG(("Updating old Service: %s\n", is->is_SvcName));
; 261: 
; 262:                 strcpy(old_svc->svc_PathName, is->is_PathName);
       | 0366  2040                           MOVEA.L   D0,A0
       | 0368  43E8 000E                      LEA       000E(A0),A1
       | 036C  4DEA 0008                      LEA       0008(A2),A6
       | 0370  12DE                           MOVE.B    (A6)+,(A1)+
       | 0372  66FC                           BNE.B     0370
; 263:                 old_svc->svc_Flags &= ~SVCF_EXPUNGE;
       | 0374  08A8 0001 0151                 BCLR      #0001,0151(A0)
; 264:             }
; 265:             else if (new_svc = AllocMem(sizeof(struct Service), MEMF_PUBLIC | MEMF_CLEAR))
       | 037A  6058                           BRA.B     03D4
       | 037C  203C 0000 0156                 MOVE.L    #00000156,D0
       | 0382  223C 0001 0001                 MOVE.L    #00010001,D1
       | 0388  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 038E  4EAE FF3A                      JSR       FF3A(A6)
       | 0392  2F40 000C                      MOVE.L    D0,000C(A7)
       | 0396  4A80                           TST.L     D0
       | 0398  673A                           BEQ.B     03D4
; 266:             {
; 267:                 DBMSG(("Adding new Service: %s\n", is->is_SvcName));
; 268: 
; 269:                 strcpy(new_svc->svc_SvcName, (UBYTE *) is->is_SvcName);
       | 039A  2040                           MOVEA.L   D0,A0
       | 039C  43E8 010E                      LEA       010E(A0),A1
       | 03A0  2C4B                           MOVEA.L   A3,A6
       | 03A2  12DE                           MOVE.B    (A6)+,(A1)+
       | 03A4  66FC                           BNE.B     03A2
; 270:                 strcpy(new_svc->svc_PathName, (UBYTE *) is->is_PathName);
       | 03A6  43E8 000E                      LEA       000E(A0),A1
       | 03AA  4DEA 0008                      LEA       0008(A2),A6
       | 03AE  12DE                           MOVE.B    (A6)+,(A1)+
       | 03B0  66FC                           BNE.B     03AE
; 271:                 AddTail((struct List *) &ServicesBase->SVCS_Services, (struct Node *) new_svc);
       | 03B2  2279  0000 0010-02             MOVEA.L   02.00000010,A1
       | 03B8  D2FC 0060                      ADDA.W    #0060,A1
       | 03BC  2049                           MOVEA.L   A1,A0
       | 03BE  2240                           MOVEA.L   D0,A1
       | 03C0  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 03C6  4EAE FF0A                      JSR       FF0A(A6)
; 272:                 new_svc->svc_Flags &= ~SVCF_EXPUNGE;
       | 03CA  206F 000C                      MOVEA.L   000C(A7),A0
       | 03CE  08A8 0001 0151                 BCLR      #0001,0151(A0)
; 273:             }
; 274: 
; 275:             ci = ci->ci_Next;
       | 03D4  206D FFF8                      MOVEA.L   FFF8(A5),A0
       | 03D8  2650                           MOVEA.L   (A0),A3
; 276:         }
       | 03DA  2B4B FFF8                      MOVE.L    A3,FFF8(A5)
       | 03DE  200B                           MOVE.L    A3,D0
       | 03E0  6600 FF72                      BNE.W     0354
; 277:         return TRUE;
       | 03E4  7001                           MOVEQ     #01,D0
       | 03E6  6002                           BRA.B     03EA
; 278:     }
; 279:     return FALSE;
       | 03E8  7000                           MOVEQ     #00,D0
; 280: }
       | 03EA  4CDF 4C00                      MOVEM.L   (A7)+,A2-A3/A6
       | 03EE  4E5D                           UNLK      A5
       | 03F0  4E75                           RTS
; 281: 
; 282: /*--------------------------------------------------------------------------*/
; 283: 
; 284: /* LocalFindService() */
; 285: 
; 286: struct Service *LocalFindService(svc_name)
; 287: STRPTR svc_name;
; 288: {
       | 03F2  48E7 0030                      MOVEM.L   A2-A3,-(A7)
       | 03F6  2648                           MOVEA.L   A0,A3
; 289:     struct Service *svc;
; 290: 
; 291:     svc = (struct Service *) ServicesBase->SVCS_Services.mlh_Head;
       | 03F8  2079  0000 0010-02             MOVEA.L   02.00000010,A0
       | 03FE  2468 0060                      MOVEA.L   0060(A0),A2
; 292:     while (svc->svc_Node.ln_Succ)
       | 0402  6018                           BRA.B     041C
; 293:     {
; 294:         DBMSG(("LocalFindService: %s == %s ?\n", svc_name, svc->svc_SvcName));
; 295:         if (!strcmp(svc_name, svc->svc_SvcName))
       | 0404  41EA 010E                      LEA       010E(A2),A0
       | 0408  224B                           MOVEA.L   A3,A1
       | 040A  1019                           MOVE.B    (A1)+,D0
       | 040C  B018                           CMP.B     (A0)+,D0
       | 040E  660A                           BNE.B     041A
       | 0410  4A00                           TST.B     D0
       | 0412  66F6                           BNE.B     040A
       | 0414  6604                           BNE.B     041A
; 296:             return (svc);
       | 0416  200A                           MOVE.L    A2,D0
       | 0418  6008                           BRA.B     0422
; 297:         svc = (struct Service *) svc->svc_Node.ln_Succ;
       | 041A  2452                           MOVEA.L   (A2),A2
; 298:     }
       | 041C  4A92                           TST.L     (A2)
       | 041E  66E4                           BNE.B     0404
; 299:     return (NULL);
       | 0420  7000                           MOVEQ     #00,D0
; 300: }
       | 0422  4CDF 0C00                      MOVEM.L   (A7)+,A2-A3
       | 0426  4E75                           RTS
; 301: 
; 302: /*--------------------------------------------------------------------------*/
; 303: 
; 304: /* ProcessEntity */
; 305: 
; 306: VOID ProcessEntity(svc_entity)
; 307: APTR svc_entity;
; 308: {
       | 0428  4E55 FFCC                      LINK      A5,#FFCC
       | 042C  48E7 0132                      MOVEM.L   D7/A2-A3/A6,-(A7)
       | 0430  2B48 FFD0                      MOVE.L    A0,FFD0(A5)
; 309:     struct Transaction *req_trans;
; 310:     struct FindServiceReq *fsr;
; 311:     struct Service *svc;
; 312:     ULONG error;
; 313: 
; 314:     ULONG sstags[8]={SSVC_UserName, 0,
; 315:     		     SSVC_Password, 0,
; 316:     		     SSVC_EntityName, 0,
; 317:     		     SSVC_HostName,0};
       | 0434  41F9  0000 0046-01             LEA       01.00000046,A0
       | 043A  43ED FFD4                      LEA       FFD4(A5),A1
       | 043E  7007                           MOVEQ     #07,D0
       | 0440  22D8                           MOVE.L    (A0)+,(A1)+
       | 0442  51C8 FFFC                      DBF       D0,0440
; 318: 
; 319:     while (req_trans = GetTransaction(svc_entity))
       | 0446  6000 00C0                      BRA.W     0508
; 320:     {
; 321:         error = SVCERR_BADSTARTSERV; /* Assume the worst. */
       | 044A  2E3C 0000 012E                 MOVE.L    #0000012E,D7
; 322: 
; 323:         if ((req_trans->trans_ReqDataLength >= sizeof(struct FindServiceReq)) &&
       | 0450  0CAB 0000 0060 002E            CMPI.L    #00000060,002E(A3)
       | 0458  6500 009E                      BCS.W     04F8
; 324:             (req_trans->trans_Command == MGRCMD_FINDSERVICE))
       | 045C  7064                           MOVEQ     #64,D0
       | 045E  B02B 001C                      CMP.B     001C(A3),D0
       | 0462  6600 0094                      BNE.W     04F8
; 325:         {
; 326:             fsr = (struct FindServiceReq *) req_trans->trans_RequestData;
       | 0466  206B 002A                      MOVEA.L   002A(A3),A0
; 327: 
; 328:             DBMSG(("Request for Service: %s\n", fsr->fsr_Service));
; 329: 
; 330:             if (svc = LocalFindService(fsr->fsr_Service))
       | 046A  2F48 0010                      MOVE.L    A0,0010(A7)
       | 046E  6182                           BSR.B     03F2
       | 0470  2440                           MOVEA.L   D0,A2
       | 0472  200A                           MOVE.L    A2,D0
       | 0474  6700 0082                      BEQ.W     04F8
; 331:             {
; 332:                 if (!(svc->svc_Flags & SVCF_EXPUNGE))
       | 0478  082A 0001 0151                 BTST      #0001,0151(A2)
       | 047E  6678                           BNE.B     04F8
; 333:                 {
; 334:                     DBMSG(("Opening %s\n",svc->svc_PathName));
; 335: 
; 336:                     if(SvcBase = OpenLibrary(svc->svc_PathName,0L))
       | 0480  41EA 000E                      LEA       000E(A2),A0
       | 0484  2248                           MOVEA.L   A0,A1
       | 0486  7000                           MOVEQ     #00,D0
       | 0488  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 048E  4EAE FDD8                      JSR       FDD8(A6)
       | 0492  23C0  0000 0014-02             MOVE.L    D0,02.00000014
       | 0498  675E                           BEQ.B     04F8
; 337:                     {
; 338:                         DBMSG(("Service Opened okay!\n"));
; 339:                         GetHostName(req_trans->trans_SourceEntity,clientHostName,127);
       | 049A  206B 0014                      MOVEA.L   0014(A3),A0
       | 049E  43F9  0000 001C-02             LEA       02.0000001C,A1
       | 04A4  707F                           MOVEQ     #7F,D0
       | 04A6  2C79  0000 0004-02             MOVEA.L   02.00000004,A6
       | 04AC  4EAE FF34                      JSR       FF34(A6)
; 340: 
; 341:                         sstags[1]=(ULONG)fsr->fsr_UserName;
       | 04B0  206F 0010                      MOVEA.L   0010(A7),A0
       | 04B4  43E8 0040                      LEA       0040(A0),A1
       | 04B8  2B49 FFD8                      MOVE.L    A1,FFD8(A5)
; 342:                         sstags[3]=(ULONG)fsr->fsr_PassWord;
       | 04BC  43E8 0050                      LEA       0050(A0),A1
       | 04C0  2B49 FFE0                      MOVE.L    A1,FFE0(A5)
; 343:                         sstags[5]=(ULONG)req_trans->trans_ResponseData;
       | 04C4  2B6B 0036 FFE8                 MOVE.L    0036(A3),FFE8(A5)
; 344:                         sstags[7]=(ULONG)clientHostName;
       | 04CA  2B7C  0000 001C-02  FFF0       MOVE.L    #02.0000001C,FFF0(A5)
; 345: 
; 346:                         error = StartServiceA((struct TagItem *)sstags);
       | 04D2  41ED FFD4                      LEA       FFD4(A5),A0
       | 04D6  2C79  0000 0014-02             MOVEA.L   02.00000014,A6
       | 04DC  4EAE FFDC                      JSR       FFDC(A6)
       | 04E0  2E00                           MOVE.L    D0,D7
; 347: 
; 348: /*                        error=StartService(SSVC_UserName,fsr->fsr_UserName,
; 349:                         		   SSVC_Password,fsr->fsr_PassWord,
; 350:                         		   SSVC_EntityName,req_trans->trans_ResponseData,
; 351:                         		   SSVC_HostName,clientHostName); */
; 352: 
; 353:                         DBMSG(("StartService() Returned %ld, EntityName: %s\n",error,req_trans->trans_ResponseData));
; 354:                         req_trans->trans_RespDataActual = sizeof(struct FindServiceAck);
       | 04E2  7040                           MOVEQ     #40,D0
       | 04E4  2740 003E                      MOVE.L    D0,003E(A3)
; 355:                         CloseLibrary(SvcBase);
       | 04E8  2279  0000 0014-02             MOVEA.L   02.00000014,A1
       | 04EE  2C79  0000 0018-02             MOVEA.L   02.00000018,A6
       | 04F4  4EAE FE62                      JSR       FE62(A6)
; 356:                     }
; 357:                 }
; 358:             }
; 359:         }
; 360:         req_trans->trans_Error = error;
       | 04F8  2747 001E                      MOVE.L    D7,001E(A3)
; 361:         ReplyTransaction(req_trans);
       | 04FC  224B                           MOVEA.L   A3,A1
       | 04FE  2C79  0000 0004-02             MOVEA.L   02.00000004,A6
       | 0504  4EAE FF58                      JSR       FF58(A6)
; 362:     }
       | 0508  206D FFD0                      MOVEA.L   FFD0(A5),A0
       | 050C  2C79  0000 0004-02             MOVEA.L   02.00000004,A6
       | 0512  4EAE FF5E                      JSR       FF5E(A6)
       | 0516  2640                           MOVEA.L   D0,A3
       | 0518  200B                           MOVE.L    A3,D0
       | 051A  6600 FF2E                      BNE.W     044A
; 363: }
       | 051E  4CDF 4C80                      MOVEM.L   (A7)+,D7/A2-A3/A6
       | 0522  4E5D                           UNLK      A5
       | 0524  4E75                           RTS

SECTION 01 " " 00000068 BYTES
0000 80 0B 10 02 00 00 00 00 80 0B 10 01 ............
000C 0000014C-00 00.0000014C
0010 80 0B 10 04 00 00 00 00 00 00 00 00 00 00 00 00 ................
0020 00 24 56 45 52 3A 20 6D 61 6E 61 67 65 72 20 33 .$VER: manager 3
0030 37 2E 37 20 28 39 2E 36 2E 39 32 29 00 00 50 52 7.7 (9.6.92)..PR
0040 45 46 49 53 56 43 80 00 20 01 00 00 00 00 80 00 EFISVC.. .......
0050 20 02 00 00 00 00 80 00 20 03 00 00 00 00 80 00  ....... .......
0060 20 04 00 00 00 00 00 00  .......

SECTION 02 " " 0000009C BYTES
