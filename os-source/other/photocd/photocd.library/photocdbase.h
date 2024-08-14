#ifndef LIBRARIES_PHOTOCDBASE_H
#define LIBRARIES_PHOTOCDBASe_H

/*
**
**      Photo CD library base
**
**      $Id: photocdbase.h,v 40.3 94/01/13 11:39:46 jjszucs Exp $
**
**      (C) Copyright 1993 Commodore-Amiga Inc.
**      All Rights Reserved
*/

#ifndef	EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef LIBRARIES_PHOTOCD_H
#include <libraries/photocd.h>
#endif /* LIBRARIES_PHOTOCD_H */

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif /* DOS_DOS_H */

/********************************************************************
 *                                                                  *
 * Photo CD library base                                            *
 *                                                                  *
 ********************************************************************/

struct PhotoCDLibrary {

    struct Library      LibNode;        /* Library node */

    UWORD               UsageCnt;       /* Usage count */
    BPTR                SegList;        /* Segment list */

    struct Library *    ExecBase;       /* exec.library base */
    struct Library *    DOSBase;        /* dos.library base */
    struct Library *    UtilityBase;    /* utility.library base */

    UWORD               ResWidth[PHOTOCD_RES_COUNT];
                                        /* Resolution width table */
    UWORD               ResHeight[PHOTOCD_RES_COUNT];
                                        /* Resolution height table */

};

#endif /* LIBRARIES_PHOTOCDBASE_H */
