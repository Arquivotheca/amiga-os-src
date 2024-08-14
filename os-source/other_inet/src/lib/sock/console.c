/*
** Amiga screen handling for rlogin/telnet, etc.
*
*  Modified to open a new screen as an option by MMH 15 Mar 90
*/

#include <exec/types.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>

#ifdef LATTICE
#include <proto/intuition.h>
#include <proto/exec.h>
#endif


extern struct IOStdReq *CreateStdIO();
extern struct MsgPort *CreatePort();
extern struct Library *OpenLibrary();


int cons_kybd_sigF;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
static int console_is_open;

static struct MsgPort *crp, *cwp;
static struct IOStdReq *cw, *cr;
static struct Window *win;
static struct Screen *scr;

static struct NewWindow nw = {
	0, 0, 640, 200, -1, -1, 0, WINDOWDRAG|WINDOWDEPTH|WINDOWSIZING|ACTIVATE,
	0, 0, (UBYTE *)"Console", 0, 0, 640, 100, -1, -1, WBENCHSCREEN
};


/* this is the screen used if screen mode is selected */
static struct NewScreen scrdef = { 0,0,0,0,
	1,	/* bit planes */
	1,0,	/* pens */
	0,CUSTOMSCREEN,NULL,NULL,NULL,NULL
};

static struct NewWindow bbwin = { 0,0,0,0,
	1,0,		/* pens */
	NULL,
	ACTIVATE | NOCAREREFRESH | BORDERLESS | BACKDROP,
	NULL,		/* pointer to first gadget */
	NULL, NULL, NULL, NULL, 0,0,-1,-1,
	CUSTOMSCREEN
};

char
console_getchar(buf, len)
	char	*buf;
	int	len;
{
	static char c;
	char rtn;

	if(!CheckIO((struct IORequest *)cr)){
		WaitIO((struct IORequest *)cr);
	}

	rtn = c;
	cr->io_Command = CMD_READ;
	cr->io_Data = (APTR)&c;
	cr->io_Length = sizeof(c);
	SendIO((struct IORequest *)cr);

	return rtn;
}

int
console_init(title, screen)
	UBYTE	*title;
	int screen;
{
	unsigned rows, cols;
	struct Screen wbscr;

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
	if(!IntuitionBase){
		return -1;
	}
	if (screen) {
		GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
		if(!GfxBase) {
			console_close();
			return -1;
		}
	}

	crp = CreatePort(0L, 0);
	cwp = CreatePort(0L, 0);
	cr = CreateStdIO(crp);
	cw = CreateStdIO(cwp);
	nw.Title = title;
	if (screen) {
		rows = GfxBase->NormalDisplayRows;
		cols = GfxBase->NormalDisplayColumns;
		GetScreenData((char *)&wbscr,sizeof(struct Screen),WBENCHSCREEN,NULL);
		if (wbscr.ViewPort.Modes&LACE)
			rows *=2;
		scrdef.Width = bbwin.Width = cols;
		scrdef.Height = bbwin.Height = rows;
		scrdef.ViewModes = (cols>=640 ? HIRES : 0) | (rows>300 ? LACE : 0);
		scr = (struct Screen *)OpenScreen(&scrdef);
		if (!scr) {
			console_close();
			return -1;
		}
		ShowTitle(scr,(BOOL)0);
		bbwin.Screen = scr;
		win = OpenWindow(&bbwin);
	} else
		win = OpenWindow(&nw);

	if(!cr || !cw || !cwp || !crp || !win){
		console_close();
		return -1;
	}

	cons_kybd_sigF = 1L << crp->mp_SigBit;
	cw->io_Data = (APTR)win;
	cw->io_Length = sizeof(*win);

	if(OpenDevice("console.device", 0, (struct IORequest *)cw, 0)){
		console_close();
		return -1;
	}

	console_is_open++;
	*cr = *cw;
	cr->io_Message.mn_ReplyPort = crp;
	cr->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	console_getchar();
	
	return 0;
}

int 
console_write(buf, len)
	unsigned char *buf;
	int	len;
{
	/* break buf into small pieces */
	while ( len > 80 )
	{
		cw->io_Data = (APTR)buf;
		cw->io_Length = 80;
		cw->io_Command = CMD_WRITE;
		DoIO((struct IORequest *)cw);
		buf += 80;
		len -= 80;
	}
	if (len)
	{
		cw->io_Data = (APTR)buf;
		cw->io_Length = len; 
		cw->io_Command = CMD_WRITE;
		DoIO((struct IORequest *)cw);
	}

	return len;
}

int
console_close()
{
	if(cw) {
		AbortIO((struct IORequest *)cw);
		WaitIO((struct IORequest *)cw);
	}

	if(cr) { 
		AbortIO((struct IORequest *)cr);
		WaitIO((struct IORequest *)cr);
	}

	if(console_is_open){
		CloseDevice((struct IORequest *)cw);
		console_is_open = 0;
	}

	if(crp){
		DeletePort(crp);
		crp = 0;
	}
	if(cwp){
		DeletePort(cwp);
		cwp = 0;
	}
	if(cr){
		DeleteStdIO(cr);
		cr = 0;
	}
	if(cw){
		DeleteStdIO(cw);
		cw = 0;
	}

	if(win){
		CloseWindow(win);
		win = 0;
	}

	if(scr){
		CloseScreen(scr);
		scr=0;
	}

	if(IntuitionBase){
		CloseLibrary(IntuitionBase);
		IntuitionBase = 0;
	}

	if(GfxBase)
		CloseLibrary((struct Library *)GfxBase);

	return 0;
}

void
console_get_window_sizes(width, height, cols, rows)
	int	*width, *height, *cols, *rows;
{
	if(width){
		*width = win->Width;
	}
	if(height){
		*height = win->Height;
	}

	/* for now, assume font 8x8  pixels  */
	if(cols){
		*cols = (int)(win->Width/8-1);
	}
	if(rows){
		*rows = (int)(win->Height/8-2);
	}
}
