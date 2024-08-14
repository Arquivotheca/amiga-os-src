#ifndef UTILS_H
#define UTILS_H


/*****************************************************************************/


#include <exec/types.h>
#include <exec/lists.h>
#include <exec/resident.h>

#define LOADRESOURCE
#include <localestr/c.h>


/*****************************************************************************/

VOID OpenCat(VOID);
VOID CloseCat(VOID);

STRPTR GetStr(ULONG id);
VOID PrintStr(ULONG id);
VOID PrintFStr(ULONG id, STRPTR arg1, ...);

struct Node *FindNameNC(struct List *list, STRPTR name);
struct Resident *FindRomTag(BPTR segList);


/*****************************************************************************/


#endif /* UTILS_H */
