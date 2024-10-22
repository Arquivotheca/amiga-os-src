/*****************************************************************************
*
*	$Id: AAAFormatFunctions.c,v 41.0 92/11/23 11:01:08 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	AAAFormatFunctions.c,v $
 * Revision 41.0  92/11/23  11:01:08  Jim2
 * Initial release.
 * 
*
******************************************************************************/
#ifndef REVSION
    #include "copdisAAA_rev.h"
#endif

#include <exec/types.h>

#include "CopDis.h"
#include "AAAFormatFunctions.h"

STATIC struct AAAWindow
{
	ULONG HStart, VStart, HStop, VStop, VBeam;
	BOOL StartDef, StopDef, BeamDef, LocState;
} Window [2];

STATIC UWORD CurrentField;

STATIC char
    * ADKCON[] =
    {
	"USE0V1",
	"USE1V2",
	"USE2V3",
	"USE3V4",
	"USE0P1",
	"USE1P2",
	"USE2P3",
	"USE3P4",
	"FAST",
	"MBSYNC",
	"WORDSYNC",
	"UARTBRK",
	"MFMPREC",
	"PRECOMP0",
	"PRECOMP1"
    },
    * ADKCONX[] =
    {
	"CODE0",
	"CODE1",
	"MODE0",
	"MODE1",
	"INSYNC",
	"LSBFIRST",
	"BOTHEDGES",
	"FASTB",
	"FASTA",
	"SYNCEN0",
	"SYNCEN1",
	"MFMPREC",
	"PRECOMP0",
	"PRECOMP1",
	"RLLPREC",
	"","","","","","","","",
	"AUDSCALE0",
	"AUDSCALE1",
	"AUDMONO",
	"UARTBRK",
	"UART2BRK",
	"SHORTPULSE"
    },
    * AUDCTL[] =
    {
	"RIGHT",
	"LEFT",
	"AUDAV",
	"AUDAP",
	"AUD16",
	"AUDPERF",
	"AUDSTOP",
	"AUDTURBO"
    },
    * BEAMCON0[] =
    {
	"HSYTRUE",
	"VSYTRUE",
	"CSYTRUE",
	"",
	"BEAMEN",
	"PAL",
	"NTSC",
	"", "", "",
	"BEANLCK",
	"", "",
	"LPENDIS",
	"",
	"COPRSEL"
    },
    * BLITEN[] =
    {
	"BLIT SLOT 0",
	"BLIT SLOT 1",
	"BLIT SLOT 2",
	"BLIT SLOT 3",
	"BLIT SLOT 4",
	"BLIT SLOT 5",
	"BLIT SLOT 6",
	"BLIT SLOT 7"
    },
    * BLTCON0[] =
    {
	"USED",
	"USEC",
	"USEB",
	"USEA",
	"","","","","",
	"SG"
    },
    * BLTCON1LINE[] =
    {
	"LINE",
	"SING",
	"AUL",
	"SUL",
	"SUD",
	"OVF",
	"SIGN",
	"DOFF"
    },
    * BLTCON1NL[] =
    {
	"",
	"DESC",
	"FCI",
	"IFE",
	"EFE",
	"", "",
	"DOFF"
    },
    * BLTFUNCX[] =
    {
	"CK",
	"FE",
	"IF",
	"EF"
    },
    * BPLCON0 []=
    {
	"ENBPLCN3",
	"ERSY",
	"LACE",
	"LPEN",
	"", "", "", "",
	"GAUD",
	"COLOR",
	"DPF",
	"", "", "", "",
	"HIRES"
    },
    * BPLCON2 [] =
    {
	"PF2PRI",
	"SOGEN",
	"",
	"KILLEHB",
	"ZDCTEN",
	"ZDBPEN"
    },
    * BPLCON3 [] =
    {
	"LACEDIS",
	"ZDCLKEN",
	"DIR",
	"BRDNTRAN",
	"BRDRBLNK",
	"BASEN"
    },
    * CLXCON [] =
    {
	"MVBP1",
	"MVBP2",
	"MVBP3",
	"MVBP4",
	"MVBP5",
	"MVBP6",
	"ENBP1",
	"ENBP2",
	"ENBP3",
	"ENBP4",
	"ENBP5",
	"ENBP6",
	"ENSP1",
	"ENSP3",
	"ENSP5",
	"ENSP7"
    },
    * CLXCONX [] =
    {
	"MVBP1",
	"MVBP2",
	"MVBP3",
	"MVBP4",
	"MVBP5",
	"MVBP6",
	"MVBP7",
	"MVBP8",
	"MVBP9",
	"MVBP10",
	"MVBP11",
	"MVBP12",
	"MVBP13",
	"MVBP14",
	"MVBP15",
	"MVBP16",
	"ENBP1",
	"ENBP2",
	"ENBP3",
	"ENBP4",
	"ENBP5",
	"ENBP6",
	"ENBP7",
	"ENBP8",
	"ENBP9",
	"ENBP10",
	"ENBP11",
	"ENBP12",
	"ENBP13",
	"ENBP14",
	"ENBP15",
	"ENBP16"
    },
    * CLXCONOEN [] =
    {
	"ENBP1",
	"ENBP2",
	"ENBP3",
	"ENBP4",
	"ENBP5",
	"ENBP6",
	"ENBP7",
	"ENBP8",
	"ENBP9",
	"ENBP10",
	"ENBP11",
	"ENBP12",
	"ENBP13",
	"ENBP14",
	"ENBP15",
	"ENBP16"
	"ENBP18",
	"ENBP19",
	"ENBP20",
	"ENBP21",
	"ENBP22",
	"ENBP23",
	"ENBP24"
    },
    * CLXCONODN [] =
    {
	"MVBP1",
	"MVBP2",
	"MVBP3",
	"MVBP4",
	"MVBP5",
	"MVBP6",
	"MVBP7",
	"MVBP8",
	"MVBP9",
	"MVBP10",
	"MVBP11",
	"MVBP12",
	"MVBP13",
	"MVBP14",
	"MVBP15",
	"MVBP16",
	"MVBP17",
	"MVBP18",
	"MVBP19",
	"MVBP20",
	"MVBP21",
	"MVBP22",
	"MVBP23",
	"MVBP24",
    },
    * DMACON [] =
    {
	"AUD0EN",
	"AUD1EN",
	"AUD2EN",
	"AUD3EN",
	"DSKEN",
	"SPREN",
	"BLTEN",
	"COPEN",
	"BPLEN",
	"DMAEN",
	"BLTPRI"
    },
    * DMACONX [] =
    {
	"AUD0ENX",
	"AUD1ENX",
	"AUD2ENX",
	"AUD3ENX",
	"DSKENX",
	"SPRENX",
	"BLTEN",
	"COPENX",
	"BPLEN",
	"DMAENX",
	"",
	"BBSRTENX",
	"",
	"BZERO",
	"BBUSY",
	"OVRLAYEN",
	"AUD4ENX",
	"AUD5ENX",
	"AUD6ENX",
	"AUD7ENX"
	"AUD4EN",
	"AUD5EN",
	"AUD6EN",
	"AUD7EN",
	"COPPRI"
    },
    * INTE [] =
    {
	"TBE",
	"DSKBLK",
	"SOFT",
	"PORTS",
	"COPER",
	"VERTB",
	"BLIT",
	"AUD0",
	"AUD1",
	"AUD2",
	"AUD3",
	"RBF",
	"DSKSYN",
	"EXTER",
	"INTEN",
	"AUD0OVR",
	"AUD1OVR",
	"AUD2OVR",
	"AUD3OVR",
	"RBF2",
	"DSKOVR",
	"VIDOVR",
	"AUD4",
	"AUD5",
	"AUD6",
	"AUD7",
	"AUD4OVR",
	"AUD5OVR",
	"AUD6OVR",
	"AUD7OVR",
	"TBE2"
    },
    * SPRXCTL [] =
    {
	"SH0",
	"EV8",
	"SV8",
	"FAST",
	"SHSH",
	"EV9",
	"SV9",
	"ATT"
    },
    * SPRXCTLX [] =
    {
	"SRES",
	"","","","","","",
	"ATT",
	"SMIRROR",
	"CONSPRT"
    };


/****** WalkSingleBits *******************************************************
*
*   NAME
*	WalkSingleBits - Prints strings corresponding to the bit map.
*
*   SYNOPSIS
*	WalkSingleBits (This, Walking)
*
*	WalkSingleBits (ULONG, char **);
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
STATIC VOID WalkSingleBits (ULONG This, char **Walking)
{
    while (This)
    {
	if (This & 0x00000001L)
	    CopDisVPrintf (" %s", *Walking);
	This /= 2;
	Walking++;
    }
} /* WalkSingleBits */



/****** BeamHigh *************************************************************
*
*   NAME
*	BeamHigh - Add any low order beam information and print the vertical position.
*
*   SYNOPSIS
*	BeamHigh (This, Field)
*
*	VOID BeamHigh (ULONG, UWORD);
*
*   FUNCTION
*	Writing to this register adds three additional bits to the vertical
*	beam position
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
VOID BeamHigh (ULONG This, UWORD Field)
{
    CopDisVPrintf (" Vert=%03lx", (Window[Field]. BeamDef ? Window[Field].VBeam : 0) + ((This & 0x0003) << 8));
} /* BeamHigh */

/****** BeamLow **************************************************************
*
*   NAME
*	BeamLow - print the low order beam position and save vertical info.
*
*   SYNOPSIS
*	BeamLow (This, Field)
*
*	VOID BeamLow (ULONG, UWORD);
*
*   FUNCTION
*	This register contains eight bits for each of the horizontal and vertical
*	beam position.
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
VOID BeamLow (ULONG This, UWORD Field)
{
    Window[Field].BeamDef = TRUE;
    CopDisVPrintf (" Horz=%03lx, Vert=%03lx", (Field & 0x00FF) << 2, Window[Field].VBeam = ((This & 0xFF00) >> 8));
} /* BeamLow */

/****** BlitterExtended ******************************************************
*
*   NAME
*	BlitterExtended - Prints the common features of the new blitter function
*		registers.
*
*   SYNOPSIS
*	BlitterExtended (This)
*
*	VOID BlitterExtended (ULONG);
*
*   FUNCTION
*	The two new thrity-two bit blitter function registers share a common
*	definition for twenty-eight of the bits.  This routine decodes those
*	common bits.
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
STATIC VOID BlitterExtended (ULONG This)
{
    WalkSingleBits ((This & 0x00020F00) >> 8, BLTCON0);
    CopDisVPrintf (" Bits/pixel=");
    switch (This & 0xFF000000)
    {
	case 0x08000000 :
	    CopDisVPrintf ("1");
	    break;
	case 0x11000000 :
	    CopDisVPrintf ("2");
	    break;
	case 0x22000000 :
	    CopDisVPrintf ("4");
	    break;
	case 0x43000000 :
	    CopDisVPrintf ("8");
	    break;
	case 0x84000000 :
	    CopDisVPrintf ("16");
	    break;
	case 0x05000000 :
	    CopDisVPrintf ("32");
	    break;
	default :
	    CopDisVPrintf ("INVALID");
	    break;
    }
    CopDisVPrintf (" Operation=");
    switch (This & 0x0000F000)
    {
	case 0x00002000 :
	    CopDisVPrintf ("New 1 Oper");
	    break;
	case 0x00003000 :
	    CopDisVPrintf ("New 2 Oper");
	    break;
	case 0x00004000 :
	    CopDisVPrintf ("New 3 Oper");
	    break;
	case 0x00005000 :
	    CopDisVPrintf ("New Reverse 1 Oper");
	    break;
	case 0x00006000 :
	    CopDisVPrintf ("New Reverse 2 Oper");
	    break;
	case 0x00007000 :
	    CopDisVPrintf ("New Reverse 3 Oper");
	    break;
	case 0x00008000 :
	    CopDisVPrintf ("New Line");
	    break;
	case 0x00009000 :
	    CopDisVPrintf ("Old mode UNUSED");
	    break;
	case 0x0000A000 :
	    CopDisVPrintf ("Old line UNUSED");
	    break;
	case 0x0000B000 :
	    CopDisVPrintf ("Tally");
	    break;
	case 0x0000C000 :
	    CopDisVPrintf ("Sort1");
	    break;
	case 0x0000D000 :
	    CopDisVPrintf ("Sort2");
	    break;
	default :
	    CopDisVPrintf ("INVALID");
	    break;
    }
    if ((This & 0x00010000) == 0)
	CopDisVPrintf (" M[7:0]=%lx", This & 0x000000FF);
    else
	switch (This & 0x000000FF)
	{
	    case 1 :
		CopDisVPrintf (" Add2");
		break;
	    case 2 :
		CopDisVPrintf (" Add3");
		break;
	    case 3 :
		CopDisVPrintf (" Sub2");
		break;
	    case 4 :
		CopDisVPrintf (" Add2 SAT");
		break;
	    case 5 :
		CopDisVPrintf (" Add3 SAT");
		break;
	    case 6 :
		CopDisVPrintf (" Sub2 SAT");
		break;
	    case 7 :
		CopDisVPrintf (" Add2 AVG");
		break;
	    default :
		CopDisVPrintf (" INVALID_OPERATION");
		break;
	}

    if (This & 0x000C00000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* BlitterExtended */

/****** AADisplayWindow ******************************************************
*
*   NAME
*	AADisplayWindow - Extracts window vertical and horizontal positions.
*
*   SYNOPSIS
*	AADisplayWindow (This, HSave, VSave)
*
*	VOID AADisplayWindow (ULONG, ULONG *, ULONG *);
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
STATIC VOID AAADisplayWindow (ULONG This,ULONG * HSave, ULONG *VSave)
{
    CopDisVPrintf (" Vert=%02lx, Horz=%03lx", *VSave = (This>>8), *HSave = ((This & 0x00FF) << 2));
} /* AAADisplayWindow */


/****** DisplayStart *********************************************************
*
*   NAME
*	DisplayStart - Defines the window start locations for a specific field.
*
*   SYNOPSIS
*	DisplayStart (This, Field)
*
*	VOID DisplayStart (ULONG, UWORD);
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
STATIC VOID DisplayStart (ULONG This, UWORD Field)
{
    Window[Field]. StartDef = TRUE;
    AAADisplayWindow (This, &(Window[Field]. HStart), &(Window[Field]. VStart));
} /* DisplayStart */


/****** DisplayStop **********************************************************
*
*   NAME
*	DisplayStop - Defines the window stop location for a specific field.
*
*   SYNOPSIS
*	DisplayStop (This, Field)
*
*	VOID DisplayStop (ULONG, UWORD);
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
static VOID DisplayStop (ULONG This, UWORD Field)
{
    Window[Field]. StopDef = TRUE;
    AAADisplayWindow (This, &(Window[Field]. HStop), &(Window[Field]. VStop));
} /* DisplayStop */

/****** XDisplay *************************************************************
*
*   NAME
*	XDisplay - Extend any previous start and stop positions for this field
*		and print the information.
*
*   SYNOPSIS
*	XDisplay (This, Field)
*
*	VOID XDisplay (ULONG, UWORD);
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
VOID XDisplay (ULONG This, UWORD Field)
{
    if (Window[Field].StopDef)
	CopDisVPrintf (" VertStop=%03lx, HorzStop=%03lx", Window[Field]. VStop + (This & 0x0300),
			Window[Field]. HStop + ((This & 0x2000) >> 3));
    else
	CopDisVPrintf (" NoDIWSTOP-V[10:8]=%lx, H[8]=%lx", (This & 0x0300) >> 8, (This & 0x2000) > 13);
    if (Window[Field].StartDef)
	CopDisVPrintf ("  VertStart=%03lx, HorzStart=%03lx", Window[Field]. VStart + ((This & 0x0003) << 8),
			Window[Field]. HStart + ((This & 0x0020) << 5));
    else
	CopDisVPrintf ("  NoDIWSTRT-V[10:8]=%lx, H[8]=%lx", (This & 0x000300), (This & 0x0020) > 5);
    if (This & 0xD8D8)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* XDisplay */



/****** AAAADUControl ********************************************************
*
*   NAME
*	AAAADUControl - Translate the bit map for register ADKCON.
*
*   SYNOPSIS
*	AAAADUControl (This)
*
*	VOID AAAADUControl (ULONG);
*
*   FUNCTION
*	ADKCON contains the ECS/AA compatable methods for controling audio,
*	disk and UART functions.
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
VOID AAAADUControl (ULONG This)
{
    if ((This & 0x8000) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits ((This & 0x7FFF), ADKCON);
} /* AAAADUControl */

/****** AAAAudioControl ******************************************************
*
*   NAME
*	AAAAudioControl - Report Audio control bits.
*
*   SYNOPSIS
*	AAAAudioControl (This)
*
*	VOID AAAAudioControl (ULONG);
*
*   FUNCTION
*	AUDxCTL contain the bits for control the audio channels in AAA mode.
*	This includes the bits from ADKCON that control audio.
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
VOID AAAAudioControl (ULONG This)
{
    WalkSingleBits (This & 0x000000FF, AUDCTL);
    if (This & 0xFFFFFF00)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAAudioControl */

/****** AAAAudioCoursePeriod *************************************************
*
*   NAME
*	AAAAudioCoursePeriod - Report the Audio Sampling period.
*
*   SYNOPSIS
*	AAAAudioCoursePeriod (This)
*
*	VOID AAAAudioCoursePeriod (ULONG);
*
*   FUNCTION
*	The audio period register contains the sampling frequency expressed in
*	279.3651ns - NTSC and 281.937ns - PAL.
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
VOID AAAAudioCoursePeriod (ULONG This)
{
    CopDisVPrintf (" %.1fKHz-NTSC %.1fKHZ-PAL", 3579.545/(This & 0xFFFF), 3546.895/(This & 0xFFFF));
    if ((This & 0xFFFF) < 70)
	CopDisVPrintf ("\n\t\t\t****INVALID period is to short\t");
} /* AAAAudioCoursePeriod */

/****** AAAAudioLength *******************************************************
*
*   NAME
*	AAAAudioLength - Translate the data for register Audio length.
*
*   SYNOPSIS
*	AAAAudioLength (This)
*
*	VOID AAAAudioLength (ULONG);
*
*   FUNCTION
*	AUDxLENX gives the number of words for audio channel DMA.
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
VOID AAAAudioLength (ULONG This)
{
    AAAAudioLength16 (This);
    if (This & 0xFFFF0000L)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAAudioLength */

/****** AAAAudioLength16 *****************************************************
*
*   NAME
*	AAAAudioLength - Report the size of the Audio DMA.
*
*   SYNOPSIS
*	AAAAudioLength (This)
*
*	VOID AAAAudioLength (ULONG);
*
*   FUNCTION
*	The audio length registers get the size of the audio DMA in words.
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
VOID AAAAudioLength16 (ULONG This)
{
    CopDisVPrintf (" %lx sixteen bit words", This & 0xFFFF);
} /* AAAAudioLength16 */

/****** AAAAudioPeriod *******************************************************
*
*   NAME
*	AAAAudioCoursePeriod - Report the Audio Sampling period.
*
*   SYNOPSIS
*	AAAAudioCoursePeriod (This)
*
*	VOID AAAAudioCoursePeriod (ULONG);
*
*   FUNCTION
*	The audio period register contains the sampling frequency in the upper
*	order word.  Unfortunately the translation of this value is dependent
*	on write to another register, so the actual frequency will not be reported.
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
VOID AAAAudioPeriod (ULONG This)
{
    CopDisVPrintf (" %lx Ticks", (This >> 16));
    if (This & 0xFFFF)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAAudioPeriod */

/****** AAAAudioVolumeSigned *************************************************
*
*   NAME
*	AAAAudioVolumeSigned - Report the Audio Volume.
*
*   SYNOPSIS
*	AAAAudioVolumeSigned (This)
*
*	VOID AAAAudioVolumeSigned (ULONG);
*
*   FUNCTION
*	The audio volume is a signed twelve bit number
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
VOID AAAAudioVolumeSigned (ULONG This)
{
    CopDisVPrintf (" %c%lx/7FF", (This & 0x8000000) ? '-' : '+', (This& 0x7FF00000) >> 20);
    if (This & 0xFFFFF)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAAudioVolumeSigned */

/****** AAAAudioVolumeUnsigned ***********************************************
*
*   NAME
*	AAAAudioVolumeUnsigned - Report the Audio Volume.
*
*   SYNOPSIS
*	AAAAudioVolumeUnsigned (This)
*
*	VOID AAAAudioVolumeUnsigned (ULONG);
*
*   FUNCTION
*	The audio volume is a unsigned six bit number with the maximum level being
*	40.
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
VOID AAAAudioVolumeUnsigned (ULONG This)
{
    if (This & 0x0040)
	CopDisVPrintf (" full volume");
    else
	CopDisVPrintf (" %lx/40", This& 0x003F);
    if (This & 0xFF80)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAAudioVolumeUnsigned */

/****** AAABeamCounter *******************************************************
*
*   NAME
*	AAABeamCounter - Report Beam Counter control bits.
*
*   SYNOPSIS
*	AAABeamCounter (This)
*
*	VOID AAABeamCounter (ULONG);
*
*   FUNCTION
*	BEAMCON0 contains the bits for control of the video beam and counters off
*	the video beam.
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
VOID AAABeamCounter (ULONG This)
{
    WalkSingleBits (This & 0xA477, BEAMCON0);
    if (This & 0x5B88)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABeamCounter */

/****** AAABeamHigh **********************************************************
*
*   NAME
*	AAABeamHigh - Translate the bit map of VPOSW.
*
*   SYNOPSIS
*	AAABeamHigh (This)
*
*	VOID AAAHigh (ULONG);
*
*   FUNCTION
*	VPOS contains high order bits for the vertical beam postion.
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
VOID AAABeamHigh (ULONG This)
{
    if (This & 0x8000)
	CopDisVPrintf (" LOF");
    if (CurrentField == 2)
    {
	CopDisVPrintf (" LongF");
	BeamHigh (This, 0);
	CopDisVPrintf (" ShortF");
	BeamHigh (This, 1);
    }
    else
	BeamHigh (This, CurrentField);
    if (This & 0x7FF8)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABeamHigh */

/****** AAABeamLow ***********************************************************
*
*   NAME
*	AAABeamLow - Translate the bit map of VHPos.
*
*   SYNOPSIS
*	AAABeamLow (This)
*
*	VOID AAABeamLow (ULONG);
*
*   FUNCTION
*	VHPOS contains low order bits for the vertical beam position and
*	eight bits of the horizontal beam position.
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
VOID AAABeamLow (ULONG This)
{
    CopDisVPrintf (" Hor=%03lx", (This & 0x00FF) << 2);
    if (CurrentField == 2)
    {
	CopDisVPrintf (" LongF");
	BeamLow (This, 0);
	CopDisVPrintf (" ShortF");
	BeamLow (This, 1);
    }
    else
	BeamLow (This, CurrentField);
} /* AAABeamLow */

/****** AAABeamPos ***********************************************************
*
*   NAME
*	AAABeamPos - Report Beam position.
*
*   SYNOPSIS
*	AAABeamPos (This)
*
*	VOID AAABeamPos (ULONG);
*
*   FUNCTION
*	VPOSWX allows writing of the light pen/beam position.
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
VOID AAABeamPos (ULONG This)
{
    CopDisVPrintf ("Hor=%0x3lx", This & 0x000007FF);
    LineCount ((This & 0x87FF0000) >> 16);
    if (This & 0x7800F800)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABeamPos */

/****** AAABitmapWidth *******************************************************
*
*   NAME
*	AAABitmapWidth - Check that the unused bits are zero.
*
*   SYNOPSIS
*	AAABitmapWidth (This)
*
*	VOID AAABitmapWidth (ULONG);
*
*   FUNCTION
*	The new registers that are used for specifing a bit map width can only
*	utilize 14 of the 32 bits.
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
VOID AAABitmapWidth (ULONG This)
{
    if (This & 0xFFFFC000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABitmapWidth */

/****** AAABlitterControl0 ***************************************************
*
*   NAME
*	AAABlitterControl0 - Report blitter control setting.
*
*   SYNOPSIS
*	AAABlitterControl0 (This)
*
*	VOID AAABlitterControl0 (ULONG);
*
*   FUNCTION
*	BLTCON0 is used for setting up some input control for the blitter.
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
VOID AAABlitterControl0 (ULONG This)
{
    CopDisVPrintf (" ASH[3:0]=%lx", (This & 0xF000)>> 12);
    WalkSingleBits((This & 0x0F00) >> 8, BLTCON0);
    AAABlitterControl0Short (This & 0x00FF);
} /* AAABlitterControl0 */

/****** AAABlitterControl0Short **********************************************
*
*   NAME
*	AAABlitterControl0Short - Report blitter control setting.
*
*   SYNOPSIS
*	AAABlitterControl0Short (This)
*
*	VOID AAABlitterControl0Short (ULONG);
*
*   FUNCTION
*	BLTCON0L can only set the minterm in the orginal blitter register set.
*	Only the low order eight bit may contain a useful value..
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
VOID AAABlitterControl0Short (ULONG This)
{
    CopDisVPrintf (" LF[7:0]=%lx", This & 0xFF);
    if (This & 0xFF00)
	CopDisVPrintf ("\n\t\t\t****Can only set low order byte with this register\t");
} /* AAABlitterControl0Short */

/****** AAABlitterControl1 ***************************************************
*
*   NAME
*	AAABlitterControl1 - Report blitter control setting.
*
*   SYNOPSIS
*	AAABlitterControl1 (This)
*
*	VOID AAABlitterControl1 (ULONG);
*
*   FUNCTION
*	BLTCON1 sets mode information for the blitter.
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
VOID AAABlitterControl1 (ULONG This)
{
    CopDisVPrintf (" BSH[3:0]=%lx", (This & 0xF000) >> 12);
    if (This & 1)
    {
	WalkSingleBits((This & 0x00FF), BLTCON1LINE);
	if (This & 0x0F00)
	    CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
    }
    else
    {
	WalkSingleBits((This & 0x009F), BLTCON1NL);
	if (This & 0x0F60)
	    CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
    }
} /* AAABlitterControl1 */

/****** AAABlitterEnable *****************************************************
*
*   NAME
*	AAABlitterEnable - Report which external blitter are enabled.
*
*   SYNOPSIS
*	AAABlitterEnable (This)
*
*	VOID AAABlitterEnable (ULONG);
*
*   FUNCTION
*	BLITEN is used to enable some set of the external blitters.
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
VOID AAABlitterEnable (ULONG This)
{
    if (This == 0)
	CopDisVPrintf (" Andrea only");
    else
	WalkSingleBits (This & 0x000F0000, BLITEN);
    if (This & 0xFFF0FFFF)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABlitterEnable */

/****** AAABlitterFunction ***************************************************
*
*   NAME
*	AAABlitterFunction - Report the function requested of the blitter.
*
*   SYNOPSIS
*	AAABlitterFunction (This)
*
*	VOID AAABlitterFunction (ULONG);
*
*   FUNCTION
*	BLTFUNCX is used to control the blitter functions.
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
VOID AAABlitterFunction (ULONG This)
{
    WalkSingleBits ((This & 0x00F00000) >> 20, BLTFUNCX);
    BlitterExtended (This);
} /* AAABlitterFunction */

/****** AAABlitterFunctionLine ***********************************************
*
*   NAME
*	AAABlitterFunctionLine - Report the function requested of the blitter.
*
*   SYNOPSIS
*	AAABlitterFunctionLine (This)
*
*	VOID AAABlitterFunctionLine (ULONG);
*
*   FUNCTION
*	BLTLFUNCX is used to control the blitter for line functions.
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
VOID AAABlitterFunctionLine (ULONG This)
{
    if ((This & 0x00800000) == 0)
	CopDisVPrintf (" No clipping");
    switch (This & 0x00600000)
    {
	case 0x00000000 :
	    CopDisVPrintf (" Clip internal inclusive");
	    break;
	case 0x00200000 :
	    CopDisVPrintf (" Clip internal exclusive");
	    break;
	case 0x00400000 :
	    CopDisVPrintf (" Clip external exclusive");
	    break;
	case 0x00600000 :
	    CopDisVPrintf (" Clip external inclusive");
	    break;
	default :
	    CopDisVPrintf (" REALLY CONFUSED CODE");
	    break;
    }
    if (This & 0x00100000)
	CopDisVPrintf (" CT");
    BlitterExtended (This);
} /* AAABlitterFunctionLine */

/****** AAABlitterPosition ***************************************************
*
*   NAME
*	AAABlitterPosition - Report the position given to the blitter.
*
*   SYNOPSIS
*	AAABlitterPosition (This)
*
*	VOID AAABlitterPosition (ULONG);
*
*   FUNCTION
*	Several registers are used for giving the new blitter pixel coordinates.
*	This routine translates theses values.
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
VOID AAABlitterPosition (ULONG This)
{
    CopDisVPrintf (" XPos=%lx YPos=%lx", (This & 0xFFFF0000) >> 16, This & 0x0000FFFF);
} /* AAABlitterPosition */

/****** AAABlitterSizeSm *****************************************************
*
*   NAME
*	AAABlitterSizeSm - Report the size given to the blitter.
*
*   SYNOPSIS
*	AAABlitterSizeSm (This)
*
*	VOID AAABlitterSizeSm (ULONG);
*
*   FUNCTION
*	Several registers can be used to tell the blitter the size of the blit.
*	This routine will decode the orginal blitter size register.
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
VOID AAABlitterSizeSm (ULONG This)
{
    CopDisVPrintf (" %lxLines %lxPixels", (This & 0x0000FFC0) >> 6, This & 0x0000003F << 4);
} /* AAABlitterSizeSm */

/****** AAABlitterSizeMedH ***************************************************
*
*   NAME
*	AAABlitterSizeMedH - Report the size given to the blitter.
*
*   SYNOPSIS
*	AAABlitterSizeMedH (This)
*
*	VOID AAABlitterSizeMedH (ULONG);
*
*   FUNCTION
*	Several registers can be used to tell the blitter the size of the blit.
*	This routine will decode the ECS blitter size register.
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
VOID AAABlitterSizeMedH (ULONG This)
{
    CopDisVPrintf (" %lxWords/LongWords", This & 0x000007FF);
    if (This & 0x0000F800)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABlitterSizeMedH */

/****** AAABlitterSizeMedV ***************************************************
*
*   NAME
*	AAABlitterSizeMedV - Report the size given to the blitter.
*
*   SYNOPSIS
*	AAABlitterSizeMedV (This)
*
*	VOID AAABlitterSizeMedV (ULONG);
*
*   FUNCTION
*	Several registers can be used to tell the blitter the size of the blit.
*	This routine will decode the ECS blitter size register.
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
VOID AAABlitterSizeMedV (ULONG This)
{
    CopDisVPrintf (" %lxLines", This & 0x00007FFF);
    if (This & 0x00008000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABlitterSizeMedV */

/****** AAABlitterSizeLar ***************************************************
*
*   NAME
*	AAABlitterSizeLar - Report the size given to the blitter.
*
*   SYNOPSIS
*	AAABlitterSizeLar (This)
*
*	VOID AAABlitterSizeLar (ULONG);
*
*   FUNCTION
*	Several registers can be used to tell the blitter the size of the blit.
*	This routine will decode the new blitter size register.
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
VOID AAABlitterSizeLar (ULONG This)
{
    CopDisVPrintf (" %lxLines %lxPixels", (This & 0x07FFF0000) >> 16, This & 0x000007FFF);
    if (This & 0x80008000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABlitterSizeLar */

/****** AAABPEnhance *********************************************************
*
*   NAME
*	AAABPEnhance - Translate the bit map for register BPLCON3.
*
*   SYNOPSIS
*	AAABPEnhance (This)
*
*	VOID AAABPEnhance (ULONG);
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
VOID AAABPEnhance (ULONG This)
{
    if ((This & 0x8000) != 0)
	switch (This & 0x0180)
	{
	    case 0x0000 :
		CopDisVPrintf (" 4x LINE");
		break;
	    case 0x0080 :
		CopDisVPrintf (" 3x LINE");
		break;
	    case 0x0100 :
		CopDisVPrintf (" 2x LINE");
		break;
	    case 0x0180 :
		CopDisVPrintf (" 1x LINE");
		break;
	}
    else
	CopDisVPrintf(" NO_LINE_RPT");
    switch (This & 0x7000)
    {
	case 0x0000 :
	    CopDisVPrintf (" BITPLN");
	    break;
	case 0x1000 :
	    CopDisVPrintf (" HYBRID");
	    break;
	case 0x2000 :
	    CopDisVPrintf (" CHUNKY");
	    break;
	case 0x3000 :
	    CopDisVPrintf (" PACKLUT");
	    break;
	case 0x4000 :
	    CopDisVPrintf (" HALFCHUNKY4");
	    break;
	case 0x5000 :
	    CopDisVPrintf (" HALFCHUNKY");
	    break;
	case 0x6000 :
	    CopDisVPrintf (" HALFCHUNKY2");
	    break;
	case 0x7000 :
	    CopDisVPrintf (" PACKHY");
	    break;
    }
    CopDisVPrintf (" PIXCLK=");
    switch (This & 0x7000)
    {
	case 0x0000 :
	    CopDisVPrintf (" Native");
	    break;
	case 0x1000 :
	    CopDisVPrintf (" 2ndHigh");
	    break;
	case 0x2000 :
	    CopDisVPrintf (" 3rdHigh");
	    break;
	case 0x3000 :
	    CopDisVPrintf (" 4thHigh");
	    break;
	case 0x4000 :
	    CopDisVPrintf (" Slowest");
	    break;
	default :
	    CopDisVPrintf (" INVALID");
	    break;
    }

    WalkSingleBits ((This & 0x007E) >> 1, BPLCON3);
    if (This & 0x0001)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABPEnhance */

/****** AAABPMiscControl ******************************************************
*
*   NAME
*	AAABPMiscControl - Translate the bit map of BPLCON0.
*
*   SYNOPSIS
*	AAABPMiscControl (This)
*
*	VOID AAABPMiscControl (ULONG);
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
VOID AAABPMiscControl (ULONG This)
{
    if (This & 0x0080)
	CopDisVPrintf (" HAM8 BP=%lx", ((This & 0x0010) >> 1) + (This & 0x7000) >> 12);
    else
    {
	if (This &0x0800)
	    CopDisVPrintf (" HAM");
	else
	    if (This &0x7030)
		CopDisVPrintf (" BPUse=%x", ((This & 0x0030) >> 1) + (This & 0x7000) >> 12);
	    else
		CopDisVPrintf (" NoBPUse");
    }
    WalkSingleBits (This & 0x870F, BPLCON0);
    if (This & 0x00000040)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAABPMiscControl */


/****** AAABPModulo **********************************************************
*
*   NAME
*	AAABPModulo - Report a bit plane modulo.
*
*   SYNOPSIS
*	AAABPModulo (This)
*
*	VOID AAABPModulo (ULONG);
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
VOID AABPModulo (ULONG This)
{
	CopDisVPrintf (" Modulo=%04lx", This & 0xFFFF);
} /* AABPModulo */

/****** AAABPPriority ********************************************************
*
*   NAME
*	AAABPPriority - Translate the bit map of BPLCON2.
*
*   SYNOPSIS
*	AAABPPriority (This)
*
*	VOID AAABPPrioiry (ULONG);
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
VOID AAABPPriority (ULONG This)
{
    CopDisVPrintf (" ZDBPSEL=%lx PF2P=%lx PF1P=%lx", (This & 0xF000) >> 12, (This & 0x0038) >> 3, This & 0x0007);
    WalkSingleBits ((This & 0x0FC0)>>6, BPLCON2);
    if (This & 0x0100)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AABPPriority */

/****** AAABPSpriteMagic *****************************************************
*
*   NAME
*	AAABPSpriteMagic - Translate the bit map of BPLCON4.
*
*   SYNOPSIS
*	AAABPSprite (This)
*
*	VOID AAABPPrioiry (ULONG);
*
*   FUNCTION
*	BPLCON4 contains a value for XORing before accessing the BP LUT.  Also
*	there are bits for offsetting the start of the Sprite LUTs.
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
VOID AAABPSpriteMagic (ULONG This)
{
    CopDisVPrintf (" BPLAM=%02lx ESPRM=%02lx OSPRM=%02lx", (This & 0xFF00) >> 8, This & 0x00F0, (This & 0x000F)<< 4);
} /* AABPSpriteMagic */

/****** AAACollision *********************************************************
*
*   NAME
*	AAACollision - Translate the bit map of CLXCON.
*
*   SYNOPSIS
*	AAACollision (This)
*
*	VOID AAACollision (ULONG);
*
*   FUNCTION
*	CLXCON contains a bit map indicated what types of collisions should be
*	detected.
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
VOID AAACollision (ULONG This)
{
    WalkSingleBits (This & 0xFFFF, CLXCON);
} /* AAACollision */

/****** AAACollisionCHUNKYEnable *********************************************
*
*   NAME
*	AAACollisionCHUNKYEnable - Translate the bit map of CLXCONOEN.
*
*   SYNOPSIS
*	AAACollisionCHUNKEnable (This)
*
*	VOID AAACollisionCHUNKYEnable (ULONG);
*
*   FUNCTION
*	CLXCONOEN contains a bit map indicated what types of collisions should be
*	detected.
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
VOID AAACollisionCHUNKYEnable (ULONG This)
{
    WalkSingleBits (This & 0x00FFFFFF, CLXCONOEN);
    if (This & 0xFF000000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAACollisionCHUNKYEnable */

/****** AAACollisionCHUNKYMatch **********************************************
*
*   NAME
*	AAACollisionCHUNKMatch - Translate the bit map of CLXCONODN.
*
*   SYNOPSIS
*	AAACollisionCHUNKYMatch (This)
*
*	VOID AAACollisionCHUNKYMatch (ULONG);
*
*   FUNCTION
*	CLXCONODN contains a bit map indicated what types of collisions should be
*	detected.
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
VOID AAACollisionCHUNKYMatch (ULONG This)
{
    WalkSingleBits (This & 0x00FFFFFF, CLXCONODN);
    if (This & 0xFF000000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAACollisionCHUNKYMatch */

/****** AAACollisionXtend ****************************************************
*
*   NAME
*	AAACollisionXtend - Translate the bit map of CLXCONX.
*
*   SYNOPSIS
*	AAACollisionXtend (This)
*
*	VOID AAACollisionXtend (ULONG);
*
*   FUNCTION
*	CLXCONX contains a bit map indicated what types of collisions should be
*	detected.
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
VOID AAACollisionXtend (ULONG This)
{
    WalkSingleBits (This, CLXCONX);
} /* AAACollisionXtend */

/****** AAACopperControl *****************************************************
*
*   NAME
*	AAACopperControl - Translate the bit map for register COPCON.
*
*   SYNOPSIS
*	AAACopperControl (This)
*
*	VOID AAACopperControl (ULONG);
*
*   FUNCTION
*	COPCON determines whether the copper can poke all registers, or not.
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
VOID AAACopperControl (ULONG This)
{
    if ((This & 0x00000001) != 0)
	CopDisVPrintf (" CDANG");
    if (This & 0xFFFFFFFE)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAACopperControl */

/****** AAACopperInterrupt ****************************************************
*
*   NAME
*	AAACopperControl - Translate the bit map for register COPCON.
*
*   SYNOPSIS
*	AAACopperControl (This)
*
*	VOID AAACopperControl (ULONG);
*
*   FUNCTION
*	COPCON determines whether the copper can poke all registers, or not.
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
VOID AAACopperInterrupt (ULONG This)
{
    if ((This & 0x80000000L) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    if ((This & 0x00010000) != 0)
	CopDisVPrintf (" BLITREQ");
    if (This & 0x7FFEFFFF)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAACopperInterrupt */

/****** AAADataFetch *********************************************************
*
*   NAME
*	AAADataFetch - Report the settings of H[9:0] for display data fetch.
*
*   SYNOPSIS
*	AAADataFetch (This)
*
*	VOID AAADataFetch (ULONG);
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
VOID AAADataFetch (ULONG This)
{
    CopDisVPrintf (" H[9:0]=%03lx", ((This & 0x00FF)<< 1) | ((This & 0x8000) >> 15));
    if (This & 0x7F00)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAADataFetch */

/****** AAADiskControl *******************************************************
*
*   NAME
*	AAADiskControl - Translate the bit map for register ADKCONX.
*
*   SYNOPSIS
*	AAADiskControl (This)
*
*	VOID AAADiskControl (ULONG);
*
*   FUNCTION
*	ADKCONX contains methods for controling disk functions.
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
VOID AAADiskControl (ULONG This)
{
    if ((This & 0x80000000L) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits ((This & 0x3FF0FFFFL), ADKCON);
    if (This & 0x400F0000L)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAADiskControl */

/****** AAADisplayNew ********************************************************
*
*   NAME
*	AAADisplayNew - Translate the extended Display Start and Stop.
*
*   SYNOPSIS
*	AAADisplayNew (This)
*
*	VOID AAADisplayNew (ULONG);
*
*   FUNCTION
*	DIWSTRTX and DIWSTOPX contain eleven bit values for the horizontal and
*	vertical start/stop positions.
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
VOID AAADisplayNew (ULONG This)
{
    CopDisVPrintf (" Vertical=%03lx Horizontal=%03lx", (This & 0x07FF0000) >> 16, This & 0x07FF);
    if (This & 0xF800F800)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAADisplayNew */

/****** AAADisplayStart ******************************************************
*
*   NAME
*	AAADisplayStart - Translate DIWSTART.
*
*   SYNOPSIS
*	AAADisplayStart (This)
*
*	VOID AADisplayStart (ULONG);
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
VOID AAADisplayStart (ULONG This)
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
} /* AAADisplayStart */

/****** AAADisplayStop *******************************************************
*
*   NAME
*	AAADisplayStop - Translate DIWSTOP.
*
*   SYNOPSIS
*	AAADisplayStop (This)
*
*	VOID AADisplayStop (ULONG);
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
VOID AAADisplayStop (ULONG This)
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
} /* AAADisplayStop */

/****** AAADMA16 *************************************************************
*
*   NAME
*	AAADMA16 - Translate DMACON.
*
*   SYNOPSIS
*	AAADMA16 (This)
*
*	VOID AAADMA16 (ULONG);
*
*   FUNCTION
*	DMACON is used to enable and disable 16 bit DMA channels.
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
VOID AAADMA16 (ULONG This)
{
    if ((This & 0x8000) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits (This & 0x07FF, DMACON);
    if (This & 0x7800)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAADMA16 */

/****** AAADMA32 *************************************************************
*
*   NAME
*	AAADMA32 - Translate DMACONX.
*
*   SYNOPSIS
*	AAADMA32 (This)
*
*	VOID AAADMA32 (ULONG);
*
*   FUNCTION
*	DMACONX is used to enable and disable 32 bit DMA channels.
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
VOID AAADMA32 (ULONG This)
{
    if ((This & 0x80000000) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits (This & 0x00FFFFFF, DMACONX);
    if (This & 0x7F000000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAADMA32 */

/****** AAAHScroll ***********************************************************
*
*   NAME
*	AAAHScroll - Translate BPLCON1.
*
*   SYNOPSIS
*	AAAHScroll (This)
*
*	VOID AAAHScroll (ULONG);
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
VOID AAAHScroll (ULONG This)
{
    CopDisVPrintf (" PF1H=%lx PF2H=%lx", (This & 0x000F), (This & 0x00F0) >> 4);
    if (This & 0x0000FF00)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAAHScroll */

/****** AAAInterrupt *********************************************************
*
*   NAME
*	AAAInterrupt - Translate the interrupt request and enable registers.
*
*   SYNOPSIS
*	AAAInterrupt (This)
*
*	VOID AAAInterrupt (ULONG);
*
*   FUNCTION
*	Translate the value written to either the interrupt request, or the
*	interrupt enable register.
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
VOID AAAInterrupt (ULONG This)
{
    if ((This & 0x8000) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits (This & 0x7FFF, INTE);
} /* AAAInterrupt */

/****** AAAInterruptXtend ****************************************************
*
*   NAME
*	AAAInterruptXtend - Translate the interrupt request and enable registers.
*
*   SYNOPSIS
*	AAAInterruptXtend (This)
*
*	VOID AAAInterruptXtend (ULONG);
*
*   FUNCTION
*	Translate the value written to either the interrupt request, or the
*	interrupt enable register.
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
VOID AAAInterruptXtend (ULONG This)
{
    if ((This & 0x80000000) == 0)
	CopDisVPrintf (" CLR");
    else
	CopDisVPrintf (" SET");
    WalkSingleBits (This & 0x7FFFFFFF, INTE);
} /* AAAInterruptXtend */

/****** AAALUTOffsets ********************************************************
*
*   NAME
*	AAALUTOffsets - Translate the bit map of BPLOFFN.
*
*   SYNOPSIS
*	AAALUTOffsets (This)
*
*	VOID AAABPPrioiry (ULONG);
*
*   FUNCTION
*	BPLOFFN contains offsets added to the values used in accessing the
*	color LUT for the following accesses.
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
VOID AAALUTOffsets (ULONG This)
{
    CopDisVPrintf (" ESPRM=%02lx OSPRM=%02lx EBPLOFF=%02lx OBPLOFF=%02lx", (This & 0xFF000000) >> 24, (This & 0x00FF0000) >> 16, (This & 0x0000FF00) >> 8, This & 0x000000FF);
} /* AAALUTOffsets */

/****** AAASpriteControl *****************************************************
*
*   NAME
*	AAASpriteControl - Translate SPRxCTL.
*
*   SYNOPSIS
*	AAASpriteControl (This)
*
*	VOID AAASpriteControl (ULONG);
*
*   FUNCTION
*	Report the vertical sprite end position, extra bits for start, and other
*	general sprite information.
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
VOID AAASpriteControl (ULONG This)
{
    CopDisVPrintf (" EV=%03lx", (This & 0xFF) >> 8);
    WalkSingleBits (This & 0x00FF, SPRXCTL);
} /* AAASpriteControl */

/****** AAASpriteControlXtend ************************************************
*
*   NAME
*	AAASpriteControlXtend - Translate SPRxCTLX.
*
*   SYNOPSIS
*	AAASpriteControlXtend (This)
*
*	VOID AAASpriteControlXtend (ULONG);
*
*   FUNCTION
*	Report the horizontal sprite start position and other general sprite
*	information.
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
VOID AAASpriteControlXtend (ULONG This)
{
    CopDisVPrintf (" HS=%03lx", This & 0x000003FF);
    WalkSingleBits ((This & 0x01C10000), SPRXCTLX);
    if (This & 0xE000FC00)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAASpriteControlXtend */

/****** AAASpriteStart *******************************************************
*
*   NAME
*	AAASpriteStart - Translate SPRxPOS.
*
*   SYNOPSIS
*	AAASpriteStart (This)
*
*	VOID AAASpriteStart (ULONG);
*
*   FUNCTION
*	Report the horizontal and vertical sprite start positions.
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
VOID AAASpriteStart (ULONG This)
{
    CopDisVPrintf (" SV=%03lx HS=%3lx", (This & 0xFF00) >> 8, (This & 0x00FF) << 2);
} /* AAASpriteStart */

/****** AAASpriteStartXtend **************************************************
*
*   NAME
*	AAASpriteStartXtend - Translate SPRxPOSX.
*
*   SYNOPSIS
*	AAASpriteStartXtend (This)
*
*	VOID AAASpriteStartXtend (ULONG);
*
*   FUNCTION
*	Report the horizontal and vertical sprite start positions.
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
VOID AAASpriteStartXtend (ULONG This)
{
    switch (This & 0x0C000000)
    {
	case 0x08000000 :
	    CopDisVPrintf (" BURST128");
	    break;
	case 0x04000000 :
	    CopDisVPrintf (" BURST64");
	    break;
	case 0x0C000000 :
	    CopDisVPrintf ("\n\t\t\t****BURST64&128????\t");
	    break;
    }
    CopDisVPrintf (" SV=%03lx EV=%3lx", (This & 0x03FF0000) >> 16, This & 0x03FF);
    if (This & 0xF000C000L)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* AAASpriteStartXtend */

/****** AAAXDisplay **********************************************************
*
*   NAME
*	AAAXDisplay - Translate the bit map of DIWHIGH.
*
*   SYNOPSIS
*	AAAXDisplay (This)
*
*	VOID AAAXDisplay (ULONG);
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
VOID AAAXDisplay (ULONG This)
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
} /* AAAXDisplay */

/****** LineCount ************************************************************
*
*   NAME
*	LineCount - report the line counter comparison value.
*
*   SYNOPSIS
*	LineCount (This)
*
*	VOID LineCount (ULONG);
*
*   FUNCTION
*	Writes the low order eleven bits of a number and checks the remainder
*	for zeros.  The bit 15 indicates the units being counted.
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
VOID LineCount (ULONG This)
{
    CopDisVPrintf (" %04lx%sLines", This & 0x07FF, (This & 0x8000) ? "Half" : "");
    if (This & 0x7800L)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* LineCount */

/****** LowResColorClk *******************************************************
*
*   NAME
*	LowResColorClk - report a position in numbers of horizontal color clocks.
*
*   SYNOPSIS
*	LowResColorClk (This)
*
*	VOID LowResColorClk (ULONG);
*
*   FUNCTION
*	Shifts the value and calls the high resolution color clock routine.
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
VOID LowResColorClk (ULONG This)
{
    HighResColorClk ((This & 0xFFFF) << 2);
} /* LowResColorClk */

/****** HighResColorClk ******************************************************
*
*   NAME
*	HighResColorClk - report a position in numbers of horizontal color clocks.
*
*   SYNOPSIS
*	HighResColorClk (This)
*
*	VOID AABPModulo (ULONG);
*
*   FUNCTION
*	Writes the low order ten bits of a number and checks the remainder
*	for zeros.
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
VOID HighResColorClk (ULONG This)
{
    CopDisVPrintf (" %04lxHighResColorClocks", This & 0x03FF);
    if (This & 0xFFFFFC00L)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* HighResColorClk */

/****** HighWordLineCount ******************************************************
*
*   NAME
*	HighWordLineCount - Report the high order word as a line count.
*
*   SYNOPSIS
*	HighWordLineCount (This)
*
*	VOID HighWordLineCount (ULONG);
*
*   FUNCTION
*	Passes the high order word of the value to the LineCount routine.
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
VOID HighWordLineCount (ULONG This)
{
    LineCount ((This & 0xFFFF0000) >>16);
    if (This & 0xFFFFL)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* HighWordLineCount */

/****** MonicaTest **********************************************************&
*
*   NAME
*	MonicaTest - translate the write to the Monica test register.
*
*   SYNOPSIS
*	MonicaTest (This)
*
*	VOID MonicaTest (ULONG);
*
*   FUNCTION
*	Translates MONTEST.
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
VOID MonicaTest (ULONG This)
{
    if (This & 0x00000004)
	CopDisVPrintf (" DIGON");
    if (This & 0xFFFFFFFBL)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
} /* MonicaTest */






/****** LocState *************************************************************
*
*   NAME
*	LocState - Report the last known state of the LOC bit.
*
*   SYNOPSIS
*	Set = LocState (Field)
*
*	BOOL LocState (ULONG);
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

/****** ResetAAADisplay ******************************************************
*
*   NAME
*	ResetAAADisplay - Reset any stored information about the current window.
*
*   SYNOPSIS
*	ResetAAADisplay ()
*
*	VOID ResetAAADisplay (VOID);
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
VOID ResetAAADisplay (VOID)
{
	Window[0].BeamDef = Window[0].StopDef = Window[0].StartDef = Window[0].LocState = Window[1].BeamDef = Window[1].StopDef = Window[1].StartDef = Window[1].LocState = FALSE;
} /* ResetAAADisplay */

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
