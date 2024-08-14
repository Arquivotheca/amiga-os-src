/*
** Emulation of Unix kernel sleep/wakeup/splimp/splnet/splx.  This module
** should be reworked to use Semaphores.
*/

#include <exec/types.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <dos/dos.h>

#include "proto.h"

#ifdef LATTICE
#include <proto/exec.h>
#endif

#ifdef AZTEC_C
#include <functions.h>
#endif

static struct wn {
	struct Node wn_node;
	unsigned long wn_chan;
	struct Task *wn_task;
	int wn_sigbit;
};

#define MAXSEM	32
#define HASH(chan) (((chan >> 16) ^ (chan >> 8) ^ chan) % MAXSEM)

static struct List wntbl[MAXSEM];
static int inited, priority;

extern struct ExecBase *SysBase;


/*
** sleep() causes the calling process to block until a wakeup occurs on
** the specified channel.  Just prior to going to sleep, the processor
** priority is returned to SPLZERO.  When the process wakens, the priority
** is returned to the original value and control is then returned.
*/

int sleep(chan, pri)
	register unsigned long chan;
	int pri;
{
	register int entry;
	struct wn wn;
//	int old;
	ULONG event, mask;

	if(!inited){
		for(entry = 0; entry < MAXSEM; entry++){
			InitList(&wntbl[entry]);
		}
		inited++;
	}

	bzero(&wn, sizeof(wn));
	wn.wn_node.ln_Pri = pri;
	wn.wn_task = FindTask(0L);
	wn.wn_chan = chan;
	wn.wn_sigbit = AllocSignal(-1L);
	if(wn.wn_sigbit == -1){
		return (-1);
	}
	mask = 1L << wn.wn_sigbit | SIGBREAKF_CTRL_C;

	Disable(); {
		Enqueue(&wntbl[HASH(chan)], (struct Node *)&wn); 
//		old = splx(-1);
	} Enable();

	/* we can lose control before we get to the Wait() */
	event = Wait(mask);
		
	Disable(); {
//		splx(old);
		Remove((struct Node *)&wn); 
	} Enable();

	FreeSignal(wn.wn_sigbit);

	if(event & SIGBREAKF_CTRL_C)
		return (1);
	return (0);
}

void wakeup(chan)
	register unsigned long chan;
{
	register struct wn *np;
	register int entry;

	if(!inited){
		for(entry = 0; entry < MAXSEM; entry++){
			InitList(&wntbl[entry]);
		}
		inited++;
	}
	
	Forbid();
	for(np = (struct wn *)wntbl[HASH(chan)].lh_Head; 
	    np->wn_node.ln_Succ; np = (struct wn *)np->wn_node.ln_Succ){
		if(np->wn_chan == chan){
			Signal(np->wn_task, 1L<<np->wn_sigbit);
		}
	}
	Permit();
}

#define NULLNODE (struct Node *)NULL

void InitList(l)
	register struct List *l;
{
	l->lh_Head = (struct Node *)&l->lh_Tail;
	l->lh_TailPred = (struct Node *)&l->lh_Head;
	l->lh_Tail = NULLNODE;
	l->lh_Type = NT_MESSAGE;
}

/********************************
splnet()
{
	struct ExecBase *ex;
	int old;

	ex = (struct ExecBase *) *(long *)4L;
	old = ex->TDNestCnt;
	Forbid();
	return old;	
}
********************************/

int splnet()
{
	int old;

	old = SysBase->TDNestCnt;
	Forbid();
	return old;	
}

/****************
*int splimp()
*{
*	return splnet();
*}
****************/


/*******************************
splx(s)
{
	struct ExecBase *ex;
	int old;

	ex = (struct ExecBase *) *(long *)4L;
	old = ex->TDNestCnt;

	if(s >= ex->TDNestCnt){
		ex->TDNestCnt = s;
	} else {
		while(s < ex->TDNestCnt){
			Permit();
		}
	}

	return old;
}
*******************************/

int splx(int s)
{
	int old;

	old = SysBase->TDNestCnt;

	if(s >= SysBase->TDNestCnt){
		SysBase->TDNestCnt = s;
	} else {
		while(s < SysBase->TDNestCnt){
			Permit();
		}
	}

	return old;
}
