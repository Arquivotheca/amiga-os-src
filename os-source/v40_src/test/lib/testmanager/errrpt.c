/* ErrRpt.c - Functions for dealing with TestManager.
   Written for AmigaDOS 2.x
   Version 1.01, 17 May 92, J.W. Lockhart
   Slight rev for name changes (from ErrMgr to TstMgr, etc.)
   Functions:
        initTstData() -- initialize test data
        freeTstData() -- free test data
        errRpt()      -- report test info to Test Manager
*/

/* Includes ------------------------------------------------------------------ */
#ifndef TMGR_COMMON_H
#include "tmgr_common.h"
#endif

#ifndef MAKE_PROTOS
#include "errRpt_protos.h"
#endif

/* Protos for internal functions */
static BOOL do_ConfigRpt(struct errReport *er, UWORD el, BYTE et, LONG ec);
static BYTE sendRpt(struct errReport *er);

/* Pragmas ------------------------------------------------------------------- */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>


/****** TestManager.library/initTstData ******************************************
*
*   NAME
*      initTstData - initialize test data-reporting mechanism
*
*   SYNOPSIS
*      tstData = initTstData(my_name, testMgrName)
*
*      struct errReport *initTstData(STRPTR, STRPTR);
*
*   FUNCTION
*      Allocates a struct errReport for you and properly
*      initializes it.  This includes forming a unique name for
*      your program, creating a MsgPort, and setting up reasonable
*      defaults.  It also puts a startup message into the testlog.
*
*   INPUTS
*      my_name - a template to use to create a unique program name.
*                This should be the same as the name of your test
*                program.  If you supply NULL for this value, you
*                will be known as "Test_Prog".
*    tstMgrName- Name of the test manager program you would like
*                to talk to.  This is often NULL, in which case the
*                default, "TestMgr", is used.
*
*   RESULT
*     tstData - a pointer to a struct errReport which you may use
*               with calls to errRpt().  THIS STRUCTURE MUST BE
*               FREED BY A MATCHING CALL TO freeTstData()!!!
*               Returns NULL for failure, in which case you should
*               exit, as your test will be useless without any
*               results displayed...
*
*   EXAMPLE
*      struct errReport *test;
*      BYTE rc;
*      int i;
*
*      if (test = initTstData("MyTest",NULL)) {
*         for (i = 0; i < MAX_TESTS; i++) {
*            \* ... perform test... *\
*           if (test_ok) {
*               \* report progress *\
*               rc = errRpt(test, LVL_PROG, PROB_NONE, NULL,
*                       "Test %ld went OK\n", (LONG) i);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test); \* must freeTstData(test) ! *\
*           }
*           else {
*               \* report error *\
*               rc = errRpt(test, LVL_ERR, PROB_IOERR, IoErr(),
*                       "Test %ld failed: %s\n", (LONG)i, MY_ERR_MSG);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test);  \* must freeTstData(test) ! *\
*           }
*         }  
*         freeTstData(test);
*      }
*
*   NOTES
*    - An invocation of TestManager MUST be running before this
*      function is called, else it will (gracefully) fail.
*    - This function MUST be called before any other TestManager.library
*      functions, otherwise the other functions WILL (gracefully) fail.
*    - freeTstData() MUST be called on exit, else you will drip RAM.
*    - This function will request an ID number from the TestManager,
*      and form the unique name from that plus the name you supply.
*    - As of V37.4 ErrReport.lib, default values will be set up so
*      you can use quickRpt() right away.  These values are:
*      LVL_PROG (progress report), PROB_NONE (no problem), 0 (no error).
*
*   BUGS
*     None.
*
*   SEE ALSO
*     tmgr_common.h, errRpt(), freeTstData()
*
******************************************************************************
* ====================== initTstData ========================================
*/
/*      if (test = initTstData("MyTest",NULL)) { */
struct errReport *initTstData(STRPTR clientName, STRPTR serverName) {
    struct errReport *er = NULL;
    STRPTR servName, cliName;
    struct MsgPort *sp = NULL;  /* server message port */
    struct errReport *er2 = NULL;


    /* set up server name */
    if (!serverName)
        servName = SERVER_NAME;
    else
        servName = serverName;


    /* fail if the server's not running */
    Forbid();
    if (!(sp = FindPort(servName))) {
        Permit();
        return(NULL);
    }
    Permit();

    /* try to set up an errRequest */
    if (er = AllocVec(sizeof(struct errReport), MEMF_PUBLIC|MEMF_CLEAR)) {
        if (er->mp = CreatePort(NULL,0L)) {
            er->msg.mn_Node.ln_Type = NT_MESSAGE;
            er->msg.mn_Length = sizeof(er);
            er->msg.mn_ReplyPort = er->mp;

            /* fill in relevant ID-request info */
            er->errLevel = LVL_INT;
            er->problem  = PROB_NEED_ID;

            /* DateStamp just before sending */
            er->dt.dat_Format = FORMAT_DOS;
            DateStamp(&(er->dt.dat_Stamp));    

            /* Find the serverport and send the message */
            /* This is a very paranoid and conservative approach */
            Forbid();
                if ((sp = FindPort(servName)) != NULL)
                    PutMsg(sp, (struct Message *)er);
            Permit();

            /* Check to see that we found the server */
            if (!sp) {
                /* We didn't send anything */
                DeletePort(er->mp);
                FreeVec(er);
                er = NULL;
                return(NULL);
            }

            /* Wait for our reply, and create a name with it... */
            WaitPort(er->mp);
            er2 = (struct errReport *)GetMsg(er->mp);
            if (!clientName) {
                cliName = DEFAULT_CLIENT_NAME;
            }
            else {
                cliName = clientName;
            }
            bsprintf(er->clientName,"%s_%ld", (LONG)cliName, er2->errCode);

            /* Send startup message */
            if (RPT_OK != errRpt(er, LVL_PROG, PROB_NONE, 0, "Starting up...\n", 0L)) {
                freeTstData(er);
                return(NULL);
            }
        }
        else {
            FreeVec(er);
            return(NULL);
        }
        do_ConfigRpt(er, LVL_PROG, PROB_NONE, 0);  /* use reasonable defaults for quickRpt() */
        return(er);
    }
    return(NULL);
}

/****** TestManager.library/freeTstData ******************************************
*
*   NAME
*     freeTstData - free errReport created by initTstData()
*
*   SYNOPSIS
*     freeTstData(test)
*
*     void freeTstData(struct errReport *test);
*
*   FUNCTION
*     Sends exit message to Test Manager, and
*     frees all allocations made by initTstData().
*     Message port is, of course, cleaned up properly.
*
*   INPUTS
*     test - struct errReport allocated by initTstData().
*            Passing NULL is not an error; freeTstData will merely return
*            without attempting to do anything.
*
*   RESULT
*     Exit message is sent, MsgPort is cleaned up and deleted, and
*     all allocations made by initTstData() are freed.  Pointer passed in
*     is no longer valid, and is set to NULL.
*
*   EXAMPLE
*      if (RPT_OK != errRpt(er, LVL_PROG, PROB_NONE, 0, 
*                               "Starting up...\n", 0L)) 
*      {
*          freeTstData(er);
*          return(NULL);
*      }
*
*     Also, see initTstData().
*
*   NOTES
*     Your pointer is NULLed by this function.
*
*   BUGS
*     None.
*
*   SEE ALSO
*     initTstData(), tmgr_common.h.
*
******************************************************************************
* ======================== freeTstData =======================================
*/
void freeTstData(struct errReport *er) {
    if (!er)
        return;
    errRpt(er, LVL_EXIT, PROB_NONE, 0, "Exiting...\n", 0L);
    while (GetMsg(er->mp))
        ;  /* do nothing */
    DeletePort(er->mp);
    FreeVec(er);
    er = NULL;
    return;
}



/****** TestManager.library/errRpt ******************************************
*
*   NAME
*      errRpt - log an error, progress, informational, or debugging message
*
*   SYNOPSIS
*      error = errRpt(ErrReport, errLevel, errType, errCode, fmt, arg1, ...)
*
*      BYTE errRpt(struct errReport *, UWORD, BYTE, LONG, STRPTR, LONG, ...);
*
*   FUNCTION
*      Sends information off to test manager task.  This includes
*      a string for the user to see, codes describing the problem,
*      a code describing the severity of the problem (if any).
*
*   INPUTS
*      errReport- pointer to structure previously initialized by
*                 initTstData().  If you pass NULL, no report will
*                 be sent and you will receive RPT_NO_SERV back.
*      errLevel - see tmgr_common.h.  This describes what your message
*                 means to the user:  error or progress report,
*                 additional information, or even debugging information.
*                 Currently defined values:
*                    LVL_PROG for a progress report;
*                    LVL_ERR  for an error message;
*                    LVL_ERR2 for addtional error information;
*                    LVL_VB1  for optional information (VB2 and VB3 as well);
*                    LVL_DB1  for debugging information (DB2 and DB3 as well).
*                 Note that LVL_ERR or LVL_ERR2 will cause the Test Manager
*                 to display a "Test Failed!" message, with appropriate
*                 bells and whistles.
*      errType  - describes what errCode means.  
*                    PROB_NONE   for no problem;
*                    PROB_IOERR  for a standard IoErr()-type error code;
*                    PROB_CUSTOM for a custom error code (strongly
*                                discouraged!).
*      errCode  - a numerical description of your problem.
*      fmt      - printf-style formatting string of the type described
*                 in exec.library/RawDoFmt().
*      arg1     - first of the arguments to plug into the fmt string,
*                 or NULL.
*
*   RESULT
*      error    - described in tmgr_common.h.  If you get anything other
*                 than RPT_OK back, you should exit immediately after
*                 freeing your resources.  Other possible returns
*                 are RPT_ABORT if the TestManager has requested that
*                 you abort, or RPT_NO_SERV if the TestManager has
*                 exited.  This permits cute games to be played,
*                 but such practice is discouraged, especially if 
*                 RPT_NO_SERV is encountered.
*
*   EXAMPLE
*      struct errReport *test;
*      BYTE rc;
*      int i;
*
*      if (test = initTstData("MyTest",NULL)) {
*         for (i = 0; i < MAX_TESTS; i++) {
*           \* ... perform test... *\
*           if (test_ok) {
*               \* report progress *\
*               rc = errRpt(test, LVL_PROG, PROB_NONE, NULL,
*                       "Test %ld went OK\n", (LONG) i);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test); \* must freeTstData(test) ! *\
*           }
*           else {
*               \* report error *\
*               rc = errRpt(test, LVL_ERR, PROB_IOERR, IoErr(),
*                       "Test %ld failed: %s\n", (LONG)i, MY_ERR_MSG);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test);  \* must freeTstData(test) ! *\
*           }
*         }  
*         freeTstData(test);
*      }
*
*   NOTES
*      Messages are automatically time-stamped, identified, and
*      reported according to the settings of the Test Manager.
*      As of V37.4 ErrReport.lib, this preserves the report configuration
*      found in the struct errReport prior to the call.  This is so you
*      can use quickRpt() for humdrum messages and errRpt() for the
*      occasional problem.
*
*   BUGS
*      None yet.
*
*   SEE ALSO
*      tmgr_common.h, initTstData(), freeTstData()
*
******************************************************************************
* =============================== errRpt =====================================
*/
BYTE errRpt(struct errReport *er, UWORD eL, BYTE eT, LONG eC, STRPTR fmt, LONG arg1, ...) {
    UWORD orig_eL;
    BYTE  orig_eT, retCode;
    LONG  orig_eC;

    if (!er)
        return(RPT_NO_SERV);

    orig_eL = er->errLevel;
    orig_eT = er->problem;
    orig_eC = er->errCode;

    if (!do_ConfigRpt(er, eL, eT, eC))
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = &arg1;

    retCode = sendRpt(er);

    do_ConfigRpt(er, orig_eL, orig_eT, orig_eC);
    return(retCode);
}

/****** TestManager.library/configRpt ******************************************
*
*   NAME
*      configRpt - configure default test data information for
*                  use with TestManager.library/quickRpt()
*
*   SYNOPSIS
*      success = configRpt(ErrReport, errLevel, errType, errCode)
*
*      BOOL configRpt(struct errReport *, UWORD, BYTE, LONG);
*
*   FUNCTION
*       configRpt() allows you to set up a lot of progress reports
*       without having to continually repeat parameters, as you
*       would have to with errRpt().  This lets you skip three
*       parameters so that you can use quickRpt() as you would
*       fprintf():  quickRpt(ErrReport, fmt, arg, ...);
*
*   INPUTS
*      errReport- pointer to structure previously initialized by
*                 initTstData().  If you pass NULL, you will
*                 receive FALSE back.
*      errLevel - see tmgr_common.h.  This describes what your message
*                 means to the user:  error or progress report,
*                 additional information, or even debugging information.
*                 Currently defined values:
*                    LVL_PROG for a progress report;
*                    LVL_ERR  for an error message;
*                    LVL_ERR2 for addtional error information;
*                    LVL_VB1  for optional information (VB2 and VB3 as well);
*                    LVL_DB1  for debugging information (DB2 and DB3 as well).
*                 Note that LVL_ERR or LVL_ERR2 will cause the Test Manager
*                 to display a "Test Failed!" message, with appropriate
*                 bells and whistles.
*      errType  - describes what errCode means.  
*                    PROB_NONE   for no problem;
*                    PROB_IOERR  for a standard IoErr()-type error code;
*                    PROB_CUSTOM for a custom error code (strongly
*                                discouraged!).
*      errCode  - a numerical description of your problem.
*
*   RESULT
*      success    - TRUE (1) if successful, FALSE (0) if not.  Since
*                   the only way you'll get FALSE is if you pass
*                   NULL for ErrReport, you shouldn't have to worry.
*
*   EXAMPLE
*      struct errReport *test;
*      BYTE rc;
*      int i;
*
*      if (test = initTstData("MyTest",NULL)) {
*         configRpt(test, LVL_PROG, PROB_NONE, NULL);
*         for (i = 0; i < MAX_TESTS; i++) {
*           \* ... perform test... *\
*           if (test_ok) {
*               \* report progress *\
*               rc = quickRpt(test, "Test %ld went OK\n", (LONG) i);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test); \* must freeTstData(test) ! *\
*           }
*           else {
*               \* report error *\
*               rc = errRpt(test, LVL_ERR, PROB_IOERR, IoErr(),
*                       "Test %ld failed: %s\n", (LONG)i, MY_ERR_MSG);
*               if (rc != RPT_OK)
*                  CleanUpAndExit(test);  \* must freeTstData(test) ! *\
*           }
*         }  
*         freeTstData(test);
*      }
*
*   NOTES
*    - Messages are automatically time-stamped, identified, and
*      reported according to the settings of the Test Manager.
*    - Information stored by configRpt() is NOT altered by errRpt().
*
*   BUGS
*      None yet.
*
*   SEE ALSO
*      tmgr_common.h, initTstData(), freeTstData(), quickRpt()
*
******************************************************************************
* ====================== configRpt ========================================
*  Actually uses internal do_ConfigRpt() routine, also used by ErrRpt().
*/

BOOL configRpt(struct errReport *er, UWORD el, BYTE et, LONG ec) {
    return(do_ConfigRpt(er, el, et, ec));
}

/* Internal do_ConfigRpt() function which is called by both configRpt and
   by errRpt().
*/
static BOOL do_ConfigRpt(struct errReport *er, UWORD el, BYTE et, LONG ec) {
    if (!er)
        return(FALSE);
   
    if (!(er->mp))
        return(FALSE);  /* not strictly true, but close enough */

    er->msg.mn_Node.ln_Type = NT_MESSAGE;
    er->msg.mn_Length = sizeof(er);
    er->msg.mn_ReplyPort = er->mp;

    /* fill in logfile message */
    er->errLevel = el;
    er->problem  = et;
    if (et != PROB_NONE) {
        er->errCode = ec;
    }
    else {
        er->errCode = 0L;
    }
    return(TRUE);
}

/* sendRpt =============================================================
 * internal routine called by errRpt() and by quickRpt() 
 * Performs datestamp and sends message off to server.
 * ALL fields of struct errReport *er MUST be properly filled in
 * beforehand.
 */
static BYTE sendRpt(struct errReport *er) {
    struct MsgPort *sp = NULL;
    struct Message *msg = NULL;
    STRPTR servName;

    /* DateStamp just before sending */
    er->dt.dat_Format = FORMAT_DOS;
    DateStamp(&(er->dt.dat_Stamp));    

    /* Set up to talk to proper Test Manager */
    if (er->serverName[0] != '\0')
        servName = er->serverName;
    else
        servName = SERVER_NAME;

    /* Find the serverport and send the message */
    /* This is a very paranoid and conservative approach */
    Forbid();
        if ((sp = FindPort(servName)) != NULL)
            PutMsg(sp, (struct Message *)er);
    Permit();

    /* Check to see that we found the server */
    if (!sp) {
        /* We didn't send anything */
        return(RPT_NO_SERVER);
    }

    /* Wait for our reply, but do nothing with it... */
    WaitPort(er->mp);
    msg = GetMsg(er->mp);

    if (er->returnCmd == TMCMD_ABORT)
        return(RPT_ABORT);
    return(RPT_OK);
}

/****** TestManager.library/quickRpt ******************************************
*
*   NAME
*      quickRpt - log another message of the same type previously reported.
*
*   SYNOPSIS
*      error = quickRpt(ErrReport, fmt, arg1, ...)
*
*      BYTE quickRpt(struct errReport *, STRPTR, LONG, ...);
*
*   FUNCTION
*      Sends information off to test manager task.  This includes
*      a string for the user to see, codes describing the problem,
*      a code describing the severity of the problem (if any).
*      Previous ErrLevel, ErrCode, and ErrType values are used.
*      These values are whatever has been stored in your 
*      ErrReport structure, either by configRpt() or initTstData().
*      Note that errRpt() is not affected by, and does not affect,
*      quickRpt() ErrLevel, ErrCode, or ErrType values.
* 
*   INPUTS 
*      errReport- pointer to structure previously initialized by 
*                 initTstData().  If you pass NULL, no report will 
*                 be sent and you will receive RPT_NO_SERV back. 
*      fmt      - printf-style formatting string of the type described 
*                 in exec.library/RawDoFmt(). 
*      arg1     - first of the arguments to plug into the fmt string, 
*                 or NULL. 
* 
*   RESULT 
*      error   - described in tmgr_common.h.  If you get anything other 
*                than RPT_OK back, you should exit immediately after 
*                freeing your resources.  Other possible returns 
*                are RPT_ABORT if the TestManager has requested that 
*                you abort, or RPT_NO_SERV if the TestManager has 
*                exited.  This permits cute games to be played, 
*                but such practice is discouraged, especially if 
*                RPT_NO_SERV is encountered. 
* 
*   EXAMPLE 
*   struct errReport *test; 
*      BYTE rc; 
*      int i; 
* 
*      if (test = initTstData("MyTest",NULL)) { 
*         for (i = 0; i < MAX_TESTS; i++) { 
*         \* ... perform test... *\ 
*           if (test_ok) { 
*               \* report progress *\ 
*               \* default is LVL_PROG, PROB_NONE, 0 *\
*               rc = quickRpt(test, "Test %ld went OK\n", (LONG) i); 
*               if (rc != RPT_OK) 
*                  CleanUpAndExit(test); \* must freeTstData(test) ! *\ 
*           } 
*           else { 
*               \* report error, no effect on quickRpt() params *\ 
*               rc = errRpt(test, LVL_ERR, PROB_IOERR, IoErr(), 
*                          "Test %ld failed: %s\n", (LONG)i, MY_ERR_MSG); 
*               if (rc != RPT_OK) 
*                 CleanUpAndExit(test);  \* must freeTstData(test) ! *\ 
*           } 
*         } 
*         freeTstData(test); 
*      } 
* 
*   NOTES 
*      Messages are automatically time-stamped, identified, and 
*      reported according to the settings of the Test Manager. 
* 
*   BUGS 
*      None yet. 
* 
*   SEE ALSO 
*   tmgr_common.h, initTstData(), freeTstData(), configRpt(), errRpt()
*
****************************************************************************** 
* =============================== quickRpt =====================================
*/ 
BYTE quickRpt(struct errReport *er, STRPTR fmt, LONG arg1, ...) {

    if (!er)
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = &arg1;

    return(sendRpt(er));
}



/****** TestManager.library/quickARpt ******************************************
*
*   NAME
*      quickARpt - log another message of the same type previously reported.
*
*   SYNOPSIS
*      error = quickARpt(ErrReport, fmt, arg1, ...)
*
*      BYTE quickARpt(struct errReport *, STRPTR, APTR, ...);
*
*   FUNCTION
*      This is exactly the same as quickRpt(), except it takes
*      the address of the first argument.
*
*      The intended use of this function is to use it within a varargs
*      function of the following form:
*         VOID myRpt(STRPTR fmt, LONG arg1, ...) {
*           \* relies on global struct ErrReport *er *\
*           if (RPT_OK != quickARpt(er, fmt, &arg1)) {
*              \* anything else you wish to do on failure... *\
*              cleanExit();
*           }
*         }
*
*   INPUTS 
*      errReport- pointer to structure previously initialized by 
*                 initTstData().  If you pass NULL, no report will 
*                 be sent and you will receive RPT_NO_SERV back. 
*      fmt      - printf-style formatting string of the type described 
*                 in exec.library/RawDoFmt(). 
*      arg1     - pointer to the first of the arguments to plug into the 
*                 fmt string, or NULL.  Note that additional args must
*                 follow the first on the stack (as above, in myRpt()).
* 
*   RESULT 
*      error   - described in tmgr_common.h.  If you get anything other 
*                than RPT_OK back, you should exit immediately after 
*                freeing your resources.  Other possible returns 
*                are RPT_ABORT if the TestManager has requested that 
*                you abort, or RPT_NO_SERV if the TestManager has 
*                exited.  This permits cute games to be played, 
*                but such practice is discouraged, especially if 
*                RPT_NO_SERV is encountered. 
* 
*   EXAMPLE 
*      See quickRpt().
* 
*   NOTES 
*      Messages are automatically time-stamped, identified, and 
*      reported according to the settings of the Test Manager. 
* 
*   BUGS 
*      None yet. 
* 
*   SEE ALSO 
*   tmgr_common.h, initTstData(), freeTstData(), configRpt(), errRpt(), 
*   quickRpt()
*
****************************************************************************** 
* =============================== quickARpt =====================================
*/ 
BYTE quickARpt(struct errReport *er, STRPTR fmt, APTR arg1, ...) {

    if (!er)
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = arg1;

    return(sendRpt(er));
}




/****** TestManager.library/forceRpt ******************************************
*
*   NAME
*      forceRpt - log another message of the same type previously reported.
*                 Forces output of one form or another...
*
*   SYNOPSIS
*      error = forceRpt(ErrReport, fmt, arg1, ...)
*
*      BYTE forceRpt(struct errReport *, STRPTR, LONG, ...);
*
*   FUNCTION
*      This is exactly the same as quickRpt(), except that on failure,
*      it will attempt to use dos.library/VPrintf() so that you
*      get some form of output.  Of course, it helps if Output() is
*      valid...
*
*
*   INPUTS 
*      errReport- pointer to structure previously initialized by 
*                 initTstData().  If you pass NULL, no report will 
*                 be sent, VPrintf() will be attempted, and you will 
*                 receive RPT_NO_SERV back. 
*      fmt      - printf-style formatting string of the type described 
*                 in exec.library/RawDoFmt(). 
*      arg1     - the first of the arguments to plug into the 
*                 fmt string, or NULL.
* 
*   RESULT 
*      error   - described in tmgr_common.h.  
*                Note that you should check for RPT_ABORT as a return,
*                in which case there IS a running server and the
*                tester wants you to QUIT, and QUIT NOW.
*                Other possible returns are RPT_OK or RPT_NO_SERV.
* 
*   EXAMPLE 
*      See quickRpt().
* 
*   NOTES 
*      Messages are automatically time-stamped, identified, and 
*      reported according to the settings of the Test Manager. 
*      If the Test Manager isn't running, you get no ID or time-stamping,
*      but you will get the equivalent of Printf(fmt, arg1, ...) instead.
* 
*   BUGS 
*      None yet. 
* 
*   SEE ALSO 
*   tmgr_common.h, initTstData(), freeTstData(), configRpt(), errRpt(), 
*   quickRpt()
*
****************************************************************************** 
* =============================== forceRpt =====================================
*/ 
BYTE forceRpt(struct errReport *er, STRPTR fmt, LONG arg1, ...) {
    BYTE err;

    if (!er) {
        VPrintf(fmt, &arg1);
        return(RPT_NO_SERV);
    }

    er->logMsg = fmt;
    er->varArgs = &arg1;

    err = sendRpt(er);
    if (err != RPT_OK) {
        VPrintf(fmt, &arg1);
    }

    return(err);
}

/****** TestManager.library/forceARpt ******************************************
*
*   NAME
*      forceARpt - log another message of the same type previously reported.
*                  Forces output of one form or another...
*
*   SYNOPSIS
*      error = forceARpt(ErrReport, fmt, arg1, ...)
*
*      BYTE forceARpt(struct errReport *, STRPTR, APTR, ...);
*
*   FUNCTION
*      This is exactly the same as forceRpt(), except it takes
*      the address of the first argument.  If TstMgr isn't running,
*      forceARpt() attempts to call dos.library/VPrintf() to give you
*      some form of output.  You should have a valid Output() before
*      using this function.
*
*      The intended use of this function is to use it within a varargs
*      function of the following form:
*         VOID myRpt(STRPTR fmt, LONG arg1, ...) {
*           \* relies on global struct ErrReport *er *\
*           if (RPT_ABORT == forceARpt(er, fmt, &arg1)) {
*              \* anything else you wish to do on exit... *\
*              cleanExit();
*           }
*         }
*
*   INPUTS 
*      errReport- pointer to structure previously initialized by 
*                 initTstData().  If you pass NULL, no report will 
*                 be sent, VPrintf() will be attempted, and you will 
*                 receive RPT_NO_SERV back. 
*      fmt      - printf-style formatting string of the type described 
*                 in exec.library/RawDoFmt(). 
*      arg1     - pointer to the first of the arguments to plug into the 
*                 fmt string, or NULL.  Note that additional args must
*                 follow the first on the stack (as above, in myRpt()).
* 
*   RESULT 
*      error   - described in tmgr_common.h.  If you get RPT_ABORT
*                back, you should exit immediately after 
*                freeing your resources.  Other possible returns 
*                are RPT_OK if the report went to TestManager OK,
*                or RPT_NO_SERV if the TestManager has 
*                exited.  
*
*   EXAMPLE 
*      See forceRpt(), or above.
* 
*   NOTES 
*      Messages are automatically time-stamped, identified, and 
*      reported according to the settings of the Test Manager. 
* 
*   BUGS 
*      None yet. 
* 
*   SEE ALSO 
*   tmgr_common.h, initTstData(), freeTstData(), configRpt(), errRpt(), quickRpt()
*
****************************************************************************** 
* =============================== forceARpt =====================================
*/ 
BYTE forceARpt(struct errReport *er, STRPTR fmt, APTR arg1, ...) {
    BYTE err;

    if (!er) {
        VPrintf(fmt, (LONG *)arg1);
        return(RPT_NO_SERV);
    }

    er->logMsg = fmt;
    er->varArgs = arg1;

    err = sendRpt(er);
    if (err != RPT_OK) {
        VPrintf(fmt, (LONG *)arg1);
    }

    return(err);
}



/****** TestManager.library/errARpt ******************************************
*
*   NAME
*      errARpt - log an error, progress, informational, or debugging message
*
*   SYNOPSIS
*      error = errRpt(ErrReport, errLevel, errType, errCode, fmt, arg1, ...)
*
*      BYTE errRpt(struct errReport *, UWORD, BYTE, LONG, STRPTR, APTR, ...);
*
*   FUNCTION
*      Exactly like errRpt(), but uses a pointer to the 1st arg so
*      that this function call may be nested inside a varargs function.
*
*      Sends information off to test manager task.  This includes
*      a string for the user to see, codes describing the problem,
*      a code describing the severity of the problem (if any).
*
*   INPUTS
*      errReport- pointer to structure previously initialized by
*                 initTstData().  If you pass NULL, no report will
*                 be sent and you will receive RPT_NO_SERV back.
*      errLevel - see tmgr_common.h.  This describes what your message
*                 means to the user:  error or progress report,
*                 additional information, or even debugging information.
*                 Currently defined values:
*                    LVL_PROG for a progress report;
*                    LVL_ERR  for an error message;
*                    LVL_ERR2 for addtional error information;
*                    LVL_VB1  for optional information (VB2 and VB3 as well);
*                    LVL_DB1  for debugging information (DB2 and DB3 as well).
*                 Note that LVL_ERR or LVL_ERR2 will cause the Test Manager
*                 to display a "Test Failed!" message, with appropriate
*                 bells and whistles.
*      errType  - describes what errCode means.  
*                    PROB_NONE   for no problem;
*                    PROB_IOERR  for a standard IoErr()-type error code;
*                    PROB_CUSTOM for a custom error code (strongly
*                                discouraged!).
*      errCode  - a numerical description of your problem.
*      fmt      - printf-style formatting string of the type described
*                 in exec.library/RawDoFmt().
*      arg1     - pointer to the first of the arguments to plug into the 
*                 fmt string, or NULL.
*
*   RESULT
*      error    - described in tmgr_common.h.  If you get anything other
*                 than RPT_OK back, you should exit immediately after
*                 freeing your resources.  Other possible returns
*                 are RPT_ABORT if the TestManager has requested that
*                 you abort, or RPT_NO_SERV if the TestManager has
*                 exited.  This permits cute games to be played,
*                 but such practice is discouraged, especially if 
*                 RPT_NO_SERV is encountered.
*
*   EXAMPLE
*      See errRpt() or forceARpt() or quickARpt().
*
*   NOTES
*      Messages are automatically time-stamped, identified, and
*      reported according to the settings of the Test Manager.
*      As of V37.4 ErrReport.lib, this preserves the report configuration
*      found in the struct errReport prior to the call.  This is so you
*      can use quickRpt() for humdrum messages and errRpt() for the
*      occasional problem.
*
*   BUGS
*      None yet.
*
*   SEE ALSO
*      tmgr_common.h, initTstData(), freeTstData()
*
******************************************************************************
* =============================== errARpt =====================================
*/
BYTE errARpt(struct errReport *er, UWORD eL, BYTE eT, LONG eC, STRPTR fmt, APTR arg1, ...) {
    UWORD orig_eL;
    BYTE  orig_eT, retCode;
    LONG  orig_eC;

    if (!er)
        return(RPT_NO_SERV);

    orig_eL = er->errLevel;
    orig_eT = er->problem;
    orig_eC = er->errCode;

    if (!do_ConfigRpt(er, eL, eT, eC))
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = arg1;

    retCode = sendRpt(er);

    do_ConfigRpt(er, orig_eL, orig_eT, orig_eC);
    return(retCode);
}

/****** TestManager.library/forceErrRpt ******************************************
*
*   NAME
*      forceErrRpt - log an error, progress, informational, or 
*                    debugging message
*
*   SYNOPSIS
*      error = forceErrRpt(ErrReport, errLevel, errType, errCode, fmt, 
*                          arg1, ...)
*
*      BYTE forceErrRpt(struct errReport *, UWORD, BYTE, LONG, STRPTR, 
*                       LONG, ...);
*
*   FUNCTION
*      Exactly like errRpt(), but will attempt to print output
*      via VPrintf() if the server can't be found.  You should be sure
*      to check the return code for RPT_ABORT if the server is running.
*
*      Sends information off to test manager task.  This includes
*      a string for the user to see, codes describing the problem,
*      a code describing the severity of the problem (if any).
*
*   INPUTS
*      errReport- pointer to structure previously initialized by
*                 initTstData().  If you pass NULL, no report will
*                 be sent, VPrintf() will be attempted, and you will 
*                 receive RPT_NO_SERV back.
*      errLevel - see tmgr_common.h.  This describes what your message
*                 means to the user:  error or progress report,
*                 additional information, or even debugging information.
*                 Currently defined values:
*                    LVL_PROG for a progress report;
*                    LVL_ERR  for an error message;
*                    LVL_ERR2 for addtional error information;
*                    LVL_VB1  for optional information (VB2 and VB3 as well);
*                    LVL_DB1  for debugging information (DB2 and DB3 as well).
*                 Note that LVL_ERR or LVL_ERR2 will cause the Test Manager
*                 to display a "Test Failed!" message, with appropriate
*                 bells and whistles.
*      errType  - describes what errCode means.  
*                    PROB_NONE   for no problem;
*                    PROB_IOERR  for a standard IoErr()-type error code;
*                    PROB_CUSTOM for a custom error code (strongly
*                                discouraged!).
*      errCode  - a numerical description of your problem.
*      fmt      - printf-style formatting string of the type described
*                 in exec.library/RawDoFmt().
*      arg1     - the first of the arguments to plug into the 
*                 fmt string, or NULL.
*
*   RESULT
*      error    - described in tmgr_common.h.  If you get RPT_ABORT
*                 back, you should exit immediately after
*                 freeing your resources.  Other possible returns
*                 are RPT_OK if a message was successfully sent,
*                 or RPT_NO_SERV if the TestManager has
*                 exited.  
*
*   EXAMPLE
*      See errRpt() or forceARpt() or quickARpt().
*
*   NOTES
*      Messages are automatically time-stamped, identified, and
*      reported according to the settings of the Test Manager, if and
*      only if the Test Manager is running.
*
*      As of V37.4 ErrReport.lib, this preserves the report configuration
*      found in the struct errReport prior to the call.  This is so you
*      can use quickRpt() for humdrum messages and errRpt() for the
*      occasional problem.
*
*      If the TstMgr isn't running, you should have a valid Output().
*      The following will be attempted:
*           VPrintf("Error %ld!\n, &errCode);
*           VPrintf(fmt, &arg1);
*
*   BUGS
*      None yet.
*
*   SEE ALSO
*      tmgr_common.h, initTstData(), freeTstData(), errRpt(), forceRpt()
*
******************************************************************************
* =============================== forceErrRpt =====================================
*/
BYTE forceErrRpt(struct errReport *er, UWORD eL, BYTE eT, LONG eC, STRPTR fmt, LONG arg1, ...) {
    UWORD orig_eL;
    BYTE  orig_eT, retCode;
    LONG  orig_eC;

    if (!er) {
        VPrintf("Error %ld!\n", &eC);
        VPrintf(fmt, &arg1);
        return(RPT_NO_SERV);
    }

    orig_eL = er->errLevel;
    orig_eT = er->problem;
    orig_eC = er->errCode;

    if (!do_ConfigRpt(er, eL, eT, eC))
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = &arg1;

    retCode = sendRpt(er);

    if (retCode == RPT_NO_SERV) {          /* or should it be anything but RPT_OK? */
        VPrintf("Error %ld!\n", &eC);
        VPrintf(fmt, &arg1);
    }
    do_ConfigRpt(er, orig_eL, orig_eT, orig_eC);
    return(retCode);
}


/****** TestManager.library/forceAErrRpt ******************************************
*
*   NAME
*      forceAErrRpt - log an error, progress, informational, or 
*                     debugging message
*
*   SYNOPSIS
*      error = forceAErrRpt(ErrReport, errLevel, errType, errCode, fmt, 
*                           arg1, ...)
*
*      BYTE forceAErrRpt(struct errReport *, UWORD, BYTE, LONG, STRPTR, 
*                        APTR, ...);
*
*   FUNCTION
*      Exactly like forceErrRpt(), but uses the address of the first argument
*      instead.  See forceErrRpt().  Implemented this way so that you
*      can call it from within a varargs function.  See the example at
*      quickARpt().
*
*   INPUTS
*      errReport- pointer to structure previously initialized by
*                 initTstData().  If you pass NULL, no report will
*                 be sent, VPrintf() will be attempted, and you will 
*                 receive RPT_NO_SERV back.
*      errLevel - see tmgr_common.h.  This describes what your message
*                 means to the user:  error or progress report,
*                 additional information, or even debugging information.
*                 Currently defined values:
*                    LVL_PROG for a progress report;
*                    LVL_ERR  for an error message;
*                    LVL_ERR2 for addtional error information;
*                    LVL_VB1  for optional information (VB2 and VB3 as well);
*                    LVL_DB1  for debugging information (DB2 and DB3 as well).
*                 Note that LVL_ERR or LVL_ERR2 will cause the Test Manager
*                 to display a "Test Failed!" message, with appropriate
*                 bells and whistles.
*      errType  - describes what errCode means.  
*                    PROB_NONE   for no problem;
*                    PROB_IOERR  for a standard IoErr()-type error code;
*                    PROB_CUSTOM for a custom error code (strongly
*                                discouraged!).
*      errCode  - a numerical description of your problem.
*      fmt      - printf-style formatting string of the type described
*                 in exec.library/RawDoFmt().
*      arg1     - pointer to the first of the arguments to plug into the 
*                 fmt string, or NULL.
*
*   RESULT
*      error    - described in tmgr_common.h.  If you get RPT_ABORT
*                 back, you should exit immediately after
*                 freeing your resources.  Other possible returns
*                 are RPT_OK if a message was successfully sent,
*                 or RPT_NO_SERV if the TestManager has
*                 exited.  
*
*   EXAMPLE
*      See errRpt() or forceARpt() or quickARpt().
*
*   NOTES
*      Messages are automatically time-stamped, identified, and
*      reported according to the settings of the Test Manager, if and
*      only if the Test Manager is running.
*
*      As of V37.4 ErrReport.lib, this preserves the report configuration
*      found in the struct errReport prior to the call.  This is so you
*      can use quickRpt() for humdrum messages and errRpt() for the
*      occasional problem.
*
*      If the TstMgr isn't running, you should have a valid Output().
*      The following will be attempted:
*           VPrintf("Error %ld!\n, &errCode);
*           VPrintf(fmt, &arg1);
*
*   BUGS
*      None yet.
*
*   SEE ALSO
*      tmgr_common.h, initTstData(), freeTstData(), errRpt(), forceRpt(),
*      forceErrRpt()
*
******************************************************************************
* =============================== forceAErrRpt =====================================
*/
BYTE forceAErrRpt(struct errReport *er, UWORD eL, BYTE eT, LONG eC, STRPTR fmt, APTR arg1, ...) {
    UWORD orig_eL;
    BYTE  orig_eT, retCode;
    LONG  orig_eC;

    if (!er) {
        VPrintf("Error %ld!\n", &eC);
        VPrintf(fmt, arg1);
        return(RPT_NO_SERV);
    }

    orig_eL = er->errLevel;
    orig_eT = er->problem;
    orig_eC = er->errCode;

    if (!do_ConfigRpt(er, eL, eT, eC))
        return(RPT_NO_SERV);

    er->logMsg = fmt;
    er->varArgs = arg1;

    retCode = sendRpt(er);

    if (retCode == RPT_NO_SERV) {          /* or should it be anything but RPT_OK? */
        VPrintf("Error %ld!\n", &eC);
        VPrintf(fmt, arg1);
    }
    do_ConfigRpt(er, orig_eL, orig_eT, orig_eC);
    return(retCode);
}


