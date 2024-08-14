/******************************************************************************
*
*	$Id: scrollvp.c,v 42.0 93/06/16 11:16:05 chrisg Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h> 
#include <exec/libraries.h>
#include <exec/memory.h>
#include "/gfx.h"
#include "/gfxbase.h"
#include "/display.h"
#include "/copper.h"
#include "/view.h"
#include "/vp_internal.h"
#include "/displayinfo_internal.h"
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include "/macros.h"

#include "c.protos"

#define SCROLLVP_DOWNCODED 

#ifndef SCROLLVP_DOWNCODED

#define SAVE_CODESPACE
/* following done by Dale  10-3-86*/
/*#define WaitBOVP(x) waitbovp(x)*/
#define WaitBOVP(x) waittovp(x,1)


/* #define DEBUG */
#define SCREENS

#ifdef DEBUG
#define D(x) {{x};}
#else
#define D(x)
#endif

extern struct Custom custom;

/**************************************************************************/
/* pokeCprList():							  */
/* given a pointer to the start of a hardware cprlist, search for	  */ 
/* coppermove to field one, if found use field two to update destdata     */
/*									  */
/**************************************************************************/
/*   		supports COPPERMOVE intstruction pokes only!!!		  */
/**************************************************************************/

struct pbCprList_info
{
	UWORD flags;
	UWORD *cil,*cis;
};


/* find first target, jam, assume next target follows */
#ifndef MAKE_PROTOS
void __regargs npokeCprList(UWORD *ci,UWORD *target,UWORD *table,int num)
{
	ULONG targ=(ULONG) target;
	/* is there a list to search through? */
	if (!ci)	return;
    /* set up target mask for searching cplist */
    targ &= 0x1FE;

	/* look for first hit */
    	for ( ; ci ; ci += 2)
    	{
		if ( ((*ci)==0xFFFF) && ((*(ci+1)==0xFFFE)) ) return;
		if (*ci == targ) break;
	}
	/* found first target now go fast for rest */
	while (num--)
	{
		/* end of list? */
		if ( ((*ci)==0xFFFF) && ((*(ci+1)==0xFFFE)) ) return;

		/* check for valid target */
		if (*ci == targ)
		{
			ci++;	/* skip address */
			*ci++ = *table;	/* put in new data */
		}
		/* always to next member in table */
		table++;
		targ += 2;
	}
}

void __regargs pokeCprList(UWORD *ci,UWORD *field1,int field2)
{
	UWORD	qwe;
	qwe = field2;
	npokeCprList(ci,field1,&qwe,1);
}

void pokebothCprList(struct pbCprList_info *pb, UWORD *field1, int field2)
{
	UWORD	qwe = field2;
	npokeCprList(pb->cil,field1,&qwe,1);	/* long frame */
	if (pb->flags&LACE)
		npokeCprList(pb->cis,field1,&qwe,1);	/* short frame */
}

/**************************************************************************/
/* pokeCopIns():							  */
/* given a pointer to the start of a CopIns List, search for operation on */
/* field one, if found use field to to update instruction		  */
/* dale added: return pointer to CopIns affected                          */
/*									  */
/**************************************************************************/

struct CopIns * pokeCopIns(struct CopIns *ci,UWORD op,UWORD *field1,short field2)
{
    struct CopIns *c;

    D(kprintf("poking another CopIns\n"));
    D(kprintf("op=%lx field1=%lx field2=%lx\n",op,field1,field2));

    if ( (c = ci) != NULL )
    {
	switch (op & 3)
	{
	    case COPPER_MOVE : 
	    {
		short error = FALSE;

		while (!error)
		{
		    switch (c->OpCode & 3)
		    {
			case COPPER_MOVE :  
					    if (c->DESTADDR == (((UWORD)field1) & 0x1fe))
					    {
						short mask;

						D(kprintf("found an instruction with the right destination address\n"));

						/* test if interlace matters */
						if ((mask = op & 0xC000))
						{
						    /* interlace, poke only if proper frame's intruction */
						    D(kprintf("looking for an interlaced frame\n"));
						    if (c->OpCode & mask)
						    {
							/* correct frame, poke new datafetch start */
							D(kprintf("found the right interlace field, so poke it\n"));
							c->DESTDATA = field2;
							return(c);
						    }

						}
						else
						{
						    /* not interlace, poke regardless */
						    /* poke new datafetch start */
						    D(kprintf("...poke it\n"));
						    c->DESTDATA = field2;
						    return(c);
						}
					    }
					    c++;
					    break;

			case COPPER_WAIT :  if (c->HWAITPOS == 255)
					    {
						/* yes, found end of list */
						D(kprintf("returning from pokeCopIns()...\n"));
						return(0);
					    }
					    else
					    {
						c++;
					    }
					    break;
			
			case CPRNXTBUF :    if (c->NXTLIST == NULL)
					    {
						error = TRUE;
					    }
					    else
					    {
						if ((c = c->NXTLIST->CopIns)==NULL)
						{
						    error = TRUE;
						}
					    }
					    break;

			default :	    error = TRUE;
					    break;
		    }	
			
		}
		/* !!! error condition !!! */
		/* break out of switch(op){} after encountering error */
		D(kprintf("error exit from pokeCopIns()...\n"));
		break;
	    }

	    case COPPER_WAIT :	
#ifdef QWEQWE
	    {
		D(kprintf("pokeCopIns: poke copper wait unsupported\n"));
		break;
	    }
#endif

	    case CPRNXTBUF :	
#ifdef QWEQWE
	    {
		D(kprintf("pokeCopIns: poke copper nextbuf unsupported\n"));
		break;
	    }
#endif

	    default :
	    {
		D(kprintf("pokeCopIns: bad opcode\n"));
		break;
	    }
	}
    }					    
	return (0);
}

#endif /* if not MAKE_PROTOS */

/**************************************************************************/
/* scrollvport(): non-destructively scroll viewport in its rastport(s) */
/*									  */
/**************************************************************************/

#define MIN_YTOP        0x15

#define HFIX(x) (((x)*mspc->ratioh)>>4)
#define VFIX(y) (((y)*mspc->ratiov)>>4)

void scrollvport(struct ViewPort *vp)
{
    struct RasInfo *r1, *r2;
    struct CopList *cl;
    UWORD *lfstart, *sfstart;
    struct CopIns *c = 0;
    short i;
    short depth;
    struct Custom *io;
    struct View *view;
    struct pbCprList_info pbcprlist;
    struct ViewExtra *ve=0;
    UWORD modes = new_mode(vp);
    struct BuildData bd;
    BOOL usefmode;

    if (vp->Modes & VP_HIDE) return; /* bart - 08.13.85 */

    D(kprintf("&custom=%lx\n",&custom));

    /* single thread access to ActiView hardware copper list */

    ObtainSemaphore(GBASE->ActiViewCprSemaphore);

    if(view = GBASE->ActiView)
    {

	if (view->Modes & EXTEND_VSTRUCT)
	{
	    ve = (struct ViewExtra *) GfxLookUp(view);
	}

	io = &custom;
	D(kprintf("io=%lx\n",io));

	cl = vp->DspIns;
	if (cl)
	{
		c = cl->CopIns;
	}

	r1 = vp->RasInfo;
	r2 = NULL;
	if (modes & DUALPF) r2 = r1->Next;  /* get second playfield */

	/* OK so far, start scrollviewport routine proper */

	/* get pointers to the start of hardware copper lists for this viewport, if any */

	pbcprlist.cil = lfstart = cl->CopLStart;
	pbcprlist.cis = sfstart = cl->CopSStart;
	/* bart - 09.25.86 remember to check lace bit for view too */
	pbcprlist.flags = modes | view->Modes;

	depth = r1->BitMap->Depth;
	D(kprintf("depth=%lx\n",depth));

	/* do stuff for standard view */

	if (getclipstuff(view, vp, &bd) == GOOD_CLIPSTUFF)
	{
	    UWORD bpr = r1->BitMap->BytesPerRow;
	    UBYTE **pptr;
	    UBYTE *p;

	    /* poke new values for ddfstrt, ddfstop, and scroll into hardware copper list(s) */

	    /* minimize update clashes */
	    WaitBOVP(vp);

	    if (usefmode = (bd.Flags & BD_FUDGEDFMODE))
	    {
		pokebothCprList(&pbcprlist,&custom.fmode,bd.FudgedFMode);
	    }

	    D(kprintf("poke new hardware long frame value for dafstrt\n"));
	    D(kprintf("&custom.ddfstrt=%lx dafstrt=%lx\n",&custom.ddfstrt,bd.dafstrt));
	    pokebothCprList(&pbcprlist,&custom.ddfstrt,bd.DDFStrt);

	    D(kprintf("poke new hardware long frame value for bpl1mod\n"));
	    D(kprintf("&custom.bpl1mod=%lx bplmod=%lx\n",&custom.bpl1mod,bplmod));
	    pokebothCprList(&pbcprlist,&custom.bpl1mod,bd.Modulo);

	    if ((depth > 1) || (bd.Flags & (BD_IS_SDBL | BD_IS_DPF)))
	    {
		D(kprintf("poke new hardware long frame bit plane modulos for more bit planes\n"));
		D(kprintf("&custom.bpl2mod=%lx bplmod=%lx\n",&custom.bpl2mod,bplmod));
		pokebothCprList(&pbcprlist,&custom.bpl2mod,bd.Modulo2);
	    }

	    D(kprintf("poke new hardware long frame value for dafstop\n"));
	    D(kprintf("&custom.ddfstop=%lx dafstop=%lx\n",&custom.ddfstop,bd.0].dafstop));
	    pokebothCprList(&pbcprlist,&custom.ddfstop,bd.DDFStop);

	    D(kprintf("poke new hardware long frame value for scroll\n"));
	    D(kprintf("&custom.bplcon1=%lx ((scroll<<4)|scroll)=%lx\n",&custom.bplcon1,scrollword));
	    pokebothCprList(&pbcprlist,&custom.bplcon1,bd.bplcon1);

	    if (c)	/* if there is an intermediate copper list */
	    {
		/* poke new values for ddfstrt, ddfstop, and scroll */
		/* into intermediate copper list */
		if (usefmode)
		{
			pokeCopIns(c,COPPER_MOVE,&custom.fmode,bd.FudgedFMode);
		}

		D(kprintf("poke new intermediate copper list value for dafstrt\n"));
		D(kprintf("&custom.ddfstrt=%lx dafstrt=%lx\n",&custom.ddfstrt,dafstrt));
		pokeCopIns(c,COPPER_MOVE,&custom.ddfstrt,bd.DDFStrt);

		D(kprintf("poke new intermediate copper list value for dafstop\n"));
		D(kprintf("&custom.ddfstrt=%lx dafstop=%lx\n",&custom.ddfstrt,bd.0].dafstop));
		pokeCopIns(c,COPPER_MOVE,&custom.ddfstop,bd.DDFStop);

		D(kprintf("poke new intermediate copper list value for scroll\n"));
		D(kprintf("&custom.bplcon1=%lx ((scroll<<4)|scroll)=%lx\n",&custom.bplcon1,scrollword));
		pokeCopIns(c,COPPER_MOVE,&custom.bplcon1, bd.bplcon1);

		D(kprintf("poke intermediate copper list bit plane modulos for first bitplane\n"));
		D(kprintf("&custom.bpl1mod=%lx bplmod=%lx\n",&custom.bpl1mod,bplmod));
		pokeCopIns(c,COPPER_MOVE,&custom.bpl1mod, bd.Modulo);
		if ((depth > 1) || (bd.Flags & (BD_IS_SDBL | BD_IS_DPF)))
		{
		    D(kprintf("poke intermediate bit plane modulos for more bit planes\n"));
		    D(kprintf("&custom.bpl2mod=%lx bplmod=%lx\n",&custom.bpl2mod,bplmod));
		    pokeCopIns(c,COPPER_MOVE,&custom.bpl2mod, bd.Modulo);
		}
	    }

	    if (r2 == NULL)
	    {
		pptr = r1->BitMap->Planes;
		for (i=0;i<depth;i++)
		{

			/* grab em and jam em */
			p = (*pptr++) + bd.Offset;

			if (modes & INTERLACE)
			{
			    int tp = (int)p + bpr;

			    D(kprintf("poke hardware copper list short frame bit plane[%lx] pointer in interlace mode\n",i));
			    npokeCprList(sfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &tp,2);
			}
			else
			{
			    D(kprintf("poke hardware long frame bit plane[%lx] pointer\n",i));
			    npokeCprList(lfstart,(UWORD *) &custom.bplpt[i],(UWORD *)&p,2);

			    if(view->Modes&INTERLACE)
			    {
				D(kprintf("poke hardware copper list short frame bit plane[%lx] pointer in view->Modes interlace mode\n",i));
				npokeCprList(sfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &p,2);
			    }
			}

			if (view->Modes&INTERLACE&modes)
			{
			    D(kprintf("poke hardware long frame bit plane[%lx] pointer\n",i));
			    npokeCprList(lfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &p,2);
			}
		}
		if (c)
		{
			pptr = r1->BitMap->Planes;
			for (i=0;i<depth;i++)
			{
    
			    /* grab em and jam em */
			    p = (*pptr++) + bd.Offset;
    
			    if (modes & INTERLACE)
			    {
				    D(kprintf("poke intermediate copper list short frame bit plane[%lx] pointer in interlace mode\n",i));
				    pokeCopIns(c,COPPER_MOVE|CPR_NT_SHT,(UWORD *) &custom.bplpt[i],(TOBB((long)p)+TOBB(bpr))>>16);
				    pokeCopIns(c,COPPER_MOVE|CPR_NT_SHT,(UWORD *)((short)(&custom.bplpt[i])+2),(TOBB((long)p)+TOBB(bpr)));
			    }
			    else
			    {
				    D(kprintf("poke intermediate copper list bit plane[%lx] pointer\n",i));
				    pokeCopIns(c,COPPER_MOVE,(UWORD *) &custom.bplpt[i],TOBB((long)p>>16));
				    pokeCopIns(c,COPPER_MOVE,(UWORD *)((short)(&custom.bplpt[i])+2),TOBB((long)p));
			    }
    
			    if (view->Modes&INTERLACE&modes)
			    {
				    D(kprintf("poke long frame bit plane[%lx] pointer in interlace mode\n",i));
				    pokeCopIns(c,COPPER_MOVE|CPR_NT_LOF,(UWORD *) &custom.bplpt[i],TOBB((long)p>>16));
				    pokeCopIns(c,COPPER_MOVE|CPR_NT_LOF,(UWORD *)((short)(&custom.bplpt[i])+2),TOBB((long)p));
			    }
			}
		}
	    }
	    else    /* do dual playfield mode */
	    {
		UWORD bpr[2];
		UBYTE **pptr1,**pptr2;
		UBYTE *p;

		bpr[0] = r1->BitMap->BytesPerRow;
		bpr[1] = r2->BitMap->BytesPerRow;
		depth += r2->BitMap->Depth;

		/* now send out bit plane pointers */
		pptr1 = r1->BitMap->Planes;
		pptr2 = r2->BitMap->Planes;
		for (i=0;i<depth;i++)
		{
		    if (i & 1)  p = *pptr2++ + bd.Offset2;
		    else        p = *pptr1++ + bd.Offset;

		    if (modes & INTERLACE)
		    {
			D(kprintf("poke hardware short frame bit plane[%lx] pointer in interlace mode\n",i));
			npokeCprList(sfstart,(UWORD *)&custom.bplpt[i],(UWORD *) &p,2);
		    }
		    else
		    {
			D(kprintf("poke hardware long frame bit plane[%lx] pointer in non-interlace mode\n",i));
			npokeCprList(lfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &p,2);

			if(view->Modes&INTERLACE)
			{
			    D(kprintf("poke hardware short frame bit plane[%lx] pointer in view->Modes interlace mode\n",i));
			    npokeCprList(sfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &p,2);
			}
		    }

		    if (view->Modes&INTERLACE&modes)
		    {
			D(kprintf("poke hardware long frame bit plane[%lx] pointer in interlace mode\n",i));
			npokeCprList(lfstart,(UWORD *) &custom.bplpt[i],(UWORD *) &p,2);
		    }
		}

		if (c)
		{
			/* poke new values for fstrt, fstop, and scroll into intermediate copper list */

			if (usefmode)
			{
				pokeCopIns(c,COPPER_MOVE,&custom.fmode, bd.FudgedFMode);
			}

			D(kprintf("poke new intermediate copper list value for fstrt\n"));
			pokeCopIns(c,COPPER_MOVE,&custom.ddfstrt, bd.DDFStrt);

			D(kprintf("poke new value for fstop\n"));
			pokeCopIns(c,COPPER_MOVE,&custom.ddfstop, bd.DDFStop);

			D(kprintf("poke new value for scroll\n"));
			pokeCopIns(c,COPPER_MOVE,&custom.bplcon1, bd.bplcon1);

			D(kprintf("poke new intermediate copper list modulo for first bitplane\n"));
			pokeCopIns(c,COPPER_MOVE,&custom.bpl1mod, bd.Modulo);

			if (depth > 1)
			{
			    D(kprintf("poke new intermediate copper list modulo for multiple bitplanes\n"));
			    pokeCopIns(c,COPPER_MOVE,&custom.bpl2mod, bd.Modulo2);
			}

			/* now send out bit plane pointers */
			pptr1 = r1->BitMap->Planes;
			pptr2 = r2->BitMap->Planes;
			for (i=0;i<depth;i++)
			{
			    if (i & 1)  p = *pptr2++ + bd.Offset2;
			    else        p = *pptr1++ + bd.Offset;

			    if (modes & INTERLACE)
			    {
				D(kprintf("poke intermediate copper list short frame bit plane[%lx] pointer in interlace mode\n",i));
				pokeCopIns(c,COPPER_MOVE|CPR_NT_SHT,(UWORD *) &custom.bplpt[i],(TOBB((long)p)+TOBB(bpr[i&1]))>>16);
				pokeCopIns(c,COPPER_MOVE|CPR_NT_SHT,(UWORD *)((short)(&custom.bplpt[i])+2),(TOBB((long)p)+TOBB(bpr[i&1])));
			    }
			    else
			    {
				D(kprintf("poke intermediate copper list bit plane[%lx] pointer in non-interlace mode\n",i));
				pokeCopIns(c,COPPER_MOVE,(UWORD *) &custom.bplpt[i],TOBB((long)p>>16));
				pokeCopIns(c,COPPER_MOVE,(UWORD *)((short)(&custom.bplpt[i])+2),TOBB((long)p));
			    }

			    if (view->Modes&INTERLACE&modes)
			    {
				D(kprintf("poke intermediate copper list long frame bit plane[%lx] pointer in interlace mode\n",i));
				pokeCopIns(c,COPPER_MOVE|CPR_NT_LOF,(UWORD *)&custom.bplpt[i],TOBB((long)p>>16));
				pokeCopIns(c,COPPER_MOVE|CPR_NT_LOF,(UWORD *)((short)(&custom.bplpt[i])+2),TOBB((long)p));
			    }
			}
		    }
	    }
	}	/* lines up with GOOD_CLIPSTUFF line */
    }

    ReleaseSemaphore(GBASE->ActiViewCprSemaphore);
}

#endif
