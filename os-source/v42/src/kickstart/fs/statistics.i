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

;STATISTICS EQU 1	include this file to turn on statistics
