
/* Subs.c - Basic network handler support routines */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1987 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the author.					   BBS:      */
/* | o	| ||   John Toebes				     (919)-471-6436  */
/* |  . |//								     */
/* ======								     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "netcomm.h"
#include "proto.h"

/*
 * misc.c  - support routines - Phillip Lindsay (C) Commodore 1986
 *  You may freely distribute this source and use it for Amiga Development -
 *  as long as the Copyright notice is left intact.
 *
 * 30-SEP-86
 */

/*
 * returnpkt() - packet support routine
 * here is the guy who sends the packet back to the sender...
 */

void
retpkt(global, packet)
NGLOBAL global;
struct DosPacket *packet;
{
    struct Message *mess;
    struct MsgPort *replyport;

    BUG(("retpkt: Enter, replyport %08lx mess %08lx\n", 
       packet->dp_Port, packet->dp_Link));

    replyport	     = packet->dp_Port;
    mess	     = packet->dp_Link;
    packet->dp_Port  = global->n.port;

    PutMsg(replyport,mess);

    BUG(("retpkt: Exit\n"))
}

/*
 * taskwait() ... Waits for a message to arrive at your port and
 *   extracts the packet address which is returned to you.
 */

struct DosPacket *
taskwait(global)
NGLOBAL global;
{
    struct Message *mymess;

    WaitPort(global->n.port);   /* wait for packet */
    mymess = (struct Message *)GetMsg(global->n.port);

    global->pkt = (struct DosPacket *) mymess->mn_Node.ln_Name;

    return(global->pkt);
}

char *
DosAllocMem(global, len)
NGLOBAL global;
long len;
{
    long *p;

    if ((p = (long *)AllocMem(len+4, MEMF_PUBLIC | MEMF_CLEAR)) == NULL) {
	if (global->pkt != NULL) {
	    global->pkt->dp_Res1 = DOS_FALSE;
	    global->pkt->dp_Res2 = ERROR_NO_FREE_STORE;
	} else {
	    /* Gee.  Out of memory AND there is nobody to tell about it ...  */
	    /* Only choice is to GURU.	Maybe we could do something clever   */
	    /* but I doubt it...					     */
	    BUG(("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!               !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!! OUT OF MEMORY !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!               !!!!!!!!\n"));
	    BUG(("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
	}
    } else {
	*p++ = len;
    }
    return((char *)p);
}

void
DosFreeMem(p)
char *p;
{
    long *lp;
    long len;

    lp = (long *)p;
    len = *--lp;
    FreeMem((char *)lp, len+4);
}

LONG
checksum(c, len)
char *c;
int len;
{
    int i, l;
    LONG sum, *lptr;
    unsigned char *uc;

    l = len/sizeof(LONG);
    lptr = (LONG *)c;
    for (i = 0, sum = 0; i < l; i++, lptr++)
	sum += *lptr;

    l = len % sizeof(LONG);
    uc = (unsigned char *)lptr;
    for (i = 0; i < l; i++, uc++)
	sum += *uc;

    return(sum);
}


