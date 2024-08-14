SAS AMIGA 680x0OBJ Module Disassembler V6.03
Copyright © 1993 SAS Institute, Inc.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

@InitEdData 0000-00    @InitPrefs 00BC-00    @CleanUpEdData 00F4-00    
@FreePrefs 0136-00    @FreeList 0190-00    @InsertListSorted 01C4-00    
@ReadPrefs 0228-00    @OpenPrefs 0444-00    @WritePrefs 04A4-00    
@SavePrefs 0676-00    @CreateDisplay 0698-00    @DisposeDisplay 0812-00    
@CenterLine 084A-00    _DrawBB 08EE-00    @ClearGadPtrs 0956-00    
@RenderGadgets 0976-00    @ProcessSpecialCommand 17F4-00    @StrtoIP 2140-00
@MakeNewDisplay 21AC-00    @IPToStr 2286-00    @nibvert 22FA-00    
@GetSpecialCmdState 232A-00    @CopyPrefs 2346-00    @DeviceRequest 2584-00    
_RequestDevice 2690-00    @AddDeviceReq 26B4-00    _StringHook 2882-00    
@IsIPDigit 28D2-00    @DummyBuffFunc 28F2-00    _BMTags 0030-01    _EM 004E-01
_EG 00DE-01

SECTION 00 "pe_custom.c" 00002904 BYTES
;   1: 
;   2: /* includes */
;   3: #include <exec/types.h>
;   4: #include <exec/libraries.h>
;   5: #include <exec/ports.h>
;   6: #include <exec/memory.h>
;   7: #include <exec/execbase.h>
;   8: #include <devices/keymap.h>
;   9: #include <intuition/intuition.h>
;  10: #include <intuition/screens.h>
;  11: #include <intuition/gadgetclass.h>
;  12: #include <graphics/text.h>
;  13: #include <libraries/asl.h>
;  14: #include <libraries/gadtools.h>
;  15: #include <libraries/locale.h>
;  16: #include <dos/dos.h>
;  17: #include <dos/exall.h>
;  18: #include <string.h>
;  19: #include <stdio.h>
;  20: 
;  21: /* prototypes */
;  22: #include <clib/exec_protos.h>
;  23: #include <clib/dos_protos.h>
;  24: #include <clib/iffparse_protos.h>
;  25: #include <clib/gadtools_protos.h>
;  26: #include <clib/intuition_protos.h>
;  27: #include <clib/graphics_protos.h>
;  28: #include <clib/alib_protos.h>
;  29: #include <clib/utility_protos.h>
;  30: #include <clib/asl_protos.h>
;  31: #include <clib/icon_protos.h>
;  32: 
;  33: /* direct ROM interface */
;  34: #include <pragmas/exec_pragmas.h>
;  35: #include <pragmas/dos_pragmas.h>
;  36: #include <pragmas/iffparse_pragmas.h>
;  37: #include <pragmas/gadtools_pragmas.h>
;  38: #include <pragmas/intuition_pragmas.h>
;  39: #include <pragmas/graphics_pragmas.h>
;  40: #include <pragmas/utility_pragmas.h>
;  41: #include <pragmas/asl_pragmas.h>
;  42: #include <pragmas/icon_pragmas.h>
;  43: 
;  44: /* application includes */
;  45: #include "pe_custom.h"
;  46: #include "pe_strings.h"
;  47: #include "pe_utils.h"
;  48: #include "pe_iff.h"
;  49: #include <devices/sana2.h>
;  50: 
;  51: #define SysBase ed->ed_SysBase
;  52: 
;  53: 
;  54: /*****************************************************************************/
;  55: 
;  56: 
;  57: /* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
;  58:  * chunks there are
;  59:  */
;  60: #define IFFPrefChunkCnt 6
;  61: static LONG far IFFPrefChunks[] =
;  62: {
;  63:     ID_PREF, ID_PRHD,
;  64:     ID_PREF, ID_NDEV,
;  65:     ID_PREF, ID_NRRM,
;  66:     ID_PREF, ID_NLRM,
;  67:     ID_PREF, ID_NIRT,
;  68:     ID_PREF, ID_HOST
;  69: };
;  70: 
;  71: BOOL DummyBuffFunc(APTR to, APTR from, LONG length);
;  72: 
;  73: struct TagItem BMTags[] =
;  74: {
;  75:     S2_CopyToBuff,	DummyBuffFunc,
;  76:     S2_CopyFromBuff,	DummyBuffFunc,
;  77:     TAG_DONE,		0
;  78: };
;  79: 
;  80: /*****************************************************************************/
;  81: 
;  82: 
;  83: /* The PrefHeader structure this editor outputs */
;  84: static struct PrefHeader far IFFPrefHeader =
;  85: {
;  86:     0,   /* version */
;  87:     0,   /* type    */
;  88:     0    /* flags   */
;  89: };
;  90: 
;  91: 
;  92: /*****************************************************************************/
;  93: 
;  94: 
;  95: #define WRITE_ALL    0
;  96: #define WRITE_WB     1
;  97: #define WRITE_SYS    2
;  98: #define WRITE_SCREEN 3
;  99: 
; 100: 
; 101: /*****************************************************************************/
; 102: 
; 103: 
; 104: EdStatus InitEdData(EdDataPtr ed)
; 105: {
       | 0000  2F0B                           MOVE.L      A3,-(A7)
       | 0002  2648                           MOVEA.L     A0,A3
; 106:     InitPrefs(&ed->ed_PrefsDefaults);
       | 0004  41EB 022A                      LEA         022A(A3),A0
       | 0008  6100 00B2                      BSR.W       00BC
; 107:     InitPrefs(&ed->ed_PrefsWork);
       | 000C  41EB 030A                      LEA         030A(A3),A0
       | 0010  6100 00AA                      BSR.W       00BC
; 108:     InitPrefs(&ed->ed_PrefsInitial);
       | 0014  41EB 03EA                      LEA         03EA(A3),A0
       | 0018  6100 00A2                      BSR.W       00BC
; 109:     ed->ed_CurrentDevice = NULL;
       | 001C  91C8                           SUBA.L      A0,A0
       | 001E  2748 0736                      MOVE.L      A0,0736(A3)
; 110:     ed->ed_CurrentRoute = NULL;
       | 0022  2748 0742                      MOVE.L      A0,0742(A3)
; 111:     ed->ed_CurrentLocRealm = NULL;
       | 0026  2748 073A                      MOVE.L      A0,073A(A3)
; 112:     ed->ed_CurrentRemRealm = NULL;
       | 002A  2748 073E                      MOVE.L      A0,073E(A3)
; 113:     ed->ed_IPGadHook.h_Entry = StringHook;
       | 002E  43FA 2852                      LEA         2852(PC),A1
       | 0032  2749 0752                      MOVE.L      A1,0752(A3)
; 114:     ed->ed_V39 = (ed->ed_IntuitionBase->lib_Version >= 39) ? TRUE : FALSE;
       | 0036  226B 0004                      MOVEA.L     0004(A3),A1
       | 003A  0C69 0027 0014                 CMPI.W      #0027,0014(A1)
       | 0040  6504                           BCS.B       0046
       | 0042  7001                           MOVEQ       #01,D0
       | 0044  6002                           BRA.B       0048
       | 0046  2008                           MOVE.L      A0,D0
       | 0048  2740 077E                      MOVE.L      D0,077E(A3)
; 115:     return(ES_NORMAL);
       | 004C  2008                           MOVE.L      A0,D0
; 116: }
       | 004E  265F                           MOVEA.L     (A7)+,A3
       | 0050  4E75                           RTS
       | 0052  286E 6577                      MOVEA.L     6577(A6),A4
       | 0056  2900                           MOVE.L      D0,-(A4)
       | 0058  252D 3136                      MOVE.L      3136(A5),-(A2)
       | 005C  2E31 3673                      MOVE.L      73(A1,D3.W*8),D7
       | 0060  2025                           MOVE.L      -(A5),D0
       | 0062  2D31 362E                      MOVE.L      2E(A1,D3.W*8),-(A6)
       | 0066  3136 7300                      MOVE.W      (A6,D7.W*2),-(A0)
       | 006A  6465                           BCC.B       00D1
       | 006C  6661                           BNE.B       00CF
       | 006E  756C                           
       | 0070  7400                           MOVEQ       #00,D2
       | 0072  252D 3136                      MOVE.L      3136(A5),-(A2)
       | 0076  2E31 3673                      MOVE.L      73(A1,D3.W*8),D7
       | 007A  2020                           MOVE.L      -(A0),D0
       | 007C  2020                           MOVE.L      -(A0),D0
       | 007E  2025                           MOVE.L      -(A5),D0
       | 0080  2D31 362E                      MOVE.L      2E(A1,D3.W*8),-(A6)
       | 0084  3136 7300                      MOVE.W      (A6,D7.W*2),-(A0)
       | 0088  3078 0000                      MOVEA.W     0000,A0
       | 008C  252E 336C                      MOVE.L      336C(A6),-(A2)
       | 0090  642E                           BCC.B       00C0
       | 0092  252E 336C                      MOVE.L      336C(A6),-(A2)
       | 0096  642E                           BCC.B       00C6
       | 0098  252E 336C                      MOVE.L      336C(A6),-(A2)
       | 009C  642E                           BCC.B       00CC
       | 009E  252E 336C                      MOVE.L      336C(A6),-(A2)
       | 00A2  6400 233F                      BCC.W       23E3
       | 00A6  2E64                           MOVEA.L     -(A4),A7
       | 00A8  6576                           BCS.B       0120
       | 00AA  6963                           BVS.B       010F
       | 00AC  6500 4445                      BCS.W       44F3
       | 00B0  5653                           ADDQ.W      #3,(A3)
       | 00B2  3A4E                           MOVEA.W     A6,A5
       | 00B4  6574                           BCS.B       012A
       | 00B6  776F                           
       | 00B8  726B                           MOVEQ       #6B,D1
       | 00BA  7300                           
; 117: 
; 118: void InitPrefs(struct ExtPrefs *prefs)
; 119: {
       | 00BC  4E55 FFFC                      LINK        A5,#FFFC
       | 00C0  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
; 120:     NewList(&prefs->ep_NIPCDevices);
       | 00C4  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 00C8  4EBA  0000-XX.1                JSR         @NewList(PC)
; 121:     NewList(&prefs->ep_NIPCRoutes);
       | 00CC  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 00D0  D0FC 002A                      ADDA.W      #002A,A0
       | 00D4  4EBA  0000-XX.1                JSR         @NewList(PC)
; 122:     NewList(&prefs->ep_NIPCLocalRealms);
       | 00D8  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 00DC  D0FC 000E                      ADDA.W      #000E,A0
       | 00E0  4EBA  0000-XX.1                JSR         @NewList(PC)
; 123:     NewList(&prefs->ep_NIPCRemoteRealms);
       | 00E4  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 00E8  D0FC 001C                      ADDA.W      #001C,A0
       | 00EC  4EBA  0000-XX.1                JSR         @NewList(PC)
; 124: }
       | 00F0  4E5D                           UNLK        A5
       | 00F2  4E75                           RTS
; 125: 
; 126: /*****************************************************************************/
; 127: 
; 128: 
; 129: VOID CleanUpEdData(EdDataPtr ed)
; 130: {
       | 00F4  4E55 FFFC                      LINK        A5,#FFFC
       | 00F8  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
; 131:     FreePrefs(ed, &ed->ed_PrefsDefaults);
       | 00FC  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0100  D0FC 022A                      ADDA.W      #022A,A0
       | 0104  2248                           MOVEA.L     A0,A1
       | 0106  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 010A  6100 002A                      BSR.W       0136
; 132:     FreePrefs(ed, &ed->ed_PrefsWork);
       | 010E  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0112  D0FC 030A                      ADDA.W      #030A,A0
       | 0116  2248                           MOVEA.L     A0,A1
       | 0118  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 011C  6100 0018                      BSR.W       0136
; 133:     FreePrefs(ed, &ed->ed_PrefsInitial);
       | 0120  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0124  D0FC 03EA                      ADDA.W      #03EA,A0
       | 0128  2248                           MOVEA.L     A0,A1
       | 012A  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 012E  6100 0006                      BSR.W       0136
; 134: }
       | 0132  4E5D                           UNLK        A5
       | 0134  4E75                           RTS
; 135: 
; 136: VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs)
; 137: {
       | 0136  4E55 FFFC                      LINK        A5,#FFFC
       | 013A  2F0B                           MOVE.L      A3,-(A7)
       | 013C  2648                           MOVEA.L     A0,A3
       | 013E  2B49 FFFC                      MOVE.L      A1,FFFC(A5)
; 138:     FreeList(ed, &prefs->ep_NIPCDevices,sizeof(struct NIPCDevice));
       | 0142  7057                           MOVEQ       #57,D0
       | 0144  E588                           LSL.L       #2,D0
       | 0146  204B                           MOVEA.L     A3,A0
       | 0148  226D FFFC                      MOVEA.L     FFFC(A5),A1
       | 014C  6100 0042                      BSR.W       0190
; 139:     FreeList(ed, &prefs->ep_NIPCLocalRealms,sizeof(struct NIPCRealm));
       | 0150  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0154  D0FC 000E                      ADDA.W      #000E,A0
       | 0158  2248                           MOVEA.L     A0,A1
       | 015A  7069                           MOVEQ       #69,D0
       | 015C  D080                           ADD.L       D0,D0
       | 015E  204B                           MOVEA.L     A3,A0
       | 0160  6100 002E                      BSR.W       0190
; 140:     FreeList(ed, &prefs->ep_NIPCRemoteRealms,sizeof(struct NIPCRealm));
       | 0164  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0168  D0FC 001C                      ADDA.W      #001C,A0
       | 016C  2248                           MOVEA.L     A0,A1
       | 016E  7069                           MOVEQ       #69,D0
       | 0170  D080                           ADD.L       D0,D0
       | 0172  204B                           MOVEA.L     A3,A0
       | 0174  6100 001A                      BSR.W       0190
; 141:     FreeList(ed, &prefs->ep_NIPCRoutes,sizeof(struct NIPCRoute));
       | 0178  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 017C  D0FC 002A                      ADDA.W      #002A,A0
       | 0180  2248                           MOVEA.L     A0,A1
       | 0182  7058                           MOVEQ       #58,D0
       | 0184  204B                           MOVEA.L     A3,A0
       | 0186  6100 0008                      BSR.W       0190
; 142: }
       | 018A  265F                           MOVEA.L     (A7)+,A3
       | 018C  4E5D                           UNLK        A5
       | 018E  4E75                           RTS
; 143: 
; 144: VOID FreeList(EdDataPtr ed, struct List *list, ULONG nodeSize)
; 145: {
       | 0190  4E55 FFF8                      LINK        A5,#FFF8
       | 0194  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 0198  2648                           MOVEA.L     A0,A3
       | 019A  2E00                           MOVE.L      D0,D7
       | 019C  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
       | 01A0  600A                           BRA.B       01AC
; 146: struct Node *node;
; 147: 
; 148:     while(node = RemHead(list))
; 149:     {
; 150:         FreeMem(node, nodeSize);
       | 01A2  224A                           MOVEA.L     A2,A1
       | 01A4  2007                           MOVE.L      D7,D0
       | 01A6  2C53                           MOVEA.L     (A3),A6
       | 01A8  4EAE FF2E                      JSR         FF2E(A6)
; 151:     }
       | 01AC  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 01B0  2C53                           MOVEA.L     (A3),A6
       | 01B2  4EAE FEFE                      JSR         FEFE(A6)
       | 01B6  2440                           MOVEA.L     D0,A2
       | 01B8  200A                           MOVE.L      A2,D0
       | 01BA  66E6                           BNE.B       01A2
; 152: }
       | 01BC  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 01C0  4E5D                           UNLK        A5
       | 01C2  4E75                           RTS
; 153: 
; 154: VOID InsertListSorted (EdDataPtr ed, struct List *list, struct Node *node)
; 155: {
       | 01C4  4E55 FFF8                      LINK        A5,#FFF8
       | 01C8  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 01CC  2648                           MOVEA.L     A0,A3
       | 01CE  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
; 156: 	struct Node *n;
; 157: 
; 158: 	for (n = list->lh_Tail; n->ln_Pred; n = n->ln_Pred)
       | 01D2  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 01D6  2468 0004                      MOVEA.L     0004(A0),A2
       | 01DA  6030                           BRA.B       020C
; 159: 	{
; 160: 		if (Stricmp(node->ln_Name,n->ln_Name) > 0)
       | 01DC  206D 0008                      MOVEA.L     0008(A5),A0
       | 01E0  2068 000A                      MOVEA.L     000A(A0),A0
       | 01E4  226A 000A                      MOVEA.L     000A(A2),A1
       | 01E8  2C6B 0020                      MOVEA.L     0020(A3),A6
       | 01EC  4EAE FF5E                      JSR         FF5E(A6)
       | 01F0  4A80                           TST.L       D0
       | 01F2  6E14                           BGT.B       0208
; 161: 		{
; 162: 			Insert(list,node,n);	// we go backwards since Insert inserts after the node
       | 01F4  2F0A                           MOVE.L      A2,-(A7)
       | 01F6  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 01FA  226D 0008                      MOVEA.L     0008(A5),A1
       | 01FE  2C53                           MOVEA.L     (A3),A6
       | 0200  4EAE FF16                      JSR         FF16(A6)
       | 0204  245F                           MOVEA.L     (A7)+,A2
; 163: 			return;
       | 0206  6018                           BRA.B       0220
; 164: 		}
       | 0208  246A 0004                      MOVEA.L     0004(A2),A2
       | 020C  4AAA 0004                      TST.L       0004(A2)
       | 0210  66CA                           BNE.B       01DC
; 165: 	}
; 166: 	AddHead(list,node);
       | 0212  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 0216  226D 0008                      MOVEA.L     0008(A5),A1
       | 021A  2C53                           MOVEA.L     (A3),A6
       | 021C  4EAE FF10                      JSR         FF10(A6)
; 167: }
       | 0220  4CDF 4C00                      MOVEM.L     (A7)+,A2-A3/A6
       | 0224  4E5D                           UNLK        A5
       | 0226  4E75                           RTS
; 168: 
; 169: /*****************************************************************************/
; 170: 
; 171: 
; 172: EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
; 173: {
       | 0228  4E55 FDB0                      LINK        A5,#FDB0
       | 022C  48E7 0132                      MOVEM.L     D7/A2-A3/A6,-(A7)
       | 0230  2648                           MOVEA.L     A0,A3
       | 0232  2B49 FDB0                      MOVE.L      A1,FDB0(A5)
; 174: struct NIPCDevicePrefs tmp_dev;
; 175: struct NIPCDevice *nd;
; 176: struct NIPCRoutePrefs tmp_route;
; 177: struct NIPCRoute *nr;
; 178: struct NIPCRealmPrefs tmp_realm;
; 179: struct NIPCRealm *nz;
; 180: struct NIPCHostPrefs tmp_host;
; 181: ULONG hostchunksize;
; 182: ULONG checkflags;
; 183: 
; 184:     tmp_host.nhp_HostFlags = 0;
       | 0236  42AD FE58                      CLR.L       FE58(A5)
; 185:     tmp_host.nhp_OwnerName[0]='\0';
       | 023A  422D FE38                      CLR.B       FE38(A5)
; 186: 
; 187:     if (cn->cn_Type != ID_PREF)
       | 023E  246D 0008                      MOVEA.L     0008(A5),A2
       | 0242  0CAA 5052 4546 000C            CMPI.L      #50524546,000C(A2)
       | 024A  6706                           BEQ.B       0252
; 188:         return(ES_IFF_UNKNOWNCHUNK);
       | 024C  7002                           MOVEQ       #02,D0
       | 024E  6000 01EC                      BRA.W       043C
; 189: 
; 190:     switch(cn->cn_ID)
       | 0252  246D 0008                      MOVEA.L     0008(A5),A2
       | 0256  202A 0008                      MOVE.L      0008(A2),D0
       | 025A  0480 484F 5354                 SUBI.L      #484F5354,D0
       | 0260  6700 0182                      BEQ.W       03E4
       | 0264  0480 05F4 F202                 SUBI.L      #05F4F202,D0
       | 026A  6722                           BEQ.B       028E
       | 026C  0480 0005 0CFE                 SUBI.L      #00050CFE,D0
       | 0272  6700 0086                      BEQ.W       02FA
       | 0276  0480 0002 FFF9                 SUBI.L      #0002FFF9,D0
       | 027C  6700 00DC                      BEQ.W       035A
       | 0280  0480 0006 0000                 SUBI.L      #00060000,D0
       | 0286  6700 00D2                      BEQ.W       035A
       | 028A  6000 01AE                      BRA.W       043A
; 191:     {
; 192:         case ID_NDEV:
; 193:                         if(ReadChunkBytes(iff,&tmp_dev,sizeof(struct NIPCDevicePrefs)) == sizeof(struct NIPCDevicePrefs))
       | 028E  206D FDB0                      MOVEA.L     FDB0(A5),A0
       | 0292  43ED FEAA                      LEA         FEAA(A5),A1
       | 0296  203C 0000 014E                 MOVE.L      #0000014E,D0
       | 029C  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 02A0  4EAE FFC4                      JSR         FFC4(A6)
       | 02A4  0C80 0000 014E                 CMPI.L      #0000014E,D0
       | 02AA  6648                           BNE.B       02F4
; 194:                         {
; 195:                             if(nd = AllocMem(sizeof(struct NIPCDevice),MEMF_CLEAR|MEMF_PUBLIC))
       | 02AC  7057                           MOVEQ       #57,D0
       | 02AE  E588                           LSL.L       #2,D0
       | 02B0  223C 0001 0001                 MOVE.L      #00010001,D1
       | 02B6  2C53                           MOVEA.L     (A3),A6
       | 02B8  4EAE FF3A                      JSR         FF3A(A6)
       | 02BC  2440                           MOVEA.L     D0,A2
       | 02BE  200A                           MOVE.L      A2,D0
       | 02C0  672C                           BEQ.B       02EE
; 196:                             {
; 197:                                 CopyMem((APTR)&tmp_dev,(APTR)&nd->nd_Prefs,sizeof(struct NIPCDevicePrefs));
       | 02C2  41EA 000E                      LEA         000E(A2),A0
       | 02C6  2248                           MOVEA.L     A0,A1
       | 02C8  41ED FEAA                      LEA         FEAA(A5),A0
       | 02CC  203C 0000 014E                 MOVE.L      #0000014E,D0
       | 02D2  4EAE FD90                      JSR         FD90(A6)
; 198:                                 AddTail(&ed->ed_PrefsWork.ep_NIPCDevices,(struct Node *)nd);
       | 02D6  41EB 030A                      LEA         030A(A3),A0
       | 02DA  224A                           MOVEA.L     A2,A1
       | 02DC  4EAE FF0A                      JSR         FF0A(A6)
; 199:                                 nd->nd_Node.ln_Name = (STRPTR)&nd->nd_Prefs.ndp_DevPathName;
       | 02E0  41EA 005B                      LEA         005B(A2),A0
       | 02E4  2548 000A                      MOVE.L      A0,000A(A2)
; 200:                                 return(ES_NORMAL);
       | 02E8  7000                           MOVEQ       #00,D0
       | 02EA  6000 0150                      BRA.W       043C
; 201:                             }
; 202:                             return(ES_NO_MEMORY);
       | 02EE  7001                           MOVEQ       #01,D0
       | 02F0  6000 014A                      BRA.W       043C
; 203:                         }
; 204:                         else
; 205:                             return(ES_IFFERROR);
       | 02F4  7004                           MOVEQ       #04,D0
       | 02F6  6000 0144                      BRA.W       043C
; 206:                         break;
; 207:         case ID_NIRT:
; 208:                         if(ReadChunkBytes(iff,&tmp_route,sizeof(struct NIPCRoutePrefs)) == sizeof(struct NIPCRoutePrefs))
       | 02FA  206D FDB0                      MOVEA.L     FDB0(A5),A0
       | 02FE  43ED FEA0                      LEA         FEA0(A5),A1
       | 0302  700A                           MOVEQ       #0A,D0
       | 0304  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 0308  4EAE FFC4                      JSR         FFC4(A6)
       | 030C  720A                           MOVEQ       #0A,D1
       | 030E  B081                           CMP.L       D1,D0
       | 0310  6642                           BNE.B       0354
; 209:                         {
; 210:                             if(nr = AllocMem(sizeof(struct NIPCRoute),MEMF_CLEAR|MEMF_PUBLIC))
       | 0312  7058                           MOVEQ       #58,D0
       | 0314  223C 0001 0001                 MOVE.L      #00010001,D1
       | 031A  2C53                           MOVEA.L     (A3),A6
       | 031C  4EAE FF3A                      JSR         FF3A(A6)
       | 0320  2440                           MOVEA.L     D0,A2
       | 0322  200A                           MOVE.L      A2,D0
       | 0324  6728                           BEQ.B       034E
; 211:                             {
; 212:                                 CopyMem((APTR)&tmp_route,(APTR)&nr->nr_Prefs,sizeof(struct NIPCRoutePrefs));
       | 0326  41EA 004E                      LEA         004E(A2),A0
       | 032A  2248                           MOVEA.L     A0,A1
       | 032C  41ED FEA0                      LEA         FEA0(A5),A0
       | 0330  700A                           MOVEQ       #0A,D0
       | 0332  4EAE FD90                      JSR         FD90(A6)
; 213:                                 AddTail(&ed->ed_PrefsWork.ep_NIPCRoutes,(struct Node *)nr);
       | 0336  41EB 0334                      LEA         0334(A3),A0
       | 033A  224A                           MOVEA.L     A2,A1
       | 033C  4EAE FF0A                      JSR         FF0A(A6)
; 214:                                 nr->nr_Node.ln_Name = (STRPTR)&nr->nr_String;
       | 0340  41EA 000E                      LEA         000E(A2),A0
       | 0344  2548 000A                      MOVE.L      A0,000A(A2)
; 215:                                 return(ES_NORMAL);
       | 0348  7000                           MOVEQ       #00,D0
       | 034A  6000 00F0                      BRA.W       043C
; 216:                             }
; 217:                             return(ES_NO_MEMORY);
       | 034E  7001                           MOVEQ       #01,D0
       | 0350  6000 00EA                      BRA.W       043C
; 218:                         }
; 219:                         else
; 220:                             return(ES_IFFERROR);
       | 0354  7004                           MOVEQ       #04,D0
       | 0356  6000 00E4                      BRA.W       043C
; 221:                         break;
; 222:         case ID_NRRM:
; 223:         case ID_NLRM:
; 224:                         if(ReadChunkBytes(iff,&tmp_realm,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
       | 035A  206D FDB0                      MOVEA.L     FDB0(A5),A0
       | 035E  43ED FE5C                      LEA         FE5C(A5),A1
       | 0362  7044                           MOVEQ       #44,D0
       | 0364  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 0368  4EAE FFC4                      JSR         FFC4(A6)
       | 036C  7244                           MOVEQ       #44,D1
       | 036E  B081                           CMP.L       D1,D0
       | 0370  666E                           BNE.B       03E0
; 225:                         {
; 226:                             if(nz = AllocMem(sizeof(struct NIPCRealm),MEMF_CLEAR|MEMF_PUBLIC))
       | 0372  7069                           MOVEQ       #69,D0
       | 0374  D080                           ADD.L       D0,D0
       | 0376  223C 0001 0001                 MOVE.L      #00010001,D1
       | 037C  2C53                           MOVEA.L     (A3),A6
       | 037E  4EAE FF3A                      JSR         FF3A(A6)
       | 0382  2440                           MOVEA.L     D0,A2
       | 0384  200A                           MOVE.L      A2,D0
       | 0386  6754                           BEQ.B       03DC
; 227:                             {
; 228:                                 ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= (NHPFLAGF_REALMS|NHPFLAGF_REALMSERVER);
       | 0388  7003                           MOVEQ       #03,D0
       | 038A  81AB 03E6                      OR.L        D0,03E6(A3)
; 229:                                 CopyMem((APTR)&tmp_realm,(APTR)&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs));
       | 038E  41EA 008E                      LEA         008E(A2),A0
       | 0392  2248                           MOVEA.L     A0,A1
       | 0394  41ED FE5C                      LEA         FE5C(A5),A0
       | 0398  7044                           MOVEQ       #44,D0
       | 039A  2C53                           MOVEA.L     (A3),A6
       | 039C  4EAE FD90                      JSR         FD90(A6)
; 230:                                 nz->nz_Node.ln_Name = (STRPTR)&nz->nz_String;
       | 03A0  41EA 000E                      LEA         000E(A2),A0
       | 03A4  2548 000A                      MOVE.L      A0,000A(A2)
; 231:                                 if(cn->cn_ID == ID_NRRM)
       | 03A8  206D 0008                      MOVEA.L     0008(A5),A0
       | 03AC  0CA8 4E52 524D 0008            CMPI.L      #4E52524D,0008(A0)
       | 03B4  6612                           BNE.B       03C8
; 232:                                     InsertListSorted(ed,&ed->ed_PrefsWork.ep_NIPCRemoteRealms,(struct Node *)nz);
       | 03B6  41EB 0326                      LEA         0326(A3),A0
       | 03BA  2F0A                           MOVE.L      A2,-(A7)
       | 03BC  2248                           MOVEA.L     A0,A1
       | 03BE  204B                           MOVEA.L     A3,A0
       | 03C0  6100 FE02                      BSR.W       01C4
       | 03C4  584F                           ADDQ.W      #4,A7
; 233:                                 else
       | 03C6  6010                           BRA.B       03D8
; 234:                                     InsertListSorted(ed,&ed->ed_PrefsWork.ep_NIPCLocalRealms,(struct Node *)nz);
       | 03C8  41EB 0318                      LEA         0318(A3),A0
       | 03CC  2F0A                           MOVE.L      A2,-(A7)
       | 03CE  2248                           MOVEA.L     A0,A1
       | 03D0  204B                           MOVEA.L     A3,A0
       | 03D2  6100 FDF0                      BSR.W       01C4
       | 03D6  584F                           ADDQ.W      #4,A7
; 235:                                 return(ES_NORMAL);
       | 03D8  7000                           MOVEQ       #00,D0
       | 03DA  6060                           BRA.B       043C
; 236:                             }
; 237:                             return(ES_NO_MEMORY);
       | 03DC  7001                           MOVEQ       #01,D0
       | 03DE  605C                           BRA.B       043C
; 238:                         }
; 239:                         else
; 240:                             return(ES_IFFERROR);
       | 03E0  7004                           MOVEQ       #04,D0
       | 03E2  6058                           BRA.B       043C
; 241:                         break;
; 242: 
; 243:         case ID_HOST:	hostchunksize = sizeof(struct NIPCHostPrefs);
       | 03E4  7E54                           MOVEQ       #54,D7
       | 03E6  DE87                           ADD.L       D7,D7
; 244:         		if(cn->cn_Size < hostchunksize)
       | 03E8  202A 0010                      MOVE.L      0010(A2),D0
       | 03EC  7254                           MOVEQ       #54,D1
       | 03EE  D281                           ADD.L       D1,D1
       | 03F0  B081                           CMP.L       D1,D0
       | 03F2  6402                           BCC.B       03F6
; 245:         		{
; 246:         		    checkflags = TRUE;
; 247:         		    hostchunksize = cn->cn_Size;
       | 03F4  2E00                           MOVE.L      D0,D7
; 248: 			}
; 249: 
; 250:                         if(ReadChunkBytes(iff,&tmp_host,hostchunksize) == hostchunksize)
       | 03F6  206D FDB0                      MOVEA.L     FDB0(A5),A0
       | 03FA  43ED FDB4                      LEA         FDB4(A5),A1
       | 03FE  2007                           MOVE.L      D7,D0
       | 0400  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 0404  4EAE FFC4                      JSR         FFC4(A6)
       | 0408  B087                           CMP.L       D7,D0
       | 040A  662A                           BNE.B       0436
; 251:                         {
; 252:                             CopyMem((APTR)&tmp_host,&ed->ed_PrefsWork.ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs));
       | 040C  41EB 0342                      LEA         0342(A3),A0
       | 0410  2248                           MOVEA.L     A0,A1
       | 0412  41ED FDB4                      LEA         FDB4(A5),A0
       | 0416  7054                           MOVEQ       #54,D0
       | 0418  D080                           ADD.L       D0,D0
       | 041A  2C53                           MOVEA.L     (A3),A6
       | 041C  4EAE FD90                      JSR         FD90(A6)
; 253:                             if(checkflags && ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr)
       | 0420  7001                           MOVEQ       #01,D0
       | 0422  4A80                           TST.L       D0
       | 0424  670C                           BEQ.B       0432
       | 0426  4AAB 03C2                      TST.L       03C2(A3)
       | 042A  6706                           BEQ.B       0432
; 254:                                 ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMS;
       | 042C  08EB 0000 03E9                 BSET        #0000,03E9(A3)
; 255: 
; 256:                             return(ES_NORMAL);
       | 0432  7000                           MOVEQ       #00,D0
       | 0434  6006                           BRA.B       043C
; 257:                         }
; 258:                         else
; 259:                             return(ES_IFFERROR);
       | 0436  7004                           MOVEQ       #04,D0
       | 0438  6002                           BRA.B       043C
; 260:                         break;
; 261:         default:
; 262:                         return(ES_IFFERROR);
       | 043A  7004                           MOVEQ       #04,D0
; 263:     }
; 264: }
       | 043C  4CDF 4C80                      MOVEM.L     (A7)+,D7/A2-A3/A6
       | 0440  4E5D                           UNLK        A5
       | 0442  4E75                           RTS
; 265: 
; 266: EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
; 267: {
       | 0444  4E55 FFFC                      LINK        A5,#FFFC
       | 0448  2F0B                           MOVE.L      A3,-(A7)
       | 044A  2649                           MOVEA.L     A1,A3
       | 044C  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
; 268:     FreePrefs(ed, &ed->ed_PrefsDefaults);
       | 0450  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0454  D0FC 022A                      ADDA.W      #022A,A0
       | 0458  2248                           MOVEA.L     A0,A1
       | 045A  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 045E  6100 FCD6                      BSR.W       0136
; 269:     FreePrefs(ed, &ed->ed_PrefsWork);
       | 0462  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0466  D0FC 030A                      ADDA.W      #030A,A0
       | 046A  2248                           MOVEA.L     A0,A1
       | 046C  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0470  6100 FCC4                      BSR.W       0136
; 270:     FreePrefs(ed, &ed->ed_PrefsInitial);
       | 0474  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0478  D0FC 03EA                      ADDA.W      #03EA,A0
       | 047C  2248                           MOVEA.L     A0,A1
       | 047E  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0482  6100 FCB2                      BSR.W       0136
; 271:     return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
       | 0486  487A FDA0                      PEA         FDA0(PC)
       | 048A  4879  0000 0000-01             PEA         01.00000000
       | 0490  7006                           MOVEQ       #06,D0
       | 0492  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0496  224B                           MOVEA.L     A3,A1
       | 0498  4EBA  0000-XX.1                JSR         @ReadIFF(PC)
; 272: }
       | 049C  266D FFF8                      MOVEA.L     FFF8(A5),A3
       | 04A0  4E5D                           UNLK        A5
       | 04A2  4E75                           RTS
; 273: 
; 274: 
; 275: /*****************************************************************************/
; 276: 
; 277: 
; 278: EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
; 279: {
       | 04A4  4E55 FFF4                      LINK        A5,#FFF4
       | 04A8  48E7 2032                      MOVEM.L     D2/A2-A3/A6,-(A7)
       | 04AC  2648                           MOVEA.L     A0,A3
       | 04AE  2B49 FFF4                      MOVE.L      A1,FFF4(A5)
; 280:     struct NIPCDevice *nd;
; 281:     struct NIPCRoute  *nr;
; 282:     struct NIPCRealm  *nz;
; 283: 
; 284:     if ((ed->ed_Write == WRITE_ALL))
       | 04B2  4A6B 0814                      TST.W       0814(A3)
       | 04B6  6600 01B4                      BNE.W       066C
; 285:     {
; 286:         if(!PushChunk(iff,0,ID_HOST,sizeof(struct NIPCHostPrefs)))
       | 04BA  246D FFF4                      MOVEA.L     FFF4(A5),A2
       | 04BE  204A                           MOVEA.L     A2,A0
       | 04C0  7000                           MOVEQ       #00,D0
       | 04C2  223C 484F 5354                 MOVE.L      #484F5354,D1
       | 04C8  7454                           MOVEQ       #54,D2
       | 04CA  D482                           ADD.L       D2,D2
       | 04CC  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 04D0  4EAE FFAC                      JSR         FFAC(A6)
       | 04D4  4A80                           TST.L       D0
       | 04D6  6618                           BNE.B       04F0
; 287:         {
; 288:             if(WriteChunkBytes(iff,&ed->ed_PrefsWork.ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs)) == sizeof(struct NIPCHostPrefs))
       | 04D8  41EB 0342                      LEA         0342(A3),A0
       | 04DC  2248                           MOVEA.L     A0,A1
       | 04DE  2002                           MOVE.L      D2,D0
       | 04E0  204A                           MOVEA.L     A2,A0
       | 04E2  4EAE FFBE                      JSR         FFBE(A6)
       | 04E6  B082                           CMP.L       D2,D0
       | 04E8  6606                           BNE.B       04F0
; 289:             {
; 290:                 PopChunk(iff);
       | 04EA  204A                           MOVEA.L     A2,A0
       | 04EC  4EAE FFA6                      JSR         FFA6(A6)
; 291:             }
; 292:         }
; 293:         nd = (struct NIPCDevice *)ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
       | 04F0  2B6B 030A FFF8                 MOVE.L      030A(A3),FFF8(A5)
; 294:         while(nd->nd_Node.ln_Succ)
       | 04F6  605E                           BRA.B       0556
; 295:         {
; 296:             if(Stricmp("(new)",(STRPTR)&nd->nd_Prefs.ndp_DevPathName))
       | 04F8  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 04FC  D0FC 005B                      ADDA.W      #005B,A0
       | 0500  2248                           MOVEA.L     A0,A1
       | 0502  41FA FB4E                      LEA         FB4E(PC),A0
       | 0506  2C6B 0020                      MOVEA.L     0020(A3),A6
       | 050A  4EAE FF5E                      JSR         FF5E(A6)
       | 050E  4A80                           TST.L       D0
       | 0510  673C                           BEQ.B       054E
; 297:             {
; 298:                 if(!PushChunk(iff,0,ID_NDEV,sizeof(struct NIPCDevicePrefs)))
       | 0512  204A                           MOVEA.L     A2,A0
       | 0514  7000                           MOVEQ       #00,D0
       | 0516  223C 4E44 4556                 MOVE.L      #4E444556,D1
       | 051C  243C 0000 014E                 MOVE.L      #0000014E,D2
       | 0522  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 0526  4EAE FFAC                      JSR         FFAC(A6)
       | 052A  4A80                           TST.L       D0
       | 052C  6620                           BNE.B       054E
; 299:                 {
; 300:                     if(WriteChunkBytes(iff,&nd->nd_Prefs,sizeof(struct NIPCDevicePrefs)) == sizeof(struct NIPCDevicePrefs))
       | 052E  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 0532  D0FC 000E                      ADDA.W      #000E,A0
       | 0536  2248                           MOVEA.L     A0,A1
       | 0538  2002                           MOVE.L      D2,D0
       | 053A  204A                           MOVEA.L     A2,A0
       | 053C  4EAE FFBE                      JSR         FFBE(A6)
       | 0540  0C80 0000 014E                 CMPI.L      #0000014E,D0
       | 0546  6606                           BNE.B       054E
; 301:                     {
; 302:                         PopChunk(iff);
       | 0548  204A                           MOVEA.L     A2,A0
       | 054A  4EAE FFA6                      JSR         FFA6(A6)
; 303:                     }
; 304:                 }
; 305:             }
; 306:             nd = (struct NIPCDevice *)nd->nd_Node.ln_Succ;
       | 054E  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 0552  2B50 FFF8                      MOVE.L      (A0),FFF8(A5)
; 307:         }
       | 0556  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 055A  4A90                           TST.L       (A0)
       | 055C  669A                           BNE.B       04F8
; 308:         nr = (struct NIPCRoute *)ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
       | 055E  246B 0334                      MOVEA.L     0334(A3),A2
; 309:         while(nr->nr_Node.ln_Succ)
       | 0562  6050                           BRA.B       05B4
; 310:         {
; 311:             if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
       | 0564  41FA FAEC                      LEA         FAEC(PC),A0
       | 0568  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 056C  2269 000A                      MOVEA.L     000A(A1),A1
       | 0570  2C6B 0020                      MOVEA.L     0020(A3),A6
       | 0574  4EAE FF5E                      JSR         FF5E(A6)
       | 0578  4A80                           TST.L       D0
       | 057A  6736                           BEQ.B       05B2
; 312:             {
; 313:                 if(!PushChunk(iff,0,ID_NIRT,sizeof(struct NIPCRoutePrefs)))
       | 057C  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 0580  7000                           MOVEQ       #00,D0
       | 0582  223C 4E49 5254                 MOVE.L      #4E495254,D1
       | 0588  740A                           MOVEQ       #0A,D2
       | 058A  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 058E  4EAE FFAC                      JSR         FFAC(A6)
       | 0592  4A80                           TST.L       D0
       | 0594  661C                           BNE.B       05B2
; 314:                 {
; 315:                     if(WriteChunkBytes(iff,&nr->nr_Prefs,sizeof(struct NIPCRoutePrefs)) == sizeof(struct NIPCRoutePrefs))
       | 0596  41EA 004E                      LEA         004E(A2),A0
       | 059A  2248                           MOVEA.L     A0,A1
       | 059C  2002                           MOVE.L      D2,D0
       | 059E  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 05A2  4EAE FFBE                      JSR         FFBE(A6)
       | 05A6  B082                           CMP.L       D2,D0
       | 05A8  6608                           BNE.B       05B2
; 316:                     {
; 317:                         PopChunk(iff);
       | 05AA  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 05AE  4EAE FFA6                      JSR         FFA6(A6)
; 318:                     }
; 319:                 }
; 320:             }
; 321:             nr = (struct NIPCRoute *)nr->nr_Node.ln_Succ;
       | 05B2  2452                           MOVEA.L     (A2),A2
; 322:         }
       | 05B4  4A92                           TST.L       (A2)
       | 05B6  66AC                           BNE.B       0564
; 323:         nz = (struct NIPCRealm *)ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
       | 05B8  246B 0318                      MOVEA.L     0318(A3),A2
; 324:         while(nz->nz_Node.ln_Succ)
       | 05BC  6050                           BRA.B       060E
; 325:         {
; 326:             if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
       | 05BE  41FA FA92                      LEA         FA92(PC),A0
       | 05C2  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 05C6  2269 000A                      MOVEA.L     000A(A1),A1
       | 05CA  2C6B 0020                      MOVEA.L     0020(A3),A6
       | 05CE  4EAE FF5E                      JSR         FF5E(A6)
       | 05D2  4A80                           TST.L       D0
       | 05D4  6736                           BEQ.B       060C
; 327:             {
; 328:                 if(!PushChunk(iff,0,ID_NLRM,sizeof(struct NIPCRealmPrefs)))
       | 05D6  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 05DA  7000                           MOVEQ       #00,D0
       | 05DC  223C 4E4C 524D                 MOVE.L      #4E4C524D,D1
       | 05E2  7444                           MOVEQ       #44,D2
       | 05E4  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 05E8  4EAE FFAC                      JSR         FFAC(A6)
       | 05EC  4A80                           TST.L       D0
       | 05EE  661C                           BNE.B       060C
; 329:                 {
; 330:                     if(WriteChunkBytes(iff,&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
       | 05F0  41EA 008E                      LEA         008E(A2),A0
       | 05F4  2248                           MOVEA.L     A0,A1
       | 05F6  2002                           MOVE.L      D2,D0
       | 05F8  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 05FC  4EAE FFBE                      JSR         FFBE(A6)
       | 0600  B082                           CMP.L       D2,D0
       | 0602  6608                           BNE.B       060C
; 331:                     {
; 332:                         PopChunk(iff);
       | 0604  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 0608  4EAE FFA6                      JSR         FFA6(A6)
; 333:                     }
; 334:                 }
; 335:             }
; 336:             nz = (struct NIPCRealm *)nz->nz_Node.ln_Succ;
       | 060C  2452                           MOVEA.L     (A2),A2
; 337:         }
       | 060E  4A92                           TST.L       (A2)
       | 0610  66AC                           BNE.B       05BE
; 338:         nz = (struct NIPCRealm *)ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
       | 0612  246B 0326                      MOVEA.L     0326(A3),A2
; 339:         while(nz->nz_Node.ln_Succ)
       | 0616  6050                           BRA.B       0668
; 340:         {
; 341:             if(Stricmp("(new)",(STRPTR)nd->nd_Node.ln_Name))
       | 0618  41FA FA38                      LEA         FA38(PC),A0
       | 061C  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 0620  2269 000A                      MOVEA.L     000A(A1),A1
       | 0624  2C6B 0020                      MOVEA.L     0020(A3),A6
       | 0628  4EAE FF5E                      JSR         FF5E(A6)
       | 062C  4A80                           TST.L       D0
       | 062E  6736                           BEQ.B       0666
; 342:             {
; 343:                 if(!PushChunk(iff,0,ID_NRRM,sizeof(struct NIPCRealmPrefs)))
       | 0630  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 0634  7000                           MOVEQ       #00,D0
       | 0636  223C 4E52 524D                 MOVE.L      #4E52524D,D1
       | 063C  7444                           MOVEQ       #44,D2
       | 063E  2C6B 0014                      MOVEA.L     0014(A3),A6
       | 0642  4EAE FFAC                      JSR         FFAC(A6)
       | 0646  4A80                           TST.L       D0
       | 0648  661C                           BNE.B       0666
; 344:                 {
; 345:                     if(WriteChunkBytes(iff,&nz->nz_Prefs,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
       | 064A  41EA 008E                      LEA         008E(A2),A0
       | 064E  2248                           MOVEA.L     A0,A1
       | 0650  2002                           MOVE.L      D2,D0
       | 0652  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 0656  4EAE FFBE                      JSR         FFBE(A6)
       | 065A  B082                           CMP.L       D2,D0
       | 065C  6608                           BNE.B       0666
; 346:                     {
; 347:                         PopChunk(iff);
       | 065E  206D FFF4                      MOVEA.L     FFF4(A5),A0
       | 0662  4EAE FFA6                      JSR         FFA6(A6)
; 348:                     }
; 349:                 }
; 350:             }
; 351:             nz = (struct NIPCRealm *)nz->nz_Node.ln_Succ;
       | 0666  2452                           MOVEA.L     (A2),A2
; 352:         }
       | 0668  4A92                           TST.L       (A2)
       | 066A  66AC                           BNE.B       0618
; 353: 
; 354:     }
; 355:     return(ES_NORMAL);
       | 066C  7000                           MOVEQ       #00,D0
; 356: }
       | 066E  4CDF 4C04                      MOVEM.L     (A7)+,D2/A2-A3/A6
       | 0672  4E5D                           UNLK        A5
       | 0674  4E75                           RTS
; 357: 
; 358: 
; 359: EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
; 360: {
       | 0676  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 067A  2648                           MOVEA.L     A0,A3
       | 067C  2449                           MOVEA.L     A1,A2
; 361:     return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
       | 067E  487A FE24                      PEA         FE24(PC)
       | 0682  4879  0000 0048-01             PEA         01.00000048
       | 0688  204B                           MOVEA.L     A3,A0
       | 068A  224A                           MOVEA.L     A2,A1
       | 068C  4EBA  0000-XX.1                JSR         @WriteIFF(PC)
       | 0690  504F                           ADDQ.W      #8,A7
; 362: }
       | 0692  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 0696  4E75                           RTS
; 363: 
; 364: 
; 365: /*****************************************************************************/
; 366: 
; 367: 
; 368: #define NW_WIDTH     600
; 369: #define NW_HEIGHT    180
; 370: #define NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
; 371: #define NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
; 372: #define NW_MINWIDTH  NW_WIDTH
; 373: #define NW_MINHEIGHT NW_HEIGHT
; 374: #define NW_MAXWIDTH  NW_WIDTH
; 375: #define NW_MAXHEIGHT NW_HEIGHT
; 376: #define ZOOMWIDTH    200
; 377: 
; 378: struct EdMenu far EM[] = {
; 379:     {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
; 380:       {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN, 0},
; 381:       {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
; 382:       {NM_ITEM, MSG_NOTHING,                EC_NOP, 0},
; 383:       {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},
; 384: 
; 385:     {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP, 0},
; 386:       {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
; 387:       {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
; 388:       {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE, 0},
; 389: 
; 390:     {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP, 0},
; 391:       {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},
; 392: 
; 393:     {NM_END,    MSG_NOTHING,                EC_NOP, 0}
; 394: };
; 395: 
; 396: /* main display gadgets */
; 397: struct EdGadget far EG[] = {
; 398: 
; 399:     /* Main Panel */
; 400: 
; 401:     {BUTTON_KIND,   10,  160,  152, 14, MSG_SAVE_GAD,         EC_SAVE,            0},
; 402:     {BUTTON_KIND,   224, 160,  152, 14, MSG_USE_GAD,          EC_USE,             0},
; 403:     {BUTTON_KIND,   444, 160,  152, 14, MSG_CANCEL_GAD,       EC_CANCEL,          0},
; 404:     {BUTTON_KIND,   402, 26,   188, 24, 0,                    EC_MAIN_ROUTES,     0},
; 405:     {BUTTON_KIND,   402, 66,   188, 24, 0,                    EC_MAIN_REALMS,     0},
; 406:     {BUTTON_KIND,   402, 106,  188, 24, 0,                    EC_MAIN_DEVICES,    0},
; 407: 
; 408:     {STRING_KIND,   156, 26,   164, 14, MSG_HOST_NAME,        EC_MAIN_HOSTNAME,   0},
; 409:     {STRING_KIND,   156, 80,   164, 14, MSG_REALM_NAME,       EC_MAIN_REALMNAME,  0},
; 410:     {STRING_KIND,   156, 96,   164, 14, MSG_REALM_ADDR,       EC_MAIN_REALMADDR,  0},
; 411: 
; 412:     {CYCLE_KIND,    10,  4,    200, 14, 0,                    EC_PANEL,           0},
; 413: 
; 414:     /* Realms Panel */
; 415: 
; 416:     {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_REALM_CANCEL,    0},
; 417:     {BUTTON_KIND,   12,  142,  132, 14, MSG_ADD_REALM,        EC_LOCREALM_ADD,    0},
; 418:     {BUTTON_KIND,   148, 142,  132, 14, MSG_DEL_REALM,        EC_LOCREALM_DELETE, 0},
; 419:     {BUTTON_KIND,   310, 142,  132, 14, MSG_ADD_REALM,        EC_REMREALM_ADD,    0},
; 420:     {BUTTON_KIND,   446, 142,  132, 14, MSG_DEL_REALM,        EC_REMREALM_DELETE, 0},
; 421: 
; 422:     {TEXT_KIND,     10,  22,   272, 12, MSG_LOC_TXT,          0,                  PLACETEXT_IN | NG_HIGHLABEL},
; 423:     {TEXT_KIND,     308, 22,   272, 12, MSG_REM_TXT,          0,                  PLACETEXT_IN | NG_HIGHLABEL},
; 424: 
; 425:     {STRING_KIND,   10,  126,  136, 14, 0,                    EC_LOCREALM_NAME,   0},
; 426:     {STRING_KIND,   146, 126,  136, 14, 0,                    EC_LOCREALM_NET,    0},
; 427:     {STRING_KIND,   308, 126,  136, 14, 0,                    EC_REMREALM_NAME,   0},
; 428:     {STRING_KIND,   444, 126,  136, 14, 0,                    EC_REMREALM_NET,   0},
; 429: 
; 430:     {LISTVIEW_KIND, 10,  50,   290, 76, MSG_LOC_REALMS,       EC_LOCREALM_LIST,   PLACETEXT_ABOVE},
; 431:     {LISTVIEW_KIND, 308, 50,   290, 76, MSG_REM_REALMS,       EC_REMREALM_LIST,   PLACETEXT_ABOVE},
; 432: 
; 433:     /* Routes Panel */
; 434: 
; 435:     {BUTTON_KIND,   10,  160,  152, 14, 0,                    EC_ROUTE_ACCEPT,    0},
; 436:     {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_ROUTE_CANCEL,    0},
; 437:     {BUTTON_KIND,   146, 130,  168, 14, MSG_ADD_ROUTE,        EC_ROUTE_ADD,       0},
; 438:     {BUTTON_KIND,   314, 130,  168, 14, MSG_DEL_ROUTE,        EC_ROUTE_DELETE,    0},
; 439: 
; 440:     {STRING_KIND,   146, 116,  168, 14, 0,                    EC_ROUTE_DEST,      0},
; 441:     {STRING_KIND,   314, 116,  168, 14, 0,                    EC_ROUTE_GATEWAY,   0},
; 442: 
; 443:     {TEXT_KIND,	    146, 26,   168, 14, MSG_ROUTE_DEST,	      0,		  PLACETEXT_IN},
; 444:     {LISTVIEW_KIND, 146, 40,   336, 76, 0,		      EC_ROUTE_LIST,      0},
; 445: 
; 446:     /* Devices Panel */
; 447: 
; 448:     {BUTTON_KIND,   10,  160,  152, 14, 0,                    EC_DEVICE_ACCEPT,    0},
; 449:     {BUTTON_KIND,   444, 160,  152, 14, 0,                    EC_DEVICE_CANCEL,    0},
; 450:     {BUTTON_KIND,   10,  126,  110, 14, MSG_DEVICE_ADD,       EC_DEVICE_ADD,       0},
; 451:     {BUTTON_KIND,   120, 126,  110, 14, MSG_DEVICE_DEL,       EC_DEVICE_DEL,       0},
; 452: 
; 453:     {TEXT_KIND,     240, 8,    356, 16, MSG_DEVICE_TXT,       0,                   PLACETEXT_IN | NG_HIGHLABEL},
; 454: 
; 455:     {CYCLE_KIND,    364, 120,  110, 14, MSG_DEVICE_STATUS,    EC_DEVICE_STATUS,    0},
; 456: 
; 457:     {CHECKBOX_KIND, 476, 42,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_MASK_ABLE, PLACETEXT_RIGHT},
; 458:     {CHECKBOX_KIND, 476, 58,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_ADDR_ABLE, PLACETEXT_RIGHT},
; 459:     {CHECKBOX_KIND, 476, 74,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_IPTYPE_ABLE,  PLACETEXT_RIGHT},
; 460:     {CHECKBOX_KIND, 476, 90,   26,  11, MSG_DEVICE_DEFAULT,   EC_DEVICE_ARPTYPE_ABLE, PLACETEXT_RIGHT},
; 461: 
; 462:     {STRING_KIND,   10,  100,  220, 14, 0,                    EC_DEVICE_NAME,      0},
; 463:     {INTEGER_KIND,  402, 104,  72,  14, MSG_DEVICE_UNIT,      EC_DEVICE_UNIT,      0},
; 464:     {STRING_KIND,   338, 24,   136, 14, MSG_DEVICE_IP,        EC_DEVICE_IP,        0},
; 465:     {STRING_KIND,   338, 40,   136, 14, MSG_DEVICE_MASK,      EC_DEVICE_MASK,      0},
; 466:     {STRING_KIND,   338, 56,   136, 14, MSG_DEVICE_ADDR,      EC_DEVICE_ADDR,      0},
; 467:     {INTEGER_KIND,  402, 72,   72,  14, MSG_DEVICE_IPTYPE,    EC_DEVICE_IPTYPE,    0},
; 468:     {INTEGER_KIND,  402, 88,   72,  14, MSG_DEVICE_ARPTYPE,   EC_DEVICE_ARPTYPE,   0},
; 469: 
; 470:     {LISTVIEW_KIND, 10,  24,   220, 116,0,                    EC_DEVICE_LIST,      PLACETEXT_ABOVE},
; 471:     {LISTVIEW_KIND, 236, 104,  360, 52, MSG_DEVICE_INFO,      0,                   PLACETEXT_ABOVE},
; 472: 
; 473:     /* New Main Panel Gadgets */
; 474: 
; 475:     {STRING_KIND,   156, 42,   164, 14, MSG_OWNER_NAME,	      EC_MAIN_OWNERNAME,   0},
; 476:     {CHECKBOX_KIND, 564, 26,   26,  11, MSG_USE_REALMSERVER,  EC_MAIN_REALMS,	   0},
; 477:     {CHECKBOX_KIND, 564, 42,  26,  11, MSG_IS_REALMSERVER,   EC_MAIN_REALMSERVER, 0},
; 478: 
; 479:     {TEXT_KIND,	    314, 26,   168, 14, MSG_ROUTE_GATE,	      0,		  PLACETEXT_IN},
; 480: 
; 481: };
; 482: 
; 483: /*****************************************************************************/
; 484: 
; 485: 
; 486: BOOL CreateDisplay(EdDataPtr ed)
; 487: {
       | 0698  4E55 FFF4                      LINK        A5,#FFF4
       | 069C  48E7 3030                      MOVEM.L     D2-D3/A2-A3,-(A7)
       | 06A0  2648                           MOVEA.L     A0,A3
; 488: UWORD zoomSize[4];
; 489: 
; 490:     zoomSize[0] = -1;
       | 06A2  70FF                           MOVEQ       #FF,D0
       | 06A4  3B40 FFF4                      MOVE.W      D0,FFF4(A5)
; 491:     zoomSize[1] = -1;
       | 06A8  3B40 FFF6                      MOVE.W      D0,FFF6(A5)
; 492:     zoomSize[2] = ZOOMWIDTH;
       | 06AC  3B7C 00C8 FFF8                 MOVE.W      #00C8,FFF8(A5)
; 493:     zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;
       | 06B2  206B 01C0                      MOVEA.L     01C0(A3),A0
       | 06B6  2268 0028                      MOVEA.L     0028(A0),A1
       | 06BA  7000                           MOVEQ       #00,D0
       | 06BC  3029 0004                      MOVE.W      0004(A1),D0
       | 06C0  1228 0023                      MOVE.B      0023(A0),D1
       | 06C4  4881                           EXT.W       D1
       | 06C6  48C1                           EXT.L       D1
       | 06C8  D280                           ADD.L       D0,D1
       | 06CA  5281                           ADDQ.L      #1,D1
       | 06CC  3B41 FFFA                      MOVE.W      D1,FFFA(A5)
; 494: 
; 495:     ed->ed_StatusLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_DEVICE_STATUSLABEL0);
       | 06D0  45EB 06CA                      LEA         06CA(A3),A2
       | 06D4  203C 0000 A434                 MOVE.L      #0000A434,D0
       | 06DA  204A                           MOVEA.L     A2,A0
       | 06DC  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 06E0  2740 0772                      MOVE.L      D0,0772(A3)
; 496:     ed->ed_StatusLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_DEVICE_STATUSLABEL1);
       | 06E4  203C 0000 A435                 MOVE.L      #0000A435,D0
       | 06EA  204A                           MOVEA.L     A2,A0
       | 06EC  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 06F0  2740 0776                      MOVE.L      D0,0776(A3)
; 497: 
; 498:     ed->ed_PanelLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_MAIN);
       | 06F4  203C 0000 A416                 MOVE.L      #0000A416,D0
       | 06FA  204A                           MOVEA.L     A2,A0
       | 06FC  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 0700  2740 075E                      MOVE.L      D0,075E(A3)
; 499:     ed->ed_PanelLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_DEVICES);
       | 0704  203C 0000 A415                 MOVE.L      #0000A415,D0
       | 070A  204A                           MOVEA.L     A2,A0
       | 070C  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 0710  2740 0762                      MOVE.L      D0,0762(A3)
; 500:     ed->ed_PanelLabels[2] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_ROUTES);
       | 0714  203C 0000 A413                 MOVE.L      #0000A413,D0
       | 071A  204A                           MOVEA.L     A2,A0
       | 071C  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 0720  2740 0766                      MOVE.L      D0,0766(A3)
; 501:     ed->ed_PanelLabels[3] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_REALMS);
       | 0724  203C 0000 A414                 MOVE.L      #0000A414,D0
       | 072A  204A                           MOVEA.L     A2,A0
       | 072C  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 0730  2740 076A                      MOVE.L      D0,076A(A3)
; 502: 
; 503:     ed->ed_LastAdded = NULL; /* CreateContext(&ed->ed_Gadgets); */
       | 0734  42AB 01B8                      CLR.L       01B8(A3)
; 504: 
; 505:     RenderGadgets(ed);
       | 0738  204B                           MOVEA.L     A3,A0
       | 073A  6100 023A                      BSR.W       0976
; 506: 
; 507:     if ((ed->ed_LastAdded)
       | 073E  4AAB 01B8                      TST.L       01B8(A3)
       | 0742  6700 00BE                      BEQ.W       0802
; 508:     &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
       | 0746  204B                           MOVEA.L     A3,A0
       | 0748  43F9  0000 004E-01             LEA         01.0000004E,A1
       | 074E  4EBA  0000-XX.1                JSR         @CreatePrefsMenus(PC)
       | 0752  2740 01C4                      MOVE.L      D0,01C4(A3)
       | 0756  6700 00AA                      BEQ.W       0802
; 509:     &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
; 510:                                             WA_InnerHeight, NW_HEIGHT,
; 511:                                             WA_MinWidth,    NW_MINWIDTH,
; 512:                                             WA_MinHeight,   NW_MINHEIGHT,
; 513:                                             WA_MaxWidth,    NW_MAXWIDTH,
; 514:                                             WA_MaxHeight,   NW_MAXHEIGHT,
; 515:                                             WA_IDCMP,       NW_IDCMP,
; 516:                                             WA_Flags,       NW_FLAGS,
; 517:                                             WA_Zoom,        zoomSize,
; 518:                                             WA_AutoAdjust,  TRUE,
; 519:                                             WA_PubScreen,   ed->ed_Screen,
; 520:                                             WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_NDEV_NAME),
       | 075A  203C 0000 A410                 MOVE.L      #0000A410,D0
       | 0760  204A                           MOVEA.L     A2,A0
       | 0762  4EBA  0000-XX.1                JSR         @GetString(PC)
; 521:                                             WA_NewLookMenus,TRUE,
; 522:                                             WA_Gadgets,     ed->ed_Gadgets,
; 523:                                             TAG_DONE)))
       | 0766  42A7                           CLR.L       -(A7)
       | 0768  2F2B 01C8                      MOVE.L      01C8(A3),-(A7)
       | 076C  2F3C 8000 006C                 MOVE.L      #8000006C,-(A7)
       | 0772  7201                           MOVEQ       #01,D1
       | 0774  2F01                           MOVE.L      D1,-(A7)
       | 0776  2F3C 8000 0093                 MOVE.L      #80000093,-(A7)
       | 077C  2F00                           MOVE.L      D0,-(A7)
       | 077E  2F3C 8000 006E                 MOVE.L      #8000006E,-(A7)
       | 0784  2F2B 01C0                      MOVE.L      01C0(A3),-(A7)
       | 0788  2F3C 8000 0079                 MOVE.L      #80000079,-(A7)
       | 078E  2F01                           MOVE.L      D1,-(A7)
       | 0790  2F3C 8000 0090                 MOVE.L      #80000090,-(A7)
       | 0796  486D FFF4                      PEA         FFF4(A5)
       | 079A  2F3C 8000 007D                 MOVE.L      #8000007D,-(A7)
       | 07A0  4878 1046                      PEA         1046
       | 07A4  2F3C 8000 006B                 MOVE.L      #8000006B,-(A7)
       | 07AA  2F3C 0040 017C                 MOVE.L      #0040017C,-(A7)
       | 07B0  2F3C 8000 006A                 MOVE.L      #8000006A,-(A7)
       | 07B6  745A                           MOVEQ       #5A,D2
       | 07B8  D482                           ADD.L       D2,D2
       | 07BA  2F02                           MOVE.L      D2,-(A7)
       | 07BC  2F3C 8000 0075                 MOVE.L      #80000075,-(A7)
       | 07C2  764B                           MOVEQ       #4B,D3
       | 07C4  E78B                           LSL.L       #3,D3
       | 07C6  2F03                           MOVE.L      D3,-(A7)
       | 07C8  2F3C 8000 0074                 MOVE.L      #80000074,-(A7)
       | 07CE  2F02                           MOVE.L      D2,-(A7)
       | 07D0  2F3C 8000 0073                 MOVE.L      #80000073,-(A7)
       | 07D6  2F03                           MOVE.L      D3,-(A7)
       | 07D8  2F3C 8000 0072                 MOVE.L      #80000072,-(A7)
       | 07DE  2F02                           MOVE.L      D2,-(A7)
       | 07E0  2F3C 8000 0077                 MOVE.L      #80000077,-(A7)
       | 07E6  2F03                           MOVE.L      D3,-(A7)
       | 07E8  2F3C 8000 0076                 MOVE.L      #80000076,-(A7)
       | 07EE  2F0B                           MOVE.L      A3,-(A7)
       | 07F0  4EBA  0000-XX.1                JSR         _OpenPrefsWindow(PC)
       | 07F4  4FEF 0078                      LEA         0078(A7),A7
       | 07F8  2740 01BC                      MOVE.L      D0,01BC(A3)
       | 07FC  6704                           BEQ.B       0802
; 524:     {
; 525:         return(TRUE);
       | 07FE  7001                           MOVEQ       #01,D0
       | 0800  6008                           BRA.B       080A
; 526:     }
; 527: 
; 528:     DisposeDisplay(ed);
       | 0802  204B                           MOVEA.L     A3,A0
       | 0804  6100 000C                      BSR.W       0812
; 529:     return(FALSE);
       | 0808  7000                           MOVEQ       #00,D0
; 530: }
       | 080A  4CDF 0C0C                      MOVEM.L     (A7)+,D2-D3/A2-A3
       | 080E  4E5D                           UNLK        A5
       | 0810  4E75                           RTS
; 531: 
; 532: 
; 533: /*****************************************************************************/
; 534: 
; 535: 
; 536: VOID DisposeDisplay(EdDataPtr ed)
; 537: {
       | 0812  48E7 0012                      MOVEM.L     A3/A6,-(A7)
       | 0816  2648                           MOVEA.L     A0,A3
; 538:     if (ed->ed_Window)
       | 0818  202B 01BC                      MOVE.L      01BC(A3),D0
       | 081C  6712                           BEQ.B       0830
; 539:     {
; 540:         ClearMenuStrip(ed->ed_Window);
       | 081E  2040                           MOVEA.L     D0,A0
       | 0820  2C6B 0004                      MOVEA.L     0004(A3),A6
       | 0824  4EAE FFCA                      JSR         FFCA(A6)
; 541:         CloseWindow(ed->ed_Window);
       | 0828  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 082C  4EAE FFB8                      JSR         FFB8(A6)
; 542:     }
; 543:     FreeMenus(ed->ed_Menus);
       | 0830  206B 01C4                      MOVEA.L     01C4(A3),A0
       | 0834  2C6B 001C                      MOVEA.L     001C(A3),A6
       | 0838  4EAE FFCA                      JSR         FFCA(A6)
; 544:     FreeGadgets(ed->ed_Gadgets);
       | 083C  206B 01C8                      MOVEA.L     01C8(A3),A0
       | 0840  4EAE FFDC                      JSR         FFDC(A6)
; 545: }
       | 0844  4CDF 4800                      MOVEM.L     (A7)+,A3/A6
       | 0848  4E75                           RTS
; 546: 
; 547: 
; 548: /*****************************************************************************/
; 549: 
; 550: 
; 551: VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
; 552:                 UWORD x, UWORD y, UWORD w)
; 553: {
       | 084A  4E55 FFF4                      LINK        A5,#FFF4
       | 084E  48E7 3F12                      MOVEM.L     D2-D7/A3/A6,-(A7)
       | 0852  2649                           MOVEA.L     A1,A3
       | 0854  2E00                           MOVE.L      D0,D7
       | 0856  2C01                           MOVE.L      D1,D6
       | 0858  3A2D 000A                      MOVE.W      000A(A5),D5
       | 085C  382D 000E                      MOVE.W      000E(A5),D4
       | 0860  2B48 FFFC                      MOVE.L      A0,FFFC(A5)
; 554: STRPTR str;
; 555: UWORD  len;
; 556: 
; 557:     str = GetString(&ed->ed_LocaleInfo,id);
       | 0864  206D FFFC                      MOVEA.L     FFFC(A5),A0
       | 0868  D0FC 06CA                      ADDA.W      #06CA,A0
       | 086C  2007                           MOVE.L      D7,D0
       | 086E  4EBA  0000-XX.1                JSR         @GetString(PC)
; 558:     len = strlen(str);
       | 0872  2040                           MOVEA.L     D0,A0
       | 0874  4A18                           TST.B       (A0)+
       | 0876  66FC                           BNE.B       0874
       | 0878  5388                           SUBQ.L      #1,A0
       | 087A  91C0                           SUBA.L      D0,A0
       | 087C  2208                           MOVE.L      A0,D1
; 559: 
; 560:     Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
       | 087E  7400                           MOVEQ       #00,D2
       | 0880  3401                           MOVE.W      D1,D2
       | 0882  2F40 0020                      MOVE.L      D0,0020(A7)
       | 0886  2040                           MOVEA.L     D0,A0
       | 0888  2F42 0024                      MOVE.L      D2,0024(A7)
       | 088C  2002                           MOVE.L      D2,D0
       | 088E  226B 0032                      MOVEA.L     0032(A3),A1
       | 0892  2C6D FFFC                      MOVEA.L     FFFC(A5),A6
       | 0896  2C6E 000C                      MOVEA.L     000C(A6),A6
       | 089A  4EAE FFCA                      JSR         FFCA(A6)
       | 089E  7200                           MOVEQ       #00,D1
       | 08A0  3206                           MOVE.W      D6,D1
       | 08A2  162B 0036                      MOVE.B      0036(A3),D3
       | 08A6  4883                           EXT.W       D3
       | 08A8  48C3                           EXT.L       D3
       | 08AA  48C0                           EXT.L       D0
       | 08AC  7400                           MOVEQ       #00,D2
       | 08AE  3404                           MOVE.W      D4,D2
       | 08B0  9480                           SUB.L       D0,D2
       | 08B2  4A82                           TST.L       D2
       | 08B4  6A02                           BPL.B       08B8
       | 08B6  5282                           ADDQ.L      #1,D2
       | 08B8  E282                           ASR.L       #1,D2
       | 08BA  D483                           ADD.L       D3,D2
       | 08BC  D481                           ADD.L       D1,D2
       | 08BE  7000                           MOVEQ       #00,D0
       | 08C0  3005                           MOVE.W      D5,D0
       | 08C2  122B 0037                      MOVE.B      0037(A3),D1
       | 08C6  4881                           EXT.W       D1
       | 08C8  48C1                           EXT.L       D1
       | 08CA  D280                           ADD.L       D0,D1
       | 08CC  2002                           MOVE.L      D2,D0
       | 08CE  226B 0032                      MOVEA.L     0032(A3),A1
       | 08D2  4EAE FF10                      JSR         FF10(A6)
; 561:     Text(wp->RPort,str,len);
       | 08D6  226B 0032                      MOVEA.L     0032(A3),A1
       | 08DA  206F 0020                      MOVEA.L     0020(A7),A0
       | 08DE  202F 0024                      MOVE.L      0024(A7),D0
       | 08E2  4EAE FFC4                      JSR         FFC4(A6)
; 562: }
       | 08E6  4CDF 48FC                      MOVEM.L     (A7)+,D2-D7/A3/A6
       | 08EA  4E5D                           UNLK        A5
       | 08EC  4E75                           RTS
; 563: 
; 564: 
; 565: /*****************************************************************************/
; 566: 
; 567: 
; 568: VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
; 569: {
       | 08EE  4E55 FFF0                      LINK        A5,#FFF0
       | 08F2  48E7 3F12                      MOVEM.L     D2-D7/A3/A6,-(A7)
       | 08F6  266D 0008                      MOVEA.L     0008(A5),A3
       | 08FA  3E2D 000E                      MOVE.W      000E(A5),D7
       | 08FE  3C2D 0012                      MOVE.W      0012(A5),D6
       | 0902  3A2D 0016                      MOVE.W      0016(A5),D5
       | 0906  382D 001A                      MOVE.W      001A(A5),D4
; 570:     DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
       | 090A  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 090E  1028 0036                      MOVE.B      0036(A0),D0
       | 0912  4880                           EXT.W       D0
       | 0914  48C0                           EXT.L       D0
       | 0916  2207                           MOVE.L      D7,D1
       | 0918  48C1                           EXT.L       D1
       | 091A  D280                           ADD.L       D0,D1
; 571:                                        y+ed->ed_Window->BorderTop,
       | 091C  1028 0037                      MOVE.B      0037(A0),D0
       | 0920  4880                           EXT.W       D0
       | 0922  48C0                           EXT.L       D0
       | 0924  2406                           MOVE.L      D6,D2
       | 0926  48C2                           EXT.L       D2
       | 0928  D480                           ADD.L       D0,D2
; 572:                                        w,h,(struct TagItem *)&tags);
       | 092A  2005                           MOVE.L      D5,D0
       | 092C  48C0                           EXT.L       D0
       | 092E  2604                           MOVE.L      D4,D3
       | 0930  48C3                           EXT.L       D3
       | 0932  2F40 002C                      MOVE.L      D0,002C(A7)
       | 0936  2001                           MOVE.L      D1,D0
       | 0938  2202                           MOVE.L      D2,D1
       | 093A  2068 0032                      MOVEA.L     0032(A0),A0
       | 093E  242F 002C                      MOVE.L      002C(A7),D2
       | 0942  43ED 001C                      LEA         001C(A5),A1
       | 0946  2C6B 001C                      MOVEA.L     001C(A3),A6
       | 094A  4EAE FF88                      JSR         FF88(A6)
; 573: }
       | 094E  4CDF 48FC                      MOVEM.L     (A7)+,D2-D7/A3/A6
       | 0952  4E5D                           UNLK        A5
       | 0954  4E75                           RTS
; 574: 
; 575: /*****************************************************************************/
; 576: 
; 577: void ClearGadPtrs(EdDataPtr ed)
; 578: {
       | 0956  48E7 0030                      MOVEM.L     A2-A3,-(A7)
       | 095A  2648                           MOVEA.L     A0,A3
; 579:     ULONG *gad;
; 580:     for (gad = (ULONG *) &ed->ed_MainHostName; gad <= (ULONG *)&ed->ed_DeviceDelete; gad++)
       | 095C  204B                           MOVEA.L     A3,A0
       | 095E  264B                           MOVEA.L     A3,A3
       | 0960  D6FC 0782                      ADDA.W      #0782,A3
       | 0964  45E8 080E                      LEA         080E(A0),A2
       | 0968  6002                           BRA.B       096C
; 581:     {
; 582:         *gad = NULL;
       | 096A  429B                           CLR.L       (A3)+
       | 096C  B7CA                           CMPA.L      A2,A3
       | 096E  63FA                           BLS.B       096A
; 583:     }
; 584: }
       | 0970  4CDF 0C00                      MOVEM.L     (A7)+,A2-A3
       | 0974  4E75                           RTS
; 585: 
; 586: /*****************************************************************************/
; 587: 
; 588: VOID RenderGadgets(EdDataPtr ed)
; 589: {
       | 0976  4E55 FFDC                      LINK        A5,#FFDC
       | 097A  48E7 0F32                      MOVEM.L     D4-D7/A2-A3/A6,-(A7)
       | 097E  2B48 FFEA                      MOVE.L      A0,FFEA(A5)
; 590: UWORD           i;
; 591: struct Node    *node;
; 592: BOOL able, checked;
; 593: ULONG num;
; 594: 
; 595:     switch(ed->ed_Panel)
       | 0982  246D FFEA                      MOVEA.L     FFEA(A5),A2
       | 0986  302A 0812                      MOVE.W      0812(A2),D0
       | 098A  4A40                           TST.W       D0
       | 098C  6716                           BEQ.B       09A4
       | 098E  5340                           SUBQ.W      #1,D0
       | 0990  6700 08EC                      BEQ.W       127E
       | 0994  5340                           SUBQ.W      #1,D0
       | 0996  6700 062A                      BEQ.W       0FC2
       | 099A  5340                           SUBQ.W      #1,D0
       | 099C  6700 01D2                      BEQ.W       0B70
       | 09A0  6000 0E4A                      BRA.W       17EC
; 596:     {
; 597:         case PANEL_MAIN:
; 598:                             if(!ed->ed_LastAdded)
       | 09A4  4AAA 01B8                      TST.L       01B8(A2)
       | 09A8  6638                           BNE.B       09E2
; 599:                             {
; 600:                                 ClearGadPtrs(ed);
       | 09AA  204A                           MOVEA.L     A2,A0
       | 09AC  61A8                           BSR.B       0956
; 601:                                 ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
       | 09AE  41EA 01C8                      LEA         01C8(A2),A0
       | 09B2  2C6A 001C                      MOVEA.L     001C(A2),A6
       | 09B6  4EAE FF8E                      JSR         FF8E(A6)
       | 09BA  2540 01B8                      MOVE.L      D0,01B8(A2)
; 602:                                 DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
       | 09BE  42A7                           CLR.L       -(A7)
       | 09C0  42A7                           CLR.L       -(A7)
       | 09C2  4879  0000 00DE-01             PEA         01.000000DE
       | 09C8  2F0A                           MOVE.L      A2,-(A7)
       | 09CA  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 603: //                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
; 604:                                 DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
       | 09CE  4297                           CLR.L       (A7)
       | 09D0  42A7                           CLR.L       -(A7)
       | 09D2  4879  0000 010E-01             PEA         01.0000010E
       | 09D8  2F0A                           MOVE.L      A2,-(A7)
       | 09DA  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 09DE  4FEF 001C                      LEA         001C(A7),A7
; 605:                             /*    DoPrefsGadget(ed,&EG[3],NULL,TAG_DONE);
; 606:                                 DoPrefsGadget(ed,&EG[4],NULL,TAG_DONE);
; 607:                                 DoPrefsGadget(ed,&EG[5],NULL,TAG_DONE); */
; 608:                             };
; 609: 
; 610:                             if((ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMSERVER) &&
       | 09E2  082A 0001 03E9                 BTST        #0001,03E9(A2)
       | 09E8  671C                           BEQ.B       0A06
; 611:                             	(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMS))
       | 09EA  082A 0000 03E9                 BTST        #0000,03E9(A2)
       | 09F0  6714                           BEQ.B       0A06
; 612:                             	ed->ed_PanelLabels[3] = GetString(&ed->ed_LocaleInfo,MSG_PANEL_REALMS);
       | 09F2  41EA 06CA                      LEA         06CA(A2),A0
       | 09F6  203C 0000 A414                 MOVE.L      #0000A414,D0
       | 09FC  4EBA  0000-XX.1                JSR         @GetString(PC)
       | 0A00  2540 076A                      MOVE.L      D0,076A(A2)
; 613: 			    else
       | 0A04  6006                           BRA.B       0A0C
; 614: 			    	ed->ed_PanelLabels[3] = NULL;
       | 0A06  91C8                           SUBA.L      A0,A0
       | 0A08  2548 076A                      MOVE.L      A0,076A(A2)
; 615: 
; 616:                             ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
; 617:                                                             GTCY_Active, (ULONG)ed->ed_Panel,
       | 0A0C  7000                           MOVEQ       #00,D0
       | 0A0E  302A 0812                      MOVE.W      0812(A2),D0
; 618:                                                             GTCY_Labels, ed->ed_PanelLabels,
       | 0A12  41EA 075E                      LEA         075E(A2),A0
; 619:                                                             TAG_DONE);
       | 0A16  42A7                           CLR.L       -(A7)
       | 0A18  2F08                           MOVE.L      A0,-(A7)
       | 0A1A  2F3C 8008 000E                 MOVE.L      #8008000E,-(A7)
       | 0A20  2F00                           MOVE.L      D0,-(A7)
       | 0A22  2F3C 8008 000F                 MOVE.L      #8008000F,-(A7)
       | 0A28  2F2A 079A                      MOVE.L      079A(A2),-(A7)
       | 0A2C  4879  0000 01B6-01             PEA         01.000001B6
       | 0A32  2F0A                           MOVE.L      A2,-(A7)
       | 0A34  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0A38  2540 079A                      MOVE.L      D0,079A(A2)
; 620: 
; 621:                             ed->ed_MainHostName = DoPrefsGadget(ed,&EG[6],ed->ed_MainHostName,
; 622:                                                                 GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostName,
       | 0A3C  41EA 0342                      LEA         0342(A2),A0
; 623:                                                                 GTST_MaxChars, 63,
; 624:                                                                 TAG_DONE);
       | 0A40  4297                           CLR.L       (A7)
       | 0A42  4878 003F                      PEA         003F
       | 0A46  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0A4C  2F08                           MOVE.L      A0,-(A7)
       | 0A4E  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0A54  2F2A 0782                      MOVE.L      0782(A2),-(A7)
       | 0A58  4879  0000 016E-01             PEA         01.0000016E
       | 0A5E  2F0A                           MOVE.L      A2,-(A7)
       | 0A60  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0A64  2540 0782                      MOVE.L      D0,0782(A2)
; 625: 
; 626:                             ed->ed_MainOwnerName = DoPrefsGadget(ed,&EG[50],ed->ed_MainOwnerName,
; 627:                                                                 GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_OwnerName,
       | 0A68  41EA 03C6                      LEA         03C6(A2),A0
; 628:                                                                 GTST_MaxChars, 63,
; 629:                                                                 TAG_DONE);
       | 0A6C  4297                           CLR.L       (A7)
       | 0A6E  4878 003F                      PEA         003F
       | 0A72  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0A78  2F08                           MOVE.L      A0,-(A7)
       | 0A7A  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0A80  2F2A 0796                      MOVE.L      0796(A2),-(A7)
       | 0A84  4879  0000 058E-01             PEA         01.0000058E
       | 0A8A  2F0A                           MOVE.L      A2,-(A7)
       | 0A8C  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0A90  4FEF 0058                      LEA         0058(A7),A7
       | 0A94  2540 0796                      MOVE.L      D0,0796(A2)
; 630: 
; 631:                             checked = (ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMS);
       | 0A98  7001                           MOVEQ       #01,D0
       | 0A9A  C0AA 03E6                      AND.L       03E6(A2),D0
; 632: 
; 633:                             ed->ed_MainUseRealms = DoPrefsGadget(ed,&EG[51],ed->ed_MainUseRealms,
; 634:                             					 GTCB_Checked, checked,
       | 0A9E  3F40 001C                      MOVE.W      D0,001C(A7)
       | 0AA2  48C0                           EXT.L       D0
; 635:                             					 TAG_DONE);
       | 0AA4  42A7                           CLR.L       -(A7)
       | 0AA6  2F00                           MOVE.L      D0,-(A7)
       | 0AA8  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 0AAE  2F2A 078E                      MOVE.L      078E(A2),-(A7)
       | 0AB2  4879  0000 05A6-01             PEA         01.000005A6
       | 0AB8  2F0A                           MOVE.L      A2,-(A7)
       | 0ABA  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0ABE  4FEF 0018                      LEA         0018(A7),A7
       | 0AC2  2540 078E                      MOVE.L      D0,078E(A2)
; 636: 
; 637:                             if(checked)
       | 0AC6  4A6F 001C                      TST.W       001C(A7)
       | 0ACA  6700 0D20                      BEQ.W       17EC
; 638:                             {
; 639:                                 ed->ed_MainRealmName = DoPrefsGadget(ed,&EG[7],ed->ed_MainRealmName,
; 640:                                                                      GTST_String,   ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName,
       | 0ACE  41EA 0382                      LEA         0382(A2),A0
; 641:                                                                      GTST_MaxChars, 63,
; 642:                                                                      TAG_DONE);
       | 0AD2  42A7                           CLR.L       -(A7)
       | 0AD4  4878 003F                      PEA         003F
       | 0AD8  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0ADE  2F08                           MOVE.L      A0,-(A7)
       | 0AE0  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0AE6  2F2A 0786                      MOVE.L      0786(A2),-(A7)
       | 0AEA  4879  0000 0186-01             PEA         01.00000186
       | 0AF0  2F0A                           MOVE.L      A2,-(A7)
       | 0AF2  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0AF6  2540 0786                      MOVE.L      D0,0786(A2)
; 643:                                 IPToStr(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr,ed->ed_Str1);
       | 0AFA  41EA 06D2                      LEA         06D2(A2),A0
       | 0AFE  2F48 003C                      MOVE.L      A0,003C(A7)
       | 0B02  202A 03C2                      MOVE.L      03C2(A2),D0
       | 0B06  6100 177E                      BSR.W       2286
; 644: 
; 645:                                 ed->ed_MainRealmAddress = DoPrefsGadget(ed,&EG[8],ed->ed_MainRealmAddress,
; 646:                                                                         GTST_String, ed->ed_Str1,
; 647:                                                                         GTST_MaxChars, 16,
; 648:                                                                         GTST_EditHook, &ed->ed_IPGadHook,
       | 0B0A  41EA 074A                      LEA         074A(A2),A0
; 649:                                                                         TAG_DONE);
       | 0B0E  4297                           CLR.L       (A7)
       | 0B10  2F08                           MOVE.L      A0,-(A7)
       | 0B12  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 0B18  4878 0010                      PEA         0010
       | 0B1C  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0B22  2F2F 004C                      MOVE.L      004C(A7),-(A7)
       | 0B26  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0B2C  2F2A 078A                      MOVE.L      078A(A2),-(A7)
       | 0B30  4879  0000 019E-01             PEA         01.0000019E
       | 0B36  2F0A                           MOVE.L      A2,-(A7)
       | 0B38  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0B3C  4FEF 0044                      LEA         0044(A7),A7
       | 0B40  2540 078A                      MOVE.L      D0,078A(A2)
; 650: 
; 651:                                 ed->ed_MainIsRealmServer = DoPrefsGadget(ed,&EG[52],ed->ed_MainIsRealmServer,
; 652:                                 					 GTCB_Checked, (ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags & NHPFLAGF_REALMSERVER),
       | 0B44  7002                           MOVEQ       #02,D0
       | 0B46  C0AA 03E6                      AND.L       03E6(A2),D0
; 653:                                 					 TAG_DONE);
       | 0B4A  42A7                           CLR.L       -(A7)
       | 0B4C  2F00                           MOVE.L      D0,-(A7)
       | 0B4E  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 0B54  2F2A 0792                      MOVE.L      0792(A2),-(A7)
       | 0B58  4879  0000 05BE-01             PEA         01.000005BE
       | 0B5E  2F0A                           MOVE.L      A2,-(A7)
       | 0B60  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0B64  4FEF 0018                      LEA         0018(A7),A7
       | 0B68  2540 0792                      MOVE.L      D0,0792(A2)
; 654:                      	    }
; 655:                             break;
       | 0B6C  6000 0C7E                      BRA.W       17EC
; 656: 
; 657:         case PANEL_REALMS:
; 658:                             if(!ed->ed_LastAdded)
       | 0B70  4AAA 01B8                      TST.L       01B8(A2)
       | 0B74  667A                           BNE.B       0BF0
; 659:                             {
; 660:                                 ClearGadPtrs(ed);
       | 0B76  204A                           MOVEA.L     A2,A0
       | 0B78  6100 FDDC                      BSR.W       0956
; 661:                                 ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
       | 0B7C  41EA 01C8                      LEA         01C8(A2),A0
       | 0B80  2C6A 001C                      MOVEA.L     001C(A2),A6
       | 0B84  4EAE FF8E                      JSR         FF8E(A6)
       | 0B88  2540 01B8                      MOVE.L      D0,01B8(A2)
; 662:                                 DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
       | 0B8C  42A7                           CLR.L       -(A7)
       | 0B8E  42A7                           CLR.L       -(A7)
       | 0B90  4879  0000 00DE-01             PEA         01.000000DE
       | 0B96  2F0A                           MOVE.L      A2,-(A7)
       | 0B98  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 663: //                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
; 664:                                 DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
       | 0B9C  4297                           CLR.L       (A7)
       | 0B9E  42A7                           CLR.L       -(A7)
       | 0BA0  4879  0000 010E-01             PEA         01.0000010E
       | 0BA6  2F0A                           MOVE.L      A2,-(A7)
       | 0BA8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 665:                                 DoPrefsGadget(ed,&EG[11],NULL,TAG_DONE);
       | 0BAC  4297                           CLR.L       (A7)
       | 0BAE  42A7                           CLR.L       -(A7)
       | 0BB0  4879  0000 01E6-01             PEA         01.000001E6
       | 0BB6  2F0A                           MOVE.L      A2,-(A7)
       | 0BB8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 666:                                 DoPrefsGadget(ed,&EG[13],NULL,TAG_DONE);
       | 0BBC  4297                           CLR.L       (A7)
       | 0BBE  42A7                           CLR.L       -(A7)
       | 0BC0  4879  0000 0216-01             PEA         01.00000216
       | 0BC6  2F0A                           MOVE.L      A2,-(A7)
       | 0BC8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 667:                                 DoPrefsGadget(ed,&EG[15],NULL,TAG_DONE);
       | 0BCC  4297                           CLR.L       (A7)
       | 0BCE  42A7                           CLR.L       -(A7)
       | 0BD0  4879  0000 0246-01             PEA         01.00000246
       | 0BD6  2F0A                           MOVE.L      A2,-(A7)
       | 0BD8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 668:                                 DoPrefsGadget(ed,&EG[16],NULL,TAG_DONE);
       | 0BDC  4297                           CLR.L       (A7)
       | 0BDE  42A7                           CLR.L       -(A7)
       | 0BE0  4879  0000 025E-01             PEA         01.0000025E
       | 0BE6  2F0A                           MOVE.L      A2,-(A7)
       | 0BE8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0BEC  4FEF 004C                      LEA         004C(A7),A7
; 669:                             };
; 670:                             ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
; 671:                                                             GTCY_Active, (ULONG)ed->ed_Panel,
       | 0BF0  7000                           MOVEQ       #00,D0
       | 0BF2  302A 0812                      MOVE.W      0812(A2),D0
; 672:                                                             GTCY_Labels, ed->ed_PanelLabels,
       | 0BF6  41EA 075E                      LEA         075E(A2),A0
; 673:                                                             TAG_DONE);
       | 0BFA  42A7                           CLR.L       -(A7)
       | 0BFC  2F08                           MOVE.L      A0,-(A7)
       | 0BFE  2F3C 8008 000E                 MOVE.L      #8008000E,-(A7)
       | 0C04  2F00                           MOVE.L      D0,-(A7)
       | 0C06  2F3C 8008 000F                 MOVE.L      #8008000F,-(A7)
       | 0C0C  2F2A 079A                      MOVE.L      079A(A2),-(A7)
       | 0C10  4879  0000 01B6-01             PEA         01.000001B6
       | 0C16  2F0A                           MOVE.L      A2,-(A7)
       | 0C18  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0C1C  2540 079A                      MOVE.L      D0,079A(A2)
; 674: 
; 675:                             ed->ed_LocalRealmDelete = DoPrefsGadget(ed,&EG[12],ed->ed_LocalRealmDelete,
; 676:                                                                     GA_Disabled, !(ed->ed_CurrentLocRealm),
       | 0C20  4AAA 073A                      TST.L       073A(A2)
       | 0C24  57C0                           SEQ         D0
       | 0C26  4400                           NEG.B       D0
       | 0C28  4880                           EXT.W       D0
       | 0C2A  48C0                           EXT.L       D0
; 677:                                                                     TAG_DONE);
       | 0C2C  4297                           CLR.L       (A7)
       | 0C2E  2F00                           MOVE.L      D0,-(A7)
       | 0C30  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0C36  2F2A 07BE                      MOVE.L      07BE(A2),-(A7)
       | 0C3A  4879  0000 01FE-01             PEA         01.000001FE
       | 0C40  2F0A                           MOVE.L      A2,-(A7)
       | 0C42  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0C46  2540 07BE                      MOVE.L      D0,07BE(A2)
; 678: 
; 679:                             ed->ed_RemoteRealmDelete = DoPrefsGadget(ed,&EG[14],ed->ed_RemoteRealmDelete,
; 680:                                                                      GA_Disabled, !(ed->ed_CurrentRemRealm),
       | 0C4A  4AAA 073E                      TST.L       073E(A2)
       | 0C4E  57C0                           SEQ         D0
       | 0C50  4400                           NEG.B       D0
       | 0C52  4880                           EXT.W       D0
       | 0C54  48C0                           EXT.L       D0
; 681:                                                                      TAG_DONE);
       | 0C56  4297                           CLR.L       (A7)
       | 0C58  2F00                           MOVE.L      D0,-(A7)
       | 0C5A  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0C60  2F2A 07CE                      MOVE.L      07CE(A2),-(A7)
       | 0C64  4879  0000 022E-01             PEA         01.0000022E
       | 0C6A  2F0A                           MOVE.L      A2,-(A7)
       | 0C6C  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0C70  4FEF 0048                      LEA         0048(A7),A7
       | 0C74  2540 07CE                      MOVE.L      D0,07CE(A2)
; 682: 
; 683:                             node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
       | 0C78  266A 0318                      MOVEA.L     0318(A2),A3
; 684:                             while(node->ln_Succ)
       | 0C7C  D4FC 06D2                      ADDA.W      #06D2,A2
       | 0C80  6024                           BRA.B       0CA6
; 685:                             {
; 686:                                 IPToStr(((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmAddr,ed->ed_Str1);
       | 0C82  202B 00CE                      MOVE.L      00CE(A3),D0
       | 0C86  204A                           MOVEA.L     A2,A0
       | 0C88  6100 15FC                      BSR.W       2286
; 687:                                 sprintf(node->ln_Name,"%-16.16s %-16.16s",((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmName,ed->ed_Str1);
       | 0C8C  41EB 008E                      LEA         008E(A3),A0
       | 0C90  2F0A                           MOVE.L      A2,-(A7)
       | 0C92  2F08                           MOVE.L      A0,-(A7)
       | 0C94  487A F3C2                      PEA         F3C2(PC)
       | 0C98  2F2B 000A                      MOVE.L      000A(A3),-(A7)
       | 0C9C  4EBA  0000-XX.1                JSR         _sprintf(PC)
       | 0CA0  4FEF 0010                      LEA         0010(A7),A7
; 688:                                 node = node->ln_Succ;
       | 0CA4  2653                           MOVEA.L     (A3),A3
; 689:                             }
       | 0CA6  4A93                           TST.L       (A3)
       | 0CA8  66D8                           BNE.B       0C82
; 690:                             node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
       | 0CAA  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 0CAE  2668 0326                      MOVEA.L     0326(A0),A3
; 691:                             while(node->ln_Succ)
       | 0CB2  6024                           BRA.B       0CD8
; 692:                             {
; 693:                                 IPToStr(((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmAddr,ed->ed_Str1);
       | 0CB4  202B 00CE                      MOVE.L      00CE(A3),D0
       | 0CB8  204A                           MOVEA.L     A2,A0
       | 0CBA  6100 15CA                      BSR.W       2286
; 694:                                 sprintf(node->ln_Name,"%-16.16s %-16.16s",((struct NIPCRealm *)node)->nz_Prefs.nzp_RealmName,ed->ed_Str1);
       | 0CBE  41EB 008E                      LEA         008E(A3),A0
       | 0CC2  2F0A                           MOVE.L      A2,-(A7)
       | 0CC4  2F08                           MOVE.L      A0,-(A7)
       | 0CC6  487A F390                      PEA         F390(PC)
       | 0CCA  2F2B 000A                      MOVE.L      000A(A3),-(A7)
       | 0CCE  4EBA  0000-XX.1                JSR         _sprintf(PC)
       | 0CD2  4FEF 0010                      LEA         0010(A7),A7
; 695:                                 node = node->ln_Succ;
       | 0CD6  2653                           MOVEA.L     (A3),A3
; 696:                             }
       | 0CD8  4A93                           TST.L       (A3)
       | 0CDA  66D8                           BNE.B       0CB4
; 697:                             if(ed->ed_CurrentLocRealm)
       | 0CDC  266D FFEA                      MOVEA.L     FFEA(A5),A3
       | 0CE0  41EB 06E6                      LEA         06E6(A3),A0
       | 0CE4  2F48 0024                      MOVE.L      A0,0024(A7)
       | 0CE8  4AAB 073A                      TST.L       073A(A3)
       | 0CEC  671C                           BEQ.B       0D0A
; 698:                             {
; 699:                                 strcpy(ed->ed_Str1,ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName);
       | 0CEE  226B 073A                      MOVEA.L     073A(A3),A1
       | 0CF2  D2FC 008E                      ADDA.W      #008E,A1
       | 0CF6  2C4A                           MOVEA.L     A2,A6
       | 0CF8  1CD9                           MOVE.B      (A1)+,(A6)+
       | 0CFA  66FC                           BNE.B       0CF8
; 700:                                 IPToStr(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmAddr,ed->ed_Str2);
       | 0CFC  226B 073A                      MOVEA.L     073A(A3),A1
       | 0D00  2029 00CE                      MOVE.L      00CE(A1),D0
       | 0D04  6100 1580                      BSR.W       2286
; 701:                             }
; 702:                             else
       | 0D08  600A                           BRA.B       0D14
; 703:                             {
; 704:                                 ed->ed_Str1[0]=0x00;
       | 0D0A  7000                           MOVEQ       #00,D0
       | 0D0C  1740 06D2                      MOVE.B      D0,06D2(A3)
; 705:                                 ed->ed_Str2[0]=0x00;
       | 0D10  1740 06E6                      MOVE.B      D0,06E6(A3)
; 706:                             }
; 707:                             ed->ed_LocalRealmName = DoPrefsGadget(ed,&EG[17],ed->ed_LocalRealmName,
; 708:                                                                   GTST_String,   ed->ed_Str1,
; 709:                                                                   GTST_MaxChars, 63,
; 710:                                                                   GA_Disabled, !(ed->ed_CurrentLocRealm),
       | 0D14  4AAB 073A                      TST.L       073A(A3)
       | 0D18  57C0                           SEQ         D0
       | 0D1A  4400                           NEG.B       D0
       | 0D1C  4880                           EXT.W       D0
       | 0D1E  48C0                           EXT.L       D0
; 711:                                                                   TAG_DONE);
       | 0D20  42A7                           CLR.L       -(A7)
       | 0D22  2F00                           MOVE.L      D0,-(A7)
       | 0D24  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0D2A  4878 003F                      PEA         003F
       | 0D2E  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0D34  2F0A                           MOVE.L      A2,-(A7)
       | 0D36  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0D3C  2F2B 07B6                      MOVE.L      07B6(A3),-(A7)
       | 0D40  4879  0000 0276-01             PEA         01.00000276
       | 0D46  2F0B                           MOVE.L      A3,-(A7)
       | 0D48  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0D4C  2740 07B6                      MOVE.L      D0,07B6(A3)
; 712: 
; 713:                             ed->ed_LocalRealmAddress = DoPrefsGadget(ed,&EG[18],ed->ed_LocalRealmAddress,
; 714:                                                                   GTST_String,   ed->ed_Str2,
; 715:                                                                   GTST_MaxChars, 63,
; 716:                                                                   GTST_EditHook, &ed->ed_IPGadHook,
       | 0D50  41EB 074A                      LEA         074A(A3),A0
; 717:                                                                   GA_Disabled, !(ed->ed_CurrentLocRealm),
       | 0D54  2F48 0048                      MOVE.L      A0,0048(A7)
       | 0D58  4AAB 073A                      TST.L       073A(A3)
       | 0D5C  57C0                           SEQ         D0
       | 0D5E  4400                           NEG.B       D0
       | 0D60  4880                           EXT.W       D0
       | 0D62  48C0                           EXT.L       D0
; 718:                                                                   TAG_DONE);
       | 0D64  4297                           CLR.L       (A7)
       | 0D66  2F00                           MOVE.L      D0,-(A7)
       | 0D68  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0D6E  2F08                           MOVE.L      A0,-(A7)
       | 0D70  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 0D76  4878 003F                      PEA         003F
       | 0D7A  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0D80  2F2F 0064                      MOVE.L      0064(A7),-(A7)
       | 0D84  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0D8A  2F2B 07BA                      MOVE.L      07BA(A3),-(A7)
       | 0D8E  4879  0000 028E-01             PEA         01.0000028E
       | 0D94  2F0B                           MOVE.L      A3,-(A7)
       | 0D96  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0D9A  4FEF 0054                      LEA         0054(A7),A7
       | 0D9E  2740 07BA                      MOVE.L      D0,07BA(A3)
; 719: 
; 720:                             if(ed->ed_CurrentRemRealm)
       | 0DA2  4AAB 073E                      TST.L       073E(A3)
       | 0DA6  6720                           BEQ.B       0DC8
; 721:                             {
; 722:                                 strcpy(ed->ed_Str1,ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName);
       | 0DA8  206B 073E                      MOVEA.L     073E(A3),A0
       | 0DAC  D0FC 008E                      ADDA.W      #008E,A0
       | 0DB0  224A                           MOVEA.L     A2,A1
       | 0DB2  12D8                           MOVE.B      (A0)+,(A1)+
       | 0DB4  66FC                           BNE.B       0DB2
; 723:                                 IPToStr(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmAddr,ed->ed_Str2);
       | 0DB6  206B 073E                      MOVEA.L     073E(A3),A0
       | 0DBA  2028 00CE                      MOVE.L      00CE(A0),D0
       | 0DBE  206F 0024                      MOVEA.L     0024(A7),A0
       | 0DC2  6100 14C2                      BSR.W       2286
; 724:                             }
; 725:                             else
       | 0DC6  600A                           BRA.B       0DD2
; 726:                             {
; 727:                                 ed->ed_Str1[0]=0x00;
       | 0DC8  7000                           MOVEQ       #00,D0
       | 0DCA  1740 06D2                      MOVE.B      D0,06D2(A3)
; 728:                                 ed->ed_Str2[0]=0x00;
       | 0DCE  1740 06E6                      MOVE.B      D0,06E6(A3)
; 729:                             }
; 730:                             ed->ed_RemoteRealmName = DoPrefsGadget(ed,&EG[19],ed->ed_RemoteRealmName,
; 731:                                                                    GTST_String,   ed->ed_Str1,
; 732:                                                                    GTST_MaxChars, 63,
; 733:                                                                    GA_Disabled, !(ed->ed_CurrentRemRealm),
       | 0DD2  4AAB 073E                      TST.L       073E(A3)
       | 0DD6  57C0                           SEQ         D0
       | 0DD8  4400                           NEG.B       D0
       | 0DDA  4880                           EXT.W       D0
       | 0DDC  48C0                           EXT.L       D0
; 734:                                                                    TAG_DONE);
       | 0DDE  42A7                           CLR.L       -(A7)
       | 0DE0  2F00                           MOVE.L      D0,-(A7)
       | 0DE2  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0DE8  4878 003F                      PEA         003F
       | 0DEC  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0DF2  2F0A                           MOVE.L      A2,-(A7)
       | 0DF4  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0DFA  2F2B 07C6                      MOVE.L      07C6(A3),-(A7)
       | 0DFE  4879  0000 02A6-01             PEA         01.000002A6
       | 0E04  2F0B                           MOVE.L      A3,-(A7)
       | 0E06  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0E0A  2740 07C6                      MOVE.L      D0,07C6(A3)
; 735: 
; 736:                             ed->ed_RemoteRealmAddress = DoPrefsGadget(ed,&EG[20],ed->ed_RemoteRealmAddress,
; 737:                                                                    GTST_String,   ed->ed_Str2,
; 738:                                                                    GTST_MaxChars, 64,
; 739:                                                                    GTST_EditHook, &ed->ed_IPGadHook,
; 740:                                                                    GA_Disabled, !(ed->ed_CurrentRemRealm),
       | 0E0E  4AAB 073E                      TST.L       073E(A3)
       | 0E12  57C0                           SEQ         D0
       | 0E14  4400                           NEG.B       D0
       | 0E16  4880                           EXT.W       D0
       | 0E18  48C0                           EXT.L       D0
; 741:                                                                    TAG_DONE);
       | 0E1A  4297                           CLR.L       (A7)
       | 0E1C  2F00                           MOVE.L      D0,-(A7)
       | 0E1E  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 0E24  2F2F 0050                      MOVE.L      0050(A7),-(A7)
       | 0E28  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 0E2E  4878 0040                      PEA         0040
       | 0E32  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 0E38  2F2F 0064                      MOVE.L      0064(A7),-(A7)
       | 0E3C  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 0E42  2F2B 07CA                      MOVE.L      07CA(A3),-(A7)
       | 0E46  4879  0000 02BE-01             PEA         01.000002BE
       | 0E4C  2F0B                           MOVE.L      A3,-(A7)
       | 0E4E  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0E52  4FEF 0054                      LEA         0054(A7),A7
       | 0E56  2740 07CA                      MOVE.L      D0,07CA(A3)
; 742: 
; 743: 			    if(ed->ed_V39)
       | 0E5A  41EB 0326                      LEA         0326(A3),A0
       | 0E5E  45EB 0318                      LEA         0318(A3),A2
       | 0E62  2F48 001C                      MOVE.L      A0,001C(A7)
       | 0E66  4AAB 077E                      TST.L       077E(A3)
       | 0E6A  6700 00E8                      BEQ.W       0F54
; 744: 			    {
; 745:                                 i = 0;
       | 0E6E  7C00                           MOVEQ       #00,D6
; 746:                                 node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
       | 0E70  266B 0318                      MOVEA.L     0318(A3),A3
; 747:                                 while(node->ln_Succ)
       | 0E74  600E                           BRA.B       0E84
; 748:                                 {
; 749:                                     if(node == (struct Node *)ed->ed_CurrentLocRealm)
       | 0E76  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 0E7A  B7E8 073A                      CMPA.L      073A(A0),A3
       | 0E7E  6708                           BEQ.B       0E88
; 750:                                         break;
; 751:                                     i++;
       | 0E80  5246                           ADDQ.W      #1,D6
; 752:                                     node = node->ln_Succ;
       | 0E82  2653                           MOVEA.L     (A3),A3
; 753:                                 }
       | 0E84  4A93                           TST.L       (A3)
       | 0E86  66EE                           BNE.B       0E76
; 754:                                 if(!node->ln_Succ)
       | 0E88  4A93                           TST.L       (A3)
       | 0E8A  6602                           BNE.B       0E8E
; 755:                                     i=~0;
       | 0E8C  7CFF                           MOVEQ       #FF,D6
; 756: 
; 757:                                 ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
; 758:                                                                       GTLV_Selected,    i,
       | 0E8E  7000                           MOVEQ       #00,D0
       | 0E90  3006                           MOVE.W      D6,D0
; 759:                                                                       GTLV_ShowSelected, 0,
; 760:                                                                       GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCLocalRealms,
; 761:                                                                       LAYOUTA_SPACING,  1,
; 762:                                                                       GTLV_ScrollWidth, 18,
; 763:                                                                       TAG_DONE);
       | 0E92  7200                           MOVEQ       #00,D1
       | 0E94  2F01                           MOVE.L      D1,-(A7)
       | 0E96  4878 0012                      PEA         0012
       | 0E9A  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 0EA0  4878 0001                      PEA         0001
       | 0EA4  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 0EAA  2F0A                           MOVE.L      A2,-(A7)
       | 0EAC  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 0EB2  2F01                           MOVE.L      D1,-(A7)
       | 0EB4  2F3C 8008 0035                 MOVE.L      #80080035,-(A7)
       | 0EBA  2F00                           MOVE.L      D0,-(A7)
       | 0EBC  2F3C 8008 0036                 MOVE.L      #80080036,-(A7)
       | 0EC2  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 0EC6  2F28 07B2                      MOVE.L      07B2(A0),-(A7)
       | 0ECA  4879  0000 02D6-01             PEA         01.000002D6
       | 0ED0  2F08                           MOVE.L      A0,-(A7)
       | 0ED2  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0ED6  4FEF 0038                      LEA         0038(A7),A7
       | 0EDA  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 0EDE  2140 07B2                      MOVE.L      D0,07B2(A0)
; 764:                                 i = 0;
       | 0EE2  7C00                           MOVEQ       #00,D6
; 765:                                 node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
       | 0EE4  2668 0326                      MOVEA.L     0326(A0),A3
; 766:                                 while(node->ln_Succ)
       | 0EE8  600A                           BRA.B       0EF4
; 767:                                 {
; 768:                                     if(node == (struct Node *)ed->ed_CurrentRemRealm)
       | 0EEA  B7EA 073E                      CMPA.L      073E(A2),A3
       | 0EEE  670C                           BEQ.B       0EFC
; 769:                                         break;
; 770:                                     i++;
       | 0EF0  5246                           ADDQ.W      #1,D6
; 771:                                     node = node->ln_Succ;
       | 0EF2  2653                           MOVEA.L     (A3),A3
; 772:                                 }
       | 0EF4  246D FFEA                      MOVEA.L     FFEA(A5),A2
       | 0EF8  4A93                           TST.L       (A3)
       | 0EFA  66EE                           BNE.B       0EEA
; 773:                                 if(!node->ln_Succ)
       | 0EFC  4A93                           TST.L       (A3)
       | 0EFE  6602                           BNE.B       0F02
; 774:                                     i=~0;
       | 0F00  7CFF                           MOVEQ       #FF,D6
; 775: 
; 776:                                 ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
; 777:                                                                        GTLV_Selected,    i,
       | 0F02  7000                           MOVEQ       #00,D0
       | 0F04  3006                           MOVE.W      D6,D0
; 778:                                                                        GTLV_ShowSelected,0,
; 779:                                                                        GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRemoteRealms,
; 780:                                                                        LAYOUTA_SPACING,  1,
; 781:                                                                        GTLV_ScrollWidth, 18,
; 782:                                                                        TAG_DONE);
       | 0F06  7200                           MOVEQ       #00,D1
       | 0F08  2F01                           MOVE.L      D1,-(A7)
       | 0F0A  4878 0012                      PEA         0012
       | 0F0E  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 0F14  4878 0001                      PEA         0001
       | 0F18  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 0F1E  2F2F 0030                      MOVE.L      0030(A7),-(A7)
       | 0F22  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 0F28  2F01                           MOVE.L      D1,-(A7)
       | 0F2A  2F3C 8008 0035                 MOVE.L      #80080035,-(A7)
       | 0F30  2F00                           MOVE.L      D0,-(A7)
       | 0F32  2F3C 8008 0036                 MOVE.L      #80080036,-(A7)
       | 0F38  2F2A 07C2                      MOVE.L      07C2(A2),-(A7)
       | 0F3C  4879  0000 02EE-01             PEA         01.000002EE
       | 0F42  2F0A                           MOVE.L      A2,-(A7)
       | 0F44  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0F48  4FEF 0038                      LEA         0038(A7),A7
       | 0F4C  2540 07C2                      MOVE.L      D0,07C2(A2)
; 783:                             }
; 784:                             else
       | 0F50  6000 089A                      BRA.W       17EC
; 785:                             {
; 786:                             	ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
; 787:                                                                       GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCLocalRealms,
; 788:                                                                       LAYOUTA_SPACING,  1,
; 789:                                                                       GTLV_ScrollWidth, 18,
; 790:                                                                       TAG_DONE);
       | 0F54  42A7                           CLR.L       -(A7)
       | 0F56  4878 0012                      PEA         0012
       | 0F5A  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 0F60  4878 0001                      PEA         0001
       | 0F64  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 0F6A  2F0A                           MOVE.L      A2,-(A7)
       | 0F6C  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 0F72  2F2B 07B2                      MOVE.L      07B2(A3),-(A7)
       | 0F76  4879  0000 02D6-01             PEA         01.000002D6
       | 0F7C  2F0B                           MOVE.L      A3,-(A7)
       | 0F7E  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0F82  2740 07B2                      MOVE.L      D0,07B2(A3)
; 791: 
; 792:                                 ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
; 793:                                                                        GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRemoteRealms,
; 794:                                                                        LAYOUTA_SPACING,  1,
; 795:                                                                        GTLV_ScrollWidth, 18,
; 796:                                                                        TAG_DONE);
       | 0F86  4297                           CLR.L       (A7)
       | 0F88  4878 0012                      PEA         0012
       | 0F8C  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 0F92  4878 0001                      PEA         0001
       | 0F96  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 0F9C  2F2F 0054                      MOVE.L      0054(A7),-(A7)
       | 0FA0  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 0FA6  2F2B 07C2                      MOVE.L      07C2(A3),-(A7)
       | 0FAA  4879  0000 02EE-01             PEA         01.000002EE
       | 0FB0  2F0B                           MOVE.L      A3,-(A7)
       | 0FB2  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 0FB6  4FEF 004C                      LEA         004C(A7),A7
       | 0FBA  2740 07C2                      MOVE.L      D0,07C2(A3)
; 797:                             }
; 798:                             break;
       | 0FBE  6000 082C                      BRA.W       17EC
; 799: 
; 800:         case PANEL_ROUTES:
; 801:                             if(!ed->ed_LastAdded)
       | 0FC2  4AAA 01B8                      TST.L       01B8(A2)
       | 0FC6  666A                           BNE.B       1032
; 802:                             {
; 803:                                 ClearGadPtrs(ed);
       | 0FC8  204A                           MOVEA.L     A2,A0
       | 0FCA  6100 F98A                      BSR.W       0956
; 804:                                 ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
       | 0FCE  41EA 01C8                      LEA         01C8(A2),A0
       | 0FD2  2C6A 001C                      MOVEA.L     001C(A2),A6
       | 0FD6  4EAE FF8E                      JSR         FF8E(A6)
       | 0FDA  2540 01B8                      MOVE.L      D0,01B8(A2)
; 805:                                 DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
       | 0FDE  42A7                           CLR.L       -(A7)
       | 0FE0  42A7                           CLR.L       -(A7)
       | 0FE2  4879  0000 00DE-01             PEA         01.000000DE
       | 0FE8  2F0A                           MOVE.L      A2,-(A7)
       | 0FEA  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 806: //                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
; 807:                                 DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
       | 0FEE  4297                           CLR.L       (A7)
       | 0FF0  42A7                           CLR.L       -(A7)
       | 0FF2  4879  0000 010E-01             PEA         01.0000010E
       | 0FF8  2F0A                           MOVE.L      A2,-(A7)
       | 0FFA  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 808:                                 DoPrefsGadget(ed,&EG[25],NULL,TAG_DONE);
       | 0FFE  4297                           CLR.L       (A7)
       | 1000  42A7                           CLR.L       -(A7)
       | 1002  4879  0000 0336-01             PEA         01.00000336
       | 1008  2F0A                           MOVE.L      A2,-(A7)
       | 100A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 809:                                /* DoPrefsGadget(ed,&EG[26],NULL,TAG_DONE); */
; 810:                                DoPrefsGadget(ed,&EG[29],NULL,TAG_DONE);
       | 100E  4297                           CLR.L       (A7)
       | 1010  42A7                           CLR.L       -(A7)
       | 1012  4879  0000 0396-01             PEA         01.00000396
       | 1018  2F0A                           MOVE.L      A2,-(A7)
       | 101A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 811:                                DoPrefsGadget(ed,&EG[53],NULL,TAG_DONE);
       | 101E  4297                           CLR.L       (A7)
       | 1020  42A7                           CLR.L       -(A7)
       | 1022  4879  0000 05D6-01             PEA         01.000005D6
       | 1028  2F0A                           MOVE.L      A2,-(A7)
       | 102A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 102E  4FEF 0040                      LEA         0040(A7),A7
; 812:                             };
; 813:                             ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
; 814:                                                             GTCY_Active, (ULONG)ed->ed_Panel,
       | 1032  7000                           MOVEQ       #00,D0
       | 1034  302A 0812                      MOVE.W      0812(A2),D0
; 815:                                                             GTCY_Labels, ed->ed_PanelLabels,
       | 1038  41EA 075E                      LEA         075E(A2),A0
; 816:                                                             TAG_DONE);
       | 103C  42A7                           CLR.L       -(A7)
       | 103E  2F08                           MOVE.L      A0,-(A7)
       | 1040  2F3C 8008 000E                 MOVE.L      #8008000E,-(A7)
       | 1046  2F00                           MOVE.L      D0,-(A7)
       | 1048  2F3C 8008 000F                 MOVE.L      #8008000F,-(A7)
       | 104E  2F2A 079A                      MOVE.L      079A(A2),-(A7)
       | 1052  4879  0000 01B6-01             PEA         01.000001B6
       | 1058  2F0A                           MOVE.L      A2,-(A7)
       | 105A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 105E  2540 079A                      MOVE.L      D0,079A(A2)
; 817: 
; 818:                             ed->ed_RouteDelete = DoPrefsGadget(ed,&EG[26],ed->ed_RouteDelete,
; 819:                                                                GA_Disabled, !(ed->ed_CurrentRoute),
       | 1062  4AAA 0742                      TST.L       0742(A2)
       | 1066  57C0                           SEQ         D0
       | 1068  4400                           NEG.B       D0
       | 106A  4880                           EXT.W       D0
       | 106C  48C0                           EXT.L       D0
; 820:                                                                TAG_DONE);
       | 106E  4297                           CLR.L       (A7)
       | 1070  2F00                           MOVE.L      D0,-(A7)
       | 1072  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1078  2F2A 07AE                      MOVE.L      07AE(A2),-(A7)
       | 107C  4879  0000 034E-01             PEA         01.0000034E
       | 1082  2F0A                           MOVE.L      A2,-(A7)
       | 1084  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1088  4FEF 0034                      LEA         0034(A7),A7
       | 108C  2540 07AE                      MOVE.L      D0,07AE(A2)
; 821:                             if(ed->ed_CurrentRoute)
       | 1090  266A 0742                      MOVEA.L     0742(A2),A3
       | 1094  D4FC 06D2                      ADDA.W      #06D2,A2
       | 1098  200B                           MOVE.L      A3,D0
       | 109A  671A                           BEQ.B       10B6
; 822:                             {
; 823:                                 if(ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork)
       | 109C  202B 004E                      MOVE.L      004E(A3),D0
       | 10A0  6708                           BEQ.B       10AA
; 824:                                     IPToStr(ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork,ed->ed_Str1);
       | 10A2  204A                           MOVEA.L     A2,A0
       | 10A4  6100 11E0                      BSR.W       2286
; 825:                                 else
       | 10A8  6014                           BRA.B       10BE
; 826:                                     strcpy(ed->ed_Str1,"default");
       | 10AA  41FA EFBE                      LEA         EFBE(PC),A0
       | 10AE  224A                           MOVEA.L     A2,A1
       | 10B0  12D8                           MOVE.B      (A0)+,(A1)+
       | 10B2  66FC                           BNE.B       10B0
; 827:                             }
; 828:                             else
       | 10B4  6008                           BRA.B       10BE
; 829:                                 ed->ed_Str1[0]=0x00;
       | 10B6  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 10BA  4228 06D2                      CLR.B       06D2(A0)
; 830: 
; 831:                             ed->ed_RouteDestination = DoPrefsGadget(ed,&EG[27],ed->ed_RouteDestination,
; 832:                                                                     GTST_String,   ed->ed_Str1,
       | 10BE  266D FFEA                      MOVEA.L     FFEA(A5),A3
; 833:                                                                     GTST_MaxChars, 16,
; 834:                                                                     GA_Disabled, !(ed->ed_CurrentRoute),
       | 10C2  4AAB 0742                      TST.L       0742(A3)
       | 10C6  57C0                           SEQ         D0
       | 10C8  4400                           NEG.B       D0
       | 10CA  4880                           EXT.W       D0
       | 10CC  48C0                           EXT.L       D0
; 835:                                                                     TAG_DONE);
       | 10CE  42A7                           CLR.L       -(A7)
       | 10D0  2F00                           MOVE.L      D0,-(A7)
       | 10D2  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 10D8  4878 0010                      PEA         0010
       | 10DC  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 10E2  2F0A                           MOVE.L      A2,-(A7)
       | 10E4  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 10EA  2F2B 079E                      MOVE.L      079E(A3),-(A7)
       | 10EE  4879  0000 0366-01             PEA         01.00000366
       | 10F4  2F0B                           MOVE.L      A3,-(A7)
       | 10F6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 10FA  4FEF 0028                      LEA         0028(A7),A7
       | 10FE  2740 079E                      MOVE.L      D0,079E(A3)
; 836:                             if(ed->ed_CurrentRoute)
       | 1102  206B 0742                      MOVEA.L     0742(A3),A0
       | 1106  2008                           MOVE.L      A0,D0
       | 1108  670C                           BEQ.B       1116
; 837:                                 IPToStr(ed->ed_CurrentRoute->nr_Prefs.nrp_Gateway,ed->ed_Str1);
       | 110A  2028 0052                      MOVE.L      0052(A0),D0
       | 110E  204A                           MOVEA.L     A2,A0
       | 1110  6100 1174                      BSR.W       2286
; 838:                             else
       | 1114  6004                           BRA.B       111A
; 839:                                 ed->ed_Str1[0]=0x00;
       | 1116  422B 06D2                      CLR.B       06D2(A3)
; 840: 
; 841:                             ed->ed_RouteGateway = DoPrefsGadget(ed,&EG[28],ed->ed_RouteGateway,
; 842:                                                                 GTST_String,   ed->ed_Str1,
; 843:                                                                 GTST_MaxChars, 16,
; 844:                                                                 GTST_EditHook, &ed->ed_IPGadHook,
       | 111A  41EB 074A                      LEA         074A(A3),A0
; 845:                                                                 GA_Disabled, !(ed->ed_CurrentRoute),
       | 111E  4AAB 0742                      TST.L       0742(A3)
       | 1122  57C0                           SEQ         D0
       | 1124  4400                           NEG.B       D0
       | 1126  4880                           EXT.W       D0
       | 1128  48C0                           EXT.L       D0
; 846:                                                                 TAG_DONE);
       | 112A  42A7                           CLR.L       -(A7)
       | 112C  2F00                           MOVE.L      D0,-(A7)
       | 112E  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1134  2F08                           MOVE.L      A0,-(A7)
       | 1136  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 113C  4878 0010                      PEA         0010
       | 1140  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 1146  2F0A                           MOVE.L      A2,-(A7)
       | 1148  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 114E  2F2B 07A2                      MOVE.L      07A2(A3),-(A7)
       | 1152  4879  0000 037E-01             PEA         01.0000037E
       | 1158  2F0B                           MOVE.L      A3,-(A7)
       | 115A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 115E  4FEF 0030                      LEA         0030(A7),A7
       | 1162  2740 07A2                      MOVE.L      D0,07A2(A3)
; 847: /*
; 848:                             if(ed->ed_CurrentRoute)
; 849:                                 num = ed->ed_CurrentRoute->nr_Prefs.nrp_Hops;
; 850:                             else
; 851:                                 num = 0;
; 852: 
; 853:                             ed->ed_RouteHops = DoPrefsGadget(ed,&EG[29],ed->ed_RouteHops,
; 854:                                                              GTIN_Number,   num,
; 855:                                                              GTIN_MaxChars, 3,
; 856:                                                              GA_Disabled, !(ed->ed_CurrentRoute),
; 857:                                                              TAG_DONE);
; 858: */
; 859: 
; 860:                             node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
       | 1166  266B 0334                      MOVEA.L     0334(A3),A3
; 861:                             while(node->ln_Succ)
       | 116A  604E                           BRA.B       11BA
; 862:                             {
; 863:                                 if(((struct NIPCRoute *)node)->nr_Prefs.nrp_DestNetwork == 0L)
       | 116C  4AAB 004E                      TST.L       004E(A3)
       | 1170  660C                           BNE.B       117E
; 864:                                     strcpy(ed->ed_Str1,"default");
       | 1172  41FA EEF6                      LEA         EEF6(PC),A0
       | 1176  224A                           MOVEA.L     A2,A1
       | 1178  12D8                           MOVE.B      (A0)+,(A1)+
       | 117A  66FC                           BNE.B       1178
; 865:                                 else
       | 117C  600A                           BRA.B       1188
; 866:                                     IPToStr(((struct NIPCRoute *)node)->nr_Prefs.nrp_DestNetwork,ed->ed_Str1);
       | 117E  202B 004E                      MOVE.L      004E(A3),D0
       | 1182  204A                           MOVEA.L     A2,A0
       | 1184  6100 1100                      BSR.W       2286
; 867:                                 IPToStr(((struct NIPCRoute *)node)->nr_Prefs.nrp_Gateway,ed->ed_Str2);
       | 1188  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 118C  D0FC 06E6                      ADDA.W      #06E6,A0
       | 1190  2F48 001C                      MOVE.L      A0,001C(A7)
       | 1194  202B 0052                      MOVE.L      0052(A3),D0
       | 1198  6100 10EC                      BSR.W       2286
; 868:                                 node->ln_Name = (STRPTR) &((struct NIPCRoute *)node)->nr_String;
       | 119C  41EB 000E                      LEA         000E(A3),A0
       | 11A0  2748 000A                      MOVE.L      A0,000A(A3)
; 869:                                 sprintf(node->ln_Name,"%-16.16s     %-16.16s",ed->ed_Str1,ed->ed_Str2);
       | 11A4  2F2F 001C                      MOVE.L      001C(A7),-(A7)
       | 11A8  2F0A                           MOVE.L      A2,-(A7)
       | 11AA  487A EEC6                      PEA         EEC6(PC)
       | 11AE  2F08                           MOVE.L      A0,-(A7)
       | 11B0  4EBA  0000-XX.1                JSR         _sprintf(PC)
       | 11B4  4FEF 0010                      LEA         0010(A7),A7
; 870:                                 node = node->ln_Succ;
       | 11B8  2653                           MOVEA.L     (A3),A3
; 871:                             }
       | 11BA  4A93                           TST.L       (A3)
       | 11BC  66AE                           BNE.B       116C
; 872: 
; 873: 			    if(ed->ed_V39)
       | 11BE  266D FFEA                      MOVEA.L     FFEA(A5),A3
       | 11C2  45EB 0334                      LEA         0334(A3),A2
       | 11C6  4AAB 077E                      TST.L       077E(A3)
       | 11CA  6778                           BEQ.B       1244
; 874: 			    {
; 875:                                 i = 0;
       | 11CC  7C00                           MOVEQ       #00,D6
; 876:                                 node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
       | 11CE  266B 0334                      MOVEA.L     0334(A3),A3
; 877:                                 while(node->ln_Succ)
       | 11D2  600E                           BRA.B       11E2
; 878:                                 {
; 879:                                     if(node == (struct Node *)ed->ed_CurrentRoute)
       | 11D4  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 11D8  B7E8 0742                      CMPA.L      0742(A0),A3
       | 11DC  6708                           BEQ.B       11E6
; 880:                                         break;
; 881:                                     i++;
       | 11DE  5246                           ADDQ.W      #1,D6
; 882:                                     node = node->ln_Succ;
       | 11E0  2653                           MOVEA.L     (A3),A3
; 883:                                 }
       | 11E2  4A93                           TST.L       (A3)
       | 11E4  66EE                           BNE.B       11D4
; 884:                                 if(!node->ln_Succ)
       | 11E6  4A93                           TST.L       (A3)
       | 11E8  6602                           BNE.B       11EC
; 885:                                     i=~0;
       | 11EA  7CFF                           MOVEQ       #FF,D6
; 886: 
; 887:                                 ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
; 888:                                                                  GTLV_Selected,    i,
       | 11EC  7000                           MOVEQ       #00,D0
       | 11EE  3006                           MOVE.W      D6,D0
; 889:                                                                  GTLV_ShowSelected, 0,
; 890:                                                                  GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRoutes,
; 891:                                                                  LAYOUTA_SPACING,  1,
; 892:                                                                  GTLV_ScrollWidth, 18,
; 893:                                                                  TAG_DONE);
       | 11F0  7200                           MOVEQ       #00,D1
       | 11F2  2F01                           MOVE.L      D1,-(A7)
       | 11F4  4878 0012                      PEA         0012
       | 11F8  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 11FE  4878 0001                      PEA         0001
       | 1202  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 1208  2F0A                           MOVE.L      A2,-(A7)
       | 120A  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1210  2F01                           MOVE.L      D1,-(A7)
       | 1212  2F3C 8008 0035                 MOVE.L      #80080035,-(A7)
       | 1218  2F00                           MOVE.L      D0,-(A7)
       | 121A  2F3C 8008 0036                 MOVE.L      #80080036,-(A7)
       | 1220  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 1224  2F28 07AA                      MOVE.L      07AA(A0),-(A7)
       | 1228  4879  0000 03AE-01             PEA         01.000003AE
       | 122E  2F08                           MOVE.L      A0,-(A7)
       | 1230  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1234  4FEF 0038                      LEA         0038(A7),A7
       | 1238  206D FFEA                      MOVEA.L     FFEA(A5),A0
       | 123C  2140 07AA                      MOVE.L      D0,07AA(A0)
; 894:                             }
; 895:                             else
       | 1240  6000 05AA                      BRA.W       17EC
; 896:                             {
; 897:                             	ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
; 898:                                                                  GTLV_Labels,      &ed->ed_PrefsWork.ep_NIPCRoutes,
; 899:                                                                  LAYOUTA_SPACING,  1,
; 900:                                                                  GTLV_ScrollWidth, 18,
; 901:                                                                  TAG_DONE);
       | 1244  42A7                           CLR.L       -(A7)
       | 1246  4878 0012                      PEA         0012
       | 124A  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 1250  4878 0001                      PEA         0001
       | 1254  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 125A  2F0A                           MOVE.L      A2,-(A7)
       | 125C  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1262  2F2B 07AA                      MOVE.L      07AA(A3),-(A7)
       | 1266  4879  0000 03AE-01             PEA         01.000003AE
       | 126C  2F0B                           MOVE.L      A3,-(A7)
       | 126E  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1272  4FEF 0028                      LEA         0028(A7),A7
       | 1276  2740 07AA                      MOVE.L      D0,07AA(A3)
; 902:                             }
; 903:                             break;
       | 127A  6000 0570                      BRA.W       17EC
; 904:         case PANEL_DEVICES:
; 905:                             if(!ed->ed_LastAdded)
       | 127E  4AAA 01B8                      TST.L       01B8(A2)
       | 1282  663A                           BNE.B       12BE
; 906:                             {
; 907:                                 ClearGadPtrs(ed);
       | 1284  204A                           MOVEA.L     A2,A0
       | 1286  6100 F6CE                      BSR.W       0956
; 908:                                 ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);
       | 128A  41EA 01C8                      LEA         01C8(A2),A0
       | 128E  2C6A 001C                      MOVEA.L     001C(A2),A6
       | 1292  4EAE FF8E                      JSR         FF8E(A6)
       | 1296  2540 01B8                      MOVE.L      D0,01B8(A2)
; 909:                                 DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
       | 129A  42A7                           CLR.L       -(A7)
       | 129C  42A7                           CLR.L       -(A7)
       | 129E  4879  0000 00DE-01             PEA         01.000000DE
       | 12A4  2F0A                           MOVE.L      A2,-(A7)
       | 12A6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
; 910: //                                DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
; 911:                                 DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
       | 12AA  4297                           CLR.L       (A7)
       | 12AC  42A7                           CLR.L       -(A7)
       | 12AE  4879  0000 010E-01             PEA         01.0000010E
       | 12B4  2F0A                           MOVE.L      A2,-(A7)
       | 12B6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 12BA  4FEF 001C                      LEA         001C(A7),A7
; 912:                               /*  DoPrefsGadget(ed,&EG[33],NULL,TAG_DONE); */
; 913:                               /*  DoPrefsGadget(ed,&EG[34],NULL,TAG_DONE); */
; 914:                               /*  DoPrefsGadget(ed,&EG[35],NULL,TAG_DONE); */
; 915:                             };
; 916: 
; 917:                             if(ed->ed_V39)
       | 12BE  47EA 030A                      LEA         030A(A2),A3
       | 12C2  204B                           MOVEA.L     A3,A0
       | 12C4  226A 0806                      MOVEA.L     0806(A2),A1
       | 12C8  2F48 0020                      MOVE.L      A0,0020(A7)
       | 12CC  2F49 001C                      MOVE.L      A1,001C(A7)
       | 12D0  4AAA 077E                      TST.L       077E(A2)
       | 12D4  6770                           BEQ.B       1346
; 918:                             {
; 919:                                 i = 0;
       | 12D6  7C00                           MOVEQ       #00,D6
; 920:                                 node = ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
       | 12D8  266A 030A                      MOVEA.L     030A(A2),A3
; 921:                                 while(node->ln_Succ)
       | 12DC  600A                           BRA.B       12E8
; 922:                                 {
; 923:                                     if(node == (struct Node *)ed->ed_CurrentDevice)
       | 12DE  B7EA 0736                      CMPA.L      0736(A2),A3
       | 12E2  670C                           BEQ.B       12F0
; 924:                                         break;
; 925:                                     i++;
       | 12E4  5246                           ADDQ.W      #1,D6
; 926:                                     node = node->ln_Succ;
       | 12E6  2653                           MOVEA.L     (A3),A3
; 927:                                 }
       | 12E8  246D FFEA                      MOVEA.L     FFEA(A5),A2
       | 12EC  4A93                           TST.L       (A3)
       | 12EE  66EE                           BNE.B       12DE
; 928:                                 if(!node->ln_Succ)
       | 12F0  4A93                           TST.L       (A3)
       | 12F2  6602                           BNE.B       12F6
; 929:                                     i=~0;
       | 12F4  7CFF                           MOVEQ       #FF,D6
; 930:                                 ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
; 931:                                                                   GTLV_Selected,     i,
       | 12F6  7000                           MOVEQ       #00,D0
       | 12F8  3006                           MOVE.W      D6,D0
; 932:                                                                   GTLV_ShowSelected, 0,
; 933:                                                                   GTLV_Labels,       &ed->ed_PrefsWork.ep_NIPCDevices,
; 934:                                                                   LAYOUTA_SPACING,   1,
; 935:                                                                   GTLV_ScrollWidth,  18,
; 936:                                                                   TAG_DONE);
       | 12FA  7200                           MOVEQ       #00,D1
       | 12FC  2F01                           MOVE.L      D1,-(A7)
       | 12FE  4878 0012                      PEA         0012
       | 1302  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 1308  4878 0001                      PEA         0001
       | 130C  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 1312  2F2F 0034                      MOVE.L      0034(A7),-(A7)
       | 1316  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 131C  2F01                           MOVE.L      D1,-(A7)
       | 131E  2F3C 8008 0035                 MOVE.L      #80080035,-(A7)
       | 1324  2F00                           MOVE.L      D0,-(A7)
       | 1326  2F3C 8008 0036                 MOVE.L      #80080036,-(A7)
       | 132C  2F2F 0048                      MOVE.L      0048(A7),-(A7)
       | 1330  4879  0000 055E-01             PEA         01.0000055E
       | 1336  2F0A                           MOVE.L      A2,-(A7)
       | 1338  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 133C  4FEF 0038                      LEA         0038(A7),A7
       | 1340  2540 0806                      MOVE.L      D0,0806(A2)
; 937:                             }
; 938:                             else
       | 1344  6036                           BRA.B       137C
; 939:                             {
; 940:                             	ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
; 941:                                                                   GTLV_Labels,       &ed->ed_PrefsWork.ep_NIPCDevices,
; 942:                                                                   LAYOUTA_SPACING,   1,
; 943:                                                                   GTLV_ScrollWidth,  18,
; 944:                                                                   TAG_DONE);
       | 1346  42A7                           CLR.L       -(A7)
       | 1348  4878 0012                      PEA         0012
       | 134C  2F3C 8008 0008                 MOVE.L      #80080008,-(A7)
       | 1352  4878 0001                      PEA         0001
       | 1356  2F3C 8003 8002                 MOVE.L      #80038002,-(A7)
       | 135C  2F0B                           MOVE.L      A3,-(A7)
       | 135E  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1364  2F2F 0038                      MOVE.L      0038(A7),-(A7)
       | 1368  4879  0000 055E-01             PEA         01.0000055E
       | 136E  2F0A                           MOVE.L      A2,-(A7)
       | 1370  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1374  4FEF 0028                      LEA         0028(A7),A7
       | 1378  2540 0806                      MOVE.L      D0,0806(A2)
; 945:                             }
; 946: 
; 947:                             EG[33].eg_TopEdge = EG[34].eg_TopEdge = ed->ed_DeviceList->Height + 24;
       | 137C  7218                           MOVEQ       #18,D1
       | 137E  2040                           MOVEA.L     D0,A0
       | 1380  D268 000A                      ADD.W       000A(A0),D1
       | 1384  33C1  0000 0414-01             MOVE.W      D1,01.00000414
       | 138A  33C1  0000 03FC-01             MOVE.W      D1,01.000003FC
; 948: 
; 949:                             ed->ed_PanelGad = DoPrefsGadget(ed,&EG[9],ed->ed_PanelGad,
; 950:                                                             GTCY_Active, (ULONG)ed->ed_Panel,
       | 1390  7000                           MOVEQ       #00,D0
       | 1392  302A 0812                      MOVE.W      0812(A2),D0
; 951:                                                             GTCY_Labels, ed->ed_PanelLabels,
       | 1396  41EA 075E                      LEA         075E(A2),A0
; 952:                                                             TAG_DONE);
       | 139A  42A7                           CLR.L       -(A7)
       | 139C  2F08                           MOVE.L      A0,-(A7)
       | 139E  2F3C 8008 000E                 MOVE.L      #8008000E,-(A7)
       | 13A4  2F00                           MOVE.L      D0,-(A7)
       | 13A6  2F3C 8008 000F                 MOVE.L      #8008000F,-(A7)
       | 13AC  2F2A 079A                      MOVE.L      079A(A2),-(A7)
       | 13B0  4879  0000 01B6-01             PEA         01.000001B6
       | 13B6  2F0A                           MOVE.L      A2,-(A7)
       | 13B8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 13BC  2540 079A                      MOVE.L      D0,079A(A2)
; 953: 
; 954: 			    ed->ed_DeviceAdd = DoPrefsGadget(ed,&EG[33],ed->ed_DeviceDelete,TAG_DONE);
       | 13C0  4297                           CLR.L       (A7)
       | 13C2  2F2A 080E                      MOVE.L      080E(A2),-(A7)
       | 13C6  4879  0000 03F6-01             PEA         01.000003F6
       | 13CC  2F0A                           MOVE.L      A2,-(A7)
       | 13CE  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 13D2  2540 080A                      MOVE.L      D0,080A(A2)
; 955: 
; 956:                             ed->ed_DeviceDelete = DoPrefsGadget(ed,&EG[34],ed->ed_DeviceDelete,
; 957:                                                                 GA_Disabled, !(ed->ed_CurrentDevice),
       | 13D6  4AAA 0736                      TST.L       0736(A2)
       | 13DA  57C0                           SEQ         D0
       | 13DC  4400                           NEG.B       D0
       | 13DE  4880                           EXT.W       D0
       | 13E0  48C0                           EXT.L       D0
; 958:                                                                 TAG_DONE);
       | 13E2  4297                           CLR.L       (A7)
       | 13E4  2F00                           MOVE.L      D0,-(A7)
       | 13E6  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 13EC  2F2A 080E                      MOVE.L      080E(A2),-(A7)
       | 13F0  4879  0000 040E-01             PEA         01.0000040E
       | 13F6  2F0A                           MOVE.L      A2,-(A7)
       | 13F8  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 13FC  4FEF 0040                      LEA         0040(A7),A7
       | 1400  2540 080E                      MOVE.L      D0,080E(A2)
; 959: 
; 960:                             if(ed->ed_CurrentDevice)
       | 1404  266A 0736                      MOVEA.L     0736(A2),A3
       | 1408  200B                           MOVE.L      A3,D0
       | 140A  670E                           BEQ.B       141A
; 961:                                 able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ONLINE);
       | 140C  7000                           MOVEQ       #00,D0
       | 140E  102B 005A                      MOVE.B      005A(A3),D0
       | 1412  7202                           MOVEQ       #02,D1
       | 1414  C081                           AND.L       D1,D0
       | 1416  2A00                           MOVE.L      D0,D5
; 962:                             else
       | 1418  6002                           BRA.B       141C
; 963:                                 able = 0;
       | 141A  7A00                           MOVEQ       #00,D5
; 964: 
; 965:                             ed->ed_DeviceStatus = DoPrefsGadget(ed,&EG[36],ed->ed_DeviceStatus,
; 966:                                                                 GTCY_Active, (ULONG)able,
       | 141C  2005                           MOVE.L      D5,D0
       | 141E  48C0                           EXT.L       D0
; 967:                                                                 GTCY_Labels, ed->ed_StatusLabels,
       | 1420  41EA 0772                      LEA         0772(A2),A0
; 968:                                                                 GA_Disabled, !(ed->ed_CurrentDevice),
       | 1424  4AAA 0736                      TST.L       0736(A2)
       | 1428  57C1                           SEQ         D1
       | 142A  4401                           NEG.B       D1
       | 142C  4881                           EXT.W       D1
       | 142E  48C1                           EXT.L       D1
; 969:                                                                 TAG_DONE);
       | 1430  42A7                           CLR.L       -(A7)
       | 1432  2F01                           MOVE.L      D1,-(A7)
       | 1434  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 143A  2F08                           MOVE.L      A0,-(A7)
       | 143C  2F3C 8008 000E                 MOVE.L      #8008000E,-(A7)
       | 1442  2F00                           MOVE.L      D0,-(A7)
       | 1444  2F3C 8008 000F                 MOVE.L      #8008000F,-(A7)
       | 144A  2F2A 07D2                      MOVE.L      07D2(A2),-(A7)
       | 144E  4879  0000 043E-01             PEA         01.0000043E
       | 1454  2F0A                           MOVE.L      A2,-(A7)
       | 1456  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 145A  4FEF 0028                      LEA         0028(A7),A7
       | 145E  2540 07D2                      MOVE.L      D0,07D2(A2)
; 970: 
; 971:                             if(ed->ed_CurrentDevice)
       | 1462  266A 0736                      MOVEA.L     0736(A2),A3
       | 1466  200B                           MOVE.L      A3,D0
       | 1468  6712                           BEQ.B       147C
; 972:                                 checked = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_SUBNET);
       | 146A  082B 0000 005A                 BTST        #0000,005A(A3)
       | 1470  57C0                           SEQ         D0
       | 1472  4400                           NEG.B       D0
       | 1474  4880                           EXT.W       D0
       | 1476  48C0                           EXT.L       D0
       | 1478  2800                           MOVE.L      D0,D4
; 973:                             else
       | 147A  6002                           BRA.B       147E
; 974:                                 checked = TRUE;
       | 147C  7801                           MOVEQ       #01,D4
; 975:                             ed->ed_DeviceSubnetAble = DoPrefsGadget(ed,&EG[37],ed->ed_DeviceSubnetAble,
; 976:                                                                     GTCB_Checked, checked,
       | 147E  2004                           MOVE.L      D4,D0
       | 1480  48C0                           EXT.L       D0
; 977:                                                                     GA_Disabled,   !(ed->ed_CurrentDevice),
       | 1482  4AAA 0736                      TST.L       0736(A2)
       | 1486  57C1                           SEQ         D1
       | 1488  4401                           NEG.B       D1
       | 148A  4881                           EXT.W       D1
       | 148C  48C1                           EXT.L       D1
; 978:                                                                     TAG_DONE);
       | 148E  42A7                           CLR.L       -(A7)
       | 1490  2F01                           MOVE.L      D1,-(A7)
       | 1492  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1498  2F00                           MOVE.L      D0,-(A7)
       | 149A  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 14A0  2F2A 07E2                      MOVE.L      07E2(A2),-(A7)
       | 14A4  4879  0000 0456-01             PEA         01.00000456
       | 14AA  2F0A                           MOVE.L      A2,-(A7)
       | 14AC  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 14B0  4FEF 0020                      LEA         0020(A7),A7
       | 14B4  2540 07E2                      MOVE.L      D0,07E2(A2)
; 979: 
; 980:                             if(ed->ed_CurrentDevice)
       | 14B8  266A 0736                      MOVEA.L     0736(A2),A3
       | 14BC  200B                           MOVE.L      A3,D0
       | 14BE  6712                           BEQ.B       14D2
; 981:                                 checked = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_HARDADDR);
       | 14C0  082B 0005 005A                 BTST        #0005,005A(A3)
       | 14C6  57C0                           SEQ         D0
       | 14C8  4400                           NEG.B       D0
       | 14CA  4880                           EXT.W       D0
       | 14CC  48C0                           EXT.L       D0
       | 14CE  2800                           MOVE.L      D0,D4
; 982:                             else
       | 14D0  6002                           BRA.B       14D4
; 983:                                 checked = TRUE;
       | 14D2  7801                           MOVEQ       #01,D4
; 984:                             ed->ed_DeviceAddressAble = DoPrefsGadget(ed,&EG[38],ed->ed_DeviceAddressAble,
; 985:                                                                      GTCB_Checked, checked,
       | 14D4  2004                           MOVE.L      D4,D0
       | 14D6  48C0                           EXT.L       D0
; 986:                                                                      GA_Disabled,   !(ed->ed_CurrentDevice),
       | 14D8  4AAA 0736                      TST.L       0736(A2)
       | 14DC  57C1                           SEQ         D1
       | 14DE  4401                           NEG.B       D1
       | 14E0  4881                           EXT.W       D1
       | 14E2  48C1                           EXT.L       D1
; 987:                                                                      TAG_DONE);
       | 14E4  42A7                           CLR.L       -(A7)
       | 14E6  2F01                           MOVE.L      D1,-(A7)
       | 14E8  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 14EE  2F00                           MOVE.L      D0,-(A7)
       | 14F0  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 14F6  2F2A 07E6                      MOVE.L      07E6(A2),-(A7)
       | 14FA  4879  0000 046E-01             PEA         01.0000046E
       | 1500  2F0A                           MOVE.L      A2,-(A7)
       | 1502  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1506  4FEF 0020                      LEA         0020(A7),A7
       | 150A  2540 07E6                      MOVE.L      D0,07E6(A2)
; 988: 
; 989:                             if(ed->ed_CurrentDevice)
       | 150E  266A 0736                      MOVEA.L     0736(A2),A3
       | 1512  200B                           MOVE.L      A3,D0
       | 1514  670E                           BEQ.B       1524
; 990:                                 checked = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_IPTYPE);
       | 1516  7000                           MOVEQ       #00,D0
       | 1518  102B 005A                      MOVE.B      005A(A3),D0
       | 151C  7208                           MOVEQ       #08,D1
       | 151E  C081                           AND.L       D1,D0
       | 1520  2800                           MOVE.L      D0,D4
; 991:                             else
       | 1522  6002                           BRA.B       1526
; 992:                                 checked = TRUE;
       | 1524  7801                           MOVEQ       #01,D4
; 993:                             ed->ed_DeviceIPTypeAble = DoPrefsGadget(ed,&EG[39],ed->ed_DeviceIPTypeAble,
; 994:                                                                     GTCB_Checked, checked,
       | 1526  2004                           MOVE.L      D4,D0
       | 1528  48C0                           EXT.L       D0
; 995:                                                                     GA_Disabled,   !(ed->ed_CurrentDevice),
       | 152A  4AAA 0736                      TST.L       0736(A2)
       | 152E  57C1                           SEQ         D1
       | 1530  4401                           NEG.B       D1
       | 1532  4881                           EXT.W       D1
       | 1534  48C1                           EXT.L       D1
; 996:                                                                     TAG_DONE);
       | 1536  42A7                           CLR.L       -(A7)
       | 1538  2F01                           MOVE.L      D1,-(A7)
       | 153A  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1540  2F00                           MOVE.L      D0,-(A7)
       | 1542  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 1548  2F2A 07EA                      MOVE.L      07EA(A2),-(A7)
       | 154C  4879  0000 0486-01             PEA         01.00000486
       | 1552  2F0A                           MOVE.L      A2,-(A7)
       | 1554  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1558  4FEF 0020                      LEA         0020(A7),A7
       | 155C  2540 07EA                      MOVE.L      D0,07EA(A2)
; 997: 
; 998:                             if(ed->ed_CurrentDevice)
       | 1560  266A 0736                      MOVEA.L     0736(A2),A3
       | 1564  200B                           MOVE.L      A3,D0
       | 1566  670E                           BEQ.B       1576
; 999:                                 checked = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ARPTYPE);
       | 1568  7000                           MOVEQ       #00,D0
       | 156A  102B 005A                      MOVE.B      005A(A3),D0
       | 156E  7210                           MOVEQ       #10,D1
       | 1570  C081                           AND.L       D1,D0
       | 1572  2800                           MOVE.L      D0,D4
;1000:                             else
       | 1574  6002                           BRA.B       1578
;1001:                                 checked = TRUE;
       | 1576  7801                           MOVEQ       #01,D4
;1002:                             ed->ed_DeviceARPTypeAble = DoPrefsGadget(ed,&EG[40],ed->ed_DeviceARPTypeAble,
;1003:                                                                      GTCB_Checked, checked,
       | 1578  2004                           MOVE.L      D4,D0
       | 157A  48C0                           EXT.L       D0
;1004:                                                                      GA_Disabled,   !(ed->ed_CurrentDevice),
       | 157C  4AAA 0736                      TST.L       0736(A2)
       | 1580  57C1                           SEQ         D1
       | 1582  4401                           NEG.B       D1
       | 1584  4881                           EXT.W       D1
       | 1586  48C1                           EXT.L       D1
;1005:                                                                      TAG_DONE);
       | 1588  42A7                           CLR.L       -(A7)
       | 158A  2F01                           MOVE.L      D1,-(A7)
       | 158C  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1592  2F00                           MOVE.L      D0,-(A7)
       | 1594  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 159A  2F2A 07EE                      MOVE.L      07EE(A2),-(A7)
       | 159E  4879  0000 049E-01             PEA         01.0000049E
       | 15A4  2F0A                           MOVE.L      A2,-(A7)
       | 15A6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 15AA  4FEF 0020                      LEA         0020(A7),A7
       | 15AE  2540 07EE                      MOVE.L      D0,07EE(A2)
;1006: 
;1007: /*
;1008:                             if(ed->ed_CurrentDevice)
;1009:                                 strcpy(ed->ed_Str1,ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName);
;1010:                             else
;1011:                                 ed->ed_Str1[0]=0x00;
;1012:                             ed->ed_DevicePathName = DoPrefsGadget(ed,&EG[41],ed->ed_DevicePathName,
;1013:                                                                   GTST_String,   ed->ed_Str1,
;1014:                                                                   GTST_MaxChars, 255,
;1015:                                                                   GA_Disabled,   !(ed->ed_CurrentDevice),
;1016:                                                                   TAG_DONE);
;1017: */
;1018: 
;1019:                             if(ed->ed_CurrentDevice)
       | 15B2  266A 0736                      MOVEA.L     0736(A2),A3
       | 15B6  200B                           MOVE.L      A3,D0
       | 15B8  6706                           BEQ.B       15C0
;1020:                                 num = ed->ed_CurrentDevice->nd_Prefs.ndp_Unit;
       | 15BA  2E2B 0016                      MOVE.L      0016(A3),D7
;1021:                             else
       | 15BE  6002                           BRA.B       15C2
;1022:                                 num = 0;
       | 15C0  7E00                           MOVEQ       #00,D7
;1023: 
;1024:                             ed->ed_DeviceUnit = DoPrefsGadget(ed,&EG[42],ed->ed_DeviceUnit,
;1025:                                                               GTIN_Number,   num,
;1026:                                                               GTIN_MaxChars, 10,
;1027:                                                               GA_Disabled, !(ed->ed_CurrentDevice),
       | 15C2  4AAA 0736                      TST.L       0736(A2)
       | 15C6  57C0                           SEQ         D0
       | 15C8  4400                           NEG.B       D0
       | 15CA  4880                           EXT.W       D0
       | 15CC  48C0                           EXT.L       D0
;1028:                                                               TAG_DONE);
       | 15CE  42A7                           CLR.L       -(A7)
       | 15D0  2F00                           MOVE.L      D0,-(A7)
       | 15D2  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 15D8  4878 000A                      PEA         000A
       | 15DC  2F3C 8008 0030                 MOVE.L      #80080030,-(A7)
       | 15E2  2F07                           MOVE.L      D7,-(A7)
       | 15E4  2F3C 8008 002F                 MOVE.L      #8008002F,-(A7)
       | 15EA  2F2A 07D6                      MOVE.L      07D6(A2),-(A7)
       | 15EE  4879  0000 04CE-01             PEA         01.000004CE
       | 15F4  2F0A                           MOVE.L      A2,-(A7)
       | 15F6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 15FA  4FEF 0028                      LEA         0028(A7),A7
       | 15FE  2540 07D6                      MOVE.L      D0,07D6(A2)
;1029: 
;1030:                             if(ed->ed_CurrentDevice)
       | 1602  206A 0736                      MOVEA.L     0736(A2),A0
       | 1606  47EA 06D2                      LEA         06D2(A2),A3
       | 160A  2008                           MOVE.L      A0,D0
       | 160C  670C                           BEQ.B       161A
;1031:                                 IPToStr(ed->ed_CurrentDevice->nd_Prefs.ndp_IPAddress,ed->ed_Str1);
       | 160E  2028 0052                      MOVE.L      0052(A0),D0
       | 1612  204B                           MOVEA.L     A3,A0
       | 1614  6100 0C70                      BSR.W       2286
;1032:                             else
       | 1618  6004                           BRA.B       161E
;1033:                                 ed->ed_Str1[0]=0x00;
       | 161A  422A 06D2                      CLR.B       06D2(A2)
;1034:                             ed->ed_DeviceIP = DoPrefsGadget(ed,&EG[43],ed->ed_DeviceIP,
;1035:                                                             GTST_String,   ed->ed_Str1,
;1036:                                                             GTST_MaxChars, 16,
;1037:                                                             GTST_EditHook, &ed->ed_IPGadHook,
       | 161E  41EA 074A                      LEA         074A(A2),A0
;1038:                                                             GA_Disabled,   !(ed->ed_CurrentDevice),
       | 1622  2F48 0020                      MOVE.L      A0,0020(A7)
       | 1626  4AAA 0736                      TST.L       0736(A2)
       | 162A  57C0                           SEQ         D0
       | 162C  4400                           NEG.B       D0
       | 162E  4880                           EXT.W       D0
       | 1630  48C0                           EXT.L       D0
;1039:                                                             TAG_DONE);
       | 1632  42A7                           CLR.L       -(A7)
       | 1634  2F00                           MOVE.L      D0,-(A7)
       | 1636  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 163C  2F08                           MOVE.L      A0,-(A7)
       | 163E  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 1644  4878 0010                      PEA         0010
       | 1648  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 164E  2F0B                           MOVE.L      A3,-(A7)
       | 1650  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 1656  2F2A 07F6                      MOVE.L      07F6(A2),-(A7)
       | 165A  4879  0000 04E6-01             PEA         01.000004E6
       | 1660  2F0A                           MOVE.L      A2,-(A7)
       | 1662  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1666  4FEF 0030                      LEA         0030(A7),A7
       | 166A  2540 07F6                      MOVE.L      D0,07F6(A2)
;1040: 
;1041:                             if(ed->ed_CurrentDevice)
       | 166E  206A 0736                      MOVEA.L     0736(A2),A0
       | 1672  2008                           MOVE.L      A0,D0
       | 1674  671C                           BEQ.B       1692
;1042:                             {
;1043:                                 able = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_SUBNET);
       | 1676  0828 0000 005A                 BTST        #0000,005A(A0)
       | 167C  57C0                           SEQ         D0
       | 167E  4400                           NEG.B       D0
       | 1680  4880                           EXT.W       D0
       | 1682  48C0                           EXT.L       D0
       | 1684  2A00                           MOVE.L      D0,D5
;1044:                                 IPToStr(ed->ed_CurrentDevice->nd_Prefs.ndp_IPSubnet,ed->ed_Str1);
       | 1686  2028 0056                      MOVE.L      0056(A0),D0
       | 168A  204B                           MOVEA.L     A3,A0
       | 168C  6100 0BF8                      BSR.W       2286
;1045:                             }
;1046:                             else
       | 1690  6006                           BRA.B       1698
;1047:                             {
;1048:                                 able = TRUE;
       | 1692  7A01                           MOVEQ       #01,D5
;1049:                                 ed->ed_Str1[0]=0x00;
       | 1694  422A 06D2                      CLR.B       06D2(A2)
;1050:                             }
;1051:                             ed->ed_DeviceMask = DoPrefsGadget(ed,&EG[44],ed->ed_DeviceMask,
;1052:                                                               GTST_String,   ed->ed_Str1,
;1053:                                                               GTST_MaxChars, 16,
;1054:                                                               GTST_EditHook, &ed->ed_IPGadHook,
;1055:                                                               GA_Disabled,   able,
       | 1698  2005                           MOVE.L      D5,D0
       | 169A  48C0                           EXT.L       D0
;1056:                                                               TAG_DONE);
       | 169C  42A7                           CLR.L       -(A7)
       | 169E  2F00                           MOVE.L      D0,-(A7)
       | 16A0  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 16A6  2F2F 002C                      MOVE.L      002C(A7),-(A7)
       | 16AA  2F3C 8008 0037                 MOVE.L      #80080037,-(A7)
       | 16B0  4878 0010                      PEA         0010
       | 16B4  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 16BA  2F0B                           MOVE.L      A3,-(A7)
       | 16BC  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 16C2  2F2A 07FA                      MOVE.L      07FA(A2),-(A7)
       | 16C6  4879  0000 04FE-01             PEA         01.000004FE
       | 16CC  2F0A                           MOVE.L      A2,-(A7)
       | 16CE  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 16D2  4FEF 0030                      LEA         0030(A7),A7
       | 16D6  2540 07FA                      MOVE.L      D0,07FA(A2)
;1057: 
;1058:                             if(ed->ed_CurrentDevice)
       | 16DA  206A 0736                      MOVEA.L     0736(A2),A0
       | 16DE  47EA 070E                      LEA         070E(A2),A3
       | 16E2  2008                           MOVE.L      A0,D0
       | 16E4  671C                           BEQ.B       1702
;1059:                             {
;1060:                                 able = !(ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_HARDADDR);
       | 16E6  0828 0005 005A                 BTST        #0005,005A(A0)
       | 16EC  57C0                           SEQ         D0
       | 16EE  4400                           NEG.B       D0
       | 16F0  4880                           EXT.W       D0
       | 16F2  48C0                           EXT.L       D0
       | 16F4  2A00                           MOVE.L      D0,D5
;1061:                                 strcpy(ed->ed_AddressStr,ed->ed_CurrentDevice->nd_Prefs.ndp_HardString);
       | 16F6  43E8 002A                      LEA         002A(A0),A1
       | 16FA  204B                           MOVEA.L     A3,A0
       | 16FC  10D9                           MOVE.B      (A1)+,(A0)+
       | 16FE  66FC                           BNE.B       16FC
;1062:                             }
;1063:                             else
       | 1700  6006                           BRA.B       1708
;1064:                             {
;1065:                                 able = TRUE;
       | 1702  7A01                           MOVEQ       #01,D5
;1066:                                 ed->ed_AddressStr[0]=0x00;
       | 1704  422A 070E                      CLR.B       070E(A2)
;1067:                             }
;1068:                             ed->ed_DeviceAddress = DoPrefsGadget(ed,&EG[45],ed->ed_DeviceAddress,
;1069:                                                                  GTST_String,   ed->ed_AddressStr,
;1070:                                                                  GTST_MaxChars, 40,
;1071:                                                                  GA_Disabled,   able,
       | 1708  2005                           MOVE.L      D5,D0
       | 170A  48C0                           EXT.L       D0
;1072:                                                                  TAG_DONE);
       | 170C  42A7                           CLR.L       -(A7)
       | 170E  2F00                           MOVE.L      D0,-(A7)
       | 1710  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 1716  4878 0028                      PEA         0028
       | 171A  2F3C 8008 002E                 MOVE.L      #8008002E,-(A7)
       | 1720  2F0B                           MOVE.L      A3,-(A7)
       | 1722  2F3C 8008 002D                 MOVE.L      #8008002D,-(A7)
       | 1728  2F2A 07FE                      MOVE.L      07FE(A2),-(A7)
       | 172C  4879  0000 0516-01             PEA         01.00000516
       | 1732  2F0A                           MOVE.L      A2,-(A7)
       | 1734  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1738  4FEF 0028                      LEA         0028(A7),A7
       | 173C  2540 07FE                      MOVE.L      D0,07FE(A2)
;1073: 
;1074:                             if(ed->ed_CurrentDevice)
       | 1740  266A 0736                      MOVEA.L     0736(A2),A3
       | 1744  200B                           MOVE.L      A3,D0
       | 1746  6712                           BEQ.B       175A
;1075:                             {
;1076:                                 num = ed->ed_CurrentDevice->nd_Prefs.ndp_IPType;
       | 1748  2E2B 000E                      MOVE.L      000E(A3),D7
;1077:                                 able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_IPTYPE);
       | 174C  7000                           MOVEQ       #00,D0
       | 174E  102B 005A                      MOVE.B      005A(A3),D0
       | 1752  7208                           MOVEQ       #08,D1
       | 1754  C081                           AND.L       D1,D0
       | 1756  2A00                           MOVE.L      D0,D5
;1078:                             }
;1079:                             else
       | 1758  6004                           BRA.B       175E
;1080:                             {
;1081:                                 num = 0;
       | 175A  7E00                           MOVEQ       #00,D7
;1082:                                 able = TRUE;
       | 175C  7A01                           MOVEQ       #01,D5
;1083:                             };
;1084:                             ed->ed_DeviceIPType = DoPrefsGadget(ed,&EG[46],ed->ed_DeviceIPType,
;1085:                                                                 GTIN_Number,   num,
;1086:                                                                 GTIN_MaxChars, 8,
;1087:                                                                 GA_Disabled,   able,
       | 175E  2005                           MOVE.L      D5,D0
       | 1760  48C0                           EXT.L       D0
;1088:                                                                 TAG_DONE);
       | 1762  42A7                           CLR.L       -(A7)
       | 1764  2F00                           MOVE.L      D0,-(A7)
       | 1766  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 176C  4878 0008                      PEA         0008
       | 1770  2F3C 8008 0030                 MOVE.L      #80080030,-(A7)
       | 1776  2F07                           MOVE.L      D7,-(A7)
       | 1778  2F3C 8008 002F                 MOVE.L      #8008002F,-(A7)
       | 177E  2F2A 07DA                      MOVE.L      07DA(A2),-(A7)
       | 1782  4879  0000 052E-01             PEA         01.0000052E
       | 1788  2F0A                           MOVE.L      A2,-(A7)
       | 178A  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 178E  4FEF 0028                      LEA         0028(A7),A7
       | 1792  2540 07DA                      MOVE.L      D0,07DA(A2)
;1089: 
;1090:                             if(ed->ed_CurrentDevice)
       | 1796  266A 0736                      MOVEA.L     0736(A2),A3
       | 179A  200B                           MOVE.L      A3,D0
       | 179C  6712                           BEQ.B       17B0
;1091:                             {
;1092:                                 num = ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType;
       | 179E  2E2B 0012                      MOVE.L      0012(A3),D7
;1093:                                 able = (ed->ed_CurrentDevice->nd_Prefs.ndp_Flags & NDPFLAGF_ARPTYPE);
       | 17A2  7000                           MOVEQ       #00,D0
       | 17A4  102B 005A                      MOVE.B      005A(A3),D0
       | 17A8  7210                           MOVEQ       #10,D1
       | 17AA  C081                           AND.L       D1,D0
       | 17AC  2A00                           MOVE.L      D0,D5
;1094:                             }
;1095:                             else
       | 17AE  6004                           BRA.B       17B4
;1096:                             {
;1097:                                 num = 0;
       | 17B0  7E00                           MOVEQ       #00,D7
;1098:                                 able = TRUE;
       | 17B2  7A01                           MOVEQ       #01,D5
;1099:                             }
;1100:                             ed->ed_DeviceARPType = DoPrefsGadget(ed,&EG[47],ed->ed_DeviceARPType,
;1101:                                                                  GTIN_Number,   num,
;1102:                                                                  GTIN_MaxChars, 8,
;1103:                                                                  GA_Disabled,   able,
       | 17B4  2005                           MOVE.L      D5,D0
       | 17B6  48C0                           EXT.L       D0
;1104:                                                                  TAG_DONE);
       | 17B8  42A7                           CLR.L       -(A7)
       | 17BA  2F00                           MOVE.L      D0,-(A7)
       | 17BC  2F3C 8003 000E                 MOVE.L      #8003000E,-(A7)
       | 17C2  4878 0008                      PEA         0008
       | 17C6  2F3C 8008 0030                 MOVE.L      #80080030,-(A7)
       | 17CC  2F07                           MOVE.L      D7,-(A7)
       | 17CE  2F3C 8008 002F                 MOVE.L      #8008002F,-(A7)
       | 17D4  2F2A 07DE                      MOVE.L      07DE(A2),-(A7)
       | 17D8  4879  0000 0546-01             PEA         01.00000546
       | 17DE  2F0A                           MOVE.L      A2,-(A7)
       | 17E0  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 17E4  4FEF 0028                      LEA         0028(A7),A7
       | 17E8  2540 07DE                      MOVE.L      D0,07DE(A2)
;1105: 
;1106:                             break;
;1107:     }
;1108: }
       | 17EC  4CDF 4CF0                      MOVEM.L     (A7)+,D4-D7/A2-A3/A6
       | 17F0  4E5D                           UNLK        A5
       | 17F2  4E75                           RTS
;1109: 
;1110: /*****************************************************************************/
;1111: 
;1112: VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
;1113: {
       | 17F4  4E55 FFE0                      LINK        A5,#FFE0
       | 17F8  48E7 2F32                      MOVEM.L     D2/D4-D7/A2-A3/A6,-(A7)
       | 17FC  2E00                           MOVE.L      D0,D7
       | 17FE  2B48 FFE6                      MOVE.L      A0,FFE6(A5)
;1114: BOOL refresh;
;1115: UWORD icode;
;1116: struct Node *node;
;1117: ULONG num;
;1118: STRPTR hardstring;
;1119: UBYTE *hardaddr, x, parselength;
;1120: struct Gadget *act = NULL;
       | 1802  97CB                           SUBA.L      A3,A3
;1121: struct Gadget *curr = NULL;
       | 1804  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 1808  42AD FFEA                      CLR.L       FFEA(A5)
;1122: struct NIPCDevice *old_dev;
;1123: struct NIPCRealm *old_realm;
;1124: struct NIPCRoute *old_route;
;1125: 
;1126: 
;1127:     refresh = FALSE;
       | 180C  7C00                           MOVEQ       #00,D6
;1128: 
;1129:     icode = ed->ed_CurrentMsg.Code;
       | 180E  246D FFE6                      MOVEA.L     FFE6(A5),A2
       | 1812  3A2A 01FC                      MOVE.W      01FC(A2),D5
;1130: 
;1131:     switch (ec)
       | 1816  2007                           MOVE.L      D7,D0
       | 1818  720B                           MOVEQ       #0B,D1
       | 181A  9081                           SUB.L       D1,D0
       | 181C  6D00 08B0                      BLT.W       20CE
       | 1820  0C80 0000 0031                 CMPI.L      #00000031,D0
       | 1826  6C00 08A6                      BGE.W       20CE
       | 182A  D040                           ADD.W       D0,D0
       | 182C  303B 0006                      MOVE.W      06(PC,D0.W),D0
       | 1830  4EFB 0004                      JMP         04(PC,D0.W)
       | 1834  0060 0076                      ORI.W       #0076,-(A0)
       | 1838  008C 0898 0898                 ORI.L       #08980898,A4
       | 183E  00C8 00DE                      CMP2.B      A0,D0
       | 1842  010C 074E                      MOVEP.W     074E(A4),D0
       | 1846  076A 0850                      BCHG.B      D3,0850(A2)
       | 184A  085A 053C                      BCHG        #003C,(A2)+
       | 184E  05E4                           BSET.B      D2,-(A4)
       | 1850  0628 066C 0574                 ADDI.B      #6C,0574(A0)
       | 1856  05AC 078C                      BCLR.B      D2,078C(A4)
       | 185A  07AE 0802                      BCLR.B      D3,0802(A6)
       | 185E  082A 07D8 0898                 BTST        #00D8,0898(A2)
       | 1864  0898 0898                      BCLR        #0098,(A0)+
       | 1868  0520                           BTST.B      D2,-(A0)
       | 186A  044C 04B4                      SUBI.W      #04B4,A4
       | 186E  04F4 03AE 0404                 CMP2.L      04(A4,D0.W*4),D0
       | 1874  0898 0898                      BCLR        #0098,(A0)+
       | 1878  0898 0254                      BCLR        #0054,(A0)+
       | 187C  01F2 021E                      BSET.B      D0,1E(A2,D0.W*2)
       | 1880  012C 01A8                      BTST.B      D0,01A8(A4)
       | 1884  0392                           BCLR.B      D1,(A2)
       | 1886  0334 035C                      BTST.B      D1,([A4])
       | 188A  0270 02EC 0898                 ANDI.W      #02EC,98(A0,D0.L)
       | 1890  0898 0898                      BCLR        #0098,(A0)+
       | 1894  00BA 41EA 0342 226A            ORI.L       #41EA0342,226A(PC)
;1132:     {
;1133:         /* Main Screen */
;1134:         case EC_MAIN_HOSTNAME   : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostName,((struct StringInfo *)ed->ed_MainHostName->SpecialInfo)->Buffer);
       | 189C  0782                           BCLR.L      D3,D2
       | 189E  2C69 0022                      MOVEA.L     0022(A1),A6
       | 18A2  2256                           MOVEA.L     (A6),A1
       | 18A4  10D9                           MOVE.B      (A1)+,(A0)+
       | 18A6  66FC                           BNE.B       18A4
;1135: //                                  act = ed->ed_MainRealmName;
;1136:                                   break;
       | 18A8  6000 0824                      BRA.W       20CE
;1137: 
;1138:         case EC_MAIN_REALMNAME  : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName,((struct StringInfo *)ed->ed_MainRealmName->SpecialInfo)->Buffer);
       | 18AC  41EA 0382                      LEA         0382(A2),A0
       | 18B0  226A 0786                      MOVEA.L     0786(A2),A1
       | 18B4  2C69 0022                      MOVEA.L     0022(A1),A6
       | 18B8  2256                           MOVEA.L     (A6),A1
       | 18BA  10D9                           MOVE.B      (A1)+,(A0)+
       | 18BC  66FC                           BNE.B       18BA
;1139: //                                  act = ed->ed_MainRealmAddress;
;1140:                                   break;
       | 18BE  6000 080E                      BRA.W       20CE
;1141: 
;1142:         case EC_MAIN_REALMADDR  : if(!StrtoIP(((struct StringInfo *)ed->ed_MainRealmAddress->SpecialInfo)->Buffer,&ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr))
       | 18C2  206A 078A                      MOVEA.L     078A(A2),A0
       | 18C6  2268 0022                      MOVEA.L     0022(A0),A1
       | 18CA  41EA 03C2                      LEA         03C2(A2),A0
       | 18CE  2F49 0020                      MOVE.L      A1,0020(A7)
       | 18D2  2248                           MOVEA.L     A0,A1
       | 18D4  206F 0020                      MOVEA.L     0020(A7),A0
       | 18D8  2050                           MOVEA.L     (A0),A0
       | 18DA  6100 0864                      BSR.W       2140
       | 18DE  4A40                           TST.W       D0
       | 18E0  6600 07EC                      BNE.W       20CE
;1143:                                   {
;1144:                                       act = ed->ed_MainRealmAddress;
       | 18E4  2B6A 078A FFEE                 MOVE.L      078A(A2),FFEE(A5)
;1145:                                       refresh = TRUE;
       | 18EA  7C01                           MOVEQ       #01,D6
;1146:                                   }
;1147:                                   break;
       | 18EC  6000 07E0                      BRA.W       20CE
;1148: 
;1149:         case EC_PANEL           : ed->ed_Panel = icode;
       | 18F0  3545 0812                      MOVE.W      D5,0812(A2)
;1150:                                   MakeNewDisplay(ed);
       | 18F4  204A                           MOVEA.L     A2,A0
       | 18F6  6100 08B4                      BSR.W       21AC
;1151:                                   break;
       | 18FA  6000 07D2                      BRA.W       20CE
;1152: 
;1153: 	case EC_MAIN_OWNERNAME  : strcpy(ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_OwnerName,((struct StringInfo *)ed->ed_MainOwnerName->SpecialInfo)->Buffer);
       | 18FE  41EA 03C6                      LEA         03C6(A2),A0
       | 1902  226A 0796                      MOVEA.L     0796(A2),A1
       | 1906  2C69 0022                      MOVEA.L     0022(A1),A6
       | 190A  2256                           MOVEA.L     (A6),A1
       | 190C  10D9                           MOVE.B      (A1)+,(A0)+
       | 190E  66FC                           BNE.B       190C
;1154: 				  break;
       | 1910  6000 07BC                      BRA.W       20CE
;1155: 
;1156: 	case EC_MAIN_REALMS	: if(ed->ed_MainUseRealms->Flags & GFLG_SELECTED)
       | 1914  206A 078E                      MOVEA.L     078E(A2),A0
       | 1918  0828 0007 000D                 BTST        #0007,000D(A0)
       | 191E  6708                           BEQ.B       1928
;1157:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMS;
       | 1920  08EA 0000 03E9                 BSET        #0000,03E9(A2)
;1158:                                   else
       | 1926  600E                           BRA.B       1936
;1159:                                   {
;1160:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags &= ~(NHPFLAGF_REALMS|NHPFLAGF_REALMSERVER);
       | 1928  70FC                           MOVEQ       #FC,D0
       | 192A  C1AA 03E6                      AND.L       D0,03E6(A2)
;1161:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmServAddr = 0L;
       | 192E  42AA 03C2                      CLR.L       03C2(A2)
;1162:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_RealmName[0]='\0';
       | 1932  422A 0382                      CLR.B       0382(A2)
;1163:                                   }
;1164:                                   MakeNewDisplay(ed);
       | 1936  204A                           MOVEA.L     A2,A0
       | 1938  6100 0872                      BSR.W       21AC
;1165:                                   refresh = TRUE;
       | 193C  7C01                           MOVEQ       #01,D6
;1166: 				  break;
       | 193E  6000 078E                      BRA.W       20CE
;1167: 
;1168: 	case EC_MAIN_REALMSERVER: if(ed->ed_MainIsRealmServer->Flags & GFLG_SELECTED)
       | 1942  206A 0792                      MOVEA.L     0792(A2),A0
       | 1946  0828 0007 000D                 BTST        #0007,000D(A0)
       | 194C  6708                           BEQ.B       1956
;1169:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags |= NHPFLAGF_REALMSERVER;
       | 194E  08EA 0001 03E9                 BSET        #0001,03E9(A2)
;1170:                                   else
       | 1954  6006                           BRA.B       195C
;1171:                                       ed->ed_PrefsWork.ep_NIPCHostPrefs.nhp_HostFlags &= ~NHPFLAGF_REALMSERVER;
       | 1956  08AA 0001 03E9                 BCLR        #0001,03E9(A2)
;1172:                                   refresh = TRUE;
       | 195C  7C01                           MOVEQ       #01,D6
;1173: 				  break;
       | 195E  6000 076E                      BRA.W       20CE
;1174: 
;1175:         /* Realms Screen */
;1176:         case EC_LOCREALM_ADD    : old_realm = ed->ed_CurrentLocRealm;
       | 1962  266A 073A                      MOVEA.L     073A(A2),A3
;1177:                                   if(ed->ed_CurrentLocRealm = AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
       | 1966  7069                           MOVEQ       #69,D0
       | 1968  D080                           ADD.L       D0,D0
       | 196A  223C 0001 0001                 MOVE.L      #00010001,D1
       | 1970  2C52                           MOVEA.L     (A2),A6
       | 1972  4EAE FF3A                      JSR         FF3A(A6)
       | 1976  2540 073A                      MOVE.L      D0,073A(A2)
       | 197A  4A80                           TST.L       D0
       | 197C  6758                           BEQ.B       19D6
;1178:                                   {
;1179:                                       ed->ed_CurrentLocRealm->nz_Node.ln_Name = (STRPTR) &ed->ed_CurrentLocRealm->nz_String;
       | 197E  2040                           MOVEA.L     D0,A0
       | 1980  43E8 000E                      LEA         000E(A0),A1
       | 1984  2149 000A                      MOVE.L      A1,000A(A0)
;1180:                                       strcpy(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName,"(new)");
       | 1988  206A 073A                      MOVEA.L     073A(A2),A0
       | 198C  D0FC 008E                      ADDA.W      #008E,A0
       | 1990  43FA E6C0                      LEA         E6C0(PC),A1
       | 1994  10D9                           MOVE.B      (A1)+,(A0)+
       | 1996  66FC                           BNE.B       1994
;1181: 				      ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
;1182:                                                                   GTLV_Labels,      ~0);
       | 1998  4878 FFFF                      PEA         FFFF
       | 199C  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 19A2  2F2A 07B2                      MOVE.L      07B2(A2),-(A7)
       | 19A6  4879  0000 02D6-01             PEA         01.000002D6
       | 19AC  2F0A                           MOVE.L      A2,-(A7)
       | 19AE  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 19B2  2540 07B2                      MOVE.L      D0,07B2(A2)
;1183:                                       InsertListSorted(ed,&ed->ed_PrefsWork.ep_NIPCLocalRealms,(struct Node *)ed->ed_CurrentLocRealm);
       | 19B6  41EA 0318                      LEA         0318(A2),A0
       | 19BA  2EAA 073A                      MOVE.L      073A(A2),(A7)
       | 19BE  2248                           MOVEA.L     A0,A1
       | 19C0  204A                           MOVEA.L     A2,A0
       | 19C2  6100 E800                      BSR.W       01C4
       | 19C6  4FEF 0014                      LEA         0014(A7),A7
;1184:                                       refresh = TRUE;
       | 19CA  7C01                           MOVEQ       #01,D6
;1185:                                       act = ed->ed_LocalRealmName;
       | 19CC  2B6A 07B6 FFEE                 MOVE.L      07B6(A2),FFEE(A5)
;1186:                                   }
;1187:                                   else
       | 19D2  6000 06FA                      BRA.W       20CE
;1188:                                       ed->ed_CurrentLocRealm = old_realm;
       | 19D6  254B 073A                      MOVE.L      A3,073A(A2)
;1189:                                   break;
       | 19DA  6000 06F2                      BRA.W       20CE
;1190: 
;1191:         case EC_LOCREALM_DELETE : if(ed->ed_CurrentLocRealm)
       | 19DE  4AAA 073A                      TST.L       073A(A2)
       | 19E2  6700 06EA                      BEQ.W       20CE
;1192:                                   {
;1193: 				      ed->ed_LocalRealmList = DoPrefsGadget(ed,&EG[21],ed->ed_LocalRealmList,
;1194:                                                                   GTLV_Labels,      ~0);
       | 19E6  4878 FFFF                      PEA         FFFF
       | 19EA  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 19F0  2F2A 07B2                      MOVE.L      07B2(A2),-(A7)
       | 19F4  4879  0000 02D6-01             PEA         01.000002D6
       | 19FA  2F0A                           MOVE.L      A2,-(A7)
       | 19FC  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1A00  4FEF 0014                      LEA         0014(A7),A7
       | 1A04  2540 07B2                      MOVE.L      D0,07B2(A2)
;1195:                                       Remove((struct Node *)ed->ed_CurrentLocRealm);
       | 1A08  226A 073A                      MOVEA.L     073A(A2),A1
       | 1A0C  2C52                           MOVEA.L     (A2),A6
       | 1A0E  4EAE FF04                      JSR         FF04(A6)
;1196:                                       FreeMem(ed->ed_CurrentLocRealm,sizeof(struct NIPCRealm));
       | 1A12  226A 073A                      MOVEA.L     073A(A2),A1
       | 1A16  7069                           MOVEQ       #69,D0
       | 1A18  D080                           ADD.L       D0,D0
       | 1A1A  4EAE FF2E                      JSR         FF2E(A6)
;1197:                                       ed->ed_CurrentLocRealm = NULL;
       | 1A1E  42AA 073A                      CLR.L       073A(A2)
;1198:                                       refresh = TRUE;
       | 1A22  7C01                           MOVEQ       #01,D6
;1199:                                   }
;1200:                                   break;
       | 1A24  6000 06A8                      BRA.W       20CE
;1201: 
;1202:         case EC_LOCREALM_NAME   : if(ed->ed_CurrentLocRealm)
       | 1A28  4AAA 073A                      TST.L       073A(A2)
       | 1A2C  6716                           BEQ.B       1A44
;1203:                                   {
;1204:                                       strcpy(ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmName,((struct StringInfo *)ed->ed_LocalRealmName->SpecialInfo)->Buffer);
       | 1A2E  206A 073A                      MOVEA.L     073A(A2),A0
       | 1A32  D0FC 008E                      ADDA.W      #008E,A0
       | 1A36  226A 07B6                      MOVEA.L     07B6(A2),A1
       | 1A3A  2C69 0022                      MOVEA.L     0022(A1),A6
       | 1A3E  2256                           MOVEA.L     (A6),A1
       | 1A40  10D9                           MOVE.B      (A1)+,(A0)+
       | 1A42  66FC                           BNE.B       1A40
;1205:                                   }
;1206:                                   if(!act)
       | 1A44  200B                           MOVE.L      A3,D0
       | 1A46  6606                           BNE.B       1A4E
;1207:                                       act = ed->ed_LocalRealmAddress;
       | 1A48  2B6A 07BA FFEE                 MOVE.L      07BA(A2),FFEE(A5)
;1208:                                   refresh = TRUE;
       | 1A4E  7C01                           MOVEQ       #01,D6
;1209:                                   break;
       | 1A50  6000 067C                      BRA.W       20CE
;1210: 
;1211:         case EC_LOCREALM_NET    : if(ed->ed_CurrentLocRealm)
       | 1A54  4AAA 073A                      TST.L       073A(A2)
       | 1A58  672A                           BEQ.B       1A84
;1212:                                   {
;1213:                                       if(!StrtoIP(((struct StringInfo *)ed->ed_LocalRealmAddress->SpecialInfo)->Buffer,&ed->ed_CurrentLocRealm->nz_Prefs.nzp_RealmAddr))
       | 1A5A  206A 07BA                      MOVEA.L     07BA(A2),A0
       | 1A5E  2268 0022                      MOVEA.L     0022(A0),A1
       | 1A62  206A 073A                      MOVEA.L     073A(A2),A0
       | 1A66  D0FC 00CE                      ADDA.W      #00CE,A0
       | 1A6A  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1A6E  2248                           MOVEA.L     A0,A1
       | 1A70  206F 0020                      MOVEA.L     0020(A7),A0
       | 1A74  2050                           MOVEA.L     (A0),A0
       | 1A76  6100 06C8                      BSR.W       2140
       | 1A7A  4A40                           TST.W       D0
       | 1A7C  6606                           BNE.B       1A84
;1214:                                       {
;1215:                                           act = ed->ed_LocalRealmAddress;
       | 1A7E  2B6A 07BA FFEE                 MOVE.L      07BA(A2),FFEE(A5)
;1216:                                       }
;1217:                                   }
;1218:                                   refresh = TRUE;
       | 1A84  7C01                           MOVEQ       #01,D6
;1219:                                   break;
       | 1A86  6000 0646                      BRA.W       20CE
;1220: 
;1221:         case EC_LOCREALM_LIST   : node = ed->ed_PrefsWork.ep_NIPCLocalRealms.lh_Head;
       | 1A8A  266A 0318                      MOVEA.L     0318(A2),A3
;1222:                                   while(icode--)
       | 1A8E  6002                           BRA.B       1A92
;1223:                                   {
;1224:                                       node = node->ln_Succ;
       | 1A90  2653                           MOVEA.L     (A3),A3
;1225:                                   }
       | 1A92  2005                           MOVE.L      D5,D0
       | 1A94  2A00                           MOVE.L      D0,D5
       | 1A96  5345                           SUBQ.W      #1,D5
       | 1A98  4A40                           TST.W       D0
       | 1A9A  66F4                           BNE.B       1A90
;1226:                                   ed->ed_CurrentLocRealm = (struct NIPCRealm *)node;
       | 1A9C  254B 073A                      MOVE.L      A3,073A(A2)
;1227:                                   refresh = TRUE;
       | 1AA0  7C01                           MOVEQ       #01,D6
;1228:                                   break;
       | 1AA2  6000 062A                      BRA.W       20CE
;1229: 
;1230:         case EC_REMREALM_ADD    : old_realm = ed->ed_CurrentRemRealm;
       | 1AA6  266A 073E                      MOVEA.L     073E(A2),A3
;1231:                                   if(ed->ed_CurrentRemRealm = AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
       | 1AAA  7069                           MOVEQ       #69,D0
       | 1AAC  D080                           ADD.L       D0,D0
       | 1AAE  223C 0001 0001                 MOVE.L      #00010001,D1
       | 1AB4  2C52                           MOVEA.L     (A2),A6
       | 1AB6  4EAE FF3A                      JSR         FF3A(A6)
       | 1ABA  2540 073E                      MOVE.L      D0,073E(A2)
       | 1ABE  4A80                           TST.L       D0
       | 1AC0  6758                           BEQ.B       1B1A
;1232:                                   {
;1233:                                       ed->ed_CurrentRemRealm->nz_Node.ln_Name = (STRPTR) &ed->ed_CurrentRemRealm->nz_String;
       | 1AC2  2040                           MOVEA.L     D0,A0
       | 1AC4  43E8 000E                      LEA         000E(A0),A1
       | 1AC8  2149 000A                      MOVE.L      A1,000A(A0)
;1234:                                       strcpy(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName,"(new)");
       | 1ACC  206A 073E                      MOVEA.L     073E(A2),A0
       | 1AD0  D0FC 008E                      ADDA.W      #008E,A0
       | 1AD4  43FA E57C                      LEA         E57C(PC),A1
       | 1AD8  10D9                           MOVE.B      (A1)+,(A0)+
       | 1ADA  66FC                           BNE.B       1AD8
;1235: 	                              ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
;1236:                                                                    GTLV_Labels,      ~0);
       | 1ADC  4878 FFFF                      PEA         FFFF
       | 1AE0  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1AE6  2F2A 07C2                      MOVE.L      07C2(A2),-(A7)
       | 1AEA  4879  0000 02EE-01             PEA         01.000002EE
       | 1AF0  2F0A                           MOVE.L      A2,-(A7)
       | 1AF2  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1AF6  2540 07C2                      MOVE.L      D0,07C2(A2)
;1237:                                       InsertListSorted(ed,&ed->ed_PrefsWork.ep_NIPCRemoteRealms,(struct Node *)ed->ed_CurrentRemRealm);
       | 1AFA  41EA 0326                      LEA         0326(A2),A0
       | 1AFE  2EAA 073E                      MOVE.L      073E(A2),(A7)
       | 1B02  2248                           MOVEA.L     A0,A1
       | 1B04  204A                           MOVEA.L     A2,A0
       | 1B06  6100 E6BC                      BSR.W       01C4
       | 1B0A  4FEF 0014                      LEA         0014(A7),A7
;1238:                                       refresh = TRUE;
       | 1B0E  7C01                           MOVEQ       #01,D6
;1239:                                       act = ed->ed_RemoteRealmName;
       | 1B10  2B6A 07C6 FFEE                 MOVE.L      07C6(A2),FFEE(A5)
;1240:                                   }
;1241:                                   else
       | 1B16  6000 05B6                      BRA.W       20CE
;1242:                                       ed->ed_CurrentRemRealm = old_realm;
       | 1B1A  254B 073E                      MOVE.L      A3,073E(A2)
;1243:                                   break;
       | 1B1E  6000 05AE                      BRA.W       20CE
;1244: 
;1245:         case EC_REMREALM_DELETE : if(ed->ed_CurrentRemRealm)
       | 1B22  4AAA 073E                      TST.L       073E(A2)
       | 1B26  673C                           BEQ.B       1B64
;1246:                                   {
;1247: 	                              ed->ed_RemoteRealmList = DoPrefsGadget(ed,&EG[22],ed->ed_RemoteRealmList,
;1248:                                                                    GTLV_Labels,      ~0);
       | 1B28  4878 FFFF                      PEA         FFFF
       | 1B2C  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1B32  2F2A 07C2                      MOVE.L      07C2(A2),-(A7)
       | 1B36  4879  0000 02EE-01             PEA         01.000002EE
       | 1B3C  2F0A                           MOVE.L      A2,-(A7)
       | 1B3E  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1B42  4FEF 0014                      LEA         0014(A7),A7
       | 1B46  2540 07C2                      MOVE.L      D0,07C2(A2)
;1249:                                       Remove((struct Node *)ed->ed_CurrentRemRealm);
       | 1B4A  226A 073E                      MOVEA.L     073E(A2),A1
       | 1B4E  2C52                           MOVEA.L     (A2),A6
       | 1B50  4EAE FF04                      JSR         FF04(A6)
;1250:                                       FreeMem(ed->ed_CurrentRemRealm,sizeof(struct NIPCRealm));
       | 1B54  226A 073E                      MOVEA.L     073E(A2),A1
       | 1B58  7069                           MOVEQ       #69,D0
       | 1B5A  D080                           ADD.L       D0,D0
       | 1B5C  4EAE FF2E                      JSR         FF2E(A6)
;1251:                                       ed->ed_CurrentRemRealm = NULL;
       | 1B60  42AA 073E                      CLR.L       073E(A2)
;1252:                                   }
;1253:                                   refresh = TRUE;
       | 1B64  7C01                           MOVEQ       #01,D6
;1254:                                   break;
       | 1B66  6000 0566                      BRA.W       20CE
;1255: 
;1256:         case EC_REMREALM_NAME   : if(ed->ed_CurrentRemRealm)
       | 1B6A  4AAA 073E                      TST.L       073E(A2)
       | 1B6E  6716                           BEQ.B       1B86
;1257:                                   {
;1258:                                       strcpy(ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmName,((struct StringInfo *)ed->ed_RemoteRealmName->SpecialInfo)->Buffer);
       | 1B70  206A 073E                      MOVEA.L     073E(A2),A0
       | 1B74  D0FC 008E                      ADDA.W      #008E,A0
       | 1B78  226A 07C6                      MOVEA.L     07C6(A2),A1
       | 1B7C  2C69 0022                      MOVEA.L     0022(A1),A6
       | 1B80  2256                           MOVEA.L     (A6),A1
       | 1B82  10D9                           MOVE.B      (A1)+,(A0)+
       | 1B84  66FC                           BNE.B       1B82
;1259:                                   }
;1260:                                   act = ed->ed_RemoteRealmAddress;
       | 1B86  2B6A 07CA FFEE                 MOVE.L      07CA(A2),FFEE(A5)
;1261:                                   refresh = TRUE;
       | 1B8C  7C01                           MOVEQ       #01,D6
;1262:                                   break;
       | 1B8E  6000 053E                      BRA.W       20CE
;1263: 
;1264:         case EC_REMREALM_NET    : if(ed->ed_CurrentRemRealm)
       | 1B92  4AAA 073E                      TST.L       073E(A2)
       | 1B96  672A                           BEQ.B       1BC2
;1265:                                   {
;1266:                                       if(!StrtoIP(((struct StringInfo *)ed->ed_RemoteRealmAddress->SpecialInfo)->Buffer,&ed->ed_CurrentRemRealm->nz_Prefs.nzp_RealmAddr))
       | 1B98  206A 07CA                      MOVEA.L     07CA(A2),A0
       | 1B9C  2268 0022                      MOVEA.L     0022(A0),A1
       | 1BA0  206A 073E                      MOVEA.L     073E(A2),A0
       | 1BA4  D0FC 00CE                      ADDA.W      #00CE,A0
       | 1BA8  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1BAC  2248                           MOVEA.L     A0,A1
       | 1BAE  206F 0020                      MOVEA.L     0020(A7),A0
       | 1BB2  2050                           MOVEA.L     (A0),A0
       | 1BB4  6100 058A                      BSR.W       2140
       | 1BB8  4A40                           TST.W       D0
       | 1BBA  6606                           BNE.B       1BC2
;1267:                                       {
;1268:                                           act = ed->ed_RemoteRealmAddress;
       | 1BBC  2B6A 07CA FFEE                 MOVE.L      07CA(A2),FFEE(A5)
;1269:                                       }
;1270:                                   }
;1271:                                   refresh = TRUE;
       | 1BC2  7C01                           MOVEQ       #01,D6
;1272:                                   break;
       | 1BC4  6000 0508                      BRA.W       20CE
;1273: 
;1274:         case EC_REMREALM_LIST   : node = ed->ed_PrefsWork.ep_NIPCRemoteRealms.lh_Head;
       | 1BC8  266A 0326                      MOVEA.L     0326(A2),A3
;1275:                                   while(icode--)
       | 1BCC  6002                           BRA.B       1BD0
;1276:                                   {
;1277:                                       node = node->ln_Succ;
       | 1BCE  2653                           MOVEA.L     (A3),A3
;1278:                                   }
       | 1BD0  2005                           MOVE.L      D5,D0
       | 1BD2  2A00                           MOVE.L      D0,D5
       | 1BD4  5345                           SUBQ.W      #1,D5
       | 1BD6  4A40                           TST.W       D0
       | 1BD8  66F4                           BNE.B       1BCE
;1279:                                   ed->ed_CurrentRemRealm = (struct NIPCRealm *)node;
       | 1BDA  254B 073E                      MOVE.L      A3,073E(A2)
;1280:                                   refresh = TRUE;
       | 1BDE  7C01                           MOVEQ       #01,D6
;1281:                                   break;
       | 1BE0  6000 04EC                      BRA.W       20CE
;1282: 
;1283:         /* Routes Screen */
;1284: 
;1285:         case EC_ROUTE_ADD       : old_route = ed->ed_CurrentRoute;
       | 1BE4  266A 0742                      MOVEA.L     0742(A2),A3
;1286:                                   if(ed->ed_CurrentRoute = AllocMem(sizeof(struct NIPCRoute),MEMF_PUBLIC|MEMF_CLEAR))
       | 1BE8  7058                           MOVEQ       #58,D0
       | 1BEA  223C 0001 0001                 MOVE.L      #00010001,D1
       | 1BF0  2C52                           MOVEA.L     (A2),A6
       | 1BF2  4EAE FF3A                      JSR         FF3A(A6)
       | 1BF6  2540 0742                      MOVE.L      D0,0742(A2)
       | 1BFA  4A80                           TST.L       D0
       | 1BFC  6732                           BEQ.B       1C30
;1287:                                   {
;1288: 	                              ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
;1289:                                                              GTLV_Labels,	~0);
       | 1BFE  4878 FFFF                      PEA         FFFF
       | 1C02  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1C08  2F2A 07AA                      MOVE.L      07AA(A2),-(A7)
       | 1C0C  4879  0000 03AE-01             PEA         01.000003AE
       | 1C12  2F0A                           MOVE.L      A2,-(A7)
       | 1C14  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1C18  4FEF 0014                      LEA         0014(A7),A7
       | 1C1C  2540 07AA                      MOVE.L      D0,07AA(A2)
;1290:                                       AddTail(&ed->ed_PrefsWork.ep_NIPCRoutes,(struct Node *)ed->ed_CurrentRoute);
       | 1C20  41EA 0334                      LEA         0334(A2),A0
       | 1C24  226A 0742                      MOVEA.L     0742(A2),A1
       | 1C28  2C52                           MOVEA.L     (A2),A6
       | 1C2A  4EAE FF0A                      JSR         FF0A(A6)
;1291:                                   }
;1292:                                   else
       | 1C2E  6004                           BRA.B       1C34
;1293:                                       ed->ed_CurrentRoute = old_route;
       | 1C30  254B 0742                      MOVE.L      A3,0742(A2)
;1294:                                   refresh = TRUE;
       | 1C34  7C01                           MOVEQ       #01,D6
;1295:                                   break;
       | 1C36  6000 0496                      BRA.W       20CE
;1296: 
;1297:         case EC_ROUTE_DELETE    : if(ed->ed_CurrentRoute)
       | 1C3A  4AAA 0742                      TST.L       0742(A2)
       | 1C3E  6700 048E                      BEQ.W       20CE
;1298:                                   {
;1299: 	                              ed->ed_RouteList = DoPrefsGadget(ed,&EG[30],ed->ed_RouteList,
;1300:                                                              GTLV_Labels,	~0);
       | 1C42  4878 FFFF                      PEA         FFFF
       | 1C46  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 1C4C  2F2A 07AA                      MOVE.L      07AA(A2),-(A7)
       | 1C50  4879  0000 03AE-01             PEA         01.000003AE
       | 1C56  2F0A                           MOVE.L      A2,-(A7)
       | 1C58  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 1C5C  4FEF 0014                      LEA         0014(A7),A7
       | 1C60  2540 07AA                      MOVE.L      D0,07AA(A2)
;1301:                                       Remove((struct Node *)ed->ed_CurrentRoute);
       | 1C64  226A 0742                      MOVEA.L     0742(A2),A1
       | 1C68  2C52                           MOVEA.L     (A2),A6
       | 1C6A  4EAE FF04                      JSR         FF04(A6)
;1302:                                       FreeMem(ed->ed_CurrentRoute,sizeof(struct NIPCRoute));
       | 1C6E  226A 0742                      MOVEA.L     0742(A2),A1
       | 1C72  7058                           MOVEQ       #58,D0
       | 1C74  4EAE FF2E                      JSR         FF2E(A6)
;1303:                                       ed->ed_CurrentRoute = NULL;
       | 1C78  42AA 0742                      CLR.L       0742(A2)
;1304:                                       refresh = TRUE;
       | 1C7C  7C01                           MOVEQ       #01,D6
;1305:                                   }
;1306:                                   break;
       | 1C7E  6000 044E                      BRA.W       20CE
;1307: 
;1308:         case EC_ROUTE_DEST      : if(ed->ed_CurrentRoute)
       | 1C82  4AAA 0742                      TST.L       0742(A2)
       | 1C86  6750                           BEQ.B       1CD8
;1309:                                   {
;1310:                                       if(!Stricmp(((struct StringInfo *)ed->ed_RouteDestination->SpecialInfo)->Buffer,"default"))
       | 1C88  206A 079E                      MOVEA.L     079E(A2),A0
       | 1C8C  2268 0022                      MOVEA.L     0022(A0),A1
       | 1C90  2051                           MOVEA.L     (A1),A0
       | 1C92  43FA E3D6                      LEA         E3D6(PC),A1
       | 1C96  2C6A 0020                      MOVEA.L     0020(A2),A6
       | 1C9A  4EAE FF5E                      JSR         FF5E(A6)
       | 1C9E  266A 0742                      MOVEA.L     0742(A2),A3
       | 1CA2  4A80                           TST.L       D0
       | 1CA4  660A                           BNE.B       1CB0
;1311:                                           ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork = 0L;
       | 1CA6  42AB 004E                      CLR.L       004E(A3)
;1312:                                       else if(!StrtoIP(((struct StringInfo *)ed->ed_RouteDestination->SpecialInfo)->Buffer,&ed->ed_CurrentRoute->nr_Prefs.nrp_DestNetwork))
       | 1CAA  266D FFEE                      MOVEA.L     FFEE(A5),A3
       | 1CAE  6028                           BRA.B       1CD8
       | 1CB0  206A 079E                      MOVEA.L     079E(A2),A0
       | 1CB4  2268 0022                      MOVEA.L     0022(A0),A1
       | 1CB8  41EB 004E                      LEA         004E(A3),A0
       | 1CBC  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1CC0  2248                           MOVEA.L     A0,A1
       | 1CC2  206F 0020                      MOVEA.L     0020(A7),A0
       | 1CC6  2050                           MOVEA.L     (A0),A0
       | 1CC8  6100 0476                      BSR.W       2140
       | 1CCC  266D FFEE                      MOVEA.L     FFEE(A5),A3
       | 1CD0  4A40                           TST.W       D0
       | 1CD2  6604                           BNE.B       1CD8
;1313:                                       {
;1314:                                           act = ed->ed_RouteDestination;
       | 1CD4  266A 079E                      MOVEA.L     079E(A2),A3
;1315:                                       }
;1316:                                   }
;1317:                                   if(!act)
       | 1CD8  200B                           MOVE.L      A3,D0
       | 1CDA  6604                           BNE.B       1CE0
;1318:                                       act = ed->ed_RouteGateway;
       | 1CDC  266A 07A2                      MOVEA.L     07A2(A2),A3
;1319: 
;1320:                                   refresh = TRUE;
       | 1CE0  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 1CE4  7C01                           MOVEQ       #01,D6
;1321:                                   break;
       | 1CE6  6000 03E6                      BRA.W       20CE
;1322: 
;1323:         case EC_ROUTE_GATEWAY   : if(ed->ed_CurrentRoute)
       | 1CEA  4AAA 0742                      TST.L       0742(A2)
       | 1CEE  6728                           BEQ.B       1D18
;1324:                                   {
;1325:                                       if(!StrtoIP(((struct StringInfo *)ed->ed_RouteGateway->SpecialInfo)->Buffer,&ed->ed_CurrentRoute->nr_Prefs.nrp_Gateway))
       | 1CF0  206A 07A2                      MOVEA.L     07A2(A2),A0
       | 1CF4  2268 0022                      MOVEA.L     0022(A0),A1
       | 1CF8  206A 0742                      MOVEA.L     0742(A2),A0
       | 1CFC  D0FC 0052                      ADDA.W      #0052,A0
       | 1D00  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1D04  2248                           MOVEA.L     A0,A1
       | 1D06  206F 0020                      MOVEA.L     0020(A7),A0
       | 1D0A  2050                           MOVEA.L     (A0),A0
       | 1D0C  6100 0432                      BSR.W       2140
       | 1D10  4A40                           TST.W       D0
       | 1D12  6604                           BNE.B       1D18
;1326:                                       {
;1327:                                           act = ed->ed_RouteGateway;
       | 1D14  266A 07A2                      MOVEA.L     07A2(A2),A3
;1328:                                       }
;1329:                                   }
;1330:                                   if(!act)
       | 1D18  200B                           MOVE.L      A3,D0
       | 1D1A  6604                           BNE.B       1D20
;1331:                                       act = ed->ed_RouteHops;
       | 1D1C  266A 07A6                      MOVEA.L     07A6(A2),A3
;1332: 
;1333:                                   refresh = TRUE;
       | 1D20  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 1D24  7C01                           MOVEQ       #01,D6
;1334:                                   break;
       | 1D26  6000 03A6                      BRA.W       20CE
;1335: 
;1336:         case EC_ROUTE_HOPS      : if(ed->ed_CurrentRoute)
       | 1D2A  266A 0742                      MOVEA.L     0742(A2),A3
       | 1D2E  200B                           MOVE.L      A3,D0
       | 1D30  671E                           BEQ.B       1D50
;1337:                                   {
;1338:                                       num = ((struct StringInfo *)ed->ed_RouteHops->SpecialInfo)->LongInt;
       | 1D32  206A 07A6                      MOVEA.L     07A6(A2),A0
       | 1D36  2268 0022                      MOVEA.L     0022(A0),A1
       | 1D3A  2E29 001C                      MOVE.L      001C(A1),D7
;1339:                                       if(num < 0)
       | 1D3E  7000                           MOVEQ       #00,D0
       | 1D40  BE80                           CMP.L       D0,D7
       | 1D42  6406                           BCC.B       1D4A
;1340:                                       {
;1341:                                           num = 0;
       | 1D44  2E00                           MOVE.L      D0,D7
;1342:                                           act = ed->ed_RouteHops;
;1343:                                           refresh = TRUE;
;1344:                                       }
       | 1D46  2B48 FFEE                      MOVE.L      A0,FFEE(A5)
;1345:                                       ed->ed_CurrentRoute->nr_Prefs.nrp_Hops = num;
       | 1D4A  2007                           MOVE.L      D7,D0
       | 1D4C  3740 0056                      MOVE.W      D0,0056(A3)
;1346:                                   }
;1347:                                   refresh = TRUE;
       | 1D50  7C01                           MOVEQ       #01,D6
;1348:                                   break;
       | 1D52  6000 037A                      BRA.W       20CE
;1349: 
;1350:         case EC_ROUTE_LIST      : node = ed->ed_PrefsWork.ep_NIPCRoutes.lh_Head;
       | 1D56  266A 0334                      MOVEA.L     0334(A2),A3
;1351:                                   while(icode--)
       | 1D5A  6002                           BRA.B       1D5E
;1352:                                   {
;1353:                                       node = node->ln_Succ;
       | 1D5C  2653                           MOVEA.L     (A3),A3
;1354:                                   }
       | 1D5E  2005                           MOVE.L      D5,D0
       | 1D60  2A00                           MOVE.L      D0,D5
       | 1D62  5345                           SUBQ.W      #1,D5
       | 1D64  4A40                           TST.W       D0
       | 1D66  66F4                           BNE.B       1D5C
;1355:                                   ed->ed_CurrentRoute = (struct NIPCRoute *)node;
       | 1D68  254B 0742                      MOVE.L      A3,0742(A2)
;1356:                                   refresh = TRUE;
       | 1D6C  7C01                           MOVEQ       #01,D6
;1357:                                   break;
       | 1D6E  6000 035E                      BRA.W       20CE
;1358: 
;1359:         /* Devices Panel */
;1360: 
;1361:         case EC_DEVICE_UNIT     : if(ed->ed_CurrentDevice)
       | 1D72  266A 0736                      MOVEA.L     0736(A2),A3
       | 1D76  200B                           MOVE.L      A3,D0
       | 1D78  671E                           BEQ.B       1D98
;1362:                                   {
;1363:                                       num = ((struct StringInfo *)ed->ed_DeviceUnit->SpecialInfo)->LongInt;
       | 1D7A  206A 07D6                      MOVEA.L     07D6(A2),A0
       | 1D7E  2268 0022                      MOVEA.L     0022(A0),A1
       | 1D82  2E29 001C                      MOVE.L      001C(A1),D7
;1364:                                       if(num < 0)
       | 1D86  7000                           MOVEQ       #00,D0
       | 1D88  BE80                           CMP.L       D0,D7
       | 1D8A  6408                           BCC.B       1D94
;1365:                                       {
;1366:                                           num = 0;
       | 1D8C  2E00                           MOVE.L      D0,D7
;1367:                                           act = ed->ed_DeviceUnit;
;1368:                                           refresh = TRUE;
       | 1D8E  7C01                           MOVEQ       #01,D6
;1369:                                       }
       | 1D90  2B48 FFEE                      MOVE.L      A0,FFEE(A5)
;1370:                                       ed->ed_CurrentDevice->nd_Prefs.ndp_Unit = num;
       | 1D94  2747 0016                      MOVE.L      D7,0016(A3)
;1371:                                   }
;1372:                                   if(!act)
       | 1D98  4AAD FFEE                      TST.L       FFEE(A5)
       | 1D9C  6600 0330                      BNE.W       20CE
;1373:                                       curr = ed->ed_DeviceUnit;
       | 1DA0  2B6A 07D6 FFEA                 MOVE.L      07D6(A2),FFEA(A5)
;1374:                                   break;
       | 1DA6  6000 0326                      BRA.W       20CE
;1375: 
;1376:         case EC_DEVICE_IPTYPE   : if(ed->ed_CurrentDevice)
       | 1DAA  266A 0736                      MOVEA.L     0736(A2),A3
       | 1DAE  200B                           MOVE.L      A3,D0
       | 1DB0  671E                           BEQ.B       1DD0
;1377:                                   {
;1378:                                       num = ((struct StringInfo *)ed->ed_DeviceIPType->SpecialInfo)->LongInt;
       | 1DB2  206A 07DA                      MOVEA.L     07DA(A2),A0
       | 1DB6  2268 0022                      MOVEA.L     0022(A0),A1
       | 1DBA  2E29 001C                      MOVE.L      001C(A1),D7
;1379:                                       if(num < 0)
       | 1DBE  7000                           MOVEQ       #00,D0
       | 1DC0  BE80                           CMP.L       D0,D7
       | 1DC2  6408                           BCC.B       1DCC
;1380:                                       {
;1381:                                           num = 0;
       | 1DC4  2E00                           MOVE.L      D0,D7
;1382:                                           act = ed->ed_DeviceIPType;
;1383:                                           refresh = TRUE;
       | 1DC6  7C01                           MOVEQ       #01,D6
;1384:                                       }
       | 1DC8  2B48 FFEE                      MOVE.L      A0,FFEE(A5)
;1385:                                       ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = num;
       | 1DCC  2747 000E                      MOVE.L      D7,000E(A3)
;1386:                                   }
;1387:                                   if(!act)
       | 1DD0  4AAD FFEE                      TST.L       FFEE(A5)
       | 1DD4  6600 02F8                      BNE.W       20CE
;1388:                                       curr = ed->ed_DeviceIPType;
       | 1DD8  2B6A 07DA FFEA                 MOVE.L      07DA(A2),FFEA(A5)
;1389:                                   break;
       | 1DDE  6000 02EE                      BRA.W       20CE
;1390: 
;1391:         case EC_DEVICE_ARPTYPE  : if(ed->ed_CurrentDevice)
       | 1DE2  266A 0736                      MOVEA.L     0736(A2),A3
       | 1DE6  200B                           MOVE.L      A3,D0
       | 1DE8  671E                           BEQ.B       1E08
;1392:                                   {
;1393:                                       num = ((struct StringInfo *)ed->ed_DeviceARPType->SpecialInfo)->LongInt;
       | 1DEA  206A 07DE                      MOVEA.L     07DE(A2),A0
       | 1DEE  2268 0022                      MOVEA.L     0022(A0),A1
       | 1DF2  2E29 001C                      MOVE.L      001C(A1),D7
;1394:                                       if(num < 0)
       | 1DF6  7000                           MOVEQ       #00,D0
       | 1DF8  BE80                           CMP.L       D0,D7
       | 1DFA  6408                           BCC.B       1E04
;1395:                                       {
;1396:                                          num = 0;
       | 1DFC  2E00                           MOVE.L      D0,D7
;1397:                                          act = ed->ed_DeviceARPType;
;1398:                                          refresh = TRUE;
       | 1DFE  7C01                           MOVEQ       #01,D6
;1399:                                       }
       | 1E00  2B48 FFEE                      MOVE.L      A0,FFEE(A5)
;1400:                                       ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = num;
       | 1E04  2747 0012                      MOVE.L      D7,0012(A3)
;1401:                                   }
;1402:                                   if(!act)
       | 1E08  4AAD FFEE                      TST.L       FFEE(A5)
       | 1E0C  6600 02C0                      BNE.W       20CE
;1403:                                       curr = ed->ed_DeviceARPTypeAble;
       | 1E10  2B6A 07EE FFEA                 MOVE.L      07EE(A2),FFEA(A5)
;1404:                                   break;
       | 1E16  6000 02B6                      BRA.W       20CE
;1405: 
;1406:         case EC_DEVICE_IP       : if(ed->ed_CurrentDevice)
       | 1E1A  4AAA 0736                      TST.L       0736(A2)
       | 1E1E  672E                           BEQ.B       1E4E
;1407:                                   {
;1408:                                       if(!StrtoIP(((struct StringInfo *)ed->ed_DeviceIP->SpecialInfo)->Buffer,&ed->ed_CurrentDevice->nd_Prefs.ndp_IPAddress))
       | 1E20  206A 07F6                      MOVEA.L     07F6(A2),A0
       | 1E24  2268 0022                      MOVEA.L     0022(A0),A1
       | 1E28  206A 0736                      MOVEA.L     0736(A2),A0
       | 1E2C  D0FC 0052                      ADDA.W      #0052,A0
       | 1E30  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1E34  2248                           MOVEA.L     A0,A1
       | 1E36  206F 0020                      MOVEA.L     0020(A7),A0
       | 1E3A  2050                           MOVEA.L     (A0),A0
       | 1E3C  6100 0302                      BSR.W       2140
       | 1E40  4A40                           TST.W       D0
       | 1E42  660A                           BNE.B       1E4E
;1409:                                       {
;1410:                                          act = ed->ed_DeviceIP;
       | 1E44  266A 07F6                      MOVEA.L     07F6(A2),A3
;1411:                                          refresh = TRUE;
       | 1E48  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 1E4C  7C01                           MOVEQ       #01,D6
;1412:                                       }
;1413:                                   }
;1414:                                   if(!act)
       | 1E4E  200B                           MOVE.L      A3,D0
       | 1E50  6600 027C                      BNE.W       20CE
;1415:                                       curr = ed->ed_DeviceIP;
       | 1E54  2B6A 07F6 FFEA                 MOVE.L      07F6(A2),FFEA(A5)
;1416:                                   break;
       | 1E5A  6000 0272                      BRA.W       20CE
;1417: 
;1418:         case EC_DEVICE_MASK     : if(ed->ed_CurrentDevice)
       | 1E5E  4AAA 0736                      TST.L       0736(A2)
       | 1E62  672E                           BEQ.B       1E92
;1419:                                   {
;1420:                                       if(!StrtoIP(((struct StringInfo *)ed->ed_DeviceMask->SpecialInfo)->Buffer,&ed->ed_CurrentDevice->nd_Prefs.ndp_IPSubnet))
       | 1E64  206A 07FA                      MOVEA.L     07FA(A2),A0
       | 1E68  2268 0022                      MOVEA.L     0022(A0),A1
       | 1E6C  206A 0736                      MOVEA.L     0736(A2),A0
       | 1E70  D0FC 0056                      ADDA.W      #0056,A0
       | 1E74  2F49 0020                      MOVE.L      A1,0020(A7)
       | 1E78  2248                           MOVEA.L     A0,A1
       | 1E7A  206F 0020                      MOVEA.L     0020(A7),A0
       | 1E7E  2050                           MOVEA.L     (A0),A0
       | 1E80  6100 02BE                      BSR.W       2140
       | 1E84  4A40                           TST.W       D0
       | 1E86  660A                           BNE.B       1E92
;1421:                                       {
;1422:                                          act = ed->ed_DeviceMask;
       | 1E88  266A 07FA                      MOVEA.L     07FA(A2),A3
;1423:                                          refresh = TRUE;
       | 1E8C  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 1E90  7C01                           MOVEQ       #01,D6
;1424:                                       }
;1425:                                   }
;1426:                                   if(!act)
       | 1E92  200B                           MOVE.L      A3,D0
       | 1E94  6600 0238                      BNE.W       20CE
;1427:                                       curr = ed->ed_DeviceMask;
       | 1E98  2B6A 07FA FFEA                 MOVE.L      07FA(A2),FFEA(A5)
;1428:                                   break;
       | 1E9E  6000 022E                      BRA.W       20CE
;1429: 
;1430:         case EC_DEVICE_ADDR     : if(ed->ed_CurrentDevice)
       | 1EA2  4AAA 0736                      TST.L       0736(A2)
       | 1EA6  6700 00CE                      BEQ.W       1F76
;1431:                                   {
;1432:                                       hardstring =((struct StringInfo *)ed->ed_DeviceAddress->SpecialInfo)->Buffer;
       | 1EAA  206A 07FE                      MOVEA.L     07FE(A2),A0
       | 1EAE  2268 0022                      MOVEA.L     0022(A0),A1
       | 1EB2  2651                           MOVEA.L     (A1),A3
;1433:                                       strncpy(ed->ed_CurrentDevice->nd_Prefs.ndp_HardString,hardstring,38);
       | 1EB4  206A 0736                      MOVEA.L     0736(A2),A0
       | 1EB8  D0FC 002A                      ADDA.W      #002A,A0
       | 1EBC  7026                           MOVEQ       #26,D0
       | 1EBE  224B                           MOVEA.L     A3,A1
       | 1EC0  4EBA  0000-XX.1                JSR         @strncpy(PC)
;1434:                                       hardaddr = ed->ed_CurrentDevice->nd_Prefs.ndp_HardAddress;
       | 1EC4  206A 0736                      MOVEA.L     0736(A2),A0
       | 1EC8  D0FC 001A                      ADDA.W      #001A,A0
;1435: 
;1436:                                       if (!Strnicmp("0x", hardstring, 2))
       | 1ECC  2F48 0020                      MOVE.L      A0,0020(A7)
       | 1ED0  41FA E1B6                      LEA         E1B6(PC),A0
       | 1ED4  224B                           MOVEA.L     A3,A1
       | 1ED6  7002                           MOVEQ       #02,D0
       | 1ED8  2C6A 0020                      MOVEA.L     0020(A2),A6
       | 1EDC  4EAE FF58                      JSR         FF58(A6)
       | 1EE0  4A80                           TST.L       D0
       | 1EE2  6600 0092                      BNE.W       1F76
;1437:                                       {
;1438:                                           parselength = strlen(hardstring);
       | 1EE6  204B                           MOVEA.L     A3,A0
       | 1EE8  4A18                           TST.B       (A0)+
       | 1EEA  66FC                           BNE.B       1EE8
       | 1EEC  5388                           SUBQ.L      #1,A0
       | 1EEE  91CB                           SUBA.L      A3,A0
;1439:                                           if ((parselength & 1L) == 1)
       | 1EF0  45EB 0002                      LEA         0002(A3),A2
       | 1EF4  2008                           MOVE.L      A0,D0
       | 1EF6  0200 0001                      ANDI.B      #01,D0
       | 1EFA  5300                           SUBQ.B      #1,D0
       | 1EFC  660C                           BNE.B       1F0A
;1440:                                               strcpy(hardstring + 1, hardstring + 2);
       | 1EFE  41EB 0001                      LEA         0001(A3),A0
       | 1F02  224A                           MOVEA.L     A2,A1
       | 1F04  10D9                           MOVE.B      (A1)+,(A0)+
       | 1F06  66FC                           BNE.B       1F04
;1441:                                           else
       | 1F08  6008                           BRA.B       1F12
;1442:                                               strcpy(hardstring, hardstring + 2);
       | 1F0A  204A                           MOVEA.L     A2,A0
       | 1F0C  224B                           MOVEA.L     A3,A1
       | 1F0E  12D8                           MOVE.B      (A0)+,(A1)+
       | 1F10  66FC                           BNE.B       1F0E
;1443:                                           for (x = 0; x < strlen(hardstring); x += 2)
       | 1F12  7800                           MOVEQ       #00,D4
       | 1F14  246F 0020                      MOVEA.L     0020(A7),A2
       | 1F18  604A                           BRA.B       1F64
;1444:                                               hardaddr[x / 2] = ((nibvert(hardstring[x]) << 4) | (nibvert(hardstring[x + 1])));
       | 1F1A  7000                           MOVEQ       #00,D0
       | 1F1C  1004                           MOVE.B      D4,D0
       | 1F1E  1233 0000                      MOVE.B      00(A3,D0.W),D1
       | 1F22  4881                           EXT.W       D1
       | 1F24  48C1                           EXT.L       D1
       | 1F26  2001                           MOVE.L      D1,D0
       | 1F28  6100 03D0                      BSR.W       22FA
       | 1F2C  7200                           MOVEQ       #00,D1
       | 1F2E  1204                           MOVE.B      D4,D1
       | 1F30  1433 1801                      MOVE.B      01(A3,D1.L),D2
       | 1F34  4882                           EXT.W       D2
       | 1F36  48C2                           EXT.L       D2
       | 1F38  1F40 0020                      MOVE.B      D0,0020(A7)
       | 1F3C  2002                           MOVE.L      D2,D0
       | 1F3E  6100 03BA                      BSR.W       22FA
       | 1F42  7200                           MOVEQ       #00,D1
       | 1F44  1204                           MOVE.B      D4,D1
       | 1F46  4A81                           TST.L       D1
       | 1F48  6A02                           BPL.B       1F4C
       | 1F4A  5281                           ADDQ.L      #1,D1
       | 1F4C  E281                           ASR.L       #1,D1
       | 1F4E  4880                           EXT.W       D0
       | 1F50  48C0                           EXT.L       D0
       | 1F52  142F 0020                      MOVE.B      0020(A7),D2
       | 1F56  4882                           EXT.W       D2
       | 1F58  48C2                           EXT.L       D2
       | 1F5A  E982                           ASL.L       #4,D2
       | 1F5C  8480                           OR.L        D0,D2
       | 1F5E  1582 1800                      MOVE.B      D2,00(A2,D1.L)
       | 1F62  5404                           ADDQ.B      #2,D4
       | 1F64  204B                           MOVEA.L     A3,A0
       | 1F66  4A18                           TST.B       (A0)+
       | 1F68  66FC                           BNE.B       1F66
       | 1F6A  5388                           SUBQ.L      #1,A0
       | 1F6C  91CB                           SUBA.L      A3,A0
       | 1F6E  7000                           MOVEQ       #00,D0
       | 1F70  1004                           MOVE.B      D4,D0
       | 1F72  B088                           CMP.L       A0,D0
       | 1F74  6DA4                           BLT.B       1F1A
;1445:                                       }
;1446:                                   }
;1447:                                   curr = ed->ed_DeviceAddress;
       | 1F76  246D FFE6                      MOVEA.L     FFE6(A5),A2
       | 1F7A  2B6A 07FE FFEA                 MOVE.L      07FE(A2),FFEA(A5)
;1448:                                   break;
       | 1F80  6000 014C                      BRA.W       20CE
;1449: 
;1450:         case EC_DEVICE_LIST     : node = ed->ed_PrefsWork.ep_NIPCDevices.lh_Head;
       | 1F84  266A 030A                      MOVEA.L     030A(A2),A3
;1451:                                   while (icode--)
       | 1F88  6002                           BRA.B       1F8C
;1452:                                   {
;1453:                                       node = node->ln_Succ;
       | 1F8A  2653                           MOVEA.L     (A3),A3
;1454:                                   }
       | 1F8C  2005                           MOVE.L      D5,D0
       | 1F8E  2A00                           MOVE.L      D0,D5
       | 1F90  5345                           SUBQ.W      #1,D5
       | 1F92  4A40                           TST.W       D0
       | 1F94  66F4                           BNE.B       1F8A
;1455:                                   ed->ed_CurrentDevice = (struct NIPCDevice *)node;
       | 1F96  254B 0736                      MOVE.L      A3,0736(A2)
;1456:                                   refresh = TRUE;
       | 1F9A  7C01                           MOVEQ       #01,D6
;1457:                                   break;
       | 1F9C  6000 0130                      BRA.W       20CE
;1458: 
;1459:         case EC_DEVICE_NAME     : strcpy((STRPTR)&ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,((struct StringInfo *)ed->ed_DevicePathName->SpecialInfo)->Buffer);
       | 1FA0  206A 0736                      MOVEA.L     0736(A2),A0
       | 1FA4  D0FC 005B                      ADDA.W      #005B,A0
       | 1FA8  226A 07F2                      MOVEA.L     07F2(A2),A1
       | 1FAC  2C69 0022                      MOVEA.L     0022(A1),A6
       | 1FB0  2256                           MOVEA.L     (A6),A1
       | 1FB2  10D9                           MOVE.B      (A1)+,(A0)+
       | 1FB4  66FC                           BNE.B       1FB2
;1460:                                   refresh = TRUE;
       | 1FB6  7C01                           MOVEQ       #01,D6
;1461:                                   curr = ed->ed_DevicePathName;
       | 1FB8  2B6A 07F2 FFEA                 MOVE.L      07F2(A2),FFEA(A5)
;1462:                                   break;
       | 1FBE  6000 010E                      BRA.W       20CE
;1463: 
;1464:         case EC_DEVICE_STATUS   : if(ed->ed_CurrentDevice)
       | 1FC2  266A 0736                      MOVEA.L     0736(A2),A3
       | 1FC6  200B                           MOVE.L      A3,D0
       | 1FC8  6700 0104                      BEQ.W       20CE
;1465:                                   {
;1466:                                       if(icode)
       | 1FCC  4A45                           TST.W       D5
       | 1FCE  670A                           BEQ.B       1FDA
;1467:                                           ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_ONLINE;
       | 1FD0  08EB 0001 005A                 BSET        #0001,005A(A3)
;1468:                                       else
       | 1FD6  6000 00F6                      BRA.W       20CE
;1469:                                           ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_ONLINE;
       | 1FDA  08AB 0001 005A                 BCLR        #0001,005A(A3)
;1470:                                   }
;1471:                                   break;
       | 1FE0  6000 00EC                      BRA.W       20CE
;1472: 
;1473:         case EC_DEVICE_MASK_ABLE    : if(ed->ed_CurrentDevice)
       | 1FE4  266A 0736                      MOVEA.L     0736(A2),A3
       | 1FE8  200B                           MOVE.L      A3,D0
       | 1FEA  6700 00E2                      BEQ.W       20CE
;1474:                                       {
;1475:                                           if(ed->ed_DeviceSubnetAble->Flags & GFLG_SELECTED)
       | 1FEE  206A 07E2                      MOVEA.L     07E2(A2),A0
       | 1FF2  0828 0007 000D                 BTST        #0007,000D(A0)
       | 1FF8  6708                           BEQ.B       2002
;1476:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_SUBNET;
       | 1FFA  08AB 0000 005A                 BCLR        #0000,005A(A3)
;1477:                                           else
       | 2000  6006                           BRA.B       2008
;1478:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_SUBNET;
       | 2002  08EB 0000 005A                 BSET        #0000,005A(A3)
;1479:                                           refresh = TRUE;
       | 2008  7C01                           MOVEQ       #01,D6
;1480:                                       }
;1481:                                       break;
       | 200A  6000 00C2                      BRA.W       20CE
;1482: 
;1483:         case EC_DEVICE_ADDR_ABLE    : if(ed->ed_CurrentDevice)
       | 200E  266A 0736                      MOVEA.L     0736(A2),A3
       | 2012  200B                           MOVE.L      A3,D0
       | 2014  6700 00B8                      BEQ.W       20CE
;1484:                                       {
;1485:                                           if(ed->ed_DeviceAddressAble->Flags & GFLG_SELECTED)
       | 2018  206A 07E6                      MOVEA.L     07E6(A2),A0
       | 201C  0828 0007 000D                 BTST        #0007,000D(A0)
       | 2022  6708                           BEQ.B       202C
;1486:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_HARDADDR;
       | 2024  08AB 0005 005A                 BCLR        #0005,005A(A3)
;1487:                                           else
       | 202A  6006                           BRA.B       2032
;1488:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_HARDADDR;
       | 202C  08EB 0005 005A                 BSET        #0005,005A(A3)
;1489:                                           refresh = TRUE;
       | 2032  7C01                           MOVEQ       #01,D6
;1490:                                       }
;1491:                                       break;
       | 2034  6000 0098                      BRA.W       20CE
;1492: 
;1493:         case EC_DEVICE_IPTYPE_ABLE  : if(ed->ed_CurrentDevice)
       | 2038  266A 0736                      MOVEA.L     0736(A2),A3
       | 203C  200B                           MOVE.L      A3,D0
       | 203E  6700 008E                      BEQ.W       20CE
;1494:                                       {
;1495:                                           if(ed->ed_DeviceIPTypeAble->Flags & GFLG_SELECTED)
       | 2042  206A 07EA                      MOVEA.L     07EA(A2),A0
       | 2046  0828 0007 000D                 BTST        #0007,000D(A0)
       | 204C  6708                           BEQ.B       2056
;1496:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_IPTYPE;
       | 204E  08EB 0003 005A                 BSET        #0003,005A(A3)
;1497:                                           else
       | 2054  6006                           BRA.B       205C
;1498:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_IPTYPE;
       | 2056  08AB 0003 005A                 BCLR        #0003,005A(A3)
;1499:                                           refresh = TRUE;
       | 205C  7C01                           MOVEQ       #01,D6
;1500:                                       }
;1501:                                       break;
       | 205E  606E                           BRA.B       20CE
;1502: 
;1503:         case EC_DEVICE_ARPTYPE_ABLE : if(ed->ed_CurrentDevice)
       | 2060  266A 0736                      MOVEA.L     0736(A2),A3
       | 2064  200B                           MOVE.L      A3,D0
       | 2066  6766                           BEQ.B       20CE
;1504:                                       {
;1505:                                           if(ed->ed_DeviceARPTypeAble->Flags & GFLG_SELECTED)
       | 2068  206A 07EE                      MOVEA.L     07EE(A2),A0
       | 206C  0828 0007 000D                 BTST        #0007,000D(A0)
       | 2072  6708                           BEQ.B       207C
;1506:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= NDPFLAGF_ARPTYPE;
       | 2074  08EB 0004 005A                 BSET        #0004,005A(A3)
;1507:                                           else
       | 207A  6006                           BRA.B       2082
;1508:                                               ed->ed_CurrentDevice->nd_Prefs.ndp_Flags &= ~NDPFLAGF_ARPTYPE;
       | 207C  08AB 0004 005A                 BCLR        #0004,005A(A3)
;1509:                                           refresh = TRUE;
       | 2082  7C01                           MOVEQ       #01,D6
;1510:                                       }
;1511:                                       break;
       | 2084  6048                           BRA.B       20CE
;1512: 
;1513:         case EC_DEVICE_ADD      : AddDeviceReq(ed);
       | 2086  204A                           MOVEA.L     A2,A0
       | 2088  6100 062A                      BSR.W       26B4
;1514:          			  refresh = TRUE;
       | 208C  7C01                           MOVEQ       #01,D6
;1515:          			  break;
       | 208E  603E                           BRA.B       20CE
;1516: 
;1517:         case EC_DEVICE_DEL      :
;1518: 	                          ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
;1519:                                                               GTLV_Labels,       ~0);
       | 2090  4878 FFFF                      PEA         FFFF
       | 2094  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 209A  2F2A 0806                      MOVE.L      0806(A2),-(A7)
       | 209E  4879  0000 055E-01             PEA         01.0000055E
       | 20A4  2F0A                           MOVE.L      A2,-(A7)
       | 20A6  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 20AA  4FEF 0014                      LEA         0014(A7),A7
       | 20AE  2540 0806                      MOVE.L      D0,0806(A2)
;1520:         			  Remove(ed->ed_CurrentDevice);
       | 20B2  226A 0736                      MOVEA.L     0736(A2),A1
       | 20B6  2C52                           MOVEA.L     (A2),A6
       | 20B8  4EAE FF04                      JSR         FF04(A6)
;1521:                                   FreeMem(ed->ed_CurrentDevice,sizeof(struct NIPCDevice));
       | 20BC  226A 0736                      MOVEA.L     0736(A2),A1
       | 20C0  7057                           MOVEQ       #57,D0
       | 20C2  E588                           LSL.L       #2,D0
       | 20C4  4EAE FF2E                      JSR         FF2E(A6)
;1522:                                   ed->ed_CurrentDevice = NULL;
       | 20C8  42AA 0736                      CLR.L       0736(A2)
;1523:                                   refresh = TRUE;
       | 20CC  7C01                           MOVEQ       #01,D6
;1524:                                   break;
;1525: 
;1526:         default                 : break;
;1527:     }
;1528: 
;1529:     if (refresh)
       | 20CE  4A46                           TST.W       D6
       | 20D0  6706                           BEQ.B       20D8
;1530:         RenderGadgets(ed);
       | 20D2  204A                           MOVEA.L     A2,A0
       | 20D4  6100 E8A0                      BSR.W       0976
;1531: 
;1532:     if(curr)
       | 20D8  4AAD FFEA                      TST.L       FFEA(A5)
       | 20DC  673E                           BEQ.B       211C
;1533:     {
;1534:         act = NULL;
       | 20DE  97CB                           SUBA.L      A3,A3
;1535:         while(act != curr)
       | 20E0  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
       | 20E4  6030                           BRA.B       2116
;1536:         {
;1537:             act = (act) ? act->NextGadget : curr->NextGadget;
       | 20E6  200B                           MOVE.L      A3,D0
       | 20E8  6704                           BEQ.B       20EE
       | 20EA  2053                           MOVEA.L     (A3),A0
       | 20EC  6006                           BRA.B       20F4
       | 20EE  226D FFEA                      MOVEA.L     FFEA(A5),A1
       | 20F2  2051                           MOVEA.L     (A1),A0
       | 20F4  2648                           MOVEA.L     A0,A3
;1538:             if(!(act->Flags & GFLG_DISABLED))
       | 20F6  2B48 FFEE                      MOVE.L      A0,FFEE(A5)
       | 20FA  0828 0000 000C                 BTST        #0000,000C(A0)
       | 2100  671A                           BEQ.B       211C
;1539:                 break;
;1540:             if(act == ed->ed_DeviceARPType)
       | 2102  2648                           MOVEA.L     A0,A3
       | 2104  246D FFE6                      MOVEA.L     FFE6(A5),A2
       | 2108  B7EA 07DE                      CMPA.L      07DE(A2),A3
       | 210C  6608                           BNE.B       2116
;1541:                 act = ed->ed_DeviceARPTypeAble;
       | 210E  266A 07EE                      MOVEA.L     07EE(A2),A3
       | 2112  2B4B FFEE                      MOVE.L      A3,FFEE(A5)
;1542: 
;1543:         }
       | 2116  B7ED FFEA                      CMPA.L      FFEA(A5),A3
       | 211A  66CA                           BNE.B       20E6
;1544:     }
;1545:     if(act)
       | 211C  266D FFEE                      MOVEA.L     FFEE(A5),A3
       | 2120  200B                           MOVE.L      A3,D0
       | 2122  6714                           BEQ.B       2138
;1546:         ActivateGadget(act,window,NULL);
       | 2124  2F0A                           MOVE.L      A2,-(A7)
       | 2126  204B                           MOVEA.L     A3,A0
       | 2128  226A 01BC                      MOVEA.L     01BC(A2),A1
       | 212C  2C6A 0004                      MOVEA.L     0004(A2),A6
       | 2130  95CA                           SUBA.L      A2,A2
       | 2132  4EAE FE32                      JSR         FE32(A6)
       | 2136  245F                           MOVEA.L     (A7)+,A2
;1547: }
       | 2138  4CDF 4CF4                      MOVEM.L     (A7)+,D2/D4-D7/A2-A3/A6
       | 213C  4E5D                           UNLK        A5
       | 213E  4E75                           RTS
;1548: 
;1549: 
;1550: /*****************************************************************************/
;1551: 
;1552: BOOL StrtoIP(STRPTR ipstr, ULONG *ipnum)
;1553: {
       | 2140  4E55 FFF4                      LINK        A5,#FFF4
       | 2144  48E7 0330                      MOVEM.L     D6-D7/A2-A3,-(A7)
       | 2148  2648                           MOVEA.L     A0,A3
       | 214A  2449                           MOVEA.L     A1,A2
;1554:     char *y;
;1555:     int x, tmp;
;1556:     ULONG add;
;1557: 
;1558:     y = ipstr;
       | 214C  204B                           MOVEA.L     A3,A0
       | 214E  2648                           MOVEA.L     A0,A3
;1559:     add = 0L;
       | 2150  7E00                           MOVEQ       #00,D7
;1560:     if(ipstr[0])
       | 2152  4A10                           TST.B       (A0)
       | 2154  674A                           BEQ.B       21A0
;1561:     {
;1562:         for(x = 0; x < 3; x++)
       | 2156  7C00                           MOVEQ       #00,D6
       | 2158  602A                           BRA.B       2184
;1563:         {
;1564:             stcd_l(y,&tmp);
       | 215A  204B                           MOVEA.L     A3,A0
       | 215C  43ED FFF4                      LEA         FFF4(A5),A1
       | 2160  4EBA  0000-XX.1                JSR         @stcd_l(PC)
;1565:             add = (add << 8) | tmp;
       | 2164  8EAD FFF4                      OR.L        FFF4(A5),D7
;1566:             y = (char *) strchr((char *) (y+1), '.');
       | 2168  41EB 0001                      LEA         0001(A3),A0
       | 216C  702E                           MOVEQ       #2E,D0
       | 216E  4EBA  0000-XX.1                JSR         @strchr(PC)
       | 2172  2640                           MOVEA.L     D0,A3
;1567:             if((!y) || (*y == 0))
       | 2174  200B                           MOVE.L      A3,D0
       | 2176  6704                           BEQ.B       217C
       | 2178  4A13                           TST.B       (A3)
       | 217A  6604                           BNE.B       2180
;1568:                 return FALSE;
       | 217C  7000                           MOVEQ       #00,D0
       | 217E  6024                           BRA.B       21A4
;1569:             y = (char *) (y + 1);
       | 2180  528B                           ADDQ.L      #1,A3
       | 2182  5286                           ADDQ.L      #1,D6
       | 2184  E187                           ASL.L       #8,D7
       | 2186  7003                           MOVEQ       #03,D0
       | 2188  BC80                           CMP.L       D0,D6
       | 218A  6DCE                           BLT.B       215A
;1570:         }
;1571:         stcd_l(y, &tmp);
       | 218C  204B                           MOVEA.L     A3,A0
       | 218E  43ED FFF4                      LEA         FFF4(A5),A1
       | 2192  4EBA  0000-XX.1                JSR         @stcd_l(PC)
;1572:         add = (add << 8) | tmp;
;1573:         *ipnum = add;
       | 2196  2007                           MOVE.L      D7,D0
       | 2198  80AD FFF4                      OR.L        FFF4(A5),D0
       | 219C  2480                           MOVE.L      D0,(A2)
;1574:     }
;1575:     else
       | 219E  6002                           BRA.B       21A2
;1576:     	*ipnum=0L;
       | 21A0  4292                           CLR.L       (A2)
;1577: 
;1578:     return(TRUE);
       | 21A2  7001                           MOVEQ       #01,D0
;1579: }
       | 21A4  4CDF 0CC0                      MOVEM.L     (A7)+,D6-D7/A2-A3
       | 21A8  4E5D                           UNLK        A5
       | 21AA  4E75                           RTS
;1580: 
;1581: /*****************************************************************************/
;1582: void MakeNewDisplay(EdDataPtr ed)
;1583: {
       | 21AC  48E7 3832                      MOVEM.L     D2-D4/A2-A3/A6,-(A7)
       | 21B0  2648                           MOVEA.L     A0,A3
;1584:     RemoveGList(window, ed->ed_Gadgets, -1);
       | 21B2  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 21B6  226B 01C8                      MOVEA.L     01C8(A3),A1
       | 21BA  70FF                           MOVEQ       #FF,D0
       | 21BC  2C6B 0004                      MOVEA.L     0004(A3),A6
       | 21C0  4EAE FE44                      JSR         FE44(A6)
;1585:     SetAPen(window->RPort, ed->ed_DrawInfo->dri_Pens[BACKGROUNDPEN]);
       | 21C4  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 21C8  226B 01D0                      MOVEA.L     01D0(A3),A1
       | 21CC  2469 0004                      MOVEA.L     0004(A1),A2
       | 21D0  7000                           MOVEQ       #00,D0
       | 21D2  302A 000E                      MOVE.W      000E(A2),D0
       | 21D6  2268 0032                      MOVEA.L     0032(A0),A1
       | 21DA  2C6B 000C                      MOVEA.L     000C(A3),A6
       | 21DE  4EAE FEAA                      JSR         FEAA(A6)
;1586:     RectFill(window->RPort, window->BorderLeft, window->BorderTop + 20, window->Width - window->BorderRight - 1, window->Height - window->BorderBottom - 25);
       | 21E2  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 21E6  1028 0036                      MOVE.B      0036(A0),D0
       | 21EA  4880                           EXT.W       D0
       | 21EC  48C0                           EXT.L       D0
       | 21EE  1228 0037                      MOVE.B      0037(A0),D1
       | 21F2  4881                           EXT.W       D1
       | 21F4  48C1                           EXT.L       D1
       | 21F6  7414                           MOVEQ       #14,D2
       | 21F8  D282                           ADD.L       D2,D1
       | 21FA  1428 0038                      MOVE.B      0038(A0),D2
       | 21FE  4882                           EXT.W       D2
       | 2200  48C2                           EXT.L       D2
       | 2202  3628 0008                      MOVE.W      0008(A0),D3
       | 2206  48C3                           EXT.L       D3
       | 2208  9682                           SUB.L       D2,D3
       | 220A  5383                           SUBQ.L      #1,D3
       | 220C  1428 0039                      MOVE.B      0039(A0),D2
       | 2210  4882                           EXT.W       D2
       | 2212  48C2                           EXT.L       D2
       | 2214  3828 000A                      MOVE.W      000A(A0),D4
       | 2218  48C4                           EXT.L       D4
       | 221A  9882                           SUB.L       D2,D4
       | 221C  7419                           MOVEQ       #19,D2
       | 221E  9882                           SUB.L       D2,D4
       | 2220  2403                           MOVE.L      D3,D2
       | 2222  2604                           MOVE.L      D4,D3
       | 2224  2268 0032                      MOVEA.L     0032(A0),A1
       | 2228  4EAE FECE                      JSR         FECE(A6)
;1587:     FreeGadgets(ed->ed_Gadgets);
       | 222C  206B 01C8                      MOVEA.L     01C8(A3),A0
       | 2230  2C6B 001C                      MOVEA.L     001C(A3),A6
       | 2234  4EAE FFDC                      JSR         FFDC(A6)
;1588: 
;1589:     ed->ed_Gadgets = NULL;
       | 2238  91C8                           SUBA.L      A0,A0
       | 223A  2748 01C8                      MOVE.L      A0,01C8(A3)
;1590:     ed->ed_LastAdded = NULL;
       | 223E  2748 01B8                      MOVE.L      A0,01B8(A3)
;1591:     RenderGadgets(ed);
       | 2242  204B                           MOVEA.L     A3,A0
       | 2244  6100 E730                      BSR.W       0976
;1592:     if (ed->ed_LastAdded)
       | 2248  4AAB 01B8                      TST.L       01B8(A3)
       | 224C  6732                           BEQ.B       2280
;1593:     {
;1594:         AddGList (window, ed->ed_Gadgets, -1, -1, NULL);
       | 224E  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 2252  226B 01C8                      MOVEA.L     01C8(A3),A1
       | 2256  70FF                           MOVEQ       #FF,D0
       | 2258  2200                           MOVE.L      D0,D1
       | 225A  95CA                           SUBA.L      A2,A2
       | 225C  2C6B 0004                      MOVEA.L     0004(A3),A6
       | 2260  4EAE FE4A                      JSR         FE4A(A6)
;1595:         RefreshGList (ed->ed_Gadgets, window, NULL, -1);
       | 2264  206B 01C8                      MOVEA.L     01C8(A3),A0
       | 2268  226B 01BC                      MOVEA.L     01BC(A3),A1
       | 226C  70FF                           MOVEQ       #FF,D0
       | 226E  4EAE FE50                      JSR         FE50(A6)
;1596:         GT_RefreshWindow (window, NULL);
       | 2272  224A                           MOVEA.L     A2,A1
       | 2274  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 2278  2C6B 001C                      MOVEA.L     001C(A3),A6
       | 227C  4EAE FFAC                      JSR         FFAC(A6)
;1597:     }
;1598: }
       | 2280  4CDF 4C1C                      MOVEM.L     (A7)+,D2-D4/A2-A3/A6
       | 2284  4E75                           RTS
;1599: 
;1600: /*****************************************************************************/
;1601: 
;1602: void IPToStr(ULONG ipnum, STRPTR ipstr)
;1603: {
       | 2286  4E55 FFFC                      LINK        A5,#FFFC
       | 228A  48E7 3910                      MOVEM.L     D2-D4/D7/A3,-(A7)
       | 228E  2E00                           MOVE.L      D0,D7
       | 2290  2648                           MOVEA.L     A0,A3
;1604:     UBYTE num[4];
;1605: 
;1606:     if(ipnum)
       | 2292  4A87                           TST.L       D7
       | 2294  675A                           BEQ.B       22F0
;1607:     {
;1608:         num[0] = (ipnum >> 24L);
       | 2296  2007                           MOVE.L      D7,D0
       | 2298  4240                           CLR.W       D0
       | 229A  4840                           SWAP        D0
       | 229C  E088                           LSR.L       #8,D0
       | 229E  1B40 FFFC                      MOVE.B      D0,FFFC(A5)
;1609:         num[1] = (ipnum >> 16L) & 0xff;
       | 22A2  2207                           MOVE.L      D7,D1
       | 22A4  4241                           CLR.W       D1
       | 22A6  4841                           SWAP        D1
       | 22A8  7400                           MOVEQ       #00,D2
       | 22AA  4602                           NOT.B       D2
       | 22AC  C282                           AND.L       D2,D1
       | 22AE  1B41 FFFD                      MOVE.B      D1,FFFD(A5)
;1610:         num[2] = (ipnum >>  8L) & 0xff;
       | 22B2  2607                           MOVE.L      D7,D3
       | 22B4  E08B                           LSR.L       #8,D3
       | 22B6  C682                           AND.L       D2,D3
       | 22B8  1B43 FFFE                      MOVE.B      D3,FFFE(A5)
;1611:         num[3] = (ipnum & 0xff);
       | 22BC  2407                           MOVE.L      D7,D2
       | 22BE  7800                           MOVEQ       #00,D4
       | 22C0  4604                           NOT.B       D4
       | 22C2  C484                           AND.L       D4,D2
       | 22C4  1B42 FFFF                      MOVE.B      D2,FFFF(A5)
;1612:         sprintf(ipstr,"%.3ld.%.3ld.%.3ld.%.3ld",num[0],num[1],num[2],num[3]);
       | 22C8  7800                           MOVEQ       #00,D4
       | 22CA  1800                           MOVE.B      D0,D4
       | 22CC  7000                           MOVEQ       #00,D0
       | 22CE  1001                           MOVE.B      D1,D0
       | 22D0  7200                           MOVEQ       #00,D1
       | 22D2  1203                           MOVE.B      D3,D1
       | 22D4  7600                           MOVEQ       #00,D3
       | 22D6  1602                           MOVE.B      D2,D3
       | 22D8  2F03                           MOVE.L      D3,-(A7)
       | 22DA  2F01                           MOVE.L      D1,-(A7)
       | 22DC  2F00                           MOVE.L      D0,-(A7)
       | 22DE  2F04                           MOVE.L      D4,-(A7)
       | 22E0  487A DDAA                      PEA         DDAA(PC)
       | 22E4  2F0B                           MOVE.L      A3,-(A7)
       | 22E6  4EBA  0000-XX.1                JSR         _sprintf(PC)
       | 22EA  4FEF 0018                      LEA         0018(A7),A7
;1613:     }
;1614:     else
       | 22EE  6002                           BRA.B       22F2
;1615:     	ipstr[0]=0;
       | 22F0  4213                           CLR.B       (A3)
;1616: }
       | 22F2  4CDF 089C                      MOVEM.L     (A7)+,D2-D4/D7/A3
       | 22F6  4E5D                           UNLK        A5
       | 22F8  4E75                           RTS
;1617: 
;1618: /*****************************************************************************/
;1619: 
;1620: char nibvert(ichar)
;1621: char ichar;
;1622: {
       | 22FA  2F07                           MOVE.L      D7,-(A7)
       | 22FC  2E00                           MOVE.L      D0,D7
;1623:     if (ichar > 96)
       | 22FE  7060                           MOVEQ       #60,D0
       | 2300  BE00                           CMP.B       D0,D7
       | 2302  6F08                           BLE.B       230C
;1624:         return ((char) (ichar - 87));
       | 2304  2007                           MOVE.L      D7,D0
       | 2306  0400 0057                      SUBI.B      #57,D0
       | 230A  601A                           BRA.B       2326
;1625:     if ((ichar >= '0') && (ichar <= '9'))
       | 230C  7030                           MOVEQ       #30,D0
       | 230E  BE00                           CMP.B       D0,D7
       | 2310  6D0E                           BLT.B       2320
       | 2312  7239                           MOVEQ       #39,D1
       | 2314  BE01                           CMP.B       D1,D7
       | 2316  6E08                           BGT.B       2320
;1626:         return ((char) (ichar - '0'));
       | 2318  2007                           MOVE.L      D7,D0
       | 231A  0400 0030                      SUBI.B      #30,D0
       | 231E  6006                           BRA.B       2326
;1627:     else
;1628:         return ((char) (ichar - 55));
       | 2320  2007                           MOVE.L      D7,D0
       | 2322  0400 0037                      SUBI.B      #37,D0
;1629: }
       | 2326  2E1F                           MOVE.L      (A7)+,D7
       | 2328  4E75                           RTS
;1630: 
;1631: /*****************************************************************************/
;1632: 
;1633: VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
;1634: {
       | 232A  4E55 FFF8                      LINK        A5,#FFF8
       | 232E  2F0B                           MOVE.L      A3,-(A7)
       | 2330  2649                           MOVEA.L     A1,A3
       | 2332  48ED 0101 FFF8                 MOVEM.L     D0/A0,FFF8(A5)
;1635:     state->cs_Available = TRUE;
       | 2338  36BC 0001                      MOVE.W      #0001,(A3)
;1636:     state->cs_Selected  = FALSE;
       | 233C  426B 0002                      CLR.W       0002(A3)
;1637: }
       | 2340  265F                           MOVEA.L     (A7)+,A3
       | 2342  4E5D                           UNLK        A5
       | 2344  4E75                           RTS
;1638: 
;1639: /*****************************************************************************/
;1640: 
;1641: VOID CopyPrefs(EdDataPtr ed, struct ExtPrefs *p1, struct ExtPrefs *p2)
;1642: {
       | 2346  4E55 FFE8                      LINK        A5,#FFE8
       | 234A  48E7 0032                      MOVEM.L     A2-A3/A6,-(A7)
       | 234E  2648                           MOVEA.L     A0,A3
       | 2350  2B49 FFF8                      MOVE.L      A1,FFF8(A5)
       | 2354  600C                           BRA.B       2362
;1643:     struct NIPCDevice *nd1, *nd2;
;1644:     struct NIPCRoute *nr1, *nr2;
;1645:     struct NIPCRealm *nz1, *nz2;
;1646: 
;1647:     while(nd1 = (struct NIPCDevice *)RemHead(&p1->ep_NIPCDevices))
;1648:     {
;1649:         FreeMem(nd1,sizeof(struct NIPCDevice));
       | 2356  224A                           MOVEA.L     A2,A1
       | 2358  7057                           MOVEQ       #57,D0
       | 235A  E588                           LSL.L       #2,D0
       | 235C  2C53                           MOVEA.L     (A3),A6
       | 235E  4EAE FF2E                      JSR         FF2E(A6)
;1650:     }
       | 2362  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 2366  2C53                           MOVEA.L     (A3),A6
       | 2368  4EAE FEFE                      JSR         FEFE(A6)
       | 236C  2440                           MOVEA.L     D0,A2
       | 236E  200A                           MOVE.L      A2,D0
       | 2370  66E4                           BNE.B       2356
       | 2372  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 2376  D0FC 002A                      ADDA.W      #002A,A0
       | 237A  2F48 0018                      MOVE.L      A0,0018(A7)
       | 237E  600A                           BRA.B       238A
;1651:     while(nr1 = (struct NIPCRoute *)RemHead(&p1->ep_NIPCRoutes))
;1652:     {
;1653:         FreeMem(nr1,sizeof(struct NIPCRoute));
       | 2380  224A                           MOVEA.L     A2,A1
       | 2382  7058                           MOVEQ       #58,D0
       | 2384  2C53                           MOVEA.L     (A3),A6
       | 2386  4EAE FF2E                      JSR         FF2E(A6)
;1654:     }
       | 238A  206F 0018                      MOVEA.L     0018(A7),A0
       | 238E  2C53                           MOVEA.L     (A3),A6
       | 2390  4EAE FEFE                      JSR         FEFE(A6)
       | 2394  2440                           MOVEA.L     D0,A2
       | 2396  200A                           MOVE.L      A2,D0
       | 2398  66E6                           BNE.B       2380
       | 239A  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 239E  D0FC 000E                      ADDA.W      #000E,A0
       | 23A2  2F48 0014                      MOVE.L      A0,0014(A7)
       | 23A6  600C                           BRA.B       23B4
;1655:     while(nz1 = (struct NIPCRealm *)RemHead(&p1->ep_NIPCLocalRealms))
;1656:     {
;1657:         FreeMem(nz1,sizeof(struct NIPCRealm));
       | 23A8  224A                           MOVEA.L     A2,A1
       | 23AA  7069                           MOVEQ       #69,D0
       | 23AC  D080                           ADD.L       D0,D0
       | 23AE  2C53                           MOVEA.L     (A3),A6
       | 23B0  4EAE FF2E                      JSR         FF2E(A6)
;1658:     }
       | 23B4  206F 0014                      MOVEA.L     0014(A7),A0
       | 23B8  2C53                           MOVEA.L     (A3),A6
       | 23BA  4EAE FEFE                      JSR         FEFE(A6)
       | 23BE  2440                           MOVEA.L     D0,A2
       | 23C0  200A                           MOVE.L      A2,D0
       | 23C2  66E4                           BNE.B       23A8
       | 23C4  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 23C8  D0FC 001C                      ADDA.W      #001C,A0
       | 23CC  2F48 0010                      MOVE.L      A0,0010(A7)
       | 23D0  600C                           BRA.B       23DE
;1659:     while(nz1 = (struct NIPCRealm *)RemHead(&p1->ep_NIPCRemoteRealms))
;1660:     {
;1661:         FreeMem(nz1,sizeof(struct NIPCRealm));
       | 23D2  224A                           MOVEA.L     A2,A1
       | 23D4  7069                           MOVEQ       #69,D0
       | 23D6  D080                           ADD.L       D0,D0
       | 23D8  2C53                           MOVEA.L     (A3),A6
       | 23DA  4EAE FF2E                      JSR         FF2E(A6)
;1662:     }
       | 23DE  206F 0010                      MOVEA.L     0010(A7),A0
       | 23E2  2C53                           MOVEA.L     (A3),A6
       | 23E4  4EAE FEFE                      JSR         FEFE(A6)
       | 23E8  2440                           MOVEA.L     D0,A2
       | 23EA  200A                           MOVE.L      A2,D0
       | 23EC  66E4                           BNE.B       23D2
;1663: 
;1664:     nd1 = (struct NIPCDevice *)p2->ep_NIPCDevices.lh_Head;
       | 23EE  206D 0008                      MOVEA.L     0008(A5),A0
       | 23F2  2450                           MOVEA.L     (A0),A2
;1665:     while(nd1->nd_Node.ln_Succ)
       | 23F4  604A                           BRA.B       2440
;1666:     {
;1667:         if(nd2 = (struct NIPCDevice *)AllocMem(sizeof(struct NIPCDevice),MEMF_PUBLIC|MEMF_CLEAR))
       | 23F6  7057                           MOVEQ       #57,D0
       | 23F8  E588                           LSL.L       #2,D0
       | 23FA  223C 0001 0001                 MOVE.L      #00010001,D1
       | 2400  2C53                           MOVEA.L     (A3),A6
       | 2402  4EAE FF3A                      JSR         FF3A(A6)
       | 2406  2F40 000C                      MOVE.L      D0,000C(A7)
       | 240A  4A80                           TST.L       D0
       | 240C  6730                           BEQ.B       243E
;1668:         {
;1669:             CopyMem(&nd1->nd_Prefs,&nd2->nd_Prefs,sizeof(struct NIPCDevicePrefs));
       | 240E  41EA 000E                      LEA         000E(A2),A0
       | 2412  2240                           MOVEA.L     D0,A1
       | 2414  4DE9 000E                      LEA         000E(A1),A6
       | 2418  224E                           MOVEA.L     A6,A1
       | 241A  203C 0000 014E                 MOVE.L      #0000014E,D0
       | 2420  2C53                           MOVEA.L     (A3),A6
       | 2422  4EAE FD90                      JSR         FD90(A6)
;1670:             nd2->nd_Node.ln_Name = (STRPTR) &nd2->nd_Prefs.ndp_DevPathName;
       | 2426  206F 000C                      MOVEA.L     000C(A7),A0
       | 242A  43E8 005B                      LEA         005B(A0),A1
       | 242E  2149 000A                      MOVE.L      A1,000A(A0)
;1671:             AddTail(&p1->ep_NIPCDevices,(struct Node *)nd2);
       | 2432  2248                           MOVEA.L     A0,A1
       | 2434  206D FFF8                      MOVEA.L     FFF8(A5),A0
       | 2438  2C53                           MOVEA.L     (A3),A6
       | 243A  4EAE FF0A                      JSR         FF0A(A6)
;1672:         }
;1673:         nd1 = (struct NIPCDevice *) nd1->nd_Node.ln_Succ;
       | 243E  2452                           MOVEA.L     (A2),A2
;1674:     }
       | 2440  4A92                           TST.L       (A2)
       | 2442  66B2                           BNE.B       23F6
;1675: 
;1676:     nz1 = (struct NIPCRealm *)p2->ep_NIPCLocalRealms.lh_Head;
       | 2444  206D 0008                      MOVEA.L     0008(A5),A0
       | 2448  2468 000E                      MOVEA.L     000E(A0),A2
;1677:     while(nz1->nz_Node.ln_Succ)
       | 244C  6046                           BRA.B       2494
;1678:     {
;1679:         if(nz2 = (struct NIPCRealm *)AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
       | 244E  7069                           MOVEQ       #69,D0
       | 2450  D080                           ADD.L       D0,D0
       | 2452  223C 0001 0001                 MOVE.L      #00010001,D1
       | 2458  2C53                           MOVEA.L     (A3),A6
       | 245A  4EAE FF3A                      JSR         FF3A(A6)
       | 245E  2F40 000C                      MOVE.L      D0,000C(A7)
       | 2462  4A80                           TST.L       D0
       | 2464  672C                           BEQ.B       2492
;1680:         {
;1681:             CopyMem(&nz1->nz_Prefs,&nz2->nz_Prefs,sizeof(struct NIPCRealmPrefs));
       | 2466  41EA 008E                      LEA         008E(A2),A0
       | 246A  2240                           MOVEA.L     D0,A1
       | 246C  4DE9 008E                      LEA         008E(A1),A6
       | 2470  224E                           MOVEA.L     A6,A1
       | 2472  7044                           MOVEQ       #44,D0
       | 2474  2C53                           MOVEA.L     (A3),A6
       | 2476  4EAE FD90                      JSR         FD90(A6)
;1682:             nz2->nz_Node.ln_Name = (STRPTR) &nz2->nz_String;
       | 247A  206F 000C                      MOVEA.L     000C(A7),A0
       | 247E  43E8 000E                      LEA         000E(A0),A1
       | 2482  2149 000A                      MOVE.L      A1,000A(A0)
;1683:             AddTail(&p1->ep_NIPCLocalRealms,(struct Node *)nz2);
       | 2486  2248                           MOVEA.L     A0,A1
       | 2488  206F 0014                      MOVEA.L     0014(A7),A0
       | 248C  2C53                           MOVEA.L     (A3),A6
       | 248E  4EAE FF0A                      JSR         FF0A(A6)
;1684:         }
;1685:         nz1 = (struct NIPCRealm *) nz1->nz_Node.ln_Succ;
       | 2492  2452                           MOVEA.L     (A2),A2
;1686:     }
       | 2494  4A92                           TST.L       (A2)
       | 2496  66B6                           BNE.B       244E
;1687: 
;1688:     nz1 = (struct NIPCRealm *)p2->ep_NIPCRemoteRealms.lh_Head;
       | 2498  206D 0008                      MOVEA.L     0008(A5),A0
       | 249C  2468 001C                      MOVEA.L     001C(A0),A2
;1689:     while(nz1->nz_Node.ln_Succ)
       | 24A0  6046                           BRA.B       24E8
;1690:     {
;1691:         if(nz2 = (struct NIPCRealm *)AllocMem(sizeof(struct NIPCRealm),MEMF_PUBLIC|MEMF_CLEAR))
       | 24A2  7069                           MOVEQ       #69,D0
       | 24A4  D080                           ADD.L       D0,D0
       | 24A6  223C 0001 0001                 MOVE.L      #00010001,D1
       | 24AC  2C53                           MOVEA.L     (A3),A6
       | 24AE  4EAE FF3A                      JSR         FF3A(A6)
       | 24B2  2F40 000C                      MOVE.L      D0,000C(A7)
       | 24B6  4A80                           TST.L       D0
       | 24B8  672C                           BEQ.B       24E6
;1692:         {
;1693:             CopyMem(&nz1->nz_Prefs,&nz2->nz_Prefs,sizeof(struct NIPCRealmPrefs));
       | 24BA  41EA 008E                      LEA         008E(A2),A0
       | 24BE  2240                           MOVEA.L     D0,A1
       | 24C0  4DE9 008E                      LEA         008E(A1),A6
       | 24C4  224E                           MOVEA.L     A6,A1
       | 24C6  7044                           MOVEQ       #44,D0
       | 24C8  2C53                           MOVEA.L     (A3),A6
       | 24CA  4EAE FD90                      JSR         FD90(A6)
;1694:             nz2->nz_Node.ln_Name = (STRPTR) &nz2->nz_String;
       | 24CE  206F 000C                      MOVEA.L     000C(A7),A0
       | 24D2  43E8 000E                      LEA         000E(A0),A1
       | 24D6  2149 000A                      MOVE.L      A1,000A(A0)
;1695:             AddTail(&p1->ep_NIPCRemoteRealms,(struct Node *)nz2);
       | 24DA  2248                           MOVEA.L     A0,A1
       | 24DC  206F 0010                      MOVEA.L     0010(A7),A0
       | 24E0  2C53                           MOVEA.L     (A3),A6
       | 24E2  4EAE FF0A                      JSR         FF0A(A6)
;1696:         }
;1697:         nz1 = (struct NIPCRealm *) nz1->nz_Node.ln_Succ;
       | 24E6  2452                           MOVEA.L     (A2),A2
;1698:     }
       | 24E8  4A92                           TST.L       (A2)
       | 24EA  66B6                           BNE.B       24A2
;1699: 
;1700:     nr1 = (struct NIPCRoute *)p2->ep_NIPCRoutes.lh_Head;
       | 24EC  206D 0008                      MOVEA.L     0008(A5),A0
       | 24F0  2468 002A                      MOVEA.L     002A(A0),A2
;1701:     while(nr1->nr_Node.ln_Succ)
       | 24F4  6044                           BRA.B       253A
;1702:     {
;1703:         if(nr2 = (struct NIPCRoute *)AllocMem(sizeof(struct NIPCRoute),MEMF_PUBLIC|MEMF_CLEAR))
       | 24F6  7058                           MOVEQ       #58,D0
       | 24F8  223C 0001 0001                 MOVE.L      #00010001,D1
       | 24FE  2C53                           MOVEA.L     (A3),A6
       | 2500  4EAE FF3A                      JSR         FF3A(A6)
       | 2504  2F40 000C                      MOVE.L      D0,000C(A7)
       | 2508  4A80                           TST.L       D0
       | 250A  672C                           BEQ.B       2538
;1704:         {
;1705:             CopyMem(&nr1->nr_Prefs,&nr2->nr_Prefs,sizeof(struct NIPCRoutePrefs));
       | 250C  41EA 004E                      LEA         004E(A2),A0
       | 2510  2240                           MOVEA.L     D0,A1
       | 2512  4DE9 004E                      LEA         004E(A1),A6
       | 2516  224E                           MOVEA.L     A6,A1
       | 2518  700A                           MOVEQ       #0A,D0
       | 251A  2C53                           MOVEA.L     (A3),A6
       | 251C  4EAE FD90                      JSR         FD90(A6)
;1706:             nr2->nr_Node.ln_Name = (STRPTR) &nr2->nr_String;
       | 2520  206F 000C                      MOVEA.L     000C(A7),A0
       | 2524  43E8 000E                      LEA         000E(A0),A1
       | 2528  2149 000A                      MOVE.L      A1,000A(A0)
;1707:             AddTail(&p1->ep_NIPCRoutes,(struct Node *)nr2);
       | 252C  2248                           MOVEA.L     A0,A1
       | 252E  206F 0018                      MOVEA.L     0018(A7),A0
       | 2532  2C53                           MOVEA.L     (A3),A6
       | 2534  4EAE FF0A                      JSR         FF0A(A6)
;1708:         }
;1709:         nr1 = (struct NIPCRoute *) nr1->nr_Node.ln_Succ;
       | 2538  2452                           MOVEA.L     (A2),A2
;1710:     }
       | 253A  4A92                           TST.L       (A2)
       | 253C  66B8                           BNE.B       24F6
;1711:     CopyMem(&p2->ep_NIPCHostPrefs,&p1->ep_NIPCHostPrefs,sizeof(struct NIPCHostPrefs));
       | 253E  206D 0008                      MOVEA.L     0008(A5),A0
       | 2542  D0FC 0038                      ADDA.W      #0038,A0
       | 2546  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 254A  D2FC 0038                      ADDA.W      #0038,A1
       | 254E  7054                           MOVEQ       #54,D0
       | 2550  D080                           ADD.L       D0,D0
       | 2552  2C53                           MOVEA.L     (A3),A6
       | 2554  4EAE FD90                      JSR         FD90(A6)
;1712:     if((p1 == (struct ExtPrefs *)&ed->ed_PrefsWork) && (!ed->ed_Cancelled))
       | 2558  41EB 030A                      LEA         030A(A3),A0
       | 255C  226D FFF8                      MOVEA.L     FFF8(A5),A1
       | 2560  B3C8                           CMPA.L      A0,A1
       | 2562  6618                           BNE.B       257C
       | 2564  4A6B 021A                      TST.W       021A(A3)
       | 2568  6612                           BNE.B       257C
;1713:     {
;1714:         ed->ed_CurrentDevice = NULL;
       | 256A  91C8                           SUBA.L      A0,A0
       | 256C  2748 0736                      MOVE.L      A0,0736(A3)
;1715:         ed->ed_CurrentLocRealm = NULL;
       | 2570  2748 073A                      MOVE.L      A0,073A(A3)
;1716:         ed->ed_CurrentRemRealm = NULL;
       | 2574  2748 073E                      MOVE.L      A0,073E(A3)
;1717:         ed->ed_CurrentRoute = NULL;
       | 2578  2748 0742                      MOVE.L      A0,0742(A3)
;1718:         RenderDisplay(ed);
;1719:     }
;1720: }
       | 257C  4CDF 4C00                      MOVEM.L     (A7)+,A2-A3/A6
       | 2580  4E5D                           UNLK        A5
       | 2582  4E75                           RTS
;1721: 
;1722: /*****************************************************************************/
;1723: 
;1724: BOOL DeviceRequest(EdDataPtr ed)
;1725: {
       | 2584  4E55 FFE8                      LINK        A5,#FFE8
       | 2588  48E7 3812                      MOVEM.L     D2-D4/A3/A6,-(A7)
       | 258C  2648                           MOVEA.L     A0,A3
;1726: BOOL        success;
;1727: struct Hook hook;
;1728: 
;1729:     hook.h_Entry = IntuiHook;    /* what should this be cast to to avoid warnings?? */
       | 258E  41FA  0000-XX.1                LEA         _IntuiHook(PC),A0
       | 2592  2B48 FFF4                      MOVE.L      A0,FFF4(A5)
;1730: 
;1731:     if (!ed->ed_DevReq)
       | 2596  4AAB 0746                      TST.L       0746(A3)
       | 259A  667C                           BNE.B       2618
;1732:     {
;1733:         if (!(ed->ed_DevReq = AllocPrefsRequest(ed, ASL_FileRequest,
;1734:                                                  ASLFR_RejectIcons,     TRUE,
;1735:                                                  ASL_Pattern,           "#?.device",
;1736:                                                  ASLFR_InitialLeftEdge, window->LeftEdge+12,
       | 259C  206B 01BC                      MOVEA.L     01BC(A3),A0
       | 25A0  3028 0004                      MOVE.W      0004(A0),D0
       | 25A4  48C0                           EXT.L       D0
       | 25A6  720C                           MOVEQ       #0C,D1
       | 25A8  D081                           ADD.L       D1,D0
;1737:                                                  ASLFR_InitialTopEdge,  window->TopEdge+12,
       | 25AA  3428 0006                      MOVE.W      0006(A0),D2
       | 25AE  48C2                           EXT.L       D2
       | 25B0  D481                           ADD.L       D1,D2
;1738:                                                  ASLFR_InitialDrawer,   "DEVS:Networks",
;1739:                                                  ASLFR_Window,          ed->ed_Window,
;1740:                                                  ASLFR_SleepWindow,     TRUE,
;1741:                                                  ASLFR_RejectIcons,     TRUE,
;1742:                                                  TAG_DONE)))
       | 25B2  7200                           MOVEQ       #00,D1
       | 25B4  2F01                           MOVE.L      D1,-(A7)
       | 25B6  7601                           MOVEQ       #01,D3
       | 25B8  2F03                           MOVE.L      D3,-(A7)
       | 25BA  283C 8008 003C                 MOVE.L      #8008003C,D4
       | 25C0  2F04                           MOVE.L      D4,-(A7)
       | 25C2  2F03                           MOVE.L      D3,-(A7)
       | 25C4  2F3C 8008 002B                 MOVE.L      #8008002B,-(A7)
       | 25CA  2F08                           MOVE.L      A0,-(A7)
       | 25CC  2F3C 8008 0002                 MOVE.L      #80080002,-(A7)
       | 25D2  487A DADA                      PEA         DADA(PC)
       | 25D6  2F3C 8008 0009                 MOVE.L      #80080009,-(A7)
       | 25DC  2F02                           MOVE.L      D2,-(A7)
       | 25DE  2F3C 8008 0004                 MOVE.L      #80080004,-(A7)
       | 25E4  2F00                           MOVE.L      D0,-(A7)
       | 25E6  2F3C 8008 0003                 MOVE.L      #80080003,-(A7)
       | 25EC  487A DAB6                      PEA         DAB6(PC)
       | 25F0  2F3C 8008 000A                 MOVE.L      #8008000A,-(A7)
       | 25F6  2F03                           MOVE.L      D3,-(A7)
       | 25F8  2F04                           MOVE.L      D4,-(A7)
       | 25FA  2F01                           MOVE.L      D1,-(A7)
       | 25FC  2F0B                           MOVE.L      A3,-(A7)
       | 25FE  4EBA  0000-XX.1                JSR         _AllocPrefsRequest(PC)
       | 2602  4FEF 004C                      LEA         004C(A7),A7
       | 2606  2740 0746                      MOVE.L      D0,0746(A3)
       | 260A  660C                           BNE.B       2618
;1743:         {
;1744:             ShowError2(ed,ES_NO_MEMORY);
       | 260C  7001                           MOVEQ       #01,D0
       | 260E  204B                           MOVEA.L     A3,A0
       | 2610  4EBA  0000-XX.1                JSR         @ShowError2(PC)
;1745:             return(FALSE);
       | 2614  7000                           MOVEQ       #00,D0
       | 2616  6070                           BRA.B       2688
;1746:         }
;1747:     }
;1748: 
;1749:     success = RequestDevice(ed,ASLFR_TitleText,    GetString(&ed->ed_LocaleInfo,MSG_REQ_LOAD),
       | 2618  41EB 06CA                      LEA         06CA(A3),A0
       | 261C  203C 0000 A411                 MOVE.L      #0000A411,D0
       | 2622  4EBA  0000-XX.1                JSR         @GetString(PC)
;1750:                                       ASLFR_DoSaveMode,   FALSE,
;1751:                                       ASLFR_IntuiMsgFunc, &hook,
;1752:                                       TAG_DONE);
       | 2626  7200                           MOVEQ       #00,D1
       | 2628  2F01                           MOVE.L      D1,-(A7)
       | 262A  486D FFEC                      PEA         FFEC(A5)
       | 262E  2F3C 8008 0046                 MOVE.L      #80080046,-(A7)
       | 2634  2F01                           MOVE.L      D1,-(A7)
       | 2636  2F3C 8008 002C                 MOVE.L      #8008002C,-(A7)
       | 263C  2F00                           MOVE.L      D0,-(A7)
       | 263E  2F3C 8008 0001                 MOVE.L      #80080001,-(A7)
       | 2644  2F0B                           MOVE.L      A3,-(A7)
       | 2646  6100 0048                      BSR.W       2690
       | 264A  4FEF 0020                      LEA         0020(A7),A7
;1753:     if (success)
       | 264E  4A40                           TST.W       D0
       | 2650  6734                           BEQ.B       2686
;1754:     {
;1755:         stccpy(ed->ed_NameBuf,ed->ed_DevReq->rf_Dir, NAMEBUFSIZE);
       | 2652  41EB 04CA                      LEA         04CA(A3),A0
       | 2656  226B 0746                      MOVEA.L     0746(A3),A1
       | 265A  2F48 0014                      MOVE.L      A0,0014(A7)
       | 265E  7040                           MOVEQ       #40,D0
       | 2660  E788                           LSL.L       #3,D0
       | 2662  2269 0008                      MOVEA.L     0008(A1),A1
       | 2666  4EBA  0000-XX.1                JSR         @stccpy(PC)
;1756:         AddPart(ed->ed_NameBuf,ed->ed_DevReq->rf_File, NAMEBUFSIZE);
       | 266A  206B 0746                      MOVEA.L     0746(A3),A0
       | 266E  222F 0014                      MOVE.L      0014(A7),D1
       | 2672  2428 0004                      MOVE.L      0004(A0),D2
       | 2676  7640                           MOVEQ       #40,D3
       | 2678  E78B                           LSL.L       #3,D3
       | 267A  2C6B 0008                      MOVEA.L     0008(A3),A6
       | 267E  4EAE FC8E                      JSR         FC8E(A6)
;1757:         return(TRUE);
       | 2682  7001                           MOVEQ       #01,D0
       | 2684  6002                           BRA.B       2688
;1758:     }
;1759: 
;1760:     return(FALSE);
       | 2686  7000                           MOVEQ       #00,D0
;1761: }
       | 2688  4CDF 481C                      MOVEM.L     (A7)+,D2-D4/A3/A6
       | 268C  4E5D                           UNLK        A5
       | 268E  4E75                           RTS
;1762: 
;1763: BOOL RequestDevice(EdDataPtr ed, ULONG tag1, ...)
;1764: {
       | 2690  4E55 0000                      LINK        A5,#0000
       | 2694  48E7 0012                      MOVEM.L     A3/A6,-(A7)
       | 2698  266D 0008                      MOVEA.L     0008(A5),A3
;1765:     return(AslRequest(ed->ed_DevReq,(struct TagItem *) &tag1));
       | 269C  206B 0746                      MOVEA.L     0746(A3),A0
       | 26A0  43ED 000C                      LEA         000C(A5),A1
       | 26A4  2C6B 0010                      MOVEA.L     0010(A3),A6
       | 26A8  4EAE FFC4                      JSR         FFC4(A6)
;1766: }
       | 26AC  4CDF 4800                      MOVEM.L     (A7)+,A3/A6
       | 26B0  4E5D                           UNLK        A5
       | 26B2  4E75                           RTS
;1767: 
;1768: /********************************************************************/
;1769: 
;1770: VOID AddDeviceReq(EdDataPtr ed)
;1771: {
       | 26B4  4E55 FFD4                      LINK        A5,#FFD4
       | 26B8  48E7 3132                      MOVEM.L     D2-D3/D7/A2-A3/A6,-(A7)
       | 26BC  2648                           MOVEA.L     A0,A3
;1772: struct NIPCDevice *old_dev;
;1773: struct Node *node;
;1774: struct IOSana2Req *ios2;
;1775: struct MsgPort *replyport;
;1776: struct Sana2DeviceQuery s2dq;
;1777: BPTR lock;
;1778: 
;1779:     old_dev = ed->ed_CurrentDevice;
       | 26BE  246B 0736                      MOVEA.L     0736(A3),A2
;1780:     if(ed->ed_CurrentDevice = AllocMem(sizeof(struct NIPCDevice),MEMF_CLEAR|MEMF_PUBLIC))
       | 26C2  7057                           MOVEQ       #57,D0
       | 26C4  E588                           LSL.L       #2,D0
       | 26C6  223C 0001 0001                 MOVE.L      #00010001,D1
       | 26CC  2C53                           MOVEA.L     (A3),A6
       | 26CE  4EAE FF3A                      JSR         FF3A(A6)
       | 26D2  2740 0736                      MOVE.L      D0,0736(A3)
       | 26D6  4A80                           TST.L       D0
       | 26D8  6700 019C                      BEQ.W       2876
;1781:     {
;1782:     	if(DeviceRequest(ed))
       | 26DC  204B                           MOVEA.L     A3,A0
       | 26DE  6100 FEA4                      BSR.W       2584
       | 26E2  4A40                           TST.W       D0
       | 26E4  6700 0168                      BEQ.W       284E
;1783:     	{
;1784: 	    ed->ed_DeviceList = DoPrefsGadget(ed,&EG[48],ed->ed_DeviceList,
;1785:                                     GTLV_Labels,       ~0);
       | 26E8  4878 FFFF                      PEA         FFFF
       | 26EC  2F3C 8008 0006                 MOVE.L      #80080006,-(A7)
       | 26F2  2F2B 0806                      MOVE.L      0806(A3),-(A7)
       | 26F6  4879  0000 055E-01             PEA         01.0000055E
       | 26FC  2F0B                           MOVE.L      A3,-(A7)
       | 26FE  4EBA  0000-XX.1                JSR         _DoPrefsGadget(PC)
       | 2702  4FEF 0014                      LEA         0014(A7),A7
       | 2706  2740 0806                      MOVE.L      D0,0806(A3)
;1786: 
;1787: 	    if(lock = Lock(ed->ed_NameBuf,ACCESS_READ))
       | 270A  45EB 04CA                      LEA         04CA(A3),A2
       | 270E  220A                           MOVE.L      A2,D1
       | 2710  74FE                           MOVEQ       #FE,D2
       | 2712  2C6B 0008                      MOVEA.L     0008(A3),A6
       | 2716  4EAE FFAC                      JSR         FFAC(A6)
       | 271A  2E00                           MOVE.L      D0,D7
       | 271C  4A87                           TST.L       D7
       | 271E  6700 015A                      BEQ.W       287A
;1788: 	    {
;1789: 	    	if(NameFromLock(lock,ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,256))
       | 2722  206B 0736                      MOVEA.L     0736(A3),A0
       | 2726  D0FC 005B                      ADDA.W      #005B,A0
       | 272A  2408                           MOVE.L      A0,D2
       | 272C  2207                           MOVE.L      D7,D1
       | 272E  7640                           MOVEQ       #40,D3
       | 2730  E58B                           LSL.L       #2,D3
       | 2732  4EAE FE6E                      JSR         FE6E(A6)
       | 2736  4A80                           TST.L       D0
       | 2738  6700 0108                      BEQ.W       2842
;1790: 	    	{
;1791:                     strcpy(ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,ed->ed_NameBuf);
       | 273C  206B 0736                      MOVEA.L     0736(A3),A0
       | 2740  D0FC 005B                      ADDA.W      #005B,A0
       | 2744  224A                           MOVEA.L     A2,A1
       | 2746  10D9                           MOVE.B      (A1)+,(A0)+
       | 2748  66FC                           BNE.B       2746
;1792:                     AddTail(&ed->ed_PrefsWork.ep_NIPCDevices,(struct Node *)ed->ed_CurrentDevice);
       | 274A  41EB 030A                      LEA         030A(A3),A0
       | 274E  226B 0736                      MOVEA.L     0736(A3),A1
       | 2752  2C53                           MOVEA.L     (A3),A6
       | 2754  4EAE FF0A                      JSR         FF0A(A6)
;1793:                     node = (struct Node *)ed->ed_CurrentDevice;
       | 2758  206B 0736                      MOVEA.L     0736(A3),A0
;1794:                     node->ln_Name = (STRPTR) &ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName[0];
       | 275C  43E8 005B                      LEA         005B(A0),A1
       | 2760  2149 000A                      MOVE.L      A1,000A(A0)
;1795:                     if(replyport = CreateMsgPort())
       | 2764  2C53                           MOVEA.L     (A3),A6
       | 2766  4EAE FD66                      JSR         FD66(A6)
       | 276A  2F40 0018                      MOVE.L      D0,0018(A7)
       | 276E  4A80                           TST.L       D0
       | 2770  6700 00D0                      BEQ.W       2842
;1796:                     {
;1797:                         if(ios2 = CreateIORequest(replyport,sizeof(struct IOSana2Req)))
       | 2774  2040                           MOVEA.L     D0,A0
       | 2776  7058                           MOVEQ       #58,D0
       | 2778  4EAE FD72                      JSR         FD72(A6)
       | 277C  2440                           MOVEA.L     D0,A2
       | 277E  200A                           MOVE.L      A2,D0
       | 2780  6700 00B6                      BEQ.W       2838
;1798:                         {
;1799:                             ios2->ios2_BufferManagement = BMTags;
       | 2784  257C  0000 0030-01  0054       MOVE.L      #01.00000030,0054(A2)
;1800: 
;1801:                             if(!OpenDevice(ed->ed_CurrentDevice->nd_Prefs.ndp_DevPathName,0,(struct IORequest *)ios2,0))
       | 278C  206B 0736                      MOVEA.L     0736(A3),A0
       | 2790  D0FC 005B                      ADDA.W      #005B,A0
       | 2794  7000                           MOVEQ       #00,D0
       | 2796  224A                           MOVEA.L     A2,A1
       | 2798  2200                           MOVE.L      D0,D1
       | 279A  2C53                           MOVEA.L     (A3),A6
       | 279C  4EAE FE44                      JSR         FE44(A6)
       | 27A0  4A00                           TST.B       D0
       | 27A2  6600 008C                      BNE.W       2830
;1802:                             {
;1803:                                 ios2->ios2_Req.io_Command = S2_DEVICEQUERY;
       | 27A6  357C 0009 001C                 MOVE.W      #0009,001C(A2)
;1804:                                 ios2->ios2_StatData = &s2dq;
       | 27AC  41ED FFDA                      LEA         FFDA(A5),A0
       | 27B0  2548 0050                      MOVE.L      A0,0050(A2)
;1805:                                 s2dq.SizeAvailable = sizeof(struct Sana2DeviceQuery);
       | 27B4  701E                           MOVEQ       #1E,D0
       | 27B6  2B40 FFDA                      MOVE.L      D0,FFDA(A5)
;1806:                                 s2dq.DevQueryFormat = 0;
       | 27BA  7000                           MOVEQ       #00,D0
       | 27BC  2B40 FFE2                      MOVE.L      D0,FFE2(A5)
;1807:                                 s2dq.DeviceLevel = 0;
       | 27C0  2B40 FFE6                      MOVE.L      D0,FFE6(A5)
;1808:                                 DoIO((struct IORequest *)ios2);
       | 27C4  224A                           MOVEA.L     A2,A1
       | 27C6  2C53                           MOVEA.L     (A3),A6
       | 27C8  4EAE FE38                      JSR         FE38(A6)
;1809:                                 switch(s2dq.HardwareType)
       | 27CC  202D FFF4                      MOVE.L      FFF4(A5),D0
       | 27D0  5380                           SUBQ.L      #1,D0
       | 27D2  6712                           BEQ.B       27E6
       | 27D4  5D80                           SUBQ.L      #6,D0
       | 27D6  672A                           BEQ.B       2802
       | 27D8  0480 0000 00F7                 SUBI.L      #000000F7,D0
       | 27DE  673E                           BEQ.B       281E
       | 27E0  5380                           SUBQ.L      #1,D0
       | 27E2  673A                           BEQ.B       281E
       | 27E4  6042                           BRA.B       2828
;1810:                                 {
;1811:                                         case S2WireType_Ethernet:       ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = 2048;
       | 27E6  206B 0736                      MOVEA.L     0736(A3),A0
       | 27EA  217C 0000 0800 000E            MOVE.L      #00000800,000E(A0)
;1812:                                                                         ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = 2054;
       | 27F2  217C 0000 0806 0012            MOVE.L      #00000806,0012(A0)
;1813:                                                                         ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
       | 27FA  7018                           MOVEQ       #18,D0
       | 27FC  8128 005A                      OR.B        D0,005A(A0)
;1814:                                                                         break;
       | 2800  6026                           BRA.B       2828
;1815:                                         case S2WireType_Arcnet:         ed->ed_CurrentDevice->nd_Prefs.ndp_IPType = 240;
       | 2802  206B 0736                      MOVEA.L     0736(A3),A0
       | 2806  217C 0000 00F0 000E            MOVE.L      #000000F0,000E(A0)
;1816:                                                                         ed->ed_CurrentDevice->nd_Prefs.ndp_ARPType = 241;
       | 280E  217C 0000 00F1 0012            MOVE.L      #000000F1,0012(A0)
;1817:                                                                         ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
       | 2816  7018                           MOVEQ       #18,D0
       | 2818  8128 005A                      OR.B        D0,005A(A0)
;1818:                                                                         break;
       | 281C  600A                           BRA.B       2828
;1819:                                         case S2WireType_SLIP:
;1820:                                         case S2WireType_CSLIP:          ed->ed_CurrentDevice->nd_Prefs.ndp_Flags |= (NDPFLAGF_IPTYPE | NDPFLAGF_ARPTYPE);
       | 281E  206B 0736                      MOVEA.L     0736(A3),A0
       | 2822  7018                           MOVEQ       #18,D0
       | 2824  8128 005A                      OR.B        D0,005A(A0)
;1821:                                                                         break;
;1822: 
;1823:                                         default:                        break;
;1824:                                 }
;1825:                                 CloseDevice((struct IORequest *)ios2);
       | 2828  224A                           MOVEA.L     A2,A1
       | 282A  2C53                           MOVEA.L     (A3),A6
       | 282C  4EAE FE3E                      JSR         FE3E(A6)
;1826:                             }
;1827:                             DeleteIORequest((struct IORequest *)ios2);
       | 2830  204A                           MOVEA.L     A2,A0
       | 2832  2C53                           MOVEA.L     (A3),A6
       | 2834  4EAE FD6C                      JSR         FD6C(A6)
;1828:                         }
;1829:                         DeleteMsgPort(replyport);
       | 2838  206F 0018                      MOVEA.L     0018(A7),A0
       | 283C  2C53                           MOVEA.L     (A3),A6
       | 283E  4EAE FD60                      JSR         FD60(A6)
;1830:                     }
;1831:                 }
;1832:                 UnLock(lock);
       | 2842  2207                           MOVE.L      D7,D1
       | 2844  2C6B 0008                      MOVEA.L     0008(A3),A6
       | 2848  4EAE FFA6                      JSR         FFA6(A6)
;1833:             }
;1834:             return;
       | 284C  602C                           BRA.B       287A
;1835:         }
;1836:         if(ed->ed_DevReq)
       | 284E  4AAB 0746                      TST.L       0746(A3)
       | 2852  670C                           BEQ.B       2860
;1837:             FreeAslRequest(ed->ed_DevReq);
       | 2854  206B 0746                      MOVEA.L     0746(A3),A0
       | 2858  2C6B 0010                      MOVEA.L     0010(A3),A6
       | 285C  4EAE FFCA                      JSR         FFCA(A6)
;1838:         ed->ed_DevReq = NULL;
       | 2860  42AB 0746                      CLR.L       0746(A3)
;1839:         FreeMem(ed->ed_CurrentDevice,sizeof(struct NIPCDevice));
       | 2864  226B 0736                      MOVEA.L     0736(A3),A1
       | 2868  7057                           MOVEQ       #57,D0
       | 286A  E588                           LSL.L       #2,D0
       | 286C  2C53                           MOVEA.L     (A3),A6
       | 286E  4EAE FF2E                      JSR         FF2E(A6)
;1840:         ed->ed_CurrentDevice = old_dev;
       | 2872  274A 0736                      MOVE.L      A2,0736(A3)
;1841:     }
;1842:     ed->ed_CurrentDevice = old_dev;
       | 2876  274A 0736                      MOVE.L      A2,0736(A3)
;1843: }
       | 287A  4CDF 4C8C                      MOVEM.L     (A7)+,D2-D3/D7/A2-A3/A6
       | 287E  4E5D                           UNLK        A5
       | 2880  4E75                           RTS
;1844: 
;1845: /*****************************************************************************/
;1846: 
;1847: #define ASM __asm
;1848: #define REG(x) register __## x
;1849: 
;1850: ULONG ASM StringHook(REG(a0) struct Hook *hook,
;1851:                    REG(a2) struct SGWork *sgw,
;1852:                    REG(a1) ULONG *msg)
;1853: {
       | 2882  4E55 FFF8                      LINK        A5,#FFF8
       | 2886  48E7 0130                      MOVEM.L     D7/A2-A3,-(A7)
       | 288A  2649                           MOVEA.L     A1,A3
       | 288C  2B48 FFF8                      MOVE.L      A0,FFF8(A5)
;1854: UBYTE *work_ptr;
;1855: ULONG return_code;
;1856: 
;1857:     return_code = ~0L;
       | 2890  7EFF                           MOVEQ       #FF,D7
;1858: 
;1859:     if(*msg == SGH_KEY)
       | 2892  7001                           MOVEQ       #01,D0
       | 2894  B093                           CMP.L       (A3),D0
       | 2896  662E                           BNE.B       28C6
;1860:     {
;1861:     	if((sgw->EditOp == EO_REPLACECHAR) ||
       | 2898  302A 002A                      MOVE.W      002A(A2),D0
       | 289C  7207                           MOVEQ       #07,D1
       | 289E  B041                           CMP.W       D1,D0
       | 28A0  6704                           BEQ.B       28A6
;1862:     	   (sgw->EditOp == EO_INSERTCHAR))
       | 28A2  5140                           SUBQ.W      #8,D0
       | 28A4  6622                           BNE.B       28C8
;1863:     	{
;1864:     	    if(!IsIPDigit(sgw->Code))
       | 28A6  302A 0018                      MOVE.W      0018(A2),D0
       | 28AA  7200                           MOVEQ       #00,D1
       | 28AC  1200                           MOVE.B      D0,D1
       | 28AE  2001                           MOVE.L      D1,D0
       | 28B0  6100 0020                      BSR.W       28D2
       | 28B4  4A40                           TST.W       D0
       | 28B6  6610                           BNE.B       28C8
;1865:     	    {
;1866:     	    	sgw->Actions |= SGA_BEEP;
       | 28B8  08EA 0002 0021                 BSET        #0002,0021(A2)
;1867:     	    	sgw->Actions &= ~SGA_USE;
       | 28BE  08AA 0000 0021                 BCLR        #0000,0021(A2)
;1868:     	    }
;1869:     	}
;1870:     }
;1871:     else
       | 28C4  6002                           BRA.B       28C8
;1872:     	return_code = 0;
       | 28C6  7E00                           MOVEQ       #00,D7
;1873: 
;1874:     return(return_code);
       | 28C8  2007                           MOVE.L      D7,D0
;1875: }
       | 28CA  4CDF 0C80                      MOVEM.L     (A7)+,D7/A2-A3
       | 28CE  4E5D                           UNLK        A5
       | 28D0  4E75                           RTS
;1876: 
;1877: BOOL IsIPDigit(UBYTE test_char)
;1878: {
       | 28D2  2F07                           MOVE.L      D7,-(A7)
       | 28D4  2E00                           MOVE.L      D0,D7
;1879:     if(((test_char >= '0') && (test_char <= '9')) ||
       | 28D6  7030                           MOVEQ       #30,D0
       | 28D8  BE00                           CMP.B       D0,D7
       | 28DA  6506                           BCS.B       28E2
       | 28DC  7039                           MOVEQ       #39,D0
       | 28DE  BE00                           CMP.B       D0,D7
       | 28E0  6306                           BLS.B       28E8
;1880:        (test_char == '.'))
       | 28E2  702E                           MOVEQ       #2E,D0
       | 28E4  BE00                           CMP.B       D0,D7
       | 28E6  6604                           BNE.B       28EC
;1881:     	return TRUE;
       | 28E8  7001                           MOVEQ       #01,D0
       | 28EA  6002                           BRA.B       28EE
;1882:     else
;1883:     	return FALSE;
       | 28EC  7000                           MOVEQ       #00,D0
;1884: }
       | 28EE  2E1F                           MOVE.L      (A7)+,D7
       | 28F0  4E75                           RTS
;1885: 
;1886: /*****************************************************************************/
;1887: 
;1888: BOOL DummyBuffFunc(APTR to, APTR from, LONG length)
;1889: {
       | 28F2  4E55 FFF4                      LINK        A5,#FFF4
       | 28F6  48ED 0301 FFF4                 MOVEM.L     D0/A0-A1,FFF4(A5)
;1890: 	return TRUE;
       | 28FC  7001                           MOVEQ       #01,D0
;1891: };
       | 28FE  4E5D                           UNLK        A5
       | 2900  4E75                           RTS

SECTION 01 " " 000005F0 BYTES
0000 50 52 45 46 50 52 48 44 50 52 45 46 4E 44 45 56 PREFPRHDPREFNDEV
0010 50 52 45 46 4E 52 52 4D 50 52 45 46 4E 4C 52 4D PREFNRRMPREFNLRM
0020 50 52 45 46 4E 49 52 54 50 52 45 46 48 4F 53 54 PREFNIRTPREFHOST
0030 80 0B 00 01 ....
0034 000028F2-00 00.000028F2
0038 80 0B 00 02 ....
003C 000028F2-00 00.000028F2
0040 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 ................
0050 00 00 00 04 00 00 00 00 00 00 02 00 00 00 00 05 ................
0060 00 00 00 01 00 00 02 00 00 00 00 06 00 00 00 03 ................
0070 00 00 02 00 00 00 00 00 00 00 00 00 00 00 02 00 ................
0080 00 00 00 07 00 00 00 09 00 00 01 00 00 00 00 08 ................
0090 00 00 00 00 00 00 02 00 00 00 00 0E 00 00 00 06 ................
00A0 00 00 02 00 00 00 00 0F 00 00 00 04 00 00 02 00 ................
00B0 00 00 00 10 00 00 00 05 00 00 01 00 00 00 00 11 ................
00C0 00 00 00 00 00 00 02 00 00 00 00 12 00 00 00 07 ................
00D0 00 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00E0 00 01 00 0A 00 A0 00 98 00 0E 00 00 00 13 00 00 ..... ..........
00F0 00 02 00 00 00 00 00 00 00 01 00 E0 00 A0 00 98 ...........`. ..
0100 00 0E 00 00 00 14 00 00 00 08 00 00 00 00 00 00 ................
0110 00 01 01 BC 00 A0 00 98 00 0E 00 00 00 15 00 00 ...<. ..........
0120 00 09 00 00 00 00 00 00 00 01 01 92 00 1A 00 BC ...............<
0130 00 18 00 00 00 00 00 00 00 0F 00 00 00 00 00 00 ................
0140 00 01 01 92 00 42 00 BC 00 18 00 00 00 00 00 00 .....B.<........
0150 00 11 00 00 00 00 00 00 00 01 01 92 00 6A 00 BC .............j.<
0160 00 18 00 00 00 00 00 00 00 0E 00 00 00 00 00 00 ................
0170 00 0C 00 9C 00 1A 00 A4 00 0E 00 00 A4 17 00 00 .......$....$...
0180 00 0B 00 00 00 00 00 00 00 0C 00 9C 00 50 00 A4 .............P.$
0190 00 0E 00 00 A4 1B 00 00 00 0C 00 00 00 00 00 00 ....$...........
01A0 00 0C 00 9C 00 60 00 A4 00 0E 00 00 A4 1C 00 00 .....`.$....$...
01B0 00 0D 00 00 00 00 00 00 00 07 00 0A 00 04 00 C8 ...............H
01C0 00 0E 00 00 00 00 00 00 00 3B 00 00 00 00 00 00 .........;......
01D0 00 01 01 BC 00 A0 00 98 00 0E 00 00 00 00 00 00 ...<. ..........
01E0 00 3A 00 00 00 00 00 00 00 01 00 0C 00 8E 00 84 .:..............
01F0 00 0E 00 00 A4 1D 00 00 00 31 00 00 00 00 00 00 ....$....1......
0200 00 01 00 94 00 8E 00 84 00 0E 00 00 A4 1E 00 00 ............$...
0210 00 32 00 00 00 00 00 00 00 01 01 36 00 8E 00 84 .2.........6....
0220 00 0E 00 00 A4 1D 00 00 00 36 00 00 00 00 00 00 ....$....6......
0230 00 01 01 BE 00 8E 00 84 00 0E 00 00 A4 1E 00 00 ...>........$...
0240 00 37 00 00 00 00 00 00 00 0D 00 0A 00 16 01 10 .7..............
0250 00 0C 00 00 A4 1F 00 00 00 00 00 00 00 30 00 00 ....$........0..
0260 00 0D 01 34 00 16 01 10 00 0C 00 00 A4 20 00 00 ...4........$ ..
0270 00 00 00 00 00 30 00 00 00 0C 00 0A 00 7E 00 88 .....0.......~..
0280 00 0E 00 00 00 00 00 00 00 2F 00 00 00 00 00 00 ........./......
0290 00 0C 00 92 00 7E 00 88 00 0E 00 00 00 00 00 00 .....~..........
02A0 00 30 00 00 00 00 00 00 00 0C 01 34 00 7E 00 88 .0.........4.~..
02B0 00 0E 00 00 00 00 00 00 00 34 00 00 00 00 00 00 .........4......
02C0 00 0C 01 BC 00 7E 00 88 00 0E 00 00 00 00 00 00 ...<.~..........
02D0 00 35 00 00 00 00 00 00 00 04 00 0A 00 32 01 22 .5...........2."
02E0 00 4C 00 00 A4 21 00 00 00 2E 00 00 00 04 00 00 .L..$!..........
02F0 00 04 01 34 00 32 01 22 00 4C 00 00 A4 22 00 00 ...4.2.".L..$"..
0300 00 33 00 00 00 04 00 00 00 01 00 0A 00 A0 00 98 .3........... ..
0310 00 0E 00 00 00 00 00 00 00 2B 00 00 00 00 00 00 .........+......
0320 00 01 01 BC 00 A0 00 98 00 0E 00 00 00 00 00 00 ...<. ..........
0330 00 2D 00 00 00 00 00 00 00 01 00 92 00 82 00 A8 .-.............(
0340 00 0E 00 00 A4 23 00 00 00 29 00 00 00 00 00 00 ....$#...)......
0350 00 01 01 3A 00 82 00 A8 00 0E 00 00 A4 24 00 00 ...:...(....$$..
0360 00 2A 00 00 00 00 00 00 00 0C 00 92 00 74 00 A8 .*...........t.(
0370 00 0E 00 00 00 00 00 00 00 26 00 00 00 00 00 00 .........&......
0380 00 0C 01 3A 00 74 00 A8 00 0E 00 00 00 00 00 00 ...:.t.(........
0390 00 27 00 00 00 00 00 00 00 0D 00 92 00 1A 00 A8 .'.............(
03A0 00 0E 00 00 A4 25 00 00 00 00 00 00 00 10 00 00 ....$%..........
03B0 00 04 00 92 00 28 01 50 00 4C 00 00 00 00 00 00 .....(.P.L......
03C0 00 25 00 00 00 00 00 00 00 01 00 0A 00 A0 00 98 .%........... ..
03D0 00 0E 00 00 00 00 00 00 00 22 00 00 00 00 00 00 ........."......
03E0 00 01 01 BC 00 A0 00 98 00 0E 00 00 00 00 00 00 ...<. ..........
03F0 00 24 00 00 00 00 00 00 00 01 00 0A 00 7E 00 6E .$...........~.n
0400 00 0E 00 00 A4 27 00 00 00 15 00 00 00 00 00 00 ....$'..........
0410 00 01 00 78 00 7E 00 6E 00 0E 00 00 A4 28 00 00 ...x.~.n....$(..
0420 00 16 00 00 00 00 00 00 00 0D 00 F0 00 08 01 64 ...........p...d
0430 00 10 00 00 A4 29 00 00 00 00 00 00 00 30 00 00 ....$).......0..
0440 00 07 01 6C 00 78 00 6E 00 0E 00 00 A4 2A 00 00 ...l.x.n....$*..
0450 00 1D 00 00 00 00 00 00 00 02 01 DC 00 2A 00 1A ...........\.*..
0460 00 0B 00 00 A4 2E 00 00 00 1E 00 00 00 02 00 00 ....$...........
0470 00 02 01 DC 00 3A 00 1A 00 0B 00 00 A4 2E 00 00 ...\.:......$...
0480 00 21 00 00 00 02 00 00 00 02 01 DC 00 4A 00 1A .!.........\.J..
0490 00 0B 00 00 A4 2E 00 00 00 1F 00 00 00 02 00 00 ....$...........
04A0 00 02 01 DC 00 5A 00 1A 00 0B 00 00 A4 2E 00 00 ...\.Z......$...
04B0 00 20 00 00 00 02 00 00 00 0C 00 0A 00 64 00 DC . ...........d.\
04C0 00 0E 00 00 00 00 00 00 00 14 00 00 00 00 00 00 ................
04D0 00 03 01 92 00 68 00 48 00 0E 00 00 A4 2B 00 00 .....h.H....$+..
04E0 00 17 00 00 00 00 00 00 00 0C 01 52 00 18 00 88 ...........R....
04F0 00 0E 00 00 A4 2F 00 00 00 18 00 00 00 00 00 00 ....$/..........
0500 00 0C 01 52 00 28 00 88 00 0E 00 00 A4 30 00 00 ...R.(......$0..
0510 00 19 00 00 00 00 00 00 00 0C 01 52 00 38 00 88 ...........R.8..
0520 00 0E 00 00 A4 31 00 00 00 1A 00 00 00 00 00 00 ....$1..........
0530 00 03 01 92 00 48 00 48 00 0E 00 00 A4 2C 00 00 .....H.H....$,..
0540 00 1B 00 00 00 00 00 00 00 03 01 92 00 58 00 48 .............X.H
0550 00 0E 00 00 A4 2D 00 00 00 1C 00 00 00 00 00 00 ....$-..........
0560 00 04 00 0A 00 18 00 DC 00 74 00 00 00 00 00 00 .......\.t......
0570 00 13 00 00 00 04 00 00 00 04 00 EC 00 68 01 68 ...........l.h.h
0580 00 34 00 00 A4 33 00 00 00 00 00 00 00 04 00 00 .4..$3..........
0590 00 0C 00 9C 00 2A 00 A4 00 0E 00 00 A4 18 00 00 .....*.$....$...
05A0 00 10 00 00 00 00 00 00 00 02 02 34 00 1A 00 1A ...........4....
05B0 00 0B 00 00 A4 19 00 00 00 11 00 00 00 00 00 00 ....$...........
05C0 00 02 02 34 00 2A 00 1A 00 0B 00 00 A4 1A 00 00 ...4.*......$...
05D0 00 12 00 00 00 00 00 00 00 0D 01 3A 00 1A 00 A8 ...........:...(
05E0 00 0E 00 00 A4 26 00 00 00 00 00 00 00 10 00 00 ....$&..........
