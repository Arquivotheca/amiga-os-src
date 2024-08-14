
#include "Toolmaker.h"
#include "Externs.h"

#include <dos/dosextens.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

/****************************************************************************/

void KillScreen(void)
  {
  DEBUGENTER("KillScreen", NULL);

  KillTagList(&screennode.TagList);
  screennode.TagCount = 0;

  RemoveScreen();

  TagScreen(STATUS_KILLED);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemoveScreen(void)
  {
  DEBUGENTER("RemoveScreen", NULL);

  if(VisualInfo)
    {
    FreeVisualInfo(VisualInfo);
    VisualInfo = NULL;
    }

  if(screennode.Screen)
    {
    if(screennode.Workbench)
      UnlockPubScreen(NULL, screennode.Screen);
    else
      CloseScreen(screennode.Screen);

    screennode.Screen = NULL;
    }

  TagScreen(STATUS_REMOVED);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillWindow(struct WindowNode *windownode)
  {
  DEBUGENTER("KillWindow", NULL);

  if(windownode)
    {
    if(currentwindow == windownode)
      {
      currentwindow = NULL;
      }

    Remove((struct Node *) windownode);
    RemoveGadgets(windownode);

    if(windownode->Window)
      {
      ClearMenuStrip(windownode->Window);
      ClearPointer(windownode->Window);
      CloseWindow(windownode->Window);
      windownode->Window = NULL;
      }

    if(windownode->Menu)
      {
      FreeMenus(windownode->Menu);
      windownode->Menu = NULL;
      }

    KillTagList(&windownode->TagList);
    KillNewGadgetList(&windownode->NewGadgetList);
    KillMenuList(&windownode->MenuList);

    DEBUGFREE(sizeof (struct WindowNode));
    FreeMem(windownode, sizeof(struct WindowNode));

    windowcount--;
    RedoWindowSignals();
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemoveWindow(struct WindowNode *windownode)
  {
  DEBUGENTER("RemoveWindow", NULL);

  if(windownode)
    {
    if(currentwindow == windownode)
      {
      currentwindow = NULL;
      }

    RemoveGadgets(windownode);

    if(windownode->Window)
      {
      ClearMenuStrip(windownode->Window);
      ClearPointer(windownode->Window);
      CloseWindow(windownode->Window);
      windownode->Window = NULL;
      }

    TagWindow(windownode, STATUS_REMOVED);
    RedoWindowSignals();
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillMainWindow(void)
  {
  struct Process *process;

  DEBUGENTER("KillMainWindow", NULL);

  if(W_Main)
    {
    process = (struct Process *) FindTask(NULL);
    process->pr_WindowPtr = oldwindowptr;
    oldwindowptr = NULL;

    ClearMenuStrip(W_Main);
    ClearPointer(W_Main);
    CloseWindow(W_Main);
    W_Main = NULL;
    }

  if(G_Main)
    {
    FreeGadgets(G_Main);
    G_Main = NULL;
    }

  if(mainwindowpri == COMMAND_KILL)
    TagMainWindow(STATUS_KILLED);
  else
    TagMainWindow(STATUS_REMOVED);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemoveGadgets(struct WindowNode *windownode)
  {
  DEBUGENTER("RemoveGadgets", NULL);

  if(windownode && windownode->Window)
    {
    if(realgadgets)
      {
      RemoveGList(windownode->Window, windownode->FirstGadget, -1);
      FreeGadgets(windownode->FirstGadget);
      windownode->FirstGadget = NULL;
      KillGadgetList(&windownode->GadgetList);
      ModifyIDCMP(windownode->Window, arrayidcmp);
      }

    SetRast(windownode->Window->RPort, 0);
    RefreshWindowFrame(windownode->Window);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillSelectedGadgets(struct WindowNode *windownode)
  {
  struct NewGadgetNode *thisnode, *nextnode;

  DEBUGENTER("KillGadgets", NULL);

  if(windownode)
    {
    MainWindowSleep();

    RemoveGadgets(windownode);

    thisnode = (struct NewGadgetNode *) (windownode->NewGadgetList.lh_Head);
    while(nextnode = (struct NewGadgetNode *) (thisnode->Node.ln_Succ))
      {
      if(thisnode->Selected)
        {
        KillTagList(&thisnode->TagList);
        Remove((struct Node *) thisnode);

        DEBUGFREE(sizeof (struct NewGadgetNode));
        FreeMem(thisnode, sizeof(struct NewGadgetNode));
        }
      thisnode = nextnode;
      }

    AddGadgets(windownode);
    modified = TRUE;

    MainWindowWakeUp();
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillTag(struct TagNode *tagnode)
  {
  struct List *stringlist;
  char **stringpointer;
  int i;

  DEBUGENTER("KillTag", NULL);

  if(tagnode)
    {
    switch(tagnode->Node.ln_Type)
      {
      case TYPE_INTEGER:
      case TYPE_IGNORE:
      case TYPE_STRING:
        if(tagnode->Data)
          {
          DEBUGFREE(STRINGSIZE);
          FreeMem((APTR) tagnode->Data, STRINGSIZE);
          }
        break;

      case TYPE_WORDLIST:
        if(tagnode->Data)
          {
          DEBUGFREE(sizeof(WORD) * (tagnode->DataCount+1));
          FreeMem((APTR) tagnode->Data, sizeof(WORD) * (tagnode->DataCount+1));
          }
        break;

      case TYPE_STRINGLIST:
        KillStringList(&tagnode->SourceLabelList);
        if(tagnode->Data)
          {
          stringpointer = (char **) tagnode->Data;
          for(i=0; i<tagnode->DataCount; i++)
            {
            if(*stringpointer)
              {
              DEBUGFREE(STRINGSIZE);
              FreeMem((APTR) *stringpointer, STRINGSIZE);
              }
            stringpointer++;
            }

          DEBUGFREE(sizeof(char *) * (tagnode->DataCount+1));
          FreeMem((APTR) tagnode->Data, sizeof(char *) * (tagnode->DataCount+1));
          }
        break;

      case TYPE_LINKEDLIST:
        KillStringList(&tagnode->SourceLabelList);
        if(tagnode->Data)
          {
          stringlist = (struct List *) tagnode->Data;
          KillStringList(stringlist);

          DEBUGFREE(sizeof(struct List));
          FreeMem((APTR) tagnode->Data, sizeof(struct List));
          }
        break;
      }

    DEBUGFREE(sizeof (struct TagNode));
    FreeMem(tagnode, sizeof(struct TagNode));
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillSubList(struct List *list)
  {
  struct SubNode *thisnode, *nextnode;

  DEBUGENTER("KillSubList", NULL);

  if(list)
    {
    thisnode = (struct SubNode *) (list->lh_Head);
    while(nextnode = (struct SubNode *) (thisnode->Node.ln_Succ))
      {
      DEBUGFREE(sizeof (struct SubNode));
      FreeMem(thisnode, sizeof(struct SubNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillItemList(struct List *list)
  {
  struct ItemNode *thisnode, *nextnode;

  DEBUGENTER("KillItemList", NULL);

  if(list)
    {
    thisnode = (struct ItemNode *) (list->lh_Head);
    while(nextnode = (struct ItemNode *) (thisnode->Node.ln_Succ))
      {
      KillSubList(&thisnode->SubList);

      DEBUGFREE(sizeof (struct ItemNode));
      FreeMem(thisnode, sizeof(struct ItemNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillMenuList(struct List *list)
  {
  struct MenuNode *thisnode, *nextnode;

  DEBUGENTER("KillMenuList", NULL);

  if(list)
    {
    thisnode = (struct MenuNode *) (list->lh_Head);
    while(nextnode = (struct MenuNode *) (thisnode->Node.ln_Succ))
      {
      KillItemList(&thisnode->ItemList);

      DEBUGFREE(sizeof (struct MenuNode));
      FreeMem(thisnode, sizeof(struct MenuNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillTagList(struct List *list)
  {
  struct TagNode *thisnode, *nextnode;

  DEBUGENTER("KillTagList", NULL);

  if(list)
    {
    thisnode = (struct TagNode *) (list->lh_Head);
    while(nextnode = (struct TagNode *) (thisnode->Node.ln_Succ))
      {
      KillTag(thisnode);
      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillNewGadgetList(struct List *list)
  {
  struct NewGadgetNode *thisnode, *nextnode;

  DEBUGENTER("KillNewGadgetList", NULL);

  if(list)
    {
    thisnode = (struct NewGadgetNode *) (list->lh_Head);
    while(nextnode = (struct NewGadgetNode *) (thisnode->Node.ln_Succ))
      {
      KillTagList(&thisnode->TagList);

      DEBUGFREE(sizeof (struct NewGadgetNode));
      FreeMem(thisnode, sizeof(struct NewGadgetNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillGadgetList(struct List *list)
  {
  struct GadgetNode *thisnode, *nextnode;

  DEBUGENTER("KillGadgetList", NULL);

  if(list)
    {
    thisnode = (struct GadgetNode *) (list->lh_Head);
    while(nextnode = (struct GadgetNode *) (thisnode->Node.ln_Succ))
      {
      DEBUGFREE(sizeof (struct GadgetNode));
      FreeMem(thisnode, sizeof(struct GadgetNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillEditTagList(struct List *list)
  {
  struct EditTagNode *thisnode, *nextnode;

  DEBUGENTER("KillEditTagList", NULL);

  if(list)
    {
    thisnode = (struct EditTagNode *) (list->lh_Head);
    while(nextnode = (struct EditTagNode *) (thisnode->Node.ln_Succ))
      {
      KillEditStringList(&thisnode->EditStringList);

      DEBUGFREE(sizeof (struct EditTagNode));
      FreeMem(thisnode, sizeof(struct EditTagNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillStringList(struct List *list)
  {
  struct StringNode *thisnode, *nextnode;

  DEBUGENTER("KillStringList", NULL);

  if(list)
    {
    thisnode = (struct StringNode *) (list->lh_Head);
    while(nextnode = (struct StringNode *) (thisnode->Node.ln_Succ))
      {
      DEBUGFREE(sizeof (struct StringNode));
      FreeMem(thisnode, sizeof(struct StringNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillEditStringList(struct List *list)
  {
  struct EditStringNode *thisnode, *nextnode;

  DEBUGENTER("KillEditStringList", NULL);

  if(list)
    {
    thisnode = (struct EditStringNode *) (list->lh_Head);
    while(nextnode = (struct EditStringNode *) (thisnode->Node.ln_Succ))
      {
      DEBUGFREE(sizeof (struct EditStringNode));
      FreeMem(thisnode, sizeof(struct EditStringNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillTextAttrList(struct List *list)
  {
  struct TextAttrNode *thisnode, *nextnode;

  DEBUGENTER("KillTextAttrList", NULL);

  if(list)
    {
    thisnode = (struct TextAttrNode *) (list->lh_Head);
    while(nextnode = (struct TextAttrNode *) (thisnode->Node.ln_Succ))
      {
      DEBUGFREE(sizeof (struct TextAttrNode));
      FreeMem(thisnode, sizeof(struct TextAttrNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void KillFontList(struct List *list)
  {
  struct FontNode *thisnode, *nextnode;

  DEBUGENTER("KillFontList", NULL);

  if(list)
    {
    thisnode = (struct FontNode *) (list->lh_Head);
    while(nextnode = (struct FontNode *) (thisnode->Node.ln_Succ))
      {
      if(thisnode->TextFont) CloseFont(thisnode->TextFont);

      DEBUGFREE(sizeof (struct FontNode));
      FreeMem(thisnode, sizeof(struct FontNode));

      thisnode = nextnode;
      }
    NewList(list);
    }

  DEBUGEXIT(FALSE, NULL);
  }

