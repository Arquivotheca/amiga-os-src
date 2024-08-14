
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

/* custom.c This file contains the custom code for a commodity */
/* you should be able to write a new commodity by changing only */
/* custom.c and custom.h */

#include "app.h"


BOOL setupCustomCX()
{
   return(setupApp());
}
VOID shutdownCustomCX()
{
	closeApp();
}
VOID handleCustomCXMsg(id)
ULONG id;
{
   switch(id)
   {
      case 0:
      default:
            /* MyHandleCXMsg(id); */
            break;
   }
}
VOID handleCustomCXCommand(id)
ULONG id;
{
   switch(id)
   {
      case 0:
      default:
            break;
   }
}

VOID handleCustomSignal(VOID)
{
   MyHandleCustomSignal();
}
