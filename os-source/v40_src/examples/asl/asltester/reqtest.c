
/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/


/****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <utility/hooks.h>
#include <libraries/asl.h>
#include <libraries/locale.h>
#include <dos/dosasl.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/locale_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "reqtest.h"
#include "gadgets.h"


/****************************************************************************/


extern struct Library *IntuitionBase;
extern struct Library *LocaleBase;
extern struct Library *SysBase;
extern struct Library *AslBase;
extern struct Library *DOSBase;
extern struct Window  *window;
extern struct Screen  *screen;


/****************************************************************************/


VOID kprintf(STRPTR,...);


/****************************************************************************/


/* Say something to the user... */
static VOID Report(STRPTR text)
{
struct EasyStruct est;

    est.es_StructSize   = sizeof(struct EasyStruct);
    est.es_Flags        = 0;
    est.es_Title        = NULL;
    est.es_TextFormat   = text;
    est.es_GadgetFormat = "Continue";

    EasyRequestArgs(window,&est,NULL,NULL);
}


/*****************************************************************************/


/* This function gets called whenever a new file or directory is about to be
 * added to to the list in a file requester. It lets an application filter out
 * entries it doesn't want. You can look at the fields in the AnchorPath
 * structure to decide whether you want this entry added to the list or not.
 * Return TRUE if the entry should be added, return FALSE otherwise.
 */
static ULONG __asm FileFilter(register __a0 struct Hook *hook,
                              register __a2 struct FileRequester *fr,
                              register __a1 struct AnchorPath *anchor)
{
    kprintf("FileFilter(): name is '%s'\n",anchor->ap_Info.fib_FileName);
    return(TRUE);
}


/*****************************************************************************/


/* This function gets called whenever a new font or size is about to be
 * added to to the lists in a font requester. It lets an application filter out
 * entries it doesn't want. You can look at the fields in the TextAttr
 * structure to decide whether you want this entry added to the list or not.
 * Return TRUE if the entry should be added, return FALSE otherwise.
 */
static ULONG __asm FontFilter(register __a0 struct Hook *hook,
                              register __a2 struct FontRequester *fo,
                              register __a1 struct TextAttr *textAttr)
{
    kprintf("FontFilter(): name is '%s', size is %ld\n",textAttr->ta_Name,textAttr->ta_YSize);
    return(TRUE);
}


/*****************************************************************************/


/* This function gets called whenever a new screen mode is about to be
 * added to to the list in a screen mode requester. It lets an application
 * filter out entries it doesn't want. You can query the display database
 * using the provided mode id to decide whether you want this entry added to
 * the list or not. Return TRUE if the entry should be added, return FALSE
 * otherwise.
 */
static ULONG __asm ScreenModeFilter(register __a0 struct Hook *hook,
                                    register __a2 struct ScreenModeRequester *smr,
                                    register __a1 ULONG modeID)
{
    kprintf("ScreenModeFilter(): mode is id 0x%08lx\n",modeID);
    return(TRUE);
}


/*****************************************************************************/


/* This function gets called whenever an IntuiMessage comes in to a
 * requester IDCMP, which is not meant for the requester itself. That is,
 * when the IntuiMessage's IDCMPWindow field does not point to the
 * requester's window. This happens when an IDCMP is shared between a
 * parent window and a requester. Within this function, you can attend to
 * the IDCMP event for your window. If your application is using many windows,
 * you can look at the IDCMPWindow field of the event to figure out to
 * which window this event is addressed.
 *
 * This function is typically used to refresh damage that occurs in a window
 * while an ASL requester is up.
 */
static ULONG __asm IntuiMsgFunc(register __a0 struct Hook *hook,
                                register __a1 struct IntuiMessage *intuiMsg,
                                register __a2 struct Requester *req)
{
    kprintf("IntuiMsgFunc(): class is 0x%08lx, code = 0x%04lx\n",intuiMsg->Class,intuiMsg->Code);
    return(TRUE);
}


/*****************************************************************************/


#define SETTAG(tag,value) {tags[cnt].ti_Tag = tag; tags[cnt].ti_Data = (ULONG)value; cnt++;}
#define SETTEXTTAG(tag,value) {if (value[0]) {tags[cnt].ti_Tag = tag; tags[cnt].ti_Data = (ULONG)value; cnt++;}}
#define SETHOOKTAG(tag,state,hook,function) {if (state) {hook.h_Entry = (HOOKFUNC)function; SETTAG(tag,&hook)}}


/* This is where it all happens...
 *
 * This function opens up an ASL requester. The requester's options are set in
 * accordance to the contents of the 4 structure passed as parameter. The
 * requesterType variable determines which type of requester gets displayed.
 */
VOID TestRequester(struct CommonData *cd,
                   struct FileReqData *frd,
                   struct FontReqData *fod,
                   struct ScreenModeReqData *smrd,
                   ULONG requesterType)
{
APTR                requester;
struct TagItem      tags[80];
ULONG               cnt;
struct Requester    sleepReq;
BOOL                sleeping;
struct Locale      *locale;
struct TextAttr     textAttr;
struct Hook         filterHook;
struct Hook         intuiMsgHook;
char                acceptPattern[sizeof(frd->AcceptPattern)];
char                rejectPattern[sizeof(frd->RejectPattern)];
STRPTR              fmodes[5];
struct List         dmodeList;
struct DisplayMode  dmode;
struct Window      *refWindow;
struct Screen      *refScreen1;
struct Screen      *refScreen2;
STRPTR              refName;

    InitRequester(&sleepReq);
    sleeping = Request(&sleepReq,window);
    SetWindowPointer(window,WA_BusyPointer,TRUE,TAG_DONE);

    memset(tags,0,sizeof(tags));
    cnt = 0;

    locale = NULL;
    if (cd->Locale[0])
        locale = OpenLocale(cd->Locale);

    textAttr.ta_Name  = cd->FontName;
    textAttr.ta_YSize = cd->FontSize;
    textAttr.ta_Style = 0;
    textAttr.ta_Flags = FPF_DISKFONT;

    refWindow  = NULL;
    refScreen1 = NULL;
    refScreen2 = NULL;
    refName    = NULL;
    if (cd->RefObject == REF_WINDOW)
    {
        refWindow = OpenWindowTags(NULL,WA_Width,         200,
                                        WA_Height,        screen->WBorTop + screen->Font->ta_YSize + 1,
                                        WA_Title,         "Reference Window",
                                        WA_PubScreen,     screen,
                                        WA_DragBar,       TRUE,
                                        WA_DepthGadget,   TRUE,
                                        WA_NoCareRefresh, TRUE,
                                        WA_SimpleRefresh, TRUE,
                                        WA_AutoAdjust,    TRUE,
                                        TAG_DONE);
    }
    else if (cd->RefObject == REF_SCREEN)
    {
        refScreen1 = OpenScreenTags(NULL,SA_LikeWorkbench,TRUE,
                                         SA_Title,        "ASLTESTER",
                                         TAG_DONE);
    }
    else if (cd->RefObject == REF_PUBSCREENNAME)
    {
        refName    = "ASLTESTER";
        refScreen2 = OpenScreenTags(NULL,SA_LikeWorkbench, TRUE,
                                         SA_PubName,       "ASLTESTER",
                                         SA_Title,         "ASLTESTER",
                                         TAG_DONE);

        if (refScreen2)
            PubScreenStatus(refScreen2,0);
    }

    if (requesterType == REQ_FILE)
    {
        SETTAG(ASLFR_Window,refWindow);
        SETTAG(ASLFR_Screen,refScreen1);
        SETTAG(ASLFR_PubScreenName,refName);
        SETTAG(ASLFR_InitialLeftEdge,cd->InitialLeftEdge);
        SETTAG(ASLFR_InitialTopEdge,cd->InitialTopEdge);
        SETTAG(ASLFR_InitialWidth,cd->InitialWidth);
        SETTAG(ASLFR_InitialHeight,cd->InitialHeight);
        SETTEXTTAG(ASLFR_TitleText,cd->TitleText);
        SETTEXTTAG(ASLFR_PositiveText,cd->PositiveText);
        SETTEXTTAG(ASLFR_NegativeText,cd->NegativeText);
        SETTAG(ASLFR_Locale,locale);
        SETTAG(ASLFR_PrivateIDCMP,cd->PrivateIDCMP);
        SETTAG(ASLFR_SleepWindow,cd->SleepWindow);
        SETHOOKTAG(ASLFR_IntuiMsgFunc,cd->IntuiMsgFunc,intuiMsgHook,IntuiMsgFunc);
        SETHOOKTAG(ASLFR_FilterFunc,cd->FilterFunc,filterHook,FileFilter);

        if (cd->FontName[0])
            SETTAG(ASLFR_TextAttr,&textAttr);

        SETTEXTTAG(ASLFR_InitialFile,frd->InitialFile);
        SETTEXTTAG(ASLFR_InitialDrawer,frd->InitialDrawer);
        SETTEXTTAG(ASLFR_InitialPattern,frd->InitialPattern);
        SETTAG(ASLFR_DoSaveMode,frd->DoSaveMode);
        SETTAG(ASLFR_DoMultiSelect,frd->DoMultiSelect);
        SETTAG(ASLFR_DoPatterns,frd->DoPatterns);
        SETTAG(ASLFR_DrawersOnly,frd->DrawersOnly);
        SETTAG(ASLFR_RejectIcons,frd->RejectIcons);
        SETTAG(ASLFR_FilterDrawers,frd->FilterDrawers);

        ParsePatternNoCase(frd->AcceptPattern,acceptPattern,sizeof(acceptPattern));
        SETTEXTTAG(ASLFR_AcceptPattern,acceptPattern);

        ParsePatternNoCase(frd->RejectPattern,rejectPattern,sizeof(rejectPattern));
        SETTEXTTAG(ASLFR_RejectPattern,rejectPattern);
    }
    else if (requesterType == REQ_FONT)
    {
        SETTAG(ASLFO_Window,refWindow);
        SETTAG(ASLFO_Screen,refScreen1);
        SETTAG(ASLFO_PubScreenName,refName);
        SETTAG(ASLFO_InitialLeftEdge,cd->InitialLeftEdge);
        SETTAG(ASLFO_InitialTopEdge,cd->InitialTopEdge);
        SETTAG(ASLFO_InitialWidth,cd->InitialWidth);
        SETTAG(ASLFO_InitialHeight,cd->InitialHeight);
        SETTEXTTAG(ASLFO_TitleText,cd->TitleText);
        SETTEXTTAG(ASLFO_PositiveText,cd->PositiveText);
        SETTEXTTAG(ASLFO_NegativeText,cd->NegativeText);
        SETTAG(ASLFO_Locale,locale);
        SETTAG(ASLFO_PrivateIDCMP,cd->PrivateIDCMP);
        SETTAG(ASLFO_SleepWindow,cd->SleepWindow);
        SETHOOKTAG(ASLFO_IntuiMsgFunc,cd->IntuiMsgFunc,intuiMsgHook,IntuiMsgFunc);
        SETHOOKTAG(ASLFO_FilterFunc,cd->FilterFunc,filterHook,FontFilter);

        if (cd->FontName[0])
            SETTAG(ASLFO_TextAttr,&textAttr);

        SETTEXTTAG(ASLFO_InitialName,fod->InitialName);
        SETTAG(ASLFO_InitialSize,fod->InitialSize);
        SETTAG(ASLFO_InitialFrontPen,fod->InitialFrontPen);
        SETTAG(ASLFO_InitialBackPen,fod->InitialBackPen);
        SETTAG(ASLFO_InitialStyle,fod->InitialStyle);
        SETTAG(ASLFO_InitialDrawMode,fod->InitialDrawMode);
        SETTAG(ASLFO_DoFrontPen,fod->DoFrontPen);
        SETTAG(ASLFO_DoBackPen,fod->DoBackPen);
        SETTAG(ASLFO_DoStyle,fod->DoStyle);
        SETTAG(ASLFO_DoDrawMode,fod->DoDrawMode);
        SETTAG(ASLFO_FixedWidthOnly,fod->FixedWidthOnly);
        SETTAG(ASLFO_MinHeight,fod->MinHeight);
        SETTAG(ASLFO_MaxHeight,fod->MaxHeight);
        SETTAG(ASLFO_MaxFrontPen,fod->MaxFrontPen);
        SETTAG(ASLFO_MaxBackPen,fod->MaxBackPen);

        if (fod->ModeList)
        {
            /* Create an array of values to replace the name and label of
             * the Mode gadget with custom stuff...
             */
            fmodes[0] = "MyModes";
            fmodes[1] = "Custom-1";
            fmodes[2] = "Custom-2";
            fmodes[3] = "Custom-3";
            fmodes[4] = NULL;
            SETTAG(ASLFO_ModeList,&fmodes);
        }
    }
    else /* if (requesterType == REQ_SCREENMODE) */
    {
        SETTAG(ASLSM_Window,refWindow);
        SETTAG(ASLSM_Screen,refScreen1);
        SETTAG(ASLSM_PubScreenName,refName);
        SETTAG(ASLSM_InitialLeftEdge,cd->InitialLeftEdge);
        SETTAG(ASLSM_InitialTopEdge,cd->InitialTopEdge);
        SETTAG(ASLSM_InitialWidth,cd->InitialWidth);
        SETTAG(ASLSM_InitialHeight,cd->InitialHeight);
        SETTEXTTAG(ASLSM_TitleText,cd->TitleText);
        SETTEXTTAG(ASLSM_PositiveText,cd->PositiveText);
        SETTEXTTAG(ASLSM_NegativeText,cd->NegativeText);
        SETTAG(ASLFO_Locale,locale);
        SETTAG(ASLSM_PrivateIDCMP,cd->PrivateIDCMP);
        SETTAG(ASLSM_SleepWindow,cd->SleepWindow);
        SETHOOKTAG(ASLSM_IntuiMsgFunc,cd->IntuiMsgFunc,intuiMsgHook,IntuiMsgFunc);
        SETHOOKTAG(ASLSM_FilterFunc,cd->FilterFunc,filterHook,ScreenModeFilter);

        if (cd->FontName[0])
            SETTAG(ASLSM_TextAttr,&textAttr);

        SETTAG(ASLSM_InitialDisplayID,smrd->InitialDisplayID);
        SETTAG(ASLSM_InitialDisplayWidth,smrd->InitialDisplayWidth);
        SETTAG(ASLSM_InitialDisplayHeight,smrd->InitialDisplayHeight);
        SETTAG(ASLSM_InitialDisplayDepth,smrd->InitialDisplayDepth);
        SETTAG(ASLSM_InitialOverscanType,smrd->InitialOverscanType);
        SETTAG(ASLSM_InitialInfoLeftEdge,smrd->InitialInfoLeftEdge);
        SETTAG(ASLSM_InitialInfoTopEdge,smrd->InitialInfoTopEdge);
        SETTAG(ASLSM_InitialInfoOpened,smrd->InitialInfoOpened);
        SETTAG(ASLSM_InitialAutoScroll,smrd->InitialAutoScroll);
        SETTAG(ASLSM_DoWidth,smrd->DoWidth);
        SETTAG(ASLSM_DoHeight,smrd->DoHeight);
        SETTAG(ASLSM_DoDepth,smrd->DoDepth);
        SETTAG(ASLSM_DoOverscanType,smrd->DoOverscanType);
        SETTAG(ASLSM_DoAutoScroll,smrd->DoAutoScroll);
        SETTAG(ASLSM_PropertyFlags,smrd->PropertyFlags);
        SETTAG(ASLSM_PropertyMask,smrd->PropertyMask);
        SETTAG(ASLSM_MinWidth,smrd->MinWidth);
        SETTAG(ASLSM_MaxWidth,smrd->MaxWidth);
        SETTAG(ASLSM_MinHeight,smrd->MinHeight);
        SETTAG(ASLSM_MaxHeight,smrd->MaxHeight);
        SETTAG(ASLSM_MinDepth,smrd->MinDepth);
        SETTAG(ASLSM_MaxDepth,smrd->MaxDepth);

        if (smrd->CustomSMList)
        {
            /* Create a list containing a single DisplayMode structure to use
             * for a custom screen mode list
             */
            memset(&dmode,0,sizeof(struct DimensionInfo));
            dmode.dm_Node.ln_Name                   = "Custom Mode";
            dmode.dm_PropertyFlags                  = DIPF_IS_PAL|DIPF_IS_GENLOCK|DIPF_IS_WB;
            dmode.dm_DimensionInfo.Header.StructID  = DTAG_DIMS;
            dmode.dm_DimensionInfo.Header.DisplayID = 0xFFFF0001;
            dmode.dm_DimensionInfo.Header.SkipID    = TAG_SKIP;
            dmode.dm_DimensionInfo.Header.Length    = sizeof(struct DimensionInfo);
            dmode.dm_DimensionInfo.MaxDepth         = 24;
            dmode.dm_DimensionInfo.MinRasterWidth   = 16;
            dmode.dm_DimensionInfo.MinRasterHeight  = 16;
            dmode.dm_DimensionInfo.MaxRasterWidth   = 2048;
            dmode.dm_DimensionInfo.MaxRasterHeight  = 2048;
            dmode.dm_DimensionInfo.Nominal.MinX     = 0;
            dmode.dm_DimensionInfo.Nominal.MinY     = 0;
            dmode.dm_DimensionInfo.Nominal.MaxX     = 640;
            dmode.dm_DimensionInfo.Nominal.MaxY     = 200;
            dmode.dm_DimensionInfo.TxtOScan         = dmode.dm_DimensionInfo.Nominal;
            dmode.dm_DimensionInfo.MaxOScan         = dmode.dm_DimensionInfo.Nominal;
            dmode.dm_DimensionInfo.VideoOScan       = dmode.dm_DimensionInfo.Nominal;
            dmode.dm_DimensionInfo.StdOScan         = dmode.dm_DimensionInfo.Nominal;

            NewList(&dmodeList);
            AddHead(&dmodeList,&dmode);

            SETTAG(ASLSM_CustomSMList,&dmodeList);
        }
    }

    if (AslBase = OpenLibrary("asl.library",39))
    {
        if (requester = AllocAslRequest(requesterType,tags))
        {
            if (AslRequest(requester,NULL))
            {
                // OK was selected
                // To be useful, we should now open a window and show the
                // values returned in the various requester structures
            }
            else
            {
                if (IoErr())
                {
                    Report("Could not present requester!");
                }
                else
                {
                    Report("Cancel was selected.");
                }
            }

            FreeAslRequest(requester);
        }
        else
        {
            Report("Could not create requester!");
        }

        CloseLibrary(AslBase);
    }
    else
    {
        Report("Could not open asl.library V39!");
    }

    if (refWindow)
        CloseWindow(refWindow);

    if (refScreen1)
        CloseScreen(refScreen1);

    if (refScreen2)
        CloseScreen(refScreen2);

    CloseLocale(locale);

    if (sleeping)
        EndRequest(&sleepReq,window);

    ClearPointer(window);
}
