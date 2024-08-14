
/****************************************************************************/
/*                                                                          */
/*  Handlers.c - IDCMP message handling functions                           */
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

int HandleGadgetDown(struct Gadget *gadget, ULONG code)
  {
  DEBUGENTER("HandleGadgetDown", NULL);

  switch(gadget->GadgetID)
    {
    case ID_SCREENMODE:    HandleScreenModeGadget(code);    break;

    case ID_NEWGADGET:
      if(!realgadgets)
        {
        switch(code)
          {
          case 0:  gadgetkind = NULL;           drag = DRAGNONE;    break;
          case 1:  gadgetkind = BUTTON_KIND;    drag = DRAGCREATE;  break;
          case 2:  gadgetkind = CHECKBOX_KIND;  drag = DRAGCREATE;  break;
          case 3:  gadgetkind = CYCLE_KIND;     drag = DRAGCREATE;  break;
          case 4:  gadgetkind = INTEGER_KIND;   drag = DRAGCREATE;  break;
          case 5:  gadgetkind = LISTVIEW_KIND;  drag = DRAGCREATE;  break;
          case 6:  gadgetkind = MX_KIND;        drag = DRAGCREATE;  break;
          case 7:  gadgetkind = NUMBER_KIND;    drag = DRAGCREATE;  break;
          case 8:  gadgetkind = PALETTE_KIND;   drag = DRAGCREATE;  break;
          case 9:  gadgetkind = SCROLLER_KIND;  drag = DRAGCREATE;  break;
          case 10: gadgetkind = SLIDER_KIND;    drag = DRAGCREATE;  break;
          case 11: gadgetkind = STRING_KIND;    drag = DRAGCREATE;  break;
          case 12: gadgetkind = TEXT_KIND;      drag = DRAGCREATE;  break;
          }
        }
      else
        {
        SelectSelect();
        drag = DRAGNONE;
        }

      break;
    }

  DEBUGEXIT(TRUE, NULL);
  return(0);
  }

/****************************************************************************/

int HandleGadgetUp(struct Gadget *gadget, ULONG code)
  {
  int done=FALSE;

  DEBUGENTER("HandleGadgetUp", NULL);

  switch(gadget->GadgetID)
    {
    case ID_TEXTPOS:       HandleTextPosGadget(code);       break;

    case ID_TEXT:          HandleTitleGadget();             break;
    case ID_TAGLIST:       HandleTagListGadget(code);       break;
    case ID_TAGSTRING:     HandleTagStringGadget(gadget);   break;
    case ID_TAGLABEL:      HandleTagLabelGadget(gadget);    break;
    case ID_AVAILABLE:     HandleAvailableGadget(code);     break;
    case ID_SELECTED:      HandleSelectedGadget(code);      break;
    case ID_TAGUSE:        HandleTagUseGadget();            break;
    case ID_TAGREMOVE:     HandleTagRemoveGadget();         break;
    case ID_TAGADD:        HandleTagAddGadget();            break;
    case ID_TAGDEL:        HandleTagDelGadget();            break;
    case ID_TAGUP:         HandleTagUpGadget();             break;
    case ID_TAGDOWN:       HandleTagDownGadget();           break;

    case ID_MENULIST:      HandleMenuListGadget(code);      break;
    case ID_MENUSTRING:    HandleMenuStringGadget();        break;
    case ID_MENULABEL:     HandleMenuLabelGadget();         break;
    case ID_MENUADD:       HandleMenuAddGadget();           break;
    case ID_MENUDEL:       HandleMenuDelGadget();           break;
    case ID_MENUUP:        HandleMenuUpGadget();            break;
    case ID_MENUDOWN:      HandleMenuDownGadget();          break;
    case ID_MENUDISABLED:  HandleMenuDisabledGadget();      break;

    case ID_ITEMLIST:      HandleItemListGadget(code);      break;
    case ID_ITEMSTRING:    HandleItemStringGadget();        break;
    case ID_ITEMLABEL:     HandleItemLabelGadget();         break;
    case ID_ITEMADD:       HandleItemAddGadget();           break;
    case ID_ITEMDEL:       HandleItemDelGadget();           break;
    case ID_ITEMUP:        HandleItemUpGadget();            break;
    case ID_ITEMDOWN:      HandleItemDownGadget();          break;
    case ID_ITEMDISABLED:  HandleItemDisabledGadget();      break;

    case ID_SUBLIST:       HandleSubListGadget(code);       break;
    case ID_SUBSTRING:     HandleSubStringGadget();         break;
    case ID_SUBLABEL:      HandleSubLabelGadget();          break;
    case ID_SUBADD:        HandleSubAddGadget();            break;
    case ID_SUBDEL:        HandleSubDelGadget();            break;
    case ID_SUBUP:         HandleSubUpGadget();             break;
    case ID_SUBDOWN:       HandleSubDownGadget();           break;
    case ID_SUBDISABLED:   HandleSubDisabledGadget();       break;

    case ID_CHECKIT:       HandleCheckItGadget();           break;
    case ID_CHECKED:       HandleCheckedGadget();           break;
    case ID_TOGGLE:        HandleToggleGadget();            break;
    case ID_COMMKEY:       HandleCommKeyGadget();           break;
    case ID_EXCLUDE:       HandleExcludeGadget();           break;

    case ID_NTSCPAL:       HandleNtscPalGadget(code);       break;
    case ID_OVERSCAN:      HandleOverscanGadget(code);      break;
    case ID_SCREENDEPTH:   HandleScreenDepthGadget(code);   break;
    case ID_SCREENWIDTH:   HandleScreenWidthGadget();       break;
    case ID_SCREENHEIGHT:  HandleScreenHeightGadget();      break;
    case ID_SCREENDWIDTH:  HandleScreenDWidthGadget();      break;
    case ID_SCREENDHEIGHT: HandleScreenDHeightGadget();     break;
    case ID_INTERLACE:     HandleInterlaceGadget();         break;
    case ID_CUSTOMCOLORS:  HandleCustomColorsGadget();      break;

    case ID_SCREENPALETTE: HandleScreenPaletteGadget(code); break;
    case ID_SCREENRED:     HandleScreenRedGadget(code);     break;
    case ID_SCREENGREEN:   HandleScreenGreenGadget(code);   break;
    case ID_SCREENBLUE:    HandleScreenBlueGadget(code);    break;

    case ID_CANCEL:        done = DONE_CANCEL;              break;
    case ID_OK:            done = DONE_OK;                  break;

    case ID_TESTGADGETS:   TestGadgets();                   break;
    case ID_TESTMENUS:     TestMenus();                     break;
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

/****************************************************************************/

int HandleGadgetMove(struct Gadget *gadget, ULONG code)
  {
  int done=FALSE;

  DEBUGENTER("HandleGadgetMove", NULL);

  switch(gadget->GadgetID)
    {
    case ID_SCREENRED:     HandleScreenRedGadget(code);     break;
    case ID_SCREENGREEN:   HandleScreenGreenGadget(code);   break;
    case ID_SCREENBLUE:    HandleScreenBlueGadget(code);    break;
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

/****************************************************************************/

void HandleChangeWindow(struct WindowNode *windownode)
  {
  struct TagNode *tagnode;

  DEBUGENTER("HandleChangeWindow", NULL);

  modified = TRUE;

  for(tagnode = (struct TagNode *) windownode->TagList.lh_Head; tagnode->Node.ln_Succ;
      tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
    {
    switch(tagnode->TagItem.ti_Tag)
      {
      case WA_Left:
        tagnode->TagItem.ti_Data = windownode->Window->LeftEdge;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;

      case WA_Top:
        tagnode->TagItem.ti_Data = windownode->Window->TopEdge;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;

      case WA_Width:
        tagnode->TagItem.ti_Data = windownode->Window->Width;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;

      case WA_Height:
        tagnode->TagItem.ti_Data = windownode->Window->Height;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;

      case WA_InnerWidth:
        tagnode->TagItem.ti_Data = windownode->Window->Width -
                                   windownode->Window->BorderLeft -
                                   windownode->Window->BorderRight;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;

      case WA_InnerHeight:
        tagnode->TagItem.ti_Data = windownode->Window->Height -
                                   windownode->Window->BorderTop -
                                   windownode->Window->BorderBottom;
        sprintf((char *) tagnode->Data, "%d", tagnode->TagItem.ti_Data);
        break;
      }
    }

  DEBUGEXIT(FALSE, NULL);
  }

/*************************************************************************/

int HandleMenuPick(ULONG menucode)
  {
  int    done, gcount;
  struct NewGadgetNode *newgadgetnode;
  struct MenuItem *menuitem;
  ULONG  itemnum, menunum, subnum;
  USHORT flags;

  DEBUGENTER("HandleMenuPick", NULL);

  done = FALSE;

  while(menucode != MENUNULL)
    {
    menunum = MENUNUM(menucode);
    itemnum = ITEMNUM(menucode);
    subnum  = SUBNUM(menucode);
    flags = (USHORT)((struct MenuItem *)ItemAddress(M_Main, menucode)->Flags);

    switch(menunum)
      {
      case MENU_PROJECT:
        switch(itemnum)
          {
          case ITEM_NEW:
            if(RequestKill()) NewProject();
            break;

          case ITEM_OPEN:
            filename[0] = '\0';
            OpenFile();
            break;

#ifndef DEMO
          case ITEM_SAVE:
            SaveFile();
            break;

          case ITEM_SAVEAS:
            filename[0] = '\0';
            SaveFile();
            break;

          case ITEM_GENERATE:
            GenerateCode();
            break;
#endif

          case ITEM_ABOUT:
            sprintf(string,
                    "%s %s\nUser interface code generator\n\n"
                    "Copyright © %s Commodore-Amiga, Inc.\n"
                    "Copyright © %s Michael J. Erwin\n"
                    "All Rights Reserved",
                    title, version,
                    cbmdate,
                    merdate);
            PutError(string, "OK");
            break;

          case ITEM_QUIT:
            if(RequestKill()) done = DONE_ENDPROGRAM;
            break;
          }
        break;

      case MENU_SCREEN:
        switch(itemnum)
          {
          case ITEM_SCREENMODE:
            EditScreen();
            break;

          case ITEM_SCREENTAGS:
            if(!screennode.Workbench) EditTags(TYPE_SCREEN, NULL, NULL);
            break;

          case ITEM_SCREENCOLORS:
            if(!screennode.Workbench) EditColors();
            break;

          case ITEM_SCREENFONT:
            EditScreenFont(&screennode);
            break;
          }
        break;

      case MENU_WINDOW:
        switch(itemnum)
          {
          case ITEM_WINDOWTAGS:
            EditTags(TYPE_WINDOW, currentwindow, NULL);
            break;

          case ITEM_WINDOWIDCMP:
            EditIDCMP();
            break;

          case ITEM_WINDOWMENUS:
            EditMenus();
            break;

          case ITEM_NEWWINDOW:
            AddNewWindow();
            break;

          case ITEM_SELECTALL:
            SelectAllGadgets(currentwindow);
            break;

          case ITEM_SORTCONTENTS:
            SortGadgets(currentwindow);
            break;

          case ITEM_KILLWINDOW:
            sprintf(string, "Warning: you cannot get back\nwhat you delete!  OK to delete:\n\nWindow \"%s\"?", currentwindow->Title);
            if(PutError(string, "OK|Cancel")) TagWindow(currentwindow, COMMAND_KILL);
            break;

          case ITEM_OPENATSTART:
            if(flags & CHECKED)
              currentwindow->TMWFlags |= WINDOWFLAG_OPENATSTART;
            else
              currentwindow->TMWFlags &= ~WINDOWFLAG_OPENATSTART;
            break;
          }
        break;

      case MENU_GADGET:
        switch(itemnum)
          {
          case ITEM_GADGETTAGS:
            EditTags(TYPE_GADGET, currentwindow, currentgadget);
            break;

          case ITEM_GADGETFONT:
            EditGadgetFont(currentgadget);
            break;

          case ITEM_COPYGADGET:
            CopyGadgets(currentwindow);
            break;

          case ITEM_KILLGADGET:
            gcount = 0;
            for(newgadgetnode = (struct NewGadgetNode *) currentwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
                newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
              {
              if(newgadgetnode->Selected) gcount++;
              }
            sprintf(string, "Warning: you cannot get back\nwhat you delete!  OK to delete:\n\n%d Gadget(s)?", gcount);
            if(PutError(string, "OK|Cancel")) KillSelectedGadgets(currentwindow);
            break;
          }
        break;

      case MENU_OPTIONS:
        switch(itemnum)
          {
          case ITEM_TOPBORDER:
            if(flags & CHECKED) autotopborderflag = TRUE; else autotopborderflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_COMMENTS:
            if(flags & CHECKED) commentflag = TRUE; else commentflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_REGPARMS:
            if(flags & CHECKED) pragmaflag = TRUE; else pragmaflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_USERSIGNAL:
            if(flags & CHECKED) usersignalflag = TRUE; else usersignalflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_AREXX:
            if(flags & CHECKED) arexxflag = TRUE; else arexxflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_CREATEICONS:
            if(flags & CHECKED) iconflag = TRUE; else iconflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_CHIPKEYWORD:
            if(flags & CHECKED) chipflag = TRUE; else chipflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_USERDATA:
            if(flags & CHECKED) userdataflag = TRUE; else userdataflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_ASL:
            if(flags & CHECKED) aslflag = TRUE; else aslflag = FALSE;
            modified = TRUE;
            break;

          case ITEM_GRID:
            switch(subnum)
              {
              case SUB_OFF:
                if(gridsize != 1) modified = TRUE;
                gridsize = 1;
                break;

              case SUB_2:
                if(gridsize != 2) modified = TRUE;
                gridsize = 2;
                break;

              case SUB_4:
                if(gridsize != 4) modified = TRUE;
                gridsize = 4;
                break;

              case SUB_8:
                if(gridsize != 8) modified = TRUE;
                gridsize = 8;
                break;
              }
            break;

#ifndef DEMO
          case ITEM_SAVESETTINGS:
            SaveSettingsFile();
            break;
#endif
          }
        break;
      }
    menuitem = ItemAddress (M_Main, menucode);
    menucode = menuitem->NextSelect;
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

