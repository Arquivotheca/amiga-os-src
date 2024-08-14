
/*
 * inetconfig.h     -- Tag defintions for the ConfigureInet() function
 *
 * $id$
 *
 */

#include <utility/tagitem.h>

/* Tags */
/* INET_Gateway specifies whether inet.library should act as an IP gateway and
 * forward IP packets or merely ignore packets that don't belong to the given
 * machine.  ti_Data of TRUE turns gateway code ON.  ti_Data of FALSE turns
 * gateway code OFF.
 */
#define INET_Dummy          (TAG_USER + 0xB8000)
#define INET_Gateway        (INET_Dummy + 1)
#define INET_Query          (INET_Dummy + 2)

struct InetQuery
{
    BOOL        iq_Gateway;
};




