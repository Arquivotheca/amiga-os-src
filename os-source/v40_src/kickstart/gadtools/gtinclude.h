/*  gtinclude.h - includes used by gadtools source
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*   All Rights Reserved.
*
*   $Id: gtinclude.h,v 39.4 92/10/16 18:26:51 vertex Exp $
*
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>
#include <graphics/text.h>
#include <utility/tagitem.h>

#define USE_BUILTIN_MATH
#include <string.h>

#include <dos.h>

#include "gadtools.h"
#include "gtinternal.h"

#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>
#include "gadtools_protos.h"
#include "gtinternal_protos.h"

#define GadToolsBase	((struct GadToolsLib *)getreg(REG_A6))
#define SysBase		(GadToolsBase->gtb_SysBase)
#define UtilityBase	(GadToolsBase->gtb_UtilityBase)
#define GfxBase		(GadToolsBase->gtb_GfxBase)
#define IntuitionBase	(GadToolsBase->gtb_IntuitionBase)
#define LayersBase      (GadToolsBase->gtb_LayersBase)

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include "gadtools_pragmas.h"
