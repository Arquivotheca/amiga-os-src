/*
 * BPB Definition:
 */
struct BPB {			/* BIOS Parameter Block */
/*
 * Start of Boot Block
 */
	UBYTE	BootJmp[3];	/* Near Jump To Boot Code (not needed) */
	UBYTE	OEMName[8];	/* ASCII Ident. */
/*
 * BPB itself:
 */
	UWORD	SectorLength;	/* Bytes Per Sector */
	UBYTE	ClusterSecs;	/* Sectors Per Cluster */
	UWORD	ResrvdSecs;	/* Reserved Sectors */
	UBYTE	NumFATs;	/* Number of FAT's */
	UWORD	RootEntries;	/* Number of Root Dir Entries */
	UWORD	NumSecs;	/* Number of Sectors in Log. Image */
	UBYTE	MediaDescr;	/* Media Descriptor */
	UWORD	FATSecs;	/* Number of FAT Sectors */
/*
 * BPB Extension
 */
	UWORD	TrackSecs;	/* Sectors Per Track */
	UWORD	NumHeads;	/* Number of Heads */
	UWORD	HiddenSecs;	/* Number of hidden Sectors */
	UBYTE	pad[512-30];	/* rest of block */
} BPBSect;

