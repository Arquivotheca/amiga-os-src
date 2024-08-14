Lattice AMIGA 68000-68020 OBJ Module Disassembler V5.04.039
Copyright © 1988, 1989 Lattice Inc.  All Rights Reserved.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_GetVisInfo 0000-00    _CreateGad 0130-00    _OpenWinTags 0166-00    
@CreateReqGads 0184-00    @DeleteReqGads 03DA-00    _passwordHook 040A-00    
_LoginRequestA 04EC-00

SECTION 00 "loginreq.c" 000009A8 BYTES
;   1: 
;   2: #include <exec/types.h>
;   3: #include <exec/lists.h>
;   4: #include <exec/memory.h>
;   5: #include <libraries/gadtools.h>
;   6: #include <graphics/text.h>
;   7: #include "req.h"
;   8: #include <utility/tagitem.h>
;   9: #include <envoy/nipc.h>
;  10: #include <envoy/envoy.h>
;  11: #include <dos/dos.h>
;  12: #include <intuition/sghooks.h>
;  13: 
;  14: #include <pragmas/dos_pragmas.h>
;  15: #include <pragmas/nipc_pragmas.h>
;  16: #include <pragmas/utility_pragmas.h>
;  17: #include <pragmas/exec_pragmas.h>
;  18: #include <pragmas/intuition_pragmas.h>
;  19: #include <pragmas/gadtools_pragmas.h>
;  20: #include <pragmas/graphics_pragmas.h>
;  21: 
;  22: #include <clib/dos_protos.h>
;  23: #include <clib/nipc_protos.h>
;  24: #include <clib/utility_protos.h>
;  25: #include <clib/exec_protos.h>
;  26: #include <clib/intuition_protos.h>
;  27: #include <clib/gadtools_protos.h>
;  28: #include <clib/graphics_protos.h>
;  29: #include <clib/alib_protos.h>
;  30: 
;  31: #include <string.h>
;  32: #include "envoybase.h"
;  33: #include "loginreq.h"
;  34: 
;  35: #define ID_USERNAME     1
;  36: #define ID_PASSWORD     2
;  37: #define ID_OK           3
;  38: #define ID_CANCEL       4
;  39: 
;  40: void kprintf(STRPTR, ...);
;  41: 
;  42: #define MAX(x, y) ((x) > (y) ? (x):(y))
;  43: 
;  44: typedef ULONG (*HOOK_FUNC)();
;  45: 
;  46: APTR GetVisInfo(struct Screen *screen, ULONG tag1, ...)
;  47: {
       | 0000  4E55 0000                      LINK      A5,#0000
       | 0004  48E7 0012                      MOVEM.L   A3/A6,-(A7)
       | 0008  266D 0008                      MOVEA.L   0008(A5),A3
;  48:     return(GetVisualInfoA(screen,(struct TagItem *)&tag1));
       | 000C  2F0E                           MOVE.L    A6,-(A7)
       | 000E  204B                           MOVEA.L   A3,A0
       | 0010  43ED 000C                      LEA       000C(A5),A1
       | 0014  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0018  4EAE FF82                      JSR       FF82(A6)
       | 001C  2C5F                           MOVEA.L   (A7)+,A6
;  49: }
       | 001E  4CDF 4800                      MOVEM.L   (A7)+,A3/A6
       | 0022  4E5D                           UNLK      A5
       | 0024  4E75                           RTS
       | 0026  4E61                           MOVE.L    A1,USP
       | 0028  6D65                           BLT.B     008F
       | 002A  0000 5061                      ORI.B     #61,D0
       | 002E  7373                           
       | 0030  776F                           
       | 0032  7264                           MOVEQ     #64,D1
       | 0034  0000 4F4B                      ORI.B     #4B,D0
       | 0038  0000 4361                      ORI.B     #61,D0
       | 003C  6E63                           BGT.B     00A1
       | 003E  656C                           BCS.B     00AC
       | 0040  0000 2020                      ORI.B     #20,D0
       | 0044  4361                           
       | 0046  6E63                           BGT.B     00AB
       | 0048  656C                           BCS.B     00B6
       | 004A  2020                           MOVE.L    -(A0),D0
       | 004C  0000 486F                      ORI.B     #6F,D0
       | 0050  6F6B                           BLE.B     00BD
       | 0052  3A20                           MOVE.W    -(A0),D5
       | 0054  2020                           MOVE.L    -(A0),D0
       | 0056  2020                           MOVE.L    -(A0),D0
       | 0058  2020                           MOVE.L    -(A0),D0
       | 005A  2020                           MOVE.L    -(A0),D0
       | 005C  2020                           MOVE.L    -(A0),D0
       | 005E  2020                           MOVE.L    -(A0),D0
       | 0060  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 0066  486F 6F6B                      PEA       6F6B(A7)
       | 006A  2D3E                           MOVE.L    ,-(A6)
       | 006C  685F                           BVC.B     00CD
       | 006E  456E                           
       | 0070  7472                           MOVEQ     #72,D2
       | 0072  793A                           
       | 0074  2020                           MOVE.L    -(A0),D0
       | 0076  2020                           MOVE.L    -(A0),D0
       | 0078  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 007E  486F 6F6B                      PEA       6F6B(A7)
       | 0082  2D3E                           MOVE.L    ,-(A6)
       | 0084  685F                           BVC.B     00E5
       | 0086  5375 6245                      SUBQ.W    #1,45(A5,D6.W*2)
       | 008A  6E74                           BGT.B     0100
       | 008C  7279                           MOVEQ     #79,D1
       | 008E  3A20                           MOVE.W    -(A0),D5
       | 0090  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 0096  486F 6F6B                      PEA       6F6B(A7)
       | 009A  2D3E                           MOVE.L    ,-(A6)
       | 009C  685F                           BVC.B     00FD
       | 009E  4461                           NEG.W     -(A1)
       | 00A0  7461                           MOVEQ     #61,D2
       | 00A2  3A20                           MOVE.W    -(A0),D5
       | 00A4  2020                           MOVE.L    -(A0),D0
       | 00A6  2020                           MOVE.L    -(A0),D0
       | 00A8  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 00AE  4C6F 6769 6E20                 DIVU.L    6E20(A7),D1:D6
       | 00B4  5265                           ADDQ.W    #1,-(A5)
       | 00B6  7175                           
       | 00B8  6573                           BCS.B     012D
       | 00BA  7465                           MOVEQ     #65,D2
       | 00BC  7200                           MOVEQ     #00,D1
       | 00BE  4564                           
       | 00C0  6974                           BVS.B     0136
       | 00C2  486F 6F6B                      PEA       6F6B(A7)
       | 00C6  3A20                           MOVE.W    -(A0),D5
       | 00C8  2020                           MOVE.L    -(A0),D0
       | 00CA  2020                           MOVE.L    -(A0),D0
       | 00CC  2020                           MOVE.L    -(A0),D0
       | 00CE  2020                           MOVE.L    -(A0),D0
       | 00D0  2020                           MOVE.L    -(A0),D0
       | 00D2  2020                           MOVE.L    -(A0),D0
       | 00D4  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 00DA  4564                           
       | 00DC  6974                           BVS.B     0152
       | 00DE  486F 6F6B                      PEA       6F6B(A7)
       | 00E2  2D3E                           MOVE.L    ,-(A6)
       | 00E4  685F                           BVC.B     0145
       | 00E6  456E                           
       | 00E8  7472                           MOVEQ     #72,D2
       | 00EA  793A                           
       | 00EC  2020                           MOVE.L    -(A0),D0
       | 00EE  2020                           MOVE.L    -(A0),D0
       | 00F0  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 00F6  4564                           
       | 00F8  6974                           BVS.B     016E
       | 00FA  486F 6F6B                      PEA       6F6B(A7)
       | 00FE  2D3E                           MOVE.L    ,-(A6)
       | 0100  685F                           BVC.B     0161
       | 0102  5375 6245                      SUBQ.W    #1,45(A5,D6.W*2)
       | 0106  6E74                           BGT.B     017C
       | 0108  7279                           MOVEQ     #79,D1
       | 010A  3A20                           MOVE.W    -(A0),D5
       | 010C  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 0112  4573                           
       | 0114  6974                           BVS.B     018A
       | 0116  486F 6F6B                      PEA       6F6B(A7)
       | 011A  2D3E                           MOVE.L    ,-(A6)
       | 011C  685F                           BVC.B     017D
       | 011E  4461                           NEG.W     -(A1)
       | 0120  7461                           MOVEQ     #61,D2
       | 0122  3A20                           MOVE.W    -(A0),D5
       | 0124  2020                           MOVE.L    -(A0),D0
       | 0126  2020                           MOVE.L    -(A0),D0
       | 0128  256C 780A 0000                 MOVE.L    780A(A4),0000(A2)
       | 012E  4E00                           TRAP      #00
;  50: 
;  51: APTR CreateGad(ULONG type, struct Gadget *last, struct NewGadget *ng, ULONG tag1, ...)
;  52: {
       | 0130  4E55 0000                      LINK      A5,#0000
       | 0134  48E7 0132                      MOVEM.L   D7/A2-A3/A6,-(A7)
       | 0138  2E2D 0008                      MOVE.L    0008(A5),D7
       | 013C  266D 000C                      MOVEA.L   000C(A5),A3
       | 0140  246D 0010                      MOVEA.L   0010(A5),A2
;  53:     return(CreateGadgetA(type, last, ng, (struct TagItem *)&tag1));
       | 0144  48E7 0022                      MOVEM.L   A2/A6,-(A7)
       | 0148  224A                           MOVEA.L   A2,A1
       | 014A  2007                           MOVE.L    D7,D0
       | 014C  204B                           MOVEA.L   A3,A0
       | 014E  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0152  45ED 0014                      LEA       0014(A5),A2
       | 0156  4EAE FFE2                      JSR       FFE2(A6)
       | 015A  4CDF 4400                      MOVEM.L   (A7)+,A2/A6
;  54: }
       | 015E  4CDF 4C80                      MOVEM.L   (A7)+,D7/A2-A3/A6
       | 0162  4E5D                           UNLK      A5
       | 0164  4E75                           RTS
;  55: 
;  56: struct Window *OpenWinTags(ULONG tag1, ...)
;  57: {
       | 0166  4E55 0000                      LINK      A5,#0000
       | 016A  2F0E                           MOVE.L    A6,-(A7)
;  58:     return(OpenWindowTagList(NULL, (struct TagItem *)&tag1));
       | 016C  2F0E                           MOVE.L    A6,-(A7)
       | 016E  91C8                           SUBA.L    A0,A0
       | 0170  43ED 0008                      LEA       0008(A5),A1
       | 0174  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 0178  4EAE FDA2                      JSR       FDA2(A6)
       | 017C  2C5F                           MOVEA.L   (A7)+,A6
;  59: }
       | 017E  2C5F                           MOVEA.L   (A7)+,A6
       | 0180  4E5D                           UNLK      A5
       | 0182  4E75                           RTS
;  60: 
;  61: BOOL CreateReqGads(struct LoginReqData *lrd)
;  62: {
       | 0184  4E55 FF6C                      LINK      A5,#FF6C
       | 0188  48E7 3F32                      MOVEM.L   D2-D7/A2-A3/A6,-(A7)
       | 018C  2648                           MOVEA.L   A0,A3
;  63:     struct NewGadget newgads[]={0,0,0,0,"Name",0,ID_USERNAME,PLACETEXT_LEFT,0,0,
;  64:     				0,0,0,0,"Password",0,ID_PASSWORD,PLACETEXT_LEFT,0,0,
;  65:     				0,0,0,0,"OK",0,ID_OK,PLACETEXT_IN,0,0,
;  66:     				0,0,0,0,"Cancel",0,ID_CANCEL,PLACETEXT_IN,0,0};
       | 018E  41F9  0000 0000-01             LEA       01.00000000,A0
       | 0194  43ED FF88                      LEA       FF88(A5),A1
       | 0198  701D                           MOVEQ     #1D,D0
       | 019A  22D8                           MOVE.L    (A0)+,(A1)+
       | 019C  51C8 FFFC                      DBF       D0,019A
;  67: 
;  68:     UWORD InnerWidth, InnerHeight,LeftOffset,TopOffset,index;
;  69:     BOOL status;
;  70: 
;  71:     LeftOffset = lrd->lrd_Window->BorderLeft;
       | 01A0  2053                           MOVEA.L   (A3),A0
       | 01A2  1028 0036                      MOVE.B    0036(A0),D0
;  72:     TopOffset = lrd->lrd_Window->BorderTop;
       | 01A6  1228 0037                      MOVE.B    0037(A0),D1
;  73:     InnerWidth = lrd->lrd_Window->Width - (LeftOffset + lrd->lrd_Window->BorderRight);
       | 01AA  1428 0038                      MOVE.B    0038(A0),D2
       | 01AE  3628 0008                      MOVE.W    0008(A0),D3
;  74:     InnerHeight = lrd->lrd_Window->Height - (TopOffset + lrd->lrd_Window->BorderBottom);
       | 01B2  1828 0039                      MOVE.B    0039(A0),D4
       | 01B6  3A28 000A                      MOVE.W    000A(A0),D5
;  75: 
;  76:     lrd->lrd_IText.IText="Password";
       | 01BA  41FA FE70                      LEA       FE70(PC),A0
       | 01BE  2748 0058                      MOVE.L    A0,0058(A3)
;  77: 
;  78:     newgads[0].ng_LeftEdge = LeftOffset + (INTERWIDTH<<1) + IntuiTextLength(&lrd->lrd_IText);
       | 01C2  41EB 004C                      LEA       004C(A3),A0
       | 01C6  1F40 0028                      MOVE.B    D0,0028(A7)
       | 01CA  1F41 0029                      MOVE.B    D1,0029(A7)
       | 01CE  2F48 002E                      MOVE.L    A0,002E(A7)
       | 01D2  2F0E                           MOVE.L    A6,-(A7)
       | 01D4  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 01D8  4EAE FEB6                      JSR       FEB6(A6)
       | 01DC  2C5F                           MOVEA.L   (A7)+,A6
       | 01DE  122F 0028                      MOVE.B    0028(A7),D1
       | 01E2  4881                           EXT.W     D1
       | 01E4  2F40 0032                      MOVE.L    D0,0032(A7)
       | 01E8  7000                           MOVEQ     #00,D0
       | 01EA  3001                           MOVE.W    D1,D0
       | 01EC  222F 0032                      MOVE.L    0032(A7),D1
       | 01F0  D280                           ADD.L     D0,D1
       | 01F2  1F42 002A                      MOVE.B    D2,002A(A7)
       | 01F6  7410                           MOVEQ     #10,D2
       | 01F8  D282                           ADD.L     D2,D1
       | 01FA  3B41 FF88                      MOVE.W    D1,FF88(A5)
;  79:     newgads[1].ng_LeftEdge = newgads[0].ng_LeftEdge;
       | 01FE  3B41 FFA6                      MOVE.W    D1,FFA6(A5)
;  80: 
;  81:     newgads[0].ng_Width = InnerWidth - INTERWIDTH - newgads[0].ng_LeftEdge + LeftOffset;
       | 0202  48C1                           EXT.L     D1
       | 0204  142F 002A                      MOVE.B    002A(A7),D2
       | 0208  4882                           EXT.W     D2
       | 020A  48C2                           EXT.L     D2
       | 020C  D480                           ADD.L     D0,D2
       | 020E  48C3                           EXT.L     D3
       | 0210  9682                           SUB.L     D2,D3
       | 0212  7400                           MOVEQ     #00,D2
       | 0214  3403                           MOVE.W    D3,D2
       | 0216  2602                           MOVE.L    D2,D3
       | 0218  9681                           SUB.L     D1,D3
       | 021A  D680                           ADD.L     D0,D3
       | 021C  5183                           SUBQ.L    #8,D3
       | 021E  3B43 FF8C                      MOVE.W    D3,FF8C(A5)
;  82:     newgads[1].ng_Width = newgads[0].ng_Width;
       | 0222  3B43 FFAA                      MOVE.W    D3,FFAA(A5)
       | 0226  122F 0029                      MOVE.B    0029(A7),D1
       | 022A  4881                           EXT.W     D1
       | 022C  7600                           MOVEQ     #00,D3
       | 022E  3601                           MOVE.W    D1,D3
;  83: 
;  84:     newgads[0].ng_TopEdge = TopOffset + 4;
       | 0230  2203                           MOVE.L    D3,D1
       | 0232  5881                           ADDQ.L    #4,D1
       | 0234  3B41 FF8A                      MOVE.W    D1,FF8A(A5)
;  85:     newgads[1].ng_TopEdge = (InnerHeight>>1) + TopOffset - (lrd->lrd_NHeight>>1) - 3 ;
       | 0238  7200                           MOVEQ     #00,D1
       | 023A  322B 0026                      MOVE.W    0026(A3),D1
       | 023E  2F40 0036                      MOVE.L    D0,0036(A7)
       | 0242  2001                           MOVE.L    D1,D0
       | 0244  E280                           ASR.L     #1,D0
       | 0246  4884                           EXT.W     D4
       | 0248  48C4                           EXT.L     D4
       | 024A  D883                           ADD.L     D3,D4
       | 024C  48C5                           EXT.L     D5
       | 024E  9A84                           SUB.L     D4,D5
       | 0250  7800                           MOVEQ     #00,D4
       | 0252  3805                           MOVE.W    D5,D4
       | 0254  2A04                           MOVE.L    D4,D5
       | 0256  E285                           ASR.L     #1,D5
       | 0258  DA83                           ADD.L     D3,D5
       | 025A  9A80                           SUB.L     D0,D5
       | 025C  5785                           SUBQ.L    #3,D5
       | 025E  3B45 FFA8                      MOVE.W    D5,FFA8(A5)
;  86: 
;  87:     newgads[2].ng_TopEdge = InnerHeight + TopOffset - lrd->lrd_NHeight - 10;
       | 0262  D883                           ADD.L     D3,D4
       | 0264  7000                           MOVEQ     #00,D0
       | 0266  302B 0026                      MOVE.W    0026(A3),D0
       | 026A  9880                           SUB.L     D0,D4
       | 026C  700A                           MOVEQ     #0A,D0
       | 026E  9880                           SUB.L     D0,D4
       | 0270  3B44 FFC6                      MOVE.W    D4,FFC6(A5)
;  88: 
;  89:     newgads[3].ng_TopEdge = newgads[2].ng_TopEdge;
       | 0274  3B44 FFE4                      MOVE.W    D4,FFE4(A5)
;  90: 
;  91:     lrd->lrd_IText.IText="  Cancel  ";
       | 0278  41FA FDC8                      LEA       FDC8(PC),A0
       | 027C  2748 0058                      MOVE.L    A0,0058(A3)
;  92: 
;  93:     newgads[2].ng_Width = 8 + IntuiTextLength(&lrd->lrd_IText);
       | 0280  2F0E                           MOVE.L    A6,-(A7)
       | 0282  206F 0032                      MOVEA.L   0032(A7),A0
       | 0286  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 028A  4EAE FEB6                      JSR       FEB6(A6)
       | 028E  2C5F                           MOVEA.L   (A7)+,A6
       | 0290  5080                           ADDQ.L    #8,D0
       | 0292  3B40 FFC8                      MOVE.W    D0,FFC8(A5)
;  94:     newgads[3].ng_Width = newgads[2].ng_Width;
       | 0296  3B40 FFE6                      MOVE.W    D0,FFE6(A5)
;  95: 
;  96:     newgads[2].ng_LeftEdge = LeftOffset + 8;
       | 029A  202F 0036                      MOVE.L    0036(A7),D0
       | 029E  2200                           MOVE.L    D0,D1
       | 02A0  5081                           ADDQ.L    #8,D1
       | 02A2  3B41 FFC4                      MOVE.W    D1,FFC4(A5)
;  97:     newgads[3].ng_LeftEdge = InnerWidth - newgads[3].ng_Width + LeftOffset - 8;
       | 02A6  322D FFE6                      MOVE.W    FFE6(A5),D1
       | 02AA  48C1                           EXT.L     D1
       | 02AC  9481                           SUB.L     D1,D2
       | 02AE  D480                           ADD.L     D0,D2
       | 02B0  5182                           SUBQ.L    #8,D2
       | 02B2  3B42 FFE2                      MOVE.W    D2,FFE2(A5)
;  98: 
;  99:     for(index=0;index<4;index++)
       | 02B6  7E00                           MOVEQ     #00,D7
       | 02B8  6022                           BRA.B     02DC
; 100:     {
; 101:     	newgads[index].ng_Height = lrd->lrd_NHeight + 6;
       | 02BA  2007                           MOVE.L    D7,D0
       | 02BC  C0FC 001E                      MULU.W    #001E,D0
       | 02C0  322B 0026                      MOVE.W    0026(A3),D1
       | 02C4  5C41                           ADDQ.W    #6,D1
       | 02C6  3B81 088E                      MOVE.W    D1,8E(A5,D0.L)
; 102:     	newgads[index].ng_TextAttr = lrd->lrd_Screen->Font;
       | 02CA  206B 0014                      MOVEA.L   0014(A3),A0
       | 02CE  2BA8 0028 0894                 MOVE.L    0028(A0),94(A5,D0.L)
; 103:     	newgads[index].ng_VisualInfo = lrd->lrd_VisualInfo;
       | 02D4  2BAB 0018 089E                 MOVE.L    0018(A3),9E(A5,D0.L)
       | 02DA  5247                           ADDQ.W    #1,D7
       | 02DC  7004                           MOVEQ     #04,D0
       | 02DE  BE40                           CMP.W     D0,D7
       | 02E0  65D8                           BCS.B     02BA
; 104:     }
; 105: 
; 106:     lrd->lrd_LastGadget = CreateContext(&lrd->lrd_GadList);
       | 02E2  41EB 002C                      LEA       002C(A3),A0
       | 02E6  2F0E                           MOVE.L    A6,-(A7)
       | 02E8  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 02EC  4EAE FF8E                      JSR       FF8E(A6)
       | 02F0  2C5F                           MOVEA.L   (A7)+,A6
       | 02F2  2740 0030                      MOVE.L    D0,0030(A3)
; 107: 
; 108:     lrd->lrd_LastGadget = lrd->lrd_UserName = CreateGad(STRING_KIND,lrd->lrd_LastGadget,&newgads[0],
; 109:     				    GTST_MaxChars, 31,
; 110:     				    TAG_DONE);
       | 02F6  42A7                           CLR.L     -(A7)
       | 02F8  4878 001F                      PEA       001F
       | 02FC  2F3C 8008 002E                 MOVE.L    #8008002E,-(A7)
       | 0302  486D FF88                      PEA       FF88(A5)
       | 0306  2F00                           MOVE.L    D0,-(A7)
       | 0308  4878 000C                      PEA       000C
       | 030C  6100 FE22                      BSR.W     0130
       | 0310  2740 0034                      MOVE.L    D0,0034(A3)
       | 0314  2740 0030                      MOVE.L    D0,0030(A3)
; 111: 
; 112:     lrd->lrd_LastGadget = lrd->lrd_Password = CreateGad(STRING_KIND,lrd->lrd_LastGadget,&newgads[1],
; 113:     				    GTST_MaxChars, 31,
; 114:     				    GTST_EditHook, (ULONG)&lrd->lrd_EditHook,
       | 0318  41EB 00D0                      LEA       00D0(A3),A0
; 115:     				    TAG_DONE);
       | 031C  4297                           CLR.L     (A7)
       | 031E  2F08                           MOVE.L    A0,-(A7)
       | 0320  2F3C 8008 0037                 MOVE.L    #80080037,-(A7)
       | 0326  4878 001F                      PEA       001F
       | 032A  2F3C 8008 002E                 MOVE.L    #8008002E,-(A7)
       | 0330  486D FFA6                      PEA       FFA6(A5)
       | 0334  2F00                           MOVE.L    D0,-(A7)
       | 0336  4878 000C                      PEA       000C
       | 033A  6100 FDF4                      BSR.W     0130
       | 033E  2740 0038                      MOVE.L    D0,0038(A3)
       | 0342  2740 0030                      MOVE.L    D0,0030(A3)
; 116: 
; 117:     lrd->lrd_LastGadget = lrd->lrd_OK = CreateGad(BUTTON_KIND,lrd->lrd_LastGadget,&newgads[2],
; 118:     				    TAG_DONE);
       | 0346  4297                           CLR.L     (A7)
       | 0348  486D FFC4                      PEA       FFC4(A5)
       | 034C  2F00                           MOVE.L    D0,-(A7)
       | 034E  4878 0001                      PEA       0001
       | 0352  6100 FDDC                      BSR.W     0130
       | 0356  2740 003C                      MOVE.L    D0,003C(A3)
       | 035A  2740 0030                      MOVE.L    D0,0030(A3)
; 119: 
; 120:     lrd->lrd_LastGadget = lrd->lrd_Cancel =CreateGad(BUTTON_KIND,lrd->lrd_LastGadget,&newgads[3],
; 121:     				    TAG_DONE);
       | 035E  4297                           CLR.L     (A7)
       | 0360  486D FFE2                      PEA       FFE2(A5)
       | 0364  2F00                           MOVE.L    D0,-(A7)
       | 0366  4878 0001                      PEA       0001
       | 036A  6100 FDC4                      BSR.W     0130
       | 036E  4FEF 004C                      LEA       004C(A7),A7
       | 0372  2740 0040                      MOVE.L    D0,0040(A3)
       | 0376  2740 0030                      MOVE.L    D0,0030(A3)
; 122: 
; 123:     if(lrd->lrd_LastGadget)
       | 037A  4A80                           TST.L     D0
       | 037C  6740                           BEQ.B     03BE
; 124:     {
; 125: 	AddGList(lrd->lrd_Window,lrd->lrd_GadList,-1,-1,0);
       | 037E  2F0E                           MOVE.L    A6,-(A7)
       | 0380  2053                           MOVEA.L   (A3),A0
       | 0382  226B 002C                      MOVEA.L   002C(A3),A1
       | 0386  70FF                           MOVEQ     #FF,D0
       | 0388  2200                           MOVE.L    D0,D1
       | 038A  95CA                           SUBA.L    A2,A2
       | 038C  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 0390  4EAE FE4A                      JSR       FE4A(A6)
       | 0394  2C5F                           MOVEA.L   (A7)+,A6
; 126: 	RefreshGList(lrd->lrd_GadList,lrd->lrd_Window,0,-1);
       | 0396  2F0E                           MOVE.L    A6,-(A7)
       | 0398  206B 002C                      MOVEA.L   002C(A3),A0
       | 039C  2253                           MOVEA.L   (A3),A1
       | 039E  70FF                           MOVEQ     #FF,D0
       | 03A0  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 03A4  4EAE FE50                      JSR       FE50(A6)
       | 03A8  2C5F                           MOVEA.L   (A7)+,A6
; 127: 	GT_RefreshWindow(lrd->lrd_Window,0L);
       | 03AA  2F0E                           MOVE.L    A6,-(A7)
       | 03AC  224A                           MOVEA.L   A2,A1
       | 03AE  2053                           MOVEA.L   (A3),A0
       | 03B0  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 03B4  4EAE FFAC                      JSR       FFAC(A6)
       | 03B8  2C5F                           MOVEA.L   (A7)+,A6
; 128: 	status = TRUE;
       | 03BA  7C01                           MOVEQ     #01,D6
; 129:     }
; 130:     else
       | 03BC  6012                           BRA.B     03D0
; 131:     {
; 132:     	status = FALSE;
       | 03BE  7C00                           MOVEQ     #00,D6
; 133:     	FreeGadgets(lrd->lrd_GadList);
       | 03C0  2F0E                           MOVE.L    A6,-(A7)
       | 03C2  206B 002C                      MOVEA.L   002C(A3),A0
       | 03C6  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 03CA  4EAE FFDC                      JSR       FFDC(A6)
       | 03CE  2C5F                           MOVEA.L   (A7)+,A6
; 134:     }
; 135: 
; 136:     return(status);
       | 03D0  2006                           MOVE.L    D6,D0
; 137: }
       | 03D2  4CDF 4CFC                      MOVEM.L   (A7)+,D2-D7/A2-A3/A6
       | 03D6  4E5D                           UNLK      A5
       | 03D8  4E75                           RTS
; 138: 
; 139: VOID DeleteReqGads(struct LoginReqData *lrd)
; 140: {
       | 03DA  48E7 0012                      MOVEM.L   A3/A6,-(A7)
       | 03DE  2648                           MOVEA.L   A0,A3
; 141:     RemoveGList(lrd->lrd_Window,lrd->lrd_GadList,-1);
       | 03E0  2F0E                           MOVE.L    A6,-(A7)
       | 03E2  2053                           MOVEA.L   (A3),A0
       | 03E4  226B 002C                      MOVEA.L   002C(A3),A1
       | 03E8  70FF                           MOVEQ     #FF,D0
       | 03EA  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 03EE  4EAE FE44                      JSR       FE44(A6)
       | 03F2  2C5F                           MOVEA.L   (A7)+,A6
; 142:     FreeGadgets(lrd->lrd_GadList);
       | 03F4  2F0E                           MOVE.L    A6,-(A7)
       | 03F6  206B 002C                      MOVEA.L   002C(A3),A0
       | 03FA  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 03FE  4EAE FFDC                      JSR       FFDC(A6)
       | 0402  2C5F                           MOVEA.L   (A7)+,A6
; 143: }
       | 0404  4CDF 4800                      MOVEM.L   (A7)+,A3/A6
       | 0408  4E75                           RTS
; 144: 
; 145: ULONG __saveds __asm passwordHook(register __a0 struct Hook *hook,
; 146: 			          register __a2 struct SGWork *sgw,
; 147: 			          register __a1 ULONG *msg)
; 148: {
       | 040A  4E55 FFF4                      LINK      A5,#FFF4
       | 040E  48E7 2130                      MOVEM.L   D2/D7/A2-A3,-(A7)
       | 0412  41EE  0000-XX.2                LEA       _LinkerDB(A6),A0
       | 0416  2648                           MOVEA.L   A0,A3
       | 0418  2B49 FFF8                      MOVE.L    A1,FFF8(A5)
; 149:     ULONG return_code;
; 150:     UBYTE *pass_ptr;
; 151: 
; 152:     pass_ptr = (STRPTR)hook->h_SubEntry;
       | 041C  204B                           MOVEA.L   A3,A0
       | 041E  2668 000C                      MOVEA.L   000C(A0),A3
; 153: 
; 154:     kprintf("Hook:             %lx\n",hook);
       | 0422  2F08                           MOVE.L    A0,-(A7)
       | 0424  487A FC28                      PEA       FC28(PC)
       | 0428  2F48 0018                      MOVE.L    A0,0018(A7)
       | 042C  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 155:     kprintf("Hook->h_Entry:    %lx\n",hook->h_Entry);
       | 0430  206F 0018                      MOVEA.L   0018(A7),A0
       | 0434  2EA8 0008                      MOVE.L    0008(A0),(A7)
       | 0438  487A FC2C                      PEA       FC2C(PC)
       | 043C  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 156:     kprintf("Hook->h_SubEntry: %lx\n",hook->h_SubEntry);
       | 0440  206F 001C                      MOVEA.L   001C(A7),A0
       | 0444  2EA8 000C                      MOVE.L    000C(A0),(A7)
       | 0448  487A FC34                      PEA       FC34(PC)
       | 044C  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 157:     kprintf("Hook->h_Data:     %lx\n",hook->h_Data);
       | 0450  206F 0020                      MOVEA.L   0020(A7),A0
       | 0454  2EA8 0010                      MOVE.L    0010(A0),(A7)
       | 0458  487A FC3C                      PEA       FC3C(PC)
       | 045C  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 0460  4FEF 0014                      LEA       0014(A7),A7
; 158: 
; 159:     return_code = ~0L;
       | 0464  7EFF                           MOVEQ     #FF,D7
; 160: 
; 161:     if(*msg == SGH_KEY)
       | 0466  206D FFF8                      MOVEA.L   FFF8(A5),A0
       | 046A  2010                           MOVE.L    (A0),D0
       | 046C  7201                           MOVEQ     #01,D1
       | 046E  B081                           CMP.L     D1,D0
       | 0470  6662                           BNE.B     04D4
; 162:     {
; 163:     	if((sgw->EditOp == EO_REPLACECHAR) ||
       | 0472  322A 002A                      MOVE.W    002A(A2),D1
       | 0476  7407                           MOVEQ     #07,D2
       | 0478  B242                           CMP.W     D2,D1
       | 047A  6706                           BEQ.B     0482
; 164:     	   (sgw->EditOp == EO_INSERTCHAR))
       | 047C  7408                           MOVEQ     #08,D2
       | 047E  B242                           CMP.W     D2,D1
       | 0480  662E                           BNE.B     04B0
; 165:     	{
; 166:     	    pass_ptr[sgw->BufferPos - 1] = sgw->WorkBuffer[sgw->BufferPos - 1];
       | 0482  342A 001A                      MOVE.W    001A(A2),D2
       | 0486  48C2                           EXT.L     D2
       | 0488  206A 0008                      MOVEA.L   0008(A2),A0
       | 048C  2248                           MOVEA.L   A0,A1
       | 048E  D3C2                           ADDA.L    D2,A1
       | 0490  342A 001A                      MOVE.W    001A(A2),D2
       | 0494  48C2                           EXT.L     D2
       | 0496  17A9 FFFF 28FF                 MOVE.B    FFFF(A1),FF(A3,D2.L)
; 167:     	    sgw->WorkBuffer[sgw->BufferPos - 1] = '·';
       | 049C  302A 001A                      MOVE.W    001A(A2),D0
       | 04A0  48C0                           EXT.L     D0
       | 04A2  206A 0008                      MOVEA.L   0008(A2),A0
       | 04A6  D1C0                           ADDA.L    D0,A0
       | 04A8  117C 00B7 FFFF                 MOVE.B    #B7,FFFF(A0)
; 168:     	}
; 169:     	else if(sgw->EditOp == EO_MOVECURSOR)
       | 04AE  6032                           BRA.B     04E2
       | 04B0  7404                           MOVEQ     #04,D2
       | 04B2  B242                           CMP.W     D2,D1
       | 04B4  660E                           BNE.B     04C4
; 170:     	{
; 171:     	    sgw->Actions |= SGA_BEEP;
       | 04B6  08EA 0002 0021                 BSET      #0002,0021(A2)
; 172:     	    sgw->Actions &= ~SGA_USE;
       | 04BC  08AA 0000 0021                 BCLR      #0000,0021(A2)
; 173:     	}
; 174:     	else if(sgw->EditOp == EO_DELBACKWARD)
       | 04C2  601E                           BRA.B     04E2
       | 04C4  5541                           SUBQ.W    #2,D1
       | 04C6  661A                           BNE.B     04E2
; 175:     	{
; 176:     	    pass_ptr[sgw->BufferPos] = 0;
       | 04C8  306A 001A                      MOVEA.W   001A(A2),A0
       | 04CC  2208                           MOVE.L    A0,D1
       | 04CE  4233 1000                      CLR.B     00(A3,D1.W)
; 177:     	}
; 178:     }
; 179:     else if(*msg == SGH_CLICK)
       | 04D2  600E                           BRA.B     04E2
       | 04D4  5580                           SUBQ.L    #2,D0
       | 04D6  6608                           BNE.B     04E0
; 180:     {
; 181:     	sgw->BufferPos = sgw->NumChars;
       | 04D8  356A 001C 001A                 MOVE.W    001C(A2),001A(A2)
; 182:     }
; 183:     else
       | 04DE  6002                           BRA.B     04E2
; 184:     	return_code = 0;
       | 04E0  7E00                           MOVEQ     #00,D7
; 185: 
; 186:     return(return_code);
       | 04E2  2007                           MOVE.L    D7,D0
; 187: }
       | 04E4  4CDF 0C84                      MOVEM.L   (A7)+,D2/D7/A2-A3
       | 04E8  4E5D                           UNLK      A5
       | 04EA  4E75                           RTS
; 188: 
; 189: 
; 190: /****** envoy.library/LoginRequestA *******************************************
; 191: *
; 192: *   NAME
; 193: *     LoginRequestA -- Create a std. requester for name and password
; 194: *
; 195: *   SYNOPSIS
; 196: *     ret = LoginRequestA(taglist)
; 197: *     D0                   A0
; 198: *
; 199: *     BOOL  LoginRequestA(struct TagList *);
; 200: *     BOOL  LoginRequest(tag1, tag2, ...);
; 201: *
; 202: *   FUNCTION
; 203: *     Creates a system requester that allows the user to enter his name
; 204: *     and password.
; 205: *
; 206: *   INPUTS
; 207: *     taglist - Made up of the following possible tags:
; 208: *
; 209: *           LREQ_NameBuff - Specifies a pointer to the buffer where you wish
; 210: *                           the user's name name to be stored when the user
; 211: *                           selects "OK".
; 212: *
; 213: *           LREQ_NameBuffLen - Maximum number of bytes allowed to be copied into
; 214: *                           the above buffer.
; 215: *
; 216: *	    LREQ_PassBuff - Specifies a pointer the buffer where you wish
; 217: *			    the user's password to be stored when the user
; 218: *			    selects "OK".
; 219: *
; 220: *	    LREQ_PassBuffLen - Maxmimum number of bytes allowed to be copied
; 221: *			    into the above buffer.
; 222: *
; 223: *           LREQ_Left     - Initial left coordinate of the requester.
; 224: *
; 225: *           LREQ_Top      - Initial top coordinate of the requester.
; 226: *
; 227: *           LREQ_Width    - Horizontal size of requester in pixels.
; 228: *
; 229: *           LREQ_Height   - Vertical size of requester in pixels.
; 230: *
; 231: *           LREQ_Screen   - Defines the screen on which this requester
; 232: *                           should be created.  If not provided, it will be
; 233: *                           opened on the workbench screen.
; 234: *
; 235: *           LREQ_Title    - Provides the name for the title bar of the
; 236: *                           requester's window.
; 237: *
; 238: *           LREQ_NoDragBar -
; 239: *                           Prevents the requester's window from opening
; 240: *                           with a dragbar gadget; the requester will be
; 241: *                           locked in at the original position.
; 242: *
; 243: *   RESULT
; 244: *     ret - either TRUE or FALSE, representing whether the requester was
; 245: *           successful or not.
; 246: *
; 247: *   EXAMPLE
; 248: *
; 249: *   NOTES
; 250: *
; 251: *   BUGS
; 252: *
; 253: *
; 254: ******************************************************************************
; 255: *
; 256: */
; 257: 
; 258: 
; 259: BOOL __asm LoginRequestA(register __a0 struct TagItem *intags)
; 260: {
       | 04EC  4E55 FFF4                      LINK      A5,#FFF4
       | 04F0  48E7 3F32                      MOVEM.L   D2-D7/A2-A3/A6,-(A7)
       | 04F4  2648                           MOVEA.L   A0,A3
; 261: 
; 262:     struct LoginReqData *lrd;
; 263:     BOOL status = FALSE;
       | 04F6  7E00                           MOVEQ     #00,D7
; 264: 
; 265:     if(lrd = AllocMem(sizeof(struct LoginReqData),MEMF_CLEAR|MEMF_PUBLIC))
       | 04F8  2F0E                           MOVE.L    A6,-(A7)
       | 04FA  7072                           MOVEQ     #72,D0
       | 04FC  D080                           ADD.L     D0,D0
       | 04FE  223C 0001 0001                 MOVE.L    #00010001,D1
       | 0504  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0508  4EAE FF3A                      JSR       FF3A(A6)
       | 050C  2C5F                           MOVEA.L   (A7)+,A6
       | 050E  2440                           MOVEA.L   D0,A2
       | 0510  200A                           MOVE.L    A2,D0
       | 0512  6700 048A                      BEQ.W     099E
; 266:     {
; 267:     	lrd->lrd_TagList = intags;
       | 0516  254B 0048                      MOVE.L    A3,0048(A2)
; 268: 
; 269:     	lrd->lrd_Width = 0;
       | 051A  7000                           MOVEQ     #00,D0
       | 051C  3540 00C0                      MOVE.W    D0,00C0(A2)
; 270:     	lrd->lrd_Height = 0;
       | 0520  3540 00C2                      MOVE.W    D0,00C2(A2)
; 271: 	lrd->lrd_DragBar = TRUE;
       | 0524  357C 0001 00C6                 MOVE.W    #0001,00C6(A2)
; 272: 	lrd->lrd_Title = "Login Requester";
       | 052A  41FA FB82                      LEA       FB82(PC),A0
       | 052E  2548 0070                      MOVE.L    A0,0070(A2)
; 273: 
; 274: 
; 275: 	lrd->lrd_EditHook.h_Entry = (HOOK_FUNC)&passwordHook;
       | 0532  41FA FED6                      LEA       FED6(PC),A0
       | 0536  2548 00D8                      MOVE.L    A0,00D8(A2)
; 276: 	lrd->lrd_EditHook.h_SubEntry = (HOOK_FUNC)&lrd->lrd_PassStr[0];
       | 053A  41EA 009C                      LEA       009C(A2),A0
       | 053E  2548 00DC                      MOVE.L    A0,00DC(A2)
; 277: 
; 278:     kprintf("EditHook:             %lx\n",&lrd->lrd_EditHook);
       | 0542  43EA 00D0                      LEA       00D0(A2),A1
       | 0546  2F09                           MOVE.L    A1,-(A7)
       | 0548  487A FB74                      PEA       FB74(PC)
       | 054C  2F48 002C                      MOVE.L    A0,002C(A7)
       | 0550  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 279:     kprintf("EditHook->h_Entry:    %lx\n",lrd->lrd_EditHook.h_Entry);
       | 0554  2EAA 00D8                      MOVE.L    00D8(A2),(A7)
       | 0558  487A FB80                      PEA       FB80(PC)
       | 055C  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 280:     kprintf("EditHook->h_SubEntry: %lx\n",lrd->lrd_EditHook.h_SubEntry);
       | 0560  2EAA 00DC                      MOVE.L    00DC(A2),(A7)
       | 0564  487A FB90                      PEA       FB90(PC)
       | 0568  4EBA  0000-XX.1                JSR       _kprintf(PC)
; 281:     kprintf("EsitHook->h_Data:     %lx\n",lrd->lrd_EditHook.h_Data);
       | 056C  2EAA 00E0                      MOVE.L    00E0(A2),(A7)
       | 0570  487A FBA0                      PEA       FBA0(PC)
       | 0574  4EBA  0000-XX.1                JSR       _kprintf(PC)
       | 0578  4FEF 0014                      LEA       0014(A7),A7
; 282: 
; 283:     	while(lrd->lrd_TagItem = NextTagItem(&lrd->lrd_TagList))
       | 057C  47EA 0048                      LEA       0048(A2),A3
       | 0580  6000 00F8                      BRA.W     067A
; 284:     	{
; 285:     	    switch(lrd->lrd_TagItem->ti_Tag)
       | 0584  206A 0044                      MOVEA.L   0044(A2),A0
       | 0588  2010                           MOVE.L    (A0),D0
       | 058A  0480 800B 1401                 SUBI.L    #800B1401,D0
       | 0590  6D00 00E8                      BLT.W     067A
       | 0594  0C80 0000 000E                 CMPI.L    #0000000E,D0
       | 059A  6C00 00DE                      BGE.W     067A
       | 059E  D040                           ADD.W     D0,D0
       | 05A0  303B 0006                      MOVE.W    06(PC,D0.W),D0
       | 05A4  4EFB 0004                      JMP       04(PC,D0.W)
       | 05A8  001A 0028                      ORI.B     #28,(A2)+
       | 05AC  0036 0044 0052                 ORI.B     #44,52(A6,D0.W)
       | 05B2  0060 006E                      ORI.W     #006E,-(A0)
       | 05B6  007C 0096                      ORI       #0096,SR
       | 05BA  008A 00CC 00A8                 ORI.L     #00CC00A8,A2
       | 05C0  00C0 00B4                      CMP2.B    D0,D0
; 286:     	    {
; 287:     	    	case LREQ_NameBuff	: lrd->lrd_NameBuff = (STRPTR)lrd->lrd_TagItem->ti_Data;
       | 05C4  206A 0044                      MOVEA.L   0044(A2),A0
       | 05C8  2568 0004 0060                 MOVE.L    0004(A0),0060(A2)
; 288:     	    				  break;
       | 05CE  6000 00AA                      BRA.W     067A
; 289: 
; 290:     	    	case LREQ_NameBuffLen	: lrd->lrd_NameBuffLen = lrd->lrd_TagItem->ti_Data;
       | 05D2  206A 0044                      MOVEA.L   0044(A2),A0
       | 05D6  2568 0004 0064                 MOVE.L    0004(A0),0064(A2)
; 291:     	    				  break;
       | 05DC  6000 009C                      BRA.W     067A
; 292: 
; 293:     	    	case LREQ_PassBuff	: lrd->lrd_PassBuff = (STRPTR)lrd->lrd_TagItem->ti_Data;
       | 05E0  206A 0044                      MOVEA.L   0044(A2),A0
       | 05E4  2568 0004 0068                 MOVE.L    0004(A0),0068(A2)
; 294:     	    				  break;
       | 05EA  6000 008E                      BRA.W     067A
; 295: 
; 296:     	    	case LREQ_PassBuffLen	: lrd->lrd_PassBuffLen = lrd->lrd_TagItem->ti_Data;
       | 05EE  206A 0044                      MOVEA.L   0044(A2),A0
       | 05F2  2568 0004 006C                 MOVE.L    0004(A0),006C(A2)
; 297:     	    				  break;
       | 05F8  6000 0080                      BRA.W     067A
; 298: 
; 299:     	    	case LREQ_Left		: lrd->lrd_Left = lrd->lrd_TagItem->ti_Data;
       | 05FC  206A 0044                      MOVEA.L   0044(A2),A0
       | 0600  2028 0004                      MOVE.L    0004(A0),D0
       | 0604  3540 00BC                      MOVE.W    D0,00BC(A2)
; 300:     	    				  break;
       | 0608  6070                           BRA.B     067A
; 301: 
; 302:     	    	case LREQ_Top		: lrd->lrd_Top = lrd->lrd_TagItem->ti_Data;
       | 060A  206A 0044                      MOVEA.L   0044(A2),A0
       | 060E  2028 0004                      MOVE.L    0004(A0),D0
       | 0612  3540 00BE                      MOVE.W    D0,00BE(A2)
; 303:     	    				  break;
       | 0616  6062                           BRA.B     067A
; 304: 
; 305:     	    	case LREQ_Width		: lrd->lrd_Width = lrd->lrd_TagItem->ti_Data;
       | 0618  206A 0044                      MOVEA.L   0044(A2),A0
       | 061C  2028 0004                      MOVE.L    0004(A0),D0
       | 0620  3540 00C0                      MOVE.W    D0,00C0(A2)
; 306:     	    				  break;
       | 0624  6054                           BRA.B     067A
; 307: 
; 308:     	    	case LREQ_Height	: lrd->lrd_Height = lrd->lrd_TagItem->ti_Data;
       | 0626  206A 0044                      MOVEA.L   0044(A2),A0
       | 062A  2028 0004                      MOVE.L    0004(A0),D0
       | 062E  3540 00C2                      MOVE.W    D0,00C2(A2)
; 309:     	    				  break;
       | 0632  6046                           BRA.B     067A
; 310: 
; 311:     	    	case LREQ_Title		: lrd->lrd_Title = (STRPTR)lrd->lrd_TagItem->ti_Data;
       | 0634  206A 0044                      MOVEA.L   0044(A2),A0
       | 0638  2568 0004 0070                 MOVE.L    0004(A0),0070(A2)
; 312:     	    				  break;
       | 063E  603A                           BRA.B     067A
; 313: 
; 314: 		case LREQ_Screen	: lrd->lrd_Screen = (struct Screen *)lrd->lrd_TagItem->ti_Data;
       | 0640  206A 0044                      MOVEA.L   0044(A2),A0
       | 0644  2568 0004 0014                 MOVE.L    0004(A0),0014(A2)
; 315: 					  lrd->lrd_CustomScreen = TRUE;
       | 064A  357C 0001 00C8                 MOVE.W    #0001,00C8(A2)
; 316: 					  break;
       | 0650  6028                           BRA.B     067A
; 317: 
; 318: 		case LREQ_Window	: lrd->lrd_BlockWindow = (struct Window *)lrd->lrd_TagItem->ti_Data;
       | 0652  206A 0044                      MOVEA.L   0044(A2),A0
       | 0656  2568 0004 0004                 MOVE.L    0004(A0),0004(A2)
; 319: 					  break;
       | 065C  601C                           BRA.B     067A
; 320: 
; 321: 		case LREQ_MsgPort	: lrd->lrd_RefreshPort = (struct MsgPort *)lrd->lrd_TagItem->ti_Data;
       | 065E  206A 0044                      MOVEA.L   0044(A2),A0
       | 0662  2568 0004 000C                 MOVE.L    0004(A0),000C(A2)
; 322: 					  break;
       | 0668  6010                           BRA.B     067A
; 323: 
; 324: 		case LREQ_CallBack	: lrd->lrd_Hook = (struct Hook *)lrd->lrd_TagItem->ti_Data;
       | 066A  206A 0044                      MOVEA.L   0044(A2),A0
       | 066E  2568 0004 0010                 MOVE.L    0004(A0),0010(A2)
; 325: 					  break;
       | 0674  6004                           BRA.B     067A
; 326: 
; 327:     	    	case LREQ_NoDragBar	: lrd->lrd_DragBar = FALSE;
       | 0676  426A 00C6                      CLR.W     00C6(A2)
; 328:     	    				  break;
; 329:     	    }
; 330:         }
       | 067A  2F0E                           MOVE.L    A6,-(A7)
       | 067C  204B                           MOVEA.L   A3,A0
       | 067E  2C6E 0026                      MOVEA.L   0026(A6),A6
       | 0682  4EAE FFD0                      JSR       FFD0(A6)
       | 0686  2C5F                           MOVEA.L   (A7)+,A6
       | 0688  2540 0044                      MOVE.L    D0,0044(A2)
       | 068C  6600 FEF6                      BNE.W     0584
; 331: 
; 332:     	if(!lrd->lrd_CustomScreen)
       | 0690  4A6A 00C8                      TST.W     00C8(A2)
       | 0694  6612                           BNE.B     06A8
; 333:     	    lrd->lrd_Screen = LockPubScreen(NULL);
       | 0696  2F0E                           MOVE.L    A6,-(A7)
       | 0698  91C8                           SUBA.L    A0,A0
       | 069A  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 069E  4EAE FE02                      JSR       FE02(A6)
       | 06A2  2C5F                           MOVEA.L   (A7)+,A6
       | 06A4  2540 0014                      MOVE.L    D0,0014(A2)
; 334: 
; 335:     	lrd->lrd_FontAttr = lrd->lrd_Screen->Font;
       | 06A8  206A 0014                      MOVEA.L   0014(A2),A0
       | 06AC  2568 0028 0020                 MOVE.L    0028(A0),0020(A2)
; 336: 
; 337:     	if(lrd->lrd_Font = OpenFont(lrd->lrd_FontAttr))
       | 06B2  2F0E                           MOVE.L    A6,-(A7)
       | 06B4  206A 0020                      MOVEA.L   0020(A2),A0
       | 06B8  2C6E 003A                      MOVEA.L   003A(A6),A6
       | 06BC  4EAE FFB8                      JSR       FFB8(A6)
       | 06C0  2C5F                           MOVEA.L   (A7)+,A6
       | 06C2  2540 001C                      MOVE.L    D0,001C(A2)
       | 06C6  6700 02C4                      BEQ.W     098C
; 338: 	{
; 339: 	    lrd->lrd_IText.ITextFont = lrd->lrd_FontAttr;
       | 06CA  256A 0020 0054                 MOVE.L    0020(A2),0054(A2)
; 340: 	    lrd->lrd_IText.IText = "N";
       | 06D0  41FA FA5C                      LEA       FA5C(PC),A0
       | 06D4  2548 0058                      MOVE.L    A0,0058(A2)
; 341: 	    lrd->lrd_NSpace = IntuiTextLength(&lrd->lrd_IText);
       | 06D8  41EA 004C                      LEA       004C(A2),A0
       | 06DC  2F0E                           MOVE.L    A6,-(A7)
       | 06DE  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 06E2  4EAE FEB6                      JSR       FEB6(A6)
       | 06E6  2C5F                           MOVEA.L   (A7)+,A6
       | 06E8  3540 0024                      MOVE.W    D0,0024(A2)
; 342: 	    lrd->lrd_NHeight = lrd->lrd_Font->tf_YSize;
       | 06EC  206A 001C                      MOVEA.L   001C(A2),A0
       | 06F0  3228 0014                      MOVE.W    0014(A0),D1
       | 06F4  3541 0026                      MOVE.W    D1,0026(A2)
; 343: 	    lrd->lrd_OptimalWidth = (lrd->lrd_NSpace)*25 + 20 + lrd->lrd_Screen->WBorLeft + lrd->lrd_Screen->WBorRight;
       | 06F8  206A 0014                      MOVEA.L   0014(A2),A0
       | 06FC  1428 0025                      MOVE.B    0025(A0),D2
       | 0700  4882                           EXT.W     D2
       | 0702  48C2                           EXT.L     D2
       | 0704  1628 0024                      MOVE.B    0024(A0),D3
       | 0708  4883                           EXT.W     D3
       | 070A  48C3                           EXT.L     D3
       | 070C  C0FC 0019                      MULU.W    #0019,D0
       | 0710  D083                           ADD.L     D3,D0
       | 0712  D082                           ADD.L     D2,D0
       | 0714  7414                           MOVEQ     #14,D2
       | 0716  D082                           ADD.L     D2,D0
       | 0718  3540 0028                      MOVE.W    D0,0028(A2)
; 344: 	    lrd->lrd_OptimalHeight = (lrd->lrd_NHeight) * 4 + 1 + 2 + 8 + 18 + 8 + lrd->lrd_Screen->WBorBottom;
       | 071C  1428 0026                      MOVE.B    0026(A0),D2
       | 0720  4882                           EXT.W     D2
       | 0722  48C2                           EXT.L     D2
       | 0724  7600                           MOVEQ     #00,D3
       | 0726  3601                           MOVE.W    D1,D3
       | 0728  E583                           ASL.L     #2,D3
       | 072A  D682                           ADD.L     D2,D3
       | 072C  7225                           MOVEQ     #25,D1
       | 072E  D681                           ADD.L     D1,D3
       | 0730  3543 002A                      MOVE.W    D3,002A(A2)
; 345: 
; 346: 	    if(!lrd->lrd_Width)
       | 0734  4A6A 00C0                      TST.W     00C0(A2)
       | 0738  6608                           BNE.B     0742
; 347: 	    	lrd->lrd_Width = lrd->lrd_OptimalWidth;
       | 073A  302A 0028                      MOVE.W    0028(A2),D0
       | 073E  3540 00C0                      MOVE.W    D0,00C0(A2)
; 348: 
; 349: 	    if(!lrd->lrd_Height)
       | 0742  4A6A 00C2                      TST.W     00C2(A2)
       | 0746  6608                           BNE.B     0750
; 350: 	    	lrd->lrd_Height = lrd->lrd_OptimalHeight;
       | 0748  302A 002A                      MOVE.W    002A(A2),D0
       | 074C  3540 00C2                      MOVE.W    D0,00C2(A2)
; 351: 
; 352:             if(lrd->lrd_Window = OpenWinTags(WA_DragBar,TRUE,
; 353:                                              WA_DepthGadget,TRUE,
; 354:                                              WA_Left,lrd->lrd_Left,
       | 0750  7000                           MOVEQ     #00,D0
       | 0752  302A 00BC                      MOVE.W    00BC(A2),D0
; 355:                                              WA_Top,lrd->lrd_Top,
       | 0756  7200                           MOVEQ     #00,D1
       | 0758  322A 00BE                      MOVE.W    00BE(A2),D1
; 356:                                              WA_Width,lrd->lrd_Width,
       | 075C  7400                           MOVEQ     #00,D2
       | 075E  342A 00C0                      MOVE.W    00C0(A2),D2
; 357:                                              WA_Height,lrd->lrd_Height,
       | 0762  7600                           MOVEQ     #00,D3
       | 0764  362A 00C2                      MOVE.W    00C2(A2),D3
; 358:                                              WA_Activate,TRUE,
; 359:                                              WA_Title,(ULONG)lrd->lrd_Title,
       | 0768  282A 0070                      MOVE.L    0070(A2),D4
; 360:                                              WA_PubScreen,lrd->lrd_Screen,
; 361:                                              WA_DragBar,lrd->lrd_DragBar,
       | 076C  3A2A 00C6                      MOVE.W    00C6(A2),D5
       | 0770  48C5                           EXT.L     D5
; 362:                                              WA_AutoAdjust,TRUE,
; 363:                                              WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP,
; 364:                                              TAG_DONE,TRUE))
       | 0772  2F40 0028                      MOVE.L    D0,0028(A7)
       | 0776  7001                           MOVEQ     #01,D0
       | 0778  2F00                           MOVE.L    D0,-(A7)
       | 077A  42A7                           CLR.L     -(A7)
       | 077C  4878 0260                      PEA       0260
       | 0780  2F3C 8000 006A                 MOVE.L    #8000006A,-(A7)
       | 0786  2F00                           MOVE.L    D0,-(A7)
       | 0788  2F3C 8000 0090                 MOVE.L    #80000090,-(A7)
       | 078E  2F05                           MOVE.L    D5,-(A7)
       | 0790  2F41 0048                      MOVE.L    D1,0048(A7)
       | 0794  223C 8000 0082                 MOVE.L    #80000082,D1
       | 079A  2F01                           MOVE.L    D1,-(A7)
       | 079C  2F2A 0014                      MOVE.L    0014(A2),-(A7)
       | 07A0  2F3C 8000 0079                 MOVE.L    #80000079,-(A7)
       | 07A6  2F04                           MOVE.L    D4,-(A7)
       | 07A8  2F3C 8000 006E                 MOVE.L    #8000006E,-(A7)
       | 07AE  2F00                           MOVE.L    D0,-(A7)
       | 07B0  2F3C 8000 0089                 MOVE.L    #80000089,-(A7)
       | 07B6  2F03                           MOVE.L    D3,-(A7)
       | 07B8  2F3C 8000 0067                 MOVE.L    #80000067,-(A7)
       | 07BE  2F02                           MOVE.L    D2,-(A7)
       | 07C0  2F3C 8000 0066                 MOVE.L    #80000066,-(A7)
       | 07C6  2F2F 0074                      MOVE.L    0074(A7),-(A7)
       | 07CA  2F3C 8000 0065                 MOVE.L    #80000065,-(A7)
       | 07D0  2F2F 0078                      MOVE.L    0078(A7),-(A7)
       | 07D4  2F3C 8000 0064                 MOVE.L    #80000064,-(A7)
       | 07DA  2F00                           MOVE.L    D0,-(A7)
       | 07DC  2F3C 8000 0083                 MOVE.L    #80000083,-(A7)
       | 07E2  2F00                           MOVE.L    D0,-(A7)
       | 07E4  2F01                           MOVE.L    D1,-(A7)
       | 07E6  6100 F97E                      BSR.W     0166
       | 07EA  4FEF 0068                      LEA       0068(A7),A7
       | 07EE  2480                           MOVE.L    D0,(A2)
       | 07F0  6700 018A                      BEQ.W     097C
; 365:        {
; 366:             	if(!lrd->lrd_CustomScreen)
       | 07F4  4A6A 00C8                      TST.W     00C8(A2)
       | 07F8  6612                           BNE.B     080C
; 367:             	    UnlockPubScreen(NULL,lrd->lrd_Screen);
       | 07FA  2F0E                           MOVE.L    A6,-(A7)
       | 07FC  91C8                           SUBA.L    A0,A0
       | 07FE  226A 0014                      MOVEA.L   0014(A2),A1
       | 0802  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 0806  4EAE FDFC                      JSR       FDFC(A6)
       | 080A  2C5F                           MOVEA.L   (A7)+,A6
; 368: 
; 369: //		    	    if(lrd->lrd_BlockWindow)
; 370: //    			        BusyWindow(lrd,lrd->lrd_BlockWindow,TRUE);
; 371: 
; 372: 
; 373:                 if(lrd->lrd_Window->Width >= lrd->lrd_OptimalWidth)
       | 080C  2052                           MOVEA.L   (A2),A0
       | 080E  3028 0008                      MOVE.W    0008(A0),D0
       | 0812  B06A 0028                      CMP.W     0028(A2),D0
       | 0816  6D00 0156                      BLT.W     096E
; 374:                 {
; 375:                     if(lrd->lrd_Window->Height >= lrd->lrd_OptimalHeight)
       | 081A  3028 000A                      MOVE.W    000A(A0),D0
       | 081E  B06A 002A                      CMP.W     002A(A2),D0
       | 0822  6D00 014A                      BLT.W     096E
; 376:                     {
; 377:                         BOOL cont;
; 378: 
; 379:                         lrd->lrd_VisualInfo = (APTR) GetVisInfo(lrd->lrd_Screen,TAG_DONE);
       | 0826  42A7                           CLR.L     -(A7)
       | 0828  2F2A 0014                      MOVE.L    0014(A2),-(A7)
       | 082C  6100 F7D2                      BSR.W     0000
       | 0830  2540 0018                      MOVE.L    D0,0018(A2)
; 380:                         if(CreateReqGads(lrd))
       | 0834  204A                           MOVEA.L   A2,A0
       | 0836  6100 F94C                      BSR.W     0184
       | 083A  504F                           ADDQ.W    #8,A7
       | 083C  4A40                           TST.W     D0
       | 083E  6700 012E                      BEQ.W     096E
; 381:                         {
; 382: 		    	    lrd->lrd_Mask = (1L << lrd->lrd_Window->UserPort->mp_SigBit);
       | 0842  2052                           MOVEA.L   (A2),A0
       | 0844  2268 0056                      MOVEA.L   0056(A0),A1
       | 0848  7000                           MOVEQ     #00,D0
       | 084A  1029 000F                      MOVE.B    000F(A1),D0
       | 084E  7201                           MOVEQ     #01,D1
       | 0850  2401                           MOVE.L    D1,D2
       | 0852  E1A2                           ASL.L     D0,D2
       | 0854  2542 0074                      MOVE.L    D2,0074(A2)
; 383: 
; 384: 		    	    if(lrd->lrd_Hook && lrd->lrd_RefreshPort && lrd->lrd_BlockWindow)
       | 0858  4AAA 0010                      TST.L     0010(A2)
       | 085C  671C                           BEQ.B     087A
       | 085E  4AAA 000C                      TST.L     000C(A2)
       | 0862  6716                           BEQ.B     087A
       | 0864  4AAA 0004                      TST.L     0004(A2)
       | 0868  6710                           BEQ.B     087A
; 385: 		    	    	lrd->lrd_BlockMask = (1L << lrd->lrd_RefreshPort->mp_SigBit);
       | 086A  206A 000C                      MOVEA.L   000C(A2),A0
       | 086E  7000                           MOVEQ     #00,D0
       | 0870  1028 000F                      MOVE.B    000F(A0),D0
       | 0874  E1A1                           ASL.L     D0,D1
       | 0876  2541 0078                      MOVE.L    D1,0078(A2)
; 386: 
; 387: 		    	    cont = TRUE;
       | 087A  7C01                           MOVEQ     #01,D6
; 388: 
; 389: 			    while(cont)
       | 087C  6000 00D4                      BRA.W     0952
; 390: 			    {
; 391: 			    	struct IntuiMessage *im;
; 392: 
; 393: 			    	ULONG sigs;
; 394: 
; 395: 			    	sigs = Wait(lrd->lrd_Mask | lrd->lrd_BlockMask);
       | 0880  202A 0074                      MOVE.L    0074(A2),D0
       | 0884  80AA 0078                      OR.L      0078(A2),D0
       | 0888  2F0E                           MOVE.L    A6,-(A7)
       | 088A  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 088E  4EAE FEC2                      JSR       FEC2(A6)
       | 0892  2C5F                           MOVEA.L   (A7)+,A6
; 396: 
; 397: 			    	if(sigs & lrd->lrd_Mask)
       | 0894  C0AA 0074                      AND.L     0074(A2),D0
       | 0898  6700 00B8                      BEQ.W     0952
       | 089C  6000 009A                      BRA.W     0938
; 398: 			    	{
; 399: 			    	    while(im = (struct IntuiMessage *) GT_GetIMsg(lrd->lrd_Window->UserPort))
; 400: 			    	    {
; 401: 			    	    	switch(im->Class)
       | 08A0  202B 0014                      MOVE.L    0014(A3),D0
       | 08A4  5980                           SUBQ.L    #4,D0
       | 08A6  6772                           BEQ.B     091A
       | 08A8  721C                           MOVEQ     #1C,D1
       | 08AA  9081                           SUB.L     D1,D0
       | 08AC  6706                           BEQ.B     08B4
       | 08AE  7220                           MOVEQ     #20,D1
       | 08B0  9081                           SUB.L     D1,D0
       | 08B2  6676                           BNE.B     092A
; 402: 			    	    	{
; 403: 			    	    		case IDCMP_GADGETUP:
; 404: 			    	    		case IDCMP_GADGETDOWN:	switch((((struct Gadget *)im->IAddress))->GadgetID)
       | 08B4  206B 001C                      MOVEA.L   001C(A3),A0
       | 08B8  3028 0026                      MOVE.W    0026(A0),D0
       | 08BC  5340                           SUBQ.W    #1,D0
       | 08BE  6740                           BEQ.B     0900
       | 08C0  5540                           SUBQ.W    #2,D0
       | 08C2  6708                           BEQ.B     08CC
       | 08C4  5340                           SUBQ.W    #1,D0
       | 08C6  6662                           BNE.B     092A
; 405: 			    	    					{
; 406: 			    	    					    case ID_CANCEL: cont = FALSE;
       | 08C8  7C00                           MOVEQ     #00,D6
; 407: 			    	    					    		    break;
       | 08CA  605E                           BRA.B     092A
; 408: 
; 409: 			    	    					    case ID_OK:     if(lrd->lrd_NameBuff)
       | 08CC  202A 0060                      MOVE.L    0060(A2),D0
       | 08D0  6714                           BEQ.B     08E6
; 410: 			    	    					    		    {
; 411: 												stccpy(lrd->lrd_NameBuff,((struct StringInfo *)lrd->lrd_UserName->SpecialInfo)->Buffer,lrd->lrd_NameBuffLen);
       | 08D2  206A 0034                      MOVEA.L   0034(A2),A0
       | 08D6  2268 0022                      MOVEA.L   0022(A0),A1
       | 08DA  2040                           MOVEA.L   D0,A0
       | 08DC  202A 0064                      MOVE.L    0064(A2),D0
       | 08E0  2251                           MOVEA.L   (A1),A1
       | 08E2  4EBA  0000-XX.1                JSR       @stccpy(PC)
; 412: 											    }
; 413: 											    if(lrd->lrd_PassBuff)
       | 08E6  202A 0068                      MOVE.L    0068(A2),D0
       | 08EA  670E                           BEQ.B     08FA
; 414: 											    {
; 415: 											    	stccpy(lrd->lrd_PassBuff,lrd->lrd_PassStr /* ((struct StringInfo *)lrd->lrd_Password->SpecialInfo)->Buffer */,lrd->lrd_PassBuffLen);
       | 08EC  2040                           MOVEA.L   D0,A0
       | 08EE  202A 006C                      MOVE.L    006C(A2),D0
       | 08F2  226F 0024                      MOVEA.L   0024(A7),A1
       | 08F6  4EBA  0000-XX.1                JSR       @stccpy(PC)
; 416: 											    }
; 417: 											    cont = FALSE;
       | 08FA  7C00                           MOVEQ     #00,D6
; 418: 											    status = TRUE;
       | 08FC  7E01                           MOVEQ     #01,D7
; 419: 											    break;
       | 08FE  602A                           BRA.B     092A
; 420: 
; 421: 									    case ID_USERNAME:
; 422: 									    		    ActivateGadget(lrd->lrd_Password,lrd->lrd_Window,NULL);
       | 0900  48E7 0022                      MOVEM.L   A2/A6,-(A7)
       | 0904  206A 0038                      MOVEA.L   0038(A2),A0
       | 0908  2252                           MOVEA.L   (A2),A1
       | 090A  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 090E  95CA                           SUBA.L    A2,A2
       | 0910  4EAE FE32                      JSR       FE32(A6)
       | 0914  4CDF 4400                      MOVEM.L   (A7)+,A2/A6
; 423: 									    		    break;
; 424: 
; 425: 									}
; 426: 			    	    					break;
       | 0918  6010                           BRA.B     092A
; 427: 
; 428: 			    	    		case IDCMP_REFRESHWINDOW:
; 429: 			    	    					GT_RefreshWindow(lrd->lrd_Window,NULL);
       | 091A  2F0E                           MOVE.L    A6,-(A7)
       | 091C  2052                           MOVEA.L   (A2),A0
       | 091E  93C9                           SUBA.L    A1,A1
       | 0920  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0924  4EAE FFAC                      JSR       FFAC(A6)
       | 0928  2C5F                           MOVEA.L   (A7)+,A6
; 430: 			    	    					break;
; 431: 
; 432: 
; 433: 
; 434: 			    	    	}
; 435: 			    	    	GT_ReplyIMsg(im);
       | 092A  2F0E                           MOVE.L    A6,-(A7)
       | 092C  224B                           MOVEA.L   A3,A1
       | 092E  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0932  4EAE FFB2                      JSR       FFB2(A6)
       | 0936  2C5F                           MOVEA.L   (A7)+,A6
; 436: 			    	    }
       | 0938  2052                           MOVEA.L   (A2),A0
       | 093A  2F0E                           MOVE.L    A6,-(A7)
       | 093C  2068 0056                      MOVEA.L   0056(A0),A0
       | 0940  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0944  4EAE FFB8                      JSR       FFB8(A6)
       | 0948  2C5F                           MOVEA.L   (A7)+,A6
       | 094A  2640                           MOVEA.L   D0,A3
       | 094C  200B                           MOVE.L    A3,D0
       | 094E  6600 FF50                      BNE.W     08A0
; 437: 			    	}
; 438: 			    }
       | 0952  4A46                           TST.W     D6
       | 0954  6600 FF2A                      BNE.W     0880
; 439: 			    FreeVisualInfo(lrd->lrd_VisualInfo);
       | 0958  2F0E                           MOVE.L    A6,-(A7)
       | 095A  206A 0018                      MOVEA.L   0018(A2),A0
       | 095E  2C6E 0032                      MOVEA.L   0032(A6),A6
       | 0962  4EAE FF7C                      JSR       FF7C(A6)
       | 0966  2C5F                           MOVEA.L   (A7)+,A6
; 440: 
; 441: 			    DeleteReqGads(lrd);
       | 0968  204A                           MOVEA.L   A2,A0
       | 096A  6100 FA6E                      BSR.W     03DA
; 442: 			}
; 443: 		    }
; 444: 		}
; 445: //			    if(lrd->lrd_BlockWindow)
; 446: //			    	BusyWindow(lrd,lrd->lrd_BlockWindow,FALSE);
; 447: 
; 448: 
; 449: 		CloseWindow(lrd->lrd_Window);
       | 096E  2F0E                           MOVE.L    A6,-(A7)
       | 0970  2052                           MOVEA.L   (A2),A0
       | 0972  2C6E 0036                      MOVEA.L   0036(A6),A6
       | 0976  4EAE FFB8                      JSR       FFB8(A6)
       | 097A  2C5F                           MOVEA.L   (A7)+,A6
; 450: 	    }
; 451: 	    CloseFont(lrd->lrd_Font);
       | 097C  2F0E                           MOVE.L    A6,-(A7)
       | 097E  226A 001C                      MOVEA.L   001C(A2),A1
       | 0982  2C6E 003A                      MOVEA.L   003A(A6),A6
       | 0986  4EAE FFB2                      JSR       FFB2(A6)
       | 098A  2C5F                           MOVEA.L   (A7)+,A6
; 452: 	}
; 453: 	FreeMem(lrd,sizeof(struct LoginReqData));
       | 098C  2F0E                           MOVE.L    A6,-(A7)
       | 098E  224A                           MOVEA.L   A2,A1
       | 0990  7072                           MOVEQ     #72,D0
       | 0992  D080                           ADD.L     D0,D0
       | 0994  2C6E 002A                      MOVEA.L   002A(A6),A6
       | 0998  4EAE FF2E                      JSR       FF2E(A6)
       | 099C  2C5F                           MOVEA.L   (A7)+,A6
; 454:     }
; 455:     return(status);
       | 099E  2007                           MOVE.L    D7,D0
; 456: }
       | 09A0  4CDF 4CFC                      MOVEM.L   (A7)+,D2-D7/A2-A3/A6
       | 09A4  4E5D                           UNLK      A5
       | 09A6  4E75                           RTS

SECTION 01 " " 00000078 BYTES
OFFSETS 0000 THROUGH 0007 CONTAIN ZERO
0008 00000026-00 00.00000026
OFFSETS 000C THROUGH 000F CONTAIN ZERO
0010 00 01 00 00 00 01 00 00 00 00 00 00 00 00 00 00 ................
OFFSETS 0020 THROUGH 0025 CONTAIN ZERO
0026 0000002C-00 00.0000002C
002A 00 00 00 00 00 02 ......
0030 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 ................
OFFSETS 0040 THROUGH 0043 CONTAIN ZERO
0044 00000036-00 00.00000036
0048 00 00 00 00 00 03 00 00 ........
0050 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
OFFSETS 0060 THROUGH 0061 CONTAIN ZERO
0062 0000003A-00 00.0000003A
0066 00 00 00 00 00 04 00 00 00 10 ..........
OFFSETS 0070 THROUGH 0077 CONTAIN ZERO
