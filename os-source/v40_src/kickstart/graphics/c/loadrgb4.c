/******************************************************************************
*
*	$Id: loadrgb4.c,v 39.2 92/01/21 13:27:45 chrisg Exp $
*
******************************************************************************/
#define	NEWPIXELALIGN

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/copper.h>
#include <hardware/custom.h>
#include <graphics/gfxbase.h>
#include "/sane_names.h"
#include "/macros.h"
#include "c.protos"

extern struct Custom custom;

/* #define DEBUG */

#define FIRSTSPRITE 16

#define PROTOTYPE_DISPLAY
#ifdef PROTOTYPE_DISPLAY
#include "/displayinfo_internal.h"
#endif 

hedley_load(vp,p)
struct ViewPort *vp;
UWORD *p;
{
}

#ifdef VP_IN_VIEW /* { */
/* find the vp in the view, return TRUE if in view, false if not in view */

vp_in_view(view,vp)
struct View *view;
struct ViewPort *vp;
{
#ifndef VP_IN_VIEW /* { */
	return( TRUE ); /* compatibility with 1.3 !!! yuck -- bart */
#else /* }{ */
	int found = FALSE;

	if( view )
	{
		struct ViewPort *vport;
		
		for(vport = view->ViewPort; vport; vport = vport->Next)
		{
			if(vport == vp)
			{
				found = TRUE; break;
			}
		}

	}

	return(found);
#endif */ } */
}
#endif */ } */

#define DOWNLOADRGB4

#ifndef DOWNLOADRGB4
/*  load color registers, rgb values, 4 bits per gun */
/*  packed in consecutive right justified words */
loadrgb4(vp,p,cnt)
struct ViewPort *vp;
UWORD *p;
short cnt;
{
    int j;
    USHORT d;
    UWORD *c;
	struct CopList *cl;

#ifdef DEBUG
	kprintf("loadrgb4(%lx,%lx,%lx)\n",vp,p,cnt);
#endif
	if (cnt > 32)	cnt = 32;

	if (vp)
	{
		struct ColorMap *cm = vp->ColorMap;
		UWORD		 newtable[32];
		ULONG 		 modes = new_mode(vp);
		/* install in ColorMap */
		if ( cm )
		{
			if (cnt > cm->Count)	cnt = cm->Count;
			c = cm->ColorTable;
    		for (j = 0; j < cnt ; j++) *c++ = *p++;
			p -= cnt;
			/* check for special genlock modes */
			if (cm->Type && (cm->Flags & COLORMAP_TRANSPARENCY))
			{
				unsigned short *t = newtable;
				for (j = 0; j < cnt; j++)
				{
					*(t+j) = *(p+j) | check_genlock(cm,j);
				}
				p = newtable;
			}
		}
		if( (vp->ExtendedModes & VPF_A2024)
		||  (modes == A2024TENHERTZ_KEY) 
		||  (modes == A2024FIFTEENHERTZ_KEY)
		||  (modes &  SUPERHIRES) )
		{
			if(cm) hedley_load(vp,cm->ColorTable);
		}
		else
		{
			/* install in various copper lists */
			if ( (cl = vp->DspIns) != 0)
			{
				/* single thread access to ActiView hardware copper list */
				ObtainSemaphore(GBASE->ActiViewCprSemaphore);

				if ( (vp->Modes & VP_HIDE) == 0)
				{
#ifdef VP_IN_VIEW /* { */
					if( vp_in_view(GBASE->ActiView,vp) ) /* vp in active view? */
#endif /* } */
					{
					/* check if we should update the copper lists 
					 * - spence Nov  9 1990
					*/
						struct RasInfo *ri;
						struct BitMap *bm;
						int vpc = 0;

						if ((ri = vp->RasInfo) && (bm = ri->BitMap))
							vpc = (1 << (bm->Depth));

						if ((vpc == 0) || (cnt <= vpc) ||
							(vp->Modes & DUALPF))	/* if dualpf, do them all anyway */
						{
							/* hardware list */
							npokeCprList(cl->CopLStart,&custom.color[0],p,cnt);
							npokeCprList(cl->CopSStart,&custom.color[0],p,cnt);
						}
						else
						{
							npokeCprList(cl->CopLStart,&custom.color[0],p,vpc);
							npokeCprList(cl->CopSStart,&custom.color[0],p,vpc);
							if (cnt > FIRSTSPRITE)
							{
								int k;
								npokeCprList(cl->CopLStart,&custom.color[FIRSTSPRITE],(k = p + FIRSTSPRITE),(j = (cnt - FIRSTSPRITE)));
								npokeCprList(cl->CopSStart,&custom.color[FIRSTSPRITE],k,j);
							}
						}
					}
				}

				ReleaseSemaphore(GBASE->ActiViewCprSemaphore);

				/* intermediate list */
				for (j = 0; j < cnt ; j++)
			    pokeCopIns(cl->CopIns,COPPER_MOVE,&custom.color[j],*p++);
			}
		}
	}
	else for (j = 0; j < cnt ; j++)	custom.color[j] = *p++;
	update_top_color(GBASE->current_monitor);
#ifdef DEBUG
	kprintf("return from loadrgb\n");
#endif
}
#endif
