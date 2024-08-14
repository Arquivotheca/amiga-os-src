#include	"/bbtbl.h"		/* Bad block table structure	*/
typedef struct disk_params {		/* description of disk type	*/
	unsigned char	d_optstep;	/* options & step rate		*/
	unsigned char	d_headhicyl;	/* heads <7:4>, hi cyl count <3:0>*/
	unsigned char	d_cyl;		/* low byte of cyl count	*/
	unsigned char	d_precomp;	/* precomp cyl / 16		*/
	unsigned char	d_reduce;	/* reduced write current / 16	*/
	unsigned char	d_sectors;	/* sectors per track		*/
	int		d_park_cyl;	/* cylinder to park heads at	*/
} DISK_TYPE;

typedef struct	boot_s_params {		/* contents of block 0		*/
	int		b_stuff1[10];	/* 1st part of boot block	*/	
	long		b_offset;	/* Pointer to start of ZEUS f sys*/
	int		b_stuff2[180];	/* Skip part of boot block	*/	
	struct badm	*b_bad_block_table;/* Pointer to bad block table*/
	long		b_free_blocks;	/* # of free good blocks on disk*/
	int		b_stuff3[28];	/* Skip part of boot block	*/	
	DISK_TYPE	b_disk_type;	/* Disk type description	*/
	int		b_stuff4[28];	/* Skip rest of boot block	*/	
} BOOTINFO;
