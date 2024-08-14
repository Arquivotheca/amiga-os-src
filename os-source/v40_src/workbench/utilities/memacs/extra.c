#include "ed.h"

#if AMIGA
#define TOUPPER(c)      ((c)>='a'&&(c)<='z'?(c)-'a'+'A':(c))

#define BLUE 0
#define WHITE 1
#define BLACK 2
#define RED 3

extern	struct Screen *screen;
extern	struct Window *window;
extern	struct Remember *RememberKey;
extern	int ttrow;
extern	int ttcol;
extern  UBYTE *PointerData;
extern  UBYTE *PrefBuffer;
extern  struct NewScreen ns;
extern	struct NewWindow nw;
extern  struct Menu menus[];
extern  int LaceFlag;
extern int fkinflag;

extern UBYTE *fkm[35];
extern UBYTE *fkmdef[35];

ULONG seconds=0,micros=0;
struct EWINDOW *oldwp=0;
int oldline= (-1),oldgoal= (-1);

struct TextAttr topaz =
        {
        "topaz.font",    /*font name*/
        8,          /* font height*/
        FS_NORMAL,  /*style*/
        FPF_ROMFONT,   /*preferences*/
        };

struct IntuiText ReqText4 ={    /* more text for error-message requester */
    BLUE, WHITE, JAM1,  /* FrontPen, BackPen, DrawMode */
    10, 35,             /* LeftEdge, TopEdge */
    &topaz,          	/* ITextFont */
    "\251Copyright 1986 Commodore-Amiga, Inc.",/* IText */
    NULL                /* NextText */
};  

struct IntuiText ReqText2 ={    /* more text for error-message requester */
    BLUE, WHITE, JAM1,  /* FrontPen, BackPen, DrawMode */
    10, 26,             /* LeftEdge, TopEdge */
    &topaz,          	/* ITextFont */
    "Amiga-ed and enhanced by Andy Finkel.",/* IText */
    &ReqText4             /* NextText */
};  

struct IntuiText ReqText3 ={    /* more text for error-message requester */
    BLUE, WHITE, JAM1,  /* FrontPen, BackPen, DrawMode */
    10, 14,             	/* LeftEdge, TopEdge */
    &topaz,               /* ITextFont */
    "(Placed in the Public Domain)", /* IText */
    &ReqText2           /* NextText */
};

struct IntuiText ReqText1 ={    /* more text for error-message requester */
    BLUE, WHITE, JAM1,  /* FrontPen, BackPen, DrawMode */
    10, 5,             	/* LeftEdge, TopEdge */
    &topaz,               /* ITextFont */
    "Original program by David Conroy.", /* IText */
    &ReqText3          /* NextText */
};

struct IntuiText ReqText = {   /* more text for error-requester */
    BLUE, WHITE, JAM1,  /* FrontPen, BackPen, DrawMode */
    6, 4,               /* LeftEdge, TopEdge */
    &topaz,             	/* ITextFont */
    "OK",           	/* IText */
    NULL                /* NextText */
};  
#endif


int InitFunctionKeys(f,n)
int f,n;
{

int i;
#if AMIGA
for (i=0; i< 35; i++) Lstrcpy(fkm[i],fkmdef[i],80); /* set defaults */
#endif
return(TRUE);
}

int MacroXe(f, n)
    int f,n;
{
         int    c;
         int    af;
         int    an;
         int	flag;
         int    s;

#if AMIGA
 if( *fkm[MacroLevel] == '\0')return(FALSE); /* no definition */
/*
        if (fkdmop!=NULL) {
                mlwrite("Not now");
                return (FALSE);
	}
*/ 
       if (n <= 0)
                return (TRUE);
        do {
	    fkinflag=FALSE;
            fkdmop = fkm[MacroLevel];
	    if ( *fkdmop++ == '?') {
		if (*fkdmop != '?')  {
		    fkinflag=TRUE;
		}
	    }
	    else fkdmop--;

            do {
                af = FALSE;
                an = 1;
                if ((c = *fkdmop++) == '') {
                    af = TRUE; /* we have an argument */
		    an=ctlu(&c);
		}
                flag=0;
                if (c == 0) c = (CTLX|')');

                if (c == METACH ) {
		    flag |= META;
		    if(!(c = *fkdmop++))c = (CTLX|')');
		}
                if (c == 0x18 ) {
		    flag=CTLX|flag;
		    if(!(c = *fkdmop++))c = (CTLX|')');
		}
                if (c < 32) c = (c+'@') | CTRL;
                if (flag) {
                        if (c>='a' && c<='z')c -= 0x20; /* Force to upper */
		}
                c=c|flag;
                s=execute(c , af, an, FALSE);
	    } while ((c != (CTLX|')'))  && (s == TRUE));
            fkdmop = NULL;
	} while ((s==TRUE) && --n);
    MacroLevel=0;
#endif
    return (s);
}

int setkey(f, n)
    int f,n;
{
int c,value,i=0;
UBYTE *kpointer;
UBYTE buffer[128];
UBYTE num[2],d,e,g;

#if AMIGA
memset(&buffer,0,127);

MacroLevel=0;
if (cmdmop) {
    c=cmdgetc(); /* do not get a key if executing a comand line */
}
else {
    strcpy(buffer,"key to define: ");
    mlwrite(buffer);
    c=getkey();
}

if (( MacroLevel< 1 ) || (MacroLevel > 34)) {
	/* maybe they typed f1-f20 or k1-k9 */
	if((c != 'f')&&(c != 'F')&&(c != 'k')&&(c != 'K')&&(c != 'h')&&(c!='H')){
		mlerase();
		return(FALSE);
	}
	num[0]=(UBYTE)c;
	num[1]=0;
	strcat(buffer,num);
	mlreply(buffer,buffer,78);
	i=nwspace(buffer);
	value=atoi(&buffer[i]);
	i=nextarg(&buffer[i]);

	c=TOUPPER(c);
	d=TOUPPER(buffer[0]);
	e=TOUPPER(buffer[1]);
	g=TOUPPER(buffer[2]);

	if((c=='F')&&(value>0)&&(value<=10))MacroLevel=value;
	else if((c=='F')&&(value>10)&&(value<=20))MacroLevel=value+14;

	else if((c=='K')&&(d== '-'))MacroLevel=24;
	else if((c=='K')&&(d== '.'))MacroLevel=22;
	else if((c=='K')&&(d== 'E'))MacroLevel=23;
	else if((c=='K')&&(value>=0)&&(value<=9))MacroLevel=value+12;

	else {
		if((c=='H')&&(d== 'E')&&(e == 'L') && (g=='P'))MacroLevel=11;
	}
	if (( MacroLevel< 1 ) || (MacroLevel > 34)) {
		mlerase();
		return(FALSE);
	}
}

kpointer = fkm[MacroLevel];
MacroLevel=0;

if(!i) {
   strcpy(buffer,kpointer);
   readpattern("def:",buffer);
   if(buffer[0]=='\000')return(FALSE);
}

memset(kpointer,0,79);
strcpy(kpointer,&buffer[i]);
#endif
return(TRUE);
}

#if AMIGA

void TellAboutEmacs()
{ 
AutoRequest(window,&ReqText1,NULL,&ReqText,0,0,410,85);
}

int MouseCursor(message)
struct IntuiMessage *message;
{ 
        int line;       /* where cursor will go */
	struct EWINDOW *wp;
	ULONG cseconds,cmicros;

/* switch to double click */
    if( message->Code == SELECTDOWN ) {
	line=(int)message->MouseY- window->BorderTop;
	line /= FontHeight;
	curgoal=((int)message->MouseX)/FontWidth;

        CurrentTime(&cseconds,&cmicros); /* what time is it ? */
		/* find out which window was clicked in */
	for (wp = wheadp; wp != NULL; wp = wp->w_wndp) {
	    if ((line >= wp->w_toprow) && (line < (wp->w_toprow + wp->w_ntrows)))
		break;
	}
	/* out of window bounds, scroll up or down  */
	if((!wp) || ((int)message->MouseY- window->BorderTop<3)) {
	    if (line==0) line = (-2);
	    wp = curwp;
	    oldline= -10;	/* no mark here, so next test fails */
	}
	if ((oldwp==wp) && (oldline == line) && (oldgoal==curgoal) && 
	    DoubleClick(seconds,micros,cseconds,cmicros))setmark(0,0);
	else mlerase();

	curwp = wp;
	curbp = wp->w_bufp;
	curwp->w_dotp = wp->w_linep;
	curwp->w_doto = 0;

	oldline=line;
	oldgoal=curgoal;
	oldwp=curwp;

	seconds=cseconds;
	micros=cmicros;

	lastflag |= CFCPCN;
	forwline(FALSE, line - curwp->w_toprow);
	curwp->w_flag |= WFHARD;
	update();
    }
return(TRUE);
}

void SetMyPointer()
{
int i,xoff,yoff,offset;

/* needs a test for 32 high pointers here */

GetPrefs((struct Preferences *)PrefBuffer,102);

xoff = (int)((struct Preferences *)PrefBuffer)->XOffset;
yoff = (int)((struct Preferences *)PrefBuffer)->YOffset;

offset=(int)((struct Preferences *)PrefBuffer)->PointerMatrix-(int)PrefBuffer;

for(i=0; i< 72; i++) PointerData[i]=0;

i=0;
while(i<=68) {
    PointerData[i]= PrefBuffer[i+offset+2];
    PointerData[i+1]= PrefBuffer[i+offset+3];
    PointerData[i+2]= PrefBuffer[i+offset];
    PointerData[i+3]= PrefBuffer[i+offset+1];
    i += 4;
}
SetPointer(window,(UWORD *)PointerData,16,16,xoff,yoff);
}

int SetLace(f, n)
int f,n;
{
BUFFER		*bp;
EWINDOW	*wp;
struct Process *process;
int oldrow,oldcol;


if(!f) {	/* toggle interlace */
    if(nw.Type == WBENCHSCREEN)return(FALSE);	/* cannot do it */
    if(LaceFlag)LaceFlag=0;
    else LaceFlag=1;
}

else if (n == 3) { /* toggle to Workbench window */
	if(nw.Type == WBENCHSCREEN) {
	    nw.Type = CUSTOMSCREEN;
	    nw.IDCMPFlags = MENUPICK|MOUSEBUTTONS|INACTIVEWINDOW;
	    nw.Flags=SMART_REFRESH|ACTIVATE|BORDERLESS;
	    nw.Title = NULL;
	    nw.TopEdge=10;
	}
	else {
	    nw.Type = WBENCHSCREEN;
	    nw.IDCMPFlags=CLOSEWINDOW|NEWSIZE|MENUPICK|MOUSEBUTTONS|
		INACTIVEWINDOW;
	    nw.Flags=WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|WINDOWCLOSE|
		SMART_REFRESH|ACTIVATE;
	    nw.Title = ns.DefaultTitle;
	    nw.Screen=NULL;
	    nw.TopEdge=0;
	}
}
else if(n != 4) { /* resize operation is OK */
    if(nw.Type == WBENCHSCREEN)return(FALSE); /* no messing with LACE on a WB */
    if(n == 2) LaceFlag=1; /* turn on interface */
    else if(n==1)LaceFlag=0;
    else return(FALSE);  
}

oldrow=getcline();
oldcol=getccol(FALSE);

onlywind(0,0);
/* wp=curwp; */

(*term.t_close)();

vtfree();

if(n == 4) {
    GetWindowSize();
}
else {
    ClearPointer(window);
    ClearMenuStrip(window);

    process = (struct Process *)FindTask(NULL); /* reset error window */
    process->pr_WindowPtr=NULL;

    if(window)CloseWindowSafely(window);
    if(screen)CloseScreen(screen);

    if(LaceFlag) {
	ns.ViewModes=(USHORT)HIRES|INTERLACE;
    }
    else {
	ns.ViewModes=(USHORT)HIRES;
    }
    GetWindow(LaceFlag);
}
vtinit();

wheadp=curwp;
wp=curwp;

bp=  wp->w_bufp;
curbp  = bp;

	wp->w_wndp  = NULL;                  /* Initialize window    */
        bp->b_nwnd  = 1;                        /* Displayed.           */
        wp->w_linep = bp->b_linep;
        wp->w_dotp  = bp->b_linep;
        wp->w_doto  = 0;
        wp->w_markp = NULL;
        wp->w_marko = 0;
        wp->w_toprow = 0;
        wp->w_ntrows = term.t_nrow-1;           /* "-1" for mode line.  */
        wp->w_force = 0;
        wp->w_flag  = WFMODE|WFHARD|WFFORCE;       /* Full.                */

/* position to same place on screen */
gotobob(0,0);
forwline(TRUE,oldrow);
gotobol(0,0);
forwchar(TRUE,oldcol);

refresh(0,0);
update();
return(TRUE);
}


#endif

int ExecuteFile(f,n,infile)
int f,n;
UBYTE infile[];
{

#if AMIGA
LONG file;
#else
FILE *file;
#endif

UBYTE fname[NFILEN];
UBYTE *filename;
int s,i;
UBYTE buffer[121];

if(f != TRUE) {
    if ((s=mlreply("File: ", fname, NFILEN)) != TRUE) return (s);
    filename=fname;
}
else filename=infile;

if(filexists(filename) == 0) return(FALSE);
#if AMIGA
if((file=Open(filename,MODE_OLDFILE))!= 0) {
#else
if ((file=fopen(filename, "r")) != NULL) {
#endif
	while((s=egetline(file,buffer,120)) == FIOSUC) {
		/* let's try passing 0 as parameters */
	    if(!command(buffer,0,0)) {
		for(i=0; i<strlen(buffer); i++) linsert(1,buffer[i]);
		lnewline();
	    }
	}
#if AMIGA
Close(file);
#else
close(file);
#endif
}
curwp->w_flag |= WFHARD|WFMODE|WFFORCE;
return(TRUE);
}

int egetline(file,buf, nbuf)

#if AMIGA
LONG file;
#else
FILE *file;
#endif
UBYTE buf[];
int nbuf;
{
int    s,i=0;
UBYTE c[2];

#if AMIGA
        while ( (s= (Read(file,c,1) == 1)) && c[0] != '\n') {
#else
        while ( (s= ((c[0]=getc(file)) != EOF)) && c[0] != '\n') {
#endif
		buf[i++] = c[0];
                if (i >= nbuf-1) {
                        buf[i]=0;
                        mlwrite("Execute file has long line");
                        return (FIOSUC);
		}
	}
	buf[i]=0;
	if(!s)return(FIOERR);
        return (FIOSUC);
}
