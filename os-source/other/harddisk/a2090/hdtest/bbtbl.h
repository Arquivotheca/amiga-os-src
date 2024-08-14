/*			Layout of Bad Block Table 		*/

/*
 * Number of bad blocks in bad block table structure
 */
#define	NBAD	63

/*
 * Structure of bad block mapping.
 */
struct badm {
	unsigned long	bm_badb;
	unsigned long	bm_newb;
};

/*
 * Structure of bad block table
 */
struct badt {
	unsigned short	bt_magic;	/* Magic (0xBADB)	*/
	unsigned short	bt_count;	/* Number of blocks	*/
	unsigned long	bt_table;	/* Pointer to next table*/
	struct badm	bt_block[NBAD];	/* Blocks		*/
};
