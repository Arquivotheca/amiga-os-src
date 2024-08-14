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


VOID AslCloseWindow(struct ASLLib *AslBase, struct Window *wp, BOOL others);
VOID AslDrawBevelBox(struct ASLLib *AslBase, struct Window *wp, WORD x, WORD y, WORD w, WORD h, ULONG tags, ...);
ULONG AslPackBoolTags(struct ASLLib *AslBase, ULONG flags, struct TagItem *tags, ULONG mappers, ...);
VOID StripExtension(struct ASLLib *AslBase, STRPTR string, STRPTR extension);
VOID BtoC(struct ASLLib *AslBase, APTR bstr, STRPTR string);
          /* C pointer to BCPL string, not BCPL pointer! */


/*****************************************************************************/


#endif /* ASLUTILS_H */
