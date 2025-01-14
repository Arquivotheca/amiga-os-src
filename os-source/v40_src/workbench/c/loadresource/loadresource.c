
#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <libraries/locale.h>
#include <dos/dosasl.h>
#include <dos.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/diskfont_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "loadresource.h"
#include "segments.h"
#include "utils.h"


/*****************************************************************************/


enum ResourceType
{
    RT_LIBRARY,
    RT_DEVICE,         /* can never be on the resource list, can't be locked... */
    RT_FONT,
    RT_CATALOG
};

struct ResourceNode
{
    struct Node rn_Link;
    APTR        rn_Resource;
};

#define rn_Type rn_Link.ln_Type
#define rn_Name rn_Link.ln_Name


/*****************************************************************************/


#define FAILLEVEL(level) args->cla_Result1 = level
#define FAILCODE(code)   args->cla_Result2 = code


/*****************************************************************************/


struct ExecBase *SysBase;
struct Library  *DOSBase;
struct Library  *LocaleBase;
struct Library  *UtilityBase;
struct Library  *GfxBase;
struct Task     *parentTask;
BPTR             mainHunk;

static struct MinList resourceList;


/*****************************************************************************/


STRPTR ProgramName(VOID)
{
    return(PROGRAMNAME);
}


/*****************************************************************************/


static VOID PrintResourceList(VOID)
{
struct ResourceNode *res;

    OpenCat();

    if (IsListEmpty((struct List *)&resourceList))
    {
        PrintStr(MSG_LR_NO_RESOURCES);
    }
    else
    {
        PrintFStr(MSG_LR_FORMAT,GetStr(MSG_LR_RESTYPE_5),GetStr(MSG_LR_NAME));

        res = (struct ResourceNode *)resourceList.mlh_Head;
        while (res->rn_Link.ln_Succ)
        {
            PrintFStr(MSG_LR_FORMAT,GetStr(MSG_LR_RESTYPE_1 + res->rn_Type),res->rn_Name);
            res = (struct ResourceNode *)res->rn_Link.ln_Succ;
        }
    }

    CloseCat();
}


/*****************************************************************************/


static BOOL AddResourceNode(STRPTR name, enum ResourceType type, APTR resource)
{
struct ResourceNode *res;

    if (res = AllocVec(sizeof(struct ResourceNode) + strlen(name) + 1,MEMF_ANY))
    {
        res->rn_Name     = (STRPTR)&res[1];
        res->rn_Type     = type;
        res->rn_Resource = resource;

        strcpy(res->rn_Name,name);

        AddTail((struct List *)&resourceList,(struct Node *)res);

        return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


static VOID RemResourceNode(struct ResourceNode *res)
{
    Remove(res);
    FreeVec(res);
}


/*****************************************************************************/


static struct ResourceNode *FindResourceNode(STRPTR name)
{
    return((struct ResourceNode *)FindNameNC((struct List *)&resourceList,name));
}


/*****************************************************************************/


static LONG LockResource(STRPTR name, BOOL keepLocked)
{
struct Library       *lib;
BPTR                 segment;
struct ResourceNode *res;
struct Catalog      *catalog;
struct TextFont     *font;
struct TextAttr      textAttr;
struct Library      *DiskfontBase;
LONG                 value;
STRPTR               fontName;
ULONG                i, len;
LONG                 result;

    result = 0;

    if (!(res = FindResourceNode(name)))
    {
        segment = LoadSeg(name);
        if (!segment)
        {
            result = IoErr();
            if ((result == ERROR_FILE_NOT_OBJECT) || (result == ERROR_OBJECT_NOT_FOUND) || (result == ERROR_OBJECT_WRONG_TYPE))
            {
                if (LocaleBase)
                {
                    if (catalog = OpenCatalog(NULL,name,OC_BuiltInLanguage,NULL,TAG_DONE))
                    {
                        if (!keepLocked || !AddResourceNode(name,RT_CATALOG,catalog))
                            CloseCatalog(catalog);

                        result = 0;
                    }
                    else
                    {
                        PrintFStr(MSG_LR_COULD_NOT_LOAD,name);
                        result = IoErr();
                    }
                }
            }
            else
            {
                PrintFStr(MSG_LR_LOAD_ERROR,name);
            }
        }
        else
        {
            if (AddSegmentNode(name,segment))
            {
                switch (FindSegmentType(segment))
                {
                    case ST_LIBRARY: if (lib = OpenLibrary(name,0))
                                     {
                                         if (!keepLocked || !AddResourceNode(name,RT_LIBRARY,lib))
                                             CloseLibrary(lib);
                                     }
                                     else
                                     {
                                         PrintFStr(MSG_LR_LOAD_ERROR,name);
                                         result = IoErr();
                                     }
                                     RemSegmentNode(segment);
                                     break;

                    case ST_DEVICE : break;

                    case ST_FONT   : len = strlen(name);

                                     i = len;
                                     while (i && (name[i] != '/') && (name[i] != ':'))
                                         i--;

                                     if (fontName = AllocVec(len+5,MEMF_ANY))  /* +5 for .font */
                                     {
                                         strcpy(fontName,name);
                                         fontName[i] = 0;
                                         strcat(fontName,".font");

                                         if (DiskfontBase = OpenLibrary("diskfont.library",37))
                                         {
                                             StrToLong(FilePart(name),&value);

                                             textAttr.ta_Name  = fontName;
                                             textAttr.ta_YSize = value;
                                             textAttr.ta_Style = 0;
                                             textAttr.ta_Flags = FPF_DISKFONT;

                                             if (font = OpenDiskFont(&textAttr))
                                             {
                                                 if (!keepLocked || !AddResourceNode(name,RT_FONT,font))
                                                     CloseFont(font);
                                             }
                                             else
                                             {
                                                 PrintFStr(MSG_LR_LOAD_ERROR,fontName);
                                                 result = IoErr();
                                             }

                                             CloseLibrary(DiskfontBase);
                                         }
                                         else
                                         {
                                             PrintStr(MSG_LR_CANT_OPEN_DISKFONT);
                                             result = IoErr();
                                         }
                                         FreeVec(fontName);
                                     }
                                     else
                                     {
                                         result = ERROR_NO_FREE_STORE;
                                     }
                                     RemSegmentNode(segment);
                                     break;

                    default        : RemSegmentNode(segment);
                                     PrintFStr(MSG_LR_COULD_NOT_LOAD,name);
                                     result = ERROR_OBJECT_WRONG_TYPE;
                                     break;
                }
            }
            else
            {
                UnLoadSeg(segment);
                result = ERROR_NO_FREE_STORE;
            }
        }
    }
    else
    {
        PrintFStr(MSG_LR_ALREADY_LOCKED,name);
    }

    return(result);
}


/*****************************************************************************/


static VOID UnlockResource(STRPTR name)
{
struct ResourceNode *res;

    if (res = FindResourceNode(name))
    {
        switch (res->rn_Type)
        {
            case RT_LIBRARY: CloseLibrary(res->rn_Resource);
                             break;

            case RT_DEVICE : /* can't happen... */
                             break;

            case RT_FONT   : CloseFont(res->rn_Resource);
                             break;

            case RT_CATALOG: CloseCatalog(res->rn_Resource);
                             break;
        }

        RemResourceNode(res);
    }
    else
    {
        PrintFStr(MSG_LR_NOT_FOUND,name);
    }
}


/*****************************************************************************/


struct ExtAnchorPath
{
    struct AnchorPath eap_Anchor;
    char              eap_Path[256];
};


VOID LoadResource(struct MsgPort *port)
{
struct CmdLineArgs   *args;
BPTR                  oldCD;
BPTR                  oldIn;
BPTR                  oldOut;
STRPTR               *ptr;
struct ExtAnchorPath  anchor;
LONG                  result;
BOOL                  scanError;

    NewList((struct List *)&resourceList);
    InitSegments();

    while (TRUE)
    {
        WaitPort(port);
        args = (struct CmdLineArgs *)GetMsg(port);

        if (args->cla_Version == CLA_VERSION)
        {
            FAILLEVEL(RETURN_OK);
            FAILCODE(0);

            oldCD  = CurrentDir(args->cla_CurrentDir);
            oldIn  = SelectInput(args->cla_Input);
            oldOut = SelectOutput(args->cla_Output);

            if (ptr = (STRPTR *)args->cla_Arguments[OPT_NAME])
            {
                while (*ptr && (args->cla_Result1 == RETURN_OK))
                {
                    memset(&anchor,0,sizeof(struct ExtAnchorPath));
                    anchor.eap_Anchor.ap_Flags     = APF_DOWILD;
                    anchor.eap_Anchor.ap_BreakBits = SIGBREAKF_CTRL_C;
                    anchor.eap_Anchor.ap_Strlen    = sizeof(anchor.eap_Path);

                    scanError = TRUE;
                    if (!(result = MatchFirst(*ptr,&anchor)))
                    {
                        do
                        {
                            if (args->cla_Arguments[OPT_UNLOCK])
                            {
                                UnlockResource(anchor.eap_Anchor.ap_Buf);
                            }
                            else
                            {
                                if (result = LockResource(anchor.eap_Anchor.ap_Buf,args->cla_Arguments[OPT_LOCK]))
                                {
                                    scanError = FALSE;
                                    break;
                                }
                            }
                        }
                        while (!(result = MatchNext(&anchor)));
                    }

                    if (result != ERROR_NO_MORE_ENTRIES)
                    {
                        FAILLEVEL(RETURN_FAIL);
                        FAILCODE(result);

                        if (scanError)
                            PrintFStr(MSG_LR_ERROR,*ptr);
                    }

                    MatchEnd(&anchor);

                    ptr++;
                }
            }
            else
            {
                PrintResourceList();
            }

            if (args->cla_Result2)
                PrintFault(args->cla_Result2,NULL);

            CurrentDir(oldCD);
            SelectInput(oldIn);
            SelectOutput(oldOut);
        }
        else
        {
            FAILLEVEL(RETURN_FAIL);
            FAILCODE(ERROR_OBJECT_WRONG_TYPE);  /* best we can do... */
        }

        /* As Elvis would say, "return to sender" */
        ReplyMsg(args);

        /* If there are no more resources locked, there are no pending
         * segments, and there are no messages in our port, then we can
         * quit
         */
        if (IsListEmpty((struct List *)&resourceList))
        {
            Forbid();

            if (IsMsgPortEmpty(port))
            {
                if (TerminateSegments())
                {
                    RemPort(port);
                    UnLoadSeg(mainHunk);
                    Exit(0);
                }
            }

            Permit();
        }
    }

    /* execution never reaches here */
}
