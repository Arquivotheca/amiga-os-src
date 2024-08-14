
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>
#include <stdio.h>


BOOL GenerateEventsFile(void)
  {
  BOOL st=TRUE;
  FILE *fp;
  struct WindowNode *windownode;
  struct MenuNode *menunode;
  struct ItemNode *itemnode;
  struct SubNode *subnode;
  struct NewGadgetNode *newgadgetnode;

  DEBUGENTER("GenerateEventsFile", NULL);

  if(userdataflag)
    {
    sprintf(fullname, "%s_events.c", filename);

    if(fp = fopen(fullname, "r"))
      {
      sprintf(fullname, "%s_events.c.new", filename);
      fclose(fp);
      }

    if(fp = fopen(fullname, "w"))
      {
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("/* Events source file */", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      st = GenerateString("#include <exec/types.h>", fp, st, NULL);
      st = GenerateString("#include <intuition/intuition.h>", fp, st, NULL);
      st = GenerateString("#include <libraries/asl.h>", fp, st, NULL);

      st = GenerateString("", fp, st, NULL);

      if(arexxflag)
        {
        st = GenerateString("#include \"SimpleRexx_tm.h\"", fp, st, NULL);
        }

      sprintf(string, "#include \"%s.h\"", savefilename);
      st = GenerateString(string, fp, st, NULL);
/*
      sprintf(string, "#include \"%s_tm.h\"", savefilename);
      st = GenerateString(string, fp, st, NULL);
*/
      st = GenerateString("", fp, st, NULL);

      /* Prototypes */

      st = GenerateString(NULL, fp, st, "Prototypes");
      st = GenerateString(NULL, fp, st, "");

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
            menunode = (struct MenuNode *) menunode->Node.ln_Succ)
          {
          for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
              itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
            {
            if((itemnode->ItemText[0] != '-') && (itemnode->SubCount == 0))
              {
              sprintf(string, "BOOL EventFunc_%s(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", itemnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              }

            for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                subnode = (struct SubNode *) subnode->Node.ln_Succ)
              {
              if(subnode->SubText[0] != '-')
                {
                sprintf(string, "BOOL EventFunc_%s(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", subnode->SourceLabel);
                st = GenerateString(string, fp, st, NULL);
                }
              }
            }
          }
        }

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->Kind != TEXT_KIND &&
             newgadgetnode->Kind != NUMBER_KIND)
            {
            sprintf(string, "BOOL EventFunc_%s(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", newgadgetnode->SourceLabel);
            st = GenerateString(string, fp, st, NULL);
            }
          }
        }
      st = GenerateString("", fp, st, NULL);

      /* UserData structures */

      st = GenerateString(NULL, fp, st, "NewMenu UserData structures");
      st = GenerateString(NULL, fp, st, "");

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
            menunode = (struct MenuNode *) menunode->Node.ln_Succ)
          {
          for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
              itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
            {
            if((itemnode->ItemText[0] != '-') && (itemnode->SubCount == 0))
              {
              sprintf(string, "TMOBJECTDATA tmobjectdata_%s = { EventFunc_%s };", itemnode->SourceLabel, itemnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              }

            for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                subnode = (struct SubNode *) subnode->Node.ln_Succ)
              {
              if(subnode->SubText[0] != '-')
                {
                sprintf(string, "TMOBJECTDATA tmobjectdata_%s = { EventFunc_%s };", subnode->SourceLabel, subnode->SourceLabel);
                st = GenerateString(string, fp, st, NULL);
                }
              }
            }
          }
        }
      st = GenerateString("", fp, st, NULL);

      st = GenerateString(NULL, fp, st, "NewGadget UserData structures");
      st = GenerateString(NULL, fp, st, "");

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->Kind != TEXT_KIND &&
             newgadgetnode->Kind != NUMBER_KIND)
            {
            sprintf(string, "TMOBJECTDATA tmobjectdata_%s = { EventFunc_%s };", newgadgetnode->SourceLabel, newgadgetnode->SourceLabel);
            st = GenerateString(string, fp, st, NULL);
            }
          }
        }
      st = GenerateString("", fp, st, NULL);

      /* Event functions */

      st = GenerateString(NULL, fp, st, "Menu event functions");
      st = GenerateString(NULL, fp, st, "");

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
            menunode = (struct MenuNode *) menunode->Node.ln_Succ)
          {
          for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
              itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
            {
            if((itemnode->ItemText[0] != '-') && (itemnode->SubCount == 0))
              {
              sprintf(string, "BOOL EventFunc_%s(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)", itemnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  {", fp, st, NULL);
              st = GenerateString("  return(FALSE);", fp, st, NULL);
              st = GenerateString("  }", fp, st, NULL);
              st = GenerateString("", fp, st, NULL);
              }

            for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                subnode = (struct SubNode *) subnode->Node.ln_Succ)
              {
              if(subnode->SubText[0] != '-')
                {
                sprintf(string, "BOOL EventFunc_%s(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)", subnode->SourceLabel);
                st = GenerateString(string, fp, st, NULL);
                st = GenerateString("  {", fp, st, NULL);
                st = GenerateString("  return(FALSE);", fp, st, NULL);
                st = GenerateString("  }", fp, st, NULL);
                st = GenerateString("", fp, st, NULL);
                }
              }
            }
          }
        }

      st = GenerateString(NULL, fp, st, "Gadget event functions");
      st = GenerateString(NULL, fp, st, "");

      for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
          windownode = (struct WindowNode *) windownode->Node.ln_Succ)
        {
        for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
            newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
          {
          if(newgadgetnode->Kind != TEXT_KIND &&
             newgadgetnode->Kind != NUMBER_KIND)
            {
            sprintf(string, "BOOL EventFunc_%s(struct TMData *TMData, struct IntuiMessage *imsg, TMOBJECTDATA *tmobjectdata)", newgadgetnode->SourceLabel);
            st = GenerateString(string, fp, st, NULL);
            st = GenerateString("  {", fp, st, NULL);
            st = GenerateString("  return(FALSE);", fp, st, NULL);
            st = GenerateString("  }", fp, st, NULL);
            st = GenerateString("", fp, st, NULL);
            }
          }
        }

      fclose(fp);

      st = UpdateIcon(fullname, "c", st);
      }
    else
      {
      st = FALSE;
      }
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

#endif /* ndef DEMO */

