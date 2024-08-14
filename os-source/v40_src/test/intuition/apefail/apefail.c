/* APEFail.c
 * 
 * This program setfunctions AttachPalExtra() to always fail
 * and then tries to open a screen.  It then removes the
 * setfunction.  Results are reported along the way.
 * Usage:  [run] apefail [[depth 1-8] [nopatch] | [help]]
 *
 * This is for verification of CDGS CritItem #237...
 *
 * (APEFail == AttachPalExtraFail...)
 * 
 * $VER: APEFail_c 1.0 (17.05.93) by J.W. Lockhart
 * 
 */

/* Includes --------------------------------------------- */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <intuition/screens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <stdlib.h>
#include <string.h>


/* Defines ------------------------------------------ */
#define PROGNAME "APEFail"
#define ERRMSG_LIBNOOPEN "Couldn't open %s V%ld (or better)!\n"
#define TEMPLATE "DEPTH/K/N,HELP/S,NOPATCH/S"
#define APE_OFFSET -834L
#define APE_NOMEM  1L      /* documented "no memory" returncode for AttachPalExtra() */
#define REG(x) register __ ## x


/* Structs ------------------------------ */
struct Opts {
    LONG *depth;
    LONG help;
    LONG nopatch;
};

/* Protos ------------------------------------------ */
static VOID GoodBye(int);
static LONG doInit(VOID);
LONG __asm __saveds newAPE(REG(a0) APTR, REG(a1) APTR);
VOID reportOSErr(ULONG);
extern VOID KPrintF(STRPTR,...);
extern VOID KPutStr(STRPTR);

/* Globals --------------------------------------- */
struct Opts     opts;
struct RDArgs  *rdargs;
struct Library *GfxBase;
struct Library *IntuitionBase;
LONG (*APExtra)();

VOID main(int argc, UBYTE *argv[]) {
    struct Screen *scr;
    ULONG oserr=0UL;
    int rc = RETURN_OK;
    ULONG depth = 2;

    if (!doInit()) {
        GoodBye(RETURN_FAIL);
    }

    if (opts.help) {
        Printf("Template: %s\n", TEMPLATE);
        PutStr("This program setfunctions AttachPalExtra() to always fail,\n");
        PutStr("and then tries to open a screen.  It then removes the\n");
        PutStr("setfunction.  Results are reported along the way.\n");
        PutStr("Usage:  [run] apefail\n");
        PutStr("  options:  Depth - set screen depth (1-8)\n");
        PutStr("            Help  - you're looking at it.\n");
        PutStr("          NoPatch - see if screen opens without patch.\n");
        PutStr("  Note: test is invalid with NoPatch option.\n\n");
        PutStr("(APEFail == AttachPalExtraFail...)\n");                       
        PutStr("\n");
        GoodBye(RETURN_OK);
    }
    if (opts.depth) {
        depth = (ULONG) *opts.depth;
        /* yes, if you're perverse, you can try to open infinitely-deep screens */
    }

    if (!opts.nopatch) {
        Forbid();
        APExtra = (LONG (*)()) SetFunction(GfxBase, APE_OFFSET, newAPE);
        Permit();
        PutStr("Applied patch to AttachPalExtra()...\n");
    }

    PutStr("Trying to open screen...\n");

    scr = OpenScreenTags(NULL,
                         SA_ErrorCode, &oserr,
                         SA_DisplayID, HIRESLACE_KEY,
                         SA_Depth, depth,
                         SA_Title, "You shouldn't be seeing this!",
                         SA_AutoScroll, TRUE,
                         TAG_DONE);
    if (scr) {
        Delay(100L);
        Printf("FAIL.  Opened a screen, addr $%08lx!\n", scr);
        CloseScreen(scr);
        rc = RETURN_FAIL;
    }
    else {
        Printf("PASS.  Couldn't open screen because:\n  ");
        reportOSErr(oserr);
    }

    if (!opts.nopatch) {
        Forbid();
        SetFunction(GfxBase, APE_OFFSET, APExtra);
        Permit();
        PutStr("Removed patch to AttachPalExtra()...\n");
    }

    GoodBye(rc);
}


/* newAPE ================================================
   replacement for AttachPalExtra()
 */
LONG __asm __saveds newAPE(REG(a0) APTR one, REG(a1) APTR two) {
/*    KPutStr("Hello, out there!\n");
      KPrintF("$%08lx,$%08lx\n", one, two);
 */
    return(APE_NOMEM);
}


/* reportOSErr ===================================================
   turn the SA_ErrorCode return into English.
 */
VOID reportOSErr(ULONG err) {
    switch (err) {
        case OSERR_NOMONITOR:
            Printf("Error %ld: Requested monitor not available.\n", err);
            break;
        case OSERR_NOCHIPS:
            Printf("Error %ld: Required chipset not available.\n", err);
            break;
        case OSERR_NOMEM:
            Printf("Error %ld: Couldn't allocate fast RAM.\n", err);
            break;
        case OSERR_NOCHIPMEM:
            Printf("Error %ld: Couldn't allocate chip RAM.\n", err);
            break;
        case OSERR_PUBNOTUNIQUE:
            Printf("Error %ld: Public screen name wasn't unique.\n", err);
            break;
        case OSERR_UNKNOWNMODE:
            Printf("Error %ld: Unknown screenmode requested.\n", err);
            break;
        case OSERR_TOODEEP:
            Printf("Error %ld: Requested screen was deeper than the hardware supports.\n", err);
            break;
        case OSERR_ATTACHFAIL:
            Printf("Error %ld: Could not attach screens as requested.\n", err);
            break;
        case OSERR_NOTAVAILABLE:
            Printf("Error %ld: Request mode not available (genlock attached?).\n", err);
            break;
        default:
            Printf("Error %ld: Unknown OSERR return!\n", err);
            break;
    }
    return;
}

/* GoodBye ===============================================
   Clean-exit routine.
 */
static VOID GoodBye(int rc) {
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    if (GfxBase) {
        CloseLibrary(GfxBase);
    }

    if (rdargs) {
        FreeArgs(rdargs);
    }

    exit(rc);
}

/* doInit =============================================
 * Open libraries, call ReadArgs() if necessary.
 * Returns TRUE for success, FALSE otherwise.
 */
static LONG doInit(VOID) {
    rdargs = ReadArgs(TEMPLATE, (LONG *)&opts, NULL);
    if (!rdargs) {
        PrintFault(IoErr(), PROGNAME);
        return(FALSE);
    }

    /* need V39 gfx for AttachPalExtra() */
    GfxBase = OpenLibrary("graphics.library", 39L);
    if (!GfxBase) {
        Printf(ERRMSG_LIBNOOPEN, "graphics.library", 39L);
        return(FALSE);
    }

    /* might need V39 intuition for some tags, but leave at 37 for now */
    IntuitionBase = OpenLibrary("intuition.library", 37L);
    if (!IntuitionBase) {
        Printf(ERRMSG_LIBNOOPEN, "intuition.library", 37L);
        return(FALSE);
    }

    return(TRUE);
}
