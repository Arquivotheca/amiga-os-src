/******************************************************************************
*
*	$Id: memory.c,v 42.0 93/06/16 11:16:19 chrisg Exp $
*
******************************************************************************/


#include <exec/types.h>
#include "/gfxbase.h"
#include "/monitor.h"
#include "/macros.h"
#include "/gfxnodes.h"
#include "/view.h"
#include "/gfxpragmas.h"
#include "gfxprivate.h"
#include "c.protos"
#include <exec/nodes.h>
#include <exec/memory.h>
#include <exec/alerts.h>


#define	FORGOTTEN -1

struct AssociatedNode
{
	struct ExtendedNode n;
	APTR	backlink;
};

/* default initialization routine for allocation of graphics */

LONG gfxinit(gfxnode)
struct ExtendedNode *gfxnode;
{
    LONG error =  FALSE;

    if(!gfxnode) 
    {
	error = TRUE;
    }
    return(error);
}

/* routines for allocation and deallocation of graphics */
/* data structures */

struct ExtendedNode * __asm GfxNew(register __d0 ULONG nodetype)
{
    struct ExtendedNode *n;

    switch (nodetype)
    {
	extern LONG gfxnode_init();
	case VIEW_EXTRA_TYPE :
	{
	    if( n = (struct ExtendedNode *) AllocMem(sizeof(struct ViewExtra), MEMF_PUBLIC|MEMF_CLEAR) )
	    {

		/* bart - 11.30.87 extended node */
		/* n->ln_Type = VIEW_EXTRA_TYPE; */
	    
		n->xln_Type = NT_GRAPHICS;
		n->xln_Subsystem = SS_GRAPHICS;
		n->xln_Subtype = VIEW_EXTRA_TYPE;
		n->xln_Library = (LONG) GBASE;
		n->xln_Init = gfxnode_init;

		/* fetch default monitorspec from list */
		{
		    /* assume that we can ALWAYS find the default monitor */
		    {
			struct MonitorSpec *mspc = 
			(struct MonitorSpec *) FindName(&(GBASE->MonitorList),DEFAULT_MONITOR_NAME);

#ifdef BARTDEBUG
			kprintf("gfxnew: default monitorspec @0x%08lx\n",mspc);
#endif
			((struct ViewExtra *)n)->Monitor = mspc;
			((struct ViewExtra *)n)->TopLine = DEFAULTTOPLINE;
		    }
		}
	    }
	}
	break;

	case VIEWPORT_EXTRA_TYPE :
	{
	    if(n = (struct ExtendedNode *) AllocMem(sizeof(struct ViewPortExtra),MEMF_PUBLIC|MEMF_CLEAR))
	    {
		/* bart - 11.30.87 extended node */
		/* n->ln_Type = VIEWPORT_EXTRA_TYPE; */

		n->xln_Type = NT_GRAPHICS;
		n->xln_Subsystem = SS_GRAPHICS;
		n->xln_Subtype = VIEWPORT_EXTRA_TYPE;
		n->xln_Library = (LONG) GBASE;
		n->xln_Init = gfxinit;
	    }
	}
	break;

	case SPECIAL_MONITOR_TYPE:
	{
	    if(n = (struct ExtendedNode *) AllocMem(sizeof(struct SpecialMonitor),MEMF_PUBLIC|MEMF_CLEAR))
	    {
		n->xln_Type = NT_GRAPHICS;
		n->xln_Subsystem = SS_GRAPHICS;
		n->xln_Subtype = SPECIAL_MONITOR_TYPE;
		n->xln_Library = (LONG) GBASE;
		n->xln_Init = gfxinit;
		((struct SpecialMonitor *)n)->do_monitor = (void *) default_monitor;
	    }
	}
	break;

	case MONITOR_SPEC_TYPE:
	{
	    extern LONG monitor_init();
	    extern LONG monitor_transform();
	    extern LONG monitor_translate();
	    extern LONG monitor_scale();
	    extern LONG monitor_bounds();
	    extern LONG video_bounds();
	    extern ULONG mrgcop();
	    extern void loadview();

	    if(n = (struct ExtendedNode *) AllocMem(sizeof(struct MonitorSpec),MEMF_PUBLIC|MEMF_CLEAR))
	    {
		n->xln_Type = NT_GRAPHICS;
		n->xln_Subsystem = SS_GRAPHICS;
		n->xln_Subtype = MONITOR_SPEC_TYPE;
		n->xln_Library = (LONG) GBASE;
		n->xln_Init = monitor_init;

		/* initialize transformation functions */
		((struct MonitorSpec *)n)->ms_transform = monitor_transform;
		((struct MonitorSpec *)n)->ms_translate = monitor_translate;
		((struct MonitorSpec *)n)->ms_scale     = monitor_scale;

		/* initialize transformation offsets */
		((struct MonitorSpec *)n)->ms_xoffset   = STANDARD_XOFFSET;
		((struct MonitorSpec *)n)->ms_yoffset   = STANDARD_YOFFSET;

		/* initialize bounding function */
		((struct MonitorSpec *)n)->ms_maxoscan  = monitor_bounds;
		((struct MonitorSpec *)n)->ms_videoscan = video_bounds;

		/* Set up the default vectors for MrgCop() and LoadView() */
		((struct MonitorSpec *)n)->ms_MrgCop = mrgcop;
		((struct MonitorSpec *)n)->ms_LoadView = loadview;

	    }
	}
	break;

	default :
	{
	    Alert(AN_GfxNewError);
	}
	break;
    }
    return (n);
}

void __asm GfxAssociate(register __a0 VOID *old, register __a1 struct AssociatedNode *new)
{
    struct AssociatedNode *n;
    struct AssociatedNode **table;
    int key = hashit((ULONG) old);

    table = (struct AssociatedNode **) GBASE->hash_table;
    new->backlink = old;

    ObtainSemaphore(GBASE->HashTableSemaphore);
    /* add this to the head of the chain */
    if (n = table[key])
    {
	new->n.xln_Succ = (struct Node *)n;
	n->n.xln_Pred = (struct Node *)new;
	table[key] = new;
    }
    else
    {
    	/* no links, so this is the first */
    	table[key] = new;
    }
    ReleaseSemaphore(GBASE->HashTableSemaphore);

}

void gfxforget(old,new)
APTR old;
struct AssociatedNode *new;
{
    struct AssociatedNode *pred, *succ;

    /* Unlink this node from the chain */
    ObtainSemaphore(GBASE->HashTableSemaphore);
    pred = new->n.xln_Pred;
    succ = new->n.xln_Succ;
    if (pred)
    {
	pred->n.xln_Succ = (struct Node *)succ;
    }
    else
    {
	/* this entry was the first in the chain */
	int key = hashit((ULONG) old);
	struct AssociatedNode **table;

	table = (struct AssociatedNode **) GBASE->hash_table;
	table[key] = succ;
    }
    if (succ)
    {
	succ->n.xln_Pred = (struct Node *)pred;
    }
    ReleaseSemaphore(GBASE->HashTableSemaphore);
}

struct AssociatedNode * __asm GfxLookUp(register __a0 VOID *old)
{
    struct AssociatedNode *n;
    int key = hashit((ULONG) old);
    struct AssociatedNode **table;

    table = (struct AssociatedNode **) GBASE->hash_table;

    ObtainSemaphore(GBASE->HashTableSemaphore);
    /* find the entry in the chain */
    for(n = table[key]; n && (n->backlink != old); n = (struct Node *)n->n.xln_Succ);
    ReleaseSemaphore(GBASE->HashTableSemaphore);
    return(n);
}

void gfxcheck(n)
struct AssociatedNode *n;
{
    if(n = gfx_GfxLookUp((struct AssociatedNode*)n->backlink))
    {
	gfxforget(n->backlink,n);
    }
}

void __asm GfxFree(register __a0 struct ExtendedNode *n)
{
    switch(n->xln_Type)
    {
	case NT_GRAPHICS :
	{
	    switch(n->xln_Subsystem)
	    {
		case SS_GRAPHICS:
		{
		    switch(n->xln_Subtype)
		    {
			case VIEW_EXTRA_TYPE :
			    gfxcheck(n); /* associated node */
			    FreeMem(n,sizeof(struct ViewExtra));
			    break;
			case VIEWPORT_EXTRA_TYPE :
			    gfxcheck(n); /* associated node */
			    FreeMem(n,sizeof(struct ViewPortExtra));
			    break;
			case SPECIAL_MONITOR_TYPE:
			    FreeMem(n,sizeof(struct SpecialMonitor));
			    break;
			case MONITOR_SPEC_TYPE:
			    FreeMem(n,sizeof(struct MonitorSpec));
			    break;
			default :       Alert(AN_GfxFreeError);
			    break;
		    }
		}   break;
		default :       Alert(AN_GfxFreeError);
		    break;
	    }
	}   break;
	default :       Alert(AN_GfxFreeError);
	    break;
    }
}

