
/*
 * Configure.c - Allow configuration of inet.library.
 *
 * $id$
 *
 */

#include <exec/types.h>
#include <netinet/inetconfig.h>
#include <utility/tagitem.h>
#include <pragmas/utility_pragmas.h>
#include <clib/utility_protos.h>
#include "proto.h"


struct Library *UtilityBase;
extern int ipforwarding;

/****** inet.library/ConfigureInet ******************************************
*
*   NAME
*     ConfigureInetA -- Set or change inet.library configuration states
*
*   SYNOPSIS
*     ConfigureInetA(taglist)
*                     A0
*
*     VOID ConfigureInetA(struct TagItem *);
*
*   FUNCTION
*     Allows the caller to manipulate certain inet.library operating
*     parameters, such as whether inet operates as a gateway or not.
*
*   INPUTS
*     taglist - A taglist containing tags defined in <netinet/inetconfig.h>
*
*   RESULT
*     None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

void __asm __saveds ConfigureInetA(register __a0 struct TagItem *tl)
{

    struct TagItem *ts;
    struct TagItem *tag;

    ts = tl;
    while (tag = NextTagItem(&ts))
    {
        switch (tag->ti_Tag)
        {
            case INET_Gateway:                  /* Turn inet.library gateway code on or off */
            {
                ipforwarding = tag->ti_Data;
                break;
            }
            case INET_Query:
            {
                struct InetQuery *iq;
                iq = tag->ti_Data;
                iq->iq_Gateway = (BOOL) ipforwarding;
                break;
            }

        }
    }
}


