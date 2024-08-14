
/****************************************************************************/
/*                                                                          */
/*  Main.c - Entrance, exit, and miscellneous functions                     */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"
#include "Externs.h"

#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dosasl.h>
#include <workbench/startup.h>
#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>
#include <clib/asl_protos.h>

/****************************************************************************/

void main(int argc, char **argv)
  {
  int i;
  BPTR progdirlock;
  BOOL success;

  if(!(IntuitionBase =     OpenLibrary("intuition.library",37))) CleanExit("Unable to open intuition.library V37");
  if(!(GfxBase =           OpenLibrary("graphics.library",37)))  CleanExit("Unable to open\ngraphics.library V37");
  if(!(GadToolsBase =      OpenLibrary("gadtools.library",37)))  CleanExit("Unable to open\ngadtools.library V37");
  if(!(AslBase =           OpenLibrary("asl.library",37)))       CleanExit("Unable to open\nasl.library V37");
  if(!(IconBase =          OpenLibrary("icon.library",37)))      CleanExit("Unable to open\nicon.library V37");
  if(!(DiskfontBase =      OpenLibrary("diskfont.library",37)))  CleanExit("Unable to open\ndiskfont.library V37");
  if(!(UtilityBase =       OpenLibrary("utility.library",37)))   CleanExit("Unable to open\nutility.library V37");
  if(!(IFFParseBase =      OpenLibrary("iffparse.library",37)))  CleanExit("Unable to open\niffparse.library V37");
  if(!(M_Main =            CreateMenus(NM_Main, GTMN_FullMenu, TRUE, TAG_DONE))) CleanExit("Unable to create menus");

  NewList(&WindowList);
  NewList(&screennode.TagList);
  NewList(&TextAttrList);
  NewList(&FontList);
  NewList(&EmptyList);

  strcpy(screennode.FontName, "");
  screennode.TextAttr.ta_Name = screennode.FontName;
  screennode.TextAttr.ta_YSize = TOPAZ_EIGHTY;
  screennode.TextAttr.ta_Style = FS_NORMAL;
  screennode.TextAttr.ta_Flags = FPF_ROMFONT;
  screennode.TMSFlags = SCREENFLAG_OPENATSTART;

  /* Get Workbench screen colors */

  if(screennode.Screen = LockPubScreen(NULL))
    {
    mainleft = 0;
    maintop = screennode.Screen->BarHeight+1;

    for(i=0; i<32; i++)
      {
      defaultcolors[i] = GetRGB4(screennode.Screen->ViewPort.ColorMap ,i);
      screennode.Color[i] = defaultcolors[i];
      }

    GetViewSize();
    constantvalues[0] = screennode.Screen->BarHeight;

    UnlockPubScreen(NULL, screennode.Screen);
    screennode.Screen = NULL;
    }
  else
    CleanExit("Error locking\nWorkbench screen");

  /* Get path of tool */

  if(progdirlock = GetProgramDir())
    {
    success = NameFromLock(progdirlock, toolpath, PATHSIZE);
    }
  else
    {
    if(progdirlock = Lock("Toolmaker:", ACCESS_READ))
      {
      success = NameFromLock(progdirlock, toolpath, PATHSIZE);
      UnLock(progdirlock);
      }
    else
      success = FALSE;
    }

  if(!success)
    CleanExit("Error finding\nToolmaker:");

  /* Handle command line args or workbench args */

  if(!(HandleArguments(argc, argv)))
    CleanExit("Error in arguments");

  EventLoop();

  CleanExit(NULL);
  }

/****************************************************************************/

void CleanExit(char *text)
  {
  DEBUGENTER("CleanExit", NULL);

  if(text)               PutError(text, "OK");

  TagWindows(COMMAND_KILL);
  TagMainWindow(COMMAND_KILL);
  ProcessWindows();

  TagScreen(COMMAND_KILL);
  ProcessScreen();

  KillFontList(&FontList);

  if(M_Main)             FreeMenus(M_Main);
  if(IFFParseBase)       CloseLibrary(IFFParseBase);
  if(UtilityBase)        CloseLibrary(UtilityBase);
  if(DiskfontBase)       CloseLibrary(DiskfontBase);
  if(IconBase)           CloseLibrary(IconBase);
  if(AslBase)            CloseLibrary(AslBase);
  if(GadToolsBase)       CloseLibrary(GadToolsBase);
  if(GfxBase)            CloseLibrary(GfxBase);
  if(IntuitionBase)      CloseLibrary(IntuitionBase);

#ifdef TEST
  if(debugflag)          fclose(confp);
#endif

  if(text)
    {
    DEBUGEXIT(TRUE, 20);
    exit(20);
    }
  else
    {
    DEBUGEXIT(TRUE, NULL);
    exit(NULL);
    }
  }

/****************************************************************************/

int HandleArguments(int argc, char **argv)
  {
  int               i;
  int               length;
  int               projectflag = FALSE;
  int               returnval = TRUE;
  char              *cptr;
  struct WBStartup  *WBenchMsg;
  struct WBArg      *wbarg;
  struct DiskObject *diskobject;
  struct RDArgs     *rdargs;
  char              **toolarray;
  char              *toolstring;

  strcpy(settingsfilename, toolpath);
  sprintf(string, "%s.prefs", title);
  AddPart(settingsfilename, string, FILESIZE);

  if(argc) /* CLI */
    {
    if(!(rdargs = ReadArgs(template, argarray, NULL)))
      puts("Error reading args");
    else
      {
      if(argarray[0])
        {
        strcpy(filename, (char *) argarray[0]);
        cptr = PathPart(filename);
        *cptr = '\0';
        strcpy(savepathname, filename);
        strcpy(filename, (char *) argarray[0]);
        length = strlen(filename);
        if(length > 3 && strcmp(&filename[length-3], extension) == 0) filename[length-3] = '\0';
        strcpy(savefilename, FilePart(filename));
        projectflag = TRUE;
        }

      if(argarray[1])
        {
        strcpy(settingsfilename, (char *) argarray[1]);
        }

      FreeArgs(rdargs);
      }
    }
  else     /* Workbench */
    {
    WBenchMsg = (struct WBStartup *) argv;
    for(i=0, wbarg=WBenchMsg->sm_ArgList; i<WBenchMsg->sm_NumArgs; i++, wbarg++)
      {
      if(i == 0) /* Tool icon */
        {
        if(diskobject = (struct DiskObject *) GetDiskObject(wbarg->wa_Name))
          {
          toolarray = (char **) diskobject->do_ToolTypes;
          if(toolstring = (char *) FindToolType(toolarray, "SETTINGS"))
            {
            strcpy(settingsfilename, toolstring);
            }
#ifdef TEST
          if(toolstring = (char *) FindToolType(toolarray, "DEBUG"))
            {
            if(!(confp = fopen(toolstring, "w")))
              returnval =  FALSE;
            else
              {
              debugflag = TRUE;
              DebugText("Toolmaker Debug", NULL);
              DebugText("sizeof(struct ScreenNode)     = ", sizeof(struct ScreenNode));
              DebugText("sizeof(struct WindowNode)     = ", sizeof(struct WindowNode));
              DebugText("sizeof(struct NewGadgetNode)  = ", sizeof(struct NewGadgetNode));
              DebugText("sizeof(struct GadgetNode)     = ", sizeof(struct GadgetNode));
              DebugText("sizeof(struct MenuNode)       = ", sizeof(struct MenuNode));
              DebugText("sizeof(struct ItemNode)       = ", sizeof(struct ItemNode));
              DebugText("sizeof(struct SubNode)        = ", sizeof(struct SubNode));
              DebugText("sizeof(struct TagNode)        = ", sizeof(struct TagNode));
              DebugText("sizeof(struct EditTagNode)    = ", sizeof(struct EditTagNode));
              DebugText("sizeof(struct EditStringNode) = ", sizeof(struct EditStringNode));
              DebugText("sizeof(struct StringNode)     = ", sizeof(struct StringNode));
              DebugText("sizeof(struct AvailableTags)  = ", sizeof(struct AvailableTags));
              DebugText("sizeof(struct AvailableIDCMP) = ", sizeof(struct AvailableIDCMP));
              DebugText("sizeof(struct StringData)     = ", sizeof(struct StringData));
              DebugText("sizeof(struct TextAttrNode)   = ", sizeof(struct TextAttrNode));
              }
            }
#endif
          FreeDiskObject(diskobject);
          }
        }
      else
        {
        if(*wbarg->wa_Name) /* Project icon */
          {
          if(wbarg->wa_Lock) CurrentDir(wbarg->wa_Lock);
          strcpy(filename, wbarg->wa_Name);

          strcpy(savefilename, FilePart(wbarg->wa_Name));
          length = strlen(savefilename);
          if(length > 3 && strcmp(&savefilename[length-3], extension) == 0) savefilename[length-3] = '\0';

          cptr = PathPart(filename);
          *cptr = '\0';
          strcpy(savepathname, filename);

          strcat(filename, savefilename);
          projectflag = TRUE;
          }
        }
      }
    }

  TagMainWindow(STATUS_KILLED);
  TagScreen(STATUS_NORMAL);

  if(projectflag)
    {
    TagScreen(COMMAND_CHANGE);
    openflag = TRUE;
    ProcessScreen();
    }
  else
    {
    TagScreen(COMMAND_CHANGE);
    newprojectflag = TRUE;
    ProcessScreen();
    }

  SetUpMenus();       /* initial set up */
  return(returnval);
  }

/****************************************************************************/

void NewProject(void)
  {
  DEBUGENTER("NewProject", NULL);

  mainleft = 0;
  maintop = screennode.Screen->BarHeight+1;

  TagWindows(COMMAND_KILL);
  TagMainWindow(COMMAND_REMOVE);
  TagScreen(COMMAND_CHANGE);

  if(W_Main) ActivateWindow(W_Main);

  newprojectflag = TRUE;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void EventLoop(void)
  {
  int                 done=FALSE;
  ULONG               signals;
  struct WindowNode   *windownode;

  DEBUGENTER("EventLoop", NULL);

  while(!done)
    {
    DEBUGTEXT("Wait", NULL);

    signals = Wait(mainsignal | arraysignals);

    /********** process messages **********/

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(windownode->Window && signals & 1L<<windownode->Window->UserPort->mp_SigBit)
        {
        if(HandleArrayIDCMP(windownode) == DONE_ENDPROGRAM) done = TRUE;
        }
      }

    if(signals & mainsignal)
      {
      if(HandleMainIDCMP(W_Main) == DONE_ENDPROGRAM) done = TRUE;
      }

    /********** Stuff after all messages processed **********/

    ProcessWindows();
    ProcessScreen();
    SetUpMenus();
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void DrawDragBox(struct WindowNode *windownode, SHORT x1, SHORT y1, SHORT x2, SHORT y2)
  {
  register struct RastPort *rp;
  register SHORT  temp;

  DEBUGENTER("DrawDragBox", NULL);

  if(x1!=x2 || y1!=y2)
    {
    if(x2 < x1) { temp = x1; x1 = x2; x2 = temp; } /* Swap x1<->x2 */
    if(y2 < y1) { temp = y1; y1 = y2; y2 = temp; } /* Swap y1<->y2 */

    rp = windownode->Window->RPort;

    Move(rp, x1, y1);

    if(x1==x2 || y1==y2)
      {
      Draw(rp, x2, y2);
      }
    else
      {
      WaitBlit();
      Draw(rp, x2-1, y1);
      Draw(rp, x2-1, y2-1);
      Draw(rp, x1,   y2-1);
      Draw(rp, x1,   y1+1);
      }
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SpinDragBox(struct WindowNode *windownode)
  {
  DEBUGENTER("SpinDragBox", NULL);

  switch(windownode->Window->RPort->LinePtrn)
    {
    case 0xF0F0:
      SetDrPt(windownode->Window->RPort, 0x3C3C);
      break;

    case 0x3C3C:
      SetDrPt(windownode->Window->RPort, 0x0F0F);
      break;

    case 0x0F0F:
      SetDrPt(windownode->Window->RPort, 0xC3C3);
      break;

    case 0xC3C3:
      SetDrPt(windownode->Window->RPort, 0xF0F0);
      break;

    default:
      SetDrPt(windownode->Window->RPort, 0xF0F0);
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SelectSelect(void)
  {
  DEBUGENTER("SelectSelect", NULL);

  if(W_Main)
    {
    GT_SetGadgetAttrs(G_GadgetList, W_Main, NULL, GTMX_Active, 0, TAG_DONE);
/*
    drag = DRAGNONE;
*/
    gadgetkind = NULL;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void MarkGadget(struct WindowNode *windownode, struct NewGadgetNode *newgadgetnode)
  {
  struct RastPort *rp;
  SHORT           top;
  SHORT           left;
  SHORT           width;
  SHORT           height;

  DEBUGENTER("MarkGadget", NULL);

  if(windownode->Window)
    {
    rp = windownode->Window->RPort;

    left =   newgadgetnode->NewGadget.ng_LeftEdge;
    top =    newgadgetnode->NewGadget.ng_TopEdge;
    width =  newgadgetnode->NewGadget.ng_Width-1;
    height = newgadgetnode->NewGadget.ng_Height-1;

    SetAPen(rp, 3);
    SetDrMd(rp, COMPLEMENT);

    RectFill(rp, left-SIZEBLOCKWIDTH/2,       top-SIZEBLOCKHEIGHT/2,        left+SIZEBLOCKWIDTH/2,       top+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left-SIZEBLOCKWIDTH/2,       top+height-SIZEBLOCKHEIGHT/2, left+SIZEBLOCKWIDTH/2,       top+height+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left+width-SIZEBLOCKWIDTH/2, top-SIZEBLOCKHEIGHT/2,        left+width+SIZEBLOCKWIDTH/2, top+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left+width-SIZEBLOCKWIDTH/2, top+height-SIZEBLOCKHEIGHT/2, left+width+SIZEBLOCKWIDTH/2, top+height+SIZEBLOCKHEIGHT/2);

    RectFill(rp, left-SIZEBLOCKWIDTH/2,         top+height/2-SIZEBLOCKHEIGHT/2, left+SIZEBLOCKWIDTH/2,         top+height/2+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left+width-SIZEBLOCKWIDTH/2,   top+height/2-SIZEBLOCKHEIGHT/2, left+width+SIZEBLOCKWIDTH/2,   top+height/2+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left+width/2-SIZEBLOCKWIDTH/2, top-SIZEBLOCKHEIGHT/2,          left+width/2+SIZEBLOCKWIDTH/2, top+SIZEBLOCKHEIGHT/2);
    RectFill(rp, left+width/2-SIZEBLOCKWIDTH/2, top+height-SIZEBLOCKHEIGHT/2,   left+width/2+SIZEBLOCKWIDTH/2, top+height+SIZEBLOCKHEIGHT/2);

    SetDrMd(rp, JAM1);
    SetAPen(rp, 1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SelectGadget(struct WindowNode *windownode, struct NewGadgetNode *newgadgetnode)
  {
  DEBUGENTER("SelectGadget", NULL);

  if(!newgadgetnode->Selected)
    {
    MarkGadget(windownode, newgadgetnode);
    newgadgetnode->Selected = TRUE;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void UnSelectGadget(struct WindowNode *windownode, struct NewGadgetNode *newgadgetnode)
  {
  DEBUGENTER("UnSelectGadget", NULL);

  if(newgadgetnode->Selected)
    {
    MarkGadget(windownode, newgadgetnode);
    newgadgetnode->Selected = FALSE;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SelectAllGadgets(struct WindowNode *windownode)
  {
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("SelectAllGadgets", NULL);

  MainWindowSleep();

  if(windownode && !realgadgets)
    {
    for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
        newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
      {
      SelectGadget(windownode, newgadgetnode);
      }
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void UnSelectAllGadgets(struct WindowNode *wn)
  {
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("UnSelectAllGadgets", NULL);

  if(wn)
    {
    for(newgadgetnode = (struct NewGadgetNode *) wn->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
        newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
      {
      UnSelectGadget(wn, newgadgetnode);
      }
    }
  else
    {
    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        UnSelectGadget(windownode, newgadgetnode);
        }
      }
    }

  currentgadget = NULL;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void DragSelectGadgets(struct WindowNode *windownode, SHORT x1, SHORT y1, SHORT x2, SHORT y2)
  {
  SHORT                top;
  SHORT                left;
  SHORT                right;
  SHORT                bottom;
  SHORT                temp;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("DragSelectGadgets", NULL);

  MainWindowSleep();

  if(x2 < x1) { temp = x1; x1 = x2; x2 = temp; }
  if(y2 < y1) { temp = y1; y1 = y2; y2 = temp; }

  for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
      newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
    {
    left =   newgadgetnode->NewGadget.ng_LeftEdge;
    top =    newgadgetnode->NewGadget.ng_TopEdge;
    right =  left + newgadgetnode->NewGadget.ng_Width;
    bottom = top + newgadgetnode->NewGadget.ng_Height;

    if(((x1<left && x2>left) || (x1<right && x2>right)   || (x1>=left && x2<=right)) &&
       ((y1<top && y2>top)   || (y1<bottom && y2>bottom) || (y1>=top && y2<=bottom)))
      {
      SelectGadget(windownode, newgadgetnode);
      }
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RedoWindowSignals(void)
  {
  struct WindowNode *windownode;

  DEBUGENTER("RedoWindowSignals", NULL);

  arraysignals = 0;
  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    if(windownode->Window && windownode->Node.ln_Pri==STATUS_NORMAL)
      {
      arraysignals |= 1L<<windownode->Window->UserPort->mp_SigBit;
      }
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

int SetUpMenus(void)
  {
  int    gadgetcount, selectcount;
  struct MenuItem *menuitem;
  struct WindowNode *windownode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("SetUpMenus", NULL);

  if(M_Main)
    {
    if(W_Main) ClearMenuStrip(W_Main);

    if(!realmenus)
      {
      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        if(windownode->Window) ClearMenuStrip(windownode->Window);
        }
      }

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_COMMENTS, NOSUB));
    if(commentflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_TOPBORDER, NOSUB));
    if(autotopborderflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_REGPARMS, NOSUB));
    if(pragmaflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_USERSIGNAL, NOSUB));
    if(usersignalflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_AREXX, NOSUB));
    if(arexxflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_CREATEICONS, NOSUB));
    if(iconflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_CHIPKEYWORD, NOSUB));
    if(chipflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_USERDATA, NOSUB));
    if(userdataflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_ASL, NOSUB));
    if(aslflag) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_GRID, SUB_OFF));
    if(gridsize == 1) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_GRID, SUB_2));
    if(gridsize == 2) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_GRID, SUB_4));
    if(gridsize == 4) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_OPTIONS, ITEM_GRID, SUB_8));
    if(gridsize == 8) menuitem->Flags |= CHECKED; else menuitem->Flags &= ~CHECKED;

    menuitem = ItemAddress(M_Main, FULLMENUNUM(MENU_WINDOW, ITEM_OPENATSTART, NOSUB));
    if(currentwindow && (currentwindow->TMWFlags & WINDOWFLAG_OPENATSTART))
      menuitem->Flags |= CHECKED;
    else
      menuitem->Flags &= ~CHECKED;

    if(W_Main)
      {
      /* Set gadgets */

      if(realgadgets)
        GT_SetGadgetAttrs(G_TestGadgets, W_Main, NULL, GTCB_Checked, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_TestGadgets, W_Main, NULL, GTCB_Checked, FALSE, TAG_DONE);

      if(realmenus)
        GT_SetGadgetAttrs(G_TestMenus, W_Main, NULL, GTCB_Checked, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_TestMenus, W_Main, NULL, GTCB_Checked, FALSE, TAG_DONE);

      /* Enable/Disable menus */

      SetMenuStrip(W_Main, M_Main);

      if(screennode.Workbench)
        {
        OffMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENTAGS, NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENCOLORS, NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENFONT, NOSUB));
        }
      else
        {
        OnMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENTAGS, NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENFONT, NOSUB));
        if(screennode.Mode & MODE_CUSTOMCOLORS)
          OnMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENCOLORS, NOSUB));
        else
          OffMenu(W_Main, FULLMENUNUM(MENU_SCREEN, ITEM_SCREENCOLORS, NOSUB));
        }

      if(windowcount >= 10)
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_NEWWINDOW, NOSUB));
      else
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_NEWWINDOW, NOSUB));

      selectcount = 0;
      gadgetcount = 0;
      if(currentwindow)
        {
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWTAGS,  NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWIDCMP, NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWMENUS, NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_KILLWINDOW,  NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_OPENATSTART, NOSUB));

        for(newgadgetnode = (struct NewGadgetNode *) currentwindow->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          gadgetcount++;
          if(newgadgetnode->Selected)
            {
            selectcount++;
            currentgadget = newgadgetnode;
            }
          }
        }
      else
        {
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWTAGS,  NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWIDCMP, NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_WINDOWMENUS, NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_KILLWINDOW,  NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_OPENATSTART, NOSUB));
        }

      if(gadgetcount && !realgadgets)
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_SELECTALL, NOSUB));
      else
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_SELECTALL, NOSUB));

      if(gadgetcount)
        OnMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_SORTCONTENTS, NOSUB));
      else
        OffMenu(W_Main, FULLMENUNUM(MENU_WINDOW, ITEM_SORTCONTENTS, NOSUB));

      if(selectcount)
        OnMenu(W_Main, FULLMENUNUM(MENU_GADGET, NOITEM, NOSUB));
      else
        OffMenu(W_Main, FULLMENUNUM(MENU_GADGET, NOITEM, NOSUB));

      if(selectcount==1)
        {
        OnMenu(W_Main, FULLMENUNUM(MENU_GADGET, ITEM_GADGETTAGS, NOSUB));
        OnMenu(W_Main, FULLMENUNUM(MENU_GADGET, ITEM_GADGETFONT, NOSUB));
        onegadgetflag = TRUE;
        }
      else
        {
        OffMenu(W_Main, FULLMENUNUM(MENU_GADGET, ITEM_GADGETTAGS, NOSUB));
        OffMenu(W_Main, FULLMENUNUM(MENU_GADGET, ITEM_GADGETFONT, NOSUB));
        onegadgetflag = FALSE;
        }
      }

    /* Set menu strips on user windows */

    if(!realmenus)
      {
      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        if(windownode->Window) SetMenuStrip(windownode->Window, M_Main);
        }
      }

    DEBUGEXIT(TRUE, TRUE);
    return(TRUE);
    }
  else
    {
    DEBUGEXIT(TRUE, FALSE);
    return(FALSE);
    }
  }

/****************************************************************************/

int PutError(char *textformat, char *gadgetformat)
  {
  int status = NULL;

  DEBUGENTER("PutError", NULL);
  DEBUGTEXT(textformat, NULL);

  easystruct.es_TextFormat   = textformat;
  easystruct.es_GadgetFormat = gadgetformat;

  if(IntuitionBase)
    {
    if(W_Main)
      {
      MainWindowSleep();
      status = EasyRequestArgs(W_Main, &easystruct, NULL, NULL);
      MainWindowWakeUp();
      }
    else
      status = EasyRequestArgs(NULL, &easystruct, NULL, NULL);
    }

  DEBUGEXIT(TRUE, (ULONG) status);
  return(status);
  }

/****************************************************************************/

void MoveNodeUp(struct List *list, int nodenum, struct Window *window, struct Gadget *listgadget)
  {
  int         i;
  struct Node *thisnode;
  struct Node *prevnode;

  DEBUGENTER("MoveNodeUp", NULL);

  GT_SetGadgetAttrs(listgadget, window, NULL, GTLV_Labels, ~0, TAG_DONE);

  thisnode = list->lh_Head;
  for(i=0; i<nodenum; i++) thisnode = thisnode->ln_Succ;
  prevnode = thisnode->ln_Pred;
  prevnode = prevnode->ln_Pred;
  if(thisnode && prevnode)
    {
    Remove(thisnode);
    Insert(list, thisnode, prevnode);
    }

  GT_SetGadgetAttrs(listgadget, window, NULL, GTLV_Labels, list, GTLV_Selected, 0, TAG_DONE);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void MoveNodeDown(struct List *list, int nodenum, struct Window *window, struct Gadget *listgadget)
  {
  int         i;
  struct Node *thisnode;
  struct Node *succnode;

  DEBUGENTER("MoveNodeDown", NULL);

  GT_SetGadgetAttrs(listgadget, window, NULL, GTLV_Labels, ~0, TAG_DONE);

  thisnode = list->lh_Head;
  for(i=0; i<nodenum; i++) thisnode = thisnode->ln_Succ;
  succnode = thisnode->ln_Succ;
  if(thisnode && succnode)
    {
    Remove(thisnode);
    Insert(list, thisnode, succnode);
    }

  GT_SetGadgetAttrs(listgadget, window, NULL, GTLV_Labels, list, GTLV_Selected, 0, TAG_DONE);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

int RequestKill(void)
  {
  int status=1;

#ifndef DEMO
  DEBUGENTER("RequestKill", NULL);

  if(modified)
    {
    if(filename[0] == '\0')
      sprintf(string, "Save project before closing?");
    else
      sprintf(string, "Save project\n%s.tm\nbefore closing?", filename);

    status = PutError(string, "Yes|No|Cancel");
    if(status == 1)
      {
      if(!SaveFile()) status = 0;
      }
    }

  DEBUGEXIT(TRUE, (ULONG) status);
#endif
  return(status);
  }

/****************************************************************************/

void MainWindowSleep(void)
  {
  struct WindowNode *windownode;

  DEBUGENTER("MainWindowSleep", NULL);

  if(!sleepcount)
    {
    if(W_Main)
      {
      SetPointer(W_Main, SleepPointer, 16, 16, -6, 0);
      InitRequester(&R_Main);
      if(Request(&R_Main, W_Main))
        {
        F_Main |= WINDOWFLAG_DISABLED;
        }
      }

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(windownode->Window)
        {
        SetPointer(windownode->Window, SleepPointer, 16, 16, -6, 0);
        InitRequester(&windownode->Requester);
        if(Request(&windownode->Requester, windownode->Window))
          {
          windownode->TMWFlags |= WINDOWFLAG_DISABLED;
          }
        }
      }
    }

  sleepcount++;

  DEBUGTEXT("Sleep:sleepcount = ", sleepcount);
  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void MainWindowWakeUp(void)
  {
  struct WindowNode *windownode;

  DEBUGENTER("MainWindowWakeUp", NULL);

  sleepcount--;

  if(!sleepcount)
    {
    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(windownode->Window)
        {
        if(windownode->TMWFlags & WINDOWFLAG_DISABLED)
          {
          EndRequest(&windownode->Requester, windownode->Window);
          windownode->TMWFlags &= ~WINDOWFLAG_DISABLED;
          }

        ClearPointer(windownode->Window);
        }
      }

    if(W_Main)
      {
      if(F_Main & WINDOWFLAG_DISABLED)
        {
        EndRequest(&R_Main, W_Main);
        F_Main &= ~WINDOWFLAG_DISABLED;
        }

      ClearPointer(W_Main);
      }
    }

  DEBUGTEXT("WakeUp:sleepcount = ", sleepcount);
  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void GetViewSize(void)
  {
  WORD   oscantype;
  ULONG  modeid;
  struct Rectangle rect;

  DEBUGENTER("GetViewSize", NULL);

  if(screennode.Mode & MODE_OSCANSTANDARD)
    oscantype = OSCAN_STANDARD;
  else if(screennode.Mode & MODE_OSCANMAX)
    oscantype = OSCAN_MAX;
  else if(screennode.Mode & MODE_OSCANVIDEO)
    oscantype = OSCAN_VIDEO;
  else
    oscantype = OSCAN_TEXT;

  if(screennode.Screen && (modeid = GetVPModeID(&screennode.Screen->ViewPort)) != INVALID_ID)
    {
    QueryOverscan(modeid, &rect, oscantype);
    viewwidth  = rect.MaxX - rect.MinX + 1;
    viewheight = rect.MaxY - rect.MinY + 1;
    }
  else
    {
    viewwidth  = 640;
    viewheight = 200;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SortGadgets(struct WindowNode *windownode)
  {
  struct NewGadgetNode *newgadgetnode1, *newgadgetnode2, *temp;

  DEBUGENTER("SortGadgets", NULL);

  MainWindowSleep();
  RemoveGadgets(windownode);

  for(newgadgetnode1 = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode1->Node.ln_Succ;
      newgadgetnode1 = (struct NewGadgetNode *) newgadgetnode1->Node.ln_Succ)
    {
    for(newgadgetnode2 = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode2->Node.ln_Succ;
        newgadgetnode2 = (struct NewGadgetNode *) newgadgetnode2->Node.ln_Succ)
      {
      if(newgadgetnode1->NewGadget.ng_LeftEdge == newgadgetnode2->NewGadget.ng_LeftEdge)
        {
        if(newgadgetnode1->NewGadget.ng_TopEdge < newgadgetnode2->NewGadget.ng_TopEdge)
          {
          SwapNodes(&windownode->NewGadgetList, (struct Node *) newgadgetnode1, (struct Node *) newgadgetnode2);
          temp = newgadgetnode1;
          newgadgetnode1 = newgadgetnode2;
          newgadgetnode2 = temp;
          }
        }
      else if(newgadgetnode1->NewGadget.ng_LeftEdge < newgadgetnode2->NewGadget.ng_LeftEdge)
        {
        SwapNodes(&windownode->NewGadgetList, (struct Node *) newgadgetnode1, (struct Node *) newgadgetnode2);
        temp = newgadgetnode1;
        newgadgetnode1 = newgadgetnode2;
        newgadgetnode2 = temp;
        }
      }
    }

  modified = TRUE;

  AddGadgets(windownode);
  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void SwapNodes(struct List *list, struct Node *node1, struct Node *node2)
  {
  struct Node tempnode1, tempnode2;

  DEBUGENTER("SwapNodes", NULL);

  Insert(list, &tempnode1, node1);
  Insert(list, &tempnode2, node2);

  Remove(node1);
  Remove(node2);

  Insert(list, node2, &tempnode1);
  Insert(list, node1, &tempnode2);

  Remove(&tempnode1);
  Remove(&tempnode2);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void TestGadgets(void)
  {
  struct WindowNode *windownode;

  DEBUGENTER("TextGadgets", NULL);

  MainWindowSleep();

  UnSelectAllGadgets(NULL);

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    RemoveGadgets(windownode);
    }

  if(!realgadgets) realgadgets = TRUE; else realgadgets = FALSE;

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    AddGadgets(windownode);
    }

  SelectSelect();
  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void TestMenus(void)
  {
  struct WindowNode *windownode;

  DEBUGENTER("TestMenus", NULL);

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    if(windownode->Window) ClearMenuStrip(windownode->Window);
    }

  if(!realmenus) realmenus = TRUE; else realmenus = FALSE;

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    if(windownode->Window)
      {
      if(realmenus)
        SetMenuStrip(windownode->Window, windownode->Menu);
      else
        SetMenuStrip(windownode->Window, M_Main);
      }
    }

  SelectSelect();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

int FileRequest(char *title, char *path, char *file, char saveflag)
  {
  int  length;
  int  fileflag;

  DEBUGENTER("FileRequest", NULL);

  if(!(request = AllocAslRequestTags(ASL_FileRequest, TAG_DONE)))
    {
    PutError("Out of memory creating file requester", "OK");
    return(FALSE);
    }
  else
    {
    MainWindowSleep();
    sprintf(string, "#?%s", extension);

    fileflag = (int) AslRequestTags(request, ASL_Hail, title,
                                             ASL_Window, W_Main,
                                             ASL_File, file,
                                             ASL_Dir, path,
                                             ASL_Pattern, string,
                                             ASL_FuncFlags, saveflag ? FILF_SAVE : 0,
                                             TAG_DONE);

    if(fileflag)
      {
      if(((struct FileRequester *) request)->rf_File[0] != '\0')
        {
        strcpy(file, ((struct FileRequester *) request)->rf_File);
        strcpy(path, ((struct FileRequester *) request)->rf_Dir );

        length = strlen(file);
        if(length > 3 && strcmp(&file[length-3], extension) == 0) file[length-3] = '\0';

        strcpy(filename, path);
        AddPart(filename, file, PATHSIZE+FILESIZE);
        }
      else
        {
        PutError("Illegal file name", "OK");
        fileflag = FALSE;
        }
      }

    if(request) FreeAslRequest(request);
    MainWindowWakeUp();
    }

  DEBUGEXIT(TRUE, (ULONG) fileflag);
  return(fileflag);
  }

/****************************************************************************/

#ifdef TEST

void DebugEnter(char *name, ULONG arg)
  {
  char spacestring[21] = "                    ";

  if(debugflag && confp)
    {
    if(debuglevel<20) spacestring[debuglevel] = '\0';

    if(arg)
      fprintf(confp, "%s%s(%d);\n\r", spacestring, name, arg);
    else
      fprintf(confp, "%s%s();\n\r", spacestring, name);

    debuglevel++;
    }
  }

void DebugExit(ULONG rflag, ULONG returnval)
  {
  char spacestring[21] = "                    ";

  if(debugflag && confp)
    {
    if(debuglevel<20) spacestring[debuglevel] = '\0';

    if(rflag)
      fprintf(confp, "%sreturn(%d)\n\r", spacestring, returnval);
    else
      fprintf(confp, "%sreturn\n\r", spacestring);

    debuglevel--;
    }
  }

void DebugAlloc(ULONG memsize)
  {
  char spacestring[21] = "                    ";

  if(debugflag && confp)
    {
    if(debuglevel<20) spacestring[debuglevel] = '\0';
    totalmem += memsize;
    fprintf(confp, "%sAllocMem(%d)  (total:%d)\n\r", spacestring, memsize, totalmem);
    }
  }

void DebugFree(ULONG memsize)
  {
  char spacestring[21] = "                    ";

  if(debugflag && confp)
    {
    if(debuglevel<20) spacestring[debuglevel] = '\0';
    totalmem -= memsize;
    fprintf(confp, "%sFreeMem(%d)  (total:%d)\n\r", spacestring, memsize, totalmem);
    }
  }

void DebugText(char *text, ULONG val)
  {
  char spacestring[21] = "                    ";

  if(debugflag && confp)
    {
    if(debuglevel<20) spacestring[debuglevel] = '\0';
    if(val)
      fprintf(confp, "%s*** %s%d (0x%x) ***\n\r", spacestring, text, val, val);
    else
      fprintf(confp, "%s*** %s ***\n\r", spacestring, text);
    }
  }

#endif

