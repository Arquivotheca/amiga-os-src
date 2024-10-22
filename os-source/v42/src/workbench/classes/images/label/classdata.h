#ifndef CLASSDATA_H
#define CLASSDATA_H

/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/


#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <intuition/screens.h>

#include "classbase.h"

/*****************************************************************************
 *
 *  Constants
 *
 *****************************************************************************/

/* Default dimensions */
#define LABEL_DEFAULT_WIDTH         40
#define LABEL_DEFAULT_HEIGHT        40

/* Default underscore prefix */
#define LABEL_DEFAULT_UNDERSCORE    '_'

/* Underscoring */
#define LABEL_UNDERSCORE_YOFFSET    2

/* Escape */
#define LABEL_UNDERSCORE_ESCAPE     '\\'

/* Blitter minterm definitions */
#define MINTERM_COPY                0xC0    /* Copy */

/*****************************************************************************
 *
 *  Structures
 *
 *****************************************************************************/

struct objectData
{

    struct DrawInfo *od_DrawInfo;

    LONG                od_BackgroundPen;
    LONG                od_TextPen;

    STRPTR              od_Text;
    struct TextAttr *   od_TextAttr;
    struct TextFont *   od_TextFont;
    UBYTE               od_Underscore;

    struct BitMap *     od_Bitmap;

};

/* Text fragment - used in text layout */
struct textFragment {

    struct textFragment *next;      /* Link */

    STRPTR              text;       /* Text */
    WORD                x, y;       /* Position */
    UWORD               width,      /* Dimension */
                        height;
    ULONG               softStyle;  /* Soft style */
    LONG                bgPen;      /* Background pen */
    LONG                fgPen;      /* Foreground pen */
    BOOL                fUnderscore;/* Underscored? */

};

/*****************************************************************************
 *
 *  Type Definitions
 *
 *****************************************************************************/

enum textCode {

    code_Bad,           /* Bad code */
    code_APen,          /* {APen <nPen>} */
    code_BoldOn,        /* {B} */
    code_Background,    /* {BG <color>} */
    code_ClearTabs,     /* {CLEARTABS} */
    code_Foreground,    /* {FG <color>} */
    code_ItalicOn,      /* {I} */
    code_JustifyCenter, /* {JCENTER} */
    code_JustifyLeft,   /* {JLEFT} */
    code_JustifyRight,  /* {JRIGHT} */
    code_Plain,         /* {PLAIN} */
    code_SetTabs,       /* {SETTABS <n> ... <n>} */
    code_Tab,           /* {TAB} */
    code_UnderlineOn,   /* {U} */
    code_BoldOff,       /* {UB} */
    code_ItalicOff,     /* {UI} */
    code_UnderlineOff   /* {UU} */

};

/*****************************************************************************
 *
 *  Prototypes
 *
 *****************************************************************************/

/* from dispatch.c */
LONG ASM ClassDispatcher(
    REG(a0) Class *pClass,
    REG(a1) ULONG *pMsg,
    REG(a2) struct Image *pImage
);

/* from text.c */
struct textFragment *layoutText(
    struct ClassLib *ClassBase,
    struct objectData *pInstanceData,
    SHORT *pWidth,
    SHORT *pHeight,
    struct DrawInfo *pDrawInfo
);
void unlayoutText(
    struct ClassLib *ClassBase,
    struct textFragment *pFragmentList
);
BOOL renderText(
    struct ClassLib *ClassBase,
    struct objectData *pInstanceData,
    struct RastPort *rastPort,
    struct textFragment *pFragmentList,
    UWORD offsetX, UWORD offsetY,
    struct DrawInfo *pDrawInfo
);
enum textCode parseTextCode(
    STRPTR codeString,
    STRPTR *pEnd
);

/* from bitmap.c */
BOOL renderBitmap(
    struct ClassLib *ClassBase,
    struct RastPort *rastPort,
    struct BitMap *pBitmap,
    UWORD offsetX, UWORD offsetY,
    UWORD width, UWORD height
);

/* from string.c */
STRPTR createString(struct ClassLib *ClassBase, STRPTR source);
void destroyString(struct ClassLib *ClassBase, STRPTR string);

#endif /* CLASSDATA_H */
