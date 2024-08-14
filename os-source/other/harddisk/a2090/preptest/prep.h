#define toupper(x) ((x)&0x5f)
#define isalpha(x) (((0x40<(x)) && ((x)<0x5b)) || ((0x60<(x)) && ((x)<0x7b)))

/* Defines shared by PREP modules */

#define	NSEC	17	/* Number of sectors per track */
#define	NBAD	126	/* Number of bad blocks in bad block table struct */

/*
 * Structure of bad area table entry
 */
typedef struct bao {
    UWORD bao_cyl;	/* Cylinder defect is on	*/
    UWORD bao_offset;	/* Offset from index of defect	*/
    UBYTE bao_head;	/* Surface defect is on		*/
    UBYTE bao_fill;	/* Make structure length even	*/
} BAO;

/*
 * Structure of bad area table
 */
typedef struct baot {	/* Passed to driver when formatting track 0 */
    BAO bad[NBAD];		/* Actual bad areas on disk	*/
} BAOT;

typedef struct disk_stuff { /* Phyisical disk info for driver */
    ULONG ds_cylinders;		/* Number of cylinders on the drive */
    ULONG ds_precomp;		/* Cylinder to begin pre-comp at    */
    ULONG ds_reduce;		/* Cylinder to begin reduced current*/
    ULONG ds_park;		/* Cylinder to park heads at	    */
    UWORD ds_sectors;		/* Sectors per track		    */
    UWORD ds_heads;		/* # of heads			    */
} DISK_INFO;

typedef struct	boot_s_params {	/* contents of 1st readable block */
    LONG b_magic;		/* 0xBABE indicate valid block	*/
    LONG *b_bad_block_table;	/* Pointer to bad block table*/
    DISK_INFO b_disk_info; 	/* Physical disk info for driver*/
    ULONG b_environment[50];	/* Environment for 1st partition */
    BYTE b_fill2[284];
} BOOTINFO;

typedef struct	fmt_data { /* data passed to driver when formatting trk 0 */
    BOOTINFO bootinfo;		/* Boot sector for selected disk */
    BAOT  baodata;		/* Bad area information		*/
} FMT_DATA;

#define	HD_FORMAT (CMD_NONSTD+2) /* format disk */
