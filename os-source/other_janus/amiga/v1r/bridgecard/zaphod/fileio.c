
/* ***************************************************************************
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
								

#define FILE_BUFFER_SIZE     82

BYTE FileBuffer[FILE_BUFFER_SIZE];
SHORT FileIndex;
LONG FileNumber;

    
/* The numeric input file format rules:
 *  - Anywhere that a ';' character appears, the rest of the line is 
 *    regarded as a comment and ignored
 *  - Numbers are separated by any non-numeric character, including
 *    blanks, spaces, end-of-line characters, the letter z, and any
 *    other delimiter you care to use.	Any word that starts with a
 *    '$' is treated as a hexadecimal number.
 */


SHORT GetNextLine(filenumber, bufptr)
LONG filenumber;
BYTE *bufptr;
{
    SHORT i;
    BYTE inkey;
    BOOL reality;

    i = 0;
    reality = TRUE;

    while (reality)
	{
	if (Read(filenumber, &inkey, 1))
	    {
	    /* OK, we managed to get another character.
	     * Now stuff it in the buffer, and if it's one of the newline
	     * characters then return.
	     */
	    *(bufptr + i) = inkey;
	    i++;
	    if (i >= FILE_BUFFER_SIZE - 1) reality = FALSE;
	    if ((inkey == '\n') || (inkey == '\j')) reality = FALSE;
	    }
	else reality = FALSE;
	}

    *(bufptr + i) = 0;
    return (i);
}


BOOL GetNextNumber(numaddress, bufptr, indexaddress, filenumber)
SHORT *numaddress, *indexaddress;
BYTE *bufptr;
LONG filenumber;
/*
 * When a number is found, it is saved in the memory location
 * pointed to by numaddress, bufptrptr is advanced to point to the first
 * non-numeric character, and this routine returns TRUE.  If no number
 * is found, numaddress and bufptrptr are unchanged and this routine 
 * returns FALSE.
 */
{
    BYTE thischar;
    SHORT index, result, base;

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
		if (NOT GetNextLine(filenumber, bufptr)) return (FALSE);
		index = 0;
		}
	    }
	if (NOT GetNextLine(filenumber, bufptr)) return (FALSE);
	index = 0;
	}
}





ShortGetNextNumber(num)
SHORT *num;
{
    GetNextNumber(num, &FileBuffer[0], &FileIndex, FileNumber);
}



VOID ReadKeyTable()
{

#ifdef JANUS
    SHORT num, i;

    FileIndex = 0;
    FileBuffer[0] = 0;
    if ( (FileNumber = Open(&SidecarKeysFile[0], MODE_OLDFILE)) == 0)
	{
	Alert(ALERT_BAD_KEYS_FILE, NULL);
	return;
	}

    ShortGetNextNumber(&num);	 PCAltCode = num;
    ShortGetNextNumber(&num);	 PCCapsLockCode = num;
    ShortGetNextNumber(&num);	 PCCtrlCode = num;
    ShortGetNextNumber(&num);	 PCLeftShiftCode = num;
    ShortGetNextNumber(&num);	 PCNumLockCode = num;
    ShortGetNextNumber(&num);	 PCPlusCode = num;
    ShortGetNextNumber(&num);	 PCPtrScrCode = num;
    ShortGetNextNumber(&num);	 PCRightShiftCode = num;
    ShortGetNextNumber(&num);	 PCScrollLockCode = num;

    ShortGetNextNumber(&num);	 AmigaCapsLockCode = num;
    ShortGetNextNumber(&num);	 AmigaLeftAltCode = num;
    ShortGetNextNumber(&num);	 AmigaNCode = num;
    ShortGetNextNumber(&num);	 AmigaPCode = num;
    ShortGetNextNumber(&num);	 AmigaPlusCode = num;
    ShortGetNextNumber(&num);	 AmigaRightAltCode = num;
    ShortGetNextNumber(&num);	 AmigaSCode = num;

    for (i = 0; i < 128; i++)
	{
	if ( ShortGetNextNumber(&num) == FALSE )
	    {
	    Alert(ALERT_BAD_KEYS_FILE, NULL);
	    goto CLOSE_RETURN;
	    }
	PCRawKeyTable[i] = num;
	}
	     
CLOSE_RETURN:
    Close(FileNumber);
#endif
}
					

ShortWrite(text, display)
UBYTE *text;
{
    if (Write(FileNumber, text, StringLength(text)) == -1)
	{
	Alert(ALERT_BAD_SETTINGS_FILE, display);
	}
}



VOID WriteSettingsFile(display)
struct Display *display;
{
    SHORT num, i, i2;
    LONG seconds, micros;
    struct Process *process;
    struct Window *procwindow, *w;

    if (NOT Lock(&display->DisplayLock)) return;

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

    /* If there's already a settings file out there, zap it! */
    DeleteFile(&SidecarSettingsFile[0]);

    if ( (FileNumber = Open(&SidecarSettingsFile[0], MODE_NEWFILE)) == 0)
	{
	Alert(ALERT_BAD_SETTINGS_FILE, display);
	goto UNLOCK_RETURN;
	}

    ShortWrite("; Sidecar Settings File\n;\n; So, how's it going, eh?\n;\n\n", 
	    display);

    num = 0;
    sprintf(&FileBuffer[0], "; Flag Word:\n    $%04lx\n\n", num);
    ShortWrite(&FileBuffer[0], display);
    
    if (display->FirstWindow)
	{
	seconds = display->FirstWindow->CursorSeconds;
	micros = display->FirstWindow->CursorMicros;
	}
    else
	{
	seconds = DEFAULT_CURSOR_SECONDS;
	micros = DEFAULT_CURSOR_MICROS;
	}
    sprintf(&FileBuffer[0], "; Default Cursor Rate:\n    %02ld %06ld\n\n",
	    seconds, micros);

    ShortWrite(&FileBuffer[0], display);
		     
    sprintf(&FileBuffer[0], "; Default Color Text Depth:\n    %1ld\n\n", 
	    DefaultColorTextDepth);
    ShortWrite(&FileBuffer[0], display);

    sprintf(&FileBuffer[0], "; Display Task Priority:\n    %02ld\n\n", 
	    DisplayPriority);
    ShortWrite(&FileBuffer[0], display);

    ShortWrite("; Default Palettes:\n", display);
    ShortWrite("; Text One Plane:\n    ", display);
    for (i = 0; i < 2; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", TextOnePlaneColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n; Text Two Plane:\n    ", display);
    for (i = 0; i < 4; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", TextTwoPlaneColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n; Text Three Plane:\n    ", display);
    for (i = 0; i < 8; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", TextThreePlaneColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n; Text Four Plane:\n    ", display);
    for (i = 0; i < 8; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", TextFourPlaneColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n    ");
    for (i = 8; i < 16; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", TextFourPlaneColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n; High Graphics:\n    ", display);
    for (i = 0; i < 2; i++)
	{
	sprintf(&FileBuffer[0], "$%04lx ", HighGraphicsColors[i]);
	ShortWrite(&FileBuffer[0], display);
	}
    ShortWrite("\n; Low Graphics Normal:\n    ", display);
    for (i = 0; i < 2; i++)
	for (i2 = 0; i2 < 4; i2++)
	    {
	    sprintf(&FileBuffer[0], "$%04lx ", LowGraphicsColors[0][i][i2]);
	    ShortWrite(&FileBuffer[0], display);
	    }
    ShortWrite("\n; Low Graphics Intensified:\n    ", display);
    for (i = 0; i < 2; i++)
	for (i2 = 0; i2 < 4; i2++)
	    {
	    sprintf(&FileBuffer[0], "$%04lx ", LowGraphicsColors[1][i][i2]);
	    ShortWrite(&FileBuffer[0], display);
	    }

    ShortWrite("\n\n; Is reality truly existential?  What do you think:");
    ShortWrite("\n    $FFFF\n\n");
       
    Close(FileNumber);
				  
UNLOCK_RETURN:
    process->pr_WindowPtr = (APTR)procwindow;

    Unlock(&display->DisplayLock);
}

 
BOOL ReadSettingsFile(display, reporterrors)
struct Display *display;
BOOL reporterrors;
/* The display argument can be NULL if there's no display for this fetch */
{
    SHORT num, i, i2;
    BOOL returnval;
    struct Process *process;
    struct Window *procwindow, *w;

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

    /* Get the Flag word */
    ShortGetNextNumber(&num);

    /* Get the default cursor times */
    /* The routine has to be changed to work with longs */
    ShortGetNextNumber(&num);
    ShortGetNextNumber(&num);

    /* Get the default text depth */
    ShortGetNextNumber(&num);
    DefaultColorTextDepth = num;

    /* The default display priority */
    ShortGetNextNumber(&num);

    /* And restore the palettes */
    /* Text One Plane: */
    for (i = 0; i < 2; i++)
	{
	ShortGetNextNumber(&num);
	TextOnePlaneColors[i] = num;
	}
    /* Text Two Plane */
    for (i = 0; i < 4; i++)
	{
	ShortGetNextNumber(&num);
	TextTwoPlaneColors[i] = num;
	}
    /* Text Three Plane */
    for (i = 0; i < 8; i++)
	{
	ShortGetNextNumber(&num);
	TextThreePlaneColors[i] = num;
	}
    /* Text Four Plane */
    for (i = 0; i < 16; i++)
	{
	ShortGetNextNumber(&num);
	TextFourPlaneColors[i] = num;
	}
    /* High Graphics */
    for (i = 0; i < 2; i++)
	{
	ShortGetNextNumber(&num);
	HighGraphicsColors[i] = num;
	}
    /* Low Graphics Normal */
    for (i = 0; i < 2; i++)
	for (i2 = 0; i2 < 4; i2++)
	    {
	    ShortGetNextNumber(&num);
	    LowGraphicsColors[0][i][i2] = num;
	    }
    /* Low Graphics Intensified */
    for (i = 0; i < 2; i++)
	for (i2 = 0; i2 < 4; i2++)
	    {
	    ShortGetNextNumber(&num);
	    LowGraphicsColors[1][i][i2] = num;
	    }

    if (NOT ShortGetNextNumber(&num))
	if (reporterrors) Alert(ALERT_BAD_SETTINGS_FILE, display);

    returnval = TRUE;

    Close(FileNumber);

UNLOCK_RETURN:
    if (display)
	Unlock(&display->DisplayLock);
		      
    process->pr_WindowPtr = (APTR)procwindow;

RETURN:
    return(returnval);
}

 
