#ifndef UTILITY_UTILITY_H
#define UTILITY_UTILITY_H
/*
**	$Id: utility.h,v 39.2 92/09/18 11:38:21 vertex Exp $
**
**	utility.library include file
**
**	(C) Copyright 1992 Commodore-Amiga Inc.
**	All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif


/*****************************************************************************/


#define UTILITYNAME "utility.library"


struct UtilityBase
{
    struct Library ub_LibNode;
    UBYTE          ub_Language;
    UBYTE          ub_Reserved;
};


/*****************************************************************************/


#endif /* UTILITY_UTILITY_H */
