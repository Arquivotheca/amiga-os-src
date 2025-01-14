/* dispatch.c
 * Copyright (C) 1993 Commodore-Amiga, Inc.
 * DataType dispatch routine for MicroSoft Windows icons
 * Written by David N. Junod
 *
 */

#include "classbase.h"

/*****************************************************************************/

struct RGBQuad
{
    UBYTE rgbBlue;
    UBYTE rgbGreen;
    UBYTE rgbRed;
    UBYTE rgbReserved;
};

/*****************************************************************************/

static BOOL GetData (struct ClassBase *cb, Class *cl, Object *o, struct TagItem *attrs)
{
    struct ColorRegister *cmap;
    struct BitMapHeader *bmhd;
    UBYTE buffer[800], color;
    struct RGBQuad *rgbx;
    struct RastPort rp;
    struct BitMap *bm;
    STRPTR title;
    LONG i, x, y;
    LONG *cregs;
    char *bptr;
    BPTR fh;

    /* Get the title of the icon */
    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    /* Get the required attributes */
    if ((GetDTAttrs (o, DTA_Handle, &fh, PDTA_BitMapHeader, &bmhd, TAG_DONE) == 2) && fh)
    {
	/* Make sure we are at the beginning of the file */
	Seek (fh, 0L, OFFSET_BEGINNING);

	/* Read in the data */
	if (Read (fh, buffer, sizeof (buffer)) == 766)
	{
	    /* Set the dimensions of the icon */
	    bmhd->bmh_Width  = bmhd->bmh_PageWidth  = 32;
	    bmhd->bmh_Height = bmhd->bmh_PageHeight = 32;
	    bmhd->bmh_Depth  = 4;

	    /* Tell the picture class that we have 16 colors */
	    SetDTAttrs (o, NULL, NULL, PDTA_NumColors, 16, TAG_DONE);

	    /* Get the colormap information */
	    GetDTAttrs (o, PDTA_ColorRegisters, (ULONG)&cmap, PDTA_CRegs, &cregs,TAG_DONE);

	    /* Make sure we got the color information */
	    if (cmap && cregs)
	    {
		/* Convert the colors to Amiga style */
		for (i = 0, rgbx = (struct RGBQuad *) &buffer[62]; i < 16; i++, rgbx++)
		{
		    /* Set the master color table */
		    cmap->red   = rgbx->rgbRed;
		    cmap->green = rgbx->rgbGreen;
		    cmap->blue  = rgbx->rgbBlue;
		    cmap++;

		    /* Set the color table used for remapping */
		    cregs[i * 3 + 0] = rgbx->rgbRed   << 24;
		    cregs[i * 3 + 1] = rgbx->rgbGreen << 24;
		    cregs[i * 3 + 2] = rgbx->rgbBlue  << 24;
		}

		/* Convert the bitmap information */
		if (bm = AllocBitMap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_CLEAR, NULL))
		{
		    /* Initialize the rastport */
		    InitRastPort (&rp);
		    rp.BitMap = bm;

		    /* Step through the lines (they are stored upside down) */
		    for (i = 0, y = 31, bptr = &buffer[126]; i < 32; i++, y--)
		    {
			/* Step through the columns */
			for (x = 0; x < 32; bptr++)
			{
			    color = (*bptr & 0xF0) >> 4;
			    SetAPen (&rp, color);
			    WritePixel (&rp, x, y);
			    x++;

			    color = (*bptr & 0xF);
			    SetAPen (&rp, color);
			    WritePixel (&rp, x, y);
			    x++;
			}
		    }

		    /* Tell mother our current status */
		    SetDTAttrs (o, NULL, NULL,
				DTA_ObjName,		title,
				DTA_NominalHoriz,	32,
				DTA_NominalVert,	32,
				PDTA_BitMap,		bm,
				PDTA_ModeID,		LORES_KEY,
				TAG_DONE);

		    /* Indicate success */
		    return TRUE;
		}
	    }
	    SetIoErr (ERROR_NO_FREE_STORE);
	}
	else
	    SetIoErr (DTERROR_INVALID_DATA);
    }
    else
	SetIoErr (ERROR_REQUIRED_ARG_MISSING);

    return (FALSE);
}

/*****************************************************************************/

static ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		if (!GetData (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList))
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    return NULL;
		}
	    }
	    return retval;

	    /* Let the superclass handle everything else */
	default:
	    return (ULONG) DoSuperMethodA (cl, o, msg);
    }
}

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    if (cl = MakeClass (DTCLASS, PICTUREDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)())Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}
