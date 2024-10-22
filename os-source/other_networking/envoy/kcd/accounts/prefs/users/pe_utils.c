
/* includes */
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>
#include <exec/memory.h>

/* prototypes */
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/* direct ROM interface */
#include <pragmas/intuition_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* application includes */
#include "pe_utils.h"
#include "pe_strings.h"

#define SysBase ed->ed_SysBase;


/*****************************************************************************/


struct Window *OpenPrefsWindow(EdDataPtr ed, ULONG tag1, ...)
{
    return(OpenWindowTagList(NULL,(struct TagItem *) &tag1));
}


/*****************************************************************************/


APTR AllocPrefsRequest(EdDataPtr ed, ULONG type, ULONG tag1, ...)
{
    return(AllocAslRequest(type,(struct TagItem *) &tag1));
}


/*****************************************************************************/


BOOL RequestPrefsFile(EdDataPtr ed, ULONG tag1, ...)
{
    return(AslRequest(ed->ed_FileReq,(struct TagItem *) &tag1));
}


/*****************************************************************************/


BOOL LayoutPrefsMenus(EdDataPtr ed, struct Menu *menus,ULONG tag1, ...)
{
    return(LayoutMenusA(menus,ed->ed_VisualInfo,(struct TagItem *) &tag1));
}


/*****************************************************************************/


struct Menu *CreatePrefsMenus(EdDataPtr ed, struct EdMenu *em)
{
UWORD           i;
struct NewMenu *nm;
struct Menu    *menus;

    i = 0;
    while (em[i++].em_Type != NM_END) {}

    if (!(nm = AllocVec(sizeof(struct NewMenu)*i,MEMF_CLEAR|MEMF_PUBLIC)))
        return(NULL);

    while (i--)
    {
        nm[i].nm_Type        = em[i].em_Type;
        nm[i].nm_Flags       = em[i].em_ItemFlags;
        nm[i].nm_UserData    = (APTR)em[i].em_Command;

        if (em[i].em_Type == NM_TITLE)
        {
            nm[i].nm_Label   = GetString(&ed->ed_LocaleInfo,em[i].em_Label);
        }
        else if (em[i].em_Command == EC_NOP)
        {
            nm[i].nm_Label   = NM_BARLABEL;
        }
        else if (em[i].em_Type != NM_END)
        {
            nm[i].nm_CommKey = GetString(&ed->ed_LocaleInfo,em[i].em_Label);
            nm[i].nm_Label   = &nm[i].nm_CommKey[2];
            if (nm[i].nm_CommKey[0] == ' ')
            {
                nm[i].nm_CommKey = NULL;
            }
        }
    }

    if (menus = CreateMenusA(nm,NULL))
    {
        if (!(LayoutPrefsMenus(ed,menus,GTMN_NewLookMenus,TRUE,
                                        TAG_DONE)))
        {
            FreeMenus(menus);
	    menus = NULL;
	}
    }

    FreeVec(nm);

    return(menus);
}


/*****************************************************************************/


struct Gadget *DoPrefsGadget(EdDataPtr ed, struct EdGadget *eg,
                             struct Gadget *gad, ULONG tags, ...)
{
struct NewGadget ng;

    if (gad)
    {
        GT_SetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
        return(gad);
    }

    ng.ng_LeftEdge   = eg->eg_LeftEdge+ed->ed_Screen->WBorLeft;
    ng.ng_TopEdge    = eg->eg_TopEdge+ed->ed_Screen->WBorTop+ed->ed_Screen->Font->ta_YSize + 1;
    ng.ng_Width      = eg->eg_Width;
    ng.ng_Height     = eg->eg_Height;
    ng.ng_GadgetText = GetString(&ed->ed_LocaleInfo,eg->eg_Label);
    ng.ng_TextAttr   = &ed->ed_TextAttr;
    ng.ng_GadgetID   = eg->eg_Command;
    ng.ng_Flags      = eg->eg_GadgetFlags;
    ng.ng_VisualInfo = ed->ed_VisualInfo;
    ng.ng_UserData   = NULL;

    return(ed->ed_LastAdded = CreateGadgetA(eg->eg_GadgetKind,ed->ed_LastAdded,
                                            &ng,(struct TagItem *)&tags));
}


/*****************************************************************************/


VOID SetGadgetAttr(EdDataPtr ed, struct Gadget *gad, ULONG tags, ...)
{
    GT_SetGadgetAttrsA(gad,ed->ed_Window,NULL,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID ShowError1(EdDataPtr ed, AppStringsID es)
{
struct EasyStruct est;
char              error[100];
STRPTR            ptr[2];

    error[0] = 0;

    if (es == MSG_DOSERROR_STAT)
    {
        Fault(ed->ed_SecondaryResult,NULL,error,100);
        ptr[0] = ed->ed_ErrorFileName;
        ptr[1] = error;
    }
    else if (es == MSG_IFFERROR_STAT)
    {
        ptr[0] = ed->ed_ErrorFileName;
        ptr[1] = NULL;
    }

    est.es_StructSize   = sizeof(struct EasyStruct);
    est.es_Flags        = 0;
    est.es_Title        = GetString(&ed->ed_LocaleInfo,MSG_PROGRAM_ERROR);
    est.es_TextFormat   = GetString(&ed->ed_LocaleInfo,es);
    est.es_GadgetFormat = GetString(&ed->ed_LocaleInfo,MSG_OK_GAD);

    EasyRequestArgs(ed->ed_Window,&est,NULL,&ptr);
}


VOID ShowError2(EdDataPtr ed, EdStatus es)
{
    if (es > ES_DOSERROR)
        ShowError1(ed,(ULONG)es+(ULONG)MSG_PREFS_NAME-(ULONG)ES_DOSERROR);
    else
        ShowError1(ed,(ULONG)es+(ULONG)MSG_NORMAL_STAT);
}
