
/* Includes */
 
#include <string.h>
#include "TstMgr_tm.h"	/* Toolmaker header file */
#include "TstMgr.h"	/* User header file */
#include "tmgr_common.h"
#include "tmgr_misc_protos.h"

#ifndef D(x)
#define D(x)
#endif

/* Global variables */
 
/* char vers[] = VERSTAG;*/	/* Bumprev version string */  /* not in MY code you don't */
struct Library *IntuitionBase;	/* intuition.library pointer */
struct Library *GfxBase;	    /* gadtools.library pointer */
struct Library *GadToolsBase;	/* gadtools.library pointer */

/* Entry function */
/* MAIN ================================================================== */ 
VOID main(int argc, UBYTE *argv[]) {
  int returncode;   /* argument for exit() */
  struct TMData *TMData;    /* data structure pointer */
  ULONG error;

  /* Open intuition.library V37 */
  if (IntuitionBase = OpenLibrary("intuition.library", 37L)) {
    /* Open gadtools.library V37 */
    if (GadToolsBase = OpenLibrary("gadtools.library", 37L))  {
    if (GfxBase = OpenLibrary("graphics.library", 37L)) {
      /* Open Toolmaker data */
      if (TMData = TM_Open(&error))  {
        /* do Initial allocations for this program */
        if (TMgr_Init(TMData)) {
          D("Got TMgr_Init\n");
          /* copy our args into the new TMgr_Globals */
          if (getArgs(TMData, argc, argv)) {
          D("Got thru getArgs() \n");
            /* Use the custom screen */
            if(OpenScreen_TESTMANA(TMData)) {  /* lies if we're to open on WB */
              /* Open the window */
              if(OpenWindow_ERRWINDO(TMData)) {
                    TM_EventLoop(TMData);   /* Process events */    /* <<<<<<<<<<< */
              }
              else {
                  TM_Request(NULL, "Error", "Error opening %s", "Abort", NULL, "window");
              }
            } /* openscreen */
            else {
                TM_Request(NULL, "Error", "Error opening %s", "Abort", NULL, "screen");
            }
          } /* getArgs */
          TMgr_Free(TMData);  /* only frees TMData->UserData */
        } /* tmgr-init */
        returncode = TMData->ReturnCode;    /* Grab returncode from TMData */
        TM_Close(TMData);   /* Close Toolmaker data */
      } /* tmopen */
      else {
        switch(error) {
          case TMERR_MEMORY:
            TM_Request(NULL, "Error", "Out of memory", "Abort", NULL, NULL);
            break;
          default:
            TM_Request(NULL, "Error", "Unknown TM error!", "Abort", NULL, NULL);
            break;
        }
      }
      CloseLibrary(GfxBase);
    } /* gfxbase */
    else {
      TM_Request(NULL, "Error", "Error opening\ngraphics.library V37", "Abort", NULL, NULL);
    }
      CloseLibrary(GadToolsBase);   /* Close gadtools.library */
    } /* openlib gadtools */
    else {
      TM_Request(NULL, "Error", "Error opening\ngadtools.library V37", "Abort", NULL, NULL);
    }
    CloseLibrary(IntuitionBase);    /* Close intuition.library */
  } /* if openlib intuition */
  exit(returncode);
} /* end MAIN ------------------------------------------------ */

/* Window event functions */
 
BOOL Window_ERRWINDO_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg) {
    struct TMgr_Globals *t;

    t = (struct TMgr_Globals *)(TMData->UserData);

    switch(((struct Gadget *)imsg->IAddress)->GadgetID) {
        case ID_ABORTTES: /* Abort Test */
            t->carryOn = FALSE;
            break;
        case ID_BYEBYE: /* Exit */
            if (t->clientCount == 0)
                return(TRUE);
            break;
    }
    return(FALSE);
}

/* This actually gets called if we're on the Workbench */
BOOL Window_ERRWINDO_CLOSEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg) {
    struct TMgr_Globals *t;

    t = (struct TMgr_Globals *)(TMData->UserData);
    if (t->clientCount == 0)
        return(TRUE);

    return(FALSE);  /* No exiting til we're done */
}

