#ifndef DISKINFO_H
#define DISKINFO_H


/*****************************************************************************/


#include <exec/types.h>
#include "mpegplayerbase.h"


/*****************************************************************************/


struct CDDisk
{
    STRPTR         cdd_Name;
    STRPTR         cdd_Notes;
    ULONG          cdd_NotesLen;
    APTR           cdd_VideoCDHandle;
    APTR           cdd_VideoCDInfo;
    ULONG          cdd_VolumeNumber;
    ULONG          cdd_NumVolumes;
    ULONG          cdd_NumTracks;
    struct MinList cdd_Sequences;
    BOOL           cdd_OnlyAudio;
};

struct CDSequence
{
    struct Node cds_Link;
    BOOL        cds_Selected;
    ULONG       cds_TrackNumber;
    ULONG       cds_StartSector;
    ULONG       cds_EndSector;

    ULONG       cds_TrackStart;
    ULONG       cds_TrackEnd;

    APTR        cds_VideoCDInfo;
    STRPTR      cds_Lyrics;
    ULONG       cds_LyricsLen;
    STRPTR      cds_Notes;
    ULONG       cds_NotesLen;
    STRPTR      cds_Performer;
    STRPTR      cds_Writer;
    STRPTR      cds_Composer;
    STRPTR      cds_Arranger;
    STRPTR      cds_Player;
    STRPTR      cds_Producer;
    STRPTR      cds_Director;
};

#define cds_Name       cds_Link.ln_Name    /* STRPTR */
#define cds_AudioTrack cds_Link.ln_Pri     /* BOOL   */
#define cds_Track      cds_Link.ln_Type    /* BOOL   */

/* Sequences can represent two kinds of items: chapter marks, or full blown
 * tracks
 *
 * The list of sequences is kept sorted in track order, with chapters
 * as the secondary sort key.
 *
 * There are two main differences between the way tracks and chapters are
 * handled. First off, chapters can always be coallessed during play, while
 * tracks can't be coallessed for MPEG. Second, only tracks show up in the
 * list selector.
 */

#define ISTRACK(s)   (s->cds_Track)
#define ISCHAPTER(s) (!s->cds_Track)


/*****************************************************************************/


struct CDDisk *GetDiskInfo(struct MPEGPlayerLib *MPEGPlayerBase);
void FreeDiskInfo(struct MPEGPlayerLib *MPEGPlayerBase, struct CDDisk *diskInfo);


/*****************************************************************************/


#endif /* DISKINFO_H */
