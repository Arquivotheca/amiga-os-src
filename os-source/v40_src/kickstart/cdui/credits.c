
/* NOTE: text strings in this file are be encrypted in order to avoid
 *       snoops from looking at the source code and seeing all this stuff.
 *
 *         YES BUB! THAT MEANS YOU!
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/videocontrol.h>
#include <graphics/view.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include "credits.h"
#include "cduibase.h"
#include "utils.h"


/*****************************************************************************/


static VOID SetPix(struct CDUILib *CDUIBase, LONG pnum, LONG rx, LONG ry)
{
    rx = (rx % 320);
    ry = (ry % 200);

    if (rx<0)
        rx = 320 + rx;

    if (ry<0)
        ry = 200 + ry;

    CDUIBase->cb_CreditVP->RasInfo->BitMap->Planes[pnum] = ry*80+CDUIBase->cb_CreditBM[rx & 15].Planes[0]+(rx>>4)*2;
}


/*****************************************************************************/


static VOID PrintStr(struct CDUILib *CDUIBase, const STRPTR str)
{
char  decrypt[80];
UWORD i;

    i = 0;
    while (str[i])
    {
        if (((str[i] >= 'A') && (str[i] <= 'M')) || ((str[i] >= 'a') && (str[i] <= 'm')))
        {
            decrypt[i] = str[i] + 13;
        }
        else if (((str[i] >= 'N') && (str[i] <= 'Z')) || ((str[i] >= 'n') && (str[i] <= 'z')))
        {
            decrypt[i] = str[i] - 13;
        }
        else
        {
            decrypt[i] = str[i];
        }
        i++;
    }
    decrypt[i] = 0;

    Text(CDUIBase->cb_CreditRP,decrypt,i);
    Move(CDUIBase->cb_CreditRP,8,CDUIBase->cb_CreditRP->cp_y+ CDUIBase->cb_CreditRP->TxHeight + 1);
}


/*****************************************************************************/


static LONG Random(struct CDUILib *CDUIBase)
{
LONG rseed;
LONG rval;

    CurrentTime(&rval,&rseed);
    rseed += 323214521;

    rseed = rseed * 123213 + 121;
    rval  = (rseed >> 5) & 65535;
    return ((6 * rval) >> 16);
}


/*****************************************************************************/


static VOID Adv(struct CDUILib *CDUIBase, WORD *o, WORD *d, WORD *n)
{
    *n = *o + *d;
    if (*n < 0)
    {
        *n = 0;
        *d = Random(CDUIBase) + 1;
    }
    else if (*n >= 128)
    {
        *n = 127;
        *d = - Random(CDUIBase) - 1;
    }
}


/*****************************************************************************/


VOID DoCredits(struct CDUILib *CDUIBase, LONG creditType)
{
struct Screen   *sp;
struct BitMap    planebm[16];
struct RastPort  rp;
struct BitMap    sbm;
struct ViewPort *vp;
LONG             i;
WORD             dr, dg, db;
WORD             or, og, ob;
WORD             nr, ng, nb;

    FadeOut(CDUIBase);

    CDUIBase->cb_CreditRP = &rp;
    CDUIBase->cb_CreditBM = planebm;

    for (i = 0; i < 16; i++)
    {
        InitBitMap(planebm+i,1,640,400);
        planebm[i].BytesPerRow = 80;  /* make sure it is correct */
        planebm[i].Planes[0]   = AllocVec(80*400,MEMF_CHIP|MEMF_CLEAR);
    }

    sbm       = planebm[0];
    sbm.Depth = 8;

    InitRastPort(&rp);
    rp.BitMap = &planebm[0];
    SetFont(&rp,OpenFont(&topazAttr));
    SetABPenDrMd(&rp,-1,0,JAM1);

    if (creditType < 0)
    {
        Move(&rp,8,8);
        PrintStr(CDUIBase," ");
        PrintStr(CDUIBase,"Fbsgjner            Uneqjner");
        PrintStr(CDUIBase," Wnzrf Onexyrl We    Puevf Pbyrl");
        PrintStr(CDUIBase," Qnir Oehzonhtu      Urqyrl Qnivf");
	PrintStr(CDUIBase," Crgre Purean        Yrj Rttroerpug");
        PrintStr(CDUIBase," Revp Pbggba         Qnagr Snoevmvb");
	PrintStr(CDUIBase," Puevf Terra         Rq Thagure");
        PrintStr(CDUIBase," Qneera Terrajnyq    Wrss Cbegre");
        PrintStr(CDUIBase," Nyyna Unirzbfr      Ovyy Evpuneq");
        PrintStr(CDUIBase," Wreel Ubenabss      Trbetr Eboovaf");
        PrintStr(CDUIBase," Enaqryy Wrfhc");
        PrintStr(CDUIBase," Qnivq A. Whabq     Puvcf");
        PrintStr(CDUIBase," Cnyzlen Cnjyvx      Rq Urcyre");
        PrintStr(CDUIBase," Fcrapre Funafba     Obo Envoyr");
        PrintStr(CDUIBase," Zvpunry Fvam        Ovyy Gubznf");
        PrintStr(CDUIBase," Znegva Gnvyyrsre");
        PrintStr(CDUIBase,"                    Zrpunavpny Qrfvta");
        PrintStr(CDUIBase,"Negjbex:             Wvz Ubbcre");
        PrintStr(CDUIBase," Wreel Unegmyre      Ureo Zbfgryyre");
        PrintStr(CDUIBase," Wvz Fnpuf           Puhpx Jbbgref");
        PrintStr(CDUIBase,"      ... Naq Znal Zber ...");
    }

    BltBitMap(planebm,0,0,planebm,320,0,320,200,0xc0,1,0);
    BltBitMap(planebm,0,0,planebm,320,200,320,200,0xc0,1,0);
    BltBitMap(planebm,0,0,planebm,0,200,320,200,0xc0,1,0);

    for (i = 1; i < 16; i++)
    {
        BltBitMap(planebm,i,0,planebm+i,0,0,320,200,0xc0,1,0);
        BltBitMap(planebm,i,0,planebm+i,320,0,320-i,200,0xc0,1,0);
        BltBitMap(planebm,i,0,planebm+i,320,200,320-i,200,0xc0,1,0);
        BltBitMap(planebm,i,0,planebm+i,0,200,320,200,0xc0,1,0);
    }

    for (i = 0; i < 8; i++)
        sbm.Planes[i] = planebm[0].Planes[0];

    sp = OpenScreenTags(NULL,SA_Width,     320,
                             SA_Height,    200,
                             SA_Depth,     8,
                             SA_DisplayID, LORES_KEY,
                             SA_BitMap,    &sbm,
                             SA_Quiet,     TRUE,
                             SA_Behind,    TRUE,
                             SA_Draggable, FALSE,
                             SA_Font,      &topazAttr,
                             TAG_DONE);

    vp                    = &sp->ViewPort;
    CDUIBase->cb_CreditVP = vp;

    for (i = 0; i < 256; i++)
    {
    LONG r, g, b;

        r = (i & 128)+(i & 64);
        g = 4*((i & 32)+(i & 16)+(i & 8));
        b = 32*((i & 4)+(i & 2)+(i & 1));

        SetRGB4(vp,i,r/16,g/16,b/16);
    }

    VideoControlTags(vp->ColorMap,VC_IntermediateCLUpdate,0,0);

    ScreenToFront(sp);
    for (i = 1200; i >= 0; i -= 3)
    {
        SetPix(CDUIBase,0,i,0);
        SetPix(CDUIBase,1,i,i);
        SetPix(CDUIBase,2,0,i);
        SetPix(CDUIBase,3,-i,0);
        SetPix(CDUIBase,4,-i,-i);
        SetPix(CDUIBase,5,i,-i);
        SetPix(CDUIBase,6,-i,i);
        SetPix(CDUIBase,7,0,-i);
        WaitTOF();
        while (VBeamPos() < 30) ;
        ScrollVPort(vp);
    }

    for (i = 15; i >= 0; i--)
    {
        WaitTOF();
        SetRGB4(vp,255,15,i,i);
    }

    for (i = 1; i < 11; i++)
        SetRGB4(vp,i,0,i+4,i+4);

    nr = 53;
    ng = 33;
    nb = 35;
    dr = -3;
    dg = 5;
    db = 7;

    for (i = 0; i < 450; i++)
    {
        or = nr;
        og = ng;
        ob = nb;
        Adv(CDUIBase, &or, &dr, &nr);
        Adv(CDUIBase, &og, &dg, &ng);
        Adv(CDUIBase, &ob, &db, &nb);

        SetRGB4(vp,255,(nr >> 3), (ng >> 3), (nb >> 3));
        WaitTOF();
    }

    for (i = 0; i < 30; i++)
        ScreenPosition(sp,SPOS_RELATIVE|SPOS_FORCEDRAG,0,10,0,0);

    CloseScreen(sp);
    for (i = 0; i < 16; i++)
        FreeVec(planebm[i].Planes[0]);

    FadeIn(CDUIBase);
}
