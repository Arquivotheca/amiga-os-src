
/* Purpose: This file contains the buffer management routines of the
 *          postscript printer driver. As of V39, printer.device only allows
 *	    the convfunc to output 200 characters on every invocation. The
 *	    routines in this file manage buffers to keep track of data waiting
 *	    to be output.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <devices/prtbase.h>
#include <devices/printer.h>
#include <string.h>

#include <clib/exec_protos.h>

#include <pragmas/exec_pragmas.h>

#include "driver.h"
#include "textbuf.h"


/*****************************************************************************/


struct Buffer
{
    struct MinNode bf_Link;
    LONG           bf_Amount;             /* amount of data in buffer */
    UBYTE          bf_Data[DRAIN_SIZE];
};


/*****************************************************************************/


extern struct PrinterData *PD;
extern struct Library     *SysBase;

static struct MinList buffers;
static struct MinList cache;

BOOL bufferHasBytes;


/*****************************************************************************/


static VOID NewList(struct MinList *list)
{
    list->mlh_Head     = (struct MinNode *)&list->mlh_Tail;
    list->mlh_Tail     = NULL;
    list->mlh_TailPred = (struct MinNode *)list;
}


/*****************************************************************************/


VOID InitTextBuf(VOID)
{
    NewList((struct List *)&buffers);
    NewList((struct List *)&cache);
    bufferHasBytes = FALSE;
}


/*****************************************************************************/


/* Add the provided data to the end of the output queue */
BOOL PSWrite(const STRPTR buf)
{
struct Buffer *bf;
ULONG          len;
ULONG          left;
ULONG          amount;
ULONG          i;

    i = 0;
    len = strlen(buf);

    if (!IsListEmpty((struct List *)&buffers))
    {
        bf   = (struct Buffer *)buffers.mlh_TailPred;   /* last buffer */
        left = DRAIN_SIZE - bf->bf_Amount;              /* # bytes free in last buffer */

        if (left)
        {
            if (len <= left)
            {
                /* just copy the data into the last buffer, and return */
                CopyMem(buf,&bf->bf_Data[bf->bf_Amount],len);
                bf->bf_Amount += len;
                return(TRUE);
            }

            /* fill up the last buffer, and enter main loop */
            CopyMem(buf,&bf->bf_Data[bf->bf_Amount],left);
            bf->bf_Amount = DRAIN_SIZE;
            len -= left;
            i    = left;
        }
    }

    while (len)
    {
        if (!(bf = (struct Buffer *)RemHead((struct List *)&cache))) /* check cache */
            if (!(bf = AllocVec(sizeof(struct Buffer),MEMF_ANY)))    /* or get mem */
                return(FALSE);                                       /* no mem */

        amount = len;
        if (amount > DRAIN_SIZE)
            amount = DRAIN_SIZE;

        CopyMem(&buf[i],bf->bf_Data,amount);
        bf->bf_Amount  = amount;
        len           -= amount;
        i             += amount;

        AddTail((struct List *)&buffers,(struct Node *)bf);
    }

    bufferHasBytes = TRUE;

    return(TRUE);
}


/*****************************************************************************/


/* Removes as many as DRAIN_SIZE bytes from the HEAD of the output queue,
 * and puts them into "buf".
 */

LONG PSDrain(STRPTR buf)
{
struct Buffer *bf;

    if (bf = (struct Buffer *)RemHead((struct List *)&buffers))
    {
        CopyMem(bf->bf_Data,buf,bf->bf_Amount);
        AddTail((struct List *)&cache,(struct Node *)bf);
        return(bf->bf_Amount);
    }

    bufferHasBytes = FALSE;

    return (0);
}


/*****************************************************************************/


/* Writes out data in the output queue to the printer */
LONG PSFlush(VOID)
{
struct Buffer *bf;
LONG           result;

    result = PDERR_NOERR;

    while (bf = (struct Buffer *)RemHead((struct List *)&buffers))
    {
        result = PWRITE(bf->bf_Data,bf->bf_Amount);

        /* Wait for it to get output */
        PWAIT();
        PWAIT();

        FreeVec(bf);

        if (result != PDERR_NOERR)
        {
            while (bf = (struct Buffer *)RemHead((struct List *)&buffers))
                FreeVec(bf);

            break;
        }
    }

    while (bf = (struct Buffer *)RemHead((struct List *)&cache))
        FreeVec(bf);

    return (result);
}
