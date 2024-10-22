/******************************************************************************
*
*	$Id: test.c,v 1.1 93/03/23 13:49:27 Jim2 Exp $
*
*****************************************************************************/

#include <exec/memory.h>
#include <exec/types.h>
#include <exec/ports.h>
#include "/gfxbase.h"
#include "/view.h"
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <clib/icon_protos.h>
#include <pragmas/icon_pragmas.h>

#include "v39:aug/src/wack/wack_protos.h"
#include "v39:aug/src/wack/wack_pragmas.h"

#include <string.h>

#include "copdis.h"


#define TEMPLATE "C=COMMENTS/S,D=DATA/S,N=NOCOLOURS/S,I=NOCOPINIT/S,F=NOFINAL/S,SAFELY/S,ID=INTERDISP/S,SS=SPRITESET/S,SC=SPRITECLEAR/S,U=USER/S,NORAW/S,NT=NOTRANS/S,WACKPORT/K"
#define OPT_COMMENT 0
#define OPT_DATA 1
#define OPT_NOCOLOURS 2
#define OPT_NOCOPINIT 3
#define OPT_NOFINAL 4
#define OPT_SAFELY 5
#define OPT_IDISPLAY 6
#define OPT_ISPRITE 7
#define OPT_ISPCLEAR 8
#define OPT_IUSER 9
#define OPT_NORAW 10
#define OPT_NOTRANS 11
#define OPT_WACK 12
#define OPT_COUNT 13


#define EXECBASE (*(struct ExecBase **)4)


struct ExecBase *SysBase = NULL;
struct Library *DOSBase = NULL;
struct GfxBase *GfxBase = NULL;

struct MsgPort *WackBase;

STATIC BOOL StandAlone;

//int CXBRK (void) { return (0) ; }	/* disable ctrl-c */

extern VOID CopDisVPrintf(STRPTR Fmt, ...);

extern APTR CopDisFindPointer(APTR Address);

extern UWORD CopDisFindWord (APTR Address);

ULONG cmd (VOID)
{
    struct Process *proc;
    struct WBStartup *msg=NULL;
    struct RDArgs *rdargs = NULL, *myRDArgs=NULL;
    struct View *ActiView;
    struct CopList *UCopperList;
    struct ViewPort *Walking, *StartOfVP;
    struct Library *IconBase;
    struct WBArg *wbArg;
    struct DiskObject *diskObj;
    BPTR tmplock;
    char **ToolTypes, *TotalString;
    ULONG totalSize=3;



    LONG result[OPT_COUNT];

    SysBase = EXECBASE;
    proc = (struct Process *) FindTask(NULL);
    if (!(proc->pr_CLI))
    {
	WaitPort(&(proc->pr_MsgPort));
	msg = (struct WBStartup *) GetMsg (&(proc->pr_MsgPort));
    }
    memset((char *)result, 0, sizeof(result));
    if ((DOSBase = OpenLibrary ("dos.library",37)) != NULL)
    {
    Printf ("Line1\n");
    VPrintf ("Line2\n");
    Printf ("Line3\n", 0);
    VPrintf ("Line4\n", 0);
    }
    if (msg)
    {
	Forbid();
	ReplyMsg((struct Message *)msg);
    }
    return(0);
}

VOID CopDisVPrintf(STRPTR Fmt, ...)
{
    if (StandAlone)
	VPrintf (Fmt, ((ULONG *)&Fmt)+1 );
    else
	Wack_VPrintf(Fmt,  ((ULONG *)&Fmt)+1);
}

APTR CopDisFindPointer(APTR Address)
{
    if (StandAlone)
	return (*((APTR *)Address));
    return (Wack_ReadPointer(Address));
}

UWORD CopDisFindWord (APTR Address)
{
    if (StandAlone)
	return (*((UWORD *) Address));
    return (Wack_ReadWord(Address));
}

