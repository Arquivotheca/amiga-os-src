/*
**	$Id$
**
**      printer support for prtface
**
**      (C) Copyright 1991 Robert R. Burns
**          All Rights Reserved
*/
#ifdef   DEBUG
#define  D(a)	printf a
#else
#define  D(a)
#endif

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <devices/prtbase.h>
#include <devices/printer.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

extern struct Library *SysBase;
extern struct Library *GfxBase;

struct MsgPort PrinterPort;
struct IODRPReq PrinterIO;
struct RastPort PrinterRP;
struct BitMap PrinterBM;
struct ColorFontColors PrinterCM;
UWORD ColorTable[2] = {
    0x0fff, 0x0000
};
WORD MaxX, MaxY, DPIX, DPIY;

void
ClosePrinter()
{
    if (PrinterBM.Planes[0]) {
	D(("FreeRaster(,%ld, %ld)\n", MaxX, MaxY));
	FreeRaster(PrinterBM.Planes[0], MaxX, MaxY);
    }
    if (PrinterIO.io_Device) {
	D(("CloseDevice printer.device\n"));
	CloseDevice(&PrinterIO);
    }
    D(("ClosePrinter done.\n"));
}

int
OpenPrinter()
{
    struct PrinterExtendedData *ped;
    WORD d;				/* printer density */
    int result;

    /* initialize PrinterPort */
    PrinterPort.mp_Flags = PA_SIGNAL;
    PrinterPort.mp_SigBit = SIGB_SINGLE;
    PrinterPort.mp_SigTask = FindTask(0);
    NewList(&PrinterPort.mp_MsgList);

    /* initialize PrinterRP (that can be done w/o knowing size) */
    InitBitMap(&PrinterBM, 1, 1000, 1000);
    PrinterBM.Planes[0] = 0;
    InitRastPort(&PrinterRP);
    PrinterRP.BitMap = &PrinterBM;

    PrinterCM.cfc_Reserved = 0;
    PrinterCM.cfc_Count = 2;
    PrinterCM.cfc_ColorTable = ColorTable;

    /* initialize PrinterIO */
    PrinterIO.io_Message.mn_ReplyPort = &PrinterPort;
    if (result = OpenDevice("printer.device", 0, &PrinterIO, 0)) {
	D(("OpenDevice of printer.device failed %ld\n", result));
	PrinterIO.io_Device = 0;
	return (result);
    }

    /* get printer dimensions */
    PrinterIO.io_Command = PRD_DUMPRPORT;
    PrinterIO.io_RastPort = &PrinterRP;
    PrinterIO.io_ColorMap = (struct ColorMap *) & PrinterCM;
    PrinterIO.io_Modes = 0;
    PrinterIO.io_SrcX = PrinterIO.io_SrcY = 0;
    PrinterIO.io_SrcWidth = PrinterIO.io_SrcHeight = 1000;
    PrinterIO.io_DestCols = PrinterIO.io_DestRows = 0;
    for (d = SPECIAL_DENSITY7; d >= 0; d -= SPECIAL_DENSITY1) {
	PrinterIO.io_Special = SPECIAL_NOPRINT | d;
	if (result = DoIO(&PrinterIO)) {
	    D(("DoIO NOPRINT DENSITY %ld failed %ld\n", d >> 8, result));
	}
	else
	    goto gotDimensions;
    }
    /* failed all densities */
    D(("failed all densities\n"));
    return (-1);

gotDimensions:
    PrinterIO.io_Special &= ~SPECIAL_NOPRINT;
#if 0
    PrinterIO.io_Special |= SPECIAL_TRUSTME;
#endif
    D(("density %ld dimensions %ld x %ld\n", d >> 8,
		    PrinterIO.io_DestCols, PrinterIO.io_DestRows));
    ped = &((struct PrinterData *) PrinterIO.io_Device)->pd_SegmentData->ps_PED;
    DPIX = ped->ped_XDotsInch;
    DPIY = ped->ped_XDotsInch;
    D(("max dots %ld %ld, DPI %ld %ld\n", ped->ped_MaxXDots, ped->ped_MaxYDots,
		    DPIX, DPIY));
#if 0
    PrinterIO.io_SrcWidth = MaxX = ped->ped_MaxXDots;
    PrinterIO.io_SrcHeight = MaxY = ped->ped_MaxYDots;
#else
    PrinterIO.io_SrcWidth = MaxX = PrinterIO.io_DestCols;
    PrinterIO.io_SrcHeight = MaxY = PrinterIO.io_DestRows;
#endif
    InitBitMap(&PrinterBM, 1, MaxX, MaxY);
    PrinterBM.Planes[0] = AllocRaster(MaxX, MaxY);
    if (!PrinterBM.Planes[0]) {
	D(("AllocRaster failure\n"));
	return (-1);
    }
    D(("OpenPrinter done.\n"));
    return (0);
}

int
Print()
{
    int result;

    D(("Print...\n"));
    if (result = DoIO(&PrinterIO)) {
	D(("Print error %ld\n", PrinterIO.io_Error));
	return (result);
    }
    CloseDevice(&PrinterIO);
    if (result = OpenDevice("printer.device", 0, &PrinterIO, 0)) {
	D(("OpenDevice of printer.device failed %ld\n", result));
	PrinterIO.io_Device = 0;
	return (result);
    }
}
