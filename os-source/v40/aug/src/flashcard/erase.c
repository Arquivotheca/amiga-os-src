#include "flash.h"

extern ULONG totalsizes[];
extern ULONG zonesizes[];

void DoErase( struct cmdVars *cmv )
{

register struct cmdVars *cv;
register ULONG total,zone,temp;
#ifdef IFCACHECONTROL
ULONG oldcachebits;
#endif
UBYTE info[80];

	cv = cmv;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];
	temp = total / zone;

	if(PromptForInsert(cv,"ERASE CONTENTS"))
	{
		ClearBox(cv,STAT_BOX_ID);
		
		if(cv->cv_ZoneHandle.zh_from = AllocMem(zone,MEMF_PUBLIC|MEMF_CLEAR))
		{
			cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;
			cv->cv_ZoneHandle.zh_size = zone;


#ifdef	IFCACHECONTROL
	/* sorry about this, but I need the data cache turned off, and do not
	  intend to protect in Forbid() -- too long
	*/

			oldcachebits = (CacheControl(0L,CACRF_EnableD) & CACRF_EnableD);
#endif

			if(CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_12V))
			{
				Stabilize1MS(&cv->cv_ZoneHandle);

				total = 0;
				while(total < temp)
				{
					DoSprintF(info,"Erase Pass: 1  Zone: %3ld of %3ld",(total+1L),temp);

					StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

					if(WriteZone(&cv->cv_ZoneHandle))
					{
						DoSprintF(info,"Erase Pass: 2  Zone: %3ld of %3ld",(total+1L),temp);

						StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

						if(!(EraseZone(&cv->cv_ZoneHandle)))
						{
							DisplayError(cv,"ERROR: Erase operation failed");
							total = temp;
						}
					}
					else
					{
						DisplayError(cv,"ERROR: Erase operation failed");
						total = temp;
					}
					cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + zone);
					total++;
				}



				CardProgramVoltage(&cv->cv_CardHandle,CARD_VOLTAGE_0V);
				Stabilize1MS(&cv->cv_ZoneHandle);
			}
			else
			{
				DisplayError(cv,"ERROR: Unable to access card");
			}

#ifdef	IFCACHECONTROL
			CacheControl(1L,oldcachebits);
#endif

			FreeMem(cv->cv_ZoneHandle.zh_from,zone);
		}
		else
		{
			DisplayError(cv,"ERROR: Insufficient Memory");
		}
	}

}