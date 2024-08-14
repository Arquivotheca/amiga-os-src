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


extern int (*sort_func) ();
extern int dflag;
extern int file_count;
extern ULONG file_size;

extern int file_selected_count;
extern ULONG file_selected_size;

extern ULONG final_archive_size;
extern int final_archive_vols;
extern ULONG vol_size;

extern int node_count;
extern ULONG node_memsize;
extern int node_entrys;
extern int most_entrys;

extern void sort PROTO((struct TreeNode *));
extern int alpha_compare PROTO((struct TreeEntry *, struct TreeEntry *));
extern int date_compare PROTO((struct TreeEntry *, struct TreeEntry *));
extern int size_compare PROTO((struct TreeEntry *, struct TreeEntry *));
extern int archive_compare PROTO((struct TreeEntry *, struct TreeEntry *));

/* Scan data collection stuff */

extern void node_status PROTO((struct TreeNode *));
extern void file_status PROTO((struct TreeNode *));
extern void select_all PROTO((struct TreeNode *));
extern void select_none PROTO((struct TreeNode *));

/* Shadowing */

extern void shadow PROTO((struct TreeNode *));
extern void un_shadow PROTO((struct TreeNode *));
extern void clear_backed PROTO((struct TreeNode *));
extern int init_criteria PROTO((void));
extern BOOL want_file PROTO((struct TreeEntry *));
extern void do_include PROTO((struct TreeNode *));
extern void do_exclude PROTO((struct TreeNode *));
