/*****************************************************************************
*
*	$Id: CopDis.h,v 39.1 92/11/18 10:34:12 Jim2 Exp Locker: Jim2 $
*
******************************************************************************
*
*	$Log:	CopDis.h,v $
 * Revision 39.1  92/11/18  10:34:12  Jim2
 * First Release - works with remote wack.
 * 
*
******************************************************************************/

#ifndef COPDIS_H

#define COPDIS_H

#ifndef EXEC_TYPES_H
    #include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
    #include "exec/libraries.h"
#endif

#ifndef EXEC_EXECBASE_H
    #include <exec/execbase.h>
#endif

#ifndef GRAPHIC_GFXBASE_H
    #include <graphics/gfxbase.h>
#endif

#ifndef VERSION
	#include "copdis_rev.h"
#endif

VOID CopDisVPrintf(STRPTR Fmt, ...);
APTR CopDisFindPointer(APTR Address);
UWORD CopDisFindWord (APTR Address);
VOID CopDisReadBlock(APTR Address, APTR Buffer, ULONG Size);

extern struct ExecBase *SysBase;
extern struct Library *DOSBase;
extern struct GfxBase *GfxBase;

#endif