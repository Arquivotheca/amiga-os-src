#ifndef JANUS_TIMESERV_H
#define JANUS_TIMESERV_H

/************************************************************************
 * (Amiga side file)
 *
 * TimeServ.h - TimeServ specific data structures
 *
 * 4-19-88 - Bill Koester - Created this file
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define TIMESERV_APPLICATION_ID 1L
#define TIMESERV_LOCAL_ID 1

/* Time request structure for Amiga Date/Time request from 8086 */

struct TimeServReq {
   UWORD tsr_Year;
   UBYTE tsr_Month;
   UBYTE tsr_Day;
   UBYTE tsr_Hour;
   UBYTE tsr_Minutes;
   UBYTE tsr_Seconds;
   UBYTE tsr_String[27];
   UBYTE tsr_Err;       /* Return code (see below) 0 if all OK */
};

/* Error codes for adr_Err, returned in low byte                        */

#define TSR_ERR_OK         0     /* No error                            */
#define TSR_ERR_NOT_SET    1     /* Time not set on Amiga               */

#endif   /* End of JANUS_TIMESERV_H conditional compilation             */
