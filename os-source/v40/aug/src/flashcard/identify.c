#include "flash.h"

#define DEBUG 1
#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

#define IDENTIFY	0x90
#define INTEL_ID	0x89
#define READMODE	0x00

/**
 ** Check that this is a known flash ID
 **/

int LookUpFLASH( struct cmdVars *cv )
{

UBYTE identifier,hold;

#ifdef	IFCACHECONTROL
ULONG oldcachebits;
#endif

UBYTE *volatile getidentifier;

	getidentifier = cv->cv_CardMemMap->cmm_CommonMemory;

	if(CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_12V))
	{
		Stabilize1MS(&cv->cv_ZoneHandle);

#ifdef	IFCACHECONTROL

		Forbid();

		oldcachebits = (CacheControl(0L,CACRF_EnableD) & CACRF_EnableD);

#endif
/* write identifier request, and read back */

		hold = *getidentifier;

		*getidentifier = IDENTIFY;

		identifier = *getidentifier;

		*getidentifier = READMODE;	/* new command - terminate ID */

#ifdef	IFCACHECONTROL
		CacheControl(1L,oldcachebits);

		Permit();
#endif
		CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_0V);

		Stabilize1MS(&cv->cv_ZoneHandle);


/* restore?? just in case this is an SRAM card */

		if(identifier == IDENTIFY)
		{
			*getidentifier = hold;
#ifdef	IFCACHECONTROL

		CacheClearU();
#endif

		}

	}
	else
	{
		return(0);
	}

	if(identifier == INTEL_ID) return((int)identifier);
	return(0);

}

