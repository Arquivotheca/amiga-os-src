
/*********************************************************************/
/*                                                                   */
/*                     Copyright (c) 1985                            */
/*                    Commodore-Amiga, Inc.                          */
/*                    All rights reserved.                           */
/*                                                                   */
/*     No part of this program may be reproduced, transmitted,       */
/*     transcribed, stored in retrieval system, or translated        */
/*     into any language or computer language, in any form or        */
/*     by any means, electronic, mechanical, magnetic, optical,      */
/*     chemical, manual or otherwise, without the prior written      */
/*     permission of:                                                */
/*                     Commodore-Amiga, Inc.                         */
/*                     983 University Ave #D                         */
/*                     Los Gatos, CA. 95030                          */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*  Program name:  printer test (built on pic)                       */
/*                                                                   */
/*  Arguments:  (none)                                               */
/*                                                                   */
/*  Notes:  1.  Open various libraries.                              */
/*          2.  Open a custom screen and window.                     */
/*          3.  Open & associate a console device with the window.   */
/*          4.  Draw some simple coloed patterns into the window.    */
/*          5.  Execute printer commands from the console.           */
/*          6.  Close the window.                                    */
/*                                                                   */
/*  Programer:  Stan Shepard                                         */
/*                                                                   */
/*  Date Released:  11-Jun-85                                        */
/*                                                                   */
/*  Modified:	14-Jun-85 by David Berezowski for V25.22 update       */
/*             15-Jun-85 by Kodiak for printer device test           */
/*             18-Jun-85 by David Berezowski for auto color & screen */
/*             25-Jun-85 by David Berezowski for error checking      */
/*                                                                   */
/*********************************************************************/

#include	"exec/types.h"
#include	"exec/nodes.h"
#include	"exec/lists.h"
#include	"exec/interrupts.h"
#include	"exec/memory.h"
#include	"exec/ports.h"
#include	"exec/tasks.h"
#include	"exec/libraries.h"
#include	"exec/devices.h"
#include	"exec/io.h"
#include	"exec/devices.h"

#include	"devices/console.h"
#include	"devices/timer.h"

#include	"libraries/dos.h"

#include	"graphics/gfx.h"
#include	"graphics/collide.h"
#include	"graphics/copper.h"
#include	"graphics/display.h"
#include	"hardware/dmabits.h"
#include	"graphics/gels.h"
#include	"graphics/clip.h"
#include	"graphics/rastport.h"
#include	"graphics/view.h"
#include	"graphics/gfxbase.h"
#include	"graphics/text.h"
#include	"hardware/intbits.h"
#include	"hardware/custom.h"
#include	"graphics/gfxmacros.h"
#include	"graphics/layers.h"
#include	"intuition/intuition.h"
#include	"devices/inputevent.h"
#include	"devices/gameport.h"

#include	"../printer/printer.h"

#define getc() Read(stdin, c, 2)
#define TXHEIGHT 8

struct TextAttr TestFont =
    {
    "topaz.font",
    TXHEIGHT,
    0,
    0,
    };

long GfxBase = 0;
long IntuitionBase = 0;

struct Window *OpenWindow();
struct Screen *OpenScreen();


main()
{
	struct Window *w;
	struct Window *w2;
	struct Window *w3;
	struct Screen *screen;
	struct RastPort *rp, *cdrp;
	struct ViewPort *vp;
	struct IntuiMessage *message;
	ULONG cdio;
	SHORT i;
	USHORT red, green, blue;
	SHORT inputChar;
	SHORT ch;
	UBYTE c[10];
	char *b, buffer[80];
	LONG err; /* the error # */

	GfxBase = OpenLibrary("graphics.library", 0);
	if (GfxBase == NULL)
	{
		printf("Unable to open graphics library\n");
		exit();
	}

	IntuitionBase = OpenLibrary("intuition.library", 0);
	if (IntuitionBase == NULL)
	{
		printf("Unable to open Intuition library\n");
		exit();
	}

/*  Open a custom HIRES screen 3 planes deep  */
	{
		struct NewScreen ns;
		ns.LeftEdge = 0;
		ns.TopEdge = 0;
		ns.Width = 640;
		ns.Height = 200;
		ns.Depth = 3;
		ns.DetailPen = 0;
		ns.BlockPen = 1;
		ns.ViewModes = HIRES;
		ns.Type = CUSTOMSCREEN;
		ns.Font = &TestFont;
		ns.DefaultTitle = "HIGH RES, 3 planes, for printer test";
		ns.Gadgets = NULL;
		screen = OpenScreen(&ns);	/* open the screen! */
	}

	if (screen == NULL)
	{
		printf("Can't open a new screen\n");
		exit();
	}

	{
		struct NewWindow nw;
		nw.DetailPen = -1;
		nw.BlockPen = -1;
		nw.Flags = WINDOWDEPTH|WINDOWSIZING|WINDOWDRAG|WINDOWCLOSE|ACTIVATE;
		nw.IDCMPFlags = CLOSEWINDOW;
		nw.FirstGadget = 0;
		nw.CheckMark = 0;
		nw.Title = "printer test window";
		nw.Type = CUSTOMSCREEN;
		nw.Screen = screen;
		nw.BitMap = NULL;
		nw.MinWidth = 80;
		nw.MinHeight = 15;
		nw.MaxWidth = 10000;
		nw.MaxHeight = 10000;
		nw.LeftEdge = 5;
		nw.TopEdge = 5;
		nw.Width = 160;
		nw.Height = 50;
		w3 = OpenWindow(&nw);	/* open the window! */
		nw.LeftEdge = 400;
		nw.TopEdge = 150;
		nw.Width = 80;
		nw.Height = 25;
		w2 = OpenWindow(&nw);	/* open the window! */
		nw.LeftEdge = 160;
		nw.TopEdge = 50;
		nw.Width = 320;
		nw.Height = 100;
		w = OpenWindow(&nw);	/* open the window! */
		
	}

	rp = w->RPort;
	vp = &w->WScreen->ViewPort;

/*  Open the console device  */
	CDOpen(w);

/*  Draw some horizontal bars  */
	for (i = 1; i < 8; i++)
	{
		SetAPen(rp,i);
		RectFill(rp,10,i*5+10,225,i*5+25);
	}

/*  Draw some verticle bars  */
	for (i = 1; i < 8; i++)
	{
		SetAPen(rp,i);
		RectFill(rp,10+(i-1)*20,40,49+(i-1)*20,98);
	}

/*  Set the color registers  */
		SetRGB4(vp,0,15,15,15);
		SetRGB4(vp,1,15,0,0);
		SetRGB4(vp,2,0,15,0);
		SetRGB4(vp,3,0,0,15);
		SetRGB4(vp,4,15,15,0);
		SetRGB4(vp,5,15,0,15);
		SetRGB4(vp,6,0,15,15);
		SetRGB4(vp,7,0,0,0);

   printf("use console\n");
	CDPutStr("\33[20h");		/* line feed new line mode */
   printf("open printer\n");
	printf("w=%ld, rp=%ld, vp=%ld\n",w,rp,vp);
	err=pOpen(AllocSignal(-1));
	printf("Open err=%ld\n",err);
/*  Wait for a CLOSEWINDOW */
	do {
		message = (struct IntuiMessage *)GetMsg(w->UserPort);
		CDPutStr("\nEnter command [w,d]: ");
		CDPutChar( ch = CDGetChar() );
		switch (ch) {
		case 'w':
			CDPutStr("\n    string: ");
			b = buffer;
			while((*b = CDGetChar()) != '\15') CDPutChar(*b++);
			CDPutChar(*b++ = '\n');
			*b = '\0';
			pWrite(buffer);
			break;
		case 'd':
/* Dump a Window */
			err=(pDumpRPort(w->RPort, vp->ColorMap, vp->Modes, 0,0, 320,100, 320,200, 0));
/*Dump a Screen */
/*			err=(pDumpRPort(&screen->RastPort,screen->ViewPort.ColorMap, screen->ViewPort.Modes, 0,0, 640,200, 160,300, 0));*/
printf("The DUMP error # is %ld\n",err);
			break;
		default:;
		}
	} while (message->Class != CLOSEWINDOW);

	pClose();
	CDClose();
	CloseWindow(w);
	CloseScreen(screen);
}
