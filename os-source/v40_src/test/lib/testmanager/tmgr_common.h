/* Common_h.c - for precompiled headers */

#ifndef TMGR_COMMON_H
#define TMGR_COMMON_H

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>  /* for additional error return codes */
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/expansion_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <math.h>

#define MAX_CLIENT_ID_LEN  64        /* ID name string for your program */
#define MAX_SERVER_ID_LEN  64        /* same, but for server/TestMgr */

extern struct DosLibrary *DOSBase;    /* set up by SAS startup code */
extern struct ExecBase *SysBase;      /* Declared in SAS C Startup */

/* A few handy macros --------------------------------------- */
/*  ...for exiting on Workbench users */
#define EXIT_IF_WB() if (!argc) {exit(RETURN_FAIL);}

/* ...for exit()ing on pre-V37 users (CLI use only) */
#define EXIT_IF_ANCIENT() if(DOSBase->dl_lib.lib_Version < 37L) { Write(Output(),"You need AmigaDOS V37 or better!\n",33); exit(RETURN_FAIL);}

/* ...for killing Lattice Break-handler */
#define NUKE_SAS_BREAK int CXBRK(void) {return(0);} 
#define REALLY_NUKE_IT int ckkabort(void) {return(0);}

/* ...for checking whether the user hit control-C */
#define USER_HIT_CTRL_C (CheckSignal(SIGBREAKF_CTRL_C))


/* errReport is the major structure used by the TestManager */

struct errReport {        /* was memRequest */
    struct Message msg;   /* exec message (note that Message.MsgPort is ID)           */
    struct DateTime dt;   /* For timing and record purposes...stamp before sending.   */
    struct MsgPort *mp;   /* Here only for convenience                                */
    UBYTE  clientName[MAX_CLIENT_ID_LEN];
                          /* Unique name for your program; appears in logfile         */
    UBYTE  serverName[MAX_SERVER_ID_LEN];
                          /* Name of Test Manager to report to                        */
    STRPTR logMsg;        /* Message for log file                                     */
    APTR   varArgs;       /* Use if log1 == printf-style formatting string, else NULL */
    UWORD  errLevel;      /* Debug info, Progress report, Error Message, etc.         */
    BYTE   problem;       /* What's your problem?  IoErr or Custom?                   */
    LONG   errCode;       /* numeric error code: prob_ioerr, custom, or none          */
    BYTE   returnCmd;     /* Possibly a command from the Report Manager               */
};

/* A note on DateTime:  initialize everything EXCEPT for the
   buffers for date and time.  Server will plug in its own buffers
   and do a DateToStr(dt) on the structure provided in the memRequest
 */

/* HERE ARE THE WOND'ROUS THINGS YOU NEED FOR errReport */

/* Error levels.  These are cumulative; if you want verbose level 2, you
   also get verbose level 1, progress reports, and error reports.
   The default is to see progress reports and error reports.
*/
/* errLevel bitdefs -------------------------------- */
#define LVLB_PROG 0     /* normal progress, no error */
#define LVLB_ERR  1     /* error message             */
#define LVLB_ERR2 2     /* addt'l error information  */
#define LVLB_EXIT 3     /* you are exiting           */
#define LVLB_VB1  4     /* verbose level 1           */
#define LVLB_VB2  5     /* verbose level 2           */
#define LVLB_VB3  6     /* verbose level 3           */
#define LVLB_DB1  7     /* debug   level 4           */
#define LVLB_DB2  8     /* debug   level 5           */
#define LVLB_DB3  9     /* debug   level 6           */
#define LVLB_INT 10     /* internal use only         */

/* errLevel Flags ----- These you pass in errReport.errLevel */
#define LVL_PROG 1<<LVLB_PROG   /* normal progress, no error */
#define LVL_ERR  1<<LVLB_ERR    /* error report              */
#define LVL_ERR2 1<<LVLB_ERR2   /* additional error info     */
#define LVL_EXIT 1<<LVLB_EXIT   /* you are exiting           */
#define LVL_VB1  1<<LVLB_VB1    /* verbose info level 1      */
#define LVL_VB2  1<<LVLB_VB2    /* verbose info level 2      */
#define LVL_VB3  1<<LVLB_VB3    /* verbose info level 3      */
#define LVL_DB1  1<<LVLB_DB1    /* debug info level 1        */
#define LVL_DB2  1<<LVLB_DB2    /* debug info level 2        */
#define LVL_DB3  1<<LVLB_DB3    /* debug info level 3        */
#define LVL_INT  1<<LVLB_INT    /* internal use only         */

/* The default is for the Test Manager to report any of these events:
   Progress report    (LVL_PROG)
   Error Message      (LVL_ERR)
   Error Explanation  (LVL_ERR2)
   Program Exit       (LVL_EXIT)
 */
#define LVLM_NORM  LVL_ERR|LVL_ERR2|LVL_EXIT|LVL_PROG    /* default value for reporting */

/* Problem descriptors for errReport.problem                                             */
/* ------------------------------------------------------------------------------------- */
#define PROB_NONE   0          /* no problem, don't report a number         */
#define PROB_IOERR  1          /* problem is covered by std IoErr() numbers */
#define PROB_CUSTOM 2          /* you're using your own errnumber...report it to user */
                               /* Note: PROB_CUSTOM is ***NOT*** encouraged */
#define PROB_NEED_ID 3         /* initTstData() use only      */


/* The Test Manager may wish to tell you something in its reply.  Here are those
   possible values.  Be prepared to exit if the Manager tells you to.
   ---------------------------------------------------------------------------------------
 */
#define TMCMD_PROCEED 0        /* continue your test */
#define TMCMD_ABORT   1        /* abort your test    */

/* Return values for errRpt() ----------------------------------------------------------- */
/* If you receive anything > 0, you should exit immediately after freeing your resources. */
#define RPT_OK      0             /* sent report ok               */
#define RPT_ABORT   1             /* Manager commands you to exit */
#define RPT_NO_SERV 2             /* Manager is not present; exit */
#define RPT_NO_SERVER RPT_NO_SERV /* as above */


/* A few ways to refer to the server's default portname */
#define SERVER_NAME      "TestMgr_0"
#define SERVER_PORT      SERVER_NAME
#define SERVER_PORT_NAME SERVER_NAME
#define ERRMGR           SERVER_NAME
#define ERR_MGR          SERVER_NAME
#define ERROR_MANAGER    SERVER_NAME
#define ERR_MGR_PORT     SERVER_NAME
#define TSTMGR           SERVER_NAME
#define TESTMGR          SERVER_NAME
#define TEST_MGR         SERVER_NAME
#define TST_MGR          SERVER_NAME
#define TEST_MANAGER     SERVER_NAME
#define TST_MGR_PORT     SERVER_NAME

#define PORT_PRIORITY 0      /* for CreatePort() */
#define DEFAULT_CLIENT_NAME "Test_Prog"

/* Stuff for fancy error reporting -- VT100 codes for serial output */
#define BELL_CHAR              0x07
#define SPACE_CHAR             ' '
#define NORMAL_VIDEO_STRING    "\x1b[0m"
#define BOLDFACE_VIDEO_STRING  "\x1b[1m"
#define UNDERLINE_VIDEO_STRING "\x1b[4m"
#define BLINKING_VIDEO_STRING  "\x1b[5m"
#define INVERSE_VIDEO_STRING   "\x1b[7m"

#define ERR_AWAKEN  "\x07\x1b[7m"
#define ERR_AWAKENP BOLDFACE_VIDEO_STRING
#define ERR_SLEEP   NORMAL_VIDEO_STRING

/* function protos */
extern VOID bsprintf(STRPTR dest, STRPTR fmt, LONG arg1, ...);

#endif /* TMGR_COMMON_H */
