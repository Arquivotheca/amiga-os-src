/* getilbm.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	DB(x)	;
#define	DC(x)	;
#define	DR(x)	;
#define	DM(x)	;

/*****************************************************************************/

/* Properties that we can deal with */
LONG ilbmprops[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    ID_ILBM, ID_CAMG,
    ID_ILBM, ID_GRAB,
    ID_ILBM, ID_NAME,
};

/*****************************************************************************/

BOOL ASM GetILBM (REG (a6) struct ClassBase * cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;
    struct IFFHandle *iff;
    struct BitMap *bm;
    ULONG modeid = 0;
    Point *point;
    STRPTR title;

    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    getdtattrs (cb, o,
		DTA_Handle, &iff,
		PDTA_BitMapHeader, &bmhd,
		PDTA_Grab, &point,
		TAG_DONE);

    /* IFF handle is already opened for reading */
    PropChunks (iff, ilbmprops, 5);
    StopChunk (iff, ID_ILBM, ID_BODY);

    DM (kprintf ("\n\nGetILBM\n"));

    if (ParseIFF (iff, IFFPARSE_SCAN) == 0L)
    {
	if (sp = FindProp (iff, ID_ILBM, ID_BMHD))
	{
	    CopyMem (sp->sp_Data, bmhd, sizeof (struct BitMapHeader));

	    if (bmhd->bmh_Depth > 8)
	    {
		DB (kprintf ("  can't handle depth of %ld\n", (ULONG) bmhd->bmh_Depth));
		SetIoErr (ERROR_OBJECT_WRONG_TYPE);
	    }
	    else
	    {
		/* Allocate a bitmap to hold the image */
		if (bm = AllocBitMap ((ULONG) bmhd->bmh_Width, (ULONG) bmhd->bmh_Height, (ULONG) bmhd->bmh_Depth, BMF_INTERLEAVED | BMF_DISPLAYABLE, NULL))
		{
		    if (sp = FindProp (iff, ID_ILBM, ID_NAME))
		    {
			DM (kprintf ("  name 0x%lx\n", sp));
			title = (STRPTR) sp->sp_Data;
		    }

		    /* Find the Grab chunk */
		    if (sp = FindProp (iff, ID_ILBM, ID_GRAB))
		    {
			DM (kprintf ("  grab 0x%lx\n", sp));
			*point = *((Point *) sp->sp_Data);
		    }

		    /* Prepare the Mode ID */
		    if (sp = FindProp (iff, ID_ILBM, ID_CAMG))
		    {
			DM (kprintf ("  camg 0x%lx\n", sp));
			modeid = *((ULONG *) sp->sp_Data);
			DM (kprintf ("  camg modeid = 0x%lx\n", modeid));
			if ((!(modeid & MONITOR_ID_MASK)) ||
			    ((modeid & EXTENDED_MODE) &&
			     (!(modeid & 0xFFFF0000))))
			{
			    modeid &= (~(EXTENDED_MODE | SPRITES | GENLOCK_AUDIO | GENLOCK_VIDEO | VP_HIDE));
			    DM (kprintf ("  unmasked modeid = 0x%lx\n", modeid));
			}
			if ((modeid & 0xFFFF0000) && (!(modeid & 0x00001000)))
			{
			    modeid = 0L;
			    DM (kprintf ("  garbage, so setting to zero\n"));
			}
		    }

		    if (modeid == 0L)
		    {
			modeid = LORES_KEY;
			if (bmhd->bmh_Width >= 640)
			    modeid = HIRES;
			if (bmhd->bmh_Height >= 400)
			    modeid |= LACE;
			DM (kprintf ("  new modeid = 0x%lx\n", modeid));
		    }

		    setdtattrs (cb, o,
				DTA_ObjName, title,
				DTA_NominalHoriz, bmhd->bmh_Width,
				DTA_NominalVert, bmhd->bmh_Height,
				PDTA_BitMap, bm,
				PDTA_ModeID, modeid,
				TAG_DONE);

		    /* Get the color information */
		    if (sp = FindProp (iff, ID_ILBM, ID_CMAP))
		    {
			struct ColorRegister *rgb, *cmap, *ccmap;
			WORD i, ncolors;
			LONG *cregs;

			i = ncolors = MAX ((sp->sp_Size / 3L), (1 << bmhd->bmh_Depth));

			DC (kprintf ("  Create %ld color entries\n", (LONG) ncolors));
			setdtattrs (cb, o, PDTA_NumColors, ncolors, TAG_DONE);

			getdtattrs (cb, o,
				    PDTA_ColorRegisters, (ULONG) & cmap,
				    PDTA_CRegs, &cregs,
				    TAG_DONE);

			rgb = (struct ColorRegister *) sp->sp_Data;
			ccmap = cmap;
			while (i--)
			{
			    *cmap = *rgb;
			    cmap++;
			    rgb++;
			}

			cmap = ccmap;
			DR (kprintf ("  ilbm.class\n"));
			for (i = 0; i < ncolors; i++)
			{
			    cregs[i * 3 + 0] = (LONG) (cmap[i].red   << 24) | (cmap[i].red << 16) | (cmap[i].red << 8) | cmap[i].red;
			    cregs[i * 3 + 1] = (LONG) (cmap[i].green << 24) | (cmap[i].green << 16) | (cmap[i].green << 8) | cmap[i].green;
			    cregs[i * 3 + 2] = (LONG) (cmap[i].blue  << 24) | (cmap[i].blue << 16) | (cmap[i].blue << 8) | cmap[i].blue;
			    DR (kprintf ("  %2ld) r=0x%02lx g=0x%02lx b=0x%02lx : r=0x%08lx g=0x%08lx b=0x%08lx\n",
				(LONG)i,
				(ULONG)cmap[i].red, (ULONG)cmap[i].green, (ULONG)cmap[i].blue,
				cregs[i * 3 + 0], cregs[i * 3 + 1], cregs[i * 3 + 2]));
			}
			DR (kprintf ("\n"));
		    }
		    else
		    {
			DC (kprintf ("  No color map!  Creating %ld entries\n", (LONG) (1 << bmhd->bmh_Depth)));
			setdtattrs (cb, o, PDTA_NumColors, (1 << bmhd->bmh_Depth), TAG_DONE);
		    }

		    if (GetBody (cb, iff, bmhd, bm) == 1)
			return (TRUE);
		    SetIoErr (DTERROR_COULDNT_OPEN);
		}
		else
		{
		    DB (kprintf ("  couldn't allocate bitmap\n"));
		    SetIoErr (ERROR_NO_FREE_STORE);
		}
	    }
	}
	else
	{
	    DB (kprintf ("  couldn't find BMHD\n"));
	    SetIoErr (ERROR_REQUIRED_ARG_MISSING);
	}
    }
    else
    {
	DB (kprintf ("  couldn't scan iff\n"));
	SetIoErr (ERROR_SEEK_ERROR);
    }

    return (FALSE);
}
