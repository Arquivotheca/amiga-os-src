
/***********************************************************************************************
 *                                                                                             *
 *  The following is an example of how the startup animation should operate.  This example is  *
 *  code that is currently used to fade a picture in and out in absence of an actual startup   *
 *  animation.  The animation should be completely self-contained just like this example.      *
 *  ALL animation data should be contained in the animation's object file.                     *
 *                                                                                             *
 *                                                              Jerry Horanoff  11/16/92       *
 *                                                                                             *
 ***********************************************************************************************/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/tagitem.h>
#include <cdtv/debox.h>
#include <libraries/lowlevel.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/debox_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/debox_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include "animation.h"
#include "cdui/cdui.h"
#include "cdui/cdui_protos.h"
#include "cdui/cdui_pragmas.h"

#define STACK_SIZE 8192

#define SysBase (*((LONG *)4))

extern UBYTE * __far strapimage;

extern LONG    __asm FakeOWB(void);

void          SendAnimMessage(int Msg);
void __saveds AnimationTask(void);

void PresentCDUI(ULONG);



/***********************************************************************************************
 *                                                                                             *
 * Set up task data in a form that will allow it to be freed automatically by RemTask()        *
 *                                                                                             *
 ***********************************************************************************************/

struct TaskTask {                                                                               /* Task  */

    struct MemList MemList;
    struct Task    Task;
    };

struct TaskStack {                                                                              /* Stack */

    struct MemList MemList;
    UBYTE          Stack[STACK_SIZE];
    };

struct TaskData {                                                                               /* Name  */

    struct MemList MemList;
    char           Name[18];
    };



/***********************************************************************************************
 *                                                                                             *
 * Fade - Scale luminance of the screen to between 0 and 16                                    *
 *                                                                                             *
 ***********************************************************************************************/

void Fade(UWORD Level, ULONG *ColorTable, struct ViewPort *ViewPort, struct GfxBase *GfxBase) {

ULONG RGBTable[258], i = 1;
ULONG NumColors = ColorTable[0] >> 16;

    do {                                                                                        /* Traverse color table */

        RGBTable[i] = (ColorTable[i++] * Level / 16) * 0x11111111;                              /* Set R, G, & B        */
        RGBTable[i] = (ColorTable[i++] * Level / 16) * 0x11111111; 
        RGBTable[i] = (ColorTable[i++] * Level / 16) * 0x11111111; 

        } while (((i-1)/3) < NumColors);                                                        /* Last color?          */

    RGBTable[i] = 0;                                                                            /* Set table length     */
    RGBTable[0] = ColorTable[0];

    LoadRGB32(ViewPort, RGBTable);                                                              /* Make colors change   */
    }




void FadeTo(LONG Target, LONG *FadeValue, ULONG *ColorTable,
            struct ViewPort *ViewPort, struct GfxBase *GfxBase) {

    if (Target > *FadeValue) {

        for (*FadeValue; *FadeValue<=Target; *FadeValue+=2) {

            Fade(*FadeValue, ColorTable, ViewPort, GfxBase);
            WaitTOF();
            }
        }

    else if (Target < *FadeValue) {

        for (*FadeValue; *FadeValue>=Target; *FadeValue-=2) {

            Fade(*FadeValue, ColorTable, ViewPort, GfxBase);
            WaitTOF();
            }
        }

    *FadeValue = Target;
    }



/*******************************************************************************************
 *                                                                                         *
 * StartAnimation - Start Animation Task                                                   *
 *                                                                                         *
 *******************************************************************************************/

int StartAnimation(void) {

struct TaskTask  *TaskTask;
struct TaskStack *TaskStack;
struct TaskData  *TaskData;

    if (TaskTask = AllocMem(sizeof(struct TaskTask), MEMF_PUBLIC | MEMF_CLEAR)) {           /* Allocate Task structure          */

        if (TaskStack = AllocMem(sizeof(struct TaskStack), MEMF_PUBLIC)) {                  /* Allocate Task stack              */

            if (TaskTask->Task.tc_UserData = (TaskData = AllocMem(sizeof(struct TaskData),  /* Allocate Task name               */
                MEMF_PUBLIC | MEMF_CLEAR)) + sizeof(struct TaskData)) {

                CopyMem("Startup Animation", &TaskData->Name[0], 18);                       /* Create the name                  */

                TaskTask->Task.tc_MemEntry.lh_Head     = (struct Node *)                    /* Initialize MemEntry list         */
                                                         &TaskTask->Task.tc_MemEntry;
                TaskTask->Task.tc_MemEntry.lh_TailPred = (struct Node *)
                                                         &TaskTask->Task.tc_MemEntry;

                TaskTask->MemList.ml_NumEntries           = 1;                              /* Assign Task structure to MemList */
                TaskTask->MemList.ml_ME[0].me_Un.meu_Addr = TaskTask;
                TaskTask->MemList.ml_ME[0].me_Length      = sizeof(struct TaskTask);
                AddHead(&TaskTask->Task.tc_MemEntry, &TaskTask->MemList);

                TaskStack->MemList.ml_NumEntries           = 1;                             /* Add Stack to MemList             */
                TaskStack->MemList.ml_ME[0].me_Un.meu_Addr = TaskStack;
                TaskStack->MemList.ml_ME[0].me_Length      = sizeof(struct TaskStack);
                AddHead(&TaskTask->Task.tc_MemEntry, &TaskStack->MemList);

                TaskData->MemList.ml_NumEntries           = 1;                              /* Add Task name to MemList         */
                TaskData->MemList.ml_ME[0].me_Un.meu_Addr = TaskData;
                TaskData->MemList.ml_ME[0].me_Length      = sizeof(struct TaskData);
                AddHead(&TaskTask->Task.tc_MemEntry, &TaskData->MemList);

                TaskTask->Task.tc_SPLower = &TaskStack->Stack[0];                           /* Initialize Task Stack pointers   */
                TaskTask->Task.tc_SPUpper = 
                TaskTask->Task.tc_SPReg   = &TaskStack->Stack[0] + STACK_SIZE;

                TaskTask->Task.tc_Node.ln_Name = &TaskData->Name[0];                        /* Assign name, type and priority   */
                TaskTask->Task.tc_Node.ln_Type = NT_TASK;
                TaskTask->Task.tc_Node.ln_Pri  = 5;

                AddTask(&TaskTask->Task, AnimationTask, 0);                                 /* Add Task to system list          */

                return(1);                                                                  /* Start of animation successful    */
                }

            FreeMem(TaskStack, sizeof(struct TaskStack));                                   /* Out of memory, free Stack        */
            }

        FreeMem(TaskTask, sizeof(struct TaskTask));                                         /* Out of memory, free Task         */
        }

    return(0);                                                                              /* Start of animation failed        */
    }




/***********************************************************************************************
 *                                                                                             *
 * AnimationTask - Basic animaiton task (for now, this simply displays a picture)              *
 *                                                                                             *
 ***********************************************************************************************/

void __saveds AnimationTask(void) {

struct DeBoxBase     *DeBoxBase;
struct IntuitionBase *IntuitionBase;
struct GfxBase       *GfxBase;
struct Library       *LowLevelBase;
struct Library       *CDUIBase;

struct MsgPort       *MsgPort;
struct Message       *Message, *ShutdownMsg = NULL;

struct BMInfo        *BMInfo;
struct BitMap        *BitMap;
struct NewScreen      NewScreen;
struct Screen        *Screen;

APTR                 OWB;
ULONG                ColorTable[258];
struct ColorSpec     ColorSpec[257];
LONG                 FadeValue = 0;

struct TagItem       TagList[] = {

    { SA_Colors,     0         },
    { SA_Overscan,   OSCAN_MAX },
    { TAG_END,       0         }
    };

LONG                 i, j;
LONG                 PortLong;

    if (MsgPort = CreateMsgPort()) {                                                            /* Create a message port      */

        MsgPort->mp_Node.ln_Name = "Startup Animation";                                         /* Animation MsgPort Name     */

        AddPort(MsgPort);                                                                       /* Add port to system         */

        if (LowLevelBase = OpenLibrary("lowlevel.library", 0)) {

          if (CDUIBase = OpenLibrary("cdui.library", 0)) {

            if (DeBoxBase = (struct DeBoxBase *)OpenLibrary("debox.library", 0)) {              /* Picture decompression      */

              if (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0)) {/* For RGB stuff              */

                if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)) {

                    SetChipRev(SETCHIPREV_BEST);                                                /* Go into AA mode            */

                    if (BMInfo = DecompBMInfo(NULL, NULL, &strapimage)) {                       /* Get Info about picture     */

                        if (BitMap = AllocBitMap(BMInfo->bmi_Width, BMInfo->bmi_Height,         /* Allocate bit map for anim. */
                            BMInfo->bmi_Depth, BMF_DISPLAYABLE, NULL)) {

                            if (!DecompBitMap(NULL, &strapimage, BitMap, NULL)) {               /* Decompress into bit map    */

                                NewScreen.LeftEdge      = 0;                                    /* Describe the anim screen   */
                                NewScreen.TopEdge       = 0;
                                NewScreen.Width         = BMInfo->bmi_Width;
                                NewScreen.Height        = BMInfo->bmi_Height;
                                NewScreen.Depth         = BMInfo->bmi_Depth;
                                NewScreen.ViewModes     = BMInfo->bmi_Modes;
                                NewScreen.Type          = CUSTOMSCREEN|CUSTOMBITMAP|SCREENQUIET;
                                NewScreen.DefaultTitle  = NULL;
                                NewScreen.Gadgets       = NULL;
                                NewScreen.CustomBitMap  = BitMap;

                                for (i=0,j=1; i!=BMInfo->bmi_NumColors; i++) {                  /* Build color tables         */

                                    ColorTable[j++] = (BMInfo->bmi_ColorMap[i] >> 8) & 0x0F;    /* Target colors              */
                                    ColorTable[j++] = (BMInfo->bmi_ColorMap[i] >> 4) & 0x0F;
                                    ColorTable[j++] =  BMInfo->bmi_ColorMap[i]       & 0x0F;

                                    ColorSpec[i].ColorIndex = i;                                /* Initial colors (all black) */
                                    ColorSpec[i].Red        =
                                    ColorSpec[i].Green      =
                                    ColorSpec[i].Blue       = 0;
                                    }

                                ColorTable[j]           = 0;                                    /* Finish tables              */
                                ColorTable[0]           = i<<16;
                                ColorSpec[i].ColorIndex = -1;
                                TagList[0].ti_Data      = (ULONG)&ColorSpec[0];

                                if (Screen = OpenScreenTagList(&NewScreen, &TagList[0])) {      /* Open a screen to our specs */

                                    FadeTo(16, &FadeValue, ColorTable,                          /* Fade to full brightness    */
                                           &Screen->ViewPort, GfxBase);

                                    while (!ShutdownMsg) {                                      /* Should we be running?      */

                                        if (Message = GetMsg(MsgPort)) {                        /* Is there a message for us? */

                                            if (Message->mn_Length == ANIMMSG_SHUTDOWN)         /* If we have a shutdown msg  */
                                                ShutdownMsg = Message;

                                            if (Message->mn_Length == ANIMMSG_BOOTING) {        /* Is a CD title now booting? */

                                                ReplyMsg(Message);                              /* Reply to "booting" message */

                                                /* A bootable disk has been inserted.  Do         */
                                                /* something visually different to indicate this. */

                                                FadeTo(8, &FadeValue, ColorTable,               /* Fade to half brightness    */
                                                       &Screen->ViewPort, GfxBase);
                                                FadeTo(16, &FadeValue, ColorTable,              /* Fade to full brightness    */
                                                       &Screen->ViewPort, GfxBase);
                                                }

                                            if (Message->mn_Length == ANIMMSG_HOLDOFFANIM) {    /* Hold off animation msg?    */

                                                ReplyMsg(Message);                              /* Reply to message           */

                                                /* The drive door has been closed.  Do            */
                                                /* something visually different to indicate this. */

                                                FadeTo(8, &FadeValue, ColorTable,               /* Fade to half brightness    */
                                                       &Screen->ViewPort, GfxBase);
                                                }

                                            if (Message->mn_Length == ANIMMSG_FALSEALARM) {     /* False alarm message?       */

                                                ReplyMsg(Message);                              /* Reply to message           */

                                                /* False alarm.  We can animate again.  */

                                                FadeTo(16, &FadeValue, ColorTable,              /* Fade to full brightness    */
                                                       &Screen->ViewPort, GfxBase);
                                                }
                                            }

                                        PortLong = ReadJoyPort(1);

                                        if (PortLong & JPF_BTN1) {

                                            FadeTo(0, &FadeValue, ColorTable,                   /* Fade to black              */
                                                   &Screen->ViewPort, GfxBase);
                                            PresentCDUI(CDUI_LANGUAGE_SELECTOR);
                                            }

                                        else if (PortLong & JPF_BTN2) {

                                            FadeTo(0, &FadeValue, ColorTable,                   /* Fade to black              */
                                                   &Screen->ViewPort, GfxBase);
                                            PresentCDUI(CDUI_NRAM_EDITOR);
                                            }

                                        /* Handle language selection */

                                        /* Animate screen */

                                        WaitTOF();
                                        }

                                    FadeTo(0, &FadeValue, ColorTable,                           /* Fade to black              */
                                           &Screen->ViewPort, GfxBase);

                                    OWB = SetFunction(IntuitionBase, -0xd2, (APTR)&FakeOWB);    /* Done showing screen        */
                                    CloseScreen(Screen);
                                    SetFunction(IntuitionBase, -0xd2, OWB);
                                    }
                                }

                            FreeBitMap(BitMap);                                                 /* Free animation bit map     */
                            }

                        FreeBMInfo(BMInfo);                                                     /* Free bit map information   */
                        }

                    CloseLibrary(GfxBase);                                                      /* Close graphics library     */
                    }

                CloseLibrary(IntuitionBase);                                                    /* Close intuition            */
                }

              CloseLibrary((struct Library *)DeBoxBase);                                        /* Close debox library        */
              }

            CloseLibrary(CDUIBase);
            }

          CloseLibrary(LowLevelBase);
          }

        RemPort(MsgPort);                                                                       /* Remove and delete MsgPort  */
        DeleteMsgPort(MsgPort);
        }

    Forbid();                                                                                   /* Forbid while expunging     */
    if (ShutdownMsg) ReplyMsg(ShutdownMsg);                                                     /* Signal that we are done    */
    RemTask(NULL);                                                                              /* Remove ourselves           */
    Permit();                                                                                   /* Let 'er rip                */
    Wait(0);
    }


