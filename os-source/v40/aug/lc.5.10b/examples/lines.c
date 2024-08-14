/* This is a line drawing demo for the Commodore/Amiga  */
/* Written by	John Riley, Lattice, Inc.		*/
/* 							*/
/* there is a single compile time option:		*/
/*							*/
/* compile with -dNOERASE  - leave trail of lines	*/
/*							*/
/* if the option is not used then old lines will be	*/
/* erased as the lines are drawn across the screen	*/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <graphics/text.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

USHORT wakeup;	/* Wake me up for event */
USHORT class;	/* Intu event class */
USHORT code;	/* Intu event code */

struct Window *w;
struct RastPort *rp,*cdrp;
struct ViewPort *vp;
struct IntuiMessage *message;
int event(void);
long rand(void);

/************************ Window Defines ***********************************/

struct NewWindow nw = {
		100,100,			/* Starting corner */
		300,100,		/* Width, height */
		2,1,			/* detail, block pens */
	CLOSEWINDOW | NEWSIZE,		/* IDCMP flags */
	WINDOWDEPTH | WINDOWDRAG | WINDOWCLOSE | GIMMEZEROZERO | WINDOWSIZING,
					/* Window flags */
		NULL,			/* Pointer to first gadget */
		NULL,			/* Pointer to checkmark */
		"Nervous Lines",	/* title */
		NULL,			/* screen pointer */
		NULL,			/* bitmap pointer */
		50,50,640,400,		/* window not sized */
		WBENCHSCREEN		/* type of screen */
		};

int x[2],y[2],xd[2],yd[2],co,ox[2][16],oy[2][16],xx[128],xlim,ylim;


/**
*
* main program - Since no command line data is used, and no file I/O
* is performed, we do not need _main from the library. We eliminate
* it by calling our main function '_main'.
*
**/
void _main()
{
	register short i;
	register int k,co;
	register long j;

/************************ Set-Up routines **********************************/
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);
	if(GfxBase == NULL) return;
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0);
	if(IntuitionBase == NULL) 
		{
		CloseLibrary((struct Library *)GfxBase);
		return;
		}
	w = OpenWindow(&nw);
	rp = w->RPort;			/* Get the raster port pointer */
	vp = &w->WScreen->ViewPort;	/* Get the view port pointer */
	SetAPen(rp,3);			/* Set foreground pen to white */
	SetDrMd(rp,JAM1);		/* Draw with foreground pen */

	xlim = w->Width;
	ylim = w->Height;
	for(i=0;i<2;i++)
	{
		x[i] = rand() % xlim + 1;
		y[i] = rand() % ylim + 1;
		xd[i] = rand() % 10 + 1;
		yd[i] = rand() % 10 + 1;
	}
	co = 1;
	j = 0;
	do {
#ifndef NOERASE
		SetAPen(rp,0);
		Move(rp,ox[0][j & 15],oy[0][j & 15]);
		Draw(rp,ox[1][j & 15 ],oy[1][j & 15]);
#endif
		SetAPen(rp,co);
		Move(rp,x[0],y[0]);
		Draw(rp,x[1],y[1]);
		if((rand() & 127) < 2)
		{
			++co;
			if(co > 3)	co = 1;
			SetAPen(rp,co);
		}
		for(i=0;i<2;i++)
		{
			ox[i][(j+10) & 15] = x[i];
			oy[i][(j+10) & 15] = y[i];
			x[i] += xd[i];
			y[i] += yd[i];
			if(x[i] < 2)
			{
				x[i] = 2;
				xd[i] = -xd[i];
			}
			else if(x[i] > xlim)
			{
				x[i] = xlim;
				xd[i] = -xd[i];
			}
			if(y[i] < 2)
			{
				y[i] = 2;
				yd[i] = -yd[i];
			}
			else if(y[i] > ylim)
			{
				y[i] = ylim;
				yd[i] = -yd[i];
			}
			if(((rand() >> 5) & 127) < 2)
			{
				if(xd[i] < 1) 	k = 1;
				xd[i] = (rand() >> 5) & 7;
				if(k == 1)	xd[i] = -xd[i];
				k = 0;
			} 
			if(((rand() >> 5) & 255) < 50)
			{
				if(yd[i] < 1)	k = 1;
				yd[i] = (rand() >> 5) & 7;
				if(k == 1)	yd[i] = -yd[i];
				k = 0;
			}
		}
		++j;
		if(w->UserPort->mp_SigBit)
		{
			message = (struct IntuiMessage *)GetMsg(w->UserPort);
			if(message != NULL)
			{
				class = message->Class;
				code = message->Code;
				ReplyMsg((struct Message *)message);
			}
		}
	} while(event());
	CloseWindow(w);
	CloseLibrary((struct Library *)GfxBase);
	CloseLibrary((struct Library *)IntuitionBase);
}

int event()
{
	switch(class)
	{
		case CLOSEWINDOW:
			return(0);
		case NEWSIZE:
			xlim = w->Width;
			ylim = w->Height;
			return(1);
	}
	return(1);
}
