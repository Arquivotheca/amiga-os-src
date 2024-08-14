extern int  HandlePrep       (char *,int,int,int,int);
extern int  DoLowLevel       (void);
extern int  LowLevelSendIO   (struct IOStdReq *);
extern void ReopenDrives     (void);
extern CPTR GetRDSK	     (char *,LONG *);
extern int  ReadBad	     (struct RigidDiskBlock *);
extern int  WriteRDSK	     (CPTR,LONG,LONG);
extern int  ReadRDSK	     (CPTR,LONG,LONG);
extern int  DoSCSI  (struct IOStdReq *, UWORD *, ULONG, UWORD *, ULONG, ULONG);
extern void SendSCSI(struct IOStdReq *, UWORD *, ULONG, UWORD *, ULONG, ULONG);
extern int  DoLongIO (char *,
		      int (*)(struct IOStdReq *),
		      int (*)(struct IOStdReq *));
extern int  VerifyRDSK(void);
extern int  DoXTVerify(void);
extern int  VerifySendIO(struct IOStdReq *);
extern int  VerifyXTSendIO(struct IOStdReq *);
extern int  VerifyXTError(struct IOStdReq *,long);
extern int  VerifyError(struct IOStdReq *);
extern LONG ReadCapacity(int,ULONG);
extern void ErrNotify(char *,long);
extern void CheckSumBlock(struct RigidDiskBlock *);
extern int  ReassignBlock(struct IOStdReq *,ULONG);

void FreeBad (struct Bad *);

int getnum (long *);
int gethexnum (long *);
int get2nums (long *, long *);
int get3nums (long *, long *, long *);

int defineit (struct Window *);
int DefineSetup (struct Window *);
struct DriveDef *DoDefine (struct Window *);
void FreeDef (struct DriveDef *);
int DoReadParms (struct Window *);
void SetGeometry (struct Window *,
		  struct RigidDiskBlock *,
		  LONG,LONG,LONG,LONG,LONG,LONG,LONG);
void strip_trail (char *);
void CtoBStr (char *);
void BtoCStr (char *);

int MakeNewPart (LONG,LONG);
int PartitionSetup (struct Window *);
void InitPartition (struct Drive *,
		    struct PartitionBlock *,
		    LONG, LONG);
void InitFFS (struct PartitionBlock *);


extern void FreeRDB (struct RigidDiskBlock *);
extern void FreeFileSys (struct FileSysHeaderBlock *);
extern void FreeLoadSegList (struct LoadSegBlock *);
extern void FreeFileSysList (struct FileSysHeaderBlock *);
extern void ReplaceFileSys (struct RigidDiskBlock *,struct FileSysHeaderBlock *,
			    struct FileSysHeaderBlock *);
extern void FreePartitionList (struct PartitionBlock *);
extern void FreeBadBlockList (struct BadBlockBlock *);
extern struct FileSysHeaderBlock *CopyFileSys (struct FileSysHeaderBlock *);
extern struct LoadSegBlock *CopyLoadSeg (struct LoadSegBlock *, ULONG);
extern struct BadBlockBlock *CopyBadBlock (struct BadBlockBlock *);
extern struct FileSysHeaderBlock *CopyFileSysList (struct FileSysHeaderBlock *);
extern struct LoadSegBlock *CopyLoadSegList (struct LoadSegBlock *);
extern struct BadBlockBlock *CopyBadBlockList(struct BadBlockBlock *);
extern struct PartitionBlock *CopyPartList (struct PartitionBlock *);
extern struct RigidDiskBlock *CopyRDB (struct RigidDiskBlock *);



int CommitChanges (struct Window *, int);
int CountBlocks (struct RigidDiskBlock *);
void CheckSumBlock (struct RigidDiskBlock *);
LONG CalcFlags (struct Drive *);
ULONG *BuildInterleaveMap (ULONG,ULONG);
ULONG FindSector (ULONG *,ULONG);
int GetRDB(struct IOStdReq *ior,struct Drive *d);
int ReadBadBlockList (
	struct IOStdReq *ior,
	struct RigidDiskBlock *rdb,
	long size,
	struct BadBlockBlock *sec);
int ReadPartitionList (
	struct IOStdReq *ior,
	struct RigidDiskBlock *rdb,
	long size,
	struct PartitionBlock *sec);
int ReadFileSysList (
	struct IOStdReq *ior,
	struct RigidDiskBlock *rdb,
	long size,
	struct FileSysHeaderBlock *sec);
int ReadDriveInitList (
	struct IOStdReq *ior,
	struct RigidDiskBlock *rdb,
	long size,
	struct LoadSegBlock *sec);
int ReadLoadSegList (
	struct IOStdReq *ior,
	struct FileSysHeaderBlock *f,
	long size,
	struct LoadSegBlock *sec);
int ReadBlock (
	register struct IOStdReq *ior,
	APTR dest,
	LONG size,
	LONG block);
int ReadBlockCheck (
	struct IOStdReq *ior,
	APTR dest,
	LONG size,
	LONG block,
	LONG id);
int CheckCheckSum (struct RigidDiskBlock *rdb);

struct FileSysHeaderBlock *MakeFileSys (
	struct Window *,
	struct RigidDiskBlock *,
	char *,
	LONG, LONG);
