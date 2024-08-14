
/* ***************************************************************************
 * 
 * This file contains the "retired" Zaphod Project routines 
 * (retired because they were downcoded)    
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * **************************************************************************/

#include "zaphod.h"


OldDrawColorText()
{   
    SHORT i, row, col, startcol; 
    USHORT CurrentPens;
    USHORT *rowptr, *textptr;

#ifdef DIAG
kprintf("In DrawColorText\n");DumpColorVars();
#endif

    rowptr = (USHORT *)ColorBuffer;	 

    CurrentPens = (*rowptr) & 0x7F; 
    SetAPen(ColorRPort, CurrentPens & 0xF);
    SetBPen(ColorRPort, (CurrentPens >> 4) & 0x7);

    for (row = 0; row < CT_Height; row++)
	{
	col = startcol = 0;
	textptr = rowptr;
	while (col < CT_NewLength[row]) 
	    {
	    if (CurrentPens != ((*(rowptr + col)) & 0x7F))
		{
		if (i = col - startcol)
		    {
		    Move(ColorRPort,
			    CT_BaseCol + startcol * COLOR_CHAR_WIDTH,
			    CT_BaseRow + row * COLOR_CHAR_HEIGHT);
		    ZaphodText(ColorRPort, textptr, i);
		    textptr += i;
		    startcol = col;
		    }
		CurrentPens = (*textptr) & 0x7F;
		SetAPen(ColorRPort, CurrentPens & 0xF);
		SetBPen(ColorRPort, (CurrentPens >> 4) & 0x7);
		}
	    col++;
	    }

	if (startcol != col)
	    {
	    Move(ColorRPort,
		    CT_BaseCol + startcol * COLOR_CHAR_WIDTH,
		    CT_BaseRow + row * COLOR_CHAR_HEIGHT);
	    ZaphodText(ColorRPort, textptr, col - startcol);
	    }		 

	rowptr += CT_Width;
	}
}


DrawMonoText()
{
}

 

OldZaphodText(rport, textptr, length)
struct RastPort *rport;
USHORT *textptr;
SHORT length;
{
    UBYTE buffer[80];
    SHORT i;
    USHORT onechar;

#ifdef DIAG
kprintf("FgPen=%ld BgPen=%ld\n",ColorRPort->FgPen, ColorRPort->BgPen);
#endif
  
*/
	     
    if (length)
	{
	for (i = 0; i < length; i++)
	    {
	    onechar = *textptr++;
	    buffer[i] = (UBYTE)(onechar >> 8); 
	    }
	Text(rport, &buffer[0], length);
	}
}



Curse(display, x, y)
struct Display *display;
SHORT x, y;
{
    SHORT startx, starty;

    startx = (x - display->TextStartCol) * CHAR_WIDTH;
    startx += display->TextBaseCol;
    starty = (y - display->TextStartRow) * CHAR_HEIGHT;
    starty += display->TextBaseRow;
    RectFill(display->RPort, startx, starty + display->CursorTop,
	    startx + CHAR_WIDTH - 1, starty + display->CursorBottom);
    if (display->CursorAltTop != -1)
	RectFill(display->RPort, startx, starty + display->CursorAltTop,
		startx + CHAR_WIDTH - 1, starty + CHAR_HEIGHT - 1);
}




/* === OTHER CRT REGISTERS (in case I ever want them back) ============ */

/* === CRT_R1 is "Horizontal Display Register" ======================== */ 

#ifdef DIAG
    reg = (*(display->PCCRTRegisterBase + CRT_R1)) & CRT_R1MASK;
    if (reg != (display->ZaphodCRT[CRT_R1] & CRT_R1MASK))
	{
if (display->Modes & VERBOSE) kprintf("<CD_R1:%lx>",reg);
	display->ZaphodCRT[CRT_R1] = reg;
	revampWindow = TRUE;
	NewColorWindow.MaxWidth = ((reg + 1) & -2) * COLOR_CHAR_WIDTH;
	}
#endif
     


/* === CRT_R6 is "Vertical Display Register" ========================= */ 

#ifdef DIAG
    reg = (*(display->PCCRTRegisterBase + CRT_R6)) & CRT_R6MASK;
    if (reg != (display->ZaphodCRT[CRT_R6] & CRT_R6MASK))
	{
if (display->Modes & VERBOSE) kprintf("<CD_R6:%lx>",reg);  
	display->ZaphodCRT[CRT_R6] = reg;
	revampWindow = TRUE;
	NewColorWindow.MaxHeight = reg * COLOR_CHAR_HEIGHT;
	}
#endif


/* === CRT_R8 is "Interlace Mode and Skew Register" ================= */

#ifdef DIAG
    reg = *(display->PCCRTRegisterBase + CRT_R8) & CRT_R8MASK;
    if (reg != (display->ZaphodCRT[CRT_R8] & CRT_R8MASK))
	{
if (display->Modes & VERBOSE) kprintf("<CD_R8:%lx>",reg);  
	display->ZaphodCRT[CRT_R8] = reg;
	switch (reg & VIDEO_MASK)
	    {
	    case VIDEO_INTERLACE:
		SetInterlace(display);
		break;
	    case VIDEO_COMPRESS:
		break;
	    default:
		/* The other two patterns must mean no interlace */
		ClearInterlace(display);
		break;
	    }	       
	}
#endif

								    

/* === CRT_R9 is "Maximum Scan Line Address Register" ============== */ 

#ifdef DIAG
    reg = *(display->PCCRTRegisterBase + CRT_R9) & CRT_R9MASK;
    if (reg != (display->ZaphodCRT[CRT_R9] & CRT_R9MASK))
	{
if (display->Modes & VERBOSE) kprintf("<CD_R9:%lx>",reg);  
	display->ZaphodCRT[CRT_R9] = reg;

	/* This has the character height - 1.  This will be
	 * useful someday when, for instance, the interlace font
	 * is true compressed interlace 14 lines tall.	But for
	 * now, ignore this.
	 */

	}
#endif




