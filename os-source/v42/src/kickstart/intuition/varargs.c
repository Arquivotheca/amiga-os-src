/*** varargs.c *****************************************************************
 *
 * varargs.c -- converts variable arguments to array pointers, mostly
 *
 *  $Id: varargs.c,v 40.0 94/02/15 17:53:10 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"

/*****************************************************************************/

ULONG SetGadgetAttrs (struct Gadget *g, struct Window *w, struct Requester *req, ULONG tag1, ...)
{
    return (SetGadgetAttrsA (g, w, req, (struct TagItem *)&tag1));
}

/*****************************************************************************/

ULONG SetAttrs (Object *o, ULONG tag1, ...)
{
    return (SetAttrsA (o, (struct TagItem *) &tag1));
}

/*****************************************************************************/

Object *NewObject (Class *cl, ClassID classid, ULONG tag1, ...)
{
    return (NewObjectA (cl, classid, (struct TagItem *) &tag1));
}

/*****************************************************************************/

ULONG NotifyAttrChanges (Object *o, void *ginfo, ULONG flags, ULONG tag1, ...)
{
    return (SendMessage (o, OM_NOTIFY, (struct TagItem *) &tag1, ginfo, flags));
}

/*****************************************************************************/

/* passed a varargs attribute taglist */
ULONG mySetSuperAttrs (Class *cl, Object *o, ULONG t1, ...)
{
    return (SendSuperMessage (cl, o, OM_SET, (struct TagItem *) &t1, NULL));
}
