#ifndef ASLUTILS_H
#define ASLUTILS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>

#include "aslbase.h"


/*****************************************************************************/


struct Window *AslOpenWindow(ULONG tag,...);
VOID AslCloseWindow(struct Window *wp, BOOL others);
VOID AslDrawBevelBox(struct Window *wp, WORD x, WORD y, WORD w, WORD h, ULONG tags, ...);
ULONG AslPackBoolTags(ULONG flags, struct TagItem *tags, ULONG mappers, ...);
VOID StripExtension(STRPTR string, STRPTR extension);
VOID BtoC(APTR bstr, STRPTR string);
          /* C pointer to BCPL string, not BCPL pointer! */


/*****************************************************************************/


#endif /* ASLUTILS_H */
