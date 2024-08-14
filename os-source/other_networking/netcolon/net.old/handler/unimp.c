/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1990       The Software Distillery.  All Rights  */
/* |. o.| || Reserved.  This program may not be distributed without the    */
/* | .  | || permission of the authors:                            BBS:    */
/* | o  | ||                   Doug Walker                                 */
/* |  . |//                                                                */
/* ======                                                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Unimplemented new 2.0 packets */

#include "handler.h"


static int doit = 0; /* To suppress the requesters automatically, use 0 here */

#define unimp(x) if(doit) doit=request((GLOBAL)global, REQ_GENERAL, x)


#define UNIMP(name) ACTFN(name) { unimp("2.0 pkt " #name); HPQ(global, hpkt); }

UNIMP(ActFormat        )
UNIMP(ActFHFromLock    )
UNIMP(ActChangeMode    )
UNIMP(ActCopyDirFH     )
UNIMP(ActParentFH      )
UNIMP(ActExamineAll    )
UNIMP(ActExamineFH     )
UNIMP(ActSameLock      )

