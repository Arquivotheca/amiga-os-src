/*
 * $Id: infowindow.c,v 39.7 93/02/15 14:46:04 mks Exp $
 *
 * $Log:	infowindow.c,v $
 * Revision 39.7  93/02/15  14:46:04  mks
 * Just changed a bit to use the global strings
 * 
 * Revision 39.6  93/02/10  12:08:42  mks
 * Added two more fallbacks and made the number displays localized
 *
 * Revision 39.5  93/02/04  11:59:07  mks
 * Completely rewrote and redesigned the creation of the
 * information window.  It is now completely font sensitive and
 * has full fall-back capability in the case where the screen font
 * is too large to use.  In addition, a more complex text length
 * control now gives localization strings more ability to change.
 * Also included is a faster/simpler refresh mechanism.
 * All of this and yet the code is smaller.
 *
 * Revision 39.4  92/11/12  08:03:06  mks
 * Removed constant that is the SAVE/CANCEL size from this file
 * and moved it to the gadget create function
 *
 * Revision 39.3  92/05/31  16:31:51  mks
 * Now locks the screen as needed and does a few ROM Space tricks...
 *
 * Revision 39.2  92/05/31  02:09:39  mks
 * Had to move the allocation of the FIB back out of the getting
 * of the LOCK routine...
 *
 * Revision 39.1  92/05/31  01:52:16  mks
 * Now is the ASYNC, LVO based INFO...
 *
 * Revision 38.7  92/04/24  18:32:39  mks
 * Some more minor changes
 *
 * Revision 38.6  92/04/24  16:14:26  mks
 * One more minor adjustment
 *
 * Revision 38.5  92/04/24  16:14:00  mks
 * Final adjustment of the list gadget size values
 *
 * Revision 38.4  92/04/24  12:45:25  mks
 * Now does 3/5/7 line tooltypes based on available screen space
 *
 * Revision 38.3  92/04/24  10:51:25  mks
 * Changed to use only tags and no NewWindow structure
 * Changed to use -1,-1 as top-left for ZoomBox
 *
 * Revision 38.2  92/03/10  11:34:05  mks
 * Added the generation of the path name for the file
 *
 * Revision 38.1  91/06/24  11:36:36  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

/*------------------------------------------------------------------------*/

#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/dos.h>
#include <graphics/regions.h>

#include "info.h"
#include "support.h"
#include "quotes.h"

#include "global.h"
#include "macros.h"
#include "sysproto.h"
#include "proto.h"

extern	char	NumbersString[];
extern	char	MM_String[];
extern	char	M_String[];

/*------------------------------------------------------------------------*/

void RenderAll(struct InfoTag *itag,BOOL firsttime)
{
	LONG		left;
	LONG		top;
	LONG		width;
	LONG		height;
struct	Region		*myregion;
struct	Rectangle	rect;

	LockLayerInfo(&itag->it_Window->WScreen->LayerInfo);

	rect=itag->it_IconRect;
	rect.MaxX-=4;
	rect.MaxY-=2;

	DrawBevelBox(itag->it_Window->RPort,
			itag->it_IconRect.MinX,
			 itag->it_IconRect.MinY,
			width=itag->it_IconRect.MaxX-itag->it_IconRect.MinX,
			 height=itag->it_IconRect.MaxY-itag->it_IconRect.MinY,
			GTBB_Recessed, TRUE,
        		GT_VisualInfo, itag->it_VisualInfo,
			TAG_DONE);

	left=(width - itag->it_Obj->do_Gadget.Width) >> 1;
	if (left<4) left=4;
	left+=rect.MinX;

	top=(height - itag->it_Obj->do_Gadget.Height) >> 1;
	if (top<2) top=2;
	top+=rect.MinY;

	if (myregion=NewRegion())
	{
		if (OrRectRegion(myregion, &rect))
		{
			myregion=InstallClipRegion(itag->it_Window->RPort->Layer,myregion);

			if (!firsttime) BeginRefresh(itag->it_Window);

			DrawImage(itag->it_Window->RPort,(struct Image *)itag->it_Obj->do_Gadget.GadgetRender,left,top);

			if (!firsttime) EndRefresh(itag->it_Window,FALSE);

			myregion=InstallClipRegion(itag->it_Window->RPort->Layer,myregion);
		}
		DisposeRegion(myregion);
	}

	if (!firsttime)	/* Do the gadtools refresh work... */
	{
		GT_BeginRefresh(itag->it_Window);
		GT_EndRefresh(itag->it_Window, TRUE);
	}

	UnlockLayerInfo(&itag->it_Window->WScreen->LayerInfo);
}

/*------------------------------------------------------------------------*/

long PixelWidth(char *text,struct TextAttr *myFont)
{
struct	IntuiText	myText;

	myText.LeftEdge=0;
	myText.TopEdge=0;
	myText.ITextFont=myFont;
	myText.IText=text;
	myText.NextText=NULL;
	return(IntuiTextLength(&myText));
}

long MaxPixelWidth(long Low,long High,struct TextAttr *myFont)
{
LONG	result=0;
LONG	temp;

	while (Low<=High)
	{
		temp=PixelWidth(Quote(Low),myFont);
		if (temp>result) result=temp;

		Low++;
	}
	return(result+PixelWidth(M_String,myFont));
}

/*
 * Close up whatever may have allocated...
 */
void CloseInfoWindow(struct InfoTag *itag)
{
void	*node;

	if (itag)
	{
        	if (itag->it_Window) CloseWindow(itag->it_Window);

		/*  FreeGadgets() checks for the NULL case */
		FreeGadgets(itag->it_FirstGadget);

		if (itag->it_InfoFont) CloseFont(itag->it_InfoFont);

		FREEVEC(itag->it_FIB);
		FREEVEC(itag->it_InfoData);
		FREEVEC(itag->it_NameInfo);

		while (node=RemHead(&itag->it_ToolTypeList)) FREEVEC(node);

		FreeVisualInfo(itag->it_VisualInfo);

		if (itag->it_Obj) FreeDiskObject(itag->it_Obj);

		FREEVEC(itag);
	}
}

struct Gadget *CreateNum(struct Gadget *gad,struct NewGadget *ng,BOOL condition,LONG number,LONG q_id)
{
	ng->ng_GadgetText=Quote(q_id);
	ng->ng_TopEdge+=ng->ng_Height+2;

	if (condition) gad=CreateGadget(NUMBER_KIND,gad,ng,GTNM_Number,number,GTNM_Format,"%lU",TAG_DONE);
	else gad=CreateGadget(TEXT_KIND,gad,ng,GTTX_Text,"---",TAG_DONE);

	return(gad);
}

struct Gadget *Do_Next_Pro(struct InfoTag *itag,struct Gadget *gad, struct NewGadget *ng, ULONG bit, ULONG q_id)
{
	ng->ng_GadgetID++;
	ng->ng_TopEdge += ng->ng_Height;
	ng->ng_GadgetText = Quote(q_id);
	ng->ng_UserData = (APTR) bit;
	return(CreateGadget(CHECKBOX_KIND,gad,ng,
				GTCB_Checked,(BOOL)((itag->it_Protection ^ 15) & bit),
				GTCB_Scaled,TRUE,
				TAG_DONE) );
}

/* Routine to get various icon information/data */
long GetIconInfo(struct InfoTag *itag)
{
struct	DiskObject	*dobj;
	BPTR		parentlock;

	if (!(dobj=itag->it_Obj)) return(FALSE);

	itag->it_MaskType = 1L << (dobj->do_Type);

	CURRENTDIR(parentlock=CURRENTDIR(NULL));
	if (!NAMEFROMLOCK(parentlock,itag->it_Path,255)) strcpy(itag->it_Path,Quote(Q_INFO_TITLE));

	/* Get tooltypes... */
	if (dobj->do_ToolTypes)
	{
	LONG	i;

		for (i = 0; dobj->do_ToolTypes[i]; i++)
		{
		struct	Node	*node;

			if (!(node = AllocNode(dobj->do_ToolTypes[i]))) return(FALSE);
			ADDTAIL(&itag->it_ToolTypeList, node);
		}
		itag->it_AfterNode = TailNode(&itag->it_ToolTypeList);
	}

	/* Get FIB if needed... */
	if (itag->it_MaskType & MASK_FIB)
	{
	BPTR	lock;

		if (!(itag->it_FIB = ALLOCVEC(sizeof(struct FileInfoBlock), MEMF_PUBLIC))) return(FALSE);

		if (lock=LOCK(itag->it_Name,ACCESS_READ))
		{
			if (EXAMINE(lock, itag->it_FIB))
			{
				itag->it_Flags |= IT_GOT_FIB;
				itag->it_Protection=itag->it_FIB->fib_Protection;
			}
			UNLOCK(lock);
		}
	}

	/* If info is needed... */
	if (itag->it_MaskType & MASK_INFODATA)
	{
		if (!(itag->it_InfoData = ALLOCVEC(sizeof(struct InfoData), MEMF_CLEAR|MEMF_PUBLIC))) return(FALSE);

		if (Info(parentlock, itag->it_InfoData))
		{
			itag->it_Flags |= IT_GOT_INFODATA;
			if (itag->it_InfoData->id_VolumeNode)
			{
			/*  Copy datestamp from VolumeNode to itag: */
				memcpy(	(APTR)&itag->it_CreateDate,
				(APTR) &(((struct DeviceList *)(BADDR(itag->it_InfoData->id_VolumeNode)))->dl_VolumeDate),
				sizeof(struct DateStamp));

				itag->it_Flags |= IT_GOT_VOLNODE;
			}
		}
	}

	/* If we get here, all is well... */
	return(TRUE);
}

/*
 * Note:  For ROM Space reasons, the Screen->WBorXXX variables are
 * commented out and the constants are used...  For example,
 * if Screen->WBorTop were to be used it would look like this:
 */	//	/*Screen->WBorTop*/2

/* Maximum fallback try...  Currently 4 different fonts are tried */
#define	MAX_FALLBACK	4

struct Window *OpenInfoWindow(char *Name,struct Screen *Screen)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	Window		*infoWindow=NULL;
struct	InfoTag		*itag;
	LONG		TryCount=0;

	while ((MAX_FALLBACK>TryCount++) && (infoWindow==NULL)) if (itag=ALLOCVEC(sizeof(struct InfoTag),MEMF_CLEAR|MEMF_PUBLIC))
	{
	struct	DiskObject	*dobj;
	struct	TextAttr	*myText;

		/*
		 * Pick the font to use, first time through, use the
		 * screen font, next time fall back to the icon font,
		 * then to the system font, finally to topaz-8
		 */
		myText=&defaultAttr;	/* If no others */
		switch(TryCount)
		{
			case 1:	myText=Screen->Font;	break;
			case 2:	myText=wb->wb_IconAttr;	break;
			case 3:	myText=wb->wb_TextAttr;	break;

		// ****	case MAX_FALLBACK: myText=&defaultAttr;	break;
		}

		NewList(&itag->it_ToolTypeList);

		itag->it_VisualInfo=GetVisualInfo(Screen,TAG_DONE);
		itag->it_InfoFont=OpenFont(myText);
		itag->it_Name=Name;
		itag->it_NameInfo=ALLOCVEC(strlen(Name)+6,MEMF_PUBLIC);
		strcpy(itag->it_NameInfo,Name);
		strcat(itag->it_NameInfo,InfoSuf);

		/* Try to get the icon... */
		if (!(dobj=GetDiskObject(Name)))
		{
			dobj=GetDiskObjectNew(Name);
			itag->it_ObjFake=TRUE;
		}
		itag->it_Obj=dobj;

		/* Now check if all is well... */
		if (itag->it_NameInfo	&&
		    itag->it_VisualInfo	&&
		    itag->it_InfoFont	&&
		    itag->it_Obj	&&
		    GetIconInfo(itag))
		{
		struct	Gadget		*gad;
			LONG		LeftEdge;
			LONG		TopEdge;
			LONG		numGad;
			LONG		numSpc;
			LONG		protGad;
			LONG		protBox;
			LONG		lowGad;
			LONG		winWidth;
			LONG		winHeight;
			LONG		iconBox;
			LONG		tmp;
			char		*dsText;
		struct	DateStamp	*ds=NULL;
			WORD		ZoomSize[4]={-1,-1,240,11};
		struct	NewGadget	ng;
			char		dsBuffer[64];

			stccpy(itag->it_Title,((itag->it_MaskType & MASK_HASNAME) ? itag->it_Name : itag->it_Path),32);
			strcat(itag->it_Title," (");
			strcat(itag->it_Title,Quote((UWORD)(itag->it_Obj->do_Type + Q_I_ICONTYPE_BASE - 1)));
			strcat(itag->it_Title,")");

			if (itag->it_MaskType & MASK_LASTCHANGED)
			{
				dsText=Quote(Q_LAST_CHANGED);
				if (itag->it_Flags & IT_GOT_FIB) ds=&itag->it_FIB->fib_Date;
			}
			else
			{
				dsText=Quote(Q_CREATED);
				if (itag->it_Flags & IT_GOT_VOLNODE) ds=&itag->it_CreateDate;
			}
			datetostring(dsBuffer,ds);

			TopEdge=/*Screen->WBorTop*/2 + Screen->BarHeight - Screen->BarVBorder;
			ZoomSize[3]=TopEdge;	/* Set ZOOM size */
			TopEdge+=1+/*Screen->WBorTop*/2;
			LeftEdge=(/*Screen->WBorLeft*/4) << 1;

			numGad=MaxPixelWidth(Q_STACK,Q_BYTES,myText);
			numSpc=PixelWidth(NumbersString,myText);
			protBox=PixelWidth(MM_String,myText)+10;

			protGad=MaxPixelWidth(Q_SCRIPT_BIT,Q_EXECUTE_BIT,myText)+4;
			tmp=MaxPixelWidth(Q_VOLUME_TEXT,Q_VOL_IS_UNKNOWN,myText);
			if (tmp>protGad) protGad=tmp;

			lowGad=MaxPixelWidth(Q_LAST_CHANGED,Q_TOOL_TYPES,myText);
			tmp=((MaxPixelWidth(Q_NEW,Q_DELETE,myText)+4)<<1)+6;
			if (tmp>lowGad) lowGad=tmp;

			/*
			 * Adjust Icon box if date string ends up
			 * being too wide...
			 */
			iconBox=127;	/* Default size */
			itag->it_IconRect.MinX=LeftEdge+numGad+numSpc+2;
			tmp=(lowGad+PixelWidth(dsBuffer,myText))-(iconBox+itag->it_IconRect.MinX);
			if (tmp>0) iconBox+=tmp;
			itag->it_IconRect.MaxX=iconBox+itag->it_IconRect.MinX;
			itag->it_IconRect.MaxY=13+(myText->ta_YSize<<2)+(itag->it_IconRect.MinY=TopEdge+myText->ta_YSize+3);

			/* Inner width of window without right_edge*2 */
			winWidth=itag->it_IconRect.MaxX+protGad+protBox;

			gad=CreateContext(&itag->it_FirstGadget);
			ng.ng_TextAttr=myText;

			/*
			 * Build the title gadget
			 * Remember to adjust for the possible
			 * longer-than-iconBox title...
			 */
			tmp=PixelWidth(itag->it_Title,myText)-iconBox;
			if (tmp>0) iconBox-=tmp;
			ng.ng_LeftEdge=itag->it_IconRect.MinX+(iconBox>>1);
			ng.ng_TopEdge=TopEdge+myText->ta_YSize+4;
			ng.ng_Width=0;
			ng.ng_Height=myText->ta_YSize;
			ng.ng_VisualInfo=itag->it_VisualInfo;
			ng.ng_Flags=PLACETEXT_ABOVE;
			ng.ng_GadgetText = itag->it_Title;
			gad=CreateGadget(TEXT_KIND,gad,&ng,TAG_DONE);

			/*
			 * File protection bits...
			 */
			if (itag->it_Flags & IT_GOT_FIB)
			{
				ng.ng_GadgetID = G_SCRIPT-1;
				ng.ng_LeftEdge=itag->it_IconRect.MaxX+protGad;
				ng.ng_Height+=3;	/* Check box size */
				ng.ng_Width=protBox;
				ng.ng_TopEdge=TopEdge-ng.ng_Height+2;
				ng.ng_Flags=NG_HIGHLABEL;

				gad=Do_Next_Pro(itag,gad,&ng,FIBF_SCRIPT,Q_SCRIPT_BIT);
				gad=Do_Next_Pro(itag,gad,&ng,FIBF_ARCHIVE,Q_ARCHIVED_BIT);
				gad=Do_Next_Pro(itag,gad,&ng,FIBF_READ,Q_READ_BIT);
				gad=Do_Next_Pro(itag,gad,&ng,FIBF_WRITE,Q_WRITE_BIT);
				gad=Do_Next_Pro(itag,gad,&ng,FIBF_EXECUTE,Q_EXECUTE_BIT);
				gad=Do_Next_Pro(itag,gad,&ng,FIBF_DELETE,Q_DELETE_BIT);

				ng.ng_Height-=3;	/* Undo it... */
			}
			else if (itag->it_MaskType & MASK_DISKSTATUS)
			{
				ng.ng_LeftEdge=itag->it_IconRect.MaxX+((protGad+protBox)>>1);
				ng.ng_TopEdge=itag->it_IconRect.MinY+ng.ng_Height+ng.ng_Height;
				ng.ng_GadgetText=Quote(Q_VOLUME_TEXT);
				gad=CreateGadget(TEXT_KIND,gad,&ng,TAG_DONE);
				ng.ng_TopEdge+=ng.ng_Height+2;

				tmp=Q_VOL_IS_UNKNOWN;
				if (itag->it_Flags & IT_GOT_INFODATA)
				{
					switch (itag->it_InfoData->id_DiskState)
					{
					case ID_WRITE_PROTECTED:
						tmp=Q_VOL_IS_READ_ONLY;
						break;
					case ID_VALIDATING:
						tmp=Q_VOL_IS_VALIDATING;
						break;
					case ID_VALIDATED:
						tmp=Q_VOL_IS_READ_WRITE;
						break;
					}
				}
				ng.ng_GadgetText=Quote(tmp);
				gad=CreateGadget(TEXT_KIND,gad,&ng,TAG_DONE);
			}

			/*
			 * General setup for the info gadgets...
			 */
			ng.ng_LeftEdge=numGad+LeftEdge;
			ng.ng_TopEdge=TopEdge+4;
			ng.ng_Flags=NG_HIGHLABEL;

			/*
			 * Disk information display
			 */
			if (itag->it_MaskType & MASK_DISK)
			{
				gad=CreateNum(gad,&ng, (BOOL)(itag->it_Flags & IT_GOT_INFODATA), itag->it_InfoData->id_NumBlocks,Q_BLOCKS);
				gad=CreateNum(gad,&ng, (BOOL)(itag->it_Flags & IT_GOT_INFODATA), itag->it_InfoData->id_NumBlocksUsed,Q_USED);
				gad=CreateNum(gad,&ng, (BOOL)(itag->it_Flags & IT_GOT_INFODATA), itag->it_InfoData->id_NumBlocks - itag->it_InfoData->id_NumBlocksUsed,Q_FREE);
				gad=CreateNum(gad,&ng, (BOOL)(itag->it_Flags & IT_GOT_INFODATA), itag->it_InfoData->id_BytesPerBlock,Q_BLOCK_SIZE);
			}

			/*
			 * File size (and stack) display
			 */
			if (itag->it_MaskType & MASK_SIZESTACK)
			{
			LONG	stacksize;

				gad=CreateNum(gad,&ng,(BOOL)(itag->it_Flags & IT_GOT_FIB),itag->it_FIB->fib_NumBlocks,Q_BLOCKS);
				gad=CreateNum(gad,&ng,(BOOL)(itag->it_Flags & IT_GOT_FIB),itag->it_FIB->fib_Size,Q_BYTES);

				ng.ng_GadgetID=G_STACK;
				ng.ng_TopEdge+=(ng.ng_Height=myText->ta_YSize+6);
				ng.ng_Width=numSpc-8;
				ng.ng_GadgetText=Quote(Q_STACK);

				if (!(stacksize=itag->it_Obj->do_StackSize)) stacksize=DEFAULT_STACKSIZE;

				if (!(itag->it_ObjFake)) itag->it_Stack=gad=CreateGadget(INTEGER_KIND,gad,&ng,GTIN_Number,stacksize,GTIN_MaxChars,NUMSTRINGLENGTH,TAG_DONE);
			}

			/*
			 * Set new top edge... Work down from here...
			 * Set the left edge (we use it for a while)
			 */
			ng.ng_TopEdge=itag->it_IconRect.MaxY+3;
			ng.ng_LeftEdge=lowGad+LeftEdge;

			/*
			 * If we have a date to display
			 */
			if (itag->it_MaskType & (MASK_LASTCHANGED | MASK_CREATEDDATE))
			{
				/*  Create a display-only "gadget" with the date: */
				ng.ng_Width=itag->it_IconRect.MaxX-ng.ng_LeftEdge;

				ng.ng_Height=myText->ta_YSize;
				ng.ng_Flags=NG_HIGHLABEL;
				ng.ng_GadgetText=dsText;
				gad = CreateGadget(TEXT_KIND,gad,&ng,
							GTTX_Text,dsBuffer,
							GTTX_CopyText,TRUE,
							TAG_DONE);

				ng.ng_TopEdge+=ng.ng_Height+4;
			}

			/*
			 * Set the general string gadget width/height here...
			 * We use it for a while below...
			 */
			ng.ng_Width=winWidth-ng.ng_LeftEdge;
			ng.ng_Height=6+myText->ta_YSize;

			/* Common code...  Fold out to here... */
			ng.ng_Flags=NG_HIGHLABEL;

			/*
			 * If there is a valid file/comment we will
			 * build the comment field
			 */
			if ((itag->it_MaskType & MASK_COMMENT) && (itag->it_Flags & IT_GOT_FIB))
			{
				ng.ng_GadgetID=G_COMMENT;
				ng.ng_GadgetText=Quote(Q_COMMENT);
				gad=CreateGadget(STRING_KIND, gad, &ng,
					GTST_String,itag->it_FIB->fib_Comment,
					GTST_MaxChars,COMMENTLENGTH-1,
					TAG_DONE);
				ng.ng_TopEdge+=ng.ng_Height+2;
				itag->it_Comment=gad;
			}

			/*
			 * Only show these gadgets if the icon is real...
			 */
			if (!(itag->it_ObjFake))
			{

				/*
				 * Default tool if needed...
				 */
				if (itag->it_MaskType & MASK_DEFAULTTOOL)
				{
					ng.ng_GadgetID=G_DEFAULTTOOL;
					ng.ng_GadgetText=Quote(Q_DEFAULT_TOOL);
					gad=CreateGadget(STRING_KIND, gad, &ng,
						GTST_String, itag->it_Obj->do_DefaultTool,
						GTST_MaxChars, DEFAULTTOOLLENGTH,
						TAG_DONE);
					ng.ng_TopEdge+=ng.ng_Height+2;
					itag->it_DefaultTool = gad;
				}


				/*
				 * Tooltypes as needed...
				 */
				if (itag->it_MaskType & MASK_TOOLTYPES)
				{
				LONG	tmp1;

					/*
					 * Some fancy footwork to figure
					 * out how much text will fit into
					 * a listview on the display and
					 * still look good.
					 */
					tmp1=myText->ta_YSize+1;
					ng.ng_Height=4+3*tmp1;
					tmp1+=tmp1;

					tmp=Screen->Height-/*Screen->WBorBottom*/2-TopEdge-tmp1-tmp1-ng.ng_TopEdge;
			/* 5 lines */	if (tmp>ng.ng_Height) ng.ng_Height+=tmp1;
			/* 7 lines */	if (tmp>ng.ng_Height) ng.ng_Height+=tmp1;

					/* Build the listview */
					ng.ng_GadgetText=Quote(Q_TOOL_TYPES);
					ng.ng_GadgetID=G_TTLIST;
					ng.ng_Flags=NG_HIGHLABEL | PLACETEXT_LEFT;

					gad=CreateGadget(LISTVIEW_KIND, gad, &ng,
						GTLV_Labels, &itag->it_ToolTypeList,
						GTLV_ScrollWidth, 18,
						LAYOUTA_SPACING, 1,
						TAG_DONE);
					itag->it_TTList=gad;
					ng.ng_TopEdge += ng.ng_Height;

					/* Build the string gadget */
					ng.ng_Height=6+myText->ta_YSize;
					ng.ng_GadgetID=G_TTSTRING;
					ng.ng_GadgetText=NULL;

					gad=CreateGadget(STRING_KIND,gad,&ng,
						GTST_MaxChars,TOOLTYPESLENGTH,
						GA_Disabled,TRUE,
						TAG_DONE);
					itag->it_TTString=gad;

					/*
					 * Now build the NEW/DEL buttons
					 */
					ng.ng_GadgetID=G_TTNEW;
					ng.ng_LeftEdge=LeftEdge;
					ng.ng_Width=(lowGad-6)/2;
					ng.ng_GadgetText=Quote(Q_NEW);
					ng.ng_Flags=0;
					gad=CreateGadget(BUTTON_KIND,gad,&ng,
						TAG_DONE);

					ng.ng_GadgetID=G_TTDEL;
					ng.ng_LeftEdge+=ng.ng_Width+2;
					ng.ng_GadgetText=Quote(Q_DELETE);
					gad=CreateGadget(BUTTON_KIND,gad,&ng,
						GA_Disabled, TRUE,
						TAG_DONE);
					itag->it_TTDel=gad;
					ng.ng_TopEdge+=ng.ng_Height+2;
				}
			}

			/*
			 * Now build the Save/Cancel gadgets
			 */
			lowGad=PixelWidth(Quote(Q_CANCEL_TEXT),myText);
			tmp=PixelWidth(Quote(Q_SAVE_TEXT),myText);
			if (tmp>lowGad) lowGad=tmp;

			lowGad+=12;

			ng.ng_GadgetID=G_SAVE;
			ng.ng_GadgetText=Quote(Q_SAVE_TEXT);
			ng.ng_LeftEdge=LeftEdge;
			ng.ng_TopEdge+=2;	/* Extra space at bottom */
			ng.ng_Width=lowGad;
			ng.ng_Flags=0;
			gad=CreateGadget(BUTTON_KIND, gad, &ng, TAG_DONE);

			ng.ng_GadgetID=G_QUIT;
			ng.ng_GadgetText=Quote(Q_CANCEL_TEXT);
			ng.ng_LeftEdge=winWidth-ng.ng_Width;
			gad=CreateGadget(BUTTON_KIND,gad,&ng,TAG_DONE);

			winHeight=ng.ng_TopEdge+ng.ng_Height+2+((/*Screen->WBorBottom*/2)<<1);
			winWidth+=/*Screen->WBorRight*/4<<1;

			if ((Screen->Height >= winHeight) &&
			    (Screen->Width  >= winWidth) &&
			    (gad))
			{
				infoWindow=OpenWindowTags(NULL,
							WA_Width,winWidth,
							WA_Height,winHeight,
							WA_PubScreen,Screen,
							WA_Title,itag->it_Path,
							WA_Zoom,ZoomSize,
							WA_IDCMP,INFOW_IDCMPFLAGS,
							WA_AutoAdjust,TRUE,
							WA_Flags,INFOW_FLAGS,
							WA_Gadgets,itag->it_FirstGadget,
							TAG_DONE);
			}
		}

		if (itag->it_Window=infoWindow)
		{
			infoWindow->UserData=(BYTE *)itag;
			RenderAll(itag,TRUE);
			GT_RefreshWindow(infoWindow,NULL);
		}
		else CloseInfoWindow(itag);
	}
	return(infoWindow);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*
******* workbench.library/WBInfo **********************************************
*
*   NAME
*	WBInfo - Bring up the Information requrester                     (V39)
*
*   SYNOPSIS
*	worked = WBInfo(lock, name, screen)
*	d0              a0    a1    a2
*
*	ULONG WBInfo(BPTR, STRPTR, struct Screen *);
*
*   FUNCTION
*	This is the LVO that Workbench calls to bring up the Icon Information
*	requester.  External applications may also call this requester.
*	In addition, if someone were to wish to replace this requester
*	with another one, they could do so via a SetFunction.
*
*   INPUTS
*	lock   - A lock on the parent directory
*	name   - The name of the icon contained within above directory
*	screen - A screen pointer on which the requester is to show up
*
*   RESULTS
*	worked - Returns TRUE if the requester came up, FALSE if it did not.
*
*   NOTE
*	Note that this LVO may be called many times by different tasks
*	before other calls return.  Thus, the code must be 100% re-entrant.
*
*   SEE ALSO
*	icon.library
*
*   BUGS
*
******************************************************************************
*/
int WBInformation(BPTR DirLock, char *Name, struct Screen *Screen)
{
struct	WorkbenchBase	*wb=getWbBase();
struct	Window		*infowindow;
struct	IntuiMessage	*imsg;
struct	InfoTag		*itag;
struct	Process		*proc;
	APTR		oldWindow;
	int		terminated=FALSE;

	if ((wb->wb_WorkbenchStarted) && !(wb->wb_Quit) && (Name) && (Screen))
	{
		proc=(struct Process *)ThisTask();
		DirLock=CURRENTDIR(DirLock);

		if (infowindow = OpenInfoWindow(Name,Screen))
		{
			oldWindow=proc->pr_WindowPtr;
			proc->pr_WindowPtr=infowindow;

			itag = (struct InfoTag *) infowindow->UserData;

			while (!terminated)
			{
				imsg=GT_GetIMsg(infowindow->UserPort);
				if (imsg)
				{
					switch (imsg->Class)
					{
					case REFRESHWINDOW:
						RenderAll(itag, FALSE);
						break;
					case CLOSEWINDOW:
						terminated=TRUE;
						break;
					case GADGETUP:
						terminated = HandleGadgetUp(infowindow,(struct Gadget *) imsg->IAddress,imsg->Code);
						break;
					}

					GT_ReplyIMsg(imsg);
				}
				else WAITPORT(infowindow->UserPort);
			}

			proc->pr_WindowPtr=oldWindow;
			CloseInfoWindow(itag);
		}
		CURRENTDIR(DirLock);
	}
	return(terminated);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*
 ******************************************************************************
 *
 * The fancy way to do ASync Info...
 *
 * Info is started in Sync mode but it becomes ASync
 *
 ******************************************************************************
 */
struct StartMSG
{
struct	Message	msg;
	BPTR	DirLock;
	char	*Name;
};

extern	void	*InfoSegment;

/*
 * This is where Workbench calls to startup Information...
 * This call will now start a new process which will do all of
 * the work...
 * (That is, Information runs on its own process...)
 */
int SyncInfo(struct WBObject *wbobj)
{
struct	MsgPort		*port=&(((struct Process *)ThisTask())->pr_MsgPort);
struct	MsgPort		*dest;
struct	StartMSG	Start;
	BPTR		Lock;
	char		*Name=DiskIconName;
	int		Failed=TRUE;

	if (CheckForType(wbobj,WBF_DISK|WBF_DRAWER|WBF_TOOL|WBF_PROJECT|WBF_GARBAGE))
	{
		if (wbobj->wo_Type != WBDISK) Name=wbobj->wo_Name;
		Lock=GetParentsLock(wbobj);

		if (Lock)
		{
			Start.DirLock=DUPLOCK(Lock);
			Start.Name=scopy(Name);
			Start.msg.mn_ReplyPort=port;

			if ((Start.DirLock) && (Start.Name))
			{
				if (dest=CREATEPROC(InformationString,2,MKBADDR(&InfoSegment),4096))
				{
					PUTMSG(dest,&(Start.msg));
					WAITPORT(port);
					GETMSG(port);
					Failed=FALSE;
				}
			}

			UNLOCK(Start.DirLock);
			FREEVEC(Start.Name);
		}
	}

	if (Failed) ErrorTitle(Quote(Q_INFO_FAILED));
	return(Failed);
}

/* The LVO jumper... */
void WB_Info(BPTR,char *,struct Screen *);

void StartInformation(void)
{
struct	MsgPort		*port=&(((struct Process *)ThisTask())->pr_MsgPort);
struct	StartMSG	*Start;
struct	Screen		*Screen;
	BPTR		Lock;
	char		*Name;


	/* Wait for the startup message */
	WAITPORT(port);
	Start=(struct StartMSG *)GETMSG(port);

	/* Get the lock and name from the startup message */
	/* We will free this memory when done... */
	Lock=Start->DirLock;	Start->DirLock=NULL;
	Name=Start->Name;	Start->Name=NULL;

	/* Lock the screen */
	Screen=LockPubScreen(SystemWorkbenchName);

	/* Reply startup message to let workbench continue... */
	REPLYMSG((struct Message *)Start);

	/* Now call Information for real... */
	WB_Info(Lock,Name,Screen);

	/* Release screen */
	UnlockPubScreen(NULL,Screen);

	/* Exit:  Cleanup and done... */
	FREEVEC(Name);
	UNLOCK(Lock);
}
