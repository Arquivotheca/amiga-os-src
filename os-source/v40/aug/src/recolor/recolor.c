/*
 * $Id: recolor.c,v 37.2 91/05/02 18:37:46 mks Exp $
 *
 * $Log:	recolor.c,v $
 * Revision 37.2  91/05/02  18:37:46  mks
 * Changed to only ask for V36 and not V37...
 * 
 * Revision 37.1  91/02/15  14:12:52  mks
 * Added the truncate tooltype to help clean up the icons that were
 * incorrectly created.  (The image depth field was incorrect)
 *
 * Updated the documentation to match.
 *
 * Revision 37.0  91/02/15  12:17:50  mks
 * Initial release
 *
 */

/*
 * Recolor is a simple AppMenuItem that will take icons selected
 * and remap swap the color of the icon as given in it's tool types.
 * If nothing is in the tool types it will default to color 1/2 swap.
 */

/*
******* Recolor **************************************************************
*
*   NAME
*	Recolor
*
*   SYNOPSIS
*	Recolor - A icon recoloring tool
*
*   FUNCTION
*	The tool is an icon recoloring tool that is fully configurable.
*	It can swap/shift the colors of the selected icons when its
*	menu item is selected.
*
*	You can also drop the selected icons into the Recolor window.
*	This will also recolor the icons.
*
*	To configure the recoloring process, you just add tooltypes to
*	the recolor icon.  The tool types express the color table
*	swaps that are to happen and the order in which they should happen.
*
*	The default tooltype in the icon is set for swapping colors 1 & 2
*	as this is what most people would need.
*
*	The TRUNCATE tooltype will tell RECOLOR to truncate the icon
*	to a depth.  Some older icon builders had made the icon image
*	depth field incorrect.  On deeper screens or while recoloring,
*	this can cause strange results.  By setting the TRUNCATE value
*	to the depth of the icon desired, Recolor will set the depth
*	and thus clean up the image.
*
*	Example tool types:
*
*		TRUNCATE=2		* Truncate to a depth of 2.
*
*		SWAP=1:2		* This is the default b/w swap
*
*		SWAP=1:3
*		SWAP=2:1		* This is a 1-3-2 swap/rotate
*
*		SWAP=7:2		* This swaps colors 7 and 2
*
*	Note:	That is the output color is more bit planes that the
*		initial icon, strange things may happen.
*
*	Recolor can also take these parameters as command line arguments.
*	For example:
*
*		Recolor			; No arguments defaults to SWAP=1:2
*		Recolor swap=2:3		; Swap 2 & 3
*		Recolor swap=1:3 swap=2:1	; Swap 1 - 3 - 2
*		Recolor truncate=2		; Truncate to 2 bit planes
*
*   INPUTS
*	tooltype -- The swapping information
*	icons -- Selected icons when the menu item is selected
*	         or the icons that are dropped on the window.
*
*   RESULTS
*	An icon written to disk with the new color setup.
*
*   SEE ALSO
*	None
*
*   BUGS
*	Does not handle icons with image structures that have non-standard
*	plane-pick and plane-on/off values.  But then, neither does
*	icon.library... :-)
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

#include    "recolor_rev.h"

extern struct Library *GfxBase;
       struct Library *LayersBase;
extern struct Library *IntuitionBase;
       struct Library *GadToolsBase;

extern struct Library *DOSBase;
extern struct Library *SysBase;
extern struct Library *IconBase;
       struct Library *WorkbenchBase;

char Title[]="Recolor" VERSTAG;

char Workbench[]="Workbench";

/*
 * The following text would need to be localized...
 */
char InfoText[]="\rDrop icons into this window to recolor\n\n"
		"\ror select icons and use the Recolor tool menu from workbench\n";

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

void RemapImage(UBYTE *map,struct Image *im)
{
USHORT *data;
USHORT *newdata;
LONG words;
LONG count;
USHORT bits;
USHORT plane;
USHORT mask;
USHORT tmp;
UBYTE value;


    if (im) if (im->Depth>0)
    {
        words=((im->Width+15) >> 4);
        words*=im->Height;
        data=im->ImageData;

        for (count=0;count<words;count++)
        {
            for (bits=0;bits<16;bits++)
            {
                mask=1L << bits;

                for (plane=0,newdata=data,value=0;plane<im->Depth;plane++,newdata=&newdata[words])
                {
                    if (*newdata & mask) value|=1 << plane;
                }

                value=map[value];

                for (plane=0,newdata=data;plane<im->Depth;plane++,newdata=&newdata[words])
                {
                    tmp=*newdata;
                    tmp&=~mask;
                    if (value & (1 << plane)) tmp|=mask;
                    *newdata=tmp;
                }
            }
            data++;
        }
    }
}

void Remap(struct Window *win,UBYTE *map,BPTR lock,char *name,long trunc)
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
            else name="disk";
        }

        if (name) if (obj=GetDiskObject(name))
        {
	    if (trunc)
	    {
	    struct Image *im;

		if (im=(struct Image *)(obj->do_Gadget.GadgetRender)) if (im->Depth>trunc) im->Depth=trunc;
		if (im=(struct Image *)(obj->do_Gadget.SelectRender)) if (im->Depth>trunc) im->Depth=trunc;
	    }

            RenderDiskObject(win,obj);

            if (!(obj->do_Gadget.Flags & GFLG_GADGHIMAGE)) obj->do_Gadget.SelectRender=NULL;

            RemapImage(map,(struct Image *)(obj->do_Gadget.GadgetRender));
            RemapImage(map,(struct Image *)(obj->do_Gadget.SelectRender));

            RenderDiskObject(win,obj);

            if (!PutDiskObject(name,obj))
            {
		/* We may wish to print error messages here... */
            }

            FreeDiskObject(obj);
        }

        CurrentDir(oldlock);
        UnLock(newlock);
        FreeVec(fib);
    }
}

long buildmap(char **rules,UBYTE *map)
{
char *c;
char *p;
long x;
long y;
long trunc=0;

    for (x=0;x<256;x++) map[x]=x;

    while (c=*rules)
    {
        if (!strnicmp("SWAP=",c,5))
        {
            c+=5;
            for (p=c;*p && *p!=':';p++);
            if (*p==':')
            {
                *p++='\0';

                StrToLong(c,&x);    /* Get source value */
                StrToLong(p,&y);    /* Get destination value */

                if ((x>=0) && (x<256) && (y>=0) && (y<256))
                {
                UBYTE t;

                    t=map[x];
                    map[x]=map[y];
                    map[y]=t;
                }

                *--p=':';
            }
        }
        else if (!strnicmp("TRUNCATE=",c,9))
        {
            c+=9;
            StrToLong(c,&trunc);
        }
        rules++;
    }
    return(trunc);
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

void main(int argc, char *argv[])
{
UBYTE map[256];
char *DefaultMatrix[]={ "SWAP=1:2",NULL };
char **rules=DefaultMatrix;
struct DiskObject *myDO=NULL;
struct MsgPort *mport;
struct Window *win;
long trunc;

    if (IntuitionBase=OpenLibrary("intuition.library",36))
    {
        if (GfxBase=OpenLibrary("graphics.library",36))
        {
            if (LayersBase=OpenLibrary("layers.library",36))
            {
                if (IconBase=OpenLibrary("icon.library",36))
                {
                    if (GadToolsBase=OpenLibrary("gadtools.library",36))
                    {
                        if (WorkbenchBase=OpenLibrary("workbench.library",36))
                        {
                            if (argv)
                            {
                                if (argc==0)
                                {
                                struct WBArg *warg=((struct WBStartup *)argv)->sm_ArgList;
                                BPTR lock;

                                    lock=CurrentDir(warg->wa_Lock);
                                    myDO=GetDiskObject(warg->wa_Name);
                                    CurrentDir(lock);

                                    if (myDO) if (myDO->do_ToolTypes) rules=myDO->do_ToolTypes;
                                }
                                else if (argc>1)
                                {
                                int i;

                                    for (i=1;i<argc;i++) argv[i-1]=argv[i];
                                    argv[argc-1]=NULL;
                                    rules=argv;
                                }
                            }

                            trunc=buildmap(rules,map);

                            if (myDO) FreeDiskObject(myDO);

			    {
			    struct Screen *sc;

				if (sc=LockPubScreen(Workbench))
				{
				    ZoomSize[3]=sc->WBorTop+sc->BarHeight-sc->BarVBorder;
				    UnlockPubScreen(NULL,sc);
				}
			    }

                            if (win=OpenWindowTagList(&nw,tags))
                            {
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
                                                    Remap(win,map,warg->wa_Lock,warg->wa_Name,trunc);
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
