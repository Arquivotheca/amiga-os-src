
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <intuition/gadgetclass.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>
#include <envoy/nipc.h>
#include <envoy/services.h>
#include <envoy/envoy.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/nipc_protos.h>
#include <clib/services_protos.h>
#include <clib/envoy_protos.h>
#include <clib/icon_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/services_pragmas.h>
#include <pragmas/envoy_pragmas.h>
#include <pragmas/icon_pragmas.h>

#include <workbench/workbench.h>
#include <string.h>
#include <stdio.h>
#include "fcconfig_rev.h"

// FIX! should be a shared internal-only include file!
#include "//fs/fs.h"

struct MountNode
{
    struct Node mnode_Node;
    STRPTR      mnode_RealName;
};

#define DLFLAGS (LDF_VOLUMES|LDF_WRITE)
#define ENVOYTYPE  (0x444F5380)
#define ACTION_EFS_INFO 64000

struct AlreadyMounted
{
    struct Node am_Node;
    struct MsgPort *am_Task;
    UBYTE       am_VolName[80];
    UBYTE       am_HostName[80];
    UBYTE       am_RemotePath[80];
};


 STRPTR revisionstring=VERSTAG;
 STRPTR mounttext="Filesystem = l:EnvoyFileSystem\nStackSize = 4000\nPriority = 5\n"
		  "GlobVec = -2\nActivate = 1\nUnit=\"%s�%s�%s�%s�\"\nSurfaces = 0\n"
		  "BlocksPerTrack = 0\nLowCyl = 0\nHighCyl = 0\nDevice =\"Envoy FS\"\nDosType = 0x444f5380";

typedef ULONG (*HOOK_FUNC)();

 extern struct Library *DOSBase;
#define SysBase (*(struct Library  **)4L)
 struct Library *IntuitionBase;
 struct Library *GadToolsBase;
 struct Library *UtilityBase;
 struct Library *NIPCBase;
 struct Library *ServicesBase;
 struct Library *EnvoyBase;
 struct Library *IconBase;
 struct TextAttr topaz8={"topaz.font",8,FS_NORMAL, 0x1};



#define ID_Host             1
#define ID_Create           4
#define ID_Cancel           5
#define ID_LV               6
#define ID_GetHosts         7

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

static struct NewGadget hng ={64,8,188,14,"Host:",0,ID_Host,PLACETEXT_LEFT,0,0};
static struct NewGadget cmng={16,132,112,14,"Connect...",0,ID_Create,PLACETEXT_IN,0,0};
static struct NewGadget cng ={280,132,112,14,"Quit",0,ID_Cancel,PLACETEXT_IN,0,0};
static struct NewGadget lvng={16,40,376,88,"Available Volumes",0,ID_LV,PLACETEXT_ABOVE,0,0};
static struct NewGadget ghng={252,8,140,14,"Select Host...",0,ID_GetHosts,PLACETEXT_IN,0,0};
UBYTE HostName[129];
UBYTE UserName[32];
UBYTE PasswordX[32];
UBYTE MountPath[256];
UBYTE fname[80];
UBYTE devname[80];
UBYTE crypted[33];
ULONG t[20]={MATCH_SERVICE,(ULONG)"Filesystem",HREQ_Buffer,0L,HREQ_BuffSize,128,HREQ_Left,0,
	     HREQ_Top,0,HREQ_Width,0,HREQ_Height,0, /* HREQ_NoResizer,*/ 0,HREQ_Title,
	     (ULONG)"Select Host",TAG_DONE,0};

void DoReq(void);

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


#define FMError_OK         0
#define FMError_BadUser    1
#define FMError_BadConnect 2
#define FMError_NoMounts   3

/*--------------------------------------------------------------------------*/

#define OSIZE 12

STRPTR FCrypt(STRPTR buffer,
              STRPTR password,
              STRPTR user)
{
        int i ;
        int k ;
        long d1 ;
        unsigned int buf[OSIZE];
        UBYTE username[32],*uptr;
        uptr = username;

        for(i=0; i<32; i++)
        {
            username[i] = ToLower(user[i]);
        }

        for(i = 0; i < OSIZE; i++)
        {
                buf[i] = 'A' + (*password? *password++:i) + (*uptr? *uptr++:i);
        }

        for(i = 0; i < OSIZE; i++)
        {
                for(k = 0; k < OSIZE; k++)
                {
                        buf[i] += buf[OSIZE-k-1];
                        UDivMod32((long)buf[i], 53L) ;
                        d1 = getreg(1) ;
                        buf[i] = (unsigned int)d1 ;
                }
                buffer[i] = buf[i] + 'A' ;
        }
        buffer[OSIZE-1] = 0;
        return(buffer) ;
}

/*--------------------------------------------------------------------------*/


/*
 * result = FindMounts(STRPTR host, STRPTR uname, STRPTR pw, struct List *DestList)
 *
 * Given a hostname, username, password, and a list to store entries on,
 * this routine will query a remote filesystem about the mounts that it
 * has available.  MountNode structures will be created for each, with
 * ln_Name pointing to the individual strings for the listview,
 * and mnode_RealName pointing to the actual mountname.
 *
 * Entry:
 *          host -- name of host to query
 *          uname -- username to verify with
 *          pw -- password for above username
 *          DestList -- list for new entries to be put on
 *
 * Exit:
 *          result --
 *                      FMError_OK - no error
 *                      FMError_BadUser - user unknown
 *                      FMError_BadConnect - couldn't get ahold of destination
 *                      FMError_NoMounts - There are no mounts available.
 */

int FindMounts(STRPTR host, STRPTR uname, STRPTR pw, struct List *DestList)
{
    struct Entity *e;
    struct MountNode *h;
    STRPTR m;
    int ecode=FMError_OK;

    struct DosList *n;
    struct List alist;
    struct AlreadyMounted *am;

    NewList(&alist);
    n = (struct DosList *) LockDosList(DLFLAGS);
    if (n)
    {
        while (n = (struct DosList *) NextDosEntry(n,LDF_VOLUMES))
        {
            if (n->dol_misc.dol_volume.dol_DiskType == ENVOYTYPE)
            {
                struct AlreadyMounted *a;
                STRPTR x=BADDR(n->dol_Name);
                a = (struct AlreadyMounted *) AllocMem(sizeof(struct AlreadyMounted),0);
                if (a)
                {
                    CopyMem(&x[1],&a->am_VolName[0],x[0]);
                    a->am_VolName[x[0]]=0;
                    a->am_Task = n->dol_Task;
                    AddTail(&alist,(struct Node *)a);
                }
            }
        }
        UnLockDosList(DLFLAGS);
    }

    am = (struct AlreadyMounted *) alist.lh_Head;
    while (am->am_Node.ln_Succ)
    {
        am->am_HostName[0]=am->am_RemotePath[0]=0;
        DoPkt(am->am_Task,ACTION_EFS_INFO,(LONG)&am->am_HostName[0],(LONG)&am->am_RemotePath[0],80,80,0);
//Printf("%s: host %s, volume %s\n",am->am_VolName,am->am_HostName,am->am_RemotePath);
        am = (struct AlreadyMounted *) am->am_Node.ln_Succ;
    }

    e = CreateEntity(ENT_AllocSignal,NULL,TAG_DONE);
    if (e)
    {
        struct Entity *re;
        re = (struct Entity *) FindService(host,"Filesystem",e,TAG_DONE);
        if (re)
        {
            struct Transaction *t;
            t = AllocTransaction(TRN_AllocReqBuffer,128,TRN_AllocRespBuffer,1024,TAG_DONE);
            if (t)
            {
                struct RequestMounts *rm;
                rm = (struct RequestMounts *) t->trans_RequestData;
                strcpy(rm->User, uname);
                strcpy(rm->Password, pw);
                t->trans_Command = FSCMD_SHOWMOUNTS;
                t->trans_Timeout = 5;
                DoTransaction(re,e,t);
                if (!t->trans_Error)
                {
                    STRPTR x;
                    ULONG f=0;
                    x = (STRPTR) t->trans_ResponseData;
                    while (f < t->trans_RespDataActual)
                    {
                        int q;
                        x[45]=0; /* no strings > 45 chars allowed */
                        if (q=strlen(x))
                        {
                            struct MountNode *h;
                            STRPTR n;
                            STRPTR statc;
                            statc = (STRPTR) AllocMem(47,MEMF_PUBLIC);
                            if (statc)
                            {
                                n = (STRPTR) AllocMem(q+1,MEMF_PUBLIC);
                                if (n)
                                {
                                    strcpy(n,x);
                                    h = (struct MountNode *) AllocMem(sizeof(struct MountNode),MEMF_PUBLIC);
                                    if (h)
                                    {
                                        int r;
                                        struct AlreadyMounted *aa;
                                        h->mnode_RealName = n;
                                        strcpy(statc,n);
                                        for (r = strlen(statc); r < 47; r++)
                                            statc[r]=' ';
                                        statc[46]=0;
                                        /* First, see if this name has any matches */
                                        aa = (struct AlreadyMounted *) alist.lh_Head;
                                        while (aa->am_Node.ln_Succ)
                                        {
                                            if ( (!stricmp(n,&aa->am_RemotePath[0])) &&
                                                 (!stricmp(host,&aa->am_HostName[0])) )
                                                CopyMem("<Connected>",&statc[32],11);
                                            aa = (struct AlreadyMounted *) aa->am_Node.ln_Succ;
                                        }

                                        h->mnode_Node.ln_Name = statc;

                                        AddTail(DestList,(struct Node *)h);
                                    }
                                    else
                                    {
                                        FreeMem(n,q+1);
                                        FreeMem(statc,47);
                                    }
                                }
                                else
                                    FreeMem(statc,47);
                            }
                        }
                        x = &x[64];
                        f += 64;
                    }
                }
                else
                {
                    if (t->trans_Error == FSERR_REJECTED_USER)
                    {
                        ecode = FMError_BadUser;
                        m = "Incorrect User/Password";
                    }
                    else
                    {
                        ecode = FMError_BadConnect;
                        m = "Couldn't contact server";
                    }
                }
                if ((IsListEmpty(DestList)) && (!ecode))
                {
                    ecode = FMError_NoMounts;
                    m = "None available";
                }
                if (ecode)
                {
                    h = (struct MountNode *) AllocMem(sizeof(struct MountNode),MEMF_PUBLIC);
                    if (h)
                    {
                        STRPTR n;
                        h->mnode_RealName=0;
                        n = (STRPTR) AllocMem(47,0);
                        if (n)
                        {
                            strcpy(n,m);
                            h->mnode_Node.ln_Name = n;
                            AddTail(DestList,(struct Node *)h);
                        }
                        else
                            FreeMem(h,sizeof(struct MountNode));
                    }
                }
                FreeTransaction(t);
            }
            LoseService(re);
        }
        else
        {
            ecode = FMError_BadConnect;
            m = "Couldn't contact server";
            h = (struct MountNode *) AllocMem(sizeof(struct MountNode),MEMF_PUBLIC);
            if (h)
            {
                STRPTR n;
                h->mnode_RealName=0;
                n = (STRPTR) AllocMem(47,0);
                if (n)
                {
                    strcpy(n,m);
                    h->mnode_Node.ln_Name = n;
                    AddTail(DestList,(struct Node *)h);
                }
                else
                    FreeMem(h,sizeof(struct MountNode));
            }
        }
        DeleteEntity(e);
    }

    while(am = (struct AlreadyMounted *) RemHead(&alist))
        FreeMem(am,sizeof(struct AlreadyMounted));

    return(ecode);
}

/*
 * DeleteList(struct List *destlist)
 *
 * Given a pointer to a list, remove all nodes from the list,
 * deallocate the null-terminated string pointed to by ln_Name
 * for each, and then deallocate the nodes themselves.
 *
 * Entry:
 *              destlist -- list to act on
 *
 * Exit:
 *              none
 */
void DeleteList(struct List *DestList)
{
    struct MountNode *n;
    while (n = (struct MountNode *) RemHead(DestList))
    {
        FreeMem(n->mnode_Node.ln_Name,47);
        if (n->mnode_RealName)
            FreeMem(n->mnode_RealName,strlen(n->mnode_RealName)+1);
        FreeMem(n,sizeof(struct MountNode));
    }
}

/*
 * success = MakeIcon(STRPTR name)
 *
 * Use icon.library to create a given default icon.
 *
 * Entry:
 *              name -- ptr to string of name of file for which
 *                      icon should be created.  Icon will be
 *                      name+".info".
 *
 * Exit:
 *              success -- either TRUE or FALSE, indicating
 *                         whether the operation has succeeded
 *                         or not.
 *
 */
BOOL MakeIcon(STRPTR name)
{

    struct DiskObject *mdo;
    mdo = (struct DiskObject *) GetDefDiskObject(WBPROJECT);
    if (mdo)
    {
        STRPTR *moresave = mdo->do_ToolTypes;
        STRPTR saveme=mdo->do_DefaultTool;
        STRPTR ptrs[2];
        mdo->do_DefaultTool = "c:mount";
        mdo->do_ToolTypes = &ptrs[0];
        ptrs[0]="DONOTWAIT";
        ptrs[1]=0L;
        PutDiskObject(name,mdo);
        mdo->do_DefaultTool = saveme;
        mdo->do_ToolTypes = moresave;
        FreeDiskObject(mdo);
    }
    else
        return(FALSE);

    return(TRUE);
}

/*
 * DoReq()
 *
 * Provides the functionality of the code -- opens a window,
 * creates several gadgets, and utilizes these to provide the user
 * with choices, and the ability to create mountfiles.
 */

void DoReq()
{

    struct Window *CWindow;

    struct Gadget *First=0L;
    struct Gadget *Next;
    struct Screen *sc;
    APTR VisInfo;
    struct Requester *rx;
    BOOL PossibleMounts=FALSE;

    struct Gadget *Host;
    struct Gadget *Create;
    struct Gadget *Cancel;
    struct Gadget *LV;

    int bh;

    struct List LVList;

    HostName[0]=UserName[0]=PasswordX[0]=MountPath[0]=0;

    NewList(&LVList);
    /* Prepare to create our gadgets, one by one */
    sc = LockPubScreen(NULL);
    bh = sc->BarHeight;
    VisInfo = GetVisualInfo(sc,TAG_DONE);
    UnlockPubScreen(NULL,sc);
    if (VisInfo)
    {
        struct MsgPort *WPort;
        BOOL cont = TRUE;
        ULONG times=0, timem=0;     /* Seconds and Micros for double-clicking the listview */
        BOOL timepressed=FALSE;     /* Have they clicked already in the lv? */
        int   codepressed=0;        /* Which listview entry did they click on last time? */
        BOOL DoAConnect=FALSE;      /* Special kludge to get a "Connect ..." via a dbl-click on the listview */
        BOOL DoItNext=FALSE;        /* Ditto */

        CreateContext(&First);
        Next = First;
        hng.ng_TextAttr = &topaz8;
        cmng.ng_TextAttr = &topaz8;
        cng.ng_TextAttr = &topaz8;
        lvng.ng_TextAttr = &topaz8;
        ghng.ng_TextAttr = &topaz8;
        hng.ng_VisualInfo = VisInfo;
        cmng.ng_VisualInfo = VisInfo;
        cng.ng_VisualInfo = VisInfo;
        lvng.ng_VisualInfo = VisInfo;
        ghng.ng_VisualInfo = VisInfo;
        hng.ng_TopEdge = 8+bh;
        cmng.ng_TopEdge = 132+bh;
        cng.ng_TopEdge = 132+bh;
        lvng.ng_TopEdge = 40+bh;
        ghng.ng_TopEdge = 8+bh;

        /* Create the gadgets */
        Host = CreateGadget(TEXT_KIND,Next,&hng,GTTX_Border,TRUE,TAG_DONE);
        if (Host)
            Next = Host;
        Create = CreateGadget(BUTTON_KIND,Next,&cmng,TAG_DONE);
        if (Create)
            Next = Create;
        Cancel = CreateGadget(BUTTON_KIND,Next,&cng,TAG_DONE);
        if (Cancel)
            Next = Cancel;
        LV = CreateGadget(LISTVIEW_KIND,Next,&lvng,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,GTLV_ShowSelected,FALSE,TAG_DONE);
        if (LV)
            Next = LV;
        CreateGadget(BUTTON_KIND,Next,&ghng,TAG_DONE);

        /* Open our window */
        CWindow = OpenWindowTags(NULL,WA_DragBar,TRUE,
                                      WA_DepthGadget,TRUE,
                                      WA_Left,0,
                                      WA_Top,22,
                                      WA_InnerWidth,402,
                                      WA_InnerHeight,152,
                                      WA_CloseGadget,TRUE,
                                      WA_Activate,TRUE,
                                      WA_Gadgets,First,
                                      WA_Title,(ULONG)"Filesystem Imports",
                                      WA_PubScreen,0L,
                                      WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|IDCMP_NEWSIZE,
                                      WA_SmartRefresh,TRUE,
                                      TAG_DONE,TRUE);
        if (CWindow)
        {

            RefreshGList(First,CWindow,0,-1);

    /* When the editor is brought up, begin in a state where the user is asked
    * for their username/password, and the machine on which they want to
    * mount something.
    */
            if (TRUE)
            {
                t[3]=(ULONG)&HostName[0];
                t[7]=CWindow->LeftEdge + 0;
                t[9]=CWindow->TopEdge + 5;
                t[11]=402;
                t[13]=142;
                if (HostRequestA((struct TagItem *)t))
                {
                    UBYTE us[33];
                    UBYTE ps[33];
                    ULONG t[18]={LREQ_NameBuff,0,LREQ_NameBuffLen,32,
                                 LREQ_PassBuff,0,LREQ_PassBuffLen,32,
                                 LREQ_Title,(ULONG)"Enter Username and Password",
                                 LREQ_OptimWindow,0,
                                 LREQ_UserName,0,
                                 LREQ_Password,0,
                                 TAG_DONE,0};
                    GT_SetGadgetAttrs(Host,CWindow,NULL,GTTX_Text,HostName,TAG_DONE);
                    t[1]=(ULONG) us;
                    t[5]=(ULONG) ps;
                    t[11]=(ULONG) CWindow;
                    t[13]=(ULONG) UserName;
                    t[15]=(ULONG) PasswordX;
                    us[0]=ps[0]=0;
                    if (LoginRequestA((struct TagItem *) t))
                    {
                        strcpy(UserName,us);
                        strcpy(PasswordX,ps);
                        if (UserName[0] && HostName[0])
                        {
                            rx = SetBusyPointer(CWindow);
                            PossibleMounts = TRUE;
                            if (FindMounts(HostName,UserName,PasswordX,&LVList))
                                PossibleMounts = FALSE;
                            ResetBusyPointer(rx,CWindow);
                        }
                    }
                }
            }
            /* Attach our list to the listview gadget */
            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,&LVList,TAG_DONE);

            GT_RefreshWindow(CWindow,0L);


            /* main loop to handle intuition messages */
            WPort = CWindow->UserPort;
            while (cont)
            {
                struct IntuiMessage *im;
                WaitPort(WPort);
                while (im = GT_GetIMsg(WPort))
                {
                    ULONG class,id;
                    if (!DoAConnect)
                    {
                        class = (ULONG) im->Class;
                        if ( (class == IDCMP_GADGETUP) || (class == IDCMP_GADGETDOWN) )
                            id = (((struct Gadget *) im->IAddress))->GadgetID;
                    }
                    else
                    {
                        class = IDCMP_GADGETDOWN;
                        id = ID_Create;
                    }

                    switch(class)
                    {

                        case IDCMP_CLOSEWINDOW:
                            cont = FALSE;
                            break;
                        case IDCMP_GADGETUP:
                        case IDCMP_GADGETDOWN:
                        {
                            switch (id)
                            {
                                /* If they click in the listview, find out which
                                 * entry they clicked on, and copy that string
                                 * (from ln_Name of that Node) into MountPath.
                                 */
                                case ID_LV:
                                {
                                    int code=im->Code;
                                    struct MountNode *am = (struct MountNode *) LVList.lh_Head;


                                    if ((timepressed) && (codepressed == code))
                                    {
                                        if (DoubleClick(times,timem,im->Seconds,im->Micros))
                                        {
                                            DoAConnect=TRUE;
                                            DoItNext = TRUE;
                                        }

                                    }

                                    codepressed = code;
                                    times = im->Seconds;
                                    timem = im->Micros;
                                    timepressed = TRUE;

                                    while ((am->mnode_Node.ln_Succ) && (code))
                                    {
                                        code--;
                                        am = (struct MountNode *) am->mnode_Node.ln_Succ;
                                    }
                                    if ((am->mnode_Node.ln_Succ) && (am->mnode_RealName))
                                        strcpy(MountPath,am->mnode_RealName);
                                    break;
                                }
                                /* If they clicked Cancel, drop out of this loop
                                 * next time around.
                                 */
                                case ID_Cancel:
                                {
                                    cont = FALSE;
                                    break;
                                }
                                /* Give the user the opportunity to reenter their
                                 * username/password and which machine they're
                                 * working on.  When information is present,
                                 * store it in UserName, PasswordX, and HostName.
                                 */
                                case ID_GetHosts:
                                {
                                    t[3]=(ULONG)&HostName[0];
                                    t[7]=CWindow->LeftEdge + 0;
                                    t[9]=CWindow->TopEdge + 5;
                                    t[11]=402;
                                    t[13]=142;
                                    if (HostRequestA((struct TagItem *)t))
                                    {
                                        UBYTE us[33];
                                        UBYTE ps[33];
                                        static ULONG t[18]={LREQ_NameBuff,0,LREQ_NameBuffLen,32,
                                                     LREQ_PassBuff,0,LREQ_PassBuffLen,32,
                                                     LREQ_Title,(ULONG)"Enter Username and Password",
                                                     LREQ_OptimWindow,0,
                                                     LREQ_UserName,0,
                                                     LREQ_Password,0,
                                                     TAG_DONE,0};
                                        GT_SetGadgetAttrs(Host,CWindow,NULL,GTTX_Text,HostName,TAG_DONE);
                                        t[1]=(ULONG) us;
                                        t[5]=(ULONG) ps;
                                        t[11]=(ULONG) CWindow;
                                        t[13]=(ULONG) UserName;
                                        t[15]=(ULONG) PasswordX;
                                        us[0]=ps[0]=0;
                                        if (LoginRequestA((struct TagItem *) t))
                                        {
                                            strcpy(UserName,us);
                                            strcpy(PasswordX,ps);
                                        }

// FIX!!!  no need to enter username!
                                        if (UserName[0] && HostName[0])
                                        {
                                            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,~0L,TAG_DONE);
                                            DeleteList(&LVList);
                                            rx = SetBusyPointer(CWindow);
                                            PossibleMounts = TRUE;
                                            if (FindMounts(HostName,UserName,PasswordX,&LVList))
                                                PossibleMounts = FALSE;
                                            ResetBusyPointer(rx,CWindow);
                                            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,&LVList,TAG_DONE);
					    // deselect
                                            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Selected,~0L,TAG_DONE);
                                            timepressed=FALSE; /* No dbl-clicks on old clicks */
					    MountPath[0] = '\0';	// Nothing selected!
                                        }
                                    }
                                    break;
                                }
                                /* Allow user to choose to create a mountfile for the
                                 * mount selected in the listview.  The mountfile will
                                 * be placed in DEVS:DosDrivers if that directory exists,
                                 * otherwise in SYS:WBStartup (if it exists).  If a temorary
                                 * mount, in t: (then deleted when mounted).
                                 */
                                case ID_Create:
                                {
                                    STRPTR t;

                                    /* We'll try to create a file in ram: (for the moment)
                                     * that holds the contents of a mountfile
                                     */

                                    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Type of Connection?",
                                                           "Would you like this connection\nto be permanent?","Yes|No|Cancel"};
                                    BOOL temporary;
                                    int e;

                                    if ((!PossibleMounts) || (!strlen(MountPath)))
                                    {
                                        struct EasyStruct rrs={sizeof(struct EasyStruct),0L,"Import Error",
                                                           "You must make a selection\nfrom the Available Volumes\nlist before creating a mount.",
                                                           "OK"};
                                        EasyRequest(0L,&rrs,0L);
                                        break;

                                    }
                                    e = EasyRequest(0L,&ers,0L);
                                    if (!e)     /* Cancel */
                                        break;

                                    temporary = (BOOL) (e-1);

                                    t = (STRPTR) AllocMem(512,MEMF_PUBLIC);
                                    if (t)
                                    {
                                        BPTR fh,ih,oh;
                                        STRPTR q;

                                        crypted[0]='$'; /* defines an encrypted pw */
                                        if (!FCrypt(&crypted[1],PasswordX,UserName))
                                            strcpy(crypted,PasswordX);

                                        sprintf(t,mounttext,HostName,MountPath,UserName,crypted);

                                        q = HostName;
                                        while (*q)
                                            if (*q == ':')
                                                break;
                                            else
                                            {
                                                q++;
                                                if (!*q)
                                                {
                                                    q = HostName;
                                                    q--;
                                                    break;
                                                }
                                            }
                                        if (*q)
                                        {
                                            UBYTE nocolon[256];
                                            int xx;
                                            strcpy(nocolon,MountPath);
                                            if (nocolon[strlen(nocolon)-1] == ':')  /* Drop trailing ':'s. */
                                                nocolon[strlen(nocolon)-1] = 0;
                                            for (xx = 0; xx < strlen(nocolon); xx++)    /* Change all other ':'s to '_'s */
                                                if (nocolon[xx] == ':')
                                                    nocolon[xx]='_';
                                            sprintf(devname,"%s-%s:",&q[1],nocolon);
                                            if (temporary)
                                            {
                                                sprintf(fname,"t:%s-%s",&q[1],nocolon);
                                            }
                                            else
                                            {
                                                BPTR lck;
                                                /* Try to get a lock on devs:DosDrivers */
                                                lck = Lock("DEVS:DosDrivers",ACCESS_READ);
                                                if (lck)
                                                {
                                                    sprintf(fname,"DEVS:DosDrivers/%s-%s",&q[1],nocolon);
                                                    UnLock(lck);
                                                }
                                                else
                                                {
                                                    lck = Lock("SYS:WBStartup",ACCESS_READ);
                                                    if (lck)
                                                    {
                                                        sprintf(fname,"SYS:WBStartup/%s-%s",&q[1],nocolon);
                                                        UnLock(lck);
                                                    }
                                                    else
                                                    {
                                                        struct EasyStruct mes={sizeof(struct EasyStruct),0L,"Import Error",
                                                                               "Can't find any logical place\nto put the mountfile!","OK"};
                                                        EasyRequest(0L,&mes,0L);
                                                        FreeMem(t,512);
                                                        break;
                                                    }
                                                }
                                            }

                                            fh = Open(fname,MODE_NEWFILE);
                                            if (fh)
                                            {
                                                UBYTE wcmd[256];
                                                ULONG notasingletag[2]={TAG_DONE};
                                                BPTR tlock;
                                                sprintf(wcmd,"C:Mount >nil: <nil: \"%s\"",fname);
                                                Write(fh,t,strlen(t));
                                                Close(fh);
                                                MakeIcon(fname);
                                                rx = SetBusyPointer(CWindow);
                                                if (ih = Open("nil:",MODE_OLDFILE))
                                                {
                                                    if (oh = Open("nil:",MODE_NEWFILE))
                                                    {
// Fix!???? what was greg doing? - REJ
                                                        ULONG ourtags[6]={SYS_Input,0,SYS_Output,0,TAG_DONE,0};
                                                        ourtags[1]=ih;
                                                        ourtags[3]=oh;
                                                        if (System(wcmd,(struct TagItem *)&notasingletag))    /* mount it now */
                                                        {
                                                            struct EasyStruct mes={sizeof(struct EasyStruct),0L,"Import Error",
                                                                                     "Attempt to issue c:Mount command failed","OK"};
                                                            EasyRequest(0L,&mes,0L);
                                                        }
                                                        Close(oh);
                                                    }
                                                    Close(ih);
                                                }
                                                tlock = Lock(devname,ACCESS_READ);
                                                if (tlock)
                                                {
                                                    struct FileInfoBlock *mf;
                                                    mf = (struct FileInfoBlock *) AllocDosObjectTags(DOS_FIB,TAG_DONE);
                                                    if (mf)
                                                    {
                                                        if (Examine(tlock,mf))
                                                        {
			                                    struct MountNode *am = (struct MountNode *) LVList.lh_Head;
                                                            struct EasyStruct mes={sizeof(struct EasyStruct),0L,"Connection Status",
                                                                                    "Connection Established","OK"};
                                                            EasyRequest(0L,&mes,0L);

		                                            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,~0L,TAG_DONE);

							    // find which node to add <connected> to
							    while (am->mnode_Node.ln_Succ)
							    {
								if (am->mnode_RealName &&
								    strcmp(MountPath,am->mnode_RealName) == 0)
									break;

								am = (struct MountNode *) am->mnode_Node.ln_Succ;
							    }
							    if (am->mnode_Node.ln_Succ)
								CopyMem("<Connected>",&(am->mnode_Node.ln_Name[32]),11);
		                                            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,&LVList,TAG_DONE);
                                                        }
                                                        else
                                                        {
                                                            struct EasyStruct mes={sizeof(struct EasyStruct),0L,"Import Error",
                                                                                    "Could not establish remote\nconnection.","OK"};
                                                            EasyRequest(0L,&mes,0L);
                                                        }
                                                        FreeDosObject(DOS_FIB,mf);
                                                    }
                                                    UnLock(tlock);
                                                }
                                                else
                                                {
                                                    struct EasyStruct mes={sizeof(struct EasyStruct),0L,"Import Error",
                                                                            "Could not establish remote\nconnection.","OK"};
                                                    EasyRequest(0L,&mes,0L);
                                                }

                                                ResetBusyPointer(rx,CWindow);

                                                if (temporary)
                                                {
                                                    DeleteFile(fname);
                                                    sprintf(wcmd,"%s.info",fname);
                                                    DeleteFile(wcmd);   /* nuke .info file too */
                                                }
                                            }
                                        }
                                        FreeMem(t,512);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    if (!((DoAConnect) && (!DoItNext)))  /* In all but the condition for a double-click, reply the imsg */
                        GT_ReplyIMsg(im);               /* (there is no msg in that lone condition) */
                    else
                        DoAConnect = FALSE;
                    DoItNext = FALSE;
                }
            }

            GT_SetGadgetAttrs(LV,CWindow,NULL,GTLV_Labels,0L,TAG_DONE);
            DeleteList(&LVList);

            CloseWindow(CWindow);
        }

        FreeGadgets(First);
    }
}

/*
 * CouldNotOpen(STRPTR libname)
 *
 * Puts up an EasyRequest() indicating that a specific library didn't
 * open.
 *
 * Entry:
 *              libname -- string of library that didn't open.
 *
 * Exit:
 *              none
 */
void CouldNotOpen(STRPTR libname)
{
    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Filesystem Imports",
                           "Could not open library\n%s","OK"};
    EasyRequest(0L,&ers,0L,libname);
}


/* Entry point:
 * Open all the libraries, then call DoReq() to actually to the worthwhile
 * part.
 */
main()
{

    IntuitionBase = (struct Library *) OpenLibrary("intuition.library",37L);
    if (IntuitionBase)
    {
        GadToolsBase = (struct Library *) OpenLibrary("gadtools.library",37L);
        if (GadToolsBase)
        {
            UtilityBase = (struct Library *) OpenLibrary("utility.library",37L);
            if (UtilityBase)
            {
                NIPCBase = (struct Library *) OpenLibrary("nipc.library",37L);
                if (NIPCBase)
                {
                    ServicesBase = (struct Library *) OpenLibrary("services.library",37L);
                    if (ServicesBase)
                    {
                        EnvoyBase = (struct Library *) OpenLibrary("envoy.library",37L);
                        if (EnvoyBase)
                        {
                            IconBase = (struct Library *) OpenLibrary("icon.library",37L);
                            if (IconBase)
                            {
                                DoReq();
                                CloseLibrary(IconBase);
                            }
                            else
                                CouldNotOpen("icon.library");
                            CloseLibrary(EnvoyBase);
                        }
                        else
                            CouldNotOpen("envoy.library");
                        CloseLibrary(ServicesBase);
                    }
                    else
                        CouldNotOpen("services.library");
                    CloseLibrary(NIPCBase);
                }
                else
                    CouldNotOpen("nipc.library");
                CloseLibrary(UtilityBase);
            }
            else
                CouldNotOpen("utility.library");
            CloseLibrary(GadToolsBase);
        }
        else
            CouldNotOpen("gadtools.library");
        CloseLibrary(IntuitionBase);
    }
}

