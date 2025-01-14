#ifndef LIBRARIES_VERSION_H
#define LIBRARIES_VERSION_H
/*
**      $Id: version.h,v 39.3 93/02/09 12:09:07 vertex Exp $
**
**      version.library interface structures and definitions
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved
*/

/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif


/*****************************************************************************/


/* This structure must only be allocated by version.library and is READ-ONLY! */
struct VersionInfo
{
    ULONG            vi_Flags;        /* see flag bit definitions below      */
    ULONG            vi_Location;     /* where this info comes from          */
    STRPTR           vi_Path;         /* full path name if from disk         */
    STRPTR           vi_InternalName; /* internal name, NULL when unknown    */
    LONG             vi_Version;      /* version number, -1 when unknown     */
    LONG             vi_Revision;     /* revision number, -1 when unknown    */
    struct DateStamp vi_Date;         /* date, fields set to -1 when unknown */
    STRPTR           vi_Comment;      /* additional comment, NULL when none  */
    char             vi_KludgeBuf[60];
};

/* constants for VersionInfo.vi_Location and for the GV_Location tag */
#define VILOCB_EXECRESIDENT  0  /* Came from the Exec resident module list */
#define VILOCB_DOSRESIDENT   1  /* Came from the DOS resident segment list */
#define VILOCB_FILESYSTEM    2  /* Came from the DOS file system list      */
#define VILOCB_LOADEDLIBRARY 3  /* Came from the Exec library list         */
#define VILOCB_LOADEDDEVICE  4  /* Came from the Exec device list          */
#define VILOCB_PATHNAME      5  /* Came from a file                        */
#define VILOCB_LIBSASSIGN    6  /* Came from a file in LIBS:               */
#define VILOCB_DEVSASSIGN    7  /* Came from a file in DEVS:               */

#define VILOCF_EXECRESIDENT  (1L << 0)
#define VILOCF_DOSRESIDENT   (1L << 1)
#define VILOCF_FILESYSTEM    (1L << 2)
#define VILOCF_LOADEDLIBRARY (1L << 3)
#define VILOCF_LOADEDDEVICE  (1L << 4)
#define VILOCF_PATH          (1L << 5)
#define VILOCF_LIBSASSIGN    (1L << 6)
#define VILOCF_DEVSASSIGN    (1L << 7)


/* Flag bits for VersionInfo.vi_Flags */
#define VIB_FOUNDITEM    0        /* found an object matching the name */
#define VIB_FOUNDINFO    1        /* found some version information    */
#define VIB_INTERNALNAME 2        /* found an internal name            */
#define VIB_VERSION      3        /* found a version number            */
#define VIB_REVISION     4        /* found a revision number           */
#define VIB_DATE         5        /* found a date                      */
#define VIB_COMMENT      6        /* found a comment                   */

#define VIF_FOUNDITEM    (1L << 0)
#define VIF_FOUNDINFO    (1L << 1)
#define VIF_INTERNALNAME (1L << 2)
#define VIF_VERSION      (1L << 3)
#define VIF_REVISION     (1L << 4)
#define VIF_DATE         (1L << 5)
#define VIF_COMMENT      (1L << 6)


/*****************************************************************************/


/* Tags for GetVersion() */
#define GV_TagBase    (TAG_USER + 0x110000)
#define GV_Kickstart  GV_TagBase+1  /* return version of Kickstart  */
#define GV_Location   GV_TagBase+2  /* limit where GetVersion looks */


/*****************************************************************************/


#endif  /* LIBRARIES_VERSION_H */
