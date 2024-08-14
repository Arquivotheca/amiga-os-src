
/* includes */
#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <dos.h>


/*****************************************************************************/


VOID __asm FillOldExtent(register __a0 struct RastPort *rp,
                         register __a1 struct Rectangle *oldExtent,
                         register __a2 struct Rectangle *newExtent,
                         register __a6 struct Library *gfxBase);
