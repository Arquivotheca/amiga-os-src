/****************************************************************************
 **									   **
 **  pref.h								   **
 **									   **
 **  Confidential Information: Commodore-Amiga, Inc.			   **
 **  Copyright (c) 1989 Commodore-Amiga, Inc.				   **
 **									   **
 ****************************************************************************
 *
 * SOURCE CONTROL
 *
 * $Header: /usr/gilligan/eric/amiga/src/editors/printer/RCS/pref.h,v 36.0 90/08/06 11:19:43 eric Exp Locker: eric $
 *
 * $Locker: eric $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>


/* ===== Printer.prefs =================================================== */

#define	FNAMESIZE	30		/* Filename size */

/* PrinterPort */
#define PARALLEL	0x00
#define SERIAL		0x01

/* PaperType */
#define FANFOLD 	0x00
#define SINGLE		0x01

/* PaperSize */
#define US_LETTER	0x00
#define US_LEGAL	0x01
#define N_TRACTOR	0x02
#define W_TRACTOR	0x03
#define CUSTOM		0x04
#define EURO_A0		0x05		/* European size A0: 841 x 1189 */
#define EURO_A1		0x06		/* European size A1: 594 x 841 */
#define EURO_A2		0x07		/* European size A2: 420 x 594 */
#define EURO_A3		0x08		/* European size A3: 297 x 420 */
#define EURO_A4		0x09		/* European size A4: 210 x 297 */
#define EURO_A5		0x0A		/* European size A5: 148 x 210 */
#define EURO_A6		0x0B		/* European size A6: 105 x 148 */
#define EURO_A7		0x0C		/* European size A7: 74 x 105 */
#define EURO_A8		0x0D		/* European size A8: 52 x 74 */

/* PrintPitch */
#define PICA		0x0000
#define ELITE		0x0001
#define FINE		0x0002

/* PrintSpacing */
#define SIX_LPI		0x0000
#define EIGHT_LPI	0x0001

/* PrintQuality */
#define DRAFT		0x0000
#define LETTER		0x0001


typedef struct PTextPref {
    LONG  pp_Reserved[4];		/* System reserved */
    UBYTE pp_Driver[FNAMESIZE];		/* printer driver filename */
    UBYTE pp_Port;			/* printer port connection  */

    UWORD pp_PaperType;			/* Default: FANFOLD */
    UWORD pp_PaperSize;			/* Default: US_LETTER */
    UWORD pp_PaperLength;		/* Paper length in # of lines */

    UWORD pp_Pitch;			/* Default: PICA */
    UWORD pp_Spacing;			/* Default: SIX_LPI */
    UWORD pp_LeftMargin;		/* Left margin */
    UWORD pp_RightMargin;		/* Right margin */
    UWORD pp_Quality;			/* Default: DRAFT */
} PREF;

#define ENV_NAME "ENV:Sys/printer.prefs"
#define ARC_NAME "ENVARC:Sys/printer.prefs"
#define PRE_DIR  "Sys:Prefs/Presets"
#define PRE_NAME "Printer.pre"
#define REQ_LOAD  "Load Printer Preferences"
#define REQ_SAVE  "Save Printer Preferences"
#define F_LENGTH (sizeof(PREF))

#define ENV_TITLE	"Printer Preferences"
#define TASK_NAME	"printer_pref_task"
#define TOOL_NAME	"SYS:Prefs/Printer"

#define ENV_DEF		{0L, 0L, 0L, 0L, "generic", PARALLEL, FANFOLD, N_TRACTOR, 66, PICA, SIX_LPI, 5, 75, DRAFT}

#endif


