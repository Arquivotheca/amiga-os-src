
/******************************************************************************
*
*	$Id: makevp.c,v 42.0 93/06/16 11:15:35 chrisg Exp $
*
******************************************************************************/


/* graphics  kernel routines */

    #include "/displayinfo.h"
    #include "/displayinfo_internal.h"
    #include "/vp_internal.h"
    #include "/gfx.h"
    #include "/gfxbase.h"
    #include "/display.h"
    #include "/copper.h"
    #include "/view.h"
    #include "/macros.h"
    #include "/gfxpragmas.h"
    #include "tune.h"
    #include "c.protos"
    #include "/d/d.protos"

#define USEGBDEBUG
/*#define DEBUG*/
/*#define WACKDEBUG*/

#ifdef USEGBDEBUG
#define GBDEBUG if (GBASE->Debug)
#else
#define GBDEBUG
#endif

#ifdef DEBUG
#define D(x) {GBDEBUG {x};}
#else
#define D(x)
#endif

#ifdef WACKDEBUG
#define WACK Debug()
#else
#define WACK
#endif

/* From the old makevp.c ..... */

ULONG new_mode(struct ViewPort *vp)
{
    ULONG  modes = vp->Modes;
    struct ColorMap *cm = vp->ColorMap;
    struct DisplayInfoRecord   *record;
	
    D(kprintf("new_mode() - modes = 0x%lx, cm = 0x%lx\n", modes, cm);)

    if ((cm && cm->Type) &&
        ((record = cm->CoerceDisplayInfo)|| (record = cm->NormalDisplayInfo)))
    {
	modes = record->rec_Control; /* v1.4 modes */
	D(kprintf("take record->Control, modes = 0x%lx, CDI = 0x%lx, NDI = 0x%lx\n",
	           modes, cm->CoerceDisplayInfo, cm->NormalDisplayInfo);)
    }
    else
    {
	D(kprintf("mask modes\n");)
	if(modes & EXTENDED_MODE)
	{            /* 8      8      4       4   = 8C0C */
	    modes &= (HIRES | HAM | DUALPF | LACE | DOUBLESCAN | ((vp->Modes & DUALPF) ? PF2PRI : 0));
	}
    }

    return(modes);
}

unsigned short check_genlock(struct ColorMap *cm,int i)
{
	if ( cm && cm->Type && (cm->Flags & COLORMAP_TRANSPARENCY))
		return(((UWORD *)cm->ColorTable)[i] & 0x8000);
	else
		return (0);
}

ULONG CleanMode(ULONG mode)
{
    if (!(mode & EXTENDED_MODE))
    {
	mode &= ~(SPRITES | VP_HIDE | GENLOCK_AUDIO | GENLOCK_VIDEO);
    }
    else
    {
	if ((mode & MONITOR_ID_MASK) == EXTENDED_MODE)
	{
		mode &= ~EXTENDED_MODE;
		D(kprintf("New mode = 0x%lx\n", mode); WACK;)
	}
    }
    return(mode);
}

ULONG __regargs RealVPModeID(struct ViewPort *vp, struct View *v)
{
    struct ColorMap *cm;
    ULONG mode = INVALID_ID;

    if ((cm = vp->ColorMap) && (cm->Type))
    {
	struct DisplayInfoRecord *record;
	if ((record = cm->CoerceDisplayInfo) || (record = cm->NormalDisplayInfo))
	{
		D(kprintf("Coerced ");)
		mode = (record->rec_MajorKey << 16) | record->rec_MinorKey;
	}
    }

    if (mode == INVALID_ID)
    {
	if ((mode = CleanMode(gfx_GetVPModeID(vp))) == INVALID_ID)
	{
		mode = CleanMode(new_mode(vp));

		/* If there is no ViewExtra, then we don't want to
		 * promote the ViewPort, so specifically use the
		 * PAL or NTSC monitors.
		 */
		if (!(v->Modes & EXTEND_VSTRUCT))
		{
			mode |= ((GBASE->Flags & PAL) ? PAL_MONITOR_ID : NTSC_MONITOR_ID);
		}
	}
    }

    return(mode);
}

void __regargs AttachVecTable(struct ViewPortExtra *vpe, struct VecInfo *vinfo, ULONG ID)
{
    struct VecInfo v;

    D(kprintf("AttachVecTable(0x%lx, 0x%lx, 0x%lx)\n", vpe, vinfo, ID);)
    if (vinfo == NULL)
    {
	if (gfx_GetDisplayInfoData(NULL, (APTR)&v, sizeof(struct VecInfo), DTAG_VEC, ID))
	{
		vinfo = &v;
	}
    }
    if (vinfo)
    {
    	if ((vpe->VecTable = vinfo->Vec) == NULL)
    	{
    		/* must have been a disk-based monitor. Do the best we can. */
		D(kprintf("find the best we can.\n");)
    		vpe->VecTable = GetVecTable(NULL, NULL);
    	}
	D(kprintf("VecTable = 0x%lx\n", vpe->VecTable);)
    }
}

/*******************************************************************************/

/* MakeVPort() is now data driven, using the data in the new (V39) ProgInfo
 * and VecInfo structures.
 */

ULONG __asm MakeVPort(register __a0 struct View *v, register __a1 struct ViewPort *vp)
{
    struct ViewPortExtra *vpe = NULL;
    ULONG err;
    ULONG mode;
    struct VecInfo vinfo;
    struct BuildData bd;		/* workspace */

	GBASE->SpecialCounter++;	/* invalidate copper list caches */

    D(kprintf("In MakeVP() - View = 0x%lx, ViewPort = 0x%lx ctr=%ld\n", v, vp,GBASE->SpecialCounter);)


    if (v && vp)	/* let's be reasonable */
    {
	/* If this ViewPort is being coerced to a different mode, get the
	 * modeid from the coercion, else use getvpmodeid().
	 */

	mode = RealVPModeID(vp, v);
	D(kprintf("Mode = 0x%lx\n", mode); WACK;)
	if ((vp->ColorMap) && (vp->ColorMap->Type))
	{
		vpe = vp->ColorMap->cm_vpe;
	}

	/* Find the VecInfo associated with this ViewPort's mode */

	if (gfx_GetDisplayInfoData(NULL, (UBYTE *)&vinfo, sizeof(struct VecInfo), DTAG_VEC, mode))
	{
		struct ProgInfo *pinfo = (struct ProgInfo *)vinfo.Data;
		char **makeit;
		int (*nextone)();
		BOOL go = TRUE;

		makeit = (GetVecTable(vp, &vinfo))->BuildVP;
		D(kprintf("VecTable = 0x%lx\n", GetVecTable(vp, &vinfo));)
		D(kprintf("makeit = 0x%lx\n", makeit);)

		/* first in the list is the Initialisation code, last one
		 * is the cleanup code.
		 */
		nextone = (int (*)())(*makeit++);
		if ((err = ((*nextone)(v, vp, &vpe, pinfo, &bd) & 0xFFFF)) == MVP_OK)
		{
			D(kprintf("vpe->Flags = 0x%lx=\n", vpe->Flags);)
			if (vpe->VecTable == NULL)
			{
				AttachVecTable(vpe, &vinfo, mode);
			}

			while (go && (nextone = (int (*)())(*makeit)))
			{
				if ((err = ((*nextone)(v, vp, vpe) & 0xFFFF)) == MVP_OK)
				{
					makeit++;
				}
				else
				{
					go = FALSE;
				}
			}
		}
		else
		{
			D(kprintf("Wot no InitMVP()???\n"); WACK;)
		}

		/* look for the cleanup routine */
		if (*makeit)
		{
			/* didn't go through to the end */
			while (*makeit)
			{
				nextone = (int (*)())(*makeit++);
			}
			(*nextone)(v, vp, vpe);
		}
	}
	else
	{
		D(kprintf("*** ARGHHH! SHIT! Where's the bleedin' VecInfo eh?\n"); WACK;)
	}
    }

    /* Check for our special case. This happens if the ViewPort is totally off the
     * bottom of the display, which can happen with attached screens, MoveScreen(),
     * or changing the DyOffset manually.
     *
     * We have to return MVP_OK in this case, but also free and NULL out the
     * ViewPort->DspIns. We cannot use the FreeVPortCopList() because that will
     * also free the UserCopperLists.
     */
    if (err == MVP_OFF_BOTTOM)
    {
    	err = MVP_OK;
	freecoplist(vp->DspIns);
	vp->DspIns = NULL;	/* ensure MrgCop() does not use any old copper lists */
    }

    return(err);

}
