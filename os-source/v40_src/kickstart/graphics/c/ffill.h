/******************************************************************************
*
*	$Id: ffill.h,v 39.0 91/08/21 17:17:54 chrisg Exp $
*
******************************************************************************/

/* ffill.c -- flood fill using dumb pixel retd routines */

/*	number of stacknodes in a block */
/*	buy in bulk and save */
#define STKBLKNUM 8

/* 	display	resolution */
#define SEARCHCOLOR	rp->AOlPen
#define FILLCOLOR	rp->FgPen

/*	variables used in fill region process */
#define	XMIN	fi->MinX
#define YMIN	fi->MinY
#define XMAX	fi->MaxX
#define YMAX	fi->MaxY

struct stacknode
{
	struct stacknode *Next;
	ULONG p1234;
};

struct stackblock
{
	struct stackblock *Next;
	short nodecount;
	struct stacknode nodes[STKBLKNUM];
};

struct flood_info
{
	struct stacknode *freelist,*top;
	struct stackblock *blocklist;
	SHORT MinX,MaxX,MinY,MaxY;
	struct RastPort *rp;
	struct BitMap *bm;
};
