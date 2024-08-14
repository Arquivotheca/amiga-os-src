
/* *** fileio.c **************************************************************
 * 
 * File Input/Output Routines  --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 14 Apr 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"

#define FILE_BUFFER_SIZE	 82

BYTE FileBuffer[FILE_BUFFER_SIZE];
SHORT FileIndex;
LONG FileNumber;

extern UBYTE ASCIIToPCTable[];

	

struct	Settings
	{
	SHORT	DefaultColorTextDepth;
	SHORT	TextOnePlaneColors[2];
	SHORT	TextTwoPlaneColors[4];
	SHORT	TextThreePlaneColors[8];
	SHORT	TextFourPlaneColors[16];
	SHORT	HighGraphicsColors[2];
	SHORT	LowGraphicsColors[2][2][4];
	SHORT	PresetMonoWidth;
	SHORT	PresetMonoHeight;
	SHORT	PresetMonoTopEdge;
	SHORT	PresetMonoLeftEdge;
	SHORT	PresetMonoDepth;
	SHORT	PresetColorWidth;
	SHORT	PresetColorHeight;
	SHORT	PresetColorTopEdge;
	SHORT	PresetColorLeftEdge;
	SHORT	PresetMonoFlags;
	SHORT	PresetColorFlags;
	};

struct	Settings Settings;

 
/* The numeric input file format rules:
 *  - Anywhere that a ';' character appears, the rest of the line is 
 *	regarded as a comment and ignored
 *  - Numbers are separated by any non-numeric character, including
 *	blanks, spaces, end-of-line characters, the letter z, and any
 *	other delimiter you care to use.		Any word that starts with a
 *	'$' is treated as a hexadecimal number.
 */


BOOL GetNextNumber(numaddress, bufptr, indexaddress, filenumber)
REGISTER SHORT *numaddress, *indexaddress;
REGISTER BYTE *bufptr;
LONG filenumber;
/*
 * When a number is found, it is saved in the memory location
 * pointed to by numaddress, bufptrptr is advanced to point to the first
 * non-numeric character, and this routine returns TRUE.  If no number
 * is found, numaddress and bufptrptr are unchanged and this routine 
 * returns FALSE.
 */
{
	REGISTER BYTE thischar;
	REGISTER SHORT index, result, base;

	result = 0;
	index = *indexaddress;
	base = 10;

	FOREVER
		{
		while (thischar = *(bufptr + index))
			{
			if ( ((thischar >= '0') && (thischar <= '9')) || (thischar == '$'))
				{
				if (thischar == '$')
					{
					base = 16;
					thischar = '0';
					}

				thischar -= '0';

				do
					{
					index++;
					result = (result * base) + thischar;
					thischar = *(bufptr + index);
					if (base == 10) thischar -= '0';
					else
						{
						if ((thischar >= 'a') && (thischar <= 'z')) 
							thischar = thischar - 'a' + 'A';
						if ((thischar >= '0') && (thischar <= '9'))
							thischar -= '0';
						else
							thischar = thischar - 'A' + 10;
						}
					}
				while ((thischar >= 0) && (thischar < base));
				*indexaddress = index;
				*numaddress = result;
				return (TRUE);
				}
			 
			index++;

			if (thischar == ';') 
				{
				if (XReadLine(bufptr, FILE_BUFFER_SIZE) == -1) return (FALSE);

				index = 0;
				}
			}
		if (XReadLine(bufptr, FILE_BUFFER_SIZE) == -1) return (FALSE);
		index = 0;
		}
}





BOOL ShortGetNextNumber(num)
SHORT *num;
{
	return(GetNextNumber(num, &FileBuffer[0], &FileIndex, FileNumber));
}



VOID ReadKeyTable()
{
	SHORT num;
	REGISTER SHORT i;

	FileIndex = 0;
	FileBuffer[0] = 0;
	if ( (FileNumber = Open(&SidecarKeysFile[0], MODE_OLDFILE)) == 0)
		{
		Alert(ALERT_BAD_KEYS_FILE, NULL);
		return;
		}

	InitXRead(FileNumber);

	ShortGetNextNumber(&num);	 PCAltCode = num;
	ShortGetNextNumber(&num);	 PCCapsLockCode = num;
	ShortGetNextNumber(&num);	 PCCtrlCode = num;
	ShortGetNextNumber(&num);	 PCLeftShiftCode = num;
	ShortGetNextNumber(&num);	 PCNumLockCode = num;
	ShortGetNextNumber(&num);	 PCPlusCode = num;
	ShortGetNextNumber(&num);	 PCPtrScrCode = num;
	ShortGetNextNumber(&num);	 PCRightShiftCode = num;
	ShortGetNextNumber(&num);	 PCScrollLockCode = num;
	ShortGetNextNumber(&num);	 PCTildeCode = num;
	ShortGetNextNumber(&num);	 PCBarCode = num;
	ShortGetNextNumber(&num);	 PCBackDashCode = num;
	ShortGetNextNumber(&num);	 PCBackSlashCode = num;

	ShortGetNextNumber(&num);	 AmigaCapsLockCode = num;
	ShortGetNextNumber(&num);	 AmigaLeftAltCode = num;
	ShortGetNextNumber(&num);	 AmigaNCode = num;
	ShortGetNextNumber(&num);	 AmigaPCode = num;
	ShortGetNextNumber(&num);	 AmigaPlusCode = num;
	ShortGetNextNumber(&num);	 AmigaRightAltCode = num;
	ShortGetNextNumber(&num);	 AmigaSCode = num;
	ShortGetNextNumber(&num);	 AmigaPrtScrCode = num;
	ShortGetNextNumber(&num);	 AmigaTildeBackDash = num;
	ShortGetNextNumber(&num);	 AmigaBarBackSlash = num;

	for (i = 0; i < 128; i++)
		{
		if ( ShortGetNextNumber(&num) == FALSE )
			{
			Alert(ALERT_BAD_KEYS_FILE, NULL);
			goto CLOSE_RETURN;
			}
		PCRawKeyTable[i] = num;
		}

	/* Is there another number to get?  If not, then we're using an 
	 * old file so use the presumably (poof) safe defaults.  
	 */
	if (ShortGetNextNumber(&num) == FALSE)
		{
		Alert(ALERT_OLD_KEYSFILE, NULL);
		goto CLOSE_RETURN;
		}
	PCKeypad0Code = num;
	ShortGetNextNumber(&num);	PCKeypad1Code = num;
	ShortGetNextNumber(&num);	PCKeypad2Code = num;
	ShortGetNextNumber(&num);	PCKeypad3Code = num;
	ShortGetNextNumber(&num);	PCKeypad4Code = num;
	ShortGetNextNumber(&num);	PCKeypad5Code = num;
	ShortGetNextNumber(&num);	PCKeypad6Code = num;
	ShortGetNextNumber(&num);	PCKeypad7Code = num;
	ShortGetNextNumber(&num);	PCKeypad8Code = num;
	ShortGetNextNumber(&num);	PCKeypad9Code = num;
	ShortGetNextNumber(&num);	PCKeypadDotCode = num;

	ShortGetNextNumber(&num);	AmigaCursorUp = num;
	ShortGetNextNumber(&num);	AmigaCursorLeft = num;
	ShortGetNextNumber(&num);	AmigaCursorRight = num;
	ShortGetNextNumber(&num);	AmigaCursorDown = num;
	ShortGetNextNumber(&num);	AmigaDelCode = num;

CLOSE_RETURN:
	Close(FileNumber);
}



VOID ReadScanCodeTable()
{
	SHORT num;
	REGISTER SHORT i;

	FileIndex = 0;
	FileBuffer[0] = 0;
	if ( (FileNumber = Open(&ScanCodeFile[0], MODE_OLDFILE)) == 0)
		{
		Alert(ALERT_BAD_SCANCODE_FILE, NULL);
		return;
		}

	InitXRead(FileNumber);

	for (i = 0; i < 128 * 2; i++)
		{
		if ( ShortGetNextNumber(&num) == FALSE )
			{
			Alert(ALERT_BAD_SCANCODE_FILE, NULL);
			goto CLOSE_RETURN;
			}
		ASCIIToPCTable[i] = num;
		}

CLOSE_RETURN:
	Close(FileNumber);
}



VOID ShortWrite(text, display)
REGISTER UBYTE *text;
REGISTER struct Display *display;
{
	if (Write(FileNumber, text, StringLength(text)) == -1)
		{
		Alert(ALERT_BAD_SETTINGS_FILE, display);
		}
}



VOID	SPrintfShortWrite(text, data, display)
REGIST	UBYTE	*text;
REGIST	ULONG	data;
REGIST	struct	Display *display;
{
	sprintf(&FileBuffer[0], text, data);
	ShortWrite(&FileBuffer[0], display);
}



BOOL	ReadSettingsGrunt(display, reporterrors)
REGIST	struct Display *display;
BOOL	reporterrors;
/* The display argument can be NULL if there's no display for this fetch */
{
	REGIST	SHORT i, i2;
	SHORT	num;
	BOOL	returnval;
	struct	Process *process;
	struct	Window *procwindow, *w;

	returnval = FALSE;

	if (display)
		if (NOT Lock(&display->DisplayLock)) goto RETURN;

	process = (struct Process *)FindTask(0);
	procwindow = (struct Window *)process->pr_WindowPtr;
	if (display)
		{
		if (display->FirstWindow)
			{
			if (w = display->FirstWindow->DisplayWindow)
				process->pr_WindowPtr = (APTR)w;
			}
		}

	if ( (FileNumber = Open(&SidecarSettingsFile[0], MODE_OLDFILE)) == 0)
		{
		if (reporterrors) Alert(ALERT_BAD_SETTINGS_FILE, display);
		goto UNLOCK_RETURN;
		}

	FileIndex = 0;
	FileBuffer[0] = 0;

	InitXRead(FileNumber);

	/* Get the Flag word */
	ShortGetNextNumber(&num);

	/* Get the default cursor times */
	/* The routine has to be changed to work with longs */
	ShortGetNextNumber(&num);
	ShortGetNextNumber(&num);

	/* Get the default text depth */
	ShortGetNextNumber(&num);
	Settings.DefaultColorTextDepth = num;

	/* The default display priority */
	ShortGetNextNumber(&num);

	/* And restore the palettes */
	/* Text One Plane: */
	for (i = 0; i < 2; i++)
		{
		ShortGetNextNumber(&num);
		Settings.TextOnePlaneColors[i] = num;
		}
	/* Text Two Plane */
	for (i = 0; i < 4; i++)
		{
		ShortGetNextNumber(&num);
		Settings.TextTwoPlaneColors[i] = num;
		}
	/* Text Three Plane */
	for (i = 0; i < 8; i++)
		{
		ShortGetNextNumber(&num);
		Settings.TextThreePlaneColors[i] = num;
		}
	/* Text Four Plane */
	for (i = 0; i < 16; i++)
		{
		ShortGetNextNumber(&num);
		Settings.TextFourPlaneColors[i] = num;
		}
	/* High Graphics */
	for (i = 0; i < 2; i++)
		{
		ShortGetNextNumber(&num);
		Settings.HighGraphicsColors[i] = num;
		}
	/* Low Graphics Normal */
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			ShortGetNextNumber(&num);
			Settings.LowGraphicsColors[0][i][i2] = num;
			}
	/* Low Graphics Intensified */
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			ShortGetNextNumber(&num);
			Settings.LowGraphicsColors[1][i][i2] = num;
			}

	if (NOT ShortGetNextNumber(&num))
		{
		if (reporterrors) Alert(ALERT_BAD_SETTINGS_FILE, display);
		returnval = FALSE;
		goto CLOSE_RETURN;
		}

	if (display == NULL) goto CLOSE_RETURN;

	ShortGetNextNumber(&num);  Settings.PresetMonoWidth = num;
	ShortGetNextNumber(&num);  Settings.PresetMonoHeight = num;
	ShortGetNextNumber(&num);  Settings.PresetMonoTopEdge = num;
	ShortGetNextNumber(&num);  Settings.PresetMonoLeftEdge = num;
	ShortGetNextNumber(&num);  Settings.PresetMonoDepth = num;
	ShortGetNextNumber(&num);  Settings.PresetMonoFlags = num;

	ShortGetNextNumber(&num);  Settings.PresetColorWidth = num;
	ShortGetNextNumber(&num);  Settings.PresetColorHeight = num;
	ShortGetNextNumber(&num);  Settings.PresetColorTopEdge = num;
	ShortGetNextNumber(&num);  Settings.PresetColorLeftEdge = num;
	if (ShortGetNextNumber(&num))
		{
		Settings.PresetColorFlags = num;
		returnval = TRUE;
		}

CLOSE_RETURN:
	Close(FileNumber);

UNLOCK_RETURN:
	if (display)
		Unlock(&display->DisplayLock);

	process->pr_WindowPtr = (APTR)procwindow;

RETURN:
	return(returnval);
}



VOID	WriteSettingsFile(display)
REGIST	struct	Display *display;
{
	REGIST	SHORT i, i2;
	SHORT	num;
	REGIST	struct Process *process;
	REGIST	struct	Window *window;
	struct	Window *procwindow;
	struct	SuperWindow *swindow;

	if (display == NULL) return;
	if ((swindow = display->FirstWindow) == NULL) return;
	if ((window = swindow->DisplayWindow) == NULL) return;

	if (NOT Lock(&display->DisplayLock)) return;

	process = (struct Process *)FindTask(0);
	procwindow = (struct Window *)process->pr_WindowPtr;
	process->pr_WindowPtr = (APTR)window;

	Settings.PresetMonoFlags = 0;
	Settings.PresetColorFlags = 0;
	ReadSettingsGrunt(display, FALSE);

	i = PRESETS_GOT;
	if (FlagIsSet(display->Modes, BORDER_VISIBLE)) 
		i |= PRESET_BORDER_ON;
	if (FlagIsClear(display->Modes, MEDIUM_RES)) 
		i |= PRESET_HIRES;
	if (FlagIsSet(swindow->Flags, WANTS_PRIVACY))
		i |= PRESET_PRIVACY;
	/* Set new settings for this display type */
	if (FlagIsClear(display->Modes, COLOR_DISPLAY))
		{
		/* Reset the Mono Settings */
		Settings.PresetMonoWidth = window->Width;
		Settings.PresetMonoHeight = window->Height;
		Settings.PresetMonoTopEdge = window->TopEdge;
		Settings.PresetMonoLeftEdge = window->LeftEdge;
		Settings.PresetMonoDepth = swindow->TextDepth;

		Settings.PresetMonoFlags = i;
		}
	else if (FlagIsSet(display->Modes, COLOR_DISPLAY))
		{
		/* Reset the Color Settings */
		Settings.PresetColorWidth = window->Width;
		Settings.PresetColorHeight = window->Height;
		Settings.PresetColorTopEdge = window->TopEdge;
		Settings.PresetColorLeftEdge = window->LeftEdge;

		Settings.PresetColorFlags = i;

		Settings.DefaultColorTextDepth = DefaultColorTextDepth;
		}

	/* If there's already a settings file out there, zap it! */
	DeleteFile(&SidecarSettingsFile[0]);

	if ( (FileNumber = Open(&SidecarSettingsFile[0], MODE_NEWFILE)) == 0)
		{
		Alert(ALERT_BAD_SETTINGS_FILE, display);
		goto UNLOCK_RETURN;
		}

	ShortWrite("; Sidecar Settings File\n;\n; So, how's it going, eh?\n", 
			display);
	ShortWrite("; RJ Mical sends his greetings!\n;\n\n", 
			display);

	num = 0;
	SPrintfShortWrite("; Flag Word:\n	$%04lx\n\n", num, display);
	
	sprintf(&FileBuffer[0], "; Default Cursor Rate:\n	%02ld %06ld\n\n",
			swindow->CursorSeconds, swindow->CursorMicros);
	ShortWrite(&FileBuffer[0], display);
					 
	SPrintfShortWrite("; Default Color Text Depth:\n	%1ld\n\n", 
			Settings.DefaultColorTextDepth, display);

	SPrintfShortWrite("; Display Task Priority:\n	%02ld\n\n", 
			DisplayPriority, display);

	ShortWrite("; Default Palettes:\n", display);
	ShortWrite("; Text One Plane:\n	", display);
	for (i = 0; i < 2; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				TextOnePlaneColors[i], display);
		}
	ShortWrite("\n; Text Two Plane:\n	", display);
	for (i = 0; i < 4; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				TextTwoPlaneColors[i], display);
		}
	ShortWrite("\n; Text Three Plane:\n	", display);
	for (i = 0; i < 8; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				TextThreePlaneColors[i], display);
		}
	ShortWrite("\n; Text Four Plane:\n	", display);
	for (i = 0; i < 8; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				TextFourPlaneColors[i], display);
		}
	ShortWrite("\n	");
	for (i = 8; i < 16; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				TextFourPlaneColors[i], display);
		}
	ShortWrite("\n; High Graphics:\n	", display);
	for (i = 0; i < 2; i++)
		{
		SPrintfShortWrite("$%04lx ", 
				HighGraphicsColors[i], display);
		}
	ShortWrite("\n; Low Graphics Normal:\n	", display);
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			SPrintfShortWrite("$%04lx ", 
				LowGraphicsColors[0][i][i2], display);
			}
	ShortWrite("\n; Low Graphics Intensified:\n	", display);
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			SPrintfShortWrite("$%04lx ", 
				LowGraphicsColors[1][i][i2], display);
			}

	ShortWrite("\n\n; Is reality truly existential?  What do you think:");
	ShortWrite("\n	$FFFF\n\n");

	SPrintfShortWrite("; Mono Width\n\t%ld\n", 
			Settings.PresetMonoWidth, display);
	SPrintfShortWrite("; Mono Height\n\t%ld\n", 
			Settings.PresetMonoHeight, display);
	SPrintfShortWrite("; Mono TopEdge\n\t%ld\n", 
			Settings.PresetMonoTopEdge, display);
	SPrintfShortWrite("; Mono LeftEdge\n\t%ld\n", 
			Settings.PresetMonoLeftEdge, display);
	SPrintfShortWrite("; Mono Depth\n\t%ld\n", 
			Settings.PresetMonoDepth, display);
	SPrintfShortWrite("; Mono Flags\n\t%ld\n", 
			Settings.PresetMonoFlags, display);

	SPrintfShortWrite("; Color Width\n\t%ld\n", 
			Settings.PresetColorWidth, display);
	SPrintfShortWrite("; Color Height\n\t%ld\n", 
			Settings.PresetColorHeight, display);
	SPrintfShortWrite("; Color TopEdge\n\t%ld\n", 
			Settings.PresetColorTopEdge, display);
	SPrintfShortWrite("; Color LeftEdge\n\t%ld\n", 
			Settings.PresetColorLeftEdge, display);
	SPrintfShortWrite("; Color Flags\n\t%ld\n", 
			Settings.PresetColorFlags, display);

	Close(FileNumber);
								  
UNLOCK_RETURN:
	process->pr_WindowPtr = (APTR)procwindow;

	Unlock(&display->DisplayLock);
}

 

BOOL ReadSettingsFile(display, reporterrors)
REGISTER struct Display *display;
BOOL reporterrors;
/* The display argument can be NULL if there's no display for this fetch */
{
	REGIST	BOOL	retvalue;
	REGISTER SHORT i, i2;

	retvalue = ReadSettingsGrunt(display, reporterrors);
	if (NOT retvalue) goto DONE;

	DefaultColorTextDepth = Settings.DefaultColorTextDepth;

	/* Text One Plane: */
	for (i = 0; i < 2; i++)
		{
		TextOnePlaneColors[i] = Settings.TextOnePlaneColors[i];
		}
	/* Text Two Plane */
	for (i = 0; i < 4; i++)
		{
		TextTwoPlaneColors[i] = Settings.TextTwoPlaneColors[i];
		}
	/* Text Three Plane */
	for (i = 0; i < 8; i++)
		{
		TextThreePlaneColors[i] = Settings.TextThreePlaneColors[i];
		}
	/* Text Four Plane */
	for (i = 0; i < 16; i++)
		{
		TextFourPlaneColors[i] = Settings.TextFourPlaneColors[i];
		}
	/* High Graphics */
	for (i = 0; i < 2; i++)
		{
		HighGraphicsColors[i] = Settings.HighGraphicsColors[i];
		}
	/* Low Graphics Normal */
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			LowGraphicsColors[0][i][i2] 
				= Settings.LowGraphicsColors[0][i][i2];
			}
	/* Low Graphics Intensified */
	for (i = 0; i < 2; i++)
		for (i2 = 0; i2 < 4; i2++)
			{
			LowGraphicsColors[1][i][i2] 
				= Settings.LowGraphicsColors[1][i][i2];
			}

	display->PresetMonoWidth = Settings.PresetMonoWidth;
	display->PresetMonoHeight = Settings.PresetMonoHeight;
	display->PresetMonoTopEdge = Settings.PresetMonoTopEdge;
	display->PresetMonoLeftEdge = Settings.PresetMonoLeftEdge;
	display->PresetMonoDepth = Settings.PresetMonoDepth;
	display->PresetColorWidth = Settings.PresetColorWidth;
	display->PresetColorHeight = Settings.PresetColorHeight;
	display->PresetColorTopEdge = Settings.PresetColorTopEdge;
	display->PresetColorLeftEdge = Settings.PresetColorLeftEdge;
	display->PresetMonoFlags = Settings.PresetMonoFlags;
	display->PresetColorDepth = DefaultColorTextDepth;
	display->PresetColorFlags = Settings.PresetColorFlags;
	SetFlag(display->Modes2, WINDOW_PRESETS);

DONE:
	return(retvalue);
}



