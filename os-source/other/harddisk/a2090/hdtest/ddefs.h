#ifndef	EXEC_TYPES_H
#include	"exec/types.h"
#endif
/* Defines shared by disk diagnostic modules */

#define MAX_HISTORY	100		/* Number of errors recorded */
#define	INTERLEAVE	1
#define MAX_UNITS	2

/*			Layout of Bad Block Table 		*/
/*
 * Number of bad blocks in bad block table structure
 */
#define	NBAD	126

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
struct badt {			/* Stored in 2nd and 3rd readable sectors */
	unsigned short	bt_magic1;	/* Magic (0xBAD1)		*/
	unsigned short	bt_count;	/* Number of blocks		*/
	unsigned long	bt_2nd;		/* Pointer to 2nd half of this table */
	struct	 badm	bt_block[NBAD];	/* Blocks			*/
	unsigned long	bt_next;	/* Pointer to next free map block */
	unsigned short	bt_left;	/* Count of remaining free map sectors*/
	unsigned short	bt_magic2;	/* Magic (0xBAD2)		*/
};

typedef struct disk_params {		/* description of disk type */
	char	d_optstep;	/* options & step rate */
	char	d_headhicyl;	/* heads <7:4>, hi cyl count <3:0> */
	char	d_cyl;		/* low byte of cyl count */
	char	d_precomp;	/* precomp cyl / 16 */
	char	d_reduce;	/* reduced write current / 16 */
	char	d_sectors;	/* sectors per track */
	WORD	d_park_cyl;	/* cylinder to park heads at */
} DISK_TYPE;

typedef struct error_struct {		/* List of disk errors */
	short		pass_no;	/* Pass # error occured in */
	char		test_ch;	/* Test running when error occured */
	char		err_no;		/* Error number detected */
} ERR_LIST;

typedef struct disk_stuff {		/* Phyisical disk info for driver */
	unsigned long	ds_cylinders;	/* Number of cylinders on the drive */
	unsigned long	ds_precomp;	/* Cylinder to begin pre-comp at    */
	unsigned long	ds_reduce;	/* Cylinder to begin reduced current*/
	unsigned long	ds_park;	/* Cylinder to park heads at	    */
	unsigned short	ds_sectors;	/* Sectors per track		    */
	unsigned short	ds_heads;	/* # of heads			    */
} DISK_INFO;

typedef struct	boot_s_params {		/* contents of 1st readable block */
	short		b_magic;	/* 0xBABE indicate valid block	*/
	short		b_fill1;	/* Skip 16 bits			*/
	struct badm	*b_bad_block_table;/* Pointer to bad block table*/
	DISK_INFO	b_disk_info;	/* Physical disk info for driver*/
	unsigned long	b_environment[50];/* Environment for 1st partition */
	char		b_fill2[284];
} BOOTINFO;

/*
 *  Controller port addresses
 */
#define	WDIO	0xFFC8			/* Data Register (word mode) */
#define	WDGO	0xFFFF			/* Go Command */
#define	WDRI	0x0000			/* Reset Interrupt */
#define	WDTDR	0x00			/* test drive ready */
#define	WDREST	0x01			/* restore to cyl 0 */
#define	WDRSS	0x03			/* Request status */
#define	WDCTF	0x05			/* check track format */
#define	WDFMTT	0x06			/* format track */
#define	WDREAD	0x08			/* Read */
#define	WDWRITE	0x0A			/* Write */
#define	WDSDP	0x0C			/* set drive parameters all units*/
#define	WDCCBA	0x0F			/* change command block address */
#define	WDDDIAG	0xE3			/* run drive diagnostics */
#define	WDCDIAG	0xE4			/* run controller diagnostics */
#define WDSEEK	0x0B			/* seek */
#define	WDSDP1	0xCC			/* set drive parameters for unit 1*/

#define	NSEC	17			/* number of sectors per track */
#define	NCYL	306			/* number of cylinders */
#define	WDIRQ	0x80			/* base of wd interrupt vector */
#define	SIXTEENuS 0x4F			/* sixteen microsecond step rate */
					/* no error correction */
typedef	struct	wdcmd {			/* command block layout */
 	char	c_opcode;	/* command class & opcode */
	char	c_lunhiaddr;	/* lun [7:5] & sector addr [4:0] */
	char	c_midaddr;	/* middle sector address */
	char	c_lowaddr;	/* low part of sector address */
	char	c_blockcnt;	/* number of blocks in I/O */
	char	c_control;	/* reserved control byte */
	char	c_highdma;	/* high DMA addr (phys segment) */
	char	c_middma;	/* middle DMA address */
	char	c_lowdma;	/* low DMA address */
	char	c_rsvd1;	/* reserved */
	char	c_rsvd2;	/* reserved */
	char	c_rsvd3;	/* reserved */
	char	c_errorbits;	/* error information */
	char	c_lunladd2;	/* error lun and high sector addr */
	char	c_ladd1;	/* error middle address */
	char	c_ladd0;	/* error low address */
} WDCMD;

typedef struct	wd_idc_params {		/* drive initialization params */
	DISK_TYPE	p_type;		/* disk type informatiom */
	char		*p_devname;	/* device name for user */
} WDINFO;

extern	WDCMD	*CMDBLKPADDR;	/* Temporarily allocate storage for Command block */
extern	char	*LONGBUFPADDR;	/* Big buffer */
extern	char	*BUFPADDR;		/* Single block buffer */

#define	HD_MOTOR	(CMD_NONSTD+0)	/* control the disk's motor */
#define	HD_SEEK		(CMD_NONSTD+1)	/* explicit seek (for testing) */
#define	HD_FORMAT	(CMD_NONSTD+2)	/* format disk */
#define	HD_REMOVE	(CMD_NONSTD+3)	/* notify when disk changes */
#define	HD_CHANGENUM	(CMD_NONSTD+4)	/* number of disk changes */
#define	HD_CHANGESTATE	(CMD_NONSTD+5)	/* is there a disk in the drive? */
#define	HD_PROTSTATUS	(CMD_NONSTD+6)	/* is the disk write protected? */
#define	HD_SPECIAL	(CMD_NONSTD+7)	/* Direct command to controller */
#define	HD_MOV_CMD_BLK	(CMD_NONSTD+8)	/* Move the cmd. blk.		*/

#define	HD_LASTCOMM	HD_MOV_CMD_BLK
extern	void	clear_hist();
extern	void	derror();
extern	void	err_hist();
extern	void	display_history();
extern	void	derror();
extern	void	block_err();
extern	void	rd_blk_err();
extern	void	wrt_blk_err();
extern	void	compare_error();
