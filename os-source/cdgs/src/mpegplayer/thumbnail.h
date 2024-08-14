#ifndef THUMBNAIL_H
#define THUMBNAIL_H


/*****************************************************************************/


#include <exec/types.h>
#include "diskinfo.h"
#include "mpegplayerbase.h"


/*****************************************************************************/


void StartThumbnail(struct MPEGPlayerLib *MPEGPlayerBase, struct CDSequence *track);
void StopThumbnail(struct MPEGPlayerLib *MPEGPlayerBase);


/*****************************************************************************/


#endif /* THUMBNAIL_H */
