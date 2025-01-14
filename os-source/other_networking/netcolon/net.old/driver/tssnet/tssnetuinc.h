#include <exec/io.h>


/********************************************************************/
/*                                                                  */
/*  TSSnetUInc.h  -  TSSnet (TM) User Include file                  */
/*                                                                  */
/*       Copyright � 1988, TollySoft,                               */
/*       All Rights Reserved.                                       */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/




/**************************************/
/*                                    */
/* Structure to use for performing    */
/* i/o with TSSnet.device. 1st part   */
/* is standard amiga device i/o       */
/* structure. 2nd part is TSSnet      */
/* user communication block.          */
/*                                    */
/*                                    */
/**************************************/


struct IOExtTSS {

	struct IOStdReq	  IOTSS ; /* standard amiga i/o request */
	struct {

        APTR                   PortNumber ;
        APTR                   gdptr ;
        UWORD                  Flags ;
	short                  Status     ;
	short                  ReadCnt    ;
	short                  WriteCnt   ;
	LONG                   bytes_readable ;
/*	short                  family ;
	short                  a_len ;
	char                   a_addr[8] ;
        ULONG                  reserved[4] ; */
	short                  UserDataSize ;
	UBYTE                  UserData [100] ;

       }     User  ;  /*   IOStdReq extension for TSSnet       */

 } ;


/**************************************/
/*                                    */
/*  commands to the device driver, to */
/*  be put in io_Command field of     */
/*  IOStdReq struct.                  */
/*                                    */
/**************************************/


#define TSCMD_QUERY            9L
#define TSCMD_CONNECT          10L
#define TSCMD_DCL_OBJECT       11L
#define TSCMD_DISCONNECT       12L
#define TSCMD_UCON_CONFIRM     13L
#define TSCMD_EXP_PORT_DELETE  14L
#define TSCMD_RTN_DATA_PTR     15L
#define TSCMD_SELECT           16L
#define TSCMD_WRITES           17L
#define TSCMD_READM            18L
#define TSCMD_FIND_NODE_NAME   19L
#define TSCMD_GET_PEER_NAME    22L


/************************************************/
/*                                              */
/*   Error return codes                         */
/*                                              */
/*                                              */
/************************************************/


#define NO_BUFFERS_AVAIL      20
#define VOL_OFF_LINE          21
#define READ_ERROR	      22
#define EOF_ERROR	      23
#define READ_ALREADY_QUEUED   24
#define REM_DISCONNECT        25
#define EWOULD_BLOCK          35



/**************************************/
/*                                    */
/*  Link Down Error Codes             */
/*                                    */
/*                                    */
/*                                    */
/**************************************/


#define USER_DELETED_PORT       -90 
#define CIRCUIT_DOWN            -91
#define REM_USER_DISC_UNEXPECT  -92
#define UNEXPECT_DISC_CONFIRM   -95
#define DISCONNECT_CONFIRM      -96
#define NUM_LINKS_EXCEEDED      -97
#define USER_DISCONNECT         -98
#define LINK_ALLOC_FAILURE      -99
#define OUT_OF_BUFFERS          -108
	


/**************************************/
/*                                    */
/*    select command masks            */
/*                                    */
/*                                    */
/*                                    */
/**************************************/

#define SELECT_READ     1
#define SELECT_WRITE    2
#define SELECT_EXCEPT   4
