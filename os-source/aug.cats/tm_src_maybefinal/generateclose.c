
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateClose(FILE *fp, BOOL st)
  {
  DEBUGENTER("GenerateClose", (ULONG) st);

  if(fp && st)
    {
    sprintf(string,     "/****** %s_tm.c/TM_Close *****************************", savefilename);
    st = GenerateString(string, fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NAME", fp, st, NULL);
    st = GenerateString("*\tTM_Close -- frees user-interface data.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SYNOPSIS", fp, st, NULL);
    st = GenerateString("*\tTM_Close(TMData)", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tVOID TM_Close(struct TMData *TMData);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   FUNCTION", fp, st, NULL);
    st = GenerateString("*\tFrees resources allocated by TM_Open().", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   INPUTS", fp, st, NULL);
    st = GenerateString("*\tTMData = pointer to TMData structure returned by TM_Open.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   RESULT", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   EXAMPLE", fp, st, NULL);
    st = GenerateString("*\tif(TMData) TM_Close(TMData);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NOTES", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   BUGS", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SEE ALSO", fp, st, NULL);
    st = GenerateString("*\tTM_Open()", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("**************************************************************", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*/", fp, st, NULL);

    st = GenerateString("VOID TM_Close(struct TMData *TMData)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);

    if(diskfontcount)
      {
      st = GenerateString("  int fontcount;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    st = GenerateString("  if(TMData)", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);

    if(diskfontcount)
      {
      sprintf(string,     "    for(fontcount=0; fontcount<%d; fontcount++)", diskfontcount);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      if(TMData->TextFont[fontcount]) CloseFont(TMData->TextFont[fontcount]);", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    st = GenerateString("    if(TMData->WindowMsgPort) DeleteMsgPort(TMData->WindowMsgPort);", fp, st, NULL);

    if(arexxflag)
      {
      st = GenerateString("    FreeARexx(TMData->ARexxContext);", fp, st, NULL);
      }

    if(!chipflag)
      {
      st = GenerateString("    if(TMData->WaitPointer) FreeMem(TMData->WaitPointer, 36*sizeof(UWORD));", fp, st, NULL);
      }

    st = GenerateString("    FreeMem(TMData, TMData->Size);", fp, st, NULL);
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
