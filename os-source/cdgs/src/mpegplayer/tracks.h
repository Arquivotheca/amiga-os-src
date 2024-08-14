#ifndef TRACKS_H
#define TRACKS_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


enum Movement
{
    MV_NOP,
    MV_UP,
    MV_DOWN,
    MV_LEFT,
    MV_RIGHT
};


VOID CreateTracks(struct MPEGPlayerLib *MPEGPlayerBase, struct CDDisk *disk);
VOID DeleteTracks(struct MPEGPlayerLib *MPEGPlayerBase);
VOID TrackState(struct MPEGPlayerLib *MPEGPlayerBase, ULONG track, BOOL selected);
ULONG HighlightTrack(struct MPEGPlayerLib *MPEGPlayerBase, ULONG currentTrack, enum Movement direction);


/*****************************************************************************/


#endif /* TRACKS_H */
