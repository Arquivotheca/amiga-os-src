
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <libraries/lowlevel.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include "utils.h"
#include "mpegplayerbase.h"
#include "defs.h"
#include "diskinfo.h"
#include "thumbnail.h"
#include "credits.h"


/*****************************************************************************/


struct Labels
{
    STRPTR Performer;
    STRPTR Writer;
    STRPTR Composer;
    STRPTR Arranger;
    STRPTR Player;
    STRPTR Producer;
    STRPTR Director;
};


static const struct Labels english =
			{"Performed By:",
			 "Written By:",
			 "Composed By:",
			 "Arranged By:",
			 "Played By:",
			 "Produced By:",
			 "Directed By:"};

static const struct Labels german =
		       	{"Darsteller:",
                         "Autor:",
                         "Komposition:",
                         "Arrangement:",
                         "Musiker:",
                         "Produktion:",
                         "Regie:"};

static const struct Labels dutch =
			{"Uitgevoerd door:",
			 "Geschreven door:",
		       	 "Gecomponeerd door:",
		       	 "Gearrangeerd door:",
		       	 "Gespeeld door:",
		       	 "Geproduceerd door:",
		       	 "Geregisseerd door:"};

static const struct Labels french =
			{"Interpr�t� par:",
                         "Ecrit par:",
                         "Compos� par:",
                         "Arrang� par:",
                         "Interpr�t� par:",
                         "Produit par:",
                         "Dirig� par:"};

static const struct Labels danish =
			{"Medvirkende:",
			 "Skrevet af:",
			 "Komponeret af:",
			 "Arrangeret af:",
			 "Indspillet af:",
			 "Produceret af:",
			 "Instrueret af:"};

static const struct Labels norwegian =
			{"Fremf�rt av:",
			 "Skrevet av:",
			 "Komponert av:",
			 "Arrangert av:",
			 "Spilt av:",
			 "Produsert av:",
			 "Regissert av:"};

static const struct Labels italian =
                        {"Eseguito da:",
                         "Scritto da:",
                         "Composto da:",
                         "Arrangiato da:",
                         "Suonato da:",
                         "Prodotto da:",
                         "Diretto da:"};

static const struct Labels portugues =
                        {"Performed By:",
                         "Argumento:",
                         "M�sica:",
                         "Adapta��o:",
                         "Played By:",
                         "Produ��o:",
                         "Realiza��o:"};


/*****************************************************************************/


void DrawCreditTitle(struct MPEGPlayerLib *MPEGPlayerBase,
                     struct CDSequence *track)
{
struct RastPort  *rp;
struct Rectangle *check;
UWORD             pix;
STRPTR            text;
ULONG             len;
ULONG             width;
ULONG             checkWidth;
char		  buffer[100];

    rp = MPEGPlayerBase->mp_RenderRP;

    SetABPenDrMd(rp,0,0,JAM1);
    RectFill(rp,TITLE_LEFT,TITLE_TOP,TITLE_RIGHT,TITLE_BOTTOM);

        SetABPenDrMd(rp,3,2,JAM1);
        SetFont(rp,&MPEGPlayerBase->mp_Compact31);

        check      = &iconsPos[CHECKMARK_ICON];
        checkWidth = check->MaxX - check->MinX + 1 + 10;
        width      = TITLE_WIDTH - checkWidth;

        text = track->cds_Name;
	if(text == NULL)
	{
		if (track->cds_AudioTrack)
		{
			nprintf(buffer,sizeof(buffer),"%ld  -  Audio",track->cds_TrackNumber);
		}
		else
		{
			nprintf(buffer,sizeof(buffer),"%ld  -  Video",track->cds_TrackNumber);
		}

		text = &buffer[0];
	}

        len  = strlen(text);
        pix  = ShadowTextLength(MPEGPlayerBase,rp,text,len);

        if (pix <= width)
        {
            Move(rp,TITLE_LEFT + checkWidth + (width-pix) / 2,TITLE_TOP+31);

            if (track->cds_Selected)
            {
                BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
                          rp->BitMap,rp->cp_x - checkWidth,TITLE_TOP+10,
                          check->MaxX - check->MinX + 1,check->MaxY - check->MinY + 1,0xc0,0x03,NULL);
            }

            ShadowText(MPEGPlayerBase,rp,text,len,NULL);
        }
        else
        {
            Move(rp,TITLE_LEFT+checkWidth,TITLE_TOP+31);

            if (track->cds_Selected)
            {
                BltBitMap(MPEGPlayerBase->mp_IconBitMap,check->MinX,check->MinY,
                          rp->BitMap,TITLE_LEFT+4,TITLE_TOP+10,
                          check->MaxX - check->MinX + 1,check->MaxY - check->MinY + 1,0xc0,0x03,NULL);
            }

            ClipText(MPEGPlayerBase,rp,text,len,width);
        }

    SwapBuffers(MPEGPlayerBase);

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],TITLE_LEFT,TITLE_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,TITLE_LEFT,TITLE_TOP,
              TITLE_WIDTH,TITLE_HEIGHT,0xc0,0x03,NULL);
}


/*****************************************************************************/


#define SCROLL_AMOUNT_Y 3
#define INTER_ITEMS     12


VOID ScrollCredits(struct MPEGPlayerLib *MPEGPlayerBase, LONG direction)
{
WORD i;
LONG top;

    if (!MPEGPlayerBase->mp_CreditsBM)
        return;

    top = MPEGPlayerBase->mp_OldCreditTop;

    if (direction < 0)
    {
        i = 0;
        do
        {
            if (top < SCROLL_AMOUNT_Y)
            {
                i   = SLOT_HEIGHT*2;
                top = 0;
            }
            else
            {
                i   += SCROLL_AMOUNT_Y;
                top -= SCROLL_AMOUNT_Y;
            }

            BltBitMap(MPEGPlayerBase->mp_CreditsBM,0,top,
                      MPEGPlayerBase->mp_RenderRP->BitMap,CREDIT_LEFT,PAGE_TOP,
                      CREDIT_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);

            SwapBuffers(MPEGPlayerBase);
        }
        while ((i < SLOT_HEIGHT*2) && top);

        MPEGPlayerBase->mp_OldCreditTop = top;
    }
    else if (direction > 0)
    {
        i = 0;
        while ((i < SLOT_HEIGHT*2) && (top+PAGE_HEIGHT < MPEGPlayerBase->mp_CreditsBMHeight))
        {
            if (top + PAGE_HEIGHT + SCROLL_AMOUNT_Y >= MPEGPlayerBase->mp_CreditsBMHeight)
            {
                i   = SLOT_HEIGHT*2;
                top = MPEGPlayerBase->mp_CreditsBMHeight - PAGE_HEIGHT;
            }
            else
            {
                i   += SCROLL_AMOUNT_Y;
                top += SCROLL_AMOUNT_Y;
            }

            BltBitMap(MPEGPlayerBase->mp_CreditsBM,0,top,
                      MPEGPlayerBase->mp_RenderRP->BitMap,CREDIT_LEFT,PAGE_TOP,
                      CREDIT_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);

            SwapBuffers(MPEGPlayerBase);
        }

        MPEGPlayerBase->mp_OldCreditTop = top;
    }
    else
    {
        MPEGPlayerBase->mp_OldCreditTop = 0;

        BltBitMap(MPEGPlayerBase->mp_CreditsBM,0,MPEGPlayerBase->mp_OldCreditTop,
                  MPEGPlayerBase->mp_RenderRP->BitMap,CREDIT_LEFT,PAGE_TOP,
                  CREDIT_WIDTH,PAGE_HEIGHT,0xc0,0xff,NULL);

        SwapBuffers(MPEGPlayerBase);
    }

    BltBitMap(MPEGPlayerBase->mp_DBBitMap[1 - MPEGPlayerBase->mp_DBCurrentBuffer],CREDIT_LEFT,PAGE_TOP,
              MPEGPlayerBase->mp_RenderRP->BitMap,CREDIT_LEFT,PAGE_TOP,
              CREDIT_WIDTH,PAGE_HEIGHT,0xc0,0x03,NULL);
}


/*****************************************************************************/


static VOID DoOne(struct MPEGPlayerLib *MPEGPlayerBase,
		  struct RastPort *rp, STRPTR label, STRPTR text, ULONG *y)
{
    if (text)
    {
        if (rp)
        {
            SetABPenDrMd(rp,1,2,JAM1);
            Move(rp,0,*y + SLOT_BASELINE);
            ClipText(MPEGPlayerBase,rp,label,strlen(label),CREDIT_WIDTH);
        }
        *y += SLOT_HEIGHT;

        if (rp)
        {
            SetABPenDrMd(rp,3,2,JAM1);
            Move(rp,20,*y + SLOT_BASELINE);
            ClipText(MPEGPlayerBase,rp,text,strlen(text),CREDIT_WIDTH - 20);
        }
        *y += SLOT_HEIGHT + INTER_ITEMS;
    }
}


/*****************************************************************************/


static ULONG DoEm(struct MPEGPlayerLib *MPEGPlayerBase,
                  struct CDSequence *track, struct RastPort *rp)
{
ULONG          y;
struct Labels *labels;

    switch (GetLanguageSelection())
    {
        case LANG_GERMAN     : labels = &german; break;
        case LANG_FRENCH     : labels = &french; break;
//        case LANG_SPANISH    : labels = &spanish; break;
        case LANG_ITALIAN    : labels = &italian; break;
        case LANG_PORTUGUESE : labels = &portugues; break;
        case LANG_DANISH     : labels = &danish; break;
        case LANG_DUTCH      : labels = &dutch; break;
        case LANG_NORWEGIAN  : labels = &norwegian; break;
//        case LANG_SWEDISH    : labels = &swedish; break;
        default              : labels = &english; break;
    }

    y = SLOT_HEIGHT / 2;

    DoOne(MPEGPlayerBase,rp,labels->Performer,track->cds_Performer,&y);
    DoOne(MPEGPlayerBase,rp,labels->Writer,track->cds_Writer,&y);
    DoOne(MPEGPlayerBase,rp,labels->Composer,track->cds_Composer,&y);
    DoOne(MPEGPlayerBase,rp,labels->Arranger,track->cds_Arranger,&y);
    DoOne(MPEGPlayerBase,rp,labels->Player,track->cds_Player,&y);
    DoOne(MPEGPlayerBase,rp,labels->Producer,track->cds_Producer,&y);
    DoOne(MPEGPlayerBase,rp,labels->Director,track->cds_Director,&y);

    y -= INTER_ITEMS;

    return (y + (SLOT_HEIGHT / 2));
}


/*****************************************************************************/


VOID CreateCredits(struct MPEGPlayerLib *MPEGPlayerBase,
		   struct CDSequence *track)
{
struct RastPort    creditRP;
UWORD              pixels;

    InitRastPort(&creditRP);
    SetFont(&creditRP,&MPEGPlayerBase->mp_Compact24);
    SetABPenDrMd(&creditRP,3,2,JAM1);

    pixels = DoEm(MPEGPlayerBase,track,NULL);
    if (pixels < PAGE_HEIGHT)
        pixels = PAGE_HEIGHT;

    MPEGPlayerBase->mp_CreditsBM       = AllocBitMap(CREDIT_WIDTH,pixels,2,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    MPEGPlayerBase->mp_CreditsBMHeight = pixels;
    creditRP.BitMap = MPEGPlayerBase->mp_CreditsBM;

    DoEm(MPEGPlayerBase,track,&creditRP);

    DrawCreditTitle(MPEGPlayerBase,track);
    StartThumbnail(MPEGPlayerBase,track);
    ScrollCredits(MPEGPlayerBase,0);
}


/*****************************************************************************/


VOID DeleteCredits(struct MPEGPlayerLib *MPEGPlayerBase)
{
    StopThumbnail(MPEGPlayerBase);
    WaitBlit();
    FreeBitMap(MPEGPlayerBase->mp_CreditsBM);
    MPEGPlayerBase->mp_CreditsBM = NULL;
}
