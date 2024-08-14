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

	ns.TopEdge = 150;
	ns.Width = 640;
	ns.Height = 100;
	ns.Depth = 1;
	ns.DetailPen = 1;
	ns.BlockPen = 0;
	ns.ViewModes = HIRES;
	screenFont.ta_YSize = 9;
	ns.DefaultTitle = "Hi Res Screen for printer test";
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
	nw.Height = 40;
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
	RectFill(rp,5,i*20+20,315,i*20+39);
    }
    rp->Mask = 0x0c;;
    for (i = 0; i < 4; i++)
    {
	SetAPen(rp,i<<2);
	RectFill(rp,i*50+5,20,i*50+55,99);
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

    CDPutStr("\33[20h");		/* line feed new line mode */


    {
	struct Preferences preferences;
	int signal, oldPrinterType;
	char ch, *t, buffer[2];

	GetPrefs(&preferences, sizeof(preferences));
	oldPrinterType = preferences.PrinterType;

	signal = AllocSignal(-1);
	x = 0;
	t = "#Test ";	/* check odd cdio string code */
	while (x <= HP_LASERJET_PLUS) {
	    preferences.PrinterType = x;
	    if (x >= HP_LASERJET)
		preferences.PrinterPort = SERIAL_PRINTER;
	    else
		preferences.PrinterPort = PARALLEL_PRINTER;
	    SetPrefs(&preferences, sizeof(preferences), FALSE);
	    if (err=pOpen(signal)) {

		CDPutStr("Test for PrinterType ");
		if (x > 9) {
		    CDPutChar('1');
		    CDPutChar('0'+x-10);
		}
		else
		    CDPutChar('0'+x);
		CDPutStr(" not available (pOpen error).\n");
		x++;
	    }
	    else {
		CDPutStr(&t[1]);
		CDPutStr(((struct PrinterData *) printerIO.io_Device)->
			pd_SegmentData->ps_PED.ped_PrinterName);
		CDPutStr("? (y,n)[n] ");
		CDPutChar(ch = CDGetChar());
		if (ch != '\n') CDPutChar('\n');
		if (ch == 'y') {
		    pWrite(((struct PrinterData *) printerIO.io_Device)->
			    pd_SegmentData->ps_PED.ped_PrinterName);
		    pWrite("\33[20h");
		    pWrite(" Alpha Test\n");
	    pWrite("\33[0z1/8\" line spacing\n");
		    for (i = 0; i < 4; i++)
			pWrite("123456789+123456789+123456789+123456789+123456789+123456789+\n");
		    pWrite("\33[1z1/6\" line spacing\n");
		    for (i = 0; i < 4; i++)
			pWrite("123456789+123456789+123456789+123456789+123456789+123456789+\n");
		    pWrite("Single character writes\n");
		    buffer[1] = 0;
		    for (i = 0; i < 4; i++) {
			for (j = 1; j <= 60; j++) {
			    k = j % 10;
			    if (k == 0)
				buffer[0] = '+';
			    else
				buffer[0] = '0'+k;
			    pWrite(buffer);
			}
			pWrite("\n");
		    }
		    printerIO2 = printerIO;
		    printerIO.io_Command = CMD_WRITE;
		    printerIO.io_Data = "----.----1----.----2-Null Terminated Write -.----5----.----6----.----7----.----8\n";
		    printerIO.io_Length = -1;
		    DoIO(&printerIO);
		    printerIO2 = printerIO;
		    printerIO.io_Command = CMD_WRITE;
		    printerIO.io_Data = "----.----1----.----2-Asynchronous write #1 -.----5----.----6----.----7----.----8\n!!!!";
		    printerIO.io_Length = 81;
		    SendIO(&printerIO);
		    printerIO2.io_Command = CMD_WRITE;
		    printerIO2.io_Data = "----.----1----.----2-Asynchronous write #2 -.----5----.----6----.----7----.----8\n!!!!";
		    printerIO2.io_Length = 81;
		    SendIO(&printerIO2);
		    WaitIO(&printerIO);
		    WaitIO(&printerIO2);
		    pWrite("----.----1----.----2----.----3----.----4----.----5----.----6----.----7----.----8----.----9----.---10----.---11----.---12----.---13----.---14----.---15----.---16----.---17----.---18----.---19----.---20----.---21----.---22----.---23----.---24----.---25----.---26----.---27----.---28----.---29----.---30\33");
		    pWrite("ENewline(^[E) started @ previous line\n");
		    pWrite("broken control sequence, 1 char apiece:");
		    pWrite("\33"); pWrite("D");
		    pWrite("Linefeed(^[D)?\n");
		    pWrite("\2331;3;4msingle character <CSI> style set\n");
		    pWrite("about to initialize...");
		    pWrite("\33#1Initialize(1)\nLinefeed(\\n)\n");
		    pWrite("\33[20hNewline mode on: \nLinefeed(\\n)\n");
		    pWrite("\33[20lNewline mode off: \nLinefeed(\\n)\n");
		    pWrite("\33DLineFeed(2)");
		    pWrite("\33#1Initialize(1)\33DLineFeed(2)");
		    pWrite("\33ENewLine(3)\33MReverseLF(4)\n\n");
		    pWrite("\nLineFeed(\\n)\n");
		    pWrite("\33[0mNormal\33[3m Italics On\33[23m and Off\33[4m Underline On");
		    pWrite("\33[24m and Off\33[1m Boldface On\33[22m and Off\n");
		    pWrite("\33[");
		    pWrite("1;3;4mEverything on, w/ multiple parameters");
		    pWrite(" & broken control string");
		    pWrite("\33[0m and off.\n");
		    pWrite("\33[0wNormal Pitch\n");
		    pWrite("\33[2wElite On\33[1m Bold\33[3m Bold Italic");
		    pWrite("\33[22m Italic\33[0m Normal\33[1w and Off\n");
		    pWrite("\33[4wCondensed On\33[1m Bold\33[3m Bold Italic");
		    pWrite("\33[22m Italic\33[0m Normal\33[3w and Off\n");
		    pWrite("\33[6wEnlarged On\33[1m Bold\33[3m Bold Italic");
		    pWrite("\33[22m Italic\33[0m Normal\33[5w and Off\n");
		    pWrite("\33[6\"zShadow Print On\33[1m Bold\33[3m Bold ");
		    pWrite("Italic\33[22m Italic\33[0m Normal\33[5\"z ");
		    pWrite("and Off\n");
		    pWrite("\33[4\"zDouble Strike On\33[1m Bold\33[3m Bold ");
		    pWrite("Italic\33[22m Italic\33[0m Normal\33[3\"z ");
		    pWrite("and Off\n");
		    pWrite("\33[2\"zNear Letter Quality On\33[1m Bold\33[3m ");
		    pWrite("Bold Italic\33[22m Italic\33[0m Normal\33[1\"z ");
		    pWrite("and Off\n");
		    pWrite("\33[2pProportional On\33[1m Bold\33[3m Bold ");
		    pWrite("Italic\33[22m Italic\33[0m Normal\33[1p and Off\n");
		    pWrite("\33[2pProportional On\33[5 E Thin Space 5");
		    pWrite("\33[20 E Thin Space 20\33[0p Proportional Clear");
		    pWrite("\33[1p and Off\n");
		    pWrite("\33[0 EThin space 0 just in case.\n");
		    pWrite("\33[30mColor Black (0)  ");
		    pWrite("\33[31mColor Red (1)  ");
		    pWrite("\33[32mColor Green (2)  ");
		    pWrite("\33[33mColor Yellow (3)\n");
		    pWrite("\33[34mColor Blue (4)  ");
		    pWrite("\33[35mColor Magenta (5)  ");
		    pWrite("\33[36mColor Cyan (6)  ");
		    pWrite("\33[37mColor White (7)\n");
		    pWrite("\33[39mColor Default (9)\n");
		    pWrite("\33[40mBgColor Black (0)  ");
		    pWrite("\33[41mBgColor Red (1)  ");
		    pWrite("\33[42mBgColor Green (2)  ");
		    pWrite("\33[43mBgColor Yellow (3)\n");
		    pWrite("\33[44mBgColor Blue (4)  ");
		    pWrite("\33[45mBgColor Magenta (5)  ");
		    pWrite("\33[46mBgColor Cyan (6)  ");
		    pWrite("\33[47mBgColor White (7)\n");
		    pWrite("\33[49mBgColor Default (9)\n");
		    pWrite("\n\33[2\"\nStart,\33[2v Superscript On\33[1v and Off\n");
		    pWrite("\nStart,\33[4v Subscript On\33[3v and Off\n");
		    pWrite("\nStart,\33[4vSub\33[4vSub\33[2vSuper");
		    pWrite("\33[0vNormalize\33LPLU\33KPLD\33KPLD\33[0vNormalize\n");
		    pWrite("\nStart,\33LPLU,\33LPLU,\33KPLD,\33KPLD,\33[0vStart?");
		    pWrite("\n");
		    pWrite("----.----1----\33#9<----2----.----3----.----4----.----5----\33#0>----6----.----7----.\n");
		    pWrite("Left and right margins set by moving to them.\n");
		    pWrite(".----2----.----3----.----4----.----5----.\n");

		    pWrite("\33[5 FAuto Left Justified\n");
		    pWrite("\33[7 FAuto Right Justified\n");
		    pWrite("\33[6 FAuto Full Justified (centered)\n");
		    pWrite("\33[2 FInterword Space (auto center!?!)\n");
		    pWrite("\33[3 FLetter Space (justify!?!)\n");
		    pWrite("\33[0 FAuto Justify Off\n");
		    pWrite("\33#3Clear Margins\n");
		    pWrite("----.----1----.----2----.----3----.----4----.----5----.----6----.----7----.\n");
		    pWrite("----.----1----.----<----.----3----.----4----.---->----.----6----.----7----.\n");
		    pWrite("\33[20;50sLeft and right margins set by setting them.\n");
		    pWrite("2----.----3----.----4----.----5\n");
		    pWrite("\33#3Clear Margins\n");
		    pWrite("----.----1----.----2----.----3----.----4----.----5----.----6----.----7----.\n");
		    pWrite("Initial\11Tabs\11T\11T\11T\11T\11T\11T\n");
		    pWrite("\33[3gClear all Horizontal Tabs\11T\11T\11T\n");
		    pWrite("Set Tabs Here \33HV and here \33HV\n");
		    pWrite("\11T\11T\11T\n");
		    pWrite("\11\33[0gClear this tab\n");
		    pWrite("\11T\11T\11T\n");
		    pWrite("\33#4Clear all H & V Tabs\11T\11T\11T\n");
		    pWrite("\33#5Set default tabs\n");
		    pWrite("\11T\11T\11T\11T\11T\11T\11T\11T\11T\n");
		    pWrite("\33(B#$@[\]^`{|}~ US\n");
		    pWrite("\33(R#$@[\]^`{|}~ French\n");
		    pWrite("\33(K#$@[\]^`{|}~ German\n");
		    pWrite("\33(A#$@[\]^`{|}~ UK\n");
		    pWrite("\33(E#$@[\]^`{|}~ Danish I\n");
		    pWrite("\33(H#$@[\]^`{|}~ Sweden\n");
		    pWrite("\33(Y#$@[\]^`{|}~ Italian\n");
		    pWrite("\33(Z#$@[\]^`{|}~ Spanish\n");
		    pWrite("\33(J#$@[\]^`{|}~ Japanese\n");
		    pWrite("\33(6#$@[\]^`{|}~ Norwegian\n");
		    pWrite("\33(C#$@[\]^`{|}~ Danish II\n");
		    pWrite("Vertical Tabs & Forms not tested.\n");
		    /* ESC[nt  set form length n 		DEC
		    /* ESC[nq  perf skip n (n>0)		+++
		    /* ESC[0q  perf skip off  			+++
		    /* ESC#8  Top margin set 			+++
		    /* ESC#2  Bottom marg set 			+++
		    /* ESC[Pn1;Pn2r  T&B margins 		DEC
		    /* ESCJ    Set vertical tabs 		ISO
		    /* ESC[1g  Clr vertical tabs 		ISO
		    /* ESC[4g  Clr all v tabs 			ISO
		    /* ESC[Pn"x extended commands       	+++ 
		    /**/
		    pWrite("End of Alpha Test.\n");

		    CDPutStr("Hit a key to test reset command...");
		    CDGetChar();
		    CDPutStr("\n");
		    pWrite("Reset Test...\n");
		    pWrite("\33cThis text occurs after the Reset code.\n");
		    pWrite("End of Reset Test.\n");

		    if(((struct PrinterData *) printerIO.io_Device)->
			    pd_SegmentData->ps_PED.ped_PrinterClass & PPCF_GFX) {
			CDPutStr("Hit a key to start B & W graphics test...");
			CDGetChar();
			CDPutStr("\n");
			((struct PrinterData *) printerIO.io_Device)->
				pd_Preferences.PrintShade = SHADE_GREYSCALE;
			if(((struct PrinterData *) printerIO.io_Device)->
				pd_SegmentData->ps_PED.ped_ColorClass
				== PCC_YMC_BW) {
			    (*(((struct PrinterData *)
				    printerIO.io_Device)->
				    pd_SegmentData->ps_PED.ped_Open))();
			}
			vp = &s[0]->ViewPort;
			pWrite("Black & White Graphics Test\n\n");
			pDumpRPort(&s[0]->RastPort, vp->ColorMap, vp->Modes,
			    0,0, s[0]->Width,s[0]->Height,
			    0,2000, SPECIAL_MILROWS);
			pWrite("\n\nEnd of Black & White Graphics Test\n\n");
			if(((struct PrinterData *) printerIO.io_Device)->
				pd_SegmentData->ps_PED.ped_ColorClass
				== PCC_YMC_BW) {
			    CDPutStr("Put in color ribbon and ");
			}
			if(((struct PrinterData *) printerIO.io_Device)->
				pd_SegmentData->ps_PED.ped_PrinterClass
				& PPCF_COLOR) {
			    CDPutStr("Hit a key to start color test...");
			    CDGetChar();
			    CDPutStr("\n");
			    ((struct PrinterData *) printerIO.io_Device)->
				    pd_Preferences.PrintShade = SHADE_COLOR;
			    if(((struct PrinterData *) printerIO.io_Device)->
				    pd_SegmentData->ps_PED.ped_ColorClass
				    == PCC_YMC_BW) {
				(*(((struct PrinterData *)
					printerIO.io_Device)->
					pd_SegmentData->ps_PED.ped_Open))();
			    }
			    vp = &s[0]->ViewPort;
			    pWrite("\n\nColor Graphics Test I\n\n");
			    pDumpRPort(&s[0]->RastPort, vp->ColorMap, vp->Modes,
				0,0, s[0]->Width,s[0]->Height, 0,0, 0);
			    pWrite("\n\nColor Graphics Test II\n\n");
			    vp = &s[1]->ViewPort;
			    pDumpRPort(&s[1]->RastPort, vp->ColorMap, vp->Modes,
				0,0, s[1]->Width,s[1]->Height, s[1]->Width,0, 0);
			    pWrite("\n\nEnd of Color Graphics Test\n\n");
			}
		    }
		}
		else
		    x++;
		pClose();
	    }
	}
	preferences.PrinterType = oldPrinterType;
	SetPrefs(&preferences, sizeof(preferences), FALSE);
    }
    CDClose();
    CloseWindow(w[0]);
    CloseWindow(w[1]);
    CloseWindow(w[2]);
    CloseScreen(s[0]);
    CloseScreen(s[1]);
    CloseScreen(s[2]);
}
