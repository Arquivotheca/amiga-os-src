/* fstype.h --- Disk types */

#define ID_NO_DISK_PRESENT	(-1)
#define ID_UNREADABLE_DISK	(0x42414400L)	/* 'BAD\0' */
#define ID_BUSY_DISK		(0x42555359L)	/* 'BUSY'  */
#define ID_DOS_DISK		(0x444F5300L)	/* 'DOS\0' */
#define ID_FFS_DISK		(0x444F5301L)	/* 'DOS\1' */
#define ID_INTER_DOS_DISK	(0x444F5302L)	/* 'DOS\2' */
#define ID_INTER_FFS_DISK	(0x444F5303L)	/* 'DOS\3' */
#define ID_DC_DOS_DISK		(0x444F5304L)	/* 'DOS\4' */
#define ID_DC_FFS_DISK		(0x444F5305L)	/* 'DOS\5' */
#define ID_NOT_REALLY_DOS	(0x4E444F53L)	/* 'NDOS'  */
#define ID_KICKSTART_DISK	(0x4B49434BL)	/* 'KICK'  */

#define FFS_MASK		(0x01L)
#define INTER_MASK		(0x02L)
#define DC_MASK			(0x04L)

struct FSTypeLabels
{
	STRPTR name[20];		/* fs name for cycle gadget */
	LONG identifier[20];		/* fs identifier	    */
	LONG blocksizeflags[20];	/* fs PatchFlags	    */
	LONG rdbflags[20];		/* fs in rdb or resource
					   default(0): fs resource
					   else(1)   : rdb          */
};
