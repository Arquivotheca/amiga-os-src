/* global include file for the shell */



#include <pragmas/exec_old_pragmas.h>

#include <pragmas/console_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/wb_pragmas.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/intuition_protos.h>

#include "shell_protos.h"

#define BUFFER(fh)	((UBYTE *) BADDR((fh)->fh_Buf))
#define TOBUFFER(buf)	(TO_BPTR(buf))

#define TO_BPTR(x)	(((LONG) (x)) >> 2)

/* sizeof, but returns longs */
#define longsize(x)	((sizeof(x)+3) >> 2)


/* resident command messages: */
#define STR_CANT_SET            -141
#define STR_TOO_MANY_MATCHES    -142
#define STR_NOT_EXECUTE         -143
#define STR_MISSING_ELSE        -144
#define STR_NEWSHELL_FAILED     -145
#define STR_CURRENT_STACK       -146
#define STR_STACK_LARGE         -147
#define STR_STACK_SMALL         -148
#define STR_PROCESS_ENDING      -149
#define STR_LAST_MSG_FAILED     -150
#define STR_NO_RETURN_CODE      -151
#define STR_CURRENT_DIR         -152
#define STR_BAD_RETURN_CODE     -153
#define STR_FAIL_LIMIT          -154
#define STR_FAULT               -155
#define STR_TH_SYSTEM           -156
#define STR_TH_INTERNAL         -157
#define STR_TH_DISABLED         -158
#define STR_NAME                -159
#define STR_TH_USE_COUNT        -160

extern LONG AbsExecBase;

/* #define bufferflush_char 0 */
#define DOS_TRUE (-1)
#define END_STREAM_CH (-1L)

/* these defines moved into dosextens.h */
#define CMD_BUILTIN CMD_INTERNAL
/*
#define CMD_DISABLED (-999L)
*/

// These are initial values, they can get larger
#define MAXCOMMAND 512
#define MAXALIAS 256

// NAMEMAX must be size minus 1!
#define NAMEMAX (104-1)
#define PROMPTMAX (60-1)
#define SETMAX (80-1)
#define FILEMAX (40-1)

struct Global {
   struct DosLibrary *DOSBase;
   struct Library *UtilityBase;
   struct FileInfoBlock ExInfo;
   UBYTE *cbuffer;
   LONG  cbuffersize;
   UBYTE *buffer; /* temporary storage buffer */
   LONG  buffersize;
   UBYTE *alias;
   LONG  aliassize;
   long cpos;
   long count;
   short echo;
   short fill;
   struct CommandLineInterface *Clip; /* pointer to CLI */
   BPTR run_module; /* these modules can't take new redirection */
   BPTR set_module;
   BPTR pipe_module;
#ifdef OLD_REDIRECT
   BOOL old_red;	/* flag to indicate old style redirection */
#endif
   LONG Res2;
};

/* for use when DOS doesn't send me one */
struct CGlobal {
   struct CommandLineInterface Clip;
   UBYTE prompt[PROMPTMAX+1];
   UBYTE commandname[NAMEMAX+1];
   UBYTE commandfile[FILEMAX+1];
   UBYTE setname[SETMAX+1];
};

#if 0
extern int link_setenv();
extern int link_endif();
extern int link_endcli();
extern int link_echo();
extern int link_resident();
extern int link_cd();
extern int link_prompt();

extern int link_getenv();
extern int link_ask();
extern int link_else();
extern int link_failat();
extern int link_fault();
extern int link_if();
extern int link_path();
extern int link_quit();
extern int link_skip();
extern int link_why();
extern int link_stack();
extern int link_which();
extern int link_newshell();
extern int link_run();
#endif
