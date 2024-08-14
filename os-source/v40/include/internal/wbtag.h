#ifndef INTERNAL_WBTAG_H
#define INTERNAL_WBTAG_H
/*
**	$Id: wbtag.h,v 36.2 90/07/16 17:22:28 peter Exp $
**
**	Workbench load message
**
**	(C) Copyright 1985,1986,1987,1988,1989,1990, Commodore-Amiga, Inc.
**	All Rights Reserved
*/

#ifndef EXEC_PORTS_H
#include "exec/ports.h"
#endif

struct LoadWBMsg {
    struct Message	lm_Message;
    ULONG		lm_Path;
    ULONG		lm_Arg;
};

#endif
