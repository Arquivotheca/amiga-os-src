#ifndef JANUS_EX5_H
#define JANUS_EX5_H

/************************************************************************
 * (Shared Amiga/PC file)
 *
 * Ex5.h - Ex5 specific data structures
 *
 * 9-12-89 - Bill Koester - Created this file on PC side
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define EX5_APPLICATION_ID 2L
#define EX5_LOCAL_ID 5

/* Ex5 request structure */

struct Ex5Req {
   UBYTE exr_Lock;
};

#endif   /* End of JANUS_EX5_H conditional compilation                  */
