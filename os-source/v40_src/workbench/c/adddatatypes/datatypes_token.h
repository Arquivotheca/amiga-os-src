/* token.h
 *
 */

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <dos/dos.h>

/*****************************************************************************/

struct Token
{
    struct SignalSemaphore	 t_Lock;		/* Used to lock the list */

    struct List			 t_AlphaList;		/* List of all datatypes */
    struct List			 t_BinaryList;		/* List of binary datatypes */
    struct List			 t_ASCIIList;		/* List of ASCII datatypes */
    struct List			 t_IFFList;		/* List of IFF datatypes */
    struct List			 t_AuxList;		/* Auxilliary list */

    ULONG			 t_NumEntries;		/* Number of entries in the list */
    ULONG			 t_MaxLen;		/* Max length need for uniqueness */
    ULONG			 t_Flags;		/* Flags */

    struct DateStamp		 t_Date;		/* When last read in */
};

/*****************************************************************************/

#define	TOKEN_NAME	"DataTypesList"
#define	TKSIZE		(sizeof(struct Token))

/*****************************************************************************/

/* No need to test pattern */
#define	DTSF_MATCHALL	0x00000001

/* Pattern contains wildcards */
#define	DTSF_ISWILD	0x00000002

/* Already sorted */
#define	DTSF_SORTED	0x00000004

