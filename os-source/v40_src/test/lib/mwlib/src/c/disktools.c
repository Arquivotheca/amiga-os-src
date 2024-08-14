/******************************************************************************
    DiskTools.c

    Mitchell/Ware Systems           Version 1.01                12/6/89

    Various useful Disk/filing system tools.

-------------------------------------------------------------------------------

    BOOL GetPathname(lock, buffer, size)    get full pathname of buffer, return TRUE
                                            if successful

******************************************************************************/

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

BOOL _GetPathname(lock, pbuf, psiz)  /* return the pathname for a given lock */
ULONG lock;     /* lock to process */
UBYTE **pbuf;   /* ptr to ptr to buffer to place the result in */
ULONG *psiz;    /* ptr to size of buffer */
{
    struct FileInfoBlock *fib = NULL;
    struct InfoData *id = NULL;
    struct DeviceList *dl;
    ULONG mommy;    /* Parent lock */
    BOOL ret = TRUE;
    int fsiz; /* size of string in fib */
    UBYTE *s;   /* general purpose pointer */

    fib = calloc(sizeof(*fib), 1);
    id = calloc(sizeof(*id), 1);

    if (!fib || !id)
        fatal("GetPathname: Out Of Memory!!!!");

    /* now check to see if this lock has a mommy...
    */
    if (mommy = ParentDir(lock)) /* I have a mommy!!! */
    {
        if (_GetPathname(mommy, pbuf, psiz) && Examine(lock, fib))
        {
            /* Put current name in buffer, adjusting the buffer pointer
               and the size.

               NOTE: A nasty fence-post problem exists with copying
                     strings to buffers of known lengths. The following
                     IF statement handles that condition properly (we hope),
                     returning and error if buffer will be overrun.
            */
            if ((fsiz = strlen(fib->fib_FileName)+1) <= (*psiz)) /* do we have room? */
            {
                strcpy(*pbuf, fib->fib_FileName);
                *pbuf += fsiz;
                (*pbuf)[-1] = '/';
                (*pbuf)[0] = '\0';
                *psiz -= fsiz;
            }
            else /* nope - no more buffer space */
                ret = FALSE;
        }
        else /* some kind of error */
            ret = FALSE;
    }
    else if (Info(lock, id)) /* I'm a root, so let's see if we can get the device name! */
    {
        dl = BADDR(id->id_VolumeNode);
        s = BSTRtoSTR(dl->dl_Name); /* remember to de-allocate this! */
        if ((fsiz = strlen(s)+1) <= (*psiz))
        {
            strcpy(*pbuf, s);
            *pbuf += fsiz;
            (*pbuf)[-1] = ':';
            (*pbuf)[0] = '\0';
            *psiz -= fsiz;
        }
        else /* nope - no more buffer space */
            ret = FALSE;
        free(s);
    }
    else /* something's wrong, folks!!!! */
        ret = FALSE;

    /* clean up and get the hell out of here!
    */
    free(fib);
    free(id);
    if (mommy)
        UnLock(mommy);
    return ret;
}

BOOL GetPathname(lock, buf, size)
ULONG lock;
UBYTE *buf;
ULONG size;
{
    if (_GetPathname(lock, &buf, &size) && *--buf == '/')
    {
        *buf = '\0'; /* kill the extra '/' */
    }
}
