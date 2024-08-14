/* global data for prep */

#include "prep_strux.h"

extern struct Library *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct DosLibrary *DOSBase;

extern int found_driver;

extern struct Drive *FirstDrive;
extern struct Drive *SelectedDrive;
extern struct DriveDef *FirstType;
extern struct DriveDef *CurrType;
extern struct FileSysHeaderBlock *FirstFileSys;
extern struct FileSysHeaderBlock *CurrFileSys;
extern struct Bad *FirstBad;
extern struct Bad *CurrBad;

extern struct Drive *DisplayDrive;
extern struct DriveDef *DisplayType;
extern struct FileSysHeaderBlock *DisplayFS;
extern struct Bad *DisplayBad;

extern WORD DisplayNum;
extern WORD DisplayTypeNum;
extern WORD DisplayFSNum;
extern WORD DisplayBadNum;

extern WORD MaxDisplayDrives;
extern WORD MaxDisplayTypes;
extern WORD MaxDisplayFS;
extern WORD MaxDisplayBad;

extern WORD NumDrives;
extern WORD NumTypes;
extern WORD NumFS;
extern WORD NumBad;

extern LONG TypeType;

extern struct DriveDef *St506_Defs;
extern struct DriveDef *SCSI_Defs;

extern WORD St506_DefNum;
extern WORD SCSI_DefNum;

extern struct RigidDiskBlock St506RDB,ScsiRDB;

extern struct FileSysHeaderBlock defaultfs;

extern char xt_name[];
extern char scsi_device[];
extern LONG scsi_addrs;
extern LONG scsi_luns;
extern LONG scsiversion;

extern BYTE ForceReboot;

#ifdef MEMCHECK
extern void mem_init(long),mem_free(void);
extern char *alloc(long);
extern void unalloc(char *,long);

#define AllocMem(size,flags)	(alloc(size))
#define FreeMem(ptr,size)	(unalloc((ptr),(size)))

#endif

extern void __stdargs kprintf();

#define AllocNew(t)	((struct t *) AllocMem(sizeof(struct t),MEMF_CLEAR))
