#ifndef MYCXAPP_H
#define MYCXAPP_H

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/commodities.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <libraries/gadtools.h>
#include <utility/tagitem.h>


#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/commodities_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cxlib_proto.h"


#include "myprotos.h"
#include "myexterns.h"
#include "mydebug.h"
#include "mycxwin.h"

#define COF_SHOW_HIDE 4

#define PRIORITY_TOOL_TYPE     "CX_PRIORITY"
#define POP_ON_START_TOOL_TYPE "CX_POPUP"
#define POPKEY_TOOL_TYPE       "CX_POPKEY"

#define POP_KEY_ID     (86L)         /* pop up identifier */

#define CMDLEN	80
#define TOTKEYS	40
#define FOFF  0
#define SOFF 10
#define AOFF 20
#define ASOFF 30

struct HKInfo {
	CxObj *cxobj;   /* if non-zero, is installed */
	UBYTE *key;
	UBYTE *command; /* if non-zero, has buffer allocated */
	};

extern struct HKInfo hkeys[];
extern UBYTE *keynames[];
extern ULONG setoffs;


/**********************************************************************/
/* Commodities specific definitions.                                  */
/*                                                                    */
/* COM_NAME  - used for the scrolling display in the Exchange program */
/* COM_TITLE - used for the window title bar and the long description */
/*             in the Exchange program                                */
/* COM_DESC  - Commodity description used by the Exchange program     */
/* CX_DEFAULT_PRIORITY - default priority for this commodities broker */
/*                       can be overidden by using icon TOOL TYPES    */
/**********************************************************************/
#define COM_NAME  "hkey"
#define COM_TITLE "HKey"
#define COM_DESCR "Assign Actions To Function Keys"
#define CX_DEFAULT_PRIORITY 0
#define CX_DEFAULT_POP_KEY ("alt f1")
#define CX_DEFAULT_POP_ON_START ("YES")

#endif /* MYCXAPP_H */
