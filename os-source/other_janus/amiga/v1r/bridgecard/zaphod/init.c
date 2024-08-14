
/* ***************************************************************************
 * 
 * Initialization Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 7 Mar 86	=RJ Mical=	Created this file from misc.c
 *
 * **************************************************************************/

#include "zaphod.h"
						

InitZaphod(display)
struct Display *display;
{
    struct TextFont *localFont;
    SHORT i;
    struct Preferences *prefs;

    if (IntuitionBase == NULL)
	IntuitionBase = (struct IntuitionBase *)
		OpenLibrary("intuition.library", 0);
    if (GfxBase == NULL)
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0);
    if (LayersBase == NULL)	  
	LayersBase = (struct LayersBase *)OpenLibrary("layers.library", 0);
    if (DiskfontBase == NULL)
	DiskfontBase = (struct DiskfontBase *)
		OpenLibrary("diskfont.library", 0);

#ifdef JANUS
    if (JanusBase == NULL)
	JanusBase = (struct JanusBase *)OpenLibrary("janus.library", 0);

    if ( (GfxBase == NULL) || (IntuitionBase == NULL)
	    || (LayersBase == NULL) || (DiskfontBase == NULL)
	    || (JanusBase == NULL) )
	Abort(ALERT_NO_LIBRARY, NULL);
#else
    if ( (GfxBase == NULL) || (IntuitionBase == NULL) 
	    || (LayersBase == NULL) || (DiskfontBase == NULL) ) 
	Abort(ALERT_NO_LIBRARY, NULL);
#endif

    for (i = 0; i < (CHAR_HEIGHT * BUFFER_WIDTH); i++)
	{
	AllSetPlane[i] = -1;
	AllClearPlane[i] = 0;
	}

    ReadKeyTable();

    if (NOT ReadSettingsFile(NULL, FALSE))
	{
	if (prefs = AllocMem(sizeof(struct Preferences), NULL))
	    {
	    GetPrefs(prefs, sizeof(struct Preferences));
	    TextTwoPlaneColors[0] = prefs->color0;
	    TextTwoPlaneColors[1] = prefs->color1;
	    TextTwoPlaneColors[2] = prefs->color2;
	    TextTwoPlaneColors[3] = prefs->color3;
	    FreeMem(prefs, sizeof(struct Preferences));
	    }
	} 

/* === Get our special compressed-data font ============================= */
    if (FontData == NULL)
	{
	if ((localFont = OpenDiskFont(&PCFont)) == NULL)
	    Abort(ALERT_NO_PCFONT, NULL);

	if ((NormalFont = AllocRemember(&GlobalKey, (256 * 8 * 2), 0))
		== NULL)
	    Abort(ALERT_NO_MEMORY, NULL);

	CopyFont(localFont->tf_CharData, NormalFont);

	if ((display->Modes & COLOR_DISPLAY) == NULL)
	    {
	    if ((UnderlineFont = AllocRemember(&GlobalKey, (256 * 8 * 2), 0))
		    == NULL)
		Abort(ALERT_NO_MEMORY, NULL);
	    CopyFont(localFont->tf_CharData, UnderlineFont);
	    Underliner();    /* Go add the underlining to the underline font */
	    }

	CloseFont(localFont);
	}
}



CopyFont(charData, bufptr)
UBYTE *charData, *bufptr;
{
    SHORT i, i2;

    for (i = 0; i < 256; i++)
	{
	for (i2 = 0; i2 < (8 * 256); i2 += 256)
	    {
	    *bufptr = *(charData + i2);
	    bufptr++;
	    *bufptr = 0;
	    bufptr++;
	    }
	charData++;
	}
}


