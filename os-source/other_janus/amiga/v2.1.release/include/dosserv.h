#ifndef JANUS_DOSSERV_H
#define JANUS_DOSSERV_H

/************************************************************************
 * (Amiga side file)
 *
 * DOSServ.h - DOSServ specific data structures
 *
 * 11-19-90 - Bill Koester - Created this file
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define DOSSERV_APPLICATION_ID 1L
#define DOSSERV_LOCAL_ID 3

/*************************/
/* DOS request structure */
/*************************/

struct DOSServReq {
   UWORD dsr_Function;
   UBYTE dsr_Lock;
   UBYTE dsr_Pad;
   UWORD dsr_Buffer_Seg;
   UWORD dsr_Buffer_Off;
   UWORD dsr_Buffer_Size;
   UWORD dsr_Err;       /* Return code (see below) 0 if all OK */
   UWORD dsr_Arg1_h;
   UWORD dsr_Arg1_l;
   UWORD dsr_Arg2_h;
   UWORD dsr_Arg2_l;
   UWORD dsr_Arg3_h;
   UWORD dsr_Arg3_l;
   UWORD dsr_Err_h;
   UWORD dsr_Err_l;
};

/***********************************/
/* Function codes for dsr_Function */
/***********************************/

#define DSR_FUNC_OPEN_OLD			1
#define DSR_FUNC_OPEN_NEW			2
#define DSR_FUNC_OPEN_READ_WRITE	3
#define DSR_FUNC_CLOSE				4
#define DSR_FUNC_READ				5
#define DSR_FUNC_WRITE				6
#define DSR_FUNC_SEEK_BEGINING		7
#define DSR_FUNC_SEEK_END			8
#define DSR_FUNC_SEEK_CURRENT		9
#define DSR_FUNC_SEEK_EXTEND	   10	
#define DSR_FUNC_CREATE_DIR	       11	
#define DSR_FUNC_LOCK		       12
#define DSR_FUNC_UNLOCK		       13
#define DSR_FUNC_EXAMINE	       14
#define DSR_FUNC_EXNEXT			15
#define DSR_FUNC_GETCURRENTDIR		16
#define DSR_FUNC_SETCURRENTDIR		17
#define DSR_FUNC_DELETEFILE		18
#define DSR_FUNC_DUPLOCK		19
#define DSR_FUNC_PARENTDIR		20
#define DSR_FUNC_RENAME			21
#define DSR_FUNC_SETPROTECTION		22
#define DSR_FUNC_PARSEPATTERN		23
#define DSR_FUNC_MATCHPATTERN		24
#define DSR_FUNC_ENDCURRENTDIR		25
#define DSR_FUNC_SETFILEDATE		26

/***************************/
/* Error codes for dsr_Err */
/***************************/

#define DSR_ERR_OK         				0
#define DSR_ERR_UNKNOWN_FUNCTION 		1
#define DSR_ERR_TOO_MANY_FILES 			2
#define DSR_ERR_OPEN_ERROR				3
#define DSR_ERR_FILE_NOT_OPEN			4
#define DSR_ERR_SEEK_ERROR			    5
#define DSR_ERR_TOO_MANY_LOCKS 			6

#endif   /* End of JANUS_DOSSERV_H conditional compilation             */
