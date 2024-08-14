#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
/* #include <local/sccs.h> */

#include "share.h"

#ifdef DO_INHIBIT
/* static char SccsId[] = SCCS_STRING; */

BOOL inhibit (filesystem, iflag)

char	*filesystem;
int      iflag;

{

    struct MsgPort	*fsport;
    struct MsgPort        *rp;
    struct StandardPacket *sp = NULL;
    BOOL			 success;

    if(iflag)
        Debug1("Inhibit %s\n", (ULONG) filesystem);
    else
        Debug1("Uninhibit %s\n", (ULONG) filesystem);

    if ((fsport = DeviceProc (filesystem)) == NULL)
        return FALSE;

    if ((sp = AllocMem(sizeof(*sp), MEMF_PUBLIC)) == NULL)
        return FALSE;

    sp->sp_Msg.mn_Node.ln_Name = (char *) &sp->sp_Pkt;
    sp->sp_Pkt.dp_Link         = &sp->sp_Msg;
    sp->sp_Pkt.dp_Port         = rp 
                               = &((struct Process *)FindTask (0))->pr_MsgPort;
    sp->sp_Pkt.dp_Type         = ACTION_INHIBIT;
    sp->sp_Pkt.dp_Arg1         = iflag;

    PutMsg (fsport, &sp->sp_Msg);

    if (WaitPort (rp) != &sp->sp_Msg) {
        Debug0("DOS Error.  ACTION_INHIBIT failed.\n");
        return 0;
    }

    Remove ((struct Node *) &sp->sp_Msg);

    success = sp->sp_Pkt.dp_Res1;
    FreeMem (sp, sizeof(*sp));
    return success;
}
#endif
