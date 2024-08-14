/* token.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

struct AmigaGuideToken *ObtainXRefToken (struct AmigaGuideLib * ag)
{
    struct AmigaGuideToken *agt = NULL;
    struct NamedObject *no;

    /* See if the token already exists */
    if (no = FindNamedObject (NULL, TOKEN_NAME, NULL))
    {
	agt = (struct AmigaGuideToken *) no->no_Object;
	ReleaseNamedObject (no);
    }

    return (agt);
}
