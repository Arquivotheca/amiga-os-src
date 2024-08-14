
/********************************************************************************
*   sdraw2.c
**
**  Purpose: CDTV screen saver
**  Compile&link: SAS/C Lattice-C  v5.10a
**
**  Date: Mar 4, 1992
**  Copyright (C) 1992, 1993 Commodore Amiga Inc.  All rights reserved.
**
********************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <libraries/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>

#include <hardware/custom.h>

#include "/playerprefsbase.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include <cdtv/debox.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <string.h>

#include "splinesaver.h"


/*******************************************************************************/
/*****    E X T E R N A L    F U N C T I O N S    D E C L A R A T I O N    *****/
/*******************************************************************************/

extern struct Custom __far custom;

void __stdargs DrawSpline(LONG oi, LONG x, LONG y, LONG x3, LONG y3);

/* pragma spoofs */
#define SysBase     (G->PPBase->ppb_ExecBase)
#define GfxBase     (G->PPBase->ppb_GfxBase)
#define DeBoxBase   (G->PPBase->ppb_DeBoxBase)

/*******************************************************************************/
/*****    I N T E R N A L    F U N C T I O N S    D E C L A R A T I O N    *****/
/*******************************************************************************/

void  blk(struct SaverGlobals *G);
WORD  Random(struct SaverGlobals *G,WORD);
void  MakeFunc(struct SaverGlobals *G);
void  Draw_S_F(struct SaverGlobals *G,register WORD *,register WORD *);
void  Draw_SF(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawS_F(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawSF(struct SaverGlobals *G,register WORD *,register WORD *);
void  Draw_LF(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawL_F(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawLF(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawnLF(struct SaverGlobals *G,register WORD *,register WORD *);
void  DrawFunc(struct SaverGlobals *G,struct box *);
void  Adv(struct SaverGlobals *G,WORD *,WORD *,WORD *,WORD);
void  AdvanceLines(struct SaverGlobals *G);
void  DrawNew(struct SaverGlobals *G);
void  StartLines(struct SaverGlobals *G);
void  Colors(struct SaverGlobals *G);



/********************************************************************************
**
** void blk(struct SaverGlobals *G)
**
** function:
**  Make view and blank display.
**
********************************************************************************/

void  blk(struct SaverGlobals *G) {

    G->g2->mdelta = -1;
    G->g2->maxlines = MAXLINES/2;
    G->g2->lastanim_secs = G->idata.cursec;                                             /* Process drawing screen saver stuff. */
    }



void UnSetBlanker(struct SaverGlobals *G) {

int i;

    for(i = 0;i < DEPTH;i++) {
        if(G->bm->Planes[i]) FreeRaster(G->bm->Planes[i],WIDTH,HEIGHT);
        }
    }


void SetUpBlanker(struct SaverGlobals *G) {

int i;

    InitBitMap(&G->g2->bitMap,DEPTH,WIDTH,HEIGHT);                                              /* Initialize the BitMaps. */

    for(i = 0;i < DEPTH;i++) G->g2->bitMap.Planes[i] = (PLANEPTR)AllocRaster(WIDTH,HEIGHT);     /* Allocate space for their Planes. */
    
    InitRastPort(&G->g2->rastPort);                                                             /* Initialize the RastPorts and link the BitMaps to them. */

    G->g2->rastPort.BitMap = &G->g2->bitMap;

    SetRast(&G->g2->rastPort,0);                                                                /* Simple form of setting drawing area to color 0. */

    G->g2->gfxbase = GfxBase;
    G->bm = &G->g2->bitMap;
    }



