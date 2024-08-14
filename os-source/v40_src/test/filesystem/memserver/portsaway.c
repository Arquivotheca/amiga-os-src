/* PortsAway.c - Wait for given port-pattern to go away before continuing. J73 junks warning about .c being a script.
lc -cs -t -j73 -oPortsAway.o PortsAway.c
if warn
    quit
endif
revit PortsAway com "by J.W. Lockhart.  © 1992 Commodore."
casm -a PortsAway_rev.asm -o PortsAway_rev.o
blink from lib:cres.o PortsAway_rev.o PortsAway.o to PortsAway lib lib:lc.lib sc sd nd define __main=__tinymain
quit

   Usage: PortsAway <pattern> [interval(secs)]
   Example:  PortsAway (MemC|Read)#? 50
             waits for MemCpy_#? and ReadFM_#? (etc) to go away before continuing.
             It checks existing ports every 50 seconds.  Default is 30 sec.
             Code is residentable.
*/
/* Use common_h pre-compiled headers */

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define PROGNAME        "PortsAway"
#define MAX_PORTS       50
#define MAX_PNAME_LEN   128
#define MAX_CMD_LINE    512
#define PORT_BUFSIZE    MAX_PORTS * MAX_PNAME_LEN
#define PAT_BUFSIZE     (MAX_CMD_LINE * 2) + 4
#define DEFAULT_DELAY   30 * TICKS_PER_SECOND

#define TEMPLATE "PORT_PATTERN/A,TIME=CHECK_INTERVAL/N"
#define OPT_PAT 0
#define OPT_SEC 1
#define NUM_OPT 2

struct Globals {
    struct RDArgs *rdargs;
    SHORT rc;
    UBYTE patBuf[PAT_BUFSIZE];  /* here so as to save stack for ParsePattern */
};

NUKE_SAS_BREAK
REALLY_NUKE_IT

/* extern VOID bsprintf(STRPTR dest, STRPTR fmt, LONG argv, ...); */
extern struct ExecBase *SysBase;
VOID GoodBye(struct Globals *g);

VOID main(int argc, UBYTE *argv[]) {
    struct Node *n;
    LONG checkTime = DEFAULT_DELAY;
    struct Globals *g = NULL;
    LONG Opts[NUM_OPT];
    BOOL done = FALSE;
    BOOL match;

    EXIT_IF_ANCIENT();
    EXIT_IF_WB();

    g = (struct Globals *)AllocVec(sizeof(struct Globals),MEMF_CLEAR);
    if (!g) {
        PutStr("Couldn't allocate RAM for internal buffers!\n");
        GoodBye(g);
    }
    g->rc = RETURN_FAIL;
    memset(Opts,0,sizeof(Opts));

    g->rdargs = ReadArgs(TEMPLATE,Opts,NULL);
    if (!(g->rdargs)) {
        PrintFault(IoErr(),PROGNAME);
        GoodBye(g);
    }

    if (Opts[OPT_SEC]) {
        checkTime = *(LONG *)Opts[OPT_SEC] * TICKS_PER_SECOND;
    }

    /* we assume that OPT_PAT is an /A and always present */
    /* PortNames are CASE SENSITIVE; so is our routine    */
    ParsePattern((STRPTR)Opts[OPT_PAT],g->patBuf,PAT_BUFSIZE);

    g->rc = RETURN_WARN;  /* should only appear if user hits ^C */

    while (!done) {
        Forbid();
            for (n = SysBase->PortList.lh_Head; n != NULL; n=n->ln_Succ) {
                if (n->ln_Name != NULL)
                      if ((match = MatchPattern(g->patBuf,n->ln_Name)))
                        break;
            }
        Permit();
          if (match) {
            /* found port pattern */
            Delay(checkTime);
            if (USER_HIT_CTRL_C)
                GoodBye(g);
        }
        else {
            /* didn't find port pattern */
            done = TRUE;
        }
    }
    g->rc = RETURN_OK;
    GoodBye(g);
}

VOID GoodBye(struct Globals *g) {
    SHORT rc = RETURN_FAIL;
    if (g) {
        rc = g->rc;

        /* cleanup ReadArgs() */
        if (g->rdargs) {
            FreeArgs(g->rdargs);
            g->rdargs = NULL;
        }

        /* Deallocate globals */
        FreeVec(g);
        g = NULL;
    }
    exit(rc);
}
