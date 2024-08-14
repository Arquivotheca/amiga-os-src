
/****************************************************************************/
/*                                                                          */
/*  Process.c - Process screen and windows when no messages remain          */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

/****************************************************************************/

void TagScreen(UBYTE command)
  {
  DEBUGENTER("TagScreen", command);

  switch(command)
    {
    case STATUS_NORMAL:
      screennode.Node.ln_Pri = STATUS_NORMAL;
      break;

    case STATUS_REMOVED:
      screennode.Node.ln_Pri = STATUS_REMOVED;
      break;

    case STATUS_KILLED:
      screennode.Node.ln_Pri = STATUS_KILLED;
      break;

    case COMMAND_KILL:
      screennode.Node.ln_Pri = COMMAND_KILL;
      break;

    case COMMAND_REMOVE:
      screennode.Node.ln_Pri = COMMAND_REMOVE;
      break;

    case COMMAND_CHANGE:
      screennode.Node.ln_Pri = COMMAND_CHANGE;
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void TagMainWindow(UBYTE command)
  {
  DEBUGENTER("TagMainWindow", command);

  switch(command)
    {
    case STATUS_NORMAL:
      mainwindowpri = STATUS_NORMAL;
      break;

    case STATUS_KILLED:
    case STATUS_REMOVED:
      mainwindowpri = STATUS_REMOVED;
      break;

    case COMMAND_KILL:
    case COMMAND_REMOVE:
      if(W_Main)
        {
        ModifyIDCMP(W_Main, IDCMP_CLOSEWINDOW);
        SetPointer(W_Main, SleepPointer, 16, 16, -6, 0);
        }
      mainwindowpri = COMMAND_KILL;
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void TagWindow(struct WindowNode *windownode, UBYTE command)
  {
  DEBUGENTER("TagWindow", command);

  switch(command)
    {
    case STATUS_NORMAL:
      windownode->Node.ln_Pri = STATUS_NORMAL;
      break;

    case STATUS_REMOVED:
      windownode->Node.ln_Pri = STATUS_REMOVED;
      break;

    case COMMAND_KILL:
      if(windownode->Window)
        {
        ModifyIDCMP(windownode->Window, IDCMP_CLOSEWINDOW);
        SetPointer(windownode->Window, SleepPointer, 16, 16, -6, 0);
        }
      windownode->Node.ln_Pri = COMMAND_KILL;
      break;

    case COMMAND_REMOVE:
      if(windownode->Window)
        {
        ModifyIDCMP(windownode->Window, IDCMP_CLOSEWINDOW);
        SetPointer(windownode->Window, SleepPointer, 16, 16, -6, 0);
        }
      windownode->Node.ln_Pri = COMMAND_REMOVE;
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void TagWindows(UBYTE command)
  {
  struct WindowNode *windownode;

  DEBUGENTER("TagWindows", NULL);

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    TagWindow(windownode, command);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void ProcessScreen(void)
  {
  struct WindowNode *windownode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("ProcessScreen", NULL);
  DEBUGTEXT("command/status = ", screennode.Node.ln_Pri);

  switch(screennode.Node.ln_Pri)
    {
    case COMMAND_KILL:
      KillScreen();
      break;

    case COMMAND_REMOVE:
      RemoveScreen();
      break;

    case COMMAND_CHANGE:
      RemoveScreen();

      if(openflag || newprojectflag)
        {
        DEBUGTEXT("openflag || newprojectflag", NULL);

        realgadgets = FALSE;
        realmenus = FALSE;
        commentflag = FALSE;
        autotopborderflag = FALSE;
        pragmaflag = FALSE;
        usersignalflag = FALSE;
        iconflag = TRUE;
        chipflag = FALSE;
        userdataflag = FALSE;
        aslflag = FALSE;
        gridsize = 4;

        KillFontList(&FontList);
        KillTagList(&screennode.TagList);
        screennode.TagCount = 0;
        TagScreen(STATUS_KILLED);
        }

      if(openflag)
        {
        DEBUGTEXT("openflag", NULL);

        ActuallyOpenFile();
        }

      if(newprojectflag)
        {
        DEBUGTEXT("newprojectflag", NULL);

        sprintf(screennode.Title, "%s Screen", title);

        screennode.DisplayID   = HIRES_KEY;
        screennode.Overscan    = NULL;
        screennode.Mode        = MODE_WORKBENCH     |
                                 MODE_DEFAULT       |
                                 MODE_HIRES         |
                                 MODE_OSCANTEXT;

        screennode.Width       = 640;
        screennode.Height      = 200;
        screennode.Depth       = 1;

        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Title, (ULONG) screennode.Title);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_DisplayID, screennode.DisplayID);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Depth, screennode.Depth);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Width, screennode.Width);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Height, screennode.Height);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Overscan, screennode.Overscan);
        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Font, (ULONG) &screennode.TextAttr);

        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Pens, (ULONG) defaultpens);

        filename[0] = '\0';
        savefilename[0] = '\0';
        savepathname[0] = '\0';

        OpenSettingsFile();

        TagScreen(STATUS_REMOVED);
        }

      if(!openflag && !newprojectflag && autotopborderflag)
        {
        for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
            windownode = (struct WindowNode *) windownode->Node.ln_Succ)
          {
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            newgadgetnode->NewGadget.ng_TopEdge -= currenttopborder;
            }
          }
        }

      AddScreen();

      if(!openflag && !newprojectflag && autotopborderflag)
        {
        for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
            windownode = (struct WindowNode *) windownode->Node.ln_Succ)
          {
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            newgadgetnode->NewGadget.ng_TopEdge += currenttopborder;
            }
          }
        }

      AddAllWindows();

      if(newprojectflag)
        {
        AddNewWindow();
        if(W_Main) ActivateWindow(W_Main);
        }

      SetUpMenus();

      if(openflag || newprojectflag) modified = FALSE;

      openflag = FALSE;
      newprojectflag = FALSE;

      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void ProcessWindows(void)
  {
  struct WindowNode *thisnode, *nextnode;

  DEBUGENTER("ProcessWindows", NULL);

  /********** User windows **********/

  thisnode = (struct WindowNode *) WindowList.lh_Head;
  while(nextnode = (struct WindowNode *) (thisnode->Node.ln_Succ))
    {
    switch(thisnode->Node.ln_Pri)
      {
      case COMMAND_KILL:
        KillWindow(thisnode);
        break;

      case COMMAND_REMOVE:
        RemoveWindow(thisnode);
        break;
      }

    thisnode = nextnode;
    }

  /********** Main window **********/

  switch(mainwindowpri)
    {
    case COMMAND_KILL:
    case COMMAND_REMOVE:
      KillMainWindow();
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

