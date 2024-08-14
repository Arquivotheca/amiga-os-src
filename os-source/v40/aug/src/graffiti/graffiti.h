/* graffiti.h
 *
 */

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/icclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <string.h>
#include <stdio.h>

#include <clib/macros.h>
#include <clib/asl_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/wb_protos.h>

#include <pragmas/asl_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/wb_pragmas.h>

/*****************************************************************************/

extern void kprintf (void *, ...);
extern void __stdargs NewList (struct List *);

/*****************************************************************************/

#define	SERVER_NAME	"Graffiti"

/*****************************************************************************/

#define ASM				__asm
#define REG(x)				register __ ## x
#define MEMORY_FOLLOWING(ptr)		((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)	((void *)( ((ULONG)ptr) + n ))
#define	V(x)				((void *)x)

/*****************************************************************************/

struct Plot
{
    ULONG		 p_X;
    ULONG		 p_Y;
};

/*****************************************************************************/

#include "globaldata.h"

/*****************************************************************************/

#include "windowclass.h"

/*****************************************************************************/

#include "nipc.h"
#include "graffiti_iprotos.h"
