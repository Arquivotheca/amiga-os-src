

#include        <exec/types.h>
#include        <exec/libraries.h>
#include        <exec/memory.h>
#include        <utility/tagitem.h>
#include        <intuition/intuition.h>
#include        <intuition/gadgetclass.h>
#include        <libraries/gadtools.h>
#include        <libraries/asl.h>
#include        <libraries/iffparse.h>
#include        <workbench/startup.h>
#include	<dos/dos.h>
#include        <envoy/envoy.h>

#include        <clib/exec_protos.h>
#include        <pragmas/exec_pragmas.h>
#include        <clib/envoy_protos.h>
#include        <pragmas/envoy_pragmas.h>
#include        <clib/accounts_protos.h>
#include        <pragmas/accounts_pragmas.h>
#include        <clib/intuition_protos.h>
#include        <pragmas/intuition_pragmas.h>
#include        <clib/gadtools_protos.h>
#include        <pragmas/gadtools_pragmas.h>
#include        <clib/dos_protos.h>
#include        <pragmas/dos_pragmas.h>
#include        <clib/iffparse_protos.h>
#include        <pragmas/iffparse_pragmas.h>
#include        <clib/asl_protos.h>
#include        <pragmas/asl_pragmas.h>
#include        <clib/alib_protos.h>

#include        "fsconfig_rev.h"
#include	<string.h>

struct ExportVolume
{
    struct Node     ev_Link;            /* Link node */
    struct List     ev_Users;           /* List of users that can access this volume */
    UBYTE           ev_VolumeName[64];  /* Name of this volume for remote users */
    ULONG           ev_Flags;           /* Flag bits for this export */
};

#define EVF_SnapshotOK          1
#define EVF_LeftoutOK           2
#define EVF_Full                4
#define EVF_NoSecurity          8

struct UserOrGroup
{
    struct Node     ug_Link;            /* Link node */
    UWORD           ug_ID;              /* ID */
    UBYTE           ug_Type;            /* see below */
    UBYTE           ug_Filler;
};

#define UGTYPE_USER             0
#define UGTYPE_GROUP            1

BOOL GUserRequest(UBYTE *ptr);
BOOL LoadConfig(struct List *);
BOOL SaveConfig(struct List *, BOOL x);

/* Globals */
extern struct Execbase *SysBase;
extern struct Library *DOSBase;
struct Library *IntuitionBase;
struct Library *GadToolsBase;
struct Library *AslBase;
struct Library *EnvoyBase;
struct Library *AccountsBase;

/* rev string */
    STRPTR revisionstring=VERSTAG;

/* Gadtools ID's */
#define ID_DirectoryPath    1
#define ID_UsersAndGroups   2
#define ID_Snapshot         3
#define ID_Leftout          4
#define ID_Full             5
#define ID_Save             6
#define ID_Cancel           7
#define ID_Add1             8
#define ID_Delete1          9
#define ID_Add2             10
#define ID_Delete2          11
#define ID_Name             12
#define ID_NoSecurity       13

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

void CouldNotOpen(STRPTR libname)
{
    struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Filesystem Exports",
                           "Could not open library\n%s","OK"};
    EasyRequest(0L,&ers,0L,libname);
}




main()
{

    struct List ExportList;
    struct ExportVolume *aev;
    struct ExportVolume *CurrentlySelected=0L;
    struct UserOrGroup *CurrentUser=0;


    NewList(&ExportList);
    IntuitionBase = (struct Library *) OpenLibrary("intuition.library",37L);
    if (IntuitionBase)
    {
        DOSBase = (struct Library *) OpenLibrary("dos.library",0L);
        if (DOSBase)
        {
            AccountsBase = (struct Library *) OpenLibrary("accounts.library",37L);
            if (AccountsBase)
            {
                EnvoyBase = (struct Library *) OpenLibrary("envoy.library",37L);
                if (EnvoyBase)
                {
                    LoadConfig(&ExportList);
                    AslBase = (struct Library *) OpenLibrary("asl.library",37L);
                    if (AslBase)
                    {
                        GadToolsBase = (struct Library *) OpenLibrary("gadtools.library",37L);
                        if (GadToolsBase)
                        {
                            int bh;
                            struct Screen *onscreen;
                            APTR VisInfo;

                            onscreen = LockPubScreen(NULL);
                            bh = onscreen->BarHeight;
                            VisInfo = GetVisualInfo(onscreen,TAG_DONE);

                            if (VisInfo)
                            {
                                struct TTextAttr topaz8 = {"topaz.font", 8, FS_NORMAL, 0x1, NULL};

                                struct NewGadget lvng={16,0,264,132,"Shared Directories",0,ID_DirectoryPath,
                                                       PLACETEXT_ABOVE,0,0};

                                struct Gadget *lv;
                                struct Gadget *GadgetList=NULL;
                                struct Gadget *FirstGadget;

                                lvng.ng_TextAttr = (struct TextAttr *) &topaz8;
                                lvng.ng_VisualInfo = VisInfo;
                                lvng.ng_TopEdge = bh+20;

                                FirstGadget = CreateContext(&GadgetList);

                                lv = CreateGadget(LISTVIEW_KIND,FirstGadget,&lvng,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,GTLV_ShowSelected,FALSE,TAG_DONE);
                                if (lv)
                                {
                                    struct NewGadget lvng2={308,0,238,60,"Users and Groups",0,ID_UsersAndGroups,
                                                            PLACETEXT_ABOVE,0,0};
                                    struct Gadget *lv2;
                                    lvng2.ng_TextAttr = (struct TextAttr *) &topaz8;
                                    lvng2.ng_TopEdge = bh+92;
                                    lvng2.ng_VisualInfo = VisInfo;

                                    lv2 = CreateGadget(LISTVIEW_KIND,lv,&lvng2,LAYOUTA_SPACING,1,GTLV_ScrollWidth,18,GTLV_ShowSelected,FALSE,TAG_DONE);
                                    if (lv2)
                                    {
                                        struct NewGadget NewSnapshot={520,0,26,11,"Allow volume snapshotting :",
                                                                   0,ID_Snapshot,PLACETEXT_LEFT,0,0};
                                        struct Gadget *Snapshot;
                                        NewSnapshot.ng_TextAttr = (struct TextAttr *) &topaz8;
                                        NewSnapshot.ng_VisualInfo = VisInfo;
                                        NewSnapshot.ng_TopEdge = bh+8;
                                        Snapshot = CreateGadget(CHECKBOX_KIND,lv2,&NewSnapshot,GTCB_Checked,TRUE,
                                                                TAG_DONE);
                                        if (Snapshot)
                                        {
                                            struct NewGadget NewLeftout={520,0,26,11,"Allow left-out icons :",
                                                                         0,ID_Leftout,PLACETEXT_LEFT,0,0};
                                            struct Gadget *Leftout;
                                            NewLeftout.ng_TextAttr = (struct TextAttr *) &topaz8;
                                            NewLeftout.ng_VisualInfo = VisInfo;
                                            NewLeftout.ng_TopEdge = bh+20;
                                            Leftout = CreateGadget(CHECKBOX_KIND,Snapshot,&NewLeftout,GTCB_Checked,TRUE,
                                                                   TAG_DONE);
                                            if (Leftout)
                                            {
                                                struct NewGadget NewFull={520,0,26,11,"Full File Security :",
                                                                          0,ID_Full,PLACETEXT_LEFT,0,0};
                                                struct Gadget *Full;
                                                NewFull.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                NewFull.ng_VisualInfo = VisInfo;
                                                NewFull.ng_TopEdge = bh+32;
                                                Full = CreateGadget(CHECKBOX_KIND,Leftout,&NewFull,GTCB_Checked,TRUE,
                                                                    TAG_DONE);
                                                if (Full)
                                                {
                                                    struct NewGadget NewOff={520,0,26,11,"No Security :",0,ID_NoSecurity,PLACETEXT_LEFT,0,0};
                                                    struct Gadget *Off;
                                                    NewOff.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                    NewOff.ng_VisualInfo = VisInfo;
                                                    NewOff.ng_TopEdge = bh+44;
                                                    Off = CreateGadget(CHECKBOX_KIND,Full,&NewOff,TAG_DONE);
                                                    if (Off)
                                                    {
                                                        struct NewGadget NewSave={16,0,88,14,"Save",0,ID_Save,PLACETEXT_IN,0,0};
                                                        struct Gadget *Save;
                                                        NewSave.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                        NewSave.ng_VisualInfo = VisInfo;
                                                        NewSave.ng_TopEdge = bh+174;
                                                        Save = CreateGadget(BUTTON_KIND,Off,&NewSave,TAG_DONE);
                                                        if (Save)
                                                        {
                                                            struct NewGadget NewCancel={458,0,88,14,"Cancel",0,ID_Cancel,PLACETEXT_IN,0,0};
                                                            struct Gadget *Cancel;
                                                            NewCancel.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                            NewCancel.ng_VisualInfo = VisInfo;
                                                            NewCancel.ng_TopEdge = bh+174;
                                                            Cancel = CreateGadget(BUTTON_KIND,Save,&NewCancel,TAG_DONE);
                                                            if (Cancel)
                                                            {
                                                                struct NewGadget NewAdd1={16,0,132,14,"Add",0,ID_Add1,PLACETEXT_IN,0,0};
                                                                struct Gadget *Add1;
                                                                NewAdd1.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                                NewAdd1.ng_VisualInfo = VisInfo;
                                                                NewAdd1.ng_TopEdge = bh+150;
                                                                Add1 = CreateGadget(BUTTON_KIND,Cancel,&NewAdd1,TAG_DONE);
                                                                if (Add1)
                                                                {
                                                                    struct NewGadget NewDel1={148,0,132,14,"Delete",0,ID_Delete1,PLACETEXT_IN,0,0};
                                                                    struct Gadget *Delete1;
                                                                    NewDel1.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                                    NewDel1.ng_VisualInfo = VisInfo;
                                                                    NewDel1.ng_TopEdge = bh+150;
                                                                    Delete1 = CreateGadget(BUTTON_KIND,Add1,&NewDel1,TAG_DONE);
                                                                    if (Delete1)
                                                                    {
                                                                        struct NewGadget NewAdd2={308,0,119,14,"Add",0,ID_Add2,PLACETEXT_IN,0,0};
                                                                        struct Gadget *Add2;
                                                                        NewAdd2.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                                        NewAdd2.ng_VisualInfo = VisInfo;
                                                                        NewAdd2.ng_TopEdge = bh+150;
                                                                        Add2 = CreateGadget(BUTTON_KIND,Delete1,&NewAdd2,TAG_DONE);
                                                                        if (Add2)
                                                                        {
                                                                            struct NewGadget NewDel2={427,0,119,14,"Delete",0,ID_Delete2,PLACETEXT_IN,0,0};
                                                                            struct Gadget *Delete2;
                                                                            NewDel2.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                                            NewDel2.ng_VisualInfo = VisInfo;
                                                                            NewDel2.ng_TopEdge = bh+150;
                                                                            Delete2 = CreateGadget(BUTTON_KIND,Add2,&NewDel2,TAG_DONE);
                                                                            if (Delete2)
                                                                            {
                                                                                struct NewGadget NewName={336,0,210,14,"Name",0,ID_Name,PLACETEXT_LEFT,0,0};
                                                                                struct Gadget *Name;
                                                                                NewName.ng_TextAttr = (struct TextAttr *) &topaz8;
                                                                                NewName.ng_VisualInfo = VisInfo;
                                                                                NewName.ng_TopEdge = bh+62;
                                                                                Name = CreateGadget(STRING_KIND,Delete2,&NewName,TAG_DONE);
                                                                                if (Name)
                                                                                {

	     ((struct StringInfo *)Name->SpecialInfo)->MaxChars = 63;

	     if (FirstGadget)
	     {

	         struct Window *ConfigWindow;

	         ConfigWindow = OpenWindowTags(NULL,WA_DragBar,TRUE,
	                                            WA_DepthGadget,TRUE,
	                                            WA_Left,0,
	                                            WA_Top,22,
//	                                            WA_Width,565,
//	                                            WA_Height,217,
	                                            WA_InnerWidth,557,
	                                            WA_InnerHeight,194,
    //	                                        WA_GimmeZeroZero,TRUE,
	                                            WA_Activate,TRUE,
	                                            WA_Title,"Filesystem Exports",
	                                            WA_SmartRefresh,TRUE,
	                                            WA_Gadgets,FirstGadget,
	                                            WA_IDCMP,IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP|LISTVIEWIDCMP|STRINGIDCMP,
	                                            TAG_DONE,TRUE);

	         if (ConfigWindow)
	         {
	             BOOL cont=TRUE;
	             struct MsgPort *WPort;
	             WPort = ConfigWindow->UserPort;
	             if (onscreen)
	             {
	                 UnlockPubScreen(NULL,onscreen);
	                 onscreen = 0L;
	             }
	             GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Labels,&ExportList,TAG_DONE);
	             /* Turn off specifics when no mount is selected */
	             GT_SetGadgetAttrs(Delete1,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	             GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,0L,TAG_DONE);
	             GT_RefreshWindow(ConfigWindow,0L);
	             while (cont)
	             {
	                 ULONG sigmask;
	                 struct IntuiMessage *im;
	                 sigmask = (1 << WPort->mp_SigBit);
	                 (void) Wait(sigmask);
	                 while (im = GT_GetIMsg(WPort))
	                 {
	                     switch((ULONG) im->Class)
	                     {
	                         case IDCMP_GADGETUP:
	                         case IDCMP_GADGETDOWN:
	                         {
	                             switch ((((struct Gadget *)im->IAddress))->GadgetID)
	                             {
	                                 case ID_Name:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         struct Gadget *sg;
	                                         struct StringInfo *si;
	                                         sg = (struct Gadget *) im->IAddress;
	                                         si = (struct StringInfo *) sg->SpecialInfo;
	                                         strcpy(CurrentlySelected->ev_VolumeName,si->Buffer);
	                                     }
	                                     break;

	                                 }
	                                 case ID_DirectoryPath:
	                                 {
	                                     struct ExportVolume *ev;
	                                     struct StringInfo *si;
	                                     int code=im->Code;
	                                     /* Because Intuition isn't bright enough to tell you anything about a string gadget, except when the user hits return,
	                                      * read in the contents here -- when the user is trying to change modes
	                                      */
	                                     si = (struct StringInfo *) Name->SpecialInfo;
	                                     if (CurrentlySelected)
	                                         strcpy(CurrentlySelected->ev_VolumeName,si->Buffer);

	                                     /* Find the new entry */
	                                     ev = (struct ExportVolume *) ExportList.lh_Head;
	                                     while (code)
	                                     {
	                                         ev = (struct ExportVolume *) ev->ev_Link.ln_Succ;
	                                         code--;
	                                     }
	                                     GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,&ev->ev_Users,TAG_DONE);
	                                     GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GTST_String,&ev->ev_VolumeName,TAG_DONE);
	                                     GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_LeftoutOK),TAG_DONE);
	                                     GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_SnapshotOK),TAG_DONE);
	                                     GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_NoSecurity),TAG_DONE);
	                                     GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_Full),TAG_DONE);

	                                     /* Turn on specifics */
	                                     GT_SetGadgetAttrs(Delete1,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,(ev->ev_Flags & EVF_NoSecurity),TAG_DONE);
	                                     GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                     GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GA_Disabled,(ev->ev_Flags & EVF_NoSecurity),GTLV_Selected,~0,TAG_DONE);
	                                     ActivateGadget(Name,ConfigWindow,NULL);
	                                     GT_RefreshWindow(ConfigWindow,0L);

	                                     CurrentlySelected = ev;
	                                     CurrentUser = 0L;
	                                     break;
	                                 }
	                                 case ID_UsersAndGroups:
	                                 {
	                                     int code=im->Code;
	                                     CurrentUser = (struct UserOrGroup *) CurrentlySelected->ev_Users.lh_Head;
	                                     while (code)
	                                     {
	                                         CurrentUser = (struct UserOrGroup *) CurrentUser->ug_Link.ln_Succ;
	                                         code--;
	                                     }
	                                     GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     GT_RefreshWindow(ConfigWindow,0L);

	                                     break;
	                                 }
	                                 case ID_Add1:
	                                 {
	                                     struct FileRequester *ar;
	                                     struct StringInfo *si;
	                                     struct Requester *r;

	                                     /* Because Intuition isn't bright enough to tell you anything about a string gadget, except when the user hits return,
	                                      * read in the contents here -- when the user is trying to change modes
	                                      */
	                                     si = (struct StringInfo *) Name->SpecialInfo;
	                                     if (CurrentlySelected)
	                                         strcpy(CurrentlySelected->ev_VolumeName,si->Buffer);

	                                     ar = (struct FileRequester *) AllocAslRequestTags(ASL_FileRequest,ASLFR_InitialDrawer,"SYS:",
	                                         ASLFR_Window,ConfigWindow,ASLFR_DrawersOnly,TRUE,TAG_DONE);

	                                     r = SetBusyPointer(ConfigWindow);
	                                     if (AslRequestTags(ar,ASLFR_TitleText,"Directory to export",TAG_DONE))
	                                     {
	                                         struct ExportVolume *ev;
	                                         ev = (struct ExportVolume *) AllocMem(sizeof(struct ExportVolume),MEMF_PUBLIC);
	                                         if (ev)
	                                         {
	                                             STRPTR path;
	                                             NewList(&ev->ev_Users);
	                                             ev->ev_VolumeName[0]=0;
	                                             ev->ev_Flags = 0L;
	                                             path = (STRPTR) AllocMem(strlen(ar->fr_Drawer)+1,MEMF_PUBLIC);
	                                             if (path)
	                                             {
	                                                 int ord;
	                                                 struct ExportVolume *x;
	                                                 strcpy(path,ar->fr_Drawer);
	                                                 ev->ev_Link.ln_Name = path;
	                                                 /* Add it into the listview */
	                                                 GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Labels,~0,TAG_DONE);
	                                                 AddTail(&ExportList,(struct Node *)ev);
	                                                 GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Labels,&ExportList,TAG_DONE);
	                                                 /* Get the Listview to highlight it, turn on the correct gadgets,
	                                                  * and update the data
	                                                  */
	                                                 /* First, find the ordinal number of this entry */
	                                                 ord = 0;
	                                                 x = (struct ExportVolume *) ExportList.lh_Head;
	                                                 while ((x->ev_Link.ln_Succ) && (x != ev))
	                                                 {
	                                                     ord++;
	                                                     x = (struct ExportVolume *) x->ev_Link.ln_Succ;
	                                                 }
	                                                 GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Selected,ord,GTLV_MakeVisible,ord,TAG_DONE);
	                                                 /* Okay, it's visible & selected.  Update the gadget information: */
	                                                 GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,&ev->ev_Users,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GTST_String,&ev->ev_VolumeName,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_LeftoutOK),TAG_DONE);
	                                                 GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_NoSecurity),TAG_DONE);
	                                                 GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_SnapshotOK),TAG_DONE);
	                                                 GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GTCB_Checked,(ev->ev_Flags & EVF_Full),TAG_DONE);

	                                                 /* Turn on specifics */
	                                                 GT_SetGadgetAttrs(Delete1,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                                 GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GA_Disabled,FALSE,GTLV_Selected,~0,TAG_DONE);
	                                                 GT_RefreshWindow(ConfigWindow,0L);
	                                                 CurrentlySelected = ev;
	                                                 CurrentUser = 0L;
	                                             }
	                                             else
	                                                 FreeMem(ev,sizeof(struct ExportVolume));
	                                         }
	                                     }
	                                     if (r)
	                                         ResetBusyPointer(r,ConfigWindow);
	                                     if (CurrentlySelected)
	                                         ActivateGadget(Name,ConfigWindow,NULL);
	                                     FreeAslRequest(ar);
	                                     break;
	                                 }
	                                 case ID_Add2:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         char newuser[64];
	                                         BOOL type;
	                                         struct Requester *r;
	                                         newuser[0]=0;
	                                         r = SetBusyPointer(ConfigWindow);
	                                         type = GUserRequest(&newuser[0]);
	                                         if (r)
	                                             ResetBusyPointer(r,ConfigWindow);
	                                         if (type && !newuser[0])
	                                             break;
	                                         if (newuser[0])
	                                         {
	                                             struct UserOrGroup *nug;
	                                             nug = (struct UserOrGroup *) AllocMem(sizeof(struct UserOrGroup),MEMF_PUBLIC);
	                                             if (nug)
	                                             {
	                                                 nug->ug_Link.ln_Name = (STRPTR) AllocMem(strlen(newuser)+1,MEMF_PUBLIC);
	                                                 if (type)
	                                                 {
	                                                     struct UserInfo *ui;
	                                                     ui = AllocUserInfo();
	                                                     if (ui)
	                                                     {
	                                                         if (!NameToUser(newuser,ui))
	                                                             nug->ug_ID = ui->ui_UserID;
	                                                         else
	                                                         {
	                                                             FreeUserInfo(ui);
	                                                             FreeMem(nug->ug_Link.ln_Name,
									     strlen(nug->ug_Link.ln_Name)+1);
	                                                             FreeMem(nug,sizeof(struct UserOrGroup));
	                                                             break;
	                                                         }
	                                                         FreeUserInfo(ui);
	                                                     }
	                                                     nug->ug_Type = UGTYPE_USER;
	                                                 }
	                                                 else
	                                                 {
	                                                     struct GroupInfo *gi;
	                                                     gi = AllocGroupInfo();
	                                                     if (gi)
	                                                     {
	                                                         if (!NameToGroup(newuser,gi))
	                                                             nug->ug_ID = gi->gi_GroupID;
	                                                         else
	                                                         {
	                                                             FreeGroupInfo(gi);
	                                                             FreeMem(nug->ug_Link.ln_Name,
									     strlen(nug->ug_Link.ln_Name)+1);
	                                                             FreeMem(nug,sizeof(struct UserOrGroup));
	                                                             break;
	                                                         }
	                                                         FreeGroupInfo(gi);
	                                                     }
	                                                     nug->ug_Type = UGTYPE_GROUP;
	                                                 }

	                                                 strcpy(nug->ug_Link.ln_Name,&newuser[0]);
	                                                 GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,~0,TAG_DONE);
	                                                 AddTail(&CurrentlySelected->ev_Users,(struct Node *)nug);
	                                                 GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,&CurrentlySelected->ev_Users,TAG_DONE);
	                                                 GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                                 GT_RefreshWindow(ConfigWindow,0L);
	                                             }

	                                         }
	                                     }
	                                     break;
	                                 }
	                                 case ID_Delete1:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         struct UserOrGroup *muog;
	                                         GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Labels,~0,TAG_DONE);
	                                         GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,0,TAG_DONE);
	                                         Remove((struct Node *) CurrentlySelected);
	                                         GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Labels,&ExportList,TAG_DONE);
						 // deselect
						 GT_SetGadgetAttrs(lv,ConfigWindow,NULL,GTLV_Selected,~0L,TAG_DONE);

	                                         /* fixme - should make sure all of gadgets are set up right now */
	                                         while (muog = (struct UserOrGroup *) RemHead(&CurrentlySelected->ev_Users))
	                                         {
	                                             FreeMem(muog->ug_Link.ln_Name,strlen(muog->ug_Link.ln_Name)+1);
	                                             FreeMem(muog,sizeof(struct UserOrGroup));
	                                         }
	                                         FreeMem(CurrentlySelected->ev_Link.ln_Name,
							 strlen(CurrentlySelected->ev_Link.ln_Name)+1);
	                                         FreeMem(CurrentlySelected,sizeof(struct ExportVolume));
	                                         CurrentlySelected = 0L;
	                                         /* Turn off specifics when no mount is selected */
	                                         GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Delete1,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,0L,TAG_DONE);
	                                         GT_RefreshWindow(ConfigWindow,0L);

	                                         GT_SetGadgetAttrs(Name,ConfigWindow,NULL,GTST_String,0L,TAG_DONE);
	                                         GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Leftout,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Snapshot,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_RefreshWindow(ConfigWindow,0L);
	                                     }

	                                     break;
	                                 }
	                                 case ID_Delete2:
	                                 {
	                                     if (CurrentUser)
	                                     {
	                                         GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,~0,TAG_DONE);
	                                         Remove((struct Node *) CurrentUser);
	                                         GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Labels,&CurrentlySelected->ev_Users,TAG_DONE);
						 // deselect
						 GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GTLV_Selected,~0L,TAG_DONE);
	                                         FreeMem(CurrentUser->ug_Link.ln_Name,
							 strlen(CurrentUser->ug_Link.ln_Name)+1);
	                                         FreeMem(CurrentUser,sizeof(struct UserOrGroup));
	                                         CurrentUser = 0L;
	                                         GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                         GT_RefreshWindow(ConfigWindow,0L);

	                                     }
	                                     break;
	                                 }
	                                 case ID_Cancel:
	                                 {
	                                     cont = FALSE;
	                                     break;
	                                 }
	                                 case ID_Snapshot:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         if (Snapshot->Flags & GFLG_SELECTED)
	                                             CurrentlySelected->ev_Flags |= EVF_SnapshotOK;
	                                         else
	                                             CurrentlySelected->ev_Flags &= ~EVF_SnapshotOK;
	                                     }
	                                     break;
	                                 }
	                                 case ID_Leftout:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         if (Leftout->Flags & GFLG_SELECTED)
	                                             CurrentlySelected->ev_Flags |= EVF_LeftoutOK;
	                                         else
	                                             CurrentlySelected->ev_Flags &= ~EVF_LeftoutOK;
	                                     }
	                                     break;
	                                 }
	                                 case ID_NoSecurity:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         if (Off->Flags & GFLG_SELECTED)
	                                         {
	                                             GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                             GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                             GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,TRUE,TAG_DONE);
	                                             CurrentlySelected->ev_Flags |= EVF_NoSecurity;
	                                         }
	                                         else
	                                         {
	                                             GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                             GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                             GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                             CurrentlySelected->ev_Flags &= ~EVF_NoSecurity;
	                                         }
	                                         CurrentlySelected->ev_Flags &= ~EVF_Full;
	                                         GT_SetGadgetAttrs(Full,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_RefreshWindow(ConfigWindow,0L);

	                                     }
	                                     break;
	                                 }
	                                 case ID_Full:
	                                 {
	                                     if (CurrentlySelected)
	                                     {
	                                         if (Full->Flags & GFLG_SELECTED)
	                                             CurrentlySelected->ev_Flags |= EVF_Full;
	                                         else
	                                             CurrentlySelected->ev_Flags &= ~EVF_Full;
	                                     }
	                                     /* If nosecurity is checked, and they just checked full perms, turn no security off */
	                                     if ((CurrentlySelected->ev_Flags & (EVF_NoSecurity|EVF_Full)) == (EVF_NoSecurity|EVF_Full))
	                                     {
	                                         CurrentlySelected->ev_Flags &= ~EVF_NoSecurity;
	                                         GT_SetGadgetAttrs(Off,ConfigWindow,NULL,GTCB_Checked,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Delete2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(lv2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                         GT_SetGadgetAttrs(Add2,ConfigWindow,NULL,GA_Disabled,FALSE,TAG_DONE);
	                                     }
	                                     break;
	                                 }
	                                 case ID_Save:
	                                 {
	                                     SaveConfig(&ExportList,FALSE);
	                                     SaveConfig(&ExportList,TRUE);
	                                     cont = FALSE;
	                                     break;
	                                 }
	                                 default:
	                                     break;
	                             }
	                         }
	                     }
	                     GT_ReplyIMsg(im);
	                 }
	             }
	             CloseWindow(ConfigWindow);
	         }
	     }
	 }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                FreeGadgets(FirstGadget);
                            }
                            if (onscreen)
                            {
                                UnlockPubScreen(NULL,onscreen);
                            }

                            CloseLibrary(GadToolsBase);
                        }
                        else
                            CouldNotOpen("gadtools.library");
                        CloseLibrary(AslBase);
                    }
                    else
                        CouldNotOpen("asl.library");
                    CloseLibrary(EnvoyBase);
                }
                else
                    CouldNotOpen("envoy.library");
                CloseLibrary(AccountsBase);
            }
            else
                CouldNotOpen("accounts.library");
            CloseLibrary(DOSBase);
        }
        else
            CouldNotOpen("dos.library");
        CloseLibrary(IntuitionBase);
    }

/* Free up our list of structures */
    while (aev = (struct ExportVolume *) RemHead(&ExportList))
    {
        struct UserOrGroup *uog;
        while (uog = (struct UserOrGroup *) RemHead(&aev->ev_Users))
        {
            FreeMem(uog->ug_Link.ln_Name,strlen(uog->ug_Link.ln_Name)+1);
            FreeMem(uog,sizeof(struct UserOrGroup));
        }
        FreeMem(aev->ev_Link.ln_Name,strlen(aev->ev_Link.ln_Name)+1);
        FreeMem(aev,sizeof(struct ExportVolume));
    }
}


BOOL GUserRequest(UBYTE *buffer)
{

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
        {
            if (strlen(userbuff))
            {
                strcpy(buffer,userbuff);
                return(TRUE);
            }
            else
            {
                strcpy(buffer,groupbuff);
                return(FALSE);
            }
        }
    }
    buffer[0]=0;
    return(TRUE);
}


/*********************/

struct ConfigStruct
{
    UBYTE   MountName[64];
    UBYTE   VolumeName[64];
    ULONG   Flags;
};

struct ConfigUser
{
    UWORD   ID;
    UBYTE   Flags;
    UBYTE   Filler;
};


#define ID_EFSC     MAKE_ID('E','F','S','C')
#define ID_VOLM     MAKE_ID('V','O','L','M')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EFSC,ID_VOLM};
struct Library *IFFParseBase;

BOOL LoadConfig(struct List *ExportList)
{

    struct ConfigStruct *cs;
    struct ConfigUser *cu;

    cs = (struct ConfigStruct *) AllocMem(sizeof(struct ConfigStruct),MEMF_PUBLIC);
    if (cs)
    {
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
                    iff->iff_Stream = (ULONG) Open("env:envoy/efs.config",MODE_OLDFILE);
                    if (!iff->iff_Stream)
                        iff->iff_Stream = (ULONG) Open("envarc:envoy/efs.config",MODE_OLDFILE);
                    if (iff->iff_Stream)
                    {
                        InitIFFasDOS(iff);
                        if (!OpenIFF(iff,IFFF_READ))
                        {
                            if (!StopChunk(iff,ID_EFSC,ID_VOLM))
                            {
                                if (!ParseIFF(iff,IFFPARSE_SCAN))
                                {
                                    while (TRUE)
                                    {
                                        struct ContextNode *cn;
                                        cn = (struct ContextNode *) CurrentChunk(iff);
                                        if (cn)
                                        {
                                            ULONG size;
                                            size = cn->cn_Size;
                                            if (ReadChunkBytes(iff,cs,sizeof(struct ConfigStruct)) == sizeof(struct ConfigStruct))
                                            {
                                                struct ExportVolume *ev;
                                                ev = (struct ExportVolume *) AllocMem(sizeof(struct ExportVolume),MEMF_PUBLIC);
                                                if (ev)
                                                {
                                                    UBYTE *namex;
                                                    namex = (UBYTE *) AllocMem(strlen(cs->MountName)+1,MEMF_PUBLIC);
                                                    if (namex)
                                                    {
                                                        strcpy (namex,cs->MountName);
                                                        strcpy (ev->ev_VolumeName,cs->VolumeName);
                                                        ev->ev_Flags = cs->Flags;
                                                        ev->ev_Link.ln_Name = namex;
                                                        NewList(&ev->ev_Users);
                                                        AddTail(ExportList,&(ev->ev_Link));
                                                    }
                                                }
                                                size -= sizeof(struct ConfigStruct);
                                                while(size)
                                                {
                                                    if (ReadChunkBytes(iff,cu,sizeof(struct ConfigUser)) == sizeof(struct ConfigUser))
                                                    {
                                                        struct UserOrGroup *ugx;
                                                        ugx = (struct UserOrGroup *) AllocMem(sizeof(struct UserOrGroup),MEMF_PUBLIC);
                                                        if (ugx)
                                                        {
                                                            UBYTE *namex;
                                                            ugx->ug_ID = cu->ID;
                                                            ugx->ug_Type = cu->Flags;
                                                            if (!ugx->ug_Type)
                                                            {
                                                                struct UserInfo *ui;
                                                                ui = AllocUserInfo();
                                                                if (ui)
                                                                {
                                                                    if (!IDToUser(cu->ID,ui))
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(strlen(ui->ui_UserName)+1,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,ui->ui_UserName);
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    else
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(8,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,"Old UID");
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    AddTail(&ev->ev_Users,&(ugx->ug_Link));
                                                                    FreeUserInfo(ui);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                struct GroupInfo *gi;
                                                                gi = AllocGroupInfo();
                                                                if (gi)
                                                                {
                                                                    if (!IDToGroup(cu->ID,gi))
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(strlen(gi->gi_GroupName)+1,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,gi->gi_GroupName);
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    else
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(8,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,"Old GID");
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    AddTail(&ev->ev_Users,&(ugx->ug_Link));
                                                                    FreeGroupInfo(gi);
                                                                }

                                                            }
                                                        }
                                                    }
                                                    else
                                                        break;
                                                    size -= sizeof(struct ConfigUser);
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
        FreeMem(cs,sizeof(struct ConfigStruct));
    }
    return(TRUE);
}



BOOL SaveConfig(struct List *ExportList, BOOL which)
{

    struct ConfigStruct *cs;
    struct ConfigUser *cu;


    cs = (struct ConfigStruct *) AllocMem(sizeof(struct ConfigStruct),MEMF_PUBLIC);
    if (cs)
    {
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
                        iff->iff_Stream = (ULONG) Open("env:envoy/efs.config",MODE_NEWFILE);
                    else
                        iff->iff_Stream = (ULONG) Open("envarc:envoy/efs.config",MODE_NEWFILE);
                    if (iff->iff_Stream)
                    {
                        InitIFFasDOS(iff);
                        if (!OpenIFF(iff,IFFF_WRITE))
                        {
                            if (!IsListEmpty(ExportList))
                            {
                                if (!PushChunk(iff,ID_EFSC,ID_FORM,IFFSIZE_UNKNOWN))
                                {
                                    struct ExportVolume *ev;
                                    ev = (struct ExportVolume *) ExportList->lh_Head;
                                    while (ev->ev_Link.ln_Succ)
                                    {
                                        if (!PushChunk(iff,ID_EFSC,ID_VOLM,IFFSIZE_UNKNOWN))
                                        {
                                            STRPTR w;
                                            strcpy(cs->MountName,ev->ev_Link.ln_Name);
                                            strcpy(cs->VolumeName,ev->ev_VolumeName);
                                            w = (STRPTR) strchr(cs->VolumeName,':');
                                            if (w)
                                                *w = 0;
                                            cs->Flags = ev->ev_Flags;
                                            WriteChunkBytes(iff,cs,sizeof(struct ConfigStruct));
                                            if (!IsListEmpty(&ev->ev_Users))
                                            {
                                                struct UserOrGroup *ug;
                                                ug = (struct UserOrGroup *) ev->ev_Users.lh_Head;
                                                while (ug->ug_Link.ln_Succ)
                                                {
                                                    cu->ID = ug->ug_ID;
                                                    cu->Flags = ug->ug_Type;
                                                    WriteChunkBytes(iff,cu,sizeof(struct ConfigUser));
                                                    ug = (struct UserOrGroup *) ug->ug_Link.ln_Succ;
                                                }
                                            }
                                            PopChunk(iff);
                                        }
                                        ev = (struct ExportVolume *) ev->ev_Link.ln_Succ;
                                    }
                                    PopChunk(iff);
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
        FreeMem(cs,sizeof(struct ConfigStruct));
    }
    return(TRUE);
}

