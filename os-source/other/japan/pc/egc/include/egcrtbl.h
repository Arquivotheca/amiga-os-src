/************************************************************************/
/*                                                                      */
/*      EGCRTBL : EGConvert romaji_tbl                                  */
/*                                                                      */
/*              Designed    by  T.Nosaka        06/15/1988              */
/*              Coded       by  T.Nosaka        06/15/1988              */
/*                                                                      */
/*      (C) Copyright 1988-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#ifndef        __EGCRTBL__
#define        __EGCRTBL__

/*
***    CHR TABLE (search of input str)
*/

static UBYTE    cvtchr[][2] =
{
/*   illegal convert (zenkaku)   */
/*     0xFF,0x80    small wa     */
/*     0xFF,0x82    old   wi     */
/*     0xFF,0x83    old   we     */
/*     0xFF,0x87    small ka     */
/*     0xFF,0x88    small ke     */
    {0xFF, 1}, {0xFF, 'X'}, {'K', 'A'}, {'K', 'E'},
    {0xFF, 2}, {0xFF, 'X'}, {'W', 'A'}, {0xFF, 'W'}, {'I', 0}, {'E', 0},
    {0xFF, 0},
/*-------------------------------------------------------------*/
/* - A - */
    {0xFF, 'A'}, {0, 0},
/* - I - */
    {0xFF, 'I'}, {0, 0},
/* - U - */
    {0xFF, 'U'}, {0, 0},
/* - E - */
    {0xFF, 'E'}, {0, 0},
/* - O - */
    {0xFF, 'O'}, {0, 0},
/* - K - */
    {0xFF, 'K'},
/*      KA          KI          KU          KE          KO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      KYA         KYI         KYU         KYE         KYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - S - */
    {0xFF, 'S'},
/*      SA          SI          SU          SE          SO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      SYA         SYI         SYU         SYE         SYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      SHA         SHI         SHU         SHE         SHO    */
    {'H', 'A'}, {'H', 'I'}, {'H', 'U'}, {'H', 'E'}, {'H', 'O'},
/* - T - */
    {0xFF, 'T'},
/*      TA          TI          TU          TE          TO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      TYA         TYI         TYU         TYE         TYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      THA         THI         THU         THE         THO    */
    {'H', 'A'}, {'H', 'I'}, {'H', 'U'}, {'H', 'E'}, {'H', 'O'},
/*      TSA         TSI         TSU         TSE         TSO    */
    {'S', 'A'}, {'S', 'I'}, {'S', 'U'}, {'S', 'E'}, {'S', 'O'},
/*      TWA         TWI         TWU         TWE         TWO    */
    {'W', 'A'}, {'W', 'I'}, {'W', 'U'}, {'W', 'E'}, {'W', 'O'},
/* - N - */
    {0xFF, 'N'},
/*      NA          NI          NU          NE          NO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      NYA         NYI         NYU         NYE         NYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      NN                                                     */
    {'N', 000},
/* - H - */
    {0xFF, 'H'},
/*      A          HI          HU          HE          HO      */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      HYA         HYI         HYU         HYE         HYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - M - */
    {0xFF, 'M'},
/*      MA          MI          MU          ME          MO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      MYA         MYI         MYU         MYE         MYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      MN                                                     */
    {'N', 000},
/* - Y - */
    {0xFF, 'Y'},
/*      YA          YI          YU          YE          YO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/* - R - */
    {0xFF, 'R'},
/*      RA          RI          RU          RE          RO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      RYA         RYI         RYU         RYE         RYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - L - */
    {0xFF, 'L'},
/*      LA          LI          LU          LE          LO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      LYA         LYI         LYU         LYE         LYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - W - */
    {0xFF, 'W'},
/*      WA          WI          WU          WE          WO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      WHA         WHI         WHU         WHE         WHO    */
    {'H', 'A'}, {'H', 'I'}, {'H', 'U'}, {'H', 'E'}, {'H', 'O'},
/* - B - */
    {0xFF, 'B'},
/*      BA          BI          BU          BE          BO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      BYA         BYI         BYU         BYE         BYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - C - */
    {0xFF, 'C'},
/*      CA          CI          CU          CE          CO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      CYA         CYI         CYU         CYE         CYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      CHA         CHI         CHU         CHE         CHO    */
    {'H', 'A'}, {'H', 'I'}, {'H', 'U'}, {'H', 'E'}, {'H', 'O'},
/* - D - */
    {0xFF, 'D'},
/*      DA          DI          DU          DE          DO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      DYA         DYI         DYU         DYE         DYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      DHA         DHI         DHU         DHE         DHO    */
    {'H', 'A'}, {'H', 'I'}, {'H', 'U'}, {'H', 'E'}, {'H', 'O'},
/*      DWA         DWI         DWU         DWE         DWO    */
    {'W', 'A'}, {'W', 'I'}, {'W', 'U'}, {'W', 'E'}, {'W', 'O'},
/* - F - */
    {0xFF, 'F'},
/*      FA          FI          FU          FE          FO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      FYA         FYI         FYU         FYE         FYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - G - */
    {0xFF, 'G'},
/*      GA          GI          GU          GE          GO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      GYA         GYI         GYU         GYE         GYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      GWA         GWI         GWU         GWE         GWO    */
    {'W', 'A'}, {'W', 'I'}, {'W', 'U'}, {'W', 'E'}, {'W', 'O'},
/* - J - */
    {0xFF, 'J'},
/*      JA          JI          JU          JE          JO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      JYA         JYI         JYU         JYE         JYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - P - */
    {0xFF, 'P'},
/*      PA          PI          PU          PE          PO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      PYA         PYI         PYU         PYE         PYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - Q - */
    {0xFF, 'Q'},
/*      QA          QI          QU          QE          QO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/* - V - */
    {0xFF, 'V'},
/*      VA          VI          VU          VE          VO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      VYA         VYI         VYU         VYE         VYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/* - X - */
    {0xFF, 'X'},
/*      XA          XI          XU          XE          XO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      XKA         XKI         XKU         XKE         XKO    */
    {'K', 'A'}, {'K', 'I'}, {'K', 'U'}, {'K', 'E'}, {'K', 'O'},
/*      XTA         XTI         XTU         XTE         XTO    */
    {'T', 'A'}, {'T', 'I'}, {'T', 'U'}, {'T', 'E'}, {'T', 'O'},
/*      XYA         XYI         XYU         XYE         XYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
/*      XWA         XWI         XWU         XWE         XWO    */
    {'W', 'A'}, {'W', 'I'}, {'W', 'U'}, {'W', 'E'}, {'W', 'O'},
/*      XN                                                     */
    {'N', 000},
/* - Z - */
    {0xFF, 'Z'},
/*      ZA          ZI          ZU          ZE          ZO     */
    {'A', 000}, {'I', 000}, {'U', 000}, {'E', 000}, {'O', 000},
/*      ZYA         ZYI         ZYU         ZYE         ZYO    */
    {'Y', 'A'}, {'Y', 'I'}, {'Y', 'U'}, {'Y', 'E'}, {'Y', 'O'},
    {0xFF, 0xFF},
};

/*
**                   ROMAJI TABLE (search of output str)
*/

static UBYTE    cvtroma[][3] = {
/*      illegal convert (zenkaku)   */
    {0x87, 0x00, 0x00}                 /* 0xFF,0x87    small ka */
    ,{0x88, 0x00, 0x00}                /* 0xFF,0x88    small ke */
    ,{0x80, 0x00, 0x00}                /* 0xFF,0x80    small wa */
    ,{0x82, 0x00, 0x00}                /* 0xFF,0x82    old   wi */
    ,{0x83, 0x00, 0x00}                /* 0xFF,0x83    old   we */
/*          A         */
    ,{0xB1, 0x00, 0x00}
/*          I         */
    ,{0xB2, 0x00, 0x00}
/*          U         */
    ,{0xB3, 0x00, 0x00}
/*          E         */
    ,{0xB4, 0x00, 0x00}
/*          O         */
    ,{0xB5, 0x00, 0x00}
/*          KA                  KI                  KU                  KE                  KO     */
    ,{0xB6, 0x00, 0x00}, {0xB7, 0x00, 0x00}, {0xB8, 0x00, 0x00}, {0xB9, 0x00, 0x00}, {0xBA, 0x00, 0x00}
/*          KYA                 KYI                 KYU                 KYE                 KYO    */
    ,{0xB7, 0xAC, 0x00}, {0xB7, 0xA8, 0x00}, {0xB7, 0xAD, 0x00}, {0xB7, 0xAA, 0x00}, {0xB7, 0xAE, 0x00}
/*          SA                  SI                  SU                  SE                  SO     */
    ,{0xBB, 0x00, 0x00}, {0xBC, 0x00, 0x00}, {0xBD, 0x00, 0x00}, {0xBE, 0x00, 0x00}, {0xBF, 0x00, 0x00}
/*          SYA                 SYI                 SYU                 SYE                 SYO    */
    ,{0xBC, 0xAC, 0x00}, {0xBC, 0xA8, 0x00}, {0xBC, 0xAD, 0x00}, {0xBC, 0xAA, 0x00}, {0xBC, 0xAE, 0x00}
/*          SHA                 SHI                 SHU                 SHE                 SHO    */
    ,{0xBC, 0xAC, 0x00}, {0xBC, 0x00, 0x00}, {0xBC, 0xAD, 0x00}, {0xBC, 0xAA, 0x00}, {0xBC, 0xAE, 0x00}
/*          TA                  TI                  TU                  TE                  TO     */
    ,{0xC0, 0x00, 0x00}, {0xC1, 0x00, 0x00}, {0xC2, 0x00, 0x00}, {0xC3, 0x00, 0x00}, {0xC4, 0x00, 0x00}
/*          TYA                 TYI                 TYU                 TYE                 TYO    */
    ,{0xC1, 0xAC, 0x00}, {0xC1, 0xA8, 0x00}, {0xC1, 0xAD, 0x00}, {0xC1, 0xAA, 0x00}, {0xC1, 0xAE, 0x00}
/*          THA                 THI                 THU                 THE                 THO    */
    ,{0xC3, 0xAC, 0x00}, {0xC3, 0xA8, 0x00}, {0xC3, 0xAD, 0x00}, {0xC3, 0xAA, 0x00}, {0xC3, 0xAE, 0x00}
/*          TSA                 TSI                 TSU                 TSE                 TSO    */
    ,{0xC2, 0xA7, 0x00}, {0xC2, 0xA8, 0x00}, {0xC2, 0x00, 0x00}, {0xC2, 0xAA, 0x00}, {0xC2, 0xAB, 0x00}
/*          TWA                 TWI                 TWU                 TWE                 TWO    */
    ,{0xC4, 0xA7, 0x00}, {0xC4, 0xA8, 0x00}, {0xC4, 0xA9, 0x00}, {0xC4, 0xAA, 0x00}, {0xC4, 0xAB, 0x00}
/*          NA                  NI                  NU                  NE                  NO     */
    ,{0xC5, 0x00, 0x00}, {0xC6, 0x00, 0x00}, {0xC7, 0x00, 0x00}, {0xC8, 0x00, 0x00}, {0xC9, 0x00, 0x00}
/*          NYA                 NYI                 NYU                 NYE                 NYO    */
    ,{0xC6, 0xAC, 0x00}, {0xC6, 0xA8, 0x00}, {0xC6, 0xAD, 0x00}, {0xC6, 0xAA, 0x00}, {0xC6, 0xAE, 0x00}
/*          NN        */
    ,{0xDD, 0x00, 0x00}
/*          HA                  HI                  HU                  HE                  HO     */
    ,{0xCA, 0x00, 0x00}, {0xCB, 0x00, 0x00}, {0xCC, 0x00, 0x00}, {0xCD, 0x00, 0x00}, {0xCE, 0x00, 0x00}
/*          HYA                 HYI                 HYU                 HYE                 HYO    */
    ,{0xCB, 0xAC, 0x00}, {0xCB, 0xA8, 0x00}, {0xCB, 0xAD, 0x00}, {0xCB, 0xAA, 0x00}, {0xCB, 0xAE, 0x00}
/*          MA                  MI                  MU                  ME                  MO     */
    ,{0xCF, 0x00, 0x00}, {0xD0, 0x00, 0x00}, {0xD1, 0x00, 0x00}, {0xD2, 0x00, 0x00}, {0xD3, 0x00, 0x00}
/*          MYA                 MYI                 MYU                 MYE                 MYO    */
    ,{0xD0, 0xAC, 0x00}, {0xD0, 0xA8, 0x00}, {0xD0, 0xAD, 0x00}, {0xD0, 0xAA, 0x00}, {0xD0, 0xAE, 0x00}
/*          MN         */
    ,{0xDD, 0x00, 0x00}
/*          YA                  YI                  YU                   YE                 YO */
    ,{0xD4, 0x00, 0x00}, {0xB2, 0x00, 0x00}, {0xD5, 0x00, 0x00}, {0xB2, 0xAA, 0x00}, {0xD6, 0x00, 0x00}
/*          RA                  RI                  RU                  RE                  RO     */
    ,{0xD7, 0x00, 0x00}, {0xD8, 0x00, 0x00}, {0xD9, 0x00, 0x00}, {0xDA, 0x00, 0x00}, {0xDB, 0x00, 0x00}
/*          RYA                 RYI                 RYU                 RYE                 RYO    */
    ,{0xD8, 0xAC, 0x00}, {0xD8, 0xA8, 0x00}, {0xD8, 0xAD, 0x00}, {0xD8, 0xAA, 0x00}, {0xD8, 0xAE, 0x00}
/*          LA                  LI                  LU                  LE                  LO     */
    ,{0xD7, 0x00, 0x00}, {0xD8, 0x00, 0x00}, {0xD9, 0x00, 0x00}, {0xDA, 0x00, 0x00}, {0xDB, 0x00, 0x00}
/*          LYA                 LYI                 LYU                 LYE                 LYO    */
    ,{0xD8, 0xAC, 0x00}, {0xD8, 0xA8, 0x00}, {0xD8, 0xAD, 0x00}, {0xD8, 0xAA, 0x00}, {0xD8, 0xAE, 0x00}
/*          WA                  WI                  WU                  WE                  WO     */
    ,{0xDC, 0x00, 0x00}, {0xB2, 0x00, 0x00}, {0xB3, 0x00, 0x00}, {0xB4, 0x00, 0x00}, {0xA6, 0x00, 0x00}
/*          WHA                 WHI                 WHU                 WHE                 WHO    */
    ,{0xB3, 0xA7, 0x00}, {0xB3, 0xA8, 0x00}, {0xB3, 0xA9, 0x00}, {0xB3, 0xAA, 0x00}, {0xB3, 0xAB, 0x00}
/*          BA                  BI                  BU                  BE                  BO     */
    ,{0xCA, 0xDE, 0x00}, {0xCB, 0xDE, 0x00}, {0xCC, 0xDE, 0x00}, {0xCD, 0xDE, 0x00}, {0xCE, 0xDE, 0x00}
/*          BYA                 BYI                 BYU                 BYE                 BYO    */
    ,{0xCB, 0xDE, 0xAC}, {0xCB, 0xDE, 0xA8}, {0xCB, 0xDE, 0xAD}, {0xCB, 0xDE, 0xAA}, {0xCB, 0xDE, 0xAE}
/*          CA                  CI                  CU                  CE                  CO     */
    ,{0xB6, 0x00, 0x00}, {0xBC, 0x00, 0x00}, {0xB8, 0x00, 0x00}, {0xBE, 0x00, 0x00}, {0xBA, 0x00, 0x00}
/*          CYA                 CYI                 CYU                 CYE                 CYO    */
    ,{0xC1, 0xAC, 0x00}, {0xC1, 0xA8, 0x00}, {0xC1, 0xAD, 0x00}, {0xC1, 0xAA, 0x00}, {0xC1, 0xAE, 0x00}
/*          CHA                 CHI                 CHU                 CHE                 CHO    */
    ,{0xC1, 0xAC, 0x00}, {0xC1, 0x00, 0x00}, {0xC1, 0xAD, 0x00}, {0xC1, 0xAA, 0x00}, {0xC1, 0xAE, 0x00}
/*          DA                  DI                  DU                  DE                  DO     */
    ,{0xC0, 0xDE, 0x00}, {0xC1, 0xDE, 0x00}, {0xC2, 0xDE, 0x00}, {0xC3, 0xDE, 0x00}, {0xC4, 0xDE, 0x00}
/*          DYA                 DYI                 DYU                 DYE                 DYO    */
    ,{0xC1, 0xDE, 0xAC}, {0xC1, 0xDE, 0xA8}, {0xC1, 0xDE, 0xAD}, {0xC1, 0xDE, 0xAA}, {0xC1, 0xDE, 0xAE}
/*          DHA                 DHI                 DHU                 DHE                 DHO    */
    ,{0xC3, 0xDE, 0xAC}, {0xC3, 0xDE, 0xA8}, {0xC3, 0xDE, 0xAD}, {0xC3, 0xDE, 0xAA}, {0xC3, 0xDE, 0xAE}
/*          DWA                 DWI                 DWU                 DWE                 DWO    */
    ,{0xC4, 0xDE, 0xA7}, {0xC4, 0xDE, 0xA8}, {0xC4, 0xDE, 0xA9}, {0xC4, 0xDE, 0xAA}, {0xC4, 0xDE, 0xAB}
/*          FA                  FI                  FU                  FE                  FO     */
    ,{0xCC, 0xA7, 0x00}, {0xCC, 0xA8, 0x00}, {0xCC, 0x00, 0x00}, {0xCC, 0xAA, 0x00}, {0xCC, 0xAB, 0x00}
/*          FYA                 FYI                 FYU                 FYE                 FYO    */
    ,{0xCC, 0xAC, 0x00}, {0xCC, 0xA8, 0x00}, {0xCC, 0xAD, 0x00}, {0xCC, 0xAA, 0x00}, {0xCC, 0xAE, 0x00}
/*          GA                  GI                  GU                  GE                  GO     */
    ,{0xB6, 0xDE, 0x00}, {0xB7, 0xDE, 0x00}, {0xB8, 0xDE, 0x00}, {0xB9, 0xDE, 0x00}, {0xBA, 0xDE, 0x00}
/*          GYA                 GYI                 GYU                 GYE                 GYO    */
    ,{0xB7, 0xDE, 0xAC}, {0xB7, 0xDE, 0xA8}, {0xB7, 0xDE, 0xAD}, {0xB7, 0xDE, 0xAA}, {0xB7, 0xDE, 0xAE}
/*          GWA                 GWI                 GWU                 GWE                 GWO    */
    ,{0xB8, 0xDE, 0xA7}, {0xB8, 0xDE, 0xA8}, {0xB8, 0xDE, 0xA9}, {0xB8, 0xDE, 0xAA}, {0xB8, 0xDE, 0xAB}
/*          JA                  JI                  JU                  JE                  JO     */
    ,{0xBC, 0xDE, 0xAC}, {0xBC, 0xDE, 0x00}, {0xBC, 0xDE, 0xAD}, {0xBC, 0xDE, 0xAA}, {0xBC, 0xDE, 0xAE}
/*          JYA                 JYI                 JYU                 JYE                 JYO    */
    ,{0xBC, 0xDE, 0xAC}, {0xBC, 0xDE, 0xA8}, {0xBC, 0xDE, 0xAD}, {0xBC, 0xDE, 0xAA}, {0xBC, 0xDE, 0xAE}
/*          PA                  PI                  PU                  PE                  PO     */
    ,{0xCA, 0xDF, 0x00}, {0xCB, 0xDF, 0x00}, {0xCC, 0xDF, 0x00}, {0xCD, 0xDF, 0x00}, {0xCE, 0xDF, 0x00}
/*          PYA                 PYI                 PYU                 PYE                 PYO    */
    ,{0xCB, 0xDF, 0xAC}, {0xCB, 0xDF, 0xA8}, {0xCB, 0xDF, 0xAD}, {0xCB, 0xDF, 0xAA}, {0xCB, 0xDF, 0xAE}
/*          QA                  QI                  QU                  QE                  QO     */
    ,{0xB8, 0xA7, 0x00}, {0xB8, 0xA8, 0x00}, {0xB8, 0xA9, 0x00}, {0xB8, 0xAA, 0x00}, {0xB8, 0xAB, 0x00}
/*          VA                  VI                  VU                  VE                  VO     */
    ,{0xB3, 0xDE, 0xA7}, {0xB3, 0xDE, 0xA8}, {0xB3, 0xDE, 0x00}, {0xB3, 0xDE, 0xAA}, {0xB3, 0xDE, 0xAB}
/*          VYA                 VYI                 VYU                 VYE                 VYO     */
    ,{0xB3, 0xDE, 0xAC}, {0xB3, 0xDE, 0xA8}, {0xB3, 0xDE, 0xAD}, {0xB3, 0xDE, 0xAA}, {0xB3, 0xDE, 0xAE}
/*          XA                  XI                  XU                  XE                  XO     */
    ,{0xA7, 0x00, 0x00}, {0xA8, 0x00, 0x00}, {0xA9, 0x00, 0x00}, {0xAA, 0x00, 0x00}, {0xAB, 0x00, 0x00}
/*          XKA                 XKI                 XKU                 XKE                 XKO    */
    ,{0xB6, 0x00, 0x00}, {0xB7, 0x00, 0x00}, {0xB8, 0x00, 0x00}, {0xB9, 0x00, 0x00}, {0xBA, 0x00, 0x00}
/*          XTA                 XTI                 XTU                 XTE                 XTO    */
    ,{0xC0, 0x00, 0x00}, {0xC1, 0x00, 0x00}, {0xAF, 0x00, 0x00}, {0xC3, 0x00, 0x00}, {0xC4, 0x00, 0x00}
/*          XYA                 XYI                 XYU                 XYE                 XYO    */
    ,{0xAC, 0x00, 0x00}, {0xA8, 0x00, 0x00}, {0xAD, 0x00, 0x00}, {0xAA, 0x00, 0x00}, {0xAE, 0x00, 0x00}
/*          XWA                 XWI                 XWU                 XWE                 XWO    */
    ,{0xDC, 0x00, 0x00}, {0xA8, 0x00, 0x00}, {0xA9, 0x00, 0x00}, {0xAA, 0x00, 0x00}, {0xAB, 0x00, 0x00}
/*          XN         */
    ,{0xDD, 0x00, 0x00}
/*          ZA                  ZI                  ZU                  ZE                  ZO     */
    ,{0xBB, 0xDE, 0x00}, {0xBC, 0xDE, 0x00}, {0xBD, 0xDE, 0x00}, {0xBE, 0xDE, 0x00}, {0xBF, 0xDE, 0x00}
/*          ZYA                 ZYI                 ZYU                 ZYE                 ZYO    */
    ,{0xBC, 0xDE, 0xAC}, {0xBC, 0xDE, 0xA8}, {0xBC, 0xDE, 0xAD}, {0xBC, 0xDE, 0xAA}, {0xBC, 0xDE, 0xAE}
};

#endif    /* __EGCRTBL__ */
