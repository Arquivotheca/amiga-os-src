
/* Includes */
 
#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/modeid.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <clib/dos_protos.h>
#include <string.h>
#include <stdlib.h>

#include "db.h"	/* User header file */
#include "db_rev.h"	/* Bumprev header file */

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

/* Global variables */
 
char vers[] = VERSTAG;	/* Bumprev version string */
struct Library *IntuitionBase;	/* intuition.library pointer */
struct Library *GadToolsBase;	/* gadtools.library pointer */
struct Library *UtilityBase;	/* utility.library pointer */
struct IntuiText BodyText = {0,1,JAM2,20,8,NULL,(UBYTE *)TEXT_NORELEASE2,NULL};
struct IntuiText NegText  = {0,1,JAM2, 6,4,NULL,(UBYTE *)TEXT_OK,NULL};

/* Entry functions */
 
#ifdef _DCC	/* for DICE compatibility */
VOID wbmain(VOID *wbmsg)
  {
  main(0, wbmsg);
  }
#endif

BOOL ConvToNum(STRPTR hexString, ULONG *value);

VOID main(int argc, char **argv)
  {

  struct TMData *TMData;	/* data structure pointer */
  ULONG error;

  if(!(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 37L)))	/* Open intuition.library V37 */
    {
    if(IntuitionBase = OpenLibrary((UBYTE *)"intuition.library", 0L))
      {
      AutoRequest(NULL, &BodyText, NULL, &NegText, 0, 0, 320, 80);
      CloseLibrary(IntuitionBase);
      }
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(GadToolsBase = OpenLibrary((UBYTE *)"gadtools.library", 39L)))	/* Open gadtools.library V37 */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "gadtools.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(UtilityBase = OpenLibrary((UBYTE *)"utility.library", 37L)))	/* Open utility.library V37 */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, "utility.library V37");
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(TMData = TM_Open(&error)))	/* Open Toolmaker data */
    {
    switch(error)
      {
      case TMERR_MEMORY:
        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);
        break;
      case TMERR_MSGPORT:
        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMSGPORT, (UBYTE *)TEXT_ABORT, NULL, NULL);
        break;
      }
    cleanexit(NULL, RETURN_FAIL);
    }

  if(!(OpenScreen_Workbench(TMData)))	/* Open default public screen */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOSCREEN, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  /* Set up for the initial ModeID entry */
  {
	#define TEMPLATE "MODE"
	#define OPT_MODE 0
	#define OPT_COUNT 1

	LONG result[OPT_COUNT];
	LONG *val;
	struct RDArgs *rdargs;

	TMData->UserData = (APTR)INVALID_ID;
	result[OPT_MODE] = NULL;
	if (rdargs = ReadArgs(TEMPLATE, result, NULL))
	{
		if (val = (LONG *)result[OPT_MODE])
		{
			ConvToNum((STRPTR)val, (ULONG *)&TMData->UserData);
		}
		else
		{
			TMData->UserData = INVALID_ID;
		}
		FreeArgs(rdargs);
	}
  }

  if(!(OpenWindow_GFXDATAB(TMData)))	/* Open the window */
    {
    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOWINDOW, (UBYTE *)TEXT_ABORT, NULL, NULL);
    cleanexit(TMData, RETURN_FAIL);
    }

  TM_EventLoop(TMData);	/* Process events */

  cleanexit(TMData, RETURN_OK);
  }

/* cleanexit function */
 
VOID cleanexit(struct TMData *TMData, int returnvalue)
  {
  if(TMData)
    {
    CloseWindow_GFXDATAB(TMData);
    CloseScreen_Workbench(TMData);
    TM_Close(TMData);
    }

  if(UtilityBase)   CloseLibrary(UtilityBase);	/* Close utility.library */
  if(GadToolsBase)  CloseLibrary(GadToolsBase);	/* Close gadtools.library */
  if(IntuitionBase) CloseLibrary(IntuitionBase);	/* Close intuition.library */

  exit(returnvalue);
  }

/* Window event functions */
 
BOOL Window_GFXDATAB_CLOSEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  return(TRUE);
  }

void PrintDB(struct TMData *TMData);
ULONG BestID(struct TMData *TMData);
BOOL Window_GFXDATAB_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  switch(((struct Gadget *)imsg->IAddress)->GadgetID)
    {
    case ID_PRINT: /* _Print */
      PrintDB(TMData);
      break;
    case ID_MODEIDS: /* ModeIDs */
      switch(imsg->Code)
        {
        case ID_MODEIDS_THIRD:
          break;
        }
      break;
    case ID_MODEID: /* _ModeID */
      /* ((struct StringInfo *)gadget_MODEID->SpecialInfo)->Buffer */
      break;
    case ID_NAME: /* _Name */
      //gadget_NAME->Flags ^= GFLG_SELECTED;
      break;
    case ID_MONITOR: /* M_onitor */
      //gadget_MONITOR->Flags ^= GFLG_SELECTED;
      break;
    case ID_DIMENSIO: /* _Dimension */
      //gadget_DIMENSIO->Flags ^= GFLG_SELECTED;
      break;
    case ID_DISPLAY: /* D_isplay */
      //gadget_DISPLAY->Flags ^= GFLG_SELECTED;
      break;
    case ID_VECTOR: /* _Vector */
      //gadget_VECTOR->Flags ^= GFLG_SELECTED;
      break;
    case ID_BESTMODE: /* _BestModeIDAttrs */
      /* ((struct StringInfo *)gadget_BESTMODE->SpecialInfo)->Buffer */
      BestID(TMData);
      break;
    }
  return(FALSE);
  }

void ToggleGadget(struct Window *win, struct Gadget *gad);
BOOL Window_GFXDATAB_VANILLAKEY(struct TMData *TMData, struct IntuiMessage *imsg)
  {
  switch(imsg->Code)
    {
    case 'D':
    case 'd':
      ToggleGadget(window_GFXDATAB, gadget_DIMENSIO);
      break;
    case 'I':
    case 'i':
      ToggleGadget(window_GFXDATAB, gadget_DISPLAY);
      break;
    case 'N':
    case 'n':
      ToggleGadget(window_GFXDATAB, gadget_NAME);
      break;
    case 'O':
    case 'o':
      ToggleGadget(window_GFXDATAB, gadget_MONITOR);
      break;
    case 'V':
    case 'v':
      ToggleGadget(window_GFXDATAB, gadget_VECTOR);
      break;
    case 'P':
    case 'p':
    case 0xd:
      PrintDB(TMData);
      break;
    case 'M':
    case 'm':
      ActivateGadget(gadget_MODEID, window_GFXDATAB, NULL);
      break;
    case 'B':
    case 'b':
      ActivateGadget(gadget_BESTMODE, window_GFXDATAB, NULL);
      break;
    }
  return(FALSE);
  }

