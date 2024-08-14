/* Stuff that really belongs in TstMgr_tm.c, but is inconvenient to keep there
   simply because TM always nukes it.
*/

#include <exec/memory.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>

#include "TstMgr.h"
#include "TstMgr_tm.h"
#include "TstMgr_text.h"
#include "TstMgr_tm_protos.h"
#include "TstMgr_protos.h"
#include "tmgr_misc_protos.h"
#ifndef TMGR_COMMON_H
#include "tmgr_common.h"
#endif

#ifndef abs(x)
#define abs(x) (((x) < 0) ? -(x) : (x))
#endif

/* GLOBALS */
struct Library *IconBase;       /* icon.library (ROM)         */
struct Library *CxBase;         /* commodities.library (DISK) */
struct Library *UtilityBase;    /* utility.library (ROM)      */


/* a few useful things from tmgr_common.h */
NUKE_SAS_BREAK
REALLY_NUKE_IT

#ifndef D(x)
#define D(x)
#endif

#ifndef D2(x)
#define D2(x) /* PutStr(x) */
#endif

#define ERR_PEN0_RED   0xB
#define ERR_PEN0_GREEN 0x2
#define ERR_PEN0_BLUE  0x2

#define ERR_PEN3_RED   0x1
#define ERR_PEN3_GREEN 0xC
#define ERR_PEN3_BLUE  0x1

/* Other protos */
extern void akprintf(STRPTR fmt, APTR args);  /* asm stub for KPrintF(), works like VPrintf() */
extern void bsprintf(STRPTR dst, STRPTR fmt, LONG arg, ...); /* asm */

/****** TstMgr_tm.c/TMgr_DoSigs ******************************************
*
*   NAME
*       TMgr_DoSigs -- Process signals received from clients
*
*   SYNOPSIS
*       done = TMgr_DoSigs(TMData)
*
*       BOOL TMgr_DoSigs(struct TMData *);
*
*   FUNCTION
*       Does all processing of client signals.
*
*   INPUTS
*       TMData - the usual for TM_ stuff.  Only uses UserData
*                portion, which must be set up by TMgr_Init().
*
*   RESULT
*       done - TRUE if you want to exit, FALSE otherwise.
*              Should always return FALSE, as the only exit is
*              from the Exit gadget onscreen.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*      TMgr_Init()
*
******************************************************************************
*
*/
BOOL TMgr_DoSigs(struct TMData *tmd) {
    struct TMgr_Globals *t;
    struct errReport *er;

D("Within DoSigs()\n");
    t = (struct TMgr_Globals *)(tmd->UserData);
    while (er = (struct errReport *)GetMsg(t->mp)) {
        if (er->errLevel == LVL_INT) {
            /* handle (ID?) request */
            handleInternalMsg(t, er, tmd);
        }
        else {
            /* deal with error report messages */
            if (er->errLevel & t->okMsg) {  /* only if we want to see the msg */
                handleReport(t, er, tmd);
            }
            /* else we nearly ignore the report */
        }
        er->returnCmd = ((t->carryOn) ? TMCMD_PROCEED : TMCMD_ABORT);
        ReplyMsg((struct Message *)er);
    }
    return(FALSE);
}

BOOL handleInternalMsg(struct TMgr_Globals *t, struct errReport *er, struct TMData *TMData) {
    if (er->problem == PROB_NEED_ID) {
        if (t->clientCount == 0) {
            GT_SetGadgetAttrs(gadget_TESTING,
                              window_ERRWINDO,
                              NULL, 
                              GTTX_Text, TEXT_W0G6T1, 
                              TAG_DONE);
        }
        er->errCode = ++(t->clientCount);
        return(TRUE);
    }
    return(FALSE);   /* bogus message received */
}

/* parPrint ================================
   DPutFmt works like VPrintf, I need a Printf at points
*/
VOID parPrint(STRPTR fmt, APTR arg, ...) {
    DPutFmt(fmt, &arg);
}

/* handleReport ==================================================================
 */
VOID handleReport(struct TMgr_Globals *t, struct errReport *er, struct TMData *tmd) {
    UBYTE *tmpSD, *tmpST;                       /* temporary dat_StrDate/Time ptrs */
    BOOL  errCondition;
    UBYTE banner[80];

    /* Need to:
          - update con/ser/par/logfile using VFPrintf(), akprintf(), DPutFmt().
          - update error status
          - update screen (colors, msgs, gadgets, etc.)
          - if client is exiting, update clientCount
          - note that telling clients to exit is elsewhere.
          
     */

    /* LEN_DATSTRING 16 size of dat_StrDate, dat_StrTime */
    /* Do TimeStamp, etc. REQUIRES that the DateStamp() be done by the client FIRST. */
    tmpSD = er->dt.dat_StrDate;
    tmpST = er->dt.dat_StrTime;
    er->dt.dat_StrDate = t->sdbuf;
    er->dt.dat_StrTime = t->stbuf;
    DateToStr(&(er->dt));

    errCondition = (er->problem > PROB_NONE);
    /* Concoct banner ONCE... */
    if (errCondition) {
        bsprintf(banner,
                 "%s%s at %s %s: !*! Error %ld:\n  ",
                 (LONG)(ERR_AWAKEN),  /* bells and inverse video */
                 er->clientName,
                 t->stbuf,
                 t->sdbuf,
                 er->errCode);
        Fault(er->errCode, er->clientName, t->lastErr, LAST_ERR_BUFSIZE);
    }
    else {
        bsprintf(banner, 
                 "%s at %s %s:\n  ", 
                 (LONG)(er->clientName),
                 t->stbuf, 
                 t->sdbuf);
    }
    /* Fault(LONG code, UBYTE *msg, UBYTE *dest, LONG destSize); */
    /* First thing is to dump serial output in case of an upcoming crash */
    if (t->outputs & OUTF_SER) {
        KPutStr(banner);
        akprintf(er->logMsg, er->varArgs);
        if (errCondition)
            KPrintF("  %s%s\n",t->lastErr,ERR_SLEEP);
    }
    if (t->outputs & OUTF_FILE) {
        FPuts(t->fh, banner);
        VFPrintf(t->fh, er->logMsg, er->varArgs);
        if (errCondition)
            FPrintf(t->fh,"  %s%s\n",t->lastErr,ERR_SLEEP);
    }
    if (t->outputs & OUTF_CON) {
        FPuts(t->cfh, banner);
        VFPrintf(t->cfh, er->logMsg, er->varArgs);
        if (errCondition)
            FPrintf(t->cfh,"  %s%s\n",t->lastErr,ERR_SLEEP);
    }
    if (t->outputs & OUTF_PAR) {    /* can't do bells/video/bold :-( */
        parPrint("%s at %s %s: !*! Error %ld:\n  ",
                 er->clientName,
                 t->stbuf,
                 t->sdbuf,
                 er->errCode);
        DPutFmt(er->logMsg, er->varArgs);
        if (errCondition)
            parPrint("  %s\n", t->lastErr);
    }
    if (er->errLevel & LVL_EXIT)  /* yes, you can exit with various other LVL_s set */
        t->clientCount--;

    updateScreen(t, er, tmd, ((errCondition) ? FAIL_TEST : PASS_TEST));

    /* Restore DateTime stuff */
    er->dt.dat_StrDate = tmpSD;
    er->dt.dat_StrTime = tmpST;
}

/* updateScreen ====================================================================
   t - globals
  er - incoming message report
 tmd - screen/gadget info
  pf - pass/fail indicator
*/
#define UPDATE_X 268
#define UPDATE_EC_Y 35  /* + BarHeight */
#define UPDATE_LE_Y 19  /* + BarHeight */
#define UPDATE_LM_Y  3  /* + BarHeight */
#define UPDATE_TEXTLEN 40

VOID updateScreen(struct TMgr_Globals *t, struct errReport *er, struct TMData *TMData, BOOL pf) {
    struct Screen *scr;
    struct Window *win;
    struct Gadget *pfg;                 /* Pass/Fail Gadget    */
    struct Gadget *itg;                 /* Idle/Testing Gadget */
/*    struct Region *reg = NULL;    */      /* experimental */
    /* Update the following:
         - p/f indicator gadget.
         - Screen colors.
         - Last Rpt, Last Err, ErrCount displays (Text())
        GT_SetGadgetAttrs (GADNAME, win, NULL, GTTX_Text, NEWTEXT, TAG_DONE);
           TEXT_W0G0T1/2 = pass/fail, see TstMgr_text.h
           TEXT_W0G1T1/2 = testing/idle, see TstMgr_text.h
        GT_RefreshWindow(win,NULL);
        SetRGB4(struct ViewPort *vp, short pen, ubyte r, ubyte g, ubyte b);
        Text(struct RastPort *rp, strptr buf, word strlen);
        Move(struct RastPort *rp, short x, short y);
        RectFill(rp, xmin, ymin, xmax, ymax);
     */
    scr = TMData->TMScreenInfo[0].Screen;
    win = TMData->TMWindowInfo[0].Window;
    pfg = TMData->TMGadgetInfo_ERRWINDO[0].Gadget;  /* ie, gadget_PASSFAIL in TstMgr_tm.h */
    itg = TMData->TMGadgetInfo_ERRWINDO[1].Gadget;  /* ie, gadget_TESTING in TstMgr_tm.h */

    /* #define ERR_PEN0_RED   0xB
       #define ERR_PEN0_GREEN 0x2
       #define ERR_PEN0_BLUE  0x2
       #define ERR_PEN3_RED   0x1
       #define ERR_PEN3_GREEN 0xC
       #define ERR_PEN3_BLUE  0x1
    */
    /* TextLocations:        !*!
       Width is 340 pixels, or about 40 characters (Topaz 8)
       X: 268 (of 640)
       Y:
       ErrorCount: 35+ScreenInfo_TESTMANA.Screen->BarHeight;
       LastErr:    19+ScreenInfo_TESTMANA.Screen->BarHeight;
       LastMsgTime: 3+ScreenInfo_TESTMANA.Screen->BarHeight;
     */

    if ((t->testStatus == PASS_TEST) && (pf == FAIL_TEST)) {
        /* Change screen colors to reflect an ERROR, if first error */
        D("Error Condition: SetRGB4\n");
        if (!t->useWB) {
          SetRGB4(&(scr->ViewPort), 0, ERR_PEN0_RED, ERR_PEN0_GREEN, ERR_PEN0_BLUE);
          SetRGB4(&(scr->ViewPort), 3, ERR_PEN3_RED, ERR_PEN3_GREEN, ERR_PEN3_BLUE);
        }
        else { /* we're on WB, gotta do something else */
          DisplayBeep(NULL);  /* Flashes EVERYONE's screen (!) */
          /* experimental: */
          blueRefreshKludge(window_ERRWINDO, gadget_LASTERRO);
        }
        t->testStatus = FAIL_TEST;
        D("Error Condition: update PASSFAIL\n");
        GT_SetGadgetAttrs(gadget_PASSFAIL, win, NULL, GTTX_Text, TEXT_W0G5T2, TAG_DONE);
    }

    /* Update Last Message field */

    /* Update name of last message sender */  /* last msg from */
    D("Update last msg sender\n");
/*    bsprintf(t->lmfBuf,"%s", (LONG)(er->clientName)); */
    stccpy(t->lmfBuf,er->clientName,DGAD_BUFLEN-1);
    blankString(t->lmfBuf, DGAD_BUFLEN);  /* use all available space */
    GT_SetGadgetAttrs(gadget_LMFrom, win, NULL, GTTX_Text, t->lmfBuf, TAG_DONE);

    /* Update time of last message */  /* last msg recd */
    D("Update last msg time\n");
    bsprintf(t->lmrBuf,"%s   on   %s", (LONG)(t->stbuf), t->sdbuf);
    GT_SetGadgetAttrs(gadget_LTime, win, NULL, GTTX_Text, t->lmrBuf, TAG_DONE);


    if (pf == FAIL_TEST) {
        /* Update Last Error (LASTERRO = from, LErr = Time) field */
        t->errCount++;

        /* Last error sender */  /* last err from */
        D("Update last Error Sender\n");
/*        bsprintf(t->lefBuf, "%s", (LONG)(er->clientName)); */
        stccpy(t->lefBuf,er->clientName,DGAD_BUFLEN-1);
        blankString(t->lefBuf, DGAD_BUFLEN);  /* use all available space */
        GT_SetGadgetAttrs(gadget_LASTERRO, win, NULL, GTTX_Text, t->lefBuf, TAG_DONE);

        /* Last error time */  /* last err recd */
        D("update last error time\n");
        bsprintf(t->lerBuf,"%s   on   %s", (LONG)(t->stbuf), t->sdbuf);
        GT_SetGadgetAttrs(gadget_LErr, win, NULL, GTTX_Text, t->lerBuf, TAG_DONE);

        /* Update Error Count field */
        D("Update error count gad\n");
        bsprintf(t->tmpBuf,"%11ld", t->errCount);
        GT_SetGadgetAttrs(gadget_ECnt, win, NULL, GTTX_Text, t->tmpBuf, TAG_DONE);

        if (t->earlyExit)
            t->carryOn = FALSE;
    }
    /* Update Testing/Idle gadget */
    if (t->clientCount == 0) {
        GT_SetGadgetAttrs(gadget_TESTING, win, NULL, GTTX_Text, TEXT_W0G6T2, TAG_DONE);
    }
}


/* Stuff that usually lives in TstMgr.c */

/* createClientPort =========================================================          */
/* Note: port_template and max_port_retries from TstMgr.h, port_priority from tmgr_common.h */
struct MsgPort *createClientPort(STRPTR portName) {
    SHORT i;
    struct MsgPort *mp = NULL;

    D("Within createClientPort()\n");
    for (i = 0; i < MAX_PORTNAME_RETRIES; i++) {
        bsprintf(portName, "%s%ld", (LONG)PORT_TEMPLATE, (LONG)i);
        Forbid();
            if (!FindPort(portName)) {
                mp = CreatePort(portName,PORT_PRIORITY);
                Permit();
                return(mp);
            }
        Permit();
    }
    return(NULL);
}

/* TMgr_Init ================================================================ */
BOOL TMgr_Init(struct TMData *tmd) {
    struct TMgr_Globals *tmg = NULL;
    extern struct TextAttr topaz8;
    
    if (tmg = AllocVec(sizeof(struct TMgr_Globals),MEMF_CLEAR)) {
        tmd->UserData = tmg;
        if (tmg->mp = createClientPort(tmg->portName)) {
            if (tmg->tmgr_font = OpenFont(&topaz8)) {
                return(TRUE);
            }
        }
        FreeVec(tmg);
        tmd->UserData = NULL;
    }
    return(FALSE);
}

/* TMgr_Free ================================================================= */
VOID TMgr_Free(struct TMData *tmd) {
    struct Message *msg = NULL;
    struct errReport *er;
    struct TMgr_Globals *t;

    t = (struct TMgr_Globals *)(tmd->UserData);
    if (t == NULL)
        return;

    /* Clean up our message port */
    if (t->mp) {
        while (msg = GetMsg(t->mp)) {
            er = (struct errReport *)msg;
            er->returnCmd = TMCMD_ABORT;  /* tell clients to GO AWAY! */
            ReplyMsg(msg);
        }
        Forbid();
        DeletePort(t->mp);
        Permit();
    }

    /* LogFile handle */
    if (t->fh) {
        Close(t->fh);
        t->fh = NULL;
    }

    /* Console Filehandle */
    if (t->cfh) {
        Close(t->cfh);
        t->cfh = NULL;
    }

    /* window's font */
    if (t->tmgr_font)
        CloseFont(t->tmgr_font);

    /* it's time to come clean if we've been lying to ToolMaker about 
       having a screen that it supposedly opened.
       We have a pointer that we got from LockPubScreen(), and we told
       ToolMaker that said pointer was really obtained from its own
       OpenScreen() call.  Not so.  We Unlocked the pubscreen when we
       opened the window; all we have to do is fix the lie here.
     */
    if (t->useWB)
        tmd->TMScreenInfo[0].Screen = NULL;

    /* Our globals */
    FreeVec(t);
    t = NULL;
    tmd->UserData = NULL;  /* catch any _more_ problems :-) */
    return;
}

/* getArgs ====================================================================
   Gets default arguments from tool's icon or from cli cmdline
   WB Magic words:
      NOSER, USEPAR, USECON, LOGFILE, ERRABORT, PRIORITY, ERRLEV, DEBUG, VERBOSE
*/
BOOL getArgs(struct TMData *TMData, int argc, UBYTE *argv[]) {
    struct TMgr_Globals *t;
    LONG tmpLong;
    BOOL rc = TRUE;
    char *foo = NULL;  /* for strtol() use only */

    t = (struct TMgr_Globals *)(TMData->UserData);
    D("Within getArgs()\n");
    /* Common initialization */
    t->okMsg = LVLM_NORM;               /* tmgr_common.h               */
    t->earlyExit = FALSE;               /* don't exit on first error   */
    t->carryOn = TRUE;                  /* do tell clients to continue */
    t->outputs = OUTF_SER;
    t->clientCount = 0;
    t->testStatus = PASS_TEST;
    t->earlyExit = FALSE;
    t->errCount = 0;
    t->useWB = FALSE;  /* use custom screen instead */

   /* ----------------- useful parts -------------- */
    if (argc == 0) {                   /* Workbench */
        UBYTE **ttypes = NULL, *tmp;

    D("getArgs() Workbench approach\n");
        if (IconBase = OpenLibrary(LIBNAME_ICONLIB, 37L)) {
         if (CxBase = OpenLibrary(LIBNAME_CXLIB, 37L)) {
            if (ttypes = ArgArrayInit((LONG)argc, argv)) {
                if (tmp = FindToolType(ttypes, TT_USEWB)) {
                    t->useWB = TRUE;
                }
                if (tmp = FindToolType(ttypes, TT_NOSER)) {
                    t->outputs &= ~((UBYTE)(OUTF_SER));
                }
                if (tmp = FindToolType(ttypes, TT_USEPAR)) {
                    t->outputs |= OUTF_PAR;
                }
                if (tmp = FindToolType(ttypes, TT_USECON)) {
                    t->outputs |= OUTF_CON;
                    stccpy(t->conName, DEFAULT_CONSPEC, MAX_FNAME_LEN);
                }
                if (tmp = FindToolType(ttypes, TT_LOGFILE)) {
                    t->outputs |= OUTF_FILE;
                    if (strlen(tmp) > stccpy(t->fileName, tmp, MAX_FNAME_LEN)) {
                        TM_Request(NULL,"TstMgr","Invalid Output File:\n'%s'\n","Exit",NULL,t->fileName);
                        rc = FALSE;
                    }
                }
                if (tmp = FindToolType(ttypes, TT_ERRABORT)) {
                    t->earlyExit = TRUE;
                }
                if (tmp = FindToolType(ttypes, TT_PRIORITY)) {
                    tmpLong = strtol(tmp,&foo,0);
                    if (*foo != '\0') {
                        TM_Request(NULL,"TstMgr","Invalid Priority, using\n'%ld'\n","Proceed",NULL,(APTR)DEFAULT_SERV_PRI);
                        SetTaskPri(FindTask(NULL), DEFAULT_SERV_PRI);
                    }
                    else {
                        SetTaskPri(FindTask(NULL), tmpLong);
                    }
                }
                else {
                    SetTaskPri(FindTask(NULL), DEFAULT_SERV_PRI);
                }
                
                if (tmp = FindToolType(ttypes, TT_ERRLEV)) {
                    tmpLong = strtol(tmp, &foo, 0);
                    if (*foo == '\0') {
                        switch (tmpLong) { 
                            case 1:
                                t->okMsg &= ~((LONG)(LVL_ERR2));
                                break;
                            case 2:
                                t->okMsg |= LVL_ERR2;
                                break;
                            default:
                                TM_Request(NULL,"TstMgr","Invalid ErrLevel '%s'! Ignored.\n","Proceed",NULL,(APTR)tmp);
                                break;
                        }
                    }
                    else
                        TM_Request(NULL,"TstMgr","Invalid ErrLevel '%s'! Ignored.\n","Proceed",NULL,(APTR)tmp);
                }
                if (tmp = FindToolType(ttypes, TT_VERBOSE)) {   /* a fall through! */
                    tmpLong = strtol(tmp, &foo, 0);
                    if (*foo == '\0') {
                        switch (tmpLong) { 
                            case 3:
                                t->okMsg |= LVL_VB3;  /* intentional: no 'break;' */
                            case 2:
                                t->okMsg |= LVL_VB2;  /* intentional: no 'break;' */
                            case 1:
                                t->okMsg |= LVL_VB1;
                                break;
                            default:
                                TM_Request(NULL,"TstMgr","Invalid VerboseLevel '%s'! Ignored.\n","Proceed",NULL,(APTR)tmp);
                                break;
                        }
                    }
                    else
                       TM_Request(NULL,"TstMgr","Invalid VerboseLevel '%s'! Ignored.\n","Proceed",NULL,(APTR)tmp);
                }
                if (tmp = FindToolType(ttypes, TT_DEBUG)) {   /* a fall through! */
                    tmpLong = strtol(tmp, &foo, 0);
                    if (*foo == '\0') {
                        switch (tmpLong) { 
                            case 3:
                                t->okMsg |= LVL_DB3;  /* intentional: no 'break;' */
                            case 2:
                                t->okMsg |= LVL_DB2;  /* intentional: no 'break;' */
                            case 1:
                                t->okMsg |= LVL_DB1;
                                break;
                            default:
                                TM_Request(NULL,"TstMgr","Invalid DebugLevel '%ld'! Ignored.\n","Proceed",NULL,(APTR)tmpLong);
                                break;
                        }
                    }
                }
                ArgArrayDone();
            }
            CloseLibrary(CxBase);
          } /* cxbase */
          else {
            rc = FALSE;
            TM_Request(NULL,"TstMgr","Can't open %s V37!\n","Exit",NULL,LIBNAME_CXLIB);
          }
          CloseLibrary(IconBase);
        } /* IconBase */
        else {
          rc = FALSE;
          TM_Request(NULL,"TstMgr","Can't open %s V37!\n","Exit",NULL,LIBNAME_ICONLIB);
        }
    } /* Workbench */
    else {  /* CLI */
        LONG Opts[NUM_OPTS];
        struct RDArgs *rdargs = NULL;

        D("getArgs() CLI approach\n");
        memset(Opts,0,sizeof(Opts));
        if (rdargs = ReadArgs(TEMPLATE,Opts,NULL)) {
            if (Opts[OPT_USEWB]) {
                t->useWB = TRUE;
            }
            if (Opts[OPT_NOSER]) {
                t->outputs &= ~((UBYTE)(OUTF_SER));
            }
            if (Opts[OPT_USEPAR]) {
                t->outputs |= OUTF_PAR;
            }
            if (Opts[OPT_USECON]) {
                t->outputs |= OUTF_CON;
                stccpy(t->conName, DEFAULT_CONSPEC, MAX_FNAME_LEN);
            }
            if (Opts[OPT_LOGFILE]) {
                t->outputs |= OUTF_FILE;
                if (strlen((STRPTR)Opts[OPT_LOGFILE]) > stccpy(t->fileName, (STRPTR)Opts[OPT_LOGFILE], MAX_FNAME_LEN)) {
                    PutStr("Error: filename too long!\n");
                    rc = FALSE;
                }
            }
            t->earlyExit = ((Opts[OPT_ERRABORT]) ? TRUE : FALSE);

            if (Opts[OPT_PRIORITY]) {
                SetTaskPri(FindTask(NULL), *(LONG *)Opts[OPT_PRIORITY]);
            }
            else {
                SetTaskPri(FindTask(NULL), DEFAULT_SERV_PRI);
            }
            
            if (Opts[OPT_ERRLEV]) {
                switch (*(LONG *)Opts[OPT_ERRLEV]) { 
                    case 1:
                        t->okMsg &= ~((ULONG)(LVL_ERR2));
                        break;
                    case 2:
                        t->okMsg |= LVL_ERR2;
                        break;
                    default:
                        PutStr("Invalid ErrLevel! Ignored.\n");
                        break;
                }
            }
            if (Opts[OPT_VERBOSE]) {   /* a fall through! */
                switch (*(LONG *)Opts[OPT_VERBOSE]) { 
                    case 3:
                        t->okMsg |= LVL_VB3;  /* intentional: no 'break;' */
                    case 2:
                        t->okMsg |= LVL_VB2;  /* intentional: no 'break;' */
                    case 1:
                        t->okMsg |= LVL_VB1;
                        break;
                    default:
                        PutStr("Invalid VerboseLevel! Ignored.\n");
                        break;
                }                    
            }
            if (Opts[OPT_DEBUG]) {   /* a fall through! */
                switch (*(LONG *)Opts[OPT_DEBUG]) { 
                    case 3:
                        t->okMsg |= LVL_DB3;  /* intentional: no 'break;' */
                    case 2:
                        t->okMsg |= LVL_DB2;  /* intentional: no 'break;' */
                    case 1:
                        t->okMsg |= LVL_DB1;
                        break;
                    default:
                        PutStr("Invalid DebugLevel! Ignored.\n");
                        break;
                }                    
            }

            FreeArgs(rdargs);
        }
        else {
            PrintFault(IoErr(),NULL);
            rc = FALSE;
        }
    } /* cli */

    D("Did ReadArgs()\n");

    /* Also open the logfile if we need to */
    if (t->outputs & OUTF_FILE) {
        D("Setting up OUTF_FILE\n");
        t->fh = Open(t->fileName,MODE_NEWFILE);
        if (t->fh == NULL) {
            rc = FALSE;
            if (argc) {
                TM_Request(NULL,"TstMgr","Can't open LogFile '%s'!\n","Exit",NULL,t->fileName);
            }
            else {
                PrintFault(IoErr(),NULL);
                Printf("Can't open file '%s'!\n",t->fileName);
            }
        }
        D("Set up OUTF_FILE\n");
    }

    /* Also open the console logfile if we need to */
    if (t->outputs & OUTF_CON) {
        D("Setting up OUTF_CON...\n");
        t->cfh = Open(t->conName,MODE_NEWFILE);
        if (t->cfh == NULL) {
            rc = FALSE;
            if (argc) {
                TM_Request(NULL,"TstMgr","Can't open Console '%s'!\n","Exit",NULL,t->conName);
            }
            else {
                PrintFault(IoErr(),NULL);
                Printf("Can't open Console '%s'!\n",t->fileName);
            }
            if (t->fh) {
                Close(t->fh);
                t->fh = NULL;
            }
        }
    D("Set up OUTF_CON\n");
    }
    if (rc)
        D("getArgs(): Returning TRUE\n");
    else
        D("getArgs(): Returning FALSE\n");
    return(rc);
}

/* blueRefreshKludge ==================================================================
   This function is a kludge to change the colors of a window on the workbench, without
   screwing up the GadTools gadgets.  There is apparently no legal way to do this
   under V37.
*/
void blueRefreshKludge(struct Window *win, struct Gadget *gad) {

    SetDrMd(win->RPort, COMPLEMENT);
    RectFill(win->RPort, 
             win->BorderLeft,   /* xmin */
             win->BorderTop,    /* ymin */
             win->Width - (win->BorderLeft + win->BorderRight),
             win->Height - (abs(win->BorderBottom) + 1)); 

/*          
        SetRast(win->RPort, 3);
        GT_BeginRefresh(win);
        GT_EndRefresh(win, TRUE);
        RefreshWindowFrame(win); 
*/
    /* might consider DisableWindow_ERRWINDO(tmd) and EnableWindow... */

    RefreshGadgets(gad, win, NULL);  /* highly illegal     */
    GT_RefreshWindow(win, NULL);     /* not strictly legal */

/*          SetDrMd(win->RPort, JAM2); */
    return;
}

/* blankString =====================================================
   s = string which should have blanks up til null
   n = size of said string.
   takes 'foo\0.......' and produces
         'foo       \0'
*/
void blankString(STRPTR str, ULONG n) {
    STRPTR s1, s2;

    s1 = str;                /* initialize */
    s2 = &(str[n-1]);        /* string's end */

    *s2 = '\0';  /* null-terminate regardless */

    while (*s1++)
        ;  /* get to the null character */

    s1--;  /* get set to nuke said null if we're not at string's end */

    while (s1 != s2)
        *s1++ = ' ';  /* make everything a space til we hit string's end */

    return;
}


