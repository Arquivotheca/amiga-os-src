/*** photocdbase.c **********************************************************
 *                                                                          *
 *  $Id: photocdbase.c,v 40.4 94/01/13 11:39:02 jjszucs Exp $
 *  photocd.library                                                         *
 *  Library Base                                                            *
 *                                                                          *
 *  Confidential Information: Commodore-Amiga, Inc.                         *
 *  Copyright © 1993 Commodore-Amiga, Inc.                                  *
 *                                                                          *
 ****************************************************************************/

/*
$Log:	photocdbase.c,v $
 * Revision 40.4  94/01/13  11:39:02  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 * 
 * Revision 40.3  93/11/19  17:42:15  jjszucs
 * Down-coded pixel copy and YCC-to-RGB conversion to assembly language
 *
 * Revision 40.2  93/11/18  20:01:03  jjszucs
 * Added RCS id and log substitions
 *
*/

#include <exec/types.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "photocd.h"
#include "photocdbase.h"

#define INTERNAL_LIBINIT
#include "internal.h"

/****************************************************************************
 *                                                                          *
 *  Prototypes                                                              *
 *                                                                          *
 ****************************************************************************/

struct Library * ASM LibInit (REG (d0) struct PhotoCDLibrary *pcdBase,
    REG (a0) BPTR segList,
    REG (a6) struct Library *SysBase);
LONG ASM LibOpen (REG (a6) struct PhotoCDLibrary *pcdBase);
LONG ASM LibClose (REG (a6) struct PhotoCDLibrary *pcdBase);
LONG ASM LibExpunge (REG (a6) struct PhotoCDLibrary *pcdBase);

/****************************************************************************
 *                                                                          *
 *  LibInit()   -   library initialization routine                          *
 *                                                                          *
 ****************************************************************************/

struct Library * ASM LibInit (REG (d0) struct PhotoCDLibrary *pcdBase,
    REG (a0) BPTR segList,
    REG (a6) struct Library *SysBase)
{

    UBYTE resolution;
    UWORD width, height;

#ifdef DEBUG
    kprintf("LibInit():\n");
    kprintf("   pcdBase=$%08lx\n",pcdBase);
    kprintf("   segList=$%08lx\n",segList);
    kprintf("   SysBase=$%08lx\n",SysBase);
#endif /* DEBUG */

    /* Copy exec.library base pointer */
    pcdBase->ExecBase=SysBase;

    /* Open dos.library */
    pcdBase->DOSBase=OpenLibrary("dos.library",KICKSTART_VERSION);
    if (!pcdBase->DOSBase) {
#ifdef DEBUG
        kprintf("   dos.library V%ld open failed\n",KICKSTART_VERSION);
        kprintf("   Return\n");
#endif /* DEBUG */
        return NULL;
    }

    /* Open utility.library */
    pcdBase->UtilityBase=OpenLibrary("utility.library",KICKSTART_VERSION);
    if (!pcdBase->UtilityBase) {
#ifdef DEBUG
        kprintf("   utility.library V%ld open failed\n",KICKSTART_VERSION);
        kprintf("   Return\n");
#endif /* DEBUG */
        return NULL;
    }

    /* Store segment list */
    pcdBase->SegList=segList;

    /* Initialize resolution tables */
    width=PHOTOCD_BASE_WIDTH/4;
    height=PHOTOCD_BASE_HEIGHT/4;
    for (resolution=PHOTOCD_RES_BASE16;
         resolution<=PHOTOCD_RES_16BASE;
         resolution++) {
         pcdBase->ResWidth[resolution-1]=width;
         pcdBase->ResHeight[resolution-1]=height;
         width*=2;
         height*=2;
    }

    /* Return library base */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   pcdBase=$%08lx\n",pcdBase);
#endif /* DEBUG */
    return pcdBase;

}

/****************************************************************************
 *                                                                          *
 *  LibOpen()   -   library open routine                                    *
 *                                                                          *
 ****************************************************************************/

LONG ASM LibOpen (REG (a6) struct PhotoCDLibrary *pcdBase)
{

    /* Increment use counter */
    pcdBase->UsageCnt++;

    /* Clear delayed expunge flag */
    pcdBase->LibNode.lib_Flags&=~LIBF_DELEXP;

    return (LONG) pcdBase;

}

/****************************************************************************
 *                                                                          *
 *  LibClose()   -   library close routine                                  *
 *                                                                          *
 ****************************************************************************/

LONG ASM LibClose (REG (a6) struct PhotoCDLibrary *pcdBase)
{

#ifdef DEBUG
    kprintf("LibClose():\n");
    kprintf("   Entry\n");
    kprintf("   pcdBase=$%08lx\n",pcdBase);
#endif /* DEBUG */

    /* Decrement usage count */
    if (pcdBase->UsageCnt) {
        pcdBase->UsageCnt--;
#ifdef DEBUG
    kprintf("   UsageCnt=%ld\n",pcdBase->UsageCnt);
#endif /* DEBUG */
    }

    /* If delayed expunge ... */
    if (pcdBase->LibNode.lib_Flags & LIBF_DELEXP) {
#ifdef DEBUG
    kprintf("   Delayed expunge\n");
    kprintf("   Return\n");
#endif /* DEBUG */
        /* Expunge */
        return LibExpunge(pcdBase);
    }

#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */
    return NULL;

}

/****************************************************************************
 *                                                                          *
 *  LibExpunge()   -   library expunge routine                              *
 *                                                                          *
 ****************************************************************************/

LONG ASM LibExpunge (REG (a6) struct PhotoCDLibrary *pcdBase)
{

    BPTR segList;

#ifdef DEBUG
    kprintf("LibExpunge():\n");
    kprintf("   Entry\n");
    kprintf("   pcdBase=$%08lx\n",pcdBase);
#endif /* DEBUG */

    /* If library is in use ... */
    if (pcdBase->UsageCnt) {
#ifdef DEBUG
    kprintf("   Expunge delayed\n");
#endif /* DEBUG */
        /* Set delayed expunge flag and return */
        pcdBase->LibNode.lib_Flags|=LIBF_DELEXP;
#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */
        return NULL;
    }

    /* Fetch segment list */
    segList=pcdBase->SegList;

    /* Remove library base */
    Remove ((struct Node *) pcdBase);

    /* Close libraries */
    CloseLibrary(pcdBase->DOSBase);
    pcdBase->DOSBase=NULL;
    CloseLibrary(pcdBase->UtilityBase);
    pcdBase->UtilityBase=NULL;

    /* Free library base */
    FreeMem (
        (APTR) ((ULONG) (pcdBase) - (ULONG) (pcdBase->LibNode.lib_NegSize)),
        pcdBase->LibNode.lib_NegSize + pcdBase->LibNode.lib_PosSize);

    /* Return segment list for dos.library/UnloadSeg() */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   segList=$%08lx\n",segList);
#endif /* DEBUG */
    return (LONG) segList;

}
