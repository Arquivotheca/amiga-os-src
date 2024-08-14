
/* global data for prep */
#include "prep_strux.h"

extern struct Drive *FirstDrive;
extern struct Drive *SelectedDrive;
extern struct PartitionBlock *CurrentPart,*LastPart;

extern LONG currcyl,totalcyls;

extern struct FileSysHeaderBlock defaultfs;
extern struct RigidDiskBlock ScsiRDB;

extern LONG scsiversion;

extern BYTE ForceReboot;

