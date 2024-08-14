#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/alerts.h>

#include <string.h>

#define ASM __asm
#define REG(x)  register __ ## x

#include <clib/exec_protos.h>
#ifndef NO_EXEC_PRAGMAS
#include <pragmas/exec_pragmas.h>
#endif

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <utility/tagitem.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>

#define DOSLIB	   "dos.library"
#define DOSVER	   36L
#define MSGBUFSIZE 80

#define OPENFAIL { Result2(ERROR_INVALID_RESIDENT_LIBRARY); }
#define EXECBASE (*(struct ExecBase **)4)

#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))
#define THISCLI     Cli()
#define Result2(x)  THISPROC->pr_Result2 = x

/* This is the amount of storage expected to be allocated for typical */
/* text environment variables.	It is used in GETENV/IF to allocate   */
/* buffers on the stack.					      */
#define ENVMAX 256

/* Return values from ReadItem(). These should be in dos.h! */
#define ITEM_NONE   0	/* nothing read        */
#define ITEM_TOKEN  1	/* lone unquoted token */
#define ITEM_STRING 2	/* Quoted string       */

/* Extensions from Utility.library for convenience                      */
/* these really belong in Utility.library and the associated include    */
/* files.                                                               */
LONG   SMod32( LONG, LONG );
ULONG  UMod32( ULONG, ULONG );
#pragma libcall UtilityBase SMod32 96 1012
#pragma libcall UtilityBase UMod32 9c 1012

#define SDiv32(a,b) SDivMod32(a,b)
#define UDiv32(a,b) UDivMod32(a,b)

#pragma libcall DOSBase WriteChars 3ae 2102
#pragma libcall DOSBase Puts       3b4  101
#pragma libcall DOSBase VPrintf    3ba 2102

/* This is used with the templates to cause the version string to be */
/* stored in the command.  It depends upon the command having a #include */
/* of the appropriate version file to provide the value for this string */
#define CMDREV  "\0$VER: " VSTRING
