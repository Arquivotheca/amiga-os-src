#include <exec/types.h>
#include <exec/memory.h>
#include <hardware/intbits.h>

#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/pointerclass.h>
#include <internal/librarytags.h>

#include <libraries/debox.h>

#include "player.h"

#include <clib/debox_protos.h>
#include <pragmas/debox_pragmas.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/battclock_protos.h>
#include <clib/intuition_protos.h>

#define EXEC_PRIVATE_PRAGMAS
struct Library *TaggedOpenLibrary(ULONG tag);
#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/battclock_pragmas.h>
#include <pragmas/intuition_pragmas.h>

static ULONG Replicate(ULONG colorByte);

extern ULONG __asm LibAnimationControlA(register __a0 struct View *,
    register __a1 struct ViewPort *, register __a2 struct TagItem *, register __a6 struct GfxBase *GfxBase );





#define SysBase         (*((LONG *)4))
#define LowLevelBase    V->LowLevelBase
#define DeBoxBase       V->DeBoxBase
#define IntuitionBase   V->IntuitionBase
#define GfxBase         V->GfxBase


/* We want a copper list which makes color 0 have the following vertical
 * spread:
 */
#define START_RED   60     /* Original CDTV background: 119 */
#define START_GREEN 110    /*                           170 */
#define START_BLUE  200    /*                           255 */
#define END_RED     18     /*                           221 */
#define END_GREEN   8      /*                           170 */
#define END_BLUE    60     /*                           153 */

#if START_RED > END_RED
#define RED_DIFF (START_RED - END_RED)
#else
#define RED_DIFF (END_RED - START_RED)
#endif

#if START_GREEN > END_GREEN
#define GREEN_DIFF (START_GREEN - END_GREEN)
#else
#define GREEN_DIFF (END_GREEN - START_GREEN)
#endif

#if START_BLUE > END_BLUE
#define BLUE_DIFF (START_BLUE - END_BLUE)
#else
#define BLUE_DIFF (END_BLUE - START_BLUE)
#endif


/*****************************************************************************/


static ULONG Replicate(ULONG colorByte)
{
    return ((colorByte << 24) | (colorByte << 16) | (colorByte << 8) | colorByte);
}


/*****************************************************************************/

VOID FadeIn(struct V *V)
{
ULONG            i,j,r,g, b;
ULONG            rInc, gInc, bInc;
struct ColorPack colors;
struct TagItem   tags[2];
ULONG copperlines = V->Screen->Height/COPPER_RATE;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    for (i = 0; i < copperlines; i++)
    {
        /* Allocated MEMF_CLEAR:
         * PlayerColorRange[i].cor_Pen    = 0;
         * PlayerColorRange[i].cor_WAIT_X = 0;
         */
        V->ColorRange[i].cor_WAIT_Y = i*COPPER_RATE;
        V->ColorRange[i].cor_Flags  = CORF_MODIFY;
    }
    
    V->ColorRange[i].cor_Pen = -1;

    rInc = Replicate(RED_DIFF) / (copperlines - 1);
    gInc = Replicate(GREEN_DIFF) / (copperlines - 1);;
    bInc = Replicate(BLUE_DIFF) / (copperlines - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)V->ColorRange;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (V->ColorPack.cp_Colors[j].Red / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Green = (V->ColorPack.cp_Colors[j].Green / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Blue  = (V->ColorPack.cp_Colors[j].Blue / NUM_FADE_STEPS) * i;
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < copperlines; j++)
        {
            V->ColorRange[j].cor_Red   = (r / NUM_FADE_STEPS) * i;
            V->ColorRange[j].cor_Green = (g / NUM_FADE_STEPS) * i;
            V->ColorRange[j].cor_Blue  = (b / NUM_FADE_STEPS) * i;

#if END_RED > START_RED
            r += rInc;
#else
        r -= rInc;
#endif

#if END_GREEN > START_GREEN
            g += gInc;
#else
        g -= gInc;
#endif

#if END_BLUE > START_BLUE
            b += bInc;
#else
        b -= bInc;
#endif
        }

        WaitTOF();
        LibAnimationControlA(ViewAddress(),&V->Screen->ViewPort,tags,GfxBase);
        LoadRGB32(&V->Screen->ViewPort,(ULONG *)&colors);

        if (i == 0)
        {
            V->ColorRange[0].cor_Flags = CORF_MODIFY|CORF_ANIMATE;
            RemakeDisplay();
        }
    }

    LoadRGB32(&V->Screen->ViewPort,(ULONG *)&V->ColorPack);
}


/*****************************************************************************/


VOID FadeOut(struct V *V) {
ULONG            i,j;
struct ColorPack colors;
ULONG            r,g,b;
ULONG            rInc, gInc, bInc;
struct TagItem   tags[2];
ULONG copperlines = V->Screen->Height/COPPER_RATE;

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    rInc = Replicate(RED_DIFF) / (copperlines - 1);
    gInc = Replicate(GREEN_DIFF) / (copperlines - 1);;
    bInc = Replicate(BLUE_DIFF) / (copperlines - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)V->ColorRange;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (V->ColorPack.cp_Colors[j].Red / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Green = (V->ColorPack.cp_Colors[j].Green / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Blue  = (V->ColorPack.cp_Colors[j].Blue / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < copperlines; j++)
        {
            V->ColorRange[j].cor_Red   = (r / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            V->ColorRange[j].cor_Green = (g / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            V->ColorRange[j].cor_Blue  = (b / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
#if END_RED > START_RED
            r += rInc;
#else
        r -= rInc;
#endif

#if END_GREEN > START_GREEN
            g += gInc;
#else
        g -= gInc;
#endif

#if END_BLUE > START_BLUE
            b += bInc;
#else
        b -= bInc;
#endif
        }

        WaitTOF();
        LibAnimationControlA(ViewAddress(),&V->Screen->ViewPort,tags,GfxBase);
        LoadRGB32(&V->Screen->ViewPort,(ULONG *)&colors);

    }

    for (i = 0; i < NUM_COLORS; i++)
    {
        colors.cp_Colors[i].Red   = 0;
        colors.cp_Colors[i].Green = 0;
        colors.cp_Colors[i].Blue  = 0;
    }
    LoadRGB32(&V->Screen->ViewPort,(ULONG *)&colors);
}


static void __saveds taskLoop(void) {

struct V            *V;
struct ColorChanger *cc;
struct CycleRange   *cr;
struct RangeInfo    *rgi;
int                  range;
int                  color;
ULONG               *ptr;
ULONG               *src;
ULONG               *dest;
ULONG               lastblue;
ULONG               lastgreen;
ULONG               lastred;

#undef  GfxBase
struct  Library     *GfxBase = TaggedOpenLibrary(OLTAG_GRAPHICS);

    Wait(SIGBREAKF_CTRL_C);

    V = FindTask(NULL)->tc_UserData;

    if (cc = AllocVec(sizeof(struct ColorChanger) + V->ScreenBMI->bmi_NumRanges * sizeof(struct CycleRange), MEMF_CLEAR)) {

        cc->ViewPort  = &V->Screen->ViewPort;
        cc->NumRanges = V->ScreenBMI->bmi_NumRanges;

        cr  = cc->Ranges = (struct CycleRange *)(cc + 1);
        rgi = V->ScreenBMI->bmi_RangeInfo;

        for (range=0; range<cc->NumRanges; range++) {

            cr->Length    = rgi->rgi_High - rgi->rgi_Low + 1;
            cr->Countdown = cr->NumFrames = rgi->rgi_MicroSeconds / 16666;

            if (ptr = AllocVec(cr->Length * 3 * sizeof(ULONG) + 2 * sizeof(ULONG), MEMF_CLEAR)) {

                cr->LoadTable = ptr;

                *ptr++ = (cr->Length << 16) + rgi->rgi_Low;

                for (color=rgi->rgi_Low; color<=rgi->rgi_High; color++) {

                    *ptr++ = V->ColorPack.cp_Colors[color].Red;
                    *ptr++ = V->ColorPack.cp_Colors[color].Green;
                    *ptr++ = V->ColorPack.cp_Colors[color].Blue;
                    }

                *ptr = 0;
                }

            cr++;
            rgi++;
            }

        while (!(SetSignal(0, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)) {

            WaitTOF();

            cr = cc->Ranges;

            for (range=0; range<cc->NumRanges; range++) {

                if (!--cr->Countdown) {

                    src       = &cr->LoadTable[3 * cr->Length];
                    dest      = src;
                    lastblue  = *src--;
                    lastgreen = *src--;
                    lastred   = *src--;

                    for (color=0; color<3*(cr->Length-1); color++) *dest-- = *src--;

                    *dest-- = lastblue;
                    *dest-- = lastgreen;
                    *dest   = lastred;

                    cr->Countdown = cr->NumFrames;
                    LoadRGB32(cc->ViewPort, cr->LoadTable);
                    }

                cr++;
                }
            }

        for (cr=cc->Ranges,range=0; range<cc->NumRanges; range++) {

            FreeVec(cr->LoadTable);
            cr++;
            }

        FreeVec(cc);
        }
    CloseLibrary(GfxBase);
    }



#define GfxBase      V->GfxBase

struct Task *CreateColorChanger(struct V *V) {

struct Task *cycletask;

    if (cycletask = CreateTask("PlayerCycler", 51, taskLoop, 4000)) {

        cycletask->tc_UserData = V;

        Signal(cycletask, SIGBREAKF_CTRL_C);

        return(cycletask);
        }

    else return(NULL);
    }


void DeleteColorChanger(struct V *V) {

struct Task *cycletask;

    if (cycletask = FindTask("PlayerCycler")) Signal(cycletask, SIGBREAKF_CTRL_C);
    }


