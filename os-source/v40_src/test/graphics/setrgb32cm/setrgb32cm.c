/*
  	setrgb32cm - program to test SetRGB32CM() function from graphics.library

	This program allocates a 256 color ColorMap and then attempts to write
	to element 256.  According to CI 53 for the Workbench 3.01 release, this
	should not cause an enforcer hit.

*/
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stddef.h>

#define NUMCOLORS 256

struct ColorMap *cm = NULL;

void QUIT(void);
void KPrintF(STRPTR,...);

void QUIT()
{
	if (cm) {
		FreeColorMap(cm);
	}
}

void main()
{
	int i = 0;

	if (! (cm = GetColorMap(NUMCOLORS))) {
		KPrintF("Couldn't allocate colormap!\n");
		printf("Couldn't allocate colormap!\n");
	}
	else {
		KPrintF("Color map allocated with %ld elements\n",(long)NUMCOLORS);
		printf("Color map allocated with %ld elements\n",(long)NUMCOLORS);
	}
	KPrintF("Calling SetRGB32CM on the %ldth element\n",(long)NUMCOLORS);
	printf("Calling SetRGB32CM on the %ldth element\n",(long)NUMCOLORS);

	SetRGB32CM(cm,NUMCOLORS,0,0,0);

	KPrintF("Done calling SetRGB32CM.\n");
	printf("Done calling SetRGB32CM.\n");

	QUIT();
}