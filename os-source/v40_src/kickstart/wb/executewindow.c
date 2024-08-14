/*
 * $Id: executewindow.c,v 39.1 93/01/11 16:52:17 mks Exp $
 *
 * $Log:	executewindow.c,v $
 * Revision 39.1  93/01/11  16:52:17  mks
 * Now supports a string filter for the RENAME requester...
 * No longer needs to check for ":" or "/" after the fact.
 * 
 * Revision 38.4  92/11/12  09:09:53  mks
 * Cleaned up gadget size to match prefs editors
 *
 * Revision 38.3  92/11/11  11:13:40  mks
 * Completely new requester that gives font sensitive
 * layout.  Required for the Japan support.
 *
 * Revision 38.2  92/05/30  16:55:40  mks
 * Now uses the GETMSG/PUTMSG stubs
 *
 * Revision 38.1  91/06/24  11:35:23  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <clib/gadtools_protos.h>

#include "support.h"
#include "quotes.h"

#include "sysproto.h"
#include "proto.h"

/*------------------------------------------------------------------------*/

/*  GadgetID definitions: */
#define G_COMMAND	1
#define G_OK		2
#define G_CANCEL	3

/*------------------------------------------------------------------------*/

/*  Size of buffer for command string gadget: */
#define COMMANDLENGTH		256

/*------------------------------------------------------------------------*/

#define	WIN_Title	WinTags[0].ti_Data
#define	WIN_Screen	WinTags[1].ti_Data
#define	WIN_Height	WinTags[2].ti_Data
#define	WIN_Width	WinTags[3].ti_Data
#define	WIN_Gadgets	WinTags[4].ti_Data
#define	WIN_IDCMP	WinTags[5].ti_Data

#define	ewin_IDCMP	IDCMP_GADGETUP|IDCMP_ACTIVEWINDOW

#define	BETWEEN	4

WORD SyncExecute(STRPTR title,STRPTR greeting,STRPTR label,STRPTR buffer,struct MsgPort *intuiport,struct Hook *hook)
{
struct	WorkbenchBase	*wb = getWbBase();
	VOID		*VisualInfo;
struct	Window		*ewin;
struct	IntuiMessage	*imsg;
struct	Screen		*Screen;
struct	Gadget		*gad;
struct	Gadget		*stringGad;
struct	TagItem		WinTags[]=
	{
		{WA_Title,NULL},
		{WA_CustomScreen,NULL},
		{WA_Height,0},
		{WA_Width,400},
		{WA_Gadgets,NULL},
		{WA_IDCMP,0},
		{WA_Flags,WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_SIMPLE_REFRESH|WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_NOCAREREFRESH},
		{TAG_DONE,0}
	};
	WORD		exit_code=0;


	BeginDiskIO();

	Screen=wb->wb_Screen;
	WIN_Title=(ULONG)title;
	WIN_Screen=(ULONG)Screen;
	WIN_Height=Screen->WBorTop + Screen->BarHeight - Screen->BarVBorder + Screen->WBorTop + BETWEEN;

	if (VisualInfo=GetVisualInfo(Screen,TAG_DONE))
	{
	LONG	label_width;
	LONG	ok_width;
	LONG	cancel_width;
	LONG	greeting_width;

		/* Find the sizes of the various strings */
		{
		struct	IntuiText	itext;

			itext.LeftEdge=0;
			itext.ITextFont=Screen->Font;
			itext.IText=label;
			label_width=IntuiTextLength(&itext);
			itext.IText=Quote(Q_OK_TEXT);
			ok_width=IntuiTextLength(&itext) + 12;
			itext.IText=Quote(Q_CANCEL_TEXT);
			cancel_width=IntuiTextLength(&itext) + 12;
			itext.IText=greeting;
			greeting_width=IntuiTextLength(&itext);
		}

		if (gad=CreateContext((struct Gadget **)&WIN_Gadgets))
		{
		struct	NewGadget	ng;

			ng.ng_TextAttr = Screen->Font;
			ng.ng_LeftEdge = Screen->WBorLeft << 1;
			ng.ng_TopEdge = WIN_Height;
			greeting_width -= (ng.ng_Width = WIN_Width - ng.ng_LeftEdge - (Screen->WBorRight<<1));
			if (greeting_width > 0)
			{
				ng.ng_Width+=greeting_width;
				WIN_Width+=greeting_width;
			}
			WIN_Height += (ng.ng_Height = Screen->Font->ta_YSize) + BETWEEN;
			ng.ng_GadgetText = greeting;
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
						GTST_EditHook,hook,
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
			ng.ng_GadgetText = Quote(Q_OK_TEXT);
			ng.ng_Flags = PLACETEXT_IN;
			gad = CreateGadget(BUTTON_KIND,gad,&ng,TAG_DONE);

			/*  Create a "CANCEL" button relative to the bottom and right: */
			ng.ng_GadgetID = G_CANCEL;
			ng.ng_LeftEdge = WIN_Width-(Screen->WBorRight << 1)-ok_width;
			ng.ng_GadgetText = Quote(Q_CANCEL_TEXT);
			gad = CreateGadget(BUTTON_KIND,gad,&ng,TAG_DONE);

			WIN_Height += Screen->WBorBottom;

			if (gad)
			{
				if (ewin=OpenWindowTagList(NULL,WinTags))
				{
					ewin->UserPort=intuiport;
					if (ModifyIDCMP(ewin,ewin_IDCMP))
					{
						GT_RefreshWindow(ewin,NULL);
						ActivateGadget(stringGad,ewin,NULL);
						while (!exit_code)
						{
							if (imsg=(struct IntuiMessage *)GETMSG(intuiport))
							{
								if (imsg->IDCMPWindow==ewin)
								{
								struct	IntuiMessage	*imsg2;

									if (imsg2=GT_FilterIMsg(imsg))
									{
										if (IDCMP_ACTIVEWINDOW == imsg2->Class) ActivateGadget(stringGad,ewin,NULL);
										else if (IDCMP_GADGETUP == imsg2->Class)
										{
											switch (((struct Gadget *) imsg2->IAddress)->GadgetID & GADTOOLMASK)
											{
											case G_OK:
											case G_COMMAND:	exit_code=1;
													stccpy(buffer,((struct StringInfo *)stringGad->SpecialInfo)->Buffer,COMMANDLENGTH);
													break;
											case G_CANCEL:	exit_code=-1;
													break;
											}
										}
										GT_PostFilterIMsg(imsg2);
									}
									REPLYMSG((struct Message *)imsg);
								}
								else StripMessage(imsg);
							}
							else WAITPORT(intuiport);

						}
					}
					ewin->Title=NULL;
					ClosePWindow(ewin,NULL);
				}
			}

			FreeGadgets((struct Gadget *)WIN_Gadgets);
		}
		FreeVisualInfo(VisualInfo);
	}

	EndDiskIO();
	return(exit_code);
}
