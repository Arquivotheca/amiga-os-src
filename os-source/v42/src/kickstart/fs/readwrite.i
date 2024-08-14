; Whenever a file is opened one of these structures is created to keep track
; of and synchronise reads and writes to this file.  If the file is opened
; multiple times then extra rwdata structures are allocated and hung off the
; statelist as required.  The filehandle contains a pointer to the rwdata
; structure (in fh_Arg0) which can be used to get the pointer to rwheader.
; Each header has an associated co-routine that remains active as long as
; the file is open.  Multiple opens of the same file won't cause multiple
; co-routines to be started.  When a packet comes in it is queued on the
; PacketList.  If the co-routine is running then that's all that is needed
; because when the running packet is completed the next packet will be taken
; from the list.  If the co-routine is not running, then it's CallCo()'ed to
; kick it off on the current packet list.
 STRUCTURE rwheader,0		header for multiple opens of same file
	ULONG	rwh_Next	next header for another file
	ULONG	rwh_FileHeader	the file header key (copied from lock)
	ULONG	rwh_Volume	the disk volume this file belongs to
	ULONG	rwh_PacketList	list of packets that need processing
	ULONG	rwh_RunPacket	the packet currently being processed
	ULONG	rwh_StateList	list of rwdata structures
	ULONG	rwh_CoBase	the associated co-routine stack base
	ULONG	rwh_RecordLocks	list of record locks on this file
 LABEL rwheader_SIZEOF

 STRUCTURE rwdata,0		hung off fh_Arg0 to keep read/write state
	ULONG	rwd_Next	linked list for multiple opens of same file
	ULONG	rwd_Owner	back pointer to owning rwheader
	ULONG	rwd_FileHeader	the file header key (copied from lock)
	ULONG	rwd_Lock	a read/write private lock on this file
	ULONG	rwd_Header	the current extension block key
	ULONG	rwd_PrevHeader	the previous extension block key
	UWORD	rwd_Sequence	the ordinal number of the current extension
	UWORD	rwd_Entry	offset to current entry in extension block
	ULONG	rwd_Offset	the current seek position in bytes
	UWORD	rwd_Altered	TRUE means this file was written to
	UWORD	rwd_NewBlocks	TRUE means we are writing to new data blocks
	ULONG	rwd_LastData	last data key accessed (used for allocations)
	ULONG	rwd_StartKey	not cached, key to start direct reads from
	ULONG	rwd_CurrSize	size of file when a write started
	ULONG	rwd_InFile	flag, TRUE means rwd_Offset < EOF
	ULONG	rwd_DataSeq	current data block number (starts at 1)
	ULONG	rwd_SavedOffset	offset when a write was started
 LABEL rwdata_SIZEOF
