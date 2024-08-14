/* hyperbrowser.h
 * Written by David N. Junod
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <graphics/gfx.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <libraries/amigaguide.h>
#include <libraries/gadtools.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/alib_protos.h>
#include <clib/amigaguide_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/amigaguide_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*****************************************************************************/

#define	BASENAME	"HYPERNOZY"
#define	BASENAME_LENGTH	9

/*****************************************************************************/

#define ASM		__asm
#define REG(x)		register __ ## x

/*****************************************************************************/

void kprintf (void *, ...);
void sprintf (void *, ...);

/* asprintf.asm */
void ASM asprintf (REG (a3) STRPTR buffer, REG (a0) STRPTR fmt, REG (a1) APTR data);

/*****************************************************************************/

#include "globaldata.h"

#include "hyperbrowser_iprotos.h"
