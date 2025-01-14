#include "configopts.h"
USHORT __chip CycImage[] = { 0x0000,  /* .... .... .... .... */ 0,
                             0x07f0,  /* .... .*** **** .... */ 0,
                             0x0c18,  /* .... **.. ...* *... */ 0,
                             0x0c18,  /* .... **.. ...* *... */ 0,
                             0x0c7e,  /* .... **.. .*** ***. */ 0,
                             0x0c3c,  /* .... **.. ..** **.. */ 0,
                             0x0c18,  /* .... **.. ...* *... */ 0,
                             0x0c00,  /* .... **.. .... .... */ 0,
                             0x0c18,  /* .... **.. ...* *... */ 0,
                             0x07f0,  /* .... .*** **** .... */ 0,
                             0x0000,  /* .... .... .... .... */ 0,
                             0x0000,  /* .... .... .... .... */ 0,
                             0x0000,  /* .... .... .... .... */ 0
                           };
USHORT __chip ChkImage[] = { 0x0000,  /* .... .... .... .... */
                             0x001c,  /* .... .... ...* **.. */
                             0x0030,  /* .... .... ..** .... */
                             0x0060,  /* .... .... .**. .... */
                             0x70c0,  /* .*** .... **.. .... */
                             0x3980,  /* ..** *..* *... .... */
                             0x1f00,  /* ...* **** .... .... */
                             0x0e00,  /* .... ***. .... .... */
                             0x0000   /* .... .... .... .... */
                           };
USHORT __chip UpImage[]  = { 0x0000,  /* .... .... .... .... */
                             0x0600,  /* .... .**. .... .... */
                             0x0f00,  /* .... **** .... .... */
                             0x0d80,  /* .... **.* *... .... */
                             0x1980,  /* ...* *..* *... .... */
                             0x30c0,  /* ..** .... **.. .... */
                             0x0000   /* .... .... .... .... */
                           };
USHORT __chip DnImage[]  = { 0x0000,  /* .... .... .... .... */
                             0x30c0,  /* ..** .... **.. .... */
                             0x1980,  /* ...* *..* *... .... */
                             0x0d80,  /* .... **.* *... .... */
                             0x0f00,  /* .... **** .... .... */
                             0x0600,  /* .... .**. .... .... */
                             0x0000   /* .... .... .... .... */
                           };
static WORD checker[2] = { 0xAAAA, 0x5555 };
static WORD hchecker[2] ={ 0x8888, 0x2222 };
static WORD solid = 0xFFFF;

void rendertext(rp, text, key, ulx, uly)
struct RastPort *rp;
char *text;
int key;
int ulx, uly;
{
   int pos;

   SetAPen(rp, 1);
   Move(rp, (WORD)ulx, (WORD)(uly+rp->TxBaseline));
   Text(rp, text, (WORD)strlen(text));

   if (key != '~')
   {
      pos = 0;
      while (text[pos] && ((text[pos] | 0x20) != key)) pos++;
      ulx += TextLength(rp, text, (WORD)pos);

      Move(rp, (WORD)ulx, (WORD)(uly+8));
      Draw(rp, (WORD)(ulx+8), (WORD)(uly+8));
   }
}

void drawbox(rp, ulx, uly, lrx, lry, pen1, pen2)
struct RastPort *rp;
int ulx, uly, lrx, lry, pen1, pen2;
{
   SetAPen(rp, (UBYTE)pen1);
   Move(rp, (WORD)ulx, (WORD)uly);      Draw(rp, (WORD)ulx, (WORD)lry);
   Move(rp, (WORD)(ulx+1), (WORD)(lry-1));
            Draw(rp, (WORD)(ulx+1), (WORD)uly);
            Draw(rp, (WORD)(lrx-1), (WORD)uly);

   SetAPen(rp, (UBYTE)pen2);
   Move(rp, (WORD)lrx, (WORD)lry);      Draw(rp, (WORD)lrx, (WORD)uly);
   Move(rp, (WORD)(lrx-1), (WORD)(uly+1));
            Draw(rp, (WORD)(lrx-1), (WORD)lry);
            Draw(rp, (WORD)(ulx+1), (WORD)lry);
}

/*
    <-22--><----------150----------->
   ++++++++++++++++++++++++++++++++++# ^
   ++ /-\ #+                        ## |
   ++ | V #+                        ##14
   ++ \_/ #+                        ## |
   +################################## v
*/
#define IMAGEWIDTH 22
void rendercyc(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct cycdata *cd;
   struct RastPort *rp = global->rp;
   struct Image image;
   char *p;
   int i;

   cd = (struct cycdata *)gadget->UserData;

   ulx = gadget->LeftEdge;      uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+gadget->Height-1;

   SetDrMd(rp, JAM1);
   if (global->reset != 1)
   {
      SetAPen(rp, 0);
      RectFill(rp, (WORD)(ulx+IMAGEWIDTH), (WORD)(uly+1),
                   (WORD)(lrx-2),          (WORD)(lry-1));
   }

   drawbox(rp, ulx, uly, lrx, lry, global->highlight, global->shadow);

   SetAPen(rp, global->highlight);
   Move(rp, (WORD)(ulx+IMAGEWIDTH-1), (WORD)(uly+2));
            Draw(rp, (WORD)(ulx+IMAGEWIDTH-1), (WORD)(lry-2));

   SetAPen(rp, global->shadow);
   Move(rp, (WORD)(ulx+IMAGEWIDTH-2), (WORD)(uly+2));
            Draw(rp, (WORD)(ulx+IMAGEWIDTH-2), (WORD)(lry-2));

   SetAPen(rp, 1);
   memset(&image, 0, sizeof(image));
   image.Width = IMAGEWIDTH-4;
   image.Height = 12;
   image.Depth = 1;
   image.PlanePick = 1;
   image.ImageData = CycImage;
   DrawImage(rp, &image, ulx+2, uly+1);

   p = cd->string;
   i = global->states[cd->slot];
   while (i-- > 0) p += strlen(p)+1;

   i = strlen(p);
   ulx += IMAGEWIDTH;
   ulx += (lrx-ulx-TextLength(rp, p, (WORD)i))/2;
   rendertext(rp, p, cd->key, ulx, uly+4);
#if 0
   Move(rp, ulx, (WORD)(uly+4+rp->TxBaseline));
   Text(rp, p, (WORD)i);
#endif
}

void renderchk(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct chkdata *cd;
   struct RastPort *rp = global->rp;
   struct Image image;
   char *p;

   cd = (struct chkdata *)gadget->UserData;

   ulx = gadget->LeftEdge;    uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+gadget->Height-1;

   if ((!global->reset) || (!global->states[cd->slot]))
   {
      SetAPen(rp, 0);
      RectFill(rp, (WORD)(ulx+2),  (WORD)(uly+1),
                   (WORD)(lrx-2),  (WORD)(lry-1));
   }

   drawbox(rp, ulx, uly, lrx, lry, global->highlight, global->shadow);

   SetAPen(rp, 1);
   if (global->states[cd->slot])
   {
      memset(&image, 0, sizeof(image));
      image.Width = 16;
      image.Height = 9;
      image.Depth = 1;
      image.PlanePick = 1;
      image.ImageData = ChkImage;
      DrawImage(rp, &image, ulx+6, uly+1);
   }

   p = cd->string;
#if 0
   Move(rp, (WORD)(lrx+4), (WORD)(uly+2+rp->TxBaseline));
   Text(rp, p, (WORD)(strlen(p)));
#endif
   rendertext(rp, p, cd->key, lrx+4, uly+2);
}

void renderbtn(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   int i;
   struct chkdata *cd;
   struct RastPort *rp = global->rp;
   char *p;

   cd = (struct chkdata *)gadget->UserData;

   ulx = gadget->LeftEdge;      uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+gadget->Height-1;
   SetAPen(rp, 0);
   RectFill(rp, (WORD)(ulx+2),  (WORD)(uly+1),
                (WORD)(lrx-2),  (WORD)(lry-1));

   drawbox(rp, ulx, uly, lrx, lry, global->highlight, global->shadow);

   SetAPen(rp, 1);

   p = cd->string;
   i = strlen(p);
   ulx += (lrx-ulx-TextLength(rp, p, (WORD)i))/2;
   rendertext(rp, p, cd->key, ulx, uly+3);
}

void renderstrnum(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct chkdata *cd;
   struct RastPort *rp = global->rp;
   char *p;

   cd = (struct chkdata *)gadget->UserData;

   ulx = gadget->LeftEdge;      uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+gadget->Height+1;

   drawbox(rp, ulx-4, uly-3, lrx+4, lry+1, global->highlight, global->shadow);
   drawbox(rp, ulx-2, uly-2, lrx+2, lry, global->shadow, global->highlight);

   SetAPen(rp, 1);

   p = cd->string;
   if (*p)
   {
      Move(rp, (WORD)(ulx-105), (WORD)(uly+2+rp->TxBaseline));
      Text(rp, p, (WORD)(strlen(p)));
   }
}

void renderbox(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct RastPort *rp = global->rp;
   int adj;
   struct Image image;

   adj = 0;
   SetAPen(rp, 1);
   memset(&image, 0, sizeof(image));
   image.Width = 12;
   image.Height = 7;
   image.Depth = 1;
   image.PlanePick = 1;

   ulx = gadget->LeftEdge;        uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;     lry = uly+gadget->Height-1;

   SetDrMd(rp, JAM1);
   if (!global->reset)
   {
      SetAPen(rp, 0);
      RectFill(rp, ulx, uly,
                   (WORD)(ulx+gadget->Width-1), (WORD)(uly+gadget->Height-1));
   }
   SetAPen(rp, 1);

   switch(gadget->GadgetID)
   {
      case SCROLL_V:
         ulx -= 3;          lrx += 3;
         uly -= 2;          lry += 2;
         break;
      case UP_V:
         image.ImageData = UpImage;
         DrawImage(rp, &image, ulx+2, uly+1);
         break;
      case DOWN_V:
         image.ImageData = DnImage;
         DrawImage(rp, &image, ulx+2, uly+1);
         break;
      case NEW_V:
         Move(rp, (WORD)(ulx+4), (WORD)(uly+4+rp->TxBaseline));
         Text(rp, "ADD", 3);
         break;
      case DEL_V:
         Move(rp, (WORD)(ulx+4), (WORD)(uly+4+rp->TxBaseline));
         Text(rp, "DEL", 3);
         break;
   }
   SetDrMd(rp, JAM2);

   drawbox(global->rp, ulx, uly, lrx, lry, global->highlight, global->shadow);
}

void renderlist(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct chkdata *cd;
   int i, j;
   WORD len;
   struct RastPort *rp = global->rp;
   char *p;

   cd = (struct chkdata *)gadget->UserData;

   ulx = gadget->LeftEdge;      uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+(3*gadget->Height)-1;

   SetAPen(rp, 1);
   Move(rp, (WORD)ulx, (WORD)(uly-12+rp->TxBaseline));
   p = cd->string;
   Text(rp, p, (WORD)(strlen(p)));

   /* Now we want to render the items (if any) */
   i = cd->slot;
   i += global->strtab[i].value+1;
   if (!global->reset)
   {
      SetAPen(rp, 0);
      RectFill(rp, ulx, uly, lrx, lry);
   }
   SetAPen(rp, 1);
   for(j = 0; j < 3; j++, i++)
   {
      if ((p = global->strtab[i].text) == NULL) break;
      Move(rp, (WORD)ulx, (WORD)(uly+2+(11*j)+rp->TxBaseline));
      len = strlen(p);
      if (len > 40) len = 40;
      Text(rp, p, len);
   }
   drawbox(global->rp, ulx-2, uly-1, lrx+2, lry+1, global->highlight, global->shadow);
}

void rendernlist(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   USHORT ulx, uly, lrx, lry;
   struct chkdata *cd;
   int i, j, val;
   char buf[20];
   struct RastPort *rp = global->rp;
   char *p;

   cd = (struct chkdata *)gadget->UserData;

   ulx = gadget->LeftEdge;      uly = gadget->TopEdge;
   lrx = ulx+gadget->Width-1;   lry = uly+(3*gadget->Height)-1;

   SetAPen(rp, 1);
   Move(rp, (WORD)ulx, (WORD)(uly-12+rp->TxBaseline));
   p = cd->string;
   Text(rp, p, (WORD)(strlen(p)));

   /* Now we want to render the items (if any) */
   i = cd->slot;
   i += global->strtab[i].value+1;
   if (!global->reset)
   {
      SetAPen(rp, 0);
      RectFill(rp, ulx, uly, lrx, lry);
   }
   SetAPen(rp, 1);
   for(j = 0; j < 3; j++, i++)
   {
      if ((val = global->strtab[i].value) == NOVAL) break;
      sprintf(buf, "%5ld %s",
                   val & 0xffff,
                        "Ignore\0 Warn\0   Error\0"+(((val >> 16) & 0xff)<<3));
      Move(rp, (WORD)ulx, (WORD)(uly+2+(11*j)+rp->TxBaseline));
      Text(rp, buf, (WORD)(strlen(buf)));
   }
   drawbox(global->rp, ulx-2, uly-1, lrx+2, lry+1, global->highlight, global->shadow);
}

void refresh(global)
struct GLOBAL *global;
{
   struct Gadget *gadget;
   struct RastPort *rp = global->rp;
   WORD   ulx, width;
   int i;


   if (global->scrnum != 1)
   {
      SetDrMd(rp, JAM1);
      SetAfPt(rp, checker, 1);
      SetAPen(rp, global->highlight);
      /*
          +------------------------------+
          |111111111111111111111111111111|
          |33+------------------------+44|
          |33|                        |44|
          |33|                        |44|
          |33|                        |44|
          |33+------------------------+44|
          |222222222222222222222222222222|
          +------------------------------+
      */
      RectFill(rp, global->left, global->top, 
                   (WORD)(global->right-1), (WORD)(global->top+1));
      RectFill(rp, global->left, (WORD)(global->bottom-23),
                   (WORD)(global->right-1), (WORD)(global->bottom-1));
      RectFill(rp, global->left, (WORD)(global->top+2), 
                   (WORD)(global->left+3), (WORD)(global->bottom-23));
      RectFill(rp, (WORD)(global->right-3), (WORD)(global->top+1), 
                   (WORD)(global->right-1), (WORD)(global->bottom-23));
      SetAfPt(rp, &solid, 0);
      if (!global->reset)
      {
         SetAPen(rp, 0);
         RectFill(rp, (WORD)(global->left+8), (WORD)(global->top+5),
                              (WORD)(global->right-8), (WORD)(global->bottom-24));
      }
      drawbox(rp, global->left  + 4, global->top    + 2,
                          global->right - 4, global->bottom - 24,
                          global->highlight, global->shadow);
      drawbox(rp, global->left  + 6, global->top    + 3,
                          global->right - 6, global->bottom - 25,
                          global->highlight, global->shadow);
      drawbox(rp, global->left  + 6, global->top    + 4,
                          global->right - 6, global->bottom - 26,
                          global->highlight, global->shadow);
   }
   for(gadget = global->gadlist; gadget != NULL; gadget = gadget->NextGadget)
   {
      render(global, gadget);
   }

   if (global->scrnum == 5)
   {
      /* Center Message 1,2,3 on the screen */
      ulx = global->left;
      width = global->right;
      i = strlen(global->message1);
      Move(rp, (WORD)(ulx + (width-TextLength(rp, global->message1,(WORD)i))/2), 85);
      Text(rp, global->message1, (WORD)i);

      i = strlen(global->message2);
      Move(rp, (WORD)(ulx + (width-TextLength(rp, global->message2,(WORD)i))/2), 95);
      Text(rp, global->message2, (WORD)i);

      i = strlen(global->message3);
      Move(rp, (WORD)(ulx + (width-TextLength(rp, global->message3,(WORD)i))/2), 105);
      Text(rp, global->message3, (WORD)i);
   }

   /* If in prompt mode, activate the string gadget */
   if ((global->scrnum == 4) || (global->scrnum == 6))
       setstate(global, 1, STRING_V, 0x55);
}

void render(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   int refresh = 0;

   switch(gadget->GadgetID)
   {
      case MULTI_V:
         rendercyc(global, gadget);
         break;
      case BUTTON_V:
         renderbtn(global, gadget);
         break;
      case CHECK_V:
         renderchk(global, gadget);
         break;
      case SCROLL_V:
         renderbox(global, gadget);
         resetscroll(global, gadget);
         break;
      case NEW_V:
      case DEL_V:
      case DOWN_V:
      case UP_V:
         renderbox(global, gadget);
         break;
      case STRING_V:
      case STR_V:
      case NUM_V:
      case NUMERIC_V:
         renderstrnum(global, gadget);
         refresh = 1;
         break;
      case LIST_V:
         renderlist(global, gadget);
         break;
      case NLIST_V:
         rendernlist(global, gadget);
         break;
      case ITEM2_V:
      case ITEM3_V:
         break;
   }

   if (refresh || (gadget->Flags & GADGDISABLED))
   {
      RefreshGList(gadget, global->window, NULL, 1);
   }
}
