#include <exec/types.h>
#include <exec/io.h>
#include <devices/parallel.h>
#include <devices/serial.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "prtbase.h"

#include "printer_iprotos.h"

#define DEBUGQ	0

PCQuery(pio) /* printer device query command */
struct IOStdReq *pio;
{
	extern struct PrinterData *PD;

	union {
		struct IOExtPar par;
		struct IOExtSer ser;
	} ioreq;
	UBYTE *ptr;

#if DEBUGQ
	kprintf("Q: enter, ");
#endif
	ptr = (UBYTE *)pio->io_Data;

	if (PD->pd_Preferences.PrinterPort == SERIAL_PRINTER) { /* serial */
#if DEBUGQ
		kprintf("calling serial query, ");
#endif
		ioreq.ser = PD->pd_ior0.pd_s0; /* copy io request */
		ioreq.ser.IOSer.io_Command = SDCMD_QUERY;
		DoIO((struct IOStdReq *)&ioreq);
		*ptr++ = (UBYTE)(ioreq.ser.io_Status & 0x00ff);
		*ptr = (UBYTE)((ioreq.ser.io_Status & 0xff00) >> 8);
		pio->io_Actual = 2;
#if DEBUGQ
		kprintf("status=%ld, ", ioreq.ser.io_Status);
#endif
	}
	else { /* parallel (or netprint - D. Berez - Sept. 5/89) */
#if DEBUGQ
		kprintf("calling parallel query, ");
#endif
		ioreq.par = PD->pd_ior0.pd_p0; /* copy io request */
		ioreq.par.IOPar.io_Command = PDCMD_QUERY;
		DoIO((struct IOStdReq *)&ioreq);
		*ptr++ = ioreq.par.io_Status;
		*ptr = 0;
		pio->io_Actual = 1;
#if DEBUGQ
		kprintf("status=%ld, ", ioreq.par.io_Status);
#endif
	}
#if DEBUGQ
	kprintf("exit\n");
#endif
}
