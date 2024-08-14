/*
   Requestor for disk devices
*/

#include <exec/types.h>
#include <intuition/intuition.h>

#include "protos.h"

#define BLUE 0
#define WHITE 1
#define BLACK 2
#define RED 3

#define RWIDTH      640
#define RHEIGHT      100
#define RTOPEDGE   0
/*#define RTOPEDGE   ((200 - RHEIGHT) / 2)*/

#define OKID   0
#define CNID   1
#define DRID   2
#define DID   3

#define OKWIDTH      300
#define OKHEIGHT   16
#define CNWIDTH      300
#define CNHEIGHT   16

#define MAXDRLEN   78
#define DRWIDTH      (MAXDRLEN * 8)
#define DRHEIGHT   9

#define MAXDEVS      10
/* #define DWIDTH      48 */
#define DWIDTH      60
#define DHEIGHT      16
#define RAMNAME      "RAM"

static LONG RPTopEdge;

static struct XY {
   SHORT x1,y1, x2,y2, x3,y3, x4,y4, x5,y5;
};

static struct XY DXY[MAXDEVS] = {
   {0,0, DWIDTH,0, DWIDTH,DHEIGHT, 0,DHEIGHT, 0,0},
};

static struct Border DBorder[MAXDEVS] = {
   {
   -1, -1,   /* LeftEdge, TopEdge */
   RED, -1, /* FrontPen, BackPen */
   JAM1,   /* DrawMode */
   5,      /* Count */
   (SHORT *)&DXY[0],   /* XY pairs */
   NULL   /* NextBorder */
   },
};

static char DText[MAXDEVS][33];

static struct IntuiText DIText[MAXDEVS] = {
   {
   BLACK, RED,   /* FrontPen, BackPen */
   JAM1,      /* DrawMode */
   8, 4,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   DText[0],   /* IText */
   NULL      /* NextText */
   },
};

static struct Gadget DGadget[MAXDEVS] = {
   {
   NULL,            /* Next Gadget */
   0,            /* LeftEdge */
   32,            /* TopEdge */
   DWIDTH-1, DHEIGHT-1,      /* Width, Height */
   GADGHNONE,         /* Flags */
   GADGIMMEDIATE,         /* Activation Flags */
   BOOLGADGET|REQGADGET,      /* Gadget Type */
   (APTR)&DBorder[0],         /* Gadget Render */
   NULL,            /* Select Render */
   &DIText[0],         /* Gadget Text */
   0,            /* Mutual Exclude */
   NULL,            /* SpecialInfo */
   DID,            /* Gadget ID */
   NULL            /* User Data */
   },
};

static SHORT DrXY[] = {0,0, DRWIDTH,0, DRWIDTH,DRHEIGHT, 0,DRHEIGHT, 0,0};

static struct Border DrBorder = {
   -1, -1,   /* LeftEdge, TopEdge */
   RED, -1,   /* FrontPen, BackPen */
   JAM1,   /* DrawMode */
   5,      /* Count */
   DrXY,   /* XY pairs */
   NULL   /* NextBorder */
};

static struct IntuiText DrIText = {
   RED, -1,   /* FrontPen, BackPen */
   JAM1,      /* DrawMode */
   (RWIDTH-6*8)/2, -10,   /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   "DRAWER",   /* IText */
   NULL      /* NextText */
};

static struct StringInfo DrInfo = {
   (UBYTE *)NULL,       /* Buffer (filled in) */
   (SHORT)  NULL,       /* UndoBuffer (filled in) */
   (SHORT)  0,          /* BufferPos */
   (SHORT)  MAXDRLEN,   /* MaxChars */
   (SHORT)  0           /* DispPos */
};

static struct Gadget DrGadget = {
   NULL,            /* Next Gadget */
   (RWIDTH-DRWIDTH)/2,      /* LeftEdge */
   -8-DRHEIGHT-OKHEIGHT,      /* TopEdge */
   DRWIDTH, DRHEIGHT-1,   /* Width, Height */
   GRELBOTTOM|GADGHCOMP,   /* Flags */
   RELVERIFY,            /* Activation Flags */
   STRGADGET|REQGADGET,   /* Gadget Type */
   (APTR)&DrBorder,            /* Gadget Render */
   NULL,               /* Select Render */
   &DrIText,            /* Gadget Text */
   0,                  /* Mutual Exclude */
   (APTR)&DrInfo,            /* SpecialInfo */
   DRID,               /* Gadget ID */
   NULL               /* User Data */
};

static SHORT CnXY[] = {0,0, CNWIDTH,0, CNWIDTH,CNHEIGHT, 0,CNHEIGHT, 0,0};

static struct Border CnBorder = {
   -1, -1,   /* LeftEdge, TopEdge */
   RED, -1,/* FrontPen, BackPen */
   JAM1,   /* DrawMode */
   5,      /* Count */
   CnXY,   /* XY pairs */
   NULL   /* NextBorder */
};

static struct IntuiText CnIText = {
   RED, -1,   /* FrontPen, BackPen */
   JAM1,      /* DrawMode */
   (CNWIDTH-6*8)/2, 4,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   "CANCEL",   /* IText */
   NULL      /* NextText */
};

static struct Gadget CnGadget = {
   &DrGadget,         /* Next Gadget */
   -CNWIDTH-4,         /* LeftEdge */
   -CNHEIGHT-1,      /* TopEdge */
   CNWIDTH-1, CNHEIGHT-1,      /* Width, Height */
   GRELRIGHT|GRELBOTTOM|GADGHCOMP,   /* Flags */
   RELVERIFY|ENDGADGET,   /* Activation Flags */
   BOOLGADGET|REQGADGET,   /* Gadget Type */
   (APTR)&CnBorder,            /* Gadget Render */
   NULL,               /* Select Render */
   &CnIText,            /* Gadget Text */
   0,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   CNID,               /* Gadget ID */
   NULL               /* User Data */
};

static SHORT OkXY[] = {0,0, OKWIDTH,0, OKWIDTH,OKHEIGHT, 0,OKHEIGHT, 0,0};

static struct Border OkBorder = {
   -1, -1,   /* LeftEdge, TopEdge */
   RED, -1,/* FrontPen, BackPen */
   JAM1,   /* DrawMode */
   5,      /* Count */
   OkXY,   /* XY pairs */
   NULL   /* NextBorder */
};

static struct IntuiText OkIText = {
   RED, -1,   /* FrontPen, BackPen */
   JAM1,      /* DrawMode */
   (OKWIDTH-2*8)/2, 4,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   "OK",      /* IText */
   NULL      /* NextText */
};

static struct Gadget OkGadget = {
   &CnGadget,         /* Next Gadget */
   4,         /* LeftEdge */
   -OKHEIGHT-1,         /* TopEdge */
   OKWIDTH-1, OKHEIGHT-1,      /* Width, Height */
   GRELBOTTOM|GADGHCOMP,   /* Flags */
   RELVERIFY|ENDGADGET,      /* Activation Flags */
   BOOLGADGET|REQGADGET,   /* Gadget Type */
   (APTR)&OkBorder,            /* Gadget Render */
   NULL,               /* Select Render */
   &OkIText,            /* Gadget Text */
   0,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   OKID,               /* Gadget ID */
   NULL               /* User Data */
};

static SHORT ReqXY[]={0,0, RWIDTH-1,0, RWIDTH-1,RHEIGHT-1, 0,RHEIGHT-1, 0,0};

static struct Border ReqBorder = {
   0, 0,   /* LeftEdge, TopEdge */
   RED, -1, /* FrontPen, BackPen */
   JAM1,    /* DrawMode */
   5,      /* Count */
   ReqXY,   /* XY pairs */
   NULL   /* NextBorder */
};

static struct IntuiText ReqIText = {
   RED, -1,   /* FrontPen, BackPen */
   JAM1,      /* DrawMode */
   0, 10,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   NULL,      /* IText */
   NULL      /* NextText */
};

static struct Requester DevReq = {
   NULL,         /* OlderRequest */
   0, RTOPEDGE,   /* LeftEdge, TopEdge */
   RWIDTH, RHEIGHT,   /* Width, Height */
   0, 0,         /* RelLeft, RelTop */
   &OkGadget,      /* ReqGadget */
   &ReqBorder,      /* ReqBorder */
   &ReqIText,      /* ReqText */
   0,         /* Flags */
   WHITE,         /* BackFill */
   NULL         /* ReqLayer */
};
   
VOID DoGadgetsHilight(struct Gadget *G,struct RastPort *rp,ULONG flag)
                                                       /* 0-off, 1-on */
{
   /* if want off and is on */
   if ((flag == 0 && G->Flags & SELECTED) ||
      /* or want on and is off */      
      (flag == 1 && !(G->Flags & SELECTED))) {
      (G->Flags) ^= SELECTED;
      ClipBlit(rp, G->LeftEdge, G->TopEdge + RPTopEdge, rp,
         G->LeftEdge, G->TopEdge + RPTopEdge, G->Width, G->Height, 0x30);
   }
}

VOID DoGadgetsSelect(struct Gadget *G,struct RastPort *rp,struct List *list,UBYTE *buf)
{
   struct Gadget *g2;
   struct Node *node;
   LONG id;

   id = G->GadgetID;
   for (g2 = &DGadget[0]; g2 != NULL; g2 = g2->NextGadget) {
      if (g2->GadgetID != id) {
         DoGadgetsHilight(g2, rp, 0); /* off */
      }
      else {
         DoGadgetsHilight(g2, rp, 1); /* on */
      }
   }
   id -= DID;
   for (node = list->lh_Head; node->ln_Succ != NULL, id > 0;
      node = node->ln_Succ, id--) {
   }
   strcpy(buf, node->ln_Name);
   strcat(buf, ":");
#if DEBUG
   kprintf("selected '%s'\n", node->ln_Name);
#endif
}

static char udrbuf[MAXDRLEN] = "";

VOID MakeDevRequest(struct List *list,UBYTE *devbuf, UBYTE *drbuf,struct Window *w,UBYTE *title)
{
   struct Node *node, *next;
   LONG i, spacing, n = 0;
   /* LONG done=0; */

   if (list != NULL) {
      /* figure out how many gadgets we are gonna need */
      for (node=list->lh_Head, n=0; node->ln_Succ!=NULL &&
         n < MAXDEVS; node=next) {
         next = node->ln_Succ;
         if (stricmp(node->ln_Name, RAMNAME) == 0) Remove(node);
         else n++;
      }
      /* init gadgets */
      node = list->lh_Head;
      strcpy(devbuf, node->ln_Name);
      strcat(devbuf, ":");
      spacing = (RWIDTH - DWIDTH * n) / (n + 1);
      DGadget[0].LeftEdge = spacing;
      DrGadget.NextGadget = &DGadget[0];
      for (i=0; i<n; i++) {
          if (i != 0) {
             DGadget[i] = DGadget[i - 1]; /* copy gadget struct */
             DBorder[i] = DBorder[i - 1]; /* copy border struct */
             DIText[i] = DIText[i - 1]; /* copy IText struct */
             DXY[i] = DXY[i - 1]; /* copy XY struct */
             /* asgn nxt gadget */
             DGadget[i - 1].NextGadget = &DGadget[i]; 
             DGadget[i].LeftEdge += DWIDTH + spacing;
              /* set border ptr */
             DGadget[i].GadgetRender = (APTR)&DBorder[i];
             DGadget[i].GadgetText= &DIText[i]; /* set itext ptr */
             (DGadget[i].GadgetID)++; /* set id # */
             DIText[i].IText = DText[i]; /* set text ptr */
             D( kprintf("DIText[%ld].IText = DText[%ld]=0x%lx\n",i,i,DText[i]); )
             DBorder[i].XY = (SHORT *)&DXY[i]; /* set border ptr */
          }
         /* strcpy((UBYTE *)DText[i], node->ln_Name);*/ /* copy string */
         stccpy((UBYTE *)DText[i], node->ln_Name,7); /* copy string */
         D( kprintf("Copying string %s\n",node->ln_Name); )
         D( kprintf("Copied  string %s\n",DText[i]); )
         /* center text */
         DIText[i].LeftEdge = (DWIDTH-strlen(node->ln_Name)*8)/2;
         node = node->ln_Succ;
      }
   }
   else DrGadget.NextGadget = NULL;
   D( kprintf("MakeDevRequest() drbuf=%s, udrbuf=%s\n",drbuf,udrbuf); )
   D( kprintf("MakeDevRequest() DrInfo.BufferPos=%ld\n",DrInfo.BufferPos); )
   D( kprintf("MakeDevRequest() DrInfo.MaxChars =%ld\n",DrInfo.MaxChars); )
   D( kprintf("MakeDevRequest() DrInfo.DispPos  =%ld\n",DrInfo.DispPos); )
   D( kprintf("string[0]=%s\n",DText[0]); )
   D( kprintf("string[1]=%s\n",DText[1]); )
   D( kprintf("string[2]=%s\n",DText[2]); )
   D( kprintf("string[3]=%s\n",DText[3]); )
   DrInfo.Buffer = drbuf;
   DrInfo.UndoBuffer = udrbuf;
   DevReq.ReqText->IText = title;
   DevReq.ReqText->LeftEdge = (RWIDTH - strlen(title) * 8) / 2;
}

LONG DevRequest(struct List *list,UBYTE *devbuf,UBYTE *drbuf,struct Window *w)
{
   struct RastPort *rp;
   struct IntuiMessage *Msg;
   struct Gadget *Gadget;
   LONG  done=0;

   *drbuf = '\0';
   if (Request(&DevReq, w)) {
      rp = &(w->WScreen->RastPort);
      RPTopEdge = w->TopEdge + RTOPEDGE;
      if (list != NULL) {
          for(Gadget= &DGadget[0];Gadget!=NULL;
         Gadget=Gadget->NextGadget)
             (Gadget->Flags) &= (~SELECTED);
          DoGadgetsHilight(&DGadget[0], rp, 1); /* on */
      }
      do {
         Wait(1 << w->UserPort->mp_SigBit);
         while ((Msg = (struct IntuiMessage *)GetMsg(w->UserPort)) != NULL) {
            switch (Msg->Class) {
               case GADGETDOWN:
                  DoGadgetsSelect((struct Gadget *)Msg->IAddress, rp, list, devbuf);
                  break;
               case GADGETUP:
                  Gadget = (struct Gadget *)Msg->IAddress;
                  if (Gadget->GadgetID != DRID) {
                     done = Gadget->GadgetID == OKID ? 1 : -1;
                  }
                  break;
               default:
                  break;
            }
            ReplyMsg((struct Message *)Msg);
         }
      } while (!done);
   }
   strcpy(udrbuf, drbuf);
   strcpy(drbuf, devbuf);
   strcat(drbuf, udrbuf);
   if (drbuf[strlen(drbuf)] == '/') {
      drbuf[strlen(drbuf)] = '\0';
   }
#if DEBUG
   kprintf("devbuf='%s', drbuf='%s'\n", devbuf, drbuf);
#endif
   return(done);
}

struct TextAttr topaz9 = {
    "topaz.font", 9, 0, 0
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

VOID Notice(struct Window *w,UBYTE *string1, UBYTE *string2)
{ 
    bodyText1.IText = string1;
    bodyText2.IText = string2;
    AutoRequest(w, &bodyText1, 0, &buttonText, 0, 0, RWIDTH, RHEIGHT);
}
