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

extern char title_string[];

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct IconBase *IconBase;
extern struct Library *DiskfontBase;
extern struct Library *AslBase;

extern struct TextFont *textfont;
extern struct TextAttr *textattr;
extern struct TextAttr *slist_textattr;

extern struct Screen *screen;

extern int sel_file_color;
extern int desel_file_color;
extern int unbacked_file_color;

extern BOOL auto_backup;
extern BOOL auto_restore;
extern BOOL auto_start;
extern BOOL embed_logfile;
extern BOOL backup_the_dirs;

extern BOOL makeicon_flag;
extern BOOL show_test_menu;

extern char bru_path[];			/* Path where bru can be found */
extern char bru_name[];			/* Name of the bru executable */
extern char bru_args[];			/* Special args to pass to bru */
extern long bru_stack;			/* Amount of stack for bru */

extern char ipcport_name[];

extern void tell_about( void );
extern void cleanup( void );
