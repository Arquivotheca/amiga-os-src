
extern int	nsec;				/* # of sectors per track */
extern long	nblkcyl;			/* number of blocks on a cylinder */

/*
 * This routine computes the block number from the cylinder, track and
 * byte offset information.
*/
long block_from_offset(cyl,trk,offset)
register int cyl;
register int trk;
register int offset;
{
	register long block_number;
	register int temp;
	register int sectors;		/* number of sectors from index */

	sectors = offset / 609; 	/* There are 597 bytes / sector */
	temp = offset % 609;		/* If error in gap, */
	if (temp > 576)			/*   and it's closer to next sector */
		sectors += 1;		/*   mark next sector bad instead */
	if (sectors > nsec)		/* if in gap past last sector */
		sectors = nsec;		/*   mark last sector bad */
	block_number = (nsec * (long)trk) + (nblkcyl * (long)cyl) +
			((nsec + trk + sectors) % nsec);
	return(block_number);
}
