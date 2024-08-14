/* clipboard.c
 *
 */

#include "wbpattern.h"

#define	DR(x)	;

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object * o, Msg msg);

/*****************************************************************************/

EdStatus EraseFunc (EdDataPtr ed)
{

    /* Store the image before erasing it */
    setgadgetattrs (ed, ed->ed_GSketch,
		    SPA_Store, TRUE,
		    SPA_Erase, TRUE,
		    TAG_DONE);

    /* Update the sketchpad graphic */
    UpdateSketch (ed);

    return (ES_NORMAL);
}

/*****************************************************************************/

EdStatus CopyFunc (EdDataPtr ed)
{
    EdStatus result = ES_NOCLIPBOARD;
    struct ColorRegister *cmap;
    struct BitMapHeader *bmhd;
    ULONG modeid = LORES_KEY;
    struct dtGeneral dtg;
    struct BitMap *bm;
    WORD i, ncolors;
    LONG *cregs;
    Object *o;

    /* How many colors do we have */
    ncolors = 1 << ed->ed_Depth;

    /* Allocate the bitmap */
    if (bm = AllocBitMap (ed->ed_Width, ed->ed_Height, ed->ed_Depth, BMF_CLEAR, NULL))
    {
	/* Should be DTA_GroupID, GID_PICTURE */
	if (o = newdtobject (ed, PORT_NAME,
			     DTA_SourceType, DTST_RAM,
			     DTA_GroupID, GID_PICTURE,
			     PDTA_NumColors, ncolors,
			     PDTA_BitMap, bm,
			     PDTA_ModeID, modeid,
			     TAG_DONE))
	{
	    /* Get a pointer to the data that we must prepare */
	    if ((getdtattrs (ed, o,
			     PDTA_BitMapHeader, &bmhd,
			     PDTA_ColorRegisters, (ULONG) & cmap,
			     PDTA_CRegs, &cregs,
			     TAG_DONE)) == 3)
	    {
		/* Prepare the bitmap header */
		bmhd->bmh_Width = ed->ed_Width;
		bmhd->bmh_Height = ed->ed_Height;
		bmhd->bmh_Depth = ed->ed_Depth;
		bmhd->bmh_PageWidth = 320;
		bmhd->bmh_PageHeight = 200;

		/* Make them the same as our screen */
		GetRGB32 (ed->ed_Screen->ViewPort.ColorMap, 0, ncolors, cregs);

		/* Set the colors */
		DR (kprintf ("wbpattern save\n"));
		for (i = 0; i < ncolors; i++)
		{
		    /* Set the master color table */
		    cmap->red = (UBYTE) (cregs[i * 3 + 0] >> 24);
		    cmap->green = (UBYTE) (cregs[i * 3 + 1] >> 24);
		    cmap->blue = (UBYTE) (cregs[i * 3 + 2] >> 24);
		    DR (kprintf ("%2ld) r=0x%02lx g=0x%02lx b=0x%02lx : r=0x%08lx g=0x%08lx b=0x%08lx\n",
				 (LONG) i,
				 (ULONG) cmap->red, (ULONG) cmap->green, (ULONG) cmap->blue,
				 cregs[i * 3 + 0], cregs[i * 3 + 1], cregs[i * 3 + 2]));
		    cmap++;
		}
		DR (kprintf ("\n"));

		/* Copy our data into it */
		bltbm (&ed->ed_PrefsWork.ep_BMap, ed->ed_Which, 0,
		       bm, 0, 0, ed->ed_Width, ed->ed_Height,
		       0xC0, 0xFF, NULL, ed->ed_GfxBase);

		/* Copy it to the clipboard */
		dtg.MethodID = DTM_COPY;
		DoDTMethodA (o, NULL, NULL, &dtg);

		/* Show that we were successful */
		result = ES_NORMAL;
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

EdStatus CutFunc (EdDataPtr ed)
{
    EdStatus result;

    /* Copy the image to the clipboard */
    if ((result = CopyFunc (ed)) == ES_NORMAL)
    {
	/* Erase the image */
	result = EraseFunc (ed);
    }

    return (result);
}

/*****************************************************************************/

#define	PATHNAMESIZE	300

/*****************************************************************************/

BOOL SelectImage (EdDataPtr ed, ULONG tag1,...)
{
    if (!ed->ed_ImageReq)
	ed->ed_ImageReq = AllocAslRequest (ASL_FileRequest, NULL);

    return (AslRequest (ed->ed_ImageReq, (struct TagItem *) & tag1));
}

/*****************************************************************************/

EdStatus PasteFunc (EdDataPtr ed, EdCommand ec)
{
    EdStatus result = ES_NOCLIPBOARD;
    struct BitMapHeader *bmhd;
    char path[PATHNAMESIZE];
    struct gpLayout gpl;
    struct BitMap *bm;
    struct Hook hook;
    ULONG stype;
    BPTR lock;
    APTR name;
    Object *o;

    if (ec == EC_PASTE)
    {
	name = (APTR) ed->ed_ClipUnit;
	stype = DTST_CLIPBOARD;
    }
    else
    {
	strcpy (path, ed->ed_ImageBuf);
	*PathPart (path) = 0;
	hook.h_Entry = IntuiHook;

	if (SelectImage (ed,
			  ASLFR_TitleText,	GetString (&ed->ed_LocaleInfo, MSG_WBP_REQ_LOAD_IMAGE),
			  ASLFR_Window,		ed->ed_Window,
			  ASLFR_InitialDrawer,	path,
			  ASLFR_InitialFile,	FilePart (ed->ed_ImageBuf),
			  ASLFR_IntuiMsgFunc,	&hook,
			  ASLFR_SleepWindow,	TRUE,
			  ASLFR_RejectIcons,	TRUE,
			  ASLFR_FilterFunc,	&ed->ed_FilterHook,
			  TAG_DONE))
	{
	    strcpy (path, ed->ed_ImageReq->fr_Drawer);
	    AddPart (path, ed->ed_ImageReq->fr_File, sizeof (path));
	    if (lock = Lock (path, ACCESS_READ))
	    {
		if (NameFromLock (lock, path, sizeof (path)))
		{
		    strcpy (ed->ed_ImageBuf, path);
		    name = (APTR) ed->ed_ImageBuf;
		    stype = DTST_FILE;
		}
		UnLock (lock);
	    }
	}
	else
	{
	    result = ES_NORMAL;
	}
    }

    if (result != ES_NORMAL)
    {
	/* Get a pointer to the object */
	if (o = newdtobject (ed, name,
			     DTA_SourceType, stype,
			     DTA_GroupID, GID_PICTURE,
			     PDTA_Screen, ed->ed_Screen,
			     PDTA_NumSparse, MIN ((1 << ed->ed_Screen->BitMap.Depth), 8),
			     PDTA_SparseTable, ed->ed_ColorTable,
			     PDTA_Remap, ed->ed_RemapImage,
			     TAG_DONE))
	{
	    /* Tell the object to remap */
	    gpl.MethodID = DTM_PROCLAYOUT;
	    gpl.gpl_GInfo = NULL;
	    gpl.gpl_Initial = 1;
	    if (DoMethodA (o, &gpl))
	    {
		if ((getdtattrs (ed, o,
				 PDTA_BitMapHeader, &bmhd,
				 PDTA_BitMap, &bm,
				 TAG_DONE) == 2) && bm)
		{
		    result = ES_NORMAL;

		    /* Store the current image */
		    Store (ed);

		    /* Clear the old image */
		    SetPrefsRast (ed, 0);

		    /* Copy in the new */
		    bltbm (bm, 0, 0,
			   &ed->ed_PrefsWork.ep_BMap, ed->ed_Which, 0,
			   MIN (bmhd->bmh_Width, ed->ed_Width),
			   MIN (bmhd->bmh_Height, ed->ed_Height),
			   0xC0, 0xFF, NULL, ed->ed_GfxBase);

		    /* Update the sketchpad graphic */
		    UpdateSketch (ed);
		}
	    }
	    DisposeDTObject (o);
	}
	else
	{
	    /* Clipboard doesn't contain graphics */
	    if (ec == EC_PASTE)
		result = ES_NOGRAPHIC;
	    else
		result = ES_NOT_A_PICTURE;
	}
    }
    return (result);
}
