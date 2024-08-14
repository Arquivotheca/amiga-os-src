
#define CATCOMP_ARRAY 1

#include "prepcard.h"



STRPTR GetString( struct cmdVars *cv, ULONG id)
{
register UWORD  i;
register STRPTR local;

    local = NULL;

    i = 0;
    while (!local)
    {
        if (CatCompArray[i].cca_ID == id)
            local = CatCompArray[i].cca_Str;
        i++;
    }

    if (LocaleBase)
        return(GetCatalogStr(cv->cv_Catalog,id,local));

    return(local);
}

VOID DisplayError( struct cmdVars *cv, ULONG id )
{
register STRPTR str;

	if(str = GetString(cv,id))
	{
		if(cv->cv_FromCLI)
		{
			PutStr(str);
		}
		else
		{
			DoEasyRequest(cv, str, GetString(cv,MSG_PREP_OK_GAD));
		}

	}

}

BOOL DisplayQuery( struct cmdVars *cv, ULONG id )
{
register STRPTR str;

	if(str = GetString(cv,id))
	{
		return( (BOOL)DoEasyRequest(cv, str, GetString(cv,MSG_PREP_QUERY_GAD)));

	}
	return(FALSE);
}

LONG DoEasyRequest( struct cmdVars *cv, STRPTR body, STRPTR gadgets )
{

struct EasyStruct es;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = GetString(cv,MSG_PREP_TITLE);
	es.es_TextFormat = body;
	es.es_GadgetFormat = gadgets;
	
	return( (LONG)EasyRequestArgs(NULL,&es,NULL,NULL) );

}