
/* *** init.c ****************************************************************
 * 
 * Initialization Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 28 Jul 88   -RJ         Changed InitZaphod() to return BOOL, now use 
 *                         Alert() rather than Abort() with errors and 
 *                         return FALSE to callers if any error
 * 27 Jul 88   -RJ         Moved data file read calls to before OpenAuxTools()
 *                         so that IMTask will have valid key definitions 
 *                         when it begins to run
 * 7 Mar 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
#include <janus/janusbase.h>


extern	UBYTE	*MakeBytePtr();
extern	USHORT	*MakeWordPtr();



BOOL	InitZaphod(display)
REGISTER struct Display *display;
{
	REGISTER struct TextFont *localFont;
	REGISTER SHORT i;
	REGISTER UBYTE *ptr;
	BOOL	retvalue;

	retvalue = FALSE;

	if (IntuitionBase == NULL)
		IntuitionBase = (struct IntuitionBase *)
				OpenLibrary((UBYTE *)"intuition.library", 0);

	MakeWaitWindow();

	if (GfxBase == NULL)
		GfxBase = (struct GfxBase *)OpenLibrary((UBYTE *)"graphics.library", 0);
	if (LayersBase == NULL)		  
		LayersBase = (struct LayersBase *)OpenLibrary((UBYTE *)"layers.library",
				0);
	if (DiskfontBase == NULL)
		DiskfontBase = (struct DiskfontBase *)
				OpenLibrary((UBYTE *)"diskfont.library", 0);

	if (JanusBase == NULL)
		JanusBase = (struct JanusBase *)OpenLibrary(
				(UBYTE *)"janus.library", 0);

	if ( (GfxBase == NULL) || (IntuitionBase == NULL)
			|| (LayersBase == NULL) || (DiskfontBase == NULL)
			|| (JanusBase == NULL) )
		{
		Alert(ALERT_NO_LIBRARY, NULL);
		goto DONE;
		}

	/* These two lines take advantage of the fact that we know that 
	 * the JanusAmiga structure resides at the first memory location 
	 * of Janus Parameter Memory.  We feel it's safe to presume this 
	 * because it's a supposedly unchangeable presumption used by 
	 * the original Janus designers (both Amiga and PC sides) to 
	 * locate JanusAmiga, as witnessed by the fact that JanusBase 
	 * has no explicit pointer to JanusAmiga and the PC code presumes 
	 * everywhere that JanusAmiga starts at location zero of janus mem.
	 */
	JanusAmigaBA = (struct JanusAmiga *)MakeBytePtr(JanusBase->jb_ParamMem);
	JanusAmigaWA = (struct JanusAmiga *)MakeWordPtr(JanusAmigaBA);


#ifdef AZTEC
	/* This pushes A4 into a memory location.  This is needed because
	 * Manx has implemented relative-addressing using A4.  
	 * Tasks must call GetA4(); to load the A4 register with the base.
	 * Task object files should be clustered around the call to SaveA4();
	 */
	SaveA4();
#endif

	MainTask = FindTask(0);

	ReadKeyTable();
	ReadScanCodeTable();

	OpenAuxTools(display);

	if (DuplicateDisplay(display)) 
		{
		Alert(ALERT_DUPLICATE_DISPLAY, NULL);
		goto DONE;
		}

	/* "If we're in text mode, then allocate only enough memory for one
	 * page of text.  If we're in graphics mode, we'll need the whole 16K."
	 * Nah, just grab all 16K and to hell with it.
	 * Turns out we might need all of it.
	 */
	i = 0x4000;

	display->Buffer = AllocRemember(&display->BufferKey, i, MEMF_CHIP);
	AllSetPlane = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
	AllClearPlane = AllocRemember(&GlobalKey, 80 * CHAR_HEIGHT, MEMF_CHIP);
	SavePrefs = (struct Preferences *)AllocRemember(&GlobalKey, 
			sizeof(struct Preferences), NULL);

	if (display->Buffer && AllSetPlane && AllClearPlane && SavePrefs)
		{
		for (i = 0; i < (CHAR_HEIGHT * BUFFER_WIDTH); i++)
			{
			AllSetPlane[i] = -1;
			AllClearPlane[i] = 0;
			}
		GetPrefs(SavePrefs, sizeof(struct Preferences));
		}
	else
		{
		Alert(ALERT_NO_MEMORY, display);
		goto DONE;
		}

	if (NOT ReadSettingsFile(display, FALSE))
		{
		TextTwoPlaneColors[0] = SavePrefs->color0;
		TextTwoPlaneColors[1] = SavePrefs->color1;
		TextTwoPlaneColors[2] = SavePrefs->color2;
		TextTwoPlaneColors[3] = SavePrefs->color3;
		} 

	/* === Get our special compressed-data font ========================== */
	if (FontData == NULL)
		{
		if ((localFont = OpenDiskFont(&PCFont)) == NULL)
			{
			Alert(ALERT_NO_PCFONT, NULL);
			localFont = OpenFont(&SafeFont);
			}

		if ((NormalFont = AllocRemember(&GlobalKey, (256 * 8 * 2),
				MEMF_CHIP | MEMF_CLEAR)) == NULL)
			{
			Alert(ALERT_NO_MEMORY, NULL);
			goto DONE;
			}

		CopyFont(localFont, NormalFont);

		if ((display->Modes & COLOR_DISPLAY) == NULL)
			{
			if ((UnderlineFont = AllocRemember(&GlobalKey, (256 * 8 * 2),
					MEMF_CHIP | MEMF_CLEAR)) == NULL)
				{
				Alert(ALERT_NO_MEMORY, NULL);
				goto DONE;
				}

			CopyFont(localFont, UnderlineFont);
			Underliner();	/* Go add the underlining to the underline font */
			}

		CloseFont(localFont);
		}

	if (OpenTextClip() == FALSE)
		{
		Alert(ALERT_INCOMPLETESYSTEM, NULL);
		goto DONE;
		}

	TopBorderOn = NTSC_TOP_ONHEIGHT;
	TopBorderOff = NTSC_TOP_OFFHEIGHT;
/*BBB	BottomBorderOn = NTSC_BOTTOM_ONHEIGHT;*/
/*BBB	BottomBorderOff = NTSC_BOTTOM_OFFHEIGHT;*/
	ClearFlag(display->Modes, PAL_PRESENCE);

	GlobalScreenHeight = GfxBase->NormalDisplayRows;
	if (GlobalScreenHeight >= ZAPHOD_HEIGHT + PAL_EXTRAHEIGHT)
		{
		TopBorderOn = PAL_TOP_ONHEIGHT;
		TopBorderOff = PAL_TOP_OFFHEIGHT;
/*BBB		BottomBorderOn = PAL_BOTTOM_ONHEIGHT;*/
/*BBB		BottomBorderOff = PAL_BOTTOM_OFFHEIGHT;*/
		SetFlag(display->Modes, PAL_PRESENCE);
		NewWindow.TopEdge = PAL_TOP_ONHEIGHT;
		}

	retvalue = TRUE;

DONE:
	return(retvalue);
}



CopyFont(font, bufptr)
struct TextFont *font;
REGISTER UBYTE *bufptr;
{
	REGISTER SHORT i, i2;
	REGISTER UBYTE *charData, *thisChar;
	REGISTER SHORT height, modulo;
	SHORT firstchar;
	REGISTER WORD *charloc;

	charData = (UBYTE *)font->tf_CharData;
	height = font->tf_YSize;
	modulo = font->tf_Modulo;
	firstchar = font->tf_LoChar;
	charloc = (WORD *)font->tf_CharLoc;

	bufptr += (firstchar * height * 2);

	for (i = firstchar; i <= font->tf_HiChar; i++)
		{
		thisChar = charData + ((*charloc) >> 3);
		charloc += 2;

		for (i2 = 0; i2 < (height * modulo); i2 += modulo)
			{
			*bufptr++ = *(thisChar + i2);
			*bufptr++ = 0;
			}
		}
}



InitDisplayBitMap(display, superwindow, width)
REGISTER struct Display *display;
REGISTER struct SuperWindow *superwindow;
REGISTER SHORT width;
{
	REGISTER SHORT i, i2;
	REGISTER UBYTE *buffer;

	InitBitMap(&display->BufferBitMap, superwindow->DisplayDepth,
			width, ZAPHOD_HEIGHT);
	InitBitMap(&display->TextBitMap, superwindow->DisplayDepth, 
			BUFFER_WIDTH * CHAR_WIDTH, CHAR_HEIGHT);

	i = (width >> 3) * ZAPHOD_HEIGHT;	/* This are for graphics mode */ 
	i2 = i;
	buffer = display->Buffer;

	display->BufferBitMap.Planes[0] = buffer;
	display->BufferBitMap.Planes[1] = buffer + i2;
	/* These four should never be used, but what the hell */
	i2 += i;
	display->BufferBitMap.Planes[2] = buffer + i2;
	i2 += i;
	display->BufferBitMap.Planes[3] = buffer + i2;
	i2 += i;
	display->BufferBitMap.Planes[4] = buffer + i2;
	i2 += i;
	display->BufferBitMap.Planes[5] = buffer + i2;
}



