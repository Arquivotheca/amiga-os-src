
/* includes */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/keymap.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/locale.h>
#include <dos/dos.h>
#include <dos/exall.h>
#include <string.h>
/* #include <stdio.h> */

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/utility_protos.h>
#include <clib/asl_protos.h>
#include <clib/icon_protos.h>
#include <clib/layers_protos.h>
#include <clib/svc_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/svc_pragmas.h>

/* application includes */
#include "pe_custom.h"
#include "pe_strings.h"
#include "pe_utils.h"
#include "pe_iff.h"
#include <devices/sana2.h>
#include <envoy/services.h>

#define SysBase ed->ed_SysBase

/*****************************************************************************/


/* The IFF chunks known to this prefs editor. IFFPrefChunkCnt says how many
 * chunks there are
 */
#define IFFPrefChunkCnt 2
static LONG far IFFPrefChunks[] =
{
    ID_PREF, ID_PRHD,
    ID_PREF, ID_ISVC,
};


/*****************************************************************************/


/* The PrefHeader structure this editor outputs */
static struct PrefHeader far IFFPrefHeader =
{
    0,   /* version */
    0,   /* type    */
    0    /* flags   */
};


/*****************************************************************************/


#define WRITE_ALL    0
#define WRITE_WB     1
#define WRITE_SYS    2
#define WRITE_SCREEN 3


/*****************************************************************************/


EdStatus InitEdData(EdDataPtr ed)
{
    InitPrefs(&ed->ed_PrefsDefaults);
    InitPrefs(&ed->ed_PrefsWork);
    InitPrefs(&ed->ed_PrefsInitial);

    return(ES_NORMAL);
}

void InitPrefs(struct ExtPrefs *prefs)
{
    NewList(&prefs->ep_Services);
}

/*****************************************************************************/


VOID CleanUpEdData(EdDataPtr ed)
{
    FreePrefs(ed, &ed->ed_PrefsDefaults);
    FreePrefs(ed, &ed->ed_PrefsWork);
    FreePrefs(ed, &ed->ed_PrefsInitial);

    FreeAslRequest(ed->ed_FileReq);

}

VOID FreePrefs(EdDataPtr ed, struct ExtPrefs *prefs)
{
struct Node *node;

    while(node = RemHead(&prefs->ep_Services))
    {
        if(((struct ServicePrefs *)node)->sp_DiskObject)
            FreeDiskObject(((struct ServicePrefs *)node)->sp_DiskObject);

        FreeMem(node,sizeof(struct ServicePrefs));
    }

}

/*****************************************************************************/


EdStatus ReadPrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
struct IFFService tmp_svc;
struct ServicePrefs *sp;

    if((cn->cn_ID == ID_ISVC) && (cn->cn_Type == ID_PREF))
    {
        if(ReadChunkBytes(iff,&tmp_svc,sizeof(struct IFFService)) == sizeof(struct IFFService))
        {
            if(sp = AllocMem(sizeof(struct ServicePrefs),MEMF_CLEAR|MEMF_PUBLIC))
            {
                CopyMem((APTR)&tmp_svc,(APTR)&sp->sp_Prefs,sizeof(struct IFFService));
                AddTail(&ed->ed_PrefsWork.ep_Services,(struct Node *)sp);
                sp->sp_Node.ln_Name = (STRPTR)sp->sp_Name;
                sp->sp_DiskObject = GetDiskObject(sp->sp_Prefs.is_PathName);
                return(ES_NORMAL);
            }
            return(ES_NO_MEMORY);
        }
        else
            return(ES_IFFERROR);
    }
    else
        return(ES_IFF_UNKNOWNCHUNK);

}

EdStatus OpenPrefs(EdDataPtr ed, STRPTR name)
{
    return(ReadIFF(ed,name,IFFPrefChunks,IFFPrefChunkCnt,ReadPrefs));
}


/*****************************************************************************/


EdStatus WritePrefs(EdDataPtr ed, struct IFFHandle *iff, struct ContextNode *cn)
{
    struct ServicePrefs *sp;

    if ((ed->ed_Write == WRITE_ALL))
    {
        sp = (struct ServicePrefs *)ed->ed_PrefsWork.ep_Services.lh_Head;
        while(sp->sp_Node.ln_Succ)
        {
            if(!PushChunk(iff,0,ID_ISVC,sizeof(struct IFFService)))
            {
                if(WriteChunkBytes(iff,&sp->sp_Prefs,sizeof(struct IFFService)) == sizeof(struct IFFService))
                {
                    PopChunk(iff);
                }
            }
            sp = (struct ServicePrefs *)sp->sp_Node.ln_Succ;
        }

    }
    return(ES_NORMAL);
}


EdStatus SavePrefs(EdDataPtr ed, STRPTR name)
{
    return(WriteIFF(ed,name,&IFFPrefHeader,WritePrefs));
}


/*****************************************************************************/


#define NW_WIDTH     384
#define NW_HEIGHT    127
#define NW_IDCMP     (IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_REFRESHWINDOW | IDCMP_INTUITICKS | BUTTONIDCMP | CHECKBOXIDCMP | SLIDERIDCMP | CYCLEIDCMP | TEXTIDCMP | LISTVIEWIDCMP)
#define NW_FLAGS     (WFLG_ACTIVATE | WFLG_DEPTHGADGET | WFLG_DRAGBAR | WFLG_SIMPLE_REFRESH)
#define NW_MINWIDTH  NW_WIDTH
#define NW_MINHEIGHT NW_HEIGHT
#define NW_MAXWIDTH  NW_WIDTH
#define NW_MAXHEIGHT NW_HEIGHT
#define ZOOMWIDTH    200

struct EdMenu far EM[] = {
    {NM_TITLE,  MSG_PROJECT_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_OPEN,           EC_OPEN, 0},
      {NM_ITEM, MSG_PROJECT_SAVE_AS,        EC_SAVEAS, 0},
      {NM_ITEM, MSG_NOTHING,                EC_NOP, 0},
      {NM_ITEM, MSG_PROJECT_QUIT,           EC_CANCEL, 0},

    {NM_TITLE,  MSG_EDIT_MENU,              EC_NOP, 0},
      {NM_ITEM, MSG_EDIT_RESET_TO_DEFAULTS, EC_RESETALL, 0},
      {NM_ITEM, MSG_EDIT_LAST_SAVED,        EC_LASTSAVED, 0},
      {NM_ITEM, MSG_EDIT_RESTORE,           EC_RESTORE, 0},

    {NM_TITLE,  MSG_OPTIONS_MENU,           EC_NOP, 0},
      {NM_ITEM, MSG_OPTIONS_SAVE_ICONS,     EC_SAVEICONS, CHECKIT|MENUTOGGLE},

    {NM_END,    MSG_NOTHING,                EC_NOP, 0}
};

/* main display gadgets */
struct EdGadget far EG[] = {

    /* Main Panel */

    {BUTTON_KIND,   10,  110,  110, 14, MSG_SAVE_GAD,         EC_SAVE,            0},
    {BUTTON_KIND,   137, 110,  110, 14, MSG_USE_GAD,          EC_USE,             0},
    {BUTTON_KIND,   264, 110,  110, 14, MSG_CANCEL_GAD,       EC_CANCEL,          0},

    {BUTTON_KIND,   10,  88,   102, 14, MSG_SMCF_ENABLE_ALL,  EC_ENABLE_ALL,      0},
    {BUTTON_KIND,   112, 88,   102, 14, MSG_SMCF_DISABLE_ALL, EC_DISABLE_ALL,     0},
    {BUTTON_KIND,   10,  88,   102, 14, MSG_SMCF_ADD_SVC,     EC_ADD_SERVICE,     0},
    {BUTTON_KIND,   112, 88,   102, 14, MSG_SMCF_REMOVE_SVC,  EC_REMOVE_SERVICE,  0},

    {CHECKBOX_KIND, 348, 72,   26,  11, MSG_SMCF_STATUS,      EC_SVC_STATUS,      0},

    {LISTVIEW_KIND, 10,  18,   204, 70, 0,                    EC_SERVICE_LIST,    0}

};

/*****************************************************************************/


BOOL CreateDisplay(EdDataPtr ed)
{
UWORD zoomSize[4];

    zoomSize[0] = -1;
    zoomSize[1] = -1;
    zoomSize[2] = ZOOMWIDTH;
    zoomSize[3] = ed->ed_Screen->WBorTop + ed->ed_Screen->Font->ta_YSize + 1;

//    ed->ed_StatusLabels[0] = GetString(&ed->ed_LocaleInfo,MSG_SMCF_STATUSLABEL0);
//    ed->ed_StatusLabels[1] = GetString(&ed->ed_LocaleInfo,MSG_SMCF_STATUSLABEL1);

    ed->ed_LastAdded = CreateContext(&ed->ed_Gadgets);

    DoPrefsGadget(ed,&EG[0],NULL,TAG_DONE);
//    DoPrefsGadget(ed,&EG[1],NULL,TAG_DONE);
    DoPrefsGadget(ed,&EG[2],NULL,TAG_DONE);
//    DoPrefsGadget(ed,&EG[3],NULL,TAG_DONE);
//    DoPrefsGadget(ed,&EG[4],NULL,TAG_DONE);
    RenderDisplay(ed);

    if ((ed->ed_LastAdded)
    &&  (ed->ed_Menus = CreatePrefsMenus(ed,EM))
    &&  (ed->ed_Window = OpenPrefsWindow(ed,WA_InnerWidth,  NW_WIDTH,
                                            WA_InnerHeight, NW_HEIGHT,
                                            WA_MinWidth,    NW_MINWIDTH,
                                            WA_MinHeight,   NW_MINHEIGHT,
                                            WA_MaxWidth,    NW_MAXWIDTH,
                                            WA_MaxHeight,   NW_MAXHEIGHT,
                                            WA_IDCMP,       NW_IDCMP,
                                            WA_Flags,       NW_FLAGS,
                                            WA_Zoom,        zoomSize,
                                            WA_AutoAdjust,  TRUE,
                                            WA_PubScreen,   ed->ed_Screen,
                                            WA_Title,       GetString(&ed->ed_LocaleInfo,MSG_SMCF_NAME),
                                            WA_NewLookMenus,TRUE,
                                            WA_Gadgets,     ed->ed_Gadgets,
                                            TAG_DONE)))
    {
        return(TRUE);
    }

    DisposeDisplay(ed);
    return(FALSE);
}


/*****************************************************************************/


VOID DisposeDisplay(EdDataPtr ed)
{
    if (ed->ed_Window)
    {
        ClearMenuStrip(ed->ed_Window);
        CloseWindow(ed->ed_Window);
    }
    FreeMenus(ed->ed_Menus);
    FreeGadgets(ed->ed_Gadgets);
}


/*****************************************************************************/


VOID CenterLine(EdDataPtr ed, struct Window *wp, AppStringsID id,
                UWORD x, UWORD y, UWORD w)
{
STRPTR str;
UWORD  len;

    str = GetString(&ed->ed_LocaleInfo,id);
    len = strlen(str);

    Move(wp->RPort,(w-TextLength(wp->RPort,str,len)) / 2 + wp->BorderLeft + x,wp->BorderTop+y);
    Text(wp->RPort,str,len);
}


/*****************************************************************************/


VOID DrawBB(EdDataPtr ed, SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...)
{
    DrawBevelBoxA(ed->ed_Window->RPort,x+ed->ed_Window->BorderLeft,
                                       y+ed->ed_Window->BorderTop,
                                       w,h,(struct TagItem *)&tags);
}

/*****************************************************************************/

/* The following routines written by MKS */

static SHORT Text_Line_Fit(EdDataPtr ed, struct RastPort *rp,SHORT width,char *text)
{
register    SHORT   count=0;
register    SHORT   t=0;
register    SHORT   totWidth=0;

    while ((text[t])&&(text[t]!='\n'))
    {
        totWidth+=TextLength(rp,&text[t],1);
        t++;
        if (totWidth<width) count=t;
        else while (text[t]) t++;
    }

    if (t!=count) for (t=count;t>0;t--) if (text[t]==' ')
    {
        while (text[t]==' ') t--;
        count=t+1;
        t=0;
    }
    return(count);
}

char *Display_Text(EdDataPtr ed, struct RastPort *rp,struct Rectangle *rect,char *text,UBYTE pen)
{
register    char    *c;
register    SHORT   count;
register    SHORT   line;
register    SHORT   linecount=0;
register    SHORT   textsize;
register    SHORT   flag;
register    SHORT   left=rect->MinX;
register    SHORT   top=rect->MinY;
register    SHORT   width=rect->MaxX-rect->MinX+1;
register    SHORT   height=rect->MaxY-rect->MinY+1;

    c=text;

    if (*c=='\r') c++;
    while (*c==' ') c++;

    for (c=text;*c;linecount++)
    {
        c=&c[Text_Line_Fit(ed, rp,width,c)];
        if (*c=='\n')
        {
            c++;
            if (*c=='\r') c++;
        }
        while (*c==' ') c++;
    }

    if (linecount)
    {
        line=linecount*(textsize=(rp->TxHeight+1));
        if (line>height)
        {
            linecount=height/textsize;
            line=linecount*textsize;
        }
        top+=(height-line) >> 1;

        SetAPen(rp,pen);
        SetDrMd(rp,JAM1);

        c=text;

        if (*c=='\r')
        {
            c++;
            flag=FALSE;
        }
        else flag=TRUE;
        while (*c==' ') c++;

        while (linecount--)
        {
            if (count=Text_Line_Fit(ed, rp,width,c))
            {
                Move(rp,left+(flag ? 0 : ((width-TextLength(rp,c,count)) >> 1)),top+rp->TxBaseline);
                Text(rp,c,count);
            }
            top+=textsize;
            c=&c[count];

            if (*c=='\n')
            {
                c++;
                if (*c=='\r')
                {
                    c++;
                    flag=FALSE;
                }
                else flag=TRUE;
            }

            while (*c==' ') c++;
        }
    }

    if (*c)
    {
        if (c[-1]=='\r') c--;
    }
    else c=NULL;
    return(c);
}

/*****************************************************************************/

/* "Modified" MKS code... */

VOID RenderRect(EdDataPtr ed, struct Rectangle *rect)
{
    SetAPen(ed->ed_Window->RPort,0);
    SetBPen(ed->ed_Window->RPort,0);
    SetDrMd(ed->ed_Window->RPort,JAM1);
    RectFill(ed->ed_Window->RPort,rect->MinX,rect->MinY,rect->MaxX,rect->MaxY);
}

UBYTE NoDiskObject[]="\rNo Icon image available.\n";
UBYTE NoCurrentService[]="\rNo Service selected.\n";

/*****************************************************************************/

/* "Modified" MKS code... */

VOID RenderDiskObject(EdDataPtr ed)
{
struct Rectangle rect;
struct Region *r;
struct Image *im;
UWORD dx, dy;
BOOL inrefresh;

    rect.MinX = 226 + ed->ed_Window->BorderLeft;
    rect.MinY = 21 + ed->ed_Window->BorderTop;

    rect.MaxX = rect.MinX + 141;
    rect.MaxY = rect.MinY + 45;

    RenderRect(ed,&rect);

    if(ed->ed_CurrentService)
    {
        if(ed->ed_CurrentService->sp_DiskObject)
        {
            im = im=(struct Image *)(ed->ed_CurrentService->sp_DiskObject->do_Gadget.GadgetRender);

            dx = (rect.MaxX-rect.MinX-im->Width+1) >> 1;
            if(dx < 0)
                dx = 0;
            dy = (rect.MaxY-rect.MinY-im->Height+1) >> 1;
            if(dy < 0)
                dy = 0;

            if(r = NewRegion())
            {
                if(OrRectRegion(r,&rect))
                {
                    inrefresh = (window->Flags & WFLG_WINDOWREFRESH) ? TRUE : FALSE;
                    if(inrefresh)
                        EndRefresh(ed->ed_Window,FALSE);

                    r = InstallClipRegion(ed->ed_Window->RPort->Layer,r);

                    if(inrefresh)
                        BeginRefresh(ed->ed_Window);

                    DrawImage(ed->ed_Window->RPort,im,rect.MinX+dx,rect.MinY+dy);

                    if(inrefresh)
                        EndRefresh(ed->ed_Window,FALSE);

                    r = InstallClipRegion(ed->ed_Window->RPort->Layer,r);

                    if(inrefresh)
                        BeginRefresh(ed->ed_Window);

                }
                DisposeRegion(r);
            }
        }
        else
        {
            Display_Text(ed, ed->ed_Window->RPort,&rect,NoDiskObject,1);
        }
    }
    else
    {
        Display_Text(ed, ed->ed_Window->RPort,&rect,NoCurrentService,1);
    }
}

/*****************************************************************************/

VOID RenderDisplay(EdDataPtr ed)
{
struct ServicePrefs *sp;
STRPTR statstr,fmt;
LONG able = 0;
ULONG num=~0;
ULONG scan=0;

    sp = (struct ServicePrefs *) ed->ed_PrefsWork.ep_Services.lh_Head;
    while(sp->sp_Node.ln_Succ)
    {
    	if(sp == ed->ed_CurrentService)
    		num = scan;

        if(sp->sp_Prefs.is_Flags & SVCFLAGF_ENABLE)
            statstr = "Enabled";
        else
            statstr = "Disabled";

        if(strlen(sp->sp_Prefs.is_SvcName) <= 12)
            fmt = "%-12s  %8s";
        else
            fmt = "%.10s... %8s";

        sprintf(sp->sp_Name,fmt,sp->sp_Prefs.is_SvcName,statstr);

        sp = (struct ServicePrefs *)sp->sp_Node.ln_Succ;
        scan++;

    }

    if(ed->ed_CurrentService)
    {
        able = (ed->ed_CurrentService->sp_Prefs.is_Flags & SVCFLAGF_ENABLE);
    }

    ed->ed_ServiceList = DoPrefsGadget(ed,&EG[8],ed->ed_ServiceList,
                                       GTLV_Labels,      &ed->ed_PrefsWork.ep_Services,
                                       GTLV_Selected,	 num,
                                       GTLV_ShowSelected,0,
                                       LAYOUTA_SPACING,  1,
                                       GTLV_ScrollWidth, 18,
                                       TAG_DONE);

    EG[5].eg_TopEdge = EG[6].eg_TopEdge = ed->ed_ServiceList->Height + EG[8].eg_TopEdge;

    ed->ed_AddSvc = DoPrefsGadget(ed,&EG[5],ed->ed_AddSvc,TAG_DONE);

    ed->ed_RemoveSvc = DoPrefsGadget(ed,&EG[6],ed->ed_RemoveSvc,
                                        GA_Disabled,    !(ed->ed_CurrentService),
                                        TAG_DONE);

    ed->ed_ServiceStatus = DoPrefsGadget(ed,&EG[7],ed->ed_ServiceStatus,
                                         GTCB_Checked,   able,
                                         GA_Disabled,    !(ed->ed_CurrentService),
                                         TAG_DONE);

    if(window)
    {
        STRPTR str;
        ULONG len;

        DrawBB(ed, 220, 18, 154, 52, GT_VisualInfo, ed->ed_VisualInfo,
                                     TAG_DONE);
        DrawBB(ed, 224, 20, 146, 48, GT_VisualInfo, ed->ed_VisualInfo,
                                     GTBB_Recessed, TRUE);

        SetAPen(ed->ed_Window->RPort,0);
        SetBPen(ed->ed_Window->RPort,0);
        SetDrMd(ed->ed_Window->RPort,JAM1);
        RectFill(ed->ed_Window->RPort,220 + ed->ed_Window->BorderLeft,
                                      ed->ed_Window->BorderTop,
                                      220 + ed->ed_Window->BorderLeft + 154,
                                      ed->ed_Window->BorderTop + 17);

        SetAPen(ed->ed_Window->RPort,1);
        SetDrMd(ed->ed_Window->RPort,JAM1);

        str = GetString(&ed->ed_LocaleInfo,MSG_SMCF_SVCNAME);
        len = strlen(str);

        Move(ed->ed_Window->RPort,10 + ed->ed_Window->BorderLeft,14 + ed->ed_Window->BorderTop);
        Text(ed->ed_Window->RPort,str,len);

        if(ed->ed_CurrentService)
        {
            str = FilePart(ed->ed_CurrentService->sp_Prefs.is_PathName);
            len = strlen(str);
            Move(ed->ed_Window->RPort,220 + ed->ed_Window->BorderLeft + (154 - TextLength(ed->ed_Window->RPort,str,len)) / 2, 14 + ed->ed_Window->BorderTop);
            Text(ed->ed_Window->RPort,str,len);
        }
        str = GetString(&ed->ed_LocaleInfo,MSG_SMCF_SVCSTATUS);
        len = strlen(str);

        Move(ed->ed_Window->RPort,(216 - 18 - TextLength(ed->ed_Window->RPort,str,len) + ed->ed_Window->BorderLeft),14 + ed->ed_Window->BorderTop);
        Text(ed->ed_Window->RPort,str,len);

        RenderDiskObject(ed);
    }
}

/*****************************************************************************/

VOID ProcessSpecialCommand(EdDataPtr ed, EdCommand ec)
{
BOOL refresh;
UWORD icode;
struct Node *node;
struct ServicePrefs *sp;


    refresh = FALSE;

    icode = ed->ed_CurrentMsg.Code;

    switch (ec)
    {
        /* Main Screen */
/*
        case EC_ENABLE_ALL      : sp = (struct ServicePrefs *)ed->ed_PrefsWork.ep_Services.lh_Head;
                                  while(sp->sp_Node.ln_Succ)
                                  {
                                      sp->sp_Prefs.is_Flags |= SVCFLAGF_ENABLE;
                                      sp = (struct ServicePrefs *)sp->sp_Node.ln_Succ;
                                  }
                                  refresh = TRUE;
                                  break;

        case EC_DISABLE_ALL     : sp = (struct ServicePrefs *)ed->ed_PrefsWork.ep_Services.lh_Head;
                                  while(sp->sp_Node.ln_Succ)
                                  {
                                      sp->sp_Prefs.is_Flags &= ~SVCFLAGF_ENABLE;
                                      sp = (struct ServicePrefs *)sp->sp_Node.ln_Succ;
                                  }
                                  refresh = TRUE;
                                  break;
*/
        case EC_ADD_SERVICE     : AddReqService(ed);
                                  refresh = TRUE;
                                  break;

        case EC_REMOVE_SERVICE  : ed->ed_ServiceList = DoPrefsGadget(ed,&EG[8],ed->ed_ServiceList,
                                                                               GTLV_Labels,      ~0,
                                                                               TAG_DONE);
                                  RemoveService(ed, ed->ed_CurrentService);
                                  ed->ed_CurrentService=NULL;
                                  refresh = TRUE;
                                  break;

        case EC_SVC_STATUS      : if(ed->ed_ServiceStatus->Flags & GFLG_SELECTED)
        			      ed->ed_CurrentService->sp_Prefs.is_Flags |= SVCFLAGF_ENABLE;
        			  else
        			      ed->ed_CurrentService->sp_Prefs.is_Flags &= ~SVCFLAGF_ENABLE;
                                  refresh = TRUE;
                                  break;

        case EC_SERVICE_LIST    : node = ed->ed_PrefsWork.ep_Services.lh_Head;
                                  while(icode--)
                                  {
                                      node = node->ln_Succ;
                                  }
                                  ed->ed_CurrentService = (struct ServicePrefs *) node;
                                  refresh = TRUE;
                                  break;
    }
    if (refresh)
        RenderDisplay(ed);

}

/*****************************************************************************/

VOID AddReqService(EdDataPtr ed)
{
struct ServicePrefs *sp;
struct Library *SvcBase;
BPTR lock;

    if(sp = AllocMem(sizeof(struct ServicePrefs),MEMF_CLEAR|MEMF_PUBLIC))
    {
        if(ServiceRequest(ed))
        {
            if(SvcBase = OpenLibrary(ed->ed_NameBuf,0L))
            {
            	if(lock = Lock(ed->ed_NameBuf,ACCESS_READ))
            	{
            	    if(NameFromLock(lock, sp->sp_Prefs.is_PathName, 256))
            	    {
                        SMCGetServiceAttrs(SvcBase, SVCAttrs_Name,sp->sp_Prefs.is_SvcName,
                                    TAG_DONE);
                        CloseLibrary(SvcBase);
                        sp->sp_DiskObject = GetDiskObject(ed->ed_NameBuf);
                        sp->sp_Node.ln_Name = (STRPTR)sp->sp_Name;
                        ed->ed_ServiceList = DoPrefsGadget(ed,&EG[8],ed->ed_ServiceList,
                               GTLV_Labels,      ~0,
                               TAG_DONE);
                        AddTail(&ed->ed_PrefsWork.ep_Services,(struct Node *)sp);
                        ed->ed_CurrentService = sp;
                        return;
                    }
                    UnLock(lock);
                }
            }
        }
        if(ed->ed_SvcReq)
                FreeAslRequest(ed->ed_SvcReq);
        ed->ed_SvcReq = NULL;
        FreeMem(sp,sizeof(struct ServicePrefs));
    }
    return;
}

/*****************************************************************************/

EdStatus AddAppService(EdDataPtr ed, STRPTR name)
{
struct ServicePrefs *sp;
struct Library *SvcBase;

    if(sp = AllocMem(sizeof(struct ServicePrefs),MEMF_CLEAR|MEMF_PUBLIC))
    {
        if(CheckSvcName(ed, name))
        {
            if(SvcBase = OpenLibrary(name,0L))
            {
                strcpy(sp->sp_Prefs.is_PathName,name);
                SMCGetServiceAttrs(SvcBase, SVCAttrs_Name,sp->sp_Prefs.is_SvcName,
                                            TAG_DONE);
                CloseLibrary(SvcBase);
                sp->sp_DiskObject = GetDiskObject(name);
                sp->sp_Node.ln_Name = (STRPTR)sp->sp_Name;
                ed->ed_ServiceList = DoPrefsGadget(ed,&EG[8],ed->ed_ServiceList,
                                       GTLV_Labels,      ~0,
                                       TAG_DONE);

                AddTail(&ed->ed_PrefsWork.ep_Services,(struct Node *)sp);
                ed->ed_CurrentService = sp;
                RenderDisplay(ed);
                return ES_NORMAL;
            }
            return ES_BAD_SERVICE;
        }
        return ES_BAD_SERVICE;
    }
    return ES_NO_MEMORY;
}

/*****************************************************************************/

BOOL CheckSvcName(EdDataPtr ed, STRPTR name)
{
STRPTR tmp;

    tmp = (STRPTR) ((ULONG)name + (ULONG) strlen(name) - 1);

    while(tmp >= name)
    {
        if(*tmp == '.')
        {
            if(!Stricmp(tmp,".service"))
                return TRUE;
            else if(!Stricmp(tmp,".library"))
                return TRUE;
            else
                return FALSE;
        }
        tmp--;
    }
    return FALSE;
}

/*****************************************************************************/

VOID RemoveService(EdDataPtr ed, struct ServicePrefs *svc)
{
    ed->ed_CurrentService = NULL;
    Remove((struct Node *)svc);
    if(svc->sp_DiskObject)
    {
        FreeDiskObject(svc->sp_DiskObject);
    }
    FreeMem(svc,sizeof(struct ServicePrefs));
    return;
}

/*****************************************************************************/

VOID GetSpecialCmdState(EdDataPtr ed, EdCommand ec, CmdStatePtr state)
{
    state->cs_Available = TRUE;
    state->cs_Selected  = FALSE;
}

/*****************************************************************************/

VOID SMCGetServiceAttrs(struct Library *SvcBase, ULONG tag1, ...)
{
    GetServiceAttrsA((struct TagItem *) &tag1);
}

/*****************************************************************************/

VOID CopyPrefs(EdDataPtr ed, struct ExtPrefs *p1, struct ExtPrefs *p2)
{
    struct ServicePrefs *sp1, *sp2;
    if((p1 == (struct ExtPrefs *)&ed->ed_PrefsWork) && (ed->ed_ServiceList))
    {
	ed->ed_ServiceList = DoPrefsGadget(ed,&EG[8],ed->ed_ServiceList,
                       			   GTLV_Labels,      ~0,
					   TAG_DONE);
    }

    while(sp1 = (struct ServicePrefs *)RemHead(&p1->ep_Services))
    {
        if(sp1->sp_DiskObject)
            FreeDiskObject(sp1->sp_DiskObject);
        FreeMem(sp1,sizeof(struct ServicePrefs));
    }

    sp1 = (struct ServicePrefs *)p2->ep_Services.lh_Head;
    while(sp1->sp_Node.ln_Succ)
    {
        if(sp2 = (struct ServicePrefs *)AllocMem(sizeof(struct ServicePrefs),MEMF_PUBLIC|MEMF_CLEAR))
        {
            CopyMem(&sp1->sp_Prefs,&sp2->sp_Prefs,sizeof(struct IFFService));
            sp2->sp_DiskObject = GetDiskObject(sp2->sp_Prefs.is_PathName);
            sp2->sp_Node.ln_Name = (STRPTR) sp2->sp_Name;
            AddTail(&p1->ep_Services,(struct Node *)sp2);
        }
        sp1 = (struct ServicePrefs *) sp1->sp_Node.ln_Succ;
    }
    if((p1 == (struct ExtPrefs *)&ed->ed_PrefsWork) && (window))
    {
        ed->ed_CurrentService = NULL;
        RenderDisplay(ed);
    }
}

/*****************************************************************************/

BOOL ServiceRequest(EdDataPtr ed)
{
BOOL        success;
struct Hook hook;

    hook.h_Entry = IntuiHook;    /* what should this be cast to to avoid warnings?? */

    if (!ed->ed_SvcReq)
    {
        if (!(ed->ed_SvcReq = AllocPrefsRequest(ed, ASL_FileRequest,
                                                 ASLFR_RejectIcons,     TRUE,
                                                 ASL_Pattern,           "#?.(service|library)",
                                                 ASLFR_InitialLeftEdge, window->LeftEdge+12,
                                                 ASLFR_InitialTopEdge,  window->TopEdge+12,
                                                 ASLFR_Window,          ed->ed_Window,
                                                 ASLFR_SleepWindow,     TRUE,
                                                 ASLFR_RejectIcons,     TRUE,
                                                 TAG_DONE)))
        {
            ShowError2(ed,ES_NO_MEMORY);
            return(FALSE);
        }
    }

    success = RequestService(ed,ASLFR_TitleText,    GetString(&ed->ed_LocaleInfo,MSG_REQ_LOAD),
                                      ASLFR_DoSaveMode,   FALSE,
                                      ASLFR_IntuiMsgFunc, &hook,
                                      TAG_DONE);
    if (success)
    {
        stccpy(ed->ed_NameBuf,ed->ed_SvcReq->rf_Dir, NAMEBUFSIZE);
        AddPart(ed->ed_NameBuf,ed->ed_SvcReq->rf_File, NAMEBUFSIZE);
        return(TRUE);
    }

    return(FALSE);
}

BOOL RequestService(EdDataPtr ed, ULONG tag1, ...)
{
    return(AslRequest(ed->ed_SvcReq,(struct TagItem *) &tag1));
}