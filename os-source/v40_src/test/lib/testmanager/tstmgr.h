
/* struct TMgr_Globals */
#ifndef TST_MGR_H
#define TST_MGR_H

#include "tmgr_common.h"

#define MAX_FNAME_LEN 256
#define SERVER_PORTNAME_SIZE 12   /* Length of 'TestMgr_999' + 1 */
#define LAST_ERR_BUFSIZE  50
#define TMP_BUFLEN        80      /* for various temp uses... prevent stack overflow/memory frag */
#define DGAD_BUFLEN       27      /* Gadget text buffer size */

struct TMgr_Globals {
    struct MsgPort *mp;                   /* message port                 */
    UBYTE outputs;                        /* where to send output         */
    UBYTE fileName[MAX_FNAME_LEN];        /* filename/path spec           */
    UBYTE conName[MAX_FNAME_LEN];         /* console window spec          */
    UBYTE portName[SERVER_PORTNAME_SIZE]; /* Space for (variable) port name */
    UBYTE lastErr[LAST_ERR_BUFSIZE];      /* Last error reported          */
    UBYTE sdbuf[LEN_DATSTRING];           /* for timestamping  StrDate    */
    UBYTE stbuf[LEN_DATSTRING];           /* for timestamping  StrTime    */
    UBYTE tmpBuf[TMP_BUFLEN];             /* misc temporary use           */
    UBYTE lmrBuf[DGAD_BUFLEN];            /* LastMsgRecd gadget text buf  */
    UBYTE lmfBuf[DGAD_BUFLEN];            /* LastMsgFrom gadget text buf  */
    UBYTE lerBuf[DGAD_BUFLEN];            /* LastErrRecd gadget text buf  */
    UBYTE lefBuf[DGAD_BUFLEN];            /* LastErrFrom gadget text buf  */
    BPTR fh;                              /* filehande for filename       */
    BPTR cfh;                             /* filehandle for console       */
    struct TextFont *tmgr_font;           /* font for window - TOPAZ8     */
    BOOL testStatus;                      /* pass/fail                    */
    BOOL earlyExit;                       /* AutoExit on first err?       */
    BOOL carryOn;                         /* Tell clients to continue?    */
    BOOL useWB;                           /* use WB or custom screen?     */ /* default: custom */
    ULONG errCount;                       /* How many errors?             */
    ULONG clientCount;                    /* open count for clients       */
    ULONG okMsg;                          /* whether user should see msg  */
                                          /* (depends on LVL_XX set)      */
};

#define PASS_TEST TRUE
#define FAIL_TEST FALSE

#define OUTB_SER  0
#define OUTB_PAR  1
#define OUTB_CON  2
#define OUTB_FILE 3

#define OUTF_SER  1<<OUTB_SER  
#define OUTF_PAR  1<<OUTB_PAR  
#define OUTF_CON  1<<OUTB_CON  
#define OUTF_FILE 1<<OUTB_FILE 
#define OUTF_DEFAULT OUTF_SER

#define DEFAULT_CONSPEC "con:0/0/640/100/TestMgr/auto/close/wait"
#define DEFAULT_MSGSPEC LVLM_NORM        /* default errorLevels to see, tmgr_common.h */

#ifndef MAX_PORTNAME_RETRIES
#define MAX_PORTNAME_RETRIES 999
#endif

#ifndef PORT_TEMPLATE
#define PORT_TEMPLATE "TestMgr_"
#endif

#ifndef DEFAULT_SERV_PRI
#define DEFAULT_SERV_PRI 4
#endif

#define LIBNAME_ICONLIB "icon.library"
#define LIBNAME_CXLIB "commodities.library"

#define TEMPLATE "USEWB/S,NOSER/S,USEPAR/S,USECON/S,LOGFILE/K,ERRABORT/S,PRIORITY/K/N,ERRLEV/K/N,VERBOSE/K/N,DEBUG/K/N"
#define OPT_USEWB    0  /* Popup on WB rather than custom screen?   */
#define OPT_NOSER    1  /* don't use ser:?                          */
#define OPT_USEPAR   2  /* use par: ?                               */
#define OPT_USECON   3  /* use console?                             */
#define OPT_LOGFILE  4  /* logfile to use                           */
#define OPT_ERRABORT 5  /* abort on first error                     */
#define OPT_PRIORITY 6  /* task priority                            */
#define OPT_ERRLEV   7  /* error level, only 2 is meaningful        */
#define OPT_VERBOSE  8  /* verbose level, only 1/2/3 meaningful     */
#define OPT_DEBUG    9  /* debug level, only 1/2/3 are meaningful   */
#define NUM_OPTS     10

/* Tool type names */
#define TT_NOSER        "NOSER"
#define TT_USEPAR       "USEPAR"
#define TT_USECON       "USECON"
#define TT_LOGFILE      "LOGFILE"
#define TT_ERRABORT     "ERRABORT"
#define TT_PRIORITY     "PRIORITY"
#define TT_WINDOW       "USEWB"
#define TT_USEWB        TT_WINDOW
#define TT_ERRLEV       "ERRLEV"
#define TT_DEBUG        "DEBUG"
#define TT_VERBOSE      "VERBOSE"

#endif /* TST_MGR_H */

