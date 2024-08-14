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

extern void do_bru_query_req PROTO((char *, char *, char *));
extern BOOL do_string_req PROTO((char *, char *));
extern BOOL do_query_req PROTO((char *, char *, char *));
extern BOOL check_cancel PROTO((void));
extern void post_cancel_req PROTO((struct Window *, char *));
extern void clear_cancel_req PROTO((void));
extern BOOL do_fdata_req PROTO((char *));
extern void post_canceling PROTO((struct Window *));
extern void clear_canceling PROTO((void));
