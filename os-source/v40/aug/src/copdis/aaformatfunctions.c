/*****************************************************************************
*
*	$Id: aaformatfunctions.c,v 39.1 92/11/18 10:39:16 Jim2 Exp Locker: Jim2 $
*
******************************************************************************
*
*	$Log:	aaformatfunctions.c,v $
 * Revision 39.1  92/11/18  10:39:16  Jim2
 * First Release - works with remote wack.
 * 
*
******************************************************************************/

#ifndef VERSION
	#include "copdis_rev.h"
#endif

#include "CopDis.h"
#include "AAFormatFunctions.h"


STATIC char
    * BPLCON0 []=
    {
	"ECSENA",
	"ERSY",
	"LACE",
	"LPEN",
	"",
	"BYPASS",
	"SHRES",
	"UHRES",
	"GAUD",
	"COLOR",
	"DPF"
    },
    * BPLCON2 [] =
    {
	"PF2PRI",
	"SOGEN",
	"RDRAM",
	"KILLEHB",
	"ZDCTEN",
	"ZDBPEN"
    },
    * BPLCON3 [] =
    {
	"EXTBLKEN",
	"BRDSPRT",
	"ZDCLKEN",
	"",
	"BRDNTRAN",
	"BRDRBLNK"
    },
    * FMODE [] =
    {
	"BPL32",
	"BPAGEM",
	"SPR32",
	"SPAGEM",
	"","","","","","","","","",""
	"BSCAN2"
	"SSCAN2"
    };

STATIC struct AAWindow
{
    ULONG HStart, VStart, HStop, VStop;
    BOOL StartDef, StopDef, LocState;
} Window [2];

STATIC CurrentField;

/****** AADisplayWindow ******************************************************
*
*   NAME
*	AADisplayWindow - Extracts window vertical and horizontal positions.
*
*   SYNOPSIS
*	AADisplayWindow (This, HSave, VSave)
*
*	VOID AADisplayWindow (UWORD, ULONG *, ULONG *);
*
*   FUNCTION
*	Extracts window positions and writes them.  Also returns the positions
*	for possible extention register operations.
*
*   INPUTS
*	This  - Register value to be parsed.
*	HSave - Pointer to location used for saving the horizontal position.
*	VSave - Pointer to location used for saving the vertical position.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
static VOID AADisplayWindow (UWORD This,ULONG * HSave, ULONG *VSave)
{
    CopDisVPrintf (" Vert=%02lx, Horz=%02lx", *VSave = (This>>8), *HSave = (This & 0x00FF));
} /* AADisplayWindow */


/****** DisplayStart *********************************************************
*
*   NAME
*	DisplayStart - Defines the window start locations for a specific field.
*
*   SYNOPSIS
*	DisplayStart (This, Field)
*
*	VOID DisplayStart (UWORD, UWORD);
*
*   FUNCTION
*	When disassembling a write to the window start register, writes the
*	starting position.  Also stores the position specifically for this
*	field so that a write to the extention register for this field will
*	report the correct value.
*
*   INPUTS
*	This  - Register value to be parsed.
*	Field - Identifies the field for which this value applies.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
STATIC VOID DisplayStart (UWORD This, UWORD Field)
{
    Window[Field]. StartDef = TRUE;
    AADisplayWindow (This, &(Window[Field]. HStart), &(Window[Field]. VStart));
} /* DisplayStart */


/****** DisplayStop **********************************************************
*
*   NAME
*	DisplayStop - Defines the window stop location for a specific field.
*
*   SYNOPSIS
*	DisplayStop (This, Field)
*
*	VOID DisplayStop (UWORD, UWORD);
*
*   FUNCTION
*	When disassembing a write to the window stop register, writes the
*	stopping position.  Also stores the position specifically for this
*	field so that a write to the extention register for this field will
*	report the correct value.
*
*   INPUTS
*	This  - Register value to be parsed.
*	Field - Identifies the field for which this value applies.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
static VOID DisplayStop (UWORD This, UWORD Field)
{
    Window[Field]. StopDef = TRUE;
    AADisplayWindow (This, &(Window[Field]. HStop), &(Window[Field]. VStop));
} /* DisplayStop */

/****** WalkSingleBits *******************************************************
*
*   NAME
*	WalkSingleBits - Prints strings corresponding to the bit map.
*
*   SYNOPSIS
*	WalkSingleBits (This, Walking)
*
*	WalkSingleBits (UWORD, char ** Walking);
*
*   FUNCTION
*	The mapping function from the bit map to the array is the least
*	significant bit is the first string in the array.
*
*	Bits that aren't to be translated are zeroed.
*
*	While the bit map is non-zero the least significant bit is checked.
*	If the bit is one the current head of the string array is printed.
*	Then the bit map is shifted right and the string array pointer is
*	advanced to the next string.
*
*   INPUTS
*	This    - Register value to be parsed.
*	Walking - Pointer to an array of strings.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
STATIC VOID WalkSingleBits (UWORD This, char **Walking)
{
    while (This)
    {
	if (This & 0x0001)
	    CopDisVPrintf (" %s", *Walking);
	This /= 2;
	Walking++;
    }
} /* WalkSingleBits */

/****** XDisplay *************************************************************
*
*   NAME
*	XDisplay - Extend any previous start and stop positions for this field
*		and print the information.
*
*   SYNOPSIS
*	XDisplay (This, Field)
*
*	VOID XDisplay (UWORD, UWORD);
*
*   FUNCTION
*	Writing to this register adds three additional bits to the vertical
*	start and stop positions for the field and a single bit to horizontal
*	start and stop positions for the field.  If either the start or stop
*	register has been written for this field, the extended value will be
*	written.  If a register has not been written this lack will be noted
*	and only the additional bits will be written.
*
*	Unused bits in this register are checked to assure that they are zeroed.
*
*   INPUTS
*	This  - Register value to be parsed.
*	Field - Identifies which field is to be affected by this write.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID XDisplay (UWORD This, UWORD Field)
{
    if (Window[Field].StopDef)
	CopDisVPrintf (" VertStop=%03lx, HorzStop=%03lx", Window[Field]. VStop + (This & 0x0300),
			Window[Field]. HStop + ((This & 0x2000) >> 5));
    else
	CopDisVPrintf (" NoDIWSTOP-V[10:8]=%lx, H[8]=%lx", (This & 0x0300) >> 8, (This & 0x2000) > 13);
    if (Window[Field].StartDef)
	CopDisVPrintf ("  VertStart=%03lx, HorzStart=%03lx", Window[Field]. VStart + ((This & 0x0003) << 8),
			Window[Field]. HStart + ((This & 0x0020) << 3));
    else
	CopDisVPrintf ("  NoDIWSTRT-V[10:8]=%lx, H[8]=%lx", (This & 0x000300), (This & 0x0020) > 5);
    if (This & 0xD8D8)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* XDisplay */


/****** AABPEnhance **********************************************************
*
*   NAME
*	AABPEnhance - Translate the bit map for register BPLCON3.
*
*   SYNOPSIS
*	AABPEnhance (This)
*
*	VOID AABPEnhance (UWORD);
*
*   FUNCTION
*	BPLCON3 contains the enchanced features for bit plane control.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AABPEnhance (UWORD This)
{
    CopDisVPrintf (" ColorBank%lx ", (This & 0xE000) >> 13);
    if (This & 0x1C00)
	CopDisVPrintf ("BPColorOffset=%lx", 1 << ((This & 0x1C00) >> 10));
    else
	CopDisVPrintf ("NoBPColorOffset");
    if (CurrentField == 2)
    {
	if (Window[0]. LocState = Window[1]. LocState = (This & 0x0200))
	    CopDisVPrintf (" LOCT");
    }
    else
	if (Window[CurrentField]. LocState = (This & 0x0200))
	    CopDisVPrintf(" LOCT");
    CopDisVPrintf (" Sprites=");
    switch (This & 0x00C0)
    {
	case 0x0000 :
	    CopDisVPrintf("ECS");
	    break;
	case 0x0040 :
	    CopDisVPrintf("140ns");
	    break;
	case 0x0080 :
	    CopDisVPrintf("70ns");
	    break;
	case 0x00C0 :
	    CopDisVPrintf("35ns");
	    break;
    }
    WalkSingleBits (This & 0x0037, BPLCON3);
    if (This & 0x0088)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AABPEnhance */


/****** AABPMiscControl ******************************************************
*
*   NAME
*	AABPMiscControl - Translate the bit map of BPLCON0.
*
*   SYNOPSIS
*	AABPMiscControl (This)
*
*	VOID AABPMiscControl (UWORD);
*
*   FUNCTION
*	BPLCON0 contains the miscellaneous control bits for bit plane control.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AABPMiscControl (UWORD This)
{
    if (This &0x8000)
	CopDisVPrintf (" HIRES");
    if ((This & 0x7810) == 0x0810)
	CopDisVPrintf (" NewHAM");
    else
    {
	if (This &0x7010)
	    CopDisVPrintf (" BPUse=%lx", ((This & 0x0010) >> 1) + (This & 0x7000) >> 12);
	else
	    CopDisVPrintf (" NoBPUse");
	if (This &0x0800)
	    CopDisVPrintf (" HAM");
    }
    WalkSingleBits (This &= 0x07EF, BPLCON0);
} /* AABPMiscControl */


/****** AABPModulo ***********************************************************
*
*   NAME
*	AABPModulo - Report a bit plane modulo.
*
*   SYNOPSIS
*	AABPModulo (This)
*
*	VOID AABPModulo (UWORD);
*
*   FUNCTION
*	Translates the value as a bit plane modulo.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AABPModulo (UWORD This)
{
    CopDisVPrintf (" Modulo=%04lx", This);
} /* AABPModulo */

/****** AABPPriority *********************************************************
*
*   NAME
*	AABPPriority - Translate the bit map of BPLCON2.
*
*   SYNOPSIS
*	AABPPriority (This)
*
*	VOID AABPPrioiry (UWORD);
*
*   FUNCTION
*	BPLCON2 contains control bits as well as the playfield priority bits.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AABPPriority (UWORD This)
{
    CopDisVPrintf (" ZDBPSEL=%lx", (This & 0x7000) >> 12);
    WalkSingleBits ((This & 0x0FC0) >> 6, BPLCON2);
    CopDisVPrintf (" PF2P=%lx PF1P=%lx", (This & 0x0038) >> 3, This & 0x0007);
    if (This & 0x8000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AABPPriority */

/****** AADataFetch ******************************************************
*
*   NAME
*	AADataFetch - Report the settings of H[8:2] for display data fetch.
*
*   SYNOPSIS
*	AADataFetch (This)
*
*	VOID AADataFetch (UWORD);
*
*   FUNCTION
*	Report the values used for horizontal timing for display data fetch.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AADataFetch (UWORD This)
{
    CopDisVPrintf (" H[8:2]=%02lx", (This & 0x00FE) >> 1);
    if (This & 0xFF01)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AADataFetch */

/****** AADisplayStart *******************************************************
*
*   NAME
*	AADisplayStart - Translate DIWSTART.
*
*   SYNOPSIS
*	AADisplayStart (This)
*
*	VOID AADisplayStart (UWORD);
*
*   FUNCTION
*	Report the horizontal and vertical display window start positions.
*
*	Record these positions for the current frame so when DIWHIGH is
*	written the correct positions will be reported.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AADisplayStart (UWORD This)
{
    if (CurrentField == 2)
    {
	CopDisVPrintf (" LongF");
	DisplayStart (This, 0);
	CopDisVPrintf (" ShortF");
	DisplayStart (This, 1);
    }
    else
	DisplayStart (This, CurrentField);
} /* AADisplayStart */


/****** AADisplayStop ********************************************************
*
*   NAME
*	AADisplayStop - Translate DIWSTOP.
*
*   SYNOPSIS
*	AADisplayStop (This)
*
*	VOID AADisplayStop (UWORD);
*
*   FUNCTION
*	Report the horizontal and vertical window stop positions.
*
*	Record these positions for the current frame so when DIWHIGH is
*	written the correct positions will be reported.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AADisplayStop (UWORD This)
{
    if (CurrentField == 2)
    {
	CopDisVPrintf (" LongF");
	DisplayStop (This, 0);
	CopDisVPrintf (" ShortF");
	DisplayStop (This, 1);
    }
    else
	DisplayStop (This, CurrentField);
} /* AADisplayStop */

/****** AAFetch **************************************************************
*
*   NAME
*	AAFetch - Translate the bit map for FMODE.
*
*   SYNOPSIS
*	AAFetch (This)
*
*	VOID AAFetch (UWORD);
*
*   FUNCTION
*	FMODE contains information on the memory fetch mode.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AAFetch (UWORD This)
{
    WalkSingleBits (This & 0xC00F, FMODE);
    if (This & 0x3FF0)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAFetch */

/****** AAHScroll ************************************************************
*
*   NAME
*	AAHScroll - Translate BPLCON1.
*
*   SYNOPSIS
*	AAHScroll (This)
*
*	VOID AAHScroll (UWORD);
*
*   FUNCTION
*	BPLCON1 contains the horizontal scroll codes for playfield 1 and 2.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AAHScroll (UWORD This)
{
    CopDisVPrintf (" PF1H=%02lx PF2H=%02lx", ((This & 0x000F) << 2) + ((This & 0x0300) >> 8) + ((This & 0x0C00) >> 4),
			((This & 0x00F0) >> 2) + ((This & 0x3000) >> 12) + ((This & 0xC000) >> 8));
} /* AAHScroll */

/****** AASpriteStart ********************************************************
*
*   NAME
*	AASpriteStart - Reports the start position for a sprite.
*
*   SYNOPSIS
*	AASpriteStart (This)
*
*	VOID AASpriteStart (UWORD);
*
*   FUNCTION
*	Report the low order eight bits of vertical start position and the high
*	order eight bits of the vertical start position of a sprite.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	String containing interpretation of the register value.
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AASpriteStart (UWORD This)
{
    CopDisVPrintf (" Vert[7:0]=%02lx Horz[10:03]=%02lx", (This & 0xFF00) >> 8, This & 0x00FF);
} /* AASpriteStart */

/****** AAXDisplay ***********************************************************
*
*   NAME
*	AAXDisplay - Translate the bit map of DIWHIGH.
*
*   SYNOPSIS
*	AAXDisplay (This)
*
*	VOID AAXDisplay (UWORD);
*
*   FUNCTION
*	DIWHIGH contains high order bits for the window start and stop
*	positions.  If DIWSTART and/or DIWSTOP have been written these
*	bits will be added to the current start and stop values and the
*	complete start and stop values printed.  If the above registers
*	have not been written, only the additional bits are written.
*
*	As with DIWSTART and DIWSTOP distinct values may be set for each
*	field.
*
*   INPUTS
*	This - Register value to be parsed.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID AAXDisplay (UWORD This)
{
    if (CurrentField == 2)
    {
	CopDisVPrintf (" LongF");
	XDisplay (This, 0);
	CopDisVPrintf (" ShortF");
	XDisplay (This, 1);
    }
    else
	XDisplay (This, CurrentField);
} /* AAXDisplay */

/****** LocState *************************************************************
*
*   NAME
*	LocState - Report the last known state of the LOC bit.
*
*   SYNOPSIS
*	Set = LocState (Field)
*
*	BOOL LocState (UWORD);
*
*   FUNCTION
*	Returns the value stoed in the LocState field of the Window record
*	for the corresponding field.
*
*   INPUTS
*	Field - 0 for long field, 1 for short field.  There is no error checking
*		on this parameter.
*
*   RESULT
*	Last state of LOC bit, or FALSE if not yet set.
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
BOOL LocState (UWORD Field)
{
    return (Window[Field].LocState);
} /* LocState */

/****** ResetAAWindow ********************************************************
*
*   NAME
*	ResetAAWindow - Reset any stored information about the current window.
*
*   SYNOPSIS
*	ResetAAWindow ()
*
*	VOID ResetAAWindow (VOID);
*
*   FUNCTION
*	This function is called whenever a new copper list is start to reset
*	any recorded information about a window.
*
*	Clear any current defintion of the window start and stop position
*	definition and the state of the LOC bit.
*
*   INPUTS
*	NONE
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
VOID ResetAADisplay (VOID)
{
    Window[0].StopDef = Window[0].StartDef = Window[0].LocState = Window[1].StopDef = Window[1].StartDef = Window[1].LocState = FALSE;
} /* ResetAADisplay */

/****** SetField *************************************************************
*
*   NAME
*	SetField - Set the current field value.
*
*   SYNOPSIS
*	SetField (Field)
*
*	VOID SetField (UWORD);
*
*   FUNCTION
*	Set the current field to one of three possible values:  0 for long
*	field; 1 for short field; or 2 for both fields.
*
*   INPUTS
*	Field - Field values as defined in Copper.h.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
UWORD SetField (UWORD Field)
{
    if (Field)
	CurrentField = ((Field >> 13) - 1) & 0x003;
    else
	CurrentField = 2;
    return (Field);
}/* SetField */
