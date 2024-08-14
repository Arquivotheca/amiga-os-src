
/* ***************************************************************************
 * 
 * Stuff the Text Buffer Routines for Zaphod (Amiga-Blue Project)
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 27 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"


StuffTextBuffer(display, startRow, endRow)
struct Display *display;
SHORT startRow, endRow;
/* This routine stuffs the text buffer with data from the current text page
 * of the color graphics card according to the current text buffer parameters
 */
{
    SHORT i, i2, rowwidth, longstartcol, longtextwidth;
    LONG *pagerow, *pagepos, *bufferpos;
					  
    if (display->Modes & MEDIUM_RES)
	rowwidth = 20;	    /* 20 longs per row */
    else 
	rowwidth = 40;	    /* 40 longs per row */   

    longstartcol = display->TextStartCol >> 1;
    longtextwidth = display->TextWidth >> 1;

    pagerow = (ULONG *)(display->PCDisplay);
    /* Stupid C compiler makes me do this assignment first, else the
     * smartly-named CMX33 variable is flagged as undefined.
     */
    i = display->TextStartRow + startRow;
    pagerow += i * rowwidth;
    bufferpos = (ULONG *)(display->Buffer);
    bufferpos += startRow * longtextwidth;

    for (i = startRow; i <= endRow; i++)
	{
	pagepos = pagerow + longstartcol;
	pagerow += rowwidth;
	for (i2 = 0; i2 < longtextwidth; i2++)
	    *bufferpos++ = *pagepos++;
	}
}


