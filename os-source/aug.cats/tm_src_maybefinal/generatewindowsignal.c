
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateWindowSignal(FILE *fp, BOOL st)
  {
  BOOL first=TRUE;
  struct WindowNode *windownode;

  DEBUGENTER("GenerateWindowSignal", (ULONG) st);

  if(fp && st)
    {
    st = GenerateString("static BOOL TM_WindowSignal(struct TMData *TMData)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  BOOL done=FALSE;", fp, st, NULL);
    st = GenerateString("  BOOL result;", fp, st, NULL);
    st = GenerateString("  struct IntuiMessage *imessage;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  while(imessage = GT_GetIMsg(TMData->WindowMsgPort))", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(first)
        {
        sprintf(string,     "    if(imessage->IDCMPWindow == WindowInfo_%s.Window)", windownode->SourceLabel);
        first = FALSE;
        }
      else
        {
        sprintf(string,     "    else if(imessage->IDCMPWindow == WindowInfo_%s.Window)", windownode->SourceLabel);
        }

      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      sprintf(string,     "      WindowInfo_%s.GT_IMsg = imessage;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "      if(result = WindowIDCMP_%s(TMData, imessage)) done = result;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "      if(WindowInfo_%s.GT_IMsg)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("        {", fp, st, NULL);
      sprintf(string,     "        GT_ReplyIMsg(WindowInfo_%s.GT_IMsg);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string,     "        WindowInfo_%s.GT_IMsg = NULL;", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("        }", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      }

    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  return(done);", fp, st, NULL);
    st = GenerateString("  }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    }
  else
    {
    st = FALSE;
    }

  DEBUGEXIT(TRUE, (ULONG) st);
  return(st);
  }

#endif
