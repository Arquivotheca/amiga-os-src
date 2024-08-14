#ifndef UTILS_H
#define UTILS_H

/*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <graphics/rastport.h>

/*****************************************************************************/

VOID __stdargs QuickBevel(struct RastPort *rp, struct IBox *, LONG, LONG);
LONG __stdargs DoMethod(Object *, ULONG, ...);
LONG __stdargs DoMethodA(Object *, VOID *);
LONG __stdargs DoSuperMethod(Class *, Object *, ULONG, ...);
LONG __stdargs DoSuperMethodA(Class *, Object *, VOID *msg);

/*****************************************************************************/

#endif /* UTILS_H */
