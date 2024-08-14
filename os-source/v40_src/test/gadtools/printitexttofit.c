;/*
LC -b0 -ms -v -rr -cfistqmc -d1 printitexttofit.c
Blink from lib:c.o printitexttofit.o lib lib:lcr.lib lib:amiga.lib lib:debug.lib
Quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/displayinfo.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/hooks.h>
#include <string.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/asl_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>


extern struct Library *SysBase;
extern struct Library *DOSBase;
       struct Library *GfxBase;
       struct Library *IntuitionBase;
       struct Library *GadToolsBase;
       struct Library *UtilityBase;
       struct Library *AslBase;


#define	TEMPLATE	""
#define	OPT_COUNT	1


#define GTJ_NOCLEAR -1

struct TextAttr topazAttr =
{
    "topaz.font",
     8,
     FS_NORMAL,
     FPF_ROMFONT
};



void cloneRastPort( struct RastPort *clonerp, struct RastPort *rp )
{
    *clonerp = *rp;

    /* Set Mask to -1, APen to 1, DrawMode to JAM1,
     * turn off Area Pattern and Line Pattern:
     */

    SetWriteMask(clonerp,-1);
    SetABPenDrMd(clonerp,1,0,JAM1);
    SetAfPt(clonerp,0,0);
    clonerp->LinePtrn = 0xFFFF;
    BNDRYOFF(clonerp);
}

struct TextFont *ISetFont( struct RastPort *rp, struct TextAttr *font )
{
    struct TextFont *fp = NULL;

    if ((font != NULL) && ((fp = OpenFont(font)) != NULL))
    {
	SetFont(rp, fp);
	SetSoftStyle(rp, font->ta_Style, 0xFF);
    }

    return (fp);
}


VOID ICloseFont(struct TextFont *fp)
{
    if (fp)
	CloseFont(fp);
}

VOID kprintf(STRPTR,...);

VOID printITextToFit(struct RastPort *rp, struct IntuiText *itext,
                     WORD LeftOffset, WORD TopOffset, UWORD Width,
                     UWORD justification)
{
struct RastPort    clonerp;
UWORD              len;
struct TextExtent  dummy;
struct TextFont   *font;
STRPTR             str;
WORD               left;
WORD               top;
WORD               xoffset;
WORD               slen;
WORD               plen, rlen, llen;

    cloneRastPort(&clonerp,rp);

    if (str = itext->IText)
    {
        font = ISetFont(&clonerp,itext->ITextFont);
        left = LeftOffset + itext->LeftEdge;
        top  = TopOffset + itext->TopEdge;
        slen = strlen(str);

        if (justification == GTJ_RIGHT)
        {
            len     = TextFit(&clonerp,str,slen,&dummy,NULL,-1,Width,32767);
            str     = (STRPTR)((ULONG)str + slen - len);
            xoffset = Width - (dummy.te_Extent.MaxX - dummy.te_Extent.MinX + 1);
        }
        else if (justification == GTJ_CENTER)
        {
            TextExtent(&clonerp,str,slen,&dummy);

            /* pixel length of whole string */
            plen = dummy.te_Extent.MaxX - dummy.te_Extent.MinX + 1;

            /* now figure out the middle character (pixel wise) */
            len = TextFit(&clonerp,str,slen,&dummy,NULL,1,plen / 2,32767);

            /* now get how many characters fit in the right side of the display */
            rlen = TextFit(&clonerp,(STRPTR)((ULONG)str+len),slen-len,&dummy,NULL,1,Width / 2,32767);

            /* and finally, the number of chars on the left side */
            llen = TextFit(&clonerp,str,len,&dummy,NULL,-1,Width / 2,32767);

            /* if the number of characters on the left that will be printed
             * is less than the total number of characters on the left side
             * then bump the string pointer
             */
            if (llen < len)
                str = (STRPTR)((ULONG)str + len - llen);

            len = llen + rlen;
            xoffset = (Width / 2) - (dummy.te_Extent.MaxX - dummy.te_Extent.MinX + 1);
        }
        else  /* GTJ_LEFT && GTJ_NOCLEAR */
        {
            len     = TextFit(&clonerp,str,slen,&dummy,NULL,1,Width,32767);
	    xoffset = 0;
        }

	SetABPenDrMd(&clonerp,itext->FrontPen,itext->BackPen,itext->DrawMode);
	Move(&clonerp,left+xoffset,top+clonerp.TxBaseline);
	Text(&clonerp,str,len);

        if (justification != GTJ_NOCLEAR)
        {
            SetAPen(&clonerp,itext->BackPen);

            if (xoffset > 0)
                RectFill(&clonerp,left,top,left+xoffset,top+clonerp.TxHeight-1);

            if (clonerp.cp_x < left+Width-1)
                RectFill(&clonerp,clonerp.cp_x,top,left+Width-1,top+clonerp.TxHeight-1);
        }

        ICloseFont(font);

        rp->cp_x = clonerp.cp_x;
        rp->cp_y = clonerp.cp_y;
    }
}


VOID main(VOID)
{
struct RdArgs *rdargs;
LONG           opts[OPT_COUNT];
struct Window *wp;
struct IntuiMessage *intuiMsg;
struct IntuiText itext;

    IntuitionBase = OpenLibrary("intuition.library", 0);
    GfxBase = OpenLibrary("graphics.library", 0);
    GadToolsBase = OpenLibrary("gadtools.library", 0);
    UtilityBase = OpenLibrary("utility.library", 0);

    memset(opts,0,sizeof(opts));
    if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
    {
        if (wp = OpenWindowTags(NULL,WA_AutoAdjust,  TRUE,
                                     WA_SizeGadget,  TRUE,
                                     WA_DepthGadget, TRUE,
                                     WA_DragBar,     TRUE,
                                     WA_CloseGadget, TRUE,
                                     WA_Activate,    TRUE,
                                     WA_IDCMP,       IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_REFRESHWINDOW,
                                     WA_Width,       275,
                                     WA_Height,      120,
                                     WA_SimpleRefresh, TRUE,
                                     TAG_DONE))
        {
            itext.LeftEdge  = 0;
            itext.TopEdge   = 0;
            itext.FrontPen  = 3;
            itext.BackPen   = 1;
            itext.DrawMode  = JAM2;
            itext.ITextFont = &topazAttr;
            itext.NextText  = NULL;
            itext.IText     = "hello world!";

            printITextToFit(wp->RPort,&itext,20,20,96,GTJ_CENTER);

            while (TRUE)
            {
                WaitPort(wp->UserPort);
                intuiMsg = (struct IntuiMessage *)GetMsg(wp->UserPort);

                if (intuiMsg->Class == IDCMP_REFRESHWINDOW)
                {
                    BeginRefresh(wp);
                    EndRefresh(wp,TRUE);
                }
                else
                {
                    break;
                }
                ReplyMsg(intuiMsg);
            }
        }
        CloseWindow(wp);

        FreeArgs(rdargs);
    }

    CloseLibrary(IntuitionBase);
    CloseLibrary(GfxBase);
    CloseLibrary(GadToolsBase);
    CloseLibrary(UtilityBase);
}
