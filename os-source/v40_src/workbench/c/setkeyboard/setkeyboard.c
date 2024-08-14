
/* includes */
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <devices/console.h>
#include <devices/keymap.h>
#include <devices/conunit.h>
#include <devices/keymap.h>
#include <libraries/locale.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "texttable.h"
#include "setkeyboard_rev.h"


/*****************************************************************************/


#define	LIB_VERSION 37


/*****************************************************************************/


#define TEMPLATE    "KEYMAP/A" VERSTAG
#define OPT_KEYMAP  0
#define OPT_COUNT   1


/*****************************************************************************/


enum AppStringsID SetMap(STRPTR mapName, APTR UtilityBase, APTR DOSBase);


/*****************************************************************************/


/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);

#define LocaleBase li.li_LocaleBase
#define catalog    li.li_Catalog


/*****************************************************************************/


int main(int argc, char *argv[])
{
enum AppStringsID  result;
LONG               failureLevel = RETURN_FAIL;
LONG               opts[OPT_COUNT];
struct RDArgs     *rdargs;

struct Library   *SysBase = (*((struct Library **) 4));
struct Library   *UtilityBase;
struct Library   *DOSBase;
struct LocaleInfo li;

    LocaleBase = NULL;

    if (DOSBase = OpenLibrary("dos.library",LIB_VERSION))
    {
        if (UtilityBase = OpenLibrary("utility.library",LIB_VERSION))
        {
            memset(opts,0,sizeof(opts));
            if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
            {
                if ((result = SetMap((STRPTR)opts[OPT_KEYMAP],UtilityBase,DOSBase)) == MSG_NORMAL)
                {
                    failureLevel = RETURN_OK;
                }
                else
                {
		    catalog = NULL;
                    if (LocaleBase = OpenLibrary("locale.library",38))
                        catalog = OpenCatalogA(NULL,"c.catalog",NULL);

                    PutStr(GetString(&li,result));

                    if (LocaleBase)
                    {
                        CloseCatalog(catalog);
                        CloseLibrary(LocaleBase);
                    }
                }
                FreeArgs(rdargs);
            }
            else
            {
                PrintFault(IoErr(),NULL);
            }
            CloseLibrary(UtilityBase);
        }
        CloseLibrary(DOSBase);
    }
    return(failureLevel);
}


/*****************************************************************************/


struct Node *FindNameNC(APTR UtilityBase, struct List *list, STRPTR name)
{
struct Node *node;
WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        result = Stricmp(name,node->ln_Name);
        if (result == 0)
            return(node);

	node = node->ln_Succ;
    }

    return(NULL);
}


/*****************************************************************************/


struct Combo
{
    struct InfoData       infoData;
    struct StandardPacket packet;
};


enum AppStringsID SetMap(STRPTR mapName, APTR UtilityBase, APTR DOSBase)
{
struct IOStdReq        ioReq;
char                   fileName[100];
struct MsgPort        *con;
BPTR                   segment = NULL;
struct KeyMapResource *kr;
struct KeyMapNode     *kn;
struct Library        *SysBase = (*((struct Library **) 4));
struct Process	      *process;
struct MsgPort        *unit;
STRPTR                 baseName;
D_S(struct Combo,cb);

    process = (struct Process *)FindTask(NULL);

    baseName = FilePart(mapName);

    /* get keymap resource */
    if (!(kr = (struct KeyMapResource *)OpenResource("keymap.resource")))
    {
	return(MSG_ERROR_CANT_OPEN_RESOURCE);
    }

    ioReq.io_Message.mn_ReplyPort = &process->pr_MsgPort;

    /* open the console device */
    if (OpenDevice("console.device",-1,&ioReq,0))
    {
	return(MSG_ERROR_CANT_OPEN_CONSOLE_DEV);
    }

    Forbid();

    /* search resource for keymap */
    if (!(kn = (struct KeyMapNode *)FindNameNC(UtilityBase,&kr->kr_List,baseName)))
    {
        Permit();

	/* keymap not in resource, needs to be loaded */
	strcpy(fileName,"KEYMAPS:");
	AddPart(fileName,mapName,sizeof(fileName));

        if (!(segment = LoadSeg(fileName)))
        {
	    CloseDevice(&ioReq);
	    return(MSG_ERROR_KEYMAP_NOT_FOUND);
	}

        kn = (struct KeyMapNode *)((segment << 2) + sizeof(BPTR));
        if (!TypeOfMem(kn->kn_Node.ln_Name) || (Stricmp(baseName,kn->kn_Node.ln_Name) != 0))
        {
	    CloseDevice(&ioReq);
	    UnLoadSeg(segment);
	    return(MSG_ERROR_INVALID_KEYMAP);
        }

        Forbid();

        if (!(kn = (struct KeyMapNode *)FindNameNC(UtilityBase,&kr->kr_List,baseName)))
        {
            kn = (struct KeyMapNode *)((segment << 2) + sizeof(BPTR));
            AddHead(&(kr->kr_List),kn);
        }
        else
        {
            UnLoadSeg(segment);
            segment = NULL;
        }
    }

    Permit();

/*
    if (global)
    {
        /* set global keymap default */
        ioReq.io_Command = CD_SETDEFAULTKEYMAP;
        ioReq.io_Data    = &(kn->kn_KeyMap);
        ioReq.io_Length  = sizeof(struct KeyMap);
        if (DoIO(&ioReq))
        {
            CloseDevice(&ioReq);
            UnLoadSeg(segment);
            return(MSG_ERROR_CANT_SET_MAP);
        }
    }
*/

    if (con = process->pr_ConsoleTask)
    {
        /* this is the console handler'ss packet port */
        memset(cb,0,sizeof(struct Combo));
        cb->packet.sp_Msg.mn_Node.ln_Name = (STRPTR)&(cb->packet.sp_Pkt);
        cb->packet.sp_Pkt.dp_Link         = &(cb->packet.sp_Msg);
        cb->packet.sp_Pkt.dp_Port         = &process->pr_MsgPort;
        cb->packet.sp_Pkt.dp_Type         = ACTION_DISK_INFO;
        cb->packet.sp_Pkt.dp_Arg1         = (LONG)(&cb->infoData) >> 2;
        PutMsg(con,&cb->packet);

        WaitPort(&process->pr_MsgPort);
        GetMsg(&process->pr_MsgPort);

        if (cb->infoData.id_InUse)
        {
            unit             = ioReq.io_Unit;
            ioReq.io_Unit    = ((struct IOStdReq *) cb->infoData.id_InUse)->io_Unit;
            ioReq.io_Command = CD_SETKEYMAP;
            ioReq.io_Data    = &(kn->kn_KeyMap);
            ioReq.io_Length  = sizeof(struct KeyMap);
            if (DoIO(&ioReq))
            {
                ioReq.io_Unit = unit;
                CloseDevice(&ioReq);
                UnLoadSeg(segment);
                return(MSG_ERROR_CANT_SET_CONSOLE_MAP);
            }
            ioReq.io_Unit = unit;
        }
    }
    CloseDevice(&ioReq);
    return(MSG_NORMAL);
}
