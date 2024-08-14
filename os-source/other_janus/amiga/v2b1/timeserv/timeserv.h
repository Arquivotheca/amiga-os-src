/************************************************************************
 *
 * TimeServ.h - TimeServ specific data structures
 *
 * This file is intended for use on both the Amiga and PC side. Therefore,
 * it should contain only information important to both sides, and should
 * not contain any information used exclusively by one side.
 *
 * 4-19-88  -  Bill Koester - Created this file
 ************************************************************************/

#define TIMESERV_APPLICATION_ID 1L
#define TIMESERV_LOCAL_ID 1

/* Time request structure for Amiga Date/Time request from 8086 */

struct TimeServReq {
   unsigned short tsr_Year;
   unsigned char  tsr_Month;
   unsigned char  tsr_Day;
   unsigned char  tsr_Hour;
   unsigned char  tsr_Minutes;
   unsigned char  tsr_Seconds;
   unsigned char  tsr_String[27];
   unsigned char  tsr_Err;       /* Return code (see below) 0 if all OK */
};

/* Error codes for adr_Err, returned in low byte                        */

#define TSR_ERR_OK         0     /* No error                            */
#define TSR_ERR_NOT_SET    1     /* Time not set on Amiga               */
