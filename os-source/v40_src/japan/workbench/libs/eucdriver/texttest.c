#include <stdio.h>
#include <string.h>

#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <graphics/text.h>

struct Library *GfxBase, *DiskfontBase;
struct Library *IntuitionBase;

struct TextAttr ta = { "coral.font", 16, FS_NORMAL, FPF_DISKFONT };
/*struct TextAttr ta = { "coral.font", 16, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED };*/

struct NewWindow nw = {
  0,0,
  600,200,
  0,1,
  IDCMP_CLOSEWINDOW,
  WFLG_DRAGBAR|WFLG_CLOSEGADGET|WFLG_SMART_REFRESH|WFLG_DEPTHGADGET|WFLG_ACTIVATE,
  NULL,
  NULL,
  (UBYTE *)"Coral test",
  NULL,
  NULL,
  10,10,
  1024,1024,
  WBENCHSCREEN
};


int main (int argc,char **argv)
{
    struct TextFont *font = NULL;
    struct RastPort *rp;
    char *str;
    struct Window *win;
    struct IntuiMessage *imsg;
    BOOL loop;
    loop = TRUE;

    if(argc == 2)
    {
	if(!(strcmp(argv[1],"?")))
	{
		printf("TXSPACING YSIZE STRING\n");
		return(0);
	}
    }
    if(argc <= 3)
    {
	printf("Missing required arguments\n");
        return(0);
    }

    if (!(GfxBase = OpenLibrary ("graphics.library", 36))) goto clean;

    ta.ta_YSize = atoi(argv[2]);

    if (!(DiskfontBase = OpenLibrary ("diskfont.library", LIBRARY_MINIMUM))) goto clean;
    if (!(font = OpenDiskFont (&ta))) goto clean;
    if (!(IntuitionBase = OpenLibrary("intuition.library",37))) goto clean;

    if(win=OpenWindow(&nw))
    {
	rp=win->RPort;
        SetFont(rp,font);

	rp->TxSpacing = 0;
	if(argc >= 2)
	{
		rp->TxSpacing = atoi(argv[1]);
	}

	    SetAPen(rp,1L);
	    SetBPen(rp,0L);

	    Move(rp,30,60);
	    Text(rp,argv[3],strlen(argv[3]));

	    Move(rp,30,90);
	    str = argv[3];

            while(*str)
            {
		Text(rp,str,1L);
                str++;
            }
	    while(loop)
	    {
	        Wait(1L<<win->UserPort->mp_SigBit);
	        while(imsg = (struct IntuiMessage *)GetMsg(win->UserPort))
	        {
			if(imsg->Class == IDCMP_CLOSEWINDOW)
			{
				loop = FALSE;
			}
			ReplyMsg((struct Message *)imsg);
	        }

	    }
	CloseWindow(win);
    }

clean:
    if (font) CloseFont (font);
    if (DiskfontBase) CloseLibrary (DiskfontBase);
    if (GfxBase) CloseLibrary (GfxBase);

    return 0;
}
