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
 *	Declarations for exported variables.
 */

extern BOOL filename_sent;
extern BOOL finished_flag;
extern BOOL tree_end_flag;
extern BOOL user_cancel_flag;

extern BOOL need_to_embed_logfile;

extern char current_file[];


/*
 *	Prototypes for exported functions.
 */
extern void init_restore ( void );
extern void perform_restore ( void );
extern void init_backup (void);
extern void perform_backup (void);
extern void init_diff ( void );
extern void perform_diff ( void );
extern void init_inspect ( void );
extern void perform_inspect ( void );
extern void recover( void );

extern char *findbru( void );

