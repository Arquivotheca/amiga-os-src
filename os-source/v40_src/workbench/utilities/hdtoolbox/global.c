/* global data for prep */
#include "prep_strux.h"

int found_driver = FALSE;	/* did we find any drivers? */

struct Drive *FirstDrive    = NULL; /* list of all drives */
struct Drive *SelectedDrive = NULL; /* which one is selected, if any */
struct DriveDef *FirstType  = NULL;
struct DriveDef *CurrType   = NULL;
struct FileSysHeaderBlock *FirstFileSys = NULL;
struct FileSysHeaderBlock *CurrFileSys  = NULL;
struct Bad *FirstBad = NULL;
struct Bad *CurrBad  = NULL;

/* which thing in list to start display */
struct Drive 		  *DisplayDrive = NULL;
struct DriveDef 	  *DisplayType  = NULL;
struct FileSysHeaderBlock *DisplayFS    = NULL;
struct Bad		  *DisplayBad   = NULL;

WORD DisplayNum     = 0;	    /* number of first drive to display */
WORD DisplayTypeNum = 0;
WORD DisplayFSNum   = 0;
WORD DisplayBadNum  = 0;

WORD MaxDisplayDrives = 5;	    /* max number of drives displayed at once */
WORD MaxDisplayTypes  = 10;	    /* max number of types displayed at once */
WORD MaxDisplayFS     = 4;	    /* these are all set dynamically */
WORD MaxDisplayBad    = 5;

WORD NumDrives = 0;		    /* Number of drives in the system */
WORD NumTypes  = 0;
WORD NumFS     = 0;
WORD NumBad    = 0;

LONG TypeType = 0;

struct DriveDef *St506_Defs = NULL; /* different lists of drives */
struct DriveDef *SCSI_Defs  = NULL;

WORD St506_DefNum = 0;		/* number of st506 drive definitions */
WORD SCSI_DefNum  = 0;

struct RigidDiskBlock St506RDB = {
	IDNAME_RIGIDDISK,
	sizeof(struct RigidDiskBlock) >> 2,
	0,7,
	512,	/* fix */
	RDBFF_DISKID | RDBFF_NORESELECT,
	NULL,NULL,NULL,NULL,
	{0,0,0,0,0,0},
	612,	/* 600 cylinders */
	17,	/* 17 sectors/track */
	4,	/* 4 heads */
	4,	/* interleave 4! */
	613,
	{0,0,0},
	612,
	612,
	3,
	{0,0,0,0,0},
	0,4*17*2,
	2,
	612-1,
	4*17,0,
	0,0,
	"","","","","","",
	{0,0,0,0,0,0,0,0,0,0},
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

struct FileSysHeaderBlock defaultfs = {
	IDNAME_FILESYSHEADER,
	sizeof(struct FileSysHeaderBlock) >> 2,
	0,7,
	NULL,
	0,
	{0,0},
	0x444f5303,	/* 'DOS\03' --- 39.13 */
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

char xt_name[7]   = "  XT  ";
char scsi_device[33] = "scsi.device";
LONG scsi_addrs   = 7;
LONG scsi_luns    = 8;
LONG scsiversion  = 0;

BYTE ForceReboot = FALSE;
