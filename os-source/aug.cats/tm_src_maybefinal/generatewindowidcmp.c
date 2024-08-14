
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateWindowIDCMP(FILE *fp, BOOL st)
  {
  struct WindowNode    *windownode;

  DEBUGENTER("GenerateWindowIDCMP", (ULONG) st);

  if(fp && st)
    {
    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      sprintf(string, "static BOOL WindowIDCMP_%s(struct TMData *TMData, struct IntuiMessage *imessage)", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  BOOL done=FALSE;", fp, st, NULL);
      st = GenerateString("  BOOL result;", fp, st, NULL);
/*
      if(windownode->IDCMP & IDCMP_MENUPICK)
        {
        st = GenerateString("  ULONG code;", fp, st, NULL);
        }
*/
      st = GenerateString("", fp, st, NULL);

/*
      if(windownode->IDCMP & IDCMP_MENUPICK)
        {
        st = GenerateString("  code = imessage->Code;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }
*/

      st = GenerateString("  switch(imessage->Class)", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);

      st = GenerateString("    case IDCMP_REFRESHWINDOW:", fp, st, "Always refresh");

      sprintf(string,     "      GT_BeginRefresh(WindowInfo_%s.Window);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);

      if(windownode->IDCMP & IDCMP_REFRESHWINDOW)
        {
        sprintf(string,   "      Window_%s_REFRESHWINDOW(TMData, imessage);", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        }

      sprintf(string,     "      GT_EndRefresh(WindowInfo_%s.Window, TRUE);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      st = GenerateString("      break;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      if(windownode->IDCMP & IDCMP_MENUPICK)
        {
        st = GenerateString("    case IDCMP_MENUPICK:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_MENUPICK(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_GADGETDOWN)
        {
        st = GenerateString("    case IDCMP_GADGETDOWN:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_GADGETDOWN(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_GADGETUP)
        {
        st = GenerateString("    case IDCMP_GADGETUP:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_GADGETUP(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_CLOSEWINDOW)
        {
        st = GenerateString("    case IDCMP_CLOSEWINDOW:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_CLOSEWINDOW(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MOUSEBUTTONS)
        {
        st = GenerateString("    case IDCMP_MOUSEBUTTONS:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_MOUSEBUTTONS(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MOUSEMOVE)
        {
        st = GenerateString("    case IDCMP_MOUSEMOVE:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_MOUSEMOVE(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_CHANGEWINDOW)
        {
        st = GenerateString("    case IDCMP_CHANGEWINDOW:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_CHANGEWINDOW(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_SIZEVERIFY)
        {
        st = GenerateString("    case IDCMP_SIZEVERIFY:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_SIZEVERIFY(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_NEWSIZE)
        {
        st = GenerateString("    case IDCMP_NEWSIZE:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_NEWSIZE(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQSET)
        {
        st = GenerateString("    case IDCMP_REQSET:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_REQSET(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_RAWKEY)
        {
        st = GenerateString("    case IDCMP_RAWKEY:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_RAWKEY(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQVERIFY)
        {
        st = GenerateString("    case IDCMP_REQVERIFY:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_REQVERIFY(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQCLEAR)
        {
        st = GenerateString("    case IDCMP_REQCLEAR:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_REQCLEAR(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MENUVERIFY)
        {
        st = GenerateString("    case IDCMP_MENUVERIFY:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_MENUVERIFY(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_NEWPREFS)
        {
        st = GenerateString("    case IDCMP_NEWPREFS:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_NEWPREFS(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DISKINSERTED)
        {
        st = GenerateString("    case IDCMP_DISKINSERTED:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_DISKINSERTED(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DISKREMOVED)
        {
        st = GenerateString("    case IDCMP_DISKREMOVED:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_DISKREMOVED(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_ACTIVEWINDOW)
        {
        st = GenerateString("    case IDCMP_ACTIVEWINDOW:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_ACTIVEWINDOW(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_INACTIVEWINDOW)
        {
        st = GenerateString("    case IDCMP_INACTIVEWINDOW:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_INACTIVEWINDOW(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DELTAMOVE)
        {
        st = GenerateString("    case IDCMP_DELTAMOVE:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_DELTAMOVE(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_VANILLAKEY)
        {
        st = GenerateString("    case IDCMP_VANILLAKEY:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_VANILLAKEY(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_INTUITICKS)
        {
        st = GenerateString("    case IDCMP_INTUITICKS:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_INTUITICKS(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_IDCMPUPDATE)
        {
        st = GenerateString("    case IDCMP_IDCMPUPDATE:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_IDCMPUPDATE(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MENUHELP)
        {
        st = GenerateString("    case IDCMP_MENUHELP:", fp, st, NULL);
        sprintf(string,   "      if(result = Window_%s_MENUHELP(TMData, imessage)) done = result;", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

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
