/* token.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct AmigaGuideToken *ObtainXRefToken (struct AGLib * ag)
{
    struct AmigaGuideToken *agt = NULL;
    struct NamedObject *no;

    /* See if the token already exists */
    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
    {
	agt = (struct AmigaGuideToken *) no->no_Object;
	ReleaseNamedObject (no);
    }
    /* No token, so create one */
    else if (no = ano (ag, TOKEN_NAME,
			ANO_NameSpace,	TRUE,
			ANO_UserSpace,	TKSIZE,
			ANO_Flags,	NSF_NODUPS | NSF_CASE,
			TAG_DONE))
    {
	if (no->no_Object)
	{
	    agt = (struct AmigaGuideToken *) no->no_Object;

	    InitSemaphore (&agt->agt_Lock);
	    NewList (&agt->agt_XRefFileList);
	    NewList (&agt->agt_XRefList);

	    /* Make the name public */
	    if (!AddNamedObject (NULL, no))
	    {
		agt = NULL;
		FreeNamedObject (no);
	    }

	    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
	    {
		agt = (struct AmigaGuideToken *) no->no_Object;
		ReleaseNamedObject (no);
	    }
	}
	else
	{
	    ReleaseNamedObject (no);
	}
    }

    return (agt);
}
