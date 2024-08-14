/* global data for prep */
#include "prep_strux.h"

struct Drive *FirstDrive    = NULL; /* list of all drives */
struct Drive *SelectedDrive = NULL; /* which one is selected, if any */
struct PartitionBlock *CurrentPart = NULL;
struct PartitionBlock *LastPart = NULL;

LONG currcyl,totalcyls,scsiversion = 0;

struct FileSysHeaderBlock defaultfs = {
	IDNAME_FILESYSHEADER,
	sizeof(struct FileSysHeaderBlock) >> 2,
	0,7,
	NULL,
	0,
	{0,0},
	0x444f5301,	/* 'DOS\01' */
	0,
	0x180,		/* globvec and seglist */
	0,
	0,
	0,
	0,
	0,		/* stacksize */
	0,		/* pri */
	0,
	NULL,		/* seglist */
	-1,		/* globvec */
	/* ... the rest are 0 */
};

struct RigidDiskBlock ScsiRDB = {
	IDNAME_RIGIDDISK,
	sizeof(struct RigidDiskBlock) >> 2,
	0,7,
	512,	/* fix */
	RDBFF_DISKID,
	NULL,NULL,NULL,NULL,
	{0,0,0,0,0,0},
	620,
	17,
	4,
	1,
	621,
	{0,0,0},
	620,
	620,
	3,
	{0,0,0,0,0},
	0,4*17*2,
	2,
	620-1,
	4*17,0,
	0,0,
	"","","","","","",
	{0,0,0,0,0,0,0,0,0,0},
};

BYTE ForceReboot = FALSE;
