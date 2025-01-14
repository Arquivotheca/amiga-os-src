/*  :ts=8 bk=0
 *
 * clicker.c:   Key click routines.
 *
 * Leo L. Schwab                                        9102.05
 ***************************************************************************
 *      This information is CONFIDENTIAL and PROPRIETARY                   *
 *      Copyright 1991, Silent Software Incorporated.                      *
 *      All Rights Reserved.                                               *
 ***************************************************************************
 */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/errors.h>

#include <devices/audio.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfxbase.h>

#include <cdtv/cdtvprefs.h>

#include "/playerprefsbase.h"
#include "/playerprefs_pragmas.h"
#include "/playerprefs_protos.h"

#include "keyclick.h"

#include <clib/exec_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>


#define MIN_PERIOD_NTSC 124
#define MIN_PERIOD_PAL  123
#define DEFAULT_LEN     16
#define DEFAULT_FREQ    16000
#define DEFAULT_NCYCLES 20


#define SysBase         (PlayerPrefsBase->ppb_ExecBase)
#define GfxBase         (PlayerPrefsBase->ppb_GfxBase)
#define DeBoxBase       (PlayerPrefsBase->ppb_DeBoxBase)


extern struct IOStdReq * installhandler(struct PlayerPrefsBase *, struct Interrupt *);

extern void removehandler(struct PlayerPrefsBase *, struct IOStdReq *, struct Interrupt *);


LONG __asm __saveds LIBInstallKeyClick(register __a6 struct PlayerPrefsBase *);

LONG __asm __saveds LIBKeyClickCommand (register __d0 ULONG, register __a0 void *, register __d1 LONG, register __d2 LONG,
                                        register __d3 LONG, register __d4 LONG, register __a6 struct PlayerPrefsBase *);

static void __saveds KeyClick(void);
static int beep(struct ClickGlobals *);
static void shutup(struct ClickGlobals *);
static void getreplies(struct ClickGlobals *);
static void setperiod (struct ClickGlobals *, LONG);
static int allocchan(struct ClickGlobals *);
static void freechan(struct ClickGlobals *);
static void cleanupsound(struct ClickGlobals *);
static int openstuff(struct ClickGlobals *);
static void closestuff(struct ClickGlobals *);

static struct InputEvent * __asm inputhandler(register __a0 struct InputEvent *, register __a1 struct ihd *);


/*
 * Data structure for input handler.
 */

struct ihd {

    struct Task    *task;
    LONG            sig;
    BYTE            norepeat, lastunit;
    };


struct ClickGlobals {

    struct ihd              ihdata;
    struct IOAudio          aud[4], audalloc;
    struct Interrupt        handlerstuff;
    struct PlayerPrefsBase *PPBase;
    struct IOStdReq        *idreq;
    struct MsgPort         *replys[4], *areply;
    LONG                    clock, length;
    UWORD                   period, minperiod, cycles;
    void                   *sample, *defaultsample;
    BYTE                    volume, keysigbit;
    BYTE                    alloced, posted, joybeep;
    };


extern char __far _LibName[];


/*
 * Sinwave to be fed directly to the audio.device.
 * Generated with (slim) help from AmigaBASIC.
 */


static BYTE __far sinewave[] = {

    0,  48,  90,  117,  127,  117,  90,  48,            /* Sinwave (mouse movement keyclick        */
    0, -49, -91, -118, -128, -118, -91, -49,

     127,  127,  127,  127,  127,  127,  127,  127,     /* squarewave (joystick movement keyclick) */
    -127, -127, -127, -127, -127, -127, -127, -127
    };



static UBYTE __far allocmap[] = {

    0xc,    /*  Left-Right  */
    0xa,    /*  Left-Right  */
    0x3,    /*  Left-Right  */
    0x5,    /*  Left-Right  */
    0x8,    /*  Left        */
    0x4,    /*  Right       */
    0x2,    /*  Right       */
    0x1     /*  Left        */
    };

char    __far clickname[] = CLICKPORTNAME;


/****** playerprefs.library/InstallKeyClick *******************************
*
*   NAME
*       InstallKeyClick -- Install system key click module.
*
*   SYNOPSIS
*       success = InstallKeyClick ();
*
*       LONG success;
*
*   FUNCTION
*       This routine installs a systemwide key click module.  When the user
*       presses a key down, an audible tone will be generated.
*
*       This call does NOT nest.
*
*   INPUTS
*
*   RESULT
*       success - non-zero if key click module was successfully installed,
*                 or is already running.
*
*   EXAMPLE
*
*   NOTES
*       This routine installs an input handler which sends a signal to the
*       key click task for every IECLASS_RAWKEY event generated with
*       IECODE_UP_PREFIX clear, and for every IECLASS_RAWMOUSE event with
*       codes equal to IECODE_LBUTTON, IECODE_RBUTTON, and IECODE_MBUTTON.
*
*       The key click task runs at priority 20, the same as the input.device.
*
*       The task allocates the sound channels through the audio.device at
*       priority -100, and does so only when it's actually going to make a
*       click.  The channel is freed immediately upon the conclusion of the
*       click sample.  This makes it very unobtrusive.  If it can't allocate
*       any sound channel, no click is produced; we don't want to muck up
*       game sound effects...
*
*       The default startup conditions are:  key clicking enabled, repeat
*       clicking enabled, full peak sinewave sampled at 16000 Hz, volume set
*       at 16, joybeep enabled.
*
*   BUGS
*       As with InstallScreenSaver(), there's no way to know if the task's
*       initialization procedures failed.
*
*   SEE ALSO
*       KeyClickCommand(), cdtv/keyclick.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBInstallKeyClick (register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

    if (FindTask (clickname)) return (1);

    return (CreateTask (clickname, 20, (APTR) KeyClick, 4096) != NULL);
    }


/****** playerprefs.library/KeyClickCommand *******************************
*
*   NAME
*       KeyClickCommand -- Send command to key click system.
*
*   SYNOPSIS
*       error = KeyClickCommand (cmd, sample, length, rate, ncycles, volume);
*
*       LONG    error;
*       LONG    cmd;
*       void    *sample;
*       LONG    length, rate, ncycles, volume;
*
*   FUNCTION
*       This routine sends a command to the key click system.  This routine
*       is synchronous, returning after the command has completed.  The
*       command is passed in the parameter 'cmd', and can have the following
*       values:
*
*       CLKCMD_DIE:
*               Terminates the key click system.  All resources are freed,
*               and the controlling task exits.  All other parameters are
*               unused.
*
*               This call DOES NOT NEST!  This means that, no matter how
*               many times you called InstallKeyClick(), when you perform
*               this operation, the task dies immediately.
*
*       CLKCMD_DISABLE:
*               Disables/enables the key click system without actually
*               terminating/restarting it.  If 'length' is set to non-zero,
*               clicking is disabled; if it is set to zero, clicking is
*               enabled (think of it as a "disabled" flag).  All other
*               parameters are unused.
*
*       CLKCMD_SETREPEAT:
*               Disables/enables key clicks for repeated keys (RAWKEY events
*               with IEQUALIFIER_REPEAT set).  If the 'length' parameter is
*               non-zero, clicks for repeated keys are enabled; if it's
*               zero, they're disabled.  All other parameters are unused.
*
*       CLKCMD_SETJOYBEEP:
*               Whenever a joystick event comes down the input food chain,
*               the key click system can recognize this and substitute a
*               completely different sound for all subsequent events.  This
*               new sound will continue to be generated until a mouse or
*               keyboard event is seen in the chain.  This special "joystick
*               mode" sound is entirely internal and unchangeable.
*
*               This operation enables/disables this feature.  If the
*               'length' parameter is non-zero, the feature is enabled; if
*               it's zero, it's disabled.  All other parameters are unused.
*
*               Since joystick events are usually never passed down the
*               input food chain, leaving this feature enabled will usually
*               never affect anything.
*
*               This feature brought to you by the West Chester Institute
*               For The Politically Insane.
*
*       CLKCMD_SETVOLUME:
*               Sets the volume level of the currently active key click
*               sound sample.  This applies to the default sound sample,
*               too.  The 'volume' parameter sets the new volume.  All other
*               parameters are unused.
*
*       CLKCMD_SETRATE:
*               Set the playback sampling rate of the currently active
*               key click sound sample.  This applies to the default sound
*               sample, too.  This is useful for changing the pitch of the
*               sound without suffering through CLKCMD_NEWSOUND.  The 'rate'
*               parameter sets the new rate, in samples per second.  All
*               other parameters are unused.
*
*       CLKCMD_CLICK:
*               Produces a key click right now.  This operation observes the
*               current CLKCMD_DISABLE setting.  All other parameters are
*               unused.
*
*       CLKCMD_NEWSOUND:
*               Installs a new sound sample to be played whenever a key is
*               pressed.
*
*               'sample' points to a sound sample, which must be located in
*               CHIP RAM.  'length' is the length of the sample in bytes.
*               'rate' is the rate at which the sample is to be played, in
*               samples per second.  'ncycles' is the number of times to
*               play the sample each time a key is pressed (e.g. if you
*               supply a single cycle of a sine wave, you may wish to set
*               ncycles to 20 so that it will play enough times to be
*               heard).
*
*               After invoking this command, the memory containing the sound
*               sample is owned by the key click system.  It will be freed
*               when either a CLKCMD_DIE or CLKCMD_NEWSOUND command is
*               received.
*
*               If 'sample' is NULL, the current client-installed sound
*               sample is freed and the internal default sample and volume
*               level are restored.
*
*               If the sample rate supplied is too fast for the hardware to
*               support, it will play the sample as fast as possible.
*
*       CLKCMD_ISDEFAULT:
*               This operation tells you if the currently active key click
*               sound sample is the default one.  If it is the default,
*               a positive non-zero value is returned.  If the sound sample
*               has been changed from the default (e.g. with
*               CLKCMD_SETSOUND), zero is returned.  All other parameters
*               are unused.
*
*       All unused parameters should be set to zero for upward
*       compatibility.
*
*   INPUTS
*       cmd     - command to be sent
*       sample  - pointer to sound sample, located in CHIP RAM
*       length  - multiple use; see above
*       rate    - rate to play sample, in samples per second
*       ncycles - number of times to play sample
*       volume  - volume level; valid range 0 - 64;
*
*   RESULT
*       error   - zero if all went well, or a negative error code.
*
*   EXAMPLE
*
*   NOTES
*       If the key click system cannot be located or does not exist, the
*       routine returns IOERR_OPENFAIL in all cases.  If the command code
*       passed is invalid, the error IOERR_NOCMD is returned.
*
*       This routine calls FindPort() protected by Forbid().  If you're
*       planning on calling this routine in a tight loop, the overhead may
*       kill you, and you should consider doing all the message passing
*       yourself (it ain't hard).  (Be aware that the other tasks can kill
*       the facility at any time.)
*
*       A joystick event is considered to be any IECLASS_RAWMOUSE event
*       whose ie_SubClass field is non-zero.
*
*   BUGS
*       This entire facility, like the screen saver, is overdesigned.
*       I mean, look at this autodoc.  Did you ever think you'd see such a
*       long and detailed autodoc page for a *key click routine??*
*
*       The non-nesting behavior of InstallKeyClick and CLKCMD_DIE was,
*       in retrospect, probably a bad move.
*
*   SEE ALSO
*       InstallKeyClick(), cdtv/keyclick.h, exec/errors.h
*
*****************************************************************************
*/

LONG __asm __saveds LIBKeyClickCommand(register __d0 ULONG command, register __a0 void *sample,
                                       register __d1 LONG length,   register __d2 LONG samplerate,
                                       register __d3 LONG ncycles,  register __d4 LONG volume,
                                       register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

struct MsgPort  *rport, *fport;
struct IOStdReq msg;

    Forbid();

    if ((fport = FindPort (clickname))) {

        if (rport = (struct MsgPort *)CreatePort (NULL, NULL)) {

            msg.io_Message.mn_ReplyPort     = rport;
            msg.io_Message.mn_Node.ln_Type  = NT_MESSAGE;
            msg.io_Command                  = command;
            msg.io_Data                     = sample;
            msg.io_Length                   = length;
            msg.io_Actual                   = samplerate;
            msg.io_Offset                   = ncycles;
            msg.io_Flags                    = volume;

            PutMsg (fport, &msg);

            Permit();
            WaitPort (rport);

            DeletePort (rport);
            return ((LONG) msg.io_Error);
            }
        }

    Permit();
    return (IOERR_OPENFAIL);
    }


#define PlayerPrefsBase (G->PPBase)

static void __saveds KeyClick() {

register struct IOStdReq *msg;
register LONG             sigwait, sigwake;
struct ClickGlobals      *G, g;
struct MsgPort           *cmdport;
LONG                      sigkey, sigaud, sigcmd;
BYTE                      disabled;
void                     *PPB;
struct CDTVPrefs          cdtvprefs;

    #undef  SysBase
    #define SysBase (* (struct Library **)4)

    G = &g;
    if (!(g.PPBase = PPB = OpenLibrary (_LibName, 0))) return;

    #undef  SysBase
    #define SysBase (PlayerPrefsBase->ppb_ExecBase)

    MemSet (&g, 0, sizeof (g));

    g.PPBase                        = PPB;                              /*  MemSet killed this.  */
    g.handlerstuff.is_Node.ln_Type  = NT_INTERRUPT;
    g.handlerstuff.is_Node.ln_Pri   = 70;
    g.handlerstuff.is_Node.ln_Name  = clickname;
    g.handlerstuff.is_Data          = (APTR) &g.ihdata;
    g.handlerstuff.is_Code          = (VOID (*)())inputhandler;
    g.keysigbit                     = -1;
    g.joybeep                       = 1;

    if (!(cmdport = (struct MsgPort *)CreatePort (clickname, 0))) goto xit;

    if (!(sigaud = openstuff (G))) goto xit;

    FillCDTVPrefs(&cdtvprefs);

    disabled = !(cdtvprefs.Flags & CDTVPF_KEYCLICK);
    sigkey   = 1L << g.keysigbit;
    sigcmd   = 1L << cmdport->mp_SigBit;
    sigwait  = sigkey | sigcmd | sigaud;

    while (1) {
        sigwake = Wait(sigwait);

        if (sigwake & sigkey && !disabled) if (!beep (G)) cleanupsound (G);

        if (sigwake & sigaud) {

            getreplies (G);
            freechan (G);
            }

        if (sigwake & sigcmd) {

            while (msg = (struct IOStdReq *)GetMsg (cmdport)) {

                msg->io_Error = 0;
                switch (msg->io_Command) {

                    case CLKCMD_DIE: goto xit;

                    case CLKCMD_DISABLE:
                        disabled = msg->io_Length;
                        break;

                    case CLKCMD_SETREPEAT:
                        g.ihdata.norepeat = !msg->io_Length;
                        break;

                    case CLKCMD_SETJOYBEEP:
                        g.joybeep = msg->io_Length;
                        break;

                    case CLKCMD_SETVOLUME:
                        g.volume = msg->io_Flags;
                        break;

                    case CLKCMD_SETRATE:
                        setperiod (G, msg->io_Actual);
                        break;

                    case CLKCMD_CLICK:
                        if (!disabled) if (!beep (G)) cleanupsound (G);
                        break;

                    case CLKCMD_NEWSOUND:
                        cleanupsound (G);

                        if (g.sample != g.defaultsample) FreeMem (g.sample, g.length);

                        if (msg->io_Data) {

                            g.sample = msg->io_Data;                                                /* Set new sample */
                            g.length = msg->io_Length;
                            g.cycles = msg->io_Offset;
                            g.volume = msg->io_Flags;
                            setperiod (G, msg->io_Actual);
                            }

                         else {

                            g.sample = g.defaultsample;                                             /* Restore default sample */
                            g.length = DEFAULT_LEN;
                            g.cycles = DEFAULT_NCYCLES;
                            g.volume = 16;
                            setperiod (G, DEFAULT_FREQ);
                            }

                        break;

                    case CLKCMD_ISDEFAULT:
                        /*
                         * This is the only case where io_Error returns something other
                         * than a true error code.  To have preserved philosophical
                         * correctness would have meant re-designing the client interface,
                         * which I didn't particularly want to do.
                         */
                        msg->io_Error =  g.sample == g.defaultsample;
                        break;

                    default:
                        msg->io_Error = IOERR_NOCMD;
                        break;
                    }

                ReplyMsg (msg);
                }
            }
        }

xit:

    Forbid();

    closestuff (G);

    if (cmdport) {
        if (msg) ReplyMsg(msg);                                                                     /* Reply the message that killed us. */
                
        while (msg = (struct IOStdReq *)GetMsg(cmdport)) ReplyMsg (msg);

        DeletePort (cmdport);
        }

    if (g.PPBase)CloseLibrary (g.PPBase);
    }



static beep(register struct ClickGlobals *G) {

register short i, f;

    if (!G->alloced) if (!allocchan(G)) return (0);

    f = (LONG) G->audalloc.ioa_Request.io_Unit;

    if (G->posted) shutup (G);                                                                      /* Probably should stick some kind of delay here. */

    G->audalloc.ioa_Request.io_Command = CMD_STOP;                                                  /* Stop channels from doing anything just yet */
    if (DoIO (&G->audalloc)) return (0);

    for (i=4; i--;) if (f & (1 << i)) BeginIO (&G->aud[i]);                                         /* Queue up writes */
    G->posted = 1;

    G->audalloc.ioa_Request.io_Command = CMD_START;                                                 /* Start playing channels */
    return (!DoIO (&G->audalloc));
    }



static void shutup(register struct ClickGlobals *G) {

    G->audalloc.ioa_Request.io_Command = ADCMD_FINISH;                                          /* Stop the noise */
    DoIO (&G->audalloc);

    G->audalloc.ioa_Request.io_Command = CMD_RESET;                                             /* Make sure they've stopped */
    DoIO (&G->audalloc);
    getreplies (G);
    }



static void getreplies(register struct ClickGlobals *G) {

register short  i, f;

    f = (LONG) G->audalloc.ioa_Request.io_Unit;

    for (i=4; i--;) if (f&(1<<i) && G->posted) {

        WaitPort(G->replys[i]);                                     /* Yes we are using this channel.  Wait for a reply on the port */
        GetMsg(G->replys[i]);
        SetSignal(0, 1L << G->replys[i]->mp_SigBit);
        G->aud[i].ioa_Request.io_Error = 0;
        }

    G->posted = 0;
    }



static void setperiod (register struct ClickGlobals *G, LONG freq) {

    if ((G->period = G->clock/freq) < G->minperiod) G->period = G->minperiod;
    }


static allocchan(register struct ClickGlobals *G) {

register short  i;

    G->audalloc.ioa_Request.io_Message.mn_Node.ln_Pri = -100;
    G->audalloc.ioa_Request.io_Command                = ADCMD_ALLOCATE;
    G->audalloc.ioa_Request.io_Flags                  = ADIOF_NOWAIT;
    G->audalloc.ioa_Data                              = allocmap;
    G->audalloc.ioa_Length                            = sizeof (allocmap);
    G->audalloc.ioa_AllocKey                          = 0;

    if (DoIO (&G->audalloc)) return(0);                                             /* Didn't get it */

//    n = G->ihdata.lastunit;

    for (i=4; i--;) {

        G->aud[i].ioa_AllocKey = G->audalloc.ioa_AllocKey;
#if 0
        if (n && G->joybeep) {

            G->aud[i].ioa_Data      = (UBYTE *)G->defaultsample + 16;
            G->aud[i].ioa_Length    = 16;
            G->aud[i].ioa_Cycles    = 10;
            G->aud[i].ioa_Period    = 1110; /* About 3200 Hz */
            G->aud[i].ioa_Volume    = 16;
            }

         else {
#endif
            G->aud[i].ioa_Data      = G->sample;
            G->aud[i].ioa_Length    = G->length;
            G->aud[i].ioa_Cycles    = G->cycles;
            G->aud[i].ioa_Period    = G->period;
            G->aud[i].ioa_Volume    = G->volume;
//          }
        }

    G->alloced = 1;
    return (1);
    }

static void freechan(register struct ClickGlobals *G) {

    G->audalloc.ioa_Request.io_Command = ADCMD_FREE;
    DoIO (&G->audalloc);
    G->alloced = 0;
    }

static void cleanupsound(register struct ClickGlobals *G) {

    shutup (G);
    freechan (G);
    }


static openstuff(register struct ClickGlobals *G) {

register short  i;
register LONG   sigaud = 0;

    if (!(G->idreq = installhandler (PlayerPrefsBase, &G->handlerstuff))) return (0);

    if ((G->keysigbit = AllocSignal (-1)) == -1) return (0);

    if (!(G->defaultsample = AllocMem (sizeof (sinewave), MEMF_CHIP))) return (0);

    if (!(G->areply = (struct MsgPort *)CreatePort (0, 0))) return (0);

    G->audalloc.ioa_Request.io_Message.mn_ReplyPort = G->areply;

    if (OpenDevice ("audio.device", 0, &G->audalloc, 0)) {

        G->audalloc.ioa_Request.io_Device = NULL;
        return (0);
        }

    for (i = 4;  i--; ) {

        if (!(G->replys[i] = (struct MsgPort *)CreatePort (0, 0))) return (0);

        sigaud |= 1L << G->replys[i]->mp_SigBit;

        CopyMem (&G->audalloc, &G->aud[i], sizeof (G->audalloc));

        G->aud[i].ioa_Request.io_Message.mn_ReplyPort = G->replys[i];
        G->aud[i].ioa_Request.io_Unit                 = (void *) (1 << i);
        G->aud[i].ioa_Request.io_Command              = CMD_WRITE;
        G->aud[i].ioa_Request.io_Flags                = ADIOF_PERVOL;
        }

    if (GfxBase->DisplayFlags & PAL) {

        G->clock     = 3546895;
        G->minperiod = MIN_PERIOD_PAL;
        }

    else {

        G->clock     = 3579545;
        G->minperiod = MIN_PERIOD_NTSC;
        }

    G->ihdata.task = FindTask (NULL);
    G->ihdata.sig  = 1L << G->keysigbit;

    CopyMem (sinewave, G->defaultsample, sizeof (sinewave));

    G->sample       = G->defaultsample;
    G->length       = DEFAULT_LEN;
    G->cycles       = DEFAULT_NCYCLES;
    G->volume       = 16;
    setperiod (G, DEFAULT_FREQ);

    return (sigaud);
    }



static void closestuff(register struct ClickGlobals *G) {

register short  i;

    cleanupsound (G);

    for (i=4; i--;) if (G->replys[i]) {

        DeletePort (G->replys[i]);
        G->replys[i] = NULL;
        }

    if (G->audalloc.ioa_Request.io_Device) {

        CloseDevice (&G->audalloc);
        G->audalloc.ioa_Request.io_Device = NULL;
        }

    removehandler(PlayerPrefsBase, G->idreq, &G->handlerstuff);

    if (G->sample && G->sample != G->defaultsample) FreeMem (G->sample, G->length);
    if (G->defaultsample)                           FreeMem (G->defaultsample, sizeof (sinewave));
    if (G->keysigbit >= 0)                          FreeSignal (G->keysigbit);
    if (G->areply)                                  DeletePort (G->areply);
    }



/***************************************************************************
 * Input handler.  If it's a RAWKEY, we send a signal to the parent task.
 * The #pragma for _Signal is required because SysBase has no scope in here,
 * even with the funny #defines.
 */


#pragma syscall _Signal 144 902

static struct InputEvent * __asm inputhandler(register __a0 struct InputEvent *ie, register __a1 struct ihd *idat) {

register struct InputEvent *save = ie;
register UWORD              code;
register UBYTE              class;

    while (ie) {

        class = ie->ie_Class;
        code = ie->ie_Code;

        if (class == IECLASS_RAWMOUSE) {

            /*
             * Through experimentation, we discovered that,
             * for IECLASS_RAWMOUSE, the gameport unit from which
             * the event came is stored in ie_SubClass.
             * Therefore, by observing changes in ie_SubClass,
             * we can (sorta) determine when the user has
             * switched from mouse mode to joystick mode and
             * back.  It is here where we track these changes.
             */
            idat->lastunit = ie->ie_SubClass;

            if (code == IECODE_LBUTTON || code == IECODE_RBUTTON || code == IECODE_MBUTTON) goto upcheck;
            }

        if (class == IECLASS_RAWKEY) {

            /*
             * Always do nice beep for keyboard events.
             */
            idat->lastunit = 0;

upcheck:    if (!(idat->norepeat && ie->ie_Qualifier & IEQUALIFIER_REPEAT) && !(code & IECODE_UP_PREFIX)) 
                _Signal (idat->task, idat->sig);
            }

        ie = ie->ie_NextEvent;
        }

    return (save);
    }


