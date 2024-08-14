/************************************************************************/
/*                                                                      */
/*      egbhdrng.h : Kanji Range Definition for HDML                    */
/*                                                                      */
/*                      Version 1.1                                     */
/*                                                                      */
/*              Designed    by  I.Iwata         07/07/1986              */
/*              Coded       by  I.Iwata         07/07/1986              */
/*              Modified    by  xxxxx           xx/xx/xxxx              */
/*                                                                      */
/*      (C) Copyright 1985-86 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/

/*  char definition (shift JIS)  */
/*  row                          */
#define  JV1FR   0x81          /* sjis lv1 start */
#define  JV1TO   0x9f          /* sjis lv1 end   */
#define  JV2FR   0xe0          /* sjis lv2 start */
#define  JV2TO   0xfc          /* sjis lv2 end   */
/*  colum   */
#define  JH1FR   0x40          /* sjis lv1 start */
#define  JH1TO   0x7e          /* sjis lv1 end   */
#define  JH2FR   0x80          /* sjis lv2 start */
#define  JH2TO   0x9e          /* sjis lv2 end   */
#define  JH3FR   0x9f          /* sjis lv3 start */
#define  JH3TO   0xfc          /* sjis lv3 end   */

/*  char definition (kuten)  */
#define  KV1FR   1             /* sjis lv1 start */
#define  KV1TO   31            /* sjis lv1 end   */
#define  KV2FR   32            /* sjis lv2 start */
#define  KV2TO   83            /* sjis lv2 end   */

#define  KH1FR   1             /* sjis lv1 start */
#define  KH1TO   63            /* sjis lv1 end   */
#define  KH2FR   64            /* sjis lv2 start */
#define  KH2TO   94            /* sjis lv2 end   */
