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

/* The menu numbers */

#define PROJECT_MENU		0
#define SORT_MENU		1
#define OPTIONS_MENU		2
#define DEVS_MENU		3
#define TEST_MENU		4

/* The menu items */

/* Project */
#define NUM_PROJECT_ITEMS	9
#define BACKUP_ITEM		0
#define INSPECT_ITEM		1
#define DIFFERENCE_ITEM		2
#define RESTORE_ITEM		3
#define RECOVER_ITEM		4
#define HELP_ITEM		5
#define ABOUT_ITEM		6
#define INFO_ITEM		7
#define QUIT_ITEM		8

/* Sort */
#define NUM_SORT_ITEMS		5
#define SORT_DIRS_ITEM		0
#define SORTBY_NAME_ITEM	1
#define SORTBY_DATE_ITEM	2
#define SORTBY_SIZE_ITEM	3
#define	SORTBY_ARCHIVE_ITEM	4

/* Options */
#define NUM_OPTIONS_ITEMS	4
#define SET_ARC_BITS_ITEM	0
#define AUTO_TRIM_ITEM		1
#define COMPRESSION_ITEM	2
#define BACK_DIR_ITEM		3

/* Options-Compression Sub-Menu */
#define NUM_COMPRESSION_ITEMS	3
#define COMPRESS_NONE_ITEM		0
#define COMPRESS_ALL_ITEM		1
#define COMPRESS_LARGER_ITEM	2


/* Devices */
#define NUM_DEVS_ITEMS		8
#define DF0_ITEM		0
#define DF1_ITEM		1
#define DF2_ITEM		2
#define DF3_ITEM		3
#define UDEV1_ITEM		4
#define UDEV2_ITEM		5
#define UDEV3_ITEM		6
#define UDEV4_ITEM		7

#define USERDEV_MAXWIDTH	40

/* Test */
#define NUM_TEST_ITEMS	7
#define WRITELOG_ITEM		0
#define T1_ITEM		1
#define T2_ITEM		2
#define T3_ITEM		3
#define T4_ITEM		4
#define T5_ITEM		5
#define T6_ITEM		6



extern struct MenuItem test_item[NUM_TEST_ITEMS];
extern struct MenuItem devs_item[NUM_DEVS_ITEMS];
extern struct MenuItem options_item[NUM_OPTIONS_ITEMS];
extern struct MenuItem compression_item[NUM_COMPRESSION_ITEMS];
extern struct MenuItem sort_item[NUM_SORT_ITEMS];
extern struct MenuItem project_item[NUM_PROJECT_ITEMS];
extern struct Menu project_menu;
extern char userdev[4][USERDEV_MAXWIDTH];

extern void init_menus PROTO(( void ));
extern void menucheckmark PROTO((struct MenuItem *, int));
extern void read_menus PROTO((void));
extern void menu_status PROTO((void));
