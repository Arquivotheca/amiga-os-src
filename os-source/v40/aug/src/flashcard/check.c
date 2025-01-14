#include "flash.h"

extern ULONG totalsizes[];
extern ULONG zonesizes[];

void DoSprintF( UBYTE *buf, STRPTR fmt, long arg1, ... )
{
	
	SPrintF(buf,fmt,&arg1);

}

void DoCheck( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register ULONG total,zone,temp;
UBYTE info[80];

	cv = cmv;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];
	temp = total / zone;

	if(PromptForInsert(cv,"CHECK BLANK STATE"))
	{
		ClearBox(cv,STAT_BOX_ID);
		
		cv->cv_ZoneHandle.zh_to = cv->cv_CardMemMap->cmm_CommonMemory;
		cv->cv_ZoneHandle.zh_size = zone;


#ifdef	IFCACHECONTROL
		CacheClearU();
#endif
		total = 0;
		while(total < temp)
		{
			DoSprintF(info,"Checking Zone: %3ld of %3ld",(total+1L),temp);

			StatusBox(cv,0,1,TRUE,STAT_BOX_ID,info);

			if(!(CheckBlank(&cv->cv_ZoneHandle)))
			{
				AbortOnOff(cv,TRUE);
				DisplayError(cv,"FlashROM is NOT Blank");
				return;
			}

			if(CheckAbort(cv))
			{
				return;
			}
			cv->cv_ZoneHandle.zh_to = (APTR)((ULONG)cv->cv_ZoneHandle.zh_to + zone);
			total++;
		}

		AbortOnOff(cv,TRUE);
		DisplayError(cv,"FlashROM is Blank");
	}
}


