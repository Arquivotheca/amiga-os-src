/*
 * Amiga Grand Wack
 *
 * envoylink.h - Definitions
 *
 * $Id: envoylink.h,v 39.4 93/05/07 17:04:30 peter Exp $
 *
 */

struct RemoteWackCommand
{
    ULONG rwc_Action;	/* Command */
    void *rwc_Address;	/* Address to affect */
    ULONG rwc_Data;	/* Depending on cmd, byte, word, or long may be used */
};

/* RemoteWackCommand actions: */

#define RWCA_FORBID		1	/* no params */
#define RWCA_PERMIT		2	/* no params */
#define RWCA_READBYTE		3	/* rwc_Address = address */
#define RWCA_READWORD		4	/* rwc_Address = address */
#define RWCA_READLONG		5	/* rwc_Address = address */
#define RWCA_READBLOCK		6	/* rwc_Address = address, rwc_Data = block size */
#define RWCA_WRITEBYTE		7	/* rwc_Address = address, rwc_Data = write-data */
#define RWCA_WRITEWORD		8	/* rwc_Address = address, rwc_Data = write-data */
#define RWCA_WRITELONG		9	/* rwc_Address = address, rwc_Data = write-data */
#define RWCA_GETVALIDMEM	10	/* no params */
#define RWCA_GETVALIDMEMCOUNT	11	/* no params */
#define RWCA_READSTRING		12	/* rwc_Address = address rwc_Data = maxlen */
#define RWCA_READNODEARRAY	13	/* rwc_Address = address rwc_Data = maxlen */

/* Specific error: a Node count of this value indicates the buffer size was
 * too small
 */
#define RWCA_RNA_TOOSMALL	((struct Node *)~0)

#define MYENVOYERR_ALLOCFAIL	1001	/* remote_error for allocation failure */
#define MYENVOYERR_LOSTCONTACT	1002	/* remote_error for broken connection */
