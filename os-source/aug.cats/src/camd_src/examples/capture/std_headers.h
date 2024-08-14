
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define unless(x) if(!(x))  
#define LEAVE goto exitit
#define offsetof(s, m)  (size_t)(&(((s *)0)->m))
#define elementsof(a)  ((sizeof(a) / sizeof(a[0])))
#ifndef MIN
#define MIN(a,b)    ((a) <= (b) ? (a) : (b))
#endif
