
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <libraries/lowlevel.h>
#include <libraries/videocd.h>
#include <devices/cd.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "utils.h"
#include "tracks.h"
#include "diskinfo.h"
#include "defs.h"
#include "mpegplayerbase.h"
#include "text.h"
#include "credits.h"
#include "play.h"
#include "thumbnail.h"
#include "eventloop.h"

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
    CMD_NOP,
    CMD_BOGUS,
    CMD_UP,
    CMD_DOWN,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_DISKCHANGE,
    CMD_SELECT_DOWN,
    CMD_SELECT_UP,
    CMD_LOOP,
    CMD_RANDOM,
    CMD_INTRO,
    CMD_PLAY,
    CMD_NEXTTRACK,
    CMD_PREVTRACK,
    CMD_EXIT
};

const struct Rectangle __far iconsPos[] =
{
    {97,0,137,23},    /* intro icon      */
    {139,0,179,25},   /* random icon     */
    {181,0,216,24},   /* loop icon       */
    {218,0,238,22},   /* checkmark icon  */

    {1,29,88,87},     /* mute icon       */
    {90,29,177,87},   /* channel 1 icon  */
    {179,29,266,87},  /* channel 2 icon  */
    {268,29,355,87},  /* intro2 icon     */
    {357,29,444,87},  /* random2 icon    */
    {446,29,533,87},  /* loop2 icon      */
    {1,89,90,147},    /* play icon       */
    {90,89,179,147},  /* pause icon      */
    {179,89,266,147}, /* ffwd icon       */
    {268,89,355,147}, /* frwd icon       */
    {357,89,444,147}, /* next track icon */
    {446,89,533,147}, /* prev track icon */
    {0,152,341,209},  /* time icon */

    {244,3,281,20},   /* intro3 icon     */
    {286,0,323,22},   /* random3 icon    */
    {328,2,360,21}    /* loop3 icon      */
};


/*****************************************************************************/


static struct CDSequence *FindTrack(struct CDDisk *disk, ULONG trackNum)
{
struct CDSequence *seq;

    if (!disk)
        return (NULL);

    seq = (struct CDSequence *)disk->cdd_Sequences.mlh_Head;
    while (TRUE)
    {
        if (ISTRACK(seq))
        {
            if (!trackNum)
                return (seq);

            trackNum--;
        }
        seq = (struct CDSequence *)seq->cds_Link.ln_Succ;
    }
}


/*****************************************************************************/


ULONG OrganizeSequences(struct MPEGPlayerLib *MPEGPlayerBase,
	                struct CDDisk *disk, LONG currentTrack,
                        ULONG options)
{
struct CDSequence *seq;
struct CDSequence *next;
struct CDSequence *new;
struct CDSequence *tempseq;
ULONG              seed;
ULONG              numTracks;
ULONG              i;
struct MinList     temp;

    D(("OrganizeSequences()\n"));

    if (!disk || !MPEGPlayerBase->mp_RandomModeChanged)
        return ((ULONG)currentTrack);

    NewList((struct List *)&temp);

    if (OPTF_RANDOM & options)
    {
        numTracks = 0;
        SCANLIST(&disk->cdd_Sequences,seq)
        {
            if (ISTRACK(seq))
                numTracks++;
        }

	D(("numTracks=%ld\n",(ULONG)numTracks));

        seed = 0;
        while (numTracks)
        {
            i = Random(numTracks,&seed) % numTracks;
            SCANLIST(&disk->cdd_Sequences,seq)
            {
                if (ISTRACK(seq))
                {
                    if (i == 0)
                        break;

                    i--;
                }
            }

            next = (struct CDSequence *)seq->cds_Link.ln_Succ;

	D(("seq->cds_StartSector=%ld seq->cds_EndSector=%ld\n",seq->cds_StartSector,seq->cds_EndSector));
	D(("next->cds_StartSector=%ld next->cds_EndSector=%ld\n",next->cds_StartSector,next->cds_EndSector));

            Remove((struct Node *)seq);
            if (seed & 1)
                AddTail((struct List *)&temp,(struct Node *)seq);
            else
                AddHead((struct List *)&temp,(struct Node *)seq);

            // also do the chapters associated with the track...
	    tempseq = next;
            while (next)
            {

	D(("tempseq = $%lx\n",tempseq));

                if (!tempseq)
                    break;

	D(("next->cds_TrackNumber %ld seq->cds_TrackNumber %ld\n",next->cds_TrackNumber,seq->cds_TrackNumber));

                if (seq->cds_TrackNumber != next->cds_TrackNumber)
                    break;

		tempseq = (struct CDSequence *)next->cds_Link.ln_Succ;

                Remove((struct Node *)next);
                Insert((struct List *)&temp,(struct Node *)next,(struct Node *)seq);

                seq = next;

		next = tempseq;

	D(("seq->cds_StartSector=%ld seq->cds_EndSector=%ld\n",seq->cds_StartSector,seq->cds_EndSector));
	D(("next->cds_StartSector=%ld next->cds_EndSector=%ld\n",next->cds_StartSector,next->cds_EndSector));

            }

            numTracks--;
        }
    }
    else
    {
        // sort the sequence list...
        while (new = (struct CDSequence *)RemHead((struct List *)&disk->cdd_Sequences))
        {
            SCANLIST(&temp,seq)
            {
                if (seq->cds_TrackNumber > new->cds_TrackNumber)
                    break;
            }
            Insert((struct List *)&temp,(struct Node *)new,(struct Node *)seq->cds_Link.ln_Pred);

            // also do the chapters associated with the track...
            while (TRUE)
            {
                seq = new;

                if (!(new = (struct CDSequence *)RemHead((struct List *)&disk->cdd_Sequences)))
                    break;

                if (new->cds_TrackNumber != seq->cds_TrackNumber)
                {
                    AddHead((struct List *)&disk->cdd_Sequences,new);
                    break;
                }

                Insert((struct List *)&temp,(struct Node *)new,(struct Node *)seq);
            }
        }
    }

    while (seq = (struct CDSequence *)RemHead((struct List *)&temp))
        AddTail((struct List *)&disk->cdd_Sequences,(struct Node *)seq);

    if (currentTrack >= 0)
    {
        if (MPEGPlayerBase->mp_CurrentPage == PAGETYPE_TRACKS)
        {
            DeleteTracks(MPEGPlayerBase);
            CreateTracks(MPEGPlayerBase,disk);
            currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_NOP);
        }

        MPEGPlayerBase->mp_RandomModeChanged = FALSE;
    }

    return ((ULONG)currentTrack);
}


/*****************************************************************************/


// select a track, and all its chapters...
static void SelectTrack(struct MPEGPlayerLib *MPEGPlayerBase,
                        struct CDDisk *disk, ULONG trackNum, BOOL selected)
{
struct CDSequence *seq;
struct CDSequence *parent;

    if (!disk)
        return;

    if (disk->cdd_NumTracks > 1)
    {
        parent = FindTrack(disk,trackNum);
        seq    = parent;
        while (TRUE)
        {
            seq->cds_Selected = selected;

            seq = (struct CDSequence *)seq->cds_Link.ln_Succ;
            if (!seq->cds_Link.ln_Succ || ISTRACK(seq))
            {
                if (MPEGPlayerBase->mp_CurrentPage == PAGETYPE_TRACKS)
                    TrackState(MPEGPlayerBase,trackNum,selected);

                else if (MPEGPlayerBase->mp_CurrentPage == PAGETYPE_TRACKCREDITS)
                    DrawCreditTitle(MPEGPlayerBase,parent);

                return;
            }
        }
    }
}


/*****************************************************************************/


static void PlopIcon(struct MPEGPlayerLib *MPEGPlayerBase,
		     enum Icons which, WORD x)
{
WORD x0, y0, w, h;

    x0 = iconsPos[which].MinX;
    y0 = iconsPos[which].MinY;
    w  = iconsPos[which].MaxX - x0 + 1;
    h  = iconsPos[which].MaxY - y0 + 1;

    BltBitMap(MPEGPlayerBase->mp_IconBitMap,x0,y0,
    	      MPEGPlayerBase->mp_RenderRP->BitMap,x,ICONS_TOP+5,
              w,h,0xc0,0xff,NULL);
}


/*****************************************************************************/


static void DrawState(struct MPEGPlayerLib *MPEGPlayerBase,
                      struct CDDisk *disk, ULONG currentTrack, ULONG options)
{
struct CDSequence *seq;
enum PageTypes     page;
WORD               x;
ULONG              sectors;
char               buffer[30];
ULONG              len;
BOOL               doRightArrow;

    // clear out time display...
    SetABPenDrMd(MPEGPlayerBase->mp_RenderRP,0,0,JAM1);
    RectFill(MPEGPlayerBase->mp_RenderRP,TIME_LEFT,TIME_TOP,TIME_RIGHT,TIME_BOTTOM);
    RectFill(MPEGPlayerBase->mp_RenderRP,ICONS_LEFT,ICONS_TOP,ICONS_RIGHT,ICONS_BOTTOM);

    BltBitMap(MPEGPlayerBase->mp_BgBitMap,PREV_LEFT,PREV_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,PREV_LEFT,PREV_TOP,
              PREV_WIDTH,PREV_HEIGHT,0xc0,0xff,NULL);

    BltBitMap(MPEGPlayerBase->mp_BgBitMap,NEXT_LEFT,NEXT_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,NEXT_LEFT,NEXT_TOP,
              NEXT_WIDTH,NEXT_HEIGHT,0xc0,0xff,NULL);

    if (disk)
    {
        // count how many selected sectors...
        sectors = 0;
        SCANLIST(&disk->cdd_Sequences,seq)
        {
            if (seq->cds_Selected)
                sectors += (seq->cds_EndSector - seq->cds_StartSector + 1);
        }

        // if nothing was selected, do current track to last...
        if (sectors == 0)
        {
            seq = FindTrack(disk,currentTrack);
            while (seq->cds_Link.ln_Succ)
            {
                sectors += (seq->cds_EndSector - seq->cds_StartSector + 1);
                seq      = (struct CDSequence *)seq->cds_Link.ln_Succ;
            }
        }

        len = nprintf(buffer,sizeof(buffer),"%2ld:%02ld",sectors / SECTORS_PER_MINUTE, (sectors / SECTORS_PER_SECOND) % 60);

        Move(MPEGPlayerBase->mp_RenderRP,TIME_LEFT,TIME_TOP + 31);
        SetABPenDrMd(MPEGPlayerBase->mp_RenderRP,3,2,JAM1);
        SetFont(MPEGPlayerBase->mp_RenderRP,&MPEGPlayerBase->mp_Compact31);
        CenterText(MPEGPlayerBase,MPEGPlayerBase->mp_RenderRP,
                   buffer,len,TIME_WIDTH);

        page = MPEGPlayerBase->mp_CurrentPage;

        if(MPEGPlayerBase->mp_DrawArrows)
        {
                // do we need a previous page arrow?
                if ((page > PAGETYPE_TRACKS)
                ||  (disk->cdd_Notes && (page == PAGETYPE_TRACKS))
                ||  (MPEGPlayerBase->mp_AudioLayout && (MPEGPlayerBase->mp_OldLeft > 0)))
                {
                    BltBitMap(MPEGPlayerBase->mp_ArrowBitMap,0,15,
                              MPEGPlayerBase->mp_RenderRP->BitMap,PREV_LEFT,PREV_TOP,
                              PREV_WIDTH,PREV_HEIGHT,0xc0,0xff,NULL);
                }

                // do we need a next page arrow?
                if (page <= PAGETYPE_TRACKLYRICS)
                {

                // we draw a next page arrow if there are too many tracks to fit
                // on this page, or this is a Karaoke disk which we ASSUME has some
                // textual information and/or related information which we can display

                     doRightArrow = FALSE;

                     if((MPEGPlayerBase->mp_OldLeft + 3)*VISIBLE_AUDIO_SLOTS < disk->cdd_NumTracks)
                     {
                        doRightArrow = TRUE;
                     }
                     if(!MPEGPlayerBase->mp_AudioLayout && MPEGPlayerBase->mp_VideoCDType == CDT_KARAOKE)
                     {
                        doRightArrow = TRUE;
                     }

                     if(doRightArrow)
                     {
                         BltBitMap(MPEGPlayerBase->mp_ArrowBitMap,0,0,
                              MPEGPlayerBase->mp_RenderRP->BitMap,NEXT_LEFT,NEXT_TOP,
                              NEXT_WIDTH,NEXT_HEIGHT,0xc0,0xff,NULL);
                     }
                }
        }
    }

    x = ICONS_LEFT + 4 + (iconsPos[LOOP_ICON].MaxX - iconsPos[LOOP_ICON].MinX) + 26;
    if (OPTF_LOOP & options)
        PlopIcon(MPEGPlayerBase,LOOP_ICON,x);

    x += (iconsPos[RANDOM_ICON].MaxX - iconsPos[RANDOM_ICON].MinX) + 26;
    if (OPTF_RANDOM & options)
        PlopIcon(MPEGPlayerBase,RANDOM_ICON,x);

    x += (iconsPos[INTRO_ICON].MaxX - iconsPos[INTRO_ICON].MinX) + 26;
    if (OPTF_INTRO & options)
        PlopIcon(MPEGPlayerBase,INTRO_ICON,x);

    SwapBuffers(MPEGPlayerBase);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],TIME_LEFT,TIME_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,TIME_LEFT,TIME_TOP,
              TIME_WIDTH,TIME_HEIGHT,0xc0,0x03,NULL);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],ICONS_LEFT,ICONS_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,ICONS_LEFT,ICONS_TOP,
              ICONS_WIDTH,ICONS_HEIGHT,0xc0,0x03,NULL);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],PREV_LEFT,PREV_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,PREV_LEFT,PREV_TOP,
              PREV_WIDTH,PREV_HEIGHT,0xc0,0xff,NULL);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],NEXT_LEFT,NEXT_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,NEXT_LEFT,NEXT_TOP,
              NEXT_WIDTH,NEXT_HEIGHT,0xc0,0xff,NULL);
}


/*****************************************************************************/


extern UBYTE * __far background;


/*****************************************************************************/


static void DrawPage(struct MPEGPlayerLib *MPEGPlayerBase,
		     struct CDDisk *disk, ULONG trackNum, ULONG options,
                     BOOL first)
{
struct BMInfo     *bmInfo;
struct ColorPack  *table;
UWORD              i;
ULONG              j;
struct RGBEntry   *ptr;

    if (!first)
    {
        SetABPenDrMd(MPEGPlayerBase->mp_RenderRP,0,0,JAM1);
        RectFill(MPEGPlayerBase->mp_RenderRP,TITLE_LEFT,TITLE_TOP,TITLE_RIGHT,TITLE_BOTTOM);
    }

    if (!disk || !first)
    {
        BltBitMap(MPEGPlayerBase->mp_BgBitMap,PAGE_LEFT,PAGE_TOP,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT,PAGE_TOP,
                  PAGE_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);
    }

    DrawState(MPEGPlayerBase,disk,trackNum,options);

    if (first)
    {
        bmInfo = DecompBMInfo(NULL, NULL, &background);
        table  = (struct ColorPack *)GetBMInfoRGB32(bmInfo,64,0);
        ptr    = &table->cp_Colors[0];

        for (i = 0; i < 64; i++)
            MPEGPlayerBase->mp_BgColors.cp_Colors[i] = *ptr++;

        // build a grey scale...
        for (i = 64; i < 128; i++)
        {
            j = (i - 64) * 4 + ((i-64) / 8);
            j = (j << 24) | (j << 16) | (j << 8) | j;
            MPEGPlayerBase->mp_BgColors.cp_Colors[i].Red   = j;
            MPEGPlayerBase->mp_BgColors.cp_Colors[i].Green = j;
            MPEGPlayerBase->mp_BgColors.cp_Colors[i].Blue  = j;
        }

        FreeBMInfoRGB32((ULONG *)table);
        FreeBMInfo(bmInfo);

        MPEGPlayerBase->mp_BgColors.cp_NumColors = NUM_COLORS;
        MPEGPlayerBase->mp_BgColors.cp_EndMarker = 0;

        FadeIn(MPEGPlayerBase,FALSE);
    }

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1-MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT,PAGE_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT,PAGE_TOP,
              PAGE_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1-MPEGPlayerBase->mp_DBCurrentBuffer],TITLE_LEFT,TITLE_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,TITLE_LEFT,TITLE_TOP,
              TITLE_WIDTH,TITLE_HEIGHT,0xc0,0x03,NULL);
}


/*****************************************************************************/


#define CHANGEPAGE(p)     {MPEGPlayerBase->mp_CurrentPage = p; selectDown = FALSE;}
#define TICKS_BEFORE_FADE 3000   /* 5 minutes */


VOID EventLoop(struct MPEGPlayerLib *MPEGPlayerBase)
{
ULONG                removalSig;
ULONG                intuiSig;
ULONG                sigs;
struct IntuiMessage *intuiMsg;
struct CDDisk       *disk;
struct CDSequence   *track;
BOOL                 first;
UWORD                icode;
UWORD                quals;
UWORD                cmd;
ULONG                numTracks;
ULONG                currentTrack;
ULONG                oldTrack;
ULONG                options;
BOOL                 selectDown;
BOOL                 selectConv;
BOOL                 ffwdDown;
BOOL                 frevDown;
BOOL                 faded;
ULONG                fadeCount;

    removalSig = (1 << MPEGPlayerBase->mp_RemovalSigBit);
    intuiSig   = (1 << MPEGPlayerBase->mp_Window->UserPort->mp_SigBit);
    disk       = NULL;
    first      = TRUE;
    options    = (OPTF_TIMEDEFAULT | OPTF_SHOWCDG);
    selectDown = FALSE;
    selectConv = TRUE;
    ffwdDown   = FALSE;
    frevDown   = FALSE;
    cmd        = CMD_DISKCHANGE;
    faded      = FALSE;
    fadeCount  = TICKS_BEFORE_FADE;

    SetWriteMask(&MPEGPlayerBase->mp_DBRastPort[0],0x03);
    SetWriteMask(&MPEGPlayerBase->mp_DBRastPort[1],0x03);

    MPEGPlayerBase->mp_DrawArrows = TRUE;

    while (TRUE)
    {
        if (cmd != CMD_NOP)
        {
            if (faded)
            {
                FadeIn(MPEGPlayerBase,TRUE);
                faded = FALSE;
            }
            fadeCount = TICKS_BEFORE_FADE;
        }

        // there are a few stateless commands which we implement
        // here, instead of having to do 'em for every state down
        // below
        switch (cmd)
        {
            case CMD_EXIT:
#if (CLEAN_EXIT)
                           FreeDiskInfo(MPEGPlayerBase,disk);
                           DeleteTracks(MPEGPlayerBase);
                           DeleteText(MPEGPlayerBase);
                           DeleteCredits(MPEGPlayerBase);
#endif
                           return;

            case CMD_DISKCHANGE: FreeDiskInfo(MPEGPlayerBase,disk);
                                 DeleteTracks(MPEGPlayerBase);
                                 DeleteText(MPEGPlayerBase);
                                 DeleteCredits(MPEGPlayerBase);

                                 options |= (OPTF_TIMEDEFAULT | OPTF_SHOWCDG);
                                 options &= (~OPTF_TIMEOVERLAY);

                                 if (disk = GetDiskInfo(MPEGPlayerBase))
                                 {
                                     CHANGEPAGE(PAGETYPE_TRACKS);
                                     numTracks    = disk->cdd_NumTracks;
                                     currentTrack = 0;

                                     // if a disk only has a single track, just
                                     // play it automatically. Specially cool for
                                     // VideoCD disks...
                                     if (numTracks == 1)
                                         options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);

                                     MPEGPlayerBase->mp_DrawArrows = FALSE;
                                     DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                                     MPEGPlayerBase->mp_RandomModeChanged = TRUE;
                                     currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                                     MPEGPlayerBase->mp_DrawArrows = TRUE;
                                     DrawState(MPEGPlayerBase,disk,0,options);
                                 }
                                 else
                                 {
                                     CHANGEPAGE(PAGETYPE_NODISK);
                                     DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                                 }
                                 first = FALSE;
                                 break;

            case CMD_RANDOM: options ^= OPTF_RANDOM;
                             MPEGPlayerBase->mp_RandomModeChanged = TRUE;
                             currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                             DrawState(MPEGPlayerBase,disk,currentTrack,options);
                             break;

            case CMD_LOOP: options ^= OPTF_LOOP;
                           DrawState(MPEGPlayerBase,disk,currentTrack,options);
                           break;

            case CMD_INTRO: options ^= OPTF_INTRO;
                            DrawState(MPEGPlayerBase,disk,currentTrack,options);
                            break;

            case CMD_SELECT_DOWN: if (track = FindTrack(disk,currentTrack))
                                  {
                                      SelectTrack(MPEGPlayerBase,disk,currentTrack,!track->cds_Selected);
                                      DrawState(MPEGPlayerBase,disk,currentTrack,options);
                                      selectDown = TRUE;
                                      selectConv = track->cds_Selected;
                                  }
                                  break;

            case CMD_SELECT_UP: selectDown = FALSE;
                                break;
        }

        if (!disk)
            cmd = CMD_NOP;

        switch (MPEGPlayerBase->mp_CurrentPage)
        {
/***** PAGETYPE_NODISK state *****/
            case PAGETYPE_NODISK:
            break;

#if (DO_TEXTDISPLAYS)
/***** PAGETYPE_ALBUMNOTES state *****/
            case PAGETYPE_ALBUMNOTES:
            switch (cmd)
            {
                case CMD_UP:
                ScrollText(MPEGPlayerBase,-1);
                break;

                case CMD_DOWN:
                ScrollText(MPEGPlayerBase,1);
                break;

                case CMD_RIGHT:
                CHANGEPAGE(PAGETYPE_TRACKS);
                DeleteText(MPEGPlayerBase);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                CreateTracks(MPEGPlayerBase,disk);
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_NOP);
                break;

                case CMD_PLAY:
                options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);
                currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                break;

                case CMD_NEXTTRACK:
                case CMD_PREVTRACK:
                // not implemented yet...
                break;
            }
            break;
#endif

/***** PAGETYPE_TRACKS state *****/
            case PAGETYPE_TRACKS:
            oldTrack = currentTrack;
            switch (cmd)
            {
                case CMD_PREVTRACK:
                case CMD_UP:
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_UP);
                if (currentTrack != oldTrack)
                {
                    if (selectDown)
                        SelectTrack(MPEGPlayerBase,disk,currentTrack,selectConv);

                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                }
                break;

                case CMD_NEXTTRACK:
                case CMD_DOWN:
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_DOWN);
                if (currentTrack != oldTrack)
                {
                    if (selectDown)
                        SelectTrack(MPEGPlayerBase,disk,currentTrack,selectConv);

                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                }
                break;

                case CMD_LEFT:
                if (disk->cdd_OnlyAudio)
                {
                    currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_LEFT);
                    if (currentTrack != oldTrack)
                    {
                        if (selectDown)
                            SelectTrack(MPEGPlayerBase,disk,currentTrack,selectConv);

                        DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    }
                }
                else
                {
                    if (disk->cdd_Notes)
                    {
                        CHANGEPAGE(PAGETYPE_ALBUMNOTES);
                        DeleteTracks(MPEGPlayerBase);
                        CreateText(MPEGPlayerBase,disk->cdd_Name,disk->cdd_Notes,disk->cdd_NotesLen);
                        DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    }
                }
                break;

                case CMD_RIGHT:
                if (disk->cdd_OnlyAudio)
                {
                    currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_RIGHT);
                    if (currentTrack != oldTrack)
                    {
                        if (selectDown)
                            SelectTrack(MPEGPlayerBase,disk,currentTrack,selectConv);

                        DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    }
                }
                else
                {
                    if(MPEGPlayerBase->mp_VideoCDType == CDT_KARAOKE)
                    {
                        DeleteTracks(MPEGPlayerBase);
                        track = FindTrack(disk,currentTrack);
    
                        if (track->cds_Lyrics)
                        {
                            CHANGEPAGE(PAGETYPE_TRACKLYRICS);
                            DrawState(MPEGPlayerBase,disk,currentTrack,options);
                            CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Lyrics,track->cds_LyricsLen);
                        }
                        else if (track->cds_Notes)
                        {
                            CHANGEPAGE(PAGETYPE_TRACKNOTES);
                            DrawState(MPEGPlayerBase,disk,currentTrack,options);
                            CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Notes,track->cds_NotesLen);
                        }
                        else if (!track->cds_AudioTrack)
                        {
                            CHANGEPAGE(PAGETYPE_TRACKCREDITS);
                            DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                            CreateCredits(MPEGPlayerBase,track);
                        }
                        else
                        {
                            DrawState(MPEGPlayerBase,disk,currentTrack,options);
                        }
                    }
                }
                break;

                case CMD_PLAY:
                options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);
                currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                break;
            }
            break;

#if (DO_TEXTDISPLAYS)
/***** PAGETYPE_TRACKLYRICS state *****/
            case PAGETYPE_TRACKLYRICS:
            switch (cmd)
            {
                case CMD_UP:
                ScrollText(MPEGPlayerBase,-1);
                break;

                case CMD_DOWN:
                ScrollText(MPEGPlayerBase,1);
                break;

                case CMD_LEFT:
                DeleteText(MPEGPlayerBase);
                CHANGEPAGE(PAGETYPE_TRACKS);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                CreateTracks(MPEGPlayerBase,disk);
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_NOP);
                break;

                case CMD_RIGHT:
                DeleteText(MPEGPlayerBase);
                track = FindTrack(disk,currentTrack);
                if (track->cds_Notes)
                {
                    CHANGEPAGE(PAGETYPE_TRACKNOTES);
                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Notes,track->cds_NotesLen);
                }
                else
                {
                    CHANGEPAGE(PAGETYPE_TRACKCREDITS);
                    DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                    CreateCredits(MPEGPlayerBase,FindTrack(disk,currentTrack));
                }
                break;

                case CMD_PLAY:
                options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);
                currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                break;

                case CMD_NEXTTRACK:
                case CMD_PREVTRACK:
                // not implemented yet...
                break;
            }
            break;
#endif

#if (DO_TEXTDISPLAYS)
/***** PAGETYPE_TRACKNOTES state *****/
            case PAGETYPE_TRACKNOTES:
            switch (cmd)
            {
                case CMD_UP:
                ScrollText(MPEGPlayerBase,-1);
                break;

                case CMD_DOWN:
                ScrollText(MPEGPlayerBase,1);
                break;

                case CMD_LEFT:
                DeleteText(MPEGPlayerBase);
                CHANGEPAGE(PAGETYPE_TRACKS);
                track = FindTrack(disk,currentTrack);
                if (track->cds_Lyrics)
                {
                    CHANGEPAGE(PAGETYPE_TRACKLYRICS);
                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Lyrics,track->cds_LyricsLen);
                }
                else
                {
                    CHANGEPAGE(PAGETYPE_TRACKS);
                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    CreateTracks(MPEGPlayerBase,disk);
                }
                break;

                case CMD_RIGHT:
                CHANGEPAGE(PAGETYPE_TRACKCREDITS);
                DeleteText(MPEGPlayerBase);
                DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                CreateCredits(MPEGPlayerBase,FindTrack(disk,currentTrack));
                break;

                case CMD_PLAY:
                options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);
                currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                break;

                case CMD_NEXTTRACK:
                case CMD_PREVTRACK:
                // not implemented yet...
                break;
            }
            break;
#endif

/***** PAGETYPE_TRACKCREDITS state *****/
            case PAGETYPE_TRACKCREDITS:
            switch (cmd)
            {
                case CMD_LEFT:
                DeleteCredits(MPEGPlayerBase);
                track = FindTrack(disk,currentTrack);
                if (track->cds_Notes)
                {
                    CHANGEPAGE(PAGETYPE_TRACKNOTES);
                    DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                    CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Notes,track->cds_NotesLen);
                }
                else if (track->cds_Lyrics)
                {
                    CHANGEPAGE(PAGETYPE_TRACKLYRICS);
                    DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                    CreateText(MPEGPlayerBase,track->cds_Name,track->cds_Lyrics,track->cds_LyricsLen);
                }
                else
                {
                    CHANGEPAGE(PAGETYPE_TRACKS);
                    DrawPage(MPEGPlayerBase,disk,currentTrack,options,first);
                    CreateTracks(MPEGPlayerBase,disk);
                    currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_NOP);
                }
                break;

                case CMD_UP:
                ScrollCredits(MPEGPlayerBase,-1);
                break;

                case CMD_DOWN:
                ScrollCredits(MPEGPlayerBase,1);
                break;

                case CMD_PLAY:
                StopThumbnail(MPEGPlayerBase);
                options = DoPlay(MPEGPlayerBase,disk,currentTrack,options);
                currentTrack = OrganizeSequences(MPEGPlayerBase,disk,currentTrack,options);
                StartThumbnail(MPEGPlayerBase,FindTrack(disk,currentTrack));
                DrawState(MPEGPlayerBase,disk,currentTrack,options);
                break;

                case CMD_NEXTTRACK:
                oldTrack = currentTrack;
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_DOWN);
                if (currentTrack != oldTrack)
                {
                    DeleteCredits(MPEGPlayerBase);
                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    track = FindTrack(disk,currentTrack);
                    CreateCredits(MPEGPlayerBase,track);
                }
                break;

                case CMD_PREVTRACK:
                oldTrack = currentTrack;
                currentTrack = HighlightTrack(MPEGPlayerBase,currentTrack,MV_UP);
                if (currentTrack != oldTrack)
                {
                    DeleteCredits(MPEGPlayerBase);
                    DrawState(MPEGPlayerBase,disk,currentTrack,options);
                    track = FindTrack(disk,currentTrack);
                    CreateCredits(MPEGPlayerBase,track);
                }
                break;
            }
            break;
        }

        cmd = CMD_NOP;

        if (intuiMsg = (struct IntuiMessage *)GetMsg(MPEGPlayerBase->mp_Window->UserPort))
        {
            icode = intuiMsg->Code;
            quals = intuiMsg->Qualifier;

            switch (intuiMsg->Class)
            {
                case IDCMP_INTUITICKS: if (!faded)
                                       {
                                           fadeCount--;
                                           if (!fadeCount)
                                           {
                                               FadeOut(MPEGPlayerBase,TRUE);
                                               faded = TRUE;
                                           }
                                       }
                                       break;

                case IDCMP_RAWKEY:
                cmd = CMD_BOGUS;   /* make sure any button press wakes up a faded screen */
                switch (icode)
                {
#if (CLEAN_EXIT)
                    case 0x45 /* ESC key */: cmd = CMD_EXIT;
                                             break;
#endif

                    case RAWKEY_PORT0_BUTTON_PLAY:
                    case RAWKEY_PORT1_BUTTON_PLAY:
                    case RAWKEY_PORT2_BUTTON_PLAY:
                    case RAWKEY_PORT3_BUTTON_PLAY: if (!(IEQUALIFIER_REPEAT & quals))
                                                       cmd = CMD_PLAY;
                                                   break;

                    case RAWKEY_PORT0_BUTTON_RED:
                    case RAWKEY_PORT1_BUTTON_RED:
                    case RAWKEY_PORT2_BUTTON_RED:
                    case RAWKEY_PORT3_BUTTON_RED: if (!(IEQUALIFIER_REPEAT & quals))
                                                      cmd = CMD_SELECT_DOWN;
                                                  break;

                    case (0x80 | RAWKEY_PORT0_BUTTON_RED):
                    case (0x80 | RAWKEY_PORT1_BUTTON_RED):
                    case (0x80 | RAWKEY_PORT2_BUTTON_RED):
                    case (0x80 | RAWKEY_PORT3_BUTTON_RED): if (!(IEQUALIFIER_REPEAT & quals))
                                                               cmd = CMD_SELECT_UP;
                                                           break;

                    case RAWKEY_PORT0_BUTTON_GREEN:
                    case RAWKEY_PORT1_BUTTON_GREEN:
                    case RAWKEY_PORT2_BUTTON_GREEN:
                    case RAWKEY_PORT3_BUTTON_GREEN: if (!(IEQUALIFIER_REPEAT & quals))
                                                        cmd = CMD_RANDOM;
                                                    break;

                    case RAWKEY_PORT0_BUTTON_YELLOW:
                    case RAWKEY_PORT1_BUTTON_YELLOW:
                    case RAWKEY_PORT2_BUTTON_YELLOW:
                    case RAWKEY_PORT3_BUTTON_YELLOW: if (!(IEQUALIFIER_REPEAT & quals))
                                                         cmd = CMD_LOOP;
                                                     break;

                    case RAWKEY_PORT0_BUTTON_REVERSE:
                    case RAWKEY_PORT1_BUTTON_REVERSE:
                    case RAWKEY_PORT2_BUTTON_REVERSE:
                    case RAWKEY_PORT3_BUTTON_REVERSE: frevDown = TRUE;
                                                      if (ffwdDown)
                                                      {
                                                          cmd      = CMD_INTRO;
                                                          ffwdDown = FALSE;
                                                          frevDown = FALSE;
                                                      }
                    				      break;

                    case (0x80 | RAWKEY_PORT0_BUTTON_REVERSE):
                    case (0x80 | RAWKEY_PORT1_BUTTON_REVERSE):
                    case (0x80 | RAWKEY_PORT2_BUTTON_REVERSE):
                    case (0x80 | RAWKEY_PORT3_BUTTON_REVERSE): if (frevDown)
                                                                   cmd = CMD_PREVTRACK;

                                                               frevDown = FALSE;
                                                               break;

                    case RAWKEY_PORT0_BUTTON_FORWARD:
                    case RAWKEY_PORT1_BUTTON_FORWARD:
                    case RAWKEY_PORT2_BUTTON_FORWARD:
                    case RAWKEY_PORT3_BUTTON_FORWARD: ffwdDown = TRUE;
                                                      if (frevDown)
                                                      {
                                                          cmd      = CMD_INTRO;
                                                          ffwdDown = FALSE;
                                                          frevDown = FALSE;
                                                      }
                    				      break;

                    case (0x80 | RAWKEY_PORT0_BUTTON_FORWARD):
                    case (0x80 | RAWKEY_PORT1_BUTTON_FORWARD):
                    case (0x80 | RAWKEY_PORT2_BUTTON_FORWARD):
                    case (0x80 | RAWKEY_PORT3_BUTTON_FORWARD): if (ffwdDown)
                                                                   cmd = CMD_NEXTTRACK;

                                                               ffwdDown = FALSE;
                                                               break;

                    case RAWKEY_PORT0_JOY_UP  :
                    case RAWKEY_PORT1_JOY_UP  :
                    case RAWKEY_PORT2_JOY_UP  :
                    case RAWKEY_PORT3_JOY_UP  :
                    case 0x4c                 : cmd = CMD_UP;
                                                break;

                    case RAWKEY_PORT0_JOY_DOWN:
                    case RAWKEY_PORT1_JOY_DOWN:
                    case RAWKEY_PORT2_JOY_DOWN:
                    case RAWKEY_PORT3_JOY_DOWN:
                    case 0x4d                 : cmd = CMD_DOWN;
                                                break;

                    case RAWKEY_PORT0_JOY_LEFT:
                    case RAWKEY_PORT1_JOY_LEFT:
                    case RAWKEY_PORT2_JOY_LEFT:
                    case RAWKEY_PORT3_JOY_LEFT:
                    case 0x4f                 : cmd = CMD_LEFT;
                                                break;

                    case RAWKEY_PORT0_JOY_RIGHT:
                    case RAWKEY_PORT1_JOY_RIGHT:
                    case RAWKEY_PORT2_JOY_RIGHT:
                    case RAWKEY_PORT3_JOY_RIGHT:
                    case 0x4e                  : cmd = CMD_RIGHT;
                                                 break;
                }
            }

            ReplyMsg(intuiMsg);
        }
        else
        {
            sigs = Wait(intuiSig | removalSig);

            if (removalSig & sigs)
                cmd = CMD_DISKCHANGE;
        }
    }
}
