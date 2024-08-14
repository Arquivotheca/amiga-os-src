
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/cghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/icclass.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <utility/tagitem.h>
#include <utility/date.h>
#include <string.h>
#include <stdio.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

#include "pe_custom.h"
#include "calendargclass.h"


/*****************************************************************************/


#define G(o) ((struct Gadget *)(o))

struct localObjData
{
    struct ClockData lod_CData;	           /* Current calendar time */
    UWORD            lod_FirstWeekday;
    UWORD            lod_DaysInMonth;      /* Number of days in the month */
    UWORD            lod_WDay;             /* First day of the month */
    UWORD            lod_HDay;             /* Last highlighted day */
    UWORD	     lod_BackupDay;
    UWORD            lod_Width;            /* Width of each cell */
    UWORD            lod_Height;           /* Height of each cell */
    UWORD            lod_CalLefts[7];      /* Left offset of each cell */
    UWORD            lod_CalTops[6];       /* Top offset of each cell */
    UWORD            lod_FontHeight;
};

#define SECS_PER_DAY (86400)


ULONG __stdargs DispatchCalendarG(Class * cl, Object * o, Msg msg);
ULONG RenderCalendarG(Class * cl, Object * o, struct gpRender * msg);
ULONG SetCalendarGAttrs(Class * cl, Object * o, struct opSet * msg);
ULONG HandleCalendarG(Class * cl, Object * o, struct gpInput * msg);
UWORD GetDay(Class * cl, Object * o, WORD x, WORD y);

/* Class prototypes */
ULONG __stdargs DoMethod (Object * o, ULONG methodID,...);
ULONG __stdargs DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG __stdargs CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG __stdargs CM (Class * cl, Object * o, Msg msg);
ULONG __stdargs DM (Object * o, Msg msg);
ULONG __stdargs DSM (Class * cl, Object * o, Msg msg);
ULONG __stdargs SetSuperAttrs (Class * cl, Object * o, ULONG data,...);


/*****************************************************************************/


#define SUPERCLASSID "gadgetclass"


/*****************************************************************************/


UWORD GetDaysInMonth(EdDataPtr ed, UWORD n_month, UWORD n_year)
{
struct ClockData cd;
ULONG            d1,d2;

    cd.sec   = 0;
    cd.min   = 0;
    cd.wday  = 0;
    cd.hour  = 1;
    cd.mday  = 1;
    cd.month = n_month;
    cd.year  = n_year;

    d1 = Date2Amiga(&cd);

    if ((++cd.month) > 12)
    {
	cd.month = 1;
	cd.year++;
    }

    d2 = Date2Amiga (&cd);

    return ((UWORD) ((d2 - d1) / SECS_PER_DAY));
}


/*****************************************************************************/


Class *InitCalendarGClass(EdDataPtr ed)
{
ULONG hookEntry ();
Class           *cl;

    if (cl = MakeClass(NULL,SUPERCLASSID,NULL,sizeof(struct localObjData),0))
    {
	cl->cl_Dispatcher.h_Entry    = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = DispatchCalendarG;
	cl->cl_UserData              = (ULONG)ed;
    }

    return (cl);
}


/*****************************************************************************/


VOID FreeCalendarGClass(Class *cl)
{
EdDataPtr ed = (EdDataPtr)cl->cl_UserData;

    FreeClass(cl);
}


/*****************************************************************************/


ULONG __stdargs DispatchCalendarG(Class * cl, Object * o, Msg msg)
{
EdDataPtr            ed = (EdDataPtr)cl->cl_UserData;
struct localObjData *lod = INST_DATA (cl, o);
struct RastPort     *rp;
ULONG                result = 0;
Object              *newobj;
UWORD                i;

    switch (msg->MethodID)
    {
	case OM_NEW        : if (newobj = (Object *) DSM (cl, o, msg))
                             {
                                 lod = INST_DATA(cl,newobj);

			 	 lod->lod_FirstWeekday = 0;
                                 lod->lod_DaysInMonth  = 31;
                                 lod->lod_WDay         = 1;
                                 lod->lod_HDay         = 99;
                                 lod->lod_FontHeight   = ed->ed_Font->tf_YSize+1;

                                 SetCalendarGAttrs(cl,newobj,(APTR)msg);
                             }

                             result = (ULONG) newobj;
                             break;

	case OM_UPDATE     :
	case OM_SET        : DSM (cl, o, msg);
                             SetCalendarGAttrs (cl, o, (APTR)msg);

                             if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
                             {
                                 struct gpRender gpr = {NULL};

                                 gpr.MethodID   = GM_RENDER;
                                 gpr.gpr_GInfo  = ((struct opSet *) msg)->ops_GInfo;
                                 gpr.gpr_RPort  = rp;
                                 gpr.gpr_Redraw = GREDRAW_REDRAW;

                                 DM(o, &gpr);

                                 ReleaseGIRPort (rp);
                             }
                             break;

	case GM_RENDER     : lod->lod_Width  = G(o)->Width / 7;
                             lod->lod_Height = (G(o)->Height - lod->lod_FontHeight) / 6;

                             for (i = 0; i < 7; i++)
                                 lod->lod_CalLefts[i] = G(o)->LeftEdge + (i * lod->lod_Width);

                             for (i = 0; i < 6; i++)
                                 lod->lod_CalTops[i] = G(o)->TopEdge + lod->lod_FontHeight + (i * lod->lod_Height);

                             RenderCalendarG(cl,o,(APTR)msg);
                             break;

	case GM_HITTEST    : if (GetDay(cl,o,((struct gpHitTest *)msg)->gpht_Mouse.X,((struct gpHitTest *)msg)->gpht_Mouse.Y - lod->lod_FontHeight) != 99)
                                 result = GMR_GADGETHIT;
                             else
                                 result = 0;
                             break;

	case GM_GOACTIVE   : lod->lod_BackupDay = lod->lod_CData.mday;
	case GM_HANDLEINPUT: result = HandleCalendarG(cl,o,(APTR)msg);
                             break;

	case OM_DISPOSE    :
	default            : result = (ULONG) DSM (cl, o, msg);
                             break;
    }

    return (result);
}


/*****************************************************************************/


VOID CalDrawBB(EdDataPtr ed, struct RastPort *rp, WORD x, WORD y,
                                                  WORD w, WORD h, ULONG tags, ...)
{
    DrawBevelBoxA(rp,x,y,w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


ULONG SetCalendarGAttrs(Class * cl, Object * o, struct opSet * msg)
{
EdDataPtr            ed = (EdDataPtr)cl->cl_UserData;
struct localObjData *lod = INST_DATA (cl, o);
struct TagItem      *tags = msg->ops_AttrList;
struct TagItem      *tstate;
struct ClockData     cd;
struct TagItem      *tag;
ULONG                refresh = 0L;
ULONG                tidata;

    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
            case BOA_ClockData   : lod->lod_CData       = *((struct ClockData *) tidata);
                                   lod->lod_DaysInMonth = GetDaysInMonth(ed,lod->lod_CData.month, lod->lod_CData.year);

                                   /* First day of month falls on which weekday? */
                                   cd.sec   = 0;
                                   cd.min   = 0;
                                   cd.wday  = 0;
                                   cd.hour  = 1;
                                   cd.mday  = 1;
                                   cd.month = lod->lod_CData.month;
                                   cd.year  = lod->lod_CData.year;
                                   Amiga2Date ((Date2Amiga (&cd)), &cd);
                                   lod->lod_WDay = cd.wday;
                                   refresh = 1;
                                   break;

            case BOA_Day         : lod->lod_CData.mday = (UWORD) tidata;
                                   refresh = 1;
                                   break;

            case BOA_FirstWeekday: lod->lod_FirstWeekday = (UWORD) tidata;
                                   refresh = 1;
                                   break;
	}
    }

    return (refresh);
}


/*****************************************************************************/


UWORD GetDay(Class * cl, Object * o, WORD x, WORD y)
{
struct localObjData *lod = INST_DATA (cl, o);
WORD                 firstCellColumn;
UWORD                day;

    if ((x < 0) || (x >= lod->lod_Width * 7))
    {
        x = -1;
    }
    else
    {
        x /= lod->lod_Width;
    }

    if ((y < 0) || (y >= lod->lod_Height * 6))
    {
        y = -1;
    }
    else
    {
        y /= lod->lod_Height;
    }

    firstCellColumn = lod->lod_WDay - lod->lod_FirstWeekday;
    if (lod->lod_WDay < lod->lod_FirstWeekday)
        firstCellColumn += 7;

    if ((y == 0) && (x >= 0) && (x < firstCellColumn))
    {
        x = -1;
    }

    day = 99;
    if ((x >= 0) && (y >= 0))
    {
        day = (y * 7) + x - firstCellColumn + 1;

        if (day > lod->lod_DaysInMonth)
        {
            day = 99;
        }
    }

    return(day);
}


/*****************************************************************************/


ULONG HandleCalendarG(Class * cl, Object * o, struct gpInput * msg)
{
struct localObjData *lod = INST_DATA (cl, o);
struct InputEvent   *ie = msg->gpi_IEvent;
ULONG                result = 0;
UWORD                day;

    /* Gadget selected? */
    if (ie->ie_Class == IECLASS_RAWMOUSE)
    {
        if (ie->ie_Code == IECODE_RBUTTON)
            day = lod->lod_BackupDay;
        else
            day = GetDay(cl,o,msg->gpi_Mouse.X,msg->gpi_Mouse.Y - lod->lod_FontHeight);

	/* Do we have a valid, new cell? */
	if (day != 99)
	{
	    /* Show which one needs to be un-highlighted */
	    lod->lod_HDay = lod->lod_CData.mday;

            if (lod->lod_CData.mday != day)
            {
                /* Remember the currently selected day */
                lod->lod_CData.mday = day;

                /* Update the graphics to reflect the current state. */
                RenderCalendarG(cl, o, (struct gpRender *) msg);
            }
	}

        if (ie->ie_Code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
        {
            result = GMR_NOREUSE | GMR_VERIFY;

            /* Return the current day */
            *msg->gpi_Termination = (0xFFFFL & lod->lod_CData.mday);
        }
        else if (ie->ie_Code == IECODE_RBUTTON)
        {
            result = GMR_NOREUSE;
        }
    }

    return (result);
}


/*****************************************************************************/


ULONG RenderCalendarG(Class * cl, Object * o, struct gpRender * msg)
{
EdDataPtr            ed = (EdDataPtr)cl->cl_UserData;
struct localObjData *lod = INST_DATA(cl, o);
struct GadgetInfo   *gi = msg->gpr_GInfo;
struct DrawInfo     *di = gi->gi_DrInfo;
struct RastPort     *rp;
ULONG                result = 0;
BOOL                 refresh;
BOOL                 clear;
BOOL                 high;
UWORD                x, y;
UWORD                row, col;
LONG                 draw;
UWORD                i;
char                 dayText[4];
STRPTR               day;

    draw = (msg->MethodID == GM_RENDER) ? msg->gpr_Redraw : GREDRAW_UPDATE;

    if (rp = ObtainGIRPort(msg->gpr_GInfo))
    {
	SetDrMd(rp,JAM1);
	SetFont(rp,ed->ed_Font);

	if (draw == GREDRAW_REDRAW)
	{
	    SetAPen(rp,di->dri_Pens[BACKGROUNDPEN]);
	    RectFill(rp,G(o)->LeftEdge,
                        G(o)->TopEdge,
                        G(o)->LeftEdge + G(o)->Width - 1,
		        G(o)->TopEdge + G(o)->Height - 1);

	    SetAPen(rp,di->dri_Pens[TEXTPEN]);
	    for (i = 0; i < 7; i++)
	    {
                day = ed->ed_Days[(i+lod->lod_FirstWeekday) % 7];
		y = G(o)->TopEdge + rp->TxBaseline;
		x = lod->lod_CalLefts[i] + ((lod->lod_Width - TextLength(rp,day,strlen(day))) / 2);
		Move (rp, x, y);
		Text (rp, day,strlen(day));
	    }
	}

	row = 0;
        col = lod->lod_WDay - lod->lod_FirstWeekday;
        if (lod->lod_WDay < lod->lod_FirstWeekday)
            col += 7;

	for (i = 1; i <= lod->lod_DaysInMonth; i++)
	{
	    x = lod->lod_CalLefts[col];
	    y = lod->lod_CalTops[row];

	    high    = FALSE;
            clear   = FALSE;
            refresh = FALSE;

	    if (draw == GREDRAW_REDRAW)
	    {
		refresh = TRUE;
	    }

	    if (i == lod->lod_HDay)
	    {
		lod->lod_HDay = 99;
		SetAPen(rp,di->dri_Pens[BACKGROUNDPEN]);
		clear = TRUE;
	    }

	    if (i == lod->lod_CData.mday)
	    {
		SetAPen(rp,di->dri_Pens[FILLPEN]);
		clear = TRUE;
                high  = TRUE;
	    }

	    if (clear)
	    {
	        refresh = TRUE;
		RectFill(rp, (x + 2),
                             (y + 1),
			     (x + lod->lod_Width - 3),
                             (y + lod->lod_Height - 2));
	    }

	    if (refresh)
	    {
		if (high)
		{
		    SetAPen(rp,di->dri_Pens[FILLTEXTPEN]);
		    CalDrawBB(ed,rp,x,y,lod->lod_Width,lod->lod_Height,
                                    GTBB_Recessed, TRUE,
                                    GT_VisualInfo, ed->ed_VisualInfo,
                                    TAG_DONE);
                }
		else
		{
		    SetAPen(rp,di->dri_Pens[TEXTPEN]);
		    CalDrawBB(ed,rp,x,y,lod->lod_Width,lod->lod_Height,
                                    GT_VisualInfo, ed->ed_VisualInfo,
                                    TAG_DONE);
                }

		sprintf(dayText,"%ld",i);
		Move(rp,x+3,y+3+rp->TxBaseline);
		Text(rp,dayText,strlen(dayText));
	    }

	    if (++col > 6)
	    {
		col = 0;
		row++;
	    }
	}

	result = 1;
	ReleaseGIRPort (rp);
    }

    return (result);
}
