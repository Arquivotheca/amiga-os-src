/* ============================================================================ *
                      large.c -- large code routines for drum machine
                                      By Talin.
                               ©1991 The Dreamers Guild
 * ============================================================================ */

#include "std_headers.h"

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

#include <intuition/classes.h>

#include <midi/camd.h>
#include <midi/mididefs.h>
#include <libraries/realtime.h>
#include <libraries/realtimebase.h>

#include <clib/camd_protos.h>

extern LONG				clocks_per_column,
						total_playback_column,
						local_playback_column,
						next_column_time;

extern WORD				clock_state;
extern struct Task		*main_task;

ULONG myHookFunc (struct pmTime *msg, struct PlayerInfo *pi);
#pragma	regcall( myHookFunc(a1,a2) )

	/* Hook function... */

ULONG myHookFunc (struct pmTime *msg, struct PlayerInfo *pi)
{	struct Conductor *cond = pi->pi_Source;

	switch (msg->pmt_Method) {
	case PM_TICK:
		if (msg->pmt_Time > next_column_time)
		{		/* I don't like this method... We need a real method... */

			total_playback_column++;
			next_column_time += clocks_per_column;
			Signal(main_task,SIGBREAKF_CTRL_F);
		}
		break;

    case PM_STATE:
    	clock_state = cond->cdt_State;
    	Signal(main_task,SIGBREAKF_CTRL_E);
    	break;

#if 0
    case PM_SHUTTLE:
    	current_time = msg->pmt_Time;
		if ( (current_time ^ last_time) & ~0x3f)
    	{	Signal(main_task,SIGBREAKF_CTRL_F);
    		last_time = current_time;
    	}
#endif
	}
	return 0L;
}
