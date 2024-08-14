
#include "Toolmaker.h"
#include "Externs.h"

#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

void EditIDCMP(void)
  {
  BOOL   done=FALSE;
  BOOL   error=FALSE;
  struct NewGadget NG_Temp;
  struct TagNode   *tagnode;
  struct Gadget    *G_List, *G_Context;
  struct Gadget    *G_OK, *G_Cancel;

  DEBUGENTER("EditIDCMP", NULL);

  MainWindowSleep();

  G_List = NULL;
  G_Context = CreateContext(&G_List);

  NG_Temp.ng_TextAttr = &TOPAZ80;
  NG_Temp.ng_VisualInfo = VisualInfo;
  NG_Temp.ng_UserData = NULL;
  NG_Temp.ng_Width = 0;
  NG_Temp.ng_Height = 0;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;

  /* First column */

  NG_Temp.ng_LeftEdge = 10;

  NG_Temp.ng_TopEdge = currenttopborder+8;
  NG_Temp.ng_GadgetText = "MOUSE_BUTTONS";
  NG_Temp.ng_GadgetID = ID_MOUSEBUTTONS;
  G_MOUSEBUTTONS = CreateGadget(CHECKBOX_KIND, G_Context, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_MOUSEBUTTONS, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+20;
  NG_Temp.ng_GadgetText = "MOUS_EMOVE";
  NG_Temp.ng_GadgetID = ID_MOUSEMOVE;
  G_MOUSEMOVE = CreateGadget(CHECKBOX_KIND, G_MOUSEBUTTONS, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_MOUSEMOVE, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+32;
  NG_Temp.ng_GadgetText = "DELTAMOVE";
  NG_Temp.ng_GadgetID = ID_DELTAMOVE;
  G_DELTAMOVE = CreateGadget(CHECKBOX_KIND, G_MOUSEMOVE, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_DELTAMOVE, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+44;
  NG_Temp.ng_GadgetText = "GADGET_DOWN";
  NG_Temp.ng_GadgetID = ID_GADGETDOWN;
  G_GADGETDOWN = CreateGadget(CHECKBOX_KIND, G_DELTAMOVE, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_GADGETDOWN, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+56;
  NG_Temp.ng_GadgetText = "GADGET_UP";
  NG_Temp.ng_GadgetID = ID_GADGETUP;
  G_GADGETUP = CreateGadget(CHECKBOX_KIND, G_GADGETDOWN, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_GADGETUP, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+68;
  NG_Temp.ng_GadgetText = "CLOSE_WINDOW";
  NG_Temp.ng_GadgetID = ID_CLOSEWINDOW;
  G_CLOSEWINDOW = CreateGadget(CHECKBOX_KIND, G_GADGETUP, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_CLOSEWINDOW, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+80;
  NG_Temp.ng_GadgetText = "_MENUPICK";
  NG_Temp.ng_GadgetID = ID_MENUPICK;
  G_MENUPICK = CreateGadget(CHECKBOX_KIND, G_CLOSEWINDOW, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_MENUPICK, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+92;
  NG_Temp.ng_GadgetText = "MENUVERIF_Y";
  NG_Temp.ng_GadgetID = ID_MENUVERIFY;
  G_MENUVERIFY = CreateGadget(CHECKBOX_KIND, G_MENUPICK, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_MENUVERIFY, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+104;
  NG_Temp.ng_GadgetText = "MENUHE_LP";
  NG_Temp.ng_GadgetID = ID_MENUHELP;
  G_MENUHELP = CreateGadget(CHECKBOX_KIND, G_MENUVERIFY, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_MENUHELP, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+116;
  NG_Temp.ng_GadgetText = "REQSET";
  NG_Temp.ng_GadgetID = ID_REQSET;
  G_REQSET = CreateGadget(CHECKBOX_KIND, G_MENUHELP, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_REQSET, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+128;
  NG_Temp.ng_GadgetText = "REQCLEAR";
  NG_Temp.ng_GadgetID = ID_REQCLEAR;
  G_REQCLEAR = CreateGadget(CHECKBOX_KIND, G_REQSET, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_REQCLEAR, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+140;
  NG_Temp.ng_GadgetText = "REQVERIFY";
  NG_Temp.ng_GadgetID = ID_REQVERIFY;
  G_REQVERIFY = CreateGadget(CHECKBOX_KIND, G_REQCLEAR, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_REQVERIFY, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+152;
  NG_Temp.ng_GadgetText = "NEW_SIZE";
  NG_Temp.ng_GadgetID = ID_NEWSIZE;
  G_NEWSIZE = CreateGadget(CHECKBOX_KIND, G_REQVERIFY, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_NEWSIZE, GT_Underscore, '_', TAG_DONE);

  /* Second column */

  NG_Temp.ng_LeftEdge = 200;

  NG_Temp.ng_TopEdge = currenttopborder+8;
  NG_Temp.ng_GadgetText = "C_HANGEWINDOW";
  NG_Temp.ng_GadgetID = ID_CHANGEWINDOW;
  G_CHANGEWINDOW = CreateGadget(CHECKBOX_KIND, G_NEWSIZE, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_CHANGEWINDOW, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+20;
  NG_Temp.ng_GadgetText = "RE_FRESHWINDOW";
  NG_Temp.ng_GadgetID = ID_REFRESHWINDOW;
  G_REFRESHWINDOW = CreateGadget(CHECKBOX_KIND, G_CHANGEWINDOW, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_REFRESHWINDOW, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+32;
  NG_Temp.ng_GadgetText = "SI_ZEVERIFY";
  NG_Temp.ng_GadgetID = ID_SIZEVERIFY;
  G_SIZEVERIFY = CreateGadget(CHECKBOX_KIND, G_REFRESHWINDOW, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_SIZEVERIFY, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+44;
  NG_Temp.ng_GadgetText = "_ACTIVEWINDOW";
  NG_Temp.ng_GadgetID = ID_ACTIVEWINDOW;
  G_ACTIVEWINDOW = CreateGadget(CHECKBOX_KIND, G_SIZEVERIFY, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_ACTIVEWINDOW, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+56;
  NG_Temp.ng_GadgetText = "_INACTIVEWINDOW";
  NG_Temp.ng_GadgetID = ID_INACTIVEWINDOW;
  G_INACTIVEWINDOW = CreateGadget(CHECKBOX_KIND, G_ACTIVEWINDOW, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_INACTIVEWINDOW, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+68;
  NG_Temp.ng_GadgetText = "_VANILLAKEY";
  NG_Temp.ng_GadgetID = ID_VANILLAKEY;
  G_VANILLAKEY = CreateGadget(CHECKBOX_KIND, G_INACTIVEWINDOW, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_VANILLAKEY, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+80;
  NG_Temp.ng_GadgetText = "_RAWKEY";
  NG_Temp.ng_GadgetID = ID_RAWKEY;
  G_RAWKEY = CreateGadget(CHECKBOX_KIND, G_VANILLAKEY, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_RAWKEY, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+92;
  NG_Temp.ng_GadgetText = "NEW_PREFS";
  NG_Temp.ng_GadgetID = ID_NEWPREFS;
  G_NEWPREFS = CreateGadget(CHECKBOX_KIND, G_RAWKEY, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_NEWPREFS, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+104;
  NG_Temp.ng_GadgetText = "DISKI_NSERTED";
  NG_Temp.ng_GadgetID = ID_DISKINSERTED;
  G_DISKINSERTED = CreateGadget(CHECKBOX_KIND, G_NEWPREFS, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_DISKINSERTED, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+116;
  NG_Temp.ng_GadgetText = "DIS_KREMOVED";
  NG_Temp.ng_GadgetID = ID_DISKREMOVED;
  G_DISKREMOVED = CreateGadget(CHECKBOX_KIND, G_DISKINSERTED, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_DISKREMOVED, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+128;
  NG_Temp.ng_GadgetText = "IN_TUITICKS";
  NG_Temp.ng_GadgetID = ID_INTUITICKS;
  G_INTUITICKS = CreateGadget(CHECKBOX_KIND, G_DISKREMOVED, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_INTUITICKS, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_TopEdge = currenttopborder+140;
  NG_Temp.ng_GadgetText = "IDCMPUPDATE";
  NG_Temp.ng_GadgetID = ID_IDCMPUPDATE;
  G_IDCMPUPDATE = CreateGadget(CHECKBOX_KIND, G_INTUITICKS, &NG_Temp, GTCB_Checked, currentwindow->IDCMP&IDCMP_IDCMPUPDATE, GT_Underscore, '_', TAG_DONE);

  /* OK and Cancel */

  NG_Temp.ng_TopEdge = currenttopborder+170;
  NG_Temp.ng_Width = 80;
  NG_Temp.ng_Height = 15;
  NG_Temp.ng_Flags = PLACETEXT_IN;

  NG_Temp.ng_LeftEdge = 10;
  NG_Temp.ng_GadgetText = "_OK";
  NG_Temp.ng_GadgetID = ID_OK;
  G_OK = CreateGadget(BUTTON_KIND, G_IDCMPUPDATE, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 265;
  NG_Temp.ng_GadgetText = "_Cancel";
  NG_Temp.ng_GadgetID = ID_CANCEL;
  G_Cancel = CreateGadget(BUTTON_KIND, G_OK, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  if(G_Cancel)
    {
    if(W_IDCMP = OpenWindowTags(NULL,
                                WA_PubScreen, screennode.Screen,
                                WA_Left, CenterHoriz(350),
                                WA_Top,  CenterVert(190),
                                WA_IDCMP, IDCMPidcmp,
                                WA_Gadgets, G_List,
                                WA_Title, "IDCMP",
                                WA_Activate, TRUE,
                                WA_DragBar, TRUE,
                                WA_DepthGadget, TRUE,
                                WA_SimpleRefresh, TRUE,
                                WA_InnerWidth, 350,
                                WA_InnerHeight, 190,
                                TAG_DONE))
      {
      GT_RefreshWindow(W_IDCMP, NULL);

      IDCMPsignal = 1L << W_IDCMP->UserPort->mp_SigBit;

      while(!done)
        {
        Wait(IDCMPsignal);
        done = HandleRequesterIDCMP(W_IDCMP);
        }

      ModifyIDCMP(W_IDCMP, NULL);
      IDCMPsignal = NULL;

      switch(done)
        {
        case DONE_CANCEL:
          break;

        case DONE_OK:
          currentwindow->IDCMP = NULL;
          if(G_SIZEVERIFY->Flags & SELECTED)     currentwindow->IDCMP |= IDCMP_SIZEVERIFY;
          if(G_NEWSIZE->Flags & SELECTED)        currentwindow->IDCMP |= IDCMP_NEWSIZE;
          if(G_REFRESHWINDOW->Flags & SELECTED)  currentwindow->IDCMP |= IDCMP_REFRESHWINDOW;
          if(G_MOUSEBUTTONS->Flags & SELECTED)   currentwindow->IDCMP |= IDCMP_MOUSEBUTTONS;
          if(G_MOUSEMOVE->Flags & SELECTED)      currentwindow->IDCMP |= IDCMP_MOUSEMOVE;
          if(G_GADGETDOWN->Flags & SELECTED)     currentwindow->IDCMP |= IDCMP_GADGETDOWN;
          if(G_GADGETUP->Flags & SELECTED)       currentwindow->IDCMP |= IDCMP_GADGETUP;
          if(G_REQSET->Flags & SELECTED)         currentwindow->IDCMP |= IDCMP_REQSET;
          if(G_MENUPICK->Flags & SELECTED)       currentwindow->IDCMP |= IDCMP_MENUPICK;
          if(G_CLOSEWINDOW->Flags & SELECTED)    currentwindow->IDCMP |= IDCMP_CLOSEWINDOW;
          if(G_RAWKEY->Flags & SELECTED)         currentwindow->IDCMP |= IDCMP_RAWKEY;
          if(G_REQVERIFY->Flags & SELECTED)      currentwindow->IDCMP |= IDCMP_REQVERIFY;
          if(G_REQCLEAR->Flags & SELECTED)       currentwindow->IDCMP |= IDCMP_REQCLEAR;
          if(G_MENUVERIFY->Flags & SELECTED)     currentwindow->IDCMP |= IDCMP_MENUVERIFY;
          if(G_NEWPREFS->Flags & SELECTED)       currentwindow->IDCMP |= IDCMP_NEWPREFS;
          if(G_DISKINSERTED->Flags & SELECTED)   currentwindow->IDCMP |= IDCMP_DISKINSERTED;
          if(G_DISKREMOVED->Flags & SELECTED)    currentwindow->IDCMP |= IDCMP_DISKREMOVED;
          if(G_ACTIVEWINDOW->Flags & SELECTED)   currentwindow->IDCMP |= IDCMP_ACTIVEWINDOW;
          if(G_INACTIVEWINDOW->Flags & SELECTED) currentwindow->IDCMP |= IDCMP_INACTIVEWINDOW;
          if(G_DELTAMOVE->Flags & SELECTED)      currentwindow->IDCMP |= IDCMP_DELTAMOVE;
          if(G_VANILLAKEY->Flags & SELECTED)     currentwindow->IDCMP |= IDCMP_VANILLAKEY;
          if(G_INTUITICKS->Flags & SELECTED)     currentwindow->IDCMP |= IDCMP_INTUITICKS;
          if(G_IDCMPUPDATE->Flags & SELECTED)    currentwindow->IDCMP |= IDCMP_IDCMPUPDATE;
          if(G_MENUHELP->Flags & SELECTED)       currentwindow->IDCMP |= IDCMP_MENUHELP;
          if(G_CHANGEWINDOW->Flags & SELECTED)   currentwindow->IDCMP |= IDCMP_CHANGEWINDOW;

          for(tagnode = (struct TagNode *) currentwindow->TagList.lh_Head; tagnode->Node.ln_Succ;
              tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
            {
            if(tagnode->TagItem.ti_Tag == WA_IDCMP) tagnode->TagItem.ti_Data = currentwindow->IDCMP;
            }

          modified = TRUE;
          break;
        }

      CloseWindow(W_IDCMP);
      }
    else
      {
      error = TRUE;
      }

    FreeGadgets(G_List);
    }
  else
    {
    error = TRUE;
    }

  if(error)
    {
    PutError("Out of memory\nopening IDCMP window", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

