/* handleprep.c */
extern int  HandlePrep	     (struct Window *,struct IntuiMessage *);
extern void redraw_drivelist (struct RastPort *);
extern void AllocDriveList   (void);
extern void HandleDriveClick (struct Window *,struct IntuiMessage *);
extern void UpdateDrive      (struct Window *);
extern int  PrepInit	     (struct Window *);
extern int  FormatHandler    (struct Window *,struct Requester *,
			      struct IntuiMessage *);
#ifdef INTERLEAVE_REQ
extern int  InterleaveHandler(struct Window *,struct Requester *,
			      struct IntuiMessage *);
#endif
extern void DoLowLevel       (struct Window *);
extern int  LowLevelSendIO   (struct Window *,struct Drive *,struct IOStdReq *);
extern WORD CountDefs        (struct DriveDef *);
extern void ReopenDrives     (void);

/* readwrite.c */
extern int  CommitChanges     (struct Window *,int);
extern int  CountBlocks       (struct RigidDiskBlock *);
extern void CheckSumBlock     (struct RigidDiskBlock *);
extern int  CheckCheckSum     (struct RigidDiskBlock *);
extern int  GetDrives	      (struct RastPort *,char *,LONG,LONG,LONG);
extern void GetDriveType      (struct IOStdReq *,struct Drive *);
extern int  GetRDB	      (struct IOStdReq *,struct Drive *);
extern int  ReadBadBlockList  (struct IOStdReq *,struct RigidDiskBlock *,long,
			       struct BadBlockBlock *);
extern int  ReadPartitionList (struct IOStdReq *,struct RigidDiskBlock *,long,
			       struct PartitionBlock *);
extern int  ReadFileSysList   (struct IOStdReq *,struct RigidDiskBlock *,long,
			       struct FileSysHeaderBlock *);
extern int  ReadDriveInitList (struct IOStdReq *,struct RigidDiskBlock *,long,
			       struct LoadSegBlock *);
extern int  ReadLoadSegList   (struct IOStdReq *,struct FileSysHeaderBlock *,long,
			       struct LoadSegBlock *);

extern int  ReadBlock	      (struct IOStdReq *,APTR,LONG,LONG);
extern int  ReadBlockCheck    (struct IOStdReq *,APTR,LONG,LONG,LONG);
extern void CtoBStr	      (char *);
extern void BtoCStr	      (char *);
extern struct DriveDef *GetDefs   (char *,int);
extern struct DriveDef *ReadDefs  (char *,struct DriveDef *,int);
extern void CheckDefDates         (struct DriveDef **,struct DriveDef *);
extern struct DriveDef *FindDrive (struct DriveDef *,char *,char *,char *);
extern int  DoSCSI  (struct IOStdReq *,UWORD *,ULONG,UWORD *,ULONG,ULONG);
extern void SendSCSI(struct IOStdReq *,UWORD *,ULONG,UWORD *,ULONG,ULONG);
extern LONG CalcFlags		(struct Drive *);
extern ULONG *BuildInterleaveMap(ULONG,ULONG);
extern ULONG FindSector		(ULONG *,ULONG);
extern struct BadBlockBlock *GetBadBlocks	(struct Drive *);

/* rdb.c */
extern void FreeRDB		(struct RigidDiskBlock *);
extern void FreeFileSys		(struct FileSysHeaderBlock *);
extern void FreeLoadSegList	(struct LoadSegBlock *);
extern void FreeFileSysList	(struct FileSysHeaderBlock *);
extern void ReplaceFileSys	(struct RigidDiskBlock *,struct FileSysHeaderBlock *,
			    	struct FileSysHeaderBlock *);
extern void FreePartitionList	(struct PartitionBlock *);
extern void FreeBadBlockList	(struct BadBlockBlock *);
extern struct FileSysHeaderBlock *CopyFileSys	(struct FileSysHeaderBlock *);
extern struct LoadSegBlock *CopyLoadSeg		(struct LoadSegBlock *,ULONG);
extern struct BadBlockBlock *CopyBadBlock	(struct BadBlockBlock *);
extern struct FileSysHeaderBlock *CopyFileSysList (struct FileSysHeaderBlock *);
extern struct LoadSegBlock *CopyLoadSegList	(struct LoadSegBlock *);
extern struct BadBlockBlock *CopyBadBlockList	(struct BadBlockBlock *);
extern struct PartitionBlock *CopyPartList	(struct PartitionBlock *);
extern struct RigidDiskBlock *CopyRDB		(struct RigidDiskBlock *);

/* handlepart.c */
extern void do_refreshPart	(struct Window *);
extern int  HandlePart		(struct Window *,struct IntuiMessage *);
extern void HandleName		(struct Window *);
extern void DeleteCurrentPart	(struct Window *);
extern int  MakeNewPart		(struct Window *,struct IntuiMessage *);
extern int  HandlePartClick	(struct Window *,struct IntuiMessage *);
extern void HandleSize		(struct Window *);
extern void HandleDrag		(struct Window *);
extern void PropRefresh		(struct Window *);
extern void SizeRefresh		(struct Window *);
extern void FixString		(struct Window *,struct Gadget *);
extern void SelectPartition	(struct Window *);
extern void SetFileSysName	(struct Window *);
extern void RefreshEverything	(struct Window *);
extern void RefreshBootable	(struct Window *);
extern void DrawPartitions	(struct RastPort *);
extern int  PartitionSetup	(struct Window *);
extern void InitPartition	(struct Drive *,struct PartitionBlock *,
			  	LONG,LONG);
extern void ReCalcTotalCyl	(struct Window *);
extern void ReCalcEndCyl	(struct Window *);
extern void ReCalcStartCyl	(struct Window *);
extern void ReCalcBootPri	(struct Window *);
extern void ReCalcHostID	(struct Window *);
extern void ReCalcBuffers	(struct Window *);
extern void PartDraw		(struct RastPort *);
extern void ReCalcSizeText	(struct Window *);
extern int  DoQuickSetup	(struct Drive *);

/* handlefs.c */
extern int  HandleFileSys	(struct Window *,struct IntuiMessage *);
extern void DeselectFS		(struct Window *,void (*)(struct Window *));
extern void DefaultSDFS		(struct Window *);		/* 39.13 */
extern void DefaultCustom	(struct Window *);
extern void DefaultUser		(struct Window *);		/* 39.13 */
extern void RebuildFSGads	(struct Window *,struct PartitionBlock *);
extern void InitFFS		(struct PartitionBlock *);
extern void InitCustom		(struct PartitionBlock *);
extern void InitUser		(struct PartitionBlock *);	/* 39.13 */
extern void InitReserved	(struct PartitionBlock *);
extern void ChangeEnvNum	(int,struct Gadget *);
extern int  FSInitialize	(struct Window *);
extern int  AllocFSType		(void);				/* 39.13 */
extern void FreeFSType		(WORD);				/* 39.13 */
extern void UpdateIdentifier	(struct Window *,LONG);		/* 39.13 */
extern void SetSDFSGadsAttrs	(struct Window *,LONG,LONG,LONG);	/* 39.13 */
extern void SetMiscGadsAttrs	(struct Window *,LONG,LONG,LONG,LONG);	/* 39.13 */
extern void UpdatePatchFlags	(LONG, WORD);			/* 39.18 */
extern int  IsThisStandardFS	(LONG);				/* 39.18 */
extern int  MakeFSLabel		(int, LONG);			/* 39.18 */
extern void SetFSBS		(struct Window *,LONG);		/* 39.18 */
extern void SetFSCBSGadsAttrs	(struct Window *,LONG);		/* 39.18 */
extern void SetBlockSizeFlags	(LONG,int);			/* 39.18 */
extern int  AddFSResource	(struct FileSysResource *,int *);	/* 39.18 */
extern int  CompareFSRPatchFlags(struct FileSysHeaderBlock *,struct FileSysResource *,int);	/* 39.18 */

/* handledefine.c */
extern int  HandleDefine	(struct Window *,struct IntuiMessage *);
extern void RecalcSize		(struct Window *);
extern void RecalcSizeSetup	(void);
extern void DefineDraw		(struct RastPort *);
extern int  DefineSetup		(struct Window *);
extern int  EditSetup		(struct Window *);
extern int  WriteDef		(struct Window *,struct DriveDef *,int);
extern void FreeDef		(struct DriveDef *);
extern void SetSCSI		(struct Window *);
extern void SetSt506		(struct Window *);
extern void DoReadParms		(struct Window *);
extern void SetGeometry		(struct Window *,struct RigidDiskBlock *,
				 LONG,LONG,LONG,LONG,LONG,LONG,LONG);
extern void MyOnGadget		(struct Window *,struct Gadget *);
extern void MyOffGadget		(struct Window *,struct Gadget *);
extern void ResetStrings	(struct Window *);
extern void strip_trail		(char *);

/* handlefsm.c */
extern int  HandleFSM		(struct Window *,struct IntuiMessage *);
extern void HandleFSMUpdate	(struct Window *);
extern void HandleFSMDelete	(struct Window *);
extern void HandleFSMAdd	(struct Window *);
extern int  GetFileHandler	(struct Window *,struct Requester *,
			   	struct IntuiMessage *);
extern int  GetTypeHandler	(struct Window *,struct Requester *,
			   	struct IntuiMessage *);
extern struct FileSysHeaderBlock *GetFileSys	(struct Window *);
extern struct FileSysHeaderBlock *MakeFileSys	(struct Window *,
						struct RigidDiskBlock *,
						char *,LONG,LONG,LONG);
extern BOOL GetFSVersion	(char *, LONG *, LONG *);
extern void redraw_fsmlist	(struct RastPort *);
extern LONG LoadSegSize		(struct LoadSegBlock *);
extern void HandleFSMClick	(struct Window *,struct IntuiMessage *);
extern void AllocFSMList	(void);
extern int  FSMInit		(struct Window *);

/* analyze.c */
extern int  AnalyzePartitions	(struct RigidDiskBlock *,struct RigidDiskBlock *);
extern int  CheckParts       	(struct PartitionBlock *,struct PartitionBlock *);
extern void AddToCommit      	(struct PartitionBlock *,short *);

/* testmain.c */
extern void FreeEverything	(void);
extern int  AllocCommit		(void);
extern void FreeCommit		(void);

/* badblock.c */
extern int  HandleBadBlock	(struct Window *,struct IntuiMessage *);
extern void BadDraw		(struct RastPort *);
extern int  BadSetup		(struct Window *);
extern void DeleteBad		(struct Window *);
extern void AddBad		(struct Window *);
extern void InsertBad		(struct Window *,struct Bad *);
extern int  BadLessEqual	(struct Bad *,struct Bad *);
extern int  GetBadHandler	(struct Window *,struct Requester *,
			 	struct IntuiMessage *);
extern struct Bad *GetBad	(struct Window *);
extern LONG BfiToSector		(LONG);
extern LONG MinBfi		(LONG);
extern LONG MaxBfi		(LONG);
extern void HandleBadClick	(struct Window *,struct IntuiMessage *);
extern void AllocBadList	(void);
extern void FreeBad		(struct Bad *);
extern struct BadBlockBlock *MakeBadBlockList	(struct Bad *);
extern struct Bad *CopyBad	(struct Bad *);
extern ULONG GetReplacement	(struct RigidDiskBlock *,struct Bad *);
extern int isbad		(struct Drive *,LONG);
extern LONG ReadDefects		(struct IOStdReq *);
extern LONG DoReadDefect	(struct IOStdReq *,char *,char *,LONG);
extern void ReassignAll		(struct Drive *,struct IOStdReq *);

/* verify.c */
extern void DoVerify		(struct Window *);
extern int  DoLongIO		(struct Window *,
				 struct Drive *,
				 char *,
				 int (*)(struct Window *,struct Drive *,struct IOStdReq *),
				 struct IntuiText *,
				 void (*)(struct RastPort *),
				 int (*)(struct Window *,struct IOStdReq *));
extern int  VerifySendIO	(struct Window *,struct Drive *,struct IOStdReq *);
extern LONG ReadCapacity	(struct Window *,struct Drive *,int,ULONG);
extern LONG ReadSectorSize	(struct Window *,struct Drive *);
extern int  VerifyError		(struct Window *,struct IOStdReq *);
extern int  VerifyXTSendIO	(struct Window *,struct Drive *,struct IOStdReq *);
extern int  MapOutHandler	(struct Window *,struct Requester *,struct IntuiMessage *);
extern int  VerifyXTError	(struct Window *,struct IOStdReq *,LONG);
extern void DoXTVerify		(struct Window *,struct Drive *);
extern int  ReassignBlock	(struct IOStdReq *,ULONG);

/* handletype.c */
extern int  HandleSetType   	(struct Window *,struct IntuiMessage *);
extern void TypeDraw	    	(struct RastPort *);
extern void HandleTypeClick 	(struct Window *,struct IntuiMessage *);
extern void AllocTypeList	(void);
extern int  ChangeDriveType 	(struct Window *,struct DriveDef *);
extern void DeleteDiskType  	(struct Window *,struct DriveDef *);
extern void redraw_drivetype	(struct RastPort *);
extern void HandleTypeListNew  	(struct Window *);
extern void WriteDiskName   	(struct RastPort *,struct RigidDiskBlock *);
extern int  SameDrive	    	(struct RigidDiskBlock *,struct RigidDiskBlock *);
extern int  TypeSetup	    	(struct Window *);
extern void ReadError	    	(struct Window *,LONG);

