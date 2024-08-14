;/* cards.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -j73 cards.c
Blink FROM LIB:wc.o,LIB:wmain.o,cards.o TO cards LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <workbench/startup.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: cards 37.7";
char *Copyright = 
  "cards v37.7\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

/* for wc.o, wmain.o */
BOOL UseMyWbCon = TRUE;
UBYTE  *MyWbCon = "CON:20/20/150/80/Cards © 1990 Commodore-Amiga, Inc.";

/*
 *  Cards.c  ---  Card file displayer by C. Scheppner (c) CBM 1988
 *
 *  ASCII Card file must start with @TEMPLATE as shown, followed
 *  by "cards" each preceded by @ line.  The @N=cardcnt line in the
 *  template is optional.  If not present, defaults to 640 cards.


@TEMPLATE
@N=500
@@@@@@@|@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 ------------------ 68000 Instructions --------------------
 Mnem
 Notes
 Modes
 Sizes
 XNZVC
@ENDTEMPLATE

@
ABCD - Add BCD with extend
src(10) + dest(10) + X -> dest(10)
Dy,Dx    -(Ay),-(Ax)
8        8
*U*U*    *U*U*

 *
 */


int findCard(UBYTE c);
void clearCard(int mode);
void colorCard(void);
void doText(void);
void setButton(ULONG n);
void doButton(ULONG n,ULONG state);
void doSortMark(ULONG n,ULONG state);
void setPens(ULONG state);
void doCard(int n);
int cardToCnum(UBYTE *cardptr);
int cnumToBnum(ULONG cnum);
int keyToBnum(UBYTE c);
void chkmsg(void);
void sortAction(void);
void selectAction(void);
int getCards(void);
void sortOn(ULONG n);
void sortCards(void);
int linecmp(UBYTE *a,UBYTE *b);
UBYTE *getLine(UBYTE *to,UBYTE *from);
int strLen(UBYTE *s);
void cleanexit(UBYTE *s, int e);
int atoI( UBYTE *s );
BOOL strEqu(UBYTE *p, UBYTE *q);
UBYTE toUpper(UBYTE c);

#define HIGHER(a,b) (((a)>(b)) ? (a) : (b))

#define LBUFSZ    160
#define CARDYOFF    4
#define FLIPDELAY   3
#define MAXCARDS    640

#define WHOLE    0
#define CENTER   1

extern struct WBStartup *WBenchMsg;

extern UBYTE *getLine();

struct TextAttr topaz80 =
   {"topaz.font",TOPAZ_EIGHTY,FS_NORMAL,FPF_ROMFONT };

struct TextFont *tf = NULL;

#define STRGID 1
#define STRGH 12
#define STRGW 180


struct IntuiText seaTxt = {2,0,JAM1,-60,0, NULL ,"Search",0};

SHORT BorderVectors4[] = {
    0,0,
    STRGW,0,
    STRGW,STRGH,
    0,STRGH,
    0,0
};

struct Border Border4b = {
    -5,-3,                        /* XY origin relative to TopLeft */
    2,0,JAM1,                     /* front pen, back pen and drawmode */
    5,                            /* number of XY vectors */
    BorderVectors4,               /* pointer to XY vectors */
    NULL                          /* next border in list */
};

struct Border Border4 = {
    -4,-2,                        /* XY origin relative to TopLeft */
    1,0,JAM1,                     /* front pen, back pen and drawmode */
    5,                            /* number of XY vectors */
    BorderVectors4,               /* pointer to XY vectors */
    &Border4b                     /* next border in list */
};

#define UNDOBUFFER NULL
/* UBYTE UNDOBUFFER[80]; */

UBYTE StringBuff[80];

struct StringInfo SearchGadSInfo = {
    StringBuff,         /* buffer where text will be edited */
    UNDOBUFFER,         /* optional undo buffer */
    0,                  /* character position in buffer */
    60,                 /* maximum number of characters to allow */
    0,                  /* first displayed character buffer position */
    0,0,0,0,0,          /* Intuition initialized and maintained variables */
    0,                  /* Rastport of gadget */
    0,                  /* initial value for integer gadgets */
    NULL                /* alternate keymap (fill in if you set the flag) */
};

struct Gadget SearchGad = {
    NULL,
    -(8 + STRGW),-(4+ STRGH),	  /* will adjust top offset after font known */
    STRGW-8,STRGH,
    GRELBOTTOM|GRELRIGHT,
    RELVERIFY|GADGIMMEDIATE,      
    STRGADGET,                    /* String gadget */
    (APTR)&Border4,
    NULL,
    &seaTxt,
    NULL,
    (APTR)&SearchGadSInfo,        /* SpecialInfo structure */
    STRGID,
    NULL
};

UWORD  txHeight, txWidth, txBaseline;
ULONG  maxCards;

ULONG cd_ArrayBytes;
UBYTE *cd_Arrays;
ULONG cd_CharWide;
ULONG cd_CharHigh;
ULONG cd_LeftChars;
ULONG cd_LeftSide;
ULONG cd_LineCnt;
ULONG cd_TextCnt;
UBYTE **cd_Text;
ULONG cd_CardCnt;
UBYTE **cd_SCard;
UBYTE **cd_SLine;
UBYTE ***cd_CLine;
LONG  cd_Error;
UBYTE *cbufmem = NULL, *cbuf;
UBYTE *cardPtr;

char     *filename;
LONG     newLock = 0, startLock = 0, file = 0;
USHORT   winMinX, winMinY, winMaxX, winMaxY;
USHORT   cMinX, cMinY, cMaxX, cMaxY;
USHORT   tMinX, tMinY, tMaxX, tMaxY;
BOOL     Done, FromWb;
int      fsize;


#define  BUTTONON     1
#define  BUTTONOFF    0

#define  CHARBUTTONL  0
#define  BUTTONSPL    0
#define  BUTTON0      1
#define  BUTTON9     10
#define  BUTTONSPM   11
#define  BUTTONA     12
#define  BUTTONZ     37
#define  BUTTONSPR   38
#define  CHARBUTTONR 38
#define  PREVBUTTON  40
#define  NEXTBUTTON  41

/* For reference:       0123456789012345678901234567890123456789012345 */
char     *lobuttons = "\n0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ{<<>>";
char     *hibuttons =  "/0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ~<<>>";
char     *visbuttons = ".0123456789.ABCDEFGHIJKLMNOPQRSTUVWXYZ. <> ";
ULONG    bMinX, bMinY, bMaxX, bMaxY;
ULONG    buttonsWidth, bNum, hbNum, sNum, oldsNum, cNum, oldcNum;

struct Library *IntuitionBase  = NULL;
struct Library *DosBase        = NULL;
struct Library *GfxBase        = NULL;

struct   Window         *win = NULL;  /* for ptr to window structure */
struct   RastPort       *wrp;         /* for ptr to RastPort  */
struct   IntuiMessage   *msg;

struct   NewWindow      nw = {
   0, 0,                                  /* LeftEdge and TopEdge */
   0, 0,                                  /* Width and Height */
   -1, -1,                                /* DetailPen and BlockPen */
					  /* IDCMP Flags*/
   CLOSEWINDOW|MOUSEBUTTONS|VANILLAKEY|GADGETUP,
					  /* Flags */
   WINDOWDEPTH|WINDOWDRAG|WINDOWCLOSE|SMART_REFRESH|ACTIVATE,
   &SearchGad, NULL,                      /* Gadget and Image pointers */
   NULL,                                  /* Title string */
   NULL,                                  /* Screen ptr null till opened */
   NULL,                                  /* BitMap pointer */
   0, 0,                                  /* MinWidth and MinHeight */
   0, 0,                                  /* MaxWidth and MaxHeight */
   WBENCHSCREEN                           /* Type of window */
   };

char *formKey = "@TEMPLATE";
char *formEndKey = "@ENDTEMPLATE";
char *errors[] = {"Unknown",
                  "No @TEMPLATE line",
                  "No @@@@|@@@@ line",
                  "No @@@@|@@@@ line",
                  "Missing labels or @ENDTEMPLATE",
                  "No @ markers before cards",
                  "Not enough memory",
                  "Need higher @N=count after @TEMPLATE"
                  };

LONG confile;
char wbUsage[] =
 "\n\nCardfile required.  Use extend select or doubleclick a cardfile.\n";
char cliUsage[] = "Usage: Cards cardfile\n";
char wbCon[] = "CON:0/40/640/100/Cards v37.7 Copyright 1990 Commodore-Amiga,Inc";

void main(int argc, char **argv)
   {
   struct Screen wbScrData;
   struct WBArg *arg;
   char   sbuf[80];

   FromWb = (argc==0) ? TRUE : FALSE;
   cd_Arrays = NULL;
   cd_Error = 0;

   if((argc > 1)&&(*argv[1] != '?'))
      {
      filename = argv[1];
      }
   else if((FromWb)&&((WBenchMsg->sm_NumArgs) > 1))
      {
      arg = WBenchMsg->sm_ArgList;
      arg++;
      filename = (char *)arg->wa_Name;
      newLock = arg->wa_Lock;
      startLock = CurrentDir(newLock);
      }
   else
      {
      if(FromWb)
         {
         if(confile=Open(wbCon,MODE_OLDFILE))
            {
            Write(confile,wbUsage,strLen(wbUsage));
            Delay(250);
            Close(confile);
            }
          }
      else printf(cliUsage);
      cleanexit("",RETURN_OK);
      }

   /* Open Libraries */
   if(!(IntuitionBase = OpenLibrary("intuition.library",33)))
      cleanexit("Can't open intuition library.\n",RETURN_FAIL);

   if(!(GfxBase = OpenLibrary("graphics.library",0)))
      cleanexit("Can't open graphics library.\n",RETURN_FAIL);


   /* Get screen params */
   if(!(GetScreenData((APTR)&wbScrData,sizeof(struct Screen),WBENCHSCREEN,NULL)))
      cleanexit("Can't get screen params\n",RETURN_FAIL);

   /* fix for 2.0 */
   txHeight = ((struct GfxBase *)GfxBase)->DefaultFont->tf_YSize;
   txWidth  = ((struct GfxBase *)GfxBase)->DefaultFont->tf_XSize;
   txBaseline = ((struct GfxBase *)GfxBase)->DefaultFont->tf_Baseline;
   SearchGad.TopEdge -= txHeight;
   seaTxt.LeftEdge = -((strlen("Search") + 2) * txWidth);

   /* Open file */
   if(!(file = Open(filename,MODE_OLDFILE)))
      cleanexit("Can't open card file\n",RETURN_WARN);

   Seek(file,0,OFFSET_END);
   if((fsize = Seek(file,0,OFFSET_BEGINING)) <= 0)
      cleanexit("Empty file\n",RETURN_WARN);

   if(!(cbufmem = (UBYTE *)AllocMem(fsize + 4,MEMF_PUBLIC|MEMF_CLEAR)))
      cleanexit("Can't allocate buffer\n",RETURN_FAIL);

   cbuf = cbufmem;

   if(Read(file,cbuf,fsize) <= 0)
      cleanexit("Can't read card file\n",RETURN_WARN);

   cNum = bNum = hbNum = sNum = 0;
   oldcNum = oldsNum = ~0;

   if(!(getCards()))
      {
      sprintf(sbuf,"Can't process card file, error: %s\n",errors[cd_Error]);
      cleanexit(sbuf,RETURN_WARN);
      }
   Close(file);
   file = NULL;

   /* Set up window size */
   nw.Width    = ((cd_CharWide) * txWidth)
			+ wbScrData.WBorLeft + wbScrData.WBorRight;
   nw.Height   = ((cd_CharHigh + 2) * txHeight)
			+ wbScrData.BarHeight + wbScrData.WBorBottom + STRGH + 8;
   nw.LeftEdge = (wbScrData.Width - nw.Width) >> 1;
   nw.TopEdge  = (wbScrData.Height - nw.Height) >> 1;
   nw.Title    = filename;

   /* Open Window */

   if(!(win = (struct Window *)OpenWindow(&nw)))
      cleanexit("Can't open window - a card line may be too long.\n",RETURN_WARN);


   wrp = win->RPort;
/*
   if(tf = OpenFont(&topaz80)) SetFont(wrp,tf);
*/
   winMinX = win->BorderLeft;
   winMinY = win->BorderTop;
   winMaxX = win->Width - win->BorderRight;
   winMaxY = win->Height - win->BorderBottom - 1;

   /* Button string area bounds */
   buttonsWidth = strLen(visbuttons) * txWidth;
   bMinX = winMinX + ((win->Width - buttonsWidth) >>1 );
   bMinY = winMaxY - (txHeight + 2);
   bMaxX = bMinX + buttonsWidth;
   bMaxY = winMaxY;

   /* Card interior bounds */
   cMinX = winMinX + cd_LeftSide;
   cMinY = winMinY + txHeight + 2;
   cMaxX = winMaxX;
   cMaxY = bMinY;

   /* Left-hand highlighted text area bounds */
   tMinX = winMinX;
   tMinY = winMinY + (txHeight << 1) + CARDYOFF;
   tMaxX = winMinX + cd_LeftSide;
   tMaxY = tMinY + (txHeight * (cd_TextCnt - 1));

   doText();

   /* Sort cards on first line and display */
   sortOn(sNum=0);
   doSortMark(sNum,BUTTONON);
   oldcNum = ~0;
   doCard(cNum=0);
   bNum = cnumToBnum(cNum);
   doButton(bNum,BUTTONON);
   hbNum = bNum;

   ActivateGadget(&SearchGad,win,NULL);

   Done = FALSE;
   while(!Done)
      {
      Wait(1<<win->UserPort->mp_SigBit);
      chkmsg();
      }
   cleanexit("",RETURN_OK);
   }

/* find card where this letter starts */
int findCard(UBYTE c)
   {
   int k;

   for(k=0; (k<cd_CardCnt)
              &&(toUpper(cd_SLine[k][0]) < toUpper(c)); k++);
   if(k==cd_CardCnt)
      {
      k--;
      }
   return(k);
   }


void clearCard(int mode)
   {
   SetAPen(wrp,0);
   SetDrMd(wrp,JAM1);

   if(mode==CENTER)
      RectFill(wrp,cMinX,cMinY+1,cMaxX,cMaxY-1);
   else
      RectFill(wrp,winMinX,winMinY,winMaxX,winMaxY);
   RefreshGList(&SearchGad,win,NULL,1);
   }

void colorCard()
   {
   SetAPen(wrp,1);
   SetDrMd(wrp,JAM1);
   RectFill(wrp,winMinX,winMinY,winMaxX,winMinY + txHeight + 2);
   RectFill(wrp,winMinX,winMinY,winMinX + cd_LeftSide,winMaxY);
   RectFill(wrp,winMinX,bMinY,winMaxX,bMaxY);
   }

void doText()
   {
   int x, y, w, h;
   ULONG k;
   char lbuf[LBUFSZ];

   clearCard(WHOLE);
   colorCard();

   SetAPen(wrp,0);
   SetDrMd(wrp,JAM1);

   w = txWidth;
   h = txHeight;
   x = winMinX;
   y = winMinY + txHeight;

   for(k=0; k<cd_TextCnt; k++, y+=h)
      {
      getLine(lbuf,cd_Text[k]);
      Move(wrp,x, k ? y + CARDYOFF : y);
      Text(wrp,lbuf,strLen(lbuf));
      }
   Move(wrp,bMinX,bMinY + txBaseline + 2);
   Text(wrp,visbuttons,strLen(visbuttons));
   }

void setButton(ULONG n)
   {
   if(n != hbNum)
      {
      doButton(hbNum,BUTTONOFF);
      doButton(n,BUTTONON);
      hbNum = n;
      }
   }

void doButton(ULONG n,ULONG state)
   {
   setPens(state);
   Move(wrp,bMinX + (n * txWidth),bMinY + txBaseline + 2);
   Text(wrp,&visbuttons[n],1);
   }

void doSortMark(ULONG n,ULONG state)
   {
   setPens(BUTTONOFF);     /* No reverse for sort mark */

   Move(wrp,tMinX, tMinY + (n * txHeight));
   Text(wrp,(state==BUTTONON) ? ">" : " ",1);
   }

void setPens(ULONG state)
   {
   int pena, penb;

   pena = (state==BUTTONON) ? 1 : 0;
   penb = (state==BUTTONON) ? 0 : 1;

   SetAPen(wrp,pena);
   SetBPen(wrp,penb);
   SetDrMd(wrp,JAM2);
   }


void doCard(int n)
   {
   UBYTE *cbp;
   int  x, y, w, h;
   char lbuf[LBUFSZ];

   if(n==oldcNum) return;

   oldcNum = n;

   clearCard(CENTER);

   SetAPen(wrp,1);
   SetDrMd(wrp,JAM1);

   w = txWidth;
   h = txHeight;

   x = winMinX + cd_LeftSide + txWidth;
   y = winMinY + (txHeight << 1);

   
   cardPtr = cd_SCard[n];	/* to position on same card after a sort */

   cbp = cd_SCard[n];
   while((cbp=getLine(lbuf,cbp))&&(*cbp != '@'))
      {
      Move(wrp,x, y + CARDYOFF);
      Text(wrp,lbuf,strLen(lbuf));
      y += txHeight;
      }
   }


int cardToCnum(UBYTE *cardptr)
   {
   int k;

   for(k=0; k<cd_CardCnt; k++)
	{
	if((cd_SCard[k])==(cardptr))	break;
	}
   if(k == cd_CardCnt)	k=0;
   return(k);		
   }

int cnumToBnum(ULONG cnum)
   {
   UBYTE c;

   c = toUpper(cd_SLine[cNum][0]);
   return(keyToBnum(c));
   }

int keyToBnum(UBYTE c)
   {
   int k;

   for(k=CHARBUTTONL; (k<=CHARBUTTONR)&&(c > hibuttons[k]); k++);
   return((k>CHARBUTTONR) ? CHARBUTTONR : k);
   }

void chkmsg()
   {
   ULONG class, code;
   SHORT mouseX, mouseY;

   while(msg = (struct IntuiMessage *)GetMsg(win->UserPort))
      {
      class = msg->Class;
      code  = msg->Code;
      mouseX = msg->MouseX;
      mouseY = msg->MouseY;

      ReplyMsg(msg);
      switch(class)
         {
         case MOUSEBUTTONS:
            if(code==SELECTDOWN)
               {
               /* Sorting */
               if((mouseX >= tMinX)&&(mouseX < tMaxX)
                 &&(mouseY >= (tMinY-txBaseline))
                 &&(mouseY <= (tMaxY-txBaseline)))
                  {
                  sNum = (mouseY - (tMinY-txBaseline)) / txHeight;
                  sortAction();
                  break;
                  }
               /* Selecting */
               else do
                  {
                  if((mouseX >= bMinX)&&(mouseX < bMaxX)
                    &&(mouseY >= bMinY)&&(mouseY < bMaxY))
                     {
                     bNum = (mouseX - bMinX) / txWidth;

                     /* Adjust if hit on a space */
                     if(bNum==PREVBUTTON-1)  bNum++;
                     if(bNum==NEXTBUTTON+1)  bNum--;

                     selectAction();
                     }
                  WaitTOF();
                  mouseX = win->MouseX;
                  mouseY = win->MouseY;
                  }
               while(!(msg=(struct IntuiMessage *)GetMsg(win->UserPort)));

               ReplyMsg(msg);
               }
            break;

         case VANILLAKEY:
            switch(code)
               {
               case 0x03:
                  Done = TRUE;
                  break;
               /* Sorting */
               case 'U': case 'D':
                  sNum = (code=='U') ? oldsNum - 1 : oldsNum + 1;
                  sortAction();
                  break;
               /* Selecting */ 
               case '<':  case ',':   case '>':  case '.':
                  bNum=((code=='<')||(code==',')) ? PREVBUTTON : NEXTBUTTON;
                  selectAction();
                  break;
	       /* activate the string gadget */
	       case 0x09: case 0x0a: case 0x0d:
		  ActivateGadget(&SearchGad,win,NULL);   
		  break;
               default:
                  bNum = keyToBnum(toUpper((UBYTE)code));
                  selectAction();
                  break;
               }   
            break;

         case GADGETUP:	/* we only have the one gadget */
	    bNum = 999;
	    selectAction(); 
            break;

         case CLOSEWINDOW:
            Done = TRUE;
            break;
         default:
            break;
         }
      }
   }


/* sNum = sort line number */
void sortAction()
   {
   UBYTE *precard;

   precard = cd_SCard[cNum];
 
   if((sNum != oldsNum)&&(sNum < cd_LineCnt))
      {
      doSortMark(oldsNum,BUTTONOFF);
      sortOn(sNum);
      doSortMark(sNum,BUTTONON);

      oldcNum = ~0;
      doCard(cNum=cardToCnum(precard));

      bNum = cnumToBnum(cNum);
      setButton(bNum);
      }
   }

/* find card number closest to search string */
int searchCard(UBYTE *ss)
   {
   int k,l,ll,n;

   if(!(ss[0]))	return(0);

   l = strLen(ss);

   if(!(strnicmp(ss,"bix:",4)))
	{
   	for(k=0; k<cd_CardCnt; k++)
	    {
	    for(ll=0; cd_SLine[k][ll] != '\n'; ll++);
	    if(ll > l)
		{
		ll = ll - l;
		n=strnicmp(ss,&cd_SLine[k][ll],l);
		}
	    if(SetSignal(0,0) & SIGBREAKF_CTRL_C)  break;
	    if(n == 0) break;
	    }
	}
   else
	{
   	for(k=0; k<cd_CardCnt; k++)
	    {
	    n=strnicmp(ss,cd_SLine[k],l);
	    if(n > 0) continue;
	    else break;
	    }
	}

   if(n)  DisplayBeep(win->WScreen);

   if(k==cd_CardCnt)	k--;

   return(k);
   }

/* entered with bNum = index line button number OR 999 for search*/
void selectAction()
   {
   ULONG k;

   if(bNum == 999)
      {
      doCard(cNum = searchCard(StringBuff));
      bNum = cnumToBnum(cNum);
      }
   else if((bNum==PREVBUTTON)&&(cNum))
      {
      doCard(--cNum);
      Delay(FLIPDELAY);
      }
   else if((bNum==NEXTBUTTON)&&(cNum<(cd_CardCnt-1)))
      {
      doCard(++cNum);
      Delay(FLIPDELAY);
      }
   else if((bNum >= CHARBUTTONL)&&(bNum <= CHARBUTTONR))
      {
      doCard(cNum = (findCard(lobuttons[bNum])));
      }

   /* Button highlighting */
   if((bNum==NEXTBUTTON)||(bNum==PREVBUTTON))
      {
      doButton(bNum,BUTTONON);
      Delay(FLIPDELAY);
      doButton(bNum,BUTTONOFF);
      k = cnumToBnum(cNum);
      }
   else k = bNum;

   setButton(k);
   }

int getCards()
   {
   ULONG utmp;
   UBYTE *tp1, *tp2, *oldcbuf;
   int k, error, ccnt = 0, tcnt = 0, lcnt = 0, line = 0;
   char lbuf[LBUFSZ];

   error = ccnt = 0;
   while((cbuf=getLine(lbuf,cbuf))&&(!(strEqu(lbuf,formKey))));

   if(!(strEqu(lbuf,formKey)))
      {
      error = 1;
      goto getCardsExit;
      }

   /* Next line is optional @N= line or card size line */
   if(!(cbuf=getLine(lbuf,cbuf)))
      {
      error = 2;
      goto getCardsExit;
      }
 
   /* new dynamic maxCards calculation */
   for(maxCards=k=0; cbuf[k]; k++)	if(cbuf[k]=='@') maxCards++;
   maxCards = maxCards + 8;

   if(((lbuf[1] | 0x20) == 'n')&&(lbuf[2]=='='))  /* maxCards line */
      {
      /* no longer needed
      maxCards = atoI(&lbuf[3]);
      if(!maxCards)  maxCards = MAXCARDS;
      */

      /* get card size (next) line */
      if(!(cbuf=getLine(lbuf,cbuf)))
         {
         error = 3;
         goto getCardsExit;
         }
      }

   k = cd_CharWide = HIGHER(strLen(lbuf),strLen(visbuttons)+4);
   /* Find the | marking offset for card data */
   for( ; (k)&&(lbuf[k]!='|'); k--);
   cd_LeftChars = k;
   cd_LeftSide = k * txWidth;

   /* Start of Text lines */
   tp1 = cbuf;
   do
      {
      if(!(cbuf=getLine(lbuf,cbuf)))
         {
         error = 4;
         goto getCardsExit;
         }
      else
         {
         tcnt++;
         }
      }
   while(!(strEqu(lbuf,formEndKey)));

   tcnt--;
   cd_TextCnt = tcnt;
   cd_CharHigh = tcnt;
   cd_LineCnt = lcnt = tcnt -1;

   /* Move to start of first card */
   while(*cbuf != '@')
      {
      cbuf++;
      if(!*cbuf)
         {
         error = 5;
         goto getCardsExit;
         }
      }
   tp2 = cbuf;

   /* cd_Text[] text ptr array,
    * cd_CLine[line][card] of ptrs to card lines
    * and SLine[] and SCard[] arrays for sorting
    */

   cd_ArrayBytes = (tcnt + lcnt + (maxCards * (lcnt + 2))) << 2;
   if(!(cd_Arrays =
     (UBYTE *)AllocMem(cd_ArrayBytes,MEMF_PUBLIC|MEMF_CLEAR)))
      {
      error = 6;
      goto getCardsExit;
      }

   cd_Text = (UBYTE **)cd_Arrays;
   cd_SLine= (UBYTE **)((ULONG)cd_Text +  (tcnt << 2));
   cd_SCard= (UBYTE **)((ULONG)cd_SLine + (maxCards << 2));
   cd_CLine= (UBYTE ***)((ULONG)cd_SCard + (maxCards << 2));
   /* 2-dim array */
   utmp =  (ULONG)((ULONG)cd_CLine) + (lcnt << 2);
   for(k=0; k<lcnt; k++, utmp += (maxCards<<2))
      {
      cd_CLine[k] = (UBYTE **)utmp;
      }


   /* Set up ptrs to text strings */
   for(k=0, cbuf=tp1; k<tcnt; k++)
      {
      cd_Text[k] = cbuf;
      cbuf = getLine(lbuf,cbuf);
      }


   /* Read card lines and store pointers */
   cbuf = getLine(lbuf,tp2);
   line = 0;  /* ccnt initialized above to 0 (used as return flag) */ 
   while((cbuf=getLine(lbuf,oldcbuf=cbuf))&&(ccnt < maxCards))
      {
      if(*lbuf != '@')
         {
         if((k=(((ULONG)cbuf-(ULONG)oldcbuf) + cd_LeftChars)) > cd_CharWide)
            cd_CharWide = k + cd_LeftChars;
         if(line < lcnt)
            {
            cd_CLine[line][ccnt] = oldcbuf;
            line++;
            }
         }
      else  ccnt++, line = 0;
      }
   if(ccnt < maxCards)  cd_CardCnt = ++ccnt;
   else
      {
      ccnt = 0;
      error = 7;
      }

   getCardsExit:
   cd_Error = error;
   return(ccnt);
   }


void sortOn(ULONG n)
   {
   if(n==oldsNum) return;
   oldsNum = n;

   /* Copy card line 0 ptrs to SCard[] specified line ptrs to SLine[] */
   CopyMem(cd_CLine[0],cd_SCard,(cd_CardCnt << 2));
   CopyMem(cd_CLine[n],cd_SLine,(cd_CardCnt << 2));

   sortCards();
   }

/* Sorts cd_SLine and cd_SCard based on cd_SLine */
void sortCards()
   {
   UWORD i, j, h, N;
   UBYTE *tmpL, *tmpC, **slp, **scp;

   /* ShellSort the SLine ptrs and swap SCard ptrs to same sort */
   N = cd_CardCnt;
   h = 1;
   while(h <= N)  h = 3 * h + 1;

   /* cheat by using ptrs backed up one element */
   slp = cd_SLine - 1;
   scp = cd_SCard - 1;

   do
      {
      h = h / 3;
      for(i=h+1; i<=N; i++)
         {
         tmpL = slp[i];
         tmpC = scp[i];
         j = i;
         while(linecmp(slp[(j-h)],tmpL) > 0)
            {
            slp[j] = slp[(j-h)];
            scp[j] = scp[(j-h)];
            j = j - h;
            if(j <= h)  break;
            }
         slp[j] = tmpL;
         scp[j] = tmpC;
         }
      }
   while(h>1);
   }


/* like strcmp but non-case sensitive and LF terminated */
/*
int linecmp(UBYTE *a,UBYTE *b)
   {
   while(toUpper(*a)==toUpper(*b))
      {
      if(*a++ == '\n')  return(0);
      b++;
      }
   if(toUpper(*a) < toUpper(*b)) return(-1);
   return(1);
   }
*/

int linecmp(UBYTE *a,UBYTE *b)
   {
   int l;
   for(l=0; a[l] != '\n'; l++);
   return(strnicmp(a,b,l));
   }

UBYTE *getLine(UBYTE *to,UBYTE *from)
   {
   UBYTE *start;
   ULONG  k;

   start = to;
   if(*from=='\0')
      {
      *to = *from;
      return((UBYTE *)NULL);
      }

   k = LBUFSZ-1;
   do
      {
      *to++ = *from;
      }
   while((*from)&&(*from++ != '\n')&&(--k));
   to--;
   if((ULONG)to == (ULONG)start)
      {
      *to = ' ';
      to++;
      }
   *to = '\0';
   return(from);
   }

int strLen(UBYTE *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }


void cleanexit(UBYTE *s, int e)
   {
   if(*s)
      {
      if(FromWb)
         {
         if(confile=Open(wbCon,MODE_OLDFILE))
            {
            Write(confile,s,strLen(s));
            Delay(250);
            Close(confile);
            }
         }
      else printf(s);
      } 

   if(cbufmem)    FreeMem(cbufmem,fsize + 4);
   if(cd_Arrays)  FreeMem(cd_Arrays,cd_ArrayBytes);
   if(startLock)  CurrentDir(startLock);

   if(win)
      {
      while(msg = (struct IntuiMessage *)GetMsg(win->UserPort))
         {
         ReplyMsg(msg);
         }
      CloseWindow(win);
      }

   if (tf)   CloseFont(tf);
   if (file) Close(file);
   if (GfxBase) CloseLibrary(GfxBase);
   if (IntuitionBase) CloseLibrary(IntuitionBase);
   exit(e);
   }


int atoI( UBYTE *s )
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }



BOOL strEqu(UBYTE *p, UBYTE *q)
   { 
   while(toUpper(*p) == toUpper(*q))
      {
      if (*(p++) == 0)  return(TRUE);
      ++q; 
      }
   return(FALSE);
   } 

UBYTE toUpper(UBYTE c)
   {
   UBYTE u = c;
   if(((u>='a')&&(u<='z'))||((u>=0xe0)&&(u<=0xfe))) u = u & 0xDF;
   return(u);
   }

/* end */
