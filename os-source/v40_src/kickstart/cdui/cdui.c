
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/pointerclass.h>
#include <graphics/displayinfo.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <libraries/lowlevel.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "cduibase.h"
#include "utils.h"
#include "languageselector.h"
#include "nrameditor.h"
#include "doanimation.h"
#include "allocpatch.h"


/*****************************************************************************/


extern struct Custom __far custom;


/*****************************************************************************/


#undef SysBase

VOID CDUITask(VOID)
{
UWORD             i;
struct CDUILib   *CDUIBase;
struct ExecBase  *SysBase = (*((struct ExecBase **) 4));
struct Rectangle  textRect;
struct Rectangle  maxRect;
LONG              excess;
struct MsgPort   *port;
struct Message   *msg;
struct BitMap    *pointerBM;
ULONG             action;
ULONG             bits;

    Forbid();

    // find the lib base. Can't use OpenLibrary(), since the library
    // only allows one opener at a time
    CDUIBase = (struct CDUILib *)FindName(&SysBase->LibList,"cdui.library");

    Permit();

    bits = SetChipRev(SETCHIPREV_AA);
    if (!(bits & GFXF_AA_LISA))
        return;                  /* we need AA */

    // make sure we're not flushed while the sub-task is running
    CDUIBase->cb_Lib.lib_OpenCnt++;

    // required by the sub-tasks in this library
    FindTask(NULL)->tc_UserData = CDUIBase;

    port = CreateMsgPort();
    port->mp_Node.ln_Name = STARTUP_ANIM_PORT;
    AddPort(port);

    // indicate to the parent task that we are ready for business
    CDUIBase->cb_CDUIPort = port;

    // allocate a blank sprite imagery...
    pointerBM = AllocBitMap(64,1,2,BMF_CLEAR,NULL);
    CDUIBase->cb_Pointer = NewObject(NULL,"pointerclass",POINTERA_BitMap, pointerBM, TAG_DONE),

    ON_SPRITE; /* required when running from ROM */

    while (TRUE)
    {
        action = DoAnimation(CDUIBase,port);
        if (action == CDUI_EXIT)
        {
            /* we're now running under Forbid() */

            RemPort(port);
            while (msg = GetMsg(port))
                ReplyMsg(msg);

            DeleteMsgPort(port);
            DisposeObject(CDUIBase->cb_Pointer);
            FreeBitMap(pointerBM);
            CloseLibrary(CDUIBase);   /* redecrement the open count */
            return;
        }

        /* provide initially black color table */
        CDUIBase->cb_Colors.cp_NumColors  = NUM_COLORS;
        CDUIBase->cb_Colors.cp_FirstColor = 0;
        CDUIBase->cb_Colors.cp_EndMarker  = 0;
        for (i = 0; i < NUM_COLORS; i++)
        {
            CDUIBase->cb_Colors.cp_Colors[i].Red   = 0;
            CDUIBase->cb_Colors.cp_Colors[i].Green = 0;
            CDUIBase->cb_Colors.cp_Colors[i].Blue  = 0;
        }

        SystemControl(SCON_AddCreateKeys,0,
                      SCON_AddCreateKeys,1,
                      SCON_AddCreateKeys,2,
                      SCON_AddCreateKeys,3,
                      TAG_DONE);

        /* get the overscan values for this display id */
        QueryOverscan(LORES_KEY,&textRect,OSCAN_TEXT);
        QueryOverscan(LORES_KEY,&maxRect,OSCAN_MAX);

        /* now, make the rectangle as tall as max overscan */
        textRect.MinY = maxRect.MinY;
        textRect.MaxY = maxRect.MaxY;

        /* and adjust things so our display is horizontally centered relative to
         * text oscan
         */
        excess         = 322 - (textRect.MaxX - textRect.MinX + 1);
        textRect.MinX -= excess / 2;
        textRect.MaxX += (excess - (excess / 2));

        IBASE_NUKEPOINTER(IntuitionBase) = (STRPTR)TRUE;

        CDUIBase->cb_Screen = OpenScreenTags(NULL,
                                     SA_Depth,           NUM_PLANES,
                                     SA_DisplayID,       LORES_KEY,
                                     SA_Interleaved,     TRUE,
                                     SA_Draggable,       FALSE,
                                     SA_Quiet,           TRUE,
                                     SA_ShowTitle,       FALSE,
                                     SA_Colors32,        &CDUIBase->cb_Colors,
                                     SA_DClip,           &textRect,
                                     SA_Font,            &topazAttr,
                                     SA_ColorMapEntries, 256,
                                     SA_BackFill,        LAYERS_NOBACKFILL,
                                     TAG_DONE);

        CDUIBase->cb_RastPort = &CDUIBase->cb_Screen->RastPort;
        CDUIBase->cb_BitMap   =  CDUIBase->cb_RastPort->BitMap;
        CDUIBase->cb_ViewPort = &CDUIBase->cb_Screen->ViewPort;

        CDUIBase->cb_Window = OpenWindowTags(NULL,
                                     WA_CustomScreen,  CDUIBase->cb_Screen,
                                     WA_Borderless,    TRUE,
                                     WA_Backdrop,      TRUE,
                                     WA_RMBTrap,       TRUE,
                                     WA_Activate,      TRUE,
                                     WA_IDCMP,         IDCMP_RAWKEY,
                                     WA_RptQueue,      1,
                                     WA_NoCareRefresh, TRUE,
                                     WA_SimpleRefresh, TRUE,
                                     WA_BackFill,      LAYERS_NOBACKFILL,
                                     TAG_DONE);

        if      (action == CDUI_LANGUAGE_SELECTOR) LanguageSelector(CDUIBase);
        else if (action  = CDUI_NVRAM_EDITOR)      NRAMEditor(CDUIBase);

        CloseWindow(CDUIBase->cb_Window);
        CloseScreenQuiet(CDUIBase,CDUIBase->cb_Screen);

        CDUIBase->cb_Window    = NULL;

        SystemControl(SCON_RemCreateKeys,0,
                      SCON_RemCreateKeys,1,
                      SCON_RemCreateKeys,2,
                      SCON_RemCreateKeys,3,
                      TAG_DONE);
    }
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


struct MsgPort * ASM StartCDUILVO(REG(a6) struct CDUILib *CDUIBase)
{
    if (CDUIBase->cb_CDUITask = CreateTask("CDUITask",15,CDUITask,4096))
    {
        // wait until the child is ready
        while (!CDUIBase->cb_CDUIPort)
            WaitTOF();

        return(CDUIBase->cb_CDUIPort);
    }

    return(NULL);
}


/*****************************************************************************/


VOID __asm InstallAllocPatch(register __a6 struct CDUILib *CDUIBase)
{
    Forbid();

    CDUIBase->cb_AllocPatch = AllocVec(patchSize,MEMF_REVERSE);
    CopyMem((APTR)PatchStart,CDUIBase->cb_AllocPatch,patchSize);
    CDUIBase->cb_AllocPatch->ap_OldAllocMem = (APTR)SetFunction(SysBase,-198,(VOID *)&CDUIBase->cb_AllocPatch->ap_PatchCodeStart);
    CacheClearU();

    Permit();
}


/*****************************************************************************/


VOID __asm RemoveAllocPatch(register __a6 struct CDUILib *CDUIBase)
{
    if (CDUIBase->cb_AllocPatch)
    {
        SetFunction(CDUIBase->cb_SysLib,-198,CDUIBase->cb_AllocPatch->ap_OldAllocMem);
        WaitTOF(); // allow any folks in the patch to get out of there...
        FreeVec(CDUIBase->cb_AllocPatch);
        CDUIBase->cb_AllocPatch = NULL;
    }
}
