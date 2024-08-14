
#include        <exec/types.h>
#include        <utility/tagitem.h>
#include        <intuition/intuition.h>
#include        <libraries/gadtools.h>
#include        <libraries/asl.h>
#include        <workbench/startup.h>
#include        <exec/memory.h>
#include        <intuition/gadgetclass.h>
#include        <libraries/iffparse.h>
#include        <envoy/envoy.h>
#include	<string.h>

#include        <clib/intuition_protos.h>
#include        <pragmas/intuition_pragmas.h>
#include        <clib/gadtools_protos.h>
#include        <pragmas/gadtools_pragmas.h>
#include        <clib/envoy_protos.h>
#include        <pragmas/envoy_pragmas.h>
#include        <clib/accounts_protos.h>
#include        <pragmas/accounts_pragmas.h>
#include        <clib/exec_protos.h>
#include        <pragmas/exec_pragmas.h>
#include        <clib/iffparse_protos.h>
#include        <pragmas/iffparse_pragmas.h>
#include        <clib/dos_protos.h>
#include        <pragmas/dos_pragmas.h>
#include        <clib/alib_protos.h>

#include        "printerserverconfig.h"

#define ID_LV   1
#define ID_Add  2
#define ID_Delete 3
#define ID_Save 4
#define ID_Use  5
#define ID_Cancel 6

STRPTR vv=VERSTAG;

struct Library *IntuitionBase;
struct Library *GadToolsBase;
struct Library *IFFParseBase;
extern struct Library *DOSBase;
extern struct Library *SysBase;
struct Library *AccountsBase;
struct Library *EnvoyBase;
struct List UserList;
struct List DataList;

struct ConfigUser
{
    UWORD   cu_ID;
    UBYTE   cu_Flags;
    UBYTE   cu_Filler;
};

struct UserData
{
    struct Node ud_Node;
    UWORD       ud_ID;
    UBYTE       ud_Flags;
    UBYTE       ud_Filler;
    struct Node *ud_Ptr;    /* Pointer to Node structure of ascii linked list of users */
};

#define UDF_GROUP   1

#define ID_EPRT     MAKE_ID('E','P','R','T')
#define ID_USER     MAKE_ID('U','S','E','R')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EPRT,ID_USER};
void BuildASCII(void);
void ConfigIt(void);

void FreeList(void)
{
    struct UserData *n;
    while (n = (struct UserData *) RemHead(&DataList))
    {
        STRPTR s;
        struct Node *m;
        m = n->ud_Ptr;
        s = (STRPTR) m->ln_Name;
        if (s)
            FreeMem(s,33);
        Remove(m);
        FreeMem(m,sizeof(struct Node));
        FreeMem(n,sizeof(struct UserData));
    }
}

void BuildASCII()
{
    struct UserData *ud;
    ud = (struct UserData *) DataList.lh_Head;
    while (ud->ud_Node.ln_Succ)
    {
        if (!(ud->ud_Flags & UDF_GROUP))
        {
            struct UserInfo *ui;
            ui = (struct UserInfo *) AllocUserInfo();
            if (ui)
            {
                if (!IDToUser(ud->ud_ID,ui))
                {
                    STRPTR x;
                    x = (STRPTR) AllocMem(33,MEMF_PUBLIC);
                    if (x)
                    {
                        struct Node *y;
                        strcpy(x,ui->ui_UserName);
                        y = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                        if (y)
                        {
                            y->ln_Name = x;
                            ud->ud_Ptr = y;
                            AddTail(&UserList,y);
                        }
                        else
                            FreeMem(x,33);
                    }
                }
                else
                {
                    STRPTR x;
                    x = (STRPTR) AllocMem(33,MEMF_PUBLIC);
                    if (x)
                    {
                        struct Node *y;
                        strcpy(x,"Unknown User");
                        y = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                        if (y)
                        {
                            y->ln_Name = x;
                            ud->ud_Ptr = y;
                            AddTail(&UserList,y);
                        }
                        else
                            FreeMem(x,33);
                    }
                }
                FreeUserInfo(ui);
            }
        }
        else
        {
            struct GroupInfo *gi;
            gi = (struct GroupInfo *) AllocGroupInfo();
            if (gi)
            {
                if (!IDToGroup(ud->ud_ID,gi))
                {
                    STRPTR x;
                    x = (STRPTR) AllocMem(33,MEMF_PUBLIC);
                    if (x)
                    {
                        struct Node *y;
                        int z,q;
                        strcpy(x,gi->gi_GroupName);
                        z = strlen(x);
                        if (z > 26)
                            z = 26;
                        for (q = z; q < 27; q++)
                            x[q]=' ';
                        strcpy(&x[27],"<Group>");
                        y = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                        if (y)
                        {
                            y->ln_Name = x;
                            ud->ud_Ptr = y;
                            AddTail(&UserList,y);
                        }
                        else
                            FreeMem(x,33);
                    }
                }
                else
                {
                    STRPTR x;
                    x = (STRPTR) AllocMem(33,MEMF_PUBLIC);
                    if (x)
                    {
                        struct Node *y;
                        strcpy(x,"Unknown Group");
                        y = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                        if (y)
                        {
                            y->ln_Name = x;
                            ud->ud_Ptr = y;
                            AddTail(&UserList,y);
                        }
                        else
                            FreeMem(x,33);
                    }
                }
                FreeGroupInfo(gi);
            }

        }
        ud = (struct UserData *) ud->ud_Node.ln_Succ;
    }
}

BOOL LoadConfig(void)
{
    struct ConfigUser *cu;

    cu = (struct ConfigUser *) AllocMem(sizeof(struct ConfigUser),MEMF_PUBLIC);
    if (cu)
    {
        IFFParseBase = (struct Library *) OpenLibrary("iffparse.library",37L);
        if (IFFParseBase)
        {
            struct IFFHandle *iff;
            iff = (struct IFFHandle *) AllocIFF();
            if (iff)
            {
                iff->iff_Stream = (ULONG) Open("env:envoy/printerexport.config",MODE_OLDFILE);
                if (!iff->iff_Stream)
                    iff->iff_Stream = (ULONG) Open("envarc:envoy/printerexport.config",MODE_OLDFILE);
                if (iff->iff_Stream)
                {
                    InitIFFasDOS(iff);
                    if (!OpenIFF(iff,IFFF_READ))
                    {
                        if (!StopChunk(iff,ID_EPRT,ID_USER))
                        {
                            if (!ParseIFF(iff,IFFPARSE_SCAN))
                            {
                                while (TRUE)
                                {
                                    struct ContextNode *cn;
                                    cn = (struct ContextNode *) CurrentChunk(iff);
                                    if (cn)
                                    {
                                        if (ReadChunkBytes(iff,cu,sizeof(struct ConfigUser)) == sizeof(struct ConfigUser))
                                        {
                                            struct UserData *ud;
                                            ud = (struct UserData *) AllocMem(sizeof(struct UserData),MEMF_PUBLIC);
                                            if (ud)
                                            {
                                                ud->ud_ID = cu->cu_ID;
                                                ud->ud_Flags = cu->cu_Flags;
                                                AddTail(&DataList,&(ud->ud_Node));
                                            }
                                        }
                                        if (ParseIFF(iff,IFFPARSE_SCAN))
                                            break;
                                    }
                                }
                            }
                        }
                        CloseIFF(iff);
                    }
                    Close(iff->iff_Stream);
                }
                FreeIFF(iff);
            }
            CloseLibrary(IFFParseBase);
        }
        FreeMem(cu,sizeof(struct ConfigUser));
    }
    return(TRUE);
}


BOOL SaveConfig(BOOL which)
{

    struct ConfigUser *cu;
    cu = (struct ConfigUser *) AllocMem(sizeof(struct ConfigUser),MEMF_PUBLIC);
    if (cu)
    {
        IFFParseBase = (struct Library *) OpenLibrary("iffparse.library",37L);
        if (IFFParseBase)
        {
            struct IFFHandle *iff;
            iff = (struct IFFHandle *) AllocIFF();
            if (iff)
            {
                if (which)
                    iff->iff_Stream = (ULONG) Open("env:envoy/printerexport.config",MODE_NEWFILE);
                else
                    iff->iff_Stream = (ULONG) Open("envarc:envoy/printerexport.config",MODE_NEWFILE);
                if (iff->iff_Stream)
                {
                    InitIFFasDOS(iff);
                    if (!OpenIFF(iff,IFFF_WRITE))
                    {
                        if (!PushChunk(iff,ID_EPRT,ID_FORM,IFFSIZE_UNKNOWN))
                        {
                            if (!IsListEmpty(&DataList))
                            {
                                struct UserData *n;
                                n = (struct UserData *) DataList.lh_Head;
                                while (n->ud_Node.ln_Succ)
                                {
                                    cu->cu_ID = n->ud_ID;
                                    cu->cu_Flags = n->ud_Flags;
                                    if (!PushChunk(iff,ID_EPRT,ID_USER,IFFSIZE_UNKNOWN))
                                    {
                                        WriteChunkBytes(iff,cu,sizeof(struct ConfigUser));
                                        PopChunk(iff);
                                    }
                                    n = (struct UserData *) n->ud_Node.ln_Succ;
                                }
                            }
                            PopChunk(iff);
                        }
                        CloseIFF(iff);
                    }
                    Close(iff->iff_Stream);
                }
                FreeIFF(iff);
            }
            CloseLibrary(IFFParseBase);
        }
        FreeMem(cu,sizeof(struct ConfigUser));
    }
    return(TRUE);
}



void ConfigIt()
{

    struct Screen *s;
    APTR VisInfo;
    struct Gadget *FirstGadget;
    struct Gadget *GadgetList;
    struct TTextAttr topaz8={"topaz.font",8,FS_NORMAL, 0x1, NULL};
    int bh;

    NewList(&UserList);
    NewList(&DataList);
    s = LockPubScreen(NULL);
    bh = s->BarHeight;
    VisInfo = GetVisualInfo(s,TAG_DONE);
    if (VisInfo)
    {
        struct NewGadget NLV={12,20,300,108,"Users and Groups",0,ID_LV,PLACETEXT_ABOVE,0,0};
        struct Gadget *LV;
        FirstGadget = CreateContext(&GadgetList);
        NLV.ng_TextAttr = (struct TextAttr *) &topaz8;
        NLV.ng_VisualInfo = VisInfo;
        NLV.ng_TopEdge = 20+bh;
        LV = CreateGadget(LISTVIEW_KIND,FirstGadget,&NLV,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,GTLV_ShowSelected,FALSE,TAG_DONE);
        if (LV)
        {
            struct NewGadget NADD={12,128,150,16,"Add",0,ID_Add,PLACETEXT_IN,0,0};
            struct Gadget *ADD;
            NADD.ng_TextAttr = (struct TextAttr *) &topaz8;
            NADD.ng_VisualInfo = VisInfo;
            NADD.ng_TopEdge = 128+bh;
            ADD = CreateGadget(BUTTON_KIND,LV,&NADD,TAG_DONE);
            if (ADD)
            {
                struct NewGadget NDEL={162,128,150,16,"Delete",0,ID_Delete,PLACETEXT_IN,0,0};
                struct Gadget *DEL;
                NDEL.ng_TextAttr = (struct TextAttr *) &topaz8;
                NDEL.ng_VisualInfo = VisInfo;
                NDEL.ng_TopEdge = 128+bh;
                DEL = CreateGadget(BUTTON_KIND,ADD,&NDEL,TAG_DONE);
                if (DEL)
                {
                    struct NewGadget NSAVE={12,152,84,16,"Save",0,ID_Save,PLACETEXT_IN,0,0};
                    struct Gadget *SAVE;
                    NSAVE.ng_TextAttr = (struct TextAttr *) &topaz8;
                    NSAVE.ng_VisualInfo = VisInfo;
                    NSAVE.ng_TopEdge = 152+bh;
                    SAVE = CreateGadget(BUTTON_KIND,DEL,&NSAVE,TAG_DONE);
                    if (SAVE)
                    {

/*                        struct NewGadget NUSE ={120,152,84,16,"Use",0,ID_Use,PLACETEXT_IN,0,0};
                        struct Gadget *USE;
                        NUSE.ng_TextAttr = (struct TextAttr *) &topaz8;
                        NUSE.ng_VisualInfo = VisInfo;
                        NUSE.ng_TopEdge = 152+bh;
                        USE = CreateGadget(BUTTON_KIND,SAVE,&NUSE,TAG_DONE);
                        if (USE)
 */
                        if (TRUE)
                        {

                            struct NewGadget NCAN ={228,152,84,16,"Cancel",0,ID_Cancel,PLACETEXT_IN,0,0};
                            struct Gadget *CAN;
                            NCAN.ng_TextAttr = (struct TextAttr *) &topaz8;
                            NCAN.ng_VisualInfo = VisInfo;
                            NCAN.ng_TopEdge = 152+bh;
//                            CAN = CreateGadget(BUTTON_KIND,USE ,&NCAN,TAG_DONE);
                            CAN = CreateGadget(BUTTON_KIND,SAVE ,&NCAN,TAG_DONE);
                            if (CAN)
                            {
                                struct Window *CW;
                                CW = OpenWindowTags(NULL,WA_DragBar,TRUE,
                                                         WA_DepthGadget,TRUE,
                                                         WA_Left,0,
                                                         WA_Top,22,
                                                         WA_InnerWidth,319,
                                                         WA_InnerHeight,175,
//                                                         WA_GimmeZeroZero,TRUE,
                                                         WA_Activate,TRUE,
                                                         WA_Title,"Printer Export Configuration",
                                                         WA_SmartRefresh,TRUE,
                                                         WA_Gadgets,FirstGadget,
                                                         WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|STRINGIDCMP,
                                                         TAG_DONE,TRUE);

                                if (CW)
                                {
                                    struct MsgPort *WPort;
                                    int code=-1;
                                    BOOL cont = TRUE;
                                    WPort = CW->UserPort;
                                    if (s)
                                    {
                                        UnlockPubScreen(NULL,s);
                                        s=0;
                                    }
                                    /* SetGadgetAttrs here */
                                    LoadConfig();
                                    BuildASCII();
                                    GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,&UserList,TAG_DONE);


                                    GT_RefreshWindow(CW,0L);

                                    while (cont)
                                    {
                                        ULONG sigmask;
                                        struct IntuiMessage *im;
                                        sigmask = (1 << WPort->mp_SigBit);
                                        Wait(sigmask);
                                        while (im = GT_GetIMsg(WPort))
                                        {
                                            switch ((ULONG) im->Class)
                                            {
                                                case IDCMP_GADGETUP:
                                                case IDCMP_GADGETDOWN:
                                                {
                                                    switch ((((struct Gadget *)im->IAddress))->GadgetID)
                                                    {
                                                        case ID_LV:
                                                        {
                                                            code = im->Code;
                                                            break;
                                                        }
                                                        case ID_Add:
                                                        {
                                                            struct Node *j;
                                                            UBYTE userbuff[33];
                                                            UBYTE groupbuff[33];
                                                            ULONG tags[12]={UGREQ_UserBuff,0,
                                                                            UGREQ_UserBuffLen,32,
                                                                            UGREQ_GroupBuff,0,
                                                                            UGREQ_GroupBuffLen,32,
                                                                            UGREQ_Title,(ULONG)"Select a User or Group",
                                                                            TAG_DONE,0};
                                                            tags[1] = (ULONG) &userbuff[0];
                                                            tags[5] = (ULONG) &groupbuff[0];
                                                            if (UserRequestA((struct TagItem *)&tags))
                                                            {
                                                                struct UserData *d;
                                                                d = (struct UserData *) AllocMem(sizeof(struct UserData),MEMF_PUBLIC|MEMF_CLEAR);
                                                                if (d)
                                                                {
                                                                    struct Node *nn;
                                                                    STRPTR thename;
                                                                    if (strlen(userbuff))
                                                                    {
                                                                        struct UserInfo *ui;
                                                                        thename = &userbuff[0];
                                                                        ui = (struct UserInfo *) AllocUserInfo();
                                                                        if (ui)
                                                                        {
                                                                            if (!NameToUser(userbuff,ui))
                                                                                d->ud_ID = ui->ui_UserID;
                                                                            FreeUserInfo(ui);
                                                                        }
                                                                    }
                                                                    else
                                                                    {
                                                                        struct GroupInfo *gi;
                                                                        if (!strlen(groupbuff))
                                                                        {
                                                                            FreeMem(d,sizeof(struct UserData));
                                                                            break;
                                                                        }
                                                                        thename = &groupbuff[0];
                                                                        gi = (struct GroupInfo *) AllocGroupInfo();
                                                                        if (gi)
                                                                        {
                                                                            if (!NameToGroup(groupbuff,gi))
                                                                            {
                                                                                d->ud_ID = gi->gi_GroupID;
                                                                                d->ud_Flags |= UDF_GROUP;
                                                                            }
                                                                            FreeGroupInfo(gi);
                                                                        }
                                                                    }
                                                                    AddTail(&DataList,(struct Node *)d);
                                                                    j = DataList.lh_Head;
                                                                    code = -1;
                                                                    while (j->ln_Succ)
                                                                    {
                                                                        code++;
                                                                        j = j->ln_Succ;
                                                                    }
                                                                    nn = (struct Node *) AllocMem(sizeof(struct Node),MEMF_PUBLIC);
                                                                    if (nn)
                                                                    {
                                                                        STRPTR f;
                                                                        f = (STRPTR) AllocMem(33,MEMF_PUBLIC);
                                                                        if (f)
                                                                        {
                                                                            strcpy(f,thename);
                                                                            if ( thename == groupbuff)
                                                                            {
                                                                                int z,q;
                                                                                z = strlen(f);
                                                                                if (z > 26)
                                                                                    z = 26;
                                                                                for (q = z; q < 27; q++)
                                                                                    f[q]=' ';
                                                                                strcpy(&f[27],"<Group>");

                                                                            }
                                                                            nn->ln_Name = f;
                                                                            d->ud_Ptr = nn;
                                                                            GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,0L,TAG_DONE);
                                                                            AddTail(&UserList,nn);
                                                                            GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,&UserList,GTLV_MakeVisible,code,GTLV_Selected,code,TAG_DONE);
                                                                            GT_RefreshWindow(CW,0L);
                                                                        }
                                                                        else
                                                                            FreeMem(nn,sizeof(struct Node));
                                                                    }

                                                                }
                                                            }
                                                            break;
                                                        }
                                                        case ID_Delete:
                                                        {
                                                            if (code != -1)
                                                            {
                                                                struct UserData *ud;
                                                                struct Node *an;
                                                                ud = (struct UserData *) DataList.lh_Head;
                                                                while (code)
                                                                {
                                                                    ud = (struct UserData *) ud->ud_Node.ln_Succ;
                                                                    code--;
                                                                }
                                                                an = ud->ud_Ptr;
                                                                GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,0L,TAG_DONE);
                                                                Remove(an);
                                                                Remove(&(ud->ud_Node));
                                                                GT_SetGadgetAttrs(LV,CW,NULL,GTLV_Labels,&UserList,TAG_DONE);
                                                                GT_RefreshWindow(CW,0L);
                                                                FreeMem(an->ln_Name,33);
                                                                FreeMem(an,sizeof(struct Node));
                                                                FreeMem(ud,sizeof(struct UserData));
                                                            }
                                                            code = -1;
                                                            break;
                                                        }
                                                        case ID_Cancel:
                                                            cont = FALSE;
                                                            break;
                                                        case ID_Save:
                                                            SaveConfig(FALSE);
                                                        case ID_Use:
                                                            SaveConfig(TRUE);
                                                            cont = FALSE;
                                                            break;
                                                    }
                                                    break;
                                                }
                                            }
                                            GT_ReplyIMsg(im);
                                        }
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
        FreeList();
    }
    if (s)
        UnlockPubScreen(NULL,s);
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
                        ConfigIt();
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



