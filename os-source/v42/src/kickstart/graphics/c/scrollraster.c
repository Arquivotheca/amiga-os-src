/******************************************************************************
*
*	$Id: scrollraster.c,v 42.0 93/06/16 11:15:54 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <hardware/blit.h>
#include <graphics/regions.h>
#include <graphics/layers.h>
#include <pragmas/layers_pragmas.h>
#include "/gfxbase.h"
#include "/macros.h"
#include "c.protos"
#include "/gfxpragmas.h"

#define COPY    NANBC|NABC|ABNC|ABC
#define REFRESHFLAGS (LAYERREFRESH | LAYERIREFRESH | LAYERIREFRESH2)

/*#define DEBUG*/

UWORD getmaxbytesperrow(rp,TempA)
struct RastPort *rp;
long *TempA;
{
#ifdef DEBUG
	printf("getmaxbytesperrow(%lx,%lx)\n",rp,TempA);
#endif
	if (*TempA = getTempA(rp)) return(TRUE);
	*TempA = getmustmem(MAXBYTESPERROW,MEMF_CHIP);
#ifdef DEBUG
	printf("getmaxbyteperrow returning false\n");
#endif
	return(FALSE);
}

#ifndef MAKE_PROTOS /* SAS doesn't handle proto generation for functions taking pointers to __asm functions */
void __regargs scroll_primitive(struct RastPort *rp,short dx,short dy,short MINX,short MINY,
				short MAXX,short MAXY,
				 void (* __asm fillptr)(register __a1 struct RastPort *rp, 
							register __d0 short xl, register __d1 short yl,
							register __d2 short xu, register __d3 short yu))
{
    struct Layer *cw;
    short direction;
    short absdx,absdy;
    short maxx,maxy;
    short height,width;
    short srcx,srcy;
    short dstx,dsty;
    char *TempA;
    struct BitMap *bm;
    UWORD using_tmpras = FALSE;

#ifdef DEBUG
    printf("rscroll(%lx,%ld,%ld,%ld,%ld,%ld,%ld)\n",rp,dx,dy,MINX,MINY,MAXX,MAXY);
#endif

#ifdef DEBUGE
    printf("[");
#endif

    TempA = 0;
    if (dy == 0) using_tmpras = getmaxbytesperrow(rp,&TempA);

    bm = rp->BitMap;
    absdx = dx;
    absdy = dy;
    direction = 0;
    if (dy < 0)
    {
		absdy = -dy;
		direction = 2;
    }
    if (dx < 0)
    {
		absdx = -dx;
		direction += 1;
    }
    switch(direction)
    {
		case 0 : /* upper left movement */
			srcx = absdx;   srcy = absdy;
			dstx = 0;       dsty = 0;
			break;
		case 1 : /* upper right movement */
			srcx = 0;       srcy = absdy;
			dstx = absdx;   dsty = 0;
			break;
		case 2 : /* lower left movement */
			srcx = absdx;   srcy = 0;
			dstx = 0;       dsty = absdy;
			break;
		case 3 : /* lower right movement */
			srcx = 0;       srcy = 0;
			dstx = absdx;   dsty = absdy;
			break;
    }


    if ( (cw = rp->Layer) == 0)
    {
		srcx += MINX;
		srcy += MINY;
		dstx += MINX;
		dsty += MINY;

		maxx = SHORTMIN(MAXX,(rp->BitMap->BytesPerRow<<3) - 1);
		maxy = SHORTMIN(MAXY,rp->BitMap->Rows - 1);
#ifdef DEBUG
    	printf("maxx=%ld maxy=%ld\n",maxx,maxy);
#endif

		height = maxy + 1 - absdy - MINY;
		width = maxx + 1 - absdx - MINX;
		if ( (height <= 0) || (width <= 0)) goto RETURN;
#ifdef DEBUG
		printf(
		"BLTBITMAP(%lx,%lx,%ld,%ld,%lx,%ld,%ld,%ld,%ld,%lx,%lx,%lx)\n",
		    	bm,srcx,srcy,
		    	bm,dstx,dsty,width,height,COPY,rp->Mask,TempA);
#endif
		gfx_BltBitMap(bm,srcx,srcy,
		    	bm,dstx,dsty,width,height,COPY,rp->Mask,TempA);
#ifdef DEBUG
		printf("return\n");
#endif
    }
    else
    {
#ifdef DEBUG
	printf("locklayerrom\n");
#endif
		LOCKLAYER(cw);

		if (cw->SuperBitMap)
		{
		    struct RastPort temp_rastport;

		    /*maxx = (bm->BytesPerRow<<3) - 1;
		    maxy = bm->Rows - 1;*/

		    SyncSBitMap(cw);

		    /* bm = rp->BitMap; */
		    /* rp->BitMap = cw->SuperBitMap; */
		    /* I'm a BAD BOY */
		    /* rp->Layer = 0; */
		    /* scrollraster(rp,dx,dy,MINX,MINY,MAXX,MAXY); */
		    /* rp->BitMap = bm; */
		    /* rp->Layer = cw; */

		    /* bart - 03.06.86 */

		    temp_rastport = *rp;
		    temp_rastport.BitMap = cw->SuperBitMap;
		    temp_rastport.Layer = NULL;
		    scroll_primitive(&temp_rastport,dx,dy,MINX,MINY,MAXX,MAXY,fillptr);

		    /* end bart - 03.06.86 */

		    CopySBitMap(cw);
		    UNLOCKLAYER(cw);
		    goto RETURN;
		}
		else
		{
		    struct ClipRect *cr,*crn;
		    struct Rectangle tmprect;
		    struct Rectangle boundsrect;
		    struct Rectangle clip; 

		    /* do tough scroll */
		    /* step 1: sort ClipRects into correct order */
		    /* step 2: for each ClipRect, move it bits internal */
		    /*         then look through rest of ClipRects for  */
		    /*         bits to move in */

		    /* scroll damage added to cw->DamageList -- bart */
		    /* dont add scroll damage added to smart -- bart */
		    /* the reason for this is that if the guy really */
		    /* wants to scroll things that dont exist he/she */
		    /* should be smart enough to refresh that region */
		    /* and not penalize everybody else 89.03.28 bart */

		    maxx = cw->bounds.MaxX - cw->bounds.MinX;
		    maxy = cw->bounds.MaxY - cw->bounds.MinY;

		    maxx = SHORTMIN(maxx,MAXX);
		    maxy = SHORTMIN(maxy,MAXY);

		    if(cw->Flags & LAYERSIMPLE) /* speedup damage calculation */
		    {
			struct Region *old;

			if(old = cw->DamageList)
			{
			    struct Region *new;

			    if(new = (struct Region *)NewRegion())
			    {
				struct Region *tmp;

				if(tmp = (struct Region *)NewRegion())
				{
				    clip.MinX = MINX;
				    clip.MinY = MINY;
				    clip.MaxX = maxx;
				    clip.MaxY = maxy;

				    for (cr = cw->ClipRect; 
					 cr; 
				         cr = cr->Next )
				    {
					if(!cr->lobs) /* onscreen only */
					{
					    tmprect = cr->bounds;

					    tmprect.MinX -= cw->bounds.MinX;
					    tmprect.MaxX -= cw->bounds.MinX;
					    tmprect.MinY -= cw->bounds.MinY;
					    tmprect.MaxY -= cw->bounds.MinY;

					    if(rectXrect(&tmprect,&clip))
					    {
						intersect(&tmprect,&clip,&tmprect);

						orrectregion(new,&tmprect.MinX);
						orrectregion(tmp,&tmprect.MinX);
					    }
					}
				    }

				    /* shift offset */
				    tmp->bounds.MinX += dx;
				    tmp->bounds.MaxX += dx;
				    tmp->bounds.MinY += dy;
				    tmp->bounds.MaxY += dy;

				    if(xorregionregion(new,tmp))
				    {
				    	if(old->RegionRectangle)
					{
					    /* only if existing damage */
					    orregionregion(old,tmp);
					}

					/* shift back */
					tmp->bounds.MinX -= dx;
					tmp->bounds.MaxX -= dx;
					tmp->bounds.MinY -= dy;
					tmp->bounds.MaxY -= dy;

					if(andregionregion(tmp,new))
					{
					    /* remove area of vacated damage */

					    if (dy > 0) 
					    {
						clip.MaxY = 
						SHORTMAX(MINY,maxy-dy);
					    }
					    else 
					    {
						if (dy < 0)
						{
						    clip.MinY = 
						    SHORTMIN(maxy,MINY+absdy);
						}
					    }
					    if (dx > 0) 
					    {
						clip.MaxX = 
						SHORTMAX(MINX,maxx-dx);
					    }
					    else 
					    {
						if (dx < 0)
						{
						    clip.MinX = 
						    SHORTMIN(maxx,MINX+absdx);
						}
					    }

					    andrectregion(new,&clip);

					    if(old->RegionRectangle)
					    {
						/* add damage to old */
						orregionregion(new,old);
					    }
					    else
					    {
						/* swap damage for old */
						cw->DamageList = new;
						new = old;
					    }

					    /* iff layer damaged - bart */
					    if(cw->DamageList->RegionRectangle)
					    {
						/* indicate damage to layer */
						cw->Flags |= REFRESHFLAGS;  
					    }
					}
				    }

				    disposeregion(tmp);
				}

				disposeregion(new);
			    }
			}
		    }

			SortLayerCR(cw,dx,dy);

		    /* initialize clipping for bit transfer */

		    clip.MinX = MINX + cw->bounds.MinX;
		    clip.MinY = MINY + cw->bounds.MinY;
		    clip.MaxX = MAXX + cw->bounds.MinX;
		    clip.MaxY = MAXY + cw->bounds.MinY;

		    for (cr = cw->ClipRect; cr ; cr = cr->Next)
		    {
				int DX,DY;
				/* skip over cliprects with no bitmaps */
#ifdef DEBUG
				printf("process cr = %lx lobs=%lx BitMap=%lx",
				cr,cr->lobs,cr->BitMap);
				Debug();
#endif

				if ( (cr->lobs == 0) || (cr->BitMap != 0))
				{

					/* do internal bit transfer */
				    
					boundsrect.MinX =  clip.MinX;
					boundsrect.MinY =  clip.MinY;
					boundsrect.MaxX =  clip.MaxX;
					boundsrect.MaxY =  clip.MaxY;

					/* if this cliprect is not involved then skip */
					if ( rectXrect(&boundsrect,&cr->bounds))
					{

						/* calculate intersection */
						intersect(&boundsrect,&cr->bounds,&boundsrect);

						DX = boundsrect.MinX - cr->bounds.MinX;
						DY = boundsrect.MinY - cr->bounds.MinY;

						width = boundsrect.MaxX - boundsrect.MinX + 1 - absdx;
						height = boundsrect.MaxY - boundsrect.MinY + 1 - absdy;
#ifdef DEBUG
    printf("do internal bit transfer w=%ld h=%ld\n",width,height);
#endif
						if ( (width > 0) && (height > 0))
						{
						    if (cr->lobs == 0)  /* screen blit */
							gfx_BltBitMap(bm,
								srcx+boundsrect.MinX,srcy+boundsrect.MinY,
								bm,dstx+boundsrect.MinX,dsty+boundsrect.MinY,
								width,height,COPY,rp->Mask,TempA);
						    else
						    {   /* obscured blit */
							gfx_BltBitMap(cr->BitMap,
							    srcx+(cr->bounds.MinX&15)+DX,srcy+DY,
							    cr->BitMap,
							    dstx+(cr->bounds.MinX&15)+DX,dsty+DY,
							    width,height,COPY,rp->Mask,TempA);
						    }
						}

					/* look at rest of cliprects and extract any needed bits */
					for (crn = cr->Next ; crn ; crn = crn->Next)
					{
#ifdef DEBUG
					    printf(" check crn=%lx ,",crn);
#endif
					    tmprect.MinX = crn->bounds.MinX;
					    tmprect.MinY = crn->bounds.MinY;
					    tmprect.MaxX = crn->bounds.MaxX;
					    tmprect.MaxY = crn->bounds.MaxY;

					    if(cw->DamageList) /* layers is special -- bart */
					    {
						if(!rectXrect(&tmprect,&clip))
						{ 
							continue;
						}
						else
						{
							intersect(&tmprect,&clip,&tmprect);
						}
					    }

					    tmprect.MinX -= dx;
					    tmprect.MinY -= dy;
					    tmprect.MaxX -= dx;
					    tmprect.MaxY -= dy;

					    /* look for bits to get */
					    if (rectXrect(&tmprect,&boundsrect))
					    {
							/* calculate intersection */
#ifdef DEBUG
    printf("now calculate intersection for (%lx,%lx)\n",cr,crn);
#endif
						intersect(&tmprect,&boundsrect,&tmprect);

						width = tmprect.MaxX-tmprect.MinX + 1;
						height = tmprect.MaxY-tmprect.MinY + 1;
						if (cr->lobs == 0)
						{
						    if (crn->lobs == 0)
							/* copy screen-> screen */
							gfx_BltBitMap(bm,tmprect.MinX + dx,
								tmprect.MinY + dy,
							bm,tmprect.MinX,tmprect.MinY,
							width,height,COPY,rp->Mask,TempA);
						    else
						    {   /* copy obscured->screen */
							
								if ( crn->BitMap )
								{
									gfx_BltBitMap(crn->BitMap,
									tmprect.MinX + dx-(crn->bounds.MinX&(~15)),
									tmprect.MinY + dy - crn->bounds.MinY,
									bm,tmprect.MinX,tmprect.MinY,
									width,
									height, 
									COPY,rp->Mask,TempA);
								}
								else    /* trigger refresh needed */
								if(cw->DamageList)
								{
#ifdef DEBUG
printf("trigger refresh\n");
#endif
									tmprect.MinX -= cw->bounds.MinX;
									tmprect.MinY -= cw->bounds.MinY;
									tmprect.MaxX = tmprect.MinX + width -1;
									tmprect.MaxY = tmprect.MinY + height -1;
									orrectregion(cw->DamageList,&tmprect);
									cw->Flags |= REFRESHFLAGS;  
								}
						    }
						}
						else
						{
						    /* cr is obscured */
						    if (crn->lobs == 0)
						    {
#ifdef DEBUG2
    printf("copy screen to obscured\n");
    printf("call BLTBITMAP(%lx,%ld,%ld,%lx,%ld,%ld,%ld,%ld,%lx\n",
				bm,tmprect.MinX + dx,
					tmprect.MinY + dy,
				cr->BitMap,tmprect.MinX - (cr->bounds.MinX&(~15)),
				tmprect.MinY - cr->bounds.MinY,
				width,height,COPY);
    Debug();
#endif
				/* copy screen-> obscured */
				gfx_BltBitMap(bm,tmprect.MinX + dx,
					tmprect.MinY + dy,
				cr->BitMap,tmprect.MinX - (cr->bounds.MinX&(~15)),
				tmprect.MinY - cr->bounds.MinY,
				width,height,COPY,rp->Mask,TempA);
			    }
			    else
			    {   /* copy obscured->obscured */
				gfx_BltBitMap(crn->BitMap,
				    tmprect.MinX + dx-(crn->bounds.MinX&(~15)),
				    tmprect.MinY + dy - crn->bounds.MinY,
				cr->BitMap,tmprect.MinX - (cr->bounds.MinX&(~15)),
				tmprect.MinY - cr->bounds.MinY,
				width,height,COPY,rp->Mask,TempA);
			    }
			}
		    }
		}
		}   /* rectXrect */
		}   /* obs,Bitmap */
	    }
	}
	UNLOCKLAYER(cw);
    }
#ifdef DEBUGE
    printf("]");
#endif
    /* now clear the area of vacated */
    {
	struct RastPort rpnew;
	gfx_InitRastPort(&rpnew);
	rpnew.BitMap = rp->BitMap;
	rpnew.Layer = rp->Layer;
	rpnew.Mask = rp->Mask;
	gfx_SetAPen(&rpnew,rp->BgPen);

	if (dy > 0) 
	    (*fillptr)(&rpnew,MINX,SHORTMAX(MINY,maxy + 1 - dy),maxx,maxy);
	else if (dy < 0)
	    (*fillptr)(&rpnew,MINX,MINY,maxx,MINY+absdy - 1);
	if (dx > 0) 
	    (*fillptr)(&rpnew,SHORTMAX(MINX,maxx + 1 - dx),MINY,maxx,maxy);
	else if (dx < 0)
	    (*fillptr)(&rpnew,MINX,MINY,MINX + absdx - 1,maxy);
    }

RETURN:

    if (dy == 0) if (!using_tmpras)	freemustmem(TempA,MAXBYTESPERROW);
#ifdef DEBUG
    printf("\nreturn rscroll\n");
#endif
}



#endif /* MAKE_PROTOS */

void __asm ScrollRaster(register __a1 struct RastPort *rp,register __d0 short dx,register __d1 short dy,
		  register __d2 short MINX,register __d3 short MINY,register __d4 short MAXX,
		  register __d5 short MAXY)

{
    scroll_primitive(rp,dx,dy,MINX,MINY,MAXX,MAXY,RectFill);
}

void __asm ScrollRasterBF(register __a1 struct RastPort *rp,register __d0 short dx,register __d1 short dy,
		  register __d2 short MINX,register __d3 short MINY,register __d4 short MAXX,
		  register __d5 short MAXY)

{
    scroll_primitive(rp,dx,dy,MINX,MINY,MAXX,MAXY,EraseRect);
}
