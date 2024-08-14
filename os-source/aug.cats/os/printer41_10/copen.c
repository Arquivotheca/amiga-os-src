#include "exec/types.h"
#include "exec/errors.h"

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "printer.h"
#include "prtbase.h"

#include "printer_iprotos.h"

#define OPDEBUG	0

#define	DB(x)	;
void kprintf (void *, ...);

extern struct PrinterData *PD;
extern struct PrinterExtendedData *PED;
extern struct PrinterExtendedData PEDDefault;
extern LoadedPrinterFile[];

#define	KNOWNPTYPES	2

#define COND1	1

static char *printKnownNames[KNOWNPTYPES] =
{
    0,				/* Custom is index 0 */
    "generic",
};

OpenPrinter ()
{
    UBYTE printer_name[64];
    char *pFileName;
    ULONG special;
    UWORD pType;
    char *s1;				/* ptr to user selected printer file */
    char *s2;				/* ptr to previously loaded printer file (may be NULL) */
    char *s3;

#if COND1
#else
	LONG prevCurrDir;
	LONG fontDir;
#endif

    GetPrefs (&(PD->pd_Preferences), sizeof (PD->pd_Preferences));
    pType = PD->pd_Preferences.PrinterType;
    if (pType == 0)
	s1 = pFileName = PD->pd_Preferences.PrinterFilename;
    else
	s1 = pFileName = printKnownNames[1];

    if (*s1 == '\0')
	return (IOERR_OPENFAIL);	/* for null custom printer */

    special = 0;
    GetSpecialDensity (&special);
    s2 = (char *)LoadedPrinterFile;
    if (strcmpnc (s1, s2) == 0)
    {				/* names match, printer file already open */
	OpenAlpha (1);
	if (PD->pd_SegmentData->ps_Version >= 35)
	{			/* if >= V1.3 */
	    PED->ped_PrintMode = 0;	/* clear mode flag (assume graphics) */
	}
	/* init special stuff (density, etc.) */
	/* the 5th parameter here is for V1.0 compatability */
	(*(PED->ped_Render)) (0, special, 0, 5, special);
	return (0);
    }

    /* if this far then loading a new printer specific file */
    if (PD->pd_PrinterSegment != 0)
    {				/* if previously loaded a segment */
	UnLoadSeg (PD->pd_PrinterSegment);	/* unload old segment */
	PD->pd_PrinterSegment = 0;	/* flag no segment loaded */
	PD->pd_PrinterType = -1;/* flag no printer loaded */
    }

#if COND1
    sprintf (printer_name, "PRINTERS:%s", pFileName);
    PD->pd_PrinterSegment = (BPTR) LoadSeg (printer_name);

    if (!PD->pd_PrinterSegment)
    {
	sprintf (printer_name, "DEVS:printers/%s", pFileName);
	PD->pd_PrinterSegment = (BPTR) LoadSeg (printer_name);
    }

    /* CAS */
    if (!PD->pd_PrinterSegment) return (IOERR_OPENFAIL);

    if (PD->pd_SegmentData = (struct PrinterSegment *) ((ULONG) (PD->pd_PrinterSegment) << 2))
    {
	PED = &(PD->pd_SegmentData->ps_PED);

	/* init printer specific code */  /* CAS */
	if(-1 == (*(PED->ped_Init))(PD))  return(IOERR_OPENFAIL);

	OpenAlpha (0);
	PD->pd_PrinterType = pType;
	s2 = (char *)LoadedPrinterFile;

        /* s3 = PED->ped_PrinterName;*/
	/* use the name of the file we just loaded */
	s3 = pFileName;

	while (*s2++ = *s3++) ;	    /* ?????? */

	if (PD->pd_SegmentData->ps_Version >= 35)
	{		    /* if >= V1.3 */
	    PED->ped_PrintMode = 0; /* clear mode flag (assume gfx) */
	}

	/* init	special	stuff (density,	etc.) */
	/* the 5th parameter here is for V1.0 compatability */
	(*(PED->ped_Render)) (0, special, 0, 5,	special);
	return (0);
    }

#else
    if ((fontDir = Lock ("DEVS:printers", SHARED_LOCK)) != 0)
    {
	prevCurrDir = CurrentDir (fontDir);
	/* load printer specific segment */
	PD->pd_PrinterSegment = (BPTR) LoadSeg (pFileName);
	if (PD->pd_SegmentData = (struct PrinterSegment *)
	    ((ULONG) (PD->pd_PrinterSegment) << 2))
	{			/* if loaded */
	    PED = &(PD->pd_SegmentData->ps_PED);
	    (*(PED->ped_Init)) (PD);	/* init printer specific code */
	    OpenAlpha (0);
	    PD->pd_PrinterType = pType;
	    s2 = LoadedPrinterFile;
/*			s3 = PED->ped_PrinterName;*/
	    s3 = pFileName;	/* use the name of the file we just loaded */
	    while (*s2++ = *s3++) ;	/* ?????? */
	    CurrentDir (prevCurrDir);
	    UnLock (fontDir);
	    if (PD->pd_SegmentData->ps_Version >= 35)
	    {			/* if >= V1.3 */
		PED->ped_PrintMode = 0;	/* clear mode flag (assume gfx) */
	    }
	    /* init special stuff (density, etc.) */
	    /* the 5th parameter here is for V1.0 compatability */
	    (*(PED->ped_Render)) (0, special, 0, 5, special);
	    return (0);
	}
	CurrentDir (prevCurrDir);
	UnLock (fontDir);
    }
#endif
    return (IOERR_OPENFAIL);
}

ExpungePrinter ()
{

    if (PD->pd_PrinterSegment != 0)
    {				/* if we have one loaded */
	(*(PED->ped_Expunge)) ();	/* expunge it */
	UnLoadSeg (PD->pd_PrinterSegment);	/* unload it */
    }
}

strcmpnc (s1, s2)		/* string compare, no case */
    char *s1, *s2;
{
    char c1, c2;

    do
    {
	c1 = *s1++;
	c2 = *s2++;
	if (c1 >= 'a' && c1 <= 'z')
	{
	    c1 -= 0x20;
	}
	if (c2 >= 'a' && c2 <= 'z')
	{
	    c2 -= 0x20;
	}
    } while (c1 == c2 && c1 && c2);
    if (c1 == c2)
    {
	return (0);
    }
    else
    {
	return (c1 == 0 ? -1 : 1);
    }
}
