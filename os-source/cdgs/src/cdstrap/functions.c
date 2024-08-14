
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/interrupts.h>
#include <libraries/dos.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/audio.h>
#include <devices/trackdisk.h>
#include <dos/filehandler.h>

#define EXEC_PRIVATE_PRAGMAS

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include "pragmas/playerprefs_private_pragmas.h"
#include "pragmas/cdfs_pragmas.h"
#include "pragmas/debox_pragmas.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include "clib/playerprefs_protos.h"
#include "clib/cdfs_protos.h"
#include "clib/debox_protos.h"

#include "cd/cd.h"
#include <cdtv/cdtvprefs.h>
#include <cdtv/debox.h>
#include "cdstrap.h"

#define SysBase         (*((struct ExecBase **)0x00000004L))
#define GfxBase         V->GfxBase
#define IntuitionBase   V->IntuitionBase
#define PlayerPrefsBase V->PlayerPrefsBase

extern int  ChkSumFD(char *);

extern struct Custom __far custom;

#define EMPTY(n)    ((ULONG)(n)->lh_Head == (ULONG)&n->lh_Tail)


/*******************************************************************************
 *                                                                             *
 *  DoIOR - Send IORequest to a device                                         *
 *                                                                             *
 *    in:   IOStdReq *Req                                                      *
 *          UWORD     Cmd                                                      *
 *          ULONG     Offset                                                   *
 *          ULONG     Length                                                   *
 *          UWORD    *Data                                                     *
 *                                                                             *
 *   out:   int       Error                                                    *
 *                                                                             *
 *******************************************************************************/

int DoIOR(struct IOStdReq *Req, UWORD cmd, ULONG Offset,
              ULONG Length, UWORD *Data, struct V *V) {

    if (Req->io_Device) {                                                       /* Is the request usable?               */

        Req->io_Command = cmd;                                                  /* Set up request                       */
        Req->io_Offset  = Offset;
        Req->io_Length  = Length;
        Req->io_Data    = (APTR)Data;

        return((int)DoIO(Req));                                                 /* Do the IORequest and return error    */
        }

    return(0);                                                                  /* Could not perform request            */
    }


/*******************************************************************************
 *                                                                             *
 *  DoIOActual - Do a simple command that returns status in io_Actual          *
 *                                                                             *
 *    in:   IOStdReq *Req                                                      *
 *          UWORD     Cmd                                                      *
 *                                                                             *
 *   out:   int       io_Actual                                                *
 *                                                                             *
 *******************************************************************************/

int GetIOActual(register struct IOStdReq *req, LONG cmd, struct V *V) {

    DoIOR(req, cmd, 0, 0, 0, V);                                                /* Do the request and return io_Actual  */
    return((int)req->io_Actual);
    }



/*******************************************************************************
 *                                                                             *
 *  GetStatus - Get status word from CD_INFO command                           *
 *                                                                             *
 *    in:   IOStdReq *Req                                                      *
 *                                                                             *
 *   out:   UWORD     Status                                                   *
 *                                                                             *
 *******************************************************************************/

UWORD GetStatus(register struct IOStdReq *req, struct V *V) {

struct CDInfo CDInfo;

    DoIOR(req, CD_INFO, 0, sizeof(struct CDInfo), (UWORD *)&CDInfo, V);          /* Do the request and return io_Actual  */
    return(CDInfo.Status);
    }



/*******************************************************************************
 *                                                                             *
 *  Compare - Compare two buffers of data                                      *
 *                                                                             *
 *  in: Data1 - First buffer                                                   *
 *      Data2 - Second buffer                                                  *
 *      Size  - Size of buffers (must be a multiple of 4)                      *
 *                                                                             *
 *******************************************************************************/

int Compare(ULONG *Data1, ULONG *Data2, int Size) {

LONG i;

    for (i=0; i<Size; i+=4) if (*Data1++ != *Data2++) return(0);                /* Compare the buffers and return result */

    return(1);
    }


/*******************************************************************************************
 *                                                                                         *
 *  InitAudio - Initialize audio device for 1.3 applications                               *
 *                                                                                         *
 *******************************************************************************************/

void InitAudio(struct V *V) {

struct Resident *Res;
struct ExecBase *EB = SysBase;

    if (!FindName((struct List *)&EB->DeviceList, "audio.device")) {                        /* Find audio device in list */

        if (Res = (struct Resident *)FindResident("audio.device")) InitResident(Res, 0L);   /* Initialize audio.device   */
        }
    }




/*******************************************************************************************
 *                                                                                         *
 *  Kludges - Apply kludges to CDTV titles                                                 *
 *                                                                                         *
 *******************************************************************************************/

void Kludges(struct V *V) {

register struct MsgPort *mp;
struct CDTVPrefs CDTVPrefs;

    InitAudio(V);                                                                           /* Do standard 1.3 patches      */
    OwnBlitter();
    WaitBlit();
    custom.intena = INTF_BLIT;
    DisownBlitter();
    FillCDTVPrefs(&CDTVPrefs);

    if (mp = (struct MsgPort *)AllocMem(sizeof(struct MsgPort), MEMF_CLEAR|MEMF_PUBLIC)) {  /* Create a MsgPort for kludges */

        mp->mp_Node.ln_Type = NT_MSGPORT;                                                   /* Initialize the port          */
        mp->mp_Node.ln_Name = "CDBOOT";

        AddPort(mp);                                                                        /* Add port to system list      */
        }   
    }






/***********************************************************************************
 *                                                                                 *
 *  InitScreen - Open a fake, blank screen and close to initialize for Player      *
 *                                                                                 *
 ***********************************************************************************/

extern LONG    __asm FakeOWB(void);

void InitScreen(struct V *V) {

APTR                 OWB;
int                  i;

struct NewScreen     NewScreen;
struct Screen       *Screen;
struct ColorSpec     ColorSpec[17];

struct TagItem       TagList[] = {

    { SA_Colors,     0         },
    { TAG_END,       0         }
    };

    NewScreen.LeftEdge      = 0;                                                    /* Define screen specs        */
    NewScreen.TopEdge       = 0;
    NewScreen.Width         = 640;
    NewScreen.Height        = 200;
    NewScreen.Depth         = 1;
    NewScreen.ViewModes     = HIRES;
    NewScreen.Type          = CUSTOMSCREEN|SCREENQUIET;
    NewScreen.DefaultTitle  = NULL;
    NewScreen.Gadgets       = NULL;
    NewScreen.CustomBitMap  = NULL;

    for (i=0; i!=16; i++) {                                                         /* Build color tables         */

        ColorSpec[i].ColorIndex = i;                                                /* Initial colors (all black) */
        ColorSpec[i].Red        =
        ColorSpec[i].Green      =
        ColorSpec[i].Blue       = 0;
        }

    ColorSpec[i].ColorIndex = -1;                                                   /* Store color table          */
    TagList[0].ti_Data      = (ULONG)&ColorSpec[0];

    if (Screen = OpenScreenTagList(&NewScreen, &TagList[0])) {                      /* Open a screen to our specs */

        OWB = SetFunction((struct Library *)IntuitionBase, -0xd2, (APTR)&FakeOWB);  /* Immediately close screen   */
        CloseScreen(Screen);
        SetFunction((struct Library *)IntuitionBase, -0xd2, OWB);
        }
    }
