
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateRequest(FILE *fp, BOOL st)
  {
  DEBUGENTER("GenerateRequest", (ULONG) st);

  if(fp && st)
    {
    sprintf(string,     "/****** %s_tm.c/TM_Request *****************************", savefilename);
    st = GenerateString(string, fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NAME", fp, st, NULL);
    st = GenerateString("*\tTM_Request -- display a message in a system requester.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SYNOPSIS", fp, st, NULL);
    st = GenerateString("*\tnum = TM_Request(Window, Title, TextFormat, GadgetFormat,", fp, st, NULL);
    st = GenerateString("*\t                 IDCMP_ptr, Arg1, Arg2, ...)", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tLONG TM_Request(struct Window *, UBYTE *, UBYTE *, UBYTE *,", fp, st, NULL);
    st = GenerateString("*\t                ULONG *, APTR, ...);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   FUNCTION", fp, st, NULL);
    st = GenerateString("*\tCalls EasyRequestArgs to display a message in a system", fp, st, NULL);
    st = GenerateString("*\trequester without having to fill in an EasyStruct structure.", fp, st, NULL);
    st = GenerateString("*\tThis function uses a varargs calling convention.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tEasyRequestArgs has the capability to display more than one", fp, st, NULL);
    st = GenerateString("*\tgadget by separating the GadgetText with the '|' character.", fp, st, NULL);
    st = GenerateString("*\tAlso printf style formatting may be used in the TextFormat", fp, st, NULL);
    st = GenerateString("*\tand GadgetFormat strings.  The formatting arguments should", fp, st, NULL);
    st = GenerateString("*\tbe added first for the TextFormat, then for the GadgetFormat.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   INPUTS", fp, st, NULL);
    st = GenerateString("*\tWindow = pointer to window (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*\tTitle = title bar string (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*\tTextFormat = body text (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*\tGadgetFormat = gadget text (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*\tIDCMP_ptr = IDCMP value (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*\tArgs = arguments (passed to EasyRequestArgs()).", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   RESULT", fp, st, NULL);
    st = GenerateString("*\tnum = gadget number returned by EasyRequestArgs().", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   EXAMPLE", fp, st, NULL);
    st = GenerateString("*\tTo display a requester with two choices:", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tresult = TM_Request(NULL, \"Question\", \"Are you sure?\",", fp, st, NULL);
    st = GenerateString("*\t                    \"OK|Cancel\", NULL, NULL);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NOTES", fp, st, NULL);
    st = GenerateString("*\tThe intuition library must be open before calling this", fp, st, NULL);
    st = GenerateString("*\tfunction.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   BUGS", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SEE ALSO", fp, st, NULL);
    st = GenerateString("*\tintuition.library/EasyRequestArgs()", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("**************************************************************", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*/", fp, st, NULL);

    st = GenerateString("LONG TM_Request(struct Window *Window, UBYTE *Title, UBYTE *TextFormat, UBYTE *GadgetFormat, ULONG *IDCMP_ptr, APTR Arg1, ...)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  struct EasyStruct es;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  es.es_StructSize = sizeof(struct EasyStruct);", fp, st, NULL);
    st = GenerateString("  es.es_Flags = 0;", fp, st, NULL);
    st = GenerateString("  es.es_Title = Title;", fp, st, NULL);
    st = GenerateString("  es.es_TextFormat = TextFormat;", fp, st, NULL);
    st = GenerateString("  es.es_GadgetFormat = GadgetFormat;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  return(EasyRequestArgs(Window, &es, IDCMP_ptr, &Arg1));", fp, st, NULL);
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
