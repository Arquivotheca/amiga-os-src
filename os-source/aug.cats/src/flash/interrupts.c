#include "flash.h"

#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

#define STOPA_AND CIACRAF_TODIN|CIACRAF_PBON|CIACRAF_OUTMODE|CIACRAF_SPMODE

	/* 
        ;
	; AND mask for use with control register A
	; (interval timer A on either CIA)
	;
	; STOP -
	;	START bit 0 == 0 (STOP IMMEDIATELY)
	;	PBON  bit 1 == same
	;	OUT   bit 2 == same
	;       RUN   bit 3 == 0 (SET CONTINUOUS MODE)
	;       LOAD  bit 4 == 0 (NO FORCE LOAD)
	;       IN    bit 5 == 0 (COUNTS 02 PULSES)
	;       SP    bit 6 == same
	;       TODIN bit 7 == same (unused on ciacra)

	*/

#define STOPB_AND CIACRBF_ALARM|CIACRBF_PBON|CIACRBF_OUTMODE

	/* 
        ;
	; AND mask for use with control register B
	; (interval timer B on either CIA)
	;
	; STOP -
	;	START bit 0 == 0 (STOP IMMEDIATELY)
	;	PBON  bit 1 == same
	;	OUT   bit 2 == same
	;       RUN   bit 3 == 0 (SET CONTINUOUS MODE)
	;       LOAD  bit 4 == 0 (NO FORCE LOAD)
	;       IN0   bit 5 == 0 (COUNTS 02 PULSES)
	;       IN1   bit 6 == 0 (COUNTS 02 PULSES)
	;       ALARM bit 7 == same (TOD alarm control bit)

	*/

UBYTE __asm StatusInt(register __a1 struct cmdVars *cv, register __d0 UBYTE status )
{
	if(status & CARD_STATUSF_WR)
	{
		Signal(cv->cv_Task,1L<<cv->cv_Signal);
	}
	return(status);
}	

void __asm RemovedInt(register __a1 struct cmdVars *cv )
{
	cv->cv_IsRemoved = TRUE;
	Signal(cv->cv_Task,1L<<cv->cv_Signal);
}

void __asm InsertInt(register __a1 struct cmdVars *cv )
{
	cv->cv_IsInserted = TRUE;
	Signal(cv->cv_Task,1L<<cv->cv_Signal);

}

void __asm TimerInt(register __a1 struct cmdVars *cv )
{
	AbleICR(CIABBase,1L<<cv->cv_ciatimerbit);
}

LONG AllocateCIA( struct cmdVars *cv )
{
struct CIA *cia;

	cv->cv_ciabint.is_Data = cv;
	cv->cv_ciabint.is_Code = (APTR)&TimerInt;

	cia = (struct CIA *)0xbfd000;

	if(!(AddICRVector(CIABBase,CIAICRB_TA,&cv->cv_ciabint)))
	{
		cv->cv_ciatimerbit = CIAICRB_TA;
		cv->cv_ZoneHandle.zh_timer = (APTR)0xbfd400;

	/* set count down mode; one shot mode */

		Disable();
		cia->ciacra &= STOPA_AND;
		cia->ciacra |= CIACRAF_RUNMODE;
		Enable();

		return(TRUE);
	}

	if(!(AddICRVector(CIABBase,CIAICRB_TB,&cv->cv_ciabint)))
	{
		cv->cv_ciatimerbit = CIAICRB_TB;
		cv->cv_ZoneHandle.zh_timer = (APTR)0xbfd600;

	/* set count down mode; one shot mode */

		Disable();
		cia->ciacrb &= STOPB_AND;
		cia->ciacrb |= CIACRBF_RUNMODE;
		Enable();

		return(TRUE);
	}
	return(FALSE);
}

