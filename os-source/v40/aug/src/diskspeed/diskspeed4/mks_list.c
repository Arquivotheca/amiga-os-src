/*
 * MKSoft Display List Gadget - Copyright (c) 1989, 1990 by MKSoft Development
 *
 *                          DiskSpeed v4.2
 *                          ScsiSpeed v4.2
 *                                by
 *                           Michael Sinz
 *
 *            Copyright (c) 1989-1992 by MKSoft Development
 *
 *			MKSoft Development
 *			163 Appledore Drive
 *			Downingtown, PA 19335
 *
 * Yes, this is yet another disk speed testing program, but with a few
 * differences.  It was designed to give the most accurate results of the
 * true disk performance in the system.  For this reason many of
 * DiskSpeed's results may look either lower or higher than current disk
 * performance tests.
 *
 ******************************************************************************
 *									      *
 *	Reading legal mush can turn your brain into guacamole!		      *
 *									      *
 *		So here is some of that legal mush:			      *
 *									      *
 * Permission is hereby granted to distribute this program's source	      *
 * executable, and documentation for non-commercial purposes, so long as the  *
 * copyright notices are not removed from the sources, executable or	      *
 * documentation.  This program may not be distributed for a profit without   *
 * the express written consent of the author Michael Sinz.		      *
 *									      *
 * This program is not in the public domain.				      *
 *									      *
 * Fred Fish is expressly granted permission to distribute this program's     *
 * source and executable as part of the "Fred Fish freely redistributable     *
 * Amiga software library."						      *
 *									      *
 * Permission is expressly granted for this program and it's source to be     *
 * distributed as part of the Amicus Amiga software disks, and the	      *
 * First Amiga User Group's Hot Mix disks.				      *
 *									      *
 ******************************************************************************
 */

#include	<exec/types.h>
#include	<exec/lists.h>
#include	<exec/nodes.h>
#include	<exec/memory.h>
#include	<graphics/text.h>
#include	<intuition/intuition.h>

#include	<clib/intuition_protos.h>
#include	<clib/exec_protos.h>
#include	<clib/graphics_protos.h>
#include	<clib/layers_protos.h>

#include	<pragmas/intuition_pragmas.h>
#include	<pragmas/exec_pragmas.h>
#include	<pragmas/graphics_pragmas.h>
#include	<pragmas/layers_pragmas.h>

#include	<string.h>

#include	"MKS_List.h"
#include	"MakeBoxes.h"

extern	struct	Library	*SysBase;
extern	struct	Library	*IntuitionBase;
extern	struct	Library	*GfxBase;
extern	struct	Library	*LayersBase;

/*
 * This is the "real" DisplayList structure...
 */
struct	RealDisplayList
{
	/*
	 * These fields are from the public structure...
	 */
	struct	DisplayList	DisplayList;

	/*
	 * The following fields are *NOT* public and are
	 * 100% maintained by the list gadget...
	 */
	struct	Node		*Top;
	struct	MinList		*TheList;
		SHORT		TopNum;
		SHORT		ListSize;
		SHORT		DisplaySize;	/* Number of display lines */
		SHORT		LineYSize;

	struct	Region		*Region;
	struct	Rectangle	Rectangle;

	struct	IntuiText	Text;
	struct	TextAttr	TextStyle;
	struct	TextFont	*TheFont;

	struct	Gadget		ListGadget;
	struct	Gadget		UpGadget;
	struct	Gadget		DnGadget;
	struct	Gadget		PropGadget;

	struct	PropInfo	PropInfo;
	struct	Image		PropImage;	/* For AutoKnob stuff... */

	struct	Border		ListGadgetBorders[4];
	struct	Border		UpGadgetBorders[5];
	struct	Border		DnGadgetBorders[5];
		SHORT		ListGadgetVectors[5*2*4];

		UBYTE		FontName[1];
};

/*
 * Whenever we have "list" as a DisplayList, we have REAL_list as a RealDisplayList
 */
#define	REAL_list	((struct RealDisplayList *)list)

#define	LIST_GADGET_ID	(0x7F01)

/*
 * Vectors that display the arrows...
 */
static	SHORT	DnArrow_V[7*2]={4,3,7,0,0,0,3,3,6,1,2,1,3,2};
static	SHORT	UpArrow_V[7*2]={4,0,7,3,0,3,3,0,6,2,2,2,3,1};
static	SHORT	ArrowBoxTopLeft_V[5*2]={12,0,0,0,0,7,1,6,1,1};
static	SHORT	ArrowBoxBottomRight_V[5*2]={1,7,13,7,13,0,12,1,12,6};

static	struct	Border	DnArrow={3,2,0,0,JAM1,7,DnArrow_V,NULL};
static	struct	Border	UpArrow={3,2,0,0,JAM1,7,UpArrow_V,NULL};

/*
 * These two DEFINEs help find the correct RastPort and BackFill colour
 */
#define	PICK_RASTPORT(window,list)	(window->RPort)
#define	PICK_BACKFILL(list)		(0)

/*
 * Just a quicky to help fill in the boarder structures...
 */
static void Do_Borders(struct Border *borders,UBYTE Highlight,UBYTE Shadow)
{
	borders[1].FrontPen=Highlight;
	borders[1].DrawMode=JAM1;
	borders[1].Count=5;
	borders[1].XY=ArrowBoxTopLeft_V;
	borders[1].NextBorder=&(borders[0]);
	borders[2].FrontPen=Shadow;
	borders[2].DrawMode=JAM1;
	borders[2].Count=5;
	borders[2].XY=ArrowBoxTopLeft_V;
	borders[2].NextBorder=&(borders[0]);
	borders[3].FrontPen=Shadow;
	borders[3].DrawMode=JAM1;
	borders[3].Count=5;
	borders[3].XY=ArrowBoxBottomRight_V;
	borders[3].NextBorder=&(borders[1]);
	borders[4].FrontPen=Highlight;
	borders[4].DrawMode=JAM1;
	borders[4].Count=5;
	borders[4].XY=ArrowBoxBottomRight_V;
	borders[4].NextBorder=&(borders[2]);
}

/*
 * This routine deallocates the DisplayList allocated below...
 * It does not do any unlinking...
 */
void FreeListGadget(struct DisplayList *list)
{
	if (list)
	{
		if (REAL_list->TheFont) CloseFont(REAL_list->TheFont);
		if (REAL_list->Region) DisposeRegion(REAL_list->Region);
		FreeMem(list,sizeof(struct RealDisplayList)+(LONG)strlen(REAL_list->FontName));
	}
}

/*
 * This routine allocates and initializes a DisplayList gadget...
 * It will link the gadgets into the gadget list given...
 *
 * Arguments:  TextAttr,**Gadget,TextPen,Highlight,Shadow,Left,Top,Width,Height
 */
struct DisplayList *InitListGadget(struct TextAttr *ta,struct Gadget **gad,UBYTE TextPen,UBYTE Highlight,UBYTE Shadow,SHORT Left,SHORT Top,SHORT Width,SHORT Height)
{
register	struct	RealDisplayList	*rlist;
register		SHORT		tmp;

	if (rlist=AllocMem(sizeof(struct RealDisplayList)+(LONG)strlen(ta->ta_Name),MEMF_PUBLIC|MEMF_CLEAR))
	{
		if (rlist->Region=NewRegion())
		{
			rlist->DisplaySize=(Height-4) / (rlist->LineYSize=((ta->ta_YSize)+2));

			/*
			 * Make borders for the main list box...
			 */
			rlist->ListGadgetBorders[0].FrontPen=Shadow;
			rlist->ListGadgetBorders[0].DrawMode=JAM1;
			rlist->ListGadgetBorders[0].Count=5;
			rlist->ListGadgetBorders[0].XY=&(rlist->ListGadgetVectors[0*5*2]);
			rlist->ListGadgetBorders[0].NextBorder=&(rlist->ListGadgetBorders[1]);
			FillTopLeft_Border(&(rlist->ListGadgetBorders[0]),Width-16,Height);

			rlist->ListGadgetBorders[1].FrontPen=Highlight;
			rlist->ListGadgetBorders[1].DrawMode=JAM1;
			rlist->ListGadgetBorders[1].Count=5;
			rlist->ListGadgetBorders[1].XY=&(rlist->ListGadgetVectors[1*5*2]);
			rlist->ListGadgetBorders[1].NextBorder=&(rlist->ListGadgetBorders[2]);
			FillBottomRight_Border(&(rlist->ListGadgetBorders[1]),Width-16,Height);

			/*
			 * Make borders for the prop gadget area...
			 * This is linked to the main list box since the prop gadget
			 * will be just a borderless AutoKnob...
			 */
			rlist->ListGadgetBorders[2].LeftEdge=Width-14;
			rlist->ListGadgetBorders[2].FrontPen=Highlight;
			rlist->ListGadgetBorders[2].DrawMode=JAM1;
			rlist->ListGadgetBorders[2].Count=5;
			rlist->ListGadgetBorders[2].XY=&(rlist->ListGadgetVectors[2*5*2]);
			rlist->ListGadgetBorders[2].NextBorder=&(rlist->ListGadgetBorders[3]);
			FillTopLeft_Border(&(rlist->ListGadgetBorders[2]),14,Height-16);

			rlist->ListGadgetBorders[3].LeftEdge=Width-14;
			rlist->ListGadgetBorders[3].FrontPen=Shadow;
			rlist->ListGadgetBorders[3].DrawMode=JAM1;
			rlist->ListGadgetBorders[3].Count=5;
			rlist->ListGadgetBorders[3].XY=&(rlist->ListGadgetVectors[3*5*2]);
			rlist->ListGadgetBorders[3].NextBorder=NULL;
			FillBottomRight_Border(&(rlist->ListGadgetBorders[3]),14,Height-16);

			/*
			 * The main list gadget...
			 */
			rlist->ListGadget.LeftEdge=Left;
			rlist->ListGadget.TopEdge=Top;
			rlist->ListGadget.Width=0;
			rlist->ListGadget.Height=0;
			rlist->ListGadget.Flags=GADGHNONE;
			rlist->ListGadget.Activation=0;
			rlist->ListGadget.GadgetType=BOOLGADGET;
			rlist->ListGadget.GadgetRender=(APTR)&(rlist->ListGadgetBorders[0]);
			rlist->ListGadget.GadgetID=LIST_GADGET_ID;
			rlist->ListGadget.UserData=(APTR)rlist;

			/*
			 * The AutoKnob is Borderless since we will be making
			 * our own 3-D style border...
			 */
			rlist->PropInfo.Flags=AUTOKNOB|FREEVERT|PROPBORDERLESS;
			rlist->PropInfo.HorizBody=MAXBODY;
			rlist->PropInfo.VertBody=MAXBODY;
			rlist->PropGadget.LeftEdge=Left+Width-11;
			rlist->PropGadget.TopEdge=Top+2;
			rlist->PropGadget.Width=8;
			rlist->PropGadget.Height=Height-20;
			rlist->PropGadget.Flags=GADGIMAGE;
			rlist->PropGadget.Activation=GADGIMMEDIATE|RELVERIFY|FOLLOWMOUSE;
			rlist->PropGadget.GadgetType=PROPGADGET;
			rlist->PropGadget.GadgetRender=(APTR)&(rlist->PropImage);
			rlist->PropGadget.SpecialInfo=(APTR)&(rlist->PropInfo);
			rlist->PropGadget.GadgetID=LIST_GADGET_ID;
			rlist->PropGadget.UserData=(APTR)rlist;

			/*
			 * Set up the Up and Down arrow gadgets...
			 */
			rlist->UpGadgetBorders[0]=UpArrow;
			rlist->UpGadgetBorders[0].FrontPen=TextPen;
			Do_Borders(rlist->UpGadgetBorders,Highlight,Shadow);
			rlist->UpGadget.LeftEdge=Left+Width-14;
			rlist->UpGadget.TopEdge=Top+Height-16;
			rlist->UpGadget.Width=14;
			rlist->UpGadget.Height=8;
			rlist->UpGadget.Flags=((Shadow!=Highlight) ? GADGHIMAGE : GADGHCOMP);
			rlist->UpGadget.Activation=RELVERIFY|GADGIMMEDIATE;
			rlist->UpGadget.GadgetType=BOOLGADGET;
			rlist->UpGadget.GadgetRender=(APTR)&(rlist->UpGadgetBorders[3]);
			rlist->UpGadget.SelectRender=(APTR)&(rlist->UpGadgetBorders[4]);
			rlist->UpGadget.GadgetID=LIST_GADGET_ID;
			rlist->UpGadget.UserData=(APTR)rlist;

			rlist->DnGadgetBorders[0]=DnArrow;
			rlist->DnGadgetBorders[0].FrontPen=TextPen;
			Do_Borders(rlist->DnGadgetBorders,Highlight,Shadow);
			rlist->DnGadget.LeftEdge=Left+Width-14;
			rlist->DnGadget.TopEdge=Top+Height-8;
			rlist->DnGadget.Width=14;
			rlist->DnGadget.Height=8;
			rlist->DnGadget.Flags=((Shadow!=Highlight) ? GADGHIMAGE : GADGHCOMP);
			rlist->DnGadget.Activation=RELVERIFY|GADGIMMEDIATE;
			rlist->DnGadget.GadgetType=BOOLGADGET;
			rlist->DnGadget.GadgetRender=(APTR)&(rlist->DnGadgetBorders[3]);
			rlist->DnGadget.SelectRender=(APTR)&(rlist->DnGadgetBorders[4]);
			rlist->DnGadget.GadgetID=LIST_GADGET_ID;
			rlist->DnGadget.UserData=(APTR)rlist;

			/*
			 * Set up the rectangle and clipping area for the list box
			 */
			rlist->Rectangle.MaxX=(rlist->Rectangle.MinX=(rlist->ListGadget.LeftEdge)+3)+Width-25;
			rlist->Rectangle.MaxY=(rlist->Rectangle.MinY=(rlist->ListGadget.TopEdge)+2)+Height-5;

			/*
			 * We want to CENTER the rectangle based on the number of
			 * text lines and the text hight.
			 */
			tmp=(rlist->Rectangle.MaxY)-(rlist->Rectangle.MinY)+1;
			tmp-=(rlist->DisplaySize)*(rlist->LineYSize);
			if (tmp>0)
			{
				rlist->Rectangle.MinY+=(tmp>>1);
				rlist->Rectangle.MaxY-=((tmp+1)>>1);
			}

			/*
			 * Pre-initialize the Intuitext structure as much as possible...
			 */
			rlist->Text.FrontPen=TextPen;
			rlist->Text.DrawMode=JAM1;
			rlist->Text.LeftEdge=1;
			rlist->Text.TopEdge=1;
			rlist->Text.ITextFont=&(rlist->TextStyle);

			/*
			 * Make a copy of the TextAttr for the list display
			 */
			rlist->TextStyle=*ta;
			strcpy(rlist->TextStyle.ta_Name=rlist->FontName,ta->ta_Name);

			/*
			 * Open the font so it stays around...
			 * Since the font had to be open when called, this just gets
			 * another open on it.  The clean up routine closes this...
			 */
			rlist->TheFont=OpenFont(&(rlist->TextStyle));

			/*
			 * Now, pre-build the region for better speed...
			 */
			if (!OrRectRegion(rlist->Region,&(rlist->Rectangle)))
			{
				FreeListGadget((struct DisplayList *)rlist);
				rlist=NULL;
			}
		}
		else
		{
			FreeListGadget((struct DisplayList *)rlist);
			rlist=NULL;
		}
	}
	if (rlist)
	{
		rlist->ListGadget.NextGadget=*gad;
		rlist->PropGadget.NextGadget=&(rlist->ListGadget);
		rlist->UpGadget.NextGadget=&(rlist->PropGadget);
		rlist->DnGadget.NextGadget=&(rlist->UpGadget);
		*gad=&(rlist->DnGadget);
	}
	return((struct DisplayList *)rlist);
}

/*
 * This routine updates the prop gadget for the list given...
 */
static void UpDate_Prop(struct Window *window,struct DisplayList *list)
{
register	long	diffsize;

	diffsize=(REAL_list->ListSize)-(REAL_list->DisplaySize);

	if (diffsize>0)
	{
		NewModifyProp(&(REAL_list->PropGadget),window,NULL,REAL_list->PropInfo.Flags,
				0,
				(USHORT)(((0xffffL)*(REAL_list->TopNum)+(diffsize>>1))/diffsize),

				0xffff,
				(USHORT)(((0xffffL)*(REAL_list->DisplaySize))/(REAL_list->ListSize)),

				1L);
	}
	else
	{
		NewModifyProp(&(REAL_list->PropGadget),window,NULL,REAL_list->PropInfo.Flags,
				0,
				0,

				0xffff,
				0xffff,

				1L);
	}
}

/*
 * This routine takes the NewTop and sets it into the TopNum and
 * positions *Top to the right node value...
 *
 * Note:  Since this routine does not do ERROR checking, the NewTop value
 *        and TheList *MUST* be valid...
 */
static void SetTopPointer(struct DisplayList *list,SHORT NewTop)
{
struct	Node	*item;

	REAL_list->TopNum=NewTop;
	item=(struct Node *)(REAL_list->TheList->mlh_Head);

	while(NewTop)
	{
		item=(struct Node *)(item->ln_Succ);
		NewTop--;
	}
	REAL_list->Top=item;
}

/*
 * This routine will display the line number given...  (Display line number)
 *
 * If the line is selected, it will be inverted via a Complement RectFill
 *
 * All actions will be clipped via a ClipRect that this routine installs.
 */
static void DisplayLine(struct RastPort *rp,struct DisplayList *list,SHORT line)
{
register	struct	Region	*r;
register	struct	Node	*item;
register		SHORT	y;
register		SHORT	y1;

	if ((line>=0)&&(line<(REAL_list->DisplaySize)))
	{
		/*
		 * Set up clipping rectangle within the borders of the gadget...
		 */
		r=InstallClipRegion(rp->Layer,REAL_list->Region);

		y1=(y=(REAL_list->Rectangle.MinY)+(line*(REAL_list->LineYSize)))+(REAL_list->LineYSize)-1;

		SetAPen(rp,PICK_BACKFILL(REAL_list));
		SetDrMd(rp,JAM1);
		RectFill(rp,REAL_list->Rectangle.MinX,y,REAL_list->Rectangle.MaxX,y1);

		if (item=REAL_list->Top)
		{
			while (line>0)
			{
				if (item->ln_Succ)
				{
					item=(struct Node *)(item->ln_Succ);
					line--;
				}
				else line=0;
			}
			if (item->ln_Succ)
			{
				REAL_list->Text.IText=item->ln_Name;
				PrintIText(rp,&(REAL_list->Text),REAL_list->Rectangle.MinX,y);
			}
		}

		InstallClipRegion(rp->Layer,r);
	}
}

/*
 * This routine will clear the list box to the backfill colour...
 */
static void ClearDisplayBox(struct RastPort *rp,struct DisplayList *list)
{
	SetAPen(rp,PICK_BACKFILL(REAL_list));
	SetDrMd(rp,JAM1);
	RectFill(rp,	REAL_list->Rectangle.MinX,REAL_list->Rectangle.MinY,
			REAL_list->Rectangle.MaxX,REAL_list->Rectangle.MaxY);
}

/*
 * This routine displays all of the lines in the list...
 */
static void DisplayAllLines(struct RastPort *rp,struct DisplayList *list)
{
register	SHORT	line;

	for (line=0;line<(REAL_list->DisplaySize);line++) DisplayLine(rp,list,line);
}

/*
 * This routine moves the list UP or DOWN to the line number give.
 *
 * It will try to make sure that this is done as best as possible...
 * It will update the prop gadget if it is not active...
 */
static void MoveList(struct Window *window,struct DisplayList *list,SHORT line)
{
register	struct	RastPort	*rp;
register		SHORT		newline;

	newline=line+(REAL_list->TopNum);
	if (newline<0) line=0-(REAL_list->TopNum);
	else if (newline>((REAL_list->ListSize)-(REAL_list->DisplaySize)))
	{
		line=(REAL_list->ListSize)-(REAL_list->DisplaySize)-(REAL_list->TopNum);
		if (line<0) line=0;
	}

	if (line)
	{
		rp=PICK_RASTPORT(window,REAL_list);
		SetTopPointer(list,line+(REAL_list->TopNum));
		if (line>0) newline=line;
		else newline=0-line;

		if ((newline*11)>(8*(REAL_list->DisplaySize)))
		{
			DisplayAllLines(rp,list);
		}
		else
		{
			SetDrMd(rp,JAM2);
			SetBPen(rp,PICK_BACKFILL(REAL_list));

			ScrollRaster(rp,0,line*(REAL_list->LineYSize),
					REAL_list->Rectangle.MinX,REAL_list->Rectangle.MinY,
					REAL_list->Rectangle.MaxX,REAL_list->Rectangle.MaxY);

			if (line<0)
			{
				newline=0-line;
				line=0;
			}
			else
			{
				newline=REAL_list->DisplaySize;
				line=(REAL_list->DisplaySize)-line;
			}
			while (line<newline) DisplayLine(rp,list,line++);
		}
		if (!((REAL_list->PropGadget.Flags)&SELECTED)) UpDate_Prop(window,list);
	}
}

/*
 * This routine will initialize a fresh list.  It clears Selected,
 * counts the entries, sets top of the display such that the bottom
 * of the list is displayed and displays the list...
 */
void FreshList(struct Window *window,struct DisplayList *list,struct MinList *newlist)
{
register	struct	Node	*item;
register		SHORT	count;

	REAL_list->TheList=newlist;
	REAL_list->Top=REAL_list->DisplayList.Selected=NULL;
	count=0;
	if (newlist)
	{
		item=(struct Node *)(REAL_list->TheList->mlh_Head);
		while (item=(struct Node *)(item->ln_Succ)) count++;
		SetTopPointer(list,(count > REAL_list->DisplaySize) ? count-REAL_list->DisplaySize : 0);
	}
	REAL_list->ListSize=count;
	UpDate_Prop(window,list);
	DisplayAllLines(PICK_RASTPORT(window,REAL_list),REAL_list);
}

/*
 * This routine loops while one of the arrow gadgets is pressed and will
 * advance the display accordingly.
 */
static void ArrowGadgetLoop(struct Window *window,struct DisplayList *list,struct Gadget *gad,SHORT change)
{
register	struct	IntuiMessage	*msg;
register		ULONG		old_IDCMP;
register		SHORT		tick_count;
register		SHORT		flag=TRUE;

	old_IDCMP=window->IDCMPFlags;
	ModifyIDCMP(window,MOUSEBUTTONS|GADGETUP|INTUITICKS);

	MoveList(window,list,change);
	tick_count=-3;

	while(flag)
	{
		WaitPort(window->UserPort);
		if (msg=(struct IntuiMessage *)GetMsg(window->UserPort))
		{
			switch (msg->Class)
			{
			case MOUSEBUTTONS:	if (msg->Code==SELECTUP) flag=FALSE;
						break;
			case GADGETUP:		flag=FALSE;
						break;
			case INTUITICKS:	tick_count++;
						if ((tick_count>0)&&(gad->Flags&SELECTED))
						{
							MoveList(window,list,change);
						}
						break;
			}
			ReplyMsg((struct Message *)msg);
		}
	}

	ModifyIDCMP(window,old_IDCMP);
}

/*
 * This routine moves the list to match the prop gadget...
 */
static void MoveToProp(struct Window *window,struct DisplayList *list)
{
register	ULONG	tmp;

	if ((tmp=((REAL_list->ListSize)-(REAL_list->DisplaySize)))>0)
	{
		tmp=(tmp*(ULONG)(REAL_list->PropInfo.VertPot)+(0xffffL >> 1))/0xffffL;
		MoveList(window,list,(SHORT)tmp-(REAL_list->TopNum));
	}
}

/*
 * This routine is entered on GADGETDOWN on the PropGadget and
 * it loops until the user releases it.  It updates the display
 * as needed by user actions...
 */
static void PropGadgetLoop(struct Window *window,struct DisplayList *list)
{
register	struct	IntuiMessage	*msg;
register		ULONG		old_IDCMP;
register		SHORT		flag=TRUE;

	old_IDCMP=window->IDCMPFlags;
	ModifyIDCMP(window,GADGETUP|MOUSEMOVE);

	while(flag)
	{
		msg=(struct IntuiMessage *)WaitPort(window->UserPort);
		while ((flag)&&(msg)) if (msg=(struct IntuiMessage *)GetMsg(window->UserPort))
		{
			if (msg->Class==GADGETUP) flag=FALSE;
			ReplyMsg((struct Message *)msg);
		}
		MoveToProp(window,list);
	}

	ModifyIDCMP(window,old_IDCMP);
}

/*
 * This is the InputEvent filter...
 * Call this routine with each message.  If it returns a pointer to
 * a DisplayList, a message was processed, if it returns NULL, nothing
 * was done with the message and you may need to process it.
 * The message is NOT ReplyMsg()ed...
 */
struct DisplayList *Check_ListGadget(struct Window *window,struct IntuiMessage *message)
{
register	struct	Gadget		*gad;
register	struct	DisplayList	*list=NULL;

	if (message)
	{
		if (message->Class==GADGETDOWN)
		{
			gad=(struct Gadget *)(message->IAddress);
			if (gad->GadgetID==LIST_GADGET_ID)
			{
				if (list=(struct DisplayList *)(gad->UserData))
				{
					if (&(REAL_list->PropGadget)==gad) PropGadgetLoop(window,list);
					else if (&(REAL_list->UpGadget)==gad) ArrowGadgetLoop(window,list,gad,-1);
					else if (&(REAL_list->DnGadget)==gad) ArrowGadgetLoop(window,list,gad,1);
				}
			}
		}
	}
	return(list);
}
