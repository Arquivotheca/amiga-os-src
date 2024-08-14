/* structure of the very first block on a hard disk */

struct hdb {
	UWORD	hdb_Magic1;		/* 0xBABE indicates valid block */
	UWORD	hdb_Fill1;		/*  Skip 16 bits	*/
	ULONG	hdb_Pt_bblocks;		/*  Pointer to 1st bad block record*/
	ULONG	hdb_Cylinders;		/*  # of cylinders on drive*/
	ULONG	hdb_Precomp;		/*  Pre-comp cylinder #*/
	ULONG	hdb_Reduce;		/*  Reduced write current cylinder*/
	ULONG	hdb_Park;		/*  Cylinder to park heads at (0 = don't)*/
	UWORD	hdb_Sectors;		/*  Sectors per track*/
	UWORD	hdb_Heads;		/*  Surfaces on device*/
	struct	Envec hdb_Environment;	/*  AmigaDOS Environment for 1st partition*/
	UWORD	hdb_Hoffset;		/*  physical offset if boot time hdlr */
	UWORD	hdb_Hsize;		/*  physical size if boot time hdlr */
	UBYTE	hdb_Hname[32];		/*  name of handler proc */
	ULONG	hdb_Tbuffs;		/*  # of track buffers (if applicable)*/
	struct sdb hdb_Parts[6];
};

/* stripped down environment vector struct for each partition */

struct sdb {
	UWORD	sdb_SectorOrigin;		/* start at 0 or 1 */
	UWORD	sdb_SectorsPerBlock;
	UWORD	sdb_BlocksPerTrack;
	UWORD	sdb_ReservedBlocks;
	ULONG	sdb_StartupValue;
	UWORD	sdb_Interleave;
	ULONG	sdb_LowerCylinder;
	ULONG	sdb_UpperCylinder;
	UWORD	sdb_Buffers;
	UWORD	sdb_Hoffset;		/*  physical offset if boot time hdlr */
	UWORD	sdb_Hsize;		/*  physical size if boot time hdlr */
	UBYTE	sdb_Name[32];
};

struct hdb1 {				/* second disk block info */
	UWORD	hdb1_Magic1;
	UWORD	hdb1_Pad1;
	struct  sdb hdb1_parts[8];
}
	
struct Envec {
	ULONG env_EnvecSize;	/* size of this structure in longwords  */
	ULONG env_SectorSize;	/* size of one sector (usually 512) 	*/
	ULONG env_SectorOrigin;	/* lowest sector number (0 ???)		*/
	ULONG env_Surfaces;		/* number of surfaces		*/
	ULONG env_SectorsPerBlock;	/* number of sectors in one block */
	ULONG env_BlocksPerTrack;	/* number of blocks in one track */
	ULONG env_ReservedBlocks;	/* blocks reserved for dos (2)	 */
	ULONG env_PreAllocated;	/* ??? this ain`t used no more		*/
	ULONG env_Interleave;	/* very rarely used			*/
	ULONG env_LowerCylinder;	/* start cylinder of this partition */
	ULONG env_UpperCylinder;	/* end cylinder of this partition */
	ULONG env_Buffers;		/* number of buffers to allocate */
	ULONG env_BuffMemType;	/* type of memory buffers need		*/
	ULONG *env_Startup;	/* startup parameter */
	char  *env_HandlerName;	/* name if used with mount */
	UWORD env_HandlerSize;		/* size of handler or 0 */
	UWORD env_HandlerOffset;	/* offset to handler or 0 */
};

