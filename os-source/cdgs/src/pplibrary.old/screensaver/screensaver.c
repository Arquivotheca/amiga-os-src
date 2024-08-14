
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>

#include <cdtv/debox.h>
#include <cdtv/cdtvprefs.h>

#include "screensaver.h"
#include "splinesaver.h"

#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"
#include "/basicio/viewmgr.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>

extern struct IOStdReq *installhandler(struct PlayerPrefsBase *, struct Interrupt *);
extern void removehandler(struct PlayerPrefsBase *, struct IOStdReq *, struct Interrupt *);

/* THE ALTLOADVIEW */

int __asm                getloadview(register __a0 void *);
struct LoadView ** __asm TrapLoadView(register __a1 struct GfxBase *);
VOID    __asm            UntrapLoadView(register __a0 struct View **);
VOID    __asm            OrgLoadView(register __a0 struct View **, register __a1 struct View *);
struct BitMap           *openCDTVRotateBm(struct PlayerPrefsBase *);
VOID                     closeCDTVRotateBm(struct PlayerPrefsBase *);
int __asm                rndwseed(register __d0 WORD,register __a0 ULONG *);

extern UWORD volatile __far vhposr;     /*  Custom chip register.  */

void __saveds SetUpBlanker(struct SaverGlobals *G);

extern struct CycleIntInfo  intdata;
struct AltLV
{
struct View *alv_View;
APTR    alv_OldFunc;
struct Library *alv_GfxBase;
};


/*
 * Spoof the #pragmas
 */
#define SysBase     (PlayerPrefsBase->ppb_ExecBase)
#define DeBoxBase   (PlayerPrefsBase->ppb_DeBoxBase)
#define GfxBase     (PlayerPrefsBase->ppb_GfxBase)

#define ROTWIDE     172
#define ROTHIGH     51
#define MINVPX      20
#define MAXVPX      127 /* (320 - ROTWIDE)  1.3 can't take this  */
#define MAXVPY      (210 - ROTHIGH)
/*------------------------------------------------------------------*/

/*
 * Local forward fucntion declarations.  (I hate ANSI.)
 */
LONG __asm __saveds
LIBInstallScreenSaver(register __a6 struct PlayerPrefsBase *);
LONG __asm __saveds
LIBScreenSaverCommand(register __d0 ULONG,
              register __a6 struct PlayerPrefsBase *);
static void __saveds __asm ScreenSave(void);

static int __saveds opensaver(struct SaverGlobals *);
static void closesaver(struct SaverGlobals *);
static int openstuff(struct SaverGlobals *);
static void closestuff(struct SaverGlobals *);
static struct InputEvent * __asm
inputhandler(register __a0 struct InputEvent *,
         register __a1 struct ihd *);

void DoSaverAnim(struct SaverGlobals *);


extern struct Custom    __far custom;
extern char __far _LibName[];



char __far  scrsav[] = SAVEPORTNAME;

/****** playerprefs.library/InstallScreenSaver ****************************
*
*   NAME
*   InstallScreenSaver -- Installs a systemwide screen saver facility.
*
*   SYNOPSIS
*   success = InstallScreenSaver ();
*
*   LONG success;
*
*   FUNCTION
*   This routine installs a screen saver.  After a time of inactivity,
*   the prevailing display is replaced with the CDTV logo moving around
*   on the screen.  This is to keep static images from burning into the
*   user's display.
*
*   This call does NOT nest.
*
*   INPUTS
*
*   RESULT
*   success - non-zero if the screen saver task was successfully
*         started, or is already running.
*
*   EXAMPLE
*
*   NOTES
*   The screen saver is implemented as an autonomous task and input
*   handler.  The input handler monitors input events.  If no meaningful
*   input event occurs after a preset time, a signal is sent to the
*   task which brings up the saver display.
*
*   The input event that takes down the saver display is eaten.
*   You should bear this in mind if you intend to permit the saver to
*   come up while your controls are still "active."
*
*   This screen saver is suitable for programs using the input
*   device.  Programs that go direct to the low level resources
*   should use this blanker in manual mode...by sending blank and
*       unblank commands as needed.
*
*   This facility SetFunction()s LoadView(), thus preventing other Views
*   from being brought to the front while the saver is active.  When the
*   saver takes down its animation, the last View that was set with
*   LoadView() will be loaded and LoadView() will be restored to normal
*   operation.
*
*   The screen saver's resource needs, when quiescent, are very modest;
*   roughly 4K of stack, and less than 1K of dynamically allocated
*   structures.  (It gets *real* big when it kicks in, though...)
*
*   BUGS
*   If the screen saver task fails to start for whatever reason, there's
*   no way to find this out, short of noticing that the task is no
*   longer present.
*
*   SEE ALSO
*   ScreenSaverCommand(), cdtv/screensaver.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBInstallScreenSaver(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    if (FindTask (scrsav)) return (1);

    return (CreateTask (scrsav, 0, (APTR) ScreenSave, 4096) != NULL);
    }

/****** playerprefs.library/ScreenSaverCommand ****************************
*
*   NAME
*   ScreenSaverCommand -- Send a command to the screen saver facility.
*
*   SYNOPSIS
*   result = ScreenSaverCommand (cmd);
*
*   LONG    result;
*   ULONG   cmd;
*
*   FUNCTION
*   This routine locates the screen saver's IPC port and sends it the
*   command supplied.  This routine is synchronous; it returns after
*   the command has completed.  The result returned is command-specific.
*
*   The commands are:
*
*   SCRSAV_DIE:
*       Kill off the screen saver facility.  If the saver is active,
*       it will take down the saver animation.  All resources are
*       freed, and the task is terminated.  Result is always non-
*       zero.
*
*       NOTE:  This operation does NOT nest!  When you call it,
*       it happens immediately, no matter how many other clients
*       opened it.
*
*   SCRSAV_SAVE:
*       Bring up the saver animation now.  This operation observes
*       the current _FAKEIT setting (q.v.).  If it successfully
*       brought up the saver, result will be non-zero.  If any of
*       the allocations failed, result will be zero.
*
*   SCRSAV_UNSAVE:
*       Take down the saver screen now.  This operation observes the
*       current _FAKEIT setting (q.v.).  The inactivity timer is
*       restarted.  Result is always non-zero.
*
*   SCRSAV_FAKEIT:
*       Keep track of inactive time, and return active/inactive when
*       interrogated with SCRSAV_ISACTIVE (q.v.), but don't
*       *actually* save the screen.  This is so outside programs can
*       poll the saver (don't busy wait!) to see if they should take
*       down their own screens in a special way before letting the
*       saver do it's thing.  This command also inhibits calls to
*       SCRSAV_SAVE.  Result is always non-zero.
*
*   SCRSAV_UNFAKEIT:
*       Cancel the above feature; make the saver behave normally.
*       This operation will bring up the saver animation immediately
*       if it's supposed to be up right now (except that it isn't
*       because you performed SCRSAV_FAKEIT earlier).  In this case,
*       result will be zero if it failed to bring up the saver
*       animation.  Otherwise, in all other cases, result will be
*       non-zero.
*
*   SCRSAV_ISACTIVE:
*       Asks the saver if it's currently displaying the saver
*       animation.  Result is non-zero if the saver is active.
*
*   If cmd is OR'ed with SCRSAV_TIMECMD, then the lower 15 bits of cmd
*   are interpreted as the amount of inactive time to wait before
*   activating the saver animation.  This time value is expressed in
*   seconds.  A time value of zero is interpreted as infinity.  Result
*   is always non-zero.
*
*   INPUTS
*   cmd - Command to saver facility.
*
*   RESULT
*   result  - Command-dependent; see above.
*
*   EXAMPLE
*
*   NOTES
*   If the screen saver facility has not been installed, or the supplied
*   command value is invalid, result is always zero.
*
*   BUGS
*
*   SEE ALSO
*   InstallScreenSaver(), cdtv/screensaver.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBScreenSaverCommand(register __d0 ULONG command,
                                          register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

struct MsgPort  *rport, *fport;
struct Message  msg;

    if (command == SCRSAV_ISACTIVE) return((LONG)PlayerPrefsBase->ppb_Blanked);

    Forbid();

    if ((fport = FindPort(scrsav))) {

        if (rport = (struct MsgPort *)CreatePort (NULL, NULL)) {

            msg.mn_ReplyPort    = rport;
            msg.mn_Node.ln_Type = NT_MESSAGE;
            msg.mn_Length       = command;

            PutMsg(fport, &msg);

            WaitPort(rport);
            DeletePort(rport);

            Permit();

            return((LONG) msg.mn_Length);
            }
        }

    Permit();

    return(0);
    }




void remakeview (register struct View *v, struct View **altlv, register struct PlayerPrefsBase *PlayerPrefsBase) {

register struct ViewPort   *vp;
struct View                 new;

    CopyMem (v, &new, sizeof(new));                              /* Copy existing View, reconstruct all ViewPorts, and remake private copy of view */
    new.LOFCprList = new.SHFCprList = NULL;

    vp = v->ViewPort;
    while (vp) {

        MakeVPort (&new, vp);
        vp = vp->Next;
        }

    MrgCop (&new);

    {
    register struct cprlist *tmp;                               /* Switch the rug while noone's looking. */

        Disable ();
        tmp = new.LOFCprList;
        new.LOFCprList = v->LOFCprList;
        v->LOFCprList = tmp;

        tmp = new.SHFCprList;
        new.SHFCprList = v->SHFCprList;
        v->SHFCprList = tmp;
        }

    if (altlv) OrgLoadView(altlv, v);                           /* Reload view and wait for it to take effect */
    else LoadView (v);                                          /*   then free up old copper lists.           */

    Enable ();

    WaitTOF ();
    FreeCprList (new.LOFCprList);
    FreeCprList (new.SHFCprList);
    }


/*  Additional spoofing for the internal routines.  */
#define PlayerPrefsBase (G->PPBase)

static VOID __asm __saveds ScreenSave(void) {

register struct Message    *msg = NULL;
register LONG               sigwait, sigwake;
struct SaverGlobals         g, *G;
struct MsgPort             *fport;
LONG                        sigblank, sigmsg, sigunblank;
BYTE                        fakeit = 0;
void                       *PPB;
ULONG                       seed;
UWORD                       retval;

#undef  SysBase
#define SysBase     (*(struct Library **) 4)

    G = &g;                                                     /*  Needed for #pragma spoofers.  */
    if (!(g.PPBase = PPB = OpenLibrary(_LibName, 0))) return;

#undef  SysBase
#define SysBase     (PlayerPrefsBase->ppb_ExecBase)

    if (!(fport = (struct MsgPort *)CreatePort (scrsav, 0))) goto xit;

    /* clear out the globals so exit code works */
    MemSet(&g,0,sizeof(struct SaverGlobals));

    rndwseed(-(vhposr & 0x7FFF), &seed);

    g.PPBase            = PPB;/* The MemSet killed it.*/
    g.blanksigbit = g.unblanksigbit = -1;
    g.handlerstuff.is_Node.ln_Type  = NT_INTERRUPT;
    g.handlerstuff.is_Node.ln_Pri   = 70;
    g.handlerstuff.is_Node.ln_Name  = scrsav;
    g.handlerstuff.is_Data          = (APTR) &g.idata;
    g.handlerstuff.is_Code          = (VOID (*)())inputhandler;
    g.fadecnt                       = rndwseed(18, &seed);
    g.SaverType                     = 1;

    if (!openstuff(G)) goto xit;
    
    sigblank = g.idata.blanksig;
    sigunblank = g.idata.unblanksig;
    sigmsg = 1L << fport->mp_SigBit;
    sigwait = sigblank | sigunblank | sigmsg;

    while (1) {
        if (g.altlv) {

            if (GfxBase->ActiView != g.view) {

                closesaver(G);
                continue;   /*  Loop through again.  */
                }

            DoSaverAnim(G);

            Forbid();
            sigwake = SetSignal(0, 0);
            SetSignal(0, sigwake);
            Permit();
            }

        else sigwake = Wait(sigwait);

        if (sigwake & sigblank && !g.altlv) {

            if (!fakeit) {

                if (g.PPBase->ppb_Blanked = opensaver(G)) blk(G);

                else closesaver(G);
                } 

            else g.PPBase->ppb_Blanked = 1;
            }

        if (sigwake & sigunblank) {

            closesaver(G);
            g.PPBase->ppb_Blanked = 0;
            }


        if (sigwake & sigmsg) {
            while (msg = GetMsg(fport)) {

                retval = 1;

                if (msg->mn_Length & SCRSAV_TIMECMD)
                    g.idata.savedelay = msg->mn_Length & ~SCRSAV_TIMECMD;

                else {
                    switch(msg->mn_Length) {
                    case SCRSAV_DIE:
                        msg->mn_Length = 1;
                        goto xit;

                    case SCRSAV_SAVE:
                        if (fakeit) {
                            g.PPBase->ppb_Blanked = 1;
                            break;
                            }

blankit:                if (!g.altlv) {

                            if (g.PPBase->ppb_Blanked = opensaver(G)) {
                                g.idata.lastsec = 0;
                                blk(G);
                                } 

                            else {
                                closesaver (G);
                                retval = 0;
                                }
                            }

                        break;

                    case SCRSAV_UNSAVE:
                        closesaver(G);
                        g.PPBase->ppb_Blanked = 0;
                        g.idata.lastsec = ~0L;
                        break;

                    case SCRSAV_FAKEIT:
                    case SCRSAV_UNFAKEIT:
                        fakeit = msg->mn_Length == SCRSAV_FAKEIT;
                        if (!fakeit && g.PPBase->ppb_Blanked) goto blankit;   /*  Look up.  */
                        if (fakeit && g.altlv)                closesaver (G);
                        break;

                    case SCRSAV_ISACTIVE:
                        retval = g.PPBase->ppb_Blanked;
                        break;

                    default:
                        retval = 0;
                        break;
                    }
                }

            msg->mn_Length = retval;
            ReplyMsg (msg);
            }
        }
    }
xit:
    closestuff(G);

    Forbid();

    if (fport) {

        if (msg) ReplyMsg(msg);
        while (msg = GetMsg(fport)) ReplyMsg(msg);
        DeletePort(fport);
        }

    g.PPBase->ppb_Blanked = 0;

    if (G->PPBase) CloseLibrary(G->PPBase);

/*    Permit();*/
    }


static __saveds opensaver(register struct SaverGlobals *G) {

    if (!(G->g2 = AllocMem(sizeof(struct SGlobal),MEMF_CLEAR))) return 0;

    SetUpBlanker(G);

    if (!(G->view = (struct View *)CreateView (NULL, G->bm, WIDTH, HEIGHT, NULL))) return 0;

    CenterCDTVView (NULL, G->view, WIDTH, HEIGHT);

    if (!(G->altlv = TrapLoadView(GfxBase))) return(0);

    OrgLoadView(G->altlv, G->view);

    Forbid();
    WaitBOVP(G->view->ViewPort);
    OFF_SPRITE
    Permit();

    return(1);
    }


static void closesaver(register struct SaverGlobals *G) {

    if (G->altlv) {

        UntrapLoadView(G->altlv);
        WaitTOF();
        G->altlv = NULL;
        if (G->sprites) ON_SPRITE
        }

    Forbid();

    if (G->view) {

        DeleteView((struct SuperView *)G->view);
        G->view = NULL;
        }

    if (G->bm) {

        UnSetBlanker(G);
        G->bm = NULL;
        }

    if (G->g2) FreeMem(G->g2, sizeof(struct SGlobal));

    G->g2 = NULL;

    Permit();
    }



static openstuff(struct SaverGlobals *G) {   

struct CDTVPrefs cdtvprefs;
    
    if (((G->blanksigbit = AllocSignal(-1)) == -1) || ((G->unblanksigbit = AllocSignal(-1)) == -1)) return (0);
    if (!(G->idreq = installhandler(PlayerPrefsBase, &G->handlerstuff)))                            return (0);
    
    FillCDTVPrefs(&cdtvprefs);

    G->idata.sigtask    = FindTask (NULL);
    G->idata.lastsec    = ~0L;
    G->idata.savedelay  = cdtvprefs.SaverTime * 60; 
    G->idata.blanksig   = 1L << G->blanksigbit;
    G->idata.unblanksig = 1L << G->unblanksigbit;
    }


static void closestuff(register struct SaverGlobals *G) {

    closesaver(G);
    removehandler(PlayerPrefsBase, G->idreq, &G->handlerstuff);

    if (G->blanksigbit >= 0)   FreeSignal(G->blanksigbit);
    if (G->unblanksigbit >= 0) FreeSignal(G->unblanksigbit);
    }


/***************************************************************************
 * Input handler.  It grabs input events and determines when a set period of
 * inactivity has elapsed.  The #pragma for _Signal is required because
 * SysBase has no scope in here, even with the funny #defines.
 */
#pragma syscall _Signal 144 902

static struct InputEvent * __asm inputhandler(register __a0 struct InputEvent *ie, register __a1 struct ihd *idat) {

register UBYTE      class;
struct InputEvent   *first = ie;

    /*
     * Special case to initialize 'lastsec' coherently.
     */
    if (idat->lastsec == ~0L && ie) if (ie->ie_TimeStamp.tv_secs) idat->lastsec = ie->ie_TimeStamp.tv_secs;

    while (ie) {

        class = ie->ie_Class;

        if (class == IECLASS_NULL  ||  class == IECLASS_TIMER) {

            /*
             * Check if time has expired.
             */
            if (idat->lastsec && idat->savedelay && ie->ie_TimeStamp.tv_secs - idat->lastsec > idat->savedelay) {
                _Signal(idat->sigtask, idat->blanksig);
                idat->lastsec = 0;
                }
            /* record time for second blanker */
            idat->cursec = ie->ie_TimeStamp.tv_secs;
            }

        else {
            /*
             * Update 'lastsec' with current timestamp.
             */
            if (!idat->lastsec) {
                if((class == IECLASS_RAWKEY)) {
                /* maybe eat the event */
                if( (ie->ie_Code != 0x72) &&
                    (ie->ie_Code != 0x73) &&
                    (ie->ie_Code != 0x74) &&
                    (ie->ie_Code != 0x75)) {
                    ie->ie_Class = IECLASS_NULL;

                    /* only unblank on key up */
                    if((ie->ie_Code & IECODE_UP_PREFIX) == 0)
                        continue;

                    }
                }
                else if((class == IECLASS_RAWMOUSE)) {
                /* eat the event */
                ie->ie_Class = IECLASS_NULL;

                /* only unblank on key up */
                if((ie->ie_Code & IECODE_UP_PREFIX) == 0)
                    continue;

                }
                /*
                 * We sent a blank signal, so send the
                 * unblank signal.
                 */
                _Signal(idat->sigtask, idat->unblanksig);
            }
            /*
             * IECLASS_DISK{INSERTED,REMOVED} have a bug where
             * the timestamp is zero.  This will barf things up
             * unless we filter it.
             */
            if (ie->ie_TimeStamp.tv_secs)
                idat->lastsec = ie->ie_TimeStamp.tv_secs;
            else {
                idat->lastsec = ~0L;
            }
        }
        ie = ie->ie_NextEvent;
    }
    return (first);
}


void DoSaverAnim(struct SaverGlobals *G) {

    WaitTOF();

    SetAPen(&G->g2->rastPort,1);
    SetRGB4(G->view->ViewPort, 1, 15, 15, 15);
/*
    Move(&G->g2->rastPort, 0, 0);
    Draw(&G->g2->rastPort, 200, 200);
*/
    }





