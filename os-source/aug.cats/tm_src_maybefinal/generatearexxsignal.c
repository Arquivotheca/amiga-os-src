
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateARexxSignal(FILE *fp, BOOL st)
  {
  DEBUGENTER("GenerateARexxSignal", (ULONG) st);

  if(fp && st)
    {
    if(arexxflag)
      {
      st = GenerateString("static BOOL TM_ARexxSignal(struct TMData *TMData)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  BOOL done=FALSE;", fp, st, NULL);
      st = GenerateString("  BOOL result;", fp, st, NULL);
      st = GenerateString("  struct RexxMsg *rexxmsg;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  while(rexxmsg = GetARexxMsg(TMData->ARexxContext))", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TMData->ARexxErrorLevel = 0;", fp, st, NULL);
      st = GenerateString("    TMData->ARexxError = NULL;", fp, st, NULL);
      st = GenerateString("    TMData->ARexxResult = NULL;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("    if(result = ARexxMessage(TMData, rexxmsg)) done = result;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("    if(TMData->ARexxError)", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      SetARexxLastError(TMData->ARexxContext, rexxmsg, TMData->ARexxError);", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      st = GenerateString("    ReplyARexxMsg(TMData->ARexxContext, rexxmsg, TMData->ARexxResult, TMData->ARexxErrorLevel);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  return(done);", fp, st, NULL);
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
