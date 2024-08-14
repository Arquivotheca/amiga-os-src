#ifndef UTILS_H
#define UTILS_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <graphics/rastport.h>

/*****************************************************************************/

LONG __stdargs DoMethod(Object *, ULONG, ...);
LONG __stdargs DoMethodA(Object *, VOID *);
LONG __stdargs DoSuperMethod(Class *, Object *, ULONG, ...);
LONG __stdargs DoSuperMethodA(Class *, Object *, VOID *msg);

/*****************************************************************************/

void ASM QuickBevel (REG (a6) struct ClassLib *, REG (a1) struct RastPort *, REG (a0) struct IBox *, REG (d0) UBYTE ulpen, REG (d1) UBYTE lrpen);
void ASM QuickBevel1 (REG (a6) struct ClassLib *, REG (a1) struct RastPort *, REG (a0) struct IBox *, REG (d0) UBYTE ulpen, REG (d1) UBYTE lrpen);

/*****************************************************************************/

#endif /* UTILS_H */
