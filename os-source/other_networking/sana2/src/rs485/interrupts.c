/*
 * interrupts - device interrupt handling.
 */

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * ONLINE state interrupt processing
 */
static void
ion_state(struct rs485Unit *up)
{
int bn, free;

	if(SR(up) & SR_TA)
	{
		for(bn = 0; bn < NBUF; bn++)
		{
			if(up->bstate[bn] == IS_CURR_XMIT)
			{
DPRINT(("Freed one.\n"));
				up->bstate[bn] = IS_FREE;
			}
		}

		for(bn = 0; bn < NBUF; bn++)
		{
			if(up->bstate[bn] == HAS_XMIT_DATA)
			{
				up->bstate[bn] = IS_CURR_XMIT;
				CMD(up) = C_SEND(bn);
DPRINT(("Queued one.\n"));
				break;
			}
		}

		if(bn == NBUF){
			up->intmask &= ~SR_TA;
		}
	}

	if(SR(up) & SR_RI)
	{
DPRINT(("ion_state(): SR(up) & SR_RI\n"));
		free = -1;
		for(bn = 0; bn < NBUF; bn++){
			if(up->bstate[bn] == IS_CURR_RCVR){
				up->bstate[bn] = HAS_RCV_DATA;
			}

			if(up->bstate[bn] == IS_FREE){
				free = bn;
			}
		}

		if(free >= 0){
			up->bstate[free] = IS_CURR_RCVR;
			CMD(up) = C_RECV(free);
		} else {
			up->intmask &= ~SR_RI;
		}
	}

	if(SR(up) & SR_RECON)
	{
		up->stats.Reconfigurations++;
		CMD(up) = C_CLR_RECON;
	}

	SETINTMASK(up, up->intmask);
	sigtask(up);
}

/*
 * note: interrupt server does not obey unit semaphore; thus it cannot touch or
 *	 rely on the state of any queue variable, addr/data regs of com20020 chip,
 *	 etc.  The interrupt driver will touch the board ONLY if the unit is in
 *	 the ONLINE state.
 */
VOID __stdargs __saveds __asm
interruptC(register __a1 struct rs485Unit *up)
{
	switch(up->state){
	case ONLINE:
		if(SR(up) & up->intmask)
		{
			ion_state(up);
		}
		break;

	case INITIAL:
	case OFFLINE:
		break;

	}
}

/*
 * add our interrupt routine to INTB_PORTS chain.  is_Data points to unit; this
 * will be used to load a6 when the interrupt chain is called.
 */
interrupts_on(struct rs485Unit *up)
{
	if(up->intr == 0){
		up->intr = (struct Interrupt *)AllocMem(sizeof(*up->intr), MEMF_PUBLIC | MEMF_CLEAR);
		if(up->intr == 0){
			return S2ERR_NO_RESOURCES;
		}

		up->intr->is_Code = interrupt;
		up->intr->is_Data = up;
		up->intr->is_Node.ln_Type = NT_INTERRUPT;
		up->intr->is_Node.ln_Pri = 127;
		up->intr->is_Node.ln_Name = 0;
		AddIntServer(INTB_PORTS, up->intr);
	}
	return S2ERR_NO_ERROR;
}

/*
 * pull interupt server off chain, free int structure
 */
void
interrupts_off(struct rs485Unit *up)
{
	if(up->intr){
		RemIntServer(INTB_PORTS, up->intr);
		FreeMem(up->intr, sizeof(*up->intr));
		up->intr = 0;
	}
}
