/*
 *  jcc.h -  These are the library GLOBALS...
 *
 */

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

struct JConversionCodeSet
{
        LONG  jcc_WhatInputCode;    /* Input Code Set */
        LONG  jcc_WhatOutputCode;   /* Output Code Set */
        UBYTE jcc_FirstByte;        /* Kanji first byte backup */
        UBYTE jcc_SecondByte;       /* Kanji second byte backup */
        UBYTE jcc_ESC[4];           /* JIS ESC sequence backup */
        UBYTE jcc_HanKata[4];       /* Hankaku Katakana backup */
        BOOL  jcc_ShiftedIn;        /* Shift in (Kanji in) mode */
};

#define CTYPE_UNKNOWN  0
#define CTYPE_ASCII    1    /* ASCII only */
#define CTYPE_BINARY   2
#define CTYPE_EUC      3
#define CTYPE_SJIS     4
#define CTYPE_NEWJIS   5
#define CTYPE_OLDJIS   6
#define CTYPE_NECJIS   7
#define CTYPE_CONTINUE 20

#define NEW_KI         "$B"
#define OLD_KI         "$@"
#define NEC_KI         "K"
#define NEW_KO         "(J"
#define OLD_KO         "(J"
#define NEC_KO         "H"

#define LF             0x0a
#define FF             0x0c
#define CR             0x0d
#define PCEOF          0x1a
#define ESC            0x1b

/* JCC data */
#define JCC_TagBase         TAG_USER + 0x10                /* ? */
#define JCC_WhatInputCode   JCC_TagBase + 1
#define JCC_WhatOutputCode  JCC_TagBase + 2
#define JCC_FirstByte       JCC_TagBase + 3
#define JCC_ESC             JCC_TagBase + 4
#define JCC_HanKata         JCC_TagBase + 5
#define JCC_ShiftedIn       JCC_TagBase + 6

/* JCC Conversion Type */
#define	JCT_EUCHanKata		JCC_TagBase + 7		/* Convert EUC half-size Katakana */
#define	JCT_SJISHanKata		JCC_TagBase + 8		/* Convert Shift-JIS half-size Katakana */
#define	JCT_UDHanKata		JCC_TagBase + 9		/* Convert to user defined 1 byte character */
#define	JCT_TextFilter		JCC_TagBase + 10	/* Text filtering */
#define	JCT_SkipESCSeq		JCC_TagBase + 11	/* Skip ESC Sequence in JIS conversion */

/************************************************************************************/
/**********************************   Macro   ***************************************/
/************************************************************************************/

#define IS_EUC_BYTE(C)    ((C)  >= 0xa1   && (C) <= 0xfe)    /* EUC 1st or 2nd byte */
#define IS_EUC_KBYTE1(C)  ((C)  == 0xa5   || (C) == 0xa1)    /* EUC Katakana 1st byte */
#define IS_EUC_KBYTE2(C)  ((C)  >= 0xa1   || (C) <= 0xf6)    /* EUC Katakana 2nd byte */
#define IS_EUC(C)         ((C)  >= 0xa1a1 && (C) <= 0xfdfe)  /* EUC */
#define IS_EUC_L0(C)      ((C)  >= 0xa1a1 && (C) <= 0xb0a0)  /* EUC except kanji */
#define IS_EUC_L1(C)      ((C)  >= 0xb0a1 && (C) <= 0xcfd3)  /* EUC JIS group 1 */
#define IS_EUC_L2(C)      ((C)  >= 0xd0a1 && (C) <= 0xf3fe)  /* EUC JIS group 2 */
#define IS_EUC_ALPHA(C)   (((C) >= 0xa3c1 && (C) <= 0xa3da) || ((C) >= 0xa3e1 && (C) <= 0xa3fa))
                                                             /* EUC alphabet */
#define IS_EUC_UPPER(C)   ((C)  >= 0xa3c1 && (C) <= 0xa3da)  /* EUC upper case alphabet */
#define IS_EUC_LOWER(C)   ((C)  >= 0xa3e1 && (C) <= 0xa3fa)  /* EUC lower case alphabet */
#define IS_EUC_DIGIT(C)   ((C)  >= 0xa3b0 && (C) <= 0xa3b9)  /* EUC digit */
#define IS_EUC_HIRA(C)    ((C)  >= 0xa4a1 && (C) <= 0xa4f3)  /* EUC hiragana */
#define IS_EUC_KANA(C)    ((C)  >= 0xa5a1 && (C) <= 0xa5f6)  /* EUC katakana */
#define IS_EUC_KIGOU(C)   ((C)  >= 0xa1a2 && (C) <= 0xa2ae)  /* EUC kigou */
#define IS_EUC_SPACE(C)   ((C)  == 0xa1a1)                   /* EUC space */

#define IS_SJIS_BYTE1(C)  (((C) >= 0x81 && (C) <= 0x9f) || ((C) >= 0xe0 && (C) <= 0xef))
#define IS_SJIS_BYTE2(C)  ((C)  >= 0x40 && (C) <= 0xfc)
#define IS_SJIS_KBYTE1(C) ((C)  == 0x83 || (C) == 0x81)      /* Shift-JIS Katakana 1st byte */
#define IS_SJIS_KBYTE2(C) ((C)  >= 0x40 && (C) <= 0x96)      /* Shift-JIS Katakana 2nd byte */
#define IS_HANKATA(C)     ((C)  >= 0xa1 && (C) <= 0xdf)      /* ANK Hankaku Katakana */
#define IS_NIGORI(A,B)    ((B)  == 0xde && (((A) >= 0xb6 && (A) <= 0xc4) || ((A) >= 0xca && (A) <= 0xce) || ((A) == 0xb3)))
                                                             /* ANK Hankaku Katakana which is possible to have dakuten */
#define IS_MARU(A,B)      ((B)  == 0xdf && ((A)>= 0xca  && (A) <= 0xce))
                                                             /* ANK Hankaku Katakana which is possible to have maru */
#define IS_NOT_EUC(A,B)   ((((A) >= 0x81 && (A) <= 0x9f) && ((B) >= 0x40 && (B) <= 0xa0)) || ((A) < 0xa1))

#define ClearConversionHandle(A) SetJConversionAttrs(A,JCC_WhatInputCode,0,JCC_WhatOutputCode,0,JCC_FirstByte,0,JCC_ESC,0,JCC_HanKata,0,JCC_ShiftedIn,0);

