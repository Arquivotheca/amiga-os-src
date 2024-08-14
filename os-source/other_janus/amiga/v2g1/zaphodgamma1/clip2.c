
/* *** clip2.c **************************************************************
 * 
 * Clipboard management routines 
 * See the document clip.doc
 *
 * Copyright (C) 1986, =Robert J. Mical=
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  --------------------------------------------
 * 1 Oct 86   =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"


extern	UBYTE	PCToAmigaTable[];



GetLowHi(display, lowx, hix, lowy, hiy)
struct Display *display;
SHORT *lowx, *hix, *lowy, *hiy;
{
	SHORT zerox, workcx, workcy;
	struct Window *window;

	workcx = ClipCurrentX;
	workcy = ClipCurrentY;

	if (ClipStartY < workcy)
		{
		*lowy = ClipStartY;
		*hiy = workcy;
		}
	else
		{
		*lowy = workcy;
		*hiy = ClipStartY;
		}
	*lowy = TextYPos(display, *lowy) - display->TextStartRow;
	*hiy = TextYPos(display, *hiy) - display->TextStartRow;

	window = display->FirstWindow->DisplayWindow;
	zerox = window->BorderLeft + 1;
	if ((*lowy < *hiy) && (ClipStartY < workcy))
		{
		if (workcx <= zerox)
			{
			*hiy = *hiy - 1;
			workcx = window->Width - 1;
			workcy = ClipStartY;
			}
		}

	if (*lowy == *hiy)
		{
		if (ClipStartX > workcx)
			{
			*lowx = workcx;
			*hix = ClipStartX;
			}
		else
			{
			*lowx = ClipStartX;
			*hix = workcx;
			}
		}
	else if (ClipStartY <= workcy)
		{
		*lowx = ClipStartX;
		*hix = workcx;
		}
	else
		{
		*lowx = workcx;
		*hix = ClipStartX;
		}
	*lowx = TextXPos(display, *lowx) - display->TextStartCol;
	*hix = TextXPos(display, *hix) - display->TextStartCol;

	if (*lowx < 0) *lowx = 0;
	if (*hix < 0) *hix = 0;
	if (*lowy < 0) *lowy = 0;
	if (*hiy < 0) *hiy = 0;

	if (*lowx >= display->TextWidth) *lowx = display->TextWidth - 1;
	if (*hix >= display->TextWidth) *hix = display->TextWidth - 1;
	if (*lowy >= display->TextHeight) *lowy = display->TextHeight - 1;
	if (*hiy >= display->TextHeight) *hiy = display->TextHeight - 1;
}



InvertClipArea(display)
struct Display *display;
{
	SHORT hix, hiy, lowx, lowy, startx, endx;

	GetLowHi(display, &lowx, &hix, &lowy, &hiy);

	/* Invert from lowy to hiy */
	startx = lowx;
	endx = display->TextWidth - 1;
	while (lowy <= hiy)
		{
		if (lowy == hiy) endx = hix;
		InvertRow(display, lowy, startx, endx);
		startx = 0;
		lowy++;
		}
}


InvertRow(display, row, startx, endx)
struct Display *display;
SHORT row, startx, endx;
{
	SHORT *bufferpos, i, nextpair;
	BYTE nextbyte, swapbyte;

	bufferpos = (SHORT *)(display->Buffer);
	bufferpos += ((row * display->TextWidth) + startx);

	for (i = startx; i <= endx; i++)
		{
		nextpair = *bufferpos;
		nextbyte = nextpair;
		swapbyte = (nextbyte >> 4) & 0x0F;
		nextbyte = (nextbyte << 4) & 0xF0;
		nextbyte = nextbyte | swapbyte;
		nextpair = (nextpair & 0xFF00) | nextbyte;
		*bufferpos = nextpair;
		bufferpos++;
		}
}



SHORT RowSize(shortptr, startx, endx, width)
SHORT *shortptr;
SHORT startx, endx, width;
{
	SHORT size, i, tobyte;
	BYTE thischar;

	size = (endx - startx) + 1;
	for (i = endx; i >= startx; i--)
		{
		tobyte = *(shortptr + i);
		thischar = (tobyte >> 8) & 0x7F;
		if (thischar != ' ') goto FOUND_CHAR;
		size--;
		}

FOUND_CHAR:
	return(size);
}



VOID WriteSelectedArea(display)
struct Display *display;
/* Writes the selected area to the clipboard after making the
 * following translations:
 *   - The characters are translated through PCToAmigaTable[]
 *   - If the last byte of the line is to be written, a carriage return
 *	 is written too
 *   - Trailing spaces are stripped
 */
{
	REGIST	SHORT tobyte;
	SHORT hix, hiy, lowx, lowy, width, size, i, currenty;
	SHORT startx, endx;
	struct Remember *key;
	BYTE *byteptr, *byteptr2;
	SHORT *shortptr;

	GetLowHi(display, &lowx, &hix, &lowy, &hiy);
	width = display->TextWidth;

	startx = lowx;
	endx = width - 1;
	currenty = lowy;
	size = 1; /* one for the trailing NULL */
	while (currenty <= hiy)
		{
		if (currenty == hiy) endx = hix;
		shortptr = (SHORT *)(display->Buffer);
		shortptr += (currenty * width);
		size += RowSize(shortptr, startx, endx, width);
		if (endx == (width - 1)) size++; /* extra for the carriage return */
		startx = 0;
		currenty++;
		}

	key = NULL;
	if ((byteptr = (BYTE *)AllocRemember(&key, size, 0)) == NULL) return;
	byteptr2 = byteptr;

	startx = lowx;
	endx = width - 1;
	currenty = lowy;
	while (currenty <= hiy)
		{
		if (currenty == hiy) endx = hix;
		shortptr = (SHORT *)(display->Buffer);
		shortptr += (currenty * width);
		size = RowSize(shortptr, startx, endx, width);
		shortptr += startx;
		for (i = 0; i < size; i++)
			{
			tobyte = *shortptr++;
			tobyte = (tobyte >> 8) & 0xFF;
			tobyte = PCToAmigaTable[tobyte];
			*byteptr2++ = tobyte;
			}
		if (endx == (width - 1)) *byteptr2++ = '\n';

		startx = 0;
		currenty++;
		}

	*byteptr2 = '\0';
	WriteTextClip(byteptr);
	FreeRemember(&key, TRUE);
}





