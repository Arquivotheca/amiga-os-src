/***********************************************************************
***
***  ISO 9660 INCLUDE FILE
***
***      Copyright (C) 1990 Commodore-Amiga Inc.
***      Developed by Carl Sassenrath for Commodore International
***
***      Feb 1991
***
***      STRICTLY CONFIDENTIAL!  Do not release!
***      This file is subject to change before product release.
***      It should not be published in any form.
***
***********************************************************************/

typedef	char	DCHAR;
typedef	char	ACHAR;
typedef	unsigned char DTIME[17];

#define	STD_IDENT "CD001"
/*                  123456789.123456789.123456789.12 */
#define	SYS_IDENT  "CDTV                            "
#define	SYS_IDENT2 "COMMODORE_AMIGA                 "

/***********************************************************************
***
***  Path Table Entry Structure
***
***********************************************************************/
struct PathTableEntry
{
	UBYTE	LenDirIdent;
	UBYTE	LenDirXAR;
	ULONG	Extent;
	UWORD	ParentDirNum;
	DCHAR	DirIdent[2];
};

typedef	struct PathTableEntry PTE; 
#define NEXTPTE(p) (PTE *) (((LONG)&(p)->DirIdent + (p)->LenDirIdent + 1) & 0xfffffffe)

/***********************************************************************
***
***  Directory Record
***
***********************************************************************/
struct	DirRecord
{
	UBYTE	LenDirRec;
	UBYTE	LenXAR;
	ULONG	ExtentX;
	ULONG	Extent;
	ULONG	LenDataX;
	ULONG	LenData;
	UBYTE	RecTime[7];
	UBYTE	FileFlags;
	UBYTE	UnitSize;
	UBYTE	GapSize;
	UWORD	VolSeqNumX;
	UWORD	VolSeqNum;
	UBYTE	LenFileIdent;
	DCHAR	FileIdent[1];
};

typedef	struct DirRecord DIREC;
#define NEXTDIR(p) (DIREC *)(((LONG)(p) + (p)->LenDirRec))

/***********************************************************************
***
***  ISO Volume Descriptor Structures
***
***********************************************************************/

/*
**  Volume Descriptor Types
*/
#define	BOOT_VOL_DESC	0
#define	PRIM_VOL_DESC	1
#define	SUPP_VOL_DESC	2
#define	PART_VOL_DESC	3
#define	TERM_VOL_DESC	255

/*
**  Bootstrap Volume Descriptor
**
**	Contains an identifier which must be checked before this
**	descriptor is used for booting.
*/
struct	BootVolDescriptor
{
	UBYTE	VolDescType;		/* set to 0	*/
	DCHAR	StdIdent[5];		/* set to CD001 */
	UBYTE	VolDescVers;		/* set to 1	*/
	DCHAR	BootSysIdent[32];
	DCHAR	BootIdent[32];
	UBYTE	BootSysUse[1976];
};
typedef	struct BootVolDescriptor BVD;

/*
**  Primary Volume Descriptor
**
**	Contains main information about the volume, including
**	pointers to other important structures.
*/
struct	PrimVolDescriptor
{
	UBYTE	VolDescType;		/* set to 1		*/
	DCHAR	StdIdent[5];		/* set to CD001		*/
	UBYTE	VolDescVers;		/* set to 1		*/
	UBYTE	Unused1;		/* always zero		*/
	ACHAR	SysIdent[32];		/* system identifier	*/
	DCHAR	VolIdent[32];		/* volume identifier	*/
	UBYTE	Unused2[8];		/* set to zeros		*/
	ULONG	VolSizeX;
	ULONG	VolSize;		/* vol space size	*/
	DCHAR	Unused3[32];		/* set to zero		*/
	UWORD	VolSetSizeX;
	UWORD	VolSetSize;
	UWORD	VolSeqNumX;
	UWORD	VolSeqNum;
	UWORD	BlockSizeX;
	UWORD	BlockSize;		/* logical block size	*/
	ULONG	PathTableSizeX;
	ULONG	PathTableSize;
	ULONG	LPathTable[2];		/* rev byte ordering	*/
	ULONG	MPathTable[2];
	DIREC	RootDir;
	DCHAR	VolSetIdent[128];
	ACHAR	PublisherIdent[128];
	ACHAR	DataPrepIdent[128];
	ACHAR	ApplicatIdent[128];
	DCHAR	CopyrightFile[37];
	DCHAR	AbstractFile[37];
	DCHAR	BiblioFile[37];
	DTIME	VolCreateTime;
	DTIME	VolModifyTime;
	DTIME	VolExpireTime;
	DTIME	VolEffectTime;
	UBYTE	FileStructVers;		/* set to 1		*/
	UBYTE	Reserved1;		/* set to 0		*/
	UBYTE	ApplicationUse[512];
	UBYTE	Reserved2[653];		/* set to 0		*/
};

typedef	struct PrimVolDescriptor PVD;

/*
**  Terminating Volume Descriptor
**
**	Terminates a set of volume descriptors.
*/
struct	TermVolDescriptor
{
	UBYTE	VolDescType;		/* set to 255	*/
	DCHAR	StdIdent[5];		/* set to CD001 */
	UBYTE	VolDescVers;		/* set to 1	*/
	UBYTE	Reserved[2040];		/* set to zeros	*/
};


/***********************************************************************
***
***  CDTV Trademark FS Option Structure
***
***	Part of the ApplicationUse area of the Primary Vol Desc.
***	See CDTV File System notes or documents.
***
***********************************************************************/
struct	TMStruct
{
	ULONG	Size;
	ULONG	Sectors[4];
};
