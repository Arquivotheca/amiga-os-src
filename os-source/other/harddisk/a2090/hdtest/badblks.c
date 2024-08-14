#include "ddefs.h"

extern	int	reserved_blocks;	/* # of blocks for future bad blocks */
extern	int	ncyl;			/* # of cylinders */
extern	int	nsec;			/* # of sectors per track */
extern	int	ntrk;			/* # of tracks per cylinder */
extern	long	nblkcyl;		/* number of blocks on a cylinder */
extern	long	bad_blocks[127];	/* bad block list */
extern	int	idx_bad_blocks;
extern	int	findlong();
extern	void	write_block();
extern	BOOTINFO bootinfo;		/* Boot sector for selected disk */
	char	too_many = 0;		/* TRUE when too many bad blocks */
	long	last_map;		/* Last block usable for mapping */

long next_good(block_no) /* Find the next good block on the disk */
long	block_no;
{
	++block_no;
	while (findlong(block_no,bad_blocks,idx_bad_blocks) != -1)
		++block_no;	/* Skip any bad blocks */
	too_many = block_no > last_map;
	return(block_no);
}

write_bad_block_table()
{
    struct badt	bb_table;	/* Bad block table			*/
    int		bb_index;	/* Index into bad_blocks		*/
    long	last_used;	/* # last reserved block used		*/
    int		temp_int;
    unsigned long b0;		/* Block number of boot block		*/
    unsigned long b1;		/* Block number of 1st half of bb table	*/
    unsigned long b2;		/* Block number of 2nd half of bb table	*/

    for (bb_index = 0;bb_index < NBAD;bb_index++)
    {
    	bb_table.bt_block[bb_index].bm_badb = 0L;
    	bb_table.bt_block[bb_index].bm_newb = 0L;
    }

    if (0 <= idx_bad_blocks)		/* if any bad blocks exist	*/
    {
	last_map =  nblkcyl*2L;		/* Compute last usable map block    */

	b0 = 0;				/* INFO goes in 1st good block	*/
	if (bad_blocks[0] == 0L) b0 = next_good(b0);
	b1 = next_good(b0);		/* 1st half of bad blk tbl next	    */
	b2 = next_good(b1);		/* 2nd half of bad blk tbl goes here*/
	last_used = b2;			/* Remaining good blocks used for maps*/

	bb_index = 0;
		/* Have pointer in disk info block point to bad block table */

	bb_table.bt_magic1 = 0xBAD1;
	bb_table.bt_magic2 = 0xBAD2;

	while (bb_index <= idx_bad_blocks) /* While blocks left */
	{
		/* Initialize Bad Block Table */
	    bb_table.bt_count = 1;	/* Will have at least one bad block */
		/* Set entry for bad block number */
	    bb_table.bt_block[0].bm_badb = bad_blocks[bb_index++];
		/* Set mapping for good block */
	    last_used = bb_table.bt_block[0].bm_newb = next_good(last_used);

	    while ((bb_index <= idx_bad_blocks) &&
		  (bb_table.bt_count < NBAD) && !too_many)
	    { /* While bad blocks exist and there is room in this table */
		temp_int = bb_table.bt_count++;
		bb_table.bt_block[temp_int].bm_badb =
			bad_blocks[bb_index++];
		bb_table.bt_block[temp_int].bm_newb =
			last_used = next_good(last_used);
	    }

		/* If there are more bad blocks, error			*/

	    if ((bb_index <= idx_bad_blocks) || too_many)
	    {
		puts("\nToo many bad blocks, try re-formatting!");
	    }
	    else
	    {
		bb_table.bt_next = next_good(last_used);
		bb_table.bt_2nd = b2;
		bb_table.bt_left = (last_map) - bb_table.bt_next;
		write_block(b1,&bb_table);	/* write 1st half */
		write_block(b2,&(bb_table.bt_block[63])); /* write 2nd half */
		bootinfo.b_bad_block_table  = (struct badm *)(b1);
		write_block(b0,&bootinfo);	/* Update info block	*/
	    }
    }
}
}
