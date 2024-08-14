
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
#include <devices/cd.h>
#include <libraries/lowlevel.h>
#include <libraries/cdgprefs.h>
#include <libraries/videocd.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/cdg_cr_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/cdg_cr_pragmas.h>

#include "utils.h"
#include "mpegplayerbase.h"
#include "defs.h"
#include "cdg.h"
#include "diskinfo.h"
#include "spliner.h"
#include "eventloop.h"
#include "play.h"

/*	Debug switches */

#ifdef	DEBUG
extern void kprintf(char *,...);
#define	D(a)	kprintf a
#else
#define	D(a)
#endif


/*****************************************************************************/


enum Commands
{
    XCMD_NOP,
    XCMD_PLAY,
    XCMD_STOP,
    XCMD_LOOP,
    XCMD_INTRO,
    XCMD_RANDOM,
    XCMD_TOGGLE_TIME_CDG,

    XCMD_FFWD_DOWN,
    XCMD_FFWD_UP,

    XCMD_FREV_DOWN,
    XCMD_FREV_UP,

    XCMD_UP,
    XCMD_DOWN
};

enum States
{
    STATE_STOPPED,
    STATE_PAUSED,
    STATE_PLAYING,
    STATE_SLOWMOTION_FWD,
    STATE_FFWD,
    STATE_FREV,
    STATE_FFWD_WAIT,
    STATE_FREV_WAIT,
    STATE_FFWD_WAIT_PAUSED,
    STATE_FREV_WAIT_PAUSED
};

// # of timer ticks between updates of the screen while playing...
#define UPDATE_DELAY 8

// # of timer ticks icons stay on the screen...
#define ICON_DELAY         200
#define ICON_DELAY_FOREVER 0xffffffff


/*****************************************************************************/


struct PlaySequence
{
    struct MinNode     ps_Link;
    struct CDSequence *ps_Reference;
    ULONG              ps_StartSector;    // cached from CDSequence
    ULONG              ps_EndSector;      // cached from CDSequence
    BOOL               ps_AudioTrack;     // cached from CDSequence
};


/*****************************************************************************/


UWORD __far dither[] = {0xaaaa, 0x5555};


/*****************************************************************************/


static void SendMainIO(struct MPEGPlayerLib *MPEGPlayerBase, APTR ioReq)
{
    if (MPEGPlayerBase->mp_Outstanding)
        WaitIO(MPEGPlayerBase->mp_Outstanding);

    SendIO(ioReq);
    MPEGPlayerBase->mp_Outstanding = ioReq;
}


/*****************************************************************************/


static BOOL CheckMainIO(struct MPEGPlayerLib *MPEGPlayerBase)
{
    if (MPEGPlayerBase->mp_Outstanding)
        return ((BOOL)(CheckIO(MPEGPlayerBase->mp_Outstanding) != 0));

    return (TRUE);
}


/*****************************************************************************/


static void AbortMainIO(struct MPEGPlayerLib *MPEGPlayerBase)
{

    D(("AbortMainIO()\n"));

    if (MPEGPlayerBase->mp_Outstanding)
    {
        AbortIO(MPEGPlayerBase->mp_Outstanding);
        WaitIO(MPEGPlayerBase->mp_Outstanding);
        MPEGPlayerBase->mp_Outstanding = NULL;
    }
}


/*****************************************************************************/


static void SendTimerIO(struct MPEGPlayerLib *MPEGPlayerBase)
{
    MPEGPlayerBase->mp_TimerReq->tr_node.io_Command = TR_ADDREQUEST;
    MPEGPlayerBase->mp_TimerReq->tr_time.tv_secs    = 0;
    MPEGPlayerBase->mp_TimerReq->tr_time.tv_micro   = 13250;  // slightly over 75 times a second
    SendIO(MPEGPlayerBase->mp_TimerReq);
}


/*****************************************************************************/


static void WaitTimerIO(struct MPEGPlayerLib *MPEGPlayerBase)
{
    WaitIO(MPEGPlayerBase->mp_TimerReq);
}


/*****************************************************************************/


static void AbortTimerIO(struct MPEGPlayerLib *MPEGPlayerBase)
{
    AbortIO(MPEGPlayerBase->mp_TimerReq);
    WaitIO(MPEGPlayerBase->mp_TimerReq);
}


/*****************************************************************************/


void FreePlayList(struct MPEGPlayerLib *MPEGPlayerBase, struct MinList *playList)
{
struct Node *node;

    while (node = RemHead((struct List *)playList))
        FreeVec(node);
}


/*****************************************************************************/


static void MakePlayList(struct MPEGPlayerLib *MPEGPlayerBase,
			 struct CDDisk *disk, struct MinList *playList)
{
struct CDSequence   *seq;
struct PlaySequence *ps;
ULONG                numSelected;

    FreePlayList(MPEGPlayerBase,playList);

    // count the number of selected sequences
    numSelected = 0;
    SCANLIST(&disk->cdd_Sequences,seq)
    {
        if (seq->cds_Selected)
            numSelected++;
    }

    SCANLIST(&disk->cdd_Sequences,seq)
    {
        if ((seq->cds_Selected) || (numSelected == 0))
        {
            if (ps = AllocVec(sizeof(struct PlaySequence),MEMF_ANY))
            {
                ps->ps_Reference   = seq;
                ps->ps_StartSector = seq->cds_StartSector;
                ps->ps_EndSector   = seq->cds_EndSector;
                ps->ps_AudioTrack  = seq->cds_AudioTrack;
                AddTail((struct List *)playList,(struct Node *)ps);
            }
        }
    }
}


/*****************************************************************************/


#define BAD_SECTOR 0xffffffff

static ULONG GetCurrentSector(struct MPEGPlayerLib *MPEGPlayerBase,
			      struct PlaySequence *current)
{
struct QCode     qcode;
ULONG            sector;
struct IOStdReq *ioReq;
APTR             CD32Base;

    sector = BAD_SECTOR;
    if (current)
    {
        if (current->ps_AudioTrack)
        {
            ioReq             = MPEGPlayerBase->mp_AudioReq2;
            ioReq->io_Command = CD_QCODELSN;
            ioReq->io_Data    = &qcode;
            ioReq->io_Length  = 0;
            ioReq->io_Offset  = 0;
            if (DoIO(ioReq) == 0)
                sector = qcode.DiskPosition.LSN;
        }
        else
        {
            if (CD32Base = MPEGPlayerBase->mp_MPEGReq2->iomr_Req.io_Device)
                sector = GetSector(MPEGPlayerBase->mp_MPEGReq2->iomr_Req.io_Unit);
        }

        if ((sector < MPEGPlayerBase->mp_LowSector) || (sector > 0x7000000))
	{
            sector = MPEGPlayerBase->mp_LowSector;
	}
        else if (sector > MPEGPlayerBase->mp_HighSector)
            sector = MPEGPlayerBase->mp_HighSector;
    }

    return (sector);
}


/*****************************************************************************/


static struct PlaySequence *GetCurrentSequence(struct MPEGPlayerLib *MPEGPlayerBase,
                                               struct PlaySequence *current, ULONG options)
{
ULONG sector;

    if (current)
    {
        // MPEG tracks are always accurate, since they don't get
        // coallessed

	// Yikes - no, since the chapter marks in this list must
	// be coalesced too!  That would have been true
	// had the chapter marks been stored as a child list of each
	// track, but they are not. -- Darren

	/*

        if (!current->ps_AudioTrack)
        {
            if (OPTF_INTRO & options)
            {
                sector = GetCurrentSector(MPEGPlayerBase,current);
                if (sector == BAD_SECTOR)
                    return (NULL);

                if (sector >= current->ps_StartSector + SECTORS_PER_INTRO)
                    AbortMainIO(MPEGPlayerBase);
            }

            return (current);
        }

	*/

        sector = GetCurrentSector(MPEGPlayerBase,current);
        if (sector != BAD_SECTOR)
        {
            while (TRUE)
            {
                if (sector > current->ps_EndSector)
                {
		    D(("sector %ld current->ps_EndSector = %ld\n",sector,current->ps_EndSector));

                    current = (struct PlaySequence *)current->ps_Link.mln_Succ;
                    if (!current->ps_Link.mln_Succ)
                        return(NULL);
                }
                else if (sector < current->ps_StartSector)
                {
		    D(("sector %ld current->ps_StartSector = %ld\n",sector,current->ps_EndSector));

                    current = (struct PlaySequence *)current->ps_Link.mln_Pred;
                    if (!current->ps_Link.mln_Pred)
                        return(NULL);
                }
                else
                {
                    if ((OPTF_INTRO & options)
                    &&  (sector >= current->ps_StartSector + SECTORS_PER_INTRO))
                        AbortMainIO(MPEGPlayerBase);

                    return(current);
                }
            }
        }
    }

    return (NULL);
}


/*****************************************************************************/


static void DoIcon(struct MPEGPlayerLib *MPEGPlayerBase, enum Icons icon)
{
WORD             x0, y0, w, h;
struct RastPort *rp;

    if (MPEGPlayerBase->mp_PlayScreen)
    {
        rp = &MPEGPlayerBase->mp_PlayScreen->RastPort;

        if (icon == NOP_ICON)
        {
            SetABPenDrMd(rp,0,0,JAM1);
            SetWriteMask(rp,0x03);
            RectFill(rp,MPEGPlayerBase->mp_PlayScreen->Width - 115,
                        MPEGPlayerBase->mp_PlayScreen->Height - 55,
                        MPEGPlayerBase->mp_PlayScreen->Width - 35,
                        MPEGPlayerBase->mp_PlayScreen->Height - 1);

            MPEGPlayerBase->mp_IconCountdown = 0;
        }
        else
        {
            SetABPenDrMd(rp,2,0,JAM1);
            SetAfPt(rp,dither,1);
            SetWriteMask(&MPEGPlayerBase->mp_PlayScreen->RastPort,0x02);
            RectFill(rp,MPEGPlayerBase->mp_PlayScreen->Width - 115,
                        MPEGPlayerBase->mp_PlayScreen->Height - 55,
                        MPEGPlayerBase->mp_PlayScreen->Width - 35,
                        MPEGPlayerBase->mp_PlayScreen->Height - 1);

            SetAfPt(rp,NULL,0);

            x0 = iconsPos[icon].MinX;
            y0 = iconsPos[icon].MinY;
            w  = iconsPos[icon].MaxX - x0 + 1;
            h  = iconsPos[icon].MaxY - y0 + 1;

            if (h >= 56)
                h = 55;

            BltBitMap(MPEGPlayerBase->mp_IconBitMap,x0,y0,
                      rp->BitMap,MPEGPlayerBase->mp_PlayScreen->Width - 120,MPEGPlayerBase->mp_PlayScreen->Height - 56,
                      w,h,0xc0,0x01,NULL);

            MPEGPlayerBase->mp_IconCountdown = ICON_DELAY;
        }

        SetABPenDrMd(rp,1,2,JAM2);
        SetWriteMask(rp,0x01);
    }
}


/*****************************************************************************/


static void DoOptionIcon(struct MPEGPlayerLib *MPEGPlayerBase, enum Icons icon,
                         ULONG options, ULONG opt)
{
    if (options & opt)
    {
        DoIcon(MPEGPlayerBase,icon);
    }
    else
    {
        DoIcon(MPEGPlayerBase,NOP_ICON);
    }
}


/*****************************************************************************/


static void PrintTime(struct MPEGPlayerLib *MPEGPlayerBase, UWORD x, UWORD y,
		      ULONG time)
{
char  buffer[30];
ULONG bufLen;

    bufLen = nprintf(buffer,sizeof(buffer),"%02.2ld:%02.2ld",
                     time / SECTORS_PER_MINUTE,
                     (time / SECTORS_PER_SECOND) % 60);

    Move(&MPEGPlayerBase->mp_PlayScreen->RastPort,x,y);
    Text(&MPEGPlayerBase->mp_PlayScreen->RastPort,buffer,bufLen);
}


/*****************************************************************************/


static void DoStatus(struct MPEGPlayerLib *MPEGPlayerBase,
		     struct MinList *playList, struct PlaySequence *current,
                     ULONG options)
{
ULONG                sector;
ULONG                start;
ULONG                end;
char                 buffer[30];
ULONG                bufLen;
struct PlaySequence *ps;
ULONG                total;
ULONG                trackElapsed;
ULONG                trackRemaining;
ULONG                diskElapsed;
ULONG                diskRemaining;
BOOL                 before;
BOOL                 displayIt;
struct RastPort     *rp;
ULONG                len;
WORD                 count, oldCount;
WORD                 y, offset, gap;

    rp = &MPEGPlayerBase->mp_PlayScreen->RastPort;

    displayIt = FALSE;
    if (current)
    {
        if (OPTF_TIMEDEFAULT & options)
        {
            if (current->ps_AudioTrack)
                displayIt = TRUE;
        }
        else if (OPTF_TIMEOVERLAY & options)
        {
            displayIt = TRUE;
        }
        else if (MPEGPlayerBase->mp_CDGState == CDG_HAVEGRAPHICS)
        {
            displayIt = TRUE;
        }
    }

    if (!displayIt)
    {
        if (MPEGPlayerBase->mp_TimeDisplay)
        {
            SetABPenDrMd(rp,0,0,JAM1);
            SetWriteMask(rp,0x03);
            RectFill(rp,0,
            		MPEGPlayerBase->mp_PlayScreen->Height - 65,
            		MPEGPlayerBase->mp_PlayScreen->Width - 1,
                        MPEGPlayerBase->mp_PlayScreen->Height - 1);
            MPEGPlayerBase->mp_TimeDisplay = FALSE;
        }
    }
    else
    {
        if (!MPEGPlayerBase->mp_TimeDisplay)
        {
            SetABPenDrMd(rp,2,0,JAM1);
            SetWriteMask(rp,0x02);
            SetAfPt(rp,dither,1);
            RectFill(rp,0,
            		MPEGPlayerBase->mp_PlayScreen->Height - 65,
            		MPEGPlayerBase->mp_PlayScreen->Width - 244,
                        MPEGPlayerBase->mp_PlayScreen->Height - 1);

            SetAfPt(rp,NULL,0);

            SetABPenDrMd(rp,1,2,JAM2);
            SetWriteMask(rp,0x01);

            BltBitMap(MPEGPlayerBase->mp_IconBitMap,iconsPos[TIME_ICON].MinX,iconsPos[TIME_ICON].MinY,
                      rp->BitMap,50,MPEGPlayerBase->mp_PlayScreen->Height - 61,
                      iconsPos[TIME_ICON].MaxX - iconsPos[TIME_ICON].MinX + 1,
                      iconsPos[TIME_ICON].MaxY - iconsPos[TIME_ICON].MinY + 1,
                      0xc0,0x01,NULL);

            MPEGPlayerBase->mp_TimeDisplay        = TRUE;
            MPEGPlayerBase->mp_LastTrack          = 0xffffffff;
            MPEGPlayerBase->mp_LastOptions        = 0;
            MPEGPlayerBase->mp_LastTrackElapsed   = 0xffffffff;
            MPEGPlayerBase->mp_LastTrackRemaining = 0xffffffff;
            MPEGPlayerBase->mp_LastDiskElapsed    = 0xffffffff;
            MPEGPlayerBase->mp_LastDiskRemaining  = 0xffffffff;
        }

        if (!current)
        {
            sector = 0;
            start  = 0;
            end    = 0;
        }
        else
        {


            start = current->ps_Reference->cds_TrackStart;
            end   = current->ps_Reference->cds_TrackEnd;

            sector = GetCurrentSector(MPEGPlayerBase,current);
            if (sector == BAD_SECTOR)
            {
                sector = start;
            }
            else if (sector < start)
            {
                sector = start;
            }
            else if (sector > end)
            {
                sector = end;
            }
        }

        total          = end - start + 1;
        trackElapsed   = sector - start;
        trackRemaining = end - sector;

        // compensate for round-off error
        if ((trackRemaining / SECTORS_PER_SECOND) + (trackElapsed / SECTORS_PER_SECOND) < (total / SECTORS_PER_SECOND))
            trackRemaining += SECTORS_PER_SECOND;

        total       = 0;
        diskElapsed = 0;
        before      = TRUE;

        SCANLIST(playList,ps)
        {
            if (ISTRACK(ps->ps_Reference))
            {
                if (before && (ps->ps_Reference->cds_TrackNumber == current->ps_Reference->cds_TrackNumber))
                {
                    diskElapsed = total + (sector - start);
                    before      = FALSE;
                }
                total += ps->ps_Reference->cds_TrackEnd - ps->ps_Reference->cds_TrackStart + 1;

            }
        }

        diskRemaining = total - diskElapsed;

        // compensate for round-off error
        if ((diskRemaining / SECTORS_PER_SECOND) + (diskElapsed / SECTORS_PER_SECOND) < (total / SECTORS_PER_SECOND))
            diskRemaining += SECTORS_PER_SECOND;

        if ((trackElapsed != MPEGPlayerBase->mp_LastTrackElapsed) || (trackRemaining != MPEGPlayerBase->mp_LastTrackRemaining))
        {
            SetFont(rp,&MPEGPlayerBase->mp_Compact24);
            PrintTime(MPEGPlayerBase,150,MPEGPlayerBase->mp_PlayScreen->Height - 35,trackElapsed);
            PrintTime(MPEGPlayerBase,150,MPEGPlayerBase->mp_PlayScreen->Height - 11,trackRemaining);
            MPEGPlayerBase->mp_LastTrackElapsed   = trackElapsed;
            MPEGPlayerBase->mp_LastTrackRemaining = trackRemaining;
        }

        if ((diskElapsed != MPEGPlayerBase->mp_LastDiskElapsed) || (diskRemaining != MPEGPlayerBase->mp_LastDiskRemaining))
        {
            SetFont(rp,&MPEGPlayerBase->mp_Compact24);
            PrintTime(MPEGPlayerBase,326,MPEGPlayerBase->mp_PlayScreen->Height - 35,diskElapsed);
            PrintTime(MPEGPlayerBase,326,MPEGPlayerBase->mp_PlayScreen->Height - 11,diskRemaining);
            MPEGPlayerBase->mp_LastDiskElapsed   = diskElapsed;
            MPEGPlayerBase->mp_LastDiskRemaining = diskRemaining;
        }

        if (current->ps_Reference->cds_TrackNumber != MPEGPlayerBase->mp_LastTrack)
        {
            bufLen = nprintf(buffer,sizeof(buffer),"%ld",current->ps_Reference->cds_TrackNumber);
            SetFont(rp,&MPEGPlayerBase->mp_Compact31);
            SetABPenDrMd(rp,0,0,JAM1);
            len = TextLength(rp,buffer,bufLen);
            RectFill(rp,10,MPEGPlayerBase->mp_PlayScreen->Height - 49,
                        40-len,MPEGPlayerBase->mp_PlayScreen->Height - 18);
            SetABPenDrMd(rp,1,0,JAM2);
            Move(rp,40-len,MPEGPlayerBase->mp_PlayScreen->Height - 21);
            Text(rp,buffer,bufLen);
            MPEGPlayerBase->mp_LastTrack = current->ps_Reference->cds_TrackNumber;
        }

        if (options != MPEGPlayerBase->mp_LastOptions)
        {
            oldCount = 0;
            if ((OPTF_RANDOM | OPTF_LOOP | OPTF_INTRO) & MPEGPlayerBase->mp_LastOptions)
                oldCount = 1;

            MPEGPlayerBase->mp_LastOptions = options;

            count = 0;
            if (OPTF_RANDOM & options)
                count++;
            if (OPTF_LOOP & options)
                count++;
            if (OPTF_INTRO & options)
                count++;

            switch (count)
            {
                case 1: offset = 20; gap = 0;  break;
                case 2: offset = 6;  gap = 10; break;
                case 3: offset = 0;  gap = 2;  break;
            }

            if (count == 0)
            {
                /* erase everything */
                SetABPenDrMd(rp,0,0,JAM1);
                SetWriteMask(rp,0x03);
                RectFill(rp,400,
                            MPEGPlayerBase->mp_PlayScreen->Height - 65,
                            444,
                            MPEGPlayerBase->mp_PlayScreen->Height - 1);
                SetABPenDrMd(rp,1,2,JAM2);
                SetWriteMask(rp,0x01);

                return;
            }

            if (oldCount == 0)
            {
                /* draw dither */
                SetABPenDrMd(rp,2,0,JAM1);
                SetWriteMask(rp,0x02);
                SetAfPt(rp,dither,1);
                RectFill(rp,400,
                            MPEGPlayerBase->mp_PlayScreen->Height - 65,
                            444,
                            MPEGPlayerBase->mp_PlayScreen->Height - 1);

                SetAfPt(rp,NULL,0);

                SetABPenDrMd(rp,1,2,JAM2);
                SetWriteMask(rp,0x01);
            }
            else
            {
                // erase previous icons...
                SetABPenDrMd(rp,0,0,JAM1);
                RectFill(rp,400,
                            MPEGPlayerBase->mp_PlayScreen->Height-65,
                            444,
                            MPEGPlayerBase->mp_PlayScreen->Height - 1);
                SetABPenDrMd(rp,1,0,JAM2);
            }

            y = MPEGPlayerBase->mp_PlayScreen->Height - 64 + offset;
            if (OPTF_LOOP & options)
            {
                BltBitMap(MPEGPlayerBase->mp_IconBitMap,iconsPos[LOOP3_ICON].MinX,iconsPos[LOOP3_ICON].MinY,
                          rp->BitMap,405,y,
                          iconsPos[LOOP3_ICON].MaxX - iconsPos[LOOP3_ICON].MinX + 1,
                          iconsPos[LOOP3_ICON].MaxY - iconsPos[LOOP3_ICON].MinY + 1,
                          0xc0,0x01,NULL);

                y += iconsPos[LOOP3_ICON].MaxY - iconsPos[LOOP3_ICON].MinY + gap;
            }

            if (OPTF_RANDOM & options)
            {
                BltBitMap(MPEGPlayerBase->mp_IconBitMap,iconsPos[RANDOM3_ICON].MinX,iconsPos[RANDOM3_ICON].MinY,
                          rp->BitMap,403,y,
                          iconsPos[RANDOM3_ICON].MaxX - iconsPos[RANDOM3_ICON].MinX + 1,
                          iconsPos[RANDOM3_ICON].MaxY - iconsPos[RANDOM3_ICON].MinY + 1,
                          0xc0,0x01,NULL);

                y += iconsPos[RANDOM3_ICON].MaxY - iconsPos[RANDOM3_ICON].MinY + gap;
            }

            if (OPTF_INTRO & options)
            {
                BltBitMap(MPEGPlayerBase->mp_IconBitMap,iconsPos[INTRO3_ICON].MinX,iconsPos[INTRO3_ICON].MinY,
                          rp->BitMap,403,y,
                          iconsPos[INTRO3_ICON].MaxX - iconsPos[INTRO3_ICON].MinX + 1,
                          iconsPos[INTRO3_ICON].MaxY - iconsPos[INTRO3_ICON].MinY + 1,
                          0xc0,0x01,NULL);
            }
        }
    }
}


/*****************************************************************************/


struct ColorBunch
{
    UWORD           cb_NumColors;
    UWORD           cb_FirstColor;
    struct RGBEntry cb_Colors[16];
    UWORD           cb_EndMarker;              /* must be 0 */
};


static void DoScreen(struct MPEGPlayerLib *MPEGPlayerBase,
		     struct MinList *playList, struct PlaySequence *current,
                     ULONG options, BOOL blanked)
{
struct MPEGVideoParamsSet mvp;
struct IOMPEGReq          *mpegReq;
struct ColorBunch          colors;
struct TagItem             vtags[2];
UWORD                      i;

    mpegReq = MPEGPlayerBase->mp_MPEGReq2;
    if (mpegReq->iomr_Req.io_Device)
    {
        memset(&mvp,0,sizeof(struct MPEGVideoParamsSet));
        if (!blanked && current && !current->ps_AudioTrack)
            mvp.mvp_Fade = 65535;

        mpegReq->iomr_Req.io_Command = MPEGCMD_SETVIDEOPARAMS;
        mpegReq->iomr_Req.io_Data    = &mvp;
        mpegReq->iomr_Req.io_Length  = sizeof(struct MPEGVideoParamsSet);
        mpegReq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
        DoIO(mpegReq);

    }

    if (blanked || !current)
    {
        if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
        {
            CDGStop();
            CDGBack();
            (*MPEGPlayerBase->mp_ScreenDepth)(MPEGPlayerBase->mp_Screen,SDEPTH_TOFRONT,0,IntuitionBase);
            MPEGPlayerBase->mp_CDGState = CDG_AVAILABLE;
        }

        if (MPEGPlayerBase->mp_PlayScreen)
        {
            StopSpliner(MPEGPlayerBase);
            CloseWindowSafely(MPEGPlayerBase,MPEGPlayerBase->mp_PlayWindow);
            CloseScreenQuiet(MPEGPlayerBase,MPEGPlayerBase->mp_PlayScreen);
            MPEGPlayerBase->mp_PlayScreen  = NULL;
            MPEGPlayerBase->mp_TimeDisplay = FALSE;
            FadeIn(MPEGPlayerBase,FALSE);
        }
    }
    else
    {
        if (!MPEGPlayerBase->mp_PlayScreen)
        {
            FadeOut(MPEGPlayerBase,FALSE);

            colors.cb_NumColors  = 16;
            colors.cb_FirstColor = 0;
            colors.cb_EndMarker  = 0;
            for (i = 0; i < 16; i++)
            {
                colors.cb_Colors[i].Red   = 0x11111111;
                colors.cb_Colors[i].Green = 0x11111111;
                colors.cb_Colors[i].Blue  = 0x11111111;
            }

            colors.cb_Colors[1].Red   = 0xdddddddd;
            colors.cb_Colors[1].Green = 0xdddddddd;
            colors.cb_Colors[1].Blue  = 0xdddddddd;

	/** old purple color

            colors.cb_Colors[2].Red   = 0x77777777;
            colors.cb_Colors[2].Green = 0x11111111;
            colors.cb_Colors[2].Blue  = 0x66666666;

	**/

            colors.cb_Colors[2].Red   = 0x1c1c1c1c;
            colors.cb_Colors[2].Green = 0x62626262;
            colors.cb_Colors[2].Blue  = 0x7a7a7a7a;


            colors.cb_Colors[3].Red   = 0xdddddddd;
            colors.cb_Colors[3].Green = 0xdddddddd;
            colors.cb_Colors[3].Blue  = 0xdddddddd;

            vtags[0].ti_Tag  = VC_IntermediateCLUpdate;
            vtags[0].ti_Data = FALSE;
            vtags[1].ti_Tag  = TAG_DONE;

            MPEGPlayerBase->mp_PlayScreen = OpenScreenTags(NULL,
                                     SA_Width,           640,
                                     SA_Depth,           2,
                                     SA_DisplayID,       HIRESLACE_KEY,
                                     SA_Interleaved,     TRUE,
                                     SA_Draggable,       FALSE,
                                     SA_Quiet,           TRUE,
                                     SA_ShowTitle,       FALSE,
                                     SA_Colors32,        &colors,
                                     SA_BackFill,        LAYERS_NOBACKFILL,
                                     SA_VideoControl,    vtags,
                                     SA_MinimizeISG,     TRUE,
                                     SA_ColorMapEntries, 10,
                                     SA_Exclusive,       TRUE,
                                     TAG_DONE);

            MPEGPlayerBase->mp_PlayWindow = OpenWindowTags(NULL,
                                         WA_CustomScreen,  MPEGPlayerBase->mp_PlayScreen,
                                         WA_Borderless,    TRUE,
                                         WA_Backdrop,      TRUE,
                                         WA_RMBTrap,       TRUE,
                                         WA_Activate,      TRUE,
                                         WA_RptQueue,      1,
                                         WA_NoCareRefresh, TRUE,
                                         WA_SimpleRefresh, TRUE,
                                         WA_BackFill,      LAYERS_NOBACKFILL,
                                         WA_Pointer,       MPEGPlayerBase->mp_Pointer,
                                         TAG_DONE);

            MPEGPlayerBase->mp_PlayWindow->UserPort = MPEGPlayerBase->mp_Window->UserPort;
            ModifyIDCMP(MPEGPlayerBase->mp_PlayWindow,IDCMP_RAWKEY);

            MPEGPlayerBase->mp_TimeDisplay = FALSE;
        }

        DoStatus(MPEGPlayerBase,playList,current,options);

        if (current->ps_AudioTrack && (MPEGPlayerBase->mp_CDGState < CDG_SHOWGRAPHICS))
            StartSpliner(MPEGPlayerBase);
        else
            StopSpliner(MPEGPlayerBase);
    }
}


/*****************************************************************************/


static BOOL DoPause(struct MPEGPlayerLib *MPEGPlayerBase,
                    struct PlaySequence *current, BOOL paused, BOOL showIcon)
{
struct IOStdReq  *ioReq;
struct IOMPEGReq *mpegReq;
BOOL              both;

    both = FALSE;
    if (!current)
    {
        paused = FALSE;
        both   = TRUE;
    }

    if (both || current->ps_AudioTrack)
    {
        if (MPEGPlayerBase->mp_CDGState >= CDG_SUBCODES)
        {
            if (paused)
                CDGPause();
            else
                CDGPlay(1);
        }

        ioReq             = MPEGPlayerBase->mp_AudioReq2;
        ioReq->io_Command = CD_PAUSE;
        ioReq->io_Data    = NULL;
        ioReq->io_Length  = paused;
        ioReq->io_Offset  = 0;
        DoIO(ioReq);
    }

    if (both || !current->ps_AudioTrack)
    {
        mpegReq = MPEGPlayerBase->mp_MPEGReq2;
        if (mpegReq->iomr_Req.io_Device)
        {
            mpegReq->iomr_Req.io_Command = MPEGCMD_PAUSE;
            mpegReq->iomr_Req.io_Data    = NULL;
            mpegReq->iomr_Req.io_Length  = 0;
            mpegReq->iomr_Req.io_Offset  = 0;
            mpegReq->iomr_PauseMode      = paused;
            DoIO(mpegReq);
        }
    }

    if (showIcon)
    {
        if (paused)
        {
            DoIcon(MPEGPlayerBase,PAUSE_ICON);

	// Apparently the idea is not to show the PAUSE icon indefinitely
	// in video because it prevents the user from watching the
	// still unhindered; likewise it interferes with watching single
	// step frame

            if (current && current->ps_AudioTrack)
                MPEGPlayerBase->mp_IconCountdown = ICON_DELAY_FOREVER;
        }
        else
        {
            DoIcon(MPEGPlayerBase,PLAY_ICON);
        }
    }

    return (paused);
}


/*****************************************************************************/


static ULONG DoSearch(struct MPEGPlayerLib *MPEGPlayerBase,
                      struct PlaySequence *current, ULONG mode)
{
struct IOStdReq  *ioReq;
struct IOMPEGReq *mpegReq;
BOOL              both;
LONG              num;

    both = FALSE;
    if (!current)
    {
        mode = CDMODE_NORMAL;
        both = TRUE;
    }

    if (both || current->ps_AudioTrack)
    {
        if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
        {
            if (mode == CDMODE_FFWD)
                CDGFastForward();
            else if (mode == CDMODE_FREV)
                CDGRewind();
            else
                CDGPlay(0);
        }

        ioReq             = MPEGPlayerBase->mp_AudioReq2;
        ioReq->io_Command = CD_SEARCH;
        ioReq->io_Data    = NULL;
        ioReq->io_Length  = mode;
        ioReq->io_Offset  = 0;
        DoIO(ioReq);
    }

    if (both || !current->ps_AudioTrack)
    {
        if (mode == CDMODE_NORMAL)
            num = 0;
        else if (mode == CDMODE_FREV)
            num = -450;
        else if (mode == CDMODE_FFWD)
            num = 450;

        mpegReq = MPEGPlayerBase->mp_MPEGReq2;
        if (mpegReq->iomr_Req.io_Device)
        {
            mpegReq->iomr_Req.io_Command = MPEGCMD_SCAN;
            mpegReq->iomr_Req.io_Data    = NULL;
            mpegReq->iomr_Req.io_Length  = 0;
            mpegReq->iomr_Req.io_Offset  = 0;
            mpegReq->iomr_SearchSpeed    = num;
            DoIO(mpegReq);
        }
    }

    if (mode == CDMODE_FFWD)
    {
        DoIcon(MPEGPlayerBase,FFWD_ICON);
        MPEGPlayerBase->mp_IconCountdown = ICON_DELAY_FOREVER;
    }
    else if (mode == CDMODE_FREV)
    {
        DoIcon(MPEGPlayerBase,FRWD_ICON);
        MPEGPlayerBase->mp_IconCountdown = ICON_DELAY_FOREVER;
    }
    else
    {
        DoIcon(MPEGPlayerBase,NOP_ICON);
    }

    return (mode);
}


/*****************************************************************************/


static void DoStep(struct MPEGPlayerLib *MPEGPlayerBase,
                   struct PlaySequence *current)
{
struct IOMPEGReq *mpegReq;

    if (current && !current->ps_AudioTrack)
    {
        mpegReq = MPEGPlayerBase->mp_MPEGReq2;
        if (mpegReq->iomr_Req.io_Device)
        {
            mpegReq->iomr_Req.io_Command = MPEGCMD_SINGLESTEP;
            mpegReq->iomr_Req.io_Data    = NULL;
            mpegReq->iomr_Req.io_Length  = 0;
            mpegReq->iomr_Req.io_Offset  = 0;
            DoIO(mpegReq);
        }
    }
}


/*****************************************************************************/


static ULONG DoSlowMotion(struct MPEGPlayerLib *MPEGPlayerBase,
                          struct PlaySequence *current, BOOL slowMotion)
{
struct IOMPEGReq *mpegReq;

    if (!current)
        slowMotion = FALSE;

    if (!current || !current->ps_AudioTrack)
    {
        mpegReq = MPEGPlayerBase->mp_MPEGReq2;
        if (mpegReq->iomr_Req.io_Device)
        {
            mpegReq->iomr_Req.io_Command = MPEGCMD_SLOWMOTION;
            mpegReq->iomr_Req.io_Data    = NULL;
            mpegReq->iomr_Req.io_Length  = 0;
            mpegReq->iomr_Req.io_Offset  = 0;
            mpegReq->iomr_SlowSpeed      = slowMotion ? 3 : 0;
            DoIO(mpegReq);
        }
    }

    return (slowMotion);
}


/*****************************************************************************/


static void DoBorder(struct MPEGPlayerLib *MPEGPlayerBase)
{
struct IOMPEGReq        *mpegReq;
struct MPEGBorderParams  bp;

    mpegReq = MPEGPlayerBase->mp_MPEGReq2;
    if (mpegReq->iomr_Req.io_Device)
    {
        bp.mbp_BorderLeft  = 0;
        bp.mbp_BorderTop   = 0;
        bp.mbp_BorderRed   = 17;
        bp.mbp_BorderGreen = 17;
        bp.mbp_BorderBlue  = 17;

        mpegReq->iomr_Req.io_Command = MPEGCMD_SETBORDER;
        mpegReq->iomr_Req.io_Data    = &bp;
        mpegReq->iomr_Req.io_Length  = sizeof(bp);
        mpegReq->iomr_Req.io_Offset  = 0;
        DoIO(mpegReq);
    }
}


/*****************************************************************************/


static void DoChannel(struct MPEGPlayerLib *MPEGPlayerBase,
                      struct PlaySequence *current, LONG delta)
{
struct IOMPEGReq       *mpegReq;
BOOL                    both;
struct MPEGAudioParams  map;

    both = FALSE;
    if (!current)
    {
        delta = 0;
        both  = TRUE;
    }

    if (both || current->ps_AudioTrack)
    {
        if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
        {
            if (MPEGPlayerBase->mp_CDGState >= CDG_HAVEGRAPHICS)
            {
                if (delta == 1)
                {
                    if (MPEGPlayerBase->mp_CDGChannel < 15)
                        MPEGPlayerBase->mp_CDGChannel++;
                    else
                        MPEGPlayerBase->mp_CDGChannel = 1;
                }
                else if (delta == -1)
                {
                    if (MPEGPlayerBase->mp_CDGChannel > 1)
                        MPEGPlayerBase->mp_CDGChannel--;
                    else
                        MPEGPlayerBase->mp_CDGChannel = 15;
                }
            }

            CDGChannel(MPEGPlayerBase->mp_CDGChannel);
        }
    }

    if (both || !current->ps_AudioTrack)
    {
        mpegReq = MPEGPlayerBase->mp_MPEGReq2;
        if (mpegReq->iomr_Req.io_Device)
        {
            if (delta == 1)
            {
                if (MPEGPlayerBase->mp_MPEGChannel < MC_NONE)
                    MPEGPlayerBase->mp_MPEGChannel++;
                else
                    MPEGPlayerBase->mp_MPEGChannel = MC_ONE;
            }
            else if (delta == -1)
            {
                if (MPEGPlayerBase->mp_MPEGChannel > MC_ONE)
                    MPEGPlayerBase->mp_MPEGChannel--;
                else
                    MPEGPlayerBase->mp_MPEGChannel = MC_NONE;
            }

            switch (MPEGPlayerBase->mp_MPEGChannel)
            {
                case MC_TWO : map.map_VolumeLeft  = 65535;
                              map.map_VolumeRight = 65535;
                              map.map_StreamID    = 1;
                              DoIcon(MPEGPlayerBase,CHANNEL2_ICON);
                              break;

                case MC_NONE: map.map_VolumeLeft  = 0;
                              map.map_VolumeRight = 0;
                              map.map_StreamID    = 0;
                              DoIcon(MPEGPlayerBase,MUTE_ICON);
                              break;

                default     :
                case MC_ONE : map.map_VolumeLeft  = 65535;
                              map.map_VolumeRight = 65535;
                              map.map_StreamID    = 0;
                              DoIcon(MPEGPlayerBase,CHANNEL1_ICON);
                              break;

            }

            mpegReq->iomr_Req.io_Command = MPEGCMD_SETAUDIOPARAMS;
            mpegReq->iomr_Req.io_Data    = &map;
            mpegReq->iomr_Req.io_Length  = sizeof(struct MPEGAudioParams);
            mpegReq->iomr_Req.io_Offset  = 0;
            DoIO(mpegReq);
        }
    }
}


/*****************************************************************************/


static enum States DoStop(struct MPEGPlayerLib *MPEGPlayerBase,
                          struct MinList *playList,
                          struct PlaySequence *current, ULONG options)
{
    DoStatus(MPEGPlayerBase,playList,NULL,0);
    DoIcon(MPEGPlayerBase,NOP_ICON);

    AbortMainIO(MPEGPlayerBase);

    DoScreen(MPEGPlayerBase,playList,current,options,TRUE);

    if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
    {
        CDGEnd();
        MPEGPlayerBase->mp_CDGState = CDG_UNAVAILABLE;
        RemakeDisplay();
    }

    DoPause(MPEGPlayerBase,current,FALSE,FALSE);
    DoSlowMotion(MPEGPlayerBase,current,FALSE);
    DoSearch(MPEGPlayerBase,current,CDMODE_NORMAL);
    FreePlayList(MPEGPlayerBase,playList);

    AbortTimerIO(MPEGPlayerBase);

    return (STATE_STOPPED);
}


/*****************************************************************************/


static struct PlaySequence *PlayNext(struct MPEGPlayerLib *MPEGPlayerBase,
                                     struct MinList *playList,
                                     struct PlaySequence *current, ULONG options,
                                     enum States state)
{
struct IOStdReq     *ioReq;
struct IOMPEGReq    *mpegReq;
struct PlaySequence *next;
struct PlaySequence *ps,*bps;
ULONG                startSector;
ULONG                setStartSector;
ULONG                endSector;
BOOL                 scan;

    D(("PlayNext() state=%ld current=$%lx start=%ld end=%ld\n",(ULONG)state,current,current->ps_StartSector,current->ps_EndSector));


    scan = TRUE;
    while(scan)
    {
	if (state == STATE_FREV)
    	{
        	if (current == (struct PlaySequence *)playList->mlh_Head)
        	{
            		next = NULL;
        	}
	        else if (!current)
        	{
	            next = (struct PlaySequence *)playList->mlh_TailPred;
        	}
	        else
        	{
	            next = (struct PlaySequence *)current->ps_Link.mln_Pred;
        	}
    	}
    	else
    	{
        	if (current == (struct PlaySequence *)playList->mlh_TailPred)
        	{
	    		D(("current == TailPred\n"));

            		next = NULL;
        	}
        	else if (!current)
        	{
	    		D(("next = mlh_Head\n"));
            		next = (struct PlaySequence *)playList->mlh_Head;
        	}
	        else
        	{
		    D(("next = mln_Succ\n"));
        	    next = (struct PlaySequence *)current->ps_Link.mln_Succ;
        	}
	}

	/* if INTRO mode, make sure we ended up on a track boundary
	 * else try next sequence until we run out out of sequences
	 * or whatever
	 *
	 * Gack, running out of time - this code still needs more work
	 * as it has no info or state telling it that the user has
	 * gotten here via pressing the left ear (REW) button once.
	 * If the previous track has a chapter mark in it, you cannot
	 * scan backward beyond the start of the current track.  Forunately
	 * scan is minimally useful, and at least it no longer spins
	 * replaying the same track (one with chapter marks).  It
	 * presumably works ok with Karaoke disks as they do not use
	 * chapter marks, and are more 'audio' disk-like.
	 */

	scan = FALSE;

	if (OPTF_INTRO & options)
	{
		if(next)
		{
			if(!(ISTRACK(next->ps_Reference)))
			{
				scan = TRUE;
				D(("PlayNext() - scan for next track\n"));
				current = next;
			}
		}
	}
    }

    if (next)
    {
        // Try to coallesse neighboring audio tracks so they play in one fell
        // swoop. This is important to CDs that have seperate tracks that
        // run into one another, like The Wall. If the coallessing is not
        // done, then gaps appear between the tracks when playing the whole
        // CD. Ick.
        //
        // We can only coallesse audio tracks, since the MPEG HW needs to
        // see individual tracks as separate entities.
        //
        // Now aside from tracks, we also deal with chapters. Chapters from
        // the same track are always coallessed.

        startSector = next->ps_StartSector;
        endSector   = next->ps_EndSector;
	setStartSector = startSector;

        bps = next;
	scan = TRUE;

    // if this is a chapter mark, scan back to a real track

	while(scan)
	{
		D(("bps = $%lx bps->ps_Reference->cds_Track = %ld\n",bps,(ULONG)bps->ps_Reference->cds_Track));

		if(!(ISTRACK(bps->ps_Reference)))
		{
			if(bps->ps_Link.mln_Pred)
			{
				bps = (struct PlaySequence *)bps->ps_Link.mln_Pred;
			}
			else
			{
				scan = FALSE;
			}
			setStartSector = bps->ps_StartSector;
		}
		else
		{
			scan = FALSE;
		}
	}


	D(("startSector = %ld  endSector = %ld\n",startSector,endSector));

        if (state == STATE_FREV)
        {
	    D(("STATE_FREV\n"));
            ps = (struct PlaySequence *)next->ps_Link.mln_Pred;


            while (ps->ps_Link.mln_Pred)
            {
	        D(("    ps = $%lx\n",ps));
		D(("    ps->ps_EndSector+1 = %ld startSector = %ld\n",ps->ps_EndSector+1,startSector));

                if (ps->ps_EndSector + 1 != startSector)
                    break;
 
                // if it's an MPEG track, and we just crossed a track boundary, don't coallesse
                D(("    ps->ps_Reference->cds_TrackEnd = %ld\n",ps->ps_Reference->cds_TrackEnd));
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
                if (!ps->ps_AudioTrack && (startSector > ps->ps_Reference->cds_TrackEnd))
                    break;
 
                startSector = ps->ps_StartSector;
                ps = (struct PlaySequence *)ps->ps_Link.mln_Pred;
		D(("  continue while loop\n"));
            }
	/* There is a potential bug in here in that the current
	 * sequence may be a chapter mark, and the end sector should
	 * be calculated up through the next real track.
	 *
	 * Since we get here as a result of holding down the left ear button,
	 * and scanning back past the beginning of a previous track,
	 * the final mark in the list is the end of the previous track.
	 * once that happens we can scan for/backward through the entire
	 * real track as much as we want.
	 *
	 * So given minimium time, I am going to let this bug slip through
	 * but it should be looked at later.  The key to this is that
	 * just jumping back a chapter is handled by the FFWD state below,
	 * which creates a PLAYLSN command encompassing the whole physical
	 * track.  Since you cannot scan back to a chapter mark which is
	 * not the end of a real track (or at least I dont think you can,
	 * if I find a case then this really should be fixed!), the problem
	 * should be moot (though its still a design oversight which occurs
	 * because chapter marks are mixed in with real tracks).
	 */

        }
        else
        {
	    D(("STATE_FFWD\n"));
            ps = (struct PlaySequence *)next->ps_Link.mln_Succ;

	    D(("ps=$%lx\n",ps));

            while (ps->ps_Link.mln_Succ)
            {
		D(("    ps->ps_StartSector = %ld endSector+1 = %ld\n",ps->ps_StartSector,endSector+1));

                if (ps->ps_StartSector != endSector+1)
                    break;
 
		D(("ps->ps_Reference->cds_Track = %ld\n",(ULONG)ps->ps_Reference->cds_Track));

                if (!ps->ps_AudioTrack && ISTRACK(ps->ps_Reference))
                    break;
 
                endSector = ps->ps_EndSector;
                ps = (struct PlaySequence *)ps->ps_Link.mln_Succ;
		D(("  continue while loop\n"));
            }

        }
 
        DoScreen(MPEGPlayerBase,playList,next,options,FALSE);

        MPEGPlayerBase->mp_LowSector  = setStartSector;
        MPEGPlayerBase->mp_HighSector = endSector;

        if (next->ps_AudioTrack)
        {
            if (MPEGPlayerBase->mp_CDGState == CDG_AVAILABLE)
            {
                CDGPlay(0);
                MPEGPlayerBase->mp_CDGState = CDG_SUBCODES;
            }

            ioReq             = MPEGPlayerBase->mp_AudioReq;
            ioReq->io_Command = CD_PLAYLSN;
            ioReq->io_Data    = NULL;
            ioReq->io_Length  = endSector - startSector + 1;
            ioReq->io_Offset  = startSector;

            SendMainIO(MPEGPlayerBase,ioReq);
        }
        else
        {
            mpegReq = MPEGPlayerBase->mp_MPEGReq;
            if (!mpegReq->iomr_Req.io_Device)
                return (NULL);

            mpegReq->iomr_Req.io_Command = MPEGCMD_PLAYLSN;
            mpegReq->iomr_Req.io_Data    = NULL;
            mpegReq->iomr_Req.io_Length  = endSector - startSector + 1;
            mpegReq->iomr_Req.io_Offset  = startSector;
            mpegReq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
            mpegReq->iomr_StreamStart    = setStartSector;
            mpegReq->iomr_SectorSize     = MPEG_SECTOR_SIZE;

	    D(("PLAY setStartSector = %ld startSector = %ld  endSector = %ld\n",setStartSector,startSector,endSector));

            SendMainIO(MPEGPlayerBase,mpegReq);
        }
    }

    return(next);
}


/*****************************************************************************/


static struct PlaySequence *SkipNext(struct MPEGPlayerLib *MPEGPlayerBase,
                                     struct PlaySequence *current, ULONG options,
				     enum States state)
{
struct PlaySequence *next;
struct IOMPEGReq *mpegReq;

    D(("SkipNext() state=%ld current=$%lx start=%ld end=%ld\n",(ULONG)state,current,current->ps_StartSector,current->ps_EndSector));

    // if we're not playing the last sequence, abort it, so the next
    // one will be scheduled.

    D(("current->ps_Link.mln_Succ->mln_Succ = $%lx\n",current->ps_Link.mln_Succ->mln_Succ));

    	if (current->ps_Link.mln_Succ->mln_Succ || (OPTF_LOOP & options))
    	{
        	if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
	            CDGNextTrack();

        	DoIcon(MPEGPlayerBase,NEXTTRACK_ICON);
	        MPEGPlayerBase->mp_IconCountdown = 75;

	        if (CheckMainIO(MPEGPlayerBase))
	        {

	/* This code was getting enforcer hits for the LOOP mode case, and
	 * spuriously jumping to the last sequence even though the disk
	 * was not playing which behaved as if the disk had ended.  Now
	 * I will treat getting behind as someone who is impatient, and wants
	 * to get to the last track on the disk
	 */

		    if(current->ps_Link.mln_Succ->mln_Succ)
		    {
		    	D(("no IO request\n"));
        	    	next = (struct PlaySequence *)current->ps_Link.mln_Succ;

			if(next->ps_Link.mln_Succ->mln_Succ)
			{
				D(("current = successor\n"));
				current = next;
			}
		    }
	        }
	        else
	        {

	/* What we are going to do here is check to see if the next sequence
	 * to be played is a track or a chapter mark.  If its a chapter
	 * mark, we will tell the cd32device driver to do seek, but if
	 * its a true track, we will abort the current track and let
	 * the state machine call PlayNext() to figure it out.  We assume
	 * the disk is not only playing, but that PlayNext() calculated a
	 * low/high sector value for us which encompasses the entire track.
	 * We then re-use that information, caring only about the chapter start
	 * position in the next sequence.
	 */


			if(current->ps_Link.mln_Succ->mln_Succ)
			{
				next = (struct PlaySequence *)current->ps_Link.mln_Succ;

				if (!(ISTRACK(next->ps_Reference)))
				{
       					mpegReq = MPEGPlayerBase->mp_MPEGReq2;
				        if (mpegReq->iomr_Req.io_Device)
				        {
						D(("low %ld high %ld\n",MPEGPlayerBase->mp_LowSector,MPEGPlayerBase->mp_HighSector));

						mpegReq->iomr_Req.io_Command = MPEGCMD_SEEKLSN;
						mpegReq->iomr_Req.io_Data    = NULL;
						mpegReq->iomr_Req.io_Length  = MPEGPlayerBase->mp_HighSector - next->ps_StartSector + 1;
						mpegReq->iomr_Req.io_Offset  = next->ps_StartSector;
						mpegReq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
						mpegReq->iomr_StreamStart    = MPEGPlayerBase->mp_LowSector;
						mpegReq->iomr_SectorSize     = MPEG_SECTOR_SIZE;

						D(("SEEKLSN offset %ld length %ld stream start %ld\n",mpegReq->iomr_Req.io_Offset,mpegReq->iomr_Req.io_Length,mpegReq->iomr_StreamStart));
						DoIO(mpegReq);
						return(next);
					}
				}
			}

			AbortMainIO(MPEGPlayerBase);
	        }
	}

    D(("current=$%lx start=%ld end=%ld\n",current,current->ps_StartSector,current->ps_EndSector));
    return (current);
}


/*****************************************************************************/


static struct PlaySequence *SkipBack(struct MPEGPlayerLib *MPEGPlayerBase,
                                     struct MinList *playList,
                                     struct PlaySequence *current, ULONG options,
                                     enum States state)
{
struct IOMPEGReq *mpegReq;
ULONG sector;
BOOL doSeek;

    D(("SkipBack() current=$%lx start=%ld end=%ld\n",current,current->ps_StartSector,current->ps_EndSector));

    // See where we are within the current track.
    //
    // If we're more than 4 seconds into the track, we should just
    // reset to the beginning of the current track. If we are less than
    // 4 seconds, then we jump back to the previous track. If the current
    // active track is the first one in the play list, we always skip to the
    // beginning of the track.

    if (current)
    {
	// assume we will not be using SEEKLSN; the only time we will do
	// so is when moving back within a track which has a chapter mark
	// before our current position, and we have previously sent
	// a PLAYLSN command whose bounds encompass the desired position

	doSeek = FALSE;
        if ((current != (struct PlaySequence *)playList->mlh_Head) || (OPTF_LOOP & options))
        {
            sector = GetCurrentSector(MPEGPlayerBase,current);
            if (sector == BAD_SECTOR)
                return(NULL);

	    if ((sector >= MPEGPlayerBase->mp_LowSector) && (sector < MPEGPlayerBase->mp_HighSector))
            {
		doSeek = TRUE;

                if (sector - current->ps_StartSector < 4 * SECTORS_PER_SECOND)
                {

                    if (current != (struct PlaySequence *)playList->mlh_Head)
		    {
                        current = (struct PlaySequence *)current->ps_Link.mln_Pred;

		    }
                    else
		    {
                        current = (struct PlaySequence *)playList->mlh_TailPred;
			doSeek = FALSE;
		    }
                }

		// check if this new position is a chapter mark, and if
		// falls within a previously sent PLAYLSN command

		D(("SkipBack to sequence $%lx\n",current));

		if(doSeek)
		{
			doSeek = FALSE;
			if(!(ISTRACK(current->ps_Reference)))
			{
				D(("low %ld high %ld\n",MPEGPlayerBase->mp_LowSector,MPEGPlayerBase->mp_HighSector));
				D(("desired position %ld\n",current->ps_StartSector));

				if(current->ps_StartSector >= MPEGPlayerBase->mp_LowSector)
				{
					if(current->ps_EndSector <= MPEGPlayerBase->mp_HighSector)
					{
						doSeek = TRUE;
					}
				}
			}
		}
            }
        }


        if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
            CDGPrevTrack();

        DoIcon(MPEGPlayerBase,PREVTRACK_ICON);
        MPEGPlayerBase->mp_IconCountdown = 75;

	if(doSeek)
	{
		mpegReq = MPEGPlayerBase->mp_MPEGReq2;
	        if (mpegReq->iomr_Req.io_Device)
      	        {
			mpegReq->iomr_Req.io_Command = MPEGCMD_SEEKLSN;
			mpegReq->iomr_Req.io_Data    = NULL;
			mpegReq->iomr_Req.io_Length  = MPEGPlayerBase->mp_HighSector - current->ps_StartSector + 1;
			mpegReq->iomr_Req.io_Offset  = current->ps_StartSector;
			mpegReq->iomr_StreamType     = MPEGSTREAM_SYSTEM;
			mpegReq->iomr_StreamStart    = MPEGPlayerBase->mp_LowSector;
			mpegReq->iomr_SectorSize     = MPEG_SECTOR_SIZE;
			
			D(("SEEKLSN offset %ld length %ld stream start %ld\n",mpegReq->iomr_Req.io_Offset,mpegReq->iomr_Req.io_Length,mpegReq->iomr_StreamStart));
			DoIO(mpegReq);
			return(current);
		}
	}

       	AbortMainIO(MPEGPlayerBase);
        if (current == (struct PlaySequence *)playList->mlh_Head)
       	{
            current = PlayNext(MPEGPlayerBase,playList,NULL,options,state);
       	}
        else
        {
	    D(("current = previous\n"));
            current = (struct PlaySequence *)current->ps_Link.mln_Pred;
       	}

    }

    D(("current=$%lx start=%ld end=%ld\n",current,current->ps_StartSector,current->ps_EndSector));
    return (current);
}


/*****************************************************************************/


ULONG DoPlay(struct MPEGPlayerLib *MPEGPlayerBase, struct CDDisk *disk,
             ULONG currentSelected, ULONG options)
{
struct MinList       playList;
ULONG                sigs;
ULONG                timerSig;
ULONG                removalSig;
ULONG                intuiSig;
ULONG                returnSig;
struct IntuiMessage *intuiMsg;
UWORD                icode;
UWORD                quals;
enum Commands        cmd;
enum States          state;
struct PlaySequence *current;
struct PlaySequence *ps,*oldps;
UBYTE                tickCount;
ULONG                numSelected;
struct CDSequence   *seq;

    NewList((struct List *)&playList);
    state     = STATE_STOPPED;
    cmd       = XCMD_PLAY;
    current   = NULL;
    tickCount = 0;

    removalSig = (1 << MPEGPlayerBase->mp_RemovalSigBit);
    intuiSig   = (1 << MPEGPlayerBase->mp_Window->UserPort->mp_SigBit);
    returnSig  = (1 << MPEGPlayerBase->mp_IOPort->mp_SigBit);
    timerSig   = (1 << MPEGPlayerBase->mp_TimerSigBit);

    SendTimerIO(MPEGPlayerBase);

    while (TRUE)
    {
        // there are a few stateless commands which we implement
        // here, instead of having to do 'em for every state down
        // below

        switch (cmd)
        {
            case XCMD_UP    : DoChannel(MPEGPlayerBase,current,1);
                              break;

            case XCMD_DOWN  : DoChannel(MPEGPlayerBase,current,-1);
                              break;

            case XCMD_LOOP  : options ^= OPTF_LOOP;
                              DoOptionIcon(MPEGPlayerBase,LOOP2_ICON,options,OPTF_LOOP);
                              break;

            case XCMD_INTRO : options ^= OPTF_INTRO;
                              DoOptionIcon(MPEGPlayerBase,INTRO2_ICON,options,OPTF_INTRO);
                              break;

            case XCMD_TOGGLE_TIME_CDG:
                              if (MPEGPlayerBase->mp_CDGState >= CDG_HAVEGRAPHICS)
                              {
                                  options ^= OPTF_SHOWCDG;
                                  if (OPTF_SHOWCDG & options)
                                  {
                                      CDGFront();
                                      MPEGPlayerBase->mp_CDGState = CDG_SHOWGRAPHICS;
                                  }
                                  else
                                  {
                                      CDGBack();
                                      (*MPEGPlayerBase->mp_ScreenDepth)(MPEGPlayerBase->mp_PlayScreen,SDEPTH_TOFRONT,0,IntuitionBase);
                                      MPEGPlayerBase->mp_CDGState = CDG_HAVEGRAPHICS;
                                  }
                                  DoScreen(MPEGPlayerBase,&playList,current,options,FALSE);
                              }
                              else
                              {
                                  if (options & OPTF_TIMEDEFAULT)
                                  {
                                      options &= (~OPTF_TIMEDEFAULT);
                                      if (current->ps_AudioTrack)
                                          options |= OPTF_TIMEOVERLAY;
                                  }
                                  options ^= OPTF_TIMEOVERLAY;
                                  DoStatus(MPEGPlayerBase,&playList,current,options);
                              }
                              break;

            case XCMD_STOP  : DoStop(MPEGPlayerBase,&playList,current,options);
                              return (options);

        }

        switch (state)
        {
/***** STATE_STOPPED *****/
            case STATE_STOPPED:
            switch (cmd)
            {
                case XCMD_PLAY: DoStop(MPEGPlayerBase,&playList,current,options);
                                MakePlayList(MPEGPlayerBase,disk,&playList);

                                numSelected = 0;
                                SCANLIST(&disk->cdd_Sequences,seq)
                                {
                                    if (seq->cds_Selected)
                                        numSelected++;
                                }


				D(("numSelected = %ld currentSelected = %ld\n",(ULONG)numSelected,(ULONG)currentSelected));

                                ps = NULL;
				oldps = NULL;

                                if (numSelected == 0)
                                {
                                    if (currentSelected)
                                    {
                                        SCANLIST(&playList,ps)
                                        {
					    D(("ps->ps_StartSector = %ld ps->ps_EndSector = %ld\n",ps->ps_StartSector,ps->ps_EndSector));

					    if(ISTRACK(ps->ps_Reference))
					    {
						oldps = ps;
                                            	if (!currentSelected)
                                                	break;

						D(("currentSelected--\n"));

                                            	currentSelected--;
					    }
                                        }
					D(("ps = $%lx oldps = $%lx\n",ps,oldps));
					if(oldps) ps = oldps;
	                                ps = (struct PlaySequence *)ps->ps_Link.mln_Pred;
                                    }
                                }

                                if (CDGBegin(MPEGPlayerBase->mp_CDGPrefs))
                                    MPEGPlayerBase->mp_CDGState = CDG_AVAILABLE;

                                DoChannel(MPEGPlayerBase,current,0);
                                DoBorder(MPEGPlayerBase);

                                state = STATE_PLAYING;
                                current = PlayNext(MPEGPlayerBase,&playList,ps,options,state);
                		break;
            }
            break;

/***** STATE_PAUSED *****/
            case STATE_PAUSED:
            switch (cmd)
            {
                case XCMD_RANDOM:
                options ^= OPTF_RANDOM;
                MPEGPlayerBase->mp_RandomModeChanged = TRUE;
                OrganizeSequences(MPEGPlayerBase,disk,-1,options);
                MakePlayList(MPEGPlayerBase,disk,&playList);
                AbortMainIO(MPEGPlayerBase);
                current = PlayNext(MPEGPlayerBase,&playList,NULL,options,state);
                DoOptionIcon(MPEGPlayerBase,RANDOM2_ICON,options,OPTF_RANDOM);
            	break;

                case XCMD_PLAY:
                if (!DoPause(MPEGPlayerBase,current,FALSE,TRUE))
                    state = STATE_PLAYING;
                break;

                case XCMD_FFWD_DOWN:
                state = STATE_FFWD_WAIT_PAUSED;
                break;

                case XCMD_FREV_DOWN:
                state = STATE_FREV_WAIT_PAUSED;
                break;
            }
            break;

/***** STATE_PLAYING *****/
            case STATE_PLAYING:
            switch (cmd)
            {
                case XCMD_RANDOM:
                options ^= OPTF_RANDOM;
                MPEGPlayerBase->mp_RandomModeChanged = TRUE;
                OrganizeSequences(MPEGPlayerBase,disk,-1,options);
                MakePlayList(MPEGPlayerBase,disk,&playList);
                AbortMainIO(MPEGPlayerBase);
                current = PlayNext(MPEGPlayerBase,&playList,NULL,options,state);
                DoOptionIcon(MPEGPlayerBase,RANDOM2_ICON,options,OPTF_RANDOM);
            	break;

                case XCMD_PLAY:
                if (DoPause(MPEGPlayerBase,current,TRUE,TRUE))
                    state = STATE_PAUSED;
                break;

                case XCMD_FFWD_DOWN:
                state = STATE_FFWD_WAIT;
                break;

                case XCMD_FREV_DOWN:
                state = STATE_FREV_WAIT;
                break;
            }
            break;

/***** STATE_FFWD_WAIT *****/
            case STATE_FFWD_WAIT:
            switch (cmd)
            {
                case XCMD_FFWD_UP:
                current = SkipNext(MPEGPlayerBase,current,options,state);
                state = STATE_PLAYING;
                break;

                case XCMD_FFWD_DOWN:
                DoSearch(MPEGPlayerBase,current,CDMODE_FFWD);
                state = STATE_FFWD;
                break;

                case XCMD_FREV_DOWN:
                options ^= OPTF_INTRO;
                DoOptionIcon(MPEGPlayerBase,INTRO2_ICON,options,OPTF_INTRO);
                state = STATE_PLAYING;
                break;
            }
            break;

/***** STATE_FREV_WAIT *****/
            case STATE_FREV_WAIT:
            switch (cmd)
            {
                case XCMD_FREV_UP:
                state = STATE_PLAYING;
                current = SkipBack(MPEGPlayerBase,&playList,current,options,state);
                break;

                case XCMD_FREV_DOWN:
                DoSearch(MPEGPlayerBase,current,CDMODE_FREV);
                state = STATE_FREV;
                break;

                case XCMD_FFWD_DOWN:
                options ^= OPTF_INTRO;
                DoOptionIcon(MPEGPlayerBase,INTRO2_ICON,options,OPTF_INTRO);
                state = STATE_PLAYING;
                break;
            }
            break;

/***** STATE_FFWD_WAIT_PAUSED *****/
            case STATE_FFWD_WAIT_PAUSED:
            switch (cmd)
            {
                case XCMD_FFWD_UP:
                if (!current->ps_AudioTrack)
                {
                    DoStep(MPEGPlayerBase,current);
                }
                state = STATE_PAUSED;
                break;

                case XCMD_FFWD_DOWN:
                if (!current->ps_AudioTrack)
                {
                    DoSlowMotion(MPEGPlayerBase,current,TRUE);
                    DoPause(MPEGPlayerBase,current,FALSE,FALSE);
                    state = STATE_SLOWMOTION_FWD;
                }
                else
                {
                    state = STATE_PAUSED;
                }
                break;

                case XCMD_FREV_DOWN:
                options ^= OPTF_INTRO;
                DoOptionIcon(MPEGPlayerBase,INTRO2_ICON,options,OPTF_INTRO);
                state = STATE_PAUSED;
                break;
            }
            break;

/***** STATE_FREV_WAIT_PAUSED *****/
            case STATE_FREV_WAIT_PAUSED:
            switch (cmd)
            {
                case XCMD_FREV_UP:
                if (!current->ps_AudioTrack)
                {
                    // we should do single step backwards, but the HW doesn't do that
                }
                state = STATE_PAUSED;
                break;

                case XCMD_FREV_DOWN:
                if (!current->ps_AudioTrack)
                {
                    // we should do reverse slow motion, but the HW doesn't do that
                }
                state = STATE_PAUSED;
                break;

                case XCMD_FFWD_DOWN:
                options ^= OPTF_INTRO;
                DoOptionIcon(MPEGPlayerBase,INTRO2_ICON,options,OPTF_INTRO);
                state = STATE_PAUSED;
                break;
            }
            break;

/***** STATE_FFWD *****/
            case STATE_FFWD:
            switch (cmd)
            {
                case XCMD_FFWD_UP:
                DoSearch(MPEGPlayerBase,current,CDMODE_NORMAL);
                state = STATE_PLAYING;
                break;
            }
            break;

/***** STATE_FREV *****/
            case STATE_FREV:
            switch (cmd)
            {
                case XCMD_FREV_UP:
                DoSearch(MPEGPlayerBase,current,CDMODE_NORMAL);
                state = STATE_PLAYING;
                break;
            }
            break;

/***** STATE_SLOWMOTION_FWD *****/
	    case STATE_SLOWMOTION_FWD:
	    switch (cmd)
	    {
	        case XCMD_FFWD_UP:
                DoPause(MPEGPlayerBase,current,TRUE,FALSE);
	        DoSlowMotion(MPEGPlayerBase,current,FALSE);
	        state = STATE_PAUSED;
	        break;
	    }
	    break;
        }

        if (current)
        {
            cmd = XCMD_NOP;

            if (intuiMsg = (struct IntuiMessage *)GetMsg(MPEGPlayerBase->mp_Window->UserPort))
            {
                icode = intuiMsg->Code;
                quals = intuiMsg->Qualifier;

                switch (intuiMsg->Class)
                {
                    case IDCMP_RAWKEY:
                    switch (icode)
                    {
                        case RAWKEY_PORT0_JOY_UP:
                        case RAWKEY_PORT1_JOY_UP:
                        case RAWKEY_PORT2_JOY_UP:
                        case RAWKEY_PORT3_JOY_UP:
                        case 0x4c               : cmd = XCMD_UP;
                                                  break;

                        case RAWKEY_PORT0_JOY_DOWN:
                        case RAWKEY_PORT1_JOY_DOWN:
                        case RAWKEY_PORT2_JOY_DOWN:
                        case RAWKEY_PORT3_JOY_DOWN:
                        case 0x4d                 : cmd = XCMD_DOWN;
                                                    break;

                        case RAWKEY_PORT0_BUTTON_YELLOW:
                        case RAWKEY_PORT1_BUTTON_YELLOW:
                        case RAWKEY_PORT2_BUTTON_YELLOW:
                        case RAWKEY_PORT3_BUTTON_YELLOW: if (!(IEQUALIFIER_REPEAT & quals))
                                                             cmd = XCMD_LOOP;
                                                         break;

                        case RAWKEY_PORT0_BUTTON_GREEN:
                        case RAWKEY_PORT1_BUTTON_GREEN:
                        case RAWKEY_PORT2_BUTTON_GREEN:
                        case RAWKEY_PORT3_BUTTON_GREEN: if (!(IEQUALIFIER_REPEAT & quals))
                                                            cmd = XCMD_RANDOM;
                                                        break;

                        case 0x45    /* ESC key */   :
                        case RAWKEY_PORT0_BUTTON_BLUE:
                        case RAWKEY_PORT1_BUTTON_BLUE:
                        case RAWKEY_PORT2_BUTTON_BLUE:
                        case RAWKEY_PORT3_BUTTON_BLUE: cmd = XCMD_STOP;
                                                       break;

                        case RAWKEY_PORT0_BUTTON_RED:
                        case RAWKEY_PORT1_BUTTON_RED:
                        case RAWKEY_PORT2_BUTTON_RED:
                        case RAWKEY_PORT3_BUTTON_RED: if (!(IEQUALIFIER_REPEAT & quals))
                                                          cmd = XCMD_TOGGLE_TIME_CDG;
                                                      break;

                        case RAWKEY_PORT0_BUTTON_PLAY:
                        case RAWKEY_PORT1_BUTTON_PLAY:
                        case RAWKEY_PORT2_BUTTON_PLAY:
                        case RAWKEY_PORT3_BUTTON_PLAY: if (!(IEQUALIFIER_REPEAT & quals))
                                                           cmd = XCMD_PLAY;
                                                       break;

                        case RAWKEY_PORT0_BUTTON_FORWARD:
                        case RAWKEY_PORT1_BUTTON_FORWARD:
                        case RAWKEY_PORT2_BUTTON_FORWARD:
                        case RAWKEY_PORT3_BUTTON_FORWARD: cmd = XCMD_FFWD_DOWN;
                                                          break;

                        case (0x80 | RAWKEY_PORT0_BUTTON_FORWARD):
                        case (0x80 | RAWKEY_PORT1_BUTTON_FORWARD):
                        case (0x80 | RAWKEY_PORT2_BUTTON_FORWARD):
                        case (0x80 | RAWKEY_PORT3_BUTTON_FORWARD): cmd = XCMD_FFWD_UP;
                                                                   break;

                        case RAWKEY_PORT0_BUTTON_REVERSE:
                        case RAWKEY_PORT1_BUTTON_REVERSE:
                        case RAWKEY_PORT2_BUTTON_REVERSE:
                        case RAWKEY_PORT3_BUTTON_REVERSE: cmd = XCMD_FREV_DOWN;
                                                          break;

                        case (0x80 | RAWKEY_PORT0_BUTTON_REVERSE):
                        case (0x80 | RAWKEY_PORT1_BUTTON_REVERSE):
                        case (0x80 | RAWKEY_PORT2_BUTTON_REVERSE):
                        case (0x80 | RAWKEY_PORT3_BUTTON_REVERSE): cmd = XCMD_FREV_UP;
                                                                   break;
                    }
                    break;
                }

                ReplyMsg(intuiMsg);
            }
            else
            {
                if (!(SetSignal(0,0) & removalSig))
                {
                    if (CheckMainIO(MPEGPlayerBase))
                    {
			D(("returned IO\n"));
                        current = PlayNext(MPEGPlayerBase,&playList,current,options,state);

                        if (!current && (OPTF_LOOP & options))
                            current = PlayNext(MPEGPlayerBase,&playList,NULL,options,state);
                    }
                }

                sigs = Wait(intuiSig | removalSig | returnSig | timerSig);

                if (removalSig & sigs)
                {
                    SetSignal(removalSig,removalSig);
                    current = NULL;

                    if (MPEGPlayerBase->mp_CDGState > CDG_UNAVAILABLE)
                        CDGDiskRemoved();

                    cmd = XCMD_STOP;
                }
                else
                {
                    if (timerSig & sigs)
                    {
                        tickCount++;
                        WaitTimerIO(MPEGPlayerBase);
                        SendTimerIO(MPEGPlayerBase);

                        if (MPEGPlayerBase->mp_IconCountdown)
                        {
                            MPEGPlayerBase->mp_IconCountdown--;
                            if (!MPEGPlayerBase->mp_IconCountdown)
                            {
                                DoIcon(MPEGPlayerBase,NOP_ICON);
                            }
                        }

                        current = GetCurrentSequence(MPEGPlayerBase,current,options);

                        if (current)
                        {
                            if (MPEGPlayerBase->mp_CDGState >= CDG_SUBCODES)
                            {
                                if (CDGF_GRAPHICS & CDGDraw(CDGF_GRAPHICS|CDGF_MIDI))
                                {
                                    if (OPTF_SHOWCDG & options)
                                    {
                                        if (MPEGPlayerBase->mp_CDGState < CDG_SHOWGRAPHICS)
                                        {
                                            StopSpliner(MPEGPlayerBase);
                                            if (state == STATE_PAUSED)
                                            {
                                                CDGClearScreen();
                                                CDGPause();
                                            }
                                            else if (state == STATE_FFWD)
                                            {
                                                CDGFastForward();
                                            }
                                            else if (state == STATE_FREV)
                                            {
                                                CDGRewind();
                                            }
                                            else
                                            {
                                                CDGClearScreen();
                                            }

                                            CDGFront();
                                            MPEGPlayerBase->mp_CDGState = CDG_SHOWGRAPHICS;
                                        }
                                    }
                                    else
                                    {
                                        MPEGPlayerBase->mp_CDGState = CDG_HAVEGRAPHICS;
                                    }
                                }
                            }
                        }

                        if (tickCount > UPDATE_DELAY)
                        {
                            DoStatus(MPEGPlayerBase,&playList,current,options);
                            tickCount = 0;
                        }
                    }
                }
            }
        }

        if (!current)
	{
	    D(("current = NULL\n"));
            cmd = XCMD_STOP;
	}
    }
}
