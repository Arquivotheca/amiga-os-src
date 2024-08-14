/*
 * S2_tracktype()		- add new tracked type to driver
 * S2_untracktype()	- remove tracked type from driver
 * S2_gettypestats()	- get statistics from tracked type
 */

#ifndef RS485_H
#include "rs485.h"
#endif

#define REMOVE	 1
#define NOREMOVE 0

/*
 * free_tracked - optionally remove & free trackedtype structure.
 */
static void
free_tracked(int remove, struct TrackedType *tp)
{
	if(remove == REMOVE){
		Remove(&tp->node);
	}
	FreeMem((void *)tp, sizeof(*tp));
}

/*
 * new_tracktype() - allocat trackedtype structure and add it to specified
 *		     unit; return pointer to new struct.
 */
static struct TrackedType *
new_tracktype(struct rs485Unit *up, ULONG ptp)
{
struct TrackedType *tp;

	tp = (struct TrackedType *)AllocMem(sizeof(*tp), MEMF_PUBLIC | MEMF_CLEAR);
	if(tp)
	{
		tp->type = ptp;
		AddTail(&up->trackedtype, &tp->node);
	}
	return tp;
}

/*
 * free any trackers
 */
void
free_alltracked(struct rs485Unit *up)
{
	struct List *lp;
	struct TrackedType *p, *q;

	lp = &up->trackedtype;
	for(p = (struct TrackedType *)lp->lh_Head; p->node.ln_Succ; p = q)
	{
		q = (struct TrackedType *)p->node.ln_Succ;
		free_tracked(REMOVE, p);
	}
}

/*
 * search tracked type structure list for specified type; return pointer to
 * struct if found, NULL otherwise.
 */
struct TrackedType *
findtracked(struct rs485Unit *up, ULONG ptp)
{
struct TrackedType *tp;

	FORALL(&up->trackedtype, tp, struct TrackedType *)
	{
		if(tp->type == ptp)
			return tp;
	}
	return 0;
}

/*
 * command processing for tracked type commands: sanity check pointers as needed,
 * perform specific type stats command.
 */
int S2_tracked(struct IOSana2Req *iob)
{
struct TrackedType *tp;

	if(iob->ios2_Req.io_Command==S2_GETTYPESTATS && !iob->ios2_StatData)
	{
		iob->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
		iob->ios2_WireError = S2WERR_NULL_POINTER;
		return COMPLETE;
	}

/*
**  checktype no longer needed because of changes to spec.
**
**	if(!checktype(iob))
**	{
**		return COMPLETE;
**	}
**
*/
	tp = findtracked(UNIT(iob), iob->ios2_PacketType);

	switch(iob->ios2_Req.io_Command){
	case S2_GETTYPESTATS:
		if(!tp)
		{
			iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
			iob->ios2_WireError = S2WERR_NOT_TRACKED;
		} else
		{
			memcpy(iob->ios2_StatData, &tp->stats, sizeof(tp->stats));
		}
		break;

	case S2_TRACKTYPE:
		if(tp)
		{
			iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
			iob->ios2_WireError = S2WERR_ALREADY_TRACKED;
		} else
		{
			tp = new_tracktype(UNIT(iob), iob->ios2_PacketType);
			if(!tp)
			{
				iob->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
			}
		}
		break;

	case S2_UNTRACKTYPE:
		if(!tp)
		{
			iob->ios2_Req.io_Error = S2ERR_BAD_STATE;
			iob->ios2_WireError = S2WERR_NOT_TRACKED;
		} else
		{
			free_tracked(REMOVE, tp);
		}
		break;
	}

	return COMPLETE;
}

