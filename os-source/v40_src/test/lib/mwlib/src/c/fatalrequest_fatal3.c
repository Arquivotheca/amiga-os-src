#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

BOOL fatal3(char *m, char *n, char *o)  /* Program bug detected - could be fatal! */
{
    struct Library *IntuitionBase = _SetIntuition();
    struct EasyStruct es;
    BOOL ret;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "<FATAL REQUEST>";
    es.es_TextFormat = "Fatal Error: %s\n%s\n%s\nDo you wish to attempt a StateSave?\n(WARNING-StateSave May CRASH MACHINE!)";
    es.es_GadgetFormat = "Attempt StateSave|Just Exit Immediately";

    ret = _GetResponse(IntuitionBase, _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, m, n, o));
    _UndoIntuition(IntuitionBase);
    return ret;
}
