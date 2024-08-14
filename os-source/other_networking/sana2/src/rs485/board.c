#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * create & initialize a new device on unit list; leave device in INITIAL state
 */
struct rs485Unit *
newunit(void)
{
struct rs485Unit *aup;

	aup = (struct rs485Unit *)AllocMem(sizeof(*aup), MEMF_CLEAR|MEMF_PUBLIC);
	if(aup == 0)
	{
		return NULL;
	}
	NewList(&aup->trackedtype);
	NewList(&aup->read_q);
	NewList(&aup->write_q);
	NewList(&aup->misc_q);
	InitSemaphore(&aup->sem);
	aup->state = INITIAL;
	AddTail(&rs485Units, &aup->node);
	return aup;
}

/*
 * free any storage allocated to unit.  At this point, all lists should be
 * empty, so just zero the struct and free storage
 */
void
freeunit(struct rs485Unit *up)
{
	Remove(&up->node);
	memset(up, (char)0, sizeof(*up));
	up->unitnum = -1;
	up->base = (UBYTE *)-1;
	FreeMem(up, sizeof(*up));
}


/*
 * getboards - search configuration list for Zorro-II rs485 boards.  Build
 *		unit list and return 0 if all went ok.  If no Zorro-II boards
 *		were found, then assume this code is running on an A500 and
 *		create an instance of the A500 rs485 board.
 */
int
getboards(void)
{
char *explib = EXPANSIONNAME;
struct rs485Unit *aup;
struct ConfigDev *cd;
int unit;

	if(!ISEMPTY(&rs485Units))
	{
		return S2ERR_NO_ERROR;
	}
	ExpansionBase = OpenLibrary(explib, 0L);
	if(!ExpansionBase)
	{
		return S2ERR_SOFTWARE;
	}
	cd = 0;
	unit = 0;
	do
	{
		cd = FindConfigDev(cd, CBM_PROD, RS485_PROD);
		if(cd)
		{
			aup = newunit();
			aup->base = (UBYTE *)cd->cd_BoardAddr;
			aup->unitnum = unit++;
		}
	} while (cd);
	CloseLibrary((void *)ExpansionBase);
	ExpansionBase = NULL;
	if(unit == 0)
	{
		aup = newunit();
		aup->base = (UBYTE *)A500_BASE;
		aup->unitnum = unit++;
	}
	return S2ERR_NO_ERROR;
}

/*
 * initialize board - basic setup of chip registers
 */
int
initboard(struct rs485Unit *up)
{
int i;

	if(up->state == INITIAL)
	{
		Disable();
		for(i = 0; i < NBUF; i++)
		{
			up->bstate[i] = IS_FREE;
		}

		CFG(up) = CFG_RESET;
		CFG(up) = 0;

		CFG(up) = CFG_ET1 | CFG_ET2 | CFG_BACKPLANE | SUB_SETUP;
		SUB_REG(up) = 0;

		CFG(up) = (CFG(up) & ~3) | SUB_NODE_ID;
		SUB_REG(up) = 0;

		CMD(up) = C_CLR_POR;
//		CMD(up) = C_CONFIG;
		SETINTMASK(up, 0);
		Enable();
	}
	return S2ERR_NO_ERROR;
}

/*
 * shutupboard - disable board, mark unit into INITIAL state.

 	OFFLINE calls this and it is broket because it zeros the address.
 */
void
shutupboard(struct rs485Unit *up)
{
	Disable();
	SETINTMASK(up, 0);
	SUB_REG(up) = 0;
	CMD(up) = C_DIS_RCVR;
	CMD(up) = C_DIS_XMIT;
	CFG(up) = CFG_RESET;
	up->state = INITIAL;
	Enable();
}