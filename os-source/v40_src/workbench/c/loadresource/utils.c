
#include <exec/types.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <libraries/locale.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define STRINGARRAY
#include "utils.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *DOSBase;
extern struct Library  *LocaleBase;
extern struct Library  *UtilityBase;

struct Catalog         *catalog;


/*****************************************************************************/


VOID OpenCat(VOID)
{
    if (!catalog && LocaleBase)
        catalog = OpenCatalogA(NULL,"sys/c.catalog",NULL);
}


/*****************************************************************************/


VOID CloseCat(VOID)
{
    if (catalog)
        CloseCatalog(catalog);
}


/*****************************************************************************/


STRPTR GetStr(ULONG id)
{
UWORD  i;
STRPTR local = NULL;

    i = 0;
    while (!local)
    {
        if (AppStrings[i].as_ID == id)
            local = AppStrings[i].as_Str;
        i++;
    }

    if (LocaleBase)
        return(GetCatalogStr(catalog,id,local));

    return(local);
}


/*****************************************************************************/


VOID PrintFStr(ULONG id, STRPTR arg1, ...)
{
    OpenCat();

    VPrintf(GetStr(id),(LONG *)&arg1);

    CloseCat();
}


/*****************************************************************************/


VOID PrintStr(ULONG id)
{
    PrintFStr(id,"");
}


/*****************************************************************************/


struct Node *FindNameNC(struct List *list, STRPTR name)
{
struct Node *node;
WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        result = Stricmp(name,node->ln_Name);
        if (result == 0)
            return(node);

	node = node->ln_Succ;
    }

    return(NULL);
}


/*****************************************************************************/


struct Resident *FindRomTag(BPTR segList)
{
struct Resident *tag;
UWORD           *seg;
ULONG            i;
ULONG           *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        seg     = (UWORD *)((ULONG)ptr);
        segList = *ptr;

        for (i=*(ptr-1)>>1; (i>0) ; i--)
        {
            if (*(seg++) == RTC_MATCHWORD)
            {
                tag = (struct Resident *)(--seg);
                if (tag->rt_MatchTag == tag)
                {
                    return(tag);
                }
            }
        }
    }

    return(NULL);
}
