
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
#include <intuition/sghooks.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>

#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>
#include <clib/layers_protos.h>

#include <string.h>
#include "envoybase.h"
#include "loginreq.h"

#define ID_USERNAME     1
#define ID_PASSWORD     2
#define ID_OK           3
#define ID_CANCEL       4

void kprintf(STRPTR, ...);

#define MAX(x, y) ((x) > (y) ? (x):(y))

extern struct TextAttr *topaz8;

typedef ULONG (*HOOK_FUNC)();

APTR GetVisInfo(struct Screen *screen, ULONG tag1, ...)
{
    return(GetVisualInfoA(screen,(struct TagItem *)&tag1));
}

APTR CreateGad(ULONG type, struct Gadget *last, struct NewGadget *ng, ULONG tag1, ...)
{
    return(CreateGadgetA(type, last, ng, (struct TagItem *)&tag1));
}

struct Window *OpenWinTags(ULONG tag1, ...)
{
    return(OpenWindowTagList(NULL, (struct TagItem *)&tag1));
}

BOOL CreateReqGads(struct LoginReqData *lrd)
{
    struct NewGadget newgads[]={0,0,0,0,"Name",0,ID_USERNAME,PLACETEXT_LEFT,0,0,
    				0,0,0,0,"Password",0,ID_PASSWORD,PLACETEXT_LEFT,0,0,
    				0,0,0,0,"OK",0,ID_OK,PLACETEXT_IN,0,0,
    				0,0,0,0,"Cancel",0,ID_CANCEL,PLACETEXT_IN,0,0};

    UWORD InnerWidth, InnerHeight,LeftOffset,TopOffset,index;
    BOOL status;

    /* First, let's figure out which font we should be using. */

    if((lrd->lrd_Window->Width < lrd->lrd_OptimalWidth) ||
       (lrd->lrd_Window->Height < lrd->lrd_OptimalHeight))
    {
    	lrd->lrd_ReqFont = ENVOYBASE->eb_TopazFont;
    	lrd->lrd_ReqFontAttr = (struct TextAttr *) &topaz8;
    }
    else
    {
    	lrd->lrd_ReqFont = lrd->lrd_SysFont;
    	lrd->lrd_ReqFontAttr = lrd->lrd_SysFontAttr;
    }
    lrd->lrd_FontHeight = lrd->lrd_ReqFont->tf_YSize;

    SetFont(lrd->lrd_Window->RPort,lrd->lrd_ReqFont);

    LeftOffset = lrd->lrd_Window->BorderLeft;
    TopOffset = lrd->lrd_Window->BorderTop;
    InnerWidth = lrd->lrd_Window->Width - (LeftOffset + lrd->lrd_Window->BorderRight);
    InnerHeight = lrd->lrd_Window->Height - (TopOffset + lrd->lrd_Window->BorderBottom);

    EraseRect(lrd->lrd_Window->RPort,LeftOffset,TopOffset,LeftOffset+InnerWidth-1,TopOffset+InnerHeight-1);

    lrd->lrd_IText.ITextFont = lrd->lrd_ReqFontAttr;
    lrd->lrd_IText.IText="Password";

    newgads[0].ng_LeftEdge = LeftOffset + INTERWIDTH + 4 + IntuiTextLength(&lrd->lrd_IText);
    newgads[1].ng_LeftEdge = newgads[0].ng_LeftEdge;

    newgads[0].ng_Width = InnerWidth + 4 - INTERWIDTH - newgads[0].ng_LeftEdge + LeftOffset;
    newgads[1].ng_Width = newgads[0].ng_Width;

    newgads[0].ng_TopEdge = TopOffset + 2;
    newgads[1].ng_TopEdge = (InnerHeight>>1) + TopOffset - (lrd->lrd_FontHeight>>1) - 3 ;

    newgads[2].ng_TopEdge = InnerHeight + TopOffset - lrd->lrd_FontHeight - 8;

    newgads[3].ng_TopEdge = newgads[2].ng_TopEdge;

    lrd->lrd_IText.IText="  Cancel  ";

    newgads[2].ng_Width = 8 + IntuiTextLength(&lrd->lrd_IText);
    newgads[3].ng_Width = newgads[2].ng_Width;

    newgads[2].ng_LeftEdge = LeftOffset + 4;
    newgads[3].ng_LeftEdge = InnerWidth - newgads[3].ng_Width + LeftOffset - 4;

    for(index=0;index<4;index++)
    {
    	newgads[index].ng_Height = lrd->lrd_FontHeight + 6;
    	newgads[index].ng_TextAttr = lrd->lrd_ReqFontAttr;
    	newgads[index].ng_VisualInfo = lrd->lrd_VisualInfo;
    }

    lrd->lrd_LastGadget = CreateContext(&lrd->lrd_GadList);

    lrd->lrd_LastGadget = lrd->lrd_UserName = CreateGad(STRING_KIND,lrd->lrd_LastGadget,&newgads[0],
    				    GTST_String,lrd->lrd_NameStr,
    				    GTST_MaxChars, 31,
    				    TAG_DONE);

    lrd->lrd_LastGadget = lrd->lrd_Password = CreateGad(STRING_KIND,lrd->lrd_LastGadget,&newgads[1],
    				    GTST_String,lrd->lrd_PassStr,
    				    GTST_MaxChars, 31,
    				    GTST_EditHook, (ULONG)&lrd->lrd_EditHook,
    				    TAG_DONE);

    lrd->lrd_LastGadget = lrd->lrd_OK = CreateGad(BUTTON_KIND,lrd->lrd_LastGadget,&newgads[2],
    				    TAG_DONE);

    lrd->lrd_LastGadget = lrd->lrd_Cancel =CreateGad(BUTTON_KIND,lrd->lrd_LastGadget,&newgads[3],
    				    TAG_DONE);

    if(lrd->lrd_LastGadget)
    {
	AddGList(lrd->lrd_Window,lrd->lrd_GadList,-1,-1,0);
	RefreshGList(lrd->lrd_GadList,lrd->lrd_Window,0,-1);
	GT_RefreshWindow(lrd->lrd_Window,0L);
	status = TRUE;
    }
    else
    {
    	status = FALSE;
    	FreeGadgets(lrd->lrd_GadList);
    }

    return(status);
}

VOID DeleteReqGads(struct LoginReqData *lrd)
{
    if(lrd->lrd_IsUP)
    {
        RemoveGList(lrd->lrd_Window,lrd->lrd_GadList,-1);
        FreeGadgets(lrd->lrd_GadList);
        lrd->lrd_IsUP = FALSE;
    }
}

ULONG __asm passwordHook(register __a0 struct Hook *hook,
			          register __a2 struct SGWork *sgw,
			          register __a1 ULONG *msg)
{
    ULONG return_code;
    UBYTE *pass_ptr;

    pass_ptr = (STRPTR)hook->h_SubEntry;

    return_code = ~0L;

    if(*msg == SGH_KEY)
    {
    	if((sgw->EditOp == EO_REPLACECHAR) ||
    	   (sgw->EditOp == EO_INSERTCHAR))
    	{
    	    pass_ptr[sgw->BufferPos - 1] = sgw->WorkBuffer[sgw->BufferPos - 1];
    	    sgw->WorkBuffer[sgw->BufferPos - 1] = '�';
    	}
    	else if(sgw->EditOp == EO_MOVECURSOR)
    	{
    	    sgw->Actions &= ~SGA_USE;
    	}
    	else if((sgw->EditOp == EO_DELBACKWARD) ||
    	        (sgw->EditOp == EO_DELFORWARD) ||
    	        (sgw->EditOp == EO_CLEAR))
    	{
    	    pass_ptr[sgw->BufferPos] = 0;
    	}
    }
    else if(*msg == SGH_CLICK)
    {
    	sgw->BufferPos = sgw->NumChars;
    }
    else
    	return_code = 0;

    return(return_code);
}


/****** envoy.library/LoginRequestA *******************************************
*
*   NAME
*     LoginRequestA -- Create a std. requester for name and password
*
*   SYNOPSIS
*     ret = LoginRequestA(taglist)
*     D0                   A0
*
*     BOOL  LoginRequestA(struct TagList *);
*     BOOL  LoginRequest(tag1, tag2, ...);
*
*   FUNCTION
*     Creates a system requester that allows the user to enter his name
*     and password.
*
*   INPUTS
*     taglist - Made up of the following possible tags:
*
*           LREQ_NameBuff - Specifies a pointer to the buffer where you wish
*                           the user's name name to be stored when the user
*                           selects "OK".
*
*           LREQ_NameBuffLen - Maximum number of bytes allowed to be copied into
*                           the above buffer.
*
*	    LREQ_PassBuff - Specifies a pointer the buffer where you wish
*			    the user's password to be stored when the user
*			    selects "OK".
*
*	    LREQ_PassBuffLen - Maxmimum number of bytes allowed to be copied
*			    into the above buffer.
*
*           LREQ_Left     - Initial left coordinate of the requester.
*
*           LREQ_Top      - Initial top coordinate of the requester.
*
*           LREQ_Width    - Horizontal size of requester in pixels.
*
*           LREQ_Height   - Vertical size of requester in pixels.
*
*           LREQ_Screen   - Defines the screen on which this requester
*                           should be created.  If not provided, it will be
*                           opened on the workbench screen.
*
*           LREQ_Title    - Provides the name for the title bar of the
*                           requester's window.
*
*           LREQ_NoDragBar -
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


BOOL __asm LoginRequestA(register __a0 struct TagItem *intags)
{

    struct LoginReqData *lrd;
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
                               WA_IDCMP,IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|IDCMP_GADGETUP|IDCMP_CLOSEWINDOW|BUTTONIDCMP,
                               TAG_DONE,TRUE };

    if(lrd = AllocMem(sizeof(struct LoginReqData),MEMF_CLEAR|MEMF_PUBLIC))
    {
    	lrd->lrd_TagList = intags;

    	lrd->lrd_Width = 0;
    	lrd->lrd_Height = 0;
	lrd->lrd_DragBar = TRUE;
	lrd->lrd_SizeGadget = TRUE;

	lrd->lrd_Title = "Login Requester";

	lrd->lrd_EditHook.h_Entry = (HOOK_FUNC)&passwordHook;
	lrd->lrd_EditHook.h_SubEntry = (HOOK_FUNC)&lrd->lrd_PassStr[0];

    	while(lrd->lrd_TagItem = NextTagItem(&lrd->lrd_TagList))
    	{
    	    switch(lrd->lrd_TagItem->ti_Tag)
    	    {
    	    	case LREQ_NameBuff	: lrd->lrd_NameBuff = (STRPTR)lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_NameBuffLen	: lrd->lrd_NameBuffLen = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_PassBuff	: lrd->lrd_PassBuff = (STRPTR)lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_PassBuffLen	: lrd->lrd_PassBuffLen = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_Left		: wintags[2].ti_Tag = WA_Left;
    	    				  wintags[2].ti_Data = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_Top		: wintags[3].ti_Tag = WA_Top;
    	    				  wintags[3].ti_Data = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_Width		: wintags[4].ti_Data = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_Height	: wintags[5].ti_Data = lrd->lrd_TagItem->ti_Data;
    	    				  break;

    	    	case LREQ_Title		: lrd->lrd_Title = (STRPTR)lrd->lrd_TagItem->ti_Data;
    	    				  break;

		case LREQ_Screen	: lrd->lrd_Screen = (struct Screen *)lrd->lrd_TagItem->ti_Data;
					  lrd->lrd_CustomScreen = TRUE;
					  break;

		case LREQ_Window	: lrd->lrd_BlockWindow = (struct Window *)lrd->lrd_TagItem->ti_Data;
					  break;

		case LREQ_MsgPort	: lrd->lrd_RefreshPort = (struct MsgPort *)lrd->lrd_TagItem->ti_Data;
					  break;

		case LREQ_CallBack	: lrd->lrd_Hook = (struct Hook *)lrd->lrd_TagItem->ti_Data;
					  break;

    	    	case LREQ_NoDragBar	: wintags[9].ti_Tag = TAG_IGNORE;
    	    				  break;

    	    	case LREQ_NoSizeGadget  : wintags[10].ti_Tag = TAG_IGNORE;
    	    				  wintags[11].ti_Tag = TAG_IGNORE;
    	    				  break;

    	    }
        }

    	if(!lrd->lrd_CustomScreen)
    	    lrd->lrd_Screen = LockPubScreen(NULL);

    	lrd->lrd_SysFontAttr = lrd->lrd_Screen->Font;

    	if(lrd->lrd_SysFont = OpenFont(lrd->lrd_SysFontAttr))
	{
	    lrd->lrd_IText.ITextFont = lrd->lrd_SysFontAttr;
	    lrd->lrd_IText.IText = "N";
	    lrd->lrd_BigSpace = IntuiTextLength(&lrd->lrd_IText);
	    lrd->lrd_BigHeight = lrd->lrd_SysFont->tf_YSize;
	    if(wintags[10].ti_Tag == WA_DragBar)
	    	lrd->lrd_TopOffset = lrd->lrd_BigHeight + 1;
	    else
	    	lrd->lrd_TopOffset = lrd->lrd_Screen->WBorTop;

	    if(wintags[11].ti_Tag == WA_SizeGadget)
	    	lrd->lrd_BottomBorder = 10;
	    else
	        lrd->lrd_BottomBorder = lrd->lrd_Screen->WBorBottom;

	    lrd->lrd_OptimalWidth = (lrd->lrd_BigSpace)*15 + 20 + lrd->lrd_Screen->WBorLeft + lrd->lrd_Screen->WBorRight;
	    lrd->lrd_OptimalHeight = (lrd->lrd_BigHeight) * 4 + 1 + 2 + 8 + 18 + 8 + lrd->lrd_BottomBorder;

	    lrd->lrd_IText.IText = "  Cancel  ";
            lrd->lrd_IText.ITextFont = (struct TextAttr *) &topaz8;

	    lrd->lrd_MinWidth = (IntuiTextLength(&lrd->lrd_IText) + 8)*2 + 12 + lrd->lrd_Screen->WBorLeft + lrd->lrd_Screen->WBorRight;
	    lrd->lrd_MinHeight = (lrd->lrd_BigHeight) + 1 + 24 + + 2 + 8 + 18 + 8 + lrd->lrd_BottomBorder;

	    wintags[12].ti_Data = lrd->lrd_MinWidth;
	    wintags[13].ti_Data = lrd->lrd_MinHeight;

	    if(!wintags[4].ti_Data)
	    	wintags[4].ti_Data = lrd->lrd_OptimalWidth + (lrd->lrd_BigSpace * 15);

	    if(!wintags[5].ti_Data)
	    	wintags[5].ti_Data = lrd->lrd_OptimalHeight;

            if(lrd->lrd_Window = OpenWindowTagList(NULL,wintags))
       	    {
            	if(!lrd->lrd_CustomScreen)
            	    UnlockPubScreen(NULL,lrd->lrd_Screen);

//		    	    if(lrd->lrd_BlockWindow)
//    			        BusyWindow(lrd,lrd->lrd_BlockWindow,TRUE);

                if(lrd->lrd_Window->Width >= lrd->lrd_MinWidth)
                {
                    if(lrd->lrd_Window->Height >= lrd->lrd_MinHeight)
                    {
                        BOOL cont;

                        lrd->lrd_VisualInfo = (APTR) GetVisInfo(lrd->lrd_Screen,TAG_DONE);
                        if(lrd->lrd_IsUP = CreateReqGads(lrd))
                        {
		    	    lrd->lrd_Mask = (1L << lrd->lrd_Window->UserPort->mp_SigBit);

		    	    if(lrd->lrd_Hook && lrd->lrd_RefreshPort && lrd->lrd_BlockWindow)
		    	    	lrd->lrd_BlockMask = (1L << lrd->lrd_RefreshPort->mp_SigBit);

		    	    cont = TRUE;

			    while(cont)
			    {
			    	struct IntuiMessage *im;

			    	ULONG sigs;

			    	sigs = Wait(lrd->lrd_Mask | lrd->lrd_BlockMask);

			    	if(sigs & lrd->lrd_Mask)
			    	{
			    	    while(im = (struct IntuiMessage *) GT_GetIMsg(lrd->lrd_Window->UserPort))
			    	    {
			    	    	switch(im->Class)
			    	    	{
			    	    		case IDCMP_GADGETUP:
			    	    		case IDCMP_GADGETDOWN:	switch((((struct Gadget *)im->IAddress))->GadgetID)
			    	    					{
			    	    					    case ID_CANCEL: cont = FALSE;
			    	    					    		    status = FALSE;
			    	    					    		    break;

			    	    					    case ID_OK:
			    	    					    case ID_PASSWORD:
			    	    					    		    if(lrd->lrd_NameBuff)
												stccpy(lrd->lrd_NameBuff,((struct StringInfo *)lrd->lrd_UserName->SpecialInfo)->Buffer,lrd->lrd_NameBuffLen);

											    if(lrd->lrd_PassBuff)
											    	stccpy(lrd->lrd_PassBuff,lrd->lrd_PassStr /* ((struct StringInfo *)lrd->lrd_Password->SpecialInfo)->Buffer */,lrd->lrd_PassBuffLen);

											    cont = FALSE;
											    status = TRUE;
											    break;

									    case ID_USERNAME:
									    		    ActivateGadget(lrd->lrd_Password,lrd->lrd_Window,NULL);
									    		    GT_RefreshWindow(lrd->lrd_Window,0L);
									    		    break;


									}
			    	    					break;

			    	    		case IDCMP_REFRESHWINDOW:
			    	    					GT_BeginRefresh(lrd->lrd_Window);
			    	    					GT_EndRefresh(lrd->lrd_Window,TRUE);
			    	    					break;

                                                case IDCMP_NEWSIZE:
                                                			if(lrd->lrd_IsUP)
                                                			{
                                                                            stccpy(lrd->lrd_NameStr,((struct StringInfo *)lrd->lrd_UserName->SpecialInfo)->Buffer,32);
                                                                            stccpy(lrd->lrd_PassStr,((struct StringInfo *)lrd->lrd_Password->SpecialInfo)->Buffer,32);
                                                                        };
                                                			DeleteReqGads(lrd);

                                                			if(lrd->lrd_IsUP = CreateReqGads(lrd))
                                                			    RefreshWindowFrame(lrd->lrd_Window);
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
			    FreeVisualInfo(lrd->lrd_VisualInfo);

			    DeleteReqGads(lrd);
			}
		    }
		}
//			    if(lrd->lrd_BlockWindow)
//			    	BusyWindow(lrd,lrd->lrd_BlockWindow,FALSE);


		CloseWindow(lrd->lrd_Window);
	    }
	    CloseFont(lrd->lrd_SysFont);
	}
	FreeMem(lrd,sizeof(struct LoginReqData));
    }
    return(status);
}

