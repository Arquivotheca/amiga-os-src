#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

BOOL ask3(m, n, o) /* An inquriry before some major operation */
char *m, *n, *o;   /* message */
{
    struct Library *IntuitionBase = _SetIntuition();
    struct EasyStruct es;
    BOOL ret;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "Inquiry";
    es.es_TextFormat = "Query: %s\n%s\n%s";
    es.es_GadgetFormat = "Yes|No";

    ret = _GetResponse(IntuitionBase, _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, m, n, o));
    _UndoIntuition(IntuitionBase);
    return ret;
}

