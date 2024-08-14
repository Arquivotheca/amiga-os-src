#ifndef FONTREQ_H
#define FONTREQ_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <graphics/text.h>

#include "aslbase.h"
#include "requtils.h"


/*****************************************************************************/


struct ExtFontReq
{
    /* stuff needed for all requesters */
    struct ReqInfo   fo_ReqInfo;

    /* public portion of structure, must match definition in asl.h */
    STRPTR           fo_TitleText;
    struct Window   *fo_Window;
    struct TextAttr  fo_Attr;        /* Returned TextAttr                  */
    UBYTE            fo_FrontPen;    /* Returned front pen, if selected    */
    UBYTE            fo_BackPen;     /* Returned back pen, if selected     */
    UBYTE            fo_DrawMode;    /* Returned drawing mode, if selected */
    UBYTE	     fo_Reserved1;
    APTR             fo_UserData;    /* You can store your own data here   */
    struct IBox	     fo_Coords;      /* Coordinates of requester on exit   */
    struct TTextAttr fo_TAttr;

    /* private portion of structure */
    struct Hook     *fo_FilterFunc;
    UWORD            fo_MinHeight;
    UWORD	     fo_MaxHeight;
    ULONG	     fo_Options;

    UBYTE           *fo_FrontPens;
    UBYTE           *fo_BackPens;

    UWORD            fo_MaxFrontPen;
    UWORD            fo_MaxBackPen;

    UWORD            fo_KludgePad; /* see note below */
    char	     fo_Name[34];

    BOOL             fo_CallOldFilter;
    STRPTR          *fo_UserModes;

    struct MinList   fo_Fonts;
    struct MinList  *fo_CurrentSizes;

    STRPTR           fo_ModeLabels[12];

    char             fo_OriginalName[32];
    UWORD            fo_OriginalYSize;
    UBYTE            fo_OriginalStyle;
    UBYTE            fo_OriginalFlags;
    UBYTE            fo_OriginalFrontPen;
    UBYTE            fo_OriginalBackPen;
    UBYTE            fo_OriginalDrawMode;
};

/* fo_KludgePad is there for ancient compatibility. Make sure this sucker is
 * always longword aligned.
 *
 * When we were all younger, AslRequest() used to return a pointer to the font
 * name instead of a normal BOOL. This version of ASL keeps doing
 * this, but in order to ensure folks that use the return value of the font
 * req as a bool always work, the address of the name string must never
 * fall on a multiple of 64K in memory. Otherwise, the font name's address
 * could look something like 0x12340000. Folks using this value as a BOOL
 * (which is only a WORD in length) would see such a value as FALSE, even
 * though it is not. The solution is to misalign the name buffer so as to
 * make sure it is not longword aligned, and thus cannot fall on a 64K
 * boundary. Tadam!
 */

#define PUBLIC_FO(fo) ((struct FontRequester *)((ULONG)fo+sizeof(struct ReqInfo)))


/*****************************************************************************/


struct ExtFontReq *AllocFontRequest(struct TagItem *tagList);
APTR FontRequest(struct ExtFontReq *requester, struct TagItem *tagList);
VOID FreeFontRequest(struct ExtFontReq *requester);

VOID FlushFontCaches(VOID);


/*****************************************************************************/


#endif /* FONTREQ_H */
