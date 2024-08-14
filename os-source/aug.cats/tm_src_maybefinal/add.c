
#include "Toolmaker.h"
#include "Externs.h"

#include <exec/memory.h>
#include <dos/dosextens.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

/****************************************************************************/

ULONG AddScreen(void)
  {
  int             i;
  int             status;
  ULONG           errorcode = NULL;
  ULONG           error = FALSE;
  SHORT           tagcount;
  struct TagNode  *tagnode;

  DEBUGENTER("AddScreen", NULL);

  DEBUGTEXT("screennode.Node.ln_Pri = ", screennode.Node.ln_Pri);
  DEBUGTEXT("screennode.Mode = ", screennode.Mode);

  if(screennode.Node.ln_Pri == STATUS_REMOVED ||
     screennode.Node.ln_Pri == STATUS_KILLED)
    {
    if(screennode.Mode & MODE_WORKBENCH)
      {
      DEBUGTEXT("Workbench screen", NULL);

      screennode.Screen = LockPubScreen(NULL);
      screennode.Workbench = TRUE;
      }
    else
      {
      DEBUGTEXT("Custom screen", NULL);

      tagcount = 0;
      for(tagnode = (struct TagNode *) screennode.TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        tagarray[tagcount].ti_Tag  = tagnode->TagItem.ti_Tag;
        tagarray[tagcount].ti_Data = tagnode->TagItem.ti_Data;
        tagcount++;
        }
      tagarray[tagcount].ti_Tag = SA_ErrorCode;
      tagarray[tagcount].ti_Data = (ULONG) &errorcode;
      tagcount++;

      tagarray[tagcount].ti_Tag = TAG_DONE;

      screennode.Screen = OpenScreenTagList(NULL, tagarray);
      screennode.Workbench = FALSE;
      }

    if(screennode.Screen)
      {
      status = TRUE;
      while(!(VisualInfo = GetVisualInfo(screennode.Screen, TAG_DONE)) && status)
        status = PutError("Error getting\nvisual info", "Retry|Abort");
      if(!status)
        {
        VisualInfo = NULL;
        error = TRUE;
        }

      if(!screennode.Workbench)
        {
        for(i=0; i<1<<screennode.Depth; i++)
          {
          if(screennode.Mode & MODE_CUSTOMCOLORS)
            SetRGB4(&screennode.Screen->ViewPort, i, COLOR2RED(screennode.Color[i]), COLOR2GREEN(screennode.Color[i]), COLOR2BLUE(screennode.Color[i]));
          else
            SetRGB4(&screennode.Screen->ViewPort, i, COLOR2RED(defaultcolors[i]), COLOR2GREEN(defaultcolors[i]), COLOR2BLUE(defaultcolors[i]));
          }
        }
      }
    else
      {
      switch(errorcode)
        {
        case OSERR_NOMONITOR:
          PutError("Unable to open screen;\nMonitor not available", "OK");
          break;
        case OSERR_NOCHIPS:
          PutError("Unable to open screen;\nNewer custom chips needed", "OK");
          break;
        case OSERR_NOMEM:
          PutError("Unable to open screen;\nOut of memory", "OK");
          break;
        case OSERR_NOCHIPMEM:
          PutError("Unable to open screen;\nOut of CHIP memory", "OK");
          break;
        case OSERR_PUBNOTUNIQUE:
          PutError("Unable to open screen;\nPublic name already used", "OK");
          break;
        case OSERR_UNKNOWNMODE:
          PutError("Unable to open screen;\nUnknown screen mode", "OK");
          break;
        }
      error = TRUE;
      }

    if(error)
      {
      screennode.Workbench = TRUE;
      screennode.Mode |= MODE_WORKBENCH;

      KillTagList(&screennode.TagList);
      screennode.TagCount = 0;
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_DisplayID, screennode.DisplayID);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Title, (ULONG) screennode.Title);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Depth, screennode.Depth);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Width, screennode.Width);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Height, screennode.Height);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Overscan, screennode.Overscan);
      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Font, (ULONG) &screennode.TextAttr);

      AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Pens, (ULONG) defaultpens);

      while(!(screennode.Screen = LockPubScreen(NULL)))
        {
        OpenWorkBench();
        PutError("Error opening\ndefault public screen", "Retry");
        }

      status = TRUE;
      while(!(VisualInfo = GetVisualInfo(screennode.Screen, TAG_DONE)) && status)
        status = PutError("Error getting\nvisual info", "Retry|Abort");

      if(!status) VisualInfo = NULL;
      }

    screennode.TMSFlags = SCREENFLAG_OPENATSTART;

    GetViewSize();
    LayoutMenus(M_Main, VisualInfo, TAG_DONE);

    currenttopborder = screennode.Screen->BarHeight+1;
    constantvalues[0] = screennode.Screen->BarHeight+1;

    screennode.Node.ln_Pri = STATUS_NORMAL;

    modified = TRUE;
    }

  DEBUGEXIT(TRUE, (ULONG) errorcode);
  return(errorcode);
  }

/****************************************************************************/

int AddMainWindow(void)
  {
  struct NewGadget NG_Temp;
  struct Process *process;

  DEBUGENTER("AddMainWindow", NULL);

  if((mainwindowpri == STATUS_REMOVED || mainwindowpri == STATUS_KILLED) &&
      screennode.Screen)
    {
    G_MainContext = CreateContext(&G_Main);

    NG_Temp.ng_VisualInfo = VisualInfo;
    NG_Temp.ng_UserData = NULL;
    NG_Temp.ng_Width = NULL;
    NG_Temp.ng_Height = NULL;
    NG_Temp.ng_TextAttr = &TOPAZ80;
    NG_Temp.ng_LeftEdge = 15;

    NG_Temp.ng_TopEdge = currenttopborder+8;
    NG_Temp.ng_GadgetText = NULL;
    NG_Temp.ng_GadgetID = ID_NEWGADGET;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_GadgetList = CreateGadget(MX_KIND, G_MainContext, &NG_Temp,
                                GTMX_Labels, gadgettext,
                                GTMX_Spacing, 1,
                                GT_Underscore, '_',
                                TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+130;
    NG_Temp.ng_GadgetText = "_Gadgets";
    NG_Temp.ng_GadgetID = ID_TESTGADGETS;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_TestGadgets = CreateGadget(CHECKBOX_KIND, G_GadgetList, &NG_Temp,
                                 GT_Underscore, '_',
                                 TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+142;
    NG_Temp.ng_GadgetText = "_Menus";
    NG_Temp.ng_GadgetID = ID_TESTMENUS;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_TestMenus = CreateGadget(CHECKBOX_KIND, G_TestGadgets, &NG_Temp,
                               GT_Underscore, '_',
                               TAG_DONE);

    if(G_TestMenus)
      {
      if(W_Main = OpenWindowTags(NULL,
                                 WA_PubScreen, screennode.Screen,
                                 WA_Left, mainleft, /* -screennode.Screen->ViewPort.DxOffset, */
                                 WA_Top, maintop, /* screennode.Screen->BarHeight + 1 - screennode.Screen->ViewPort.DyOffset, */
                                 WA_IDCMP, mainidcmp,
                                 WA_Gadgets, G_Main,
                                 WA_Title, "TM",
                                 WA_Activate, TRUE,
                                 WA_DragBar, TRUE,
                                 WA_DepthGadget, TRUE,
                                 WA_CloseGadget, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_ScreenTitle, title,
                                 WA_InnerWidth, 115,
                                 WA_InnerHeight, 130+27,
                                 TAG_DONE))
        {
        GT_RefreshWindow(W_Main, NULL);

        process = (struct Process *) FindTask(NULL);
        oldwindowptr = process->pr_WindowPtr;
        process->pr_WindowPtr = W_Main;

        SetMenuStrip(W_Main, M_Main);
        mainsignal = 1L<<W_Main->UserPort->mp_SigBit;

        TagMainWindow(STATUS_NORMAL);

        DEBUGEXIT(TRUE, TRUE);
        return(TRUE);
        }

      FreeGadgets(G_Main);
      G_Main = NULL;
      }
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

int AddNewWindow(void)
  {
  int    status;
  struct WindowNode *windownode;

  DEBUGENTER("AddNewWindow", NULL);

  if(windowcount < 10)
    {
    status = TRUE;
    while(!(windownode = (struct WindowNode *) AllocMem(sizeof(struct WindowNode), MEMF_CLEAR)) && status)
      status = PutError("Out of memory", "Retry|Abort");

    if(status)
      {
      DEBUGALLOC(sizeof (struct WindowNode));

      NewList(&windownode->TagList);
      NewList(&windownode->NewGadgetList);
      NewList(&windownode->GadgetList);
      NewList(&windownode->MenuList);

      strcpy(windownode->Title, "New window");
      windownode->IDCMP = IDCMP_CLOSEWINDOW;
      windownode->TMWFlags = WINDOWFLAG_OPENATSTART;
      windownode->Menu = NULL;

      AddTail(&WindowList, (struct Node *) windownode);
      windowcount++;

      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_Title,         (ULONG) windownode->Title);
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_IDCMP,         arrayidcmp);

      sprintf(string, "%d", 150+windowcount*10-screennode.Screen->ViewPort.DxOffset);
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_Left,          (ULONG) string);
      sprintf(string, "%d", 40+windowcount*5-screennode.Screen->ViewPort.DyOffset);
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_Top,           (ULONG) string);
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_InnerWidth,    (ULONG) "300");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_InnerHeight,   (ULONG) "100");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_DragBar,       (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_DepthGadget,   (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_CloseGadget,   (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_SimpleRefresh, (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_Activate,      (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_SizeGadget,    (ULONG) "TRUE");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_MaxWidth,      (ULONG) "-1");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_MaxHeight,     (ULONG) "-1");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_MinWidth,      (ULONG) "70");
      AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_MinHeight,     (ULONG) "30");

      TagWindow(windownode, STATUS_REMOVED);

      if(AddWindow(windownode))
        {
        modified = TRUE;

        DEBUGEXIT(TRUE, TRUE);
        return(TRUE);
        }

      KillTagList(&windownode->TagList);

      DEBUGFREE(sizeof (struct WindowNode));
      FreeMem(windownode, sizeof(struct WindowNode));
      }
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

int AddWindow(struct WindowNode *windownode)
  {
  SHORT                tagcount;
  struct TagNode       *tagnode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("AddWindow", NULL);

  if(windownode)
    {
    if(windownode->Node.ln_Pri == STATUS_NORMAL)
      {
      DEBUGEXIT(TRUE, TRUE);
      return(TRUE);
      }

    if(screennode.Screen)
      {
      tagcount = 0;
      for(tagnode = (struct TagNode *) windownode->TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(tagnode->Node.ln_Type)
          {
          case TYPE_VOID:
          case TYPE_INTEGER:
          case TYPE_STRING:
          case TYPE_CHARACTER:
          case TYPE_WORDLIST:
          case TYPE_STRINGLIST:
          case TYPE_LINKEDLIST:
            tagarray[tagcount].ti_Tag  = tagnode->TagItem.ti_Tag;
            tagarray[tagcount].ti_Data = tagnode->TagItem.ti_Data;
            tagcount++;
            break;
          }
        }
      tagarray[tagcount].ti_Tag  = WA_SmartRefresh;
      tagarray[tagcount].ti_Data = TRUE;
      tagcount++;
      tagarray[tagcount].ti_Tag  = WA_PubScreen;
      tagarray[tagcount].ti_Data = (ULONG) screennode.Screen;
      tagcount++;
      tagarray[tagcount].ti_Tag = TAG_DONE;

      if(windownode->Window = OpenWindowTagList(NULL, tagarray))
        {
        SetPointer(windownode->Window, SleepPointer, 16, 16, -6, 0);

        if(autotopborderflag && openflag)
          {
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            newgadgetnode->NewGadget.ng_TopEdge += windownode->Window->BorderTop;
            newgadgetnode->NewGadget.ng_LeftEdge += windownode->Window->BorderLeft;
            }
          }

        if(AddGadgets(windownode))
          {
          if(AddMenus(windownode))
            {
            currentwindow = windownode;

            TagWindow(windownode, STATUS_NORMAL);
            RedoWindowSignals();

            ClearPointer(windownode->Window);

            DEBUGEXIT(TRUE, TRUE);
            return(TRUE);
            }
          }

        RemoveWindow(windownode);
        }
      }
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

int AddAllWindows(void)
  {
  int    status;
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("AddAllWindows", NULL);

  status = TRUE;
  while(!(AddMainWindow()) && status) status = PutError("Error creating\nmain window", "Retry|Abort");

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
        newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
      {
      newgadgetnode->NewGadget.ng_VisualInfo = VisualInfo;
      }
    }

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    status = TRUE;
    while(!AddWindow(windownode) && status) status = PutError("Error creating\nuser window", "Retry|Abort");
    }

  if(W_Main)
    {
    WindowToFront(W_Main);
    ActivateWindow(W_Main);
    }

  DEBUGEXIT(TRUE, (ULONG) TRUE);
  return(TRUE);
  }

/****************************************************************************/

int AddMenus(struct WindowNode *windownode)
  {
  int    status;
  SHORT  count;
  SHORT  itemcount;
  SHORT  subcount;
  struct NewMenu  *newmenu;
  struct NewMenu  *thismenu;
  struct MenuNode *menunode;
  struct ItemNode *itemnode;
  struct SubNode  *subnode;

  DEBUGENTER("AddMenus", NULL);

  if(windownode)
    {
    count = 1;
    for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
        menunode = (struct MenuNode *) menunode->Node.ln_Succ)
      {
      count++;
      for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
          itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
        {
        count++;
        for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
            subnode = (struct SubNode *) subnode->Node.ln_Succ)
          {
          count++;
          }
        }
      }

    if(count == 1)
      {
      DEBUGTEXT("No menus", NULL);

      if(!realmenus && windownode->Window)
        {
        SetMenuStrip(windownode->Window, M_Main);
        }

      windownode->Menu = NULL;

      DEBUGEXIT(TRUE, TRUE);
      return(TRUE);
      }

    status = TRUE;
    while(!(newmenu = (struct NewMenu *) AllocMem(sizeof(struct NewMenu) * count, MEMF_CLEAR)) && status)
      status = PutError("Out of memory", "Retry|Abort");

    if(status)
      {
      DEBUGALLOC(sizeof (struct NewMenu) * count);

      thismenu = newmenu;

      for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
          menunode = (struct MenuNode *) menunode->Node.ln_Succ)
        {
        thismenu->nm_Type = NM_TITLE;
        thismenu->nm_Label = menunode->MenuText;
        thismenu->nm_CommKey = NULL;
        thismenu->nm_Flags = menunode->Flags;
        thismenu->nm_MutualExclude = NULL;
        thismenu->nm_UserData = NULL;
        thismenu++;
        itemcount = 0;

        for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
            itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
          {
          thismenu->nm_Type = NM_ITEM;

          if(itemnode->ItemText[0] == '-')
            thismenu->nm_Label = NM_BARLABEL;
          else
            thismenu->nm_Label = itemnode->ItemText;

          thismenu->nm_Flags = itemnode->Flags;

          if(itemnode->CommKey[0] == '\0')
            thismenu->nm_CommKey = NULL;
          else
            thismenu->nm_CommKey = itemnode->CommKey;

          if(itemnode->TMMFlags & MENUFLAG_AUTOEXCLUDE)
            thismenu->nm_MutualExclude = ~(1 << itemcount);
          else
            thismenu->nm_MutualExclude = NULL;

          thismenu->nm_UserData = NULL;
          thismenu++;
          itemcount++;

          subcount = 0;
          for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
              subnode = (struct SubNode *) subnode->Node.ln_Succ)
            {
            thismenu->nm_Type = NM_SUB;

            if(subnode->SubText[0] == '-')
              thismenu->nm_Label = NM_BARLABEL;
            else
              thismenu->nm_Label = subnode->SubText;

            thismenu->nm_Flags = subnode->Flags;

            if(subnode->CommKey[0] == '\0')
              thismenu->nm_CommKey = NULL;
            else
              thismenu->nm_CommKey = subnode->CommKey;

            if(subnode->TMMFlags & MENUFLAG_AUTOEXCLUDE)
              thismenu->nm_MutualExclude = ~(1 << subcount);
            else
              thismenu->nm_MutualExclude = NULL;

            thismenu->nm_UserData = NULL;
            thismenu++;
            subcount++;
            }
          }
        }

      thismenu->nm_Type = NM_END;

      if(windownode->Menu = CreateMenus(newmenu, GTMN_FullMenu, TRUE, TAG_DONE))
        {
        if(LayoutMenus(windownode->Menu, VisualInfo, TAG_DONE))
          {
          ClearMenuStrip(windownode->Window);

          if(realmenus)
            SetMenuStrip(windownode->Window, windownode->Menu);
          else
            SetMenuStrip(windownode->Window, M_Main);

          DEBUGEXIT(TRUE, TRUE);
          return(TRUE);
          }

        FreeMenus(windownode->Menu);
        }

      DEBUGFREE(sizeof (struct NewMenu) * count);
      FreeMem(newmenu, sizeof(struct NewMenu) * count);
      }
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

struct TagNode *AddTag(int type, struct Node *node, ULONG tag, ULONG data)
  {
  int    i;
  int    count;
  int    status;
  WORD   *wordarray;
  WORD   *wordpointer;
  WORD   *wordpointer2;
  char   **stringpointer;
  char   **stringpointer2;
  struct List          *stringlist;
  struct TagNode       *tagnode;
  struct AvailableTags *thisavailabletag;
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;
  struct StringData    *stringdata;
  struct StringNode    *stringnode;

  DEBUGENTER("AddTag", NULL);

  windownode    = (struct WindowNode *) node;
  newgadgetnode = (struct NewGadgetNode *) node;

  status = TRUE;
  while(!(tagnode = (struct TagNode *) AllocMem(sizeof(struct TagNode), MEMF_CLEAR)) && status)
    status = PutError("Out of memory", "Retry|Abort");

  if(status)
    {
    DEBUGALLOC(sizeof(struct TagNode));

    NewList(&tagnode->SourceLabelList);

    switch(type)
      {
      case TYPE_SCREEN:
        DEBUGTEXT("TYPE_SCREEN", NULL);
        thisavailabletag = &AvailableScreenTags[0];
        break;
      case TYPE_WINDOW:
        DEBUGTEXT("TYPE_WINDOW", NULL);
        thisavailabletag = &AvailableWindowTags[0];
        break;
      case TYPE_GADGET:
        DEBUGTEXT("TYPE_GADGET", NULL);
        switch(((struct NewGadgetNode *) node)->Kind)
          {
          case BUTTON_KIND:
            DEBUGTEXT("BUTTON_KIND", NULL);
            thisavailabletag = &AvailableBUGadgetTags[0];
            break;
          case CHECKBOX_KIND:
            DEBUGTEXT("CHECKBOX_KIND", NULL);
            thisavailabletag = &AvailableCBGadgetTags[0];
            break;
          case CYCLE_KIND:
            DEBUGTEXT("CYCLE_KIND", NULL);
            thisavailabletag = &AvailableCYGadgetTags[0];
            break;
          case INTEGER_KIND:
            DEBUGTEXT("INTEGER_KIND", NULL);
            thisavailabletag = &AvailableINGadgetTags[0];
            break;
          case LISTVIEW_KIND:
            DEBUGTEXT("LISTVIEW_KIND", NULL);
            thisavailabletag = &AvailableLVGadgetTags[0];
            break;
          case MX_KIND:
            DEBUGTEXT("MX_KIND", NULL);
            thisavailabletag = &AvailableMXGadgetTags[0];
            break;
          case NUMBER_KIND:
            DEBUGTEXT("NUMBER_KIND", NULL);
            thisavailabletag = &AvailableNMGadgetTags[0];
            break;
          case PALETTE_KIND:
            DEBUGTEXT("PALETTE_KIND", NULL);
            thisavailabletag = &AvailablePAGadgetTags[0];
            break;
          case SCROLLER_KIND:
            DEBUGTEXT("SCROLLER_KIND", NULL);
            thisavailabletag = &AvailableSCGadgetTags[0];
            break;
          case SLIDER_KIND:
            DEBUGTEXT("SLIDER_KIND", NULL);
            thisavailabletag = &AvailableSLGadgetTags[0];
            break;
          case STRING_KIND:
            DEBUGTEXT("STRING_KIND", NULL);
            thisavailabletag = &AvailableSTGadgetTags[0];
            break;
          case TEXT_KIND:
            DEBUGTEXT("TEXT_KIND", NULL);
            thisavailabletag = &AvailableTXGadgetTags[0];
            break;
          }
        break;
      }

    count = 0;
    while(thisavailabletag->Tag != tag && thisavailabletag->Tag != TAG_DONE)
      {
      thisavailabletag++;
      count++;
      }

    tagnode->Node.ln_Type   = thisavailabletag->Type;
    tagnode->Node.ln_Name   = thisavailabletag->Name;
    tagnode->Node.ln_Pri    = count;

    tagnode->TagItem.ti_Tag = tag;

    switch(tagnode->Node.ln_Type)
      {
      case TYPE_CHARACTER:
      case TYPE_VOID:
        DEBUGTEXT("TYPE_CHARACTER/VOID", NULL);

        tagnode->DataCount = 0;
        tagnode->TagItem.ti_Data = data;
        break;

      case TYPE_INTEGER:
        DEBUGTEXT("TYPE_INTEGER", NULL);

        tagnode->DataCount = 1;

        status = TRUE;
        while(!(tagnode->Data = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(STRINGSIZE);

          strcpy((char *) tagnode->Data, (char *) data);

          if((isdigit(((char *) tagnode->Data)[0])) || ((((char *) tagnode->Data)[0]) == '-'))
            {
            tagnode->TagItem.ti_Data = atoi((char *) tagnode->Data);
            }
          else
            {
            for(i=0; i<NUMCONSTANTS && strcmp((char *) data, constantlabels[i]) != 0; i++);

            if(strcmp((char *) data, constantlabels[i]) == 0)
              {
              tagnode->TagItem.ti_Data = constantvalues[i];
              }
            else
              {
              tagnode->TagItem.ti_Data = 0;
              PutError("Invalid constant value\n(defaulting to 0)", "OK");
              }
            }
          }
        break;

      case TYPE_STRING:
      case TYPE_IGNORE:
        DEBUGTEXT("TYPE_STRING/IGNORE", NULL);

        tagnode->DataCount = 1;

        status = TRUE;
        while(!(tagnode->Data = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(STRINGSIZE);

          strcpy((char *) tagnode->Data, (char *) data);
          tagnode->TagItem.ti_Data = (ULONG) tagnode->Data;
          }
        break;

      case TYPE_WORDLIST:
        DEBUGTEXT("TYPE_WORDLIST", NULL);

        wordarray = (WORD *) data;
        wordpointer = wordarray;
        tagnode->DataCount = *wordpointer;
        DEBUGTEXT("datacount = ", *wordpointer);

        wordpointer++;

        status = TRUE;
        while(!(tagnode->Data = AllocMem(sizeof(WORD) * (tagnode->DataCount+1), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof(WORD) * (tagnode->DataCount+1));

          wordpointer2 = (WORD *) tagnode->Data;
          for(i=0; i<tagnode->DataCount; i++)
            {
            *wordpointer2 = *wordpointer;
            wordpointer++;
            wordpointer2++;
            }
          *wordpointer2 = -1;
          tagnode->TagItem.ti_Data = (ULONG) tagnode->Data;
          }
        break;

      case TYPE_STRINGLIST:
        DEBUGTEXT("TYPE_STRINGLIST", NULL);

        stringdata = (struct StringData *) data;
        tagnode->DataCount = stringdata->StringCount;
        stringpointer = stringdata->StringArray;

        status = TRUE;
        while(!(tagnode->Data = AllocMem(sizeof(char *) * (tagnode->DataCount+1), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof(char *) * (tagnode->DataCount+1));

          stringpointer2 = (char **) tagnode->Data;
          for(i=0; i<tagnode->DataCount; i++)
            {
            status = TRUE;
            while(!(*stringpointer2 = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
              status = PutError("Out of memory", "Retry|Abort");

            if(status)
              {
              DEBUGALLOC(STRINGSIZE);

              strcpy(*stringpointer2, *stringpointer);
              stringpointer2++;
              stringpointer++;
              }
            else
              {
              *stringpointer2 = NULL;
              }
            }
          *stringpointer2 = NULL;
          tagnode->TagItem.ti_Data = (ULONG) tagnode->Data;
          }
        else
          {
          tagnode->DataCount = 0;
          tagnode->TagItem.ti_Data = 0;
          }
        break;

      case TYPE_LINKEDLIST:
        DEBUGTEXT("TYPE_LINKEDLIST", NULL);

        stringdata = (struct StringData *) data;
        tagnode->DataCount = stringdata->StringCount;
        stringpointer = stringdata->StringArray;

        status = TRUE;
        while(!(tagnode->Data = AllocMem(sizeof(struct List), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof(struct List));

          stringlist = (struct List *) tagnode->Data;
          NewList(stringlist);
          for(i=0; i<tagnode->DataCount; i++)
            {
            status = TRUE;
            while(!(stringnode = AllocMem(sizeof(struct StringNode), MEMF_CLEAR)) && status)
              status = PutError("Out of memory", "Retry|Abort");

            if(status)
              {
              DEBUGALLOC(sizeof(struct StringNode));

              strcpy(stringnode->String, *stringpointer);
              stringnode->Node.ln_Name = stringnode->String;
              AddTail(stringlist, (struct Node *) stringnode);
              stringpointer++;
              }
            }
          tagnode->TagItem.ti_Data = (ULONG) tagnode->Data;
          }
        else
          {
          tagnode->DataCount = 0;
          tagnode->TagItem.ti_Data = 0;
          }
        break;
      }

    switch(type)
      {
      case TYPE_SCREEN:
        AddTail(&screennode.TagList, (struct Node *) tagnode);
        screennode.TagCount++;
        break;
      case TYPE_WINDOW:
        AddTail(&windownode->TagList, (struct Node *) tagnode);
        windownode->TagCount++;
        break;
      case TYPE_GADGET:
        AddTail(&newgadgetnode->TagList, (struct Node *) tagnode);
        newgadgetnode->TagCount++;
        break;
      }
    }

  DEBUGEXIT(TRUE, TRUE);
  return(tagnode);
  }

/****************************************************************************/

int AddNewGadget(struct WindowNode *windownode, SHORT x1, SHORT y1, SHORT x2, SHORT y2)
  {
  int    i;
  int    status;
  int    error = FALSE;
  struct NewGadgetNode *newgadgetnode;
  struct TagNode *tagnode;
  struct StringNode *stringnode;
  SHORT  temp;

  DEBUGENTER("AddNewGadget", NULL);

  if(windownode)
    {
    if(x2<x1) { temp = x1; x1 = x2; x2 = temp; }
    if(y2<y1) { temp = y1; y1 = y2; y2 = temp; }

    if(x2-x1<MINGADGETWIDTH)  x2 = x1+MINGADGETWIDTH;
    if(y2-y1<MINGADGETHEIGHT) y2 = y1+MINGADGETHEIGHT;

    UnSelectAllGadgets(NULL);
    RemoveGadgets(windownode);

    status = TRUE;
    while(!(newgadgetnode = (struct NewGadgetNode *) AllocMem(sizeof(struct NewGadgetNode), MEMF_CLEAR)) && status)
      status = PutError("Out of memory", "Retry|Abort");

    if(status)
      {
      DEBUGALLOC(sizeof (struct NewGadgetNode));

      NewList(&newgadgetnode->TagList);

      newgadgetnode->NewGadget.ng_LeftEdge = x1;
      newgadgetnode->NewGadget.ng_TopEdge = y1;
      newgadgetnode->NewGadget.ng_Width = x2 - x1;
      newgadgetnode->NewGadget.ng_Height = y2 - y1;
      newgadgetnode->NewGadget.ng_GadgetID = 0;
      newgadgetnode->NewGadget.ng_VisualInfo = VisualInfo;
      newgadgetnode->NewGadget.ng_UserData = NULL;
      newgadgetnode->NewGadget.ng_GadgetText = newgadgetnode->GadgetText;

      strcpy(newgadgetnode->FontName, "topaz.font");
      newgadgetnode->TextAttr.ta_Name = newgadgetnode->FontName;
      newgadgetnode->TextAttr.ta_YSize = TOPAZ_EIGHTY;
      newgadgetnode->TextAttr.ta_Style = FS_NORMAL;
      newgadgetnode->TextAttr.ta_Flags = FPF_ROMFONT;
      newgadgetnode->NewGadget.ng_TextAttr = &newgadgetnode->TextAttr;

      switch(gadgetkind)
        {
        case BUTTON_KIND:
          DEBUGTEXT("BUTTON_KIND", NULL);
          newgadgetnode->Kind = BUTTON_KIND;
          strcpy(newgadgetnode->GadgetText, "Button");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_IN;
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case CHECKBOX_KIND:
          DEBUGTEXT("CHECKBOX_KIND", NULL);
          newgadgetnode->Kind = CHECKBOX_KIND;
          strcpy(newgadgetnode->GadgetText, "CheckBox");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_RIGHT;
          newgadgetnode->NewGadget.ng_Width = CHECKBOXWIDTH;
          newgadgetnode->NewGadget.ng_Height = CHECKBOXHEIGHT;
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case CYCLE_KIND:
          DEBUGTEXT("CYCLE_KIND", NULL);
          newgadgetnode->Kind = CYCLE_KIND;
          strcpy(newgadgetnode->GadgetText, "Cycle");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          if(tagnode = AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTCY_Labels, (ULONG) &defaultlabels))
            {
            for(i=0; i<3; i++)
              {
              if(stringnode = (struct StringNode *) AllocMem(sizeof(struct StringNode), MEMF_CLEAR))
                {
                DEBUGALLOC(sizeof(struct StringNode));

                strcpy(stringnode->String, "");
                AddTail(&tagnode->SourceLabelList, (struct Node *) stringnode);
                }
              else
                {
                error = TRUE;
                }
              }
            }
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case INTEGER_KIND:
          DEBUGTEXT("INTEGER_KIND", NULL);
          newgadgetnode->Kind = INTEGER_KIND;
          strcpy(newgadgetnode->GadgetText, "Integer");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          if(newgadgetnode->NewGadget.ng_Height < MININPUTHEIGHT) newgadgetnode->NewGadget.ng_Height = MININPUTHEIGHT;
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case LISTVIEW_KIND:
          DEBUGTEXT("LISTVIEW_KIND", NULL);
          newgadgetnode->Kind = LISTVIEW_KIND;
          strcpy(newgadgetnode->GadgetText, "ListView");
          if(newgadgetnode->NewGadget.ng_Width < MINLVWIDTH) newgadgetnode->NewGadget.ng_Width = MINLVWIDTH;
          if(newgadgetnode->NewGadget.ng_Height < MINLVHEIGHT) newgadgetnode->NewGadget.ng_Height = MINLVHEIGHT;
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_ABOVE;
          windownode->IDCMP |= IDCMP_GADGETDOWN | IDCMP_GADGETUP;
          break;

        case MX_KIND:
          DEBUGTEXT("MX_KIND", NULL);
          newgadgetnode->Kind = MX_KIND;
          strcpy(newgadgetnode->GadgetText, "");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_RIGHT;
          newgadgetnode->NewGadget.ng_Width = MXWIDTH;
          newgadgetnode->NewGadget.ng_Height = MXHEIGHT;
          if(tagnode = AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTMX_Labels, (ULONG) &defaultlabels))
            {
            for(i=0; i<3; i++)
              {
              if(stringnode = (struct StringNode *) AllocMem(sizeof(struct StringNode), MEMF_CLEAR))
                {
                DEBUGALLOC(sizeof(struct StringNode));

                strcpy(stringnode->String, "");
                AddTail(&tagnode->SourceLabelList, (struct Node *) stringnode);
                }
              else
                {
                error = TRUE;
                }
              }
            }
          windownode->IDCMP |= IDCMP_GADGETDOWN;
          break;

        case NUMBER_KIND:
          DEBUGTEXT("NUMBER_KIND", NULL);
          newgadgetnode->Kind = NUMBER_KIND;
          strcpy(newgadgetnode->GadgetText, "Number");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTNM_Border, (ULONG) "TRUE");
          break;

        case PALETTE_KIND:
          DEBUGTEXT("PALETTE_KIND", NULL);
          newgadgetnode->Kind = PALETTE_KIND;
          strcpy(newgadgetnode->GadgetText, "Palette");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_ABOVE;
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case SCROLLER_KIND:
          DEBUGTEXT("SCROLLER_KIND", NULL);
          newgadgetnode->Kind = SCROLLER_KIND;
          strcpy(newgadgetnode->GadgetText, "Scroller");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_ABOVE;
          windownode->IDCMP |= IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_MOUSEMOVE;
          break;

        case SLIDER_KIND:
          DEBUGTEXT("SLIDER_KIND", NULL);
          newgadgetnode->Kind = SLIDER_KIND;
          strcpy(newgadgetnode->GadgetText, "Slider");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          windownode->IDCMP |= IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_MOUSEMOVE;
          break;

        case STRING_KIND:
          DEBUGTEXT("STRING_KIND", NULL);
          newgadgetnode->Kind = STRING_KIND;
          strcpy(newgadgetnode->GadgetText, "String");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          if(newgadgetnode->NewGadget.ng_Height < MININPUTHEIGHT) newgadgetnode->NewGadget.ng_Height = MININPUTHEIGHT;
          AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTST_String, (ULONG) "string");
          windownode->IDCMP |= IDCMP_GADGETUP;
          break;

        case TEXT_KIND:
          DEBUGTEXT("TEXT_KIND", NULL);
          newgadgetnode->Kind = TEXT_KIND;
          strcpy(newgadgetnode->GadgetText, "Text");
          newgadgetnode->NewGadget.ng_Flags = PLACETEXT_LEFT;
          AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTTX_Text, (ULONG) "text");
          AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, GTTX_Border, (ULONG) "TRUE");
          break;
        }

      if(!error)
        {
        AddHead(&windownode->NewGadgetList, (struct Node *) newgadgetnode);
        if(AddGadgets(windownode))
          {
          SelectGadget(windownode, newgadgetnode);
          modified = TRUE;

          DEBUGEXIT(TRUE, TRUE);
          return(TRUE);
          }
        }
      else
        {
        PutError("Out of memory", "OK");
        }

      DEBUGFREE(sizeof (struct NewGadgetNode));
      FreeMem(newgadgetnode, sizeof(struct NewGadgetNode));
      }
    }

  DEBUGEXIT(TRUE, FALSE);
  return(FALSE);
  }

/****************************************************************************/

int AddGadgets(struct WindowNode *windownode)
  {
  int    status;
  SHORT  tagcount;
  struct NewGadgetNode *newgadgetnode;
  struct GadgetNode *gadgetnode;
  struct TagNode *tagnode;
  struct Gadget *previousgadget;

  DEBUGENTER("AddGadgets", NULL);

  if(windownode && windownode->Window)
    {
    windownode->FirstGadget = NULL;

    status = TRUE;
    while(!(windownode->ContextGadget = CreateContext(&windownode->FirstGadget)) && status)
      status = PutError("Error creating\ncontext gadget", "Retry|Abort");

    if(status)
      {
      NewList(&windownode->GadgetList);

      previousgadget = windownode->ContextGadget;
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        status = TRUE;
        while(!(gadgetnode = (struct GadgetNode *) AllocMem(sizeof(struct GadgetNode), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof(struct GadgetNode));

          tagcount = 0;
          for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
              tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
            {
            DEBUGTEXT("Tag type = ", tagnode->Node.ln_Type);
            DEBUGTEXT("Tag = ", tagnode->TagItem.ti_Tag);
            DEBUGTEXT("Tag data = ", tagnode->TagItem.ti_Tag);

            switch(tagnode->Node.ln_Type)
              {
              case TYPE_VOID:
              case TYPE_INTEGER:
              case TYPE_STRING:
              case TYPE_CHARACTER:
              case TYPE_WORDLIST:
              case TYPE_STRINGLIST:
              case TYPE_LINKEDLIST:
                tagarray[tagcount].ti_Tag  = tagnode->TagItem.ti_Tag;
                tagarray[tagcount].ti_Data = tagnode->TagItem.ti_Data;
                tagcount++;
                break;
              }
            }
          tagarray[tagcount].ti_Tag = TAG_DONE;

          status = TRUE;

          DEBUGTEXT("CreateGadgetA()", NULL);
          DEBUGTEXT("Kind = ", newgadgetnode->Kind);

          while(!(gadgetnode->Gadget = CreateGadgetA(newgadgetnode->Kind, previousgadget, &newgadgetnode->NewGadget, tagarray)) && status)
            status = PutError("Error creating gadget", "Retry|Abort");

          if(status)
            {
            DEBUGTEXT("AddHead()", NULL);

            AddHead(&windownode->GadgetList, (struct Node *) gadgetnode);
            previousgadget = gadgetnode->Gadget;
            }
          }
        }

      DEBUGTEXT("AddGList()", NULL);

      AddGList(windownode->Window, windownode->FirstGadget, -1, -1, NULL);
      RefreshGadgets(windownode->ContextGadget, windownode->Window, NULL);
      GT_RefreshWindow(windownode->Window, NULL);

      if(realgadgets)
        {
        ModifyIDCMP(windownode->Window, arrayidcmp | gadgetidcmp);
        }
      else
        {
        RemoveGList(windownode->Window, windownode->FirstGadget, -1);
        FreeGadgets(windownode->FirstGadget);
        KillGadgetList(&windownode->GadgetList);
        }

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(newgadgetnode->Selected) MarkGadget(windownode, newgadgetnode);
        }
      }
    }

  DEBUGEXIT(TRUE, TRUE);
  return(TRUE);
  }

/****************************************************************************/

int CopyGadgets(struct WindowNode *windownode)
  {
  int    i;
  int    status;
  int    labelcount;
  BOOL   error = FALSE;
  WORD   *wordpointer, *newwordpointer;
  char   **stringpointer, **newstringpointer;
  struct List          *stringlist, *newstringlist;
  struct TagNode       *tagnode, *newtagnode;
  struct StringNode    *stringnode, *newstringnode;
  struct NewGadgetNode *thisnode;
  struct NewGadgetNode *nextnode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("CopyGadgets", NULL);

  if(windownode)
    {
    MainWindowSleep();
    RemoveGadgets(windownode);

    thisnode = (struct NewGadgetNode *) (windownode->NewGadgetList.lh_Head);
    while(nextnode = (struct NewGadgetNode *) (thisnode->Node.ln_Succ))
      {
      if(thisnode->Selected)
        {
        status = TRUE;
        while(!(newgadgetnode = (struct NewGadgetNode *) AllocMem(sizeof(struct NewGadgetNode), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof(struct NewGadgetNode));

          NewList(&newgadgetnode->TagList);

          strcpy(newgadgetnode->GadgetText, thisnode->GadgetText);
          newgadgetnode->NewGadget.ng_GadgetText = newgadgetnode->GadgetText;

          strcpy(newgadgetnode->FontName, thisnode->FontName);
          newgadgetnode->TextAttr.ta_Name = newgadgetnode->FontName;

          newgadgetnode->Kind                    = thisnode->Kind;
          newgadgetnode->NewGadget.ng_LeftEdge   = thisnode->NewGadget.ng_LeftEdge + 8;
          newgadgetnode->NewGadget.ng_TopEdge    = thisnode->NewGadget.ng_TopEdge + 8;
          newgadgetnode->NewGadget.ng_Width      = thisnode->NewGadget.ng_Width;
          newgadgetnode->NewGadget.ng_Height     = thisnode->NewGadget.ng_Height;
          newgadgetnode->NewGadget.ng_GadgetID   = thisnode->NewGadget.ng_GadgetID;
          newgadgetnode->NewGadget.ng_Flags      = thisnode->NewGadget.ng_Flags;
          newgadgetnode->NewGadget.ng_VisualInfo = thisnode->NewGadget.ng_VisualInfo;
          newgadgetnode->NewGadget.ng_UserData   = thisnode->NewGadget.ng_UserData;
          newgadgetnode->TextAttr.ta_YSize       = thisnode->TextAttr.ta_YSize;
          newgadgetnode->TextAttr.ta_Style       = thisnode->TextAttr.ta_Style;
          newgadgetnode->TextAttr.ta_Flags       = thisnode->TextAttr.ta_Flags;

          newgadgetnode->NewGadget.ng_TextAttr   = &newgadgetnode->TextAttr;

          for(tagnode = (struct TagNode *) thisnode->TagList.lh_Head; tagnode->Node.ln_Succ;
              tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
            {
            status = TRUE;
            while(!(newtagnode = (struct TagNode *) AllocMem(sizeof(struct TagNode), MEMF_CLEAR)) && status)
              status = PutError("Out of memory", "Retry|Abort");

            if(status)
              {
              DEBUGALLOC(sizeof(struct TagNode));

              NewList(&newtagnode->SourceLabelList);
              newtagnode->Node.ln_Type = tagnode->Node.ln_Type;
              newtagnode->Node.ln_Name = tagnode->Node.ln_Name;
              newtagnode->Node.ln_Pri  = tagnode->Node.ln_Pri;
              newtagnode->DataCount = tagnode->DataCount;
              newtagnode->TagItem.ti_Tag = tagnode->TagItem.ti_Tag;

              switch(tagnode->Node.ln_Type)
                {
                case TYPE_VOID:
                case TYPE_CHARACTER:
                  newtagnode->TagItem.ti_Data = tagnode->TagItem.ti_Data;
                  break;

                case TYPE_INTEGER:
                  status = TRUE;
                  while(!(newtagnode->Data = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
                    status = PutError("Out of memory", "Retry|Abort");

                  if(status)
                    {
                    DEBUGALLOC(STRINGSIZE);

                    strcpy((char *) newtagnode->Data, (char *) tagnode->Data);
                    newtagnode->TagItem.ti_Data = tagnode->TagItem.ti_Data;
                    }
                  else
                    {
                    error = TRUE;
                    }
                  break;

                case TYPE_IGNORE:
                case TYPE_STRING:
                  status = TRUE;
                  while(!(newtagnode->Data = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
                    status = PutError("Out of memory", "Retry|Abort");

                  if(status)
                    {
                    DEBUGALLOC(STRINGSIZE);

                    strcpy((char *) newtagnode->Data, (char *) tagnode->Data);
                    newtagnode->TagItem.ti_Data = (ULONG) newtagnode->Data;
                    }
                  else
                    {
                    error = TRUE;
                    }
                  break;

                case TYPE_WORDLIST:
                  status = TRUE;
                  while(!(newtagnode->Data = AllocMem(sizeof(WORD) * (tagnode->DataCount+1), MEMF_CLEAR)) && status)
                    status = PutError("Out of memory", "Retry|Abort");

                  if(status)
                    {
                    DEBUGALLOC(sizeof(WORD) * (tagnode->DataCount+1));

                    wordpointer = (WORD *) tagnode->Data;
                    newwordpointer = (WORD *) newtagnode->Data;
                    for(i=0; i<tagnode->DataCount; i++)
                      {
                      *newwordpointer = *wordpointer;
                      wordpointer++;
                      newwordpointer++;
                      }
                    *newwordpointer = -1;
                    newtagnode->TagItem.ti_Data = (ULONG) newtagnode->Data;
                    }
                  else
                    {
                    error = TRUE;
                    }
                  break;

                case TYPE_STRINGLIST:
                  status = TRUE;
                  while(!(newtagnode->Data = AllocMem(sizeof(char *) * (tagnode->DataCount+1), MEMF_CLEAR)) && status)
                    status = PutError("Out of memory", "Retry|Abort");

                  if(status)
                    {
                    DEBUGALLOC(sizeof(char *) * (tagnode->DataCount+1));

                    labelcount = 0;
                    stringpointer = (char **) tagnode->Data;
                    newstringpointer = (char **) newtagnode->Data;

                    for(i=0; i<tagnode->DataCount; i++)
                      {
                      status = TRUE;
                      while(!(*newstringpointer = AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
                        status = PutError("Out of memory", "Retry|Abort");

                      if(status)
                        {
                        DEBUGALLOC(STRINGSIZE);

                        strcpy((char *) *newstringpointer, (char *) *stringpointer);
                        stringpointer++;
                        newstringpointer++;

                        labelcount++;
                        }
                      else
                        {
                        error = TRUE;
                        }
                      }
                    *newstringpointer = NULL;
                    newtagnode->TagItem.ti_Data = (ULONG) newtagnode->Data;

                    for(i=0; i<labelcount; i++)
                      {
                      status = TRUE;
                      while(!(newstringnode = AllocMem(sizeof(struct StringNode), MEMF_CLEAR)) && status)
                        status = PutError("Out of memory", "Retry|Abort");

                      if(status)
                        {
                        DEBUGALLOC(STRINGSIZE);

                        strcpy(newstringnode->String, "");
                        AddTail(&newtagnode->SourceLabelList, (struct Node *) newstringnode);
                        }
                      else
                        {
                        error = TRUE;
                        }
                      }
                    }
                  else
                    {
                    error = TRUE;
                    }
                  break;

                case TYPE_LINKEDLIST:
                  status = TRUE;
                  while(!(newtagnode->Data = AllocMem(sizeof(struct List), MEMF_CLEAR)) && status)
                    status = PutError("Out of memory", "Retry|Abort");

                  if(status)
                    {
                    DEBUGALLOC(sizeof(struct List));

                    labelcount = 0;
                    stringlist = (struct List *) tagnode->Data;
                    newstringlist = (struct List *) newtagnode->Data;

                    NewList(newstringlist);
                    for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
                        stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
                      {
                      status = TRUE;
                      while(!(newstringnode = AllocMem(sizeof(struct StringNode), MEMF_CLEAR)) && status)
                        status = PutError("Out of memory", "Retry|Abort");

                      if(status)
                        {
                        DEBUGALLOC(sizeof(struct StringNode));

                        strcpy(newstringnode->String, stringnode->String);
                        AddTail(newstringlist, (struct Node *) newstringnode);

                        labelcount++;
                        }
                      else
                        {
                        error = TRUE;
                        }
                      }
                    newtagnode->TagItem.ti_Data = (ULONG) newtagnode->Data;

                    for(i=0; i<labelcount; i++)
                      {
                      status = TRUE;
                      while(!(newstringnode = AllocMem(sizeof(struct StringNode), MEMF_CLEAR)) && status)
                        status = PutError("Out of memory", "Retry|Abort");

                      if(status)
                        {
                        DEBUGALLOC(STRINGSIZE);

                        strcpy(newstringnode->String, "");
                        AddTail(&newtagnode->SourceLabelList, (struct Node *) newstringnode);
                        }
                      else
                        {
                        error = TRUE;
                        }
                      }
                    }
                  else
                    {
                    error = TRUE;
                    }
                  break;
                }

              AddTail(&newgadgetnode->TagList, (struct Node *) newtagnode);
              }
            else
              {
              error = TRUE;  /* Allocate tag memory fail */
              }
            }

          newgadgetnode->Selected = TRUE;

          newgadgetnode->TMGFlags = thisnode->TMGFlags;
          newgadgetnode->TagCount = thisnode->TagCount;
          newgadgetnode->xstart = thisnode->xstart;
          newgadgetnode->ystart = thisnode->ystart;
          newgadgetnode->xend = thisnode->xend;
          newgadgetnode->yend = thisnode->yend;

          AddHead(&windownode->NewGadgetList, (struct Node *) newgadgetnode);

          thisnode->Selected = FALSE;
          modified = TRUE;
          }
        else
          {
          error = TRUE;
          }
        }

      thisnode = nextnode;
      }

    if(!(AddGadgets(windownode)))
      {
      error = TRUE;
      }

    MainWindowWakeUp();
    }
  else
    {
    error = TRUE;  /* no windownode */
    }

  if(error)
    {
    DEBUGEXIT(TRUE, FALSE);
    return(FALSE);
    }
  else
    {
    DEBUGEXIT(TRUE, TRUE);
    return(TRUE);
    }
  }

