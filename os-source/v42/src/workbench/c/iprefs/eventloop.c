
/* includes */
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/notify.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <internal/iprefs.h>
#include <utility/tagitem.h>
#include <libraries/locale.h>
#include <string.h>
#include <dos.h>

/* prototypes */
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/locale_protos.h>
#include <clib/alib_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "iprefs.h"
#include "resetwb.h"
#include "pread.h"
#include "sound.h"
#include "backdrop.h"
#include "texttable.h"


/*****************************************************************************/


struct ExecBase *SysBase;
struct Library  *IntuitionBase;
struct Library  *GfxBase;
struct Library  *DOSBase;
struct Library  *UtilityBase;
struct Library  *LayersBase;
struct Library  *WorkbenchBase;
struct LIbrary  *IFFParseBase;
struct Library  *LocaleBase;
struct Library  *DataTypesBase;


/*****************************************************************************/


struct MsgPort  audioPort;
BOOL		doResetWB;
struct Task    *parentTask;
struct Task    *iprefsTask;


/*****************************************************************************/


/* must end with a | ! */
#define PREFS_FILES "screenmode|overscan|icontrol|printer|printergfx|serial|input|pointer|palette|locale|sound|font|wbpattern|"


/*****************************************************************************/


VOID ExtractPrefsName(UWORD index, STRPTR name)
{
UWORD i,j,k;

    i = 0;
    j = 0;
    while (TRUE)
    {
        strcpy(name,"ENV:Sys/");
        k = 8;
        while (PREFS_FILES[j] != '|')
            name[k++] = PREFS_FILES[j++];
        j++;
        name[k] = 0;
        strcat(name,".prefs");

        if (i == index)
            return;

        i++;
    }
 }


/*****************************************************************************/


VOID EventLoop(struct MsgPort *notifyPort, struct SignalSemaphore *iprefsLock)
{
struct NotifyMessage *nmsg;
ULONG                 sigs;
BOOL                  didPrefs;
char                  name[30];

    while (TRUE)
    {
        didPrefs = FALSE;
	while (nmsg = (struct NotifyMessage *)GetMsg(notifyPort))
	{
            ExtractPrefsName(nmsg->nm_NReq->nr_UserData,name);

            ObtainSemaphore(iprefsLock);
	    ReadPrefsFile(name);
	    ReleaseSemaphore(iprefsLock);

	    ReplyMsg(nmsg);
	    didPrefs = TRUE;
	}

	if (doResetWB && didPrefs)
    	    doResetWB = !ResetWB();

	SignalParent();

        sigs = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);

        if (sigs & SIGBREAKF_CTRL_D)
            CleanUpAudio();

        if (sigs & SIGBREAKF_CTRL_F)
            DoAudio();

        if (sigs & SIGBREAKF_CTRL_E)
            UpdateDTBackdrops();
    }
}
