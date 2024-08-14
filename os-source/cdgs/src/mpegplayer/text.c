
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "utils.h"
#include "mpegplayerbase.h"
#include "defs.h"
#include "tracks.h"


/*****************************************************************************/


#define ln_Length ln_Type


/*****************************************************************************/


static void DrawPageTitle(struct MPEGPlayerLib *MPEGPlayerBase, STRPTR text)
{
#if (DO_TEXTDISPLAYS)
struct RastPort *rp;

    rp = MPEGPlayerBase->mp_RenderRP;

    SetABPenDrMd(rp,0,0,JAM1);
    RectFill(rp,PAGE_LEFT,PAGE_TOP,PAGE_LEFT + PAGE_WIDTH - 1,TEXT_TOP-1);

    if (text)
    {
        Move(rp,PAGE_LEFT+4,PAGE_TOP+30);

        if (MPEGPlayerBase->mp_CurrentPage <= PAGETYPE_TRACKS)
            SetABPenDrMd(rp,3,2,JAM1);
        else
            SetABPenDrMd(rp,1,2,JAM1);

        SetFont(rp,&MPEGPlayerBase->mp_Compact31);
        CenterText(MPEGPlayerBase,rp,text,strlen(text),PAGE_WIDTH-8);
    }
#endif
}


/*****************************************************************************/


static VOID RenderText(struct MPEGPlayerLib *MPEGPlayerBase, STRPTR title)
{
#if (DO_TEXTDISPLAYS)
struct Rectangle rect;
WORD             y;
struct RastPort *rp;
struct Node     *node;

    rect.MinX = TEXT_LEFT;
    rect.MinY = TEXT_TOP;
    rect.MaxX = TEXT_LEFT + TEXT_WIDTH - 1;
    rect.MaxY = TEXT_TOP + TEXT_HEIGHT - 1;

    rp = MPEGPlayerBase->mp_RenderRP;
    SetFont(rp,&MPEGPlayerBase->mp_Compact24);

    SetABPenDrMd(rp,0,0,JAM1);
    RectFill(rp,PAGE_LEFT,TEXT_TOP,PAGE_LEFT + PAGE_WIDTH - 1,TEXT_TOP + TEXT_HEIGHT - 1);

    SetABPenDrMd(rp,1,2,JAM1);

    y = TEXT_TOP + (SLOT_HEIGHT / 2);
    SCANLIST(&MPEGPlayerBase->mp_TextList,node)
    {
        if (y > TEXT_TOP + TEXT_HEIGHT + SLOT_HEIGHT)
            break;

        if (node->ln_Name)
        {
            Move(rp,TEXT_LEFT,y + rp->TxBaseline);
            ShadowText(MPEGPlayerBase,rp,node->ln_Name,node->ln_Length,&rect);
            y += SLOT_HEIGHT;
        }
        else
        {
            y += 10;
        }
    }

    SwapBuffers(MPEGPlayerBase);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT,PAGE_TOP,
              MPEGPlayerBase->mp_DBBitMap[MPEGPlayerBase->mp_DBCurrentBuffer],PAGE_LEFT,PAGE_TOP,
              PAGE_WIDTH,PAGE_HEIGHT,0xc0,0x03,NULL);
#endif
}


/*****************************************************************************/


void ScrollText(struct MPEGPlayerLib *MPEGPlayerBase, LONG direction)
{
}


/*****************************************************************************/


VOID CreateText(struct MPEGPlayerLib *MPEGPlayerBase,
                STRPTR title, STRPTR text, ULONG textLen)
{
#if (DO_TEXTDISPLAYS)
struct Node *node;
UWORD        chars, len;
ULONG        paraLen;

    NewList((struct List *)&MPEGPlayerBase->mp_TextList);

    if (!text)
       return;

    MPEGPlayerBase->mp_TextPool = CreatePool(MEMF_ANY,sizeof(struct Node)*32,128);

    DrawPageTitle(MPEGPlayerBase,title);

    SetFont(MPEGPlayerBase->mp_RenderRP,&MPEGPlayerBase->mp_Compact24);

    while (textLen)
    {
        paraLen = 0;
        while ((paraLen < textLen) && (text[paraLen] != '\n'))
            paraLen++;

        textLen -= paraLen;
        if (textLen)
            textLen--;

        if (paraLen > 0)
        {
            while (paraLen)
            {
                node = AllocPooled(MPEGPlayerBase->mp_TextPool,sizeof(struct Node));
                AddTail((struct List *)&MPEGPlayerBase->mp_TextList,node);

                node->ln_Name = text;

                chars = ShadowFit(MPEGPlayerBase,MPEGPlayerBase->mp_RenderRP,text,paraLen,TEXT_WIDTH);
                len = chars;

                if (len < paraLen)
                {
                    while (len && (text[len] != ' '))
                        len--;

                    if (len == 0)
                        len = chars;
                }

                node->ln_Length = len;

                if (len < paraLen)
                    len++;

                while (len < paraLen && (text[len] == ' '))
                    len++;

                text     = &text[len];
                paraLen -= len;
            }
        }
        else
        {
            // paragraph separator
            node = AllocPooled(MPEGPlayerBase->mp_TextPool,sizeof(struct Node));
            AddTail((struct List *)&MPEGPlayerBase->mp_TextList,node);
            node->ln_Name   = "";
            node->ln_Length = 0;
        }

        text = &text[1];   // skip over \n
    }

    MPEGPlayerBase->mp_TopNode = MPEGPlayerBase->mp_TextList.mlh_Head;

/*
    SCANLIST(&MPEGPlayerBase->mp_TextList,node)
    {
    char buffer[30];

        nprintf(buffer,sizeof(buffer),"%%-.%lds\n",node->ln_Length);
        kprintf(buffer,node->ln_Name);
    }
*/

    RenderText(MPEGPlayerBase,title);
#endif
}


/*****************************************************************************/


VOID DeleteText(struct MPEGPlayerLib *MPEGPlayerBase)
{
#if (DO_TEXTDISPLAYS)
    DeletePool(MPEGPlayerBase->mp_TextPool);
    MPEGPlayerBase->mp_TextPool = NULL;
    MPEGPlayerBase->mp_TopNode  = NULL;
#endif
}
