
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>
#include <graphics/text.h>
#include "req.h"
#include <utility/tagitem.h>
#include <envoy/nipc.h>
#include <envoy/envoy.h>
#include <envoy/accounts.h>
#include <dos/dos.h>
#include <intuition/sghooks.h>
#include <intuition/gadgetclass.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/accounts_pragmas.h>

#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/layers_protos.h>
#include <clib/accounts_protos.h>

#include <string.h>
#include "envoybase.h"
#include "userreq.h"

#define ID_LISTVIEW     1
#define ID_OK           2
#define ID_CANCEL       3

void kprintf(STRPTR, ...);

#define MAX(x, y) ((x) > (y) ? (x):(y))

#define AccountsBase 	(urd->urd_AccountsBase)

extern struct TextAttr *topaz8;

typedef ULONG (*HOOK_FUNC)();

APTR UGetVisInfo(struct Screen *screen, ULONG tag1, ...)
{
    return(GetVisualInfoA(screen,(struct TagItem *)&tag1));
}

APTR UCreateGad(ULONG type, struct Gadget *last, struct NewGadget *ng, ULONG tag1, ...)
{
    return(CreateGadgetA(type, last, ng, (struct TagItem *)&tag1));
}

VOID SetGadAttrs(struct Gadget *gad, struct Window *window, ULONG tag1, ...)
{
    GT_SetGadgetAttrsA(gad, window, NULL, (struct TagItem *)&tag1);
}

struct Window *UOpenWinTags(ULONG tag1, ...)
{
    return(OpenWindowTagList(NULL, (struct TagItem *)&tag1));
}

BOOL UCreateReqGads(struct UserReqData *urd)
{
    struct NewGadget newgads[]={0,0,0,0,0,0,ID_LISTVIEW,PLACETEXT_LEFT,0,0,
    				0,0,0,0,"OK",0,ID_OK,PLACETEXT_IN,0,0,
    				0,0,0,0,"Cancel",0,ID_CANCEL,PLACETEXT_IN,0,0};

    UWORD InnerWidth, InnerHeight,LeftOffset,TopOffset,index;
    UWORD BottomLVEdge, BottomBorderEdge;

    BOOL status;

    LeftOffset = urd->urd_Window->BorderLeft;
    TopOffset = urd->urd_Window->BorderTop;
    InnerWidth = urd->urd_Window->Width - (LeftOffset + urd->urd_Window->BorderRight);
    InnerHeight = urd->urd_Window->Height - (TopOffset + urd->urd_Window->BorderBottom);

    EraseRect(urd->urd_Window->RPort,LeftOffset,TopOffset,LeftOffset+InnerWidth-1,TopOffset+InnerHeight-1);

    newgads[0].ng_LeftEdge = LeftOffset + 4;

    newgads[0].ng_Width = InnerWidth - 8;

    newgads[0].ng_TopEdge = TopOffset + 2;

    newgads[0].ng_Height = InnerHeight - (urd->urd_SysFont->tf_YSize+6) - 6;

    urd->urd_IText.IText="  Cancel  ";
    urd->urd_IText.ITextFont = urd->urd_SysFontAttr;

    newgads[1].ng_Width = 8 + IntuiTextLength(&urd->urd_IText);
    newgads[2].ng_Width = newgads[1].ng_Width;

    newgads[1].ng_LeftEdge = LeftOffset + 4;
    newgads[2].ng_LeftEdge = InnerWidth - newgads[2].ng_Width + LeftOffset - 4;

    newgads[1].ng_Height = newgads[2].ng_Height = urd->urd_SysFont->tf_YSize + 6;

    for(index=0;index<3;index++)
    {
    	newgads[index].ng_TextAttr = urd->urd_SysFontAttr;
    	newgads[index].ng_VisualInfo = urd->urd_VisualInfo;
    }

    newgads[0].ng_TextAttr = &urd->urd_LVTextAttr;

    urd->urd_LastGadget = CreateContext(&urd->urd_GadList);

    urd->urd_LastGadget = urd->urd_ListView = UCreateGad(LISTVIEW_KIND,urd->urd_LastGadget,&newgads[0],
    				    GTLV_ShowSelected,0,
    				    LAYOUTA_SPACING, 1,
    				    GTLV_ScrollWidth, 18,
    				    TAG_DONE);

    BottomLVEdge = newgads[0].ng_TopEdge + urd->urd_ListView->Height;
    BottomBorderEdge = urd->urd_Window->Height - urd->urd_Window->BorderBottom;

    newgads[1].ng_TopEdge = BottomLVEdge + ((BottomBorderEdge - BottomLVEdge)>>1) - (newgads[1].ng_Height >> 1);

    newgads[2].ng_TopEdge = newgads[1].ng_TopEdge;


    urd->urd_LastGadget = urd->urd_OK = UCreateGad(BUTTON_KIND,urd->urd_LastGadget,&newgads[1],
    				    TAG_DONE);

    urd->urd_LastGadget = urd->urd_Cancel =UCreateGad(BUTTON_KIND,urd->urd_LastGadget,&newgads[2],
    				    TAG_DONE);

    if(urd->urd_LastGadget)
    {
	AddGList(urd->urd_Window,urd->urd_GadList,-1,-1,0);
	RefreshGList(urd->urd_GadList,urd->urd_Window,0,-1);
	GT_RefreshWindow(urd->urd_Window,0L);
	status = TRUE;
    }
    else
    {
    	status = FALSE;
    	FreeGadgets(urd->urd_GadList);
    }

    return(status);
}

VOID UDeleteReqGads(struct UserReqData *urd)
{
    if(urd->urd_IsUP)
    {
    	SetGadAttrs(urd->urd_ListView,urd->urd_Window,GTLV_Labels, ~0, TAG_DONE);
        RemoveGList(urd->urd_Window,urd->urd_GadList,-1);
        FreeGadgets(urd->urd_GadList);
        urd->urd_IsUP = FALSE;
    }
}

VOID AddNodeSorted(struct UserReqData *urd, struct List *list, struct Node *node)
{
    struct Node *current;

    current = list->lh_TailPred;

    while(current->ln_Pred)
    {
    	if(Stricmp(node->ln_Name,current->ln_Name))
    	{
    	    Insert(list, node, current);
    	    break;
    	}
    	current = current->ln_Pred;
    }
    if(!current->ln_Pred)
    {
    	AddHead(list, node);
    }
}

VOID BuildUserList(struct UserReqData *urd)
{
    struct IntUserInfo *udat;

    NewList((struct List *)&urd->urd_UserList);

    if(urd->urd_UserInfo = AllocUserInfo())
    {
    	urd->urd_UserInfo->ui_UserID = 0;

    	while(!NextUser(urd->urd_UserInfo))
    	{
    	    if(udat = AllocMem(sizeof(struct IntUserInfo),MEMF_CLEAR|MEMF_PUBLIC))
    	    {
    	    	udat->Node.ln_Name = udat->UserName;
    	    	udat->Node.ln_Type = 252;
    	    	stccpy(udat->UserName,urd->urd_UserInfo->ui_UserName,32);

    	    	AddNodeSorted(urd,(struct List *)&urd->urd_UserList,(struct Node *)udat);
    	    }
    	}
    	FreeUserInfo(urd->urd_UserInfo);
    }
}

VOID BuildGroupList(struct UserReqData *urd)
{
    struct IntGroupInfo *gdat;

    NewList((struct List *)&urd->urd_GroupList);

    if(urd->urd_GroupInfo = AllocGroupInfo())
    {
    	urd->urd_GroupInfo->gi_GroupID = 0;

    	while(!NextGroup(urd->urd_GroupInfo))
    	{
    	    if(gdat = AllocMem(sizeof(struct IntGroupInfo),MEMF_CLEAR|MEMF_PUBLIC))
    	    {
    	    	gdat->Node.ln_Name = gdat->GroupName;
    	    	gdat->Node.ln_Type = 253;
    	    	stccpy(gdat->GroupName,urd->urd_GroupInfo->gi_GroupName,32);

    	    	AddNodeSorted(urd,(struct List *)&urd->urd_GroupList,(struct Node *)gdat);
    	    }
    	}
    	FreeGroupInfo(urd->urd_GroupInfo);
    }
}

VOID MergeLists(struct UserReqData *urd)
{
    struct Node *node;

    NewList((struct List *)&urd->urd_ListviewList);

    while(node = RemHead((struct List *)&urd->urd_GroupList))
    	AddTail((struct List *)&urd->urd_ListviewList,node);

    while(node = RemHead((struct List *)&urd->urd_UserList))
    	AddTail((struct List *)&urd->urd_ListviewList,node);

}

VOID FreeList(struct UserReqData *urd)
{
    struct Node *node;

    while(node = RemHead((struct List *)&urd->urd_ListviewList))
    {
    	if(node->ln_Type = 252)
    	    FreeMem(node,sizeof(struct IntUserInfo));
    	else
    	    FreeMem(node,sizeof(struct IntGroupInfo));
    }
}

VOID FixGroupNames(struct UserReqData *urd)
{
    struct IntGroupInfo *gdat;

    UWORD NumTotChars, NumDrwChars, NumGrpChars, LVWidth;

    LVWidth = urd->urd_ListView->Width - 18 - 8;

    NumTotChars = LVWidth / GfxBase->DefaultFont->tf_XSize;
    NumDrwChars = strlen(" <Group>");
    NumGrpChars = NumTotChars - NumDrwChars;

    if(NumGrpChars > 31)
    	NumGrpChars = 31;

    sprintf(urd->urd_LVFmt,"%%-%ld.%lds <Group>\0",(ULONG)NumGrpChars,(ULONG)NumGrpChars);

    gdat = (struct IntGroupInfo *)urd->urd_ListviewList.mlh_Head;

    while(gdat->Node.ln_Succ)
    {
    	if(gdat->Node.ln_Type == 253)
    	{
    	    gdat->Node.ln_Name = gdat->ListName;

    	    sprintf(gdat->ListName,urd->urd_LVFmt,gdat->GroupName);
    	}
    	gdat = (struct IntGroupInfo *)gdat->Node.ln_Succ;
    }

    SetGadAttrs(urd->urd_ListView,urd->urd_Window,GTLV_Labels, (ULONG)&urd->urd_ListviewList, TAG_DONE);
}

/****** envoy.library/UserRequestA *******************************************
*
*   NAME
*     UserRequestA -- Create a std. requester for choosing a user.
*
*   SYNOPSIS
*     ret = UserRequestA(taglist)
*     D0                   A0
*
*     BOOL  UserRequestA(struct TagList *);
*     BOOL  UserRequest(tag1, tag2, ...);
*
*   FUNCTION
*     Creates a system requester that allows the user to choose a username
*     from a list of available users on his system.
*
*   INPUTS
*     taglist - Made up of the following possible tags:
*
*           UGREQ_NameBuff - Specifies a pointer to the buffer where you wish
*                           the user's name name to be stored when the user
*                           selects "OK".
*
*           UGREQ_NameBuffLen - Maximum number of bytes allowed to be copied into
*                           the above buffer.
*
*           UGREQ_Left     - Initial left coordinate of the requester.
*
*           UGREQ_Top      - Initial top coordinate of the requester.
*
*           UGREQ_Width    - Horizontal size of requester in pixels.
*
*           UGREQ_Height   - Vertical size of requester in pixels.
*
*           UGREQ_Screen   - Defines the screen on which this requester
*                           should be created.  If not provided, it will be
*                           opened on the workbench screen.
*
*           UGREQ_Title    - Provides the name for the title bar of the
*                           requester's window.
*
*           UGREQ_NoDragBar -
*                           Prevents the requester's window from opening
*                           with a dragbar gadget; the requester will be
*                           locked in at the original position.
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
*
******************************************************************************
*
*/


BOOL __asm UserRequestA(register __a0 struct TagItem *intags)
{

    struct UserReqData *urd;
    struct Library *ABase;
    BOOL status,newsize = FALSE;

    struct TagItem wintags[]={ WA_DragBar,TRUE,			/* 0 */
                               WA_DepthGadget,TRUE,             /* 1 */
                               TAG_IGNORE,0,                    /* 2 */
                               TAG_IGNORE,0,                    /* 3 */
                               WA_Width,0,                      /* 4 */
                               WA_Height,0,                     /* 5 */
                               WA_Activate,TRUE,                /* 6 */
                               WA_Title,0,                      /* 7 */
                               WA_PubScreen,0,                  /* 8 */
                               WA_DragBar,TRUE,                 /* 9 */
                               WA_SizeGadget,TRUE,              /* 10 */
                               WA_SizeBBottom,TRUE,             /* 11 */
                               WA_MinWidth, 0,			/* 12 */
                               WA_MinHeight, 0,			/* 13 */
                               WA_MaxWidth, -1,			/* 14 */
                               WA_MaxHeight, -1,		/* 15 */
                               WA_AutoAdjust,TRUE,
                               WA_IDCMP,IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|LISTVIEWIDCMP|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP,
                               TAG_DONE,TRUE };

    if(ABase = OpenLibrary("accounts.library",37L))
    {
        if(urd = AllocMem(sizeof(struct UserReqData),MEMF_CLEAR|MEMF_PUBLIC))
        {
            AccountsBase = ABase;
            urd->urd_TagList = intags;

            urd->urd_DragBar = TRUE;
            urd->urd_SizeGadget = TRUE;

            urd->urd_Title = "Login Requester";

            while(urd->urd_TagItem = NextTagItem(&urd->urd_TagList))
            {
                switch(urd->urd_TagItem->ti_Tag)
                {
                    case UGREQ_UserBuff     : urd->urd_UserBuff = (STRPTR)urd->urd_TagItem->ti_Data;
                    			      *(urd->urd_UserBuff) = '\0';
                                              break;

                    case UGREQ_UserBuffLen  : urd->urd_UserBuffLen = urd->urd_TagItem->ti_Data;
                                              break;

		    case UGREQ_GroupBuff    : urd->urd_GroupBuff = (STRPTR)urd->urd_TagItem->ti_Data;
		    			      *(urd->urd_GroupBuff) = '\0';
		    			      break;

		    case UGREQ_GroupBuffLen : urd->urd_GroupBuffLen = urd->urd_TagItem->ti_Data;
		    			      break;

                    case UGREQ_Left         : wintags[2].ti_Tag = WA_Left;
                                              wintags[2].ti_Data = urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_Top          : wintags[3].ti_Tag = WA_Top;
                                              wintags[3].ti_Data = urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_Width        : wintags[4].ti_Data = urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_Height       : wintags[5].ti_Data = urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_Title        : urd->urd_Title = (STRPTR)urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_Screen       : urd->urd_Screen = (struct Screen *)urd->urd_TagItem->ti_Data;
                                              urd->urd_CustomScreen = TRUE;
                                              break;

                    case UGREQ_Window       : urd->urd_BlockWindow = (struct Window *)urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_MsgPort      : urd->urd_RefreshPort = (struct MsgPort *)urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_CallBack     : urd->urd_Hook = (struct Hook *)urd->urd_TagItem->ti_Data;
                                              break;

                    case UGREQ_NoDragBar    : wintags[9].ti_Tag = TAG_IGNORE;
                                              break;

                    case UGREQ_NoSizeGadget : wintags[10].ti_Tag = TAG_IGNORE;
                                              wintags[11].ti_Tag = TAG_IGNORE;
                                              break;

                }
            }

            if(!urd->urd_CustomScreen)
                urd->urd_Screen = LockPubScreen(NULL);

            urd->urd_SysFontAttr = urd->urd_Screen->Font;

            if(urd->urd_SysFont = OpenFont(urd->urd_SysFontAttr))
            {
                if(wintags[10].ti_Tag == WA_DragBar)
                    urd->urd_TopOffset = urd->urd_SysFont->tf_YSize + 1;
                else
                    urd->urd_TopOffset = urd->urd_Screen->WBorTop;

                if(wintags[11].ti_Tag == WA_SizeGadget)
                    urd->urd_BottomBorder = 10;
                else
                    urd->urd_BottomBorder = urd->urd_Screen->WBorBottom;

		urd->urd_LVTextAttr.ta_Name = urd->urd_LVTextName;
		urd->urd_LVTextAttr.ta_YSize = GfxBase->DefaultFont->tf_YSize;
		urd->urd_LVTextAttr.ta_Style = 0;
		urd->urd_LVTextAttr.ta_Flags = 0; /* GfxBase->DefaultFont->tf_Flags; */

		strcpy(urd->urd_LVTextName,GfxBase->DefaultFont->tf_Message.mn_Node.ln_Name);

                urd->urd_IText.IText = "  Cancel  ";
                urd->urd_IText.ITextFont = urd->urd_SysFontAttr;

                urd->urd_MinWidth = (IntuiTextLength(&urd->urd_IText) + 8)*2 + 12 + urd->urd_Screen->WBorLeft + urd->urd_Screen->WBorRight;
                urd->urd_MinHeight = urd->urd_SysFont->tf_YSize*2 + (urd->urd_LVTextAttr.ta_YSize+1)*4 + 8 + urd->urd_BottomBorder + 1 + 18;

		kprintf("urd->urd_MinHeight: %ld\n",(ULONG)urd->urd_MinHeight);
                wintags[12].ti_Data = urd->urd_MinWidth;
                wintags[13].ti_Data = urd->urd_MinHeight;

                if(!wintags[4].ti_Data)
                    wintags[4].ti_Data = urd->urd_MinWidth << 1;

                if(!wintags[5].ti_Data)
                    wintags[5].ti_Data = urd->urd_MinHeight << 1;

                if(urd->urd_Window = OpenWindowTagList(NULL,wintags))
                {
                    if(!urd->urd_CustomScreen)
                        UnlockPubScreen(NULL,urd->urd_Screen);

    //                          if(urd->urd_BlockWindow)
    //                              BusyWindow(urd,urd->urd_BlockWindow,TRUE);

                    if(urd->urd_Window->Width >= urd->urd_MinWidth)
                    {
                        if(urd->urd_Window->Height >= urd->urd_MinHeight)
                        {
                            BOOL cont;

                            urd->urd_VisualInfo = (APTR) UGetVisInfo(urd->urd_Screen,TAG_DONE);
                            if(urd->urd_IsUP = UCreateReqGads(urd))
                            {
                                urd->urd_Mask = (1L << urd->urd_Window->UserPort->mp_SigBit);

                                if(urd->urd_Hook && urd->urd_RefreshPort && urd->urd_BlockWindow)
                                    urd->urd_BlockMask = (1L << urd->urd_RefreshPort->mp_SigBit);

				if(urd->urd_UserBuff)
                                    BuildUserList(urd);

                                if(urd->urd_GroupBuff)
                               	    BuildGroupList(urd);

                               	MergeLists(urd);

				FixGroupNames(urd);

                                cont = TRUE;

                                while(cont)
                                {
                                    struct IntuiMessage *im;

                                    ULONG sigs;

                                    sigs = Wait(urd->urd_Mask | urd->urd_BlockMask);

                                    if(sigs & urd->urd_Mask)
                                    {
                                        while(im = (struct IntuiMessage *) GT_GetIMsg(urd->urd_Window->UserPort))
                                        {
                                            switch(im->Class)
                                            {
                                                    case IDCMP_GADGETUP:
                                                    case IDCMP_GADGETDOWN:  switch((((struct Gadget *)im->IAddress))->GadgetID)
                                                                            {
                                                                                case ID_CANCEL: cont = FALSE;
                                                                                                status = FALSE;
                                                                                                break;

                                                                                case ID_OK:
                                                                                                cont = FALSE;
                                                                                                status = TRUE;
                                                                                                break;

                                                                                case ID_LISTVIEW:
                                                                                		{
                                                                                		    struct Node *node;
                                                                                		    ULONG i;

                                                                                		    i = im->Code;

                                                                                		    node = (struct Node *)urd->urd_ListviewList.mlh_Head;
                                                                                		    while(i--)
                                                                                		    {
													node = node->ln_Succ;
												    }
												    if(node->ln_Type == 252)
												    {
												    	struct IntUserInfo *udat;

												    	udat = (struct IntUserData *)node;

												    	if(urd->urd_UserBuff)
												    	    stccpy(urd->urd_UserBuff,udat->UserName,32);
												    	if(urd->urd_GroupBuff)
												    	    *(urd->urd_GroupBuff)='\0';
												    }
												    else
												    {
												    	struct IntGroupInfo *gdat;

												    	gdat = (struct IntGroupData *)node;

												    	if(urd->urd_GroupBuff)
												    	    stccpy(urd->urd_GroupBuff,gdat->GroupName,32);
												    	if(urd->urd_UserBuff)
												    	    *(urd->urd_UserBuff)='\0';
												    }
												}
												if((urd->urd_ICode == im->Code) &&
												    DoubleClick(urd->urd_Seconds,urd->urd_Micros,
												    		im->Seconds,im->Micros))
												{
												    cont = FALSE;
												    status = TRUE;
												}
												else
												{
												    urd->urd_ICode = im->Code;
												    urd->urd_Seconds = im->Seconds;
												    urd->urd_Micros = im->Micros;
												}
												break;

                                                                            }
                                                                            break;

                                                    case IDCMP_REFRESHWINDOW:
                                                                            GT_BeginRefresh(urd->urd_Window);
                                                                            GT_EndRefresh(urd->urd_Window,TRUE);
                                                                            break;

                                                    case IDCMP_NEWSIZE:
                                                                            UDeleteReqGads(urd);

                                                                            if(urd->urd_IsUP = UCreateReqGads(urd))
                                                                            {
                                                                                FixGroupNames(urd);
                                                                                RefreshWindowFrame(urd->urd_Window);
                                                                            }
                                                                            else
                                                                            {
                                                                                cont = FALSE;
                                                                                status = TRUE;
                                                                            }

                                            }
                                            GT_ReplyIMsg(im);
                                        }

                                    }
                                }
                                FreeVisualInfo(urd->urd_VisualInfo);

                                FreeList(urd);

                                UDeleteReqGads(urd);
                            }
                        }
                    }
    //                          if(urd->urd_BlockWindow)
    //                              BusyWindow(urd,urd->urd_BlockWindow,FALSE);


                    CloseWindow(urd->urd_Window);
                }
                CloseFont(urd->urd_SysFont);
            }
            FreeMem(urd,sizeof(struct UserReqData));
        }
        CloseLibrary(ABase);
    }
    return(status);
}

