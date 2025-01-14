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
 * In the following structure, the member Size can also be a pointer
 * to the TreeNode structure containing the file entrys in a directory.
 * This should probably be a union, but that gets messy in my opinion,
 * and since any access to this member should be through a function
 * call in this file, this duality should be well encapsulated.
 */

struct TreeEntry {
    ULONG Date;			/* Date in secs since 1978 */
    ULONG Size;			/* File size in bytes or pointer to a
				 * TreeNode if a directory entry. */
    UBYTE Flags;		/* Our own flag bits */
    UBYTE Status;		/* Contains AmigaDOS status bits */
    char Name[MAX_NAME];	/* The file name */
};

		/* TreeEntry.Flags */
#define ENTRY_SELECTED	0x01
#define ENTRY_IS_DIR	0x02
#define ENTRY_BACKEDUP	0x04
#define ENTRY_SHADOWED	0x08	/* means a dir above is de-selected */
#define ENTRY_RESTORED	0x10

/*
 * At this time, the entrys in the scratch buffer are TreeEntry
 * structures.  If this needs to change for some reason (such as
 * a compaction of strings, etc. for the actual tree entrys) the only
 * places changes should need to be made are here, in changing this into
 * it's own structure, and in the scratch_to_node() function.  Also if
 * course the function which stuffs directory information into the
 * scratchbuffer!
 */

struct ScratchEntry {
    ULONG Date;			/* Date in days since 1978? */
    ULONG Size;			/* File size in bytes or pointer to a
				 * TreeNode if a directory entry. */
    UBYTE Flags;		/* Our own flag bits */
    UBYTE Status;		/* Contains AmigaDOS status bits */
    char Name[MAX_NAME];	/* The file name */
};


struct TreeNode {
    struct TreeNode *Parent;
    ULONG Size;
    USHORT Entries;
    struct TreeEntry FirstEntry;
    /* This is a variable length structure.  There will usually
     * be more TreeEntry structures immediately after FirstEntry.
     */
};

/*
 *	Exported variables.
 */

extern struct TreeNode *current_node;
extern UBYTE *entry_color;
extern struct TreeNode *root_node;
extern char current_path[MAX_CUR_PATH];
extern char (*entry_string)[1][70];
extern char root_name[MAX_CUR_PATH];

/*
 *	Exported functions.
 */

extern struct TreeEntry *find_entry PROTO((struct TreeNode *, int));
extern void free_tree PROTO((struct TreeNode *));
extern BOOL walk_nodes PROTO((struct TreeNode *, BOOL (*)(), char *));
extern struct TreeNode *create_tree PROTO((char *));
extern void descend PROTO((struct TreeEntry *));
extern void ascend PROTO((void));
extern void to_root PROTO((void));
extern BOOL walk_files PROTO((struct TreeNode *, BOOL (*)()));
extern BOOL walk_entrys PROTO((struct TreeNode *, BOOL (*)(), char *));
extern struct TreeEntry *find_filename PROTO((struct TreeNode *, char *));
