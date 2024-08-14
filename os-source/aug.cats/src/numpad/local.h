
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

/* local.h - Definitions used by standard modules */

#ifdef LATTICE
#include <clib/all_protos.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "app.h"

/**********/
/* main.c */
/**********/
extern struct IntuitionBase *IntuitionBase;
extern struct Library       *CxBase;
extern struct GfxBase       *GfxBase;
extern struct DosLibrary    *DOSBase;
extern struct Library       *GadToolsBase;

extern struct MsgPort *cxport;      /* commodities messages here      */
extern ULONG          cxsigflag;    /* signal for above               */

extern struct MsgPort *iport;       /* Intuition IDCMP messages here   */
extern ULONG          isigflag;     /* signal for above                */

/********/
/* cx.c */
/********/
extern char   hotkeybuff[];   /* holds the string describing the popup */
                              /* hotkey. Used for the window title     */

VOID handleCxMsg(struct Message *);
BOOL setupCX(char **);
VOID shutdownCX(VOID);

/************/
/* window.c */
/************/
VOID   handleIMsg(struct IntuiMessage *);
struct Window *getNewWindow(VOID);
int    addGadgets(struct Window *);
VOID   removeGadgets(VOID);

#define PRIORITY_TOOL_TYPE     "CX_PRIORITY"
#define POP_ON_START_TOOL_TYPE "CX_POPUP"
#define POPKEY_TOOL_TYPE       "CX_POPKEY"
