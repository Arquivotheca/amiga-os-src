
/****************************************************************************/
/*                                                                          */
/*  HandleMouse.c - Mouse functions                                         */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"
#include "Externs.h"

#include <devices/inputevent.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

/****************************************************************************/

void HandleMouseButtons(struct WindowNode *windownode, struct IntuiMessage *imsg)
  {
  static ULONG oldsecs=0, oldmicros=0;
  static struct NewGadgetNode *thisngn;
  UWORD code;
  ULONG secs;
  ULONG micros;
  UWORD qualifier;
  SHORT left, top, width, height, temp;
  SHORT leftmin, rightmax, topmin, bottommax;
  SHORT leftedge, rightedge, topedge, bottomedge;
  SHORT x, y;
  SHORT xx, yy;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("HandleMouseButtons", NULL);

  code = imsg->Code;
  secs = imsg->Seconds;
  micros = imsg->Micros;
  qualifier = imsg->Qualifier;

  x = imsg->MouseX;
  y = imsg->MouseY;
  leftedge   = windownode->Window->BorderLeft;
  topedge    = windownode->Window->BorderTop;
  rightedge  = windownode->Window->Width  - windownode->Window->BorderRight;
  bottomedge = windownode->Window->Height - windownode->Window->BorderBottom;

  leftmin = windownode->Window->Width;
  topmin  = windownode->Window->Height;
  rightmax  = 0;
  bottommax = 0;

  switch(code)
    {
    case SELECTDOWN:
      if(selectflag)
        {
        AbortSelection();
        drag = DRAGNONE;
        }
      else if(x>=leftedge && x<=rightedge && y>=topedge && y<=bottomedge)
        {
        selectflag   = TRUE;
        actuallymove = FALSE;
        selectwindow = windownode;
        ModifyIDCMP(selectwindow->Window, IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);

        Forbid();
        selectwindow->Window->Flags |= WFLG_RMBTRAP;
        Permit();

        for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(drag == DRAGNONE)
            {
            left   = newgadgetnode->NewGadget.ng_LeftEdge;
            top    = newgadgetnode->NewGadget.ng_TopEdge;
            width  = newgadgetnode->NewGadget.ng_Width - 1;
            height = newgadgetnode->NewGadget.ng_Height - 1;

            xmark = x;
            ymark = y;

            if(drag != DRAGCREATE)
              {
              if(newgadgetnode->Selected)
                {
                if(x>=left-SIZEBLOCKWIDTH/2 &&
                   x<=left+SIZEBLOCKWIDTH/2 &&
                   y>=top-SIZEBLOCKHEIGHT/2 &&
                   y<=top+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZEUL;
                  }
                else if(x>=left+width-SIZEBLOCKWIDTH/2 &&
                        x<=left+width+SIZEBLOCKWIDTH/2 &&
                        y>=top-SIZEBLOCKHEIGHT/2 &&
                        y<=top+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZEUR;
                  }
                else if(x>=left-SIZEBLOCKWIDTH/2 &&
                        x<=left+SIZEBLOCKWIDTH/2 &&
                        y>=top+height-SIZEBLOCKHEIGHT/2 &&
                        y<=top+height+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZELL;
                  }
                else if(x>=left+width-SIZEBLOCKWIDTH/2 &&
                        x<=left+width+SIZEBLOCKWIDTH/2 &&
                        y>=top+height-SIZEBLOCKHEIGHT/2 &&
                        y<=top+height+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZELR;
                  }
                else if(x>=left-SIZEBLOCKWIDTH/2 &&
                        x<=left+SIZEBLOCKWIDTH/2 &&
                        y>=top+height/2-SIZEBLOCKHEIGHT/2 &&
                        y<=top+height/2+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZEL;
                  }
                else if(x>=left+width-SIZEBLOCKWIDTH/2 &&
                        x<=left+width+SIZEBLOCKWIDTH/2 &&
                        y>=top+height/2-SIZEBLOCKHEIGHT/2 &&
                        y<=top+height/2+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZER;
                  }
                else if(x>=left+width/2-SIZEBLOCKWIDTH/2 &&
                        x<=left+width/2+SIZEBLOCKWIDTH/2 &&
                        y>=top-SIZEBLOCKHEIGHT/2 &&
                        y<=top+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZET;
                  }
                else if(x>=left+width/2-SIZEBLOCKWIDTH/2 &&
                        x<=left+width/2+SIZEBLOCKWIDTH/2 &&
                        y>=top+height-SIZEBLOCKHEIGHT/2 &&
                        y<=top+height+SIZEBLOCKHEIGHT/2)
                  {
                  thisngn = newgadgetnode;
                  drag = DRAGSIZEB;
                  }
                }

              if(x>=left && x<=left+width && y>=top && y<=top+height && drag==DRAGNONE)
                {
                if(!(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))) UnSelectAllGadgets(NULL);
                SelectGadget(selectwindow, newgadgetnode);
                thisngn = newgadgetnode;
                drag = DRAGMOVE;
                }
              }
            }
          }

        if(drag == DRAGNONE)
          {
          drag = DRAGSELECT;
          if(!(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
            {
            UnSelectAllGadgets(NULL);
            }
          }

        SetDrPt(selectwindow->Window->RPort, 0xF0F0);
        SetDrMd(selectwindow->Window->RPort, COMPLEMENT);

        switch(drag)
          {
          case DRAGSELECT:
            xstart = x;
            ystart = y;
            xend = xstart;
            yend = ystart;
            DrawDragBox(selectwindow, xstart, ystart, xend, yend);
            leftmin = x;
            rightmax = x;
            topmin = y;
            bottommax = y;
            if((DoubleClick(oldsecs, oldmicros, secs, micros)) &&
               (selectedobject==(ULONG) selectwindow))
              {
              AbortSelection();
              drag = DRAGNONE;
              EditTags(TYPE_WINDOW, selectwindow, NULL);
              selectedobject = NULL;
              }
            else
              {
              oldsecs = secs;
              oldmicros = micros;
              selectedobject = (ULONG) selectwindow;
              }
            break;

          case DRAGCREATE:
            xstart = ConvertGridX(x);
            ystart = ConvertGridY(y);
            xend = xstart;
            yend = ystart;
            DrawDragBox(selectwindow, xstart, ystart, xend, yend);
            leftmin = x;
            rightmax = x;
            topmin = y;
            bottommax = y;
            break;

          case DRAGMOVE:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xstart < leftmin) leftmin   = newgadgetnode->xstart;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->ystart < topmin)  topmin    = newgadgetnode->ystart;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }

            if(DoubleClick(oldsecs, oldmicros, secs, micros) && onegadgetflag && selectedobject==(ULONG)thisngn)
              {
              AbortSelection();
              drag = DRAGNONE;
              EditTags(TYPE_GADGET, selectwindow, thisngn);
              selectedobject = NULL;
              }
            else
              {
              oldsecs = secs;
              oldmicros = micros;
              selectedobject = (ULONG) thisngn;
              }
            break;

          case DRAGSIZEUL:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZEUR:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZELL:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZELR:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZEL:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZER:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZET:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;

          case DRAGSIZEB:
            for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected)
                {
                newgadgetnode->xstart = newgadgetnode->NewGadget.ng_LeftEdge;
                newgadgetnode->ystart = newgadgetnode->NewGadget.ng_TopEdge;
                newgadgetnode->xend =   newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width;
                newgadgetnode->yend =   newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height;
                DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
                if(newgadgetnode->xend < leftmin)   leftmin   = newgadgetnode->xend;
                if(newgadgetnode->xend > rightmax)  rightmax  = newgadgetnode->xend;
                if(newgadgetnode->yend < topmin)    topmin    = newgadgetnode->yend;
                if(newgadgetnode->yend > bottommax) bottommax = newgadgetnode->yend;
                }
              }
            break;
          }

        leftbound   = leftedge   + (x - leftmin);
        topbound    = topedge    + (y - topmin);
        rightbound  = rightedge  - (rightmax  - x);
        bottombound = bottomedge - (bottommax - y);
        }
      break;

    case SELECTUP:
      if(selectflag)
        {
        selectflag = FALSE;

        if(realgadgets)
          ModifyIDCMP(selectwindow->Window, arrayidcmp | gadgetidcmp);
        else
          ModifyIDCMP(selectwindow->Window, arrayidcmp);

        Forbid();
        selectwindow->Window->Flags &= ~WFLG_RMBTRAP;
        Permit();

        if(x < leftbound)
          xx = leftbound;
        else if(x > rightbound)
          xx = rightbound;
        else
          xx = x;

        if(y < topbound)
          yy = topbound;
        else if(y > bottombound)
          yy = bottombound;
        else
          yy = y;

        for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->xstart>newgadgetnode->xend)
            {
            temp = newgadgetnode->xstart;
            newgadgetnode->xstart = newgadgetnode->xend;
            newgadgetnode->xend = temp;
            }
          if(newgadgetnode->ystart>newgadgetnode->yend)
            {
            temp = newgadgetnode->ystart;
            newgadgetnode->ystart = newgadgetnode->yend;
            newgadgetnode->yend = temp;
            }
          }

        AbortSelection();

        switch(drag)
          {
          case DRAGSELECT:
            DragSelectGadgets(selectwindow, xstart, ystart, xend, yend);
            break;

          case DRAGCREATE:
            AddNewGadget(selectwindow, xstart, ystart, xend, yend);
            SelectSelect();
            break;

          case DRAGMOVE:
            if(thisngn->xstart != thisngn->NewGadget.ng_LeftEdge ||
               thisngn->ystart != thisngn->NewGadget.ng_TopEdge)
              {
              MainWindowSleep();
              RemoveGadgets(selectwindow);
              for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                  newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
                {
                if(newgadgetnode->Selected)
                  {
                  newgadgetnode->NewGadget.ng_LeftEdge = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + (xx-xmark));
                  newgadgetnode->NewGadget.ng_TopEdge  = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + (yy-ymark));
                  }
                }
              AddGadgets(selectwindow);
              MainWindowWakeUp();
              modified = TRUE;
              }
            break;

          case DRAGSIZEUL:
          case DRAGSIZEUR:
          case DRAGSIZELL:
          case DRAGSIZELR:
          case DRAGSIZEL:
          case DRAGSIZER:
          case DRAGSIZET:
          case DRAGSIZEB:
            if(thisngn->xstart != thisngn->NewGadget.ng_LeftEdge ||
               thisngn->xend   != thisngn->NewGadget.ng_LeftEdge + thisngn->NewGadget.ng_Width ||
               thisngn->ystart != thisngn->NewGadget.ng_TopEdge ||
               thisngn->yend   != thisngn->NewGadget.ng_TopEdge + thisngn->NewGadget.ng_Height)
              {
              MainWindowSleep();
              RemoveGadgets(selectwindow);
              for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                  newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
                {
                if(newgadgetnode->Selected)
                  {
                  newgadgetnode->NewGadget.ng_LeftEdge = newgadgetnode->xstart;
                  newgadgetnode->NewGadget.ng_TopEdge = newgadgetnode->ystart;
                  newgadgetnode->NewGadget.ng_Width = newgadgetnode->xend - newgadgetnode->xstart;
                  newgadgetnode->NewGadget.ng_Height = newgadgetnode->yend - newgadgetnode->ystart;

                  switch(newgadgetnode->Kind)
                    {
                    case MX_KIND:
                      newgadgetnode->NewGadget.ng_Width = MXWIDTH;
                      newgadgetnode->NewGadget.ng_Height = newgadgetnode->TextAttr.ta_YSize + 1;
                      break;

                    case LISTVIEW_KIND:
                      if(newgadgetnode->NewGadget.ng_Width < MINLVWIDTH) newgadgetnode->NewGadget.ng_Width = MINLVWIDTH;
                      if(newgadgetnode->NewGadget.ng_Height < MINLVHEIGHT) newgadgetnode->NewGadget.ng_Height = MINLVHEIGHT;
                      break;

                    case CHECKBOX_KIND:
                      newgadgetnode->NewGadget.ng_Width = CHECKBOXWIDTH;
                      newgadgetnode->NewGadget.ng_Height = newgadgetnode->TextAttr.ta_YSize + 3;
                      break;

                    case STRING_KIND:
                    case INTEGER_KIND:
                      if(newgadgetnode->NewGadget.ng_Height < MININPUTHEIGHT) newgadgetnode->NewGadget.ng_Height = MININPUTHEIGHT;
                      break;
                    }

                  if(newgadgetnode->NewGadget.ng_Width < MINGADGETWIDTH) newgadgetnode->NewGadget.ng_Width = MINGADGETWIDTH;
                  if(newgadgetnode->NewGadget.ng_Height < MINGADGETHEIGHT) newgadgetnode->NewGadget.ng_Height = MINGADGETHEIGHT;
                  }
                }
              AddGadgets(selectwindow);
              MainWindowWakeUp();
              modified = TRUE;
              }
            break;
          }

        drag = DRAGNONE;
        }
      break;

    case MENUDOWN:
      AbortSelection();
      if((drag == DRAGMOVE) && (actuallymove == FALSE))
        {
        UnSelectGadget(selectwindow, thisngn);
        }

      drag = DRAGNONE;
      SelectSelect();
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void AbortSelection(void)
  {
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("AbortSelection", NULL);

  if(realgadgets)
    ModifyIDCMP(selectwindow->Window, arrayidcmp | gadgetidcmp);
  else
    ModifyIDCMP(selectwindow->Window, arrayidcmp);

  Forbid();
  selectwindow->Window->Flags &= ~WFLG_RMBTRAP;
  Permit();

  switch(drag)
    {
    case DRAGSELECT:
      DrawDragBox(selectwindow, xstart, ystart, xend, yend);
      break;

    case DRAGCREATE:
      DrawDragBox(selectwindow, xstart, ystart, xend, yend);
      break;

    case DRAGMOVE:
    case DRAGSIZEUL:
    case DRAGSIZEUR:
    case DRAGSIZELL:
    case DRAGSIZELR:
    case DRAGSIZEL:
    case DRAGSIZER:
    case DRAGSIZET:
    case DRAGSIZEB:
      for(newgadgetnode = (struct NewGadgetNode *) selectwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          DrawDragBox(selectwindow, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;
    }

  selectflag = FALSE;

  SetDrPt(selectwindow->Window->RPort, 0xFFFF);
  SetDrMd(selectwindow->Window->RPort, JAM1);

  RefreshWindowFrame(selectwindow->Window);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMouseMove(struct WindowNode *windownode, struct IntuiMessage *imsg)
  {
  register struct NewGadgetNode *newgadgetnode;
  register SHORT x, y;
  register SHORT xx, yy;

  DEBUGENTER("HandleMouseMove", NULL);

  x = imsg->MouseX;
  y = imsg->MouseY;

  if(x < leftbound)
    xx = leftbound;
  else if(x > rightbound)
    xx = rightbound;
  else
    xx = x;

  if(y < topbound)
    yy = topbound;
  else if(y > bottombound)
    yy = bottombound;
  else
    yy = y;

  if(!actuallymove &&
     ((abs(x-xmark)>=ACTUALLYMOVESIZE) || (abs(y-ymark)>=ACTUALLYMOVESIZE)))
    {
    actuallymove = TRUE;
    }

  switch(drag)
    {
    case DRAGMOVE:
      if(actuallymove)
        {
        for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }

        SpinDragBox(windownode);

        for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->Selected)
            {
            newgadgetnode->xstart = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + (xx-xmark));
            newgadgetnode->ystart = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + (yy-ymark));
            newgadgetnode->xend =   newgadgetnode->xstart + newgadgetnode->NewGadget.ng_Width;
            newgadgetnode->yend =   newgadgetnode->ystart + newgadgetnode->NewGadget.ng_Height;
            DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
            }
          }
        }
      break;

    case DRAGSELECT:
/*
      if(actuallymove)
        {
*/
        DrawDragBox(windownode, xstart, ystart, xend, yend);
        xend = xx;
        yend = yy;
        SpinDragBox(windownode);
        DrawDragBox(windownode, xstart, ystart, xend, yend);
/*
        }
*/
      break;

    case DRAGCREATE:
/*
      if(actuallymove)
        {
*/
        DrawDragBox(windownode, xstart, ystart, xend, yend);
        xend = ConvertGridX(xx);
        yend = ConvertGridY(yy);
        SpinDragBox(windownode);
        DrawDragBox(windownode, xstart, ystart, xend, yend);
/*
        }
*/
      break;

    case DRAGSIZELR:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width + (xx-xmark));
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZELL:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + (xx-xmark));
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZEUR:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width + (xx-xmark));
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZEUL:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + (xx-xmark));
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZEL:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + (xx-xmark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZER:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->xend = ConvertGridX(newgadgetnode->NewGadget.ng_LeftEdge + newgadgetnode->NewGadget.ng_Width + (xx-xmark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZET:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGSIZEB:
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
        }

      SpinDragBox(windownode);

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected)
          {
          newgadgetnode->yend = ConvertGridY(newgadgetnode->NewGadget.ng_TopEdge + newgadgetnode->NewGadget.ng_Height + (yy-ymark));
          DrawDragBox(windownode, newgadgetnode->xstart, newgadgetnode->ystart, newgadgetnode->xend, newgadgetnode->yend);
          }
        }
      break;

    case DRAGNONE:
      if(realgadgets)
        ModifyIDCMP(windownode->Window, arrayidcmp | gadgetidcmp);
      else
        ModifyIDCMP(windownode->Window, arrayidcmp);
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

