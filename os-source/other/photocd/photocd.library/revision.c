/*** revision.c *****************************************************************
 *
 *  $Id: revision.c,v 40.3 93/11/19 17:42:25 jjszucs Exp $
 *
 *  photocd.library
 *  Revision Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:	revision.c,v $
 * Revision 40.3  93/11/19  17:42:25  jjszucs
 * Down-coded pixel copy and YCC-to-RGB conversion to assembly language
 * 
 * Revision 40.2  93/11/18  20:01:12  jjszucs
 * Added RCS id and log substitions
 *
*/

/*
 * System includes
 */

#include <exec/types.h>

/*
 * Local includes
 */

#include "photocd_rev.h"

/*
 * External definitions
 */

UBYTE *VersionTag=VERSTAG;
