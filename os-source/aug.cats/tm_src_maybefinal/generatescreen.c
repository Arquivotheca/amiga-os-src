
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateScreen(FILE *fp, BOOL st)
  {
  struct TagNode   *tagnode;

  DEBUGENTER("GenerateScreen", (ULONG) st);

  if(fp && st)
    {
    if(screennode.Workbench)
      {
      sprintf(string,     "/****** %s_tm.c/OpenScreen_Workbench *****************************", savefilename);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      st = GenerateString("*\tOpenScreen_Workbench -- Use the default public screen.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      st = GenerateString("*\tsuccess = OpenScreen_Workbench(TMData)", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*\tBOOL OpenScreen_Workbench(struct TMData *);", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      st = GenerateString("*\tLocks the default public screen and gets its visual info.  If the", fp, st, NULL);
      st = GenerateString("*\tscreen was already locked, it will be brought to front.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*\tsuccess = TRUE if command was successfully completed.", fp, st, NULL);
      st = GenerateString("*\t          FALSE if not.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      st = GenerateString("*\tif(!(OpenScreen_Workbench(TMData)))", fp, st, NULL);
      st = GenerateString("*\t  {", fp, st, NULL);
      st = GenerateString("*\t  TM_Request(NULL, \"Error\", \"Error opening screen\", \"Abort\", NULL, NULL);", fp, st, NULL);
      st = GenerateString("*\t  CleanExit(TMData, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("*\t  }", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      st = GenerateString("*\tCloseScreen_Workbench()", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      st = GenerateString("BOOL OpenScreen_Workbench(struct TMData *TMData)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  if(ScreenInfo_Workbench.Screen)", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    ScreenToFront(ScreenInfo_Workbench.Screen);", fp, st, NULL);
      st = GenerateString("    return(TRUE);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  else", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    if(ScreenInfo_Workbench.Screen = LockPubScreen(NULL))", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      if(ScreenInfo_Workbench.VisualInfo = GetVisualInfo(ScreenInfo_Workbench.Screen, TAG_DONE))", fp, st, NULL);
      st = GenerateString("        {", fp, st, NULL);
      st = GenerateString("        return(TRUE);", fp, st, NULL);
      st = GenerateString("        }", fp, st, NULL);
      st = GenerateString("      UnlockPubScreen(NULL, ScreenInfo_Workbench.Screen);", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  return(FALSE);", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);



      sprintf(string,     "/****** %s_tm.c/CloseScreen_Workbench *****************************", savefilename);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      st = GenerateString("*\tCloseScreen_Workbench -- Free the default public screen.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      st = GenerateString("*\tCloseScreen_Workbench(TMData)", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*\tVOID CloseScreen_Workbench(struct TMData *);", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      st = GenerateString("*\tUnlocks the default public screen and frees the visual info.", fp, st, NULL);
      st = GenerateString("*\tIt's ok to re-close a screen that is already closed.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      st = GenerateString("*\tCloseScreen_Workbench(TMData);", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      st = GenerateString("*\tOpenScreen_Workbench()", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      st = GenerateString("VOID CloseScreen_Workbench(struct TMData *TMData)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  if(ScreenInfo_Workbench.VisualInfo)", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    FreeVisualInfo(ScreenInfo_Workbench.VisualInfo);", fp, st, NULL);
      st = GenerateString("    ScreenInfo_Workbench.VisualInfo = NULL;", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  if(ScreenInfo_Workbench.Screen)", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    UnlockPubScreen(NULL, ScreenInfo_Workbench.Screen);", fp, st, NULL);
      st = GenerateString("    ScreenInfo_Workbench.Screen = NULL;", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }
    else
      {
      sprintf(string,     "/****** %s_tm.c/OpenScreen_%s *****************************", savefilename, screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tOpenScreen_%s -- Open custom screen \"%s\".", screennode.SourceLabel, screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tsuccess = OpenScreen_%s(TMData)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tBOOL OpenScreen_%s(struct TMData *);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tCreates and displays the custom screen labeled \"%s\"", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\tand gets its visual info.  If the screen was already open", fp, st, NULL);
      st = GenerateString("*\tit will be brought to front.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*\tsuccess = TRUE if command was successfully completed.", fp, st, NULL);
      st = GenerateString("*\t          FALSE if not.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      sprintf(string,     "*\tif(!(OpenScreen_%s(TMData)))", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\t  {", fp, st, NULL);
      st = GenerateString("*\t  TM_Request(NULL, \"Error\", \"Error opening screen\", \"Abort\", NULL, NULL);", fp, st, NULL);
      st = GenerateString("*\t  CleanExit(TMData, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("*\t  }", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tCloseScreen_%s()", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string,     "BOOL OpenScreen_%s(struct TMData *TMData)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  ULONG lasttag;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      sprintf(string,     "  if(ScreenInfo_%s.Screen)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    ScreenToFront(ScreenInfo_%s.Screen);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    return(TRUE);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  else", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);

      sprintf(string,     "    lasttag = ScreenInfo_%s.MoreTags ? TAG_MORE : TAG_DONE;", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      sprintf(string,     "    if(ScreenInfo_%s.Screen = OpenScreenTags(NULL,", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      sprintf(string, "\t\tSA_Title, SCREENTEXT_%s,", screennode.SourceLabel);
      st = GenerateString(string, fp, st, screennode.Title);
      sprintf(string, "\t\tSA_DisplayID, %s,", Mode2String(screennode.Mode));
      st = GenerateString(string, fp, st, NULL);
      sprintf(string, "\t\tSA_Overscan, %s,", Overscan2String(screennode.Overscan));
      st = GenerateString(string, fp, st, NULL);

      if(screennode.Mode & MODE_CUSTOMCOLORS)
        {
        st = GenerateString("\t\tSA_Colors, ColorSpec,", fp, st, NULL);
        }

      if(!(screennode.Mode & MODE_DEFAULTWIDTH))
        {
        sprintf(string, "\t\tSA_Width, %d,", screennode.Width);
        st = GenerateString(string, fp, st, NULL);
        }

      if(!(screennode.Mode & MODE_DEFAULTHEIGHT))
        {
        sprintf(string, "\t\tSA_Height, %d,", screennode.Height);
        st = GenerateString(string, fp, st, NULL);
        }

      sprintf(string, "\t\tSA_Depth, %d,", screennode.Depth);
      st = GenerateString(string, fp, st, NULL);

      if(screennode.FontName[0] != '\0' &&
         screennode.FontName[0] != '.')
        {
        sprintf(string, "\t\tSA_Font, &%s,", TextAttrName(&screennode.TextAttr));
        st = GenerateString(string, fp, st, NULL);
        }

      /* Screen tag labels */

      for(tagnode = (struct TagNode *) screennode.TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(AvailableScreenTags[tagnode->Node.ln_Pri].Type)
          {
          case TYPE_CHARACTER:
            sprintf(string, "\t\t%s, '%c',", tagnode->Node.ln_Name, tagnode->TagItem.ti_Data);
            st = GenerateString(string, fp, st, NULL);
            break;
          case TYPE_INTEGER:
          case TYPE_IGNORE:
            sprintf(string, "\t\t%s, %s,", tagnode->Node.ln_Name, tagnode->Data);
            st = GenerateString(string, fp, st, NULL);
            break;
          case TYPE_WORDLIST:
          case TYPE_STRINGLIST:
          case TYPE_STRING:
            sprintf(string, "\t\t%s, %s_%s,", tagnode->Node.ln_Name, AvailableScreenTags[tagnode->Node.ln_Pri].Name, screennode.SourceLabel);
            st = GenerateString(string, fp, st, NULL);
            break;
          }
        }
      sprintf(string,       "\t\tlasttag, ScreenInfo_%s.MoreTags))", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      st = GenerateString(  "      {", fp, st, NULL);
      sprintf(string,       "      if(ScreenInfo_%s.VisualInfo = GetVisualInfo(ScreenInfo_%s.Screen, TAG_DONE))", screennode.SourceLabel, screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString(  "        {", fp, st, NULL);
      st = GenerateString(  "        return(TRUE);", fp, st, NULL);
      st = GenerateString(  "        }", fp, st, NULL);
      sprintf(string,       "      CloseScreen(ScreenInfo_%s.Screen);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString(  "      }", fp, st, NULL);
      st = GenerateString(  "    }", fp, st, NULL);
      st = GenerateString(  "", fp, st, NULL);
      st = GenerateString(  "  return(FALSE);", fp, st, NULL);
      st = GenerateString(  "  }", fp, st, NULL);
      st = GenerateString(  "", fp, st, NULL);



      sprintf(string,     "/****** %s_tm.c/CloseScreen_%s *****************************", savefilename, screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NAME", fp, st, NULL);
      sprintf(string,     "*\tCloseScreen_%s -- Close custom screen \"%s\".", screennode.SourceLabel, screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SYNOPSIS", fp, st, NULL);
      sprintf(string,     "*\tCloseScreen_%s(TMData)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      sprintf(string,     "*\tVOID CloseScreen_%s(struct TMData *);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   FUNCTION", fp, st, NULL);
      sprintf(string,     "*\tCloses the custom screen labeled \"%s\"", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*\tand frees the visual info.  It's ok to re-close a screen", fp, st, NULL);
      st = GenerateString("*\tthat is already closed.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   INPUTS", fp, st, NULL);
      st = GenerateString("*\tTMData = pointer to the TMData structure returned by TM_Open.", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   RESULT", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   EXAMPLE", fp, st, NULL);
      sprintf(string,     "*\tCloseScreen_%s(TMData);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   NOTES", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   BUGS", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("*   SEE ALSO", fp, st, NULL);
      sprintf(string,     "*\tOpenScreen_%s()", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);

      st = GenerateString("**************************************************************", fp, st, NULL);
      st = GenerateString("*", fp, st, NULL);
      st = GenerateString("*/", fp, st, NULL);

      sprintf(string,     "VOID CloseScreen_%s(struct TMData *TMData)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      sprintf(string,     "  if(ScreenInfo_%s.VisualInfo)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    FreeVisualInfo(ScreenInfo_%s.VisualInfo);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    ScreenInfo_%s.VisualInfo = NULL;", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      sprintf(string,     "  if(ScreenInfo_%s.Screen)", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      sprintf(string,     "    CloseScreen(ScreenInfo_%s.Screen);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "    ScreenInfo_%s.Screen = NULL;", screennode.SourceLabel);
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
