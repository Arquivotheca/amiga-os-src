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

extern char bru_logdir[80];
extern char logfile_pattern[80];

extern char logfile_icon_template[80];
extern char logdir_icon_template[80];

extern ULONG logfile_size( BOOL scanflag );
extern void save_tree ( struct TreeNode *tn );
extern int save_embedded_logfile( struct TreeNode *tn );
extern void load_tree ( void );

