	STRUCTURE fs,0
		ULONG  fs_Unit		unit number of device 	 
		ULONG  fs_DevName	name of the device (BSTR)
		ULONG  fs_Env		environment vector (BPTR)
		ULONG  fs_Flags		flags for opening the device	 
	LABEL fs_SIZEOF

	STRUCTURE Envec,0
		ULONG  EnvecSize	size of this structure in longwords  
		ULONG  SectorSize	size of one sector (usually 512)  
		ULONG  SectorOrigin	lowest sector number (0 ???)	 
		ULONG  Surfaces		number of surfaces		 
		ULONG  SectorsPerBlock	number of sectors in one block 
		ULONG  BlocksPerTrack	number of blocks in one track 
		ULONG  ReservedBlocks	blocks reserved for dos (2)	 
		ULONG  PreAllocated	??? this ain`t used no more	 
		ULONG  Interleave	very rarely used		 
		ULONG  LowerCylinder	start cylinder of this partition  
		ULONG  UpperCylinder	end cylinder of this partition 
		ULONG  Buffers		number of buffers to allocate 
		ULONG  BuffMemType	type of memory buffers need	 
		ULONG  MaxTransfer
		ULONG  AddressMask
		ULONG  BootPri
		ULONG  dostype
	LABEL env_SIZEOF

	STRUCTURE RootBlock,0
		ULONG  rb_Type		t.short
		ULONG  rb_OwnKey	always 0
		ULONG  rb_SeqNum	always 0
		ULONG  rb_HTSize	size of hashtable
		ULONG  rb_Nothing1
		ULONG  rb_Checksum	checksum of whole rootblock
		LABEL  rb_HashTable
; These labels have all been added for variable block size support.  Members
; are accessed relative to the very end of the block.  Just add cb_SIZEOF and
; the block size to the cache buffer pointer then use these labels to access
; the various items at the end of the block relative to that pointer.
vrb_BitmapFlag	EQU -200	TRUE (-1) if bitmap is valid
vrb_Bitmap	EQU -196	indicates where bitmap pages are
vrb_BitExtend	EQU -96		extender key for bitmap (0=none)
vrb_Days	EQU -92		last altered time and date....
vrb_Mins	EQU -88		...for the root directory
vrb_Ticks	EQU -84
vrb_Name	EQU -80		volume name as a BCPL string
vrb_Nothing4	EQU -44		Link in others
vrb_DiskMod	EQU -40		last time volume was altered
vrb_CreateDays	EQU -28		volume creation date and time
vrb_CreateMins	EQU -24
vrb_CreateTicks	EQU -20
vrb_Nothing2	EQU -16		hashchain in others
vrb_Nothing3	EQU -12		parent directory (always 0)
vrb_DirList	EQU -8		must be the same as vudb_DirList
vrb_SecType	EQU -4		secondary type, st.root, indicates root

	STRUCTURE UserDirectoryBlock,0
		ULONG  udb_Type		t.short
		ULONG  udb_OwnKey	key of this block
		ULONG  udb_SeqNum	highest seq (always 0)
		ULONG  udb_DataSize	always 0
		ULONG  udb_NextBlock	always 0
		ULONG  udb_Checksum	checksum of this block
		LABEL  udb_HashTable
; These labels have all been added for variable block size support.  Members
; are accessed relative to the very end of the block.  Just add cb_SIZEOF and
; the block size to the cache buffer pointer then use these labels to access
; the various items at the end of the block relative to that pointer.
vudb_Spare1	EQU -200
vudb_OwnerXID	EQU -196	owner UID/GID
vudb_Protect	EQU -192	protection bits
vudb_Junk	EQU -188	not used (always 0)
vudb_Comment	EQU -184	comment as a BCPL string
vudb_Days	EQU -92		creation date and time
vudb_Mins	EQU -88
vudb_Ticks	EQU -84
vudb_DirName	EQU -80		name as a BCPL string
vudb_Link	EQU -44		pointer (key number) to object linked to
vudb_BackLink	EQU -40		back pointer to previous hard link header
vudb_HashChain	EQU -16		next entry with same hash value
vudb_Parent	EQU -12		pointer back to parent
vudb_Extension	EQU -8		extension blocks (always 0)
vudb_DirList	EQU vudb_Extension	dos\4 uses this for the dirlist
vudb_SecType	EQU -4		secondary type st.userdir

	STRUCTURE FileHeaderBlock,0
		ULONG  fhb_Type		t.short
		ULONG  fhb_OwnKey	key of this block
		ULONG  fhb_HighSeq	total blocks in file
		ULONG  fhb_DataSize	number of data block slots used
		ULONG  fhb_FirstBlock	first block in this file
		ULONG  fhb_Checksum	checksum of this block
		LABEL  fhb_HashTable	actually data block keys
; These labels have all been added for variable block size support.  Members
; are accessed relative to the very end of the block.  Just add cb_SIZEOF and
; the block size to the cache buffer pointer then use these labels to access
; the various items at the end of the block relative to that pointer.
vfhb_Junk1	EQU -200	not used
vfhb_OwnerXID	EQU -196	owner UID/GID
vfhb_Protect	EQU -192	protection bits
vfhb_ByteSize	EQU -188	total size of file in bytes
vfhb_Comment	EQU -184	comment as a BCPL string
vfhb_Days	EQU -92		creation date and time
vfhb_Mins	EQU -88
vfhb_Ticks	EQU -84
vfhb_FileName	EQU -80		filename as BCPL string
vfhb_Link	EQU -44		pointer to object header is linked to
vfhb_BackLink	EQU -40		pointer to previous object in link chain
vfhb_HashChain	EQU -16		next entry with same hash value
vfhb_Parent	EQU -12		pointer back to parent directory
vfhb_Extension	EQU -8		0 or pointer to first extension block
vfhb_SecType	EQU -4		st.file

; File list blocks (linked into fhb_Extension) can be treated exactly the same
; as a struct FileHeaderBlock except that the info area will be invalid and
; the block will be of type t.list.
	STRUCTURE FileDataBlock,0
		ULONG  fdb_Type		t.data
		ULONG  fdb_OwnKey	key of this block
		ULONG  fdb_SeqNum	number of this block
		ULONG  fdb_DataSize	amount of data on this block
		ULONG  fdb_NextBlock	next block in this file
		ULONG  fdb_Checksum	checksum of this block
		LABEL  fdb_Data		where the data starts
	LABEL fdb_SIZEOF		data follows here

; compressed directory lists are much like data blocks.  Seqnum isn't used,
; and numentries could be a word, but that would cause extra code to be needed
; elsewhere.  Seqnum was replaced by fhl_Parent
	STRUCTURE FileListBlock,0
		ULONG  fhl_Type		t.data
		ULONG  fhl_OwnKey	key of this block
		ULONG  fhl_Parent	key of parent directory
		ULONG  fhl_NumEntries	amount of data on this block
		ULONG  fhl_NextBlock	next block in this file
		ULONG  fhl_Checksum	checksum of this block
		LABEL  fhl_Entries	where the data starts
	LABEL fhl_SIZEOF		data follows here

; this is the maximum size
	STRUCTURE ListEntry,0
		ULONG	fle_Key		Key (fhb block) of file
		ULONG	fle_Size	size in bytes
		ULONG	fle_Protection	protection bits
		ULONG	fle_OwnerXID	(note: first beta didn't have these)
		UWORD	fle_Days	date (allows 179 years)
		UWORD	fle_Min		0..(60*24 - 1)
		UWORD	fle_Tick	0..3599
		UBYTE	fle_Type	st.xxxxx (all fit in a byte)
	    LABEL	fle_Fixed	end of the fixed fields
		STRUCT	fle_FileName,31	Actually 1-31 bytes
		STRUCT	fle_Comment,80	Actually 1-80 bytes
	LABEL fle_SIZEOF

; file type constants
T_DELETED	EQU 1
T_SHORT		EQU 2
T_LONG		EQU 4
T_DATA		EQU 8
T_LIST		EQU 16
; for dirlist see private.i

; secondary type constants
ST_SOFT_LINK	EQU 3	; a soft link (path/name held in header for ReadLink)
ST_FLINK	EQU -4	; a file link
ST_DLINK	EQU 4	; a directory link

; protection bits
PROT_READ	EQU 8
PROT_WRITE	EQU 4
PROT_EXECUTE	EQU 2
PROT_DELETE	EQU 1


	STRUCTURE Control,0
		ULONG	c_Entries	number of entries in buffer (return)
		ULONG	c_LastKey	for keeping track (must be 0 to start)
		APTR	c_MatchString	pointer to BSTR for pattern matching
		APTR	c_MatchFunc	pointer to function for comparing
	LABEL Control_SIZEOF

EA_NAME					EQU 1
EA_NAME_TYPE				EQU 2
EA_NAME_TYPE_SIZE			EQU 3
EA_NAME_TYPE_SIZE_PROT			EQU 4
EA_NAME_TYPE_SIZE_PROT_DATE		EQU 5
EA_NAME_TYPE_SIZE_PROT_DATE_COMM	EQU 6
