/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: rastrect.c,v 39.3 92/02/20 13:50:52 chrisg Exp $
*
******************************************************************************/

/* hookfill code */

#include <exec/types.h>
#include <exec/nodes.h>
#include <utility/hooks.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <pragmas/utility_pragmas.h>
#include "/macros.h"
#include "c.protos"

/* intersect rect with cliprects and call hook */

void rastrect(struct RastPort  *rp, struct Rectangle rect,
						struct Hook *hook)
{
    struct Layer  *l = rp->Layer;

    if(l) /* intersect rect with ClipRects, calling hook for each */
    {
		struct RastPort tmprp;	/* work rastport */
		struct BitMap   *sbm;	/* shared bitmap */
		struct ClipRect *cr;	/* cliprect */

		LOCKLAYER(l);

		tmprp = *rp;

		sbm = tmprp.BitMap;

		for (cr = l->ClipRect; cr; cr = cr->Next)
		{
			struct Rectangle clip;

			clip = rect;

			clip.MinX += (l->bounds.MinX-l->Scroll_X);
			clip.MaxX += (l->bounds.MinX-l->Scroll_X);
			clip.MinY += (l->bounds.MinY-l->Scroll_Y);
			clip.MaxY += (l->bounds.MinY-l->Scroll_Y);

			if( rectXrect(&clip,&cr->bounds) )
			{
				WORD offsetx;
				WORD offsety;

				intersect(&clip,&cr->bounds,&clip);

				offsetx = clip.MinX - (l->bounds.MinX-l->Scroll_X);
				offsety = clip.MinY - (l->bounds.MinY-l->Scroll_Y);

				if (cr->lobs)		      /* obscured? */
				{
					struct BitMap *bm;

					if (bm = cr->BitMap)
					{

						/* adjust obscured cliprect origin */

						clip.MinX -= cr->bounds.MinX;
						clip.MaxX -= cr->bounds.MinX; 
						clip.MinY -= cr->bounds.MinY;
						clip.MaxY -= cr->bounds.MinY; 

						/* re-align for off-screen bitmaps */

						clip.MinX += (cr->bounds.MinX & 0xF);
						clip.MaxX += (cr->bounds.MinX & 0xF);

						/* obscured bitmaps are nonlayered */

						tmprp.BitMap = bm;
						tmprp.Layer  = NULL;

						if(hook)
						{
						   CallRastRectHook(hook,&tmprp,l,PASSRECT(clip),offsetx,offsety);
						}

						tmprp.Layer  = l;
					}
				}
				else			  /* on screen */
				{
					/* onscreen bitmaps always layered */

					tmprp.BitMap = sbm;

					CallRastRectHook(hook,&tmprp,l,clip,offsetx,offsety);
				}
						
			}
		}

		if(sbm = l->SuperBitMap) 
		{
			tmprp.BitMap = sbm;
			tmprp.Layer  = NULL;

			for (cr = l->SuperClipRect; cr; cr = cr->Next)
			{
				struct Rectangle clip;

				clip = rect;

				if( rectXrect(&clip,&cr->bounds) )
				{
					WORD offsetx;
					WORD offsety;

					intersect(&clip,&cr->bounds,&clip);

					offsetx = clip.MinX;
					offsety = clip.MinY;

					CallRastRectHook(hook,&tmprp,NULL,PASSRECT(clip),offsetx,offsety);
				}
			}
		}

		UNLOCKLAYER(l); 

    }
    else
    {
		if(hook) CallRastRectHook(hook,rp,l,rect,rect.MinX,rect.MinY);
    }
}

int CallRastRectHook( struct Hook *hook,void *object, struct Layer *layer,
								struct Rectangle bounds,WORD x, WORD y)
{
    if (hook) return( CallHookPkt( hook, object, &layer) ); else return( NULL );
}
