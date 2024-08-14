
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <clib/gadtools_protos.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>

/*------------------------------------------------------------------------*/

/*  GadgetID definitions: */
#define G_COMMAND   1
#define G_OK        2
#define G_CANCEL    3

/*------------------------------------------------------------------------*/

/*  Size of buffer for command string gadget: */
#define COMMANDLENGTH       99

/*------------------------------------------------------------------------*/

#define WIN_Title   WinTags[0].ti_Data
#define WIN_Screen  WinTags[1].ti_Data
#define WIN_Height  WinTags[2].ti_Data
#define WIN_Width   WinTags[3].ti_Data
#define WIN_Gadgets WinTags[4].ti_Data
#define WIN_IDCMP   WinTags[5].ti_Data

#define ewin_IDCMP  IDCMP_GADGETUP|IDCMP_ACTIVEWINDOW

#define BETWEEN 4

#define SysBase (*((struct ExecBase **)4))

struct Library *DOSBase;
struct Library *IntuitionBase;
struct Library *GadToolsBase;

WORD GetString(char *, char *, char *, char *);

/*****************************************************************************/


#define TEMPLATE      "TITLE/A,BODY/A,LABEL/A,DEFAULT/K"
#define OPT_TITLE     0
#define OPT_BODY      1
#define OPT_LABEL     2
#define OPT_DEFAULT   3
#define OPT_COUNT     4


int main (int argc, char **argv) {

struct RDArgs     *rdargs;
char              *opts[OPT_COUNT];
char               buffer[COMMANDLENGTH+1];
int                exit_code = 0;

    if (DOSBase = OpenLibrary("dos.library", 0)) {

        if (IntuitionBase = OpenLibrary("intuition.library", 0)) {

            memset(opts, 0, sizeof(opts));

            if (rdargs = ReadArgs(TEMPLATE, (LONG *)opts, NULL)) {

                if (opts[OPT_DEFAULT]) strcpy(&buffer[0], opts[OPT_DEFAULT]);

                if (GetString(opts[OPT_TITLE],
                              opts[OPT_BODY],
                              opts[OPT_LABEL],
                              &buffer[0]) == 1) {

                    printf("%s\n", buffer);
                    }

                else exit_code = 5;

                FreeArgs(rdargs);
                }

            else printf("required argument missing\n");

            CloseLibrary(IntuitionBase);
            }

        CloseLibrary(DOSBase);
        }

    exit(exit_code);
    }


void StripIntuiMessages(struct MsgPort *mp, struct Window *win) {

struct IntuiMessage *msg;
struct Node         *succ;

    msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;

    while (succ = msg->ExecMessage.mn_Node.ln_Succ) {

        if (msg->IDCMPWindow == win) {

            Remove(msg);
            ReplyMsg(msg);
            }
        
        msg = (struct IntuiMessage *) succ;
        }
    }


WORD GetString(char *title,
               char *body,
               char *label,
               char *buffer) {

VOID                   *VisualInfo;
struct Window          *ewin;
struct IntuiMessage    *imsg, *imsg2;
struct Screen          *Screen;
struct Gadget          *gad;
struct Gadget          *stringGad;
struct MsgPort         *IPort;
struct  NewGadget       ng;

struct TagItem          WinTags[] = {

    {WA_Title,NULL},
    {WA_CustomScreen,NULL},
    {WA_Height,0},
    {WA_Width,400},
    {WA_Gadgets,NULL},
    {WA_IDCMP,0},
    {WA_Flags,WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SIMPLE_REFRESH|WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_NOCAREREFRESH},
    {TAG_DONE,0}
    };

WORD exit_code=0;

    IPort = CreateMsgPort();

    if (IntuitionBase = OpenLibrary("intuition.library", 0)) {

        if (GadToolsBase = OpenLibrary("gadtools.library", 0)) {

            Screen=LockPubScreen(NULL);
            UnlockPubScreen(NULL, Screen);

            WIN_Title=(ULONG)title;
            WIN_Screen=(ULONG)Screen;
            WIN_Height=Screen->WBorTop + Screen->BarHeight - Screen->BarVBorder + Screen->WBorTop + BETWEEN;
        
            if (VisualInfo=GetVisualInfo(Screen,TAG_DONE)) {
        
                LONG    label_width;
                LONG    ok_width;
                LONG    cancel_width;
                LONG    body_width;
        
                /* Find the sizes of the various strings */
                {
                    struct  IntuiText   itext;
        
                        itext.LeftEdge=0;
                        itext.ITextFont=Screen->Font;
                        itext.IText=label;
                        label_width=IntuiTextLength(&itext);
                        itext.IText="Ok";
                        ok_width=IntuiTextLength(&itext) + 12;
                        itext.IText="Cancel";
                        cancel_width=IntuiTextLength(&itext) + 12;
                        itext.IText=body;
                        body_width=IntuiTextLength(&itext);
                        }
        
                if (gad=CreateContext((struct Gadget **)&WIN_Gadgets)) {
        
                    ng.ng_TextAttr = Screen->Font;
                    ng.ng_LeftEdge = Screen->WBorLeft << 1;
                    ng.ng_TopEdge = WIN_Height;
                    body_width -= (ng.ng_Width = WIN_Width - ng.ng_LeftEdge - (Screen->WBorRight<<1));
        
                    if (body_width > 0) {
        
                        ng.ng_Width+=body_width;
                        WIN_Width+=body_width;
                        }
        
                    WIN_Height += (ng.ng_Height = Screen->Font->ta_YSize) + BETWEEN;
                    ng.ng_GadgetText = body;
                    ng.ng_Flags = PLACETEXT_IN;
                    ng.ng_VisualInfo = VisualInfo;
                    gad=CreateGadget(TEXT_KIND,gad,&ng,TAG_DONE);
        
                    ng.ng_GadgetID = G_COMMAND;
                    ng.ng_LeftEdge+= label_width+10;
                    ng.ng_TopEdge = WIN_Height;
                    ng.ng_Width = WIN_Width - ng.ng_LeftEdge - (Screen->WBorRight << 1);
                    WIN_Height += (ng.ng_Height = Screen->Font->ta_YSize+6) + BETWEEN;
                    ng.ng_GadgetText = label;
                    ng.ng_Flags = PLACETEXT_LEFT;
                    gad = CreateGadget(STRING_KIND, gad, &ng,
                                GTST_String,buffer,
                                GTST_MaxChars,COMMANDLENGTH,
                                GTST_EditHook,NULL,
                                GA_TabCycle,FALSE,
                                TAG_DONE);
        
                    if (stringGad=gad) ((struct StringInfo *)gad->SpecialInfo)->BufferPos = strlen(buffer);
        
                    if (cancel_width>ok_width) ok_width=cancel_width;
        
                    /*  Create a "OK" button relative to the bottom: */
                    ng.ng_GadgetID = G_OK;
                    ng.ng_LeftEdge = Screen->WBorLeft << 1;
                    ng.ng_TopEdge = WIN_Height;
                    ng.ng_Width = ok_width;
                    WIN_Height += (ng.ng_Height = Screen->Font->ta_YSize+6) + BETWEEN;
                    ng.ng_GadgetText = "Ok";
                    ng.ng_Flags = PLACETEXT_IN;
                    gad = CreateGadget(BUTTON_KIND,gad,&ng,TAG_DONE);
        
                    /*  Create a "CANCEL" button relative to the bottom and right: */
                    ng.ng_GadgetID = G_CANCEL;
                    ng.ng_LeftEdge = WIN_Width-(Screen->WBorRight << 1)-ok_width;
                    ng.ng_GadgetText = "Cancel";
                    gad = CreateGadget(BUTTON_KIND,gad,&ng,TAG_DONE);
        
                    WIN_Height += Screen->WBorBottom;
        
                    if (gad) {
        
                        if (ewin=OpenWindowTagList(NULL,WinTags)) {
        
                            ewin->UserPort = IPort;
                            if (ModifyIDCMP(ewin,ewin_IDCMP)) {
        
                                GT_RefreshWindow(ewin,NULL);
                                ActivateGadget(stringGad,ewin,NULL);
        
                                while (!exit_code) {
        
                                    if (imsg=(struct IntuiMessage *)GetMsg(IPort)) {
        
                                        if (imsg->IDCMPWindow == ewin) {
        
                                            if (imsg2=GT_FilterIMsg(imsg)) {
        
                                                if      (IDCMP_ACTIVEWINDOW == imsg2->Class) ActivateGadget(stringGad,ewin,NULL);
                                                else if (IDCMP_GADGETUP     == imsg2->Class) {
        
                                                    switch (((struct Gadget *) imsg2->IAddress)->GadgetID & GADTOOLMASK) {
                                                        case G_OK:
                                                        case G_COMMAND: exit_code=1;
                                                                        stccpy(buffer,((struct StringInfo *)stringGad->SpecialInfo)->Buffer,COMMANDLENGTH);
                                                                        break;
        
                                                        case G_CANCEL:  exit_code=-1;
                                                                        break;
                                                        }
                                                    }
        
                                                GT_PostFilterIMsg(imsg2);
                                                }
        
                                            ReplyMsg((struct Message *)imsg);
                                            }
        
                                        else StripIntuiMessages(IPort, ewin);
                                        }
        
                                    else WaitPort(IPort);
                                    }
                                }
        
                            ewin->UserPort = NULL;
                            ModifyIDCMP(ewin, NULL);
                            StripIntuiMessages(IPort, ewin);
                            CloseWindow(ewin);
                            }
                        }
        
                    FreeGadgets((struct Gadget *)WIN_Gadgets);
                    }
        
                FreeVisualInfo(VisualInfo);
                }
        
            CloseLibrary(GadToolsBase);
            }
        
        CloseLibrary(IntuitionBase);
        }
        
    DeleteMsgPort(IPort);

    return(exit_code);
    }

