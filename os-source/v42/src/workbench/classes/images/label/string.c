/*
 *  $Id$
 *
 *  label.image
 *  String Handling Module
 *  Copyright (C) 1994 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *  Unauthorized Duplication or Distribution Prohibited
 *
 */

/*
$Log$
*/

/*****************************************************************************
 *
 *  Includes
 *
 *****************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>

#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/cghooks.h>

#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/macros.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <string.h>

#include "classbase.h"
#include "classdata.h"

/*****************************************************************************
 *
 *  createString()  -   create dynamic string
 *
 *****************************************************************************/

STRPTR createString (struct ClassLib *ClassBase, STRPTR source)
{

    int length;
    STRPTR string;

    length=strlen(source)+1;
    string=AllocVec(length, NULL);
    if (string) {

        strcpy(string, source);

    }

    return string;

}

/*****************************************************************************
 *
 *  destroyString()  -   destroy dynamic string
 *
 *****************************************************************************/

void destroyString (struct ClassLib *ClassBase, STRPTR string)
{
    FreeVec(string);
}
