
/****************************************************************************/
/*                                                                          */
/*  HandleKeys.c - IDCMP VanillaKey functions                               */
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

int HandleMainVanillaKey(UWORD code, UWORD qualifier)
  {
  struct WindowNode *windownode;

  DEBUGENTER("HandleMainVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  if(!realgadgets)
    {
    switch(code)
      {
      case 'e':  case 'E':
        gadgetkind = NULL;
        drag = DRAGNONE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 0, TAG_DONE);
        break;

      case 'b':  case 'B':
        gadgetkind = BUTTON_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 1, TAG_DONE);
        break;

      case 'h':  case 'H':
        gadgetkind = CHECKBOX_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 2, TAG_DONE);
        break;

      case 'c':  case 'C':
        gadgetkind = CYCLE_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 3, TAG_DONE);
        break;

      case 'i':  case 'I':
        gadgetkind = INTEGER_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 4, TAG_DONE);
        break;

      case 'l':  case 'L':
        gadgetkind = LISTVIEW_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 5, TAG_DONE);
        break;

      case 'x':  case 'X':
        gadgetkind = MX_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 6, TAG_DONE);
        break;

      case 'n':  case 'N':
        gadgetkind = NUMBER_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 7, TAG_DONE);
        break;

      case 'p':  case 'P':
        gadgetkind = PALETTE_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 8, TAG_DONE);
        break;

      case 'r':  case 'R':
        gadgetkind = SCROLLER_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 9, TAG_DONE);
        break;

      case 'd':  case 'D':
        gadgetkind = SLIDER_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 10, TAG_DONE);
        break;

      case 's':  case 'S':
        gadgetkind = STRING_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 11, TAG_DONE);
        break;

      case 't':  case 'T':
        gadgetkind = TEXT_KIND;
        drag = DRAGCREATE;
        if(W_Main) GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 12, TAG_DONE);
        break;
      }
    }
  else
    {
    SelectSelect();
    }

  switch(code)
    {
    case 'W': case 'w': case 23: /* 23 = Control-W */
      if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
        if(currentwindow) /* user window selected - find previous window */
          {
          windownode = (struct WindowNode *) currentwindow->Node.ln_Pred;
          if(windownode->Node.ln_Pred && windownode->Window)
            {
            ActivateWindow(windownode->Window);
            if(code == 23) WindowToFront(windownode->Window);
            }
          else if(W_Main) /* no previous user window - select main window */
            {
            ActivateWindow(W_Main);
            if(code == 23) WindowToFront(W_Main);
            }
          }
        else  /* main window selected - find last user window */
          {
          windownode = (struct WindowNode *) WindowList.lh_TailPred;
          if(windownode && windownode->Window)
            {
            ActivateWindow(windownode->Window);
            if(code == 23) WindowToFront(windownode->Window);
            }
          }
        }
      else
        {
        if(currentwindow) /* user window selected - find next window */
          {
          windownode = (struct WindowNode *) currentwindow->Node.ln_Succ;
          if(windownode->Node.ln_Succ && windownode->Window)
            {
            ActivateWindow(windownode->Window);
            if(code == 23) WindowToFront(windownode->Window);
            }
          else if(W_Main) /* no next user window - select main window */
            {
            ActivateWindow(W_Main);
            if(code == 23) WindowToFront(W_Main);
            }
          }
        else  /* main window selected - find first user window */
          {
          windownode = (struct WindowNode *) WindowList.lh_Head;
          if(windownode && windownode->Window)
            {
            ActivateWindow(windownode->Window);
            if(code == 23) WindowToFront(windownode->Window);
            }
          }
        }
      break;

    case 'G':  case 'g':  TestGadgets();  break;
    case 'M':  case 'm':  TestMenus();    break;
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

int HandleEditVanillaKey(UWORD code, UWORD qualifier)
  {
  int done=FALSE;

  DEBUGENTER("HandleEditVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  if(edittagtype == TYPE_GADGET)
    {
    switch(code)
      {
      case 'H': case 'h':
        if(G_Highlight->Flags & SELECTED)
          GT_SetGadgetAttrs(G_Highlight, W_Edit, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_Highlight, W_Edit, NULL, GTCB_Checked, TRUE, TAG_DONE);
        break;

      case 'P': case 'p':
        if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
          {
          if(--textpos < 0) textpos = 4;
          }
        else
          {
          if(++textpos > 4) textpos = 0;
          }
        GT_SetGadgetAttrs(G_TextPos, W_Edit, NULL, GTCY_Active, textpos, TAG_DONE);
        break;

      case 'T': case 't': ActivateGadget(G_TopEdge, W_Edit, NULL);  break;
      case 'F': case 'f': ActivateGadget(G_LeftEdge, W_Edit, NULL); break;
      case 'W': case 'w': ActivateGadget(G_Width, W_Edit, NULL);    break;
      case 'E': case 'e': ActivateGadget(G_Height, W_Edit, NULL);   break;
      }
    }

  switch(code)
    {
    case 'I': case 'i':
      ActivateGadget(G_Title, W_Edit, NULL);
      break;

    case 'B': case 'b':
      ActivateGadget(G_Label, W_Edit, NULL);
      break;

    case 'X': case 'x':
      if(!(G_String->Flags & GFLG_DISABLED)) ActivateGadget(G_String, W_Edit, NULL);
      break;

    case 'L': case 'l':
      if(!(G_TagLabel->Flags & GFLG_DISABLED)) ActivateGadget(G_TagLabel, W_Edit, NULL);
      break;

    case 'A': case 'a':
      if(!(G_TagAdd->Flags & GFLG_DISABLED)) HandleTagAddGadget();
      break;

    case 'D': case 'd':
      if(!(G_TagDel->Flags & GFLG_DISABLED)) HandleTagDelGadget();
      break;

    case 'V': case 'v':
      if(availablecount)
        {
        if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
          {
          if(availablecode == 0) availablecode = availablecount-1; else availablecode--;
          }
        else
          {
          if(availablecode == availablecount-1) availablecode = 0; else availablecode++;
          }
        GT_SetGadgetAttrs(G_AvailableTags, W_Edit, NULL, GTLV_Selected, availablecode, GTLV_Top, availablecode, TAG_DONE);
        HandleAvailableGadget(availablecode);
        }
      break;

    case 'S': case 's':
      if(selectedcount)
        {
        if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
          {
          if(selectedcode == 0) selectedcode = selectedcount-1; else selectedcode--;
          }
        else
          {
          if(selectedcode == selectedcount-1) selectedcode = 0; else selectedcode++;
          }
        GT_SetGadgetAttrs(G_SelectedTags, W_Edit, NULL, GTLV_Selected, selectedcode, GTLV_Top, selectedcode, TAG_DONE);
        HandleSelectedGadget(selectedcode);
        }
      break;

    case 'G': case 'g':
      if(selectedtag)
        {
        if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
          {
          if(selectedstringnum == 0) selectedstringnum = selectedtag->StringCount-1; else selectedstringnum--;
          }
        else
          {
          if(selectedstringnum == selectedtag->StringCount-1) selectedstringnum = 0; else selectedstringnum++;
          }
        GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Top, selectedstringnum, TAG_DONE);
        HandleTagListGadget(selectedstringnum);
        }
      break;

    case '>': HandleTagUseGadget();    break;
    case '<': HandleTagRemoveGadget(); break;

    case 'O': case 'o': done = DONE_OK;     break;
    case 'C': case 'c': done = DONE_CANCEL; break;
    }

  DEBUGEXIT(TRUE, (int) done);
  return(done);
  }

/****************************************************************************/

int HandleIDCMPVanillaKey(UWORD code, UWORD qualifier)
  {
  int done=FALSE;

  DEBUGENTER("HandleIDCMPVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  switch(code)
    {
    case 'Z': case 'z':
      if(G_SIZEVERIFY->Flags & SELECTED)
        GT_SetGadgetAttrs(G_SIZEVERIFY, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_SIZEVERIFY, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'S': case 's':
      if(G_NEWSIZE->Flags & SELECTED)
        GT_SetGadgetAttrs(G_NEWSIZE, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_NEWSIZE, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'F': case 'f':
      if(G_REFRESHWINDOW->Flags & SELECTED)
        GT_SetGadgetAttrs(G_REFRESHWINDOW, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_REFRESHWINDOW, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'B': case 'b':
      if(G_MOUSEBUTTONS->Flags & SELECTED)
        GT_SetGadgetAttrs(G_MOUSEBUTTONS, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_MOUSEBUTTONS, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'E': case 'e':
      if(G_MOUSEMOVE->Flags & SELECTED)
        GT_SetGadgetAttrs(G_MOUSEMOVE, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_MOUSEMOVE, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'D': case 'd':
      if(G_GADGETDOWN->Flags & SELECTED)
        GT_SetGadgetAttrs(G_GADGETDOWN, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_GADGETDOWN, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'U': case 'u':
      if(G_GADGETUP->Flags & SELECTED)
        GT_SetGadgetAttrs(G_GADGETUP, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_GADGETUP, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'M': case 'm':
      if(G_MENUPICK->Flags & SELECTED)
        GT_SetGadgetAttrs(G_MENUPICK, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_MENUPICK, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'W': case 'w':
      if(G_CLOSEWINDOW->Flags & SELECTED)
        GT_SetGadgetAttrs(G_CLOSEWINDOW, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_CLOSEWINDOW, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'R': case 'r':
      if(G_RAWKEY->Flags & SELECTED)
        GT_SetGadgetAttrs(G_RAWKEY, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_RAWKEY, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'Y': case 'y':
      if(G_MENUVERIFY->Flags & SELECTED)
        GT_SetGadgetAttrs(G_MENUVERIFY, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_MENUVERIFY, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'P': case 'p':
      if(G_NEWPREFS->Flags & SELECTED)
        GT_SetGadgetAttrs(G_NEWPREFS, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_NEWPREFS, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'N': case 'n':
      if(G_DISKINSERTED->Flags & SELECTED)
        GT_SetGadgetAttrs(G_DISKINSERTED, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_DISKINSERTED, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'K': case 'k':
      if(G_DISKREMOVED->Flags & SELECTED)
        GT_SetGadgetAttrs(G_DISKREMOVED, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_DISKREMOVED, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'A': case 'a':
      if(G_ACTIVEWINDOW->Flags & SELECTED)
        GT_SetGadgetAttrs(G_ACTIVEWINDOW, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_ACTIVEWINDOW, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'I': case 'i':
      if(G_INACTIVEWINDOW->Flags & SELECTED)
        GT_SetGadgetAttrs(G_INACTIVEWINDOW, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_INACTIVEWINDOW, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'V': case 'v':
      if(G_VANILLAKEY->Flags & SELECTED)
        GT_SetGadgetAttrs(G_VANILLAKEY, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_VANILLAKEY, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'T': case 't':
      if(G_INTUITICKS->Flags & SELECTED)
        GT_SetGadgetAttrs(G_INTUITICKS, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_INTUITICKS, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'L': case 'l':
      if(G_MENUHELP->Flags & SELECTED)
        GT_SetGadgetAttrs(G_MENUHELP, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_MENUHELP, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'H': case 'h':
      if(G_CHANGEWINDOW->Flags & SELECTED)
        GT_SetGadgetAttrs(G_CHANGEWINDOW, W_IDCMP, NULL, GTCB_Checked, FALSE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_CHANGEWINDOW, W_IDCMP, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;

    case 'O': case 'o': done = DONE_OK;     break;
    case 'C': case 'c': done = DONE_CANCEL; break;
    }

  DEBUGEXIT(TRUE, (int) done);
  return(done);
  }

/****************************************************************************/

int HandleMenuVanillaKey(UWORD code, UWORD qualifier)
  {
  int done=FALSE;

  DEBUGENTER("HandleMenuVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  switch(code)
    {
    case 'M': case 'm':
      if(menupart != 1)
        {
        ShowCurrentMenuList(1);
        }
      else
        {
        if(menucount)
          {
          if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
            {
            if(selectedmenunum == 0) selectedmenunum = menucount-1; else selectedmenunum--;
            }
          else
            {
            if(selectedmenunum == menucount-1) selectedmenunum = 0; else selectedmenunum++;
            }
          GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Selected, selectedmenunum, GTLV_Top, selectedmenunum, TAG_DONE);
          HandleMenuListGadget(selectedmenunum);
          }
        }
      break;

    case 'I': case 'i':
      if(menupart != 2)
        {
        ShowCurrentMenuList(2);
        }
      else
        {
        if(selectedmenu->ItemCount)
          {
          if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
            {
            if(selecteditemnum == 0) selecteditemnum = selectedmenu->ItemCount-1; else selecteditemnum--;
            }
          else
            {
            if(selecteditemnum == selectedmenu->ItemCount-1) selecteditemnum = 0; else selecteditemnum++;
            }
          GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Selected, selecteditemnum, GTLV_Top, selecteditemnum, TAG_DONE);
          HandleItemListGadget(selecteditemnum);
          }
        }
      break;

    case 'S': case 's':
      if(menupart != 3)
        {
        ShowCurrentMenuList(3);
        }
      else
        {
        if(selecteditem->SubCount)
          {
          if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
            {
            if(selectedsubnum == 0) selectedsubnum = selecteditem->SubCount-1; else selectedsubnum--;
            }
          else
            {
            if(selectedsubnum == selecteditem->SubCount-1) selectedsubnum = 0; else selectedsubnum++;
            }
          GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Selected, selectedsubnum, GTLV_Top, selectedsubnum, TAG_DONE);
          HandleSubListGadget(selectedsubnum);
          }
        }
      break;

    case 'X': case 'x':
      switch(menupart)
        {
        case 1: if(!(G_MenuString->Flags & GFLG_DISABLED)) ActivateGadget(G_MenuString, W_Menu, NULL); break;
        case 2: if(!(G_ItemString->Flags & GFLG_DISABLED)) ActivateGadget(G_ItemString, W_Menu, NULL); break;
        case 3: if(!(G_SubString->Flags & GFLG_DISABLED))  ActivateGadget(G_SubString, W_Menu, NULL);  break;
        }
      break;

    case 'L': case 'l':
      switch(menupart)
        {
        case 1: if(!(G_MenuString->Flags & GFLG_DISABLED)) ActivateGadget(G_MenuLabel, W_Menu, NULL); break;
        case 2: if(!(G_ItemString->Flags & GFLG_DISABLED)) ActivateGadget(G_ItemLabel, W_Menu, NULL); break;
        case 3: if(!(G_SubString->Flags & GFLG_DISABLED))  ActivateGadget(G_SubLabel, W_Menu, NULL);  break;
        }
      break;

    case 'A': case 'a':
      switch(menupart)
        {
        case 1:  if(!(G_MenuAdd->Flags & GFLG_DISABLED)) HandleMenuAddGadget(); break;
        case 2:  if(!(G_ItemAdd->Flags & GFLG_DISABLED)) HandleItemAddGadget(); break;
        case 3:  if(!(G_SubAdd->Flags & GFLG_DISABLED))  HandleSubAddGadget();  break;
        }
      break;

    case 'D': case 'd':
      switch(menupart)
        {
        case 1:  if(!(G_MenuDel->Flags & GFLG_DISABLED)) HandleMenuDelGadget(); break;
        case 2:  if(!(G_ItemDel->Flags & GFLG_DISABLED)) HandleItemDelGadget(); break;
        case 3:  if(!(G_SubDel->Flags & GFLG_DISABLED))  HandleSubDelGadget();  break;
        }
      break;

    case 'B': case 'b':
      switch(menupart)
        {
        case 1:
          if(!(G_MenuDisabled->Flags & GFLG_DISABLED))
            {
            if(G_MenuDisabled->Flags & SELECTED)
              GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
            else
              GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
            HandleMenuDisabledGadget();
            }
          break;

        case 2:
          if(!(G_ItemDisabled->Flags & GFLG_DISABLED))
            {
            if(G_ItemDisabled->Flags & SELECTED)
              GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
            else
              GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
            HandleItemDisabledGadget();
            }
          break;

        case 3:
          if(!(G_SubDisabled->Flags & GFLG_DISABLED))
            {
            if(G_SubDisabled->Flags & SELECTED)
              GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
            else
              GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
            HandleSubDisabledGadget();
            }
          break;
        }
      break;

    case 'H': case 'h':
      if(!(G_CheckIt->Flags & GFLG_DISABLED))
        {
        if(G_CheckIt->Flags & SELECTED)
          GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleCheckItGadget();
        }
      break;

    case 'K': case 'k':
      if(!(G_Checked->Flags & GFLG_DISABLED))
        {
        if(G_Checked->Flags & SELECTED)
          GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleCheckedGadget();
        }
      break;

    case 'T': case 't':
      if(!(G_Toggle->Flags & GFLG_DISABLED))
        {
        if(G_Toggle->Flags & SELECTED)
          GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleToggleGadget();
        }
      break;

    case 'E': case 'e':
      if(!(G_Exclude->Flags & GFLG_DISABLED))
        {
        if(G_Exclude->Flags & SELECTED)
          GT_SetGadgetAttrs(G_Exclude, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_Exclude, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleExcludeGadget();
        }
      break;

    case 'Y': case 'y':
      if(!(G_CommKey->Flags & GFLG_DISABLED))
        {
        ActivateGadget(G_CommKey, W_Menu, NULL);
        }
      break;

    case 'O': case 'o': done = DONE_OK;     break;
    case 'C': case 'c': done = DONE_CANCEL; break;
    }

  DEBUGEXIT(TRUE, (int) done);
  return(done);
  }

/****************************************************************************/

int HandleScreenVanillaKey(UWORD code, UWORD qualifier)
  {
  int done=FALSE;

  DEBUGENTER("HandleScreenVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  switch(code)
    {
    case 'B': case 'b':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 0, TAG_DONE);
      HandleScreenModeGadget(0);
      break;

    case 'R': case 'r':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 1, TAG_DONE);
      HandleScreenModeGadget(1);
      break;

    case 'U': case 'u':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 2, TAG_DONE);
      HandleScreenModeGadget(2);
      break;

    case 'Y': case 'y':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 3, TAG_DONE);
      HandleScreenModeGadget(3);
      break;

    case '0':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 4, TAG_DONE);
      HandleScreenModeGadget(4);
      break;

    case '5':
      GT_SetGadgetAttrs(G_ScreenMode, W_Screen, NULL, GTMX_Active, 5, TAG_DONE);
      HandleScreenModeGadget(5);
      break;

    case 'V': case 'v':
      if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
        if(--ntscpalcode < 0) ntscpalcode = 2;
        }
      else
        {
        if(++ntscpalcode > 2) ntscpalcode = 0;
        }
      GT_SetGadgetAttrs(G_NtscPal, W_Screen, NULL, GTCY_Active, ntscpalcode, TAG_DONE);
      HandleNtscPalGadget(ntscpalcode);
      break;

    case 'S': case 's':
      if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
        if(--oscancode < 0) oscancode = 3;
        }
      else
        {
        if(++oscancode > 3) oscancode = 0;
        }
      GT_SetGadgetAttrs(G_Overscan, W_Screen, NULL, GTCY_Active, oscancode, TAG_DONE);
      HandleOverscanGadget(oscancode);
      break;

    case 'I': case 'i':
      if(!(G_Interlace->Flags & GFLG_DISABLED))
        {
        if(G_Interlace->Flags & SELECTED)
          GT_SetGadgetAttrs(G_Interlace, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_Interlace, W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleInterlaceGadget();
        }
      break;

    case 'P': case 'p':
      if(!(G_CustomColors->Flags & GFLG_DISABLED))
        {
        if(G_CustomColors->Flags & SELECTED)
          GT_SetGadgetAttrs(G_CustomColors, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_CustomColors, W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleCustomColorsGadget();
        }
      break;

    case 'E': case 'e':
      if(!(G_ScreenDWidth->Flags & GFLG_DISABLED))
        {
        if(G_ScreenDWidth->Flags & SELECTED)
          GT_SetGadgetAttrs(G_ScreenDWidth, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_ScreenDWidth, W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleScreenDWidthGadget();
        }
      break;

    case 'F': case 'f':
      if(!(G_ScreenDHeight->Flags & GFLG_DISABLED))
        {
        if(G_ScreenDHeight->Flags & SELECTED)
          GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
        else
          GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
        HandleScreenDHeightGadget();
        }
      break;

    case 'D': case 'd':
      if(qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
        {
        if(--newscreennode.Depth < 1) newscreennode.Depth = maxdepth;
        }
      else
        {
        if(++newscreennode.Depth > maxdepth) newscreennode.Depth = 1;
        }
      GT_SetGadgetAttrs(G_ScreenDepth, W_Screen, NULL, GTSL_Level, newscreennode.Depth, TAG_DONE);
      HandleScreenDepthGadget(newscreennode.Depth);
      break;

    case 'W': case 'w': if(!(G_ScreenWidth->Flags & GFLG_DISABLED)) ActivateGadget(G_ScreenWidth, W_Screen, NULL);  break;
    case 'H': case 'h': if(!(G_ScreenHeight->Flags & GFLG_DISABLED)) ActivateGadget(G_ScreenHeight, W_Screen, NULL); break;

    case 'O': case 'o': done = DONE_OK;     break;
    case 'C': case 'c': done = DONE_CANCEL; break;
    }

  DEBUGEXIT(TRUE, (int) done);
  return(done);
  }

/****************************************************************************/

int HandleColorsVanillaKey(UWORD code, UWORD qualifier)
  {
  int done=FALSE;

  DEBUGENTER("HandleColorsVanillaKey", NULL);

  DEBUGTEXT("code = ", code);
  DEBUGTEXT("qualifier = ", qualifier);

  switch(code)
    {
    case 'O': case 'o': done = DONE_OK;     break;
    case 'C': case 'c': done = DONE_CANCEL; break;
    }

  DEBUGEXIT(TRUE, (int) done);
  return(done);
  }

