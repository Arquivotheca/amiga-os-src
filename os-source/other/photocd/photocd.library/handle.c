/*** handle.c ***************************************************************
 *
 *  $Id: handle.c,v 1.4 94/01/13 11:37:30 jjszucs Exp Locker: jjszucs $
 *
 *  photocd.library
 *  Handle Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
$Log:   handle.c,v $
 * Revision 1.4  94/01/13  11:37:30  jjszucs
 * Changed structure tag of Photo CD library base to PhotoCDLibrary
 * Eliminated (bogus, IMHO) warnings caused by assigning ~0 to
 *     UWORD values
 *
 * Revision 1.3  93/11/29  17:42:57  jjszucs
 * Pixel copy and interpolation and YCC-to-RGB conversion functions
 * completely re-written.
 * PCD_GammaCorrect tag of GetPCDImageDataA() allows caller to
 * select gamma-corrected or uncorrected RGB data for PHOTOCD_FORMAT_RGB.
 * Increased size of Image Component Data (ICD) cache to 12 lines of
 * Y component data and 6 lines of C1 and C2 component data; this most
 * closely matches the 14-sector pre-fetch of cd.device for a Base
 * (768x512) image.
 * Eliminated vulnerabilities in ICD cache hit/miss detection code for
 * PCD_File source in GetPCDImageDataA().
 * OpenPhotoCD() now invalidates ICD cache when handle is initialized.
 *
 * Revision 1.2  93/11/18  19:59:53  jjszucs
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

#include <string.h>

/*
 *  Local includes
 */

#include "photocd.h"
#include "photocdbase.h"

#include "internal.h"

/****** photocd.library/OpenPhotoCD ******************************************
*
*   NAME
*       OpenPhotoCD()   -   open Photo CD
*
*   SYNOPSIS
*       void *OpenPhotoCD(Tag firstTag,...);
*       void *OpenPhotoCDA(struct TagItem *tags);
*
*       pcdHandle=OpenPhotoCD(firstTag,...);
*       pcdHandle=OpenPhotoCDA(tags);
*
*   FUNCTION
*       Open Photo CD handle. This handle is passed to other photocd.library
*       functions (such as ObtainPhotoCDInfo()) that need to access the
*       Photo CD filesystem structure.
*
*   INPUTS
*       tags            -   tag array containing additional parameters
*
*   INPUT TAGS
*       All tags are optional unless otherwise indicated.
*
*       PCD_ErrorCode       (ULONG *) Pointer to ULONG where error code
*                           (one of PHOTOCD_ERR_*) is deposited if this
*                           function fails. This variable is unchanged
*                           if the operation is successful.
*
*       PCD_Source          (STRPTR) Path to Photo CD filesystem structure.
*                           As an example, if the Photo CD filesystem
*                           structure is in the directory PhotoCD/Disc2
*                           of DOS device HD1:, the value for this tag
*                           is "HD1:PhotoCD/Disc2." The default is
*                           "CD0:", which is the first CD-ROM drive device.
*
*   RESULT
*       pcdHandle       -   Photo CD handle; NULL if failure
*
*   EXAMPLE
*
*   NOTES
*       The caller must be a process (not a task) for this and related
*       functions, which are dependent on dos.library calls, to operate
*       correctly.
*
*       The Photo CD handle returned is private to photocd.library. Do
*       not examine or modify it.
*
*   BUGS
*
*   SEE ALSO
*       ClosePhotoCD()
*
******************************************************************************
*
*/

void ASM *OpenPhotoCDA(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) struct TagItem *tags)
{

    struct PhotoCDHandle *pHandle;

    char *sourcePath;

    ULONG errorCode;
    ULONG *pErrorCode;

    BPTR oldCurrentDir;

    /*
     *  Initialization
     */

#ifdef DEBUG
    kprintf("OpenPhotCDA():\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   tags=$%08lx\n",tags);
#endif /* DEBUG */

    /* No error */
    errorCode=PHOTOCD_ERR_NONE;

    /*
     *  Create handle
     */

    /* Allocate handle;
       memory is cleared to initialize cached data as invalid */
    pHandle=AllocVec(sizeof(*pHandle),MEMF_CLEAR);
    if (pHandle) {

        /* Determine source path */
        sourcePath=(char *) GetTagData(PCD_Source,(ULONG) PHOTOCD_DEFAULT_SOURCE,tags);

        /* Lock source path */
        pHandle->sourceLock=Lock(sourcePath,ACCESS_READ);
        if (pHandle->sourceLock) {

            /* Open PHOTO_CD/INFO.PCD */
            oldCurrentDir=CurrentDir(pHandle->sourceLock);
            if (!(pHandle->infoHandle=Open("PHOTO_CD/INFO.PCD",MODE_OLDFILE))) {

                switch (IoErr()) {

                    case ERROR_NO_DISK:
                        errorCode=PHOTOCD_ERR_NO_DISC;
                        break;

                    case ERROR_OBJECT_NOT_FOUND:
                        errorCode=PHOTOCD_ERR_DATA_FORMAT;
                        break;

                    default:
                        errorCode=PHOTOCD_ERR_DOS;
                        break;

                }

            }

            /* Restore current directory */
            CurrentDir(oldCurrentDir);

            /* Clear cache key */
            pHandle->cacheImage=0;
            pHandle->cacheRes=Largest(UBYTE);
            pHandle->cacheLine=Largest(UWORD);

        } else {

            switch (IoErr()) {

                case ERROR_NO_DISK:
                    errorCode=PHOTOCD_ERR_NO_DISC;
                    break;

                case ERROR_OBJECT_NOT_FOUND:
                    errorCode=PHOTOCD_ERR_DATA_FORMAT;
                    break;

                default:
                    errorCode=PHOTOCD_ERR_DOS;
                    break;

            }

        }

    } else {

        errorCode=PHOTOCD_ERR_MEMORY;

    }

    /* If error ... */
    if (errorCode!=PHOTOCD_ERR_NONE) {

        /* Close handle */
        ClosePhotoCD(PhotoCDBase,pHandle);
        pHandle=NULL;

        /* Return error code */
        pErrorCode=(ULONG *) GetTagData(PCD_ErrorCode,NULL,tags);
        if (pErrorCode) {
            *pErrorCode=errorCode;
        }

    }

    /* Return handle (NULL if error) */
#ifdef DEBUG
    kprintf("   Return\n");
    kprintf("   pHandle=$%08lx\n",pHandle);
#endif /* DEBUG */
    return pHandle;

}

/****** photocd.library/ClosePhotoCD ******************************************
*
*   NAME
*       ClosePhotoCD()      -   close Photo CD handle
*
*   SYNOPSIS
*       void ClosePhotoCD(void *pcdHandle);
*
*       ClosePhotoCD(pcdHandle);
*
*   FUNCTION
*       Close Photo CD handle opened with OpenPhotoCD().
*
*   INPUTS
*       pcdHandle           -   Photo CD handle returned by OpenPhotoCD();
*                               passing NULL for this argument is safe
*
*   RESULT
*       None
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       OpenPhotoCD()
*
******************************************************************************
*
*/

void ASM ClosePhotoCD(REG (a6) struct PhotoCDLibrary *PhotoCDBase,
    REG (a0) void *pcdHandle)
{

    struct PhotoCDHandle *pHandle;

#ifdef DEBUG
    kprintf("ClosePhotoCD():\n");
    kprintf("   Entry\n");
    kprintf("   PhotoCDBase=$%08lx\n",PhotoCDBase);
    kprintf("   pcdHandle=$%08lx\n",pcdHandle);
#endif /* DEBUG */

    pHandle=pcdHandle;

    /* If handle passed ... */
    if (pHandle) {

        /* Close PHOTO_CD/INFO.PCD */
        if (pHandle->infoHandle) {
            Close(pHandle->infoHandle);
        }

        /* Unlock source filesystem */
        if (pHandle->sourceLock) {
            UnLock(pHandle->sourceLock);
        }

        /* Free handle */
        FreeVec(pHandle);

    }

#ifdef DEBUG
    kprintf("   Return\n");
#endif /* DEBUG */

}
