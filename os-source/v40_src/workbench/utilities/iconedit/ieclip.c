/***************************/
/* REPLACEMENT FOR IEIFF.C */
/***************************/

/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/clipboard.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* application includes */
#include "ieclip.h"
#include "iemain.h"
#include "iemisc.h"
#include "ieio.h"
#include "iemenus.h"
#include "texttable.h"
#include "ieutils.h"
#include "iegads.h"

/*****************************************************************************/

extern struct Library *GfxBase;
extern struct Library *IntuitionBase;
extern struct Library *SysBase;
extern struct Library *AslBase;
extern struct Library *DOSBase;
extern struct Library *IFFParseBase;
extern struct Library *UtilityBase;
extern struct Library *DataTypesBase;
extern struct Library *IconBase;

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object * o, Msg msg);

/*****************************************************************************/

APTR newdtobject (STRPTR name, Tag tag1,...)
{
    return (NewDTObjectA (name, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG getdtattrs (Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) &data));
}

/*****************************************************************************/

ULONG setdtattrs (Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) &data));
}

/*****************************************************************************/

static UWORD __chip BrushImageData[] =
{
    0x0000, 0x0000, 0x0000, 0x0400, 0x0000, 0x0000, 0x0000, 0x0C00,
    0x0000, 0x0000, 0x0000, 0x0C00, 0x0067, 0xE7F0, 0x0000, 0x0C00,
    0x0066, 0x0600, 0x0000, 0x0C00, 0x0067, 0xC7C0, 0x0060, 0x0C00,
    0x0066, 0x0600, 0x0180, 0x0C00, 0x0066, 0x0600, 0x0600, 0x0C00,
    0x0066, 0x0600, 0x1800, 0x0C00, 0x0000, 0x0000, 0x6000, 0x0C00,
    0x0000, 0x0003, 0x8000, 0x0C00, 0x0000, 0x000E, 0x0000, 0x0C00,
    0x0000, 0x0038, 0x0000, 0x0C00, 0x0000, 0x00E0, 0x0000, 0x0C00,
    0x0000, 0x0380, 0x0000, 0x0C00, 0x0000, 0x0E00, 0x0000, 0x0C00,
    0x0000, 0x7800, 0xAA00, 0x0C00, 0x0003, 0xE0AA, 0x8000, 0x0C00,
    0x00FE, 0xAAA0, 0x0000, 0x0C00, 0x0000, 0x0000, 0x0000, 0x0C00,
    0x0000, 0x0000, 0x0000, 0x0C00, 0x7FFF, 0xFFFF, 0xFFFF, 0xFC00,

    0xFFFF, 0xFFFF, 0xFFFF, 0xF800, 0xD555, 0x5555, 0x5555, 0x5000,
    0xD555, 0x5555, 0x5555, 0x5000, 0xD510, 0x1005, 0x5555, 0x5000,
    0xD511, 0x5155, 0x5555, 0x5000, 0xD510, 0x1015, 0x5515, 0x5000,
    0xD511, 0x5155, 0x5455, 0x5000, 0xD511, 0x5155, 0x5155, 0x5000,
    0xD511, 0x5155, 0x4555, 0x5000, 0xD555, 0x5555, 0x1555, 0x5000,
    0xD555, 0x5554, 0x5555, 0x5000, 0xD555, 0x5551, 0x5555, 0x5000,
    0xD555, 0x5545, 0x5555, 0x5000, 0xD555, 0x5515, 0x5555, 0x5000,
    0xD555, 0x5455, 0x5555, 0x5000, 0xD555, 0x5155, 0x5555, 0x5000,
    0xD555, 0x7D55, 0xFF55, 0x5000, 0xD557, 0xF5FF, 0xD555, 0x5000,
    0xD5FF, 0xFFF5, 0x5555, 0x5000, 0xD555, 0x5555, 0x5555, 0x5000,
    0xD555, 0x5555, 0x5555, 0x5000, 0x8000, 0x0000, 0x0000, 0x0000,
};

static struct Image BrushImage =
{
    0, 0,			/* Upper left corner */
    54, 22, 2,			/* Width, Height, Depth */
    BrushImageData,		/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

static struct DiskObject BrushIcon =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 54, 23,		/* Left,Top,Width,Height */
	GADGIMAGE | GADGHCOMP,	/* Flags */
	RELVERIFY,		/* Activation Flags */
	BOOLGADGET,		/* Gadget Type */
	(APTR) & BrushImage,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	0,			/* Gadget ID */
	NULL,			/* User Data */
    },
    WBPROJECT,			/* Icon Type */
    (char *) "MultiView",	/* Default Tool */
    NULL,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    0				/* Stack Size */
};

/*****************************************************************************/

BOOL ShowClipFunc (WindowInfoPtr wi)
{
    struct IFFHandle *iff;
    struct DataType *dtn;
    ULONG gid = 0;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (wi->w_ClipUnit))
	{
	    InitIFFasClip (iff);
	    if (OpenIFF (iff, IFFF_READ) == 0)
	    {
		if (dtn = ObtainDataTypeA (DTST_CLIPBOARD, (APTR) iff, NULL))
		{
		    gid = dtn->dtn_Header->dth_GroupID;
		    ReleaseDataType (dtn);
		}
		CloseIFF (iff);
	    }
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}
	FreeIFF (iff);
    }

    if (gid == GID_PICTURE)
    {
	System (wi->w_ShowClip, NULL);
    }
    else
    {
	NotifyUser (MSG_IE_ERROR_NOT_ILBM, NULL);
    }
    return (FALSE);
}

/*****************************************************************************/

BOOL savePicture (WindowInfoPtr wi, DynamicImagePtr di, UWORD mode, STRPTR filename)
{
    struct ColorRegister *cmap, *acmap;
    struct BitMapHeader *bmhd;
    ULONG modeid = LORES_KEY;
    BOOL result = FALSE;
    struct BitMap *bm;
    WORD i, ncolors;
    LONG *cregs;
    Object *o;

    struct RastPort *rp = &di->di_RPort;
    ULONG x, y, mx = 0, my = 0;
    ULONG height;
    ULONG width;
    UWORD depth;
    UBYTE p;

    /* How many colors do we have */
    depth = (di->di_Image.Depth < 3) ? di->di_Image.Depth : 3;
    ncolors = 1 << depth;

    /* Get the real size of the image */
    width  = (ULONG) di->di_Image.Width;
    height = (ULONG) di->di_Image.Height;
    for (x = 0; x < width; x++)
    {
	for (y = 0; y < height; y++)
	{
	    if ((p = ReadPixel (rp, x, y)) > 0)
	    {
		mx = MAX (x, mx);
		my = MAX (y, my);
	    }
	}
    }
    width  = mx + 1;
    height = my + 1;

    /* Allocate the bitmap */
    if (bm = AllocBitMap (width, height, depth, BMF_CLEAR, NULL))
    {
	/* Should be DTA_GroupID, GID_PICTURE */
	if (o = newdtobject ((APTR) "IconEdit",
			     DTA_SourceType,		DTST_RAM,
			     DTA_GroupID,		GID_PICTURE,
			     PDTA_NumColors,		ncolors,
			     PDTA_BitMap,		bm,
			     PDTA_ModeID,		modeid,
			     TAG_DONE))
	{
	    /* Get a pointer to the data that we must prepare */
	    if ((getdtattrs (o,
			     PDTA_BitMapHeader,		&bmhd,
			     PDTA_ColorRegisters,	&acmap,
			     PDTA_CRegs,		&cregs,
			     TAG_DONE)) == 3)
	    {
		/* Prepare the bitmap header */
		bmhd->bmh_Width      = width;
		bmhd->bmh_Height     = height;
		bmhd->bmh_Depth      = depth;
		bmhd->bmh_PageWidth  = 320;
		bmhd->bmh_PageHeight = 200;

		/* Remember the points */
		cmap = acmap;

		/* How many colors do we have... */
		if (ncolors < 8)
		{
		    /* Make them the same as our screen */
		    GetRGB32 (wi->mysc->ViewPort.ColorMap, 0, ncolors, cregs);
		}
		else
		{
		    /* Get the first four colors */
		    GetRGB32 (wi->mysc->ViewPort.ColorMap, 0, 4, &cregs[0*3]);

		    /* Get the last four colors */
		    GetRGB32 (wi->mysc->ViewPort.ColorMap, wi->mysc->ViewPort.ColorMap->Count - 4, 4, &cregs[4*3]);
		}

		/* Set the colors */
		for (i = 0; i < ncolors; i++)
		{
		    /* Set the master color table */
		    cmap->red   = (UBYTE) (cregs[i * 3 + 0] >> 24);
		    cmap->green = (UBYTE) (cregs[i * 3 + 1] >> 24);
		    cmap->blue  = (UBYTE) (cregs[i * 3 + 2] >> 24);
		    cmap++;
		}

		/* Copy our data into it */
		BltBitMap (&di->di_BMap, 0, 0, bm, 0, 0, width, height, 0xC0, 0xFF, NULL);

		if (mode == 0)
		{
		    struct dtGeneral dtg;

		    /* Copy it to the clipboard */
		    dtg.MethodID = DTM_COPY;
		    DoDTMethodA (o, NULL, NULL, &dtg);

		    /* Show that we were successful */
		    result = TRUE;
		}
		else
		{
		    struct DiskObject *dob;
		    struct dtWrite dtw;
		    BPTR fh;

		    if (fh = Open (filename, MODE_NEWFILE))
		    {
			dtw.MethodID = DTM_WRITE;
			dtw.dtw_GInfo = NULL;
			dtw.dtw_FileHandle = fh;
			dtw.dtw_Mode = DTWM_IFF;
			dtw.dtw_AttrList = NULL;

			if (DoMethodA (o, &dtw))
			{
			    /* Get the an Icon for the file */
			    if (dob = GetDiskObject (filename))
			    {
				FreeDiskObject (dob);
			    }
			    else if (dob = GetDiskObject ("ENV:Sys/def_ilbm"))
			    {
				PutDiskObject (filename, dob);
				FreeDiskObject (dob);
			    }
			    else
			    {
				PutDiskObject (filename, &BrushIcon);
			    }

			    /* Show that we were successful */
			    result = TRUE;
			}

			Close (fh);
		    }
		}
	    }
	    DisposeDTObject (o);
	}
	else
	{
	    FreeBitMap (bm);
	}
    }

    return (result);
}

/*****************************************************************************/

BOOL CopyClipFunc (WindowInfoPtr wi)
{
    if (!savePicture (wi, &(wi->images[wi->currentwin]), 0, NULL))
    {
	NotifyUser (MSG_IE_ERROR_COPY_BRUSH, NULL);
    }
    return (FALSE);
}

/*****************************************************************************/

BOOL CutClipFunc (WindowInfoPtr wi)
{
    if (savePicture (wi, &(wi->images[wi->currentwin]), 0, NULL))
    {
	SPClear (wi->wi_sketch);
	wi->changed |= CH_MAJOR;
    }
    else
    {
	NotifyUser (MSG_IE_ERROR_COPY_BRUSH, NULL);
    }
    return (FALSE);
}

/*****************************************************************************/

BOOL loadPicture (WindowInfoPtr wi, DynamicImagePtr di, UWORD mode, STRPTR filename)
{
    struct BitMapHeader *bmhd;
    BOOL result = FALSE;
    struct gpLayout gpl;
    struct BitMap *bm;
    APTR name;
    ULONG stype;
    Object *o;

    if (mode == 0)
    {
	stype = DTST_CLIPBOARD;
	name = (APTR) wi->w_ClipUnit;
    }
    else
    {
	stype = DTST_FILE;
	name = (APTR) filename;
    }

    /* Get a pointer to the object */
    if (o = newdtobject (name,
			 DTA_SourceType, stype,
			 DTA_GroupID, GID_PICTURE,
			 PDTA_Screen, wi->mysc,
			 PDTA_NumSparse, MIN ((1 << wi->mysc->BitMap.Depth), 8),
			 PDTA_SparseTable, wi->w_ColorTable,
			 PDTA_Remap, wi->w_RemapImage,
			 TAG_DONE))
    {
	/* Tell the object to remap */
	gpl.MethodID = DTM_PROCLAYOUT;
	gpl.gpl_GInfo = NULL;
	gpl.gpl_Initial = 1;
	if (DoMethodA (o, &gpl))
	{
	    if ((getdtattrs (o,
			     PDTA_BitMapHeader, &bmhd,
			     PDTA_BitMap, &bm,
			     TAG_DONE) == 2) && bm)
	    {
		/* Save to undo buffer */
		SPSaveToUndo (wi->wi_sketch);

		/* Clear the image */
		SetAPen (&di->di_RPort, 0);
		RectFill (&di->di_RPort, 0, 0, ICON_WIDTH, ICON_HEIGHT);

		/* Copy our data into it */
		BltBitMap (bm, 0, 0,
			   &di->di_BMap, 0, 0,
			   MIN (bmhd->bmh_Width, di->di_Image.Width),
			   MIN (bmhd->bmh_Height, di->di_Image.Height),
			   0xC0, 0xFF, NULL);

		RefreshImages (wi);
		SPRefresh (wi->wi_sketch);
		wi->changed |= CH_MAJOR;
		wi->clippable = FALSE;

		result = TRUE;
	    }
	}
	DisposeDTObject (o);
    }
    return (result);
}

/*****************************************************************************/

BOOL PasteClipFunc (WindowInfoPtr wi)
{
    if (!loadPicture (wi, &(wi->images[wi->currentwin]), 0, NULL))
    {
	/* Clipboard doesn't contain graphics */
	NotifyUser (MSG_IE_ERROR_NOT_ILBM, NULL);
    }
    return (FALSE);
}

/*****************************************************************************/

BOOL IERequestFile (WindowInfoPtr wi, AppStringsID hail, AppStringsID ok, LONG funcflags)
{
    UBYTE path[255], name[50];
    struct TagItem tg[11];
    struct Window *wp;
    BOOL result;

    wp = wi->win;

    if (wi->Width == (-1))
    {
	wi->LeftEdge = wp->LeftEdge + 12;
	wi->TopEdge = wp->TopEdge + 12;
	wi->Width = 320;
	wi->Height = 175;
    }

    tg[0].ti_Tag = ASL_Hail;
    tg[0].ti_Data = (ULONG) GetString (hail);
    tg[1].ti_Tag = ASL_FuncFlags;
    tg[1].ti_Data = funcflags;
    tg[2].ti_Tag = ASL_OKText;
    tg[2].ti_Data = (ULONG) GetString (ok);
    tg[3].ti_Tag = ASL_CancelText;
    tg[3].ti_Data = (ULONG) GetString (MSG_IE_CANCEL_GAD);
    tg[4].ti_Tag = ASL_Window;
    tg[4].ti_Data = (ULONG) wp;
    tg[5].ti_Tag = ASL_LeftEdge;
    tg[5].ti_Data = (ULONG) wp->LeftEdge + wi->LeftEdge;
    tg[6].ti_Tag = ASL_TopEdge;
    tg[6].ti_Data = (ULONG) wp->TopEdge + wi->TopEdge;
    tg[7].ti_Tag = ASL_Width;
    tg[7].ti_Data = (ULONG) wi->Width;
    tg[8].ti_Tag = ASL_Height;
    tg[8].ti_Data = (ULONG) wi->Height;
    tg[9].ti_Tag = ASL_Pattern;
    tg[9].ti_Data = (ULONG) "~(#?.info)";
    tg[10].ti_Tag = TAG_DONE;

    result = FALSE;
    if (AslRequest (wi->w_FR[FR_CLIP], tg))
    {
	FixFileAndPath (wi->w_FR[FR_CLIP], NULL);
	strcpy (name, wi->w_FR[FR_CLIP]->rf_File);
	strcpy (path, wi->w_FR[FR_CLIP]->rf_Dir);
	strcpy (wi->w_Tmp, path);
	AddPart (wi->w_Tmp, name, 254);
	FixFileAndPath (wi->w_FR[FR_CLIP], wi->w_Tmp);
	result = TRUE;
    }

    wi->LeftEdge = wi->w_FR[FR_CLIP]->rf_LeftEdge - wp->LeftEdge;
    wi->TopEdge = wi->w_FR[FR_CLIP]->rf_TopEdge - wp->TopEdge;
    wi->Width = wi->w_FR[FR_CLIP]->rf_Width;
    wi->Height = wi->w_FR[FR_CLIP]->rf_Height;

    return (result);
}

/*****************************************************************************/

BOOL OpenClipFunc (WindowInfoPtr wi)
{
#if 1
    struct dtGeneral dtg;
    struct gpLayout gpl;
    Object *o;

    if (IERequestFile (wi, MSG_IE_LOADBRUSH_TITLE, MSG_IE_LOAD_GAD, NULL))
    {
	/* Read file and place in clipboard */
	if (o = newdtobject ((APTR) wi->w_Tmp,
			     DTA_SourceType, DTST_FILE,
			     DTA_GroupID, GID_PICTURE,
			     PDTA_Screen, wi->mysc,
			     PDTA_Remap, FALSE,
			     TAG_DONE))
	{
	    /* Tell the object to remap */
	    gpl.MethodID = DTM_PROCLAYOUT;
	    gpl.gpl_GInfo = NULL;
	    gpl.gpl_Initial = 1;
	    if (DoMethodA (o, &gpl))
	    {
		/* Copy it to the clipboard */
		dtg.MethodID = DTM_COPY;
		DoDTMethodA (o, NULL, NULL, &dtg);
	    }
	    DisposeDTObject (o);
	}
	else
	{
	    NotifyUser (MSG_IE_ERROR_READ_BRUSH, NULL);
	}
    }
#else
    ILBMPtr ir;

    if (IERequestFile (wi, MSG_IE_LOADBRUSH_TITLE, MSG_IE_LOAD_GAD, NULL))
    {
	if (ir = ReadILBM (NULL, wi->w_Tmp, NULL))
	{
	    if (!(PutILBM (wi->w_Clipper, ir)))
	    {
		NotifyUser (MSG_IE_ERROR_COPY_BRUSH, NULL);
	    }

	    FreeILBM (ir);
	}
	else
	{
	    NotifyUser (MSG_IE_ERROR_READ_BRUSH, NULL);
	}
    }
#endif
    return (FALSE);
}

/*****************************************************************************/

BOOL SaveClipFunc (WindowInfoPtr wi)
{
#if 1
    struct DiskObject *dob;
    struct gpLayout gpl;
    BOOL result = FALSE;
    struct dtWrite dtw;
    Object *o;
    BPTR fh;

    /* Get a pointer to the object */
    if (o = newdtobject ((APTR) wi->w_ClipUnit,
			 DTA_SourceType, DTST_CLIPBOARD,
			 DTA_GroupID, GID_PICTURE,
			 PDTA_Screen, wi->mysc,
			 PDTA_Remap, FALSE,
			 TAG_DONE))
    {
	/* Tell the object to remap */
	gpl.MethodID = DTM_PROCLAYOUT;
	gpl.gpl_GInfo = NULL;
	gpl.gpl_Initial = 1;
	if (DoMethodA (o, &gpl))
	{
	    if (IERequestFile (wi, MSG_IE_SAVEBRUSH_TITLE, MSG_IE_SAVE_GAD, FILF_SAVE))
	    {
		if (fh = Open (wi->w_Tmp, MODE_NEWFILE))
		{
		    dtw.MethodID = DTM_WRITE;
		    dtw.dtw_GInfo = NULL;
		    dtw.dtw_FileHandle = fh;
		    dtw.dtw_Mode = DTWM_IFF;
		    dtw.dtw_AttrList = NULL;

		    if (DoMethodA (o, &dtw))
		    {
			/* Get the an Icon for the file */
			if (dob = GetDiskObject (wi->w_Tmp))
			{
			    FreeDiskObject (dob);
			}
			else if (dob = GetDiskObject ("ENV:Sys/def_ilbm"))
			{
			    PutDiskObject (wi->w_Tmp, dob);
			    FreeDiskObject (dob);
			}
			else
			{
			    PutDiskObject (wi->w_Tmp, &BrushIcon);
			}
			result = TRUE;
		    }

		    Close (fh);
		}

		if (!result)
		{
		    NotifyUser (MSG_IE_ERROR_SAVE_BRUSH, NULL);
		}
	    }
	}
    }
    else
    {
	NotifyUser (MSG_IE_ERROR_NOT_ILBM, NULL);
    }
#else
    DynamicImagePtr di = &wi->images[wi->currentwin];
    struct IFFHandle *iff;
    ULONG type, id;
    ILBMPtr ir;

    iff = wi->w_Clipper;

    QueryClip (iff, (ULONG *) & type, (ULONG *) & id);
    if (type == ID_ILBM)
    {
	if (ir = GetILBM (iff))
	{
	    if (IERequestFile (wi, MSG_IE_SAVEBRUSH_TITLE, MSG_IE_SAVE_GAD, FILF_SAVE))
	    {
		if (!(SaveIFFBrush (wi, wi->w_Tmp, di)))
		{
		    NotifyUser (MSG_IE_ERROR_SAVE_BRUSH, NULL);
		}
	    }

	    FreeILBM (ir);
	}
	else
	{
	    NotifyUser (MSG_IE_ERROR_PASTE_BRUSH, NULL);
	}
    }
    else
    {
	NotifyUser (MSG_IE_ERROR_NOT_ILBM, NULL);
    }
#endif
    return (FALSE);
}

/*****************************************************************************/

BOOL LoadIFFImage (WindowInfoPtr wi, STRPTR name, USHORT cur, BOOL errs)
{
    BOOL result;

    if (!(result = loadPicture (wi, &(wi->images[wi->currentwin]), 1, name)))
    {
	NotifyUser (MSG_IE_ERROR_READ_BRUSH, NULL);
    }
    return (result);
}

/*****************************************************************************/

BOOL SaveIFFBrush (WindowInfoPtr wi, STRPTR name, DynamicImagePtr di)
{
    BOOL result;

    if (!(result = savePicture (wi, di, 1, name)))
    {
	NotifyUser (MSG_IE_ERROR_SAVE_BRUSH, NULL);
    }
    return (result);
}
