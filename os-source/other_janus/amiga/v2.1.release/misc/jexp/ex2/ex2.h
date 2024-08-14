#ifndef JANUS_EX2_H
#define JANUS_EX2_H

/************************************************************************
 * (Shared Amiga/PC file)
 *
 * Ex2.h - Ex2 specific data structures
 *
 * 4-13-89 - Bill Koester - Created this file on Amiga side
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define EX2_APPLICATION_ID 2L
#define EX2_LOCAL_ID 2

/* Ex2 request structure */

struct Ex2Req {
   UBYTE exr_Function;
   UWORD exr_Called;
   UBYTE exr_Error;
};

/* Error codes for exr_Error */

#define Exr_ERR_OK         0     /* No error                            */
#define Exr_ERR_FAILED     1     /* Misc Error                          */

#endif   /* End of JANUS_EX2_H conditional compilation                  */
