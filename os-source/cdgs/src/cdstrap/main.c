
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/interrupts.h>
#include <libraries/dos.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <intuition/preferences.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/modeid.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/audio.h>
#include <devices/trackdisk.h>
#include <dos/filehandler.h>
#include <internal/iprefs.h>
#include <utility/utility.h>
#include <internal/cdui.h>
#include <libraries/lowlevel.h>

#define EXEC_PRIVATE_PRAGMAS

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include "pragmas/cardres_pragmas.h"
#include "pragmas/playerprefs_private_pragmas.h"
#include "pragmas/cdfs_pragmas.h"
#include "pragmas/debox_pragmas.h"
#include "pragmas/utility_pragmas.h"
#include "pragmas/nonvolatile_pragmas.h"
#include "pragmas/lowlevel_pragmas.h"
#include "cdgs:include/pragmas/videocd_pragmas.h"
#include "cdui/cdui_pragmas.h"
#include "/nofastmem/nofastmem_pragmas.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include "clib/cardres_protos.h"
#include "clib/playerprefs_protos.h"
#include "clib/cdfs_protos.h"
#include "clib/debox_protos.h"
#include "clib/utility_protos.h"
#include "clib/nonvolatile_protos.h"
#include "clib/lowlevel_protos.h"
#include "cdgs:include/clib/videocd_protos.h"
#include "cdui/cdui_protos.h"
#include "/nofastmem/nofastmem_protos.h"

#include "gs:cd/cd.h"
#include "cdtv/cdtvprefs.h"
#include "gs:pplibrary/playerprefsbase.h"
#include "cdgs:include/libraries/videocd.h"
#include "cdtv:include/internal/card.h"
#include <cdtv/debox.h>

#include "cdstrap.h"
#include "functions.h"
#include "animation.h"

#pragma libcall IntuitionBase SetIPrefs 240 10803
APTR SetIPrefs(APTR ptr, LONG size, LONG type );

#pragma libcall CDUIBase StartCDUI 1e 00
struct MsgPort *StartCDUI(void);

#pragma libcall MPEGPlayerBase CDPlayer 1e 0
BOOL CDPlayer( void );


#define BUFF_SIZE (512*2)

extern char              __far ModIdent[];
extern struct CompHeader __far trademark;
extern struct CompHeader __far cdtvtm;
extern UBYTE *           __far picture;

int         Main(void);
int         MainLoop(struct V *V);
int         CheckTM(struct V *V);
int         PollXIPCard(struct V *V);
int         ScanBootList(struct V *V);
ULONG       OpenCDUI(struct V *V);
void        StripDevices(struct V *V);
void        StripMemory(struct V *V);
void        CenterScreen(struct V *V);
void        SendAnimMessage(int Msg);
void        RemoveReqZapper(struct V *V);

extern int  StartAnimation(void);
extern int  EjectReset(int Flag, struct V *V);
extern int  ChkSumFD(char *);

VOID __asm UnlinkRomTag(register __a0 struct Resident *kt);

extern struct Custom __far custom;

ULONG ReadGayle(void);

/*******************************************************************************************
 *                                                                                         *
 *  Main - CDStrap entry point.  Initializes/Frees CDStrap resources.                      *
 *                                                                                         *
 *  I'm not at all happy with the way this module works.  We really need to integrate this *
 *  in with strap.                                                                         *
 *                                                                                         *
 *******************************************************************************************/

int Main(void) {

struct V            Variables;
struct V           *V = &Variables;
struct UtilityBase *UtilityBase;
struct Library     *NVBase;
char               *ReservedArea;
ULONG               PortLong;

struct TagItem ConfigList[] = {

    { TAGCD_EJECTRESET,  1   },
    { TAG_END,           0   }
    };


struct TagItem ConfigDS[] = {

    { TAGCD_READSPEED,   150 },
    { TAG_END,           0   }
    };



    V->ExecBase          = *((struct ExecBase **)0x00000004L);                              /* Effectively open exec.library    */

#define SysBase (*((struct ExecBase **)0x00000004L))                                        /* Redirect exec calls through this */

    V->ExpansionBase    = (struct ExpansionBase *)OpenLibrary("expansion.library", 0L);     /* Open all relevant libraries      */
    V->GfxBase          = (struct GfxBase *)OpenLibrary("graphics.library", 0L);
    V->DeBoxBase        = (struct DeBoxBase *)OpenLibrary("debox.library", 0L);
    V->IntuitionBase    = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
    V->CDFSBase         = (struct CDFSBase *)OpenLibrary("cdfs.library", 0L);
    V->CardResource     = (struct Library *)OpenResource((STRPTR)CARDRESNAME);
    V->PlayerPrefsBase  = (struct PlayerPrefsBase *)OpenLibrary("playerprefs.library", 0L);
    V->CDUIBase         = NULL;
    V->LowLevelBase     = OpenLibrary("lowlevel.library", 0);

#define ExpansionBase   V->ExpansionBase                                                    /* Redirect library calls           */
#define GfxBase         V->GfxBase
#define DeBoxBase       V->DeBoxBase
#define IntuitionBase   V->IntuitionBase
#define CDFSBase        V->CDFSBase
#define CardResource    V->CardResource
#define PlayerPrefsBase V->PlayerPrefsBase
#define CDUIBase        V->CDUIBase
#define LowLevelBase    V->LowLevelBase

    if (ExpansionBase && GfxBase && DeBoxBase && IntuitionBase && PlayerPrefsBase) {        /* Make sure all libraries openned  */

        if (UtilityBase = (struct UtilityBase *)OpenLibrary("utility.library", 0)) {        /* Open utility.library             */

            if (NVBase = OpenLibrary("nonvolatile.library", 0)) {                           /* Open nonvolatile.library         */

                if (ReservedArea = GetCopyNV("", "", TRUE)) {                               /* Get reserved nonvolatile memory  */

                    V->AudioBoot = ((ReservedArea[1] & 0x01) == 0);                         /* Go to player if audio disk here? */

                    UtilityBase->ub_Language = ReservedArea[0];                             /* First byte of reserved=language  */
                    FreeNVData(ReservedArea);
                    }

                CloseLibrary(NVBase);                                                       /* Done with nonvolaitle.library    */
                }

            CloseLibrary(UtilityBase);                                                      /* Done with utility.library        */
            }

        V->IOPort.mp_Flags     = 0;                                                         /* Init MsgPort and add to system   */
        V->IOPort.mp_SigBit    = AllocSignal(-1);
        V->IOPort.mp_SigTask   = FindTask(NULL);

        V->IOPort.mp_MsgList.lh_Head     = (struct Node *)&V->IOPort.mp_MsgList.lh_Tail;    /* NewList                          */
        V->IOPort.mp_MsgList.lh_Tail     = 0;
        V->IOPort.mp_MsgList.lh_TailPred = (struct Node *)&V->IOPort.mp_MsgList.lh_Head;

        V->IOReq.io_Message.mn_Node.ln_Type = NT_MESSAGE;                                   /* Init an I/O request              */
        V->IOReq.io_Message.mn_ReplyPort    = &V->IOPort;
        V->IOReq.io_Device                  = NULL;

        V->CDReq          = V->IOReq;                                                       /* Make 2 copies of the request     */
        V->SReq.iotd_Req  = V->IOReq;
        
        V->CardHandle = (struct CardHandle *)AllocMem((ULONG)sizeof(struct CardHandle),     /* Allocate card handle             */
                                                      MEMF_CLEAR);

        V->CardPolled = FALSE;                                                              /* Card has not yet been polled     */

        V->BootPri    = -32768;                                                             /* BootList not yet scanned         */

        V->CDChgCnt    = V->CardChgCnt  = -1;                                               /* Each device gets checked once    */
        V->TDChgCnt[0] = V->TDChgCnt[1] = V->TDChgCnt[2] = V->TDChgCnt[3] = -1;

        V->Animating = 0;

        if (!(PortLong = ReadJoyPort(1) & ~JP_TYPE_MASK))
              PortLong = ReadJoyPort(0) & ~JP_TYPE_MASK;

        if      (PortLong & JPF_BTN1) {

            CenterScreen(V);                                                                /* and make sure screen is centered */                            
 
            if (OpenCDUI(V)) {                                                              /* Don't allow animation to come up */

                SendAnimMessage(ANIMMSG_BLUE_BUTTON);
                SendAnimMessage(ANIMMSG_SHUTDOWN);
                ColdReboot();
                }
            }

        else if (PortLong & JPF_BTN2) {

            CenterScreen(V);                                                                /* and make sure screen is centered */                            
 
            if (OpenCDUI(V)) {                                                              /* Don't allow animation to come up */

                SendAnimMessage(ANIMMSG_RED_BUTTON);
                SendAnimMessage(ANIMMSG_SHUTDOWN);
                ColdReboot();
                }
            }

        if (!OpenDevice("cd.device", 0, &V->CDReq, 0)) {                                    /* cd.device better work            */

            switch (MainLoop(V)) {

                case -32766: /* AmigaCD Title */

                        if (ScanBootList(V) <= 2) {                                         /* Higher boot device present?      */

                            DoIOR(&V->CDReq, CD_CONFIG, 0, 0,                               /* Reset on disk removal            */
                                (UWORD *)&ConfigList[0], V);

                            if (!V->Animating) {

                                CenterScreen(V);                                            /* and make sure screen is centered */                            
                                if (!OpenCDUI(V)) break;                                    /* Do disk insertion Animation      */
                                }

                            if (!(ExpansionBase->Flags & EBF_DOSFLAG)) {

                                SendAnimMessage(ANIMMSG_HOLDOFFANIM);                       /* Booting an AmigaCD title         */
                                SendAnimMessage(ANIMMSG_BOOTING);
                                StripDevices(V);                                            /* Strip unneeded devices for memory*/
                                }

                            else SendAnimMessage(ANIMMSG_SHUTDOWN);

                            SysBase->LastAlert[3] = 0;                                      /* Turn off guru msgs otherwise     */
                            }

                        else {

                            RemoveReqZapper(V);                                             /* Remove request zapper            */
                            DoIOR(&V->CDReq, CD_CONFIG, 0, 0, (UWORD *)&ConfigDS[0], V);    /* Double speed when booting non CD */
                            }

                        break;

                case -32765: /* CDTV Title */

                        RemoveReqZapper(V);                                                 /* Remove request zapper            */

                        if (ScanBootList(V) <= 2) {                                         /* Higher boot device present?      */

                            if (V->Animating) {                                             /* Reboot if animation is present   */

                                SendAnimMessage(ANIMMSG_SHUTDOWN);                          /* Free animation now               */
                                ColdReboot();
                                }

                            else {                                                          /* Do CDTV kludges                  */

                                DoIOR(&V->CDReq, CD_CONFIG, 0, 0,                           /* Reset on disk removal            */
                                   (UWORD*)&ConfigList[0], V);

                                if (!(ExpansionBase->Flags & EBF_DOSFLAG)) InitScreen(V);   /* Perform kludges                  */
                                Kludges(V);
                                }

                            SysBase->LastAlert[3] = 0;                                      /* Turn off guru msgs.              */
                            }

                        else {

                            DoIOR(&V->CDReq, CD_CONFIG, 0, 0, (UWORD *)&ConfigDS[0], V);    /* Double speed when booting non CD */
                            }

                        break;

                default: /* Anything Else */

                        RemoveReqZapper(V);                                                 /* Remove request zapper            */
                        DoIOR(&V->CDReq, CD_CONFIG, 0, 0, (UWORD *)&ConfigDS[0], V);        /* Double speed when booting non CD */

                        if (V->Animating) {

                            SendAnimMessage(ANIMMSG_SHUTDOWN);                              /* Free animation now               */
                            ColdReboot();
                            }

                        break;
                }

            MountFS();                                                                      /* Start the file system            */
            }

        if (V->CardHandle) FreeMem(V->CardHandle, (ULONG)sizeof(struct CardHandle));

        FreeSignal(V->IOPort.mp_SigBit);
        }

    CloseLibrary((struct Library *)LowLevelBase);                                           /* Close openned libraries          */
    CloseLibrary((struct Library *)PlayerPrefsBase);
    CloseLibrary((struct Library *)CDFSBase);
    CloseLibrary((struct Library *)IntuitionBase);
    CloseLibrary((struct Library *)DeBoxBase);
    CloseLibrary((struct Library *)GfxBase);
    CloseLibrary((struct Library *)ExpansionBase);

    if (CDUIBase) CloseLibrary((struct Library *)CDUIBase);

    return(0);                                                                              /* All done                         */
    }



void StartCDPlayer1(struct V *V) {

    if (V->Animating) SendAnimMessage(ANIMMSG_SHUTDOWN);                                    /* Fade to black for player screen  */
    else              CenterScreen(V);                                                      /* Center the screen.               */

    InitAudio(V);                                                                           /* Do that vodo that you do so well */
    OwnBlitter();
    WaitBlit();
    custom.intena = INTF_BLIT;
    DisownBlitter();

    DoPlayer();                                                                             /* Ok, just run the old player      */
    }


void StartCDPlayer2(struct V *V) {

struct Library  *MPEGPlayerBase;
struct Resident *Resident;

    if (V->Animating) SendAnimMessage(ANIMMSG_SHUTDOWN);                                    /* Fade to black for player screen  */
    else              CenterScreen(V);                                                      /* Center the screen.               */

    InitAudio(V);                                                                           /* Do that vodo that you do so well */
    OwnBlitter();
    WaitBlit();
    custom.intena = INTF_BLIT;
    DisownBlitter();

    if (MPEGPlayerBase = OpenLibrary("mpegplayer.library", 0)) CDPlayer();                  /* Run the MPEG player              */

    else if (Resident = FindResident("mpegplayer.library")) {                               /* If not present, try finding it   */

        InitResident(Resident, 0);                                                          /* Found it, init it.               */

        if (MPEGPlayerBase = OpenLibrary("mpegplayer.library", 0)) CDPlayer();              /* Try running it now               */
        }

    DoPlayer();                                                                             /* Ok, just run the old player      */
    }

/***************************************************************************************
 *                                                                                     *
 *  Main boot device scanning loop.  Checks floppy drives, CD-ROM drives, and devices  *
 *  in the BootList for a device to boot off of.  When one is found, this loop         *
 *  terminates and returns.                                                            *
 *                                                                                     *
 *  out:  -32767 = XIP Card                                                            *
 *        -32766 = CDGS title present                                                  *
 *        -32765 = CDTV title present                                                  *
 *        else   = BootNode pri                                                        *
 *                                                                                     *
 ***************************************************************************************/

int MainLoop(struct V *V) {

struct CDInfo   CDInfo;
UWORD           Status, LastStatus = CDSTSF_CLOSED, AnimHeldOff = 0;
int             BootPri;
LONG            CDType, cnt;
struct Library *VideoCDBase;

    while (1) {                                                                         /* Loop forever.  return() gets us out  */

        if (PollXIPCard(V)) return(-32767);                                             /* poll for a execute-in-place card     */

        if ((Status = GetStatus(&V->CDReq, V)) != LastStatus) {                         /* Check for door close                 */

            if ((Status & CDSTSF_CLOSED) && !(LastStatus & CDSTSF_CLOSED)) {            /* Has door just been closed?           */

                SendAnimMessage(ANIMMSG_HOLDOFFANIM);                                   /* Tell animation task to not animate   */
                AnimHeldOff = 1;
                }

            LastStatus = Status;                                                        /* Remember our current state           */
            }

        if ((cnt = GetIOActual(&V->CDReq, CD_CHANGENUM, V)) > V->CDChgCnt) {            /* See if a CD change event has occured */

            V->CDChgCnt = cnt;                                                          /* Event being handled                  */

            if (!GetIOActual(&V->CDReq, CD_CHANGESTATE, V)) {                           /* Is there a disk now?                 */

                DoIOR(&V->CDReq, CD_INFO, 0, sizeof(struct CDInfo), (UWORD *)&CDInfo, V);

                if (!V->CDReq.io_Error && (CDInfo.Status & CDSTSF_CDROM)) {             /* Data disk?                           */

                    switch (CheckTM(V)) {                                               /* Check disk validity                  */

                        case 0:
                                if (VideoCDBase = OpenLibrary("videocd.library", 0)) {  /* No trademark, check disk type        */

                                    CDType = GetCDTypeA(NULL, NULL);                    /* Get type                             */

                                    if (  (CDType == CDT_KARAOKE)                       /* MPEG disk?                           */
                                       || (CDType == CDT_CDIFMV)
                                       || (CDType == CDT_VIDEOCD)) {

                                        CloseLibrary(VideoCDBase);                      /* Done with VideoCD library            */

                                        if ((ScanBootList(V) <= 2) || V->AudioBoot) {   /* Does audio player have priority?     */

                                            StartCDPlayer2(V);                          /* Start CD Player                      */
                                            }
                                        }

                                    CloseLibrary(VideoCDBase);                          /* Close VideoCD library                */
                                    }

                                if (AnimHeldOff) {                                      /* Not a valid disk                     */

                                    SendAnimMessage(ANIMMSG_FALSEALARM);                /* False alarm                          */
                                    AnimHeldOff = 0;
                                    }

                                break;

                        case 1: return(-32766);
                        case 2: return(-32765);
                        }
                    }

                else if ((ScanBootList(V) <= 2) || V->AudioBoot) StartCDPlayer1(V);     /* Run player if it has priority        */
                }

            else if (AnimHeldOff) {                                                     /* No disk, resume animation            */

                SendAnimMessage(ANIMMSG_FALSEALARM);
                AnimHeldOff = 0;
                }
            }

        if ((BootPri = ScanBootList(V)) != -32768) return(BootPri);                     /* BootList device present              */

        if (!V->Animating) {                                                            /* If we didn't start booting something,*/

            CenterScreen(V);                                                            /* center the screen...                 */
            if (OpenCDUI(V)) SendAnimMessage(ANIMMSG_STARTUP);                          /* and start animation                  */
            }

        WaitTOF();                                                                      /* Give animation some time to run      */
        }
    }



/*******************************************************************************************
 *                                                                                         *
 *  CheckTM - Check for trademark information on CDs.                                      *
 *                                                                                         *
 *  out:    result = 0 - no trademark, 1 = CDGS trademark, 2 = CDTV trademark              *
 *                                                                                         *
 *******************************************************************************************/

int CheckTM(struct V *V) {

struct TMStruct {

    ULONG   Size;
    ULONG   Sectors[4];
    };

struct TMStruct   *TMInfo;
struct CompHeader *Header;
UBYTE              Buff[2048], *TMBuff;
int                result = 0;

    if (ValidDisk()) {                                                                      /* Valid CD-ROM disk in drive?      */

        TMInfo = (struct TMStruct *)GetTMInfo();                                            /* Get sector where trademark is    */

        if (TMInfo->Sectors[0]) {                                                           /* Does a trademark exist?          */

            if (!DoIOR(&V->CDReq, CD_READ, TMInfo->Sectors[0]*2048, 2048,                   /* Read a sector of trademark info  */
                      (UWORD *)Buff, V)) {

                Header = &trademark;                                                        /* We are going to compare with TM  */

                if (TMBuff = AllocMem(Header->ci_Size, MEMF_PUBLIC)) {                      /* Allocate memory to decompress TM */

                    if (DecompData(NULL, Header, TMBuff)) {                                 /* Decompress trademark             */

                        if (Compare((ULONG *)Buff, (ULONG *)TMBuff, 2048)) result = 1;      /* Does disk have a valid TM?       */
                        }

                    FreeMem(TMBuff, Header->ci_Size);                                       /* Free trademark buffer            */
                    }

                if (!result) {                                                              /* Does a CDGS trademark exist?     */

                    Header = &cdtvtm;                                                       /* We are going to compare with TM  */

                    if (TMBuff = AllocMem(Header->ci_Size, MEMF_PUBLIC)) {                  /* Allocate memory to decompress TM */

                        if (DecompData(NULL, Header, TMBuff)) {                             /* Decompress trademark             */

                            if (Compare((ULONG *)Buff, (ULONG *)TMBuff, 2048)) result = 2;  /* Does disk have a valid TM?       */
                            }

                        FreeMem(TMBuff, Header->ci_Size);                                   /* Free trademark buffer            */
                        }
                    }
                }
            }
        }

    return(result);                                                                         /* Return result of TM search       */
    }



/***************************************************************************
 *                                                                         *
 *  PollXIPCard - See if an eXecute In Place card is inserted              *
 *                                                                         *
 ***************************************************************************/

int PollXIPCard(struct V *V) {

int   foundXIP;
int   cardfree;
ULONG cardcount;

struct Resident *rc;

    foundXIP = FALSE;                                                       /* Initialize state flags                       */
    cardfree = TRUE;

    if (CardResource && V->CardHandle) {                                    /* Does a credit card port exist?               */

        V->CardHandle->cah_CardFlags = CARDF_IFAVAILABLE;                   /* Reinit CardFlags before calling OwnCard()    */
        
        if (!(OwnCard(V->CardHandle))) {                                    /* Allocate the credit card                     */

            if ((cardcount = CardChangeCount()) != V->CardChgCnt) {         /* Don't check again if count hasn't changed    */

                V->CardChgCnt = cardcount;                                  /* The event is now recognized                  */

                if (rc = IfAmigaXIP(V->CardHandle)) {                       /* Is this an XIP card?                         */

                    foundXIP = TRUE;                                        /* ... Yep, it sure is                          */

                    if (V->CardPolled == FALSE) {                           /* Have we already recognized the card?         */

                        foundXIP = FALSE;                                   /* Don't do it again                            */

                        if (CardResetRemove(V->CardHandle, TRUE)) {         /* Make the computer reboot if card is removed  */

                            if (BeginCardAccess(V->CardHandle)) {           /* Turn on CC light                             */

                                if (InitResident(rc, NULL)) {               /* Allow credit card to insert a BootNode       */

                                    cardfree        = FALSE;                /* In use forever                               */
                                    CardResource    = NULL;

                                    V->CardHandle   = NULL;                 /* DO NOT FREE HANDLE!                          */
                                    }
                                }
                            }
                        }
                    }
                }

            if (cardfree) ReleaseCard(V->CardHandle, CARDF_REMOVEHANDLE);   /* reset state (light off, & reset-remove off)  */
            }
        }

    V->CardPolled = TRUE;                                                   /* Indicate we have run this code at least once */

    return(foundXIP);                                                       /* Return result                                */
    }




/***************************************************************************
 *                                                                         *
 *  ScanBootList - Check for a boot device on the Mount List               *
 *                                                                         *
 *      out:  Boot priority or -32768 if no boot device present            *
 *                                                                         *
 ***************************************************************************/

int ScanBootList(struct V *V) {

struct BootNode          *ln;
struct DeviceNode        *dn;
struct FileSysStartupMsg *fssm;

char   *devname;
LONG    devunit;
LONG   *envec;
int     boot, Poll;
UBYTE   Buff[BUFF_SIZE];
LONG    cnt;


    if (V->BootPri != -32768) return(V->BootPri);                                   /* If scanned twice, return same result */

    ln = (struct BootNode *)ExpansionBase->MountList.lh_Head;

    if (ln->bn_Node.ln_Succ) {                                                      /* Anything on the list ?               */

         while (ln) {                                                               /* Traverse whole device list           */

            if (ln->bn_Node.ln_Type == NT_BOOTNODE) {                               /* Only look at boot nodes on list      */

                boot = 1;                                                           /* If everything goes well, should boot */

                fssm=NULL;                                                          /* Try and find a FSSM                  */

                if (dn = (struct DeviceNode *)                                      /* Get device node pointer              */
                    ((struct BootNode *)ln->bn_DeviceNode)) {

                    if (fssm = BADDR(dn->dn_Startup)) {                             /* Get file system startup message      */

                        devname = (char *)BADDR(fssm->fssm_Device)+1;               /* Get FSSM info                        */
                        devunit = fssm->fssm_Unit;
                        envec   = (LONG *)(BADDR(fssm->fssm_Environ));
                        }
                    }

                if (fssm && ((envec[0] >= DE_BOOTBLOCKS))) {                        /* Is it a bootblock device ?           */

                    if (!OpenDevice(devname, devunit, &V->SReq, 0)) {               /* opened it                            */

                        Poll = 0;

                        if (Compare((ULONG *)devname,                               /* Are we scanning trackdisk.device?    */
                                    (ULONG *)"trackdisk.device", 16)) {

                            if ((cnt = GetIOActual(&V->SReq, TD_CHANGENUM, V))      /* See if a floppy change event occured */
                                                          > V->TDChgCnt[devunit]) {

                                V->TDChgCnt[devunit] = cnt;                         /* Event being handled                  */

                                if (!GetIOActual(&V->SReq, TD_CHANGESTATE, V)) {    /* See if a floppy change event occured */

                                    Poll = 1;                                       /* Only poll floppy drive after change  */
                                    }
                                }
                            }

                        else Poll = 1;

                        if (Poll) {

                            DoIOR(&V->SReq, CMD_CLEAR, 0, 0, 0, V);                 /* Clear cache!                         */

                            if (DoIOR(&V->SReq, CMD_READ, 0, BUFF_SIZE,             /* Read boot information                */
                               (UWORD *)Buff, V))
                                boot = 0;

                            DoIOR(&V->SReq, TD_MOTOR, 0, 0, 0, V);                  /* Motor off                            */

                            if (boot && (((*((LONG *)Buff) & 0xFFF0)                /* Is this a recognizible boot disk?    */
                                == (ID_DOS_DISK&0xFFF0))) || (*((LONG *)Buff)
                                == 'BOOT')) {

                                if (ChkSumFD(Buff)) boot = 0;                       /* Bad checksum.  Don't boot            */
                                }

                            else boot = 0;
                            }

                        else boot = 0;

                        CloseDevice(&V->SReq);
                        }
                    }

                if (boot) return(V->BootPri = ln->bn_Node.ln_Pri);                  /* Is the disk bootable?                */
                }

            ln = (struct BootNode *)ln->bn_Node.ln_Succ;                            /* Next node                            */
            }
        }

    return(-32768);                                                                 /* No boot device present               */
    }



/***************************************************************************
 *                                                                         *
 *  StripMemory - Remove all but CHIP memory                               *
 *                                                                         *
 ***************************************************************************/

void StripMemory(struct V *V) {

#define NoFastMemBase V->NoFastMemBase

    if (NoFastMemBase = OpenResource("nofastmem.resource")) RemoveFastMem();        /* Remove fast RAM from system          */
    }



/***************************************************************************
 *                                                                         *
 *  StripDevices - Remove unused devices (trackdisk and/or ide)            *
 *                                                                         *
 ***************************************************************************/

void StripDevices(struct V *V) {

struct BootNode          *ln;
struct DeviceNode        *dn;
struct FileSysStartupMsg *fssm;
char                     *devname;
LONG                      devunit;
UBYTE                     Buff[BUFF_SIZE];
int                       boot;

    Forbid();                                                                       /* Let's get started                    */
    ln = (struct BootNode *)ExpansionBase->MountList.lh_Head;

    if (ln->bn_Node.ln_Succ) {                                                      /* Anything on the list?                */

         while (ln) {                                                               /* Traverse whole device list           */

            if (dn = (struct DeviceNode *)                                          /* Get device node pointer              */
                ((struct BootNode *)ln->bn_DeviceNode)) {

                if (fssm = BADDR(dn->dn_Startup)) {                                 /* Get file system startup message      */

                    devname = (char *)BADDR(fssm->fssm_Device)+1;                   /* Get FSSM info                        */
                    devunit = fssm->fssm_Unit;

                    if (Compare((ULONG *)devname,                                   /* Remove all references to trackdisk   */
                                (ULONG *)"trackdisk.device", 16)) {

                        boot = 0;

                        if (!OpenDevice(devname, devunit, &V->SReq, 0)) {           /* opened it                            */

                            DoIOR(&V->SReq, CMD_CLEAR, 0, 0, 0, V);                 /* Clear cache!                         */

                            if (!DoIOR(&V->SReq, CMD_READ, 0, BUFF_SIZE,            /* Read boot information                */
                               (UWORD *)Buff, V))
                               boot = 1;

                            DoIOR(&V->SReq, TD_MOTOR, 0, 0, 0, V);                  /* Motor off                            */

                            CloseDevice(&V->SReq);
                            }

                        if (!boot) Remove((struct Node *)ln);                       /* Remove the device                    */
                        }

                    else if (Compare((ULONG *)devname,                              /* Remove all references to scsi.device */
                                     (ULONG *)"scsi.device", 11)) {

                        Remove((struct Node *)ln);                                  /* Remove the device                    */
                        }
                    }
                }

            ln = (struct BootNode *)ln->bn_Node.ln_Succ;                            /* Next node                            */
            }
        }

    Permit();                                                                       /* All done                             */
    }





/***************************************************************************
 *                                                                         *
 *  CenterScreen - Center the screen                                       *
 *                                                                         *
 ***************************************************************************/

#define TV_VIEW_X   120
#define ORIG_VIEW_X 129
#define ORIG_VIEW_Y 44

void CenterScreen(struct V *V) {

struct IOverscanPrefs ios;
struct DimensionInfo  diminfo;
ULONG  ModeID;

    if (GfxBase->DisplayFlags & PAL) ModeID = PAL_MONITOR_ID  | HIRES_KEY;          /* Read current dimensions of MODE_ID   */
    else                             ModeID = NTSC_MONITOR_ID | HIRES_KEY;

    if (GetDisplayInfoData(NULL, (UBYTE *)&diminfo,                                 /* Get display information              */
            sizeof(struct DimensionInfo), DTAG_DIMS, ModeID)) {

        ios.ios_ViewPos.x = TV_VIEW_X;                                              /* Center the X coordinate              */
        ios.ios_ViewPos.y = ORIG_VIEW_Y;

        ios.ios_DisplayID = ModeID;                                                 /* Proper mode                          */

        ios.ios_Text.x    = diminfo.TxtOScan.MaxX - diminfo.TxtOScan.MinX + 1;      /* Leave all this unchanged             */
        ios.ios_Text.y    = diminfo.TxtOScan.MaxY - diminfo.TxtOScan.MinY + 1;
        ios.ios_Standard  = diminfo.StdOScan;

        SetIPrefs(&ios, sizeof(struct IOverscanPrefs), IP_OVERSCAN);                /* Center the screen                    */
        }
    }




/*******************************************************************************
 *                                                                             *
 * SendAnimMessage - Send a message to animation task                          *
 *                                                                             *
 *******************************************************************************/

void SendAnimMessage(int Msg) {

struct MsgPort *AnimPort;
struct MsgPort *ReplyPort;
struct Message *ShutdownMsg;

    if (AnimPort = FindPort("Startup Animation")) {                             /* See if animation is running */

        if (ReplyPort = CreateMsgPort()) {                                      /* Create a message reply port */

            if (ShutdownMsg = (struct Message *)                                /* Allocate a message to send  */
                AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR)) {

                ShutdownMsg->mn_Length    = Msg;                                /* Send this message to anim   */
                ShutdownMsg->mn_ReplyPort = ReplyPort;
                PutMsg(AnimPort, ShutdownMsg);

                WaitPort(ReplyPort);                                            /* Wait for message to return  */
                while(GetMsg(ReplyPort));

                FreeMem(ShutdownMsg, sizeof(struct Message));                   /* Free the message            */
                }

            DeleteMsgPort(ReplyPort);                                           /* Delete MsgPort              */
            }
        }
    }



/*******************************************************************************
 *                                                                             *
 * RemoveReqZapper - Turn on gurus and task held requesters.                   *
 *                                                                             *
 *******************************************************************************/

void RemoveReqZapper(struct V *V) {

struct Resident *res;

    Forbid();

    if (res = FindResident("reqzapper")) {                                      /* Find request zapper         */

        UnlinkRomTag(res);                                                      /* Remove request zapper       */

        V->ExecBase->KickCheckSum = (APTR)SumKickData();                        /* Rechecksum ROMTag stuff     */
        }

    Permit();
    }



/*******************************************************************************
 *                                                                             *
 * OpenCDUI - Start up CDUI library                                            *
 *                                                                             *
 *******************************************************************************/

ULONG OpenCDUI(struct V *V) {

        StripMemory(V);                                                         /* Remove all but CHIP memory   */

#undef  CDUIBase                                                                /* Open CDUI                    */
    if (V->CDUIBase = OpenLibrary("cdui.library", 0)) {
#define CDUIBase        V->CDUIBase

        StartCDUI();                                                            /* Start CDUI                   */
        V->Animating = 1;
        }

    return((ULONG)CDUIBase);                                                    /* Return result                */
    }
