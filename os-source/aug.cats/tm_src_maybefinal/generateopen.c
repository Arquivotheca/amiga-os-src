
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateOpen(FILE *fp, BOOL st)
  {
  int fcount;
  int tabpos;
  BOOL first;
  struct TextAttrNode *textattrnode;

  DEBUGENTER("GenerateOpen", st);

  if(fp && st)
    {
    sprintf(string,     "/****** %s_tm.c/TM_Open *****************************", savefilename);
    st = GenerateString(string, fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NAME", fp, st, NULL);
    st = GenerateString("*\tTM_Open -- initialize user-interface data.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SYNOPSIS", fp, st, NULL);
    st = GenerateString("*\tTMData = TM_Open(ErrorCode)", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*\tstruct TMData *TM_Open(ULONG *error);", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   FUNCTION", fp, st, NULL);
    st = GenerateString("*\tCreates an initializes a TMData structure.  This", fp, st, NULL);
    st = GenerateString("*\tincludes allocating memory for a TMData structure,", fp, st, NULL);
    st = GenerateString("*\tcreating a message port for window IDCMP messages,", fp, st, NULL);
    st = GenerateString("*\tand opening any fonts on disk.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   INPUTS", fp, st, NULL);
    st = GenerateString("*\terror = pointer to ULONG to store an error number.", fp, st, NULL);
    st = GenerateString("*\t        Error numbers are defined in the _tm.h file.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   RESULT", fp, st, NULL);
    st = GenerateString("*\tTMData = pointer to an initialized TMData structure", fp, st, NULL);
    st = GenerateString("*\t         or NULL if an error occurred.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   EXAMPLE", fp, st, NULL);
    st = GenerateString("*\tif(!(TMData = TM_Open(&error)))", fp, st, NULL);
    st = GenerateString("*\t  {", fp, st, NULL);
    st = GenerateString("*\t  switch(error)", fp, st, NULL);
    st = GenerateString("*\t    {", fp, st, NULL);
    st = GenerateString("*\t    case TMERR_MEMORY:", fp, st, NULL);
    st = GenerateString("*\t      TM_Request(NULL, \"Error\", \"Out of memory\", \"Abort\", NULL, NULL);", fp, st, NULL);
    st = GenerateString("*\t      break;", fp, st, NULL);
    st = GenerateString("*\t    case TMERR_MSGPORT:", fp, st, NULL);
    st = GenerateString("*\t      TM_Request(NULL, \"Error\", \"Error creating\\nmessage port\", \"Abort\", NULL, NULL);", fp, st, NULL);
    st = GenerateString("*\t      break;", fp, st, NULL);
    st = GenerateString("*\t    }", fp, st, NULL);
    st = GenerateString("*\t  CleanExit(NULL, RETURN_FAIL);", fp, st, NULL);
    st = GenerateString("*\t  }", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   NOTES", fp, st, NULL);
    st = GenerateString("*\tThe intuition and gadtools libraries must be open before", fp, st, NULL);
    st = GenerateString("*\tcalling this function.  If any fonts other than topaz 8", fp, st, NULL);
    st = GenerateString("*\tand topaz 9 are used, the diskfont and graphics libraries", fp, st, NULL);
    st = GenerateString("*\tmust also be open.", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   BUGS", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("*   SEE ALSO", fp, st, NULL);
    st = GenerateString("*\tTM_Close()", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);

    st = GenerateString("**************************************************************", fp, st, NULL);
    st = GenerateString("*", fp, st, NULL);
    st = GenerateString("*/", fp, st, NULL);

    for(tabpos=0; tabpos<1023; tabpos++) tabstring[tabpos] = ' ';
    tabpos = 0;
    tabstring[tabpos] = '\0';

    st = GenerateString("struct TMData *TM_Open(ULONG *error)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);

    st = GenerateString("  struct TMData *tmdata;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* allocate memory */

    st = GenerateString("  if(tmdata = AllocMem(sizeof(struct TMData), MEMF_CLEAR))", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    tmdata->Size = sizeof(struct TMData);", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    if(arexxflag)
      {
      st = GenerateString("    tmdata->ARexxContext = InitARexx(AREXXNAME, NULL);", fp, st, filename);
      st = GenerateString("    tmdata->ARexxSignal = ARexxSignal(tmdata->ARexxContext);", fp, st, NULL);
      }

    if(!chipflag)
      {
      st = GenerateString("    if(tmdata->WaitPointer = AllocMem(36*sizeof(UWORD), MEMF_CHIP))", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      CopyMem(WaitPointer, tmdata->WaitPointer, 36*sizeof(UWORD));", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      tabstring[tabpos] = ' ';
      tabpos += 2;
      tabstring[tabpos] = '\0';
      }

    /* open message port */

    sprintf(string, "%s    if(tmdata->WindowMsgPort = CreateMsgPort())", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s      {", tabstring);
    st = GenerateString(string, fp, st, NULL);

    tabstring[tabpos] = ' ';
    tabpos += 2;
    tabstring[tabpos] = '\0';

    /* open fonts */

    fcount = 0;
    for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_Head; textattrnode->Node.ln_Succ;
        textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Succ)
      {
      if(!((strcmp(textattrnode->FontName, "topaz.font") == 0) &&
          ((textattrnode->TextAttr.ta_YSize == 8) ||
           (textattrnode->TextAttr.ta_YSize == 9))))
        {
        sprintf(string, "%s    if(tmdata->TextFont[%d] = OpenDiskFont(&%s))", tabstring, fcount, textattrnode->Name);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string, "%s      {", tabstring);
        st = GenerateString(string, fp, st, NULL);

        tabstring[tabpos] = ' ';
        tabpos += 2;
        tabstring[tabpos] = '\0';

        fcount++;
        }
      }

    /* return if no errors */

    sprintf(string, "%s    *error = TMERR_OK;", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s    return(tmdata);", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s    }", tabstring);
    st = GenerateString(string, fp, st, NULL);

    /* close fonts */

    first = TRUE;
    for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_TailPred; textattrnode->Node.ln_Pred;
        textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Pred)
      {
      if(!((strcmp(textattrnode->FontName, "topaz.font") == 0) &&
          ((textattrnode->TextAttr.ta_YSize == 8) ||
           (textattrnode->TextAttr.ta_YSize == 9))))
        {
        fcount--;

        tabstring[tabpos] = ' ';
        tabpos -= 2;
        tabstring[tabpos] = '\0';

        if(first)
          first = FALSE;
        else
          {
          sprintf(string, "%s      CloseFont(tmdata->TextFont[%d]);", tabstring, fcount);
          st = GenerateString(string, fp, st, NULL);
          sprintf(string, "%s      }", tabstring);
          st = GenerateString(string, fp, st, NULL);
          }

        sprintf(string, "%s    else", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string, "%s      {", tabstring);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string, "%s      *error = TMERR_%s;", tabstring, textattrnode->Name);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string, "%s      }", tabstring);
        st = GenerateString(string, fp, st, NULL);
        }
      }

    tabstring[tabpos] = ' ';
    tabpos -= 2;
    tabstring[tabpos] = '\0';

    if(first)
      first = FALSE;
    else
      {
      sprintf(string, "%s      DeleteMsgPort(tmdata->WindowMsgPort);", tabstring);
      st = GenerateString(string, fp, st, NULL);
      sprintf(string, "%s      }", tabstring);
      st = GenerateString(string, fp, st, NULL);
      }

    sprintf(string, "%s    else", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s      {", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s      *error = TMERR_MSGPORT;", tabstring);
    st = GenerateString(string, fp, st, NULL);
    sprintf(string, "%s      }", tabstring);
    st = GenerateString(string, fp, st, NULL);

    if(!chipflag)
      {
      if(first)
        first = FALSE;
      else
        {
        st = GenerateString("      FreeMem(tmdata->WaitPointer, 36*sizeof(UWORD));", fp, st, NULL);
        st = GenerateString("      }", fp, st, NULL);
        st = GenerateString("    else", fp, st, NULL);
        st = GenerateString("      {", fp, st, NULL);
        st = GenerateString("      *error = TMERR_MEMORY;", fp, st, NULL);
        st = GenerateString("      }", fp, st, NULL);
        }
      }

    if(arexxflag)
      {
      st = GenerateString("    FreeARexx(tmdata->ARexxContext);", fp, st, NULL);
      }

    st = GenerateString("    FreeMem(tmdata, sizeof(struct TMData));", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("  else", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    *error = TMERR_MEMORY;", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  return(NULL);", fp, st, NULL);
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
