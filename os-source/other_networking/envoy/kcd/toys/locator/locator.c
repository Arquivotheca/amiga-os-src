
/* Includes */
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <dos.h>
#include <stdlib.h>

/* #include "locator.h" */
#include "locator_tm.h"

/* Pragma includes for register parameters */

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

/* Library pointers */

extern struct Library *SysBase; /* exec.library */
struct Library *IntuitionBase;  /* intuition.library */
struct Library *GadToolsBase;   /* gadtools.library */
struct Library *NIPCBase;       /* nipc.library */
struct Result
{
    struct Node res_Node;
    UBYTE res_Data[64];
};

struct NameHeader
{
    ULONG   nh_Reserved0;
    UWORD   nh_ID;
    UBYTE   nh_Type;
    UBYTE   nh_Class;
    UWORD   nh_Length;
    UBYTE   nh_RealmLength;
    UBYTE   nh_HostLength;
    UBYTE   nh_ServiceLength;
    UBYTE   nh_EntityLength;
};

struct MinList ResultList;
void Clear_Results(void);
void Do_Query(struct TMData *TMData);
void __saveds __asm MyHookFunc(register __a0 struct Hook *hook,
            register __a2 struct NameHeader *nh,
            register __a1 APTR message);
UBYTE AmigaName[64];
UBYTE EntityName[64];
UBYTE ServiceName[64];
UBYTE RealmName[64];
ULONG query_Class=1;
ULONG query_MaxTime = 5;

/* main function */

VOID main(int argc, char **argv)
{
    struct TMData *TMData = NULL;   /* Data structure pointer */
    ULONG TMError;      /* holds error returned by TM_Open */
    int returncode;     /* holds error returned by TM_Open */

    NewList(&ResultList);

    if (NIPCBase = OpenLibrary("nipc.library",0L))
    {
    if (IntuitionBase = OpenLibrary("intuition.library", 37L))  /* Open
                                     * intuition.library V37 */
    {
        if (GadToolsBase = OpenLibrary("gadtools.library", 37L))        /* Open gadtools.library
                                         * V37 */
        {
        if (!(TMError = TM_Open(&TMData)))  /* Open Toolmaker data */
        {
            Window_LC_Window(TMData, DISPLAY);      /* Display the window */
            TM_EventLoop(TMData);   /* Process events */
            Window_LC_Window(TMData, REMOVE);       /* Remove the window */
            returncode = TMData->ReturnCode;        /* Process events */
            TM_Close(TMData);       /* Close Toolmaker data */
        }
        else
        {
            switch (TMError)
            {
            case TMERROR_MEMORY:
                TM_Request(NULL, "Error", "Error allocating\ndata memory", "Ok");
                break;
            case TMERROR_MSGPORT:
                TM_Request(NULL, "Error", "Error creating\nmessage port", "Ok");
                break;
            case TMERROR_SCREEN:
                TM_Request(NULL, "Error", "Error opening\nscreen", "Ok");
                break;
            case TMERROR_VISUALINFO:
                TM_Request(NULL, "Error", "Error getting\nvisual info", "Ok");
                break;
            }
        }
        CloseLibrary(GadToolsBase); /* Close gadtools.library */
        }
        else
        {
        TM_Request(NULL, "Error", "Error opening\ngadtools.library", "Ok");
        }
        CloseLibrary(IntuitionBase);    /* Close intuition.library */
    }
    CloseLibrary(NIPCBase);
    }
    else
    {
        TM_Request(NULL, "Error", "Error opening\nnipc.library","Ok");
    }
    Clear_Results();
    exit(returncode);
}

/* Window event functions */

BOOL Window_LC_Window_GADGETDOWN(struct TMData * TMData, struct IntuiMessage * imsg)
{
    switch (((struct Gadget *) imsg->IAddress)->GadgetID)
    {
        case ID_LC_Result:  /* Query Result */
        break;
    }
    return (FALSE);
}

BOOL Window_LC_Window_GADGETUP(struct TMData * TMData, struct IntuiMessage * imsg)
{
    switch (((struct Gadget *) imsg->IAddress)->GadgetID)
    {
    case ID_LC_Send:    /* Send Query */
        Clear_Results();
        Do_Query(TMData);
        break;
    case ID_LC_Timeout: /* Timeout */
        query_MaxTime = ((struct StringInfo *)gadget_LC_Timeout->SpecialInfo)->LongInt;
        break;
    case ID_LC_Class:   /* Query Class */
        query_Class = imsg->Code + 1;
        break;
    case ID_LC_Entity:  /* Entity Name */
        strcpy(EntityName,((struct StringInfo *)gadget_LC_Entity->SpecialInfo)->Buffer);
        break;
    case ID_LC_Service: /* Service Name */
        strcpy(ServiceName,((struct StringInfo *)gadget_LC_Service->SpecialInfo)->Buffer);
        break;
    case ID_LC_Host:    /* Amiga Name */
        strcpy(AmigaName,((struct StringInfo *)gadget_LC_Host->SpecialInfo)->Buffer);
        break;
    case ID_LC_Realm:   /* Realm Name */
        strcpy(RealmName,((struct StringInfo *)gadget_LC_Realm->SpecialInfo)->Buffer);
        break;
    }
    return (FALSE);
}

BOOL Window_LC_Window_CLOSEWINDOW(struct TMData * TMData, struct IntuiMessage * imsg)
{
    return (TRUE);
}

BOOL UserSignal(struct TMData * TMData, ULONG signal)
{
    return (FALSE);
}

void Do_Query(struct TMData *TMData)
{
    struct Hook myhook;

    myhook.h_Entry = MyHookFunc;
    myhook.h_SubEntry = NULL;
    myhook.h_Data = &ResultList;

    NetQuery(&myhook,query_MaxTime,
    		numresponses,
    		&myhook,
    		(APTR)FindTask(0L),
          1000L, RealmName,
          1001L, AmigaName,
          1002L, ServiceName,
          1003L, EntityName,
          TAG_DONE);

    Wait(SIGBREAKF_CTRL_F);
    GT_SetGadgetAttrs(gadget_LC_Result,window_LC_Window,NULL,
               GTLV_Labels,-1L,TAG_DONE);

    GT_SetGadgetAttrs(gadget_LC_Result,window_LC_Window,NULL,
               GTLV_Labels,&ResultList,TAG_DONE);

    GT_RefreshWindow(TMData->TMWindowInfo[0].Window, NULL);
}

void __saveds __asm MyHookFunc(register __a0 struct Hook *hook,
            register __a2 struct TagItem *tagList,
            register __a1 struct Task *task)
{
    STRPTR str;
    struct Result *res,*tmp;
    UBYTE i;

    if(nh)
    {
        switch(nh->nh_Class)
        {
            case 1:
                    str = (STRPTR)nh + sizeof(struct NameHeader);
                    tmp = (struct Result *)((struct List *)hook->h_Data)->lh_Head;
                    while(tmp->res_Node.ln_Succ)
                    {
                        if(!stricmp(str,tmp->res_Node.ln_Name))
                            return;
                        tmp = (struct Result *)tmp->res_Node.ln_Succ;
                    }
                    if(res = (struct Result *)AllocMem(sizeof(struct Result),MEMF_CLEAR|MEMF_PUBLIC))
                    {
                        strcpy(res->res_Data,str);
                        res->res_Node.ln_Name = (STRPTR) res->res_Data;
                        AddTail((struct List *)hook->h_Data,(struct Node *)res);
                    }
                    break;
            case 2:
            case 3:
            case 4:
                    switch(nh->nh_Class)
                    {
                    	case 2:  i = nh->nh_ServiceLength;
                    	         break;
                    	case 3:  i = nh->nh_EntityLength;
                    		 break;
                    	case 4:  i = nh->nh_RealmLength;
                    	         break;
                    }
                    str = (STRPTR)nh + sizeof(struct NameHeader);
                    tmp = (struct Result *)((struct List *)hook->h_Data)->lh_Head;
                    while(tmp->res_Node.ln_Succ)
                    {
                        if(!stricmp(str,tmp->res_Node.ln_Name))
                            return;
                        tmp = (struct Result *)tmp->res_Node.ln_Succ;
                    }
                    while(i)
                    {
                        if(res = (struct Result *)AllocMem(sizeof(struct Result),MEMF_CLEAR|MEMF_PUBLIC))
                        {
                            strcpy(res->res_Data,str);
                            res->res_Node.ln_Name = (STRPTR) res->res_Data;
                            AddTail((struct List *)hook->h_Data,(struct Node *)res);
                        }
                        str = (STRPTR) str + strlen(str) + 1;
                        i--;
                    }
                    break;
        }
    }
    else
    {
    	Signal(task,SIGBREAKF_CTRL_F);
    }
}

void Clear_Results()
{
    struct Result *res;
    while(res = (struct Result *) RemHead((struct List *)&ResultList))
        FreeMem(res,sizeof(struct Result));
}