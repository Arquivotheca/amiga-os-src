/* gadgets.h */

#include <intuition/sghooks.h>	/* 39.13 */

extern struct TextAttr TOPAZ80;

/****************************** Preparation ********************************/

extern struct Gadget *SetType;
extern struct Gadget *BadBlock;
extern struct Gadget *LowFormat;
extern struct Gadget *PartitionDrive;
extern struct Gadget *SurfCheck;
extern struct Gadget *FormatDrive;
extern struct Gadget *HelpPrep;
extern struct Gadget *ExitPrep;
extern struct Gadget *DriveList;

/****************************** Partitioning *******************************/

extern struct Gadget *FileSys;
extern struct Gadget *AddFileSys;
extern struct Gadget *ChangeFileSys;
extern struct Gadget *HostID;
extern struct Gadget *BootPri;
extern struct Gadget *Buffers;
extern struct Gadget *TotalCyl;
extern struct Gadget *EndCyl;
extern struct Gadget *StartCyl;
extern struct Gadget *NamePart;
extern struct Gadget *Advanced;
extern struct Gadget *CancelPart;
extern struct Gadget *DonePart;
extern struct Gadget *Help;
extern struct Gadget *QuickSetup;
extern struct Gadget *NewPart;
extern struct Gadget *DeletePart;
extern struct Gadget *Bootable;
extern struct Gadget SizePart;
extern struct Gadget DragPart;

/********************** File System Charatacteristics **********************/

extern struct Gadget *PartName;
extern struct Gadget *FSOk;
extern struct Gadget *FSCancel;
extern struct Gadget *FSType;		/* 39.13 */
extern struct Gadget *FSBlockSize;	/* 39.13 */
extern struct Gadget *FSIdentifier;
extern struct Gadget *Mask;
extern struct Gadget *MaxTransfer;
extern struct Gadget *ReservedBegin;
extern struct Gadget *ReservedEnd;
extern struct Gadget *CustomNum;
extern struct Gadget *FFSCheck;		/* 39.13 */
extern struct Gadget *IntlMode;		/* 39.13 */
extern struct Gadget *DirCache;		/* 39.13 */
extern struct Gadget *AutoMount;
extern struct Gadget *CustomBoot;

/************************* File System Maintenance *************************/

extern struct Gadget *FSMOk;
extern struct Gadget *FSMCancel;
extern struct Gadget *FSMAdd;
extern struct Gadget *FSMDelete;
extern struct Gadget *FSMUpdate;
extern struct Gadget *FSMList;

/*************************** Define/Edit Drive Type ************************/

extern struct Gadget *NDOk;
extern struct Gadget *NDCancel;
extern struct Gadget *NDReadParms;
extern struct Gadget *NDRevisionName;
extern struct Gadget *NDDriveName;
extern struct Gadget *NDManuName;
extern struct Gadget *NDName;
extern struct Gadget *NDCylinders;
extern struct Gadget *NDHeads;
extern struct Gadget *NDBlocks;
extern struct Gadget *NDCylBlocks;
extern struct Gadget *NDReduced;
extern struct Gadget *NDPreComp;
extern struct Gadget *NDParkWhere;
extern struct Gadget *NDReselect;
extern struct Gadget *NDSize;

/******************************* Bad Blocks ********************************/

extern struct Gadget *BadOk;
extern struct Gadget *BadCancel;
extern struct Gadget *BadAdd;
extern struct Gadget *BadDelete;
extern struct Gadget *BadList;

/***************************** Set Drive Type ******************************/

extern struct Gadget *SetTypeOK;
extern struct Gadget *SetTypeCancel;
extern struct Gadget *DefineDrive;
extern struct Gadget *EditType;
extern struct Gadget *DeleteType;
extern struct Gadget *TypeList;
extern struct Gadget *SCSIType;

/***************************** Requester stuff *****************************/

extern struct Gadget NotifyGad;
extern struct Gadget SureContinueGad;
extern struct Gadget SureCancelGad;

extern struct Gadget NoLLFormat;
extern struct Gadget LLFormat;
extern struct Gadget LLCancel;

extern struct Gadget GetFileName;
extern struct Gadget FsGetNameOk;
extern struct Gadget FsGetNameCancel;

extern struct Gadget GetDosType;
extern struct Gadget GetFileName;
extern struct Gadget FsGetTypeOk;	// 39.9
extern struct Gadget FsGetTypeCancel;	// 39.9

extern struct Gadget BadGetOK;
extern struct Gadget BadGetCancel;
extern struct Gadget BadCylinder;
extern struct Gadget BadHead;
extern struct Gadget BadBfi;
extern struct Gadget BadSector;

extern struct Gadget MapOut;
extern struct Gadget MapIgnore;
extern struct Gadget MapStop;

/***************************** Reserved stuff ******************************/

//extern struct Gadget UndoLast;
//extern struct Gadget Priority;
//extern struct Gadget BufMemType;

//extern struct Gadget StackSize;
//extern struct Gadget GlobVec;
//extern struct Gadget NDOmti;

extern struct Gadget NDAdaptec;
extern struct Gadget NDNormal;
extern struct Gadget NDNormalST506;
extern struct Gadget NDSynchronous;
extern struct Gadget NDParkHead;
extern struct Gadget NDParkSeconds;

/****************************** Special stuff ******************************/

extern struct PropInfo   DragPartSInfo;
extern struct PropInfo   SizePartSInfo;

extern struct StringInfo NamePartSInfo;
extern struct StringInfo MaskSInfo;
extern struct StringInfo FSIdentifierSInfo;
extern struct StringInfo MaxTransferSInfo;
extern struct StringInfo NDNameSInfo;
extern struct StringInfo NDManuNameSInfo;
extern struct StringInfo NDDriveNameSInfo;
extern struct StringInfo NDRevisionNameSInfo;
extern struct StringInfo GetVersionSInfo;
extern struct StringInfo GetRevisionSInfo;	// 39.9
extern struct StringInfo GetDosTypeSInfo;
extern struct StringInfo GetFileNameSInfo;
extern struct StringInfo BadCylinderSInfo;
extern struct StringInfo BadHeadSInfo;
extern struct StringInfo BadBfiSInfo;
extern struct StringInfo BadSectorSInfo;

extern struct StringExtend StringExt;	/* 39.13 */

extern UBYTE  NamePartSIBuff[];
extern UBYTE  MaskSIBuff[];
extern UBYTE  MaxTransferSIBuff[];
extern UBYTE  FSIdentifierSIBuff[];
extern UBYTE  NDRevisionNameSIBuff[];
extern UBYTE  NDDriveNameSIBuff[];
extern UBYTE  NDManuNameSIBuff[];
extern UBYTE  NDNameSIBuff[];
extern UBYTE  GetFileNameSIBuff[];
extern UBYTE  GetDosTypeSIBuff[];
extern UBYTE  GetVersionSIBuff[];
extern UBYTE  GetRevisionSIBuff[];	// 39.9
extern UBYTE  BadSectorSIBuff[];
extern UBYTE  BadBfiSIBuff[];
extern UBYTE  BadHeadSIBuff[];
extern UBYTE  BadCylinderSIBuff[];
