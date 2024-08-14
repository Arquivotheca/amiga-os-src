#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

BOOL warning2(m, n) /* Far less serious - non-fatal error */
char *m, *n;    /* messages */
{
    struct Library *IntuitionBase = _SetIntuition();
    struct EasyStruct es;
    BOOL ret;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "Warning";
    es.es_TextFormat = "Error: %s\n%s";
    es.es_GadgetFormat = "Continue|Abort";

    ret = _GetResponse(IntuitionBase, _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, m, n));
    _UndoIntuition(IntuitionBase);
    return ret;
}

