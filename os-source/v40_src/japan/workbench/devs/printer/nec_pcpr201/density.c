/*
 * density.c
 *
 */
#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

void SetDensity(ULONG density_code)
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	/* NEC PCPR201/60 supports only 160 dpi ... ;-( */

#if 0
	/* default is HD Pika 80 chars (1280 dots),
	   W_TRACTOR (B4) is HD Pika 136 chars (2176 dots) */
	PED->ped_MaxColumns =
		PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;

	PED->ped_MaxXDots =
		PD->pd_Preferences.PaperSize == W_TRACTOR ? 2176 : 1280;
#endif

	/* 2176/136 = 1280/80 = 16 dots for HD pika */
	PED->ped_MaxColumns = PD->pd_Preferences.PrintRightMargin - PD->pd_Preferences.PrintLeftMargin;
	PED->ped_MaxXDots = PED->ped_MaxColumns * 16;
}