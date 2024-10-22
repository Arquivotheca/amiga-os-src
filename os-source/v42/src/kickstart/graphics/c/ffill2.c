/******************************************************************************
*
*	$Id: ffill2.c,v 42.0 93/06/16 11:15:26 chrisg Exp $
*
******************************************************************************/

/* ffill.c -- flood fill using dumb pixel retd routines
 *
 *
 */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

#include "ffill.h"
#include "c.protos"

/*#define DEBUG*/
/*#define BLOCKDEBUG*/

#define NEWCODE
#define NEWCODE2
#define  FFBUG

#define DOWNCODE

#ifndef DOWNCODE
USHORT  *address(bm,x,y)
struct BitMap *bm;
{
    return((USHORT *)(bm->Planes[0] +
	 umuls(bm->BytesPerRow,y) + 
	( (x>>3) & 0xFFFE)));
}
#endif

static __regargs EXTREM(struct flood_info *fi, SHORT x, SHORT y, SHORT *left, SHORT *right);
static __regargs EXPLOR(struct flood_info *fi, SHORT x1, SHORT x2, SHORT YCHECK, SHORT direction );

/*********************************************************************/
/* SEARCHCOLOR is used to determine boundaries of a region */

__regargs fillfast_region(struct RastPort *rp, struct RastPort *destrp,  SHORT x, SHORT ty )        /* FILL REGION WITH COLOR */
{
    SHORT left, right, y, direction;
    SHORT WORKY, nuleft, nuright;
    struct flood_info flood_info,*fi;
    int error = FALSE;

#ifdef DEBUG
    printf("fill_region(%lx,%ld,%ld)\n",rp,x,ty);
#endif

    y = ty;

    /*  abort if starting at SEARCHCOLOR */
    if( READPIXEL(rp, x, y ) == SEARCHCOLOR ) return(TRUE);

    /* initialize flood info structure */
    fi = &flood_info;

    init_fi(fi,rp);

/*  scan current line, and save limits. */
    EXTREM(fi, x, y, &left, &right);
    if (!(PUSH( fi, left, right, y,  1 ))) error = TRUE;

/*  scan above   line, and save limits. */
    x = left;        /* bart - added this line 08.05.85 */
    y -= 1;
    if (y >=  fi->MinY)
    {
	/* line is on screen. find new limits. */
	if (EXTREM(fi, x, y, &nuleft, &nuright))
	{
#ifdef DEBUG
     printf("scan of above line-> left=%ld,right=%ld\n",nuleft,nuright);
#endif

	    if ( (nuleft <= right)&&(nuright >= left))
	    {
		/* bart - 08.05.85 - modified the following line for bugfix */
		/* if ( (x <= nuright) && (x >= nuleft) ) */
		if (!(PUSH( fi, nuleft, nuright, y, -1 ))) error = TRUE;
		if ( (nuleft < left) || (nuright > right) )
		  if (!(PUSH( fi, nuleft, nuright,y, 1))) error = TRUE;

		/* bart - bugfix 08.29.85 */
		/* check if this newright does not extend all the way right */
		/* to previous right: there may be other connecting paths */
		/* in this direction */

		if( nuright < right)
		{
		   /* bart - 04.07.86 removed following line - handled above */
		   /* if (!(EXPLOR(fi, nuright, right, y, 1))) error = TRUE; */
		   /* bart - 04.07.86 still need the next line */
		   /* bart - 06.23.86 adjust right limit */
		   /* if (!(EXPLOR(fi, nuright, right+1, y, -1))) error = TRUE; */

		   /* bart - 07.22.86 adjust nuright limit */
		   if (!(EXPLOR(fi, nuright+1, right+1, y, -1))) error = TRUE;
		   /* bart - 07.22.86 restored following line - not handled above */
		   if (!(EXPLOR(fi, nuright+1, right+1, y, 1))) error = TRUE;
		}

		/* check if this newleft does not extend all the way left */
		/* to previous left: there may be other connecting paths */
		/* in this direction */

		if( nuleft > left)
		{
		   /* bart - 04.07.86 removed following line - handled above */
		   /* if (!(EXPLOR(fi, left, nuleft, y, 1))) error = TRUE; */
		   /* bart - 04.07.86 still need the next line */
		   if (!(EXPLOR(fi, left, nuleft, y, -1))) error = TRUE;
		}
		/* end bart - 08.29.85 */

	    }


	}

    }

  while( fi->top )
  {
/* get and draw the next line */
    POP( fi, &left, &right, &y, &direction );
	MOVE(rp, left, y );
	DRAW(rp, right, y );
#ifdef DEBUG
	printf("Line from %ld to %ld row=%ld\n",left,right,y);
#endif
    if (rp != destrp)
    {
	MOVE(destrp, left, y );
	DRAW(destrp, right, y );
    }
/* wait after line is drawn */

/* scan next line if it is on-screen. */
    WORKY = y + direction;
    if( (WORKY >= YMIN)  &&  (WORKY <= YMAX) )
    {

/* line is on screen. find new limits. */
	if (EXTREM(fi, left, WORKY, &nuleft, &nuright ))

/*   break if left and right margins cross,
   or if the working line is already filled. */

	if( (nuleft <= right)  &&  (READPIXEL(rp, nuleft, WORKY ) != FILLCOLOR))
	{

/* bart - bugfix 08.29.85 */
	    /* check if this newright does not extend all the way right */
	    /* to previous right: there may be other connecting paths */
	    /* in this direction */

	    if( nuright < right)
	       /* EXPLOR(fi, nuright, right, WORKY, -direction ); */

	       /* bart - 09.30.85 */
	       {
		   		/* bart - 04.07.86 removed following line - handled above */
		   		/* if (!(EXPLOR(fi, nuright, right+1, WORKY, -direction )))
		     		error = TRUE; */

		   		/* bart - 04.07.86 still need the next line */
		   		/* bart - 04.08.86 little bugfix? - replace next line */
		   		/* if (!(EXPLOR(fi, nuright+1, right+1, WORKY, direction )))
		     		error = TRUE; */
		   		/* bart - 06.23.86 limits for EXPLOR? - add one to right */
		   		/* if (!(EXPLOR(fi, nuright, right+1, WORKY, direction )))
		     		error = TRUE; */

		   		/* bart - 07.22.86 limits for EXPLOR? - add one to nuright */
		   		if (!(EXPLOR(fi, nuright+1, right+1, WORKY, direction )))
		     		error = TRUE;
		   		/* bart - 07.22.86 */
				/* restored following line - not handled above after all */
		   		if (!(EXPLOR(fi, nuright+1, right+1, WORKY, -direction )))
		     		error = TRUE;

	       }
	       /* end bart - 09.30.85 */

	    /* check if this newleft does not extend all the way left */
	    /* to previous left: there may be other connecting paths */
	    /* in this direction */

		/* bart - 07.22.86 changed -direction to +direction */
	    if( nuleft > left)
	       if (!(EXPLOR(fi, left, nuleft, WORKY, direction )))
		 error = TRUE;

/* end bart - 08.29.85 */

/*  explore for ever widening horizons */

	    if( nuleft < --left)
	       if (!(EXPLOR(fi, nuleft, left, y, -direction )))
		 error = TRUE;

	    if( nuright > ++right)
	       if (!(EXPLOR(fi, right, nuright+1, y, -direction )))
		 error = TRUE;
	       /* if (!(EXPLOR(fi, right, nuright, y, -direction )))
		 error = TRUE; */ /* bart - 05.09.86 bugfix */

	    if(!(PUSH( fi, nuleft, nuright, WORKY, direction ))) error = TRUE;

/* bart - 04.07.86 removed following chunk... unnecessary ? */
/* bart - 07.22.86 restored following chunk... it WAS necessary! */

		/* bart - bugfix 08.29.85 */
	    if ( (nuleft < left) || (nuright > right) )
	      if (!(PUSH( fi, nuleft, nuright, WORKY, -direction))) error = TRUE;
		/* end bart - 08.29.85 */

	}   /* end while margins not crossed AND scan line is NOT filled */

    }       /* end while working line is on screen */

  }         /* end while stack not resolved */

    /* bart - 10.09.85 removed to make way for blocklist cleanup code */
    /* free off the unused freelist nodes */
    /*
    while ( p = fi->freelist)
    {
	fi->freelist = p->Next;
	FreeMem(p,sizeof(struct stacknode));
    }
    */

    /* bart - 10.09.85 blocklist cleanup code */

#ifdef BLOCKDEBUG
    printf("fill_region: about to call CLEANUP\n");
    Debug();
#endif

    CLEANUP(fi);

    if (error) return(FALSE);
    else return(TRUE);
    
}           /* end fill_region  */

/*********************************************************************/
static __regargs EXPLOR(fi, x1, x2, YCHECK, direction ) /* scan left to right */
register struct flood_info *fi;
SHORT x1, x2, YCHECK, direction;
{
    SHORT x, left, right;
    register USHORT data,mask,*p;

#ifdef DEBUG
    printf("EXPLOR : L %ld, R %ld, Y %ld, DIR %ld",x1, x2, YCHECK, direction );
#endif

    p = (SHORT *)address(fi->bm,x1,YCHECK);
    mask = 0x8000 >> (x1 & 15);
    data = *p;

    for( x=x1; x<x2; x++ )
    {
	if ( (data & mask) == 0)
	{
	    EXTREM(fi, x, YCHECK, &left, &right );
	    if (!(PUSH( fi, left, right, YCHECK, direction ))) return(FALSE);
	    x = right + 1;
	    p = (SHORT *)address(fi->bm,x,YCHECK);
	    mask = 0x8000 >> (x & 15);
	    data = *p;
	}
	mask >>= 1;
	if (mask == 0)
	{
	    mask = 0x8000;
	    while ( (~(data = *++p)) == 0)
	    {
		x += 16;
		if (x >= x2) break;
	    }
	}
    }
    return(TRUE);
}

/*********************************************************************/
static __regargs EXTREM(fi, x, y, left, right)        /* find left and right margins */
struct flood_info *fi;
SHORT y, *left, *right;
register SHORT x;
{
    register USHORT *p,mask,data;
    SHORT ctr=XMAX;

#ifdef DEBUG
    printf("(%lx,%lx,%ld,%ld)",rp,rp->BitMap->Planes[0],x,y);
#endif
    p = (SHORT *)address(fi->bm,x,y);
    mask = 0x8000 >> (x & 15);
    data = *p;
#ifdef DEBUG
    printf("x=%lx,p = %lx,mask=%lx,data=%lx",x,p,mask,data);
#endif

    if (data & mask)	/* if starting on outline */
	while (data & mask)	/* find the left margin,search rightward */
	{
	    if (x > XMAX) return(FALSE);	/* could not find a left margin */
	    x++;
	    ctr = x;
	    mask >>= 1;
	    if (mask == 0)
	    {
		while ( (~(data = *++p)) == 0)	/* look 16 bits at a time */
		{
		    x += 16;
		    if ( x >= XMAX)
		    {
			x -= 16;
			ctr = x;
			mask = 0x8000; /* bart - 09.18.85 */
			break;
		    }
		}
		ctr = x;
		mask = 0x8000;
	    }
	}
    else
	while ( (data & mask) == 0)
	{
	    ctr = x;
	    if ( x == XMIN) break;
	    --x;
	    mask <<= 1;
	    if (mask == 0)
	    {
		while ( (data = *--p) == 0)
		{
		    x -= 16;
		    if (x <= XMIN) /* bart - 09.18.85 */
		    {
			x += 16;
			mask = 1; /* bart - 09.18.85 */
			break;
		    }
		    /* ctr = x; */ /* bart - 09.18.85 */
		    ctr -= 16;
		}
		mask = 1;
	    }
	}
    *left = x = ctr;                /* starting from the left margin,    */
    p = (SHORT *)address(fi->bm,x,y);
    mask = 0x8000 >> (x & 15);
    data = *p;
    do
    {
	ctr = x;
	if (x == XMAX)  break;
	x++;
	mask >>= 1;
	if (mask == 0)
	{
	    while ( (data = *++p) == 0)
	    {
		x += 16;
		if ( x >= XMAX )
		{
		    x -= 16;
		    break;
		}
		ctr = x-1;
	    }
	    mask = 0x8000;
	}
    }   while ( (data & mask) == 0);
    *right = ctr;
#ifdef DEBUG
    printf("\nEXTREM : L %ld, R %ld, Y %ld", *left, *right, y );
#endif
	return(TRUE);
}

