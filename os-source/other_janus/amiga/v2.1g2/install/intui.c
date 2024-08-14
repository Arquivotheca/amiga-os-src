/*
   Prompt routines for install.
*/

#include <exec/types.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>

#include <proto/all.h>

#include "gadgets.h"
#include "display.h"

#include "install.h"

#include "intui.h"

#define DEBUG2   0

#define ABS(a)   ((a) < 0 ? -(a) : (a))

#define NUMROWS   ((WHEIGHT - TOPOFFSET - 2) / 8)
#define NUMCOLS   ((WWIDTH - UPWIDTH) / 8 - 1)

#define RWIDTH      640
#define RHEIGHT      100

struct Screen *s;		/* our screen */
struct Window *w;		/* our window */
struct RastPort *rp;		/* our rastport */

struct TextAttr topaz9 = {
    "topaz.font", 9, 0, 0
};

struct TextAttr topaz8 = {
    "topaz.font", 8, 0, 0
};

struct IntuiText buttonText = {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE,
    &topaz9, "OK", AUTONEXTTEXT
};

struct IntuiText bodyText2 = {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE+12,
    &topaz9, 0, AUTONEXTTEXT
};
struct IntuiText bodyText1 = {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE,
    &topaz9, 0, &bodyText2
};


extern struct Gadget UpGadget;
extern struct Gadget PrGadget;
extern struct PropInfo PrInfo;
extern struct Image CheckMark;
extern struct Image XMark;

static struct List *List;
static struct file_node *TopNode;
static LONG Step;

VOID ResetProp(VOID)
{
	ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
		   0, PrInfo.HorizBody, PrInfo.VertBody);
}

VOID Notice(struct Window *w,UBYTE *string1, UBYTE *string2)
{ 
    bodyText1.IText = string1;
    bodyText2.IText = string2;
    AutoRequest(w, &bodyText1, 0, &buttonText, 0, 0, RWIDTH, RHEIGHT);
}

VOID PrintMsg(UBYTE *msg)
{
#define PM_BUFSIZE	80

char buf[PM_BUFSIZE];

	strcpy(buf, msg);
	setmem(&buf[strlen(buf)], PM_BUFSIZE - 1 - strlen(buf), ' ');
	SetAPen(rp,3);
	Move(rp, 4, 18);
	Text(rp, buf, PM_BUFSIZE - 1);
	SetAPen(rp,1);
}

VOID ClearMsg(VOID)
{
	PrintMsg("");
}

VOID InformMsg(LONG Required)
{
	if (Required >= 0)
		PrintMsg("Select files to delete until (Still Needed) is < 0.");
	else
		PrintMsg("Ready to delete selected files.      (Hit OK to delete)");
			/*12345678901234567890123456789012345678901234567890123456789012345*/
}

void Status(struct Window *w,
	    LONG *list_blocks,
	    LONG *src_blocks,
	    LONG *dest_blocks,
	    int del)
{
static char buf[58] =  "  File Blocks To Delete[     ]    Still Needed[     ]    ";
		      /*012345678901234567890123456789012345678901234567890123456*/
static char abuf[58] = "  File Blocks Selected To Install[     ]                 ";

	if (del) {
		sprintf(&buf[24], "%5ld", *list_blocks);
		buf[29] = ']'; /* overwrite NULL */

		sprintf(&buf[47], "%5ld", -(*src_blocks) - *dest_blocks);
		buf[52] = ']'; /* overwrite null */

		SetWindowTitles(w, buf,(char *) -1);
		InformMsg(-(*src_blocks) - *dest_blocks);
	} else {
		sprintf(&abuf[34], "%5ld", *list_blocks);
		abuf[39] = ']'; /* overwrite NULL */

		SetWindowTitles(w, abuf,(char *) -1);
	}
}


LONG UnpackNode(struct file_node *node,UBYTE *buf)
{
LONG len, check;
static char DirString[] = " (Dir)";

	sprintf(buf, "%5ld ", node->blocks);
	len = 6;

	setmem(&buf[len], node->depth * 4, ' ');
	len += node->depth * 4;

	buf[len]=' ';
	if (node->selected)
		check = len;
	else
		check = -1;
	len++;

	sprintf(&buf[len], "%s", node->dest_name);
	len += strlen(node->dest_name);

	if (node->type == FILENODE_TYPE_DIR) {
		strcpy(&buf[len], DirString);
		len += 6;
	}

	setmem(&buf[len], NUMCOLS - len, ' ');

	return check;
}

VOID DisplayNode(struct file_node *node, int del)
{
char buf[NUMCOLS];
LONG check,y;

	check=UnpackNode(node, buf)*8;
	y=CHARYOFFSET + (node->idx - TopNode->idx) * 8;
	Move(rp, CHARXOFFSET, y);
	Text(rp, buf, NUMCOLS - 1);

	if(check >= 0) {
		if(!del)
			DrawImage(rp,&CheckMark,CHARXOFFSET+check,y);
		else
			DrawImage(rp,&XMark,CHARXOFFSET+check,y);
	}
}

VOID RefreshDisplay(struct file_node *node, int del)
{
struct file_node *node2;
LONG count, dy, i;
char buf[NUMCOLS];

	count = NUMROWS;
	if (List->lh_Head->ln_Succ != NULL) { /* if list not empty */
		node2 = node;
		if (TopNode != NULL) {
			/* not first time */
			if ((count = ABS(node->idx - TopNode->idx)) < NUMROWS) {
				/* some of the display is still useful */
				dy = count * 8;
				if (node->idx > TopNode->idx) {
					/* scroll up */
					ScrollRaster(rp, 0, dy, /* dx, dy */
						     CHARXOFFSET, TOPOFFSET + 2,/* xmin,ymin */
						     NUMCOLS * 8 - 1,	/* xmax */
						     TOPOFFSET + 2 + NUMROWS *8 - 1);/* ymax */

					/* skip (drawing starts in middle) */
					for (i = 0; i < NUMROWS - count; i++) {
						node2 = (struct file_node *)node2->node.ln_Succ;
					}
				} else {
					/* scroll down */
					ScrollRaster(rp, 0, -dy, /* dx, dy */
						     CHARXOFFSET, TOPOFFSET +2,/* xmin, ymin */
						     NUMCOLS * 8 - 1,	/* xmax */
						     TOPOFFSET + 2 + NUMROWS *8 - 1);/* ymax */
				}
			} else {
				/* full refresh */
				count = NUMROWS;
			}
		} else {
			/* first time */
			count = NUMROWS;
		}

		/* set new display top */
		TopNode = node;
		for (; node2->node.ln_Succ; node2 = (struct file_node *)node2->node.ln_Succ) {
			DisplayNode(node2, del);
			if (--count == 0) {
				break;
			}
		}
	}

	/* clear unused area */
	setmem(buf, NUMCOLS, ' ');
	i = (NUMROWS - count) * 8;
	while (count > 0) {
		/* erase to bottom */
		Move(rp, CHARXOFFSET, CHARYOFFSET + i);
		Text(rp, buf, NUMCOLS - 1);
		count--;
		i += 8;
	}
}

VOID SelectUp(int del)
{
	if (TopNode != (struct file_node *)List->lh_Head) {
		RefreshDisplay((struct file_node *)TopNode->node.ln_Pred, del);
		ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
			Step * TopNode->idx, PrInfo.HorizBody, PrInfo.VertBody);
	}
}

VOID SelectDown(int del)
{
	if (TopNode->idx + NUMROWS <= ((struct file_node *)List->lh_TailPred)->idx) {
		RefreshDisplay((struct file_node *)TopNode->node.ln_Succ, del);
		ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
			Step * TopNode->idx, PrInfo.HorizBody, PrInfo.VertBody);
	}
}

VOID DoProp(struct Gadget *Gadget, int del)
{
struct PropInfo *Prop;
struct file_node *node;
LONG num;

	Prop = (struct PropInfo *)Gadget->SpecialInfo;

	if (Step != -1) {
		num = Prop->VertPot / Step;

		/* find node 'num' in the list */
		for (node = (struct file_node *)List->lh_Head;
		     (node->node.ln_Succ) && (node->idx != num);
		     node = (struct file_node *)node->node.ln_Succ)
			;

		/* if this isn't the top of the display already, fix */
		if (node != TopNode) {
			RefreshDisplay(node, del);
		}
	}
}

LONG DoGadgets(struct IntuiMessage *Msg, int del)
{
	struct Gadget *Gadget;
	LONG done = 0;

	Gadget = (struct Gadget *)Msg->IAddress;
	switch (Gadget->GadgetID) {
		case UPID:
			SelectUp(del);
			break;
		case DNID:
			SelectDown(del);
			break;
		case OKID:
			done = 1;
			break;
		case CNID:
			done = -1;
			break;
		case PRID:
			DoProp(Gadget, del);
			break;
		default:
			break;
	}
	return(done);
}

VOID DoMouse(struct IntuiMessage *Msg,
	     LONG *list_blocks,	/* blocks in list */
	     LONG *src_blocks,		/* blocks going in */
	     LONG *dest_blocks,	/* blocks available */
	     int del)
{
struct file_node *node;
LONG x, y;

	if (Msg->Code == SELECTDOWN) {
		x = Msg->MouseX / 8;
		y = (Msg->MouseY - CHARYOFFSET + 6) / 8;

		if ((x < NUMCOLS) && (y >= 0 && y < NUMROWS)) {
			for (node = TopNode;
			     node->node.ln_Succ;
			     node = (struct file_node *)node->node.ln_Succ) {
				if (y-- < 1) {
					node->selected = !node->selected;

					DisplayNode(node, del);

					if (node->selected)
						*list_blocks += node->blocks;
					else
						*list_blocks -= node->blocks;

					Status(w, list_blocks, src_blocks, dest_blocks, del);
					break;
				}
			}
		}
	}
}

LONG mark_file_list(struct List *list,		/* the list */
		    LONG *list_blocks,		/* blocks in list */
		    LONG *src_blocks,		/* blocks going in */
		    LONG *dest_blocks,		/* blocks available */
		    int del)
{
struct IntuiMessage *Msg;
LONG VertBody, n;
LONG done = 0;
struct file_node *node;

	/* set up globals */
	List = list;

	/* number the nodes */
	n = 0;
	for (node = (struct file_node *)List->lh_Head;
	     node->node.ln_Succ;
	     node = (struct file_node *) node->node.ln_Succ)
		node->idx = n++;

	/* do we have any? */
	if (n) {
		/* yes.  will they all fit on screen? */
		if (n > NUMROWS) {
			/* no. */
			VertBody = 65535 / n * NUMROWS;
			Step = 65535 / (n - NUMROWS);
		} else {
			/* yes. */
			VertBody = 65535;
			Step = -1;
		}

		ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
			   PrInfo.VertPot, PrInfo.HorizBody, VertBody);

		/* another global */
		TopNode = (struct file_node *)0;

		/* fix display */
		RefreshDisplay((struct file_node *)List->lh_Head, del);

		/* give him numbers */
		Status(w, list_blocks, src_blocks, dest_blocks, del);

		/* let user fiddle with things */
		while (done == 0) {
			Wait(1 << w->UserPort->mp_SigBit);
			while ((Msg = (struct IntuiMessage *)GetMsg(w->UserPort)) != NULL) {
				switch (Msg->Class) {
				case MOUSEBUTTONS:
					DoMouse(Msg, list_blocks, src_blocks, dest_blocks, del);
					break;
				case GADGETUP:
					done = DoGadgets(Msg, del);
					break;
				case CLOSEWINDOW:
					done= -1;
					break;
				default:
					break;
				}
				ReplyMsg((struct Message *)Msg);
			}
		}
	}

	return(done);
}

