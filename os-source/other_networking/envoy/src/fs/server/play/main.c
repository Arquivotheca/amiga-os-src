
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <stdlib.h>

/* #include "play.h" */
#include "play_tm.h"

struct Library *IntuitionBase;
struct Library *GadToolsBase;

VOID main(int argc, char **argv)
  {
  struct TMData *TMData = NULL;
  ULONG TMError;
  int returncode;

  if(IntuitionBase = OpenLibrary("intuition.library", 37L))
    {
    if(GadToolsBase = OpenLibrary("gadtools.library", 37L))
      {
      if(!(TMError = TM_Open(&TMData)))
        {
        Window_label1(TMData, DISPLAY);
        TM_EventLoop(TMData);
        Window_label1(TMData, REMOVE);
        returncode = TMData->ReturnCode;
        TM_Close(TMData);
        }
      else
        {
        switch(TMError)
          {
          case TMERROR_MEMORY:
            TM_Request(NULL, "Error", "Error allocating\ndata memory", "Ok");
            break;
          case TMERROR_MSGPORT:
            TM_Request(NULL, "Error", "Error creating\nmessage port", "Ok");
            break;
          case TMERROR_SCREEN:
            TM_Request(NULL, "Error", "Error opening\nscreen", "Ok");
            break;
          case TMERROR_VISUALINFO:
            TM_Request(NULL, "Error", "Error getting\nvisual info", "Ok");
            break;
          }
        }
      CloseLibrary(GadToolsBase);
      }
    else
      {
      TM_Request(NULL, "Error", "Error opening\ngadtools.library", "Ok");
      }
    CloseLibrary(IntuitionBase);
    }
  exit(returncode);
  }

BOOL Window_label1_GADGETDOWN(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  switch(((struct Gadget *)imsg->IAddress)->GadgetID)
    {
    case ID_label8: /* Users and Groups */
      break;
    case ID_label11: /* Shared Directories */
      break;
    }
  return(FALSE);
  }

BOOL Window_label1_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  switch(((struct Gadget *)imsg->IAddress)->GadgetID)
    {
    case ID_label2: /* Cancel */
      break;
    case ID_label3: /* Save */
      break;
    case ID_label4: /* Delete */
      break;
    case ID_label5: /* Add */
      break;
    case ID_label6: /* Delete */
      break;
    case ID_label7: /* Add */
      break;
    case ID_label8: /* Users and Groups */
      /* imsg->Code */
      break;
    case ID_label9: /* Allow left-out icons */
      /* gadget_label9->Flags & GFLG_SELECTED */
      break;
    case ID_label10: /* Allow volume snapshotting */
      /* gadget_label10->Flags & GFLG_SELECTED */
      break;
    case ID_label11: /* Shared Directories */
      /* imsg->Code */
      break;
    }
  return(FALSE);
  }

BOOL Window_label1_CLOSEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  return(TRUE);
  }

