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

#define WINWIDTH		500
#define WINHEIGHT		160

#define NUM_GADGETS		5

#define ROOT_GADGET		0 
#define PARENT_GADGET		1
#define INCLUDE_GADGET		2
#define EXCLUDE_GADGET		3
#define START_GADGET		4

#define ROOT_GADGET_ID		1
#define PARENT_GADGET_ID	2
#define INCLUDE_ID		3
#define EXCLUDE_ID		4
#define ARCHIVE_ID		5
#define START_ID		6

#define DATE_GADGET_ID		10
#define SIZE_GADGET_ID		11
#define PATTERN_GADGET_ID	12
#define ARC_GADGET_ID		13

extern int processed_count;
extern struct NewWindow new_mainwin;
extern struct Window *mainwin;
extern char dext[];
extern char ihost[];
extern char number_buffer[];
extern struct IntuiText mainwin_gadget_text[];
extern struct Gadget mainwin_gadget[];
extern ULONG mainwin_sig_bit;
extern struct IntuiText date_text[];
extern struct Gadget date_gadget[];
extern struct IntuiText size_text[];
extern struct Gadget size_gadget[];
extern struct IntuiText pattern_text[];
extern struct Gadget pattern_gadget[];
extern struct IntuiText arc_text[];
extern struct Gadget arc_gadget[];

extern void open_mainwin(void);
extern void close_mainwin(void);
extern void stats(void);
extern void disp_run_count(int, char *);
extern void disp_processed(char *);
extern void disp_path(char *);
