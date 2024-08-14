/*** varargs.c *****************************************************************
 *
 * varargs.c -- converts variable arguments to array pointers, mostly
 *
 *  $Id: varargs.c,v 38.0 91/06/12 14:34:25 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include "intuall.h"
#include "classusr.h"
#include "classes.h"

SetGadgetAttrs( g, w, req, tag1 )
struct Gadget	*g;
struct Window	*w;
struct Requester	*req;
ULONG		tag1;
{
    return ( SetGadgetAttrsA( g, w, req, &tag1 ) );
}

SetAttrs( o, tag1 )
Object	*o;
ULONG	tag1;
{
    return ( SetAttrsA( o, &tag1 ) );
}

Object   *
NewObject( cl, classid, tag1, data1 )
Class	*cl;
ClassID classid;
ULONG	tag1;
{
    Object   *NewObjectA();

    return( NewObjectA( cl, classid, &tag1 ) );
}

NotifyAttrChanges( o, ginfo, flags, tag1 )
Object	*o;
void	*ginfo;
ULONG	flags;
ULONG	tag1;
{
    return ( SendMessage( o, OM_NOTIFY, &tag1, ginfo, flags ) );
}

/* passed a varargs attribute taglist */
SetSuperAttrs( cl, o, t1, d1, t2, d2 )
Class		*cl;
Object		*o;
ULONG		t1;
{
    return ( SendSuperMessage( cl, o, OM_SET, &t1, NULL ) );
}

