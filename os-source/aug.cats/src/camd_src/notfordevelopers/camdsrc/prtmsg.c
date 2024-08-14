
#include <exec/types.h>
#include <exec/execbase.h>	/* for FlushDevice() */
#include <exec/interrupts.h>
#include <hardware/cia.h>	/* for DTR bit */
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <midi/camddevices.h>
#include <resources/misc.h>
#include <string.h>

void PrintMsg(char *str)
{
#if 0
	BPTR	prt;

	if (prt = Open("PRT:", MODE_OLDFILE))
	{
		Write(prt,str,strlen(str));
		Close(prt);
	}
#endif
}
