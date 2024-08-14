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

#define NO_MODE			0
#define BACKUP_MODE		1
#define RESTORE_MODE		2
#define BACKUP_INPROGRESS	3
#define RESTORE_INPROGRESS	4
#define BACKUP_COMPLETE		5
#define RESTORE_COMPLETE	6
#define INSPECT_MODE		7
#define INSPECT_INPROGRESS	8
#define DIFF_MODE		9
#define DIFF_INPROGRESS		10

extern int mode;

extern char arch_level_buf[SBUF_SIZE];
extern LONG arch_level;

extern void eventloop PROTO((void));
extern void setmode PROTO((int));
extern void format_entrys PROTO((struct TreeNode *));
