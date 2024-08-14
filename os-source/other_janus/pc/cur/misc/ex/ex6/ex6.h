#ifndef JANUS_EX6_H
#define JANUS_EX6_H

/************************************************************************
 * (Shared Amiga/PC file)
 *
 * Ex6.h - Ex6 specific data structures
 *
 * 4-13-89 - Bill Koester - Created this file on Amiga side
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define EX6_APPLICATION_ID 2L
#define EX6_LOCAL_ID 6

/* Ex6 request structure */

struct Ex6Req {
   RPTR exr_RememberKey1;
   RPTR exr_RememberKey2;
   RPTR exr_RememberKey3;
};

#endif   /* End of JANUS_EX6_H conditional compilation                  */
