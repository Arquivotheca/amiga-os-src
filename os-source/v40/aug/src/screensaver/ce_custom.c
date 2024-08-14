/* ce_custom.c
 * screen save commodity
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <devices/inputevent.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <libraries/commodities.h>
#include <libraries/iffparse.h>
#include <workbench/workbench.h>
#include <iff/ilbm.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/commodities_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/icon_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/commodities_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include "ce_custom.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

extern struct Library *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct Library *IconBase;
extern struct Library *GfxBase;
extern struct Library *CxBase;
extern LONG cxSignal;

/*****************************************************************************/

struct Library *IFFParseBase;
STRPTR sshotkey = "shift alt s";
LONG unit = 0L;

/*****************************************************************************/

BOOL PutBody (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct BitMap * bm)
{
    BOOL success = TRUE;
    UBYTE p, *pptr;
    ULONG msize;
    UWORD i;

    if ((PushChunk (iff, 0, ID_BODY, IFFSIZE_UNKNOWN)) == 0L)
    {
	msize = 2 * ((bmhd->w + 15) / 16);
	for (i = 0; (i < bmhd->h) && success; i++)
	{
	    for (p = 0; (p < bmhd->nplanes) && success; p++)
	    {
		pptr = (UBYTE *) bm->Planes[p];
		if (WriteChunkBytes (iff, &pptr[i * bm->BytesPerRow], msize) != msize)
		    success = FALSE;
	    }
	}

	if ((PopChunk (iff)) == 0L)
	    return (success);
    }

    return (FALSE);
}

/*****************************************************************************/

BOOL PutColors (struct IFFHandle * iff, struct BitMapHeader * bmhd, struct ColorRegister * cmap)
{
    if ((cmap != NULL) && (PushChunk (iff, 0, ID_CMAP, IFFSIZE_UNKNOWN) == 0L))
	if ((WriteChunkRecords (iff, cmap, sizeof (struct ColorRegister), (1 << bmhd->nplanes))) == (1 << bmhd->nplanes))
	    if ((PopChunk (iff)) == 0L)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

BOOL PutBMHD (struct IFFHandle * iff, struct BitMapHeader * bmhd)
{
    if (PushChunk (iff, 0, ID_BMHD, sizeof (struct BitMapHeader)) == 0)
	if (WriteChunkBytes (iff, bmhd, sizeof (struct BitMapHeader)) == sizeof (struct BitMapHeader))
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

BOOL PutCAMG (struct IFFHandle * iff, ULONG modeid)
{
    if (PushChunk (iff, 0, ID_CAMG, sizeof (ULONG)) == 0)
	if (WriteChunkBytes (iff, &modeid, sizeof (ULONG)) == sizeof (ULONG))
	    if (PopChunk (iff) == 0)
		return (TRUE);
    return (FALSE);
}

/*****************************************************************************/

#ifndef	ID_NAME
#define	ID_NAME		MAKE_ID('N','A','M','E')
#endif

BOOL PutName (struct IFFHandle * iff, STRPTR name)
{
    ULONG msize;

    if (name)
    {
	msize = strlen (name) + 1;
	if (PushChunk (iff, 0, ID_NAME, msize) == 0)
	    if (WriteChunkBytes (iff, name, msize) == msize)
		if (PopChunk (iff) == 0)
		    return (TRUE);
	return (FALSE);
    }
    return (TRUE);
}

/*****************************************************************************/

VOID __stdargs __saveds ScreenSave (CxMsg * cxm, CxObj * co)
{
    struct BitMapHeader bmhd = {NULL};
    struct ColorRegister *colors;
    struct DisplayInfo di;
    struct IFFHandle *iff;
    struct Rectangle rect;
    struct ViewPort *vp;
    struct Screen *scr;
    UWORD i, numcolors;
    struct BitMap *bm;
    ULONG *table;
    ULONG modeid;
    LONG lock;

    DB (kprintf ("ScreenSave enter\n"));

    lock = LockIBase (0);

    if (scr = IntuitionBase->FirstScreen)
    {
	DB (kprintf ("AllocIFF\n"));
	if (iff = AllocIFF ())
	{
	    DB (kprintf ("OpenClipboard\n"));
	    if (iff->iff_Stream = (ULONG) OpenClipboard (unit))
	    {
		DB (kprintf ("InitIFFasClip\n"));
		InitIFFasClip (iff);

		DB (kprintf ("Get mode id\n"));
		vp = &scr->ViewPort;
		modeid = GetVPModeID (vp);
		DB (kprintf ("QueryOverScan\n"));
		QueryOverscan (modeid, &rect, OSCAN_TEXT);
		DB (kprintf ("GetDisplayInfoData\n"));
		GetDisplayInfoData (NULL, (UBYTE *) & di, sizeof (struct DisplayInfo), DTAG_DISP, modeid);

		numcolors = 1 << scr->BitMap.Depth;

		bmhd.w = scr->Width;
		bmhd.h = scr->Height;
		bmhd.x = 0;
		bmhd.y = 0;
		bmhd.nplanes = scr->BitMap.Depth;
		bmhd.XAspect = di.Resolution.x;
		bmhd.YAspect = di.Resolution.y;
		bmhd.PageWidth = rect.MaxX + 1;
		bmhd.PageHeight = rect.MaxY + 1;

		DB (kprintf ("AllocBitMap\n"));
		if (bm = AllocBitMap ((ULONG) bmhd.w, (ULONG) bmhd.h, (ULONG) bmhd.nplanes, BMF_DISPLAYABLE | BMF_STANDARD | BMF_CLEAR, NULL))
		{
		    DB (kprintf ("BltBitMap\n"));
		    BltBitMap (&scr->BitMap, bmhd.x, bmhd.y, bm, 0, 0, bmhd.w, bmhd.h, 0xC0, 0xFF, NULL);
		    DB (kprintf ("AllocVec colors and table\n"));
		    if (table = AllocVec (sizeof (ULONG) * 3 * numcolors, MEMF_CLEAR))
		    {
			if (colors = AllocVec (sizeof (struct ColorRegister) * numcolors + 2, MEMF_CLEAR))
			{
			    DB (kprintf ("GetRGB32 %ld\n", (LONG)numcolors));
			    GetRGB32 (vp->ColorMap, 0, numcolors, table);
			    for (i = 0; i < numcolors; i++)
			    {
				colors[i].red = table[i * 3 + 0] >> 24;
				colors[i].green = table[i * 3 + 1] >> 24;
				colors[i].blue = table[i * 3 + 2] >> 24;
			    }

			    DB (kprintf ("OpenIFF\n"));
			    if (OpenIFF (iff, IFFF_WRITE) == 0)
			    {
				DB (kprintf ("PushChunk Form\n"));
				if (PushChunk (iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN) == 0)
				    if (PutName (iff, scr->DefaultTitle))
					if (PutCAMG (iff, modeid))
					    if (PutBMHD (iff, &bmhd))
						if (PutColors (iff, &bmhd, colors))
						    if (PutBody (iff, &bmhd, bm))
							if (PopChunk (iff) == 0)
							    DisplayBeep (scr);
				CloseIFF (iff);
			    }
			    FreeVec (colors);
			}
			FreeVec (table);
		    }
		    FreeBitMap (bm);
		}
		CloseClipboard ((struct ClipboardHandler *) iff->iff_Stream);
	    }
	    FreeIFF (iff);
	}
    }

    UnlockIBase (lock);
}

/*****************************************************************************/

BOOL CreateCustomCx (CxObj * broker)
{
    CxObj *filter;

    if (SysBase->lib_Version >= 39)
    {
	if (IFFParseBase = OpenLibrary ("iffparse.library", 0))
	{
	    if (filter = CxFilter (sshotkey))
	    {
		AttachCxObj (filter, CxCustom (ScreenSave, 0));
		AttachCxObj (filter, CxTranslate (NULL));

		if (CxObjError (filter) == 0)
		{
		    AttachCxObj (broker, filter);
		    return (TRUE);
		}
		DeleteCxObjAll (filter);
	    }
	}
    }

    return (FALSE);
}

/*****************************************************************************/

VOID DisposeCustomCx (VOID)
{
    CloseLibrary (IFFParseBase);
}

/*****************************************************************************/

BOOL ProcessCustomArgs (struct WBStartup * wbMsg, struct DiskObject * diskObj, ULONG * cliOpts)
{
    STRPTR arg;

    if (diskObj)
    {
	if (arg = FindToolType (diskObj->do_ToolTypes, "CX_POPKEY"))
	    sshotkey = arg;
    }
    return (TRUE);
}

/*****************************************************************************/

VOID ProcessCustomCxMsg (ULONG cmd)
{
}

/*****************************************************************************/

VOID ProcessCustomCxCmd (ULONG cmd)
{
}

/*****************************************************************************/

VOID ProcessCustomCxSig (VOID)
{
}
