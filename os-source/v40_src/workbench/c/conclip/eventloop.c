
#include <exec/types.h>
#include <utility/hooks.h>
#include <libraries/locale.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "conclip.h"
#include "clipio.h"

#define CONCLIP
#include <localestr/c.h>


/*****************************************************************************/


struct ExecBase *SysBase;
struct Library  *IntuitionBase;
struct Library  *DOSBase;
struct Library  *LocaleBase;
struct Device   *ConsoleBase;

BOOL            paste;
ULONG           clipUnit;


/*****************************************************************************/


static ULONG __asm SnipHook(register __a0 struct Hook *hook,
                            register __a1 struct SnipHookMsg *snipMsg)
{
struct MsgPort *port;

    geta4();

    paste = TRUE;
    port  = (struct MsgPort *)hook->h_Data;
    Signal(port->mp_SigTask, 1 << port->mp_SigBit);

    return(0);
}


/*****************************************************************************/


static STRPTR GetStr(struct Catalog *catalog, ULONG id, STRPTR defaultStr)
{
    if (catalog)
        return(GetCatalogStr(catalog,id,defaultStr));

    return(defaultStr);
}


/*****************************************************************************/


static VOID PullRequester(LONG error)
{
struct EasyStruct  es;
LONG               ignore;
struct Catalog    *catalog;

    catalog = NULL;
    if (LocaleBase)
        catalog = OpenCatalogA(NULL,"sys/c.catalog",NULL);

    ignore = FALSE;

    es.es_StructSize   = sizeof(struct EasyStruct);
    es.es_Flags        = 0;
    es.es_Title        = NULL;
    es.es_GadgetFormat = GetStr(catalog,MSG_CC_OK_GAD,MSG_CC_OK_GAD_STR);

    switch (error)
    {
        case ERR_NOIFFPARSE : es.es_TextFormat = GetStr(catalog,MSG_CC_NOIFFPARSE,MSG_CC_NOIFFPARSE_STR);
                              break;

        case ERR_NOCLIPBOARD: es.es_TextFormat = GetStr(catalog,MSG_CC_NOCLIPBOARD,MSG_CC_NOCLIPBOARD_STR);
                              break;

        case ERR_NOMEMORY   : es.es_TextFormat = GetStr(catalog,MSG_CC_NOMEMORY,MSG_CC_NOMEMORY_STR);
                              break;

        default             : ignore = TRUE;
                              break;
    }

    if (!ignore)
	EasyRequest(NULL,&es,NULL,"");

    if (LocaleBase)
        CloseCatalog(catalog);
}


/*****************************************************************************/


/* This function never returns */

VOID EventLoop(struct MsgPort *port)
{
struct CmdMsg    *cm;
LONG              error;
struct IORequest  conIO;
struct Hook       conHook;

    conHook.h_Entry = SnipHook;
    conHook.h_Data  = port;

/*  Initialized by BSS hunk
    paste       = FALSE;
    ConsoleBase = NULL;
*/

    while (TRUE)
    {
        while (cm = (struct CmdMsg *)GetMsg(port))
        {
            if (cm->cm_Type == TYPE_ARGS)
            {
                clipUnit = cm->cm_Unit;
                if (cm->cm_Shutdown)
                {
                    if (ConsoleBase)
                    {
                        Forbid();

                        do
                        {
                            ReplyMsg(cm);
                        }
                        while (cm = (struct CmdMsg *)GetMsg(port));

                        RemPort(port);
                        RemConSnipHook(&conHook);

                        Permit();

                        CloseDevice(&conIO);
                        ConsoleBase = NULL;
                    }
                }
                else if (!ConsoleBase)
                {
                    /* Once we do this, we effectively have disabled the
                     * internal console.device pasting feature.  The console
                     * device will start broadcasting "<CSI>0 v" whenever
                     * the user hits RIGHT AMIGA V.  CON: will then start
                     * sending messages to this utility asking for the IFF
                     * TEXT in the clipboard.device.
                     */

                    OpenDevice("console.device", -1, &conIO, 0);  /* can't fail! */
                    ConsoleBase = conIO.io_Device;
                    AddConSnipHook(&conHook);

                    port->mp_Node.ln_Name = "ConClip.rendezvous";
                    port->mp_Node.ln_Pri  = -1;
                    AddPort(port);
                }
            }
            else if (cm->cm_Type == TYPE_READ)
            {
                error       = GetChars(cm);
                cm->cm_Type = TYPE_REPLY;
                if (error)
                    PullRequester(error);
            }

            /* ignore any other type of message, just reply */

            if (cm)
                ReplyMsg(cm);
        }

        Wait(1 << port->mp_SigBit);

        if (paste)
        {
            error = PutChars();
            paste = FALSE;

            if (error)
                PullRequester(error);
        }
    }
}
