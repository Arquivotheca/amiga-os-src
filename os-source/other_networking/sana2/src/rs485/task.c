/*
 * task.c - background task to manage queued requests, and packet arrivals.
 *	    each unit is assigned its own task, thus it is safe for the
 *	    task to block on events (eg for future dynamic address discovery mode)
 */

#ifndef RS485_H
#include "rs485.h"
#endif

/*
 * on_state: ONLINE state processing of board events.
 */
static void on_state(struct rs485Unit *up)
{
	int i;

	/* pull any received packets from interface */
	for(i = 0; i < NBUF; i++){
		if(up->bstate[i] == HAS_RCV_DATA){
			recv(up, i);
			up->bstate[i] = IS_FREE;
		}
	}

	/* transmit queued write packets */
	if(!ISEMPTY(&up->write_q)){
		while(startxmit(up) > 0){
		}
	}

	/* restart receiver if stopped */
	Disable();
	if(!(up->intmask & SR_RI)){
		for(i = 0; i < NBUF; i++){
			if(up->bstate[i] == IS_FREE)
			{
				up->intmask |= SR_RI;
				SETINTMASK(up, up->intmask);
				up->bstate[i] = IS_CURR_RCVR;
				CMD(up) = C_RECV(i);
				break;
			}
		}
	}
	Enable();
}

/*
 * listener - called from assembly language stub.  This task handles incoming
 * packet traffic, non-immediate I/O, timed conditions, etc.  listener can
 * touch the board/unit structure only after the unit is in the ONLINE state.
 * When task starts, it waits for CTRL_C to tell it to search unit list for
 * which unit it will handle.
 *
 * NOTE: this procedure is multi-threaded!  Each open unit has a task running
 * this code.
 */
void __stdargs __saveds
listener(void)
{
	struct rs485Unit *up;

	do {
		Wait(SIGBREAKF_CTRL_C);
		FORALL(&rs485Units, up, struct rs485Unit *){
			if(up->task == FindTask(0L)){
				break;
			}
		}
	} while(!up->node.ln_Succ);

	up->netbit = AllocSignal(-1L);
	do {
		Wait(1L << up->netbit);

		ObtainSemaphore(&up->sem);
		switch(up->state){
		case ONLINE:
			on_state(up);
			break;

		case OFFLINE:
		case INITIAL:
			break;
		}
		ReleaseSemaphore(&up->sem);
	} while(1);
}

/*
 * start listener process for given unit, unless already running.  After the
 * task it spawned, it waits for us to initialize up->task and then signal
 * it with a SIGBREAKF_CTRL_C.  Couldn't think of any cleaner way of telling
 * the task which unit it handles (initializing the stack of the task is not
 * clean, sorry).
 */
starttask(struct rs485Unit *up)
{
	if(!up->task){
		up->task = CreateTask((void *)0, 0, (APTR)taskent, 512);
		if(!up->task){
			return S2ERR_SOFTWARE;
		}
		Signal(up->task, SIGBREAKF_CTRL_C);
	}
	return S2ERR_NO_ERROR;
}

/*
 * stop listener task. Since the interrupt driver sigtask() routines uses a
 * non zero task variable as a flag to indicate listener is running, some
 * care has to be exercised in first setting task=0 with ints off, then
 * DeleteTasking() later.
 */
void
stoptask(struct rs485Unit *up)
{
	struct Task *oldtask;

	if(up->task){
		Disable();
		oldtask = up->task;
		up->task = 0;
		Enable();
		DeleteTask(oldtask);
	}
}

/*
 * signal task that some condition needs attention; if task is not running,
 * as indicated by task==0, then just return.
 */
void sigtask(struct rs485Unit *up)
{
	if(up->task){
		Signal(up->task, 1L << up->netbit);
	}
}
