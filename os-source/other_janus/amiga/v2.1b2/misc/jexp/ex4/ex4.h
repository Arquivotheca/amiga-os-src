#ifndef JANUS_EX4_H
#define JANUS_EX4_H

/************************************************************************
 * (Shared Amiga/PC file)
 *
 * Ex4.h - Ex4 specific data structures
 *
 * 4-13-89 - Bill Koester - Created this file on PC side
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define EX4_APPLICATION_ID 2L
#define EX4_LOCAL_ID 4

/* Ex4 request structure */

struct Ex4Req {
   UBYTE exr_Function;
   UWORD exr_Called;
   UBYTE exr_Error;
};

/* Error codes for exr_Error */

#define Exr_ERR_OK         0     /* No error                            */
#define Exr_ERR_FAILED     1     /* Misc Error                          */

#endif   /* End of JANUS_EX4_H conditional compilation                  */
