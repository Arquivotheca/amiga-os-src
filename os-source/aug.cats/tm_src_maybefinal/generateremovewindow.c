
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

BOOL GenerateRemoveWindow(FILE *fp, BOOL st)
  {
  DEBUGENTER("GenerateRemoveWindow", (ULONG) st);

  if(fp && st)
    {
    st = GenerateString("static VOID TM_RemoveWindow(struct TMWindowInfo *TMWindowInfo)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  struct IntuiMessage *imessage;", fp, st, NULL);
    st = GenerateString("  struct Node *succ;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  if(TMWindowInfo->Window)", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    if(TMWindowInfo->Menu)", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      ClearMenuStrip(TMWindowInfo->Window);", fp, st, NULL);
    st = GenerateString("      FreeMenus(TMWindowInfo->Menu);", fp, st, NULL);
    st = GenerateString("      TMWindowInfo->Menu = NULL;", fp, st, NULL);
    st = GenerateString("      }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("    if(TMWindowInfo->GT_IMsg)", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      GT_ReplyIMsg(TMWindowInfo->GT_IMsg);", fp, st, NULL);
    st = GenerateString("      TMWindowInfo->GT_IMsg = NULL;", fp, st, NULL);
    st = GenerateString("      }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("    if(TMWindowInfo->Window->UserPort)", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      Forbid();", fp, st, NULL);
    st = GenerateString("      imessage = (struct IntuiMessage *) TMWindowInfo->Window->UserPort->mp_MsgList.lh_Head;", fp, st, NULL);
    st = GenerateString("      while(succ = imessage->ExecMessage.mn_Node.ln_Succ)", fp, st, NULL);
    st = GenerateString("        {", fp, st, NULL);
    st = GenerateString("        if(imessage->IDCMPWindow == TMWindowInfo->Window)", fp, st, NULL);
    st = GenerateString("          {", fp, st, NULL);
    st = GenerateString("          Remove((struct Node *) imessage);", fp, st, NULL);
    st = GenerateString("          ReplyMsg((struct Message *) imessage);", fp, st, NULL);
    st = GenerateString("          }", fp, st, NULL);
    st = GenerateString("        imessage = (struct IntuiMessage *) succ;", fp, st, NULL);
    st = GenerateString("        }", fp, st, NULL);
    st = GenerateString("      TMWindowInfo->Window->UserPort = NULL;", fp, st, NULL);
    st = GenerateString("      ModifyIDCMP(TMWindowInfo->Window, 0L);", fp, st, NULL);
    st = GenerateString("      Permit();", fp, st, NULL);
    st = GenerateString("      }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("    CloseWindow(TMWindowInfo->Window);", fp, st, NULL);
    st = GenerateString("    TMWindowInfo->Window = NULL;", fp, st, NULL);
    st = GenerateString("    TMWindowInfo->Flags = 0;", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  if(TMWindowInfo->FirstGadget)", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    FreeGadgets(TMWindowInfo->FirstGadget);", fp, st, NULL);
    st = GenerateString("    TMWindowInfo->FirstGadget = NULL;", fp, st, NULL);
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
