/*
 *	Written by Joe Pearce and Talin
 */

/****** camdlists.lib/AllocListRequest **********************************************
*
*   NAME
*       AllocListRequest -- allocate a scrolling list requester
*
*   SYNOPSIS
*       list = AllocListRequest( firsttag )
*
*		void *AllocListRequest( Tag, ... );
*
*       list = AllocListRequestA( taglist )
*
*		void *AllocListRequestA( struct TagItem * );
*
*   FUNCTION
*       This function allocates a list requester structure and fills it in
*       using the supplied tag list.
*
*   INPUTS
*		The following is a list of tags recognized. Note that since the
*		functionality of the list requester is very similar to the ASL
*		requesters, the actual ASL tag values have been used, and the
*		listrequester tags have been defined in terms of the ASL values.
*
*		These tags have the exact same meaning as the corresponding ASL tag:
*
*			LISTREQ_Window		   		ASLFR_Window
*			LISTREQ_Screen	       		ASLFR_Screen
*			LISTREQ_UserData	   		ASLFR_UserData
*			LISTREQ_TitleText      		ASLFR_TitleText
*			LISTREQ_PositiveText   		ASLFR_PositiveText
*			LISTREQ_NegativeText   		ASLFR_NegativeText
*			LISTREQ_InitialLeftEdge		ASLFR_InitialLeftEdge
*			LISTREQ_InitialTopEdge 		ASLFR_InitialTopEdge
*			LISTREQ_InitialWidth  		ASLFR_InitialWidth
*			LISTREQ_InitialHeight  		ASLFR_InitialHeight
*			LISTREQ_HookFunc	     	ASLFR_HookFunc
*
*		These additional tags are defined
*
*			LISTREQ_Labels - pointer to struct List of labels to display.
*
*			LISTREQ_Selected - The number of the selected list element.
*
*			LISTREQ_Buffer - an optional pointer to a character buffer.
*				Supplying this tag will cause a string gadget to appear
*				below the list, where the user can type the name of an
*				element not in the list. In addition, clicking on a
*				list element will copy that element's text into the
*				buffer, overwriting the string. When the requester
*				terminates, the contents of the string gadget will be copied
*				to the buffer.
*
*				In addition, this buffer can be filled in by the application
*				before calling this function to initialize the string gadget.
*
*			LISTREQ_BufferSize - size of the supplied buffer.
*
*		These tags have been defined but not yet implemented:
*
*			LISTREQ_PubScreenName  		ASLFR_PubScreenName
*			LISTREQ_PrivateIDCMP   		ASLFR_PrivateIDCMP
*			LISTREQ_IntuiMsgFunc   		ASLFR_IntuiMsgFunc
*			LISTREQ_SleepWindow    		ASLFR_SleepWindow
*			LISTREQ_TextAttr	   		ASLFR_TextAttr
*			LISTREQ_Locale	       		ASLFR_Locale
*
*
*   RESULT
*      list     -- a pointer to a list request.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       ListRequest(), FreeListRequest()
*
*****************************************************************************
*/

/****** camdlists.lib/ListRequest **********************************************
*
*   NAME
*       ListRequest -- display a scrolling list requester
*
*   SYNOPSIS
*       confirm = ListRequest( listreq, firsttag )
*
*		LONG ListRequest( void *, Tag, ... );
*
*       confirm = ListRequestA( listreq, taglist )
*
*		LONG ListRequestA( void *, struct TagItem * );
*
*   FUNCTION
*       This function displays the list requester previously created by
*       AllocListRequest
*
*   INPUTS
*		listreq -- pointer to a previously allocated list requester
*		tags -- the same tags values as AllocListRequest
*
*   RESULT
*      	confirm -- returns TRUE if the positive choice button was clicked,
*		or a list entry was double-clicked, returns FALSE if the negative
*		choice button was clicked.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       AllocListRequest(), FreeListRequest()
*
*****************************************************************************
*/

/****** camdlists.lib/FreeListRequest **********************************************
*
*   NAME
*       FreeListRequest -- display a scrolling list requester
*
*   SYNOPSIS
*       FreeListRequest( listreq )
*
*		FreeListRequest( void * );
*
*   FUNCTION
*       This function deallocates the list requester previously created by
*       AllocListRequest
*
*   INPUTS
*		listreq -- pointer to a previously allocated list requester
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       AllocListRequest(), ListRequest()
*
*****************************************************************************
*/

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <string.h>

#include "listreq.h"
#include "listreqlib.h"

struct Gadget *build_listreq_gadgets(struct PrivateListReq *req,
	struct Window *window, struct Gadget **glist, APTR visinfo);
void CloseWindowSafely(struct Window *win);
void StripIntuiMessages(struct MsgPort *mp, struct Window *win);

#define LR_IDCMP	(LISTVIEWIDCMP | BUTTONIDCMP | IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_MENUPICK)

	/* layout constants */

#define VERT_SPACING		4
#define HORIZ_SPACING		8
#define SLIDER_WIDTH		18

#define BUTTON_WIDTH		80

#define STRING_LENGTH		80

enum {
	OK_ID=1,
	CANCEL_ID,
	LIST_ID,
	ENTRY_ID
};

extern struct GfxBase	*GfxBase;
extern struct Library	*IntuitionBase;
extern struct Library	*GadToolsBase;
extern struct Library	*UtilityBase;

static struct NewMenu control_nm[] = {
	{	NM_TITLE,	"Control",			NULL,	NULL,	0L },
	{	NM_ITEM,	"Last Entry",		"L",	NULL,	0L,		NULL },
	{	NM_ITEM,	"Next Entry",		"N",	NULL,	0L,		NULL },
	{	NM_ITEM,	NM_BARLABEL,		NULL,	NULL,	0L },
	{	NM_ITEM,	"OK",				"O",	NULL,	0L,		NULL },
	{	NM_ITEM,	"Cancel",			"C",	NULL,	0L,		NULL },
	{	NM_END }
};

enum {
	ITEM_LAST=0,
	ITEM_NEXT,
	ITEM_CONTROL_NULL1,
	ITEM_OK,
	ITEM_CANCEL
};

static struct TextAttr
	topaz_8 = { "topaz.font",8,FS_NORMAL ,FPF_ROMFONT };

LONG do_listreq(struct PrivateListReq *req)
{	struct Window		*window = NULL;
	struct Screen		*screen = NULL;
	struct Gadget		*glist = NULL;
	struct Menu			*menus = NULL;
	APTR				visinfo = NULL;
	struct TextFont		*sysfont;
	struct TextAttr		ta;
	LONG				result = 0;

	sysfont = GfxBase->DefaultFont;

	if (req->Window != NULL) screen = req->Window->WScreen;
	else if (req->Screen != NULL) screen = req->Screen;

	window = OpenWindowTags(NULL,
		WA_Left,			req->LeftEdge,
		WA_Top,				req->TopEdge,
		WA_Width,			req->Width,
		WA_Height,			req->Height,
		WA_MinWidth,		200,
		WA_MinHeight,		100,
		WA_MaxWidth,		-1,
		WA_MaxHeight,		-1,
		WA_Title,			req->TitleText,
		WA_Activate,		TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_SizeGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_DepthGadget,		TRUE,
		WA_CloseGadget,		TRUE,
		WA_SizeBBottom,		TRUE,
		WA_AutoAdjust,		TRUE,
		WA_NewLookMenus,	TRUE,
		screen ? WA_CustomScreen : TAG_IGNORE, screen,
		TAG_END);
	if (window == NULL) goto bye;

	screen = window->WScreen;

	if (req->Window != NULL && !(req->Flags & LISTREQF_NEWIDCMP))
		window->UserPort = req->Window->UserPort;
	ModifyIDCMP(window,LR_IDCMP);

		/* make a text attr from a text font */

	ta.ta_Name = sysfont->tf_Message.mn_Node.ln_Name;
	ta.ta_YSize = sysfont->tf_YSize;
	ta.ta_Style = sysfont->tf_Style;
	ta.ta_Flags = sysfont->tf_Flags;

	if ( !(visinfo = GetVisualInfo(screen,TAG_END)) ) goto bye;

		/* make gadgets */

	if (!build_listreq_gadgets(req, window, &glist, visinfo)) goto bye;

	AddGList(window,glist,-1,-1,NULL);
	RefreshGList(glist, window, NULL, -1);
	GT_RefreshWindow(window,NULL);

	if (req->Active >= 0)
	{
		GT_SetGadgetAttrs(req->ListGadget, window, NULL,
			GTLV_Selected,	req->Active,
			TAG_END );
	}
	else if (req->Buffer != NULL)
	{
		GT_SetGadgetAttrs(req->StrGadget, window, NULL,
			GTST_String,	req->Buffer,
			TAG_END);
	}

	if ( !(menus = CreateMenus(control_nm, TAG_END)) ) goto bye;
	if ( !LayoutMenus(menus,visinfo,TAG_END) ) goto bye;
	SetMenuStrip(window,menus);

	while (1)
	{	struct IntuiMessage	imsg, *wimsg;

		Wait(1 << window->UserPort->mp_SigBit);

		while (wimsg = GT_GetIMsg(window->UserPort))
		{
			imsg = *wimsg;

			GT_ReplyIMsg(wimsg);

			if (imsg.IDCMPWindow == window)
			{
				switch (imsg.Class) {

				case IDCMP_CLOSEWINDOW:
					goto bye;

				case IDCMP_REFRESHWINDOW:
					GT_BeginRefresh(window);
					GT_EndRefresh(window,TRUE);
					break;

				case IDCMP_NEWSIZE:
					RemoveGList(window,glist,-1);

					if (req->Buffer != NULL)
					{
						strcpy(req->Buffer,
							((struct StringInfo *)req->StrGadget->SpecialInfo)->Buffer );
					}

					FreeGadgets(glist);
					glist = NULL;

					Forbid();
					StripIntuiMessages(window->UserPort, window);
					Permit();

					if (!build_listreq_gadgets(req, window, &glist, visinfo))
						goto bye;

					EraseRect(window->RPort,
						window->BorderLeft, window->BorderTop,
						window->Width - window->BorderRight - 1,
						window->Height - window->BorderBottom - 1);

					AddGList(window,glist,-1,-1,NULL);
					RefreshGList(glist, window, NULL, -1);
					GT_RefreshWindow(window,NULL);
					RefreshWindowFrame(window);

					if (req->Active >= 0)
					{
						GT_SetGadgetAttrs(req->ListGadget, window, NULL,
							GTLV_Selected,	req->Active,
							TAG_END );
					}

					if (req->Buffer != NULL)
					{
						GT_SetGadgetAttrs(req->StrGadget, window, NULL,
							GTST_String,	req->Buffer,
							TAG_END);
					}
					break;

				case IDCMP_GADGETUP:
					switch ( ((struct Gadget *)imsg.IAddress)->GadgetID ) {

					case OK_ID:
						result = 1;
						goto bye;

					case CANCEL_ID:
						goto bye;

					case LIST_ID:
						req->Active = imsg.Code;
						break;

					case ENTRY_ID:
						break;
					}
					break;

				case IDCMP_MENUPICK:
					if (imsg.Code != MENUNULL)
					{
						switch( ITEMNUM(imsg.Code) ) {

						case ITEM_LAST:
							break;

						case ITEM_NEXT:
							break;

						case ITEM_OK:
							result = 1;
							goto bye;

						case ITEM_CANCEL:
							goto bye;
						}
					}
					break;
				}
			}
		}
	}

bye:

	if (window != NULL)
	{
		req->LeftEdge = window->LeftEdge;
		req->TopEdge = window->TopEdge;
		req->Width = window->Width;
		req->Height = window->Height;

		ClearMenuStrip(window);

		if (req->Window == NULL || (req->Flags & LISTREQF_NEWIDCMP))
			 CloseWindow(window);
		else CloseWindowSafely(window);

		if (req->Buffer != NULL && req->StrGadget != NULL)
		{
			strcpy(req->Buffer,
				((struct StringInfo *)req->StrGadget->SpecialInfo)->Buffer );
		}

		if (glist) FreeGadgets(glist);

		if (menus) FreeMenus(menus);

		if (visinfo) FreeVisualInfo(visinfo);
	}

	return result;
}

void parse_list_tags(struct PrivateListReq *req, struct TagItem *ti)
{	struct TagItem	*tlist = ti;
	ULONG			tdata;

	while (ti = NextTagItem(&tlist))
	{
		tdata = ti->ti_Data;

		switch (ti->ti_Tag) {

		case LISTREQ_Window:
			req->Window = (struct Window *)tdata;
			break;

		case LISTREQ_Screen:
			req->Screen = (struct Screen *)tdata;
			break;

		case LISTREQ_PubScreenName:
		case LISTREQ_PrivateIDCMP:
		case LISTREQ_IntuiMsgFunc:
		case LISTREQ_SleepWindow:
			break;

		case LISTREQ_UserData:
			req->UserData = (APTR)tdata;
			break;

		case LISTREQ_TextAttr:
		case LISTREQ_Locale:
			break;

		case LISTREQ_TitleText:
			req->TitleText = (char *)tdata;
			break;

		case LISTREQ_PositiveText:
			req->PosText = (char *)tdata;
			break;

		case LISTREQ_NegativeText:
			req->NegText = (char *)tdata;
			break;

		case LISTREQ_InitialLeftEdge:
			req->LeftEdge = tdata;
			break;

		case LISTREQ_InitialTopEdge:
			req->TopEdge = tdata;
			break;

		case LISTREQ_InitialWidth:
			req->Width = tdata;
			break;

		case LISTREQ_InitialHeight:
			req->Height = tdata;
			break;

		case LISTREQ_Labels:
			req->List = (struct List *)tdata;
			break;

		case LISTREQ_Selected:
			req->Active = tdata;
			break;

		case LISTREQ_Buffer:
			req->Buffer = (char *)tdata;
			break;

		case LISTREQ_BufferSize:
			req->BufferSize = tdata;
			break;

		case LISTREQ_HookFunc:
			req->HookFunc = (void *)tdata;
			break;
		}
	}
}

LONG ListRequestA(void *req, struct TagItem *ti)
{
	parse_list_tags(req, ti);

	return do_listreq(req);
}

LONG ListRequest( void *req, Tag tag, ... )
{	return ListRequestA( req, (struct TagItem *)&tag );
}

void *AllocListRequestA(struct TagItem *ti)
{	struct PrivateListReq	*req;

	if (req = AllocMem(sizeof *req, MEMF_CLEAR | MEMF_PUBLIC))
	{
		req->LeftEdge = 10;
		req->TopEdge = 10;
		req->Width = 260;
		req->Height = 180;
		req->TitleText = "Select Entry";
		req->PosText = "OK";
		req->NegText = "Cancel";
		req->Active = -1;

		parse_list_tags(req, ti);
	}

	return req;
}

void *AllocListRequest( Tag tag, ... )
{	return AllocListRequestA( (struct TagItem *)&tag );
}

void FreeListRequest(void *req)
{
	if (req) FreeMem(req, sizeof(struct PrivateListReq));
}


struct Gadget *build_listreq_gadgets(struct PrivateListReq *req,
	struct Window *window, struct Gadget **glist, APTR visinfo)
{
	struct NewGadget	ng;
	struct Gadget		*gad;
	WORD				width;

	gad = CreateContext(glist);

	width = window->Width - window->BorderLeft - window->BorderRight;

	ng.ng_VisualInfo = visinfo;
	ng.ng_TextAttr = &topaz_8;		/* fod->sysfont_attr; */
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_Height = 8+3;				/* systemfont->tf_YSize+3; */
	ng.ng_TopEdge = window->Height - window->BorderBottom - ng.ng_Height -
		VERT_SPACING;

		/* ok button */

	ng.ng_GadgetText = req->PosText;
	ng.ng_LeftEdge = window->BorderLeft + HORIZ_SPACING;
	ng.ng_Width = BUTTON_WIDTH;
	ng.ng_GadgetID = OK_ID;

	req->OKGadget = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);

		/* cancel button */

	ng.ng_GadgetText = req->NegText;
	ng.ng_LeftEdge = window->Width - window->BorderRight - HORIZ_SPACING -
		BUTTON_WIDTH;
	ng.ng_GadgetID = CANCEL_ID;

	req->CancelGadget = gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);

		/* file string... */

	if (req->Buffer != NULL && req->BufferSize != 0)
	{
		ng.ng_Flags = PLACETEXT_ABOVE;
		ng.ng_Width = window->Width - window->BorderLeft - window->BorderRight -
			2 * HORIZ_SPACING;
		ng.ng_Height = 8 + 6;
		ng.ng_GadgetText = NULL;
		ng.ng_GadgetID = ENTRY_ID;

		req->StrGadget = gad = CreateGadget(STRING_KIND, gad, &ng,
			GTST_MaxChars,	req->BufferSize,
			GTST_String,	req->Buffer,
			TAG_END);
	}

		/* list */

	ng.ng_Flags = PLACETEXT_ABOVE;
	ng.ng_Width = window->Width - window->BorderLeft - window->BorderRight -
		2 * HORIZ_SPACING;
	ng.ng_Height = window->Height - window->BorderTop - window->BorderBottom -
		3 * VERT_SPACING - (8+3);
	ng.ng_LeftEdge = window->BorderLeft + HORIZ_SPACING;
	ng.ng_TopEdge = window->BorderTop + VERT_SPACING;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = LIST_ID;

	req->ListGadget = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
		GTLV_Labels,		req->List,
		GTLV_ScrollWidth,	16,
		GTLV_ShowSelected,	req->StrGadget,
		TAG_END );

	return gad;
}

void CloseWindowSafely(struct Window *win)
{
    /* we forbid here to keep out of race conditions with Intuition */
    Forbid();

    /* send back any messages for this window
     * that have not yet been processed
     */
    StripIntuiMessages( win->UserPort, win );

    /* clear UserPort so Intuition will not free it */
    win->UserPort = NULL;

    /* tell Intuition to stop sending more messages */
    ModifyIDCMP( win, 0L );

    /* turn multitasking back on */
    Permit();

    /* and really close the window */
    CloseWindow( win );
}

/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * (note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it)
 */

void StripIntuiMessages(struct MsgPort *mp, struct Window *win)
{
    struct IntuiMessage *msg;
    struct Node *succ;

    msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

    while( succ =  msg->ExecMessage.mn_Node.ln_Succ ) {

		if( msg->IDCMPWindow ==  win ) {

		    /* Intuition is about to free this message.
		     * Make sure that we have politely sent it back.
		     */
		    Remove( &msg->ExecMessage.mn_Node );

		    ReplyMsg( &msg->ExecMessage );
		}

		msg = (struct IntuiMessage *) succ;
	}
}
