SAS AMIGA 680x0OBJ Module Disassembler 6.50
Copyright © 1993 SAS Institute, Inc.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_OpenTest 0000-00    _SysBase 0000-02    _DOSBase 0004-02    
_IntuitionBase 0008-02    _GfxBase 000C-02

SECTION 00 "text" 000007A0 BYTES
;   1: #include <exec/types.h>
;   2: #include <exec/memory.h>
;   3: #include <dos/dos.h>
;   4: #include <dos/dosextens.h>
;   5: #include <dos/dostags.h>
;   6: #include <dos/rdargs.h>
;   7: #include <devices/cd.h>
;   8: #include <graphics/rastport.h>
;   9: #include <clib/exec_protos.h>
;  10: #include <clib/intuition_protos.h>
;  11: #include <clib/dos_protos.h>
;  12: #include <clib/graphics_protos.h>
;  13: #include <pragmas/exec_pragmas.h>
;  14: #include <pragmas/intuition_pragmas.h>
;  15: #include <pragmas/dos_pragmas.h>
;  16: #include <pragmas/graphics_pragmas.h>
;  17: #include "/mpeg.h"
;  18: #include "asyncio.h"
;  19: 
;  20: #define TEMPLATE	"TRACK/N"
;  21: #define DATA_SIZE	2328
;  22: 
;  23: struct Library *SysBase,*DOSBase,*IntuitionBase,*GfxBase;
;  24: 
;  25: ULONG OpenTest(VOID)
       | 0000  9EFC 006C                      SUBA.W      #006C,A7
       | 0004  48E7 3F36                      MOVEM.L     D2-D7/A2-A3/A5-A6,-(A7)
;  26: {
;  27:     struct RDArgs *rdargs;
;  28:     struct MsgPort *port,*data_port;
;  29:     struct IOMPEGReq *iomr_cmd,*iomr_data;
;  30:     struct IOStdReq *iocd;
;  31:     LONG args[4]={0,0,0,0};
       | 0008  41F9  0000 0000-01.2           LEA         01.00000000,A0
       | 000E  43EF 0078                      LEA         0078(A7),A1
       | 0012  22D8                           MOVE.L      (A0)+,(A1)+
       | 0014  22D8                           MOVE.L      (A0)+,(A1)+
       | 0016  22D8                           MOVE.L      (A0)+,(A1)+
       | 0018  22D8                           MOVE.L      (A0)+,(A1)+
;  32:     struct AsyncFile *file;
;  33:     ULONG iocnt=0,cnt;
       | 001A  42AF 0070                      CLR.L       0070(A7)
;  34:     ULONG streamType = 0;
       | 001E  42AF 0068                      CLR.L       0068(A7)
;  35:     WORD showVParams=100;
       | 0022  3F7C 0064 002A                 MOVE.W      #0064,002A(A7)
;  36:     BOOL done=FALSE;
       | 0028  7E00                           MOVEQ       #00,D7
;  37:     ULONG signals,waitsignals;
;  38:     struct Screen *myScreen;
;  39:     struct Window *myWindow;
;  40:     union CDTOC *toc;
;  41:     struct MPEGFrameStore mfs;
;  42:     struct MPEGStreamInfo msi;
;  43:     ULONG secTags[] = { TAGCD_SECTORSIZE, 2328, TAG_END, TRUE };
       | 002A  41F9  0000 0010-01.2           LEA         01.00000010,A0
       | 0030  43EF 0030                      LEA         0030(A7),A1
       | 0034  22D8                           MOVE.L      (A0)+,(A1)+
       | 0036  22D8                           MOVE.L      (A0)+,(A1)+
       | 0038  22D8                           MOVE.L      (A0)+,(A1)+
       | 003A  22D8                           MOVE.L      (A0)+,(A1)+
;  44: 
;  45:     SysBase = *(struct Library **)4L;
       | 003C  307C 0004                      MOVEA.W     #0004,A0
       | 0040  2250                           MOVEA.L     (A0),A1
       | 0042  23C9  0000 0000-02             MOVE.L      A1,02.00000000
;  46: 
;  47:     if(DOSBase = OpenLibrary("dos.library",37L))
       | 0048  43F9  0000 0020-01.2           LEA         01.00000020,A1
       | 004E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0054  7025                           MOVEQ       #25,D0
       | 0056  4EAE FDD8                      JSR         FDD8(A6)
       | 005A  23C0  0000 0004-02             MOVE.L      D0,02.00000004
       | 0060  6700 0732                      BEQ.W       0794
;  48:     {
;  49:         if(IntuitionBase = OpenLibrary("intuition.library",37L))
       | 0064  43F9  0000 002C-01.2           LEA         01.0000002C,A1
       | 006A  7025                           MOVEQ       #25,D0
       | 006C  4EAE FDD8                      JSR         FDD8(A6)
       | 0070  23C0  0000 0008-02             MOVE.L      D0,02.00000008
       | 0076  6700 071C                      BEQ.W       0794
;  50:         {
;  51:             if(GfxBase = OpenLibrary("graphics.library",40L))
       | 007A  43F9  0000 003E-01.2           LEA         01.0000003E,A1
       | 0080  7028                           MOVEQ       #28,D0
       | 0082  4EAE FDD8                      JSR         FDD8(A6)
       | 0086  23C0  0000 000C-02             MOVE.L      D0,02.0000000C
       | 008C  6700 06F6                      BEQ.W       0784
;  52:             {
;  53:                 if(rdargs = ReadArgs(TEMPLATE, args, NULL))
       | 0090  41F9  0000 0050-01.2           LEA         01.00000050,A0
       | 0096  2208                           MOVE.L      A0,D1
       | 0098  41EF 0078                      LEA         0078(A7),A0
       | 009C  2408                           MOVE.L      A0,D2
       | 009E  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 00A4  7600                           MOVEQ       #00,D3
       | 00A6  4EAE FCE2                      JSR         FCE2(A6)
       | 00AA  2A40                           MOVEA.L     D0,A5
       | 00AC  200D                           MOVE.L      A5,D0
       | 00AE  6700 06C4                      BEQ.W       0774
;  54:                 {
;  55:                     if(port = CreateMsgPort())
       | 00B2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 00B8  4EAE FD66                      JSR         FD66(A6)
       | 00BC  2640                           MOVEA.L     D0,A3
       | 00BE  200B                           MOVE.L      A3,D0
       | 00C0  6700 06A2                      BEQ.W       0764
;  56:                     {
;  57:                         if(data_port = CreateMsgPort())
       | 00C4  4EAE FD66                      JSR         FD66(A6)
       | 00C8  2440                           MOVEA.L     D0,A2
       | 00CA  200A                           MOVE.L      A2,D0
       | 00CC  6700 068A                      BEQ.W       0758
;  58:                         {
;  59:                             if(iomr_cmd = (struct IOMPEGReq *)CreateIORequest(port,sizeof(struct IOMPEGReq)))
       | 00D0  204B                           MOVEA.L     A3,A0
       | 00D2  7056                           MOVEQ       #56,D0
       | 00D4  4EAE FD72                      JSR         FD72(A6)
       | 00D8  2F40 0090                      MOVE.L      D0,0090(A7)
       | 00DC  6700 066E                      BEQ.W       074C
;  60:                             {
;  61:                                 if(iocd = (struct IOStdReq *)CreateIORequest(port,sizeof(struct IOStdReq)))
       | 00E0  204B                           MOVEA.L     A3,A0
       | 00E2  7030                           MOVEQ       #30,D0
       | 00E4  4EAE FD72                      JSR         FD72(A6)
       | 00E8  2F40 0088                      MOVE.L      D0,0088(A7)
       | 00EC  6700 0650                      BEQ.W       073E
;  62:                                 {
;  63:                                     if(!OpenDevice("cd.device",0,(struct IORequest *)iocd,0))
       | 00F0  2240                           MOVEA.L     D0,A1
       | 00F2  41F9  0000 0058-01.2           LEA         01.00000058,A0
       | 00F8  7000                           MOVEQ       #00,D0
       | 00FA  2200                           MOVE.L      D0,D1
       | 00FC  4EAE FE44                      JSR         FE44(A6)
       | 0100  4A00                           TST.B       D0
       | 0102  6600 062C                      BNE.W       0730
;  64:                                     {
;  65:                                         Printf("cd.device opened.\n");
       | 0106  41F9  0000 0062-01.2           LEA         01.00000062,A0
       | 010C  2208                           MOVE.L      A0,D1
       | 010E  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 0114  4EAE FC46                      JSR         FC46(A6)
;  66: 
;  67:                                         if(toc = AllocVec(20*sizeof(union CDTOC),MEMF_CLEAR))
       | 0118  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 011E  7078                           MOVEQ       #78,D0
       | 0120  7201                           MOVEQ       #01,D1
       | 0122  4841                           SWAP        D1
       | 0124  4EAE FD54                      JSR         FD54(A6)
       | 0128  2F40 005C                      MOVE.L      D0,005C(A7)
       | 012C  4A80                           TST.L       D0
       | 012E  6700 05F2                      BEQ.W       0722
;  68:                                         {
;  69:                                                 Printf("Got TOC memory.\n");
       | 0132  41F9  0000 0076-01.2           LEA         01.00000076,A0
       | 0138  2208                           MOVE.L      A0,D1
       | 013A  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 0140  4EAE FC46                      JSR         FC46(A6)
;  70: 
;  71:                                             iocd->io_Command = CD_TOCLSN;
       | 0144  206F 0088                      MOVEA.L     0088(A7),A0
       | 0148  317C 0023 001C                 MOVE.W      #0023,001C(A0)
;  72:                                             iocd->io_Data = (APTR)toc;
       | 014E  216F 005C 0028                 MOVE.L      005C(A7),0028(A0)
;  73:                                             iocd->io_Length = 20;
       | 0154  7014                           MOVEQ       #14,D0
       | 0156  2140 0024                      MOVE.L      D0,0024(A0)
;  74: 
;  75:                                             if(!DoIO((struct IORequest *)iocd))
       | 015A  2248                           MOVEA.L     A0,A1
       | 015C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0162  4EAE FE38                      JSR         FE38(A6)
       | 0166  4A00                           TST.B       D0
       | 0168  6600 05B8                      BNE.W       0722
;  76:                                             {
;  77:                                                 Printf("Read TOC\n");
       | 016C  41F9  0000 0088-01.2           LEA         01.00000088,A0
       | 0172  2208                           MOVE.L      A0,D1
       | 0174  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 017A  4EAE FC46                      JSR         FC46(A6)
;  78: 
;  79:                                                 /* Ick */
;  80: 
;  81:                                                 toc[toc[0].Summary.LastTrack+1].Entry.Position.LSN = toc[0].Summary.LeadOut.LSN;
       | 017E  7000                           MOVEQ       #00,D0
       | 0180  206F 005C                      MOVEA.L     005C(A7),A0
       | 0184  1028 0001                      MOVE.B      0001(A0),D0
       | 0188  2200                           MOVE.L      D0,D1
       | 018A  D281                           ADD.L       D1,D1
       | 018C  D280                           ADD.L       D0,D1
       | 018E  D281                           ADD.L       D1,D1
       | 0190  21A8 0002 1808                 MOVE.L      0002(A0),08(A0,D1.L)
;  82: 
;  83:                                                 if(!OpenDevice("cd32mpeg.device",0,(struct IORequest *)iomr_cmd,0))
       | 0196  41F9  0000 0092-01.2           LEA         01.00000092,A0
       | 019C  226F 0090                      MOVEA.L     0090(A7),A1
       | 01A0  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 01A6  7000                           MOVEQ       #00,D0
       | 01A8  2200                           MOVE.L      D0,D1
       | 01AA  4EAE FE44                      JSR         FE44(A6)
       | 01AE  4A00                           TST.B       D0
       | 01B0  6600 055E                      BNE.W       0710
;  84:                                                 {
;  85:                                                     PutStr("Device opened okay.\n");
       | 01B4  41F9  0000 00A2-01.2           LEA         01.000000A2,A0
       | 01BA  2208                           MOVE.L      A0,D1
       | 01BC  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 01C2  4EAE FC4C                      JSR         FC4C(A6)
;  86: 
;  87:                                                     if(args[0])
       | 01C6  202F 0078                      MOVE.L      0078(A7),D0
       | 01CA  6700 0534                      BEQ.W       0700
;  88:                                                     {
;  89:                                                         msi.msi_Start = toc[*(LONG *)args[0]].Entry.Position.LSN;
       | 01CE  2040                           MOVEA.L     D0,A0
       | 01D0  2210                           MOVE.L      (A0),D1
       | 01D2  2401                           MOVE.L      D1,D2
       | 01D4  D482                           ADD.L       D2,D2
       | 01D6  D481                           ADD.L       D1,D2
       | 01D8  D482                           ADD.L       D2,D2
       | 01DA  226F 005C                      MOVEA.L     005C(A7),A1
       | 01DE  2231 2802                      MOVE.L      02(A1,D2.L),D1
       | 01E2  2F41 0040                      MOVE.L      D1,0040(A7)
;  90:                                                         msi.msi_End = toc[*(LONG *)args[0]+1].Entry.Position.LSN - 1;
       | 01E6  2040                           MOVEA.L     D0,A0
       | 01E8  2410                           MOVE.L      (A0),D2
       | 01EA  5282                           ADDQ.L      #1,D2
       | 01EC  2002                           MOVE.L      D2,D0
       | 01EE  D080                           ADD.L       D0,D0
       | 01F0  D082                           ADD.L       D2,D0
       | 01F2  D080                           ADD.L       D0,D0
       | 01F4  2431 0802                      MOVE.L      02(A1,D0.L),D2
       | 01F8  5382                           SUBQ.L      #1,D2
       | 01FA  2F42 0044                      MOVE.L      D2,0044(A7)
;  91:                                                         msi.msi_SectorSize = 2328;
       | 01FE  2F7C 0000 0918 0048            MOVE.L      #00000918,0048(A7)
;  92: 
;  93:                                                         Printf("Start: %ld\n",msi.msi_Start);
       | 0206  2F01                           MOVE.L      D1,-(A7)
       | 0208  41F9  0000 00B8-01.2           LEA         01.000000B8,A0
       | 020E  2208                           MOVE.L      A0,D1
       | 0210  240F                           MOVE.L      A7,D2
       | 0212  4EAE FC46                      JSR         FC46(A6)
       | 0216  4FEF 0004                      LEA         0004(A7),A7
;  94:                                                         Printf("End:   %ld\n",msi.msi_End);
       | 021A  2F2F 0044                      MOVE.L      0044(A7),-(A7)
       | 021E  41F9  0000 00C4-01.2           LEA         01.000000C4,A0
       | 0224  2208                           MOVE.L      A0,D1
       | 0226  240F                           MOVE.L      A7,D2
       | 0228  4EAE FC46                      JSR         FC46(A6)
       | 022C  4FEF 0004                      LEA         0004(A7),A7
;  95: 
;  96:                                                         if(myScreen = OpenScreenTags(NULL, SA_Overscan, OSCAN_VIDEO,
;  97:                                                                                            SA_ShowTitle, FALSE,
;  98:                                                                                            SA_Depth, 8,
;  99:                                                                                            SA_Behind, FALSE,
; 100:                                                                                            SA_Quiet, TRUE,
; 101:                                                                                            SA_Width, 352,
; 102:                                                                                            SA_Height, 240,
; 103:                                                                                            SA_DisplayID, DEFAULT_MONITOR_ID,
; 104:                                                                                            TAG_DONE))
       | 0230  42A7                           CLR.L       -(A7)
       | 0232  42A7                           CLR.L       -(A7)
       | 0234  2F3C 8000 0032                 MOVE.L      #80000032,-(A7)
       | 023A  4878 00F0                      PEA         00F0
       | 023E  2F3C 8000 0024                 MOVE.L      #80000024,-(A7)
       | 0244  4878 0160                      PEA         0160
       | 0248  2F3C 8000 0023                 MOVE.L      #80000023,-(A7)
       | 024E  4878 0001                      PEA         0001
       | 0252  2F3C 8000 0038                 MOVE.L      #80000038,-(A7)
       | 0258  42A7                           CLR.L       -(A7)
       | 025A  2F3C 8000 0037                 MOVE.L      #80000037,-(A7)
       | 0260  4878 0008                      PEA         0008
       | 0264  2F3C 8000 0025                 MOVE.L      #80000025,-(A7)
       | 026A  42A7                           CLR.L       -(A7)
       | 026C  2F3C 8000 0036                 MOVE.L      #80000036,-(A7)
       | 0272  4878 0004                      PEA         0004
       | 0276  2F3C 8000 0034                 MOVE.L      #80000034,-(A7)
       | 027C  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 0282  91C8                           SUBA.L      A0,A0
       | 0284  224F                           MOVEA.L     A7,A1
       | 0286  4EAE FD9C                      JSR         FD9C(A6)
       | 028A  4FEF 0044                      LEA         0044(A7),A7
       | 028E  2F40 0064                      MOVE.L      D0,0064(A7)
       | 0292  6700 046C                      BEQ.W       0700
; 105:                                                         {
; 106:                                                             if(myWindow = OpenWindowTags(NULL, WA_Activate, FALSE,
; 107:                                                                                                WA_CustomScreen, myScreen,
; 108:                                                                                                WA_Flags, WFLG_SIMPLE_REFRESH | WFLG_BORDERLESS | WFLG_BACKDROP,
; 109:                                                                                                TAG_DONE))
       | 0296  42A7                           CLR.L       -(A7)
       | 0298  4878 0940                      PEA         0940
       | 029C  2F3C 8000 006B                 MOVE.L      #8000006B,-(A7)
       | 02A2  2F00                           MOVE.L      D0,-(A7)
       | 02A4  2F3C 8000 0070                 MOVE.L      #80000070,-(A7)
       | 02AA  42A7                           CLR.L       -(A7)
       | 02AC  2F3C 8000 0089                 MOVE.L      #80000089,-(A7)
       | 02B2  91C8                           SUBA.L      A0,A0
       | 02B4  224F                           MOVEA.L     A7,A1
       | 02B6  4EAE FDA2                      JSR         FDA2(A6)
       | 02BA  4FEF 001C                      LEA         001C(A7),A7
       | 02BE  2F40 0060                      MOVE.L      D0,0060(A7)
       | 02C2  6700 042E                      BEQ.W       06F2
; 110: 
; 111:                                                             {
; 112:                                                                 mfs.mfs_Width = 352;
       | 02C6  3F7C 0160 004C                 MOVE.W      #0160,004C(A7)
; 113:                                                                 mfs.mfs_Height = 240;
       | 02CC  3F7C 00F0 004E                 MOVE.W      #00F0,004E(A7)
; 114:                                                                 mfs.mfs_Luma = AllocVec(352*240,0);
       | 02D2  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 02D8  203C 0001 4A00                 MOVE.L      #00014A00,D0
       | 02DE  7200                           MOVEQ       #00,D1
       | 02E0  4EAE FD54                      JSR         FD54(A6)
       | 02E4  2F40 0050                      MOVE.L      D0,0050(A7)
; 115:                                                                 mfs.mfs_Cr = AllocVec(352*240,0);
       | 02E8  203C 0001 4A00                 MOVE.L      #00014A00,D0
       | 02EE  7200                           MOVEQ       #00,D1
       | 02F0  4EAE FD54                      JSR         FD54(A6)
       | 02F4  2F40 0054                      MOVE.L      D0,0054(A7)
; 116:                                                                 mfs.mfs_Cb = AllocVec(352*240,0);
       | 02F8  203C 0001 4A00                 MOVE.L      #00014A00,D0
       | 02FE  7200                           MOVEQ       #00,D1
       | 0300  4EAE FD54                      JSR         FD54(A6)
       | 0304  2F40 0058                      MOVE.L      D0,0058(A7)
; 117: 
; 118:                                                                 {
; 119:                                                                     ULONG c;
; 120:                                                                     for(c=0;c<256;c++)
       | 0308  42AF 002C                      CLR.L       002C(A7)
       | 030C  202F 002C                      MOVE.L      002C(A7),D0
       | 0310  0C80 0000 0100                 CMPI.L      #00000100,D0
       | 0316  6424                           BCC.B       033C
; 121:                                                                         SetRGB32(&myScreen->ViewPort, c, c<<24, c<<24, c<<24);
       | 0318  206F 0064                      MOVEA.L     0064(A7),A0
       | 031C  D0FC 002C                      ADDA.W      #002C,A0
       | 0320  2200                           MOVE.L      D0,D1
       | 0322  4841                           SWAP        D1
       | 0324  4241                           CLR.W       D1
       | 0326  E181                           ASL.L       #8,D1
       | 0328  2401                           MOVE.L      D1,D2
       | 032A  2601                           MOVE.L      D1,D3
       | 032C  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 0332  4EAE FCAC                      JSR         FCAC(A6)
       | 0336  52AF 002C                      ADDQ.L      #1,002C(A7)
       | 033A  60D0                           BRA.B       030C
; 122:                                                                 }
; 123:                                                                 {
; 124:                                                                     struct MPEGVideoParamsSet mvp;
; 125:                                                                     mvp.mvp_Fade = 65535;
       | 033C  3F7C FFFF 002E                 MOVE.W      #FFFF,002E(A7)
; 126:                                                                     iomr_cmd->iomr_Req.io_Data = &mvp;
       | 0342  41EF 002E                      LEA         002E(A7),A0
       | 0346  226F 0090                      MOVEA.L     0090(A7),A1
       | 034A  2348 0028                      MOVE.L      A0,0028(A1)
; 127:                                                                     iomr_cmd->iomr_Req.io_Command = MPEGCMD_SETVIDEOPARAMS;
       | 034E  337C 0013 001C                 MOVE.W      #0013,001C(A1)
; 128:                                                                     iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGVideoParamsSet);
       | 0354  7002                           MOVEQ       #02,D0
       | 0356  2340 0024                      MOVE.L      D0,0024(A1)
; 129:                                                                     DoIO((struct IORequest *)iomr_cmd);
       | 035A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0360  4EAE FE38                      JSR         FE38(A6)
; 130:                                                                 }
; 131:                                                                 iomr_cmd->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
       | 0364  206F 0090                      MOVEA.L     0090(A7),A0
       | 0368  317C 0015 001C                 MOVE.W      #0015,001C(A0)
; 132:                                                                 iomr_cmd->iomr_Req.io_Data = (APTR)&msi;
       | 036E  43EF 0040                      LEA         0040(A7),A1
       | 0372  2149 0028                      MOVE.L      A1,0028(A0)
; 133:                                                                 iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGStreamInfo);
       | 0376  700C                           MOVEQ       #0C,D0
       | 0378  2140 0024                      MOVE.L      D0,0024(A0)
; 134:                                                                 iomr_cmd->iomr_Req.io_Offset = msi.msi_Start; // +60*75;
       | 037C  216F 0040 002C                 MOVE.L      0040(A7),002C(A0)
; 135:                                                                 iomr_cmd->iomr_MPEGFlags = 0;
       | 0382  42A8 0034                      CLR.L       0034(A0)
; 136:                                                                 iomr_cmd->iomr_StreamType = MPEGSTREAM_SYSTEM;
       | 0386  117C 0003 0033                 MOVE.B      #03,0033(A0)
; 137:                                                                 iomr_cmd->iomr_Speed = 4;
       | 038C  7004                           MOVEQ       #04,D0
       | 038E  2140 0038                      MOVE.L      D0,0038(A0)
; 138: 
; 139:                                                                 if(!DoIO((struct IORequest *)iomr_cmd))
       | 0392  2248                           MOVEA.L     A0,A1
       | 0394  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 039A  4EAE FE38                      JSR         FE38(A6)
       | 039E  4A00                           TST.B       D0
       | 03A0  6600 0306                      BNE.W       06A8
; 140:                                                                 {
; 141:                                                                     waitsignals = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D |
; 142:                                                                                   SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F |
; 143:                                                                                   1L << port->mp_SigBit |
       | 03A4  7000                           MOVEQ       #00,D0
       | 03A6  102B 000F                      MOVE.B      000F(A3),D0
       | 03AA  7200                           MOVEQ       #00,D1
       | 03AC  01C1                           BSET.L      D0,D1
       | 03AE  0041 F000                      ORI.W       #F000,D1
; 144:                                                                                   1L << data_port->mp_SigBit;
       | 03B2  7000                           MOVEQ       #00,D0
       | 03B4  102A 000F                      MOVE.B      000F(A2),D0
       | 03B8  7400                           MOVEQ       #00,D2
       | 03BA  01C2                           BSET.L      D0,D2
       | 03BC  8282                           OR.L        D2,D1
       | 03BE  2A01                           MOVE.L      D1,D5
; 145: 
; 146:                                                                     while(!done)
       | 03C0  4A47                           TST.W       D7
       | 03C2  6600 02CC                      BNE.W       0690
; 147:                                                                     {
; 148:                                                                         signals = Wait(waitsignals);
       | 03C6  2005                           MOVE.L      D5,D0
       | 03C8  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 03CE  4EAE FEC2                      JSR         FEC2(A6)
       | 03D2  2C00                           MOVE.L      D0,D6
; 149: 
; 150:                                                                         if(signals & SIGBREAKF_CTRL_C)
       | 03D4  0806 000C                      BTST        #000C,D6
       | 03D8  674C                           BEQ.B       0426
; 151:                                                                         {
; 152:                                                                             if(!CheckIO((struct IORequest *)iomr_cmd))
       | 03DA  226F 0090                      MOVEA.L     0090(A7),A1
       | 03DE  4EAE FE2C                      JSR         FE2C(A6)
       | 03E2  4A40                           TST.W       D0
       | 03E4  6610                           BNE.B       03F6
; 153:                                                                             {
; 154:                                                                                 AbortIO((struct IORequest *)iomr_cmd);
       | 03E6  226F 0090                      MOVEA.L     0090(A7),A1
       | 03EA  4EAE FE20                      JSR         FE20(A6)
; 155:                                                                                 WaitIO((struct IORequest *)iomr_cmd);
       | 03EE  226F 0090                      MOVEA.L     0090(A7),A1
       | 03F2  4EAE FE26                      JSR         FE26(A6)
; 156:                                                                             }
; 157:                                                                             iomr_cmd->iomr_Req.io_Command = CMD_STOP;
       | 03F6  206F 0090                      MOVEA.L     0090(A7),A0
       | 03FA  317C 0006 001C                 MOVE.W      #0006,001C(A0)
; 158:                                                                             DoIO((struct IORequest *)iomr_cmd);
       | 0400  2248                           MOVEA.L     A0,A1
       | 0402  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0408  4EAE FE38                      JSR         FE38(A6)
; 159: 
; 160:                                                                             iomr_cmd->iomr_Req.io_Command = CMD_FLUSH;
       | 040C  206F 0090                      MOVEA.L     0090(A7),A0
       | 0410  317C 0008 001C                 MOVE.W      #0008,001C(A0)
; 161:                                                                             DoIO((struct IORequest *)iomr_cmd);
       | 0416  2248                           MOVEA.L     A0,A1
       | 0418  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 041E  4EAE FE38                      JSR         FE38(A6)
; 162: 
; 163:                                                                             done=TRUE;
       | 0422  7E01                           MOVEQ       #01,D7
       | 0424  609A                           BRA.B       03C0
; 164:                                                                         }
; 165:                                                                         else if(signals & SIGBREAKF_CTRL_D)
       | 0426  0806 000D                      BTST        #000D,D6
       | 042A  6700 00B2                      BEQ.W       04DE
; 166:                                                                         {
; 167:                                                                             SetAPen(&myScreen->RastPort,0);
       | 042E  206F 0064                      MOVEA.L     0064(A7),A0
       | 0432  D0FC 0054                      ADDA.W      #0054,A0
       | 0436  2248                           MOVEA.L     A0,A1
       | 0438  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 043E  7000                           MOVEQ       #00,D0
       | 0440  4EAE FEAA                      JSR         FEAA(A6)
; 168:                                                                             SetDrMd(&myScreen->RastPort,JAM1);
       | 0444  206F 0064                      MOVEA.L     0064(A7),A0
       | 0448  D0FC 0054                      ADDA.W      #0054,A0
       | 044C  2248                           MOVEA.L     A0,A1
       | 044E  7000                           MOVEQ       #00,D0
       | 0450  4EAE FE9E                      JSR         FE9E(A6)
; 169:                                                                             BltPattern(&myScreen->RastPort, NULL, 0, 0, 351, 239, 0);
       | 0454  206F 0064                      MOVEA.L     0064(A7),A0
       | 0458  D0FC 0054                      ADDA.W      #0054,A0
       | 045C  2248                           MOVEA.L     A0,A1
       | 045E  91C8                           SUBA.L      A0,A0
       | 0460  7000                           MOVEQ       #00,D0
       | 0462  2200                           MOVE.L      D0,D1
       | 0464  243C 0000 015F                 MOVE.L      #0000015F,D2
       | 046A  7610                           MOVEQ       #10,D3
       | 046C  4603                           NOT.B       D3
       | 046E  2800                           MOVE.L      D0,D4
       | 0470  4EAE FEC8                      JSR         FEC8(A6)
; 170: 
; 171:                                                                             iomr_cmd->iomr_Req.io_Command = MPEGCMD_SHUTTLELSN;
       | 0474  206F 0090                      MOVEA.L     0090(A7),A0
       | 0478  317C 0019 001C                 MOVE.W      #0019,001C(A0)
; 172:                                                                             iomr_cmd->iomr_Req.io_Data = (APTR)&msi;
       | 047E  43EF 0040                      LEA         0040(A7),A1
       | 0482  2149 0028                      MOVE.L      A1,0028(A0)
; 173:                                                                             iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGStreamInfo);
       | 0486  700C                           MOVEQ       #0C,D0
       | 0488  2140 0024                      MOVE.L      D0,0024(A0)
; 174:                                                                             iomr_cmd->iomr_MPEGFlags = MPEGF_USE_CURRENT_OFFSET;
       | 048C  7002                           MOVEQ       #02,D0
       | 048E  2140 0034                      MOVE.L      D0,0034(A0)
; 175:                                                                             iomr_cmd->iomr_Speed = 150;
       | 0492  217C 0000 0096 0038            MOVE.L      #00000096,0038(A0)
; 176: 
; 177:                                                                             if(DoIO((struct IORequest *)iomr_cmd))
       | 049A  2248                           MOVEA.L     A0,A1
       | 049C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 04A2  4EAE FE38                      JSR         FE38(A6)
       | 04A6  4A00                           TST.B       D0
       | 04A8  6700 FF16                      BEQ.W       03C0
; 178:                                                                                 Printf("Error: %ld %ld\n",iomr_cmd->iomr_Req.io_Error, iomr_cmd->iomr_MPEGError);
       | 04AC  206F 0090                      MOVEA.L     0090(A7),A0
       | 04B0  1028 001F                      MOVE.B      001F(A0),D0
       | 04B4  4880                           EXT.W       D0
       | 04B6  48C0                           EXT.L       D0
       | 04B8  7200                           MOVEQ       #00,D1
       | 04BA  3228 0030                      MOVE.W      0030(A0),D1
       | 04BE  2F01                           MOVE.L      D1,-(A7)
       | 04C0  2F00                           MOVE.L      D0,-(A7)
       | 04C2  41F9  0000 00D0-01.2           LEA         01.000000D0,A0
       | 04C8  2208                           MOVE.L      A0,D1
       | 04CA  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 04D0  240F                           MOVE.L      A7,D2
       | 04D2  4EAE FC46                      JSR         FC46(A6)
       | 04D6  4FEF 0008                      LEA         0008(A7),A7
       | 04DA  6000 FEE4                      BRA.W       03C0
; 179:                                                                         }
; 180:                                                                         else if(signals & SIGBREAKF_CTRL_E)
       | 04DE  0806 000E                      BTST        #000E,D6
       | 04E2  6700 00FC                      BEQ.W       05E0
; 181:                                                                         {
; 182:     //                                                                        iomr_cmd->iomr_Req.io_Command = MPEGCMD_SHUTTLELSN;
; 183:     //                                                                        iomr_cmd->iomr_MPEGFlags = MPEGF_USE_CURRENT_OFFSET;
; 184:     //                                                                        iomr_cmd->iomr_Speed = -150;
; 185: 
; 186:                                                                             iomr_cmd->iomr_Req.io_Command = MPEGCMD_STEPLSN;
       | 04E6  206F 0090                      MOVEA.L     0090(A7),A0
       | 04EA  317C 0018 001C                 MOVE.W      #0018,001C(A0)
; 187:                                                                             iomr_cmd->iomr_Req.io_Data = (APTR)&msi;
       | 04F0  43EF 0040                      LEA         0040(A7),A1
       | 04F4  2149 0028                      MOVE.L      A1,0028(A0)
; 188:                                                                             iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGStreamInfo);
       | 04F8  700C                           MOVEQ       #0C,D0
       | 04FA  2140 0024                      MOVE.L      D0,0024(A0)
; 189: 
; 190:                                                                             iomr_cmd->iomr_MPEGFlags = MPEGF_USE_CURRENT_OFFSET;
       | 04FE  7002                           MOVEQ       #02,D0
       | 0500  2140 0034                      MOVE.L      D0,0034(A0)
; 191:                                                                             if(DoIO((struct IORequest *)iomr_cmd))
       | 0504  2248                           MOVEA.L     A0,A1
       | 0506  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 050C  4EAE FE38                      JSR         FE38(A6)
       | 0510  4A00                           TST.B       D0
       | 0512  672E                           BEQ.B       0542
; 192:                                                                                 Printf("Error: %ld %ld\n",iomr_cmd->iomr_Req.io_Error, iomr_cmd->iomr_MPEGError);
       | 0514  206F 0090                      MOVEA.L     0090(A7),A0
       | 0518  1028 001F                      MOVE.B      001F(A0),D0
       | 051C  4880                           EXT.W       D0
       | 051E  48C0                           EXT.L       D0
       | 0520  7200                           MOVEQ       #00,D1
       | 0522  3228 0030                      MOVE.W      0030(A0),D1
       | 0526  2F01                           MOVE.L      D1,-(A7)
       | 0528  2F00                           MOVE.L      D0,-(A7)
       | 052A  41F9  0000 00E0-01.2           LEA         01.000000E0,A0
       | 0530  2208                           MOVE.L      A0,D1
       | 0532  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 0538  240F                           MOVE.L      A7,D2
       | 053A  4EAE FC46                      JSR         FC46(A6)
       | 053E  4FEF 0008                      LEA         0008(A7),A7
; 193: 
; 194:                                                                             iomr_cmd->iomr_Req.io_Command = MPEGCMD_READFRAME;
       | 0542  206F 0090                      MOVEA.L     0090(A7),A0
       | 0546  317C 001A 001C                 MOVE.W      #001A,001C(A0)
; 195:                                                                             iomr_cmd->iomr_Req.io_Data = &mfs;
       | 054C  43EF 004C                      LEA         004C(A7),A1
       | 0550  2149 0028                      MOVE.L      A1,0028(A0)
; 196:                                                                             iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGFrameStore);
       | 0554  7010                           MOVEQ       #10,D0
       | 0556  2140 0024                      MOVE.L      D0,0024(A0)
; 197:                                                                             if(DoIO((struct IORequest *)iomr_cmd))
       | 055A  2248                           MOVEA.L     A0,A1
       | 055C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0562  4EAE FE38                      JSR         FE38(A6)
       | 0566  4A00                           TST.B       D0
       | 0568  672E                           BEQ.B       0598
; 198:                                                                                 Printf("Error: %ld %ld\n",iomr_cmd->iomr_Req.io_Error, iomr_cmd->iomr_MPEGError);
       | 056A  206F 0090                      MOVEA.L     0090(A7),A0
       | 056E  1028 001F                      MOVE.B      001F(A0),D0
       | 0572  4880                           EXT.W       D0
       | 0574  48C0                           EXT.L       D0
       | 0576  7200                           MOVEQ       #00,D1
       | 0578  3228 0030                      MOVE.W      0030(A0),D1
       | 057C  2F01                           MOVE.L      D1,-(A7)
       | 057E  2F00                           MOVE.L      D0,-(A7)
       | 0580  41F9  0000 00F0-01.2           LEA         01.000000F0,A0
       | 0586  2208                           MOVE.L      A0,D1
       | 0588  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 058E  240F                           MOVE.L      A7,D2
       | 0590  4EAE FC46                      JSR         FC46(A6)
       | 0594  4FEF 0008                      LEA         0008(A7),A7
; 199: #if 0
; 200: 									    {
; 201: 									        BPTR file;
; 202: 									        if(file=Open("df0:mpeg.dump",MODE_NEWFILE))
; 203: 									        {
; 204: 									            Write(file,mfs.mfs_Luma,352*240);
; 205: 									            Close(file);
; 206: 									        }
; 207: 									    }
; 208: #endif
; 209:                                                                             WriteChunkyPixels(&myScreen->RastPort,0,0,351,239,mfs.mfs_Cr,352);
       | 0598  206F 0064                      MOVEA.L     0064(A7),A0
       | 059C  D0FC 0054                      ADDA.W      #0054,A0
       | 05A0  2F0A                           MOVE.L      A2,-(A7)
       | 05A2  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 05A8  7000                           MOVEQ       #00,D0
       | 05AA  2200                           MOVE.L      D0,D1
       | 05AC  243C 0000 015F                 MOVE.L      #0000015F,D2
       | 05B2  7610                           MOVEQ       #10,D3
       | 05B4  4603                           NOT.B       D3
       | 05B6  246F 0058                      MOVEA.L     0058(A7),A2
       | 05BA  7858                           MOVEQ       #58,D4
       | 05BC  E58C                           LSL.L       #2,D4
       | 05BE  4EAE FBE0                      JSR         FBE0(A6)
       | 05C2  245F                           MOVEA.L     (A7)+,A2
; 210:                                                                             WriteChunkyPixels(&myScreen->RastPort,0,0,351,239,mfs.mfs_Cb,352);
       | 05C4  206F 0064                      MOVEA.L     0064(A7),A0
       | 05C8  D0FC 0054                      ADDA.W      #0054,A0
       | 05CC  2F0A                           MOVE.L      A2,-(A7)
       | 05CE  7000                           MOVEQ       #00,D0
       | 05D0  2200                           MOVE.L      D0,D1
       | 05D2  246F 005C                      MOVEA.L     005C(A7),A2
       | 05D6  4EAE FBE0                      JSR         FBE0(A6)
       | 05DA  245F                           MOVEA.L     (A7)+,A2
       | 05DC  6000 FDE2                      BRA.W       03C0
; 211:                                                                         }
; 212:                                                                         else if(signals & SIGBREAKF_CTRL_F)
       | 05E0  0806 000F                      BTST        #000F,D6
       | 05E4  6700 FDDA                      BEQ.W       03C0
; 213:                                                                         {
; 214:                                                                             SetAPen(&myScreen->RastPort,0);
       | 05E8  206F 0064                      MOVEA.L     0064(A7),A0
       | 05EC  D0FC 0054                      ADDA.W      #0054,A0
       | 05F0  2248                           MOVEA.L     A0,A1
       | 05F2  2C79  0000 000C-02             MOVEA.L     02.0000000C,A6
       | 05F8  7000                           MOVEQ       #00,D0
       | 05FA  4EAE FEAA                      JSR         FEAA(A6)
; 215:                                                                             SetDrMd(&myScreen->RastPort,JAM1);
       | 05FE  206F 0064                      MOVEA.L     0064(A7),A0
       | 0602  D0FC 0054                      ADDA.W      #0054,A0
       | 0606  2248                           MOVEA.L     A0,A1
       | 0608  7000                           MOVEQ       #00,D0
       | 060A  4EAE FE9E                      JSR         FE9E(A6)
; 216:                                                                             BltPattern(&myScreen->RastPort, NULL, 0, 0, 351, 239, 0);
       | 060E  206F 0064                      MOVEA.L     0064(A7),A0
       | 0612  D0FC 0054                      ADDA.W      #0054,A0
       | 0616  2248                           MOVEA.L     A0,A1
       | 0618  91C8                           SUBA.L      A0,A0
       | 061A  7000                           MOVEQ       #00,D0
       | 061C  2200                           MOVE.L      D0,D1
       | 061E  243C 0000 015F                 MOVE.L      #0000015F,D2
       | 0624  7610                           MOVEQ       #10,D3
       | 0626  4603                           NOT.B       D3
       | 0628  2800                           MOVE.L      D0,D4
       | 062A  4EAE FEC8                      JSR         FEC8(A6)
; 217: 
; 218:                                                                             iomr_cmd->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
       | 062E  206F 0090                      MOVEA.L     0090(A7),A0
       | 0632  317C 0015 001C                 MOVE.W      #0015,001C(A0)
; 219:                                                                             iomr_cmd->iomr_Req.io_Data = (APTR)&msi;
       | 0638  43EF 0040                      LEA         0040(A7),A1
       | 063C  2149 0028                      MOVE.L      A1,0028(A0)
; 220:                                                                             iomr_cmd->iomr_Req.io_Length = sizeof(struct MPEGStreamInfo);
       | 0640  700C                           MOVEQ       #0C,D0
       | 0642  2140 0024                      MOVE.L      D0,0024(A0)
; 221:                                                                             iomr_cmd->iomr_MPEGFlags = MPEGF_USE_CURRENT_OFFSET;
       | 0646  7002                           MOVEQ       #02,D0
       | 0648  2140 0034                      MOVE.L      D0,0034(A0)
; 222: 
; 223:                                                                             if(DoIO((struct IORequest *)iomr_cmd))
       | 064C  2248                           MOVEA.L     A0,A1
       | 064E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0654  4EAE FE38                      JSR         FE38(A6)
       | 0658  4A00                           TST.B       D0
       | 065A  6700 FD64                      BEQ.W       03C0
; 224:                                                                                 Printf("Error: %ld %ld\n",iomr_cmd->iomr_Req.io_Error, iomr_cmd->iomr_MPEGError);
       | 065E  206F 0090                      MOVEA.L     0090(A7),A0
       | 0662  1028 001F                      MOVE.B      001F(A0),D0
       | 0666  4880                           EXT.W       D0
       | 0668  48C0                           EXT.L       D0
       | 066A  7200                           MOVEQ       #00,D1
       | 066C  3228 0030                      MOVE.W      0030(A0),D1
       | 0670  2F01                           MOVE.L      D1,-(A7)
       | 0672  2F00                           MOVE.L      D0,-(A7)
       | 0674  41F9  0000 0100-01.2           LEA         01.00000100,A0
       | 067A  2208                           MOVE.L      A0,D1
       | 067C  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 0682  240F                           MOVE.L      A7,D2
       | 0684  4EAE FC46                      JSR         FC46(A6)
       | 0688  4FEF 0008                      LEA         0008(A7),A7
; 225:                                                                         }
; 226:                                                                     }
       | 068C  6000 FD32                      BRA.W       03C0
; 227:                                                                     iomr_cmd->iomr_Req.io_Command = CMD_STOP;
       | 0690  206F 0090                      MOVEA.L     0090(A7),A0
       | 0694  317C 0006 001C                 MOVE.W      #0006,001C(A0)
; 228:                                                                     DoIO((struct IORequest *)iomr_cmd);
       | 069A  2248                           MOVEA.L     A0,A1
       | 069C  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 06A2  4EAE FE38                      JSR         FE38(A6)
       | 06A6  602E                           BRA.B       06D6
; 229:                                                                 }
; 230:                                                                 else
; 231:                                                                     Printf("Error: %ld %ld\n",iomr_cmd->iomr_Req.io_Error, iomr_cmd->iomr_MPEGError);
       | 06A8  206F 0090                      MOVEA.L     0090(A7),A0
       | 06AC  1028 001F                      MOVE.B      001F(A0),D0
       | 06B0  4880                           EXT.W       D0
       | 06B2  48C0                           EXT.L       D0
       | 06B4  7200                           MOVEQ       #00,D1
       | 06B6  3228 0030                      MOVE.W      0030(A0),D1
       | 06BA  2F01                           MOVE.L      D1,-(A7)
       | 06BC  2F00                           MOVE.L      D0,-(A7)
       | 06BE  41F9  0000 0110-01.2           LEA         01.00000110,A0
       | 06C4  2208                           MOVE.L      A0,D1
       | 06C6  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 06CC  240F                           MOVE.L      A7,D2
       | 06CE  4EAE FC46                      JSR         FC46(A6)
       | 06D2  4FEF 0008                      LEA         0008(A7),A7
; 232: 
; 233: 								FreeVec(mfs.mfs_Luma);
       | 06D6  226F 0050                      MOVEA.L     0050(A7),A1
       | 06DA  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 06E0  4EAE FD4E                      JSR         FD4E(A6)
; 234:                                                                 CloseWindow(myWindow);
       | 06E4  206F 0060                      MOVEA.L     0060(A7),A0
       | 06E8  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 06EE  4EAE FFB8                      JSR         FFB8(A6)
; 235:                                                             }
; 236:                                                             CloseScreen(myScreen);
       | 06F2  206F 0064                      MOVEA.L     0064(A7),A0
       | 06F6  2C79  0000 0008-02             MOVEA.L     02.00000008,A6
       | 06FC  4EAE FFBE                      JSR         FFBE(A6)
; 237:                                                         }
; 238:                                                     }
; 239:                                                     CloseDevice((struct IORequest *)iomr_cmd);
       | 0700  226F 0090                      MOVEA.L     0090(A7),A1
       | 0704  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 070A  4EAE FE3E                      JSR         FE3E(A6)
       | 070E  6012                           BRA.B       0722
; 240:                                                 }
; 241:                                                 else
; 242:                                                     PutStr("Device open failed.\n");
       | 0710  41F9  0000 0120-01.2           LEA         01.00000120,A0
       | 0716  2208                           MOVE.L      A0,D1
       | 0718  2C79  0000 0004-02             MOVEA.L     02.00000004,A6
       | 071E  4EAE FC4C                      JSR         FC4C(A6)
; 243:                                             }
; 244:                                         }
; 245:                                         FreeVec(toc);
       | 0722  226F 005C                      MOVEA.L     005C(A7),A1
       | 0726  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 072C  4EAE FD4E                      JSR         FD4E(A6)
; 246:                                     }
; 247:                                     CloseDevice((struct IORequest *)iocd);
       | 0730  226F 0088                      MOVEA.L     0088(A7),A1
       | 0734  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 073A  4EAE FE3E                      JSR         FE3E(A6)
; 248:                                 }
; 249:                                 DeleteIORequest((struct IORequest *)iomr_cmd);
       | 073E  206F 0090                      MOVEA.L     0090(A7),A0
       | 0742  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0748  4EAE FD6C                      JSR         FD6C(A6)
; 250:                             }
; 251:                             DeleteMsgPort(data_port);
       | 074C  204A                           MOVEA.L     A2,A0
       | 074E  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0754  4EAE FD60                      JSR         FD60(A6)
; 252:                         }
; 253:                         DeleteMsgPort(port);
       | 0758  204B                           MOVEA.L     A3,A0
       | 075A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0760  4EAE FD60                      JSR         FD60(A6)
; 254:                     }
; 255:                     CloseLibrary(GfxBase);
       | 0764  2279  0000 000C-02             MOVEA.L     02.0000000C,A1
       | 076A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0770  4EAE FE62                      JSR         FE62(A6)
; 256:                 }
; 257:                 CloseLibrary(IntuitionBase);
       | 0774  2279  0000 0008-02             MOVEA.L     02.00000008,A1
       | 077A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0780  4EAE FE62                      JSR         FE62(A6)
; 258:             }
; 259:             CloseLibrary(DOSBase);
       | 0784  2279  0000 0004-02             MOVEA.L     02.00000004,A1
       | 078A  2C79  0000 0000-02             MOVEA.L     02.00000000,A6
       | 0790  4EAE FE62                      JSR         FE62(A6)
; 260:         }
; 261:     }
; 262:     return(0L);
       | 0794  7000                           MOVEQ       #00,D0
; 263: }**END OF SOURCE FILE
       | 0796  4CDF 6CFC                      MOVEM.L     (A7)+,D2-D7/A2-A3/A5-A6
       | 079A  DEFC 006C                      ADDA.W      #006C,A7
       | 079E  4E75                           RTS

SECTION 01 "data" 00000138 BYTES
OFFSETS 0000 THROUGH 000F CONTAIN ZERO
0010 00 00 00 04 00 00 09 18 00 00 00 00 00 00 00 01 ................
0020 64 6F 73 2E 6C 69 62 72 61 72 79 00 69 6E 74 75 dos.library.intu
0030 69 74 69 6F 6E 2E 6C 69 62 72 61 72 79 00 67 72 ition.library.gr
0040 61 70 68 69 63 73 2E 6C 69 62 72 61 72 79 00 00 aphics.library..
0050 54 52 41 43 4B 2F 4E 00 63 64 2E 64 65 76 69 63 TRACK/N.cd.devic
0060 65 00 63 64 2E 64 65 76 69 63 65 20 6F 70 65 6E e.cd.device open
0070 65 64 2E 0A 00 00 47 6F 74 20 54 4F 43 20 6D 65 ed....Got TOC me
0080 6D 6F 72 79 2E 0A 00 00 52 65 61 64 20 54 4F 43 mory....Read TOC
0090 0A 00 63 64 33 32 6D 70 65 67 2E 64 65 76 69 63 ..cd32mpeg.devic
00A0 65 00 44 65 76 69 63 65 20 6F 70 65 6E 65 64 20 e.Device opened 
00B0 6F 6B 61 79 2E 0A 00 00 53 74 61 72 74 3A 20 25 okay....Start: %
00C0 6C 64 0A 00 45 6E 64 3A 20 20 20 25 6C 64 0A 00 ld..End:   %ld..
00D0 45 72 72 6F 72 3A 20 25 6C 64 20 25 6C 64 0A 00 Error: %ld %ld..
00E0 45 72 72 6F 72 3A 20 25 6C 64 20 25 6C 64 0A 00 Error: %ld %ld..
00F0 45 72 72 6F 72 3A 20 25 6C 64 20 25 6C 64 0A 00 Error: %ld %ld..
0100 45 72 72 6F 72 3A 20 25 6C 64 20 25 6C 64 0A 00 Error: %ld %ld..
0110 45 72 72 6F 72 3A 20 25 6C 64 20 25 6C 64 0A 00 Error: %ld %ld..
0120 44 65 76 69 63 65 20 6F 70 65 6E 20 66 61 69 6C Device open fail
0130 65 64 2E 0A 00 00 00 00 ed......

SECTION 02 "udata" 00000010 BYTES
