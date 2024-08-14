
#include        <exec/types.h>
#include        <utility/tagitem.h>
#include        <intuition/intuition.h>
#include        <clib/intuition_protos.h>
#include        <clib/gadtools_protos.h>
#include        <libraries/gadtools.h>
#include        <libraries/asl.h>
#include        <workbench/startup.h>
#include        <exec/memory.h>
#include        <devices/timer.h>
#include        <intuition/gadgetclass.h>
#include        <libraries/iffparse.h>
#include        <envoy/envoy.h>
#include        <pragmas/envoy_pragmas.h>
#include        <clib/envoy_protos.h>
#include        <pragmas/accounts_pragmas.h>
#include        <clib/accounts_protos.h>
#include        <pragmas/intuition_pragmas.h>
#include        <clib/intuition_protos.h>
#include        "fstool.h"
#include        "//fs.h"
#include        "/fsdbase.h"
#undef IntuitionBase
#undef FSDBase
#undef SysBase
#undef DOSBase
#undef UtilityBase
#undef NIPCBase
#undef IFFParseBase
#undef AccountsBase
#undef Mounts

#pragma libcall FSDBase CleanupDeadMount 30 801
#pragma libcall FSDBase SetFSMode 36 001
void SetFSMode( unsigned long mode );

#define TIMERDELAY          15

#define ID_LV2              1
#define ID_UNLOCKCLOSE      2
#define ID_USER             3
#define ID_HOST             4
#define ID_VOLUMENAME       5
#define ID_MOUNT            6

#define ID_LV               1
#define ID_DISCONNECT       2
#define ID_VIEW             3
#define ID_TOTALUSERS       4
#define ID_TOTALLOCKS       5
#define ID_MODE             6

/* I grabbed this from 2.0 Libs RKM, pg. 275 */
UWORD __chip waitPointer[] =
{
   0x0000,0x0000,
   0x0400,0x07c0,
   0x0000,0x07c0,
   0x0100,0x0380,
   0x0000,0x07e0,
   0x07c0,0x1ff8,
   0x1ff0,0x3fec,
   0x3ff8,0x7fde,
   0x3ff8,0x7fbe,
   0x7ffc,0xff7f,
   0x7efc,0xffff,
   0x7ffc,0xffff,
   0x3ff8,0x7ffe,
   0x3ff8,0x7ffe,
   0x1ff0,0x3ffc,
   0x07c0,0x1ff8,
   0x0000,0x07e0,
   0x0000,0x0000
};



STRPTR vv=VERSTAG;

struct Library *IntuitionBase;
struct Library *GadToolsBase;
struct Library *IFFParseBase;
extern struct Library *DOSBase;
struct Library *AccountsBase;
struct Library *EnvoyBase;
struct FSDSvc *FSDBase;
struct List HostUserList;

struct HostUser
{
    struct Node     hu_Node;
    STRPTR          hu_Host;
    STRPTR          hu_User;
    STRPTR          hu_Mount;
    STRPTR          hu_Volume;
    struct List     hu_Locks;
    struct MountedFSInfo *hu_MFSI;
};

struct StoreLock
{
    struct Node     sl_Node;
    UBYTE           sl_Code;
    UBYTE           sl_Filler;
    STRPTR          sl_Path;
};

/* Code above is:
 */
#define CODE_LOCK           0
#define CODE_FILEHANDLE     1


/*
 * requester = SetBusyPointer(struct Window *win)
 *
 * Conditionally creates a busy pointer and locks input to the target
 * window, based on which version of the OS the system is running
 * under.
 *
 * Entry:
 *          win - window to act on
 *
 * Exit:
 *          requester - a pointer to a requester structure that
 *                      must be passed to ResetBusyPointer()
 *                      to remove this condition.
 *
 */
struct Requester * SetBusyPointer(struct Window *win)
{
    ULONG tags[3]={WA_BusyPointer,TRUE,TAG_DONE};
    struct Requester *r;

    r = (struct Requester *) AllocMem(sizeof(struct Requester),MEMF_PUBLIC);
    if (r)
    {
        InitRequester(r);

        Request(r,win);
        if (IntuitionBase->lib_Version < 39)
            SetPointer(win,waitPointer,16,16,-6,0);
        else
            SetWindowPointerA(win,(struct TagItem *) tags);
    }
    return(r);
}

/*
 * ResetBusyPointer(struct Requester *r, struct Window *w)
 *
 * Removes a busy pointer created with SetBusyPointer(),
 * and restores the default pointer.  Also, unlocks input
 * to the target window.
 *
 * Entry:
 *          win - window to act on
 *          r - requester structure pointer from SetBusyPointer().
 *
 * Exit:
 *          none
 */

void ResetBusyPointer(struct Requester *r, struct Window *win)
{
   ULONG tags[5]={WA_Pointer,NULL,WA_BusyPointer,FALSE,TAG_DONE};

    if (IntuitionBase->lib_Version < 39)
        ClearPointer(win);
    else
        SetWindowPointerA(win,(struct TagItem *) tags);
    EndRequest(r,win);
    FreeMem(r,sizeof(struct Requester));
}



void FreeNode(struct HostUser *h)
{
    struct StoreLock *sl;
    while (sl = (struct StoreLock *) RemHead(&h->hu_Locks))
    {
        if (sl->sl_Node.ln_Name)
            FreeMem(sl->sl_Node.ln_Name,30);
        if (sl->sl_Path)
            FreeMem(sl->sl_Path,strlen(sl->sl_Path)+1);
        FreeMem(sl,sizeof(struct StoreLock));
    }
    FreeMem(h->hu_Node.ln_Name,strlen(h->hu_Node.ln_Name)+1);
    FreeMem(h->hu_Host,strlen(h->hu_Host)+1);
    FreeMem(h->hu_User,strlen(h->hu_User)+1);
    FreeMem(h->hu_Mount,strlen(h->hu_Mount)+1);
    FreeMem(h->hu_Volume,strlen(h->hu_Volume)+1);
    FreeMem(h,sizeof(struct HostUser));
}

void FreeList(struct List *l)
{
    struct HostUser *h;
    while (h = (struct HostUser *) RemHead(l))
        FreeNode(h);
}

void BuildList(struct List *l)
{
    struct MountedFSInfo *m;
    struct HostUser *h;
    struct StoreLock *sl;
    struct ResourcesUsed *ru;
    ObtainSemaphore(&FSDBase->FSD_CurrentLock);
    m = (struct MountedFSInfo *) FSDBase->FSD_Current.mlh_Head;
    while (m->mfsi_Link.mln_Succ)
    {
        h = (struct HostUser *) AllocMem(sizeof(struct HostUser),MEMF_CLEAR);
        if (h)
        {
            h->hu_MFSI = m;
            NewList(&h->hu_Locks);
            h->hu_Host = h->hu_User = 0L;
            h->hu_Host = (STRPTR) AllocMem(strlen(m->mfsi_HostName)+1,0);
            if (h->hu_Host)
            {
                strcpy(h->hu_Host,m->mfsi_HostName);
                h->hu_User = (STRPTR) AllocMem(strlen(m->mfsi_UserName)+1,0);
                if (h->hu_User)
                {
                    strcpy(h->hu_User,m->mfsi_UserName);
                    h->hu_Node.ln_Name = (char *) AllocMem(36,0);
                    if (h->hu_Node.ln_Name)
                    {
                        int x;
                        UBYTE tbuff[80];
                        for (x = 0; x < 36; x++)
                            h->hu_Node.ln_Name[x]=' ';
                        h->hu_Node.ln_Name[35]=0;
                        x = strlen(h->hu_Host);
                        x = (x < 16) ? x:15;
                        CopyMem(h->hu_Host,h->hu_Node.ln_Name,x);
                        x = strlen(h->hu_User);
                        x = (x < 10) ? x:10;
                        CopyMem(h->hu_User,&h->hu_Node.ln_Name[16],x);
                        if(NameFromLock(m->mfsi_BaseLock,tbuff,80))
                        {
                            h->hu_Mount = (STRPTR) AllocMem(strlen(tbuff)+1,0);
                            if (h->hu_Mount)
                            {
                                strcpy(h->hu_Mount,tbuff);
                            }
                        }
                        h->hu_Volume = (STRPTR) AllocMem(strlen(m->mfsi_VolumeName)+1,0);
                        if (h->hu_Volume)
                        {
                            strcpy(h->hu_Volume,m->mfsi_VolumeName);
                        }

                        ru = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
                        while (ru->ru_link.ln_Succ)
                        {
                            sl = (struct StoreLock *) AllocMem(sizeof(struct StoreLock),0);
                            if (sl)
                            {
                                UBYTE path[128];
                                if (NameFromLock((BPTR)ru->ru_Resource,path,128))
                                {
                                    sl->sl_Code = CODE_LOCK;
                                    sl->sl_Path = (STRPTR) AllocMem(strlen(path)+1,MEMF_CLEAR);
                                    if (sl->sl_Path)
                                    {
                                        strcpy(sl->sl_Path,path);
                                    }
                                    sl->sl_Node.ln_Name = (char *) AllocMem(30,0);
                                    if (sl->sl_Node.ln_Name)
                                    {
                                        for (x = 0; x < 30; x++)
                                            sl->sl_Node.ln_Name[x]=' ';
                                        sl->sl_Node.ln_Name[29]=0;
                                        CopyMem("<LOCK>",&sl->sl_Node.ln_Name[23],6);
                                        x = strlen(path);
                                        x = (x < 21) ? x:21;
                                        CopyMem(path,sl->sl_Node.ln_Name,x);
                                    }
                                }
                                AddTail(&h->hu_Locks,(struct Node *)sl);
                            }
                            ru = (struct ResourcesUsed *) ru->ru_link.ln_Succ;
                        }

                        ru = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
                        while (ru->ru_link.ln_Succ)
                        {
                            sl = (struct StoreLock *) AllocMem(sizeof(struct StoreLock),0);
                            if (sl)
                            {
                                UBYTE path[128];
                                struct FileHandle *fh;
                                fh = (struct FileHandle *) ru->ru_Resource;
                                if (NameFromFH((BPTR)MKBADDR(fh),path,128))
                                {
                                    sl->sl_Code = CODE_FILEHANDLE;
                                    sl->sl_Path = (STRPTR) AllocMem(strlen(path)+1,MEMF_CLEAR);
                                    if (sl->sl_Path)
                                    {
                                        strcpy(sl->sl_Path,path);
                                    }
                                    sl->sl_Node.ln_Name = (char *) AllocMem(30,0);
                                    if (sl->sl_Node.ln_Name)
                                    {
                                        for (x = 0; x < 30; x++)
                                            sl->sl_Node.ln_Name[x]=' ';
                                        sl->sl_Node.ln_Name[29]=0;
                                        CopyMem("<OPEN>",&sl->sl_Node.ln_Name[23],6);
                                        x = strlen(path);
                                        x = (x < 21) ? x:21;
                                        CopyMem(path,sl->sl_Node.ln_Name,x);
                                    }
                                }
                                AddTail(&h->hu_Locks,(struct Node *)sl);
                            }
                            ru = (struct ResourcesUsed *) ru->ru_link.ln_Succ;
                        }

                        AddTail(l,(struct Node *)h);
                        goto next;

//                        FreeMem(h->hu_Node.ln_Name,36);
                    }
                    FreeMem(h->hu_Host,strlen(h->hu_Host)+1);
                }
                FreeMem(h->hu_Host,strlen(h->hu_User)+1);
            }
            FreeMem(h,sizeof(struct HostUser));
        }
next:
        m = (struct MountedFSInfo *) m->mfsi_Link.mln_Succ;
    }
    ReleaseSemaphore(&FSDBase->FSD_CurrentLock);
}



void Window2(struct HostUser *h, struct timerequest *tio, struct MsgPort *rp)
{
    struct List *LockList;
    struct Screen *s;
    APTR VisInfo;
    struct Gadget *FirstGadget;
    struct Gadget *GadgetList;
    struct TTextAttr topaz8={"topaz.font",8,FS_NORMAL, 0x1, NULL};
    int bh;
    struct MountedFSInfo *sm;

    sm = h->hu_MFSI;

    AbortIO(tio);
    WaitIO(tio);
    tio->tr_time.tv_secs = TIMERDELAY;
    SendIO(tio);

    s = LockPubScreen(NULL);
    bh = s->BarHeight;
    VisInfo = GetVisualInfo(s,TAG_DONE);
    if (VisInfo)
    {
        struct NewGadget NLV={12,20,272,128,"Locks and Files",0,ID_LV2,PLACETEXT_ABOVE,0,0};
        struct Gadget *LV;
        FirstGadget = CreateContext(&GadgetList);
        NLV.ng_TextAttr = (struct TextAttr *) &topaz8;
        NLV.ng_VisualInfo = VisInfo;
        NLV.ng_TopEdge = 20+bh;
        LV = CreateGadget(LISTVIEW_KIND,FirstGadget,&NLV,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,TAG_DONE);
        if (LV)
        {
            struct NewGadget NUSER={384,16,172,14,"User",0,ID_USER,PLACETEXT_LEFT,0,0};
            struct Gadget *USER;
            NUSER.ng_TextAttr = (struct TextAttr *) &topaz8;
            NUSER.ng_VisualInfo = VisInfo;
            NUSER.ng_TopEdge = 16+bh;
            USER = CreateGadget(TEXT_KIND,LV,&NUSER,GTTX_Border,TRUE,TAG_DONE);
            if (USER)
            {
                struct NewGadget NHOST={384,36,172,14,"Host",0,ID_HOST,PLACETEXT_LEFT,0,0};
                struct Gadget *HOST;
                NHOST.ng_TextAttr = (struct TextAttr *) &topaz8;
                NHOST.ng_VisualInfo = VisInfo;
                NHOST.ng_TopEdge = 36+bh;
                HOST = CreateGadget(TEXT_KIND,USER,&NHOST,GTTX_Border,TRUE,TAG_DONE);
                if (HOST)
                {
                    struct NewGadget NVOL ={384,56,172,14,"Volume Name",0,ID_VOLUMENAME,PLACETEXT_LEFT,0,0};
                    struct Gadget *VOL;
                    NVOL.ng_TextAttr = (struct TextAttr *) &topaz8;
                    NVOL.ng_VisualInfo = VisInfo;
                    NVOL.ng_TopEdge = 56+bh;
                    VOL = CreateGadget(TEXT_KIND,HOST,&NVOL,GTTX_Border,TRUE,TAG_DONE);
                    if (VOL)
                    {
                        struct NewGadget NMNT ={384,76,172,14,"Mount",0,ID_MOUNT,PLACETEXT_LEFT,0,0};
                        struct Gadget *MNT;
                        NMNT.ng_TextAttr = (struct TextAttr *) &topaz8;
                        NMNT.ng_VisualInfo = VisInfo;
                        NMNT.ng_TopEdge = 76+bh;
                        MNT = CreateGadget(TEXT_KIND,VOL,&NMNT,GTTX_Border,TRUE,TAG_DONE);
                        if (MNT)
                        {
                            struct Window *CW;
                            CW = OpenWindowTags(NULL,WA_DragBar,TRUE,
                                                     WA_DepthGadget,TRUE,
                                                     WA_Left,0,
                                                     WA_Top,0,
                                                     WA_InnerWidth,558,
                                                     WA_InnerHeight,168,
                                                     WA_Activate,TRUE,
                                                     WA_Title,"Filesystem Export Status",
                                                     WA_SmartRefresh,TRUE,
                                                     WA_Gadgets,FirstGadget,
                                                     WA_CloseGadget,TRUE,
                                                     WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|STRINGIDCMP,
                                                     TAG_DONE,TRUE);

                            if (CW)
                            {
                                struct MsgPort *WPort;
                                struct HostUser *ch;
                                int code=-1;
                                BOOL cont = TRUE;
                                BOOL exit = FALSE;
                                WPort = CW->UserPort;
                                if (s)
                                {
                                    UnlockPubScreen(NULL,s);
                                    s=0;
                                }
                                while (!exit)
                                {
                                    exit = FALSE;
                                    cont = TRUE;
                                    ch = (struct HostUser *) HostUserList.lh_Head;
                                    while (ch->hu_Node.ln_Succ)
                                    {
                                        if (ch->hu_MFSI == sm)
                                            break;
                                        ch = (struct HostUser *) ch->hu_Node.ln_Succ;
                                    }
                                    if (ch->hu_Node.ln_Succ)
                                    {
                                        LockList = &ch->hu_Locks;

                                        /* SetGadgetAttrs here */
                                        GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,LockList,TAG_DONE);
                                        GT_SetGadgetAttrs(USER,CW,NULL,GTTX_Text,ch->hu_User,TAG_DONE);
                                        GT_SetGadgetAttrs(HOST,CW,NULL,GTTX_Text,ch->hu_Host,TAG_DONE);
                                        if (ch->hu_Mount)
                                        {
                                            GT_SetGadgetAttrs(VOL,CW,NULL,GTTX_Text,ch->hu_Volume,TAG_DONE);
                                        }
                                        if (ch->hu_Volume)
                                        {
                                            GT_SetGadgetAttrs(MNT,CW,NULL,GTTX_Text,ch->hu_Mount,TAG_DONE);
                                        }
                                        GT_RefreshWindow(CW,0L);

                                        while (cont)
                                        {
                                            ULONG sigmask;
                                            struct timerequest *tio2;
                                            struct IntuiMessage *im;
                                            sigmask = (1 << WPort->mp_SigBit) | (1 << rp->mp_SigBit);
                                            Wait(sigmask);
                                            while ( (tio2 = (struct timerequest *) GetMsg(rp)) == tio)
                                            {
                                                GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,0L,TAG_DONE);
                                                GT_SetGadgetAttrs(MNT,CW,NULL,GTTX_Text,0,TAG_DONE);
                                                GT_SetGadgetAttrs(VOL,CW,NULL,GTTX_Text,0,TAG_DONE);
                                                GT_SetGadgetAttrs(HOST,CW,NULL,GTTX_Text,0,TAG_DONE);
                                                GT_SetGadgetAttrs(USER,CW,NULL,GTTX_Text,0,TAG_DONE);
                                                FreeList(&HostUserList);
                                                BuildList(&HostUserList);

                                                tio->tr_time.tv_secs = TIMERDELAY;
                                                SendIO(tio);
                                                cont = FALSE;
                                                break;
                                            }
                                            while (im = GT_GetIMsg(WPort))
                                            {
                                                switch ((ULONG) im->Class)
                                                {
                                                    case IDCMP_CLOSEWINDOW:
                                                        cont = FALSE;
                                                        exit = TRUE;
                                                        break;
                                                    case IDCMP_GADGETUP:
                                                    case IDCMP_GADGETDOWN:
                                                    {
                                                        switch ((((struct Gadget *)im->IAddress))->GadgetID)
                                                        {
                                                            case ID_LV2:
                                                            {
                                                                code = im->Code;
                                                                break;
                                                            }
                                                        }
                                                        break;
                                                    }
                                                }
                                                GT_ReplyIMsg(im);
                                            }
                                        }
                                    }
                                    else
                                        exit = TRUE;
                                }
                                GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,0L,TAG_DONE);

                                CloseWindow(CW);
                            }
                        }
                    }

                }
            }
        }
        FreeGadgets(FirstGadget);
    }
    if (s)
        UnlockPubScreen(NULL,s);
}




void ConfigIt()
{

    struct Screen *s;
    APTR VisInfo;
    struct Gadget *FirstGadget;
    struct Gadget *GadgetList;
    struct TTextAttr topaz8={"topaz.font",8,FS_NORMAL, 0x1, NULL};
    int bh;
    struct timerequest *tio;
    struct MsgPort *rp;

    rp = (struct MsgPort *) CreateMsgPort();
    if (rp)
    {
        tio = (struct timerequest *) CreateIORequest(rp,sizeof(struct timerequest));
        if (tio)
        {
            if (!OpenDevice("timer.device",UNIT_VBLANK,tio,0))
            {
                tio->tr_time.tv_secs = TIMERDELAY;
                tio->tr_node.io_Command = TR_ADDREQUEST;
                SendIO(tio);

                NewList(&HostUserList);
                s = LockPubScreen(NULL);
                bh = s->BarHeight;
                VisInfo = GetVisualInfo(s,TAG_DONE);
                if (VisInfo)
                {
                    struct NewGadget NLV={12,20,304,144,"Host                 User    ",0,ID_LV,PLACETEXT_ABOVE,0,0};
                    struct Gadget *LV;
                    FirstGadget = CreateContext(&GadgetList);
                    NLV.ng_TextAttr = (struct TextAttr *) &topaz8;
                    NLV.ng_VisualInfo = VisInfo;
                    NLV.ng_TopEdge = 20+bh;
                    LV = CreateGadget(LISTVIEW_KIND,FirstGadget,&NLV,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,GTLV_ShowSelected,FALSE,TAG_DONE);
                    if (LV)
                    {
                        struct NewGadget NDIS={12,160,152,14,"Disconnect",0,ID_DISCONNECT,PLACETEXT_IN,0,0};
                        struct Gadget *DIS;
                        NDIS.ng_TextAttr = (struct TextAttr *) &topaz8;
                        NDIS.ng_VisualInfo = VisInfo;
                        NDIS.ng_TopEdge = 160+bh;
                        DIS = CreateGadget(BUTTON_KIND,LV,&NDIS,TAG_DONE);
                        if (DIS)
                        {
                            struct NewGadget NVIEW={164,160,152,14,"View",0,ID_VIEW,PLACETEXT_IN,0,0};
                            struct Gadget *VIEW;
                            NVIEW.ng_TextAttr = (struct TextAttr *) &topaz8;
                            NVIEW.ng_VisualInfo = VisInfo;
                            NVIEW.ng_TopEdge = 160+bh;
                            VIEW = CreateGadget(BUTTON_KIND,DIS,&NVIEW,TAG_DONE);
                            if (VIEW)
                            {
                                struct NewGadget NUSER={544,24,48,14,"Total Active Users",0,ID_TOTALUSERS,PLACETEXT_LEFT,0,0};
                                struct Gadget *USER;
                                NUSER.ng_TextAttr = (struct TextAttr *) &topaz8;
                                NUSER.ng_VisualInfo = VisInfo;
                                NUSER.ng_TopEdge = 24+bh;
                                USER = CreateGadget(TEXT_KIND,VIEW,&NUSER,GTTX_Border,TRUE,TAG_DONE);
                                if (USER)
                                {
                                    struct NewGadget NLOCKS ={544,40,48,14,"Total Outstanding Locks",0,ID_TOTALLOCKS,PLACETEXT_LEFT,0,0};
                                    struct Gadget *LOCKS;
                                    NLOCKS.ng_TextAttr = (struct TextAttr *) &topaz8;
                                    NLOCKS.ng_VisualInfo = VisInfo;
                                    NLOCKS.ng_TopEdge = 40+bh;
                                    LOCKS = CreateGadget(TEXT_KIND,USER,&NLOCKS,GTTX_Border,TRUE,TAG_DONE);
                                    if (LOCKS)
                                    {
                                        struct NewGadget NMODE={384,160,172,14,"Mode",0,ID_MODE,PLACETEXT_LEFT,0,0};
                                        struct Gadget *MODE;
                                        STRPTR x[3]={"Normal","Disabled",0L};
                                        NMODE.ng_TextAttr = (struct TextAttr *) &topaz8;
                                        NMODE.ng_VisualInfo = VisInfo;
                                        NMODE.ng_TopEdge = 160 + bh;
                                        MODE = CreateGadget(CYCLE_KIND,LOCKS,&NMODE,GTCY_Labels,x,TAG_DONE);
                                        if (MODE)
                                        {
                                            struct Window *CW;
                                            CW = OpenWindowTags(NULL,WA_DragBar,TRUE,
                                                                     WA_DepthGadget,TRUE,
                                                                     WA_Left,0,
                                                                     WA_Top,0,
                                                                     WA_InnerWidth,597,
                                                                     WA_InnerHeight,180,
                                                                     WA_Activate,TRUE,
                                                                     WA_Title,"Filesystem Export Status",
                                                                     WA_SmartRefresh,TRUE,
                                                                     WA_Gadgets,FirstGadget,
                                                                     WA_CloseGadget,TRUE,
                                                                     WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|STRINGIDCMP,
                                                                     TAG_DONE,TRUE);

                                            if (CW)
                                            {
                                                int locks=0, users=0;
                                                UBYTE LockString[16], UsersString[16];
                                                struct MsgPort *WPort;
                                                struct HostUser *hx;
                                                int code=-1;
                                                BOOL cont = TRUE;
                                                BOOL exit = FALSE;
                                                WPort = CW->UserPort;
                                                if (s)
                                                {
                                                    UnlockPubScreen(NULL,s);
                                                    s=0;
                                                }
                                                /* SetGadgetAttrs here */
                                                while (!exit)
                                                {
                                                    int mode=FSDBase->FSD_Mode;
                                                    exit = FALSE;
                                                    cont = TRUE;
                                                    BuildList(&HostUserList);
                                                    locks = users = 0;
                                                    hx = (struct HostUser *) HostUserList.lh_Head;
                                                    while (hx->hu_Node.ln_Succ)
                                                    {
                                                        struct StoreLock *sl;
                                                        users++;
                                                        sl = (struct StoreLock *) hx->hu_Locks.lh_Head;
                                                        while (sl->sl_Node.ln_Succ)
                                                        {
                                                            locks++;
                                                            sl = (struct StoreLock *) sl->sl_Node.ln_Succ;
                                                        }
                                                        hx = (struct HostUser *) hx->hu_Node.ln_Succ;
                                                    }
                                                    sprintf(LockString,"%ld",locks);
                                                    sprintf(UsersString,"%ld",users);

                                                    GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,&HostUserList,TAG_DONE);

                                                    GT_SetGadgetAttrs(DIS,CW,NULL,GA_Disabled,TRUE,TAG_DONE);
                                                    GT_SetGadgetAttrs(VIEW,CW,NULL,GA_Disabled,TRUE,TAG_DONE);

                                                    GT_SetGadgetAttrs(USER,CW,NULL,GTTX_Text,UsersString,TAG_DONE);
                                                    GT_SetGadgetAttrs(LOCKS,CW,NULL,GTTX_Text,LockString,TAG_DONE);
                                                    GT_SetGadgetAttrs(MODE,CW,NULL,GTCY_Active,mode,TAG_DONE);

                                                    GT_RefreshWindow(CW,0L);

                                                    while (cont)
                                                    {
                                                        ULONG sigmask;
                                                        struct IntuiMessage *im;
                                                        sigmask = (1 << WPort->mp_SigBit) | (1 << rp->mp_SigBit);
                                                        Wait(sigmask);
                                                        if (GetMsg(rp))
                                                        {
                                                            cont=FALSE;
                                                            tio->tr_time.tv_secs = TIMERDELAY;
                                                            SendIO(tio);
                                                        }
                                                        while (im = GT_GetIMsg(WPort))
                                                        {
                                                            switch ((ULONG) im->Class)
                                                            {
                                                                case IDCMP_CLOSEWINDOW:
                                                                    cont = FALSE;
                                                                    exit = TRUE;
                                                                    break;
                                                                case IDCMP_GADGETUP:
                                                                case IDCMP_GADGETDOWN:
                                                                {
                                                                    AbortIO(tio);
                                                                    WaitIO(tio);
                                                                    tio->tr_time.tv_secs = TIMERDELAY;
                                                                    SendIO(tio);
                                                                    switch ((((struct Gadget *)im->IAddress))->GadgetID)
                                                                    {
                                                                        case ID_LV:
                                                                        {
                                                                            code = im->Code;
                                                                            GT_SetGadgetAttrs(DIS,CW,NULL,GA_Disabled,FALSE,TAG_DONE);
                                                                            GT_SetGadgetAttrs(VIEW,CW,NULL,GA_Disabled,FALSE,TAG_DONE);
                                                                            GT_RefreshWindow(CW,0L);
                                                                            break;
                                                                        }
                                                                        case ID_DISCONNECT:
                                                                        {
                                                                            if (code != -1)
                                                                            {
                                                                                struct HostUser *h;
                                                                                struct MountedFSInfo *x;
                                                                                struct SignalSemaphore *ss=&FSDBase->FSD_CurrentLock;
                                                                                int c=code;
                                                                                h = (struct HostUser *) HostUserList.lh_Head;
                                                                                while (c)
                                                                                {
                                                                                    c--;
                                                                                    h = (struct HostUser *) h->hu_Node.ln_Succ;
                                                                                }
                                                                                Remove((struct Node *)h);
                                                                                ObtainSemaphore(ss);
                                                                                x = (struct MountedFSInfo *)FSDBase->FSD_Current.mlh_Head;
                                                                                while (x->mfsi_Link.mln_Succ)
                                                                                {
                                                                                    if (x == h->hu_MFSI)
                                                                                    {
                                                                                        Remove(x);
                                                                                        CleanupDeadMount(x);
                                                                                        break;
                                                                                    }
                                                                                    x = (struct MountedFSInfo *) x->mfsi_Link.mln_Succ;
                                                                                }
                                                                                ReleaseSemaphore(ss);
                                                                                FreeNode(h);

                                                                                code = -1;
                                                                                GT_SetGadgetAttrs(DIS,CW,NULL,GA_Disabled,TRUE,TAG_DONE);
                                                                                GT_SetGadgetAttrs(VIEW,CW,NULL,GA_Disabled,TRUE,TAG_DONE);
                                                                                GT_RefreshWindow(CW,0L);
                                                                                cont = FALSE;
                                                                            }
                                                                            break;
                                                                        }
                                                                        case ID_VIEW:
                                                                        {
                                                                            if (code != -1)
                                                                            {
                                                                                struct HostUser *h;
                                                                                struct Requester *rq;
                                                                                int c=code;
                                                                                h = (struct HostUser *) HostUserList.lh_Head;
                                                                                while (c)
                                                                                {
                                                                                    c--;
                                                                                    h = (struct HostUser *) h->hu_Node.ln_Succ;
                                                                                }
                                                                                AbortIO(tio);
                                                                                WaitIO(tio);
                                                                                rq = SetBusyPointer(CW);
                                                                                Window2(h,tio,rp);
                                                                                ResetBusyPointer(rq,CW);
                                                                                tio->tr_time.tv_secs = TIMERDELAY;
                                                                                SendIO(tio);
                                                                            }
                                                                            break;
                                                                        }
                                                                        case ID_MODE:
                                                                        {
                                                                            BOOL setit=TRUE;
                                                                            struct EasyStruct ers={sizeof(struct EasyStruct),0L,"FS Tool",
                                                                                                   "Changing the mode to 'Disabled' will\ndisconnect all users from the filesystem.\nAre you sure you want to do this?","Yes|No"};
                                                                            if (!mode)
                                                                                setit = EasyRequest(0L,&ers,0L);
                                                                            if (setit)
                                                                            {
                                                                                mode = 1-mode;
                                                                                SetFSMode(mode);
                                                                            }
                                                                            else
                                                                            {
                                                                                GT_SetGadgetAttrs(MODE,CW,NULL,GTCY_Active,mode,TAG_DONE);
                                                                            }
                                                                            break;
                                                                        }
                                                                    }
                                                                    break;
                                                                }
                                                            }
                                                            GT_ReplyIMsg(im);
                                                        }
                                                    }
                                                    GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,0L,TAG_DONE);
                                                    FreeList(&HostUserList);
                                                }
                                                CloseWindow(CW);
                                            }

                                        }
                                    }
                                }

                            }

                        }

                    }
                    FreeGadgets(FirstGadget);
                }
                if (s)
                    UnlockPubScreen(NULL,s);
                AbortIO(tio);
                WaitIO(tio);
                CloseDevice(tio);
            }
            DeleteIORequest(tio);
        }
        DeleteMsgPort(rp);
    }
}

void CouldNotOpen(STRPTR libname)
{
    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Printer Export",
                           "Could not open library\n%s","OK"};
    EasyRequest(0L,&ers,0L,libname);
}


main()
{

    if (DOSBase = (struct Library *) OpenLibrary("dos.library",37L))
    {
        if (IntuitionBase = (struct Library *) OpenLibrary("intuition.library",37L))
        {
            if (GadToolsBase = (struct Library *) OpenLibrary("gadtools.library",37L))
            {
                if (EnvoyBase = (struct Library *) OpenLibrary("envoy.library",37L))
                {
                    if (AccountsBase = (struct Library *) OpenLibrary("accounts.library",37L))
                    {
                        if (FSDBase = (struct FSDSvc *) OpenLibrary("filesystem.service",37L))
                        {
                            ConfigIt();
                            CloseLibrary((struct Library *) FSDBase);
                        }
                        else
                            CouldNotOpen("filesystem.service\nFilesystem Server is not running.");
                        CloseLibrary(AccountsBase);
                    }
                    else
                        CouldNotOpen("accounts.library");
                    CloseLibrary(EnvoyBase);
                }
                else
                    CouldNotOpen("envoy.library");
                CloseLibrary(GadToolsBase);
            }
            else
                CouldNotOpen("gadtools.library");
            CloseLibrary(IntuitionBase);
        }
        CloseLibrary(DOSBase);
    }

}



