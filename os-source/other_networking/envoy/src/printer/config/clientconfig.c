
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
//#include        <clib/accounts_protos.h>
//#include        <pragmas/accounts_pragmas.h>
#include        <clib/exec_protos.h>
#include        <pragmas/exec_pragmas.h>
#include        <clib/iffparse_protos.h>
#include        <pragmas/iffparse_pragmas.h>
#include        <clib/dos_protos.h>
#include        <pragmas/dos_pragmas.h>
#include        <clib/alib_protos.h>

#include        "printerclientconfig.h"

#define ID_Host 1
#define ID_User 2
#define ID_HostText 3
#define ID_NameText 4
#define ID_PWText 5
#define ID_Save   6
#define ID_Use    7
#define ID_Cancel 8

STRPTR vv=VERSTAG;

struct Library *IntuitionBase;
struct Library *GadToolsBase;
struct Library *IFFParseBase;
extern struct Library *DOSBase;
extern struct Library *SysBase;
struct Library *AccountsBase;
struct Library *EnvoyBase;

struct ConfigUser
{
    UBYTE   cu_HostName[65];
    UBYTE   cu_UserName[33];
    UBYTE   cu_Password[33];
    UBYTE   cu_Filler;
};

#define ID_EPRC     MAKE_ID('E','P','R','C')
#define ID_UDTA     MAKE_ID('U','D','T','A')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EPRC,ID_UDTA};

void ConfigIt(void);

BOOL LoadConfig(STRPTR host, STRPTR user, STRPTR pw)
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
                iff->iff_Stream = (ULONG) Open("env:envoy/importprinter.config",MODE_OLDFILE);
                if (!iff->iff_Stream)
                    iff->iff_Stream = (ULONG) Open("envarc:envoy/importprinter.config",MODE_OLDFILE);
                if (iff->iff_Stream)
                {
                    InitIFFasDOS(iff);
                    if (!OpenIFF(iff,IFFF_READ))
                    {
                        if (!StopChunk(iff,ID_EPRC,ID_UDTA))
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
                                            strcpy(host,cu->cu_HostName);
                                            strcpy(user,cu->cu_UserName);
                                            strcpy(pw,cu->cu_Password);
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



BOOL SaveConfig(BOOL which, STRPTR host, STRPTR user, STRPTR pw)
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
                    iff->iff_Stream = (ULONG) Open("env:envoy/importprinter.config",MODE_NEWFILE);
                else
                    iff->iff_Stream = (ULONG) Open("envarc:envoy/importprinter.config",MODE_NEWFILE);
                if (iff->iff_Stream)
                {
                    InitIFFasDOS(iff);
                    if (!OpenIFF(iff,IFFF_WRITE))
                    {
                        if (!PushChunk(iff,ID_EPRC,ID_FORM,IFFSIZE_UNKNOWN))
                        {
                            if (!PushChunk(iff,ID_EPRC,ID_UDTA,IFFSIZE_UNKNOWN))
                            {
                                strcpy(cu->cu_HostName,host);
                                strcpy(cu->cu_UserName,user);
                                strcpy(cu->cu_Password,pw);
                                WriteChunkBytes(iff,cu,sizeof(struct ConfigUser));
                                PopChunk(iff);
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

    UBYTE username[32];
    UBYTE passcode[32];
    UBYTE password[32];
    UBYTE hostname[64];

    struct Screen *s;
    APTR VisInfo;
    struct Gadget *FirstGadget;
    struct Gadget *GadgetList;
    struct TTextAttr topaz8={"topaz.font",8,FS_NORMAL, 0x1, NULL};
    int bh;

    username[0]=passcode[0]=password[0]=hostname[0]=0;

    s = LockPubScreen(NULL);
    bh = s->BarHeight;
    VisInfo = GetVisualInfo(s,TAG_DONE);
    if (VisInfo)
    {
        struct NewGadget NHost={253,16,160,14,"Select Host...",0,ID_Host,PLACETEXT_IN,0,0};
        struct Gadget *Host;
        FirstGadget = CreateContext(&GadgetList);
        NHost.ng_TopEdge = 16+bh;
        NHost.ng_TextAttr = (struct TextAttr *) &topaz8;
        NHost.ng_VisualInfo = VisInfo;
        Host = CreateGadget(BUTTON_KIND,FirstGadget,&NHost,TAG_DONE);
        if (Host)
        {
            struct NewGadget NHostText={62,16,192,14,"Host:",0,ID_HostText,PLACETEXT_LEFT,0,0};
            struct Gadget *HostText;
            NHostText.ng_TextAttr = (struct TextAttr *) &topaz8;
            NHostText.ng_VisualInfo = VisInfo;
            NHostText.ng_TopEdge = 16+bh;
            HostText = CreateGadget(TEXT_KIND,Host,&NHostText,GTTX_Border,TRUE,TAG_DONE);
            if (HostText)
            {
                struct NewGadget NSave ={13,53,90,14,"Save",0,ID_Save,PLACETEXT_IN,0,0};
                struct Gadget *Save;
                NSave.ng_TextAttr = (struct TextAttr *) &topaz8;
                NSave.ng_VisualInfo = VisInfo;
                NSave.ng_TopEdge = 53+bh;
                Save = CreateGadget(BUTTON_KIND,HostText,&NSave,TAG_DONE);
                if (Save)
                {
                    struct NewGadget NUse  ={168,53,90,14,"Use",0,ID_Use,PLACETEXT_IN,0,0};
                    struct Gadget *Use;
                    NUse.ng_TextAttr = (struct TextAttr *) &topaz8;
                    NUse.ng_VisualInfo = VisInfo;
                    NUse.ng_TopEdge = 53+bh;
                    Use = CreateGadget(BUTTON_KIND,Save,&NUse,TAG_DONE);
                    if (Use)
                    {

                        struct NewGadget NCancel ={323,53,90,14,"Cancel",0,ID_Cancel,PLACETEXT_IN,0,0};
                        struct Gadget *Cancel;
                        NCancel.ng_TextAttr = (struct TextAttr *) &topaz8;
                        NCancel.ng_VisualInfo = VisInfo;
                        NCancel.ng_TopEdge = 53+bh;
                        Cancel = CreateGadget(BUTTON_KIND,Use,&NCancel,TAG_DONE);
                        if (Cancel)
                        {

                            struct Window *CW;
                            CW = OpenWindowTags(NULL,WA_DragBar,TRUE,
                                                     WA_DepthGadget,TRUE,
                                                     WA_Left,0,
                                                     WA_Top,22,
                                                     WA_InnerWidth,425,
                                                     WA_InnerHeight,72,
                                                     WA_Activate,TRUE,
                                                     WA_Title,"Network Printer Configuration",
                                                     WA_SmartRefresh,TRUE,
                                                     WA_Gadgets,FirstGadget,
                                                     WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|STRINGIDCMP,
                                                     TAG_DONE,TRUE);

                            if (CW)
                            {
                                struct MsgPort *WPort;
                                int f, g;
                                BOOL cont = TRUE;
                                WPort = CW->UserPort;
                                if (s)
                                {
                                    UnlockPubScreen(NULL,s);
                                    s=0;
                                }

                                LoadConfig(hostname,username,password);

                                f = strlen(password);
                                for (g = 0; g < f; g++)
                                {
                                    passcode[g]=0xB7; /* $B7 = "bullet" character */
                                    passcode[g+1]=0;
                                }


                                GT_SetGadgetAttrs(HostText,CW,0,GTTX_Text,hostname,TAG_DONE);

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
                                                    case ID_Cancel:
                                                        cont = FALSE;
                                                        break;
                                                    case ID_Host:
                                                    {
                                                        UBYTE host[65];
                                                        ULONG tags[9]={HREQ_Buffer,0,HREQ_BuffSize,64,
                                                                       HREQ_Title,(ULONG) "Select Host",
                                                                       MATCH_SERVICE,(ULONG)"Print Spooler",
                                                                       TAG_DONE};
                                                        tags[1] = (ULONG) host;
                                                        if (HostRequestA((struct TagItem *) tags))
                                                        {
                                                            ULONG tags[16]={LREQ_NameBuff,0,LREQ_NameBuffLen,32,
                                                                            LREQ_PassBuff,0,LREQ_PassBuffLen,32,
                                                                            LREQ_Title,(ULONG)"Enter Username & Password",
                                                                            LREQ_UserName,0,
                                                                            LREQ_Password,0,
                                                                            TAG_DONE,0};
                                                            UBYTE tu[33],tp[33];
                                                            hostname[0]=0;
                                                            GT_SetGadgetAttrs(HostText,CW,0,GTTX_Text,0L,TAG_DONE);
                                                            GT_RefreshWindow(CW,0L);
                                                            strcpy(hostname,host);
                                                            GT_SetGadgetAttrs(HostText,CW,0,GTTX_Text,hostname,TAG_DONE);
                                                            tags[1] = (ULONG) username;
                                                            tags[5] = (ULONG) password;
                                                            tags[11] = (ULONG) tu;
                                                            tags[13] = (ULONG) tp;
                                                            strcpy(tu,username);
                                                            strcpy(tp,password);
                                                            username[0]=password[0]=0;
                                                            GT_RefreshWindow(CW,0L);
                                                            LoginRequestA((struct TagItem *) tags);
                                                        }
                                                        break;
                                                    }
                                                    case ID_Save:
                                                        SaveConfig(FALSE,hostname,username,password);
                                                    case ID_Use:
                                                        SaveConfig(TRUE,hostname,username,password);
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
        FreeGadgets(FirstGadget);
    }
    if (s)
        UnlockPubScreen(NULL,s);
}



void CouldNotOpen(STRPTR libname)
{
    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Printer Import",
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
                    ConfigIt();
                    CloseLibrary(AccountsBase);
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




