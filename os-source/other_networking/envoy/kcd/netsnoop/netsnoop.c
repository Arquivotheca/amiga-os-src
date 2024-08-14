#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <envoy/nipc.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <clib/nipc_protos.h>

#include <stdlib.h>
#include <string.h>

#include "tagstruct.h"  /* also defs of Globals & Opts structs */

/* structs */
struct netNode {
    struct Node node;    /* ln_Name will be ptr to taglist     */
    ULONG addr;          /* ip address, for uniqueness testing */
    ULONG printed;       /* have we printed it yet?            */
};

/* defines */
#define TEMPLATE "QUERY/K/M,T=MAXTIME/K/N,R=MAXRESP/K/N,IPADDR,REALM,HOST,SERVICE,ENTITY,OWNER,MACHDESC,CPUFLAG,LIBVERSION,CHIPREV,MAXFASTRAM/N,AVAILFASTRAM/N,MAXCHIPRAM/N,AVAILCHIPRAM/N,KICKVER,WBVER,NIPCVER,NAQ=NOAUTOQUIT/S,HELP/S,DEBUG/S"
#define PROGNAME "NetSnoop"
#define ERR_NOLIB "Can't open %s, version %ld or better."
#define DEFAULT_MAXTIME 15UL
#define DEFAULT_MAXRESP 5000UL
#define DO_ALLOC 1UL
#define DO_FREE  0UL
#define INTOLERABLY_QUIET 3UL
#define PRI_COOKIE 22      /* arbritrary priority for all valid nodes */
#define PAUSE_TICKS 18     /* kludgey excuse for a timer.device wakeup */

/* globals */
struct Globals globals, *g;
struct TagItem inqTags[NUM_TAGITEMS];
struct Hook theHook;
struct Opts opts;
struct Library *UtilityBase;
struct Library *NIPCBase;
extern struct DosLibrary *DOSBase;


/* protos */
VOID  GoodBye(int);
ULONG handleArgs(VOID);
ULONG matchQuery(STRPTR, ULONG *);
LONG  detailClone(struct TagItem *, struct TagItem *, LONG);
BOOL  isUseful(ULONG);
VOID  doHelp(VOID);
BOOL anythingToPrint(struct Node *);
extern VOID printTagArray(struct TagItem *);
extern BOOL xStrToULong(STRPTR, ULONG *);  /* XStrToULong.c */
extern ULONG strToVer(STRPTR);             /* strToVer.c */
extern ULONG makeIPAddr(STRPTR);           /* makeIPAddr.c, requires socket.library */
extern STRPTR addrFromULong(ULONG);        /* makeIPAddr.c, requires socket.library */
ULONG __saveds __asm netHook(register __a0 struct Hook *, register __a2 struct Task *, \
                             register __a1 struct TagItem *);

VOID main(int argc, UBYTE *argv[]) {
    int i, rc;
    ULONG sigs, silence=0UL, printables=TRUE;
    struct Node *nn;

    /* not from workbench... */
    if (!argc) {
        exit(RETURN_FAIL);
    }

    /* Release 2.04 or better */
    if (DOSBase->dl_lib.lib_Version < 37L) { 
        Write(Output(),"You need AmigaDOS V37 or better!\n",33); 
        exit(RETURN_FAIL);
    }

    /* init globals */
    g = &globals;
    globals.inqTags = inqTags;
    globals.myHook = &theHook;
    globals.opts = &opts;
    globals.rc = RETURN_OK;
    globals.sigBit = -1;

    /* Initialize tagitem array */
    for (i = 0; i < Q_ENDSTRUCT; i++)
        inqTags[i].ti_Tag = TAG_IGNORE;
    inqTags[Q_ENDSTRUCT].ti_Tag = TAG_DONE;

    /* init the hook structure */
    theHook.h_Data = g;
    theHook.h_Entry = netHook;
    /* SubEntry and MinNode are both 0... */

    /* go get commandline args, if any */
    g->rdargs = ReadArgs(TEMPLATE,(LONG *)&opts,NULL);
    if (!(g->rdargs)) {
        PrintFault(IoErr(), PROGNAME);
        GoodBye(RETURN_FAIL);
    }

    if (opts.help) {
        doHelp();
        GoodBye(RETURN_OK);
    }

    /* allocate a signal */
    g->sigBit = AllocSignal(-1);
    if (g->sigBit == -1) {
        PutStr("Couldn't allocate signal bit!\n");
        GoodBye(RETURN_FAIL);
    }
    g->sigMask = (1UL << (g->sigBit));

    /* open any version of nipc.library */
    NIPCBase = OpenLibrary("nipc.library",0);
    if (!NIPCBase) {
        Printf(ERR_NOLIB, "nipc.library", 0L);
        GoodBye(RETURN_FAIL);
    }
    globals.NIPCBase = NIPCBase;

    /* open V37 or better utility.library */
    UtilityBase = OpenLibrary("utility.library", 37L);
    if (!UtilityBase) {
        Printf(ERR_NOLIB, "utility.library", 37L);
        GoodBye(RETURN_FAIL);
    }
    globals.UtilityBase = UtilityBase;

    globals.l = &(globals.foundList);
    NewList(globals.l);
    globals.foundList.lh_Type = NT_USER;


    if (!handleArgs()) {
        PutStr("This program is exceptionally boring without arguments.  Try again.\n");
        GoodBye(RETURN_WARN);
    }

    if (opts.examineInput) {
        Printf("Feeding the following tags to NIPCInquiryA():\nTime: %lu sec, MaxResponses: %lu.\n",
                g->maxTime, g->maxResp);
        printTagArray(inqTags);
    }

    if (NIPCInquiryA(g->myHook, g->maxTime, g->maxResp, inqTags)) {
        PutStr("Successful NIPCInquiry.\n");
        rc = g->rc;  /* may be WARN due to a MATCH_WHATEVER not matching */
        sigs = Wait(g->sigMask | SIGBREAKF_CTRL_C);
        if (sigs & SIGBREAKF_CTRL_C) {
            GoodBye(RETURN_OK);
        }
        if (opts.examineInput) {  /* DEBUG option */
            Printf("Err2=%lu, ClonedTags=%lu, HostLen=%lu, SvcLen=%lu\n", 
                    g->err2, g->clonedTags, g->hostLen, g->svcLen);
        }
        Delay(50L);  /* make sure there are plenty of entries */
        while (!(g->hookDone) || (printables == TRUE)) {
            /* NIPCInquiry is asynchronous.  Our hook will signal us
             * when it's time to do some printing.
             * If we screw up and wait too long, a CTRL_C will break us out.
             */
            if (CheckSignal(SIGBREAKF_CTRL_C)) {
                g->hookDone = TRUE;
                Delay(PAUSE_TICKS);
                break;
            }
            else {
                /* print whatever tags are in our list */
                nn = g->l->lh_Head;
                while (nn) {
                    if (CheckSignal(SIGBREAKF_CTRL_C)) {
                        g->hookDone = TRUE;
                        break;
                    }
                    if (!(((struct netNode *)nn)->printed) && (nn->ln_Pri == PRI_COOKIE)) {
                        printTagArray((struct TagItem *)(nn->ln_Name)); /* NULL OK */
                        ((struct netNode *)nn)->printed = TRUE;
                    }
                    nn = nn->ln_Succ;
                }
            }
            /* Now, wait to see if there's anything 
             * new to print...imperfect, but hey.
             */
            Delay(PAUSE_TICKS);  
            printables = anythingToPrint(g->l->lh_Head);
            if (!printables) {
                silence++;
                if ((silence > INTOLERABLY_QUIET) && !(opts.noAutoQuit)) {
                    g->hookDone = TRUE;          /* tell hook to quit  */
                    Delay(PAUSE_TICKS);          /* let it notice, too */
                }
                else {
                    printables = TRUE;  /* continue onwards for a while */
                }
            }
            else {
                silence = 0UL;
            }
        } /* end while */
        /* make sure we don't exit when nipc/hook aren't aware. */
        if (!(g->hookDone)) {
            g->hookDone = TRUE;  /* turn off hook */
            /* we'll let the hook notice before the final GoodBye... */
        }
    }
    else {
        PrintFault(IoErr(),PROGNAME);
        PutStr("Unsuccessful NIPCInquiry!\n");
        rc = RETURN_ERROR;
    }

    /* Okay, really, REALLY emphasize to nipc/hook that We Are Leaving. */
    /* The Hook Itself Won't Be Here To Execute.  Go Away.              */
    theHook.h_Data = NULL;
    g->hookDone = TRUE;

    Delay(50L);

    GoodBye(rc);
}

/* anythingToPrint ==============================================-
   given the top node of a list, see if there's anything to print.
*/
BOOL anythingToPrint(struct Node *anode) {
    struct netNode *nn;

    nn = (struct netNode *)anode;
    while (nn) {
        if (!(nn->printed) && (nn->node.ln_Type == PRI_COOKIE))
            return(TRUE);
        nn = (struct netNode *)(nn->node.ln_Succ);
    }
    return(FALSE);
}

/* matchQuery ====================================================
   solely for dealing with the QUERY/M in the template.
   Case-insensitive matching...

   Inputs:
      STRPTR q - word which user typed
      ULONG *n - array index variable, letting you know where in tagitem to work.

   Returns 0xFFFFFFFF for failure, otherwise matching QUERY_WHATEVER.

   Requires:
        proper defs in "tagstruct.h"
        open UtilityBase
 */
ULONG matchQuery(STRPTR q, ULONG *n) {

    if (!q || !n) {
        return(0xFFFFFFFF);
    }

    if (!Stricmp(q,S_IPADDR)) {
        *n = Q_IPADDR;
        return(QUERY_IPADDR);
    }
    if (!Stricmp(q,S_REALMS)) {
        *n = (Q_REALMS);
        return(QUERY_REALMS);
    }
    if (!Stricmp(q,S_HOSTS)) {
        *n = (Q_HOSTNAME);
        return(QUERY_HOSTNAME);
    }
    if (!Stricmp(q,S_SERVICE)) {
        *n = (Q_SERVICE);
        return(QUERY_SERVICE);
    }
    if (!Stricmp(q,S_ENTITY)) {
        *n = (Q_ENTITY);
        return(QUERY_ENTITY);
    }
    if (!Stricmp(q,S_OWNER)) {
        *n = (Q_OWNER);
        return(QUERY_OWNER);
    }
    if (!Stricmp(q,S_MACHDESC)) {
        *n = (Q_MACHDESC);
        return(QUERY_MACHDESC);
    }
    if (!Stricmp(q,S_ATTNFLAGS)) {
        *n = (Q_ATTNFLAGS);
        return(QUERY_ATTNFLAGS);
    }
    if (!Stricmp(q,S_LIBVERSION)) {
        *n = (Q_LIBVERSION);
        return(QUERY_LIBVERSION);
    }
    if (!Stricmp(q,S_CHIPREVBITS)) {
        *n = (Q_CHIPREVBITS);
        return(QUERY_CHIPREVBITS);
    }
    if (!Stricmp(q,S_MAXFASTMEM)) {
        *n = (Q_MAXFASTMEM);
        return(QUERY_MAXFASTMEM);
    }
    if (!Stricmp(q,S_AVAILFASTMEM)) {
        *n = (Q_AVAILFASTMEM);
        return(QUERY_AVAILFASTMEM);
    }
    if (!Stricmp(q,S_MAXCHIPMEM)) {
        *n = (Q_MAXCHIPMEM);
        return(QUERY_MAXCHIPMEM);
    }
    if (!Stricmp(q,S_AVAILCHIPMEM)) {
        *n = (Q_AVAILCHIPMEM);
        return(QUERY_AVAILCHIPMEM);
    }
    if (!Stricmp(q,S_KICKVERSION)) {
        *n = (Q_KICKVERSION);
        return(QUERY_KICKVERSION);
    }
    if (!Stricmp(q,S_WBVERSION)) {
        *n = (Q_WBVERSION);
        return(QUERY_WBVERSION);
    }
    if (!Stricmp(q,S_NIPCVERSION)) {
        *n = (Q_NIPCVERSION);
        return(QUERY_NIPCVERSION);
    }

    return(0xFFFFFFFF);
}

/* handleArgs ==================================================================
   deals with the plethora of possible cmdline arguments.  
   Plugs them into the [disturbing number of] global variables: specifically,
   depends on tagInq, globals, opts, etc.
 */

ULONG handleArgs(VOID) {
    int i=0;
    STRPTR *str;
    ULONG tagNumber=0UL, tagTag, didSomething=FALSE;

    /* do the various types of queries possible */
    if (str = opts.query) {
        while (str[i]) {
            if (0xFFFFFFFF == (tagTag = matchQuery(str[i],&tagNumber))) {
                /* bad keyword */
                Printf("%s: invalid query object: '%s'!\n", PROGNAME, str[i]);
            }
            else {
                /* found valid keyword */
                inqTags[tagNumber].ti_Tag = tagTag;
                didSomething = TRUE;
                /* ti_Data setup?  Needed? */
            }
            i++;
        }
    }

    if (opts.maxTime) {
        g->maxTime = *opts.maxTime;
    }
    else {
        g->maxTime = DEFAULT_MAXTIME;
    }

    if (opts.maxResp) {
        g->maxResp = *opts.maxResp;
    }
    else {
        g->maxResp = DEFAULT_MAXRESP;
    }

    if (opts.ipaddr) {
        g->ipaddr_real = makeIPAddr(opts.ipaddr);
        inqTags[M_IPADDR].ti_Tag  = MATCH_IPADDR;
        inqTags[M_IPADDR].ti_Data = (ULONG)(g->ipaddr_real);
        didSomething = TRUE;
    }
    if (opts.realm) {
        inqTags[M_REALM].ti_Tag  = MATCH_REALM;
        inqTags[M_REALM].ti_Data = (ULONG)(opts.realm);
        didSomething = TRUE;
    }
    if (opts.hostName) {
        inqTags[M_HOSTNAME].ti_Tag  = MATCH_HOSTNAME;
        inqTags[M_HOSTNAME].ti_Data = (ULONG)(opts.hostName);
        didSomething = TRUE;
    }
    if (opts.service) {
        inqTags[M_SERVICE].ti_Tag  = MATCH_SERVICE;
        inqTags[M_SERVICE].ti_Data = (ULONG)(opts.service);
        didSomething = TRUE;
    }
    if (opts.entity) {
        inqTags[M_ENTITY].ti_Tag  = MATCH_ENTITY;
        inqTags[M_ENTITY].ti_Data = (ULONG)(opts.entity);
        didSomething = TRUE;
    }
    if (opts.owner) {
        inqTags[M_OWNER].ti_Tag  = MATCH_OWNER;
        inqTags[M_OWNER].ti_Data = (ULONG)(opts.owner);
        didSomething = TRUE;
    }
    if (opts.machDesc) {   /* so what is machDesc??? */
        inqTags[M_MACHDESC].ti_Tag  = MATCH_MACHDESC;
        inqTags[M_MACHDESC].ti_Data = (ULONG)(opts.machDesc);
        didSomething = TRUE;
    }
    if (opts.attnFlags) {
        if (xStrToULong(opts.attnFlags, &(inqTags[M_ATTNFLAGS].ti_Data))) {
            inqTags[M_ATTNFLAGS].ti_Tag  = MATCH_ATTNFLAGS;
            didSomething = TRUE;
        }
    }
    if (opts.libVersion) {
        inqTags[M_LIBVERSION].ti_Tag  = MATCH_LIBVERSION;
        inqTags[M_LIBVERSION].ti_Data = strToVer(opts.libVersion);
        didSomething = TRUE;
    }
    if (opts.chipRevBits) {
        if (xStrToULong(opts.chipRevBits, &(inqTags[M_CHIPREVBITS].ti_Data))) {
            inqTags[M_CHIPREVBITS].ti_Tag  = MATCH_CHIPREVBITS;
            didSomething = TRUE;
        }
    }
    if (opts.maxFastMem) {
        inqTags[M_MAXFASTMEM].ti_Tag  = MATCH_MAXFASTMEM;
        inqTags[M_MAXFASTMEM].ti_Data = *opts.maxFastMem;
        didSomething = TRUE;
    }
    if (opts.availFastMem) {
        inqTags[M_AVAILFASTMEM].ti_Tag  = MATCH_AVAILFASTMEM;
        inqTags[M_AVAILFASTMEM].ti_Data = *opts.availFastMem;
        didSomething = TRUE;
    }
    if (opts.maxChipMem) {
        inqTags[M_MAXCHIPMEM].ti_Tag  = MATCH_MAXCHIPMEM;
        inqTags[M_MAXCHIPMEM].ti_Data = *opts.maxChipMem;
        didSomething = TRUE;
    }
    if (opts.availChipMem) {
        inqTags[M_AVAILCHIPMEM].ti_Tag  = MATCH_AVAILCHIPMEM;
        inqTags[M_AVAILCHIPMEM].ti_Data = *opts.availChipMem;
        didSomething = TRUE;
    }
    if (opts.kickVersion) {
        inqTags[M_KICKVERSION].ti_Tag  = MATCH_KICKVERSION;
        inqTags[M_KICKVERSION].ti_Data = strToVer(opts.kickVersion);
        didSomething = TRUE;
    }
    if (opts.wbVersion) {
        inqTags[M_WBVERSION].ti_Tag  = MATCH_WBVERSION;
        inqTags[M_WBVERSION].ti_Data = strToVer(opts.wbVersion);
        didSomething = TRUE;
    }
    if (opts.nipcVersion) {
        inqTags[M_NIPCVERSION].ti_Tag  = MATCH_NIPCVERSION;
        inqTags[M_NIPCVERSION].ti_Data = strToVer(opts.nipcVersion);
        didSomething = TRUE;
    }

    /* setup so we can tell what we have already done... */
    if (inqTags[M_IPADDR].ti_Tag != TAG_IGNORE) {
        g->pr_ipaddr = TRUE;         /* user wants to see it */
        g->addr_tag = MATCH_IPADDR;  /* used by hook         */
    }
    else {
        g->addr_tag = QUERY_IPADDR;  /* used by hook */

        if (inqTags[Q_IPADDR].ti_Tag != TAG_IGNORE) {
            g->pr_ipaddr = TRUE;  /* user wants to see 'em */
        }
        else {
            /* we need some IPAddrs to determine uniqueness */
            inqTags[Q_IPADDR].ti_Tag = QUERY_IPADDR;
            g->pr_ipaddr = FALSE;  /* user doesn't want to see 'em */
        }
    }

    return(didSomething);
}

/* GoodBye =============================================================
   clean-exit routine 
 */
VOID GoodBye(int rc) {
    struct Node *nn;

    if (g->rdargs) {
        FreeArgs(g->rdargs);
    }

    if (g->l) {
        while (nn = RemHead(g->l)) {
            if (nn->ln_Name) {
                detailClone((struct TagItem *)(nn->ln_Name), NULL, DO_FREE);
                FreeTagItems((struct TagItem *)(nn->ln_Name));
            }
            FreeVec(nn);
        }
    }

    if (g->sigMask) {
        FreeSignal(g->sigBit);
    }

    if (UtilityBase) {
        CloseLibrary(UtilityBase);
    }

    if (NIPCBase) {
        CloseLibrary(NIPCBase);
    }

    exit(rc);
}


/*  What's the bait on the hook, anyway?
struct Hook {
    struct MinNode h_MinNode;
    ULONG          (*h_Entry)();        / assembler entry point /
    ULONG          (*h_SubEntry)();     / often HLL entry point /
    APTR           h_Data;              / owner specific        /
};
*/



/* netHook =======================================================================
   The hook which is called in NIPCInquiry()...
   It's not a speedy hook, which could cause problems in some cases.
 */
/* For NIPCInquiry, myHook = hook data structure
                    object = ptr to struct Task of NIPCInq caller, for signalling purposes.
                    message= ptr to array of TagItem structs containing response

   returns FALSE to cause abnormal termination of inquiry, TRUE otherwise.
 */
ULONG __saveds __asm netHook(register __a0 struct Hook    *myHook,
                             register __a2 struct Task    *callingTask,
                             register __a1 struct TagItem *tagArr) 
{
    struct Globals *gl;
    struct TagItem *ti, *newTI;
    struct netNode *nn, *newNN;
    ULONG ipAddr=0UL;
    BOOL unique=TRUE;
    BOOL bad = FALSE;  /* for allocation tracking */

    if (!myHook)
        return(FALSE);

    if (!tagArr)
        return(FALSE);

    /* convenience & a little speed */
    gl = (struct Globals *)(myHook->h_Data);

    if (!gl) {
        return(FALSE);  /* can't deal with lack of appropriate data */
    }

    if (gl->hookDone) {
        return(FALSE);  /* yo, main program finished, don't call hook no more :-) */
    }


    /* Get relevant ipaddr.
     * addr_tag is set up in handleArgs()
     * and indicates whether to use MATCH_IPADDR or QUERY_IPADDR.
     */
    if (ti = FindTagItem(gl->addr_tag, tagArr)) {
        ipAddr = ti->ti_Data;
        /* find out if we have an entry for that */
        nn = (struct netNode *)(gl->l->lh_Head);
        while (nn) {
            if (nn->addr == ipAddr) {
                unique = FALSE;
                break;
            }
            nn = (struct netNode *)(nn->node.ln_Succ);
        }
    }

    if (gl->hookDone) {
        return(FALSE);  /* yo, main program finished, don't call hook no more :-) */
    }

    /* If we have run into something we haven't seen,
     * by all means, add it to our List of Things To Print.
     */
    if (unique) {
        /* must add entry */
        if (newNN = AllocVec(sizeof(struct netNode),MEMF_CLEAR)) {
            if (newTI = CloneTagItems(tagArr)) {
                if (detailClone(newTI, tagArr, DO_ALLOC)) {
                    newNN->node.ln_Name = (char *)newTI;
                    newNN->node.ln_Pri = PRI_COOKIE;
                    newNN->node.ln_Type = NT_USER;
                    newNN->printed = FALSE;
                    newNN->addr = ipAddr;  /* 0 or address from above */
                    AddTail(g->l, (struct Node *)newNN);
                }
                else {
                    detailClone(newTI, NULL, DO_FREE);
                    bad = TRUE;
                }
                if (bad) {
                    FreeTagItems(newTI);
                }
            }
            else {
                bad = TRUE;
            }
            if (bad) {
                FreeVec(newNN);
            }
        }
        /* signal our task so that it can process the tags. */
        Signal(callingTask, gl->sigMask);
    }

    /* Else, no signal, no joy in Printerville this time */

    /* we can realize when we're done by counting the
     * responses... probably saves a little bit of time.
     */
    gl->maxResp--;
    if (!(gl->maxResp)) {
        /* we're done */
        gl->hookDone = TRUE;
        /* let the calling task wake up, if necessary. */
        Signal(callingTask, gl->sigMask);
    }

    /* return FALSE to terminate, TRUE to continue.
     * If user typed ^C to our task, gl->hookDone is set, so
     * stop looking over the net.  Or if we have 5000 pings
     * of the same machine going and there's nothing new
     * to print, the main code will quit out this way.
     */
    return((ULONG)(!(gl->hookDone)));  
}

/* detailClone ============================================================
   Give this puppy two TagItem arrays and a DO_ALLOC or DO_FREE.
   It will allocate its own strings if necessary, and copy the existing
   ones into it.

   Conversely, if given DO_FREE, it'll deallocate only the strings.
   You'll still have to FreeTagItems(array) yourself.

   Returns TRUE if every allocation succeeded, FALSE otherwise.

   to allocate:
       dest = CloneTagItems(src);
       detailClone(dest, src, DO_ALLOC);
   to free:
       detailClone(dest, NULL, DO_FREE);
       ONLY WORKS WITH THINGS ALLOCATED BY DETAILCLONE!  CRASHES OTHERWISE!!
*/
LONG detailClone(struct TagItem *tid, struct TagItem *tis, LONG doAlloc) {
    ULONG len;
    struct TagItem *ti, *ti2;          /* temp ptrs for dest & src  */
    struct TagItem *dtCtrl, *stCtrl;   /* for NextTagItem() control */
    struct TagItem *ct;                /* for cloning src tag       */
    STRPTR s1, s2;

    if (doAlloc) {
        if (!tid || !tis) {
            g->err2 = NS_ERR_NULL_ARG;
            return(FALSE);
        }
        ct = CloneTagItems(tis);
        if (!ct) {
            g->err2 = NS_ERR_CLONEFAIL;
            return(FALSE);
        }

        dtCtrl = tid;
        stCtrl = ct;

        /* search only for tags which need allocations... */
        while (ti = NextTagItem(&dtCtrl)) {
            if (ti->ti_Tag == TAG_DONE) {
                break;
            }

            /* MUST null the originals *first* so we can deallocate sensibly
               if we fail!
             */
            if (isUseful(ti->ti_Tag)) {
                ti->ti_Data = 0UL;
            }
        }

        /* now do some real cloning */
        dtCtrl = tid;     /* reset from last use */
        while (ti = NextTagItem(&dtCtrl)) {
            if (ti->ti_Tag == TAG_DONE) {
                break;
            }

            ti2 = NextTagItem(&stCtrl);
            if (!ti2) {
                /* paranoia */
                g->err2 = NS_ERR_TAG_MISMATCH;
                break;
            }
            if (ti2->ti_Tag == TAG_DONE) {
                break;  /* shouldn't happen, either! */
            }

            if (isUseful(ti->ti_Tag)) {
                if (ti2->ti_Tag == ti->ti_Tag) {
                    s2 = (STRPTR)(ti2->ti_Data);
                    if (!s2) {
                        g->err2 = NS_ERR_NULL_STRING;
                    }
                    else if (*s2 == '\0') {
                        g->err2 = NS_ERR_EMPTY_STRING;
                    }
                    len = strlen(s2);
                    /* debug */
                    if (ti->ti_Tag == MATCH_HOSTNAME)
                        g->hostLen = len;
                    if (ti->ti_Tag == QUERY_SERVICE)
                        g->svcLen = len;
                    /* end debug */
                    ti->ti_Data = (ULONG)AllocVec(len+1,MEMF_CLEAR);
                    s1 = (STRPTR)(ti->ti_Data);
                    if (!s1) {
                        g->err2 = NS_ERR_NO_MEM;
                        FreeTagItems(ct);
                        return(FALSE);
                    }
                    strcpy(s1, s2);  /* strcpy(to, from) */
                    g->clonedTags++;
                }
                else {
                    /* Only happens if there's a bug in utility.library
                       or if dest wasn't a clone of src.
                     */
                    g->err2 = NS_ERR_CLONEFAIL;
                    FreeTagItems(ct);
                    return(FALSE);
                }
            }
        }
        if (ct) {
            FreeTagItems(ct);
        }
    }
    else {       /* Free a [perhaps failed] allocation */
        if (!tid) {
            return(FALSE);
        }
        dtCtrl = tid;

        /* search only for tags which need deallocations */
        while (ti = NextTagItem(&dtCtrl)) {
            if (ti->ti_Tag == TAG_DONE) {
                break;
            }
            if (isUseful(ti->ti_Tag)) {
                if (ti->ti_Data) {
                    FreeVec((APTR)(ti->ti_Data));
                    g->clonedTags--;
                }
            }
        }
    }
    return(TRUE);
}

/* isUseful ===================================================================
   pick out STRPTR-related tags; used with detailClone().
 */
BOOL isUseful(ULONG tag) {
    /* realm, host, service, entity, owner, machdesc */
    if (tag == MATCH_REALM)
        return(TRUE);
    if (tag == QUERY_REALMS)
        return(TRUE);
    if (tag == MATCH_HOSTNAME)
        return(TRUE);
    if (tag == QUERY_HOSTNAME)
        return(TRUE);
    if (tag == MATCH_SERVICE)
        return(TRUE);
    if (tag == QUERY_SERVICE)
        return(TRUE);
    if (tag == MATCH_OWNER)
        return(TRUE);
    if (tag == QUERY_OWNER)
        return(TRUE);
    if (tag == MATCH_MACHDESC)
        return(TRUE);
    if (tag == QUERY_MACHDESC)
        return(TRUE);
    if (tag == MATCH_ENTITY)
        return(TRUE);
    if (tag == QUERY_ENTITY)
        return(TRUE);
    return(FALSE);
}
/* doHelp ==================================================================
   prints help to Output()...
 */
VOID doHelp(VOID) {

    Printf("Template:\n%s\n", TEMPLATE);
    
    PutStr("T=MAXTIME/K/N - max number of seconds to scan (15)\n");
    PutStr("R=MAXRESP/K/N - max number of responses to process (5000)\n\n");
    PutStr("IPADDR       - match specific IP address (ex: 169.143.25.69)\n");
    PutStr("REALM        - match specific realm\n");
    PutStr("HOST         - match specific host\n");
    PutStr("SERVICE      - match specific service\n");
    PutStr("ENTITY       - match specific entity\n");
    PutStr("OWNER        - match specific owner\n");
    PutStr("MACHDESC     - match specific machine description\n");
    PutStr("CPUFLAG      - (HEX) match specific CPU flags, see <exec/execbase.h>\n");
    PutStr("LIBVERSION   - match specific library version (ex: \"39 106\")\n");
    PutStr("CHIPREV      - (HEX) match specific chip revisions, see <graphics/gfxbase.h>\n");
    PutStr("MAXFASTRAM   - match specific amount of fast RAM installed\n");
    PutStr("AVAILFASTRAM - match specific amount of fast RAM available\n");
    PutStr("MAXCHIPRAM   - match specific amount of chip RAM installed\n");
    PutStr("AVAILCHIPRAM - match specific amount of chip RAM available\n");
    PutStr("KICKVER      - match kickstart versions >= this one (ex: \"39 106\")\n");
    PutStr("WBVER        - match workbench versions >= this one (ex: \"39 46\")\n");
    PutStr("NIPCVER      - match nipc.library versions >= this one (ex: \"37 103\")\n");
    PutStr("NOAUTOQUIT   - wait for full amount of time before exiting.  Default\n");
    PutStr("               behavior is to quit if there's no new info for a while.\n");
    PutStr("               With this option, use Control-C to exit.\n");
    PutStr("HELP/S       - print this message.\n");

    if (CheckSignal(SIGBREAKF_CTRL_C))
        return;

    PutStr("QUERY/K/M Options:\n");
    Printf("  %10s -- look for all %s\n",  S_IPADDR, "IP addresses");
    Printf("  %10s -- look for all %s\n",  S_REALMS, "realms");
    Printf("  %10s -- look for all %s\n",  S_HOSTS, "host names");
    Printf("  %10s -- look for all %s\n",  S_SERVICE, "services");
    Printf("  %10s -- look for all %s\n",  S_ENTITY, "entities");
    Printf("  %10s -- look for all %s\n",  S_OWNER, "machine owners");
    Printf("  %10s -- look for all %s\n",  S_MACHDESC, "machine descriptions");
    Printf("  %10s -- look for all %s\n",  S_ATTNFLAGS, "cpu-type flags (execbase)");
    Printf("  %10s -- look for all %s\n",  S_LIBVERSION, "lib versions");
    Printf("  %10s -- look for all %s\n",  S_CHIPREVBITS, "gfx chip revisions");
    Printf("  %10s -- check max amounts of %s RAM\n",  S_MAXFASTMEM,  "fast");
    Printf("  %10s -- check available amounts of %s RAM\n",  S_AVAILFASTMEM,  "fast");
    Printf("  %10s -- check max amounts of %s RAM\n",  S_MAXCHIPMEM,  "chip");
    Printf("  %10s -- check available amounts of %s RAM\n",  S_AVAILCHIPMEM,  "chip");
    Printf("  %10s -- look for all %s\n",  S_KICKVERSION, "kickstart versions");
    Printf("  %10s -- look for all %s\n",  S_WBVERSION, "workbench versions");
    Printf("  %10s -- look for all %s\n\n",  S_NIPCVERSION, "nipc.library versions");

    PutStr("To exit the program, press control-C at any time.\n");
}
