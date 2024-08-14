/*
   Prompt routines for install.
*/

#include "exec/types.h"
#include "extern.h"
#include "gadgets.h"
#include "prompt.h"
#include "list.h"
#include "display.h"

#include "protos.h"

#define DEBUG2   0

#define ABS(a)   ((a) < 0 ? -(a) : (a))

#define NUMROWS   ((WHEIGHT - TOPOFFSET - 2) / 8)
#define NUMCOLS   ((WWIDTH - UPWIDTH) / 8 - 1)

extern LONG Pass;
extern struct Gadget UpGadget;
extern struct Gadget PrGadget;
extern struct PropInfo PrInfo;
extern struct Image CheckMark;
extern struct Image XMark;

static FILELIST *List;
static FILENODE *TopNode;
static LONG Step;

LONG UnpackNode(FILENODE *node,UBYTE *buf)
{
   LONG len, check;
   static char DirString[] = " (Dir)";
#if DEBUG
kprintf("\nUN: enter, node=%06lx, buf=%06lx\n", node, buf);
#endif
#if DEBUG
kprintf("NumBlocks=%ld\n", node->NumBlocks);
#endif
   sprintf(buf, "%5ld ", node->NumBlocks);
   len = 6;
#if DEBUG
kprintf("len=%ld, Depth=%ld\n", len, node->Depth);
#endif
   setmem(&buf[len], node->Depth * 4, ' ');
   len += node->Depth * 4;
#if DEBUG
kprintf("len=%ld, Flags=%ld\n", len, node->Flags);
#endif
/*   buf[len] = node->Flags == 0 ? ' ' : '*';*/
   buf[len]=' ';
   check = node->Flags == 0 ? -1 : len;
   len++;
#if DEBUG
kprintf("len=%ld, name='%s'\n",len,  node->Name);
#endif
   sprintf(&buf[len], "%s", node->Name);
   len += strlen(node->Name);
#if DEBUG
kprintf("len=%ld, DirEntryType=%ld\n", len, node->DirEntryType);
#endif
   if (node->DirEntryType > 0) {
#if DEBUG
kprintf("adding ' (Dir)', len=%ld, &buf[len]=%06lx\n", len, &buf[len]);
#endif
      strcpy(&buf[len], DirString);
      len += 6;
   }
#if DEBUG2
   sprintf(&buf[len], " %03ld", node->Num);
   len += 4;
#endif
#if DEBUG
kprintf("NUMCOLS=%ld, len=%ld, setting %ld bytes to ' '\n",
NUMCOLS, len, NUMCOLS - len);
#endif
   setmem(&buf[len], NUMCOLS - len, ' ');
#if DEBUG
kprintf("UN: len=%ld, check=%ld, exit\n", len, check);
#endif
   return(check);
}

VOID DisplayNode(FILENODE *node)
{
   char buf[NUMCOLS];
   LONG check,y;

#if DEBUG
kprintf("DN: enter, node=%06lx, TopNode=%06lx, ", node, TopNode);
#endif
   check=UnpackNode(node, buf)*8;
   y=CHARYOFFSET + (node->Num - TopNode->Num) * 8;
   Move(rp, CHARXOFFSET, y);
   Text(rp, buf, NUMCOLS - 1);
   if(check >= 0) {
      if(!Pass)DrawImage(rp,&CheckMark,CHARXOFFSET+check,y);
      else DrawImage(rp,&XMark,CHARXOFFSET+check,y);
   }
#if DEBUG
kprintf("exit\n");
#endif
}

VOID RefreshDisplay(FILENODE *node)
{
   FILENODE *node2;
   LONG count, dy, i;
   char buf[NUMCOLS];

#if DEBUG
kprintf("RD: enter, TopNode=%06lx, node=%06lx, List=%06lx\n",
TopNode, node, List);
#endif
   count = NUMROWS;
   if (List->FirstFN->FNext != NULL) { /* if list not empty */
#if DEBUG
kprintf("RD: list not empty\n");
#endif
      node2 = node;
      if (TopNode != NULL) { /* if not first time */
#if DEBUG
kprintf("RD: not first time\n");
#endif
         if ((count = ABS(node->Num - TopNode->Num)) < NUMROWS) {
#if DEBUG
kprintf("RD: partial refresh\n");
#endif
            dy = count * 8;
            if (node->Num > TopNode->Num) {
#if DEBUG
kprintf("RD: scroll up\n");
#endif
                /* scroll up */
                ScrollRaster(rp, 0, dy, /* dx, dy */
               CHARXOFFSET, TOPOFFSET + 2,/* xmin,ymin */
               NUMCOLS * 8 - 1,   /* xmax */
               TOPOFFSET + 2 + NUMROWS *8 - 1);/* ymax */
                for (i = 0; i < NUMROWS - count; i++) {
                  node2 = node2->FNext;
                }
            }
            else {
#if DEBUG
kprintf("RD: scroll down\n");
#endif
                /* scroll down */
                ScrollRaster(rp, 0, -dy, /* dx, dy */
               CHARXOFFSET, TOPOFFSET +2,/* xmin, ymin */
               NUMCOLS * 8 - 1,   /* xmax */
               TOPOFFSET + 2 + NUMROWS *8 - 1);/* ymax */
            }
         }
         else { /* full refresh */
#if DEBUG
kprintf("RD: full refresh\n");
#endif
            count = NUMROWS;
         }
      }
      else { /* first time */
#if DEBUG
kprintf("RD: first time\n");
#endif
         count = NUMROWS;
      }
#if DEBUG
kprintf("RD: calling DisplayNode for each node\n");
#endif
      TopNode = node;
      for (; node2->FNext != NULL; node2 = node2->FNext) {
         DisplayNode(node2);
         if (--count == 0) {
            break;
         }
      }
   }
   setmem(buf, NUMCOLS, ' ');
   i = (NUMROWS - count) * 8;
#if DEBUG
   kprintf("RD: count=%ld, i=%ld\n", count, i);
#endif
   while (count > 0) {
      /* erase to bottom */
      Move(rp, CHARXOFFSET, CHARYOFFSET + i);
      Text(rp, buf, NUMCOLS - 1);
      count--;
      i += 8;
   }
#if DEBUG
   kprintf("RD: exit\n");
#endif
}

VOID SelectUp(VOID)
{
   if (TopNode != List->FirstFN) {
      RefreshDisplay(TopNode->FPrev);
      ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
         Step * TopNode->Num, PrInfo.HorizBody, PrInfo.VertBody);
   }
}

VOID SelectDown(VOID)
{
   if (TopNode->Num + NUMROWS <= List->LastFN->Num) {
      RefreshDisplay(TopNode->FNext);
      ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
         Step * TopNode->Num, PrInfo.HorizBody, PrInfo.VertBody);
   }
}

VOID DoProp(struct Gadget *Gadget)
{
   struct PropInfo *Prop;
   FILENODE *node;
   LONG num;

   Prop = (struct PropInfo *)Gadget->SpecialInfo;
   if (Step != -1) {
      num = Prop->VertPot / Step;
      node = List->FirstFN;
      while (node->FNext != NULL && node->Num != num) {
         node = node->FNext;
      }
      if (node != TopNode) {
         RefreshDisplay(node);
      }
   }
}

LONG DoGadgets(struct IntuiMessage *Msg)
{
   struct Gadget *Gadget;
   LONG done = 0;

   Gadget = (struct Gadget *)Msg->IAddress;
   switch (Gadget->GadgetID) {
      case UPID:
         SelectUp();
         break;
      case DNID:
         SelectDown();
         break;
      case OKID:
         done = 1;
         break;
      case CNID:
         done = -1;
         break;
      case PRID:
         DoProp(Gadget);
         break;
      default:
         break;
   }
   return(done);
}

VOID DoMouse(struct IntuiMessage *Msg,LONG *Selected, LONG *Required, LONG Flag)
{
   FILENODE *node;
   LONG x, y;
   /* LONG ypos; */

   if (Msg->Code == SELECTDOWN) {
      x = Msg->MouseX / 8;
      y = (Msg->MouseY - CHARYOFFSET + 6) / 8;
      if (x < NUMCOLS) {
         if (y >= 0 && y < NUMROWS) {
            for (node=TopNode; node->FNext!=NULL; node=node->FNext) {
               if (y-- < 1) {
                  node->Flags ^= Flag;
                  DisplayNode(node);
                  if (node->Flags & Flag) {
                     *Selected += node->NumBlocks;
                     *Required -= node->NumBlocks;
                  }
                  else {
                     *Selected -= node->NumBlocks;
                     *Required += node->NumBlocks;
                  }
                  Status(w, *Selected, *Required,Pass);
                  break;
               }
            }
         }
      }
   }
}

LONG Prompt(FILELIST *list,LONG *Selected,LONG *Required,LONG Flag)
{
   struct IntuiMessage *Msg;
   LONG VertBody, n;
   LONG done = 0;

   if (list->FirstFN->FNext != NULL) { /* if list not empty */
      n = list->LastFN->Num + 1;
      if (n > NUMROWS) {
         VertBody = 65535 / n * NUMROWS;
         Step = 65535 / (n - NUMROWS);
      }
      else {
         VertBody = 65535;
         Step = -1;
      }
#if DEBUG
      kprintf("P: enter, n=%ld, VertBody=%ld, Step=%ld\n",
         n, VertBody, Step);
#endif
      ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
         PrInfo.VertPot, PrInfo.HorizBody, VertBody);
   }
   List = list;
   TopNode = NULL;
   RefreshDisplay(List->FirstFN);
   Status(w, *Selected, *Required,Pass);
   while (done == 0 && List != NULL) {
      Wait(1 << w->UserPort->mp_SigBit);
      while ((Msg = (struct IntuiMessage *)GetMsg(w->UserPort)) != NULL) {
         switch (Msg->Class) {
            case MOUSEBUTTONS:
               DoMouse(Msg, Selected, Required, Flag);
               break;
            case GADGETUP:
               done = DoGadgets(Msg);
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
#if DEBUG
   kprintf("P: exit\n");
#endif
   return(done);
}

VOID ResetProp(VOID)
{
    ModifyProp(&PrGadget, w, NULL, PrInfo.Flags, PrInfo.HorizPot,
   0, PrInfo.HorizBody, PrInfo.VertBody);
}
