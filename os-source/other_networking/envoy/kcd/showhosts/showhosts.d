SAS AMIGA 680x0OBJ Module Disassembler V6.0
Copyright © 1992 SAS Institute, Inc.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_main 0000-00    _CallBack 0294-00    _NIPCBase 0000-02    
_UtilityBase 0004-02    _GlobalList 0008-02

SECTION 00 "text" 000003D0 BYTES
       | 0000  200F                           MOVE.L      A7,D0
       | 0002  907C 00A4                      SUB.W       #00A4,D0
       | 0006  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 000A  6500  0000-XX.1                BCS.W       __XCOVF
       | 000E  9EFC 0088                      SUBA.W      #0088,A7
       | 0012  48E7 3F16                      MOVEM.L     D2-D7/A3/A5-A6,-(A7)
       | 0016  2E2F 00B0                      MOVE.L      00B0(A7),D7
       | 001A  2A6F 00B4                      MOVEA.L     00B4(A7),A5
       | 001E  41EC  0000-01.2                LEA         01.00000000(A4),A0
       | 0022  43EF 0078                      LEA         0078(A7),A1
       | 0026  7007                           MOVEQ       #07,D0
       | 0028  22D8                           MOVE.L      (A0)+,(A1)+
       | 002A  51C8 FFFC                      DBF         D0,0028
       | 002E  7C00                           MOVEQ       #00,D6
       | 0030  7A00                           MOVEQ       #00,D5
       | 0032  486C  0008-02.2                PEA         02.00000008(A4)
       | 0036  4EBA  0000-XX.1                JSR         _NewList(PC)
       | 003A  584F                           ADDQ.W      #4,A7
       | 003C  2007                           MOVE.L      D7,D0
       | 003E  5580                           SUBQ.L      #2,D0
       | 0040  661C                           BNE.B       005E
       | 0042  206D 0004                      MOVEA.L     0004(A5),A0
       | 0046  5288                           ADDQ.L      #1,A0
       | 0048  703F                           MOVEQ       #3F,D0
       | 004A  B010                           CMP.B       (A0),D0
       | 004C  6610                           BNE.B       005E
       | 004E  486C  0020-01.2                PEA         01.00000020(A4)
       | 0052  4EBA  0000-XX.1                JSR         __writes(PC)
       | 0056  4297                           CLR.L       (A7)
       | 0058  4EBA  0000-XX.1                JSR         _exit(PC)
       | 005C  584F                           ADDQ.W      #4,A7
       | 005E  7A00                           MOVEQ       #00,D5
       | 0060  7002                           MOVEQ       #02,D0
       | 0062  BE80                           CMP.L       D0,D7
       | 0064  6608                           BNE.B       006E
       | 0066  4AAD 0004                      TST.L       0004(A5)
       | 006A  6602                           BNE.B       006E
       | 006C  7A01                           MOVEQ       #01,D5
       | 006E  2007                           MOVE.L      D7,D0
       | 0070  5380                           SUBQ.L      #1,D0
       | 0072  6602                           BNE.B       0076
       | 0074  7A01                           MOVEQ       #01,D5
       | 0076  4A45                           TST.W       D5
       | 0078  6778                           BEQ.B       00F2
       | 007A  43EC  0044-01.2                LEA         01.00000044(A4),A1
       | 007E  7025                           MOVEQ       #25,D0
       | 0080  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0084  4EAE FDD8                      JSR         FDD8(A6)
       | 0088  2940  0000-02.2                MOVE.L      D0,02.00000000(A4)
       | 008C  660A                           BNE.B       0098
       | 008E  4878 0001                      PEA         0001
       | 0092  4EBA  0000-XX.1                JSR         _exit(PC)
       | 0096  584F                           ADDQ.W      #4,A7
       | 0098  91C8                           SUBA.L      A0,A0
       | 009A  43EF 0038                      LEA         0038(A7),A1
       | 009E  7040                           MOVEQ       #40,D0
       | 00A0  2C6C  0000-02.2                MOVEA.L     02.00000000(A4),A6
       | 00A4  4EAE FF34                      JSR         FF34(A6)
       | 00A8  224E                           MOVEA.L     A6,A1
       | 00AA  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 00AE  4EAE FE62                      JSR         FE62(A6)
       | 00B2  486F 0038                      PEA         0038(A7)
       | 00B6  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 00BA  584F                           ADDQ.W      #4,A7
       | 00BC  42AF 0034                      CLR.L       0034(A7)
       | 00C0  2F40 0030                      MOVE.L      D0,0030(A7)
       | 00C4  202F 0034                      MOVE.L      0034(A7),D0
       | 00C8  B0AF 0030                      CMP.L       0030(A7),D0
       | 00CC  6C1A                           BGE.B       00E8
       | 00CE  41EF 0038                      LEA         0038(A7),A0
       | 00D2  2248                           MOVEA.L     A0,A1
       | 00D4  D3C0                           ADDA.L      D0,A1
       | 00D6  723A                           MOVEQ       #3A,D1
       | 00D8  B211                           CMP.B       (A1),D1
       | 00DA  6606                           BNE.B       00E2
       | 00DC  D1C0                           ADDA.L      D0,A0
       | 00DE  4210                           CLR.B       (A0)
       | 00E0  7C01                           MOVEQ       #01,D6
       | 00E2  52AF 0034                      ADDQ.L      #1,0034(A7)
       | 00E6  60DC                           BRA.B       00C4
       | 00E8  4A46                           TST.W       D6
       | 00EA  670C                           BEQ.B       00F8
       | 00EC  47EF 0038                      LEA         0038(A7),A3
       | 00F0  6006                           BRA.B       00F8
       | 00F2  266D 0004                      MOVEA.L     0004(A5),A3
       | 00F6  7C01                           MOVEQ       #01,D6
       | 00F8  2F0B                           MOVE.L      A3,-(A7)
       | 00FA  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 00FE  584F                           ADDQ.W      #4,A7
       | 0100  4A80                           TST.L       D0
       | 0102  6606                           BNE.B       010A
       | 0104  4A46                           TST.W       D6
       | 0106  6600 0182                      BNE.W       028A
       | 010A  43EC  0052-01.2                LEA         01.00000052(A4),A1
       | 010E  7025                           MOVEQ       #25,D0
       | 0110  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0114  4EAE FDD8                      JSR         FDD8(A6)
       | 0118  2940  0004-02.2                MOVE.L      D0,02.00000004(A4)
       | 011C  6700 016C                      BEQ.W       028A
       | 0120  43EC  0062-01.2                LEA         01.00000062(A4),A1
       | 0124  7025                           MOVEQ       #25,D0
       | 0126  4EAE FDD8                      JSR         FDD8(A6)
       | 012A  2940  0000-02.2                MOVE.L      D0,02.00000000(A4)
       | 012E  6700 014E                      BEQ.W       027E
       | 0132  2F4B 007C                      MOVE.L      A3,007C(A7)
       | 0136  4A46                           TST.W       D6
       | 0138  6606                           BNE.B       0140
       | 013A  7201                           MOVEQ       #01,D1
       | 013C  2F41 0078                      MOVE.L      D1,0078(A7)
       | 0140  41FA 0152                      LEA         0152(PC),A0
       | 0144  2F48 00A0                      MOVE.L      A0,00A0(A7)
       | 0148  2C40                           MOVEA.L     D0,A6
       | 014A  41EF 0098                      LEA         0098(A7),A0
       | 014E  7002                           MOVEQ       #02,D0
       | 0150  727D                           MOVEQ       #7D,D1
       | 0152  E589                           LSL.L       #2,D1
       | 0154  43EF 0078                      LEA         0078(A7),A1
       | 0158  4EAE FF2E                      JSR         FF2E(A6)
       | 015C  4A40                           TST.W       D0
       | 015E  670E                           BEQ.B       016E
       | 0160  203C 0000 8000                 MOVE.L      #00008000,D0
       | 0166  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 016A  4EAE FEC2                      JSR         FEC2(A6)
       | 016E  2F6C  0008-02.2  002C          MOVE.L      02.00000008(A4),002C(A7)
       | 0174  206F 002C                      MOVEA.L     002C(A7),A0
       | 0178  4A90                           TST.L       (A0)
       | 017A  6700 0080                      BEQ.W       01FC
       | 017E  2F48 0028                      MOVE.L      A0,0028(A7)
       | 0182  206F 0028                      MOVEA.L     0028(A7),A0
       | 0186  4A90                           TST.L       (A0)
       | 0188  6766                           BEQ.B       01F0
       | 018A  226F 002C                      MOVEA.L     002C(A7),A1
       | 018E  2F29 000A                      MOVE.L      000A(A1),-(A7)
       | 0192  2F28 000A                      MOVE.L      000A(A0),-(A7)
       | 0196  4EBA  0000-XX.1                JSR         _strcmp(PC)
       | 019A  504F                           ADDQ.W      #8,A7
       | 019C  4A80                           TST.L       D0
       | 019E  6646                           BNE.B       01E6
       | 01A0  206F 0028                      MOVEA.L     0028(A7),A0
       | 01A4  226F 002C                      MOVEA.L     002C(A7),A1
       | 01A8  B3C8                           CMPA.L      A0,A1
       | 01AA  673A                           BEQ.B       01E6
       | 01AC  2F68 0004 0028                 MOVE.L      0004(A0),0028(A7)
       | 01B2  2F48 0024                      MOVE.L      A0,0024(A7)
       | 01B6  2248                           MOVEA.L     A0,A1
       | 01B8  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 01BC  4EAE FF04                      JSR         FF04(A6)
       | 01C0  206F 0024                      MOVEA.L     0024(A7),A0
       | 01C4  2F28 000A                      MOVE.L      000A(A0),-(A7)
       | 01C8  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 01CC  584F                           ADDQ.W      #4,A7
       | 01CE  5280                           ADDQ.L      #1,D0
       | 01D0  226F 0024                      MOVEA.L     0024(A7),A1
       | 01D4  2269 000A                      MOVEA.L     000A(A1),A1
       | 01D8  4EAE FF2E                      JSR         FF2E(A6)
       | 01DC  226F 0024                      MOVEA.L     0024(A7),A1
       | 01E0  7016                           MOVEQ       #16,D0
       | 01E2  4EAE FF2E                      JSR         FF2E(A6)
       | 01E6  206F 0028                      MOVEA.L     0028(A7),A0
       | 01EA  2F50 0028                      MOVE.L      (A0),0028(A7)
       | 01EE  6092                           BRA.B       0182
       | 01F0  206F 002C                      MOVEA.L     002C(A7),A0
       | 01F4  2F50 002C                      MOVE.L      (A0),002C(A7)
       | 01F8  6000 FF7A                      BRA.W       0174
       | 01FC  41EC  0008-02.2                LEA         02.00000008(A4),A0
       | 0200  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0204  4EAE FEFE                      JSR         FEFE(A6)
       | 0208  2F40 002C                      MOVE.L      D0,002C(A7)
       | 020C  6764                           BEQ.B       0272
       | 020E  2040                           MOVEA.L     D0,A0
       | 0210  2228 000E                      MOVE.L      000E(A0),D1
       | 0214  2401                           MOVE.L      D1,D2
       | 0216  4242                           CLR.W       D2
       | 0218  4842                           SWAP        D2
       | 021A  E08A                           LSR.L       #8,D2
       | 021C  2601                           MOVE.L      D1,D3
       | 021E  4243                           CLR.W       D3
       | 0220  4843                           SWAP        D3
       | 0222  7800                           MOVEQ       #00,D4
       | 0224  4604                           NOT.B       D4
       | 0226  C684                           AND.L       D4,D3
       | 0228  2001                           MOVE.L      D1,D0
       | 022A  E088                           LSR.L       #8,D0
       | 022C  C084                           AND.L       D4,D0
       | 022E  C284                           AND.L       D4,D1
       | 0230  2F28 0012                      MOVE.L      0012(A0),-(A7)
       | 0234  2F01                           MOVE.L      D1,-(A7)
       | 0236  2F00                           MOVE.L      D0,-(A7)
       | 0238  2F03                           MOVE.L      D3,-(A7)
       | 023A  2F02                           MOVE.L      D2,-(A7)
       | 023C  2F28 000A                      MOVE.L      000A(A0),-(A7)
       | 0240  486C  0070-01.2                PEA         01.00000070(A4)
       | 0244  4EBA  0000-XX.1                JSR         _printf(PC)
       | 0248  206F 0048                      MOVEA.L     0048(A7),A0
       | 024C  2EA8 000A                      MOVE.L      000A(A0),(A7)
       | 0250  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0254  4FEF 001C                      LEA         001C(A7),A7
       | 0258  5280                           ADDQ.L      #1,D0
       | 025A  226F 002C                      MOVEA.L     002C(A7),A1
       | 025E  2269 000A                      MOVEA.L     000A(A1),A1
       | 0262  4EAE FF2E                      JSR         FF2E(A6)
       | 0266  226F 002C                      MOVEA.L     002C(A7),A1
       | 026A  7016                           MOVEQ       #16,D0
       | 026C  4EAE FF2E                      JSR         FF2E(A6)
       | 0270  608A                           BRA.B       01FC
       | 0272  226C  0000-02.2                MOVEA.L     02.00000000(A4),A1
       | 0276  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 027A  4EAE FE62                      JSR         FE62(A6)
       | 027E  226C  0004-02.2                MOVEA.L     02.00000004(A4),A1
       | 0282  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0286  4EAE FE62                      JSR         FE62(A6)
       | 028A  4CDF 68FC                      MOVEM.L     (A7)+,D2-D7/A3/A5-A6
       | 028E  DEFC 0088                      ADDA.W      #0088,A7
       | 0292  4E75                           RTS
       | 0294  200F                           MOVE.L      A7,D0
       | 0296  907C 001C                      SUB.W       #001C,D0
       | 029A  B0B9  0000 0000-XX             CMP.L       ___base,D0
       | 02A0  6500  0000-XX.1                BCS.W       __XCOVF
       | 02A4  9EFC 0014                      SUBA.W      #0014,A7
       | 02A8  48E7 013E                      MOVEM.L     D7/A2-A6,-(A7)
       | 02AC  2649                           MOVEA.L     A1,A3
       | 02AE  2A48                           MOVEA.L     A0,A5
       | 02B0  49F9  0000 0000-XX             LEA         _LinkerDB,A4
       | 02B6  2F4B 0024                      MOVE.L      A3,0024(A7)
       | 02BA  42AF 0020                      CLR.L       0020(A7)
       | 02BE  200B                           MOVE.L      A3,D0
       | 02C0  6700 00F2                      BEQ.W       03B4
       | 02C4  41EF 0024                      LEA         0024(A7),A0
       | 02C8  2C6C  0004-02.2                MOVEA.L     02.00000004(A4),A6
       | 02CC  4EAE FFD0                      JSR         FFD0(A6)
       | 02D0  2F40 0028                      MOVE.L      D0,0028(A7)
       | 02D4  6700 00EE                      BEQ.W       03C4
       | 02D8  2040                           MOVEA.L     D0,A0
       | 02DA  2010                           MOVE.L      (A0),D0
       | 02DC  0480 8000 2000                 SUBI.L      #80002000,D0
       | 02E2  6718                           BEQ.B       02FC
       | 02E4  5580                           SUBQ.L      #2,D0
       | 02E6  6740                           BEQ.B       0328
       | 02E8  5580                           SUBQ.L      #2,D0
       | 02EA  673C                           BEQ.B       0328
       | 02EC  5580                           SUBQ.L      #2,D0
       | 02EE  6738                           BEQ.B       0328
       | 02F0  5580                           SUBQ.L      #2,D0
       | 02F2  6734                           BEQ.B       0328
       | 02F4  720E                           MOVEQ       #0E,D1
       | 02F6  9081                           SUB.L       D1,D0
       | 02F8  6718                           BEQ.B       0312
       | 02FA  60C8                           BRA.B       02C4
       | 02FC  4AAF 0020                      TST.L       0020(A7)
       | 0300  67C2                           BEQ.B       02C4
       | 0302  206F 0028                      MOVEA.L     0028(A7),A0
       | 0306  226F 0020                      MOVEA.L     0020(A7),A1
       | 030A  2368 0004 000E                 MOVE.L      0004(A0),000E(A1)
       | 0310  60B2                           BRA.B       02C4
       | 0312  4AAF 0020                      TST.L       0020(A7)
       | 0316  67AC                           BEQ.B       02C4
       | 0318  206F 0028                      MOVEA.L     0028(A7),A0
       | 031C  226F 0020                      MOVEA.L     0020(A7),A1
       | 0320  2368 0004 0012                 MOVE.L      0004(A0),0012(A1)
       | 0326  609C                           BRA.B       02C4
       | 0328  206F 0028                      MOVEA.L     0028(A7),A0
       | 032C  2F28 0004                      MOVE.L      0004(A0),-(A7)
       | 0330  4EBA  0000-XX.1                JSR         _strlen(PC)
       | 0334  584F                           ADDQ.W      #4,A7
       | 0336  2E00                           MOVE.L      D0,D7
       | 0338  2007                           MOVE.L      D7,D0
       | 033A  5280                           ADDQ.L      #1,D0
       | 033C  7201                           MOVEQ       #01,D1
       | 033E  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0342  4EAE FF3A                      JSR         FF3A(A6)
       | 0346  2F40 001C                      MOVE.L      D0,001C(A7)
       | 034A  7016                           MOVEQ       #16,D0
       | 034C  7201                           MOVEQ       #01,D1
       | 034E  4EAE FF3A                      JSR         FF3A(A6)
       | 0352  2F40 0018                      MOVE.L      D0,0018(A7)
       | 0356  222F 001C                      MOVE.L      001C(A7),D1
       | 035A  6732                           BEQ.B       038E
       | 035C  4A80                           TST.L       D0
       | 035E  672E                           BEQ.B       038E
       | 0360  2040                           MOVEA.L     D0,A0
       | 0362  2141 000A                      MOVE.L      D1,000A(A0)
       | 0366  206F 0028                      MOVEA.L     0028(A7),A0
       | 036A  2F28 0004                      MOVE.L      0004(A0),-(A7)
       | 036E  2F01                           MOVE.L      D1,-(A7)
       | 0370  2F40 0028                      MOVE.L      D0,0028(A7)
       | 0374  4EBA  0000-XX.1                JSR         _strcpy(PC)
       | 0378  504F                           ADDQ.W      #8,A7
       | 037A  41EC  0008-02.2                LEA         02.00000008(A4),A0
       | 037E  226F 0018                      MOVEA.L     0018(A7),A1
       | 0382  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 0386  4EAE FF0A                      JSR         FF0A(A6)
       | 038A  6000 FF38                      BRA.W       02C4
       | 038E  42AF 0020                      CLR.L       0020(A7)
       | 0392  4A81                           TST.L       D1
       | 0394  670A                           BEQ.B       03A0
       | 0396  2007                           MOVE.L      D7,D0
       | 0398  5280                           ADDQ.L      #1,D0
       | 039A  2241                           MOVEA.L     D1,A1
       | 039C  4EAE FF2E                      JSR         FF2E(A6)
       | 03A0  202F 0018                      MOVE.L      0018(A7),D0
       | 03A4  6700 FF1E                      BEQ.W       02C4
       | 03A8  2240                           MOVEA.L     D0,A1
       | 03AA  7016                           MOVEQ       #16,D0
       | 03AC  4EAE FF2E                      JSR         FF2E(A6)
       | 03B0  6000 FF12                      BRA.W       02C4
       | 03B4  224A                           MOVEA.L     A2,A1
       | 03B6  203C 0000 8000                 MOVE.L      #00008000,D0
       | 03BC  2C6C  0000-XX.2                MOVEA.L     _SysBase(A4),A6
       | 03C0  4EAE FEBC                      JSR         FEBC(A6)
       | 03C4  7001                           MOVEQ       #01,D0
       | 03C6  4CDF 7C80                      MOVEM.L     (A7)+,D7/A2-A6
       | 03CA  DEFC 0014                      ADDA.W      #0014,A7
       | 03CE  4E75                           RTS

SECTION 01 "__MERGED" 00000098 BYTES
0000 80 00 20 03 00 00 00 00 80 00 20 04 00 00 00 00 .. ....... .....
0010 80 00 20 00 00 00 00 00 00 00 00 00 00 00 00 00 .. .............
0020 53 79 6E 74 61 78 3A 20 73 68 6F 77 68 6F 73 74 Syntax: showhost
0030 73 20 3C 6F 70 74 69 6F 6E 61 6C 20 72 65 61 6C s <optional real
0040 6D 3E 0A 00 6E 69 70 63 2E 6C 69 62 72 61 72 79 m>..nipc.library
0050 00 00 75 74 69 6C 69 74 79 2E 6C 69 62 72 61 72 ..utility.librar
0060 79 00 6E 69 70 63 2E 6C 69 62 72 61 72 79 00 00 y.nipc.library..
0070 52 65 73 70 6F 6E 73 65 3A 20 25 33 32 73 09 09 Response: %32s..
0080 25 6C 64 2E 25 6C 64 2E 25 6C 64 2E 25 6C 64 09 %ld.%ld.%ld.%ld.
0090 25 6C 64 0A 00 00 00 00 %ld.....

SECTION 02 "__MERGED" 00000018 BYTES
