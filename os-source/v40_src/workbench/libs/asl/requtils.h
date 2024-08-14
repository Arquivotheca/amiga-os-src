#ifndef REQUTILS_H
#define REQUTILS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/locale.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>

#include "aslbase.h"
#include "texttable.h"


/*****************************************************************************/


struct ReqInfo
{
    ULONG              ri_ReqType;   /* ASL_FileRequest, ASL_FontRequest, etc... */
    ULONG	       ri_A4;        /* A4 on entry to AslRequest(), to be used for the callback hooks */

    /* these are things requested through tags, or by clients of this
     * module
     */
    struct IBox        ri_Coords;
    WORD               ri_MinWidth;
    WORD               ri_MinHeight;
    struct Window     *ri_UserWindow;
    struct Screen     *ri_UserScreen;
    STRPTR             ri_UserScreenName;
    struct TextAttr   *ri_UserAttr;
    AppStringsID       ri_TitleID;
    STRPTR             ri_TitleText;
    STRPTR             ri_PositiveText;
    STRPTR             ri_NegativeText;
    struct Locale     *ri_UserLocale;
    struct Hook       *ri_IntuiHook;
    ULONG __stdargs  (*ri_OldHook)(ULONG,ULONG,ULONG);
    ULONG	       ri_IDCMPFlags;
    ULONG	       ri_WindowFlags;
    BOOLEAN            ri_MakeSleep;
    BOOLEAN	       ri_PrivateIDCMP;
    BOOLEAN	       ri_CallOldHook;
    UBYTE              ri_Pad;

    /* these are fields initialized by OpenRequester() */
    struct LocaleInfo  ri_LocaleInfo;
    struct Window     *ri_Window;
    struct Screen     *ri_Screen;
    struct RastPort   *ri_RastPort;
    struct Menu       *ri_Menus;
    struct Gadget     *ri_Gadgets;
    struct DrawInfo   *ri_DrawInfo;
    APTR               ri_VisualInfo;
    struct Hook        ri_BackFill;

    struct TextFont   *ri_DFont;		/* Default font */
    struct TextAttr   *ri_DTextAttr;		/* Default text attribute */
    struct TextFont   *ri_Font;			/* Current font */
    struct TextAttr   *ri_TextAttr;		/* Current text attribute */

    struct Gadget     *ri_LastAdded;
    struct ASLGadget  *ri_Template;

    struct Requester   ri_SleepReq;
    BOOLEAN	       ri_ASleep;
    BOOLEAN	       ri_SharedIDCMP;

    struct Requester   ri_LocalSleepReq;
    BOOLEAN	       ri_LocalASleep;
    UBYTE              ri_Pad2;
};


/*****************************************************************************/


struct ASLMenu
{
    UBYTE        am_Type;        /* NM_XXX from gadtools.h */
    UBYTE        am_Command;
    AppStringsID am_Label;
};


/*****************************************************************************/


APTR PrepareRequester(struct ReqInfo *ri, APTR templates, ULONG templateSize, ULONG memorySize);
VOID PrepareLocale(struct ReqInfo *ri);
VOID CleanupRequester(struct ReqInfo *ri, APTR memory);

BOOL OpenRequester(struct ReqInfo *ri, struct ASLMenu *menus);
VOID CloseRequester(struct ReqInfo *ri);
VOID ProcessStdTag(struct ReqInfo *ri, struct TagItem *tag);
struct IntuiMessage *GetReqMsg(struct ReqInfo *ri, APTR object, struct Window *aux, ULONG extraSigs);

VOID SleepRequester(struct ReqInfo *ri);
VOID WakeupRequester(struct ReqInfo *ri);

ULONG CallFunc(ULONG __stdargs  (*oldHook)(ULONG,ULONG,ULONG),
               ULONG arg1, ULONG arg2, ULONG arg3, ULONG a4);



/*****************************************************************************/


#endif /* REQUTILS_H */
