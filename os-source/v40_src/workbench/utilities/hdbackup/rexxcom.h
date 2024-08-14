/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 * Result error codes which may be sent back to the macro program
 * as the primary result code.
 */

#define RXERR_UNKNOWN_CMD	1L	/* Not a valid command keyword */
#define RXERR_FAILED		2L	/* Misc. failure */
#define RXERR_MENTION		3L
#define RXERR_RETRY		4L
#define RXERR_ARG_TOO_LONG	5L
#define RXERR_NO_ARG		6L
#define RXERR_RESULTS		7L	/* Result string not requested */

#define RXERR_INVALID_ARG	10L	/* for any bad argument */

extern struct RxsLib *RexxSysBase;

extern struct MsgPort *ipcport;
extern ULONG ipc_sig_bit;

extern struct Task *bru_tcb;

extern void check_ipc_port PROTO((void));
extern ERRORCODE init_ipc PROTO((void));
extern void cleanup_ipc PROTO((void));
void file_failure PROTO((char *));

