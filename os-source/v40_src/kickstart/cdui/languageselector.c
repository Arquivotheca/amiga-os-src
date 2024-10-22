
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/videocontrol.h>
#include <libraries/lowlevel.h>
#include <cdtv/debox.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "languageselector.h"
#include "cduibase.h"
#include "utils.h"
#include "credits.h"
#include "waveflag.h"


/*****************************************************************************/


/* things we can do */
#define CMD_NOP    0
#define CMD_UP     1
#define CMD_DOWN   2
#define CMD_SELECT 3
#define CMD_CANCEL 4


/*****************************************************************************/


/* map from a language number to a slot in the picture which contains
 * the imagery for that language
 */
static const UBYTE LangMap[] =
{
    3,0,3,2,5,4,6,9,1,7,8,10,11,14,13,0xff,0xff,0xff,12
};

/* map from a slot number to a flag number
 */
static const UBYTE FlagMap[] =
{
    24,26,25,30,10,32,35,29,3,5,23,11,9,31,4
};

/* map from a slot number to a language number */
static const UBYTE SlotMap[] =
{
    LANG_AMERICAN,
    LANG_DANISH,
    LANG_GERMAN,
    LANG_ENGLISH,
    LANG_SPANISH,
    LANG_FRENCH,
    LANG_ITALIAN,
    LANG_DUTCH,
    LANG_NORWEGIAN,
    LANG_PORTUGUESE,
    LANG_FINNISH,
    LANG_SWEDISH,
    LANG_KOREAN,
    LANG_CHINESE,
    LANG_JAPANESE
};


/*****************************************************************************/


/* info on language bitmap */
#define NUM_LANGUAGES      15

#define FLAGS_WIDTH        293
#define FLAGS_HEIGHT       197

#define FLAG_WIDTH  48
#define FLAG_HEIGHT 32

/* where things go on the screen */
#define SCR_X_OFFSET       (CDUIBase->cb_XOffset)
#define SCR_Y_OFFSET       (CDUIBase->cb_YOffset)
#define SCR_ITEM_HEIGHT    15
#define SCR_INBOX_X_OFFSET (SCR_X_OFFSET + 21)
#define SCR_INBOX_Y_OFFSET (SCR_Y_OFFSET + 18)
#define SCR_INBOX_WIDTH    85
#define SCR_INBOX_HEIGHT   93


/*****************************************************************************/


/* debox magic data */
extern UBYTE * __far languageimage;
extern UBYTE * __far languagenames;
extern UBYTE * __far globes;


/*****************************************************************************/


#undef SysBase

/* MAX_AMPLITUDE is the maximum amplitude of the wave,in pixels
 * AMPLITUDE_SCALE is the multiplier that the amplitude is scaled
 * down by inside MakeFlag().  This allows us a fractional amplitude
 * effect.
 */
#define MAX_AMPLITUDE   5
#define AMPLITUDE_SCALE 4


/* phase=0 corresponds to 0 degrees.
 * phase=256 corresponds to 360 degrees.
 * This function returns the sine of the "angle" phase, where
 * the amplitude runs from +/- 255.
 */
static LONG Sine(UBYTE phase)
{
LONG answer;

    static const UBYTE sin_table[128] =
    {
           0,    6,   12,   18,   24,   31,   37,   43,
          49,   55,   61,   68,   74,   79,   85,   91,
          97,  103,  109,  114,  120,  125,  131,  136,
         141,  146,  151,  156,  161,  166,  171,  175,
         180,  184,  188,  193,  197,  201,  204,  208,
         212,  215,  218,  221,  224,  227,  230,  233,
         235,  237,  240,  242,  244,  245,  247,  248,
         250,  251,  252,  253,  253,  254,  254,  254,
         255,  254,  254,  254,  253,  253,  252,  251,
         250,  248,  247,  245,  244,  242,  240,  237,
         235,  233,  230,  227,  224,  221,  218,  215,
         212,  208,  204,  201,  197,  193,  188,  184,
         180,  175,  171,  166,  161,  156,  151,  146,
         141,  136,  131,  125,  120,  114,  109,  103,
          97,   91,   85,   79,   74,   68,   61,   55,
          49,   43,   37,   31,   24,   18,   12,    6,
    };

    if ( phase < 128 )
    {
        answer = sin_table[ phase ];
    }
    else
    {
        answer = -sin_table[ phase - 128 ];
    }
    return( answer );
}


/*****************************************************************************/


/* Returns an amplitude factor based on the position in the flag.
 * Full-scale = 255.
 * The function is a logarithmic growth from 0 to 255.
 */
static LONG FlagAmplitude( LONG x )
{
    /* Here is a table of normalized logarithms.  They're
     * log(1)/log(46) through log(46)/log(46), scaled
     * to a max of 255
     */
    static const UBYTE logtable[ FLAG_WIDTH-2 ] =
    {
           0,   46,   73,   92,  107,  119,  129,  138,
         146,  153,  159,  165,  170,  175,  180,  184,
         188,  192,  196,  199,  202,  205,  208,  211,
         214,  216,  219,  221,  224,  226,  228,  230,
         232,  234,  236,  238,  240,  242,  244,  245,
         247,  248,  250,  252,  253,  255,
    };
    LONG answer = 255;
    if ( x < FLAG_WIDTH-2 )
    {
        answer = logtable[x];
    }
    return( answer );
}


/*****************************************************************************/


/* For a flag of width flagwidth pixels, generate an array of Column
 * structures for a waving flag.  The initial phase (0..255) is the
 * phase parameter.  The y-origin is the yorg parameter.  The frequency
 * parameter is freq, specified as the number of phase increments
 * per pixel.  The amplitude is the desired maximum amplitude of
 * the flag wave, plus one.
 */
static VOID MakeWave(struct Column *out,
                     ULONG phase, LONG yorg, ULONG freq, LONG amplitude)
{
LONG x;

    /* We omit the first and last columns of the flag, to shorten
     * the resulting flag's image, since it would appear shorter
     * when the flag fabric is waving in and out of the screen
     */
    for (x = 1; x < FLAG_WIDTH-1; x++)
    {
        out->srcx = x;
        /* Sine() and FlagAmplitude() return a maximum of 255 each, thus
         * we must divide down by 1<<16 to normalize those.  The amplitude
         * variable is exaggerated by a factor of AMPLITUDE_SCALE, so
         * divide that out too.
         */
        out->dsty = yorg + ((Sine(phase) * FlagAmplitude(x-1) * amplitude)/((1L<<16)*AMPLITUDE_SCALE));
        out++;
        phase += freq;
        if (phase >= 256)
            phase -= 256;
    }
    out->srcx = -1;
}


/*****************************************************************************/


static VOID DoOneWave(struct CDUILib *CDUIBase, LONG flag,
                      LONG i, LONG amplitude)
{
struct Column colTable[FLAG_WIDTH+1];

    BltBitMap(CDUIBase->cb_SaveBM,0,0,
              CDUIBase->cb_WorkBM,0,0,
              FLAG_WIDTH,FLAG_HEIGHT+10,0xc0,0xff,NULL);

    /* out (storage for column table) = col
     * t (initial phase of this frame) = i
     * yorg = 5
     * freq = 4
     * amplitude = 0..5
     */
    MakeWave(colTable,i,5,4,amplitude);

    WaitBlit();
    ColumnCopyBM(CDUIBase->cb_CurrentFlagBM,
                 CDUIBase->cb_WorkBM,
                 colTable,0,FLAG_HEIGHT);

    BltBitMap(CDUIBase->cb_WorkBM,0,0,
              CDUIBase->cb_BitMap,SCR_X_OFFSET + (FLAG_WIDTH+1) * (flag % 6),SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6) - 5,
              FLAG_WIDTH,FLAG_HEIGHT+10,0xc0,0xff,NULL);
}


/*****************************************************************************/


static VOID FlagWaver(VOID)
{
LONG             i;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;
LONG             oldFlag;
LONG             amplitude = 0;
LONG             flag = 0;

    SysBase  = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    i       = 255;
    oldFlag = 0;
    while ((SetSignal(0,0) & SIGBREAKF_CTRL_C) == 0)
    {
        WaitTOF();

        if (SetSignal(0,SIGBREAKF_CTRL_D) & SIGBREAKF_CTRL_D)
        {
            while (amplitude > 0)
            {
                WaitTOF();

                DoOneWave(CDUIBase,flag,i,amplitude);

                i -= 14;
                if (i < 0)
                    i += 256;

                amplitude -= 2;
            }
            Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
            Wait(SIGBREAKF_CTRL_D);
        }
        else
        {
            ObtainSemaphore(&CDUIBase->cb_FlagLock);

            flag = CDUIBase->cb_FlagToWave;
            if (flag != oldFlag)
            {
                oldFlag = flag;
                amplitude = AMPLITUDE_SCALE;        /* 1 pixel */
            }

            DoOneWave(CDUIBase,flag,i,amplitude);

            ReleaseSemaphore(&CDUIBase->cb_FlagLock);

            i -= 14;

            if (i < 0)
                i += 256;

            if (amplitude < MAX_AMPLITUDE*AMPLITUDE_SCALE)
                amplitude++;
        }
    }

    CDUIBase->cb_WaverTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


#undef SysBase

static VOID GlobeSpinner(VOID)
{
ULONG            i,j;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;

    SysBase  = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    i = 0;
    while ((SetSignal(0,0) & SIGBREAKF_CTRL_C) == 0)
    {
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[(i) % 72],CDUIBase->cb_Sprites[(i+4) % 72],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[(i+1) % 72],CDUIBase->cb_Sprites[(i+1+4) % 72],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[(i+2) % 72],CDUIBase->cb_Sprites[(i+2+4) % 72],NULL);
        ChangeExtSpriteA(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[(i+3) % 72],CDUIBase->cb_Sprites[(i+3+4) % 72],NULL);

        for (j = 0; j < 5; j++)
            WaitTOF();

        i += 4;
    }

    CDUIBase->cb_SpinnerTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib


/*****************************************************************************/


static VOID MakeGlobes(struct CDUILib *CDUIBase)
{
struct BitMap      *tmpBM;
struct BitMap      *globesBM;
struct BMInfo      *globesBMInfo;
UWORD               i,j;

    GetBM(CDUIBase,&globes,&globesBM,&globesBMInfo);

    for (i = 0; i < 36; i++)
    {
        tmpBM = AllocBitMap(64,86,4,BMF_CLEAR|BMF_INTERLEAVED,NULL);
        BltBitMap(globesBM,(i % 12) * 43, (i / 12) * 86,
                  tmpBM,(1 - (i & 1)) * 21,0,
                  43,86,0xc0,0xff,NULL);

        WaitBlit();

        for (j = 0; j < 2; j++)
        {
            CDUIBase->cb_Sprites[i*2+j] = AllocSpriteData(tmpBM,SPRITEA_Width,    63,
                                                               SPRITEA_Attached, j & 1,
                                                               TAG_DONE);
        }
        FreeBitMap(tmpBM);
    }
    FreeBM(CDUIBase,globesBM,globesBMInfo);

    for (i = 0; i < 4; i++)
        GetExtSprite(CDUIBase->cb_Sprites[i],GSTAG_SPRITE_NUM,i+2,TAG_DONE);

    RemakeDisplay();

    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[0],SCR_X_OFFSET + 126,SCR_Y_OFFSET + 70);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[1],SCR_X_OFFSET + 126,SCR_Y_OFFSET + 70);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[2],SCR_X_OFFSET + 126 + 64,SCR_Y_OFFSET + 70);
    MoveSprite(CDUIBase->cb_ViewPort,CDUIBase->cb_Sprites[3],SCR_X_OFFSET + 126 + 64,SCR_Y_OFFSET + 70);

    CDUIBase->cb_SpinnerTask = CreateTask("Spinner",21,GlobeSpinner,4096);
    CDUIBase->cb_WaverTask = CreateTask("Waver",-1,FlagWaver,4096);
}


/*****************************************************************************/


static VOID NukeGlobes(struct CDUILib *CDUIBase)
{
UWORD i;

    Signal(CDUIBase->cb_SpinnerTask,SIGBREAKF_CTRL_C);
    Signal(CDUIBase->cb_WaverTask,SIGBREAKF_CTRL_C);

    while (CDUIBase->cb_SpinnerTask || CDUIBase->cb_WaverTask)
        WaitTOF();

    FreeSprite(2);
    FreeSprite(3);
    FreeSprite(4);
    FreeSprite(5);

    for (i = 0; i < 72; i++)
        FreeSpriteData(CDUIBase->cb_Sprites[i]);
}


/*****************************************************************************/


static LONG GetFlagNum(struct CDUILib *CDUIBase, LONG langNum)
{
LONG flag;

    flag = FlagMap[langNum];
    if (((flag == 30) || (flag == 32)) && (CDUIBase->cb_DoBetterFlag))
        flag = 17;

    return(flag);
}


/*****************************************************************************/


static VOID ShowLanguage(struct CDUILib *CDUIBase, struct BitMap *languageBM,
                         LONG newLang, LONG oldLang)
{
WORD            i;
UWORD           flag;
struct RastPort rp;

    if (oldLang == -1)
    {
        BltBitMap(languageBM,0,newLang*SCR_ITEM_HEIGHT + 6,
                  CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET + 10,SCR_INBOX_Y_OFFSET,
                  SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0x01,NULL);

        ObtainSemaphore(&CDUIBase->cb_FlagLock);
    }
    else
    {
        if (newLang < oldLang)
        {
            for (i = 1; i <= SCR_ITEM_HEIGHT; i++)
            {
                WaitBeam(CDUIBase,SCR_INBOX_Y_OFFSET + SCR_INBOX_HEIGHT);
                BltBitMap(languageBM,0,oldLang*SCR_ITEM_HEIGHT - i + 6,
                          CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET + 10,SCR_INBOX_Y_OFFSET,
                          SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0x01,NULL);
                WaitTOF();
            }
        }
        else if (newLang > oldLang)
        {
            for (i = 1; i <= SCR_ITEM_HEIGHT; i++)
            {
                WaitBeam(CDUIBase,SCR_INBOX_Y_OFFSET + SCR_INBOX_HEIGHT);
                BltBitMap(languageBM,0,oldLang*SCR_ITEM_HEIGHT + i + 6,
                          CDUIBase->cb_BitMap,SCR_INBOX_X_OFFSET + 10,SCR_INBOX_Y_OFFSET,
                          SCR_INBOX_WIDTH,SCR_INBOX_HEIGHT,0xc0,0x01,NULL);
                WaitTOF();
            }
        }

        /* tell the subtask to wind down */
        SyncSignal(CDUIBase,CDUIBase->cb_WaverTask,SIGBREAKF_CTRL_D);

        ObtainSemaphore(&CDUIBase->cb_FlagLock);

        /* tell the subtask to get going again */
        Signal(CDUIBase->cb_WaverTask,SIGBREAKF_CTRL_D);

        /* restore original flag imagery on the screen */
        flag = GetFlagNum(CDUIBase,oldLang);

        WaitBeam(CDUIBase,SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6) - FLAG_HEIGHT + 10);
        BltBitMap(CDUIBase->cb_SaveBM,0,0,
                  CDUIBase->cb_BitMap,SCR_X_OFFSET + (FLAG_WIDTH+1) * (flag % 6),SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6) - 5,
                  FLAG_WIDTH,FLAG_HEIGHT+10,0xc0,0xff,NULL);

        SetWriteMask(CDUIBase->cb_RastPort,0x20);
        SetAPen(CDUIBase->cb_RastPort,0xff);
        RectFill(CDUIBase->cb_RastPort,SCR_X_OFFSET + (FLAG_WIDTH+1) * (flag % 6),
                                       SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6),
                                       SCR_X_OFFSET + (FLAG_WIDTH+1) * (flag % 6) + FLAG_WIDTH - 1,
                                       SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6) + FLAG_HEIGHT - 1);
        SetWriteMask(CDUIBase->cb_RastPort,0xff);

        BltBitMap(CDUIBase->cb_CurrentFlagBM,0,0,
                  CDUIBase->cb_BitMap,SCR_X_OFFSET + (FLAG_WIDTH+1) * (flag % 6),SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6),
                  FLAG_WIDTH,FLAG_HEIGHT,0xc0,0x1f,NULL);
    }

    flag = GetFlagNum(CDUIBase,newLang);

    CDUIBase->cb_FlagToWave = flag;

    /* copy new current flag to off-screen bit map */
    InitRastPort(&rp);
    rp.BitMap = CDUIBase->cb_CurrentFlagBM;
    SetAPen(&rp,0);
    RectFill(&rp,0,0,FLAG_WIDTH-1,FLAG_HEIGHT-1);

    BltBitMap(CDUIBase->cb_BitMap,SCR_X_OFFSET + (FLAG_WIDTH + 1) * (flag % 6), SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6),
              CDUIBase->cb_CurrentFlagBM,0,0,
              FLAG_WIDTH,FLAG_HEIGHT,0xc0,0x1f,NULL);  /* leave high-plane alone */

    BltBitMap(CDUIBase->cb_BitMap,SCR_X_OFFSET + (FLAG_WIDTH + 1) * (flag % 6), SCR_Y_OFFSET + (FLAG_HEIGHT + 1) * (flag / 6) - 5,
              CDUIBase->cb_SaveBM,0,0,
              FLAG_WIDTH,FLAG_HEIGHT+10,0xc0,0xff,NULL);

    InitRastPort(&rp);
    rp.BitMap = CDUIBase->cb_SaveBM;
    SetAPen(&rp,0);
    RectFill(&rp,0,5,FLAG_WIDTH-1,FLAG_HEIGHT+5-1);

    ReleaseSemaphore(&CDUIBase->cb_FlagLock);

    /* slow things down... */
    for (i = 0; i < 10; i++)
        WaitTOF();
}


/*****************************************************************************/


static VOID StampFlags(struct CDUILib *CDUIBase)
{
struct BitMap    *flagsBM;
struct BMInfo    *flagsBMInfo;
struct Rectangle  rect;
ULONG             id;

    GetBM(CDUIBase,&languageimage,&flagsBM,&flagsBMInfo);

    id = GetVPModeID(CDUIBase->cb_ViewPort);
    QueryOverscan(id,&rect,OSCAN_TEXT);

    CDUIBase->cb_XOffset = (CDUIBase->cb_Screen->Width - flagsBMInfo->bmi_Width) / 2;
    CDUIBase->cb_YOffset = (rect.MaxY - flagsBMInfo->bmi_Height + 1) / 2 - CDUIBase->cb_Screen->TopEdge;

    BltBitMap(flagsBM,0,0,
              CDUIBase->cb_BitMap,SCR_X_OFFSET,SCR_Y_OFFSET,
              FLAGS_WIDTH,FLAGS_HEIGHT,0xc0,0xff,NULL);

    WaitBlit();
    FreeBM(CDUIBase,flagsBM,flagsBMInfo);
}


/*****************************************************************************/


VOID LanguageSelector(struct CDUILib *CDUIBase)
{
struct BitMap       *bitMap;
struct BMInfo       *bmInfo;
BOOLEAN              quit;
LONG                 currentLanguage;
UBYTE                cmd;
UWORD                creditState;
UWORD                icode;
struct IntuiMessage *intuiMsg;
BOOL                 first;

    VideoControlTags(CDUIBase->cb_Screen->ViewPort.ColorMap,
                     VTAG_BORDERSPRITE_SET,     TRUE,
                     VC_IntermediateCLUpdate,   TRUE,
                     VTAG_SPODD_BASE_SET,       NUM_PLAYFIELD_COLORS,
                     VTAG_SPEVEN_BASE_SET,      NUM_PLAYFIELD_COLORS,
                     VTAG_FULLPALETTE_SET,      TRUE,
                     TAG_DONE);

    GetColors(CDUIBase,&languageimage,&globes);

    StampFlags(CDUIBase);

    currentLanguage = LangMap[GetLanguageSelection()];

    CDUIBase->cb_CurrentFlagBM = AllocBitMap(FLAG_WIDTH,FLAG_HEIGHT,6,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    CDUIBase->cb_SaveBM        = AllocBitMap(FLAG_WIDTH,FLAG_HEIGHT+10,6,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    CDUIBase->cb_WorkBM        = AllocBitMap(FLAG_WIDTH,FLAG_HEIGHT+10,6,BMF_CLEAR|BMF_INTERLEAVED,NULL);
    CDUIBase->cb_FlagToWave    = currentLanguage;
    InitSemaphore(&CDUIBase->cb_FlagLock);

    GetBM(CDUIBase,&languagenames,&bitMap,&bmInfo);

    ShowLanguage(CDUIBase,bitMap,currentLanguage,-1);

    MakeGlobes(CDUIBase);

    FadeIn(CDUIBase);

    quit = FALSE;
    creditState = 0;
    first = TRUE;

    while (!quit)
    {
        WaitPort(CDUIBase->cb_Window->UserPort);
        intuiMsg = (struct IntuiMessage *)GetMsg(CDUIBase->cb_Window->UserPort);
        icode = intuiMsg->Code;

        if (first)
        {
            if (intuiMsg->Qualifier & IEQUALIFIER_REPEAT)
                icode = 0;
            else
                first = FALSE;
        }

        cmd = CMD_NOP;

#ifdef TEST_VERSION
        if (icode == 0x40)
        {
            NukeGlobes(CDUIBase);
            DoCredits(CDUIBase,currentLanguage);
            CDUIBase->cb_DoBetterFlag = TRUE;
            MakeGlobes(CDUIBase);
        }
#endif

        if (creditState == 10)
        {
            if (icode == RAWKEY_PORT0_BUTTON_RED)
            {
                NukeGlobes(CDUIBase);
                DoCredits(CDUIBase,-1);
                CDUIBase->cb_DoBetterFlag = TRUE;
                MakeGlobes(CDUIBase);
            }

            if (!(0x80 & icode))   /* don't clear the state on upstrokes */
                creditState = 0;
        }
        else
        {
            switch (icode)
            {
                case RAWKEY_PORT0_BUTTON_RED:
                case RAWKEY_PORT1_BUTTON_RED:
                case RAWKEY_PORT2_BUTTON_RED:
                case RAWKEY_PORT3_BUTTON_RED:
                case 0x44  /* RETURN key */ : cmd = CMD_SELECT;
                                              creditState = 0;
                                              break;

                case RAWKEY_PORT0_BUTTON_BLUE:
                case RAWKEY_PORT1_BUTTON_BLUE:
                case RAWKEY_PORT2_BUTTON_BLUE:
                case RAWKEY_PORT3_BUTTON_BLUE: cmd = CMD_SELECT;
                                               creditState = 0;
                                               break;

                case 0x45  /* ESC key */     : cmd = CMD_CANCEL;
                                               creditState = 0;
                                               break;

                case RAWKEY_PORT0_JOY_UP:
                case RAWKEY_PORT1_JOY_UP:
                case RAWKEY_PORT2_JOY_UP:
                case RAWKEY_PORT3_JOY_UP:
                case 0x4c  /* cursor */ : cmd = CMD_UP;
                                          creditState = 0;
                                          break;

                case RAWKEY_PORT0_JOY_DOWN:
                case RAWKEY_PORT1_JOY_DOWN:
                case RAWKEY_PORT2_JOY_DOWN:
                case RAWKEY_PORT3_JOY_DOWN:
                case 0x4d  /* cursor */   : cmd = CMD_DOWN;
                                            creditState = 0;
                                            break;

                case RAWKEY_PORT0_BUTTON_FORWARD: switch (creditState)
                                                  {
                                                      case 0 : /* s */
                                                      case 1 : /* p */
                                                      case 3 : /* l */
                                                      case 4 : /* l */
                                                      case 5 : /* b */
                                                      case 8 : /* n */
                                                      case 9 : /* d */
                                                               creditState++;
                                                               break;

                                                      default: creditState = 0;
                                                  }
                                                  break;

                case RAWKEY_PORT0_BUTTON_REVERSE: switch (creditState)
                                                  {
                                                      case 2 : /* e */
                                                      case 6 : /* o */
                                                      case 7 : /* u */
                                                               creditState++;
                                                               break;

                                                      default: creditState = 0;
                                                  }
                                                  break;

                default : if (!(0x80 & icode))   /* don't clear the state on upstrokes */
                              creditState = 0;
                          break;

            }
        }

        ReplyMsg(intuiMsg);

        switch (cmd)
        {
            case CMD_UP    : if (currentLanguage > 0)
                             {
                                 currentLanguage--;
                                 ShowLanguage(CDUIBase,bitMap,currentLanguage,currentLanguage + 1);
                             }
                             break;

            case CMD_DOWN  : if (currentLanguage < NUM_LANGUAGES - 1)
                             {
                                 currentLanguage++;
                                 ShowLanguage(CDUIBase,bitMap,currentLanguage,currentLanguage - 1);
                             }
                             break;

            case CMD_CANCEL:
            case CMD_SELECT: FadeOut(CDUIBase);
                             NukeGlobes(CDUIBase);
                             WaitBlit();
                             FreeBitMap(CDUIBase->cb_SaveBM);
                             FreeBitMap(CDUIBase->cb_WorkBM);
                             FreeBitMap(CDUIBase->cb_CurrentFlagBM);
                             FreeBM(CDUIBase,bitMap,bmInfo);
#ifndef TEST_VERSION
                             if (cmd != CMD_CANCEL)
                                 SetLanguageSelection(SlotMap[currentLanguage]);
#endif
                             return;
        }
    }
}
