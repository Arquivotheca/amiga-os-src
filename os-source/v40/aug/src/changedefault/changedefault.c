/*
 * $Id$
 *
 * $Log$
 */

/*
******* ChangeDefault ********************************************************
*
*   NAME
*	ChangeDefault
*
*   SYNOPSIS
*	ChangeDefault - A drawer icon image replacement tool.
*
*   FUNCTION
*	This tool will help you change drawer icon images to match the
*	default image you use in your system.  Just select your icons
*	and then select the AppMenu item Change Default.
*
*	You can also drop the selected icons into the window.
*
*	Change Default will not change anything other than the icon image.
*	The settings of the drawer window are not changed.
*
*	The default icon is read at startup of this program.
*
*   INPUTS
*	icons -- Selected icons when the menu item is selected
*	         or the icons that are dropped on the window.
*
*   RESULTS
*	An icon written to disk with the default image.
*
*   SEE ALSO
*	None
*
*   BUGS
*	None
*
******************************************************************************
*/

#include    <exec/types.h>
#include    <exec/memory.h>
#include    <libraries/gadtools.h>
#include    <intuition/intuition.h>
#include    <workbench/workbench.h>
#include    <workbench/startup.h>
#include    <utility/tagitem.h>

#include    <clib/exec_protos.h>
#include    <clib/dos_protos.h>
#include    <clib/icon_protos.h>
#include    <clib/wb_protos.h>

#include    <pragmas/exec_pragmas.h>
#include    <pragmas/dos_pragmas.h>
#include    <pragmas/icon_pragmas.h>
#include    <pragmas/wb_pragmas.h>

#include    <clib/intuition_protos.h>
#include    <clib/graphics_protos.h>
#include    <clib/layers_protos.h>
#include    <clib/gadtools_protos.h>

#include    <pragmas/intuition_pragmas.h>
#include    <pragmas/graphics_pragmas.h>
#include    <pragmas/layers_pragmas.h>
#include    <pragmas/gadtools_pragmas.h>

#include    <string.h>

#include    "changedefault_rev.h"

extern struct Library *GfxBase;
       struct Library *LayersBase;
extern struct Library *IntuitionBase;
       struct Library *GadToolsBase;

extern struct Library *DOSBase;
extern struct Library *SysBase;
extern struct Library *IconBase;
       struct Library *WorkbenchBase;

char Title[]="Change Default" VERSTAG;

char Workbench[]="Workbench";

/*
 * The following text would need to be localized...
 */
char InfoText[]="\rDrop drawer icons into this window to change them to the default drawer\n";

/*
 * This routine returns the number of characters that should be rendered
 * for the next line given the width...
 */
static SHORT Text_Line_Fit(struct RastPort *rp,SHORT width,char *text)
{
register	SHORT	count=0;
register	SHORT	t=0;
register	SHORT	totWidth=0;

	while ((text[t])&&(text[t]!='\n'))
	{
		totWidth+=TextLength(rp,&text[t],1);
		t++;
		if (totWidth<width) count=t;
		else while (text[t]) t++;
	}

	if (t!=count) for (t=count;t>0;t--) if (text[t]==' ')
	{
		while (text[t]==' ') t--;
		count=t+1;
		t=0;
	}
	return(count);
}

/*
 * This routine displays the text asked for in the RastPort...
 * It returns a pointer to the text that did not fit...
 * It "WRAPS" the text to fit.
 *
 * '\n' in the line means newline...
 * '\r' as the first character or first character after '\n' means "Centering"
 *	until next '\n'
 */
char *Display_Text(struct RastPort *rp,struct Rectangle *rect,char *text,UBYTE pen)
{
register	char	*c;
register	SHORT	count;
register	SHORT	line;
register	SHORT	linecount=0;
register	SHORT	textsize;
register	SHORT	flag;
register	SHORT	left=rect->MinX;
register	SHORT	top=rect->MinY;
register	SHORT	width=rect->MaxX-rect->MinX+1;
register	SHORT	height=rect->MaxY-rect->MinY+1;

	c=text;

	if (*c=='\r') c++;
	while (*c==' ') c++;

	for (c=text;*c;linecount++)
	{
		c=&c[Text_Line_Fit(rp,width,c)];
		if (*c=='\n')
		{
			c++;
			if (*c=='\r') c++;
		}
		while (*c==' ') c++;
	}

	if (linecount)
	{
		line=linecount*(textsize=(rp->TxHeight+1));
		if (line>height)
		{
			linecount=height/textsize;
			line=linecount*textsize;
		}
		top+=(height-line) >> 1;

		SetAPen(rp,pen);
		SetDrMd(rp,JAM1);

		c=text;

		if (*c=='\r')
		{
			c++;
			flag=FALSE;
		}
		else flag=TRUE;
		while (*c==' ') c++;

		while (linecount--)
		{
			if (count=Text_Line_Fit(rp,width,c))
			{
				Move(rp,left+(flag ? 0 : ((width-TextLength(rp,c,count)) >> 1)),top+rp->TxBaseline);
				Text(rp,c,count);
			}
			top+=textsize;
			c=&c[count];

			if (*c=='\n')
			{
				c++;
				if (*c=='\r')
				{
					c++;
					flag=FALSE;
				}
				else flag=TRUE;
			}

			while (*c==' ') c++;
		}
	}

	if (*c)
	{
		if (c[-1]=='\r') c--;
	}
	else c=NULL;
	return(c);
}

void RenderRect(struct Window *win,struct Rectangle *rect)
{
void *vi;

    if (vi=GetVisualInfoA(win->WScreen,NULL))
    {
        DrawBevelBox(win->RPort,rect->MinX,rect->MinY,
                                rect->MaxX-rect->MinX+1,
                                rect->MaxY-rect->MinY+1,
                                GT_VisualInfo, vi,
                                GTBB_Recessed, TRUE,
                                TAG_END);

        rect->MinX+=4;
        rect->MinY+=2;
        rect->MaxX-=4;
        rect->MaxY-=2;
        FreeVisualInfo(vi);
    }

    SetAPen(win->RPort,0);
    SetBPen(win->RPort,0);
    SetDrMd(win->RPort,JAM1);
    RectFill(win->RPort,rect->MinX,rect->MinY,rect->MaxX,rect->MaxY);
}

void RenderDiskObject(struct Window *win,struct DiskObject *obj)
{
struct Rectangle rect;
struct Region *r;
struct Image *im;
short dx;
short dy;

    dx=(win->Width-win->BorderLeft-win->BorderBottom-1) >> 1;
    rect.MinX=dx-45;
    rect.MaxX=dx+46;

    rect.MaxY=win->Height - (win->BorderBottom*2) - 1;
    rect.MinY=rect.MaxY-45;

    if (rect.MaxY>win->BorderTop)
    {
        RenderRect(win,&rect);

        if (obj)
        {
            im=(struct Image *)(obj->do_Gadget.GadgetRender);

            dx=(rect.MaxX-rect.MinX-im->Width+1) >> 1;
            if (dx<0) dx=0;
            dy=(rect.MaxY-rect.MinY-im->Height+1) >> 1;
            if (dy<0) dy=0;

            if (r=NewRegion())
            {
                if (OrRectRegion(r,&rect))
                {
                    r=InstallClipRegion(win->RPort->Layer,r);

                    DrawImage(win->RPort,im,rect.MinX+dx,rect.MinY+dy);

                    r=InstallClipRegion(win->RPort->Layer,r);
                }
                DisposeRegion(r);
            }
        }
        else
        {
            rect.MinX=win->BorderLeft*2;
            rect.MinY=win->BorderTop+win->WScreen->WBorTop;
            rect.MaxX=win->Width-(win->BorderRight*2)-1;
            rect.MaxY=win->Height - (win->BorderBottom*4) - 46;

            RenderRect(win,&rect);
            Display_Text(win->RPort,&rect,InfoText,1);
        }
    }
}


void ChangeDefault(struct Window *win,BPTR lock,char *name,struct DiskObject *NewDO)
{
struct DiskObject *obj;
BPTR oldlock;
BPTR newlock=NULL;
struct FileInfoBlock *fib=NULL;

    if (lock)
    {
        oldlock=CurrentDir(lock);

        if (name) if (!*name) name=NULL;
        if (!name)
        {
            if (newlock=ParentDir(lock))
            {

                CurrentDir(newlock);
                if (fib=AllocVec(sizeof(struct FileInfoBlock),MEMF_PUBLIC))
                {
                    Examine(lock,fib);
                    name=fib->fib_FileName;
                }
            }
        }
        else name=NULL;	/* Only drawers are used here... */

        if (name) if (obj=GetDiskObject(name))
        {
            RenderDiskObject(win,obj);

	    obj->do_Gadget.Width=NewDO->do_Gadget.Width;
	    obj->do_Gadget.Height=NewDO->do_Gadget.Height;
	    obj->do_Gadget.Flags=NewDO->do_Gadget.Flags;
	    obj->do_Gadget.GadgetRender=NewDO->do_Gadget.GadgetRender;
	    obj->do_Gadget.SelectRender=NewDO->do_Gadget.SelectRender;

            if (!PutDiskObject(name,obj))
            {
		/* We may wish to print error messages here... */
            }
            else RenderDiskObject(win,obj);

            FreeDiskObject(obj);
        }

        CurrentDir(oldlock);
        UnLock(newlock);
        FreeVec(fib);
    }
}

struct NewWindow nw=
{
    0,0,0,0,0,0, /* We don't care */
    IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE,
    WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_NOCAREREFRESH|WFLG_RMBTRAP,
    NULL, /* No gadgets */
    NULL, /* No Checkmark */
    Title,
    NULL, /* No screen */
    NULL, /* No bitmap */
    0,0,0,0, /* No sizing */
    WBENCHSCREEN
};

WORD ZoomSize[4]={0,0,150,11};

struct TagItem tags[]=
{
    {WA_PubScreenName,(ULONG)Workbench},
    {WA_InnerWidth,250},
    {WA_InnerHeight,130},
    {WA_AutoAdjust,TRUE},
    {WA_Zoom,&ZoomSize},
    {TAG_DONE,NULL}
};

void main(void)
{
    if (IntuitionBase=OpenLibrary("intuition.library",37))
    {
        if (GfxBase=OpenLibrary("graphics.library",37))
        {
            if (LayersBase=OpenLibrary("layers.library",37))
            {
                if (IconBase=OpenLibrary("icon.library",37))
                {
                    if (GadToolsBase=OpenLibrary("gadtools.library",37))
                    {
                        if (WorkbenchBase=OpenLibrary("workbench.library",37))
                        {
			struct DiskObject *NewDO;

			    if (NewDO=GetDefDiskObject(WBDRAWER))
			    {
                            struct Screen *sc;

                                if (sc=LockPubScreen(Workbench))
                                {
                                struct Window *win;

                                    ZoomSize[3]=sc->WBorTop+sc->BarHeight-sc->BarVBorder;

                                    if (win=OpenWindowTagList(&nw,tags))
                                    {
                                    struct MsgPort *mport;

                                        RenderDiskObject(win,NULL);

                                        if (mport=CreateMsgPort())
                                        {
                                        void *apkey1;
                                        void *apkey2;

                                            apkey1=AddAppWindowA(0,0,win,mport,NULL);
                                            apkey2=AddAppMenuItemA(0,0,Title,mport,NULL);

                                            if ((apkey1) || (apkey2))
                                            {
                                            struct AppMessage *amsg;
                                            short quit=FALSE;

                                                while (!quit)
                                                {
                                                    Wait((1L << mport->mp_SigBit) | (1L << win->UserPort->mp_SigBit));
                                                    {
                                                    struct IntuiMessage *imsg;

                                                        while (imsg=(void *)GetMsg(win->UserPort))
                                                        {
                                                            if (imsg->Class==IDCMP_CLOSEWINDOW) quit=TRUE;
                                                            else if (imsg->Class==IDCMP_NEWSIZE) RenderDiskObject(win,NULL);
                                                            ReplyMsg((void *)imsg);
                                                        }
                                                    }

                                                    while (amsg=(struct AppMessage *)GetMsg(mport))
                                                    {
                                                    struct WBArg *warg=amsg->am_ArgList;
                                                    long args=amsg->am_NumArgs;

                                                        while (args--)
                                                        {
                                                            ChangeDefault(win,warg->wa_Lock,warg->wa_Name,NewDO);
                                                            warg++;
                                                        }
                                                        ReplyMsg(amsg);
                                                    }
                                                }
                                            }
                                            RemoveAppWindow(apkey1);
                                            RemoveAppMenuItem(apkey2);
                                            DeleteMsgPort(mport);
                                        }
                                        CloseWindow(win);
                                    }
                                    UnlockPubScreen(NULL,sc);
                                }
				FreeDiskObject(NewDO);
			    }
                            CloseLibrary(WorkbenchBase);
                        }
                        CloseLibrary(IconBase);
		    }
		    CloseLibrary(GadToolsBase);
                }
                CloseLibrary(LayersBase);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
}
