/* hosthandle.h
 *
 */

#include <exec/types.h>
#include <utility/hooks.h>
#include <intuition/classusr.h>

/*****************************************************************************/

struct HostHandle
{
    struct Hook		 hh_Dispatcher;			/* Dispatcher */
    ULONG		 hh_Reserved;			/* Must be 0 */
    ULONG		 hh_Flags;
    ULONG		 hh_UseCnt;			/* Number of open nodes */
    APTR		 hh_SystemData;			/* Reserved for system use */
    APTR		 hh_UserData;			/* Anything you want... */
    Object		*hh_DB;				/* Database for host */
};

#define	HHSIZE	(sizeof(struct HostHandle))

