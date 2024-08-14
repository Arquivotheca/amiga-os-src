
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateWindow(FILE *fp, BOOL st)
  {
  static char fontname[STRINGSIZE];
  int    gcount, firstgad;
  char   *kind;
  char   tabstring[5];
  struct NewGadget     ng;
  struct WindowNode    *windownode;
  struct TagNode       *tagnode;
  struct NewGadgetNode *newgadgetnode;
  struct NewGadgetNode *lastgadgetnode;
  struct AvailableTags *availabletags;

  DEBUGENTER("GenerateWindow", (ULONG) st);

  if(fp && st)
    {
    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      sprintf(string,     "/****** %s_tm.c/OpenWindow_%s *****************************", savefilename, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tOpenWindow_%s -- Open window \"%s\".", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tsuccess = OpenWindow_%s(TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tBOOL OpenWindow_%s(struct TMData *);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tOpens the window with label \"%s\"", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\talong with any gadgets it contains.  If the window was already", fp, st, NULL);
      st = GenerateString("*\topen, it will be brought to front.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*\tsuccess = TRUE if command was successfully completed.", fp, st, NULL);
      st = GenerateString("*\t          FALSE if not.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      sprintf(string,     "*\tif(!OpenWindow_%s(TMData))", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\t  {", fp, st, NULL);
      st = GenerateString("*\t  TMRequest(NULL, \"Error\", \"Error opening window\", \"Abort\", NULL, NULL);", fp, st, NULL);
      st = GenerateString("*\t  CleanExit(TMData, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("*\t  }", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tCloseWindow_%s()", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string, "BOOL OpenWindow_%s(struct TMData *TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  ULONG lasttag;", fp, st, NULL);

      gcount = 0;
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        gcount++;
        }

      if(gcount)
        {
        st = GenerateString("  struct NewGadget ng;", fp, st, NULL);
        }

      st = GenerateString("", fp, st, NULL);
      sprintf(string,     "  if(WindowInfo_%s.Window)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    WindowToFront(WindowInfo_%s.Window);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    ActivateWindow(WindowInfo_%s.Window);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    return(TRUE);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  else", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);

      if(gcount)
        {
        sprintf(string,   "    WindowInfo_%s.FirstGadget = NULL;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "    WindowInfo_%s.ContextGadget = CreateContext(&WindowInfo_%s.FirstGadget);", windownode->SourceLabel, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("", fp, st, NULL);

        if(screennode.Workbench)
          strcpy(string,  "    ng.ng_VisualInfo = ScreenInfo_Workbench.VisualInfo;");
        else
          sprintf(string, "    ng.ng_VisualInfo = ScreenInfo_%s.VisualInfo;", screennode.SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      gcount = 0;
      firstgad = TRUE;
      strcpy(fontname, "");
      for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
          newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
        {
        if(firstgad || newgadgetnode->NewGadget.ng_LeftEdge != ng.ng_LeftEdge)
          {
          if(windownode->TMWFlags & WINDOWFLAG_GZZ)
            {
            sprintf(string, "    ng.ng_LeftEdge = %d;", newgadgetnode->NewGadget.ng_LeftEdge - windownode->Window->BorderLeft);
            }
          else
            {
            sprintf(string, "    ng.ng_LeftEdge = %d;", newgadgetnode->NewGadget.ng_LeftEdge);
            }
          st = GenerateString(string, fp, st, NULL);
          ng.ng_LeftEdge = newgadgetnode->NewGadget.ng_LeftEdge;
          }

        if(firstgad || newgadgetnode->NewGadget.ng_TopEdge != ng.ng_TopEdge)
          {
          if(windownode->TMWFlags & WINDOWFLAG_GZZ)
            {
            sprintf(string, "    ng.ng_TopEdge = %d;", newgadgetnode->NewGadget.ng_TopEdge - currenttopborder);
            }
          else
            {
            if(autotopborderflag)
              {
              if(screennode.Workbench)
                sprintf(string, "    ng.ng_TopEdge = %d+ScreenInfo_Workbench.Screen->BarHeight;", newgadgetnode->NewGadget.ng_TopEdge - currenttopborder + 1);
              else
                sprintf(string, "    ng.ng_TopEdge = %d+ScreenInfo_%s.Screen->BarHeight;", newgadgetnode->NewGadget.ng_TopEdge - currenttopborder + 1, screennode.SourceLabel);
              }
            else
              {
              sprintf(string, "    ng.ng_TopEdge = %d;", newgadgetnode->NewGadget.ng_TopEdge);
              }
            }
          st = GenerateString(string, fp, st, NULL);
          ng.ng_TopEdge = newgadgetnode->NewGadget.ng_TopEdge;
          }

        if(firstgad || newgadgetnode->NewGadget.ng_Width != ng.ng_Width)
          {
          sprintf(string, "    ng.ng_Width = %d;", newgadgetnode->NewGadget.ng_Width);
          st = GenerateString(string, fp, st, NULL);
          ng.ng_Width = newgadgetnode->NewGadget.ng_Width;
          }

        if(firstgad || newgadgetnode->NewGadget.ng_Height != ng.ng_Height)
          {
          sprintf(string, "    ng.ng_Height = %d;", newgadgetnode->NewGadget.ng_Height);
          st = GenerateString(string, fp, st, NULL);
          ng.ng_Height = newgadgetnode->NewGadget.ng_Height;
          }

        if(firstgad || newgadgetnode->NewGadget.ng_Flags != ng.ng_Flags)
          {
          sprintf(string, "    ng.ng_Flags = %s;", NGFlags2String(newgadgetnode->NewGadget.ng_Flags));
          st = GenerateString(string, fp, st, NULL);
          ng.ng_Flags = newgadgetnode->NewGadget.ng_Flags;
          }

        if(firstgad || strcmp(TextAttrName(&newgadgetnode->TextAttr), fontname))
          {
          sprintf(string, "    ng.ng_TextAttr = &%s;", TextAttrName(&newgadgetnode->TextAttr));
          st = GenerateString(string, fp, st, NULL);
          strcpy(fontname, TextAttrName(&newgadgetnode->TextAttr));
          }

        sprintf(string,   "    ng.ng_GadgetText = (UBYTE *)GADGETTEXT_%s;", newgadgetnode->SourceLabel);
        st = GenerateString(string, fp, st, newgadgetnode->NewGadget.ng_GadgetText);

        if(newgadgetnode->Kind == TEXT_KIND ||
           newgadgetnode->Kind == NUMBER_KIND)
          {
          st = GenerateString("    ng.ng_GadgetID = 0;", fp, st, NULL);
          }
        else
          {
          sprintf(string, "    ng.ng_GadgetID = ID_%s;", newgadgetnode->SourceLabel);
          st = GenerateString(string, fp, st, NULL);
          }

        if(userdataflag)
          {
          if(newgadgetnode->Kind == TEXT_KIND ||
             newgadgetnode->Kind == NUMBER_KIND)
            {
            st = GenerateString("    ng.ng_UserData = NULL;", fp, st, NULL);
            }
          else
            {
            sprintf(string, "    ng.ng_UserData = %s;", UserDataName(newgadgetnode->SourceLabel));
            st = GenerateString(string, fp, st, NULL);
            }
          }

        switch(newgadgetnode->Kind)
          {
          case BUTTON_KIND:
            kind = "BUTTON_KIND";
            availabletags = &AvailableBUGadgetTags[0];
            break;
          case CHECKBOX_KIND:
            kind = "CHECKBOX_KIND";
            availabletags = &AvailableCBGadgetTags[0];
            break;
          case CYCLE_KIND:
            kind = "CYCLE_KIND";
            availabletags = &AvailableCYGadgetTags[0];
            break;
          case INTEGER_KIND:
            kind = "INTEGER_KIND";
            availabletags = &AvailableINGadgetTags[0];
            break;
          case LISTVIEW_KIND:
            kind = "LISTVIEW_KIND";
            availabletags = &AvailableLVGadgetTags[0];
            break;
          case MX_KIND:
            kind = "MX_KIND";
            availabletags = &AvailableMXGadgetTags[0];
            break;
          case NUMBER_KIND:
            kind = "NUMBER_KIND";
            availabletags = &AvailableNMGadgetTags[0];
            break;
          case PALETTE_KIND:
            kind = "PALETTE_KIND";
            availabletags = &AvailablePAGadgetTags[0];
            break;
          case SCROLLER_KIND:
            kind = "SCROLLER_KIND";
            availabletags = &AvailableSCGadgetTags[0];
            break;
          case SLIDER_KIND:
            kind = "SLIDER_KIND";
            availabletags = &AvailableSLGadgetTags[0];
            break;
          case STRING_KIND:
            kind = "STRING_KIND";
            availabletags = &AvailableSTGadgetTags[0];
            break;
          case TEXT_KIND:
            kind = "TEXT_KIND";
            availabletags = &AvailableTXGadgetTags[0];
            break;
          }

        sprintf(string,   "    lasttag = GadgetInfo_%s.MoreTags ? TAG_MORE : TAG_DONE;", newgadgetnode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);

        if(firstgad)
          {
          sprintf(string, "    GadgetInfo_%s.Gadget = CreateGadget(%s, WindowInfo_%s.ContextGadget, &ng,", newgadgetnode->SourceLabel, kind, windownode->SourceLabel);
          st = GenerateString(string, fp, st, NULL);
          }
        else
          {
          sprintf(string, "    GadgetInfo_%s.Gadget = CreateGadget(%s, GadgetInfo_%s.Gadget, &ng,", newgadgetnode->SourceLabel, kind, ((struct NewGadgetNode *)newgadgetnode->Node.ln_Pred)->SourceLabel);
          st = GenerateString(string, fp, st, NULL);
          }

        /* Gadget tag labels */

        for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
            tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
          {
          switch(availabletags[tagnode->Node.ln_Pri].Type)
            {
            case TYPE_CHARACTER:
              sprintf(string, "\t\t%s, '%c',", tagnode->Node.ln_Name, tagnode->TagItem.ti_Data);
              st = GenerateString(string, fp, st, NULL);
              break;

            case TYPE_INTEGER:
              sprintf(string, "\t\t%s, %s,", tagnode->Node.ln_Name, tagnode->Data);
              st = GenerateString(string, fp, st, NULL);
              break;

            case TYPE_WORDLIST:
            case TYPE_STRINGLIST:
            case TYPE_STRING:
              sprintf(string, "\t\t%s, %s_%s,", tagnode->Node.ln_Name, availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              break;

            case TYPE_LINKEDLIST:
              sprintf(string, "\t\t%s, &%s_%s,", tagnode->Node.ln_Name, availabletags[tagnode->Node.ln_Pri].Name, newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              break;

            case TYPE_IGNORE:
              sprintf(string, "\t\t%s, %s,", tagnode->Node.ln_Name, tagnode->Data);
              st = GenerateString(string, fp, st, NULL);
              break;
            }
          }

        sprintf(string, "\t\tlasttag, GadgetInfo_%s.MoreTags);", newgadgetnode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("", fp, st, NULL);

        firstgad = FALSE;
        lastgadgetnode = newgadgetnode;
        gcount++;
        }

      /****** Check Gadgets ******/

      if(gcount)
        {
        sprintf(string,     "    if(GadgetInfo_%s.Gadget)", lastgadgetnode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      {", fp, st, NULL);
        strcpy(tabstring, "  ");
        }
      else
        {
        strcpy(tabstring, "");
        }

      /****** Open Window ******/

      sprintf(string,     "%s    lasttag = WindowInfo_%s.MoreTags ? TAG_MORE : TAG_DONE;", tabstring, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      st = GenerateString("", fp, st, NULL);
      sprintf(string,     "%s    if((WindowInfo_%s.Window = OpenWindowTags(NULL,", tabstring, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      if(screennode.Workbench)
        strcpy(string,  "\t\tWA_PubScreen, ScreenInfo_Workbench.Screen,");
      else
        sprintf(string, "\t\tWA_PubScreen, ScreenInfo_%s.Screen,", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      if(windownode->Title[0] != '\0')
        {
        sprintf(string, "\t\tWA_Title, WINDOWTEXT_%s,", windownode->SourceLabel);
        st = GenerateString(string, fp, st, windownode->Title);
        }

      st = GenerateString("\t\tWA_IDCMP, NULL,", fp, st, NULL);

      if(gcount)
        {
        sprintf(string, "\t\tWA_Gadgets, WindowInfo_%s.FirstGadget,", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        }

      /* Window tag labels */

      for(tagnode = (struct TagNode *) windownode->TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(AvailableWindowTags[tagnode->Node.ln_Pri].Type)
          {
          case TYPE_VOID:
          case TYPE_LINKEDLIST:
            break;

          case TYPE_CHARACTER:
            sprintf(string, "\t\t%s, '%c',", tagnode->Node.ln_Name, tagnode->TagItem.ti_Data);
            st = GenerateString(string, fp, st, NULL);
            break;

          case TYPE_INTEGER:
            sprintf(string, "\t\t%s, %s,", tagnode->Node.ln_Name, tagnode->Data);
            st = GenerateString(string, fp, st, NULL);
            break;

          case TYPE_WORDLIST:
          case TYPE_STRINGLIST:
          case TYPE_STRING:
            sprintf(string, "\t\t%s, %s_%s,", tagnode->Node.ln_Name, AvailableWindowTags[tagnode->Node.ln_Pri].Name, windownode->SourceLabel);
            st = GenerateString(string, fp, st, NULL);
            break;

          case TYPE_IGNORE:
            sprintf(string, "\t\t%s, %s,", tagnode->Node.ln_Name, tagnode->Data);
            st = GenerateString(string, fp, st, NULL);
            break;
          }
        }

      sprintf(string, "\t\tlasttag, WindowInfo_%s.MoreTags)))", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string, "%s      {", tabstring);
      st = GenerateString(string, fp, st, NULL);

      /****** Create Menus ******/

      if(windownode->Menu)
        {
        sprintf(string,   "%s      if((WindowInfo_%s.Menu = CreateMenus(newmenu_%s, TAG_DONE)))", tabstring, windownode->SourceLabel, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s        {", tabstring);
        st = GenerateString(string, fp, st, NULL);

        if(screennode.Workbench)
          sprintf(string, "%s        if((LayoutMenus(WindowInfo_%s.Menu, ScreenInfo_Workbench.VisualInfo, TAG_DONE)))", tabstring, windownode->SourceLabel);
        else
          sprintf(string, "%s        if((LayoutMenus(WindowInfo_%s.Menu, ScreenInfo_%s.VisualInfo, TAG_DONE)))", tabstring, windownode->SourceLabel, screennode.SourceLabel);
        st = GenerateString(string, fp, st, NULL);

        sprintf(string,   "%s          {", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s          if((SetMenuStrip(WindowInfo_%s.Window, WindowInfo_%s.Menu)))", tabstring, windownode->SourceLabel, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            {", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            WindowInfo_%s.Window->UserPort = TMData->WindowMsgPort;", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            ModifyIDCMP(WindowInfo_%s.Window,\n\t\t\tIDCMP_REFRESHWINDOW%s%s);", tabstring, windownode->SourceLabel, GadgetIDCMP(windownode), IDCMP2String(windownode->IDCMP));
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            GT_RefreshWindow(WindowInfo_%s.Window, NULL);", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            WindowInfo_%s.Flags |= TMWF_OPENED;", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            return(TRUE);", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s            }", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s          }", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s        FreeMenus(WindowInfo_%s.Menu);", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s        }", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s      CloseWindow(WindowInfo_%s.Window);", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        }
      else
        {
        sprintf(string,   "%s      WindowInfo_%s.Window->UserPort = TMData->WindowMsgPort;", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s      ModifyIDCMP(WindowInfo_%s.Window,\n\t\t\tIDCMP_REFRESHWINDOW%s%s);", tabstring, windownode->SourceLabel, GadgetIDCMP(windownode), IDCMP2String(windownode->IDCMP));
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s      GT_RefreshWindow(WindowInfo_%s.Window, NULL);", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s      WindowInfo_%s.Flags |= TMWF_OPENED;", tabstring, windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,   "%s      return(TRUE);", tabstring);
        st = GenerateString(string, fp, st, NULL);
        }

      sprintf(string,     "%s      }", tabstring);
      st = GenerateString(string, fp, st, NULL);

      if(gcount)
        {
        sprintf(string,     "      FreeGadgets(WindowInfo_%s.FirstGadget);", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      }", fp, st, NULL);
        }

      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  return(FALSE);", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      sprintf(string,     "/****** %s_tm.c/CloseWindow_%s *****************************", savefilename, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tCloseWindow_%s -- Close window \"%s\".", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tCloseWindow_%s(TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tVOID CloseWindow_%s(struct TMData *);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tCloses the window with label \"%s\"", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\tand frees its resources.  It's ok to re-close a window", fp, st, NULL);
      st = GenerateString("*\tthat is already closed.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tOpenWindow_%s()", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string, "VOID CloseWindow_%s(struct TMData *TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      st = GenerateString("  {", fp, st, NULL);
      sprintf(string,     "  TM_RemoveWindow(&WindowInfo_%s);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      sprintf(string,     "/****** %s_tm.c/DisableWindow_%s *****************************", savefilename, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tDisableWindow_%s -- disable input to window \"%s\".", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tDisableWindow_%s(TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tVOID DisableWindow_%s(struct TMData *);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tDisables all input to window with label \"%s\"", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\tand changes its mouse pointer.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tEnableWindow_%s()", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string, "VOID DisableWindow_%s(struct TMData *TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      sprintf(string,     "  if((WindowInfo_%s.DisableCount == 0) && (WindowInfo_%s.Flags & TMWF_OPENED))", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    InitRequester(&WindowInfo_%s.Requester);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    if(Request(&WindowInfo_%s.Requester, WindowInfo_%s.Window))", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      sprintf(string,     "      WindowInfo_%s.Flags |= TMWF_DISABLED;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      if(chipflag)
        sprintf(string,     "      SetPointer(WindowInfo_%s.Window, WaitPointer, 16, 16, -6, 0);", windownode->SourceLabel);
      else
        sprintf(string,     "      SetPointer(WindowInfo_%s.Window, TMData->WaitPointer, 16, 16, -6, 0);", windownode->SourceLabel);

      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      sprintf(string,     "  WindowInfo_%s.DisableCount++;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      sprintf(string,     "/****** %s_tm.c/EnableWindow_%s *****************************", savefilename, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tEnableWindow_%s -- enable input to window \"%s\".", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tEnableWindow_%s(TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tVOID EnableWindow_%s(struct TMData *);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tEnables input to window with label \"%s\"", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\tand clears its mouse pointer.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tDisableWindow_%s()", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string, "VOID EnableWindow_%s(struct TMData *TMData)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      sprintf(string,     "  WindowInfo_%s.DisableCount--;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "  if((WindowInfo_%s.DisableCount == 0) && (WindowInfo_%s.Flags & (TMWF_DISABLED | TMWF_OPENED)))", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    EndRequest(&WindowInfo_%s.Requester, WindowInfo_%s.Window);", windownode->SourceLabel, windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    WindowInfo_%s.Flags &= ~TMWF_DISABLED;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    ClearPointer(WindowInfo_%s.Window);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
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
