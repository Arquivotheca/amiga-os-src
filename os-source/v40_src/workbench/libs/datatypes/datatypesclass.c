/* datatypesclass.c
 *
 */

#include "datatypesbase.h"
#include <dos/stdio.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>

/*****************************************************************************/

#define	G(o)	((struct Gadget *)(o))

/*****************************************************************************/

struct DTObject
{
    struct Node		 dto_Node;

    VOID		*dto_Handle;
    /* (struct IFFHandle *) if SourceType is DTST_FILE and DataType is IFF */
    /* (BPTR)		if SourceType is DTST_FILE */
    /* Undefined and random for all other types */

    struct DataType	*dto_DTNode;
    struct IBox		 dto_Domain;		/* In units */
    struct IBox		 dto_Select;		/* In pixels??? */
    struct Rectangle	 dto_Anchor;
    LONG		 dto_Width;		/* Pixel width */
    LONG		 dto_Height;		/* Pixel height */
    LONG		 dto_NomWidth;		/* Nominal width in pixels */
    LONG		 dto_NomHeight;		/* Nominal height in pixels */

    STRPTR		 dto_Name;
    STRPTR		 dto_Author;
    STRPTR		 dto_Annotation;
    STRPTR		 dto_Copyright;
    STRPTR		 dto_Version;
    ULONG		 dto_ObjectID;
    APTR		 dto_UserData;
    struct FrameInfo	 dto_FrameInfo;
    ULONG		 dto_Flags;

    struct DTSpecialInfo dto_SpecialInfo;

    LONG		 dto_OTopVert;
    LONG		 dto_OTopHoriz;
    WORD		 dto_LastX;
    WORD		 dto_LastY;
    WORD		 dto_XOffset;
    WORD		 dto_YOffset;
    UWORD		 dto_LinePtrn;

    struct Task		*dto_PrinterProc;	/* Process to signal to abort printing */
    struct Task		*dto_LayoutProc;	/* Process to signal to abort layout */
};

/*****************************************************************************/

#define	DTO(o)	((struct DTObject *)(o))

#define	DTOF_CHANGED	(1<<0)

/*****************************************************************************/

/* Set attributes of an object */
static ULONG setDTAttrs (struct DataTypesLib * dtl, Class * cl, Object * o, struct opSet * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct TagItem *tags = msg->ops_AttrList;
    struct DTObject *dto = INST_DATA (cl, o);
    struct TagItem *tstate;
    struct TagItem *tag;
    ULONG refresh = 0;
    ULONG tidata;
    ULONG msize;

    LONG topv = si->si_TopVert,  visv = si->si_VisVert,  totv = si->si_TotVert;
    LONG toph = si->si_TopHoriz, vish = si->si_VisHoriz, toth = si->si_TotHoriz;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case DTA_SelectDomain:
		dto->dto_Select = *((struct IBox *) tidata);
		break;

	    case DTA_ObjName:
		if (dto->dto_Name)
		    FreeVec (dto->dto_Name);
		dto->dto_Name = NULL;
		if (tidata && (msize = strlen ((STRPTR) tidata)))
		    if (dto->dto_Name = AllocVec (msize + 1, MEMF_CLEAR))
			strcpy (dto->dto_Name, (STRPTR) tidata);
		break;

	    case DTA_ObjAuthor:
		if (dto->dto_Author)
		    FreeVec (dto->dto_Author);
		dto->dto_Author = NULL;
		if (tidata && (msize = strlen ((STRPTR) tidata)))
		    if (dto->dto_Author = AllocVec (msize + 1, MEMF_CLEAR))
			strcpy (dto->dto_Author, (STRPTR) tidata);
		break;

	    case DTA_ObjAnnotation:
		if (dto->dto_Annotation)
		    FreeVec (dto->dto_Annotation);
		dto->dto_Annotation = NULL;
		if (tidata && (msize = strlen ((STRPTR) tidata)))
		    if (dto->dto_Annotation = AllocVec (msize + 1, MEMF_CLEAR))
			strcpy (dto->dto_Annotation, (STRPTR) tidata);
		break;

	    case DTA_ObjCopyright:
		if (dto->dto_Copyright)
		    FreeVec (dto->dto_Copyright);
		dto->dto_Copyright = NULL;
		if (tidata && (msize = strlen ((STRPTR) tidata)))
		    if (dto->dto_Copyright = AllocVec (msize + 1, MEMF_CLEAR))
			strcpy (dto->dto_Copyright, (STRPTR) tidata);
		break;

	    case DTA_ObjVersion:
		if (dto->dto_Version)
		    FreeVec (dto->dto_Version);
		dto->dto_Version = NULL;
		if (tidata && (msize = strlen ((STRPTR) tidata)))
		    if (dto->dto_Version = AllocVec (msize + 1, MEMF_CLEAR))
			strcpy (dto->dto_Version, (STRPTR) tidata);
		break;

	    case DTA_ObjectID:
		dto->dto_ObjectID = tidata;
		break;

	    case DTA_UserData:
		dto->dto_UserData = (APTR) tidata;
		break;

	    case DTA_PrinterProc:
		dto->dto_PrinterProc = (struct Task *) tidata;
		break;

	    case DTA_LayoutProc:
		dto->dto_LayoutProc = (struct Task *) tidata;
		break;

	    case DTA_TopVert:
		topv = tidata;
		break;
	    case DTA_VisibleVert:
		visv = tidata;
		break;
	    case DTA_TotalVert:
		totv = tidata;
		break;
	    case DTA_VertUnit:
		si->si_VertUnit = tidata;
		break;
	    case DTA_NominalVert:
		dto->dto_NomHeight = tidata;
		break;

	    case DTA_TopHoriz:
		toph = tidata;
		break;
	    case DTA_VisibleHoriz:
		vish = tidata;
		break;
	    case DTA_TotalHoriz:
		toth = tidata;
		break;
	    case DTA_HorizUnit:
		si->si_HorizUnit = tidata;
		break;
	    case DTA_NominalHoriz:
		dto->dto_NomWidth = tidata;
		break;

	}
    }

    if (topv < 0)
	topv = 0;
    if (visv > totv)
	topv = 0;
    else if (topv + visv > totv)
	topv = (totv - visv);
    if (topv != si->si_TopVert)
    {
	si->si_TopVert = topv;
	refresh = 1;
    }
    if (visv != si->si_VisVert)
    {
	si->si_VisVert = visv;
	refresh = 1;
    }
    if (totv != si->si_TotVert)
    {
	si->si_TotVert = totv;
	refresh = 1;
    }

    if (toph < 0)
	toph = 0;
    if (vish > toth)
	toph = 0;
    else if (toph + vish > toth)
	toph = (toth - vish);
    if (toph != si->si_TopHoriz)
    {
	si->si_TopHoriz = toph;
	refresh = 1;
    }
    if (vish != si->si_VisHoriz)
    {
	si->si_VisHoriz = vish;
	refresh = 1;
    }
    if (toth != si->si_TotHoriz)
    {
	si->si_TotHoriz = toth;
	refresh = 1;
    }

    return (refresh);
}

/*****************************************************************************/

/* Get attributes of an object */
static ULONG getDTAttrs (struct DataTypesLib * dtl, Class * cl, Object * o, struct opGet * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct DTObject *dto = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	case DTA_Name:
	    *msg->opg_Storage = (ULONG) dto->dto_Node.ln_Name;
	    break;

	case DTA_SourceType:
	    *msg->opg_Storage = (ULONG) dto->dto_Node.ln_Type;
	    break;

	case DTA_DataType:
	    *msg->opg_Storage = (ULONG) dto->dto_DTNode;
	    break;

	case DTA_Handle:
	    *msg->opg_Storage = (ULONG) dto->dto_Handle;
	    break;

	case DTA_Domain:
	    *msg->opg_Storage = (ULONG) & dto->dto_Domain;
	    break;

	case DTA_ObjectID:
	    *msg->opg_Storage = (ULONG) dto->dto_ObjectID;
	    break;

	case DTA_UserData:
	    *msg->opg_Storage = (ULONG) dto->dto_UserData;
	    break;

	case DTA_ObjName:
	    *msg->opg_Storage = (ULONG) dto->dto_Name;
	    break;

	case DTA_ObjAuthor:
	    *msg->opg_Storage = (ULONG) dto->dto_Author;
	    break;

	case DTA_ObjAnnotation:
	    *msg->opg_Storage = (ULONG) dto->dto_Annotation;
	    break;

	case DTA_ObjCopyright:
	    *msg->opg_Storage = (ULONG) dto->dto_Copyright;
	    break;

	case DTA_ObjVersion:
	    *msg->opg_Storage = (ULONG) dto->dto_Version;
	    break;

	case DTA_FrameInfo:
	    *msg->opg_Storage = (ULONG) & dto->dto_FrameInfo;
	    break;

	case DTA_TopVert:
	    *msg->opg_Storage = si->si_TopVert;
	    break;

	case DTA_VisibleVert:
	    *msg->opg_Storage = si->si_VisVert;
	    break;

	case DTA_TotalVert:
	    *msg->opg_Storage = si->si_TotVert;
	    break;

	case DTA_TotalPVert:
	    *msg->opg_Storage = dto->dto_Height;
	    break;

	case DTA_VertUnit:
	    *msg->opg_Storage = si->si_VertUnit;
	    break;


	case DTA_TopHoriz:
	    *msg->opg_Storage = si->si_TopHoriz;
	    break;

	case DTA_VisibleHoriz:
	    *msg->opg_Storage = si->si_VisHoriz;
	    break;

	case DTA_TotalHoriz:
	    *msg->opg_Storage = si->si_TotHoriz;
	    break;

	case DTA_TotalPHoriz:
	    *msg->opg_Storage = dto->dto_Width;
	    break;

	case DTA_HorizUnit:
	    *msg->opg_Storage = si->si_HorizUnit;
	    break;

	case DTA_NominalHoriz:
	    *msg->opg_Storage = dto->dto_NomWidth;
	    break;

	case DTA_NominalVert:
	    *msg->opg_Storage = dto->dto_NomHeight;
	    break;

	case DTA_SelectDomain:
	    if (si->si_Flags & DTSIF_HIGHLIGHT)
		*msg->opg_Storage = (ULONG)&dto->dto_Select;
	    else
		*msg->opg_Storage = NULL;
	    break;

	case DTA_PrinterProc:
	    *msg->opg_Storage = (ULONG) dto->dto_PrinterProc;
	    break;

	case DTA_LayoutProc:
	    *msg->opg_Storage = (ULONG) dto->dto_LayoutProc;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (1L);
}

/*****************************************************************************/

static ULONG layoutMethod (struct DataTypesLib * dtl, Class * cl, Object * o, struct gpLayout *gpl)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct DTObject *dto = INST_DATA (cl, o);
    struct Gadget *g = G (o);
    struct IBox obox;
    struct Window *w;
    struct IBox *box;

    box = (struct IBox *) & g->LeftEdge;
    obox = *(&dto->dto_Domain);
    dto->dto_Domain = *box;

    /* Convert the relative coordinates to absolute */
    if (gpl->gpl_GInfo && (w = gpl->gpl_GInfo->gi_Window))
    {
	if (dto->dto_Domain.Left < 0)
	    dto->dto_Domain.Left += w->Width - 1;

	if (dto->dto_Domain.Top < 0)
	    dto->dto_Domain.Top += w->Height - 1;

	if (dto->dto_Domain.Width < 0)
	    dto->dto_Domain.Width += w->Width;

	if (dto->dto_Domain.Height < 0)
	    dto->dto_Domain.Height += w->Height;
    }

    dto->dto_Width = (si->si_HorizUnit) ? (si->si_TotHoriz * si->si_HorizUnit) : dto->dto_Domain.Width;
    dto->dto_Height = (si->si_VertUnit) ? (si->si_TotVert * si->si_VertUnit) : dto->dto_Domain.Height;

    return (1L);
}

/*****************************************************************************/

static ULONG goinactive (struct DataTypesLib * dtl, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTObject *dto = INST_DATA (cl, o);
    struct DTSpecialInfo *si;
    struct Gadget *g = G (o);
    struct dtSelect dts;
    struct RastPort *rp;
    struct IBox *b;
#if 0
    LONG px, py;
#endif

    si = (struct DTSpecialInfo *) g->SpecialInfo;

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    SetDrMd (rp, COMPLEMENT);
	    SetAPen (rp, ~0);
	    SetDrPt (rp, dto->dto_LinePtrn);
	    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);

#if 0
	    px = (si->si_HorizUnit) ? si->si_HorizUnit : 1;
	    py = (si->si_VertUnit) ? si->si_VertUnit : 1;
#endif

	    if (g->Flags & SELECTED)
	    {
		/* Do the DTM_SELECT method here */
		dts.MethodID = DTM_SELECT;
		dts.dts_GInfo = msg->gpi_GInfo;

#if 1
		CheckSortRect (&dto->dto_Anchor);
		dts.dts_Select = *(&dto->dto_Anchor);
#else

		dts.dts_Select.MinX = dto->dto_XOffset - dto->dto_Domain.Left;
		dts.dts_Select.MinY = dto->dto_YOffset - dto->dto_Domain.Top;
		dts.dts_Select.MaxX = dto->dto_LastX - dto->dto_Domain.Left;
		dts.dts_Select.MaxY = dto->dto_LastY - dto->dto_Domain.Top;
		CheckSortRect (&dts.dts_Select);
#endif

		b = &dto->dto_Select;
#if 1
		b->Left   = dto->dto_Anchor.MinX;
		b->Top    = dto->dto_Anchor.MinY;
		b->Width  = dto->dto_Anchor.MaxX - dto->dto_Anchor.MinX + 1;
		b->Height = dto->dto_Anchor.MaxY - dto->dto_Anchor.MinY + 1;
#else
		b->Left = dts.dts_Select.MinX / px;
		b->Top = dts.dts_Select.MinY / py;
		b->Width = (dts.dts_Select.MaxX - dts.dts_Select.MinX + 1) / px;
		b->Height = (dts.dts_Select.MaxY - dts.dts_Select.MinY + 1) / py;
#endif
		si->si_Flags |= DTSIF_HIGHLIGHT;
		DoMethodA (o, &dts);
	    }

	    /* Clear the Select pointer */
	    notifyAttrChanges (dtl, o, msg->gpi_GInfo, NULL,
				   GA_ID,	G (o)->GadgetID,
				   DTA_Busy,	FALSE,
				   TAG_DONE);

	}

	if (dto->dto_Flags & DTOF_CHANGED)
	{
	    notifyAttrChanges (dtl, o, msg->gpi_GInfo, NULL,
			       GA_ID,		G(o)->GadgetID,
			       DTA_Sync,	TRUE,
			       DTA_TopVert,	si->si_TopVert,
			       DTA_TopHoriz,	si->si_TopHoriz,
			       TAG_DONE);
	    dto->dto_Flags &= ~DTOF_CHANGED;
	}

	ReleaseGIRPort (rp);
    }

    si->si_Flags &= ~(DTSIF_DRAGGING | DTSIF_DRAGSELECT);
    g->Flags &= ~(SELECTED | ACTIVEGADGET);

    return (0);
}

/*****************************************************************************/

static VOID scrollMethod (struct DataTypesLib * dtl, Class * cl, Object * o, struct gpInput * msg, struct RastPort * rp, struct IBox * box)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct DTObject *dto = INST_DATA (cl, o);
    struct gpRender gpr;
    LONG px, py;
    LONG ox, oy;
    LONG topv;
    LONG toph;

    px = (si->si_HorizUnit) ? si->si_HorizUnit : 1;
    py = (si->si_VertUnit) ? si->si_VertUnit : 1;
    toph = ox = si->si_TopHoriz;
    topv = oy = si->si_TopVert;

    if (msg->gpi_Mouse.X < 0)
	toph += (msg->gpi_Mouse.X / px);
    else if (msg->gpi_Mouse.X >= box->Width)
	toph -= ((box->Width - msg->gpi_Mouse.X) / px) - 1;

    if (msg->gpi_Mouse.Y < 0)
	topv += (msg->gpi_Mouse.Y / py);
    else if (msg->gpi_Mouse.Y >= box->Height)
	topv -= ((box->Height - msg->gpi_Mouse.Y) / py) - 1;

    if (toph > si->si_TotHoriz - si->si_VisHoriz)
	toph = si->si_TotHoriz - si->si_VisHoriz;
    if (toph < 0)
	toph = 0;

    if (topv > si->si_TotVert - si->si_VisVert)
	topv = si->si_TotVert - si->si_VisVert;
    if (topv < 0)
	topv = 0;

    if ((ox != toph) || (oy != topv))
    {
	dto->dto_Flags |= DTOF_CHANGED;

	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    SetDrMd (rp, COMPLEMENT);
	    SetAPen (rp, ~0);
	    SetDrPt (rp, dto->dto_LinePtrn);
	    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);
	}

	/* Force a redraw */
	si->si_TopHoriz = toph;
	si->si_TopVert  = topv;
	gpr.MethodID   = GM_RENDER;
	gpr.gpr_GInfo  = msg->gpi_GInfo;
	gpr.gpr_RPort  = rp;
	gpr.gpr_Redraw = GREDRAW_UPDATE;
	DoMethodA (o, &gpr);

	if (si->si_Flags & DTSIF_DRAGSELECT)
	{
	    dto->dto_XOffset = dto->dto_Domain.Left + dto->dto_Anchor.MinX - si->si_TopHoriz;
	    if (dto->dto_XOffset < dto->dto_Domain.Left)
		dto->dto_XOffset = dto->dto_Domain.Left;
	    if (dto->dto_XOffset > dto->dto_Domain.Left + dto->dto_Domain.Width - 1)
		dto->dto_XOffset = dto->dto_Domain.Left + dto->dto_Domain.Width - 1;

	    dto->dto_YOffset = dto->dto_Domain.Top  + dto->dto_Anchor.MinY - si->si_TopVert;
	    if (dto->dto_YOffset < dto->dto_Domain.Top)
		dto->dto_YOffset = dto->dto_Domain.Top;
	    if (dto->dto_YOffset > dto->dto_Domain.Top + dto->dto_Domain.Height - 1)
		dto->dto_YOffset = dto->dto_Domain.Top + dto->dto_Domain.Height - 1;

	    SetDrPt (rp, dto->dto_LinePtrn);
	    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);
	    dto->dto_LinePtrn = rp->LinePtrn;
	}
    }
}

/*****************************************************************************/

static ULONG handleInput (struct DataTypesLib * dtl, Class * cl, Object * o, struct gpInput * msg)
{
    struct DTObject *dto = INST_DATA (cl, o);
    struct InputEvent *ie = msg->gpi_IEvent;
    ULONG retval = GMR_MEACTIVE;
    struct Gadget *g = G (o);
    struct DTSpecialInfo *si;
    struct RastPort *rp;
    struct IBox *box;
    struct IBox *sel;
    LONG px, py;

    if ((msg->MethodID == GM_GOACTIVE) && (ie == NULL))
	return (GMR_NOREUSE);

    si = (struct DTSpecialInfo *) g->SpecialInfo;
    if (si->si_Flags & DTSIF_LAYOUT)
	return (GMR_NOREUSE);

    if (!AttemptSemaphoreShared (&(si->si_Lock)))
	return (GMR_NOREUSE);

    /* Get a pointer to the rastport */
    if (rp = ObtainGIRPort (msg->gpi_GInfo))
    {
	SetDrMd (rp, COMPLEMENT);
	SetAPen (rp, ~0);

	GetAttr (DTA_Domain, o, (ULONG *) & box);

	px = (si->si_HorizUnit) ? si->si_HorizUnit : 1;
	py = (si->si_VertUnit) ? si->si_VertUnit : 1;

	/* Determine what to do... */
	switch (ie->ie_Class)
	{
		/* Mouse button or movement */
	    case IECLASS_RAWMOUSE:
		switch (ie->ie_Code)
		{
			/* Select button pressed */
		    case SELECTDOWN:
			g->Flags |= SELECTED;

			dto->dto_OTopHoriz = si->si_TopHoriz;
			dto->dto_OTopVert = si->si_TopVert;

			if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* Clear selection */
			    if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
			    {
				si->si_Flags &= ~DTSIF_HIGHLIGHT;
				DoMethod (o, DTM_CLEARSELECTED, msg->gpi_GInfo);
			    }

			    si->si_Flags |= DTSIF_DRAGGING | DTSIF_DRAGSELECT;

			    dto->dto_LastX = msg->gpi_Mouse.X;
			    dto->dto_LastY = msg->gpi_Mouse.Y;

			    /* Range check X */
			    if (dto->dto_LastX > dto->dto_Width - 1)
				dto->dto_LastX = dto->dto_Width - 1;
			    dto->dto_Anchor.MinX = (si->si_TopHoriz * px) + dto->dto_LastX;
			    dto->dto_LastX = (dto->dto_LastX / px) * px;
			    dto->dto_XOffset = dto->dto_LastX = g->LeftEdge + dto->dto_LastX;

			    /* Range check Y */
			    if (dto->dto_LastY > dto->dto_Height - 1)
				dto->dto_LastY = dto->dto_Height - 1;
			    dto->dto_Anchor.MinY = (si->si_TopVert * py) + dto->dto_LastY;
			    dto->dto_LastY = (dto->dto_LastY / py) * py;
			    dto->dto_YOffset = dto->dto_LastY = g->TopEdge + dto->dto_LastY;

			    dto->dto_LinePtrn = 0xFF00;
			    SetDrPt (rp, dto->dto_LinePtrn);
			    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);
			    dto->dto_LinePtrn = rp->LinePtrn;
			}
			break;

			/* Select button released */
		    case SELECTUP:
			retval = (GMR_NOREUSE | GMR_VERIFY);
			break;

			/* Menu (ABORT) button pressed */
		    case MENUDOWN:
			si->si_TopHoriz = dto->dto_OTopHoriz;
			si->si_TopVert = dto->dto_OTopVert;
			g->Flags &= ~SELECTED;
			retval = GMR_NOREUSE;
			break;

		    default:
			if (si->si_Flags & DTSIF_DRAGSELECT)
			{
			    /* erase old */
			    SetDrPt (rp, dto->dto_LinePtrn);
			    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);

			    /* Get the values */
			    dto->dto_LastX = msg->gpi_Mouse.X;
			    dto->dto_LastY = msg->gpi_Mouse.Y;

			    /* Range check X */
			    if (dto->dto_LastX < 0)
				dto->dto_LastX = 0;
			    else if (dto->dto_LastX > box->Width)
				dto->dto_LastX = box->Width;
			    if (dto->dto_LastX > dto->dto_Width)
				dto->dto_LastX = dto->dto_Width;
			    dto->dto_LastX = ((dto->dto_LastX / px) * px) - 1;
			    dto->dto_Anchor.MaxX = (si->si_TopHoriz * px) + dto->dto_LastX;
			    dto->dto_LastX += g->LeftEdge;

			    /* Range check Y */
			    if (dto->dto_LastY < 0)
				dto->dto_LastY = 0;
			    else if (dto->dto_LastY > box->Height)
				dto->dto_LastY = box->Height;
			    if (dto->dto_LastY > dto->dto_Height)
				dto->dto_LastY = dto->dto_Height;
			    dto->dto_LastY = ((dto->dto_LastY / py) * py) - 1;
			    dto->dto_Anchor.MaxY = (si->si_TopVert * py) + dto->dto_LastY;
			    dto->dto_LastY += g->TopEdge;

			    /* draw new */
			    DrawBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);
			    dto->dto_LinePtrn = rp->LinePtrn;
			}
			break;
		}
		break;

	    case IECLASS_TIMER:
		scrollMethod (dtl, cl, o, msg, rp, box);
		if (si->si_Flags & DTSIF_DRAGSELECT)
		{
		    SetDrPt (rp, dto->dto_LinePtrn);
		    AnimateDragSelectBox (dtl, rp, dto->dto_XOffset, dto->dto_YOffset, dto->dto_LastX, dto->dto_LastY);
		    dto->dto_LinePtrn = rp->LinePtrn;
		}
		break;
	}

	ReleaseGIRPort (rp);
    }

    ReleaseSemaphore (&si->si_Lock);

    return (retval);
}

/*****************************************************************************/

static ULONG ASM dispatchDTClass (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct DataTypesLib *dtl = (struct DataTypesLib *) cl->cl_UserData;
    struct DTObject *dto = INST_DATA (cl, o);
    struct DataTypeHeader *dth;
    struct IFFHandle *iff;
    Object *newobj;
    ULONG retval;
    UWORD dtype;
    APTR handle;
    ULONG error;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (newobj = (Object *) DoSuperMethodA (cl, o, msg))
	    {
		struct TagItem *attrs = ((struct opSet *) msg)->ops_AttrList;
		BOOL success = FALSE;
		struct TagItem *tag;
		ULONG msize;

		dto = INST_DATA (cl, newobj);
		G (newobj)->Flags |= GFLG_RELSPECIAL;
		G (newobj)->SpecialInfo = &dto->dto_SpecialInfo;
		InitSemaphore (&(dto->dto_SpecialInfo.si_Lock));

		dto->dto_Node.ln_Type = (UWORD) GetTagData (DTA_SourceType, DTST_FILE, attrs);
		handle = (APTR) GetTagData (DTA_Handle, NULL, attrs);

		if (tag = FindTagItem (DTA_Name, attrs))
		{
		    msize = strlen ((char *) tag->ti_Data) + 1;
		    if (dto->dto_Node.ln_Name = AllocVec (msize, MEMF_CLEAR))
		    {
			strcpy (dto->dto_Node.ln_Name, (char *) tag->ti_Data);

			if (dto->dto_DTNode = (struct DataType *) GetTagData (DTA_DataType, NULL, attrs))
			{
			    dth = dto->dto_DTNode->dtn_Header;
			    dtype = dth->dth_Flags & DTF_TYPE_MASK;

			    switch (dto->dto_Node.ln_Type)
			    {
				case DTST_FILE:
				    switch (dtype)
				    {
					/* IFFHandle */
					case DTF_IFF:
					    if (dto->dto_Handle = iff = AllocIFF ())
					    {
						if (iff->iff_Stream = Open (dto->dto_Node.ln_Name, MODE_OLDFILE))
						{
						    InitIFFasDOS (iff);
						    if (OpenIFF (iff, IFFF_READ) == 0)
							success = TRUE;
						}
					    }
					    UnLock ((BPTR) handle);
					    break;

					/* Directory or disk */
					case DTF_MISC:
					    dto->dto_Handle = handle;
					    success = TRUE;
					    break;

					/* All others */
					default:
					    if (dto->dto_Handle = (VOID *) Open (dto->dto_Node.ln_Name, MODE_OLDFILE))
						success = TRUE;
					    UnLock ((BPTR) handle);
					    break;
				    }
				    break;

				case DTST_CLIPBOARD:
				    if (dto->dto_Handle = iff = (struct IFFHandle *) handle)
					success = TRUE;
				    break;
			    }
			}
			else
			    success = TRUE;
		    }
		    else
			SetIoErr (ERROR_NO_FREE_STORE);
		}
		else
		    SetIoErr (ERROR_REQUIRED_ARG_MISSING);

		if (success)
		    setDTAttrs (dtl, cl, newobj, (struct opSet *) msg);
		else
		{
		    CoerceMethod (cl, newobj, OM_DISPOSE);
		    newobj = NULL;
		}
	    }
	    else
	    {
		SetIoErr (ERROR_NO_FREE_STORE);
	    }

	    retval = (ULONG) newobj;
	    break;

	case GM_HITTEST:
	    retval = GMR_GADGETHIT;
	    break;

	case GM_GOACTIVE:
	    dto->dto_Width  = (si->si_HorizUnit) ? (si->si_TotHoriz * si->si_HorizUnit) : dto->dto_Domain.Width;
	    dto->dto_Height = (si->si_VertUnit)  ? (si->si_TotVert  * si->si_VertUnit)  : dto->dto_Domain.Height;
	case GM_HANDLEINPUT:
	    retval = handleInput (dtl, cl, o, (struct gpInput *) msg);
	    break;

	case GM_GOINACTIVE:
	    retval = goinactive (dtl, cl, o, (struct gpInput *) msg);
	    break;

	case DTM_PROCLAYOUT:
	case GM_LAYOUT:
	    retval = layoutMethod (dtl, cl, o, (struct gpLayout *) msg);

	case DTM_FRAMEBOX:
	    if (dto->dto_Node.ln_Type == DTST_CLIPBOARD)
	    {
		if (iff = (struct IFFHandle *) dto->dto_Handle)
		{
		    CloseClipIFF (dtl, iff);
		    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
		    FreeIFF (iff);
		    dto->dto_Handle = NULL;
		}
	    }
	    break;

	case OM_GET:
	    retval = getDTAttrs (dtl, cl, o, (struct opGet *) msg);
	    break;

	case OM_UPDATE:
	case OM_SET:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    retval += setDTAttrs (dtl, cl, o, (struct opSet *) msg);
	    break;

	case DTM_ABORTPRINT:
	    Forbid();
	    if (dto->dto_PrinterProc)
		Signal (dto->dto_PrinterProc, SIGBREAKF_CTRL_C);
	    Permit();
	    break;

	case OM_DISPOSE:
	    dto = INST_DATA (cl, o);

	    /* Cache the error message */
	    error = IoErr ();

	    if (dto->dto_DTNode && dto->dto_Handle)
	    {
		dth = dto->dto_DTNode->dtn_Header;
		dtype = dth->dth_Flags & DTF_TYPE_MASK;
		switch (dto->dto_Node.ln_Type)
		{
		    case DTST_FILE:
			switch (dtype)
			{
			    case DTF_IFF:
				iff = (struct IFFHandle *) dto->dto_Handle;
				if (iff->iff_Stream)
				{
				    CloseIFF (iff);
				    Close (iff->iff_Stream);
				}
				FreeIFF (iff);
				break;

			    case DTF_MISC:
				UnLock ((BPTR) dto->dto_Handle);
				break;

			    default:
				Close ((BPTR) dto->dto_Handle);
				break;
			}
			break;

		    case DTST_CLIPBOARD:
			if (iff = (struct IFFHandle *) dto->dto_Handle)
			{
			    CloseClipIFF (dtl, iff);
			    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
			    FreeIFF (iff);
			}
			break;
		}
	    }

	    if (dto->dto_Node.ln_Name)
		FreeVec (dto->dto_Node.ln_Name);
	    if (dto->dto_Name)
		FreeVec (dto->dto_Name);
	    if (dto->dto_Author)
		FreeVec (dto->dto_Author);
	    if (dto->dto_Annotation)
		FreeVec (dto->dto_Annotation);
	    if (dto->dto_Copyright)
		FreeVec (dto->dto_Copyright);
	    if (dto->dto_Version)
		FreeVec (dto->dto_Version);

	    /* Restore the error message */
	    if (error == ERROR_OBJECT_NOT_FOUND)
		error = DTERROR_COULDNT_OPEN;
	    SetIoErr (error);

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

Class * ASM initDTClass (REG (a6) struct DataTypesLib * dtl)
{
    Class *cl;

    if (cl = MakeClass (DATATYPESCLASS, GADGETCLASS, NULL, sizeof (struct DTObject), 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)()) dispatchDTClass;
	cl->cl_UserData           = (ULONG) dtl;
	AddClass (cl);
    }

    return (cl);
}
