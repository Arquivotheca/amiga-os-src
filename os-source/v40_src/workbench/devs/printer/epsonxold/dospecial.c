/*
	DoSpecial for EpsonXOld (copy of EpsonX) driver.
	David Berezowski - March/88.

	Modified to make less assumptions about how
	"Epson compatbile" the printer might be.
	by Bryce Nesbitt - Thu Jun  2 18:07:53 EDT 1988

*/

#include "exec/types.h"
#include "devices/printer.h"
#include "devices/prtbase.h"

DoSpecial(command, outputBuffer, vline, currentVMI, crlfFlag, Parms)
char outputBuffer[];
UWORD *command;
BYTE *vline;
BYTE *currentVMI;
BYTE *crlfFlag;
UBYTE Parms[];
{
/* Warning: the Gemini-10X uses \033P for vertical tabbing,
   sets default tabs to 10, and is offset by 1 character on
   all tabs */

extern struct PrinterData *PD;

int x = 0, y = 0;
	/*
		00-02	\033x\001	select nlq mode
		03-04	\0332		select 6 lpi
		05-05	\022		cancel condensed mode
		06-07	\0335		italics off
		08-10	\033-\000	turn underline off
		11-12	\033F		cancel emphasized mode
		13-15	\033W\000	turn off double width
		16-17	\033T		cancel subscript
		18-18	\015		carriage return
	*/
static char initThisPrinter[19] =
"\033x\001\0332\022\0335\033-\000\033F\033W\000\033T\015";

	if (*command == aRIN) {
		while (x < 19) {
			outputBuffer[x] = initThisPrinter[x];
			x++;
		}

		if (PD->pd_Preferences.PrintQuality == DRAFT) {
			outputBuffer[2] = 0;
		}

		*currentVMI = 12; /* assume 1/6 line spacing (12/72 => 1/6) */
		if (PD->pd_Preferences.PrintSpacing == EIGHT_LPI) {
			outputBuffer[4] = '0';
			*currentVMI = 9; /* 9/72 => 1/8 */
		}


		if (PD->pd_Preferences.PrintPitch == ELITE) {
			outputBuffer[5] = '\017'; /* Set condensed */
		}
		else if (PD->pd_Preferences.PrintPitch == FINE) {
			outputBuffer[5] = '\017'; /* Set condensed */
		}

		return(x);
	}

	if (*command == aPLU) {
		if (*vline == 0) {
			*vline = 1;
			*command = aSUS2;
			return(0);
		}
		if (*vline < 0) {
			*vline = 0;
			*command = aSUS3;
			return(0);
		}
		return(-1);
	}

	if (*command == aPLD) {
		if (*vline == 0) {
			*vline = -1;
			*command = aSUS4;
			return(0);
		}
		if (*vline > 0) {
			*vline = 0;
			*command = aSUS1;
			return(0);
		}
		return(-1);
	}

	if (*command == aSUS0) {
		*vline = 0;
	}
	if (*command == aSUS1) {
		*vline = 0;
	}
	if (*command == aSUS2) {
		*vline = 1;
	}
	if (*command == aSUS3) {
		*vline = 0;
	}
	if (*command == aSUS4) {
		*vline = -1;
	}

	if (*command == aVERP0) {
		*currentVMI = 9;
	}

	if (*command == aVERP1) {
		*currentVMI = 12;
	}

	if (*command == aRIS) {
		PD->pd_PWaitEnabled = 253;
	}

	return(0);
}
