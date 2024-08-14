/* hkinternal.c -- based on j mackraz ihelp */

#include "mycxapp.h"

#define	IMINWIDTH	40
#define	IMINHEIGHT	30
#define	MIN(A,B)	(((A)<(B))?(A):(B))
#define	MAX(A,B)	(((A)>(B))?(A):(B))


#define		CYCLE				0
#define		CYCLEBACK			1
#define		MAKEBIG				2
#define		MAKESMALL			3
#define		SHELLTOFRONT			4

#define ICMDCNT 5

UBYTE *icommands[ICMDCNT] =
    {	"@cycle",
	"@cycleback",
	"@makebig",
	"@makesmall",
	"@shelltofront",
    };


void doInternal(UBYTE *command)
{
	int k;

	for(k=0; k<ICMDCNT; k++)
	    {
	    if(!(stricmp(command,icommands[k]))) break;
	    }

	switch(k)
	    {
	    case CYCLE:
		cycleforward();
		break;
	    case CYCLEBACK:
		cyclebackward();
		break;
	    case MAKESMALL:
		makesize(MAKESMALL);
		break;
	    case MAKEBIG:
		makesize(MAKEBIG);
		break;
	    case SHELLTOFRONT:
		shelltofront();
		break;
	    default:
		break;
	    }
}


void makesize(int command)
{
	ULONG			ilock;
	struct Window	*awindow;
	struct Screen	*ascreen;
	SHORT			deltaw;
	SHORT			deltah;

	ilock = LockIBase(0L);
	awindow = IntuitionBase->ActiveWindow;
	ascreen = awindow->WScreen;

	switch (command)
	{
	case MAKESMALL:
		deltaw = MAX(awindow->MinWidth, IMINWIDTH) - awindow->Width;
		deltah = MAX(awindow->MinHeight, IMINHEIGHT) - awindow->Height;
		break;

	case MAKEBIG:
		deltaw =
			MIN(ascreen->Width - awindow->LeftEdge, (unsigned) awindow->MaxWidth)
			- awindow->Width;
		deltah =
			MIN(ascreen->Height-awindow->TopEdge, (unsigned) awindow->MaxHeight)
			- awindow->Height;
		break;
	default:
		deltaw = 0;
		deltah = 0;
	}

	SizeWindow(awindow, (LONG) deltaw, (LONG) deltah);

	UnlockIBase(ilock);
}


void cyclebackward()
{
	;
#if PSEUDOCODE
	lock intuition base
	find active window
	get its screen
	get frontmost layer from layer info
	get second layer too (not WBENCHWINDOW)
	unlock ibase

	is active window frontmost?
		if second is not backdrop
			active window to back
			activate second window
		else
			abort
	else
		activate frontmost
#endif
}

void cycleforward()
{
	LONG			ilock;
	struct Window	*awindow;
	struct Screen	*ascreen;
	struct Layer	*rearlayer;		/* rearmost so far		*/
	struct Layer	*layer;			/* runs through layers	*/
	struct Window	*lwindow;		/* layer->Window		*/


	ilock = LockIBase(0L);
	awindow = IntuitionBase->ActiveWindow;
	ascreen = awindow->WScreen;

	D( kprintf("active window/screen: %lx/%lx\n", awindow, ascreen) );

	/* for now, only pull this on the workbench	*/
	if ((ascreen->Flags & SCREENTYPE) != WBENCHSCREEN)
	{
		D( kprintf("not wbscreen\n") );
		UnlockIBase(ilock);
		goto OUT;
	}

	/* find rearmost layer which is not a backdrop window,
	 * nor the bar layer
	 */
	rearlayer = NULL;
	for (layer = ascreen->LayerInfo.top_layer;layer; layer = layer->back)
	{
		lwindow = (struct Window *) layer->Window;
		D( kprintf("layer %lx window %lx\n",layer, lwindow) );
		if (layer == ascreen->BarLayer)	
		{
			D( kprintf("is bar layer\n") );
			continue;
		}
		if (layer->Flags & LAYERBACKDROP)
		{
			D( kprintf("backdrop layer\n") );
			continue;
		}
		rearlayer = layer;
	}
	D( kprintf("let's try it\n") );
	UnlockIBase(ilock);
	D( kprintf("unlocked\n") );

	if (rearlayer)
	{
		lwindow = (struct Window *) rearlayer->Window;
		D( kprintf("window of choice: %lx\n") );
		WindowToFront(lwindow);
		if (lwindow != awindow) ActivateWindow(lwindow);
	}

OUT:
	D( kprintf("cycleforward done\n") );
	return;

}


char shellname[] = "AmigaShell";

BOOL shelltofront()
{
	LONG		ilock;
	BOOL		success = TRUE; 
	struct Window	*awindow;
	struct Window	*swindow;
	struct Window	*aswindow = NULL;
	struct Screen	*ascreen;

	ilock = LockIBase(0L);
	awindow = IntuitionBase->ActiveWindow;
	ascreen = awindow->WScreen;

	D( kprintf("active window/screen: %lx/%lx\n", awindow, ascreen) );

	/* for now, only pull this on the workbench */
	if ((ascreen->Flags & SCREENTYPE) != WBENCHSCREEN)
	{
		D( kprintf("not wbscreen\n") );
		UnlockIBase(ilock);
		success = FALSE;
		goto OUT;
	}

	for (swindow = ascreen->FirstWindow; 
		(swindow);
			swindow = swindow->NextWindow)
		{
		if(strcmp(swindow->Title,shellname)==0)
			{
                	if(swindow->Flags & WINDOWACTIVE) aswindow = swindow;
                	else break;
			}
		} 

	D( kprintf("let's try it\n") );
	UnlockIBase(ilock);
	D( kprintf("unlocked\n") );

	/* if no inactive shell, bring forward active shell */
	if((!swindow)&&(aswindow)) swindow = aswindow;
	if (swindow)
	{
		D( kprintf("window of choice: %lx\n",swindow) );
		WindowToFront(swindow);
		ActivateWindow(swindow);
	}
	else success = FALSE;

OUT:
	D( kprintf("shelltofront done\n") );
	return(success);
}


