#include <exec/types.h>
#include <exec/execbase.h>
#include <devices/parallel.h>
#include <devices/serial.h>
#include <hardware/cia.h>
#include "../printer/prtbase.h"

#define PRTFLAGS	(CIAF_PRTRSEL | CIAF_PRTRPOUT | CIAF_PRTRBUSY)

#define DEBUG	0

/*
	This routine is a special kludge required for this printer
	due to a problem with communications hand shaking between multiple
	dumps.  This code checks to make sure that the communication port is
	SELECTED, NOT OUT OF PAPER, and NOT BUSY before returning control.
*/
PrinterReady()
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;
	extern struct ExecBase *SysBase;

	union {
		struct IOExtPar par;
		struct IOExtSer ser;
	} ioreq;
	LONG timeout;
	UWORD status;

	timeout = PED->ped_TimeoutSecs * SysBase->VBlankFrequency;
#if DEBUG
	kprintf("timeout=%ld", timeout);
#endif DEBUG
	if (PD->pd_Preferences.PrinterPort == PARALLEL_PRINTER) {
		ioreq.par = PD->pd_ior0.pd_p0; /* copy io request */
		ioreq.par.IOPar.io_Command = PDCMD_QUERY;
		do { /* wait until timeout or printer ready */
#if DEBUG
			kprintf("+");
#endif DEBUG
			DoIO(&ioreq); /* check parallel printer status */
#if DEBUG
			kprintf("-");
#endif DEBUG
			status = ioreq.par.io_Status;
			if ((status & PRTFLAGS) != CIAF_PRTRSEL) {
#if DEBUG
					kprintf("(");
#endif DEBUG
					WaitTOF();
#if DEBUG
					kprintf(") ");
#endif DEBUG
			}
			else {
				break;
			}
		} while (--timeout);
	}
	else { /* SERIAL_PRINTER */
		ioreq.ser = PD->pd_ior0.pd_s0; /* copy io request */
		ioreq.ser.IOSer.io_Command = SDCMD_QUERY;
		do { /* wait until timeout or printer ready */
#if DEBUG
			kprintf("+");
#endif DEBUG
			DoIO(&ioreq);
#if DEBUG
			kprintf("-");
#endif DEBUG
			status = ioreq.ser.io_Status;
			if (status & CIAF_COMCTS) {
#if DEBUG
					kprintf("(");
#endif DEBUG
					WaitTOF();
#if DEBUG
					kprintf(") ");
#endif DEBUG
			}
			else {
				break;
			}
		} while (--timeout);
	}
#if DEBUG
	if ((status & PRTFLAGS) != CIAF_PRTRSEL) {
		kprintf("end: busy\n");
	}
	else {
		kprintf("end: ready\n");
	}
#endif DEBUG
}
