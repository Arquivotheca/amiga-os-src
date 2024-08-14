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
#include	"../printer/prtbase.h"

extern struct IOStdReq printerIO;
struct IOStdReq printerIO2;

struct TextAttr screenFont = {
    "topaz.font",
    9,
    0, 0
};

long GfxBase = 0;
long IntuitionBase = 0;

struct Window *OpenWindow();
struct Screen *OpenScreen();

main()
{
    struct Window *w[3];
    struct Screen *s[3];
    struct ViewPort *vp;
    struct RastPort *rp;
    int i, j, k, x;
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

/*  Open some custom screens */
    {
	struct NewScreen ns;
	ns.LeftEdge = 0;
	ns.TopEdge = 50;
	ns.Width = 320;
	ns.Height = 200;
	ns.Depth = 6;
	ns.DetailPen = 1;
	ns.BlockPen = 2;
	ns.ViewModes = HAM;
	ns.Type = CUSTOMSCREEN;
	screenFont.ta_YSize = 8;
	ns.Font = &screenFont;
	ns.DefaultTitle = "Lo Res HAM Screen for printer test";
	ns.Gadgets = NULL;
	s[0] = OpenScreen(&ns);
	if (s[0] == NULL)
	{
	    printf("Can't open screen s[0]\n");
	    exit();
	}

	ns.TopEdge = 200;
	ns.Depth = 4;
	ns.ViewModes = LACE;
	ns.DefaultTitle = "Lo Res LACE Screen for printer test";
	s[1] = OpenScreen(&ns);
	if (s[1] == NULL)
	{
	    printf("Can't open screen s[1]\n");
	    exit();
	}

	ns.TopEdge = 300;
	ns.Width = 640;
	ns.Height = 200;
	ns.Depth = 1;
	ns.DetailPen = 1;
	ns.BlockPen = 0;
	ns.ViewModes = HIRES | LACE;
	screenFont.ta_YSize = 9;
	ns.DefaultTitle = "Hi Res LACE Screen for printer test";
	s[2] = OpenScreen(&ns);
	if (s[2] == NULL)
	{
	    printf("Can't open screen s[2]\n");
	    exit();
	}
    }


    {
	struct NewWindow nw;

	nw.DetailPen = -1;
	nw.BlockPen = -1;
	nw.Flags = 0;
	nw.IDCMPFlags = 0;
	nw.FirstGadget = 0;
	nw.CheckMark = 0;
	nw.Title = "printer test";
	nw.Type = CUSTOMSCREEN;
	nw.Screen = s[0];
	nw.BitMap = NULL;
	nw.MinWidth = 80;
	nw.MinHeight = 15;
	nw.MaxWidth = 10000;
	nw.MaxHeight = 10000;
	nw.LeftEdge = 0;
	nw.TopEdge = 10;
	nw.Width = 320;
	nw.Height = 190;
	w[0] = OpenWindow(&nw);
	if (w[0] == NULL)
	{
	    printf("Can't open window w[0]\n");
	    exit();
	}

	nw.Screen = s[1];
	w[1] = OpenWindow(&nw);
	if (w[1] == NULL)
	{
	    printf("Can't open window w[1]\n");
	    exit();
	}

	nw.Width = 640;
	nw.Height = 140;
	nw.Flags = WINDOWDEPTH|WINDOWSIZING|WINDOWDRAG|WINDOWCLOSE|ACTIVATE;
	nw.Screen = s[2];
	w[2] = OpenWindow(&nw);
	if (w[2] == NULL)
	{
	    printf("Can't open window w[2]\n");
	    exit();
	}
	    
    }

/* initialize the low res HAM window */
    rp = w[0]->RPort;
    vp = &w[0]->WScreen->ViewPort;
    SetDrMd(rp,0);
    x = 22;
    SetAPen(rp,32);
    Move(rp,x, 11);
    Draw(rp,x++, 11);
    SetAPen(rp,48);
    Draw(rp,x++, 11);
    Move(rp,x, 11);
    for (i = 0; i < 16; ++i)
    {
	SetAPen(rp,16 | i);
	Draw(rp,x, 11 * i + 11);	
    }
    ++x;

    for (i = 0; i < 16; ++i)
    {
	SetAPen(rp,48);
	Move(rp,x, 11);
	Draw(rp,x++, 186);
	SetAPen(rp,32 | i);
	Move(rp,x, 11);
	Draw(rp,x++, 186);
	for (j = 1; j < 16; ++j)
	{
	    SetAPen(rp,48 | j);
	    Move(rp,x, 11);
	    Draw(rp,x++, 186);
	}
    }

/* initialize the low res LACE window */
    rp = w[1]->RPort;
    vp = &w[1]->WScreen->ViewPort;
    /*  Draw checkerboard */
    rp->Mask = 0x03;;
    for (i = 0; i < 4; i++)
    {
	SetAPen(rp,i);
	RectFill(rp,5,i*20+20,315,i*20+40);
    }
    rp->Mask = 0x0c;;
    for (i = 0; i < 4; i++)
    {
	SetAPen(rp,i<<2);
	RectFill(rp,i*50+5,11,i*50+55,185);
    }
    rp->Mask = 0xff;;

/* initialize the hi res window */
    rp = w[2]->RPort;
    vp = &w[2]->WScreen->ViewPort;
    SetRGB4(vp,0,0,0,15);
    SetRGB4(vp,1,8,15,15);
    /*  Open the console device on the hi res window */
    CDOpen(w[2]);
    /*  Draw checkerboard */
    rp->Mask = 0x03;
    for (i = 0; i < 4; i++)
    {
	SetAPen(rp,i);
	RectFill(rp,5,i*15+20,635,i*15+40);
    }
    rp->Mask = 0x0c;
    for (i = 0; i < 4; i++)
    {
	SetAPen(rp,i<<2);
	RectFill(rp,i*100+5,10,i*100+105,85);
    }
    rp->Mask = 0xff;

/* put a box in all windows */
    for (i = 0; i < 3; i++) {
	rp = w[i]->RPort;
	SetAPen(rp,5);
	RectFill(rp, 9, 9, 101, 101);
	SetAPen(rp,10);
	RectFill(rp, 10, 10, 100, 100);
	SetAPen(rp,15);
	RectFill(rp, 11, 11, 99, 99);
	RectFill(rp, 10, 40, 100, 60);
	RectFill(rp, 40, 10, 60, 100);
    }
    CDPutStr("\33[20h");		/* line feed new line mode */


    {
	int signal;
	char ch, berror[32];

	signal = AllocSignal(-1);
	if (pOpen(signal) == 0) {
	    pWrite("123456789+123456789+123456789+123456789+123456789+123456789+123456789+123456789+\n");
	    vp = &s[2]->ViewPort;
	    pWrite("\nSquare #2: 2\" wide 0 high\n");
	    sprintf(berror, "\n    Error: %ld\n",
		    pDumpRPort(w[2]->RPort, vp->ColorMap, vp->Modes,
		    10,10, 91,91, 2000,0, SPECIAL_MILCOLS));
	    pWrite(berror);
	    pWrite("\nSquare #2: 0 wide 1\" high\n");
	    sprintf(berror, "\n    Error: %ld\n",
		    pDumpRPort(w[2]->RPort, vp->ColorMap, vp->Modes,
		    10,10, 91,91, 0,1000, SPECIAL_MILROWS));
	    pWrite(berror);
	    pWrite("\nSquare #2: 1/2 wide 1/6 high\n");
	    sprintf(berror, "\n    Error: %ld\n",
		    pDumpRPort(w[2]->RPort, vp->ColorMap, vp->Modes,
		    10,10, 91,91, 0x80000000,0x2AAAAAAA,
		    SPECIAL_FRACCOLS | SPECIAL_FRACROWS));
	    pWrite(berror);
	    vp = &s[1]->ViewPort;
	    pWrite("\nSquare #1: 0 wide 1\" high\n");
	    sprintf(berror, "\n    Error: %ld\n",
		    pDumpRPort(w[1]->RPort, vp->ColorMap, vp->Modes,
		    10,10, 91,91, 0,1000, SPECIAL_MILROWS));
	    pWrite(berror);
	    vp = &s[0]->ViewPort;
	    pWrite("\nSquare #0: 0 wide 1\" high\n");
	    sprintf(berror, "\n    Error: %ld\n",
		    pDumpRPort(w[0]->RPort, vp->ColorMap, vp->Modes,
		    10,10, 91,91, 0,1000, SPECIAL_MILROWS));
	    pWrite(berror);
	    pClose();
	}
    }
    CDClose();
    CloseWindow(w[0]);
    CloseWindow(w[1]);
    CloseWindow(w[2]);
    CloseScreen(s[0]);
    CloseScreen(s[1]);
    CloseScreen(s[2]);
}
