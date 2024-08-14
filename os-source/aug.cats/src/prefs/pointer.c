/* pointer.c
 * pointer preferences
 * Written by David N. Junod
 */

#include "prefsbase.h"
#include <intuition/intuitionbase.h>
#include <graphics/sprite.h>

void kprintf (void *,...);

#define	DB(x)	;
#define	D(x)	;

ILBM *ReadILBM (BPTR drawer, STRPTR name, struct TagItem * attrs);
VOID FreeILBM (ILBM * ilbm);

/* extra memory (in bytes ) allocated for control info and terminator	*/
#define SPMEM_EXTRA		( 2 * sizeof (short int) + 2 *  sizeof (short int))
#define SPMEM_BYTES( height )	(SPMEM_EXTRA+2*sizeof (short int) *(height))

/* word index to reserved at bottom	*/
#define SPMEM_RSRVD( height ) 	(2 + 2 * (height))
#define	SPRF_FORCED	(1)

struct PointerPref *
getpointerpref (BPTR drawer, STRPTR name, struct TagItem * attrs)
{
    struct PointerPref *pp = NULL;
    ILBM *ilbm;

    /* Get the ILBM record */
    if (ilbm = ReadILBM (drawer, name, attrs))
    {
	struct RastPort *srp = &(ilbm->ir_RPort);

	/* Allocate the preference structure */
	if (pp = (struct PointerPref *) AllocVec (sizeof (*pp), MEMF_CLEAR))
	{
	    /* Setup the dimension information */
	    pp->pp_Height = ilbm->ir_Height;
	    pp->pp_Width = 16;	/* ilbm->ir_Width; */
	    pp->pp_XOffset = -(ilbm->ir_Grab.x);
	    pp->pp_YOffset = -(ilbm->ir_Grab.y);

	    /* Calculate size of sprite data */
	    pp->pp_DSize = (LONG) SPMEM_BYTES (pp->pp_Height);

	    /* Allocate sprite data */
	    if (pp->pp_PData = (UWORD *)AllocVec (pp->pp_DSize, MEMF_CLEAR | MEMF_CHIP))
	    {
		/* Lotsa stack */
		struct BitMap spr_BMap = {NULL};
		struct RastPort spr_RPort = {NULL};
		struct RastPort *drp = &spr_RPort;
		struct BitMap *dbm = &spr_BMap;

		/* Setup position control terminator */
		pp->pp_PData[SPMEM_RSRVD (pp->pp_Height)] = 0;
		pp->pp_PData[SPMEM_RSRVD (pp->pp_Height) + 1] = 0;

		/* Initialize the bitmap */
		InitBitMap (dbm, 2L, 32L, (LONG) pp->pp_Height);

		/* Do weird magic with the planes */
		dbm->Planes[0] = (PLANEPTR) (pp->pp_PData + 2);
		dbm->Planes[1] = (PLANEPTR) (pp->pp_PData + 3);

		/* Initialize the RastPort */
		InitRastPort (drp);
		drp->BitMap = dbm;
		SetRast (drp, 0);

		/* Copy the ILBM into the sprite */
		ClipBlit (srp, 0, 0, drp, 0, 0, pp->pp_Width, pp->pp_Height, 0xC0);
	    }
	    else
	    {
		FreeVec ((APTR) pp);
		pp = NULL;
	    }
	}

	/* Free the temporary ILBM record */
	FreeILBM (ilbm);
    }

    return (pp);
}

static UWORD chip WaitPointer[] =
{
    0x0000, 0x0000,

    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07E0,

    0x0000, 0x0000,		/* reserved, must be NULL */
};

static UWORD chip DefaultPointer[] =
{
    0x0000, 0x0000,

    0xC000, 0x4000,
    0x7000, 0xB000,
    0x3C00, 0x4C00,
    0x3F00, 0x4300,
    0x1FC0, 0x20C0,
    0x1FC0, 0x2000,
    0x0F00, 0x1100,
    0x0D80, 0x1280,
    0x04C0, 0x0940,
    0x0460, 0x08A0,
    0x0020, 0x0040,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,		/* reserved, must be NULL */
};

struct PointerPref *GetPointerPref (
	BPTR drawer,
	STRPTR name,
	struct PointerPref * opp,
	struct TagItem * attrs)
{
    struct LibBase *lb = ((struct ExtLibrary *) PrefsBase)->el_Base;
    struct Process *proc = (struct Process *) FindTask (NULL);
    LONG msize = sizeof (struct PointerPref);
    struct PointerPref *pp = NULL;

    /* First try getting it from the passed directory */
    if (!(pp = getpointerpref (drawer, name, attrs)))
    {
	/* If the file wasn't found, then try in the system drawer */
	if (!(CompairLocks (drawer, lb->lb_SysPrefs)))
	{
	    /* Try getting the file from the system drawer */
	    if (pp = getpointerpref (lb->lb_SysPrefs, name, attrs))
	    {
		struct Prefs *p = &(pp->pp_Header);

		/* Tell where we came from */
		p->p_Flags |= PREFS_SYSTEM_F;
	    }
	}
    }

    /* See if we don't have a preference record due to no file */
    if (proc->pr_Result2 == ERROR_OBJECT_NOT_FOUND)
    {
	/* Allocate a default preference file */
	if (pp = (struct PointerPref *) AllocVec (msize, MEMF_CLEAR))
	{
	    struct Prefs *p = &(pp->pp_Header);

	    /* Tell where we came from */
	    p->p_Flags |= PREFS_DEFAULT_F;

	    /* Point the data at the right spot */
	    pp->pp_PData = DefaultPointer;
	    if (FindTagItem (PREFS_PRIVATE_1, attrs))
	    {
		pp->pp_PData = WaitPointer;
		pp->pp_XOffset = (-6);
	    }

	    /* Set up the size */
	    pp->pp_Height = 16;
	    pp->pp_Width = 16;
	}
    }

    /* See if we're freshening */
    if (opp && pp)
    {
	VOID FreePref (VOID *);
	struct Prefs *op = &(opp->pp_Header);
	UWORD *tmp;
	LONG tmps;

	/* Clear the where from flags */
	op->p_Flags &= ~PREFS_WHERE_F;

	/* Swap the data */
	tmp = pp->pp_PData;
	pp->pp_PData = opp->pp_PData;
	opp->pp_PData = tmp;

	/* Swap the size */
	tmps = pp->pp_DSize;
	pp->pp_DSize = opp->pp_DSize;
	opp->pp_DSize = tmps;

	/* Copy the rest of the data */
	opp->pp_Height = pp->pp_Height;
	opp->pp_Width = pp->pp_Width;
	opp->pp_XOffset = pp->pp_XOffset;
	opp->pp_YOffset = pp->pp_YOffset;
	opp->pp_CRegs[0] = pp->pp_CRegs[0];
	opp->pp_CRegs[1] = pp->pp_CRegs[1];
	opp->pp_CRegs[2] = pp->pp_CRegs[2];

	/* Free the new preference file */
	FreePref (pp);

	/* Return the same old preference file with new information */
	pp = opp;
    }

    DB (kprintf("GetPointer 0x%lx\n", pp));
    return (pp);
}
