/******************************************************************************
*
*	$Id: layersbase.h,v 39.1 92/06/05 11:47:26 mks Exp $
*
******************************************************************************/

#ifndef GRAPHICS_LAYERSBASE_H
#define GRAPHICS_LAYERSBASE_H

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "layers_internal_protos.h"

/* SAS/C include for the fancy getreg() stuff */
#include <dos.h>
#include <string.h>

/* private stuff */
/*internal only, not to be made public */
struct LayerInfo_extra
{
	long		env[13];    /* for setjmp/longjmp */
struct	MinList		mem;
	UBYTE		opencount;
	UBYTE		pad[3];
};

/* secret defines */
#define	LMN_REGION	(-1)
#define	LMN_BITMAP	(-2)

struct LayersBase
{
    struct Library   LibNode;
    struct GfxBase  *GfxBase;
    struct ExecBase *ExecBase;
    struct Library  *UtilityBase;
};

/* Define to make LAYERSBASE be read from A6 */
#define	LAYERSBASE	((struct LayersBase *)getreg(REG_A6))

/* Defines to make the #pragmas work right */
#define	GfxBase		LAYERSBASE->GfxBase
#define	SysBase		LAYERSBASE->ExecBase
#define	UtilityBase	LAYERSBASE->UtilityBase

#endif
