#include <exec/types.h>
#include "printer.h"
#include "prtbase.h"

#include "printer_iprotos.h"

#define DEBUGOA		0
#define DEBUGPCP	0
#define DEBUGTC		0

extern struct PrinterData *PD;
extern struct PrinterExtendedData *PED;

extern UBYTE InputBuffer[];
extern BYTE vline;
extern BYTE crlfFlag;
extern UBYTE currentVMI;
extern int Counter;
extern int ParmCount;
extern int RawCount;

USHORT AprintPitch;
USHORT AprintQuality;
UWORD AprintSpacing;
UWORD AprintLeftMargin;
UWORD AprintRightMargin;
UWORD ApaperLength;
int HoldPrefs=0;

VOID OpenAlpha(start)
int start; /* 0-first open, 1-already open */
{
	vline=0;
	crlfFlag=0;
	PD->pd_PWaitEnabled=0;
	Counter=0;
	RawCount = 0;
	if ((!start) || (HoldPrefs) || (cmppPref())) {
		savepPref();
		HoldPrefs = 1;
		/* force a read from the preferences */
		Counter = 3;
		InputBuffer[0] = '\033';
		InputBuffer[1] = '#';
		InputBuffer[2] = '1'; /* initialize */
	}
#if DEBUGOA
	kprintf("OA: PWaitEnabled = %ld\n", PD->pd_PWaitEnabled);
#endif /* DEBUGOA */
}

savepPref()
{
	AprintPitch = PD->pd_Preferences.PrintPitch;
	AprintQuality = PD->pd_Preferences.PrintQuality;
	AprintSpacing = PD->pd_Preferences.PrintSpacing;
	AprintLeftMargin = PD->pd_Preferences.PrintLeftMargin;
	AprintRightMargin = PD->pd_Preferences.PrintRightMargin;
	ApaperLength = PD->pd_Preferences.PaperLength;
	return (0);
}


cmppPref()
{
	if ((AprintPitch != PD->pd_Preferences.PrintPitch) ||
		(AprintQuality != PD->pd_Preferences.PrintQuality) ||
		(AprintSpacing != PD->pd_Preferences.PrintSpacing) ||
		(AprintLeftMargin != PD->pd_Preferences.PrintLeftMargin) ||
		(AprintRightMargin != PD->pd_Preferences.PrintRightMargin) ||
		(ApaperLength != PD->pd_Preferences.PaperLength)) {
		return(-1);
	}
	return(0);
}

/****** printer.device/PRD_PRTCOMMAND ***************************************
*
*   NAME
*       PRD_PRTCOMMAND -- send a command to the printer
*
*   FUNCTION
*      This function sends a command to either the parallel or serial
*      device.  The printer device maps this command to the control
*      code set of the current printer.  The commands supported can
*      be found with the printer.device/Write command.  All printers
*      may not support all functions.
*
*   IO REQUEST IOPrtCmdReq
*       io_Message      mn_ReplyPort set
*       io_Device       preset by OpenDevice
*       io_Unit         preset by OpenDevice
*       io_Command      PRD_PRTCOMMAND
*       io_PrtCommand   the actual command number
*       io_Parm0        parameter for the command
*       io_Parm1        parameter for the command
*       io_Parm2        parameter for the command
*       io_Parm3        parameter for the command
*
*   RESULTS
*       Errors: if the PRD_PRTCOMMAND succeeded, then io_Error will be zero.
*       Otherwise io_Error will be non-zero.  An error of -1 indicates that
*       the command is not supported by the current printer driver.  This
*       could be used to check if the connected printer supports a particular
*       command (italics for example).
*
*   SEE ALSO
*       printer.device/Write printer.h, parallel.device, Preferences
*
************************************************************************/


BYTE PCPrtCommand(ior)
struct IOPrtCmdReq *ior;
{
	int length, status, command;
	char output[P_BUFSIZE + 1];
	UBYTE Params[4];
	char *ptrstart;
	char EnPWait;
	char *ptr;

	Params[0] = ior->io_Parm0;
	Params[1] = ior->io_Parm1;
	Params[2] = ior->io_Parm2;
	Params[3] = ior->io_Parm3;
	command = ior->io_PrtCommand;

#if DEBUGPCP
	kprintf("PCPC: cmd=%ld, P0=%ld, P1=%ld, P2=%ld, P3=%ld\n",
		command, Params[0], Params[1], Params[2], Params[3]);
#endif /* DEBUGPCP */
	if (command == aRAW) {
		RawCount = Params[0];
#if DEBUGPCP
		kprintf("PCPC: aRAW, RawCount=%ld\n", RawCount);
#endif /* DEBUGPCP */
		return(0);
	}
	length = translateCommand(ior->io_PrtCommand, output, Params);
#if DEBUGPCP
	kprintf("PCPC: translateCommand returned %ld\n", length);
#endif /* DEBUGPCP */
	if (length < 1) {
		return((BYTE)length);
	}
	status = 0;
	if (command == aRIS) { /* reset */
		status = PWait(1, 0);
	}
	if (status == 0) {
		EnPWait = PD->pd_PWaitEnabled;
#if DEBUGPCP
		kprintf("PCP: EnPWait=%ld\n", EnPWait);
#endif /* DEBUGPCP */
		if (EnPWait == 0) {
			status = PWrite(output, length);
			if (status == 0) {
				status=PBothReady(); /* wait until buffer is empty */
			}
		}
		else {
			ptr = output;
			do {
				ptrstart = ptr; /* reset start to current position */
				/* go until encountered PWait char or at end of string */
				while ((*ptr != EnPWait) && length) {
					ptr++; /* advance to next char */
					length--; /* one less chat to check */
				}
				/* send partial string */
				status = PWrite(ptrstart, ptr - ptrstart);
				if (status == 0) {
					status=PBothReady(); /* wait until buffer is empty */
				}
				if (*ptr == EnPWait) { /* did we encounter the PWait char? */
#if DEBUGPCP
					kprintf("PCP: waiting...");
#endif /* DEBUGPCP */
					PWait(1, 0); /* yes, so wait */
#if DEBUGPCP
					kprintf("ok\n");
#endif /* DEBUGPCP */
					ptr++; /* skip over PWaitEnabled char */
					length--; /* one less char to check */
				}
			} while (length && (status == 0));
		}
	}
	/* if 'reset' and status ok */
	if ((command == aRIS) && (status == 0)) {
		status = PWait(1, 0);
	}
	return((BYTE)status);
}

translateCommand(point, outputBuffer, Parms)
char outputBuffer[];
UWORD point;
UBYTE Parms[];
{
	char inchar;
	int x;
	int status;
	char *CommandPoint;

	x = 0;
	/* do special stuff */
	status = (*(PED->ped_DoSpecial))(&point, outputBuffer, &vline, &currentVMI, &crlfFlag, Parms);
#if DEBUGTC
	kprintf("TC: DoSpecial returned %ld\n", status);
#endif /* DEBUGTC */
	if (status != 0) {
#if DEBUGTC
		kprintf("TC: returning 0\n", x);
#endif /* DEBUGTC */
		return(status);
	}
	CommandPoint = (char *)((PED->ped_Commands)[point]); /* get ptr to cmd string */
	/* for all chars in the cmd */
	while ((inchar = CommandPoint[x]) != 0 ) {
		if (inchar == -1) { /* '\377' */
			return(-1);	/* printer doesnt have this one */
		}
		if (inchar == -2) { /* '\376' */
			inchar= '\000'; /* kludge for 0 */
		}
		outputBuffer[x++] = inchar; /* copy char to output buffer */
	}
	/* if 'set proportional offset', 'set form length' or 'perf skip n' */
	if ((point == aTSS) || (point == aSLPP) || (point == aPERF)) {
		outputBuffer[x++] = Parms[0]; /* insert single parameter */
	}
#if DEBUGTC
	kprintf("TC: returning %ld\n", x);
#endif /* DEBUGTC */
	return(x);
}
