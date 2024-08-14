#ifndef EVENTLOOP_H
#define EVENTLOOP_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


VOID EventLoop(struct MPEGPlayerLib *MPEGPlayerBase);

ULONG OrganizeSequences(struct MPEGPlayerLib *MPEGPlayerBase,
		 	struct CDDisk *disk, LONG currentTrack,
                        ULONG options);


/*****************************************************************************/


#endif /* EVENTLOOP_H */
