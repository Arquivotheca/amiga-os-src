
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/lowlevel.h>
#include <graphics/gfx.h>
#include <cdtv/debox.h>
#include <devices/audio.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "cduibase.h"
#include "utils.h"
#include "animate.h"


/*****************************************************************************/


struct VoiceHeader
{
    ULONG vh_OneShotHiSamples;
    ULONG vh_RepeatHiSamples;
    ULONG vh_SamplesPerHiCycle;
    UWORD vh_SamplesPerSec;
    UBYTE vh_Octaves;
    UBYTE vh_Compression;
    ULONG vh_Volume;
};


struct ColorPackShort
{
    UWORD           cp_NumColors;
    UWORD           cp_FirstColor;
    struct RGBEntry cp_Colors[NUM_COLORS-1];
    UWORD           cp_EndMarker;
};


/*****************************************************************************/


#define NUM_FADE_STEPS 16


/*****************************************************************************/


struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};


/*****************************************************************************/


/* Spence's magic function */
ULONG ASM LibAnimationControlA(REG(a0) struct View     *view,
		               REG(a1) struct ViewPort *viewPort,
			       REG(a2) struct TagItem  *tags,
			       REG(a6) struct Library  *gfxBase);


/*****************************************************************************/


/* We want a copper list which makes color 0 have the following vertical
 * spread:
 */
#define START_RED       60     /* Original CDTV background: 112 */
#define END_RED         18     /*                           208 */
#define START_GREEN     110    /*                           160 */
#define END_GREEN       8      /*                           160 */
#define START_BLUE      200    /*                           240 */
#define END_BLUE        60     /*                           144 */
#define LINES_PER_COLOR 3

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


VOID FadeIn(struct CDUILib *CDUIBase)
{
ULONG            i,j,r,g, b;
ULONG            rInc, gInc, bInc;
struct ColorPack colors;
struct TagItem   tags[2];

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    for (i = 0; i < CDUIBase->cb_Screen->Height / LINES_PER_COLOR; i++)
    {
        CDUIBase->cb_Range[i].cor_Pen    = 0;
        CDUIBase->cb_Range[i].cor_WAIT_X = 0;
        CDUIBase->cb_Range[i].cor_WAIT_Y = i * LINES_PER_COLOR;
        CDUIBase->cb_Range[i].cor_Flags  = CORF_MODIFY;
    }
    CDUIBase->cb_Range[i].cor_Pen = -1;

    rInc = Replicate(RED_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);
    gInc = Replicate(GREEN_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);;
    bInc = Replicate(BLUE_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)&CDUIBase->cb_Range;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (CDUIBase->cb_Colors.cp_Colors[j].Red / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Green = (CDUIBase->cb_Colors.cp_Colors[j].Green / NUM_FADE_STEPS) * i;
            colors.cp_Colors[j].Blue  = (CDUIBase->cb_Colors.cp_Colors[j].Blue / NUM_FADE_STEPS) * i;
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < CDUIBase->cb_Screen->Height / LINES_PER_COLOR; j++)
        {
            CDUIBase->cb_Range[j].cor_Red   = (r / NUM_FADE_STEPS) * i;
            CDUIBase->cb_Range[j].cor_Green = (g / NUM_FADE_STEPS) * i;
            CDUIBase->cb_Range[j].cor_Blue  = (b / NUM_FADE_STEPS) * i;

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
        LibAnimationControlA(ViewAddress(),&CDUIBase->cb_Screen->ViewPort,tags,GfxBase);
        LoadRGB32(&CDUIBase->cb_Screen->ViewPort,(ULONG *)&colors);

        if (i == 0)
        {
            CDUIBase->cb_Range[0].cor_Flags = CORF_MODIFY|CORF_ANIMATE;
            RemakeDisplay();
        }
    }

    LoadRGB32(&CDUIBase->cb_Screen->ViewPort,(ULONG *)&CDUIBase->cb_Colors);
}


/*****************************************************************************/


VOID FadeOut(struct CDUILib *CDUIBase)
{
ULONG            i,j;
struct ColorPack colors;
ULONG            r,g,b;
ULONG            rInc, gInc, bInc;
struct TagItem   tags[2];

    colors.cp_NumColors  = NUM_COLORS;
    colors.cp_FirstColor = 0;
    colors.cp_EndMarker  = 0;

    rInc = Replicate(RED_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);
    gInc = Replicate(GREEN_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);;
    bInc = Replicate(BLUE_DIFF) / (CDUIBase->cb_Screen->Height / LINES_PER_COLOR - 1);

    tags[0].ti_Tag  = ACTAG_ColorRange;
    tags[0].ti_Data = (ULONG)&CDUIBase->cb_Range;
    tags[1].ti_Tag  = TAG_DONE;

    for (i = 0; i <= NUM_FADE_STEPS; i++)
    {
        for (j = 0; j < NUM_COLORS; j++)
        {
            colors.cp_Colors[j].Red   = (CDUIBase->cb_Colors.cp_Colors[j].Red / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Green = (CDUIBase->cb_Colors.cp_Colors[j].Green / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            colors.cp_Colors[j].Blue  = (CDUIBase->cb_Colors.cp_Colors[j].Blue / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
        }

        r = Replicate(START_RED);
        g = Replicate(START_GREEN);
        b = Replicate(START_BLUE);
        for (j = 0; j < CDUIBase->cb_Screen->Height / LINES_PER_COLOR; j++)
        {
            CDUIBase->cb_Range[j].cor_Red   = (r / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            CDUIBase->cb_Range[j].cor_Green = (g / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
            CDUIBase->cb_Range[j].cor_Blue  = (b / NUM_FADE_STEPS) * (NUM_FADE_STEPS - i);
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
        LibAnimationControlA(ViewAddress(),&CDUIBase->cb_Screen->ViewPort,tags,GfxBase);
        LoadRGB32(&CDUIBase->cb_Screen->ViewPort,(ULONG *)&colors);
    }

    for (i = 0; i < NUM_COLORS; i++)
    {
        colors.cp_Colors[i].Red   = 0;
        colors.cp_Colors[i].Green = 0;
        colors.cp_Colors[i].Blue  = 0;
    }
    LoadRGB32(&CDUIBase->cb_Screen->ViewPort,(ULONG *)&colors);
}


/*****************************************************************************/


VOID GetBM(struct CDUILib *CDUIBase, APTR deboxData,
           struct BitMap **resultBM, struct BMInfo **resultBMInfo)
{
struct BitMap *tmpBM;

    *resultBMInfo = DecompBMInfo(NULL, NULL, deboxData);
    *resultBM     = AllocBitMap((*resultBMInfo)->bmi_Width,(*resultBMInfo)->bmi_Height,(*resultBMInfo)->bmi_Depth, BMF_DISPLAYABLE | BMF_INTERLEAVED, NULL);
    tmpBM         = AllocBitMap((*resultBMInfo)->bmi_Width,(*resultBMInfo)->bmi_Height,(*resultBMInfo)->bmi_Depth, 0, NULL);
    DecompBitMap(NULL, deboxData, tmpBM, NULL);

    BltBitMap(tmpBM,0,0,*resultBM,0,0,
              (*resultBMInfo)->bmi_Width,(*resultBMInfo)->bmi_Height,0xc0,0xff,NULL);

    WaitBlit();
    FreeBitMap(tmpBM);
}


/*****************************************************************************/


VOID FreeBM(struct CDUILib *CDUIBase, struct BitMap *bitMap, struct BMInfo *bmInfo)
{
    if (bmInfo)
        FreeBMInfo(bmInfo);

    WaitBlit();
    FreeBitMap(bitMap);
}


/*****************************************************************************/


VOID GetColors(struct CDUILib *CDUIBase, APTR backData, APTR spriteData)
{
struct BMInfo    *bmInfo;
UWORD             i, j;
struct ColorPack *table;

    bmInfo = DecompBMInfo(NULL, NULL, backData);

    table = (struct ColorPack *)GetBMInfoRGB32(bmInfo,NUM_PLAYFIELD_COLORS,0);
    CDUIBase->cb_Colors              = *table;
    CDUIBase->cb_Colors.cp_NumColors = NUM_COLORS;
    CDUIBase->cb_Colors.cp_EndMarker = 0;
    FreeBMInfoRGB32((ULONG *)table);
    FreeBMInfo(bmInfo);

    bmInfo = DecompBMInfo(NULL, NULL, spriteData);
    table = (struct ColorPack *)GetBMInfoRGB32(bmInfo,NUM_PLAYFIELD_COLORS,0);

    if (bmInfo->bmi_Depth == 2)
    {
        for (i = NUM_PLAYFIELD_COLORS; i < NUM_PLAYFIELD_COLORS + 4; i++)
        {
            j = i - NUM_PLAYFIELD_COLORS;
            CDUIBase->cb_Colors.cp_Colors[i+8] = CDUIBase->cb_Colors.cp_Colors[i+4] = CDUIBase->cb_Colors.cp_Colors[i] = table->cp_Colors[j];
        }
    }
    else
    {
        for (i = NUM_PLAYFIELD_COLORS; i < NUM_COLORS; i++)
        {
            j = i - NUM_PLAYFIELD_COLORS;
            CDUIBase->cb_Colors.cp_Colors[i] = table->cp_Colors[j];
        }
    }
    FreeBMInfoRGB32((ULONG *)table);
    FreeBMInfo(bmInfo);
}

/* SAS BUG:
        for (i = NUM_PLAYFIELD_COLORS; i < NUM_COLORS; i++)
        {
            j = i - NUM_PLAYFIELD_COLORS;
            CDUIBase->cb_Colors.cp_Colors[i].Red   = Make32(bmInfo->bmi_ColorMap[j] / 256);
            CDUIBase->cb_Colors.cp_Colors[i].Green = Make32(bmInfo->bmi_ColorMap[j] / 16);
            CDUIBase->cb_Colors.cp_Colors[i].Blue  = Make32(bmInfo->bmi_ColorMap[j]);
        }
*/


/*****************************************************************************/


static UBYTE audioChannels[] = {3, 6, 1, 2, 4, 8};

VOID PlaySample(struct CDUILib *CDUIBase, UBYTE *data)
{
struct IOAudio      ioa;
struct VoiceHeader *header;
ULONG              *sample;
UBYTE              *buffer;

    header = (APTR)&data[20];   /* skip FORM and CHUNK headers  */
    sample = (APTR)&data[44];   /* first longword after BODY id */

    buffer = AllocVec(sample[0],MEMF_CHIP);
    CopyMem(&sample[1],buffer,sample[0]);

    ioa.ioa_Request.io_Message.mn_Node.ln_Pri  = 85;
    ioa.ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ioa.ioa_Request.io_Message.mn_Length       = sizeof(struct IOAudio);
    ioa.ioa_Request.io_Message.mn_ReplyPort    = CreateMsgPort();
    ioa.ioa_Data                               = audioChannels;
    ioa.ioa_Length                             = sizeof(audioChannels);

    if (OpenDevice("audio.device",0,&ioa,0) != 0)
    {
        InitResident(FindResident("audio.device"),NULL);
        OpenDevice("audio.device",0,&ioa,0);
    }

    ioa.ioa_Request.io_Flags   = ADIOF_PERVOL;
    ioa.ioa_Request.io_Command = CMD_WRITE;
    ioa.ioa_Volume             = 64;
    ioa.ioa_Period             = (SysBase->ex_EClockFrequency * 5) / header->vh_SamplesPerSec;
    ioa.ioa_Cycles             = 1;
    ioa.ioa_Length             = sample[0];         /* length of chunk          */
    ioa.ioa_Data               = buffer;

    BeginIO(&ioa);
    WaitIO(&ioa);

    FreeVec(buffer);
    CloseDevice(&ioa);
    DeleteMsgPort(ioa.ioa_Request.io_Message.mn_ReplyPort);
}


/*****************************************************************************/


VOID WaitBeam(struct CDUILib *CDUIBase, ULONG pos)
{
    while (VBeamPos() < GfxBase->ActiView->DyOffset + CDUIBase->cb_Screen->ViewPort.DyOffset + pos)
    {
    }
}


/*****************************************************************************/


ULONG __asm OpenWorkBenchPatch(void)
{
    return(0);
}


/*****************************************************************************/


VOID CloseScreenQuiet(struct CDUILib *CDUIBase, struct Screen *screen)
{
APTR OWB;

    if (screen)
    {
        OWB = SetFunction(IntuitionBase, -0xd2, (APTR)OpenWorkBenchPatch);
        CloseScreen(screen);
        SetFunction(IntuitionBase, -0xd2, OWB);
    }
}


/*****************************************************************************/


VOID SyncSignal(struct CDUILib *CDUIBase, struct Task *task, ULONG sigMask)
{
    SetSignal(0,SIGF_SINGLE);
    Signal(task,sigMask);
    Wait(SIGF_SINGLE);
}
