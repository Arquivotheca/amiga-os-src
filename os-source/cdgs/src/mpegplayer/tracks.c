
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <string.h>
#include <stdio.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/layers_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/layers_pragmas.h>

#include "utils.h"
#include "mpegplayerbase.h"
#include "defs.h"
#include "diskinfo.h"
#include "tracks.h"


/*****************************************************************************/


#define COLUMN_WIDTH (PAGE_WIDTH / 3)
#define FUDGE        ((SLOT_HEIGHT / 2) + 2)


/*****************************************************************************/


extern UBYTE * __far staff;


static void DrawTitle(struct MPEGPlayerLib *MPEGPlayerBase, struct CDDisk *disk)
{
struct RastPort *rp;
struct BitMap   *bm;
struct BMInfo   *bmInfo;
char             buffer[100];
UWORD            len;
UWORD		 pix;

    rp = MPEGPlayerBase->mp_RenderRP;

    if (disk->cdd_Name)
    {
        if (disk->cdd_NumVolumes > 0)
            len = nprintf(buffer,sizeof(buffer),"%s (%ld / %ld)",disk->cdd_Name,disk->cdd_VolumeNumber,disk->cdd_NumVolumes);
        else
            len = nprintf(buffer,sizeof(buffer),"%s",disk->cdd_Name);

        Move(rp,TITLE_LEFT+4,TITLE_TOP + 31);
        SetABPenDrMd(rp,1,2,JAM1);
        SetFont(rp,&MPEGPlayerBase->mp_Compact31);

	/* if this string is too long to fit, fall back to smaller font */

	pix = ShadowTextLength(MPEGPlayerBase,rp,buffer,len);

	if(pix > TITLE_WIDTH-8)
	{
		SetFont(rp,&MPEGPlayerBase->mp_Compact24);
		Move(rp,TITLE_LEFT+4,TITLE_TOP + 27);
	}

        CenterText(MPEGPlayerBase,rp,buffer,len,TITLE_WIDTH-8);
    }
    else if (disk->cdd_OnlyAudio)
    {
        GetBM(MPEGPlayerBase,&staff,&bm,&bmInfo);

        BltBitMap(bm,0,0,
                  rp->BitMap,TITLE_LEFT + (TITLE_WIDTH - bmInfo->bmi_Width) / 2,
                             TITLE_TOP + (TITLE_HEIGHT - bmInfo->bmi_Height) / 2,
                             bmInfo->bmi_Width,bmInfo->bmi_Height,
                             0xc0,0x03,NULL);

        FreeBMInfo(bmInfo);
        WaitBlit();
        FreeBitMap(bm);
    }

    SwapBuffers(MPEGPlayerBase);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],TITLE_LEFT,TITLE_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,TITLE_LEFT,TITLE_TOP,
              TITLE_WIDTH,TITLE_HEIGHT,0xc0,0x03,NULL);
}


/*****************************************************************************/


static void GetRowColumn(ULONG numTracks, ULONG trackNum,
                         ULONG *row, ULONG *column)
{
ULONG tracksPerColumn;
ULONG extra, column0, column1;

    if (numTracks == 0)
    {
        *row    = 0;
        *column = 0;
    }
    else if (numTracks < VISIBLE_AUDIO_SLOTS)
    {
        *row    = trackNum;
        *column = 1;
    }
    else if (numTracks <= VISIBLE_AUDIO_SLOTS * 3)
    {
        tracksPerColumn = numTracks / 3;
        extra           = numTracks % 3;

        column0 = tracksPerColumn;
        if (extra)
        {
            extra--;
            column0++;
        }
        column1 = tracksPerColumn + extra;

        *column = 0;
        if (trackNum >= column0)
        {
            (*column)++;
            trackNum -= column0;

            if (trackNum >= column1)
            {
                (*column)++;
                trackNum -= column1;
            }
        }

        *row = trackNum;
    }
    else
    {
        *row    = trackNum % VISIBLE_AUDIO_SLOTS;
        *column = trackNum / VISIBLE_AUDIO_SLOTS;
    }
}


/*****************************************************************************/


static LONG GetTrackNum(ULONG numTracks, ULONG row, ULONG column)
{
ULONG tracksPerColumn;
ULONG trackNum;
LONG  extra, column0, column1;

    if (numTracks == 0)
    {
        return (-1);
    }
    else if (numTracks < VISIBLE_AUDIO_SLOTS)
    {
        if ((column != 1) || (row >= numTracks))
            return(-1);

        trackNum = row;
    }
    else if (numTracks <= VISIBLE_AUDIO_SLOTS * 3)
    {
        if (column > 2)
            return (-1);

        tracksPerColumn = numTracks / 3;
        extra           = numTracks % 3;

        column0 = tracksPerColumn;
        if (extra)
        {
            extra--;
            column0++;
        }
        column1 = tracksPerColumn + extra;

        if (column == 2)
        {
            if (row >= tracksPerColumn)
                return(-1);

            return ((LONG)(column0+column1+row));
        }

        if (column == 1)
        {
            if (row >= column1)
                return(-1);

            return ((LONG)(column0+row));
        }

        if (row >= column0)
            return(-1);

        return((LONG)row);
    }
    else
    {
        trackNum = column * VISIBLE_AUDIO_SLOTS + row;
        if (trackNum >= numTracks)
            return (-1);
    }

    return ((LONG)trackNum);
}


/*****************************************************************************/


static void FillBar(struct MPEGPlayerLib *MPEGPlayerBase,
		    LONG x, LONG y, LONG w, LONG h)
{
    SetABPenDrMd(MPEGPlayerBase->mp_RenderRP,1,0,JAM1);
    RectFill(MPEGPlayerBase->mp_RenderRP,x,y,x+w-1,y+h-1);

    BltMaskBitMapRastPort(MPEGPlayerBase->mp_TrackBM,MPEGPlayerBase->mp_TrackBMXOffset + x - PAGE_LEFT,MPEGPlayerBase->mp_TrackBMYOffset + y - PAGE_TOP,
    			  MPEGPlayerBase->mp_RenderRP,x,y,w,h,
    			  (ABC|ABNC|ANBC),MPEGPlayerBase->mp_TrackBM->Planes[1]);

    SwapBuffers(MPEGPlayerBase);
}


/*****************************************************************************/


void TrackState(struct MPEGPlayerLib *MPEGPlayerBase,
                ULONG track, BOOL selected)
{
struct RastPort   trackRP;
WORD              x,y,w,h;
struct Rectangle *check;
ULONG             row,column;

    check = &iconsPos[CHECKMARK_ICON];

    if (!MPEGPlayerBase->mp_TrackBM)
        return;

    InitRastPort(&trackRP);
    trackRP.BitMap = MPEGPlayerBase->mp_TrackBM;
    SetFont(&trackRP,&MPEGPlayerBase->mp_Compact24);

    w = check->MaxX - check->MinX + 1;
    h = check->MaxY - check->MinY + 1;

    if (MPEGPlayerBase->mp_AudioLayout)
    {
        GetRowColumn(MPEGPlayerBase->mp_TrackCount,track,&row,&column);

        x = (column * COLUMN_WIDTH) + 5;
        y = (row * SLOT_HEIGHT) + 2;

        if (selected)
        {
            BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
                      MPEGPlayerBase->mp_TrackBM,x,y,
                      w,h,0xc0,0x03,NULL);
        }
        else
        {
            SetAPen(&trackRP,0);
            RectFill(&trackRP,x,y,x + w - 1,y + h - 1);
        }

        BltBitMap(MPEGPlayerBase->mp_TrackBM,x,y,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT + (column - MPEGPlayerBase->mp_OldLeft)*COLUMN_WIDTH + 5,PAGE_TOP + y,
                  w,h,0xc0,0xff,NULL);

        FillBar(MPEGPlayerBase,PAGE_LEFT + (column - MPEGPlayerBase->mp_OldLeft)*COLUMN_WIDTH + 5,
                               PAGE_TOP + y,
                               w,h);

        BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT + (column - MPEGPlayerBase->mp_OldLeft)*COLUMN_WIDTH + 5, PAGE_TOP + y,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT + (column - MPEGPlayerBase->mp_OldLeft)*COLUMN_WIDTH + 5,PAGE_TOP + y,
                  w,h,0xc0,0x03,NULL);
    }
    else
    {
        if (selected)
        {
            BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
            	      MPEGPlayerBase->mp_TrackBM,5,(track * SLOT_HEIGHT) + FUDGE + 2,
            	      w,h,0xc0,0x03,NULL);
        }
        else
        {
            SetAPen(&trackRP,0);
            RectFill(&trackRP,5,(track * SLOT_HEIGHT) + FUDGE + 2,
                              5 + w - 1,(track * SLOT_HEIGHT) + FUDGE + 2 + h - 1);
        }

        y = PAGE_TOP + ((track - MPEGPlayerBase->mp_OldTop)*SLOT_HEIGHT) + FUDGE + 2;

        BltBitMap(MPEGPlayerBase->mp_TrackBM,5,(track * SLOT_HEIGHT) + FUDGE + 2,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT+5,y,
                  w,h,0xc0,0xff,NULL);

        FillBar(MPEGPlayerBase,PAGE_LEFT+5,y,w,h);

        BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT+5,y,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT+5,y,
                  w,h,0xc0,0x03,NULL);
    }
}


/*****************************************************************************/


static void BltTracks(struct MPEGPlayerLib *MPEGPlayerBase, WORD srcX, WORD srcY)
{
    BltBitMap(MPEGPlayerBase->mp_TrackBM,srcX,srcY,
              MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT,PAGE_TOP,
              PAGE_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);

    MPEGPlayerBase->mp_TrackBMXOffset = srcX;
    MPEGPlayerBase->mp_TrackBMYOffset = srcY;
}


/*****************************************************************************/


#define SCROLL_AMOUNT_X 20
#define SCROLL_AMOUNT_Y 5

ULONG HighlightTrack(struct MPEGPlayerLib *MPEGPlayerBase, ULONG currentTrack,
                     enum Movement direction)
{
WORD  i, j;
WORD  rowDelta, colDelta;
WORD  rowMax;
LONG  oldTop;
BOOL  scrolled;
ULONG newTrack;
ULONG srcRow, srcCol;
ULONG dstRow, dstCol;

    oldTop   = MPEGPlayerBase->mp_OldTop;
    scrolled = FALSE;

    if (MPEGPlayerBase->mp_TrackCount <= 1)
        direction = MV_NOP;

    if (MPEGPlayerBase->mp_AudioLayout)
    {
         GetRowColumn(MPEGPlayerBase->mp_TrackCount,currentTrack,&srcRow,&srcCol);

         newTrack = currentTrack;
         switch (direction)
         {
             case MV_UP   : if (currentTrack)
                                newTrack = currentTrack-1;
                            break;

             case MV_DOWN : if (currentTrack+1 < MPEGPlayerBase->mp_TrackCount)
                                newTrack = currentTrack+1;
                            break;

             case MV_LEFT : if (srcCol)
                                newTrack = GetTrackNum(MPEGPlayerBase->mp_TrackCount,srcRow,srcCol-1);
                            break;

             case MV_RIGHT : newTrack = GetTrackNum(MPEGPlayerBase->mp_TrackCount,srcRow,srcCol+1);
                             break;
         }

         if (newTrack == 0xffffffff)
             newTrack = currentTrack;

         if (MPEGPlayerBase->mp_TrackBM)
         {
             GetRowColumn(MPEGPlayerBase->mp_TrackCount,newTrack,&dstRow,&dstCol);

             if (dstCol < MPEGPlayerBase->mp_OldLeft)
             {
                 i = 0;
                 do
                 {
                     i += SCROLL_AMOUNT_X;

                     if (i > COLUMN_WIDTH)
                         i = COLUMN_WIDTH;

                     BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH - i,0);
                     FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + dstRow*SLOT_HEIGHT,COLUMN_WIDTH,SLOT_HEIGHT);
                 }
                 while (i < COLUMN_WIDTH);

                 MPEGPlayerBase->mp_OldLeft--;
             }
             else if (dstCol > MPEGPlayerBase->mp_OldLeft + 2)
             {
                 i = 0;
                 do
                 {
                     i += SCROLL_AMOUNT_X;

                     if (i > COLUMN_WIDTH)
                         i = COLUMN_WIDTH;

                     BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH + i,0);
                     FillBar(MPEGPlayerBase,PAGE_LEFT + (2 * COLUMN_WIDTH),PAGE_TOP + dstRow*SLOT_HEIGHT,COLUMN_WIDTH,SLOT_HEIGHT);
                 }
                 while (i < COLUMN_WIDTH);

                 MPEGPlayerBase->mp_OldLeft++;
             }
             else if (newTrack != currentTrack)
             {
                 if ((srcRow != dstRow) && (srcCol != dstCol))
                 {
                     i = 0;
                     j = 0;

                     if (dstRow < srcRow)
                     {
                         rowDelta = -1;
                         rowMax   = (srcRow - dstRow) * SLOT_HEIGHT;
                     }
                     else
                     {
                         rowDelta = 1;
                         rowMax   = (dstRow - srcRow) * SLOT_HEIGHT;
                     }

                     if (dstCol < srcCol)
                     {
                         colDelta = -1;
                     }
                     else
                     {
                         colDelta = 1;
                     }

                     do
                     {
                         i += SCROLL_AMOUNT_X;
                         if (i > COLUMN_WIDTH)
                             i = COLUMN_WIDTH;

                         j += SCROLL_AMOUNT_Y * 3 + 2;
                         if (j > rowMax)
                             j = rowMax;

                         BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);
                         FillBar(MPEGPlayerBase,PAGE_LEFT + (srcCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH + i*colDelta,PAGE_TOP + srcRow*SLOT_HEIGHT + j*rowDelta,COLUMN_WIDTH,SLOT_HEIGHT);
                     }
                     while ((i < COLUMN_WIDTH) || (j < rowMax));
                 }
                 else if (dstRow > srcRow)
                 {
                     i = 0;
                     do
                     {
                         i += SCROLL_AMOUNT_Y;

                         if (i > SLOT_HEIGHT)
                             i = SLOT_HEIGHT;

                         BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);
                         FillBar(MPEGPlayerBase,PAGE_LEFT + (dstCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH,PAGE_TOP + srcRow*SLOT_HEIGHT + i,COLUMN_WIDTH,SLOT_HEIGHT);
                     }
                     while (i < SLOT_HEIGHT);
                 }
                 else if (dstRow < srcRow)
                 {
                     i = 0;
                     do
                     {
                         i += SCROLL_AMOUNT_Y;

                         if (i > SLOT_HEIGHT)
                             i = SLOT_HEIGHT;

                         BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);
                         FillBar(MPEGPlayerBase,PAGE_LEFT + (dstCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH,PAGE_TOP + srcRow*SLOT_HEIGHT - i,COLUMN_WIDTH,SLOT_HEIGHT);
                     }
                     while (i < SLOT_HEIGHT);
                 }
                 else if (dstCol > srcCol)
                 {
                     i = 0;
                     do
                     {
                         i += SCROLL_AMOUNT_X;

                         if (i > COLUMN_WIDTH)
                             i = COLUMN_WIDTH;

                         BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);
                         FillBar(MPEGPlayerBase,PAGE_LEFT + (srcCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH + i,PAGE_TOP + dstRow*SLOT_HEIGHT,COLUMN_WIDTH,SLOT_HEIGHT);
                     }
                     while (i < COLUMN_WIDTH);
                 }
                 else // if (dstCol < srcCol)
                 {
                     i = 0;
                     do
                     {
                         i += SCROLL_AMOUNT_X;

                         if (i > COLUMN_WIDTH)
                             i = COLUMN_WIDTH;

                         BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);
                         FillBar(MPEGPlayerBase,PAGE_LEFT + (srcCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH - i,PAGE_TOP + dstRow*SLOT_HEIGHT,COLUMN_WIDTH,SLOT_HEIGHT);
                     }
                     while (i < COLUMN_WIDTH);
                 }
             }
             else if (direction == MV_NOP)
             {
                 BltTracks(MPEGPlayerBase,MPEGPlayerBase->mp_OldLeft * COLUMN_WIDTH,0);

                 if (MPEGPlayerBase->mp_TrackCount > 1)
                     FillBar(MPEGPlayerBase,PAGE_LEFT + (dstCol - MPEGPlayerBase->mp_OldLeft) * COLUMN_WIDTH,PAGE_TOP + dstRow*SLOT_HEIGHT,COLUMN_WIDTH,SLOT_HEIGHT);
                 else
                     SwapBuffers(MPEGPlayerBase);
             }
         }
         currentTrack = newTrack;
    }
    else
    {
        newTrack = currentTrack;
        if (direction == MV_DOWN)
        {
            if (newTrack+1 < MPEGPlayerBase->mp_TrackCount)
                newTrack++;
        }
        else if (direction == MV_UP)
        {
            if (newTrack > 0)
                newTrack--;
        }

        if (MPEGPlayerBase->mp_TrackBM)
        {
            if (direction != MV_NOP)
            {
                if (newTrack < oldTop)
                {
                    scrolled = TRUE;
                    i = 0;
                    do
                    {
                        i += SCROLL_AMOUNT_Y;

                        if (i > SLOT_HEIGHT)
                            i = SLOT_HEIGHT;

                        BltTracks(MPEGPlayerBase,0,oldTop*SLOT_HEIGHT - i);
                        FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + FUDGE,PAGE_WIDTH,SLOT_HEIGHT);
                    }
                    while (i < SLOT_HEIGHT);

                    MPEGPlayerBase->mp_OldTop--;
                }
                else if (newTrack >= oldTop + VISIBLE_SLOTS)
                {
                    scrolled = TRUE;
                    i = 0;
                    do
                    {
                        i += SCROLL_AMOUNT_Y;

                        if (i > SLOT_HEIGHT)
                            i = SLOT_HEIGHT;

                        BltTracks(MPEGPlayerBase,0,oldTop*SLOT_HEIGHT + i);
                        FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + SLOT_HEIGHT*(VISIBLE_SLOTS-1) + FUDGE,PAGE_WIDTH,SLOT_HEIGHT);
                    }
                    while (i < SLOT_HEIGHT);

                    MPEGPlayerBase->mp_OldTop++;
                }
                else if (newTrack > currentTrack)
                {
                    scrolled = TRUE;
                    i = 0;
                    do
                    {
                        i += SCROLL_AMOUNT_Y;

                        if (i > SLOT_HEIGHT)
                            i = SLOT_HEIGHT;

                        BltTracks(MPEGPlayerBase,0,MPEGPlayerBase->mp_OldTop*SLOT_HEIGHT);
                        FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + (newTrack-MPEGPlayerBase->mp_OldTop - 1)*SLOT_HEIGHT + FUDGE + i,PAGE_WIDTH,SLOT_HEIGHT);
                    }
                    while (i < SLOT_HEIGHT);
                }
                else if (newTrack < currentTrack)
                {
                    scrolled = TRUE;
                    i = 0;
                    do
                    {
                        i += SCROLL_AMOUNT_Y;

                        if (i > SLOT_HEIGHT)
                            i = SLOT_HEIGHT;

                        BltTracks(MPEGPlayerBase,0,MPEGPlayerBase->mp_OldTop*SLOT_HEIGHT);
                        FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + (newTrack-MPEGPlayerBase->mp_OldTop+1)*SLOT_HEIGHT + FUDGE - i,PAGE_WIDTH,SLOT_HEIGHT);
                    }
                    while (i < SLOT_HEIGHT);
                }
            }
        }
        currentTrack = newTrack;

        if (!scrolled && MPEGPlayerBase->mp_TrackBM)
        {
            if (currentTrack >= MPEGPlayerBase->mp_OldTop + VISIBLE_SLOTS)
                MPEGPlayerBase->mp_OldTop = currentTrack - VISIBLE_SLOTS + 1;

            BltTracks(MPEGPlayerBase,0,MPEGPlayerBase->mp_OldTop*SLOT_HEIGHT);

            if (MPEGPlayerBase->mp_TrackCount > 1)
                FillBar(MPEGPlayerBase,PAGE_LEFT,PAGE_TOP + (newTrack-MPEGPlayerBase->mp_OldTop)*SLOT_HEIGHT + FUDGE,PAGE_WIDTH,SLOT_HEIGHT);
            else
                SwapBuffers(MPEGPlayerBase);
        }
    }

    if (MPEGPlayerBase->mp_TrackBM)
    {
        BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT,PAGE_TOP,
                  MPEGPlayerBase->mp_RenderRP->BitMap,PAGE_LEFT,PAGE_TOP,
                  PAGE_WIDTH,PAGE_HEIGHT,0xc0,0x03,NULL);
    }

    return(currentTrack);
}


/*****************************************************************************/


VOID CreateTracks(struct MPEGPlayerLib *MPEGPlayerBase,
                  struct CDDisk *disk)
{
UWORD              x, y;
char               buffer[80];
char               time[10];
struct RastPort    trackRP;
struct CDSequence *track;
UWORD              num;
ULONG              pixels;
UWORD              timeLen;
struct Rectangle  *check;
ULONG              len;
ULONG              columns;
ULONG              width;
ULONG              row, column;
ULONG              trackNum;

    check = &iconsPos[CHECKMARK_ICON];

    if (!disk)
        return;

    InitRastPort(&trackRP);
    SetFont(&trackRP,&MPEGPlayerBase->mp_Compact24);
    SetABPenDrMd(&trackRP,3,2,JAM1);

    MPEGPlayerBase->mp_OldLeft        = 0;
    MPEGPlayerBase->mp_OldTop         = 0;
    MPEGPlayerBase->mp_TrackCount     = disk->cdd_NumTracks;
    MPEGPlayerBase->mp_AudioLayout    = disk->cdd_OnlyAudio;

    if (MPEGPlayerBase->mp_AudioLayout)
    {
        columns = disk->cdd_NumTracks / VISIBLE_AUDIO_SLOTS + 1;
        width = PAGE_WIDTH;
        if (columns > 3)
            width = COLUMN_WIDTH * columns;

        MPEGPlayerBase->mp_TrackBM = AllocBitMap(width,PAGE_HEIGHT,2,BMF_CLEAR|BMF_INTERLEAVED,NULL);
        trackRP.BitMap = MPEGPlayerBase->mp_TrackBM;

        trackNum = 0;
        SCANLIST(&disk->cdd_Sequences,track)
        {
            if (ISTRACK(track))
            {
                GetRowColumn(disk->cdd_NumTracks,trackNum,&row,&column);

                x = column * COLUMN_WIDTH;
                y = (row * SLOT_HEIGHT);

                if (track->cds_Selected)
                {
                    BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
                              MPEGPlayerBase->mp_TrackBM,x+5,y+2,
                              check->MaxX - check->MinX + 1,
                              check->MaxY - check->MinY + 1,
                              0xc0,0x03,NULL);
                }

                if (disk->cdd_NumTracks == 1)
                {
                    len = nprintf(buffer,sizeof(buffer),"1  -  %ld:%02ld",
                                  (track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_MINUTE,
                                  ((track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_SECOND) % 60);

                    Move(&trackRP, 0, 100);
                    CenterText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH);
                }
                else
                {
                    len = nprintf(buffer,sizeof(buffer),"%ld  -  %ld:%02ld",track->cds_TrackNumber,
                                  (track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_MINUTE,
                                  ((track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_SECOND) % 60);

                    if ((disk->cdd_NumTracks >= 10) && (track->cds_TrackNumber < 10))
                    {
                        Move(&trackRP, x + TEXT_OFFSET + 13, y + SLOT_BASELINE);
                        ClipText(MPEGPlayerBase,&trackRP,buffer,len,COLUMN_WIDTH-TEXT_OFFSET-13);
                    }
                    else
                    {
                        Move(&trackRP, x + TEXT_OFFSET, y + SLOT_BASELINE);
                        ClipText(MPEGPlayerBase,&trackRP,buffer,len,COLUMN_WIDTH-TEXT_OFFSET);
                    }
                }

                trackNum++;
            }
        }
    }
    else
    {
        y = FUDGE;

        num = disk->cdd_NumTracks;
        if (num < VISIBLE_SLOTS)
            num = VISIBLE_SLOTS;

        pixels = (num + 2)*SLOT_HEIGHT; // add room for the blank areas...

        MPEGPlayerBase->mp_TrackBM = AllocBitMap(PAGE_WIDTH,pixels,2,BMF_CLEAR|BMF_INTERLEAVED,NULL);
        trackRP.BitMap = MPEGPlayerBase->mp_TrackBM;

        SCANLIST(&disk->cdd_Sequences,track)
        {
            if (ISTRACK(track))
            {
                if (track->cds_Selected)
                {
                    BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
                              MPEGPlayerBase->mp_TrackBM,5,y+2,
                              check->MaxX - check->MinX + 1,
                              check->MaxY - check->MinY + 1,
                              0xc0,0x03,NULL);
                }

                len = nprintf(time,sizeof(time),"%ld:%02ld",
                              (track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_MINUTE,
                              ((track->cds_TrackEnd - track->cds_TrackStart + 1) / SECTORS_PER_SECOND) % 60);

                timeLen = ShadowTextLength(MPEGPlayerBase,&trackRP,time,len);

                if (!track->cds_Name)
                {
                    if (disk->cdd_NumTracks == 1)
                    {
                        len = nprintf(buffer,sizeof(buffer),"1  -  %s",time);
                        Move(&trackRP, 0, 100);
                        CenterText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH);
                    }
                    else
                    {
                        if (track->cds_AudioTrack)
                            len = nprintf(buffer,sizeof(buffer),"%ld  -  Audio",track->cds_TrackNumber);
                        else
                            len = nprintf(buffer,sizeof(buffer),"%ld  -  Video",track->cds_TrackNumber);

                        if ((disk->cdd_NumTracks >= 10) && (track->cds_TrackNumber < 10))
                        {
                            Move(&trackRP, TEXT_OFFSET + 13, y + SLOT_BASELINE);
                            ClipText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH-TEXT_OFFSET-timeLen-20-13);
                        }
                        else
                        {
                            Move(&trackRP, TEXT_OFFSET, y + SLOT_BASELINE);
                            ClipText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH-TEXT_OFFSET-timeLen-20);
                        }

                        Move(&trackRP, PAGE_WIDTH - timeLen - 10, y + SLOT_BASELINE);
                        ShadowText(MPEGPlayerBase,&trackRP,time,strlen(time),NULL);
                    }
                }
                else
                {
                    if (disk->cdd_NumTracks == 1)
                    {
                        Move(&trackRP,0,100);
                        CenterText(MPEGPlayerBase,&trackRP,track->cds_Name,strlen(track->cds_Name),PAGE_WIDTH);

                        Move(&trackRP,0,100+SLOT_HEIGHT);
                        CenterText(MPEGPlayerBase,&trackRP,time,len,PAGE_WIDTH);
                    }
                    else
                    {
                        len = nprintf(buffer,sizeof(buffer),"%ld  -  %s",track->cds_TrackNumber,track->cds_Name);

                        if ((disk->cdd_NumTracks >= 10) && (track->cds_TrackNumber < 10))
                        {
                            Move(&trackRP, TEXT_OFFSET + 13, y + SLOT_BASELINE);
                            ClipText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH-TEXT_OFFSET-timeLen-20-13);
                        }
                        else
                        {
                            Move(&trackRP, TEXT_OFFSET, y + SLOT_BASELINE);
                            ClipText(MPEGPlayerBase,&trackRP,buffer,len,PAGE_WIDTH-TEXT_OFFSET-timeLen-20);
                        }

                        Move(&trackRP, PAGE_WIDTH - timeLen - 10, y + SLOT_BASELINE);
                        ShadowText(MPEGPlayerBase,&trackRP,time,strlen(time),NULL);
                    }
                }

                y += SLOT_HEIGHT;
            }
        }
    }

    DrawTitle(MPEGPlayerBase,disk);
}


/*****************************************************************************/


VOID DeleteTracks(struct MPEGPlayerLib *MPEGPlayerBase)
{
    WaitBlit();                               // just to be safe...
    FreeBitMap(MPEGPlayerBase->mp_TrackBM);
    MPEGPlayerBase->mp_TrackBM = NULL;
}
