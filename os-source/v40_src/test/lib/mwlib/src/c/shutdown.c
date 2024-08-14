/****** MWLib/c/ShutDown ******************************************************

    NAME
        ShutDown -- ShutDown stub

    SYNOPSIS
        ShutDown()

    FUNCTION
        Calls exit(20) after displaying a warning.

******************************************************************************/

#include <exec/types.h>
#include "Tools.h"

void ShutDown()
{
    notice("ShutDown Stub called. Resources may not be relinquished.");
    exit(20);
}