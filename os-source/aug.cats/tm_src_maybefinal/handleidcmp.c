
/****************************************************************************/
/*                                                                          */
/*  HandleIDCMP.c - IDCMP message dispatching functions                     */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"
#include "Externs.h"

#include <devices/inputevent.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

/****************************************************************************/

int HandleMainIDCMP(struct Window *window)
  {
  int    done=FALSE;
  int    result;
  struct IntuiMessage *imessage;

  DEBUGENTER("HandleMainIDCMP", NULL);

  while(imessage = GT_GetIMsg(window->UserPort))
    {
    switch(imessage->Class)
      {
      case IDCMP_CLOSEWINDOW:
        if(RequestKill()) done = DONE_ENDPROGRAM;
        break;

      case IDCMP_MENUPICK:
        if(result = HandleMenuPick(imessage->Code)) done = result;
        break;

      case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(window);
        GT_EndRefresh(window, TRUE);
        break;

      case IDCMP_GADGETDOWN:
        if(result = HandleGadgetDown((struct Gadget *) imessage->IAddress, imessage->Code)) done = result;
        break;

/*
      case IDCMP_MOUSEMOVE:
*/
      case IDCMP_GADGETUP:
        if(result = HandleGadgetUp((struct Gadget *) imessage->IAddress, imessage->Code)) done = result;
        break;

      case IDCMP_VANILLAKEY:
        if(result = HandleMainVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
        break;

      case IDCMP_ACTIVEWINDOW:
        UnSelectAllGadgets(NULL);
        currentwindow = NULL;
        break;

      case IDCMP_CHANGEWINDOW:
        mainleft = window->LeftEdge;
        maintop = window->TopEdge;
        break;
      }

    GT_ReplyIMsg(imessage);
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

/****************************************************************************/

int HandleRequesterIDCMP(struct Window *window)
  {
  int    done=FALSE;
  int    result;
  struct IntuiMessage *imessage;

  DEBUGENTER("HandleRequesterIDCMP", NULL);

  while(imessage = GT_GetIMsg(window->UserPort))
    {
    switch(imessage->Class)
      {
      case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(window);
        GT_EndRefresh(window, TRUE);
        break;

      case IDCMP_GADGETDOWN:
        if(result = HandleGadgetDown((struct Gadget *) imessage->IAddress, imessage->Code)) done = result;
        break;

      case IDCMP_MOUSEMOVE:
        if(result = HandleGadgetMove((struct Gadget *) imessage->IAddress, imessage->Code)) done = result;
        break;

      case IDCMP_GADGETUP:
        if(result = HandleGadgetUp((struct Gadget *) imessage->IAddress, imessage->Code)) done = result;
        break;

      case IDCMP_VANILLAKEY:
        if(window == W_Edit)
          {
          if(result = HandleEditVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
          }
        else if(window == W_IDCMP)
          {
          if(result = HandleIDCMPVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
          }
        else if(window == W_Menu)
          {
          if(result = HandleMenuVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
          }
        else if(window == W_Screen)
          {
          if(result = HandleScreenVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
          }
        else if(window == W_Colors)
          {
          if(result = HandleColorsVanillaKey(imessage->Code, imessage->Qualifier)) done = result;
          }
        break;
      }

    GT_ReplyIMsg(imessage);
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

/****************************************************************************/

int HandleArrayIDCMP(struct WindowNode *thiswindownode)
  {
  int    done = FALSE;
  struct WindowNode *windownode;
  struct IntuiMessage *imessage;

  DEBUGENTER("HandleArrayIDCMP", NULL);

  while(thiswindownode && (imessage = GT_GetIMsg(thiswindownode->Window->UserPort)))
    {
    switch(imessage->Class)
      {
      case IDCMP_MENUPICK:
        if(!realmenus) done = HandleMenuPick(imessage->Code);
        break;

      case IDCMP_MOUSEBUTTONS:
        if(!realgadgets) HandleMouseButtons(thiswindownode, imessage);
        break;

      case IDCMP_INTUITICKS:
        HandleMouseMove(thiswindownode, imessage);
        break;

      case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(thiswindownode->Window);
        GT_EndRefresh(thiswindownode->Window, TRUE);
        if(!realgadgets)
          {
          SetRast(thiswindownode->Window->RPort, 0);
          RefreshWindowFrame(thiswindownode->Window);
          AddGadgets(thiswindownode);
          }
        break;

      case IDCMP_VANILLAKEY:
        HandleMainVanillaKey(imessage->Code, imessage->Qualifier);
        break;

      case IDCMP_CHANGEWINDOW:
        HandleChangeWindow(thiswindownode);
        break;

      case IDCMP_INACTIVEWINDOW:
        break;

      case IDCMP_ACTIVEWINDOW:
        currentwindow = thiswindownode;
        for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
            windownode = (struct WindowNode *) windownode->Node.ln_Succ)
          {
          if(windownode != currentwindow) UnSelectAllGadgets(windownode);
          }
        break;
      }

    GT_ReplyIMsg(imessage);
    }

  DEBUGEXIT(TRUE, (ULONG) done);
  return(done);
  }

