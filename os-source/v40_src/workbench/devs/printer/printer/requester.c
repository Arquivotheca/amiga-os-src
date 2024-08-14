/* request.c
 * Pop up a requester for printer timeouts.
 *
 */

#define	PRINTER_DEVICE	TRUE
#define	STRINGARRAY	TRUE

/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <devices/parallel.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/locale.h>
#include <localestr/devs.h>
#include "printer.h"
#include "prtbase.h"

/* prototypes */
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/*****************************************************************************/

LONG __stdargs TimeDelay( long unit, unsigned long secs, unsigned long microsecs );

/*****************************************************************************/

extern struct IntuitionBase *IntuitionBase;
extern struct Library *SysBase;

/*****************************************************************************/

struct Library *LocaleBase;
struct Catalog *catalog;

/*****************************************************************************/

STRPTR GetString (LONG id)
{
    STRPTR local = NULL;
    UWORD i;

    i = 0;
    while (!local)
    {
	if (AppStrings[i].as_ID == id)
	    local = AppStrings[i].as_Str;
	i++;
    }

    if (LocaleBase)
    {
	return (GetCatalogStr (catalog, id, local));
    }

    return (local);
}

/*****************************************************************************/

VOID OpenCat (VOID)
{
    if (LocaleBase = OpenLibrary ("locale.library", 38))
	catalog = OpenCatalogA (NULL, "sys/devs.catalog", NULL);
}

/*****************************************************************************/

VOID CloseCat (VOID)
{
    if (LocaleBase)
    {
	CloseCatalog (catalog);
	CloseLibrary (LocaleBase);
	LocaleBase = NULL;
    }
}

#if 0
constructMessage:
		MOVE.L	_OpenTask,A0
		CMP.B	#NT_PROCESS,LN_TYPE(A0)
		BNE.S	noProcess
		MOVE.L	pr_WindowPtr(A0),A0
		CMPA	#-1,A0		;-1 as WindowPtr ?
		BNE.S	itPatch		;no, give the requester
		CLR.L  	D0		;clear
		BRA.S	itPatch1	;as if cancel were hit

noProcess:
		SUBA.L	A0,A0
itPatch:

		;window ptr in A0, message # in D1

		BSR	_TroubleNotify

itPatch1:
		MOVEM.L	(A7)+,D2/D3/A2/A3
		TST.L	D0			;cancel (0-yes, 1-no)
		BNE.S	continue		;no
#endif

/*****************************************************************************
 * pulls easy-request but keeps checking for conditions where it thinks it
 * might be able to continue successfully.  When that happens, it will pull
 * down the autorequest and return TRUE, just as it will if the user selects
 * RETRY in the requester.
 */
LONG __asm TroubleNotify (register __a0 struct Window * win, register __d1 ULONG message_number)
{
    extern struct PrinterExtendedData *PED;
    extern struct PrinterData *PD;
    struct EasyStruct es;
    LONG result;
    STRPTR arg;

    es.es_StructSize = sizeof (struct EasyStruct);
    es.es_Flags = 0;
    es.es_Title = NULL;		/* Use default (localized) System Request string for title */
    es.es_TextFormat = GetString (MSG_PRI_DEV_TROUBLE);
    es.es_GadgetFormat = GetString (MSG_PRI_DEV_DOQUERY);

    message_number += 2;
    arg = GetString (message_number);
#if 1
    result = EasyRequestArgs (win, &es, NULL, &arg);
#else
    struct IOExtPar par;

    win = BuildEasyRequest (win, &es, NULL, arg);
    while (TRUE)
    {
	TimeDelay (UNIT_MICROHZ, 1, 0);

	result = SysReqHandler (win, NULL, FALSE);
	if (result >= 0)
	    break;

	if (PD->pd_Preferences.PrinterPort == PARALLEL_PRINTER)
	{
	    par = PD->pd_ior0.pd_p0;
	    par.IOPar.io_Command = PDCMD_QUERY;
	    DoIO(&par);

	    result = 1;
	    if ((message_number == MSG_PRI_DEV_OFFLINE) && !(par.io_Status & 0x1))
		break;
	    else if ((message_number == MSG_PRI_DEV_NOPAPER) && !(par.io_Status & 0x3))
		break;
	}
    }

    FreeSysRequest (win);
#endif

    return (result);
}
