
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateEventLoop(FILE *fp, BOOL st)
  {
  DEBUGENTER("GenerateEventLoop", (ULONG) st);

  if(fp && st)
    {
    sprintf(string,     "/****** %s_tm.c/TM_EventLoop *****************************", savefilename);
    st = GenerateString(string, fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NAME", fp, st, NULL);
    st = GenerateString("*\tTM_EventLoop -- waits for all events.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SYNOPSIS", fp, st, NULL);
    st = GenerateString("*\tTM_EventLoop(TMData)", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tVOID TM_EventLoop(struct TMData *);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   FUNCTION", fp, st, NULL);
    st = GenerateString("*\tWaits for any events to signal program.  All IDCMP, ARexx,", fp, st, NULL);
    st = GenerateString("*\tand a user signal messages received are sent to the", fp, st, NULL);
    st = GenerateString("*\tappropriate signal function.  A return value of TRUE from", fp, st, NULL);
    st = GenerateString("*\tany of the signal functions will terminate the loop.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   INPUTS", fp, st, NULL);
    st = GenerateString("*\tTMData = pointer to TMData structure returned by TM_Open.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   RESULT", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   EXAMPLE", fp, st, NULL);
    st = GenerateString("*\tTM_EventLoop(TMData);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NOTES", fp, st, NULL);
    st = GenerateString("*\tAn event signal must be set up before calling TM_EventLoop.", fp, st, NULL);
    st = GenerateString("*\tEither a window must be open, the SimpleRexx option selected,", fp, st, NULL);
    st = GenerateString("*\tor a user signal set up.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   BUGS", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SEE ALSO", fp, st, NULL);
    st = GenerateString("*\tTM_WindowSignal(), TM_ARexxSignal(), exec.library/Wait()", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("**************************************************************", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*/", fp, st, NULL);

    st = GenerateString("VOID TM_EventLoop(struct TMData *TMData)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  BOOL done=FALSE, result;", fp, st, NULL);
    st = GenerateString("  ULONG windowsignal, signals;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  windowsignal = 1L << TMData->WindowMsgPort->mp_SigBit;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  while(!done)", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);

    strcpy(string, "    signals = Wait(windowsignal");

    if(arexxflag)
      {
      strcat(string, " | TMData->ARexxSignal");
      }

    if(usersignalflag)
      {
      strcat(string, " | TMData->UserSignal");
      }

    strcat(string, ");");
    st = GenerateString(string, fp, st, NULL);

    st = GenerateString("", fp, st, NULL);

    st = GenerateString("    if(signals & windowsignal)", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      if(result = TM_WindowSignal(TMData)) done = result;", fp, st, NULL);
    st = GenerateString("      }", fp, st, NULL);

    if(arexxflag)
      {
      st = GenerateString("    if(signals & TMData->ARexxSignal)", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      if(result = TM_ARexxSignal(TMData)) done = result;", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      }

    if(usersignalflag)
      {
      st = GenerateString("    if(signals & TMData->UserSignal)", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      if(result = UserSignal(TMData, signals)) done = result;", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      }

    st = GenerateString("    }", fp, st, NULL);
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
