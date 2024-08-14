/*****************************************************************************
*
*	$Id: nonvolatilebase.h,v 40.4 93/05/06 14:07:18 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	nonvolatilebase.h,v $
 * Revision 40.4  93/05/06  14:07:18  brummer
 * Add nv_Flags to base to allow for multicommand option.
 * 
 * Revision 40.3  93/03/08  14:04:13  brummer
 * Add InitDisk Semaphore to lib base
 *
 * Revision 40.2  93/03/05  12:50:31  brummer
 * Add storage location for lowlevel libraray base pointer
 *
 * Revision 40.1  93/03/01  15:11:14  brummer
 * Added ptr for utility.library base.
 * Added semaphore for NVRAM device access
 *
 * Revision 40.0  93/02/19  15:30:17  Jim2
 * Changed INCLUDE that prevents this file from being included multiple times.
 *
 * Revision 39.1  93/02/03  11:17:02  Jim2
 * *** empty log message ***
 *
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************/

#ifndef PRIVATE_NONVOLATILEBASE_H
#define PRIVATE_NONVOLATILEBASE_H

#ifndef EXEC_TYPES_H
    #include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
    #include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef EXEC_SEMAPHORES_H
    #include <exec/semaphores.h>
#endif

#define NONVOLATILELIBRARYNAME "nonvolatile.library"


struct NVBase
{
    struct Library          NVLibrary;
    UBYTE	nv_Pad0;
    UBYTE	nv_Flags;
    APTR	nv_SegList;
    struct Library 	*nv_ExecBase;
    struct Library	*nv_DOSBase;
    struct Library	*nv_UTILBase;
    struct Library	*nv_LowLevelBase;
    APTR	nv_NVRAMCopy;
    APTR	nv_DiskLock;
    struct SignalSemaphore  nv_Semaphore;
    struct SignalSemaphore  nv_DiskInitSema;
    };

#endif /* NONVOLATILEBASE_H */
