/* prep_strux.h */

#include <exec/types.h>
#include <libraries/dos.h>

#include "rdb.h"
#include "commit.h"

struct Bad
{
	struct Bad *Next;
	LONG   flags;
	LONG   cyl;
	LONG   head;
	LONG   sector;
	LONG   bfi;
	LONG   block;
	LONG   replacement;
};

#define OLDBADBLOCK 1	/* for flags - block and replacement are valid */
#define BADSECTOR   2	/* user entered a sector, not bfi */

struct DriveDef
{
	struct DriveDef *Prev;
	struct DriveDef *Succ;
	struct DateStamp ds;
	char   Filename[128];
	ULONG  Offset;			/* stored at offset in file */
	struct RigidDiskBlock *Initial_RDB;
	UBYTE  Reserved1;
	ULONG  ControllerType;		/* ST506/SCSI/OMTI/ADAPTEC */
	ULONG  Reserved2;
};

struct DiskDrive			/* DriveDef file structure */
{
	ULONG  Ident;			/* == 'DRIV' */
	struct DateStamp ds;
	UBYTE  Reserved1;
	ULONG  ControllerType;
	ULONG  Reserved2;
	struct RigidDiskBlock rdb;
	ULONG  Reserved3[10];
};

#define IDNAME_DRIVE 'DRIV'

struct Drive
{
	struct Drive *NextDrive;
	struct RigidDiskBlock *rdb;
	struct RigidDiskBlock *oldrdb;	/* same as disk */
	struct Bad *bad;		/* for adaptec, etc */
	ULONG  Flags;			/* ST506|SCSI|PREPED|UPDATE */
	UBYTE  Addr;
	UBYTE  Lun;
	ULONG  Block;			/* what block it came from */
	UBYTE  DeviceName[32];		/* xt.device, scsi.device, etc */
	ULONG  SCSIBadBlocks;		/* the number of blocks mapped out */
					/* by the scsi software of the drive */
};

#define ST506		1
#define SCSI		2
#define PREPED		4
#define UPDATE		8	/* means we need to write out the rdb/etc */
#define OMTI		16
#define ADAPTEC		32
#define WRITTEN	 	64	/* wrote out to this drive */
#define MINOR_UPDATE	128	/* minor changes, no reboot needed */

#define Update(var,new,upd) if ((var) != (new)) {(var)=(new); (upd)|=UPDATE;}

#define HOST_ID 7
