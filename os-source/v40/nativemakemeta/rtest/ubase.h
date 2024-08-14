/***********************************************************************
 *                                                                 
 * ubase.h -- definition of utility.library base                  
 *                                                               
 * Copyright (C) 1985, 1989 Commodore Amiga Inc.  All rights reserved.
 *
 * $Id: ubase.h,v 36.2 90/11/05 18:55:13 peter Exp $
 ***********************************************************************/

#ifndef  UTILITY_BASE_H
#define UTILITY_BASE_H


#ifndef  EXEC_TYPES_H
#include  "exec/types.h"
#endif

#ifndef  EXEC_LISTS_H
#include  "exec/lists.h"
#endif

#ifndef  EXEC_LIBRARIES_H
#include  "exec/libraries.h"
#endif


/* library data structures */

struct UtilityBase {
    struct Library	ub_Library;
    UBYTE		ub_Flags;
    UBYTE		ub_pad;
    ULONG		ub_SysLib;
    ULONG		ub_SegList;
};

#endif
