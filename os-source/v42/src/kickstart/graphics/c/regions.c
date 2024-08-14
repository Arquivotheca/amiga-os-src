/******************************************************************************
*
*	$Id: regions.c,v 39.0 91/08/21 17:21:25 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <graphics/gfx.h>
#include "/macros.h"
#include "c.protos"

/*#define DEBUG*/
/*#define XORDEBUG*/
/*#define DEBUGAND*/

#include <graphics/regions.h>
#define OPERATION_OR    0
#define OPERATION_AND	1

#ifdef OLDRECTSPLIT

#define OPERATION_XOR	2
#define OPERATION_CLEAR	3

#endif

#define AN_REGIONMEMORY	0x8201000B

#define DOWNREGIONS

#ifndef DOWNREGIONS

obscured(r1,r2)
struct Rectangle *r1,*r2;
{
#ifdef DEBUG
    printf("obscured(%lx,%lx)\n",r1,r2);
#endif
    if ( (r1->MinX <= r2->MinX) &&
     (r1->MaxX >= r2->MaxX) &&
     (r1->MinY <= r2->MinY) &&
     (r1->MaxY >= r2->MaxY) )    return(TRUE);
    else                            return(FALSE);
}


freerr(rr)
struct RegionRectangle *rr;
{
#ifdef DEBUG
    printf("freerr(%lx)\n",rr);
#endif
	FreeMem(rr,sizeof(struct RegionRectangle));
}

struct RegionRectangle *newrect()
{
	return( (struct RegionRectangle *)
					AllocMem(sizeof(struct RegionRectangle),MEMF_CLEAR));
}

struct Region *NewRegion()
{
	return(
    	(struct Region *)AllocMem(sizeof(struct Region),MEMF_CLEAR));
}

#ifdef OLDRECTSPLIT
rectsplit(rgn,r1,r2,op)
struct Region *rgn;
struct Rectangle r1,*r2;
{
    /* split r1 in pieces that do not overlap r2 */
#ifdef DEBUG
    printf("rectsplit(%lx[%x,%x,%x,%x][%x,%x,%x,%x] op:%lx \n",rgn,r1,r2,op);
#endif
    if (rectXrect(&r1,r2))
    {
#ifdef DEBUG
    printf(" they cross over\n");
#endif
		if (r1.MinX < r2->MinX)
		{
		    struct Rectangle temp;
		    temp = r1;
		    temp.MaxX = r2->MinX - 1;
		    rectsplit(rgn,temp,r2,op);
		    r1.MinX = r2->MinX;
		}
		if (r1.MaxX > r2->MaxX)
		{
		    struct Rectangle temp;
		    temp = r1;
		    temp.MinX = r2->MaxX + 1;
		    rectsplit(rgn,temp,r2,op);
		    r1.MaxX = r2->MaxX;
		}
		if (r1.MinY < r2->MinY)
		{
		    struct Rectangle temp;
		    temp = r1;
		    temp.MaxY = r2->MinY - 1;
		    rectsplit(rgn,temp,r2,op);
		    r1.MinY = r2->MinY;
		}
		if (r1.MaxY > r2->MaxY)
		{
		    struct Rectangle temp;
		    temp = r1;
		    temp.MinY = r2->MaxY + 1;
		    rectsplit(rgn,temp,r2,op);
		    r1.MaxY = r2->MaxY;
		}
		/* what is left is inside r2 */
#ifdef DEBUG
    printf("what is left is inside\n");
#endif
		/* for OR we do not want the overlap */
		if (op == OPERATION_OR) return;
		/* if (op == OPERATION_CLEAR)	return; */
		/* op == OPERATION_AND */
		goto appendit;
		/*
		{
			struct RegionRectangle *nr;
			nr = newrect();
			if (!nr)	Alert(AN_REGIONMEMORY);
			nr->bounds = r1;
			appendrr(rgn,nr);
			return;
		}
		*/
    }
    /* if we get here then r1 is outside of r2 */
	if (op == OPERATION_AND)	return;	/* don't want this if anding */
	/* for OPERATION_OR,OPERATION_CLEAR */
appendit:
    {
		struct RegionRectangle *nr;
		nr = newrect();
		if (!nr)	Alert(AN_REGIONMEMORY);
		nr->bounds = r1;
		appendrr(rgn,nr);
    }
}

#else

rectsplit(rgn,r1,r2,op)
struct Region *rgn;
struct Rectangle r1,*r2;
{
    struct RegionRectangle *nr = NULL; 
    int xrect = rectXrect( &r1, r2 );

    switch( op )
    {
	case( OPERATION_OR ):
	{
	    if( xrect )
	    {
		if( r2->MaxX < r1.MaxX ) 
		{
		    if(nr = newrect())
		    {
			nr->bounds = r1;
			nr->bounds.MinX = r2->MaxX+1;
			appendrr(rgn,nr);
			r1.MaxX = r2->MaxX;
		    }
		}
		if( r2->MinX > r1.MinX )
		{
		    if(nr = newrect())
		    {
			nr->bounds = r1;
			nr->bounds.MaxX = r2->MinX-1;
			appendrr(rgn,nr);
			r1.MinX = r2->MinX;
		    }
		}
		if( r2->MaxY < r1.MaxY ) 
		{
		    if(nr = newrect())
		    {
			nr->bounds = r1;
			nr->bounds.MinY = r2->MaxY+1;
			appendrr(rgn,nr);
			r1.MaxY = r2->MaxY;
		    }
		}
		if( r2->MinY > r1.MinY )
		{
		    if(nr = newrect())
		    {
			nr->bounds = r1;
			nr->bounds.MaxY = r2->MinY-1;
			appendrr(rgn,nr);
			r1.MinY = r2->MinY;
		    }
		}
	    }
	    else
	    {
		if(nr = newrect())
		{
		    nr->bounds = r1;
		    appendrr(rgn,nr);
		}
	    }
	}   break;
	case( OPERATION_AND ):
	{
	    if( xrect )
	    {
		if(nr = newrect())
		{
		    if( r2->MaxX < r1.MaxX ) r1.MaxX = r2->MaxX;
		    if( r2->MinX > r1.MinX ) r1.MinX = r2->MinX;
		    if( r2->MaxY < r1.MaxY ) r1.MaxY = r2->MaxY;
		    if( r2->MinY > r1.MinY ) r1.MinY = r2->MinY;
		    nr->bounds = r1;
		    appendrr(rgn,nr);
		}
	    }
	}   break;
    }
}

#endif

#endif !DOWNREGIONS


#ifndef DOWNREGIONS

/*
OffsetRectangle(r,x,y)
struct Rectangle *r;
short x,y;
{
	r->MinX += x;
	r->MinY += y;
	r->MaxX += x;
	r->MaxY += y;
}
*/

OffsetRegionRectangle(r,x,y)
struct RegionRectangle *r;
{
    r->bounds.MinX -= x;
    r->bounds.MaxX -= x;
    r->bounds.MinY -= y;
    r->bounds.MaxY -= y;
    /*OffsetRectangle(&r->bounds,-x,-y);*/
}

adjustregionrectangles(rgn,x,y)
struct Region *rgn;
{
    struct RegionRectangle *r;
    for (r = rgn->RegionRectangle; r ; r = r->Next) OffsetRegionRectangle(r,x,y);
}

#endif !DOWNREGIONS

#ifndef DOWNREGIONS

adjustregion(newrgn,rgn,rect)
struct Region *newrgn,*rgn;
struct Rectangle *rect;
{
    /* first calculate new bounds */
    if (rgn->RegionRectangle == 0)   newrgn->bounds = *rect;
    else
    {
	newrgn->bounds.MinX = SHORTMIN(rgn->bounds.MinX,rect->MinX);
	newrgn->bounds.MinY = SHORTMIN(rgn->bounds.MinY,rect->MinY);
	newrgn->bounds.MaxX = SHORTMAX(rgn->bounds.MaxX,rect->MaxX);
	newrgn->bounds.MaxY = SHORTMAX(rgn->bounds.MaxY,rect->MaxY);
		    
	/* do I need to readjust all Rectangle currently there? */
	if ( (newrgn->bounds.MinX != rgn->bounds.MinX) ||
	     (newrgn->bounds.MinY != rgn->bounds.MinY) )
	{
	    adjustregionrectangles(rgn,newrgn->bounds.MinX-rgn->bounds.MinX,
				       newrgn->bounds.MinY-rgn->bounds.MinY);
	}
    }
    newrgn->RegionRectangle = 0;
    rgn->bounds = newrgn->bounds;
}

appendrr(rgn,r)
struct Region *rgn;
struct RegionRectangle *r;
{
    struct RegionRectangle *p,*q;
#ifdef DEBUG
    printf("prependrr(%lx,%lx)\n",rgn,r);
#endif
    q = &rgn->RegionRectangle;
    while (p = q->Next) q = p;
    q->Next = r;
    r->Next = 0;
    r->Prev = q;
}

prependrr(rgn,r)
struct Region *rgn;
struct RegionRectangle *r;
{
    struct RegionRectangle *p,*q;
#ifdef DEBUG
    printf("prependrr(%lx,%lx)\n",rgn,r);
#endif
    q = r->Prev = &rgn->RegionRectangle;
    if(p = r->Next = q->Next)
    {
	p->Prev = r;
    }
    q->Next = r;
}

removerr(rgn,r)
struct Region *rgn;
struct RegionRectangle *r;
{
#ifdef DEBUG
    printf("removerr(%lx,%lx)\n",rgn,r);
#endif
    if (r->Next) r->Next->Prev = r->Prev;
    if (r->Prev == &rgn->RegionRectangle)
    {
	rgn->RegionRectangle = r->Next;
    }
    else
    {
	r->Prev->Next = r->Next;
    }
}

orrectregion(rgn,rect)
struct Region *rgn;
struct Rectangle *rect;
{
    struct RegionRectangle *r,*r1,*r2;
    struct Region newrgn;
    
#ifdef DEBUG
	printf("OrRectRegion(%lx,%d,%d,%d,%d)",rgn,*rect);
#endif
	/* first calculate new bounds */
	adjustregion(&newrgn,rgn,rect);
    /* now everyone is in the same coordinate system */
    if (r = newrect())
    {
	    prependrr(&newrgn,r);
    	r->bounds = *rect;  /* copy rectangle into real storage */
    	OffsetRegionRectangle(r,rgn->bounds.MinX,rgn->bounds.MinY);
    	/* for r = all cliprects in current region */
    	for (r = rgn->RegionRectangle; r ; r = r->Next)
    	{
	    /* for r1 = all cliprects in tmp region list */
	    for (r1 = newrgn.RegionRectangle ; r1 ; r1 = r2)
	    {
		r2 = r1->Next;
#ifdef DEBUG
		printf("checking r=%lx r1=%lx r2=%lx\n",r,r1,r2);
#endif
		if (rectXrect(&r->bounds,&r1->bounds))
		{
		    if (!obscured(&r->bounds,&r1->bounds) )
		    {   /* need to cut it up into parts */
			rectsplit(&newrgn,r1->bounds,&r->bounds,OPERATION_OR);
			if (r2 == 0)	r2 = r1->Next;
		    }
		    removerr(&newrgn,r1);
		    freerr(r1);
#ifdef DEBUG
		    printf("nr.rr=%lx\n",newrgn.RegionRectangle);
#endif
		}
	    }
    	}
#ifdef DEBUG
	printf("anything left? %lx\n",newrgn.RegionRectangle);
#endif
    	/* add whatever is left to list */
    	for (r = newrgn.RegionRectangle ; r ; r = r1 )
    	{
	    r1 = r->Next;
	    prependrr(rgn,r);
    	}
#ifdef DEBUG
	printf("orrectregion returning\n");
#ifdef DEBUGDEBUG
	Debug();
#endif
#endif
	return (TRUE);
    }
    return (FALSE);
}

andrectregion(rgn,rect)
struct Region *rgn;
struct Rectangle *rect;
{
    struct RegionRectangle *r,*r1;
    struct Region newrgn;
	struct RegionRectangle rr_tmp;
    
#ifdef DEBUGAND
	printf("AndRectRegion(%lx,%d,%d,%d,%d)",rgn,*rect);
#ifdef DEBUGDEBUG
	Debug();
#endif
#endif
	/* first calculate new bounds */
	newrgn.RegionRectangle = 0;
    if (rgn->RegionRectangle == 0)	return;		/* null region */
    else
    {
		/* intersect(&rgn->bounds,&rect,&newrgn.bounds); */ 
		/* bart found this here bug ... */
		intersect(&rgn->bounds,rect,&newrgn.bounds);
		    
		/* do I need to readjust all Rectangle currently there? */
		if ( (newrgn.bounds.MinX != rgn->bounds.MinX) ||
		     (newrgn.bounds.MinY != rgn->bounds.MinY) )
		{
		    adjustregionrectangles(rgn,newrgn.bounds.MinX-rgn->bounds.MinX,
					     newrgn.bounds.MinY-rgn->bounds.MinY);
		}
		rgn->bounds = newrgn.bounds;
    }
    /* now everyone is in the same coordinate system */
	r = &rr_tmp;
	r->bounds = *rect;
    OffsetRegionRectangle(r,rgn->bounds.MinX,rgn->bounds.MinY);

    /* for r = all cliprects in current region */
    for (r = rgn->RegionRectangle; r ; r = r1)
    {
	r1 = r->Next;
	if (rectXrect(&rr_tmp.bounds,&r->bounds))
	{
	    if (!obscured(&r->bounds,&rr_tmp.bounds) )
	    {   /* need to cut it up into parts */
		if (obscured(&rr_tmp.bounds,&r->bounds))
		{
		    removerr(rgn,r);
		    prependrr(&newrgn,r);
		}
		else 
		{
		    rectsplit(&newrgn,rr_tmp.bounds,&r->bounds,OPERATION_AND);
		}
	    }
	    else
	    {
		/* accept completely as the NewRegion this new rect */
		r = newrgn.RegionRectangle = newrect();
		if (!r)	Alert(AN_REGIONMEMORY);
		r->bounds = rr_tmp.bounds;
		break;
	    }
	}
    }
    /* AND is now in NewRegion */
    /* we now clean out the old list */
    replaceregion(&newrgn,rgn);
    /* now must adjust to minimums again */
#ifdef DEBUGAND
    printf("return from andrectregion\n");
#ifdef DEBUGDEBUG
    Debug();
#endif
#endif
}

#endif !DOWNREGIONS

#ifndef DOWNREGIONS

replaceregion(from,to)
struct Region *from,*to;
{
	clearregion(to);
	*to = *from;
	/* now patch up backwards pointer */
	if (to->RegionRectangle)
		to->RegionRectangle->Prev = &to->RegionRectangle;
	from->RegionRectangle = 0;
	SettleDown(to);
}

#ifdef QWE
SettleDown(rgn)
struct Region *rgn;
{
	/*  check all offsets, make sure they are at minimum */
	short x,y;
	short xmax,ymax;
	struct RegionRectangle *r;

	y = x = 15000;
	xmax = ymax = -1;
	for (r = rgn->RegionRectangle; r ; r = r->Next)
	{
		x = SHORTMIN(x,r->bounds.MinX);
		y = SHORTMIN(y,r->bounds.MinY);
		xmax = SHORTMAX(xmax,r->bounds.MaxX);
		ymax = SHORTMAX(ymax,r->bounds.MaxY);
	}
	/* we now have minimum, reoffset all */
	adjustregionrectangles(rgn,x,y);
	rgn->bounds.MinX = x;
	rgn->bounds.MaxX = xmax;
	rgn->bounds.MinY = y;
	rgn->bounds.MaxY = ymax;
}
#else
SettleDown(rgn)
struct Region *rgn;
{
    /*  check all offsets, make sure they are at minimum */
    register short x,y;
    struct RegionRectangle *r;

    y = x = 15000;
    for (r = rgn->RegionRectangle; r ; r = r->Next)
    {
        x = SHORTMIN(x,r->bounds.MinX);
        y = SHORTMIN(y,r->bounds.MinY);
    }
    /* we now have minimum, reoffset all */
    adjustregionrectangles(rgn,x,y);
	/*OffsetRectangle(&rgn->bounds,x,y);*/
    rgn->bounds.MinX += x;
    rgn->bounds.MaxX += x;
    rgn->bounds.MinY += y;
    rgn->bounds.MaxY += y;
}
#endif

clearregion(rgn)
struct Region *rgn;
{
    struct RegionRectangle *r,*r1;
    rgn->bounds.MinX = 0;
    rgn->bounds.MinY = 0;
    rgn->bounds.MaxX = 0;
    rgn->bounds.MaxY = 0;
    for (r = rgn->RegionRectangle; r ; r = r1)  
    {
		r1 = r->Next;
		freerr(r);
    }
    rgn->RegionRectangle = 0;
}

#endif

#ifndef DOWNREGIONS

disposeregion(rgn)
struct Region *rgn;
{
    struct RegionRectangle *r,*r1;
#ifdef DEBUG
	printf("disposeregion(%lx)\n",rgn);
#endif
	clearregion(rgn);
    FreeMem(rgn,sizeof(struct Region));
}

xorrectregion(rgn,rect)
struct Region *rgn;
struct Rectangle *rect;
{
	/* handle simple case */
	if (rgn->RegionRectangle == 0) return(orrectregion(rgn,rect));
	else
	{
		struct Region outsideregion,insideregion;
		struct RegionRectangle *nrect,*r,*r1,*r2;

		adjustregion(&outsideregion,rgn,rect);
		insideregion = outsideregion;	/* make identical */
		nrect = newrect();
		if (!nrect)	Alert(AN_REGIONMEMORY);
		nrect->bounds = *rect;
		OffsetRegionRectangle(nrect,rgn->bounds.MinX,rgn->bounds.MinY);
		/* is rect in rgn coordinates */
		/* now construct outside list */
#ifdef XORDEBUG
		printf("start xor first loop\n");
#endif
		for (r = rgn->RegionRectangle; r ; r = r->Next)
		{
			/* or in everything outside of both */
			rectsplit(&outsideregion,r->bounds,&nrect->bounds,OPERATION_OR);
		}
		/* now have outsideregion with everything outside of rect */
		/* what is inside now */
#ifdef XORDEBUG
		printf("call prependrr\n");
#endif
		prependrr(&insideregion,nrect);	/* start with everything */
#ifdef DEBUG
		printf("enter second loop\n");
#endif
		for (r = rgn->RegionRectangle; r ; r = r->Next)
		{
			r1 = insideregion.RegionRectangle;
			insideregion.RegionRectangle = 0; /* start from scratch */
			for ( ; r1 ; r1 = r2)
			{
				/* or in everything outside of both */
				rectsplit(&insideregion,r1->bounds,&r->bounds,OPERATION_OR);
				r2 = r1->Next;
				freerr(r1);
			}
		}
		/* now have 2 disjoint regions inside and outside */
#ifdef XORDEBUG
		printf("combine inside and outside regions\n");
#endif
		while  ( r = insideregion.RegionRectangle )
		{
			removerr(&insideregion,r);
			prependrr(&outsideregion,r);
		}
#ifdef DEBUG
		printf("call replace region\n");
#endif
		replaceregion(&outsideregion,rgn);
	}
	return (TRUE);
}

#endif

#ifndef DOWNREGIONS

orregionregion(rgnsrc,rgndst)
struct Region *rgnsrc,*rgndst;
{
    struct RegionRectangle *rr;
    struct Rectangle rect;
    for (rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next)
    {
        rect = rr->bounds;
        rect.MinX += rgnsrc->bounds.MinX;
        rect.MaxX += rgnsrc->bounds.MinX;
        rect.MinY += rgnsrc->bounds.MinY;
        rect.MaxY += rgnsrc->bounds.MinY;
        orrectregion(rgndst,&rect);
    }
    return (TRUE);
}

xorregionregion(rgnsrc,rgndst)
struct Region *rgnsrc,*rgndst;
{
    struct RegionRectangle *rr;
    struct Rectangle rect;
    for (rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next)
    {
        rect = rr->bounds;
        rect.MinX += rgnsrc->bounds.MinX;
        rect.MaxX += rgnsrc->bounds.MinX;
        rect.MinY += rgnsrc->bounds.MinY;
        rect.MaxY += rgnsrc->bounds.MinY;
        xorrectregion(rgndst,&rect);
    }
    return (TRUE);
}

clearrectregion(rgn,rect)
struct Region *rgn;
struct Rectangle *rect;
{
	struct Region *rgntmp;
	if (rgntmp = NewRegion())
	{
		orregionregion(rgn,rgntmp);
		andrectregion(rgntmp,rect);
		xorregionregion(rgntmp,rgn);
		disposeregion(rgntmp);
		return (TRUE);
	}
	return (FALSE);
}

andregionregion(rgnsrc,rgndst)
struct Region *rgnsrc,*rgndst;
{
	struct Region *newrgn,*rgn;
	struct RegionRectangle *rr;
	struct Rectangle rect;
	int out_of_memory = TRUE;
#ifdef DEBUG
	printf("AndRegionRegion(%lx,%lx)\n",rgnsrc,rgndst);
	Debug();
#endif
	newrgn = NewRegion();	/* collect pieces in here */
	if (newrgn == 0)	return(FALSE);
	rgn = NewRegion();
	if (rgn == 0)	goto abortarr;
	for( rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next )
	{
		if (!orregionregion(rgndst,rgn))	goto abortarr;
		rect = rr->bounds;
		rect.MinX += rgnsrc->bounds.MinX;
		rect.MaxX += rgnsrc->bounds.MinX;
		rect.MinY += rgnsrc->bounds.MinY;
		rect.MaxY += rgnsrc->bounds.MinY;
		andrectregion(rgn,&rect);
		if (!orregionregion(rgn,newrgn))	goto abortarr;	/* accumulate the ANDS */
		clearregion(rgn);
	}
	disposeregion(rgn);
	/*copyregion(newrgn,rgndst);*/
	replaceregion(newrgn,rgndst);
	disposeregion(newrgn);
#ifdef DEBUG
	printf("return from AndRegionRegion\n");
	Debug();
#endif
	return(TRUE);
abortarr:
	if (rgn)	disposeregion(rgn);
	if (newrgn)	disposeregion(newrgn);
	return(FALSE);
}

#endif

