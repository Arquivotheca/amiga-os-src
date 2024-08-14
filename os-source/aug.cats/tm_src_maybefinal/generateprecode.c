
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GeneratePrecode(FILE *fp, BOOL st)
  {
  static char commkey[15], exclude[10];
  int menucount, itemcount, subcount, i;
  BOOL more;
  WORD *wordpointer;
  char **stringpointer;
  struct WindowNode    *windownode;
  struct MenuNode      *menunode;
  struct ItemNode      *itemnode;
  struct SubNode       *subnode;
  struct TagNode       *tagnode;
  struct NewGadgetNode *newgadgetnode;
  struct AvailableTags *availabletags;
  struct TextAttrNode  *textattrnode;
  struct StringNode    *stringnode;
  struct StringNode    *labelnode;

  DEBUGENTER("GeneratePrecode", (ULONG) st);

  if(fp && st)
    {
    /****************************** Includes *********************************/

    st = GenerateString(NULL, fp, st, "Includes");
    st = GenerateString(NULL, fp, st, "");

    st = GenerateString("#include <stdlib.h>", fp, st, NULL);
    st = GenerateString("#include <exec/types.h>", fp, st, NULL);
    st = GenerateString("#include <intuition/intuition.h>", fp, st, NULL);
    st = GenerateString("#include <intuition/gadgetclass.h>", fp, st, NULL);
    st = GenerateString("#include <libraries/gadtools.h>", fp, st, NULL);
    st = GenerateString("#include <exec/memory.h>", fp, st, NULL);
    st = GenerateString("#include <graphics/view.h>", fp, st, NULL);
    st = GenerateString("#include <graphics/displayinfo.h>", fp, st, NULL);
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
    sprintf(string, "#include \"%s_tm_text.h\"", savefilename);
    st = GenerateString(string, fp, st, NULL);
*/
    st = GenerateString("", fp, st, NULL);

    st = GenerateString("#include <clib/exec_protos.h>", fp, st, NULL);
    st = GenerateString("#include <clib/intuition_protos.h>", fp, st, NULL);
    st = GenerateString("#include <clib/gadtools_protos.h>", fp, st, NULL);

    if(diskfontcount)
      {
      st = GenerateString("#include <clib/graphics_protos.h>", fp, st, NULL);
      st = GenerateString("#include <clib/diskfont_protos.h>", fp, st, NULL);
      }
    st = GenerateString("", fp, st, NULL);

    /************************** Pragma includes ******************************/

    if(pragmaflag)
      {
      st = GenerateString(NULL, fp, st, "Pragma includes for register parameters");
      st = GenerateString(NULL, fp, st, "");

      st = GenerateString("#include <pragmas/exec_pragmas.h>", fp, st, NULL);
      st = GenerateString("#include <pragmas/intuition_pragmas.h>", fp, st, NULL);
      st = GenerateString("#include <pragmas/gadtools_pragmas.h>", fp, st, NULL);

      if(diskfontcount)
        {
        st = GenerateString("#include <pragmas/graphics_pragmas.h>", fp, st, NULL);
        st = GenerateString("#include <pragmas/diskfont_pragmas.h>", fp, st, NULL);
        }

      st = GenerateString("", fp, st, NULL);
      }

    /***************************** Prototypes ********************************/

    st = GenerateString(NULL, fp, st, "static function prototypes");
    st = GenerateString(NULL, fp, st, "");

    st = GenerateString("static BOOL TM_WindowSignal(struct TMData *);", fp, st, NULL);

    if(arexxflag)
      {
      st = GenerateString("static BOOL TM_ARexxSignal(struct TMData *TMData);", fp, st, NULL);
      }

    st = GenerateString("static VOID TM_RemoveWindow(struct TMWindowInfo *);", fp, st, NULL);

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      sprintf(string, "static BOOL WindowIDCMP_%s(struct TMData *, struct IntuiMessage *);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      }
    st = GenerateString("", fp, st, NULL);

    /************************* Sleep Pointer Data ****************************/

    st = GenerateString(NULL, fp, st, "Mouse pointer data for disabled window");
    st = GenerateString(NULL, fp, st, "");

    if(chipflag)
      {
      st = GenerateString("#ifdef _DCC    /* For DICE compatibiliity */", fp, st, NULL);
      st = GenerateString("__chip UWORD WaitPointer[36] =", fp, st, NULL);
      st = GenerateString("#else", fp, st, NULL);
      st = GenerateString("UWORD __chip WaitPointer[36] =", fp, st, NULL);
      st = GenerateString("#endif", fp, st, NULL);
      }
    else
      {
      st = GenerateString("UWORD WaitPointer[36] =", fp, st, NULL);
      }

    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  0x0000, 0x0000,", fp, st, NULL);
    st = GenerateString("  0x0400, 0x07C0,", fp, st, NULL);
    st = GenerateString("  0x0000, 0x07C0,", fp, st, NULL);
    st = GenerateString("  0x0100, 0x0380,", fp, st, NULL);
    st = GenerateString("  0x0000, 0x07E0,", fp, st, NULL);
    st = GenerateString("  0x07C0, 0x1FF8,", fp, st, NULL);
    st = GenerateString("  0x1FF0, 0x3FEC,", fp, st, NULL);
    st = GenerateString("  0x3FF8, 0x7FDE,", fp, st, NULL);
    st = GenerateString("  0x3FF8, 0x7FBE,", fp, st, NULL);
    st = GenerateString("  0x7FFC, 0xFF7F,", fp, st, NULL);
    st = GenerateString("  0x7EFC, 0xFFFF,", fp, st, NULL);
    st = GenerateString("  0x7FFC, 0xFFFF,", fp, st, NULL);
    st = GenerateString("  0x3FF8, 0x7FFE,", fp, st, NULL);
    st = GenerateString("  0x3FF8, 0x7FFE,", fp, st, NULL);
    st = GenerateString("  0x1FF0, 0x3FFC,", fp, st, NULL);
    st = GenerateString("  0x07C0, 0x1FF8,", fp, st, NULL);
    st = GenerateString("  0x0000, 0x07E0,", fp, st, NULL);
    st = GenerateString("  0x0000, 0x0000,", fp, st, NULL);
    st = GenerateString("  };", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /***************************** Screen Data *******************************/

    st = GenerateString(NULL, fp, st, "Screen data");
    st = GenerateString(NULL, fp, st, "");

    if(!screennode.Workbench)
      {
      /* Screen colors */

      if(screennode.Mode & MODE_CUSTOMCOLORS)
        {
        st = GenerateString("struct ColorSpec ColorSpec[] = ", fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        for(i=0; i<1<<screennode.Depth; i++)
          {
          sprintf(string, "    {%d, 0x%x, 0x%x, 0x%x},", i,
                                    COLOR2RED(screennode.Color[i]),
                                    COLOR2GREEN(screennode.Color[i]),
                                    COLOR2BLUE(screennode.Color[i]));
          st = GenerateString(string, fp, st, NULL);
          }

        st = GenerateString("    {-1, 0x0, 0x0, 0x0}", fp, st, NULL);
        st = GenerateString("  };", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      /* Screen tag labels */

      for(tagnode = (struct TagNode *) screennode.TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(AvailableScreenTags[tagnode->Node.ln_Pri].Type)
          {
          case TYPE_WORDLIST:
            sprintf(string, "WORD %s_%s[] = {", AvailableScreenTags[tagnode->Node.ln_Pri].Name, screennode.SourceLabel);
            more = FALSE;
            wordpointer = (WORD *) tagnode->Data;
            for(i=0; i<tagnode->DataCount; i++)
              {
              if(more) strcat(string, ",");
              sprintf(string2, "%d", *wordpointer);
              strcat(string, string2);
              more = TRUE;
              wordpointer++;
              }
            strcat(string, "};");
            st = GenerateString(string, fp, st, NULL);
            st = GenerateString("", fp, st, NULL);
            break;
          }
        }
      st = GenerateString("", fp, st, NULL);
      }

    /***************************** Window Data *******************************/

    st = GenerateString(NULL, fp, st, "Window data");
    st = GenerateString(NULL, fp, st, "");

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      /****** NewMenu structures ******/

      menucount = 0;
      for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
          menunode = (struct MenuNode *) menunode->Node.ln_Succ)
        {
        for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
            itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
          {
          for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
              subnode = (struct SubNode *) subnode->Node.ln_Succ)
            {
            menucount++;
            }
          menucount++;
          }
        menucount++;
        }

      if(menucount)
        {
        sprintf(string, "struct NewMenu newmenu_%s[] =", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);

        for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
            menunode = (struct MenuNode *) menunode->Node.ln_Succ)
          {
          sprintf(string, "  {NM_TITLE, (UBYTE *)MENUTEXT_%s,\tNULL,\t%s,\t0,\tNULL},", menunode->SourceLabel, MenuFlags2String(menunode->Flags));
          st = GenerateString(string, fp, st, menunode->MenuText);

          itemcount = 0;
          for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
              itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
            {
            if(itemnode->CommKey[0] == '\0')
              strcpy(commkey, "NULL");
            else
              sprintf(commkey, "(UBYTE *)\"%s\"", itemnode->CommKey);

            if(itemnode->TMMFlags & MENUFLAG_AUTOEXCLUDE)
              sprintf(exclude, "~%d", 1 << itemcount);
            else
              strcpy(exclude, "0");

            if(itemnode->ItemText[0] == '-')
              sprintf(string, "  {NM_ITEM,  NM_BARLABEL,\t%s,\t%s,\t%s,\tNULL},", commkey, ItemFlags2String(itemnode->Flags), exclude);
            else
              {
              if(itemnode->SubCount)
                sprintf(string, "  {NM_ITEM,  (UBYTE *)ITEMTEXT_%s,\t%s,\t%s,\t%s,\t%s},", itemnode->SourceLabel, commkey, ItemFlags2String(itemnode->Flags), exclude, "NULL");
              else
                sprintf(string, "  {NM_ITEM,  (UBYTE *)ITEMTEXT_%s,\t%s,\t%s,\t%s,\t%s},", itemnode->SourceLabel, commkey, ItemFlags2String(itemnode->Flags), exclude, UserDataName(itemnode->SourceLabel));
              }

            st = GenerateString(string, fp, st, itemnode->ItemText);

            subcount = 0;
            for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                subnode = (struct SubNode *) subnode->Node.ln_Succ)
              {
              if(subnode->CommKey[0] == '\0')
                strcpy(commkey, "NULL");
              else
                sprintf(commkey, "(UBYTE *)\"%s\"", subnode->CommKey);

              if(subnode->TMMFlags & MENUFLAG_AUTOEXCLUDE)
                sprintf(exclude, "~%d", 1 << subcount);
              else
                strcpy(exclude, "0");

              if(subnode->SubText[0] == '-')
                sprintf(string, "  {NM_SUB,   NM_BARLABEL,\t%s,\t%s,\t%s,\tNULL},", commkey, ItemFlags2String(subnode->Flags), exclude);
              else
                sprintf(string, "  {NM_SUB,   (UBYTE *)SUBTEXT_%s,\t%s,\t%s,\t%s,\t%s},", subnode->SourceLabel, commkey, ItemFlags2String(subnode->Flags), exclude, UserDataName(subnode->SourceLabel));

              st = GenerateString(string, fp, st, subnode->SubText);

              subcount++;
              }
            itemcount++;
            }
          }
        st = GenerateString("  {NM_END,   NULL,\tNULL,\t0,\t0,\tNULL}", fp, st, NULL);
        st = GenerateString("  };", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      /****** Window tag labels ******/

      for(tagnode = (struct TagNode *) windownode->TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(AvailableWindowTags[tagnode->Node.ln_Pri].Type)
          {
          case TYPE_WORDLIST:
            sprintf(string, "WORD %s_%s[] = {", AvailableWindowTags[tagnode->Node.ln_Pri].Name, windownode->SourceLabel);
            more = FALSE;
            wordpointer = (WORD *) tagnode->Data;
            for(i=0; i<tagnode->DataCount; i++)
              {
              if(more) strcat(string, ",");
              sprintf(string2, "%d", *wordpointer);
              strcat(string, string2);
              more = TRUE;
              wordpointer++;
              }
            strcat(string, "};");
            st = GenerateString(string, fp, st, NULL);
            st = GenerateString("", fp, st, NULL);
            break;
          }
        }

      /****** Gadget tag labels ******/

      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
            tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
          {
          switch(newgadgetnode->Kind)
            {
            case BUTTON_KIND:
              availabletags = &AvailableBUGadgetTags[0];
              break;
            case CHECKBOX_KIND:
              availabletags = &AvailableCBGadgetTags[0];
              break;
            case CYCLE_KIND:
              availabletags = &AvailableCYGadgetTags[0];
              break;
            case INTEGER_KIND:
              availabletags = &AvailableINGadgetTags[0];
              break;
            case LISTVIEW_KIND:
              availabletags = &AvailableLVGadgetTags[0];
              break;
            case MX_KIND:
              availabletags = &AvailableMXGadgetTags[0];
              break;
            case NUMBER_KIND:
              availabletags = &AvailableNMGadgetTags[0];
              break;
            case PALETTE_KIND:
              availabletags = &AvailablePAGadgetTags[0];
              break;
            case SCROLLER_KIND:
              availabletags = &AvailableSCGadgetTags[0];
              break;
            case SLIDER_KIND:
              availabletags = &AvailableSLGadgetTags[0];
              break;
            case STRING_KIND:
              availabletags = &AvailableSTGadgetTags[0];
              break;
            case TEXT_KIND:
              availabletags = &AvailableTXGadgetTags[0];
              break;
            }

          switch(availabletags[tagnode->Node.ln_Pri].Type)
            {
            case TYPE_STRINGLIST:
              stringpointer = (char **) tagnode->Data;
              labelnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
              sprintf(string, "char *%s_%s[] =", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  {", fp, st, NULL);
              for(i=0; i<tagnode->DataCount; i++)
                {
                sprintf(string,   "  %s_%s_%s,", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, labelnode->String);
                st = GenerateString(string, fp, st, *stringpointer);
                stringpointer++;
                labelnode = (struct StringNode *) ((struct Node *) labelnode)->ln_Succ;
                }
              st = GenerateString("  NULL", fp, st, NULL);
              st = GenerateString("  };", fp, st, NULL);
              st = GenerateString("", fp, st, NULL);
              break;

            case TYPE_LINKEDLIST:
              sprintf(string, "extern struct List %s_%s;", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("", fp, st, NULL);

              stringnode = (struct StringNode *) ((struct List *) tagnode->Data)->lh_Head;
              labelnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
              sprintf(string, "struct Node %s_%s_node[] =", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  {", fp, st, NULL);

              for(i=0; i<tagnode->DataCount; i++)
                {
                if(i == 0)  /* First in list */
                  {
                  st = GenerateString("    {", fp, st, NULL);
                  sprintf(string,     "    &%s_%s_node[%d],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, i+1);
                  st = GenerateString(string, fp, st, NULL);
                  sprintf(string,     "    (struct Node *) &%s_%s.lh_Head,", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
                  st = GenerateString(string, fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  sprintf(string,     "    %s_%s_%s", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, labelnode->String);
                  st = GenerateString(string, fp, st, stringnode->String);
                  st = GenerateString("    },", fp, st, NULL);
                  }
                else if(i == tagnode->DataCount-1)  /* Last in list */
                  {
                  st = GenerateString("    {", fp, st, NULL);
                  sprintf(string,     "    (struct Node *) &%s_%s.lh_Tail,", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
                  st = GenerateString(string, fp, st, NULL);
                  sprintf(string,     "    &%s_%s_node[%d],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, i-1);
                  st = GenerateString(string, fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  sprintf(string,     "    %s_%s_%s", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, labelnode->String);
                  st = GenerateString(string, fp, st, stringnode->String);
                  st = GenerateString("    }", fp, st, NULL);
                  }
                else  /* Middle of list */
                  {
                  st = GenerateString("    {", fp, st, NULL);
                  sprintf(string,     "    &%s_%s_node[%d],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, i+1);
                  st = GenerateString(string, fp, st, NULL);
                  sprintf(string,     "    &%s_%s_node[%d],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, i-1);
                  st = GenerateString(string, fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  st = GenerateString("    0,", fp, st, NULL);
                  sprintf(string,     "    %s_%s_%s", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, labelnode->String);
                  st = GenerateString(string, fp, st, stringnode->String);
                  st = GenerateString("    },", fp, st, NULL);
                  }

                stringnode = (struct StringNode *) ((struct Node *) stringnode)->ln_Succ;
                labelnode = (struct StringNode *) ((struct Node *) labelnode)->ln_Succ;
                }

              st = GenerateString("  };", fp, st, NULL);
              st = GenerateString("", fp, st, NULL);

              sprintf(string,     "struct List %s_%s =", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  {", fp, st, NULL);
              sprintf(string,     "  &%s_%s_node[0],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  NULL,", fp, st, NULL);
              sprintf(string,     "  &%s_%s_node[%d],", availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel, tagnode->DataCount-1);
              st = GenerateString(string, fp, st, NULL);
              st = GenerateString("  0,", fp, st, NULL);
              st = GenerateString("  0", fp, st, NULL);
              st = GenerateString("  };", fp, st, NULL);
              st = GenerateString("", fp, st, NULL);
              break;
            }
          }
        }
      }
    st = GenerateString("", fp, st, NULL);

    /****************************** Font Data ********************************/

    st = GenerateString(NULL, fp, st, "Font data");
    st = GenerateString(NULL, fp, st, "");

    for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_Head; textattrnode->Node.ln_Succ;
        textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Succ)
      {
      sprintf(string,   "struct TextAttr %s =", textattrnode->Name);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      sprintf(string,   "  (STRPTR)\"%s\",", textattrnode->FontName);
      st = GenerateString(string, fp, st, "ta_Name");
      sprintf(string,   "  %d,", textattrnode->TextAttr.ta_YSize);
      st = GenerateString(string, fp, st, "ta_YSize");
      sprintf(string,   "  %s,", TextAttrStyle(&textattrnode->TextAttr));
      st = GenerateString(string, fp, st, "ta_Style");
      sprintf(string,   "  %s", TextAttrFlags(&textattrnode->TextAttr));
      st = GenerateString(string, fp, st, "ta_Flags");
      st = GenerateString("  };", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }
    }
  else
    {
    st = FALSE;
    }

  DEBUGEXIT(TRUE, (ULONG) st);
  return(st);
  }

#endif
