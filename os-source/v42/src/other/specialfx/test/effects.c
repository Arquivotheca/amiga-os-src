;/*
SC effects.c link nostkchk LIB lib:debug.lib
quit
*/

#include <exec/types.h>
#include <exec/libraries.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>
#include <graphics/layers.h>
#include <graphics/gfxbase.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <graphics/gfx.h>
#include <graphics/videocontrol.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/specialfx.h>
#include <string.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
//#include <clib/specialfx_protos.h>

#include <pragmas/exec_pragmas.h>
//#include <pragmas/datatypes_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
//#include <pragmas/specialfx_pragmas.h>
#include <proto/SpecialFX_protos.h>
#include <pragmas/SpecialFX_pragmas.h>


/*****************************************************************************/



BOOL LoadBM(STRPTR picName);
VOID FreeBM(VOID);
VOID DoCoolStuff(VOID);
ULONG __stdargs DoMethodA (Object *obj, APTR message);


/*****************************************************************************/


#define	TEMPLATE  "NAME/A"
#define OPT_NAME  0
#define	OPT_COUNT 1


/*****************************************************************************/


extern struct Library *SysBase;
extern struct Library *DOSBase;
extern struct Library *IntuitionBase;
extern struct GfxBase *GfxBase;

struct Library *DataTypesBase;
struct Library *SpecialFXBase;
struct Screen  *screen;

Object              *obj;
struct BitMap       *bm;
struct BitMapHeader *bmhd;
ULONG                modeID;
ULONG               *cmap;
ULONG                numColors;


/*****************************************************************************/


void main(void)
{
struct RDArgs   *rdargs;
LONG             opts[OPT_COUNT];
struct Window   *window;
ULONG            r,g,b,i;

    DataTypesBase = OpenLibrary("datatypes.library",0);
    SpecialFXBase = OpenLibrary("specialfx.library",0);

    memset(opts,0,sizeof(opts));
    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
    {
        if (LoadBM((STRPTR)opts[OPT_NAME]))
        {
            if (screen = OpenScreenTags(NULL,SA_Left,         0, //- (bmhd->bmh_Width / 2),
                                             SA_Top,          2,
                                             SA_Width,        bmhd->bmh_Width * 2,
                                             SA_Height,       bmhd->bmh_Height + 4,
                                             SA_Depth,        bmhd->bmh_Depth,
                                             SA_Overscan,     OSCAN_TEXT,
                                             SA_DisplayID,    0x11000,
                                             SA_BackFill,     LAYERS_NOBACKFILL,
                                             SA_AutoScroll,   TRUE,
                                             TAG_END))
            {
                /* Load the color map */
                if (cmap)
                {
                    for (i = 0; i < numColors; i++)
                    {
                        r = cmap[i * 3 + 0];
                        g = cmap[i * 3 + 1];
                        b = cmap[i * 3 + 2];
                        SetRGB32(&screen->ViewPort, i, r, g, b);
                    }
                }

                if (window = OpenWindowTags(NULL,WA_CustomScreen, screen,
                                                 WA_IDCMP,        IDCMP_RAWKEY,
                                                 WA_Borderless,   TRUE,
                                                 WA_Activate,     TRUE,
                                                 TAG_DONE))
                {
                    BltBitMap(bm,0,0,
                              screen->RastPort.BitMap,/*bmhd->bmh_Width / 2*/ 0,2,
                              bmhd->bmh_Width,bmhd->bmh_Height,192,0xff,NULL);

                    DoCoolStuff();

                    WaitPort(window->UserPort);
                    CloseWindow(window);
                }

                CloseScreen(screen);
            }

            FreeBM();
        }

        FreeArgs(rdargs);
    }
    else
    {
        PrintFault(IoErr(),NULL);
    }

    CloseLibrary(DataTypesBase);
    CloseLibrary(SpecialFXBase);
}


/*****************************************************************************/


#define NUM_LINECONTROL 10

void DoCoolStuff(void)
{
APTR                 linectrl;
APTR                 dh;
struct LineControl **lc;
struct RasInfo       ri[NUM_LINECONTROL];
UWORD                i, j;

    if (linectrl = AllocFX(&screen->ViewPort, SFX_LineControl, NUM_LINECONTROL, NULL))
    {
        lc = linectrl;

        for (i = 0; i < NUM_LINECONTROL; i++)
        {
            ri[i] = *screen->ViewPort.RasInfo;
            ri[i].RyOffset    = 2 + i*10;

            lc[i]->lc_RasInfo = &ri[i];
            lc[i]->lc_Line    = 2 + i*10;
            lc[i]->lc_Count   = 10;
        }

	if (dh = InstallFX(ViewAddress(), &screen->ViewPort,
                           SFX_InstallEffect, linectrl,
                           TAG_DONE))
        {
            MakeScreen(screen);
            RethinkDisplay();

            for (i = 0; i < NUM_LINECONTROL; i++)
                lc[i]->lc_Flags = LCF_MODIFY;

            for (i = 0; i < bmhd->bmh_Width; i += 4)
            {
                for (j = 0; j < NUM_LINECONTROL; j += 2)
                {
                    ri[j].RxOffset += 4;
                    ri[j+1].RxOffset -= 4;
                }

                WaitTOF();
                AnimateFX(dh);
            }

            RemoveFX(dh);
	}
	FreeFX(linectrl);
    }
}


/*****************************************************************************/


BOOL LoadBM(STRPTR picName)
{
struct gpLayout gpl;

    if (obj = NewDTObject(picName,
                          DTA_SourceType,        DTST_FILE,
                          PDTA_Remap,            FALSE,
                          PDTA_FreeSourceBitMap, TRUE,
                          TAG_DONE))
    {
        gpl.MethodID    = DTM_PROCLAYOUT;
        gpl.gpl_GInfo   = NULL;
        gpl.gpl_Initial = 1;

        if (DoMethodA(obj,&gpl))
        {

            GetDTAttrs(obj,PDTA_BitMapHeader, &bmhd,
                           PDTA_BitMap,       &bm,
                           PDTA_ModeID,       &modeID,
                           PDTA_CRegs,        &cmap,
                           PDTA_NumColors,    &numColors,
                           TAG_DONE);

            if (bm)
                return(TRUE);
        }

        DisposeDTObject(obj);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID FreeBM(void)
{
    DisposeDTObject(obj);
}
