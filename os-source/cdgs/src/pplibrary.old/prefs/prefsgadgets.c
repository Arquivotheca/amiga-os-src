
#include <cdtv/debox.h>
#include "/playerprefs.h"

extern     GADInsDraw(struct RastPort *rp, struct GadDir *g, int val, UWORD state );
extern     GADNumber(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);
extern     GADSTDBox(struct RastPort *rp, struct GadDir *g, int val, UWORD state);
extern     RNormal(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);
extern     GADScreenPos(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);
extern     GADScrSave(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);
extern     GAD2SRender(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);
extern     GADCardRen(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, UWORD old, UWORD new);
extern int GADLanguage(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new);

struct GADSTDBox d_LanguageBoxInfo = {

    -5, -4, 101, 22,
    {1, 1,  0,   28},
    {1, 1,  110, 28}
    };

struct GADSTDBox d_ScreenBoxInfo = {

    -8, -7, 101, 71,
    {0, 1,  125, 60},
    {1, 1,  125, 60}
    };

struct GADSTDBox d_AmPm24BoxInfo = {

    -6,-4, 28,  17,
    {1, 1, 166, 9},
    {1, 1, 194, 9}
    };

struct GADSTDBox d_LangBoxInfo = {

    -4, 38, 109, 20,
    {1, 1,  110, 29},
    {1, 1,  0,   29}
    };

UWORD d_AM[] = {168, 0, 184, 0, 200, 0};

BYTE d_HourBoxInfo[] = {

    INS_MOVE,       4,0,
    INS_PEN,        49,21,
    INS_DRAW,       37,0,
    INS_PEN,        49,26,
    INS_DRAW,       40,3,
    INS_PEN,        32,24,
    INS_MOVE,       40,4,
    INS_DRAW,       40,20,
    INS_PEN,        50,21,
    INS_MOVE,       40,21,
    INS_DRAW,       37,24,
    INS_PEN,        50,23,
    INS_DRAW,       3,24,
    INS_PEN,        50,25,
    INS_DRAW,       1,22,
    INS_PEN,        49,22,
    INS_MOVE,       0,21,
    INS_DRAW,       0,4,
    INS_PEN,        17,27,
    INS_MOVE,       0,3,
    INS_DRAW,       3,0,
    INS_END
    };

BYTE d_MinBoxInfo[] = {

    INS_MOVE,       4,0,
    INS_PEN,        49,21,
    INS_DRAW,       37,0,
    INS_PEN,        49,26,
    INS_DRAW,       40,3,
    INS_PEN,        50,24,
    INS_MOVE,       40,4,
    INS_DRAW,       40,21,
    INS_PEN,        50,21,
    INS_DRAW,       37,24,
    INS_PEN,        50,23,
    INS_DRAW,       3,24,
    INS_PEN,        50,25,
    INS_DRAW,       1,22,
    INS_PEN,        49,22,
    INS_MOVE,       0,21,
    INS_DRAW,       0,20,
    INS_PEN,        32,22,
    INS_DRAW,       0,3,
    INS_PEN,        49,22,
    INS_DRAW,       1,2,
    INS_PEN,        17,27,
    INS_DRAW,       3,0,
    INS_END
    };

BYTE d_MonthBoxInfo[] = {

    INS_MOVE,       0,14,
    INS_PEN,        49,22,
    INS_DRAW,       0,2,
    INS_PEN,        49,27,
    INS_DRAW,       2,0,
    INS_PEN,        49,21,
    INS_DRAW,       16,0,
    INS_PEN,        49,26,
    INS_DRAW,       17,1,
    INS_MOVE,       18,2,
    INS_PEN,        50,24,
    INS_DRAW,       18,14,
    INS_PEN,        50,21,
    INS_DRAW,       16,16,
    INS_PEN,        50,23,
    INS_DRAW,       2,16,
    INS_PEN,        50,25,
    INS_DRAW,       1,15,
    INS_END         
    };

BYTE d_DayBoxInfo[] = {

    INS_MOVE,       0,14,
    INS_PEN,        49,22,
    INS_DRAW,       0,2,
    INS_PEN,        49,27,
    INS_DRAW,       2,0,
    INS_PEN,        49,21,
    INS_DRAW,       21,0,
    INS_PEN,        49,26,
    INS_DRAW,       22,1,
    INS_MOVE,       23,2,
    INS_PEN,        50,24,
    INS_DRAW,       23,14,
    INS_PEN,        50,21,
    INS_DRAW,       21,16,
    INS_PEN,        50,23,
    INS_DRAW,       2,16,
    INS_PEN,        50,25,
    INS_DRAW,       1,15,
    INS_END         
    };

BYTE d_YearBoxInfo[] = {

    INS_MOVE,       0,14,
    INS_PEN,        49,22,
    INS_DRAW,       0,2,
    INS_PEN,        49,27,
    INS_DRAW,       2,0,
    INS_PEN,        49,21,
    INS_DRAW,       39,0,
    INS_PEN,        49,26,
    INS_DRAW,       40,1,
    INS_MOVE,       41,2,
    INS_PEN,        50,24,
    INS_DRAW,       41,14,
    INS_PEN,        50,21,
    INS_DRAW,       39,16,
    INS_PEN,        50,23,
    INS_DRAW,       2,16,
    INS_PEN,        50,25,
    INS_DRAW,       1,15,
    INS_END         
    };             
                   
BYTE d_ScrSaveBoxInfo[] = {

    INS_OFF_REDRAW,
    INS_MOVE,       1,1,
    INS_PEN,        8,22,
    INS_DRAW,       25,1,
    INS_PEN,        8,27,
    INS_DRAW,       25,55,
    INS_PEN,        8,21,
    INS_DRAW,       1,55,
    INS_PEN,        8,26,
    INS_DRAW,       1,1,
    INS_END
    };

BYTE d_LaceClickBoxInfo[] = {

    INS_OFF_REDRAW,
    INS_MOVE,       1,0,
    INS_PEN,        8,22,
    INS_DRAW,       97,0,
    INS_MOVE,       98,1,
    INS_PEN,        8,27,
    INS_DRAW,       98,36,
    INS_MOVE,       97,37,
    INS_PEN,        8,21,
    INS_DRAW,       1,37,
    INS_MOVE,       0,36,
    INS_PEN,        8,26,
    INS_DRAW,       0,1,
    INS_END
    };

BYTE d_PrpCardBoxInfo[] = {

    INS_OFF_REDRAW,
    INS_MOVE,       1,1,
    INS_PEN,        8,22,
    INS_DRAW,       32,1,
    INS_MOVE,       32,2,
    INS_PEN,        8,27,
    INS_DRAW,       32,31,
    INS_MOVE,       32,32,
    INS_PEN,        8,21,
    INS_DRAW,       2,32,
    INS_MOVE,       1,32,
    INS_PEN,        8,26,
    INS_DRAW,       1,1,
    INS_END
    };

struct GADNumberInfo d_HourMinNumInfo = {

    0, 9, 14, 19,
    5, 3, 18, 2,
    10, 0,
    0, 0, 0, 0,
    NULL
    };

struct GADNumberInfo d_MonthNumInfo = {

    0,0,8,9,
    7,4,9,2,
    20,0,
    1,4,13,165,
    GNIF_LEADONE
    };

struct GADNumberInfo d_DayNumInfo = {

    0,0,8,9,
    4,4,9,2,
    20,0,
    1,4,14,166,
    NULL
    };

struct GADNumberInfo d_YearNumInfo = {

    0,0,8,9,
    4,4,9,4,
    20,0,
    1,4,14,166,
    NULL
    };

struct Render2S_Info d_clickR[2] = {

    {(APTR)&d_LaceClickBoxInfo,140,196,0},
    {(APTR)&d_LaceClickBoxInfo,91,229,1}
    };

struct Render2S_Info d_laceR[2] = {

    {(APTR)&d_LaceClickBoxInfo,140,144,0},
    {(APTR)&d_LaceClickBoxInfo,91,189,1}
    };

struct Render2S_Info d_prpcardR[2] = {

    {(APTR)&d_PrpCardBoxInfo,293,200,0},
    {(APTR)&d_PrpCardBoxInfo,0,69,1}
    };

struct Render2S_Info d_prpramR[2] = {

    {(APTR)&d_PrpCardBoxInfo,257,200,0},
    {(APTR)&d_PrpCardBoxInfo,39,69,1}
    };

struct Render2S_Info d_prpstopR[2] = {

    {(APTR)&d_PrpCardBoxInfo,170,69,1},
    {(APTR)&d_PrpCardBoxInfo,76,69,1}
    };

struct Render2S_Info d_prpgoR[2] = {

    {(APTR)&d_PrpCardBoxInfo,133,69,1},
    {(APTR)&d_PrpCardBoxInfo,223,25,1}
    };

struct GadDir   PrefsGADList[ MAX_PEGAD+1 ] = {

    {PEGAD_HOUR,    0x00, 0, 1,  100,   PEGAD_LANG,    PEGAD_MONTH,   PEGAD_SCRSAVE, PEGAD_MIN,     24,  64,  65,  100, GADInsDraw, &d_HourBoxInfo,      GADNumber,    &d_HourMinNumInfo},
    {PEGAD_MIN,     0x00, 0, 1,  100,   PEGAD_LANG,    PEGAD_AM,      PEGAD_HOUR,    PEGAD_AM,      70,  64,  110, 88,  GADInsDraw, &d_MinBoxInfo,       GADNumber,    &d_HourMinNumInfo},
    {PEGAD_AM,      0x00, 0, 2,  3,     PEGAD_MIN,     PEGAD_YEAR,    PEGAD_MIN,     PEGAD_MONTH,   85,  93,  101, 102, GADSTDBox,  &d_AmPm24BoxInfo,    RNormal,      &d_AM},
    {PEGAD_MONTH,   0x00, 0, 1,  100,   PEGAD_HOUR,    PEGAD_LANG,    PEGAD_AM,      PEGAD_DAY,     23,  111, 41,  127, GADInsDraw, &d_MonthBoxInfo,     GADNumber,    &d_MonthNumInfo},
    {PEGAD_DAY,     0x00, 0, 1,  100,   PEGAD_HOUR,    PEGAD_LANG,    PEGAD_MONTH,   PEGAD_YEAR,    44,  111, 67,  127, GADInsDraw, &d_DayBoxInfo,       GADNumber,    &d_DayNumInfo},
    {PEGAD_YEAR,    0x00, 0, 1,  10000, PEGAD_AM,      PEGAD_LANG,    PEGAD_DAY,     PEGAD_SCREEN,  70,  111, 111, 127, GADInsDraw, &d_YearBoxInfo,      GADNumber,    &d_YearNumInfo},
    {PEGAD_SCREEN,  0x00, 0, 3,  -1,    PEGAD_CLICK,   PEGAD_LACE,    PEGAD_YEAR,    PEGAD_SCRSAVE, 133, 67,  202, 108, GADSTDBox,  &d_ScreenBoxInfo,    GADScreenPos, NULL},
    {PEGAD_SCRSAVE, 0x80, 0, 5,  -1,    PEGAD_PRPCARD, PEGAD_PRPCARD, PEGAD_SCREEN,  PEGAD_HOUR,    304, 67,  330, 123, GADInsDraw, &d_ScrSaveBoxInfo,   GADScrSave,   NULL},
    {PEGAD_LACE,    0x80, 0, 4,  2,     PEGAD_SCREEN,  PEGAD_CLICK,   PEGAD_LANG,    PEGAD_PRPRAM,  140, 144, 239, 182, GADInsDraw, &d_LaceClickBoxInfo, GAD2SRender,  &d_laceR},
    {PEGAD_LANG,    0x00, 0, 8,  -1,    PEGAD_YEAR,    PEGAD_HOUR,    PEGAD_PRPCARD, PEGAD_CLICK,   22,  141, 123, 238, GADSTDBox,  &d_LangBoxInfo,      GADLanguage,  NULL},
    {PEGAD_CLICK,   0x80, 0, 4,  2,     PEGAD_LACE,    PEGAD_SCREEN,  PEGAD_LANG,    PEGAD_PRPRAM,  140, 196, 239, 235, GADInsDraw, &d_LaceClickBoxInfo, GAD2SRender,  &d_clickR},
    {PEGAD_PRPRAM,  0x80, 0, 9,  64,    PEGAD_SCRSAVE, PEGAD_SCRSAVE, PEGAD_CLICK,   PEGAD_PRPCARD, 257, 200, 292, 235, GADInsDraw, &d_PrpCardBoxInfo,   GAD2SRender,  &d_prpramR},
    {PEGAD_PRPCARD, 0x80, 0, 9,  64,    PEGAD_SCRSAVE, PEGAD_SCRSAVE, PEGAD_PRPRAM,  PEGAD_LANG,    293, 200, 330, 235, GADInsDraw, &d_PrpCardBoxInfo,   GAD2SRender,  &d_prpcardR},
    {PEGAD_CAUTION, 0x80, 0, 10, 64,    PEGAD_LANG,    PEGAD_LANG,    PEGAD_LANG,    PEGAD_LANG,    259, 147, 324, 193, GADInsDraw, &d_PrpCardBoxInfo,   GADCardRen,   NULL},

    {PLGAD_NL, 0x00, 0, 0xff, 0xff, 0xff, 0xff, 0, 0xff, 0, 0, 0, 0, NULL, NULL, NULL }
    };
    
struct GadDir altdate[] = {

    {PEGAD_MONTH, 0x00, 0, 1, 100, PEGAD_HOUR, PEGAD_LACE, PEGAD_DAY, PEGAD_YEAR,  49, 111, 67, 127, GADInsDraw, &d_MonthBoxInfo, GADNumber, &d_MonthNumInfo},
    {PEGAD_DAY,   0x00, 0, 1, 100, PEGAD_HOUR, PEGAD_LACE, PEGAD_AM,  PEGAD_MONTH, 23, 111, 66, 127, GADInsDraw, &d_DayBoxInfo,   GADNumber, &d_DayNumInfo},
    };


