/*** stubs.c *****************************************************************
 *
 *  $Id: stubs.c,v 40.3 94/01/13 11:39:22 jjszucs Exp $
 *  photocd.library
 *  Internal Variable Argument Stubs Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:	stubs.c,v $
 * Revision 40.3  94/01/13  11:39:22  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 * 
 * Revision 40.2  93/11/18  20:01:19  jjszucs
 * Added RCS id and log substitions
 *
*/

/*
 *  Amiga includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>

#include <utility/tagitem.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*
 *  ANSI includes
 */

#include <stdio.h>
#include <string.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/****************************************************************************
 *                                                                          *
 *  AllocPCDImageBuffer(resolution,Tag firstTag,...) - variable             *
 *      arguments stub for AllocPCDImageBuffer()                            *
 *                                                                          *
 ****************************************************************************/

UBYTE *AllocPCDImageBuffer(struct PhotoCDLibrary *PhotoCDBase,UBYTE resolution,Tag firstTag,...)
{

    return AllocPCDImageBufferA(PhotoCDBase,resolution,(struct TagItem *) &firstTag);

}

/****************************************************************************
 *                                                                          *
 *  GetPCDImageData(void *pcdHandle, UBYTE *imageBuffer,                    *
 *      Tag firstTag, ...) - variable arguments stub for GetPCDImageDataA() *
 *                                                                          *
 ****************************************************************************/

BOOL GetPCDImageData(struct PhotoCDLibrary *PhotoCDBase,void *pcdHandle, UBYTE *imageBuffer, Tag firstTag,...)
{

    return GetPCDImageDataA(PhotoCDBase,pcdHandle,imageBuffer,(struct TagItem *) &firstTag);

}

/****************************************************************************
 *                                                                          *
 *  OpenPhotoCD(Tag firstTag,...) - variable arguments stub                 *
 *      for OpenPhotoCDA()                                                  *
 *                                                                          *
 ****************************************************************************/

void *OpenPhotoCD(struct PhotoCDLibrary *PhotoCDBase,Tag firstTag,...)
{

    return OpenPhotoCDA(PhotoCDBase,(struct TagItem *) &firstTag);

}

/****************************************************************************
 *                                                                          *
 *  ObtainPhotoCDInfo(void *pcdHandle,Tag firstTag,...) -                   *
 *      variable arguments stub for ObtainPhotoCDInfoA()                    *
 *                                                                          *
 ****************************************************************************/

struct TagItem *ObtainPhotoCDInfo(struct PhotoCDLibrary *PhotoCDBase,void *pcdHandle,Tag firstTag,...)
{

    return ObtainPhotoCDInfoA(PhotoCDBase,pcdHandle,(struct TagItem *) &firstTag);

}
