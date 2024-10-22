
#include <exec/types.h>
#include <string.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include "texttable.h"
#include "say_utils.h"


/*****************************************************************************/


extern struct Library     *GfxBase;
extern struct Library     *IntuitionBase;
extern struct Library     *GadToolsBase;

extern struct Window      *wp;
extern struct Screen      *sp;
extern struct LocaleInfo   li;
extern APTR                vi;
extern struct TextAttr     topazAttr;
extern struct Gadget      *lastAdded;


/*****************************************************************************/


VOID DrawSayBevelBox(SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(wp->RPort,x+wp->BorderLeft,
                            y+wp->BorderTop,
                            w,h,(struct TagItem *)&tags);
}


/*****************************************************************************/


VOID SetGadgetAttr(struct Gadget *gad, ULONG tags, ...)
{
    GT_SetGadgetAttrsA(gad,wp,NULL,(struct TagItem *)&tags);
}


/*****************************************************************************/


struct Window *OpenSayWindow(ULONG tag1, ...)
{
    return(OpenWindowTagList(NULL,(struct TagItem *) &tag1));
}


/*****************************************************************************/


struct Gadget *CreateSayGadget(struct SayGadget *sg, ULONG tags, ...)
{
struct NewGadget ng;

    ng.ng_LeftEdge   = sg->sg_LeftEdge+sp->WBorLeft;
    ng.ng_TopEdge    = sg->sg_TopEdge+sp->WBorTop+sp->Font->ta_YSize+1;
    ng.ng_Width      = sg->sg_Width;
    ng.ng_Height     = sg->sg_Height;
    ng.ng_GadgetText = GetString(&li,sg->sg_Label);
    ng.ng_TextAttr   = &topazAttr;
    ng.ng_GadgetID   = 0;
    ng.ng_Flags      = sg->sg_GadgetFlags;
    ng.ng_VisualInfo = vi;
    ng.ng_UserData   = (APTR)sg->sg_Command;

    return(lastAdded = CreateGadgetA(sg->sg_GadgetKind,lastAdded,&ng,(struct TagItem *)&tags));
}


/*****************************************************************************/


VOID CenterLine(AppStringsID id, UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&li,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID ShowError(enum SayStatus error)
{
struct EasyStruct est;
AppStringsID      body;

    switch (error)
    {
        case SS_NOLIBRARY	: body = MSG_SAY_NO_TRANSLATOR;
                                  break;

        case SS_NODEVICE	: body = MSG_SAY_NO_NARRATOR;
                                  break;

        case SS_NOMEMORY	: body = MSG_SAY_NO_MEMORY;
                                  break;

        case SS_STRINGTOOLONG	: body = MSG_SAY_STRING_TOO_LONG;
                                  break;

        case SS_BADFILE		: body = MSG_SAY_FILE_ERROR;
                                  break;

        default                 : body = MSG_NOTHING;
    }

    if (body != MSG_NOTHING)
    {
        est.es_StructSize   = sizeof(struct EasyStruct);
        est.es_Flags        = 0;
        est.es_Title        = GetString(&li,MSG_PROGRAM_ERROR);
        est.es_TextFormat   = GetString(&li,body);
        est.es_GadgetFormat = GetString(&li,MSG_SAY_OK_GAD);

        EasyRequestArgs(wp,&est,NULL,NULL);
    }
}
