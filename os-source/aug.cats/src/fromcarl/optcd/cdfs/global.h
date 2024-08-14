/* GLOBAL DATA SEGMENT */

/******	GENERAL *******************************************************/

GLOBAL	struct	Library *LibBase;	/* CD FS base	*/
GLOBAL	LONG	DebugLevel;
GLOBAL	struct	BootNode CDFSBoot;
GLOBAL	struct	DosList *DOSNode;
GLOBAL	struct	DosLibrary *DOSBase;
GLOBAL	struct	ExpansionBase *ExpBase;
GLOBAL	struct	Library *TimerBase;	/* ewhac/KMY (2414) */
GLOBAL	struct	Process *BootProc;
GLOBAL	struct	Process *FSProc;
GLOBAL	struct	MsgPort *FSPort;
GLOBAL	struct	MsgPort BootPort;
GLOBAL	struct	IOStdReq DevIOReq;
GLOBAL	struct	IOStdReq DevChgReq;
GLOBAL	struct	timerequest TimeReq;	/* ewhac/KMY (2414) */
GLOBAL	struct	Interrupt DiskChgInt;
GLOBAL	struct	IOStdReq InpReq;
GLOBAL	struct	DosPacket InpPacket;
GLOBAL	struct	InputEvent InpEvent;

/******	DOS VARIABLES *************************************************/

GLOBAL	UBYTE	DeviceName[32];
GLOBAL	BSTR	HandlerName;
GLOBAL	ULONG	UnitNumber;
GLOBAL	ULONG	OpenFlags;
GLOBAL	APTR	EnvrVec;
GLOBAL	struct	VolDev *ThisVol;	/* ptr to our volume's dev-list entry */
GLOBAL	struct	DosInfo	*DOSInfo;

/******	PACKET VARIABLES **********************************************/

GLOBAL	struct	DosPacket *Packet;
GLOBAL	LONG	PktArg1, PktArg2, PktArg3, PktArg4;
GLOBAL	LONG	PktRes1, PktRes2;

/******	ISO VARIABLES *************************************************/

GLOBAL	ULONG	BootPVDLSN;	/* sector for Boot PVD	*/
GLOBAL	ULONG	BootRecLSN;	/* sector for Boot Rec, 0 if none */
GLOBAL	UBYTE	BootFile[32];	/* bootstrap file name 	*/
GLOBAL	ULONG	MaxBlocks;	/* max LBN's on disk	*/
GLOBAL	ULONG	BlockSize;	/* LBN size		*/
GLOBAL	ULONG	BlockShift;	/* as a power of two	*/
GLOBAL	ULONG	BlockMask;	/* BlockSize - 1	*/
GLOBAL	ULONG	RootDirLBN;	/* LSN of root dir	*/
GLOBAL	ULONG	PathTableSize;	/* size in bytes	*/
GLOBAL	ULONG	MaxPaths;	/* total paths		*/
GLOBAL	struct	PathTableEntry *PathTable;
GLOBAL	struct	PathTableEntry **PathIndex;

/******	CACHE VARIABLES ***********************************************/

GLOBAL	ULONG	CacheBlock;
GLOBAL	ULONG	CacheSize;		/* size of cache in Blocks	*/
GLOBAL	UBYTE	*Cache;			/* ptr to cache memory		*/
GLOBAL	ULONG	DirCacheBlock;
GLOBAL	ULONG	DirCacheSize;		/* size of cache in Blocks	*/
GLOBAL	ULONG	*DirCacheIndex;
GLOBAL	UBYTE	*DirCache;		/* ptr to cache memory		*/

/******	OTHER VARIABLES ***********************************************/

GLOBAL	ULONG	LockPoolSize;		/* number of elements in pool	*/
GLOBAL	ULONG	FilePoolSize;		/* number of elements in pool	*/
GLOBAL	POOL	*LockPool;		/* Memory pool of file locks	*/
GLOBAL	POOL	*FilePool;		/* Memory pool of file handles	*/
GLOBAL	UCHAR	*CharSet;		/* Pointer to char map		*/
GLOBAL	UBYTE	*ScratchBlock;		/* Temp block used for PVD	*/
GLOBAL	LONG	ReadErrs;		/* Number of read errors	*/
GLOBAL	LONG	Retry;			/* Number of retries on error	*/
GLOBAL	LONG	NoReset;		/* no reset on disk eject	*/
GLOBAL	LONG	InhibitCount;		/* inhibit nesting counter	*/
GLOBAL	LONG	BlockCount;		/* blocks requested (total)	*/
GLOBAL	LONG	HitCount;		/* blocks hit in cache		*/
GLOBAL	LONG	ReadCount;		/* blocks read from disk	*/
GLOBAL	LONG	DirectBytes;		/* counter: direct read bytes	*/
GLOBAL	LONG	ZeroPoint;		/* Location of block zero	*/	/* ewhac/KMY (2414) */
GLOBAL	LONG	SimuSpeed;		/* Reading speed to simulate	*/	/* ewhac/KMY (2414) */
GLOBAL	struct	TMStruct TMInfo;	/* Trade mark sector info V22.4	*/

GLOBAL	LOGIC	InsertFlag;		/* Indicates disk status chgd	*/
GLOBAL	LOGIC	InputPends;		/* Input event has not returned	*/
GLOBAL	LOGIC	NoPrefs;		/* For Welcome disk		*/
GLOBAL	LOGIC	FastSearch;		/* Flag that we want fast search*/

GLOBAL	LOGIC	DirectRead;		/* Direct file reading 	V23.3	*/
GLOBAL	LOGIC	UsingCDTV;		/* TRUE if using cdtv.device	*/	/* ewhac/KMY (2414) */
GLOBAL	LOGIC	FSECollect;		/* Toggle FSE msg collection	*/	/* KMY (2710) */
GLOBAL	LOGIC	Pad;

/******	FSE VARIABLES *************************************************/
/* ewhac/KMY (2707) */
GLOBAL	struct	MsgPort FSEPort;
GLOBAL	struct	MemHeader FSEPool;
GLOBAL	struct	SignalSemaphore FSELock;
GLOBAL	LONG	FSEMonitor;

IMPORT	UCHAR	UpperChars[];
