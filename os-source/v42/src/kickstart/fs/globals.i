;==============================================================================
; This is a statistics block maintained by various portions of the file system.
; Use the new packet type ACTION_STATS to get a copy of the current statistics.
; When an ACTION_STATS is recieved, the statistics are copied to a user array
; and the current statistics are zeroed.  This makes it easy to do running stat
; averages by examining the s_TIMER field to see how many seconds passed since
; the last stat call.
;==============================================================================
	STRUCTURE stats,0
; counts of the number of requests received for the various packet types
		ULONG	s_TIMER			secs since last stat request
		ULONG	s_READ
		ULONG	s_WRITE
		ULONG	s_SEEK
		ULONG	s_EXAMINE_NEXT
		ULONG	s_EXAMINE_OBJECT
		ULONG	s_LOCATE_OBJECT
		ULONG	s_COPY_DIR
		ULONG	s_FREE_LOCK
		ULONG	s_FIND_OUTPUT
		ULONG	s_FIND_INPUT
		ULONG	s_FIND_UPDATE
		ULONG	s_END
		ULONG	s_PARENT
		ULONG	s_TRUNCATE
		ULONG	s_CREATE_DIR
		ULONG	s_RENAME_OBJECT
		ULONG	s_DELETE_OBJECT
		ULONG	s_INFO
		ULONG	s_DISK_INFO
		ULONG	s_CURRENT_VOLUME
		ULONG	s_FLUSH
		ULONG	s_MORE_CACHE
		ULONG	s_INHIBIT
		ULONG	s_SET_DATE
		ULONG	s_SET_COMMENT
		ULONG	s_SET_PROTECT
		ULONG	s_RENAME_DISK
		ULONG	s_WRITE_PROTECT

		ULONG	s_LargestRead		largest read request
		ULONG	s_SmallestRead		smallest read request
		ULONG	s_TotalRead		total bytes in read requests

		ULONG	s_LargestWrite		largest write request
		ULONG	s_SmallestWrite		smallest write request
		ULONG	s_TotalWrite		total bytes in write requests

; s_DiskReads-s_HeaderReads = number of data reads (may be more than 1 block).
		ULONG	s_DiskReads		# sendio's for read
		ULONG	s_DiskWrites		# sendio's for write
; use same formula for writes
		ULONG	s_HeaderReads		# reads of file headers
		ULONG	s_HeaderWrites		# writes of file headers

; these are only filled in at the time of the statistics request
		ULONG	s_NumFree		cache buffers on free queue
		ULONG	s_NumPending		cache buffers waiting for IO
		ULONG	s_NumWaiting		cache buffers just waiting
		ULONG	s_NumValid		cache buffers cached
		ULONG	s_NumLocks		number of locks on this vol

; bitmap management statistics and file fragmentation stuff
		ULONG	s_KeysFreed		number of keys freed
		ULONG	s_KeysAlloced		number of keys allocated
		ULONG	s_AllocBreaks		times a contiguous alloc failed

; various cache performance stats, can combine with DiskReads and DiskWrites
; to pull out further info.  Cache buffers are ONLY used for block sized I/O
		ULONG	s_ValidHits		times buffer found in valid
		ULONG	s_ValidMisses		times buffer not on valid
		ULONG	s_ValidTrash		time write killed valid buff
		ULONG	s_NotFree		had to wait for a free block
	LABEL stats_SIZEOF

;STATISTICS EQU 1	define this to turn on statistics

;==============================================================================
; dsem: this is used for directory locks.  see dirlock.asm
;==============================================================================

	STRUCTURE dsem,0	
		STRUCT	dsem_Node,MLN_SIZE
		LONG	dsem_Block	block number locked/waiting or 0
		APTR	dsem_NextCo	points to another dsem
		APTR	dsem_OwnCo	which coroutine owns this dsem
	LABEL dsem_SIZEOF

;==============================================================================
; The following global data structure is allocated at startup time and pointed
; at by (a5) for the rest of the program.  This gives us a re-entrant and pure
; filehandler with separate globals for each invocation. (Neat huh ?)
;==============================================================================
	STRUCTURE globals,0
; note: this file must be included AFTER the statistics file if stats are
; required.  This will insert the statistics block at the head of the globals
		IFD	STATISTICS
		STRUCT  StatStuff,stats_SIZEOF
		ENDC

; required information about the device this filing system is mounted on
		ULONG	IsTrackDisk	TRUE if using trackdisk.device
		ULONG	UnitNumber	unit number of device we're opening
		ULONG	OpenFlags	flags for opening device
		ULONG	DeviceName	name of device we're opening (BSTR)
		APTR	CurrentEnv	pointer to environment vector
		APTR	DevEntry	pointer to our DOS device node
		APTR	ReqString	buffer for requester strings
		APTR	ButtonString	buffer for Retry|Cancel string
		APTR	ReqArgs		arguments for printf style strings

; general IO request block pointers
		APTR	DiskDev		disk IO request block
		APTR	DiskExtraDev	spare IO request block
		APTR	TimerDev	timer request block
		APTR	TimerExtraDev	for getting datestamps
		APTR	DoIOPort	where DoIO requests go to
		APTR	InputDev	input device IORequest
		ULONG	TimerOpened	0 if true
		ULONG	DiskOpened	0 if true
		ULONG	InputOpened	0 if true
		APTR	TimerPacket	linked into timer IO requests
		APTR	InputPacket	linked into input dev packets
		APTR	InpEvent	an inputevent struct
		APTR	ChangeInt	handler for disk changed interrupt
		UWORD	DiskChanged	flag, 0=disk changed, !=0 = not changed
		UWORD	ChangeInhibit	flag, 0=notice disk changes
		ULONG	DiskSig		signal bit for disk changes

; this is all the required information about the current physical disk device
		ULONG	NSurfaces	number of surfaces
		ULONG	NSecsPerTrack	blocks in one track
		ULONG	NSecsPerCyl	calculated from above
		ULONG	PreAlloc	not used any more
		ULONG	Intleave	amount of interleave to use
		ULONG	LowerCyl	start cylinder
		ULONG	UpperCyl	end cylinder
		ULONG	BufferMemType	type of memory to read into
		ULONG	NumBuffers	amount of cache buffers to use
		ULONG	NBlocks		total blocks on this volume
		ULONG	RootKey		where the root is
		ULONG	LargestWrite	maximum size of write requests

; New stuff added for better support of variable sized blocks.
		ULONG	BlockSize	byte size of a disk block
		ULONG	BytesPerData	OFS format, #data bytes in a block
		ULONG	HTSize		number of entries in hashtable
		UWORD	MaxEntry	maximum byte offset (to end of hashtab)
		UWORD	BlockShift	#shifts to convert bytes -> blocks
		ULONG	LowerByte	start block byte offset

; New stuff added for better bitmap support
		ULONG	HighestBlock	highest possible key number
		ULONG	Reserved	blocks reserved at beginning of disk
		APTR	BitmapKeys	pointer to list of bitmap keys
		ULONG	BlocksFree	number of blocks free on the disk
		ULONG	StartSearch	where to start searching from
		ULONG	BlocksPerBM	#data blocks per bitmap block
		ULONG	BitmapCount	number of bitmap blocks

; new field, place holder for the list of open files
		ULONG	OpenFiles

; a bit more global info about the physical disk currently in the drive
		UWORD	WrProt		flag, write protected
		UWORD	UserWrProt	users requested write protection
		ULONG	Password	password for soft write protect
		ULONG	DiskType	DOS,KICK,BAD etc.
		ULONG	UsedBlocks	number of allocated blocks

		UWORD	DiskRunning	flag, disk co-routine running
		UWORD	MotorTicks	Current number of motor ticks
		ULONG	CurrentVolume	current volume ID
		ULONG	SoftErrors	errors on this volume

; various flags and pointers associated with bitmap management when validating
		UWORD	BitmapValid	if TRUE bitmap on disk is OK (write)
		UWORD	FilesOpen	if TRUE bitmap buffers need flushing
		APTR	BMBlock		spare cache buffer for bitmap blocks
		ULONG	ClosePkt	Packet to return when Flush called

; the various queues maintained by the filing system for cache management.
		STRUCT	ValidBufferQueue,MLH_SIZE linked list of valid buffers
		STRUCT	PendingQueue,MLH_SIZE     about to read/write
		STRUCT	WaitingQueue,MLH_SIZE	  waiting to be written
		APTR	FreeBufferQueue		  buffers not in use
		APTR	CacheHash		  pointer to cache hash table

		APTR	LockQueue		list of current locks
		APTR	RLockQueue		list of record locks

; directory lock queue, for locking access across WaitCo's
		STRUCT	DirSemList,MLH_SIZE	list of locked dir blocks
		APTR	RenameCo		current rename coroutine
		APTR	RenameWaitCo		head of renameco  waiters
		STRUCT	Rename_dsem,dsem_SIZEOF second dirlock for Rename

; Various cached things to save a bit of time in different routines
		APTR	ThisTask	task struct pointer
		APTR	ThisDevProc	process part of task pointer (MsgPort)
		APTR	DosLib		the dos library (for getting rootstruct)
		APTR	UtilityBase	Utility library pointer (requires 2.0!)

; important co-routine support pointers
		APTR	RootCo		always the same
		APTR	CurrentCo	varies
		APTR	StackQueue	queue of available co-routine stacks
		APTR	CoChunkList	queue of mem chunks for stacks
; base pointers for the major co-routines that hang about all the time.
		APTR	DiskCo		the main disk co-routine
		APTR	KillCo		coroutine killer

		ULONG	ErrorCode	last error reported
		ULONG	PrevKey		previous key set by findentry
		ULONG	ObjKey		  object key ................

		ULONG	GMaxTransfer	maximum to transfer in one go
		ULONG	GAddressMask	valid memory for transfer

		UWORD	TimerRunning	flag, timer request is pending
		UWORD	InputSent	flag, inputevent is in use
		UWORD	IntChars	flag, support international chrs
		UWORD	TotallyFull	no room for the bitmap
	LABEL global_SIZEOF

; CONSTANTS
TRUE	EQU	-1
FALSE	EQU	0

BUFF_VALID EQU TRUE
BUFF_INVALID EQU FALSE

GRAB EQU TRUE
WAIT EQU FALSE

