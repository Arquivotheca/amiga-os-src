#ifndef ASLBASE_H
#define ASLBASE_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <utility/tagitem.h>
#include <libraries/diskfont.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <dos.h>


/*****************************************************************************/


struct ASLLib
{
    struct Library           ASL_Lib;
    UWORD                    ASL_UsageCnt;
    APTR                     ASL_IntuitionBase;
    struct GfxBase          *ASL_GfxBase;
    APTR                     ASL_DosBase;
    APTR                     ASL_UtilityBase;
    APTR                     ASL_GadToolsBase;
    APTR                     ASL_LayersBase;
    APTR		     ASL_DiskfontBase;
    APTR		     ASL_WorkbenchBase;
    APTR                     ASL_SysBase;
    APTR		     ASL_IconBase;
    BPTR                     ASL_SegList;

    APTR	             ASL_SMult32;
    APTR	             ASL_UMult32;
    APTR	             ASL_SDivMod32;
    APTR	             ASL_UDivMod32;

    /* For layout engine */
    struct TextFont         *ASL_FFont;		/* Fallback font */
    struct TextAttr         *ASL_FTextAttr;	/* Fallback text attribute */

    /* For font list cacher */
    struct AvailFontsHeader *ASL_AFH;
    struct MinList           ASL_FontCache;
    struct SignalSemaphore   ASL_FontCacheLock;
};

#define ASM           __asm
#define REG(x)	      register __ ## x

#define AslBase       ((struct ASLLib *)getreg(REG_A6))
#define SysBase       AslBase->ASL_SysBase
#define DOSBase       AslBase->ASL_DosBase
#define IntuitionBase AslBase->ASL_IntuitionBase
#define UtilityBase   AslBase->ASL_UtilityBase
#define GfxBase       AslBase->ASL_GfxBase
#define GadToolsBase  AslBase->ASL_GadToolsBase
#define LayersBase    AslBase->ASL_LayersBase
#define DiskfontBase  AslBase->ASL_DiskfontBase
#define WorkbenchBase AslBase->ASL_WorkbenchBase
#define	IconBase      AslBase->ASL_IconBase

#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);
kprintf(STRPTR,...);
sprintf(STRPTR,...);


/*****************************************************************************/


/* common tags, handled by single routine */
#define ASL_Screen          ASLFR_Screen
#define ASL_PubScreenName   ASLFR_PubScreenName
#define ASL_PrivateIDCMP    ASLFR_PrivateIDCMP
#define ASL_IntuiMsgFunc    ASLFR_IntuiMsgFunc
#define ASL_SleepWindow     ASLFR_SleepWindow

#define ASL_TextAttr        ASLFR_TextAttr
#define ASL_Locale          ASLFR_Locale
#define ASL_TitleText       ASLFR_TitleText
#define ASL_PositiveText    ASLFR_PositiveText
#define ASL_NegativeText    ASLFR_NegativeText

#define ASL_InitialLeftEdge ASLFR_InitialLeftEdge
#define ASL_InitialTopEdge  ASLFR_InitialTopEdge
#define ASL_InitialWidth    ASLFR_InitialWidth
#define ASL_InitialHeight   ASLFR_InitialHeight


/*****************************************************************************/


#ifndef GTST_EditHook
#define GTST_EditHook GT_TagBase+55
#endif

#ifndef GTIN_EditHook
#define GTIN_EditHook GT_TagBase+55
#endif


/*****************************************************************************/


#define BOOLEAN UBYTE    /* byte boolean */


/*****************************************************************************/


APTR ASM AllocAslRequest(REG(d0) ULONG reqType,
                         REG(a0) struct TagItem *tagList);

VOID ASM FreeAslRequest(REG(a0) APTR requester);

APTR ASM AslRequest(REG(a0) APTR requester,
		    REG(a1) struct TagItem *tagList);

VOID ASM FlushLib(VOID);


/*****************************************************************************/


#endif /* ASLBASE_H */
