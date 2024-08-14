#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <pragmas/intuition_pragmas.h>
#include "protos.h"

BOOL ask(m) /* An inquriry before some major operation */
char *m;    /* message */
{
    struct EasyStruct es;
    struct Library *IntuitionBase = _SetIntuition();
    BOOL ret;

    es.es_StructSize = sizeof(es);
    es.es_Flags = NULL;
    es.es_Title = "Inquiry";
    es.es_TextFormat = "Query: %s";
    es.es_GadgetFormat = "Yes|No";

    ret = _GetResponse(IntuitionBase, _FRBuildEasyRequest(IntuitionBase, NULL, &es, NULL, m));

    _UndoIntuition(IntuitionBase);
    return ret;
}

