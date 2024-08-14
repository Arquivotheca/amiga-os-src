
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>
#include <graphics/text.h>
#include "req.h"
#include <utility/tagitem.h>
#include <envoy/nipc.h>
#include <envoy/envoy.h>
#include <dos/dos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include <string.h>
#include "envoybase.h"

#define ID_LISTVIEW     1
#define ID_OK           2
#define ID_CANCEL       3
#define ID_REALMS       4
#define ID_HOST         5

struct NecessaryInfo
{
    struct  List    *Target;
    struct  Library *UBase;
};

typedef ULONG (*HOOK_FUNC)();

void kprintf(STRPTR, ...);

extern struct TextAttr *topaz8;

BOOL __asm CallBack(register __a0 struct Hook *hook,
                    register __a1 struct TagItem *taglist,
                    register __a2 struct Task *task);

#define MAX(x, y) ((x) > (y) ? (x):(y))

/*
 * FindPosition()
 *
 * Calculates the gadget positions given the fonts size and window size.
 *
 */
BOOL FindPosition(struct HostGadgetInfo *h, UWORD winwidth, UWORD winheight, struct RastPort *rp)
{

    char *OK="OK";
    char *Realms="Realms";
    char *Cancel="Cancel";
    char *Host="Host";

    UWORD OKSize, RealmsSize, CancelSize, HostSize;

    UWORD ButtonWidth, ButtonHeight;
    UWORD Spacing;
    UWORD LVHeight;
    UWORD StrHeight;

    OKSize = TextLength(rp,OK,strlen(OK));
    RealmsSize = TextLength(rp,Realms,strlen(Realms));
    CancelSize = TextLength(rp,Cancel,strlen(Cancel));
    HostSize = TextLength(rp,Host,strlen(Host))+16;

    ButtonWidth = MAX(OKSize, RealmsSize);
    ButtonWidth = MAX(ButtonWidth,CancelSize)+16;

/* If the buttons given the specified font won't fit in the given
 * window size, refuse to do anything else.
 */
    if (ButtonWidth*3 + (5*4) > winwidth)
        return(FALSE);

    ButtonHeight = rp->Font->tf_YSize+4;

    Spacing = 2+2+2+2;

    StrHeight = ButtonHeight;

    LVHeight = winheight-Spacing-ButtonHeight-StrHeight;


/* If the listview doesn't have room enough for at least two lines,
 * don't bother trying.
 */
    if (LVHeight < (rp->Font->tf_YSize*2 + 8))
        return(FALSE);

    h->ListViewLeft = 4;
    h->ListViewTop = 2;
    h->ListViewWidth = winwidth-4;
    h->ListViewHeight = LVHeight;

    h->StrTop = h->ListViewTop+h->ListViewHeight+2;
    h->StrLeft = HostSize+8;
    h->StrWidth = winwidth - h->StrLeft;
    h->StrHeight = StrHeight;

    h->Gadget1Top = h->Gadget2Top = h->Gadget3Top = h->StrTop + StrHeight + 2;
    h->Gadget1Height = h->Gadget2Height = h->Gadget3Height = ButtonHeight;
    h->Gadget1Width = h->Gadget2Width = h->Gadget3Width = ButtonWidth;
    h->Gadget1Left = 4;
    h->Gadget2Left = (winwidth-ButtonWidth)/2;
    h->Gadget3Left = (winwidth-ButtonWidth)-4;

    return(TRUE);
}


BOOL CreateRequest(struct Window *ReqWindow, UWORD ww, UWORD wh,
                   struct Gadget **FirstGadget, struct Gadget **LV, struct Gadget **Realms,
                   struct Gadget **Cancel, struct Gadget **String, struct Gadget **OK,
                   struct Screen *onscreen, APTR VisInfo, struct TextFont *goodfont)
{
    struct RastPort *rp;

    if (ReqWindow)
    {

        if (VisInfo)
        {
            struct HostGadgetInfo h;
            struct TTextAttr *text;
            struct Gadget *GadgetList = NULL;
            struct Gadget *NextGadget;

            struct NewGadget lvng={0,0,0,0,"",0,ID_LISTVIEW,PLACETEXT_ABOVE,0,0};
            struct NewGadget okng={0,0,0,0,"OK",0,ID_OK,PLACETEXT_IN,0,0};
            struct NewGadget realmng={0,0,0,0,"Realms",0,ID_REALMS,PLACETEXT_IN,0,0};
            struct NewGadget cancelng={0,0,0,0,"Cancel",0,ID_CANCEL,PLACETEXT_IN,0,0};
            struct NewGadget stringng={0,0,0,0,"Host",0,ID_HOST,PLACETEXT_LEFT,0,0};

            rp = (struct RastPort *) ReqWindow->RPort;
            text = (struct TTextAttr *) onscreen->Font;
            SetFont(rp,goodfont);

            if (!FindPosition(&h,ww,wh,rp))
            {
                /* Failed with the screen font -- set to topaz and try again */
                struct TextFont *df=ENVOYBASE->eb_TopazFont;
                text = (struct TTextAttr *) &topaz8;
                SetFont(rp,df);
                if (!FindPosition(&h,ww,wh,rp))
                {
                    return(FALSE);
                }
            }

            stringng.ng_LeftEdge = h.StrLeft;
            stringng.ng_TopEdge = h.StrTop;
            stringng.ng_Width = h.StrWidth;
            stringng.ng_Height = h.StrHeight;

            lvng.ng_LeftEdge = h.ListViewLeft;
            lvng.ng_TopEdge = h.ListViewTop;
            lvng.ng_Width = h.ListViewWidth;
            lvng.ng_Height = h.ListViewHeight;

            okng.ng_LeftEdge = h.Gadget1Left;
            okng.ng_TopEdge = h.Gadget1Top;
            okng.ng_Width = h.Gadget1Width;
            okng.ng_Height = h.Gadget1Height;

            realmng.ng_LeftEdge = h.Gadget2Left;
            realmng.ng_TopEdge = h.Gadget2Top;
            realmng.ng_Width = h.Gadget2Width;
            realmng.ng_Height = h.Gadget2Height;

            cancelng.ng_LeftEdge = h.Gadget3Left;
            cancelng.ng_TopEdge = h.Gadget3Top;
            cancelng.ng_Width = h.Gadget3Width;
            cancelng.ng_Height = h.Gadget3Height;

            lvng.ng_TextAttr = (struct TextAttr *) text;
            okng.ng_TextAttr = (struct TextAttr *) text;
            realmng.ng_TextAttr = (struct TextAttr *) text;
            cancelng.ng_TextAttr = (struct TextAttr *) text;
            stringng.ng_TextAttr = (struct TextAttr *) text;

            lvng.ng_VisualInfo = VisInfo;
            okng.ng_VisualInfo = VisInfo;
            realmng.ng_VisualInfo = VisInfo;
            cancelng.ng_VisualInfo = VisInfo;
            stringng.ng_VisualInfo = VisInfo;

            *FirstGadget = (struct Gadget *) CreateContext(&GadgetList);
            if (*FirstGadget)
            {
                ULONG ss[4]={GTLV_ShowSelected,0,TAG_DONE,0};
                ULONG t[2]={TAG_DONE,0};
                UWORD extra, vextra=0;
                NextGadget = *FirstGadget;
                if(IntuitionBase->lib_Version < 39)
                    ss[0] = TAG_IGNORE;
                *LV = (struct Gadget *) CreateGadgetA(LISTVIEW_KIND,NextGadget,&lvng,(struct TagItem *)&ss[0]);
                if (*LV)
                    NextGadget = *LV;
                if (IntuitionBase->lib_Version < 39)
                    vextra = stringng.ng_Height;
                extra = lvng.ng_Height - ((*LV)->Height + vextra);
                *OK = (struct Gadget *) CreateGadgetA(BUTTON_KIND,NextGadget,&okng,(struct TagItem *)&t);
                if (*OK)
                    NextGadget = *OK;
                *Realms = (struct Gadget *) CreateGadgetA(BUTTON_KIND,NextGadget,&realmng,(struct TagItem *)&t);
                if (*Realms)
                    NextGadget = *Realms;
                *Cancel = (struct Gadget *) CreateGadgetA(BUTTON_KIND,NextGadget,&cancelng,(struct TagItem *)&t);
                if (*Cancel)
                    NextGadget = *Cancel;
                stringng.ng_TopEdge -= extra/2;
                *String = (struct Gadget *) CreateGadgetA(STRING_KIND,NextGadget,&stringng,(struct TagItem *)&t);
/* The following aren't necessary w/o another gadget ... */
//                if (*String)
//                    NextGadget = *String;

                AddGList(ReqWindow,*FirstGadget,-1,-1,0);
                SetRast(ReqWindow->RPort,0L);
                RefreshGList(*FirstGadget,ReqWindow,0,-1);
                GT_RefreshWindow(ReqWindow,0L);
            }
        }
    }
    return(TRUE);
}

BOOL DeleteRequest(struct Window *ReqWindow,
                   struct Gadget **FirstGadget)

{
    RemoveGList(ReqWindow,*FirstGadget,-1);
    FreeGadgets(*FirstGadget);
    return(TRUE);
}

BOOL GetList(struct List *tolist,STRPTR startrealm,UBYTE query,struct TagItem *mt)
{
    ULONG searchH[8]={MATCH_REALM,0,QUERY_HOSTNAME,0,TAG_DONE,0,TAG_DONE,0};
    ULONG searchR[4]={QUERY_REALMS,0,TAG_DONE,0};
    ULONG *search;
    struct Hook myhook;
    struct Node *n;
    struct NecessaryInfo *ni;

    if (mt)
    {
        searchH[4]=TAG_MORE;
        searchH[5]=(ULONG)mt;
    }

    ni = (struct NecessaryInfo *) AllocMem(sizeof(struct NecessaryInfo),MEMF_CLEAR);
    if (ni)
    {
        ni->UBase = UtilityBase;
        ni->Target = tolist;
        NewList(tolist);

        searchH[1]=(LONG)startrealm;

        search = &searchH[0];
        if (query == 1)
            search = &searchR[0];

        myhook.h_Entry = (HOOK_FUNC) &CallBack;
        myhook.h_SubEntry = (APTR) ni;

        if (NIPCInquiryA(&myhook,2,500,(struct TagItem *)search))
            Wait(SIGBREAKF_CTRL_F);
        else
            return(FALSE);

        /* Weed out copies */
        n = (struct Node *) tolist->lh_Head;
        while (n->ln_Succ)
        {
            struct Node *m;
            m = n;
            while (m->ln_Succ)
            {
                if ((!strcmp(m->ln_Name,n->ln_Name)) && (m != n))
                {
                    struct Node *o;
                    o = m;
                    m = m->ln_Pred;
                    Remove(o);
                    FreeMem(o->ln_Name,strlen(o->ln_Name)+1);
                    FreeMem(o,sizeof(struct Node));
                }
                m = m->ln_Succ;
            }
            n = n->ln_Succ;
        }
        /* Sort what's left */
        n = (struct Node *) tolist->lh_Head;
        while (n->ln_Succ)
        {
            struct Node *m;
            m = n;
            while (m->ln_Succ)
            {
                if ((Stricmp(m->ln_Name,n->ln_Name) < 0) && (m != n))
                {
                    STRPTR o;
                    o = m->ln_Name;
                    m->ln_Name = n->ln_Name;
                    n->ln_Name = o;
                }
                m = m->ln_Succ;
            }
            n = n->ln_Succ;
        }

        FreeMem(ni,sizeof(struct NecessaryInfo));
    }
    return(TRUE);
}

BOOL DeleteList(struct List *fromlist)
{
    struct Node *n;

    while (n = (struct Node *) RemHead(fromlist))
    {
        FreeMem(n->ln_Name,strlen(n->ln_Name)+1);
        FreeMem(n,sizeof(struct Node));
    }
    return(TRUE);
}

#undef SysBase
#undef UtilityBase
#define SysBase (*(void **)4L)

BOOL __asm CallBack(register __a0 struct Hook *hook,
                             register __a1 struct TagItem *taglist,
                             register __a2 struct Task *task)
{
    struct TagItem *tag;
    struct TagItem *tstate = taglist;
    struct NecessaryInfo *ni = (struct NecessaryInfo *) hook->h_SubEntry;
    struct List *tolist = ni->Target;
    struct Library *UtilityBase = ni->UBase;

    if (taglist)
    {
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case QUERY_REALMS:
                case QUERY_HOSTNAME:
                {
                    STRPTR name;
                    struct Node *newnode;
                    int x;
                    x = strlen((char *)tag->ti_Data);
                    name = (STRPTR) AllocMem(x+1,MEMF_PUBLIC);
                    newnode = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                    if ((name) && (newnode))
                    {
                        STRPTR frm;
                        frm = (char *) tag->ti_Data;
                        while (*frm)
                        {
                            if (*frm == ':')
                            {
                                frm = &frm[1];
                                break;
                            }
                            frm++;
                        }
                        if (!(*frm))                             /* If no :, no realm name exists */
                            frm = (char *) tag->ti_Data;

                        newnode->ln_Name = name;
                        strcpy(name,frm);
                        AddTail(tolist,newnode);
                    }
                    else
                    {
                        if (name)
                            FreeMem(name,x+1);
                        if (newnode)
                            FreeMem(newnode,sizeof(struct Node));
                    }
                    break;
                }
            }
        }
    }
    else
        Signal(task,SIGBREAKF_CTRL_F);

    return(TRUE);
}

#undef SysBase
#define SysBase             (ENVOYBASE->eb_SysBase)
#define UtilityBase         (ENVOYBASE->eb_UtilityBase)

/****** envoy.library/HostRequestA *******************************************
*
*   NAME
*     HostRequestA -- Create a std. requester for selecting a host
*
*   SYNOPSIS
*     ret = HostRequestA(taglist)
*     D0                   A0
*
*     BOOL  HostRequestA(struct TagList *);
*     BOOL  HostRequest(tag1, tag2, ...);
*
*   FUNCTION
*     Creates a system requester that allows the user to search for and
*     select the different hosts and realms known on your network.
*
*   INPUTS
*     taglist - Made up of the following possible tags:
*
*           HREQ_Buffer   - Specifies a pointer to the buffer where you wish
*                           the resolved host and/or realm name to be stored
*                           when the user selected "OK".
*
*                           If a given machine exists in a realm, the string
*                           returned will be "realmname:hostname".
*
*           HREQ_BuffSize - Maximum number of bytes allowed to be copied into
*                           the above buffer.
*
*           HREQ_Left     - Initial left coordinate of the requester.
*
*           HREQ_Top      - Initial top coordinate of the requester.
*
*           HREQ_Width    - Horizontal size of requester in pixels.
*
*           HREQ_Height   - Vertical size of requester in pixels.
*
*           HREQ_DefaultRealm -
*                           Defines the realm to initially search for machines
*                           in when the requester first appears.  (String
*                           should NOT include a ':'.)
*
*           HREQ_NoRealms - Removes the 'realms' button, and prevents users
*                           from switching realms by typing "realm:" in the
*                           string gadget.  The response will only contain a
*                           hostname.
*
*           HREQ_Screen   - Defines the screen on which this requester
*                           should be created.  If not provided, it will be
*                           opened on the workbench screen.
*
*           HREQ_Title    - Provides the name for the title bar of the
*                           requester's window.
*
*           HREQ_NoResizer -
*                           Prevents the requester's window from opening
*                           with a resizer gadget; the requester will be
*                           locked in at the initial size.
*           HREQ_NoDragBar -
*                           Prevents the requester's window from opening
*                           with a dragbar gadget; the requester will be
*                           locked in at the original position.
*
*           MATCH_ ...      Any of the MATCH tags for
*                           nipc.library/NIPCInquiry() can be included, and
*                           will be used to limit hosts that appear to
*                           those that meet the given criteria.
*
*   RESULT
*     ret - either TRUE or FALSE, representing whether the requester was
*           successful or not.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*     nipc.library/NIPCInquiry, nipc.library/GetHostName
*
******************************************************************************
*
*/


BOOL __asm HostRequestA(register __a0 struct TagItem *intags)
{

    BOOL cont, retstat=TRUE;
    struct TagItem *taglist;
    struct TagItem *tag;
    struct List CurrentList;
    UBYTE HostSelection[65];
    UBYTE RealmSelection[65];
    UBYTE TotalSelection[129];
    UBYTE mode=0;

    struct Screen *onscreen;
    struct Screen *ToScreen=0L;
    APTR  VisInfo;

    struct Gadget *FirstGadget;
    struct Gadget *LV;
    struct Gadget *OK;
    struct Gadget *Realms;
    struct Gadget *Cancel;
    struct Gadget *String;
    ULONG t[2]={TAG_DONE,0};

    ULONG stsx[4]={GTST_String,0,TAG_DONE,0};

    ULONG MatchTags[20];
    ULONG mtp=0;

    ULONG wintags[40]={
                                    WA_DragBar,TRUE,                     /* 0 */
                                    WA_DepthGadget,TRUE,                 /* 2 */
                                    WA_Left,0,                           /* 4 */
                                    WA_Top,0,                            /* 6 */
                                    WA_MinWidth,0xEA,                    /* 8 */
                                    WA_MinHeight,0x4F,                   /* 10 */
                                    WA_MaxWidth,-1,                      /* 12 */
                                    WA_MaxHeight,-1,                     /* 14 */
                                    WA_InnerWidth,00,                    /* 16 */
                                    WA_InnerHeight,00,                   /* 18 */
                                    WA_GimmeZeroZero,TRUE,               /* 20 */
                                    WA_Activate,TRUE,                    /* 22 */
                                    WA_SizeGadget,TRUE,                  /* 24 */
                                    WA_Title,(ULONG)"Host Request",      /* 26 */
                                    WA_SmartRefresh,TRUE,                /* 28 */
                                    WA_PubScreen,0L,                     /* 30 */
                                    WA_DragBar,TRUE,                     /* 32 */
                                    WA_SizeBBottom,TRUE,                 /* 34 */
                                    WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|IDCMP_NEWSIZE,
                                    TAG_DONE,TRUE
                      };


    int x, y;
    UWORD ww=300, wh=100;
    UWORD dw, dh;
    struct Window *ReqWindow;
    struct MsgPort *WPort;
    ULONG gta[4]={GTLV_Labels,0L,TAG_DONE,0};
    ULONG gtc[4]={GTLV_Labels,0L,TAG_DONE,0};
    struct TextFont *goodfont;
    struct TextAttr *gfattr;
    BOOL isup;

    STRPTR ExitBuffer=NULL;
    ULONG  ExitBufferLength;

    stsx[1]=(ULONG)&TotalSelection[0];

    gtc[1]= (ULONG) &CurrentList;

    NewList(&CurrentList);

    if (!GetHostName(0L,RealmSelection,128))
        return(FALSE);

    y = strlen(RealmSelection);
    for (x = 0; x < y; x++)
    {
        if (RealmSelection[x] == ':')
        {
            RealmSelection[x]=0;
            break;
        }
    }
    if (x == y)
        RealmSelection[0]=0;

    wintags[17]=ww;
    wintags[19]=wh;

    taglist = intags;
    while (tag = (struct TagItem *) NextTagItem(&taglist))
    {
        switch (tag->ti_Tag)
        {
            case HREQ_Buffer:
                ExitBuffer = (STRPTR) tag->ti_Data;
                break;
            case HREQ_BuffSize:
                ExitBufferLength = (ULONG) tag->ti_Data;
                break;
            case HREQ_Top:
                wintags[7] = tag->ti_Data;
                break;
            case HREQ_Left:
                wintags[5] = tag->ti_Data;
                break;
            case HREQ_Width:
                if (tag->ti_Data >= 0xEA)
                {
                    wintags[17] = tag->ti_Data;
                    ww = tag->ti_Data;
                }
                break;
            case HREQ_Height:
                if (tag->ti_Data >= 0x4F)
                {
                    wintags[19] = tag->ti_Data;
                    wh = tag->ti_Data;
                }
                break;
            case HREQ_DefaultRealm:
                strcpy(RealmSelection,(STRPTR)tag->ti_Data);
                break;
            case HREQ_Screen:
                ToScreen = (struct Screen *) tag->ti_Data;
                break;
            case HREQ_Title:
                wintags[27]= (ULONG) tag->ti_Data;
                break;
            case HREQ_NoDragBar:
                wintags[33] = FALSE;
                break;
            case HREQ_NoResizer:
                wintags[25]=FALSE;
                break;
            case MATCH_REALM:
            case MATCH_IPADDR:
            case MATCH_HOSTNAME:
            case MATCH_SERVICE:
            case MATCH_ENTITY:
            case MATCH_OWNER:
            case MATCH_MACHDESC:
            case MATCH_ATTNFLAGS:
            case MATCH_LIBVERSION:
            case MATCH_CHIPREVBITS:
            case MATCH_MAXFASTMEM:
            case MATCH_AVAILFASTMEM:
            case MATCH_MAXCHIPMEM:
            case MATCH_AVAILCHIPMEM:
            case MATCH_KICKVERSION:
            case MATCH_WBVERSION:
                MatchTags[mtp++]=tag->ti_Tag;
                MatchTags[mtp++]=tag->ti_Data;
                MatchTags[mtp]=TAG_DONE;
                break;
        }
    }

    if (!ToScreen)
        onscreen = (struct Screen *) LockPubScreen(NULL);
    else
        onscreen = ToScreen;

    wintags[31] = (ULONG) onscreen;

    strcpy(TotalSelection,RealmSelection);
    y = strlen(TotalSelection);
    TotalSelection[y]=':';
    TotalSelection[y+1]=0;

    gfattr = onscreen->Font;
    goodfont = OpenFont(gfattr);
    if (goodfont)
    {
        ReqWindow = (struct Window *) OpenWindowTagList(NULL,(struct TagItem *)&wintags);
        if (ReqWindow)
        {
            dw = ReqWindow->Width - ww;
            dh = ReqWindow->Height - wh;
            if ((onscreen) && (!ToScreen))
                UnlockPubScreen(NULL,onscreen);

            WPort = ReqWindow->UserPort;

            VisInfo = (APTR) GetVisualInfoA(onscreen,(struct TagItem *)&t);
            isup = CreateRequest(ReqWindow,ww,wh,&FirstGadget,&LV,&Realms,&Cancel,&String,&OK,onscreen,VisInfo,goodfont);
            if (GetList(&CurrentList,&RealmSelection[0],0,(mtp!=0) ? (struct TagItem *) &MatchTags[0] : (struct TagItem *) 0));
            {
                struct Node *nn;
                ULONG gt[4]={GTLV_Labels,0,TAG_DONE,0};
                gt[1]=(ULONG) &CurrentList;
                nn = CurrentList.lh_Head;
                while (nn->ln_Succ)
                    nn = (struct Node *) nn->ln_Succ;

                GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *) &gt);
                GT_SetGadgetAttrsA(String,ReqWindow,NULL,(struct TagItem *) &stsx[0]);
            }
            GT_RefreshWindow(ReqWindow,0L);

            cont = TRUE;
            while (cont)
            {
                struct IntuiMessage *im;
                ULONG sigmask;
                sigmask = (1 << WPort->mp_SigBit);
                Wait(sigmask);

                while (im = (struct IntuiMessage *) GT_GetIMsg(WPort))
                {
                    switch((ULONG) im->Class)
                    {
                        case IDCMP_NEWSIZE:
                        {
                            ULONG sts[4]={GTST_String,0,TAG_DONE,0};
                            sts[1]=(ULONG) &TotalSelection[0];
                            if (isup)
                            {
                                GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *)&gta);
                                DeleteRequest(ReqWindow, &FirstGadget);
                            }
                            ww = ReqWindow->Width;
                            wh = ReqWindow->Height;
                            VisInfo = (APTR) GetVisualInfoA(onscreen,(struct TagItem *)&t);
                            isup = CreateRequest(ReqWindow,ww-dw,wh-dh,&FirstGadget,&LV,&Realms,&Cancel,&String,&OK,onscreen,VisInfo,goodfont);
                            if (isup)
                            {
                                GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *)&gtc);
                                GT_SetGadgetAttrsA(String,ReqWindow,NULL,(struct TagItem *) &sts);
                                GT_RefreshWindow(ReqWindow,0L);
                            }
                            break;
                        }
                        case IDCMP_GADGETUP:
                        case IDCMP_GADGETDOWN:
                        if (isup)
                        {
                            switch((((struct Gadget *)im->IAddress))->GadgetID)
                            {
                                case ID_CANCEL:
                                    cont = FALSE;
                                    retstat = FALSE;
                                    break;
                                case ID_OK:
                                    if (ExitBuffer)
                                        strncpy(ExitBuffer,TotalSelection,ExitBufferLength);
                                    cont = FALSE;
                                    break;
                                case ID_LISTVIEW:
                                {
                                    ULONG sts[4]={GTST_String,0,TAG_DONE,0};
                                    int acode=im->Code;
                                    struct Node *cn;
                                    sts[1]=(ULONG)&TotalSelection[0];
                                    cn = (struct Node *) CurrentList.lh_Head;
                                    while (acode)
                                    {
                                        cn = (struct Node *) cn->ln_Succ;
                                        acode--;
                                    }
                                    if (mode)
                                    {
                                        strcpy(&RealmSelection[0],cn->ln_Name);
                                        HostSelection[0]=0;
                                    }
                                    else
                                        strcpy(&HostSelection[0],cn->ln_Name);
                                    strcpy(&TotalSelection[0],&RealmSelection[0]);
                                    if (strlen(TotalSelection))
                                    {
                                        int k;
                                        k = strlen(TotalSelection);
                                        TotalSelection[k]=':';
                                        TotalSelection[k+1]=0;
                                    }
                                    strcpy(&TotalSelection[strlen(&TotalSelection[0])],&HostSelection[0]);

                                    GT_SetGadgetAttrsA(String,ReqWindow,NULL,(struct TagItem *) &sts[0]);
                                    if (mode)
                                    {
                                        mode = 0;
                                        GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *) &gta);
                                        DeleteList(&CurrentList);
                                        if (GetList(&CurrentList,&RealmSelection[0],0,(mtp!=0) ? (struct TagItem *) &MatchTags[0] : 0));
                                            GT_SetGadgetAttrsA(LV,ReqWindow,NULL, (struct TagItem *) &gtc);
                                    }
                                    GT_SetGadgetAttrsA(String,ReqWindow,NULL,(struct TagItem *) &sts);
                                    GT_RefreshWindow(ReqWindow,0L);
                                    break;
                                }
                                case ID_HOST:
                                {
                                    ULONG sts[4]={GTST_String,0,TAG_DONE,0};
                                    struct Gadget *sg;
                                    struct StringInfo *si;
                                    STRPTR q;
                                    sts[1]=(ULONG)&TotalSelection[0];
                                    sg = (struct Gadget *) im->IAddress;
                                    si = (struct StringInfo *) sg->SpecialInfo;
                                    q = (STRPTR) si->Buffer;
                                    while ((*q) && (*q != ':'))
                                        q++;
                                    if (*q)
                                    {
                                        ULONG w = (ULONG) q - (ULONG) si->Buffer;
                                        strncpy(&RealmSelection[0],si->Buffer,w);
                                        RealmSelection[w]=0;
                                    }
                                    else
                                        q = (STRPTR) si->Buffer;
                                    strcpy(&HostSelection[0],&q[1]);
                                    strcpy(&TotalSelection[0],&RealmSelection[0]);
                                    if (strlen(TotalSelection))
                                    {
                                        int k;
                                        k = strlen(TotalSelection);
                                        TotalSelection[k]=':';
                                        TotalSelection[k+1]=0;
                                        /* If they typed in a realm name, and hit return, do a 'Hosts' on that realm */
                                        if ((*q == ':'))
                                        {
                                            mode = 0;
                                            GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *)&gta);
                                            DeleteList(&CurrentList);
                                            if (GetList(&CurrentList,&RealmSelection[0],0,(mtp!=0) ? (struct TagItem *) &MatchTags[0] : 0));
                                                GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *)&gtc);
                                        }
                                    }
                                    strcpy(&TotalSelection[strlen(&TotalSelection[0])],&HostSelection[0]);
                                    GT_SetGadgetAttrsA(String,ReqWindow,NULL,(struct TagItem *) &sts);
                                    GT_RefreshWindow(ReqWindow,0L);
                                    break;
                                }
                                case ID_REALMS:
                                    mode = 1;
                                    GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *) &gta);
                                    DeleteList(&CurrentList);
                                    if (GetList(&CurrentList,&RealmSelection[0],1,(mtp!=0) ? (struct TagItem *) &MatchTags[0] : 0));
                                        GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *) &gtc);
                                    break;
                            }
                        }
                    }
                    GT_ReplyIMsg(im);
                }
            }

            GT_SetGadgetAttrsA(LV,ReqWindow,NULL,(struct TagItem *) &gta);

            DeleteList(&CurrentList);

            if (isup)
                DeleteRequest(ReqWindow,&FirstGadget);
            CloseWindow(ReqWindow);

            CloseFont(goodfont);
            onscreen = 0L;
        }
    }
    if ((onscreen) && (!ToScreen))
        UnlockPubScreen(NULL,onscreen);

    return(retstat);
}






