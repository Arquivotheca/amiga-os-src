Lattice AMIGA 68000-68020 OBJ Module Disassembler V5.04.039
Copyright © 1988, 1989 Lattice Inc.  All Rights Reserved.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_StartService 0000-00    _GetServiceAttrsA 01C8-00    _Server 01FA-00

SECTION 00 "lpd.c" 00000830 BYTES
;   1: #include <exec/types.h>
;   2: #include <exec/memory.h>
;   3: #include <exec/ports.h>
;   4: #include <exec/semaphores.h>
;   5: #include <exec/io.h>
;   6: #include <devices/printer.h>
;   7: #include <devices/timer.h>
;   8: #include <devices/parallel.h>
;   9: #include <devices/serial.h>
;  10: #include <envoy/nipc.h>
;  11: #include <envoy/services.h>
;  12: #include "lpdbase.h"
;  13: 
;  14: #include <pragmas/exec_pragmas.h>
;  15: #include <pragmas/nipc_pragmas.h>
;  16: #include <pragmas/dos_pragmas.h>
;  17: #include <pragmas/utility_pragmas.h>
;  18: 
;  19: #include <utility/tagitem.h>
;  20: #include <clib/dos_protos.h>
;  21: #include <clib/exec_protos.h>
;  22: #include <clib/nipc_protos.h>
;  23: #include <clib/alib_protos.h>
;  24: #include <clib/utility_protos.h>
;  25: 
;  26: #include "dos.h"
;  27: #include "string.h"
;  28: 
;  29: #define LPCMD_START_PRT 1
;  30: #define LPCMD_START_PAR 2
;  31: #define LPCMD_START_SER 3
;  32: #define LPCMD_DATA      4
;  33: #define LPCMD_END       5
;  34: 
;  35: #define LP_IDLE         1
;  36: #define LP_PRINTING     2
;  37: #define LP_PAUSED       3
;  38: #define LP_SLEEPING     4
;  39: #define LP_ERROR        255
;  40: 
;  41: struct PrinterJob
;  42: {
;  43:     struct MinNode pj_Node;
;  44:     BPTR           pj_File;
;  45:     ULONG          pj_Type;
;  46:     ULONG          pj_Status;
;  47:     ULONG          pj_JobNum;
;  48:     ULONG          pj_Size;
;  49:     UBYTE          pj_JobName[32];
;  50:     UBYTE          pj_FileName[256];
;  51: };
;  52: 
;  53: extern STRPTR       LPDUser;
;  54: extern STRPTR       LPDPassword;
;  55: extern STRPTR       LPDEntityName;
;  56: extern ULONG        LPDSignalMask;
;  57: extern ULONG        LPDError;
;  58: extern struct Task *LPDSMProc;
;  59: 
;  60: extern __far LPDServer(void);
;  61: 
;  62: ULONG ASM StartService(REG(a0) STRPTR userName,
;  63:                        REG(a1) STRPTR password,
;  64:                        REG(a2) STRPTR entityName)
;  65: {
       | 0000  4E55 FFE0                      LINK      A5,#FFE0
       | 0004  48E7 0032                      MOVEM.L   A2-A3/A6,-(A7)
       | 0008  2648                           MOVEA.L   A0,A3
       | 000A  2B49 FFE8                      MOVE.L    A1,FFE8(A5)
;  66:     ULONG cptags[5];
;  67:     struct Process *lpdproc;
;  68:     BOOL status = FALSE;
;  69:     UBYTE  sigbit;
;  70:     geta4();
       | 000E  41EE  0000-XX.2                LEA       _LinkerDB(A6),A0
;  71:     kprintf("FUCK!!!\n");
       | 0012  487A 0102                      PEA       0102(PC)
       | 0016  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 001A  584F                           ADDQ.W    #4,A7
;  72:     if(!LPDBase->LPD_Entity)
       | 001C  4AAE 0032                      TST.L     0032(A6)
       | 0020  6600 00CC                      BNE.W     00EE
;  73:     {
;  74:         kprintf("Starting process.\n");
       | 0024  487A 00FA                      PEA       00FA(PC)
       | 0028  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 002C  584F                           ADDQ.W    #4,A7
;  75:         if(sigbit = AllocSignal(-1L))
       | 002E  2F0E                           MOVE.L    A6,-(A7)
       | 0030  70FF                           MOVEQ     #FF,D0
       | 0032  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0036  4EAE FEB6                      JSR       FEB6(A6)
       | 003A  2C5F                           MOVEA.L   (A7)+,A6
       | 003C  1F40 0010                      MOVE.B    D0,0010(A7)
       | 0040  4A00                           TST.B     D0
       | 0042  6700 00C4                      BEQ.W     0108
;  76:         {
;  77:             LPDSMProc = FindTask(0L);
       | 0046  2F0E                           MOVE.L    A6,-(A7)
       | 0048  93C9                           SUBA.L    A1,A1
       | 004A  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 004E  4EAE FEDA                      JSR       FEDA(A6)
       | 0052  2C5F                           MOVEA.L   (A7)+,A6
       | 0054  23C0  0000 0000-XX             MOVE.L    D0,_LPDSMProc
;  78:             LPDSignalMask = (1L<<sigbit);
       | 005A  102F 0010                      MOVE.B    0010(A7),D0
       | 005E  7200                           MOVEQ     #00,D1
       | 0060  1200                           MOVE.B    D0,D1
       | 0062  7001                           MOVEQ     #01,D0
       | 0064  E3A0                           ASL.L     D1,D0
       | 0066  23C0  0000 0000-XX             MOVE.L    D0,_LPDSignalMask
;  79: 
;  80:             cptags[0] = NP_Entry;
       | 006C  2B7C 8000 03EB FFEC            MOVE.L    #800003EB,FFEC(A5)
;  81:             cptags[1] = (ULONG) LPDServer;
       | 0074  2B7C  0000 0000-XX  FFF0       MOVE.L    #_LPDServer,FFF0(A5)
;  82:             cptags[2] = NP_Name;
       | 007C  2B7C 8000 03F4 FFF4            MOVE.L    #800003F4,FFF4(A5)
;  83:             cptags[3] = (ULONG) "LPD Daemon";
       | 0084  41FA 00AE                      LEA       00AE(PC),A0
       | 0088  2B48 FFF8                      MOVE.L    A0,FFF8(A5)
;  84:             cptags[4] = TAG_DONE;
       | 008C  42AD FFFC                      CLR.L     FFFC(A5)
;  85: 
;  86:             LPDUser = userName;
       | 0090  23CB  0000 0000-XX             MOVE.L    A3,_LPDUser
;  87:             LPDPassword = password;
       | 0096  23ED FFE8  0000 0000-XX        MOVE.L    FFE8(A5),_LPDPassword
;  88:             LPDEntityName = entityName;
       | 009E  23CA  0000 0000-XX             MOVE.L    A2,_LPDEntityName
;  89: 
;  90:             if(lpdproc = CreateNewProc((struct TagItem *)cptags))
       | 00A4  2F40 000C                      MOVE.L    D0,000C(A7)
       | 00A8  2F0E                           MOVE.L    A6,-(A7)
       | 00AA  41ED FFEC                      LEA       FFEC(A5),A0
       | 00AE  2208                           MOVE.L    A0,D1
       | 00B0  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 00B4  4EAE FE0E                      JSR       FE0E(A6)
       | 00B8  2C5F                           MOVEA.L   (A7)+,A6
       | 00BA  4A80                           TST.L     D0
       | 00BC  6710                           BEQ.B     00CE
;  91:             {
;  92:                 Wait(1L<<sigbit);
       | 00BE  2F0E                           MOVE.L    A6,-(A7)
       | 00C0  202F 0010                      MOVE.L    0010(A7),D0
       | 00C4  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 00C8  4EAE FEC2                      JSR       FEC2(A6)
       | 00CC  2C5F                           MOVEA.L   (A7)+,A6
;  93:                 status = TRUE;
;  94:             }
;  95:             FreeSignal(sigbit);
       | 00CE  7000                           MOVEQ     #00,D0
       | 00D0  102F 0010                      MOVE.B    0010(A7),D0
       | 00D4  2F0E                           MOVE.L    A6,-(A7)
       | 00D6  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 00DA  4EAE FEB0                      JSR       FEB0(A6)
       | 00DE  2C5F                           MOVEA.L   (A7)+,A6
;  96:             kprintf("SS(): %s\n",entityName);
       | 00E0  2F0A                           MOVE.L    A2,-(A7)
       | 00E2  487A 005C                      PEA       005C(PC)
       | 00E6  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 00EA  504F                           ADDQ.W    #8,A7
;  97:         }
;  98:     }
;  99:     else
       | 00EC  601A                           BRA.B     0108
; 100:     {
; 101:         LPDError = 0;
       | 00EE  42B9  0000 0000-XX             CLR.L     _LPDError
; 102:         kprintf("Process already running.\n");
       | 00F4  487A 0054                      PEA       0054(PC)
       | 00F8  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 00FC  584F                           ADDQ.W    #4,A7
; 103:         strcpy(entityName,"LPD_Service");
       | 00FE  41FA 0064                      LEA       0064(PC),A0
       | 0102  224A                           MOVEA.L   A2,A1
       | 0104  12D8                           MOVE.B    (A0)+,(A1)+
       | 0106  66FC                           BNE.B     0104
; 104:     }
; 105:     return LPDError;
       | 0108  2039  0000 0000-XX             MOVE.L    _LPDError,D0
; 106: }
       | 010E  4CDF 4C00                      MOVEM.L   (A7)+,A2-A3/A6
       | 0112  4E5D                           UNLK      A5
       | 0114  4E75                           RTS
       | 0116  4655                           NOT.W     (A5)
       | 0118  434B                           
       | 011A  2121                           MOVE.L    -(A1),-(A0)
       | 011C  210A                           MOVE.L    A2,-(A0)
       | 011E  0000 5374                      ORI.B     #74,D0
       | 0122  6172                           BSR.B     0196
       | 0124  7469                           MOVEQ     #69,D2
       | 0126  6E67                           BGT.B     018F
       | 0128  2070 726F                      MOVEA.L   6F(A0,D7.W*2),A0
       | 012C  6365                           BLS.B     0193
       | 012E  7373                           
       | 0130  2E0A                           MOVE.L    A2,D7
       | 0132  0000 4C50                      ORI.B     #50,D0
       | 0136  4420                           NEG.B     -(A0)
       | 0138  4461                           NEG.W     -(A1)
       | 013A  656D                           BCS.B     01A9
       | 013C  6F6E                           BLE.B     01AC
       | 013E  0000 5353                      ORI.B     #53,D0
       | 0142  2829 3A20                      MOVE.L    3A20(A1),D4
       | 0146  2573 0A00 5072                 MOVE.L    00(A3,D0.L*2),5072(A2)
       | 014C  6F63                           BLE.B     01B1
       | 014E  6573                           BCS.B     01C3
       | 0150  7320                           
       | 0152  616C                           BSR.B     01C0
       | 0154  7265                           MOVEQ     #65,D1
       | 0156  6164                           BSR.B     01BC
       | 0158  7920                           
       | 015A  7275                           MOVEQ     #75,D1
       | 015C  6E6E                           BGT.B     01CC
       | 015E  696E                           BVS.B     01CE
       | 0160  672E                           BEQ.B     0190
       | 0162  0A00 4C50                      EORI.B    #50,D0
       | 0166  445F                           NEG.W     (A7)+
       | 0168  5365                           SUBQ.W    #1,-(A5)
       | 016A  7276                           MOVEQ     #76,D1
       | 016C  6963                           BVS.B     01D1
       | 016E  6500 5072                      BCS.W     51E2
       | 0172  696E                           BVS.B     01E2
       | 0174  7465                           MOVEQ     #65,D2
       | 0176  725F                           MOVEQ     #5F,D1
       | 0178  5365                           SUBQ.W    #1,-(A5)
       | 017A  7276                           MOVEQ     #76,D1
       | 017C  6963                           BVS.B     01E1
       | 017E  6500 6C70                      BCS.W     6DF0
       | 0182  642E                           BCC.B     01B2
       | 0184  7365                           
       | 0186  7276                           MOVEQ     #76,D1
       | 0188  6963                           BVS.B     01ED
       | 018A  6500 7370                      BCS.W     74FC
       | 018E  6F6F                           BLE.B     01FF
       | 0190  6C3A                           BGE.B     01CC
       | 0192  6A6F                           BPL.B     0203
       | 0194  6225                           BHI.B     01BB
       | 0196  6C64                           BGE.B     01FC
       | 0198  0000 7072                      ORI.B     #72,D0
       | 019C  696E                           BVS.B     020C
       | 019E  7465                           MOVEQ     #65,D2
       | 01A0  722E                           MOVEQ     #2E,D1
       | 01A2  6465                           BCC.B     0209
       | 01A4  7669                           MOVEQ     #69,D3
       | 01A6  6365                           BLS.B     020D
       | 01A8  0000 7061                      ORI.B     #61,D0
       | 01AC  7261                           MOVEQ     #61,D1
       | 01AE  6C6C                           BGE.B     021C
       | 01B0  656C                           BCS.B     021E
       | 01B2  2E64                           MOVEA.L   -(A4),A7
       | 01B4  6576                           BCS.B     022C
       | 01B6  6963                           BVS.B     021B
       | 01B8  6500 7469                      BCS.W     7623
       | 01BC  6D65                           BLT.B     0223
       | 01BE  722E                           MOVEQ     #2E,D1
       | 01C0  6465                           BCC.B     0227
       | 01C2  7669                           MOVEQ     #69,D3
       | 01C4  6365                           BLS.B     022B
       | 01C6  0000 48E7                      ORI.B     #E7,D0
; 107: 
; 108: VOID ASM GetServiceAttrsA(REG(a0) struct TagItem *tagList)
; 109: {
       | 01CA  0012 2648                      ORI.B     #48,(A2)
; 110:     struct TagItem *ti;
; 111: 
; 112:     if(ti=FindTagItem( SVCAttrs_Name, tagList))
       | 01CE  2F0E                           MOVE.L    A6,-(A7)
       | 01D0  203C 8000 1001                 MOVE.L    #80001001,D0
       | 01D6  204B                           MOVEA.L   A3,A0
       | 01D8  2C6E 002E                      MOVEA.L   002E(A6),A6
       | 01DC  4EAE FFE2                      JSR       FFE2(A6)
       | 01E0  2C5F                           MOVEA.L   (A7)+,A6
       | 01E2  4A80                           TST.L     D0
       | 01E4  670E                           BEQ.B     01F4
; 113:     {
; 114:         strcpy((STRPTR)ti->ti_Data,"Printer_Service");
       | 01E6  2240                           MOVEA.L   D0,A1
       | 01E8  2069 0004                      MOVEA.L   0004(A1),A0
       | 01EC  43FA FF82                      LEA       FF82(PC),A1
       | 01F0  10D9                           MOVE.B    (A1)+,(A0)+
       | 01F2  66FC                           BNE.B     01F0
; 115:     }
; 116: }
       | 01F4  4CDF 4800                      MOVEM.L   (A7)+,A3/A6
       | 01F8  4E75                           RTS
; 117: 
; 118: VOID ASM Server(REG(a0) STRPTR userName,
; 119:                 REG(a1) STRPTR password,
; 120:                 REG(a2) STRPTR EntityName)
; 121: {
       | 01FA  4E55 FF80                      LINK      A5,#FF80
       | 01FE  48E7 3F32                      MOVEM.L   D2-D7/A2-A3/A6,-(A7)
       | 0202  48ED 0700 FFB0                 MOVEM.L   A0-A2,FFB0(A5)
; 122:     struct Library *SvcBase;
; 123:     struct LPDSvc *lb = LPDBase;
; 124:     ULONG nextfilenumber,signals,die,dead;
; 125:     ULONG status,loop,waitmask;
; 126:     ULONG StartupError = 0;
; 127:     ULONG ent_sigbit;
; 128:     struct MsgPort *prt_port,*timer_port;
; 129: 
; 130:     struct PrinterJob *inc_job,*prt_job = NULL;
       | 0208  42AD FFDC                      CLR.L     FFDC(A5)
; 131: 
; 132:     UBYTE *prt_buffer;
; 133: 
; 134:     struct IOStdReq *prt_io;
; 135:     struct timerequest *timer_io;
; 136: 
; 137:     void *entity;
; 138:     struct Transaction *trans;
; 139: 
; 140:     ULONG cetags[8]={ENT_Name, 0, ENT_Public, TRUE, ENT_AllocSignal, 0, TAG_END, 0};
       | 020C  2F4E 0048                      MOVE.L    A6,0048(A7)
       | 0210  41F9  0000 0000-01             LEA       01.00000000,A0
       | 0216  43ED FFBC                      LEA       FFBC(A5),A1
       | 021A  7007                           MOVEQ     #07,D0
       | 021C  22D8                           MOVE.L    (A0)+,(A1)+
       | 021E  51C8 FFFC                      DBF       D0,021C
; 141: 
; 142:     geta4();
       | 0222  41EE  0000-XX.2                LEA       _LinkerDB(A6),A0
; 143:     nextfilenumber = 1;
       | 0226  7A01                           MOVEQ     #01,D5
; 144:     die = FALSE;
; 145:     dead = FALSE;
       | 0228  7800                           MOVEQ     #00,D4
; 146:     ent_sigbit = 0;
       | 022A  42AD FFE0                      CLR.L     FFE0(A5)
; 147:     cetags[1] = (ULONG) "LPD_Service";
       | 022E  41FA FF34                      LEA       FF34(PC),A0
       | 0232  2B48 FFC0                      MOVE.L    A0,FFC0(A5)
; 148:     cetags[5] = (ULONG) &ent_sigbit;
       | 0236  41ED FFE0                      LEA       FFE0(A5),A0
       | 023A  2B48 FFD0                      MOVE.L    A0,FFD0(A5)
; 149: 
; 150:     StartupError = -1;
       | 023E  70FF                           MOVEQ     #FF,D0
       | 0240  2B40 FFE4                      MOVE.L    D0,FFE4(A5)
; 151:     if (SvcBase = OpenLibrary("lpd.service", 0L))
       | 0244  2F0E                           MOVE.L    A6,-(A7)
       | 0246  43FA FF38                      LEA       FF38(PC),A1
       | 024A  7000                           MOVEQ     #00,D0
       | 024C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0250  4EAE FDD8                      JSR       FDD8(A6)
       | 0254  2C5F                           MOVEA.L   (A7)+,A6
       | 0256  2F40 0044                      MOVE.L    D0,0044(A7)
       | 025A  4A80                           TST.L     D0
       | 025C  6700 05C8                      BEQ.W     0826
; 152:     {
; 153:         if (entity = CreateEntityA((struct TagItem *) cetags))
       | 0260  2F0E                           MOVE.L    A6,-(A7)
       | 0262  41ED FFBC                      LEA       FFBC(A5),A0
       | 0266  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 026A  4EAE FF82                      JSR       FF82(A6)
       | 026E  2C5F                           MOVEA.L   (A7)+,A6
       | 0270  2F40 0040                      MOVE.L    D0,0040(A7)
       | 0274  4A80                           TST.L     D0
       | 0276  6700 056C                      BEQ.W     07E4
; 154:         {
; 155:             if(prt_buffer = AllocMem(1024,MEMF_CLEAR));
       | 027A  2F0E                           MOVE.L    A6,-(A7)
       | 027C  7040                           MOVEQ     #40,D0
       | 027E  E988                           LSL.L     #4,D0
       | 0280  7201                           MOVEQ     #01,D1
       | 0282  4841                           SWAP      D1
       | 0284  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0288  4EAE FF3A                      JSR       FF3A(A6)
       | 028C  2C5F                           MOVEA.L   (A7)+,A6
; 156:             {
; 157:                 if(prt_port = CreateMsgPort())
       | 028E  2F40 003C                      MOVE.L    D0,003C(A7)
       | 0292  2F0E                           MOVE.L    A6,-(A7)
       | 0294  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0298  4EAE FD66                      JSR       FD66(A6)
       | 029C  2C5F                           MOVEA.L   (A7)+,A6
       | 029E  2640                           MOVEA.L   D0,A3
       | 02A0  204B                           MOVEA.L   A3,A0
       | 02A2  2F48 0038                      MOVE.L    A0,0038(A7)
       | 02A6  200B                           MOVE.L    A3,D0
       | 02A8  6700 0516                      BEQ.W     07C0
; 158:                 {
; 159:                     if(timer_port = CreateMsgPort())
       | 02AC  2F0E                           MOVE.L    A6,-(A7)
       | 02AE  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 02B2  4EAE FD66                      JSR       FD66(A6)
       | 02B6  2C5F                           MOVEA.L   (A7)+,A6
       | 02B8  2440                           MOVEA.L   D0,A2
       | 02BA  204A                           MOVEA.L   A2,A0
       | 02BC  2F48 0034                      MOVE.L    A0,0034(A7)
       | 02C0  200A                           MOVE.L    A2,D0
       | 02C2  6700 04EE                      BEQ.W     07B2
; 160:                     {
; 161:                         if(timer_io = (struct timerequest *) CreateIORequest(timer_port,sizeof(struct timerequest)))
       | 02C6  2F0E                           MOVE.L    A6,-(A7)
       | 02C8  204A                           MOVEA.L   A2,A0
       | 02CA  7028                           MOVEQ     #28,D0
       | 02CC  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 02D0  4EAE FD72                      JSR       FD72(A6)
       | 02D4  2C5F                           MOVEA.L   (A7)+,A6
       | 02D6  2F40 0030                      MOVE.L    D0,0030(A7)
       | 02DA  4A80                           TST.L     D0
       | 02DC  6700 04C6                      BEQ.W     07A4
; 162:                         {
; 163:                             if(prt_io = (struct IOStdReq *) CreateIORequest(prt_port,sizeof(struct IOExtPar)))
       | 02E0  2F0E                           MOVE.L    A6,-(A7)
       | 02E2  204B                           MOVEA.L   A3,A0
       | 02E4  703E                           MOVEQ     #3E,D0
       | 02E6  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 02EA  4EAE FD72                      JSR       FD72(A6)
       | 02EE  2C5F                           MOVEA.L   (A7)+,A6
       | 02F0  2F40 002C                      MOVE.L    D0,002C(A7)
       | 02F4  4A80                           TST.L     D0
       | 02F6  6700 0494                      BEQ.W     078C
; 164:                             {
; 165:                                 LPDBase->LPD_Entity = entity;
       | 02FA  2D6F 0040 0032                 MOVE.L    0040(A7),0032(A6)
; 166: 
; 167:                                 strcpy(EntityName,"LPD_Service");
       | 0300  41FA FE62                      LEA       FE62(PC),A0
       | 0304  226D FFB8                      MOVEA.L   FFB8(A5),A1
       | 0308  12D8                           MOVE.B    (A0)+,(A1)+
       | 030A  66FC                           BNE.B     0308
; 168: 
; 169:                                 LPDError = StartupError = 0;
       | 030C  7200                           MOVEQ     #00,D1
       | 030E  23C1  0000 0000-XX             MOVE.L    D1,_LPDError
; 170: 
; 171:                                 Signal(LPDSMProc, LPDSignalMask);
       | 0314  2B41 FFE4                      MOVE.L    D1,FFE4(A5)
       | 0318  2F0E                           MOVE.L    A6,-(A7)
       | 031A  2279  0000 0000-XX             MOVEA.L   _LPDSMProc,A1
       | 0320  2039  0000 0000-XX             MOVE.L    _LPDSignalMask,D0
       | 0326  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 032A  4EAE FEBC                      JSR       FEBC(A6)
       | 032E  2C5F                           MOVEA.L   (A7)+,A6
; 172: 
; 173:                                 waitmask = (1<<prt_port->mp_SigBit) | (1<<ent_sigbit) | (1<<timer_port->mp_SigBit) | SIGBREAKF_CTRL_F;
       | 0330  7000                           MOVEQ     #00,D0
       | 0332  102A 000F                      MOVE.B    000F(A2),D0
       | 0336  7201                           MOVEQ     #01,D1
       | 0338  2401                           MOVE.L    D1,D2
       | 033A  E1A2                           ASL.L     D0,D2
       | 033C  2001                           MOVE.L    D1,D0
       | 033E  262D FFE0                      MOVE.L    FFE0(A5),D3
       | 0342  E7A0                           ASL.L     D3,D0
       | 0344  7600                           MOVEQ     #00,D3
       | 0346  162B 000F                      MOVE.B    000F(A3),D3
       | 034A  2F43 0050                      MOVE.L    D3,0050(A7)
       | 034E  2601                           MOVE.L    D1,D3
       | 0350  2F42 0024                      MOVE.L    D2,0024(A7)
       | 0354  242F 0050                      MOVE.L    0050(A7),D2
       | 0358  E5A3                           ASL.L     D2,D3
       | 035A  8680                           OR.L      D0,D3
       | 035C  202F 0024                      MOVE.L    0024(A7),D0
       | 0360  8680                           OR.L      D0,D3
       | 0362  0043 8000                      ORI.W     #8000,D3
; 174: 
; 175:                                 status = LP_IDLE;
       | 0366  2E01                           MOVE.L    D1,D7
; 176: 
; 177:                                 while(TRUE)
       | 0368  2F43 0028                      MOVE.L    D3,0028(A7)
; 178:                                 {
; 179:                                     signals = Wait(waitmask);
       | 036C  2F0E                           MOVE.L    A6,-(A7)
       | 036E  202F 002C                      MOVE.L    002C(A7),D0
       | 0372  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0376  4EAE FEC2                      JSR       FEC2(A6)
       | 037A  2C5F                           MOVEA.L   (A7)+,A6
; 180: 
; 181:                                   /*  if(signals & SIGBREAKF_CTRL_F)
; 182:                                         die = TRUE;
; 183:                                     */
; 184:                                     loop = TRUE;
       | 037C  7C01                           MOVEQ     #01,D6
; 185: 
; 186:                                     while(loop)
       | 037E  6000 03F0                      BRA.W     0770
; 187:                                     {
; 188:                                         loop = FALSE;
       | 0382  7C00                           MOVEQ     #00,D6
; 189:                                         if(trans = GetTransaction(entity))
       | 0384  2F0E                           MOVE.L    A6,-(A7)
       | 0386  206F 0044                      MOVEA.L   0044(A7),A0
       | 038A  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 038E  4EAE FF5E                      JSR       FF5E(A6)
       | 0392  2C5F                           MOVEA.L   (A7)+,A6
       | 0394  2440                           MOVEA.L   D0,A2
       | 0396  200A                           MOVE.L    A2,D0
       | 0398  6700 0176                      BEQ.W     0510
; 190:                                         {
; 191:                                             loop = TRUE;
       | 039C  7C01                           MOVEQ     #01,D6
; 192:                                             switch(trans->trans_Command)
       | 039E  7000                           MOVEQ     #00,D0
       | 03A0  102A 001C                      MOVE.B    001C(A2),D0
       | 03A4  5340                           SUBQ.W    #1,D0
       | 03A6  6D00 0168                      BLT.W     0510
       | 03AA  0C40 0005                      CMPI.W    #0005,D0
       | 03AE  6C00 0160                      BGE.W     0510
       | 03B2  D040                           ADD.W     D0,D0
       | 03B4  303B 0006                      MOVE.W    06(PC,D0.W),D0
       | 03B8  4EFB 0004                      JMP       04(PC,D0.W)
       | 03BC  0008 0008                      ORI.B     #08,A0
       | 03C0  0008 00C6                      ORI.B     #C6,A0
       | 03C4  0102                           BTST.L    D0,D2
; 193:                                             {
; 194:                                                 case LPCMD_START_PRT:
; 195:                                                 case LPCMD_START_SER:
; 196:                                                 case LPCMD_START_PAR:   if(inc_job = AllocMem(sizeof(struct PrinterJob),MEMF_CLEAR))
       | 03C6  2F0E                           MOVE.L    A6,-(A7)
       | 03C8  704F                           MOVEQ     #4F,D0
       | 03CA  E588                           LSL.L     #2,D0
       | 03CC  7201                           MOVEQ     #01,D1
       | 03CE  4841                           SWAP      D1
       | 03D0  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 03D4  4EAE FF3A                      JSR       FF3A(A6)
       | 03D8  2C5F                           MOVEA.L   (A7)+,A6
       | 03DA  2640                           MOVEA.L   D0,A3
       | 03DC  200B                           MOVE.L    A3,D0
       | 03DE  6700 0088                      BEQ.W     0468
; 197:                                                                         {
; 198:                                                                             sprintf(inc_job->pj_JobName,"spool:job%ld",nextfilenumber);
       | 03E2  41EB 001C                      LEA       001C(A3),A0
       | 03E6  2F05                           MOVE.L    D5,-(A7)
       | 03E8  487A FDA2                      PEA       FDA2(PC)
       | 03EC  2F08                           MOVE.L    A0,-(A7)
       | 03EE  2F48 0030                      MOVE.L    A0,0030(A7)
       | 03F2  4EBA  0000-XX.1                JSR       _sprintf(PC)
       | 03F6  4FEF 000C                      LEA       000C(A7),A7
; 199:                                                                             nextfilenumber++;
       | 03FA  5285                           ADDQ.L    #1,D5
; 200:                                                                             if(inc_job->pj_File = Open(inc_job->pj_JobName,MODE_NEWFILE))
       | 03FC  2F0E                           MOVE.L    A6,-(A7)
       | 03FE  222F 0028                      MOVE.L    0028(A7),D1
       | 0402  243C 0000 03EE                 MOVE.L    #000003EE,D2
       | 0408  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 040C  4EAE FFE2                      JSR       FFE2(A6)
       | 0410  2C5F                           MOVEA.L   (A7)+,A6
       | 0412  2740 0008                      MOVE.L    D0,0008(A3)
       | 0416  6732                           BEQ.B     044A
; 201:                                                                             {
; 202:                                                                                 inc_job->pj_Type = trans->trans_Command;
       | 0418  7000                           MOVEQ     #00,D0
       | 041A  102A 001C                      MOVE.B    001C(A2),D0
       | 041E  2740 000C                      MOVE.L    D0,000C(A3)
; 203:                                                                                 inc_job->pj_File = inc_job->pj_File;
; 204:                                                                                /* strcpy(inc_job->pj_FileName,trans->trans_RequestData); */
; 205:                                                                                 *(ULONG *)(trans->trans_ResponseData) = (ULONG) inc_job;
       | 0422  206A 0036                      MOVEA.L   0036(A2),A0
       | 0426  208B                           MOVE.L    A3,(A0)
; 206:                                                                                 trans->trans_RespDataActual = 4;
       | 0428  7004                           MOVEQ     #04,D0
       | 042A  2540 003E                      MOVE.L    D0,003E(A2)
; 207:                                                                                 trans->trans_Error = 0;
       | 042E  42AA 001E                      CLR.L     001E(A2)
; 208:                                                                                 AddTail((struct List *)&lb->LPD_Incoming,(struct Node *)inc_job);
       | 0432  206F 0048                      MOVEA.L   0048(A7),A0
       | 0436  D0FC 00A2                      ADDA.W    #00A2,A0
       | 043A  2F0E                           MOVE.L    A6,-(A7)
       | 043C  224B                           MOVEA.L   A3,A1
       | 043E  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0442  4EAE FF0A                      JSR       FF0A(A6)
       | 0446  2C5F                           MOVEA.L   (A7)+,A6
; 209:                                                                             }
; 210:                                                                             else
       | 0448  6028                           BRA.B     0472
; 211:                                                                             {
; 212:                                                                                 trans->trans_Error = 2;
       | 044A  7002                           MOVEQ     #02,D0
       | 044C  2540 001E                      MOVE.L    D0,001E(A2)
; 213:                                                                                 trans->trans_RespDataActual = 0;
       | 0450  42AA 003E                      CLR.L     003E(A2)
; 214:                                                                                 FreeMem(inc_job,sizeof(struct PrinterJob));
       | 0454  2F0E                           MOVE.L    A6,-(A7)
       | 0456  224B                           MOVEA.L   A3,A1
       | 0458  704F                           MOVEQ     #4F,D0
       | 045A  E588                           LSL.L     #2,D0
       | 045C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0460  4EAE FF2E                      JSR       FF2E(A6)
       | 0464  2C5F                           MOVEA.L   (A7)+,A6
; 215:                                                                             }
; 216:                                                                         }
; 217:                                                                         else
       | 0466  600A                           BRA.B     0472
; 218:                                                                         {
; 219:                                                                             trans->trans_Error = 1;
       | 0468  7001                           MOVEQ     #01,D0
       | 046A  2540 001E                      MOVE.L    D0,001E(A2)
; 220:                                                                             trans->trans_RespDataActual = 0;
       | 046E  42AA 003E                      CLR.L     003E(A2)
; 221:                                                                         }
; 222:                                                                         ReplyTransaction(trans);
       | 0472  2F0E                           MOVE.L    A6,-(A7)
       | 0474  224A                           MOVEA.L   A2,A1
       | 0476  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 047A  4EAE FF58                      JSR       FF58(A6)
       | 047E  2C5F                           MOVEA.L   (A7)+,A6
; 223:                                                                         break;
       | 0480  6000 008E                      BRA.W     0510
; 224: 
; 225:                                                 case LPCMD_DATA:        inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
       | 0484  206A 002A                      MOVEA.L   002A(A2),A0
       | 0488  2010                           MOVE.L    (A0),D0
; 226:                                                                         FWrite(inc_job->pj_File,(UBYTE *)((UBYTE *)trans->trans_RequestData + 4),1,(trans->trans_ReqDataActual - 4));
       | 048A  43E8 0004                      LEA       0004(A0),A1
       | 048E  222A 0032                      MOVE.L    0032(A2),D1
       | 0492  5981                           SUBQ.L    #4,D1
       | 0494  48E7 0802                      MOVEM.L   D4/A6,-(A7)
       | 0498  2409                           MOVE.L    A1,D2
       | 049A  2801                           MOVE.L    D1,D4
       | 049C  2040                           MOVEA.L   D0,A0
       | 049E  2228 0008                      MOVE.L    0008(A0),D1
       | 04A2  7601                           MOVEQ     #01,D3
       | 04A4  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 04A8  4EAE FEB6                      JSR       FEB6(A6)
       | 04AC  4CDF 4010                      MOVEM.L   (A7)+,D4/A6
; 227:                                                                         ReplyTransaction(trans);
       | 04B0  2F0E                           MOVE.L    A6,-(A7)
       | 04B2  224A                           MOVEA.L   A2,A1
       | 04B4  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 04B8  4EAE FF58                      JSR       FF58(A6)
       | 04BC  2C5F                           MOVEA.L   (A7)+,A6
; 228:                                                                         break;
       | 04BE  6050                           BRA.B     0510
; 229: 
; 230:                                                 case LPCMD_END:         inc_job = (struct PrinterJob *) *(ULONG *)(trans->trans_RequestData);
       | 04C0  206A 002A                      MOVEA.L   002A(A2),A0
       | 04C4  2250                           MOVEA.L   (A0),A1
; 231:                                                                         Close(inc_job->pj_File);
       | 04C6  2F49 0024                      MOVE.L    A1,0024(A7)
       | 04CA  2F0E                           MOVE.L    A6,-(A7)
       | 04CC  2229 0008                      MOVE.L    0008(A1),D1
       | 04D0  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 04D4  4EAE FFDC                      JSR       FFDC(A6)
       | 04D8  2C5F                           MOVEA.L   (A7)+,A6
; 232:                                                                         ReplyTransaction(trans);
       | 04DA  2F0E                           MOVE.L    A6,-(A7)
       | 04DC  224A                           MOVEA.L   A2,A1
       | 04DE  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 04E2  4EAE FF58                      JSR       FF58(A6)
       | 04E6  2C5F                           MOVEA.L   (A7)+,A6
; 233:                                                                         Remove((struct Node *)inc_job);
       | 04E8  2F0E                           MOVE.L    A6,-(A7)
       | 04EA  226F 0028                      MOVEA.L   0028(A7),A1
       | 04EE  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 04F2  4EAE FF04                      JSR       FF04(A6)
       | 04F6  2C5F                           MOVEA.L   (A7)+,A6
; 234:                                                                         AddTail((struct List *)&lb->LPD_Spool,(struct Node *)inc_job);
       | 04F8  206F 0048                      MOVEA.L   0048(A7),A0
       | 04FC  D0FC 0068                      ADDA.W    #0068,A0
       | 0500  2F0E                           MOVE.L    A6,-(A7)
       | 0502  226F 0028                      MOVEA.L   0028(A7),A1
       | 0506  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 050A  4EAE FF0A                      JSR       FF0A(A6)
       | 050E  2C5F                           MOVEA.L   (A7)+,A6
; 235:                                                                         break;
; 236:                                             }
; 237:                                         }
; 238:                                         if(GetMsg(prt_port))
       | 0510  2F0E                           MOVE.L    A6,-(A7)
       | 0512  206F 003C                      MOVEA.L   003C(A7),A0
       | 0516  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 051A  4EAE FE8C                      JSR       FE8C(A6)
       | 051E  2C5F                           MOVEA.L   (A7)+,A6
       | 0520  246F 002C                      MOVEA.L   002C(A7),A2
       | 0524  4A80                           TST.L     D0
       | 0526  6700 0084                      BEQ.W     05AC
; 239:                                         {
; 240:                                             loop = TRUE;
       | 052A  7C01                           MOVEQ     #01,D6
; 241:                                             if(prt_io->io_Length = FRead(prt_job->pj_File,prt_buffer,1,1024))
       | 052C  266D FFDC                      MOVEA.L   FFDC(A5),A3
       | 0530  48E7 0802                      MOVEM.L   D4/A6,-(A7)
       | 0534  222B 0008                      MOVE.L    0008(A3),D1
       | 0538  242F 0044                      MOVE.L    0044(A7),D2
       | 053C  7601                           MOVEQ     #01,D3
       | 053E  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 0542  7840                           MOVEQ     #40,D4
       | 0544  E98C                           LSL.L     #4,D4
       | 0546  4EAE FEBC                      JSR       FEBC(A6)
       | 054A  4CDF 4010                      MOVEM.L   (A7)+,D4/A6
       | 054E  2540 0024                      MOVE.L    D0,0024(A2)
       | 0552  6710                           BEQ.B     0564
; 242:                                             {
; 243:                                                 SendIO((struct IORequest *)prt_io);
       | 0554  2F0E                           MOVE.L    A6,-(A7)
       | 0556  224A                           MOVEA.L   A2,A1
       | 0558  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 055C  4EAE FE32                      JSR       FE32(A6)
       | 0560  2C5F                           MOVEA.L   (A7)+,A6
; 244:                                             }
; 245:                                             else
       | 0562  6048                           BRA.B     05AC
; 246:                                             {
; 247:                                                 Close(prt_job->pj_File);
       | 0564  2F0E                           MOVE.L    A6,-(A7)
       | 0566  222B 0008                      MOVE.L    0008(A3),D1
       | 056A  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 056E  4EAE FFDC                      JSR       FFDC(A6)
       | 0572  2C5F                           MOVEA.L   (A7)+,A6
; 248:                                                 DeleteFile((STRPTR)prt_job->pj_JobName);
       | 0574  41EB 001C                      LEA       001C(A3),A0
       | 0578  2F0E                           MOVE.L    A6,-(A7)
       | 057A  2208                           MOVE.L    A0,D1
       | 057C  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 0580  4EAE FFB8                      JSR       FFB8(A6)
       | 0584  2C5F                           MOVEA.L   (A7)+,A6
; 249:                                                 FreeMem(prt_job,sizeof(struct PrinterJob));
       | 0586  2F0E                           MOVE.L    A6,-(A7)
       | 0588  224B                           MOVEA.L   A3,A1
       | 058A  704F                           MOVEQ     #4F,D0
       | 058C  E588                           LSL.L     #2,D0
       | 058E  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0592  4EAE FF2E                      JSR       FF2E(A6)
       | 0596  2C5F                           MOVEA.L   (A7)+,A6
; 250:                                                 CloseDevice((struct IORequest *)prt_io);
       | 0598  2F0E                           MOVE.L    A6,-(A7)
       | 059A  224A                           MOVEA.L   A2,A1
       | 059C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 05A0  4EAE FE3E                      JSR       FE3E(A6)
       | 05A4  2C5F                           MOVEA.L   (A7)+,A6
; 251:                                                 status = LP_IDLE;
       | 05A6  2E03                           MOVE.L    D3,D7
; 252:                                                 prt_job = NULL;
       | 05A8  42AD FFDC                      CLR.L     FFDC(A5)
; 253:                                             }
; 254:                                         }
; 255:                                         if(GetMsg(timer_port))
       | 05AC  2F0E                           MOVE.L    A6,-(A7)
       | 05AE  206F 0038                      MOVEA.L   0038(A7),A0
       | 05B2  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 05B6  4EAE FE8C                      JSR       FE8C(A6)
       | 05BA  2C5F                           MOVEA.L   (A7)+,A6
       | 05BC  4A80                           TST.L     D0
       | 05BE  671A                           BEQ.B     05DA
; 256:                                         {
; 257:                                             CloseDevice((struct IORequest *)timer_io);
       | 05C0  2F0E                           MOVE.L    A6,-(A7)
       | 05C2  226F 0034                      MOVEA.L   0034(A7),A1
       | 05C6  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 05CA  4EAE FE3E                      JSR       FE3E(A6)
       | 05CE  2C5F                           MOVEA.L   (A7)+,A6
; 258:                                             timer_io->tr_node.io_Device = NULL;
       | 05D0  206F 0030                      MOVEA.L   0030(A7),A0
       | 05D4  42A8 0014                      CLR.L     0014(A0)
; 259:                                             status = LP_IDLE;
       | 05D8  7E01                           MOVEQ     #01,D7
; 260:                                         }
; 261:                                         if(status == LP_IDLE)
       | 05DA  7001                           MOVEQ     #01,D0
       | 05DC  BE80                           CMP.L     D0,D7
       | 05DE  6600 0172                      BNE.W     0752
; 262:                                         {
; 263:                                             if(prt_job = (struct PrinterJob *) RemHead((struct List *)&lb->LPD_Spool))
       | 05E2  206F 0048                      MOVEA.L   0048(A7),A0
       | 05E6  D0FC 0068                      ADDA.W    #0068,A0
       | 05EA  2F48 0024                      MOVE.L    A0,0024(A7)
       | 05EE  2F0E                           MOVE.L    A6,-(A7)
       | 05F0  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 05F4  4EAE FEFE                      JSR       FEFE(A6)
       | 05F8  2C5F                           MOVEA.L   (A7)+,A6
       | 05FA  2640                           MOVEA.L   D0,A3
       | 05FC  2B4B FFDC                      MOVE.L    A3,FFDC(A5)
       | 0600  200B                           MOVE.L    A3,D0
       | 0602  6700 014E                      BEQ.W     0752
; 264:                                             {
; 265:                                                 loop = TRUE;
       | 0606  7C01                           MOVEQ     #01,D6
; 266:                                                 status = LP_PRINTING;
       | 0608  7E02                           MOVEQ     #02,D7
; 267:                                                 if(prt_job->pj_Type == LPCMD_START_PRT)
       | 060A  7001                           MOVEQ     #01,D0
       | 060C  B0AB 000C                      CMP.L     000C(A3),D0
       | 0610  6626                           BNE.B     0638
; 268:                                                 {
; 269:                                                     if(!OpenDevice("printer.device",0L,(struct IORequest *)prt_io,0))
       | 0612  2F0E                           MOVE.L    A6,-(A7)
       | 0614  41FA FB84                      LEA       FB84(PC),A0
       | 0618  7000                           MOVEQ     #00,D0
       | 061A  224A                           MOVEA.L   A2,A1
       | 061C  2200                           MOVE.L    D0,D1
       | 061E  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0622  4EAE FE44                      JSR       FE44(A6)
       | 0626  2C5F                           MOVEA.L   (A7)+,A6
       | 0628  4A00                           TST.B     D0
       | 062A  6608                           BNE.B     0634
; 270:                                                     {
; 271:                                                         prt_io->io_Command = CMD_WRITE;
       | 062C  357C 0003 001C                 MOVE.W    #0003,001C(A2)
; 272:                                                     }
; 273:                                                     else
       | 0632  6066                           BRA.B     069A
; 274:                                                     {
; 275:                                                         status = LP_SLEEPING;
       | 0634  7E04                           MOVEQ     #04,D7
; 276:                                                     }
; 277:                                                 }
; 278:                                                 else if(prt_job->pj_Type == LPCMD_START_SER)
       | 0636  6062                           BRA.B     069A
       | 0638  7003                           MOVEQ     #03,D0
       | 063A  B0AB 000C                      CMP.L     000C(A3),D0
       | 063E  662E                           BNE.B     066E
; 279:                                                 {
; 280:                                                     status = LP_IDLE;
       | 0640  7E01                           MOVEQ     #01,D7
; 281:                                                     prt_job = NULL;
       | 0642  97CB                           SUBA.L    A3,A3
; 282:                                                     DeleteFile((STRPTR)prt_job->pj_JobName);
       | 0644  2B4B FFDC                      MOVE.L    A3,FFDC(A5)
       | 0648  41EB 001C                      LEA       001C(A3),A0
       | 064C  2F0E                           MOVE.L    A6,-(A7)
       | 064E  2208                           MOVE.L    A0,D1
       | 0650  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 0654  4EAE FFB8                      JSR       FFB8(A6)
       | 0658  2C5F                           MOVEA.L   (A7)+,A6
; 283:                                                     FreeMem(prt_job,sizeof(struct PrinterJob));
       | 065A  2F0E                           MOVE.L    A6,-(A7)
       | 065C  93C9                           SUBA.L    A1,A1
       | 065E  704F                           MOVEQ     #4F,D0
       | 0660  E588                           LSL.L     #2,D0
       | 0662  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0666  4EAE FF2E                      JSR       FF2E(A6)
       | 066A  2C5F                           MOVEA.L   (A7)+,A6
; 284:                                                 }
; 285:                                                 else if(prt_job->pj_Type == LPCMD_START_PAR)
       | 066C  602C                           BRA.B     069A
       | 066E  7002                           MOVEQ     #02,D0
       | 0670  B0AB 000C                      CMP.L     000C(A3),D0
       | 0674  6624                           BNE.B     069A
; 286:                                                 {
; 287:                                                     if(!OpenDevice("parallel.device",0L,(struct IORequest *)prt_io,0))
       | 0676  2F0E                           MOVE.L    A6,-(A7)
       | 0678  41FA FB30                      LEA       FB30(PC),A0
       | 067C  7000                           MOVEQ     #00,D0
       | 067E  224A                           MOVEA.L   A2,A1
       | 0680  2200                           MOVE.L    D0,D1
       | 0682  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0686  4EAE FE44                      JSR       FE44(A6)
       | 068A  2C5F                           MOVEA.L   (A7)+,A6
       | 068C  4A00                           TST.B     D0
       | 068E  6608                           BNE.B     0698
; 288:                                                     {
; 289:                                                         prt_io->io_Command = CMD_WRITE;
       | 0690  357C 0003 001C                 MOVE.W    #0003,001C(A2)
; 290:                                                     }
; 291:                                                     else
       | 0696  6002                           BRA.B     069A
; 292:                                                     {
; 293:                                                         status = LP_SLEEPING;
       | 0698  7E04                           MOVEQ     #04,D7
; 294:                                                     }
; 295:                                                 }
; 296:                                                 if (status == LP_PRINTING)
       | 069A  7002                           MOVEQ     #02,D0
       | 069C  BE80                           CMP.L     D0,D7
       | 069E  665E                           BNE.B     06FE
; 297:                                                 {
; 298:                                                     prt_io->io_Data = prt_buffer;
       | 06A0  256F 003C 0028                 MOVE.L    003C(A7),0028(A2)
; 299: 
; 300:                                                     if(prt_job->pj_File = Open(prt_job->pj_JobName,MODE_OLDFILE))
       | 06A6  41EB 001C                      LEA       001C(A3),A0
       | 06AA  2F0E                           MOVE.L    A6,-(A7)
       | 06AC  2208                           MOVE.L    A0,D1
       | 06AE  243C 0000 03ED                 MOVE.L    #000003ED,D2
       | 06B4  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 06B8  4EAE FFE2                      JSR       FFE2(A6)
       | 06BC  2C5F                           MOVEA.L   (A7)+,A6
       | 06BE  2740 0008                      MOVE.L    D0,0008(A3)
       | 06C2  6700 008E                      BEQ.W     0752
; 301:                                                     {
; 302:                                                         if(prt_io->io_Length = FRead(prt_job->pj_File,prt_buffer,1,1024))
       | 06C6  48E7 0802                      MOVEM.L   D4/A6,-(A7)
       | 06CA  2200                           MOVE.L    D0,D1
       | 06CC  242F 0044                      MOVE.L    0044(A7),D2
       | 06D0  7601                           MOVEQ     #01,D3
       | 06D2  2C6E 0022                      MOVEA.L   0022(A6),A6
       | 06D6  7840                           MOVEQ     #40,D4
       | 06D8  E98C                           LSL.L     #4,D4
       | 06DA  4EAE FEBC                      JSR       FEBC(A6)
       | 06DE  4CDF 4010                      MOVEM.L   (A7)+,D4/A6
       | 06E2  2540 0024                      MOVE.L    D0,0024(A2)
       | 06E6  6710                           BEQ.B     06F8
; 303:                                                         {
; 304:                                                             SendIO((struct IORequest *)prt_io);
       | 06E8  2F0E                           MOVE.L    A6,-(A7)
       | 06EA  224A                           MOVEA.L   A2,A1
       | 06EC  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 06F0  4EAE FE32                      JSR       FE32(A6)
       | 06F4  2C5F                           MOVEA.L   (A7)+,A6
; 305:                                                         }
; 306:                                                         else
       | 06F6  605A                           BRA.B     0752
; 307:                                                             status = LP_ERROR;
       | 06F8  7E00                           MOVEQ     #00,D7
       | 06FA  4607                           NOT.B     D7
; 308:                                                     }
; 309:                                                 }
; 310:                                                 else if(status == LP_SLEEPING)
       | 06FC  6054                           BRA.B     0752
       | 06FE  7004                           MOVEQ     #04,D0
       | 0700  BE80                           CMP.L     D0,D7
       | 0702  664E                           BNE.B     0752
; 311:                                                 {
; 312:                                                     AddHead((struct List *)&lb->LPD_Spool,(struct Node *)prt_job);
       | 0704  2F0E                           MOVE.L    A6,-(A7)
       | 0706  206F 0028                      MOVEA.L   0028(A7),A0
       | 070A  224B                           MOVEA.L   A3,A1
       | 070C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0710  4EAE FF10                      JSR       FF10(A6)
       | 0714  2C5F                           MOVEA.L   (A7)+,A6
; 313:                                                     status = LP_SLEEPING;
       | 0716  7E04                           MOVEQ     #04,D7
; 314:                                                     OpenDevice("timer.device",UNIT_VBLANK,(struct IORequest *)timer_io,0);
       | 0718  2F0E                           MOVE.L    A6,-(A7)
       | 071A  41FA FA9E                      LEA       FA9E(PC),A0
       | 071E  7001                           MOVEQ     #01,D0
       | 0720  226F 0034                      MOVEA.L   0034(A7),A1
       | 0724  7200                           MOVEQ     #00,D1
       | 0726  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 072A  4EAE FE44                      JSR       FE44(A6)
       | 072E  2C5F                           MOVEA.L   (A7)+,A6
; 315:                                                     timer_io->tr_node.io_Command = TR_ADDREQUEST;
       | 0730  206F 0030                      MOVEA.L   0030(A7),A0
       | 0734  317C 0009 001C                 MOVE.W    #0009,001C(A0)
; 316:                                                     timer_io->tr_time.tv_secs = 30;
       | 073A  701E                           MOVEQ     #1E,D0
       | 073C  2140 0020                      MOVE.L    D0,0020(A0)
; 317:                                                     timer_io->tr_time.tv_micro = 0;
       | 0740  42A8 0024                      CLR.L     0024(A0)
; 318:                                                     SendIO((struct IORequest *)timer_io);
       | 0744  2F0E                           MOVE.L    A6,-(A7)
       | 0746  2248                           MOVEA.L   A0,A1
       | 0748  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 074C  4EAE FE32                      JSR       FE32(A6)
       | 0750  2C5F                           MOVEA.L   (A7)+,A6
; 319:                                                 }
; 320:                                             }
; 321:                                         }
; 322:                                         if((status == LP_IDLE) && IsListEmpty((struct List *)&lb->LPD_Incoming))
       | 0752  7001                           MOVEQ     #01,D0
       | 0754  BE80                           CMP.L     D0,D7
       | 0756  6618                           BNE.B     0770
       | 0758  206F 0048                      MOVEA.L   0048(A7),A0
       | 075C  D0FC 00A2                      ADDA.W    #00A2,A0
       | 0760  2268 0008                      MOVEA.L   0008(A0),A1
       | 0764  B3C8                           CMPA.L    A0,A1
       | 0766  6608                           BNE.B     0770
; 323:                                         {
; 324:                                             LPDBase->LPD_Entity = NULL;
       | 0768  42AE 0032                      CLR.L     0032(A6)
; 325:                                             dead = TRUE;
       | 076C  2800                           MOVE.L    D0,D4
; 326:                                             break;
       | 076E  6006                           BRA.B     0776
; 327:                                         }
; 328:                                     }
       | 0770  4A86                           TST.L     D6
       | 0772  6600 FC0E                      BNE.W     0382
; 329:                                     if(dead)
       | 0776  4A84                           TST.L     D4
       | 0778  6700 FBF2                      BEQ.W     036C
; 330:                                         break;
; 331:                                 }
; 332:                                 DeleteIORequest(prt_io);
       | 077C  2F0E                           MOVE.L    A6,-(A7)
       | 077E  206F 0030                      MOVEA.L   0030(A7),A0
       | 0782  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0786  4EAE FD6C                      JSR       FD6C(A6)
       | 078A  2C5F                           MOVEA.L   (A7)+,A6
; 333:                             }
; 334:                             DeleteIORequest(timer_io);
       | 078C  266F 0038                      MOVEA.L   0038(A7),A3
       | 0790  246F 0034                      MOVEA.L   0034(A7),A2
       | 0794  2F0E                           MOVE.L    A6,-(A7)
       | 0796  206F 0034                      MOVEA.L   0034(A7),A0
       | 079A  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 079E  4EAE FD6C                      JSR       FD6C(A6)
       | 07A2  2C5F                           MOVEA.L   (A7)+,A6
; 335:                         }
; 336:                         DeleteMsgPort(timer_port);
       | 07A4  2F0E                           MOVE.L    A6,-(A7)
       | 07A6  204A                           MOVEA.L   A2,A0
       | 07A8  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 07AC  4EAE FD60                      JSR       FD60(A6)
       | 07B0  2C5F                           MOVEA.L   (A7)+,A6
; 337:                     }
; 338:                     DeleteMsgPort(prt_port);
       | 07B2  2F0E                           MOVE.L    A6,-(A7)
       | 07B4  204B                           MOVEA.L   A3,A0
       | 07B6  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 07BA  4EAE FD60                      JSR       FD60(A6)
       | 07BE  2C5F                           MOVEA.L   (A7)+,A6
; 339:                 }
; 340:                 FreeMem(prt_buffer,1024);
       | 07C0  2F0E                           MOVE.L    A6,-(A7)
       | 07C2  226F 0040                      MOVEA.L   0040(A7),A1
       | 07C6  7040                           MOVEQ     #40,D0
       | 07C8  E988                           LSL.L     #4,D0
       | 07CA  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 07CE  4EAE FF2E                      JSR       FF2E(A6)
       | 07D2  2C5F                           MOVEA.L   (A7)+,A6
; 341:             }
; 342:             DeleteEntity(entity);
       | 07D4  2F0E                           MOVE.L    A6,-(A7)
       | 07D6  206F 0044                      MOVEA.L   0044(A7),A0
       | 07DA  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 07DE  4EAE FF7C                      JSR       FF7C(A6)
       | 07E2  2C5F                           MOVEA.L   (A7)+,A6
; 343:         }
; 344:         if(StartupError)                       /* If we failed for some reason, we still have to let the Services Manager
       | 07E4  2E2D FFE4                      MOVE.L    FFE4(A5),D7
       | 07E8  4A87                           TST.L     D7
       | 07EA  671E                           BEQ.B     080A
; 345:                                                know about it. */
; 346:         {
; 347:             LPDError = StartupError;
       | 07EC  23C7  0000 0000-XX             MOVE.L    D7,_LPDError
; 348:             Signal(LPDSMProc, LPDSignalMask);
       | 07F2  2F0E                           MOVE.L    A6,-(A7)
       | 07F4  2279  0000 0000-XX             MOVEA.L   _LPDSMProc,A1
       | 07FA  2039  0000 0000-XX             MOVE.L    _LPDSignalMask,D0
       | 0800  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0804  4EAE FEBC                      JSR       FEBC(A6)
       | 0808  2C5F                           MOVEA.L   (A7)+,A6
; 349:         }
; 350:         Forbid();                               /* Make sure we exit before someone could call expunge! */
       | 080A  2F0E                           MOVE.L    A6,-(A7)
       | 080C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0810  4EAE FF7C                      JSR       FF7C(A6)
       | 0814  2C5F                           MOVEA.L   (A7)+,A6
; 351:         CloseLibrary(SvcBase);
       | 0816  2F0E                           MOVE.L    A6,-(A7)
       | 0818  226F 0048                      MOVEA.L   0048(A7),A1
       | 081C  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0820  4EAE FE62                      JSR       FE62(A6)
       | 0824  2C5F                           MOVEA.L   (A7)+,A6
; 352:     }
; 353: }
       | 0826  4CDF 4CFC                      MOVEM.L   (A7)+,D2-D7/A2-A3/A6
       | 082A  4E5D                           UNLK      A5
       | 082C  4E75                           RTS

SECTION 01 " " 00000020 BYTES
0000 80 0B 10 01 00 00 00 00 80 0B 10 02 00 00 00 01 ................
0010 80 0B 10 04 00 00 00 00 00 00 00 00 00 00 00 00 ................
