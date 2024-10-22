
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/videocd.h>
#include <devices/cd.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/videocd_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/videocd_pragmas.h>

#include "defs.h"
#include "utils.h"
#include "mpegplayerbase.h"
#include "diskinfo.h"

/*	Debug switches */

#ifdef	DEBUG
extern void kprintf(char *,...);
#define	D(a)	kprintf a
#else
#define	D(a)
#endif

/*****************************************************************************/


static STRPTR GetStringTag(struct MPEGPlayerLib *MPEGPlayerBase,
                           ULONG tag, struct TagItem *tagList)
{
struct CDText *ct;

    if (ct = (struct CDText *)GetTagData(tag,NULL,tagList))
        return(ct->cs_Text);

    return(NULL);
}


/*****************************************************************************/


struct CDDisk *GetDiskInfo(struct MPEGPlayerLib *MPEGPlayerBase)
{
struct TagItem    *tags;
struct CDDisk     *diskInfo;
struct CDSequence *seqInfo;
struct CDSequence *chapInfo;
struct CDSequence *next;
struct CDSequence *old;
ULONG              trackNum;
ULONG		   oldtrackNum;
ULONG              chapNum;
LONG               type;
union CDTOC        ac[100];
ULONG             *marks;
ULONG              numMarks;
ULONG		   numDVTracks;
struct MTrack	 *mtracks;
struct MinList	 *mlist;
ULONG		   EndTrack;
ULONG		   LastTrack;
struct MinList     temp;

#if (DO_TEXTDISPLAY)
struct CDText     *cdtext;
#endif

    if (diskInfo = AllocVec(sizeof(struct CDDisk),MEMF_CLEAR))
    {
        NewList((struct List *)&diskInfo->cdd_Sequences);

        MPEGPlayerBase->mp_AudioReq->io_Command = CD_TOCLSN;
        MPEGPlayerBase->mp_AudioReq->io_Data    = &ac;
        MPEGPlayerBase->mp_AudioReq->io_Length  = 100;
        MPEGPlayerBase->mp_AudioReq->io_Offset  = 0;
        if (DoIO(MPEGPlayerBase->mp_AudioReq) == 0)
        {
            diskInfo->cdd_OnlyAudio = TRUE;
            diskInfo->cdd_NumTracks = ac[0].Summary.LastTrack - ac[0].Summary.FirstTrack + 1;

	D(("diskInfo->cdd_NumTracks = %ld\n",(ULONG)diskInfo->cdd_NumTracks));

            // First off, get all the tracks on this disk...

            trackNum = 1;
            while (trackNum <= diskInfo->cdd_NumTracks)
            {
                if (seqInfo = AllocVec(sizeof(struct CDSequence),MEMF_CLEAR))
                {
                    seqInfo->cds_TrackNumber = ac[trackNum].Entry.Track;
                    seqInfo->cds_AudioTrack  = TRUE;
                    seqInfo->cds_Track       = TRUE;
                    seqInfo->cds_StartSector = ac[trackNum].Entry.Position.LSN;

	D(("seqInfo->cds_TrackNumber = %ld seqInfo->cds_StartSector = %ld\n",(ULONG)seqInfo->cds_TrackNumber,(ULONG)seqInfo->cds_StartSector));

                    if (trackNum < diskInfo->cdd_NumTracks)
                        seqInfo->cds_EndSector = ac[trackNum+1].Entry.Position.LSN - 1;
                    else
                        seqInfo->cds_EndSector = ac[0].Summary.LeadOut.LSN - 1;

                    seqInfo->cds_TrackStart = seqInfo->cds_StartSector;
                    seqInfo->cds_TrackEnd   = seqInfo->cds_EndSector;

	D(("seqInfo->cds_TrackStart = %ld seqInfo->cds_TrackEnd = %ld\n",(ULONG)seqInfo->cds_TrackStart,(ULONG)seqInfo->cds_TrackEnd));

                    if (ac[trackNum].Entry.CtlAdr & CTL_DATA)
                    {
                        if (MPEGPlayerBase->mp_MPEGReq->iomr_Req.io_Device)
                        {
                            diskInfo->cdd_OnlyAudio = FALSE;
                            seqInfo->cds_AudioTrack = FALSE;
                        }
                        else
                        {
                            // there's no mpeg board, don't allow data tracks
                            FreeVec(seqInfo);
                            seqInfo = NULL;
                        }
                    }

                    if (seqInfo)
                        AddTail((struct List *)&diskInfo->cdd_Sequences,seqInfo);
                }

                trackNum++;
            }

            // Now that we have all the tracks, try to get extra info
            // for them.

	    MPEGPlayerBase->mp_VideoCDType = CDT_UNKNOWN;

            if (VideoCDBase = OpenLibrary("videocd.library",0))
            {
                type = GetCDTypeA(NULL,NULL);

	    	MPEGPlayerBase->mp_VideoCDType = type;

	D(("CD type = %ld\n",(ULONG)type));

                if ((type == CDT_ERROR) || (type == CDT_UNKNOWN))
                {
                    // dunno what this is, give up...
                    FreeDiskInfo(MPEGPlayerBase,diskInfo);
                    return(NULL);
                }
                else if ((type != CDT_VIDEOCD) && (type != CDT_KARAOKE) && (type != CDT_CDIFMV))
                {
                    // must be a CD32/CDTV disk, let the system do its thing...
                    FadeOut(MPEGPlayerBase,FALSE);
                    ColdReboot();
                }
	// if this is a CDI FMV disk, Jerry returns 2 tracks in cd.device,
	// and we need to kill the last one

		if(type == CDT_CDIFMV)
		{

			while(diskInfo->cdd_NumTracks > 1)
			{
				seqInfo = (struct CDSequence *)RemTail((struct List *)&diskInfo->cdd_Sequences);
				diskInfo->cdd_NumTracks--;
				FreeVec(seqInfo);
			}
		}

	// if this is a Karaoke or Video CD, we know the first track is
	// an ISO image and we can/want to ignore it

		if((type == CDT_VIDEOCD) || (type == CDT_KARAOKE))
		{
	D(("remove ISO track\n"));

			seqInfo = (struct CDSequence *)RemHead((struct List *)&diskInfo->cdd_Sequences);
			diskInfo->cdd_NumTracks--;
			oldtrackNum = seqInfo->cds_TrackNumber;
			FreeVec(seqInfo);

	// now we have to renumber all of the tracks (and for safety sake, we
	// will bump all the track numbers instead of just numbering 1-n

                        SCANLIST(&diskInfo->cdd_Sequences,seqInfo)
			{
				trackNum = oldtrackNum;
				oldtrackNum = seqInfo->cds_TrackNumber;
				seqInfo->cds_TrackNumber = trackNum;
	D(("seqInfo->cds_TrackNumber = %ld seqInfo->cds_StartSector = %ld\n",(ULONG)seqInfo->cds_TrackNumber,(ULONG)seqInfo->cds_StartSector));
	D(("seqInfo->cds_TrackStart = %ld seqInfo->cds_TrackEnd = %ld\n",(ULONG)seqInfo->cds_TrackStart,(ULONG)seqInfo->cds_TrackEnd));

			}
		}

                if (diskInfo->cdd_VideoCDHandle = ObtainCDHandleA(NULL,NULL))
                {
                    if (tags = GetVideoCDInfo(diskInfo->cdd_VideoCDHandle,0,GVCD_DiscTitle,0,
									    GVCD_InfoNumberVolumes,0,
									    GVCD_InfoVolumeNumber,0,
#if (DO_TEXTDISPLAY)
                    							    GVCD_DiscDescription,0,
#endif
                    							    TAG_DONE))
                    {
                        diskInfo->cdd_Name         = GetStringTag(MPEGPlayerBase,GVCD_DiscTitle,tags);
                        diskInfo->cdd_VideoCDInfo  = tags;
                        diskInfo->cdd_VolumeNumber = GetTagData(GVCD_InfoVolumeNumber,0,tags);
                        diskInfo->cdd_NumVolumes   = GetTagData(GVCD_InfoNumberVolumes,0,tags);

#if (DO_TEXTDISPLAY)
                        if (cdtext = (struct CDText *)GetTagData(GVCD_DiscDescription,0,tags))
                        {
                            diskInfo->cdd_Notes    = cdtext->cs_Text;
                            diskInfo->cdd_NotesLen = cdtext->cs_Length;
                        }
#endif

		/* for CDI disks, check for a track list which defines sequences on this disk */

			if(type == CDT_CDIFMV)
			{
				if(numDVTracks = GetTagData(GVCD_NumDigitalVideoTracks,0,tags))
				{
					D(("numDVTracks=%ld\n",numDVTracks));
					if(mlist = (struct MinList *)GetTagData(GVCD_DigitalVideoTrackList,NULL,tags))
					{
						mtracks = (struct MTrack *)mlist->mlh_Head;
						NewList((struct List *)&temp);

						LastTrack = 1L;

						while(numDVTracks)
						{
							if(seqInfo=(struct CDSequence *)RemHead((struct List *)&diskInfo->cdd_Sequences))
							{
								EndTrack = seqInfo->cds_TrackEnd;
								LastTrack = seqInfo->cds_TrackNumber;

								diskInfo->cdd_NumTracks--;

								D(("Remove a track\n"));
							}
							else
							{

								if (seqInfo = AllocVec(sizeof(struct CDSequence),MEMF_CLEAR))
								{
									seqInfo->cds_TrackNumber = LastTrack;
									seqInfo->cds_AudioTrack = FALSE;
									seqInfo->cds_Track = TRUE;
					                                seqInfo->cds_VideoCDInfo = seqInfo;	/* TRUE - is a valid track */
									D(("Allocate a new track\n"));


								}

							}
							if(seqInfo)
							{
								seqInfo->cds_StartSector = mtracks->mt_Block;
								seqInfo->cds_TrackStart = seqInfo->cds_StartSector;
								seqInfo->cds_EndSector = seqInfo->cds_StartSector + mtracks->mt_Size;
								seqInfo->cds_TrackEnd = seqInfo->cds_EndSector;

								seqInfo->cds_TrackNumber = LastTrack;
								D(("start %ld end %ld number %ld\n",seqInfo->cds_StartSector,seqInfo->cds_EndSector,seqInfo->cds_TrackNumber));

								AddTail((struct List *)&temp,(struct Node *)seqInfo);
								LastTrack++;

								diskInfo->cdd_NumTracks++;
							}
							mtracks = (struct MTrack *)mtracks->mt_Node.mln_Succ;
							numDVTracks--;
						}
						
						while(seqInfo = (struct CDSequence *)RemHead((struct List *)&temp))
						{
							AddTail((struct List *)&diskInfo->cdd_Sequences,(struct Node *)seqInfo);
						}
					}
				}
			}

                        numMarks = 0;
			if (marks = (APTR)GetTagData(GVCD_EntriesArray,0,tags))
			{
			    numMarks = GetTagData(GVCD_EntriesUsed,0,tags);

		D(("numMarks = %ld\n",(ULONG)numMarks));
			}

                        SCANLIST(&diskInfo->cdd_Sequences,seqInfo)
                        {
                            D(("seqInfo = %ld\n",(ULONG)seqInfo));

                            if (tags = GetVideoCDInfo(diskInfo->cdd_VideoCDHandle,seqInfo->cds_TrackNumber,GVCD_SequenceStartSector,0,
                            										   GVCD_SequenceEndSector,0,
                            										   GVCD_SequenceTitle,0,
                            										   GVCD_SequencePerformer,0,
                            										   GVCD_SequenceWriter,0,
                            										   GVCD_SequenceComposer,0,
                            										   GVCD_SequenceArranger,0,
                            										   GVCD_SequencePlayer,0,
#if (DO_TEXTDISPLAY)
                            										   GVCD_SequenceDescription,0,
                            										   GVCD_SequenceText,0,
#endif
                                                                                                           TAG_DONE))
                            {
                                seqInfo->cds_Name          = GetStringTag(MPEGPlayerBase,GVCD_SequenceTitle,tags);
                                seqInfo->cds_StartSector   = (ULONG)GetTagData(GVCD_SequenceStartSector,seqInfo->cds_StartSector,tags);
                                seqInfo->cds_EndSector     = (ULONG)GetTagData(GVCD_SequenceEndSector,seqInfo->cds_EndSector,tags);
                                seqInfo->cds_TrackStart    = seqInfo->cds_StartSector;
                                seqInfo->cds_TrackEnd      = seqInfo->cds_EndSector;
                                seqInfo->cds_VideoCDInfo   = tags;
                                seqInfo->cds_Performer     = GetStringTag(MPEGPlayerBase,GVCD_SequencePerformer, tags);
                                seqInfo->cds_Writer        = GetStringTag(MPEGPlayerBase,GVCD_SequenceWriter, tags);
                                seqInfo->cds_Composer      = GetStringTag(MPEGPlayerBase,GVCD_SequenceComposer, tags);
                                seqInfo->cds_Arranger      = GetStringTag(MPEGPlayerBase,GVCD_SequenceArranger, tags);
                                seqInfo->cds_Player        = GetStringTag(MPEGPlayerBase,GVCD_SequencePlayer, tags);

			D(("seqInfo->cds_StartSector = %ld seqInfo->cds_EndSector = %ld\n",(ULONG)seqInfo->cds_StartSector,(ULONG)seqInfo->cds_EndSector));

#if (DO_TEXTDISPLAY)
                                if (cdtext = (struct CDText *)GetTagData(GVCD_SequenceDescription,0,tags))
                                {
                                    seqInfo->cds_Notes    = cdtext->cs_Text;
                                    seqInfo->cds_NotesLen = cdtext->cs_Length;
                                }

                                if (cdtext = (struct CDText *)GetTagData(GVCD_SequenceText,0,tags))
                                {
                                    seqInfo->cds_Lyrics    = cdtext->cs_Text;
                                    seqInfo->cds_LyricsLen = cdtext->cs_Length;
                                }
#endif
                            }
                        }

                // coalesce VideoCD chapter marks if any

                        if(numMarks)
                        {
                            chapNum = 0;
                            SCANLIST(&diskInfo->cdd_Sequences,seqInfo)
                            {
                                old = seqInfo;

                                while(chapNum < numMarks)
                                {

                                    D(("marks[%ld] = %ld\n",(ULONG)chapNum,(ULONG)marks[chapNum]));

                                    if (marks[chapNum] > seqInfo->cds_StartSector)
                                    {
                // this chapter mark is a potential candidate for adding to the list
                                        D(("> StartSector\n"));

                                        if(marks[chapNum] < seqInfo->cds_EndSector)
                                        {

                                                D(("< EndSector\n"));

                                                if (chapInfo = AllocVec(sizeof(struct CDSequence),MEMF_ANY))
                                                {
                                                    D(("Add chapter mark\n"));
                                                    *chapInfo                   = *seqInfo;
                                                    chapInfo->cds_StartSector   = marks[chapNum];
                                                    chapInfo->cds_EndSector     = seqInfo->cds_TrackEnd;
                                                    chapInfo->cds_Track         = FALSE;
                                                    old->cds_EndSector          = chapInfo->cds_StartSector - 1;
                                                    Insert((struct List *)&diskInfo->cdd_Sequences,chapInfo,old);
                                                    old = chapInfo;
                                                }
                                        }
                                        else
                                        {
                // this chapter mark is past the end of this sequence, ignore it
                                                D(("> EndSector\n"));
                                                break;
                                        }
                                    }

                // else just increment chapNum
                                    D(("+ chapNum\n"));
                                    chapNum++;
                                }
                            }
                        }
                    }
                }
            }

            // any data tracks with no video info must be thrown away...

	D(("prune track list\n"));

            seqInfo = (struct CDSequence *)diskInfo->cdd_Sequences.mlh_Head;
            while (next = (struct CDSequence *)seqInfo->cds_Link.ln_Succ)
            {

	D(("seqInfo->cds_Name = %s seqInfo->cds_StartSector %ld seqInfo->cds_EndSector %ld\n",
		seqInfo->cds_Name,seqInfo->cds_StartSector,seqInfo->cds_EndSector));

	D(("seqInfo->cds_AudioTrack = %ld  seqInfo->cds_VideoCDInfo = %ld\n",
		(ULONG)seqInfo->cds_AudioTrack,(ULONG)seqInfo->cds_VideoCDInfo));

                if (!seqInfo->cds_AudioTrack && !seqInfo->cds_VideoCDInfo)
                {
                    Remove((struct Node *)seqInfo);
                    FreeVec(seqInfo);
                    diskInfo->cdd_NumTracks--;
		    D(("-"));
                }
		D(("+"));
                seqInfo = next;
            }
        }

        // if the sequence list is empty, just return nothingness...
        if (!diskInfo->cdd_Sequences.mlh_Head->mln_Succ)
        {
            FreeDiskInfo(MPEGPlayerBase,diskInfo);
            return(NULL);
        }
    }

    return (diskInfo);
}


/*****************************************************************************/


void FreeDiskInfo(struct MPEGPlayerLib *MPEGPlayerBase,
                  struct CDDisk *diskInfo)
{
struct CDSequence *seq;

    if (diskInfo)
    {
        while (seq = (struct CDSequence *)RemHead((struct List *)&diskInfo->cdd_Sequences))
        {
            if (ISTRACK(seq))
                if (seq->cds_VideoCDInfo)
		{
		/* is this a real info struct, or just a TRUE flag? */
		    if(seq->cds_VideoCDInfo != seq)
		    {
                    	FreeVideoCDInfo(seq->cds_VideoCDInfo);
		    }
		}
            FreeVec(seq);
        }

        if (diskInfo->cdd_VideoCDInfo)
            FreeVideoCDInfo(diskInfo->cdd_VideoCDInfo);

        if (diskInfo->cdd_VideoCDHandle)
            ReleaseCDHandle(diskInfo->cdd_VideoCDHandle);

        CloseLibrary(VideoCDBase);
        VideoCDBase = NULL;

        FreeVec(diskInfo);
    }
}
