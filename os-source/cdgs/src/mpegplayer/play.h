#ifndef PLAY_H
#define PLAY_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


ULONG DoPlay(struct MPEGPlayerLib *MPEGPlayerBase, struct CDDisk *disk,
             ULONG currentSelected, ULONG options);


/*****************************************************************************/


#endif /* PLAY_H */
