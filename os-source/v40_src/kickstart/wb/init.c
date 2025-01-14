/*
 * $Id: init.c,v 38.2 92/06/11 09:26:47 mks Exp $
 *
 * $Log:	init.c,v $
 * Revision 38.2  92/06/11  09:26:47  mks
 * Now calls OpenPWindow() with better parameters...
 * 
 * Revision 38.1  91/06/24  11:36:41  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/alerts.h"
#include "workbench.h"
#include "intuition/intuition.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

struct Window *
OpenDiskWin(obj)
struct WBObject *obj;
{
    struct WorkbenchBase *wb = getWbBase();
    struct NewDD *dd;
    struct Window *win;
    struct NewWindow *nw;
    struct Process *pr = (struct Process *)(wb->wb_Task);
    UBYTE *ptr;
    int barheight=wb->wb_Screen->BarHeight + 1;

    dd = obj->wo_DrawerData;
    nw = &dd->dd_NewWindow;
    DP(("OpenDiskWindow: obj=%lx (%s), dd=%lx, nw=%lx, DrawerOpen=%ld\n",obj, obj->wo_Name, dd, nw, obj->wo_DrawerOpen));

    if ((wb->wb_Backdrop) || (wb->wb_Config.wbc_Version==0))
    {
	DP(("Using default full-screen settings\n"));
        nw->LeftEdge = 0;
        nw->TopEdge = barheight;
        nw->MinWidth = nw->MaxWidth = nw->Width = wb->wb_Screen->Width;
        nw->MinHeight = nw->MaxHeight = nw->Height = wb->wb_Screen->Height - barheight;
    }
    if (obj->wo_DrawerOpen) { /* re-open (ie. an OpenWorkbench) */
	if (wb->wb_Backdrop) {
	    DP(("ODW: going to backdrop\n"));
	    goto backdrop; /* open as a backdrop window */
	}
	else {
	    DP(("ODW: calling FormatWindowName\n"));
	    FormatWindowName(obj, &ptr, NULL); /* null initial window */

	    if (!dd->dd_VertScroll.NextGadget) {
		DP(("ODW: calling InitGadgets\n"));
		InitGadgets(dd);
	    }

	    DP(("ODW: calling OpenPWindow\n"));
	    win = OpenPWindow(&dd->dd_NewWindow, SIMPLE_REFRESH|
		WINDOWSIZING|WINDOWDRAG|WBENCHWINDOW|WINDOWDEPTH|
		SIZEBBOTTOM|SIZEBRIGHT|REPORTMOUSE|ACTIVATE|WINDOWCLOSE,
		&dd->dd_HorizScroll,		/* gadgets */
		ptr,				/* title */
		DISKINSERTED | DISKREMOVED,	/* extraIDCMP */
		TRUE,				/* wb pattern flag */
		dd->dd_ViewModes		/* view modes */
	    );
	    DP(("\tOK\n"));
	    obj->wo_DrawerData->dd_DrawerWin = win;
	}
    }
    else
    {
	/* open (not a re-open) */
	if (wb->wb_Backdrop) {
backdrop:
	    DP(("ODW: calling OpenPWindow\n"));
	    win = OpenPWindow(&dd->dd_NewWindow,
		SIMPLE_REFRESH|BACKDROP|BORDERLESS|WBENCHWINDOW|
		REPORTMOUSE|ACTIVATE,
		NULL,				/* gadgets */
		NULL,				/* title */
		DISKINSERTED | DISKREMOVED,	/* extraIDCMP */
		TRUE,				/* wb pattern flag */
		dd->dd_ViewModes		/* view modes */
	    );
	    DP(("\tOK\n"));
	    if (dd->dd_DrawerWin = win)
	    {
		dd->dd_CurrentX=0;
		dd->dd_CurrentY=0;

		/* protect against sizing and set up the "view" */
		win->MinWidth = win->MaxWidth = win->Width;
		win->MinHeight = win->MaxHeight = win->Height;
	    }
	}
	else
	{
	    DP(("ODW: calling PotionOpen\n"));
	    PotionOpen(obj);
	    DP(("\tOK\n"));
	    win = obj->wo_DrawerData->dd_DrawerWin;
	}
    }
    if (win)
    {
        wb->wb_BackWindow = win;
	wb->wb_Screen->RastPort.GelsInfo = wb->wb_GelsInfo;
	pr->pr_WindowPtr = (APTR)win;
	DP(("OpenDiskWin: pr=$%lx, pr->pr_WindowPtr=$%lx (%s)\n",pr, win, win->Title));
	InitEmbosing();
	obj->wo_DrawerOpen = 0; /* drawer is now fully open */
    }
    DP(("OpenDiskWin: BackWindow=%lx, dd->dd_DrawerWin=%lx\n\tobj->wo_DrawerData->dd_DrawerWin=%lx, IconWin=%lx\n", wb->wb_BackWindow, dd->dd_DrawerWin, obj->wo_DrawerData->dd_DrawerWin, obj->wo_IconWin));
    DP(("OpenDiskWin: exit, returning %lx\n", win));
    return(win);
}

int InitPotion(void)
{
struct WorkbenchBase *wb = getWbBase();
struct WBObject *obj;
struct NewDD *dd;
struct Window *win;
int result=FALSE;

    if (obj = wb->wb_RootObject = WBAllocWBObject(SystemWorkbenchName))
    {
        DP(("InitPotion: calling AllocDrawer\n"));
        if (dd = AllocDrawer( obj ))
        {
	    dd->dd_NewWindow.DetailPen = -1;
	    dd->dd_NewWindow.BlockPen = -1;
	    dd->dd_NewWindow.Type = WBENCHSCREEN;

            /* Get the sizes from the wb_Config structure... */
	    COPY_WBOX(&(dd->dd_NewWindow.LeftEdge),&(wb->wb_Config.wbc_LeftEdge));

            if (win = OpenDiskWin(obj))
            {
                DP(("init: root=%lx (%s)\n\tIconWin=%lx (%s)\n\tDrawerWin=%lx (%s)\n",obj, obj->wo_Name, obj->wo_IconWin,obj->wo_IconWin ? obj->wo_IconWin->Title : "NULL",dd->dd_DrawerWin, dd->dd_DrawerWin->Title));
                OnOffMenus(obj);
		result=1;
	    }
	}
    }
/*    if (!result) FatalNoMem(AN_WBInitPotionAllocDrawer);*/
    return(result);
}

void InitEmbosing()
{
    struct WorkbenchBase *wb = getWbBase();

    struct DrawInfo *dri;

    dri = GetScreenDrawInfo(wb->wb_Screen);
    wb->wb_shinePen = dri->dri_Pens[shinePen];
    wb->wb_shadowPen = dri->dri_Pens[shadowPen];
    FreeScreenDrawInfo(wb->wb_Screen, dri);

    wb->wb_EmboseBorderTop = 3;
    wb->wb_EmboseBorderBottom = 3;
    wb->wb_EmboseBorderLeft = 4;
    wb->wb_EmboseBorderRight = 4;
}
