#ifndef lint
static char     rcsid[] = "$Id: abort.c,v 3.4 91/06/20 20:33:36 katogi Exp Locker: katogi $";
#endif
#include <stdio.h>
#include <stdlib.h>
#include "edef.h"

/***********************************************************/
/*   egcabort()                                            */
/***********************************************************/
VOID            egcabort();

VOID
egcabort()
{
    fprintf(stdout, "\n Abort \n");
    exit(1);
}
