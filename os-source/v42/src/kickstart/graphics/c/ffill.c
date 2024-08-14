/******************************************************************************
*
*	$Id: ffill.c,v 42.0 93/06/16 11:15:21 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include "/macros.h"

#include "ffill.h"
#include "c.protos"
/*#define DEBUG*/
/*#define BLOCKDEBUG*/

#ifdef DEBUG
#define KPRINTF
#endif

#ifdef KPRINTF
#define printf kprintf
#endif

static void EXPLOR(struct flood_info *fi, SHORT x1, SHORT x2, SHORT YCHECK, SHORT direction );
static int EXTREM(struct flood_info *fi, SHORT xstart, SHORT y, SHORT *left, SHORT *right);
void init_fi(struct flood_info *fi, struct RastPort *rp);

flood(rp,mode,x,y)
register struct RastPort *rp;
int mode,x,y;
{
    struct Layer *cw;
    int error = FALSE;

#ifdef DEBUG 
    printf("flood: starting flood(%lx,%ld,%ld,%ld)\n",rp,mode,x,y);
#endif

    if (cw = rp->Layer)
    {
	struct BitMap *bm;
	LOCKLAYER(cw);
	if ( bm = cw->SuperBitMap)
	{
	    struct RastPort tmprastport;

	    if(rp->TmpRas != 0)
	    {
#ifdef DEBUG
		printf("superbitmap flood\n");
#endif
		/* bart - 06.27.85 - newfill generates correct mask */
		/* easy just use super bitmap */
		SyncSBitMap(cw);

		/* bart - 01.28.86 no null rp layer pointers! */

		/* bm = rp->BitMap; */
		/* rp->BitMap = cw->SuperBitMap; */
		/* rp->Layer = NULL; */
		/* bart - 01.28.86 clone rp instead... */

		tmprastport = *rp;
		tmprastport.BitMap = bm;
		tmprastport.Layer = NULL;

		if (!(newfill(&tmprastport,0,mode,x,y)))
		{
		    error = TRUE;
		}

		/* rp->Layer = cw; */
		/* rp->BitMap = bm; */

		/* end bart - 01.28.86 no null rp layer pointers! */

		CopySBitMap(cw);
	    }
	    else
	    {
		error = TRUE;
	    }
	}
	else
	{
	    if (rp->TmpRas == 0)
	    {
			if (mode == 0)  fill_region(rp,rp,x,y);
			else
			{
#ifdef DEBUG 
	        	printf("can't fill yet\n");
#endif
				UNLOCKLAYER(cw);
				return(FALSE);
			}
	    }
	    else
	    {
#ifdef DEBUG 
	        	printf("new code\n");
#endif
			/* bart - 06.27.85 - newfill generates correct mask */
			/* bart - 07.13.85 - newfill handles layers correctly */
			if (!(newfill(rp,cw,mode,x,y))) error = TRUE;
	    }
	}
	UNLOCKLAYER(cw);
    }
    else
    {
	if (rp->TmpRas == 0)
	{
	    if (mode == 0)  fill_region(rp,rp,x,y);
	    else            
		{
#ifdef DEBUG
			printf("really can't fill yet\n");
#endif
			return(FALSE);
		}
	}
	else
	{
#ifdef DEBUG 
	    printf("flood: rp->Layer == NULL...\n");
#endif
	    if (!(newfill(rp,0,mode,x,y))) error = TRUE;
	}
    }

#ifdef DEBUG 
    printf("flood: returning from flood...\n");
#endif

    if (error) return(FALSE);
    else return(TRUE);

}


/*********************************************************************/
/* SEARCHCOLOR is used to determine boundaries of a region */

fill_region(rp, destrp,  x, ty )        /* FILL REGION WITH COLOR */
register struct RastPort *rp,*destrp;
SHORT x, ty;
{
    SHORT left, right, y, direction;
    SHORT PIX, WORKY, nuleft, nuright, temp;
    struct flood_info flood_info,*fi;

#ifdef DEBUG
    printf("fill_region(%lx,%ld,%ld)\n",rp,x,ty);
#endif

    y = ty;

    /*  abort if starting at SEARCHCOLOR */
    if(( PIX = READPIXEL(rp, x, y )) == -1) return (-1); /* out of bounds */
    if( PIX == SEARCHCOLOR ) return(TRUE); /* nothing to do */

    /* initialize flood info structure */
    fi = &flood_info;

    init_fi(fi,rp);

/*  scan current line, and save limits. */
    EXTREM(fi, x, y, &left, &right);
    PUSH( fi, left, right, y,  1 );

/*  scan above   line, and save limits. */
    x = left;        /* bart - added this line 08.05.85 */
    y -= 1;
    if (y >=  fi->MinY)
    {
		if (EXTREM(fi, x, y, &nuleft, &nuright))
		{
#ifdef DEBUG
			printf("scan of above line-> left=%ld,right=%ld\n",nuleft,nuright);
#endif
			if ( (nuleft <= right)&&(nuright >= left) )
			{
				/* bart - 08.05.85 - modified the following line for bugfix */
				/* if ( (x <= nuright) && (x >= nuleft) ) */
				  	PUSH( fi, nuleft, nuright, y, -1 );
				if ( (nuleft < left) || (nuright > right) )
	    			PUSH( fi, nuleft, nuright,y, 1);

				/* bart - bugfix 08.29.85 */
				/* check if this newright does not extend all the way right */
				/* to previous right: there may be other connecting paths */
				/* in this direction */

				if( nuright < right)
				{
				   EXPLOR(fi, nuright, right, y, 1);
				   EXPLOR(fi, nuright, right, y, -1);
				}

				/* check if this newleft does not extend all the way left */
				/* to previous left: there may be other connecting paths */
				/* in this direction */

				if( nuleft > left)
				{
				   EXPLOR(fi, left, nuleft, y, 1);
				   EXPLOR(fi, left, nuleft, y, -1);
				}
				/* end bart - 08.29.85 */
			    }
		}
    }

  while( fi->top )
  {
/* get and draw the next line */
    POP( fi, &left, &right, &y, &direction );
    MOVE(destrp, left, y );
    DRAW(destrp, right, y );
    if (rp != destrp)
    {
	MOVE(rp, left, y );
	DRAW(rp, right, y );
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
	       EXPLOR(fi, nuright, right, WORKY, -direction );

	    /* check if this newleft does not extend all the way left */
	    /* to previous left: there may be other connecting paths */
	    /* in this direction */

	    if( nuleft > left)
	       EXPLOR(fi, left, nuleft, WORKY, -direction );
/* end bart - 08.29.85 */

/*  explore for ever widening horizons */
	    if( nuleft < --left)
	       EXPLOR(fi, nuleft, left, y, -direction );

	    if( nuright > ++right)
	       EXPLOR(fi, right, nuright, y, -direction );

/*  explore for ever narrowing horizons */
	    temp = --right - 1;
	    if( nuright < temp)
	       EXPLOR(fi, nuright+2, right, WORKY, direction );

	    PUSH( fi, nuleft, nuright, WORKY, direction );

/* bart - bugfix 08.29.85 */
	    if ( (nuleft < left) || (nuright > right) )
		  PUSH( fi, nuleft, nuright, WORKY, -direction);
/* end bart - 08.29.85 */

	}   /* end while margins not crossed AND scan line is NOT filled */
    }       /* end while working line is on screen */
  }         /* end while stack not resolved */


    /* bart - 10.09.85 removed to make way for blocklist cleanup code */
    /* free off the stack */
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
    return(TRUE); /* sucessful completion */

}           /* end fill_region  */

/*********************************************************************/
static void EXPLOR(fi, x1, x2, YCHECK, direction ) /* scan left to right */
register struct flood_info *fi;
SHORT x1, x2, YCHECK, direction;
{
    SHORT x, PIX, left, right;
    register struct RastPort *rp;
    rp = fi->rp;

#ifdef DEBUG
    printf("EXPLOR : L %ld, R %ld, Y %ld, DIR %ld",x1, x2, YCHECK, direction );
#endif

    for( x=x1; x<x2; x++ )
    {
	PIX = READPIXEL(rp, x, YCHECK );
	if( PIX != FILLCOLOR )
	  if( PIX != SEARCHCOLOR )
	{
	    EXTREM(fi, x, YCHECK, &left, &right );
	    PUSH( fi, left, right, YCHECK, direction );
	    x = right + 1;
	}
    }
}

/*********************************************************************/
static int EXTREM(struct flood_info *fi, SHORT xstart, SHORT y, SHORT *left, SHORT *right)
/* find left and right margins */
{
    SHORT PIX, x;
    register struct RastPort *rp=fi->rp;

    x = xstart;
    *left = XMAX;
    PIX = READPIXEL(rp, x, y );

    if( PIX == SEARCHCOLOR )            /* if starting pixel is the outline, */
    	while( PIX == SEARCHCOLOR )     /* find the left margin to the right.*/
    	{
			if( x > XMAX ) return(FALSE);
			x++;
			*left = x;
			PIX = READPIXEL(rp, x, y );
    	}
    else /* if( PIX != SEARCHCOLOR )    /* if starting pixel is NOT outline, */
    while( PIX != SEARCHCOLOR )         /* find the left margin to the left. */
    {
	*left = x;
	if( x == XMIN ) break;  --x;
	PIX = READPIXEL(rp, x, y );
    }
    PIX = -1;
    x = *left;                      /* starting from the left margin,    */
    while( PIX != SEARCHCOLOR )         /* find the right margin at the right*/
    {
	*right = x;
	if( x == XMAX ) break;  x++;
	PIX = READPIXEL(rp, x, y );
    }
#ifdef DEBUG
    printf("\nEXTREM : L %ld, R %ld, Y %ld", *left, *right, y );
#endif
	return(TRUE);
}

void init_fi(struct flood_info *fi, struct RastPort *rp)
{

    fi->freelist = 0;
	fi->top = 0;
	fi->blocklist = 0;
    fi->rp = rp;
	fi->bm = rp->BitMap;

    fi->MinX = 0;
    fi->MinY = 0;
    fi->MaxX = (rp->BitMap->BytesPerRow<<3)-1;
    fi->MaxY = rp->BitMap->Rows-1;
}
