		INCLUDE	"exec/ports.i"
		INCLUDE	"devices/timer.i"
		INCLUDE	"libraries/dosextens.i"

; Minimum number of buffers allowed for a device
MINBUFFERS	EQU 3

; buffer types for quick determination of what queue a buffer is on
HASHFREE	EQU 0
HASHVALID	EQU 1
HASHPENDING	EQU 2
HASHWAITING	EQU 3

; stuff needed in bitmap routine
ERROR_KEYRANGE  EQU 7364
ERROR_NO_BITMAP EQU 8723
ERROR_CHECKSUM	EQU 9955

RAW    EQU -3		when writing, NEVER checksum this kind of block
BITMAP EQU -2		when writing, checksum goes in first longword
ROOT_BM_COUNT EQU 25	# bitmap pointers in the root block (dynamic in rest)

	STRUCTURE CacheBuff,MLN_SIZE
		APTR	cb_NextHash	next on a hash chain
		UWORD	cb_QueueType	what queue we are on
		UWORD	cb_State	state on the pending queue (read/write)
		ULONG	cb_Header	file header this block belongs to
		APTR	cb_CoPkt	owning co-routine or packet
		ULONG	cb_Size		size of data area or 0,-1,-2 or >0
		ULONG	cb_Key		what disk key this corresponds to
		ULONG	cb_Error	error that occured when reading
		ULONG	cb_Actual	amount that got read
		ULONG	cb_Suppress	if TRUE don`t post write error requesters
		APTR	cb_Buff		buf.buf, pointer to data area
;data area will be directly after this struct if we are doing 1 block at a time
	LABEL cb_SIZEOF

; new structure for use with examine and ExNext to make it much F-A-S-T-E-R !!
	STRUCTURE ExamineThing,0
		UBYTE	ET_Type			type of examinething == 0
		UBYTE	ET_Pad1
		UWORD	ET_Pad2
		APTR	ET_CurrentKey		pointer to next key to use
		LABEL	ET_Entries		start of list of keys (HTSIZE+1)
	LABEL ET_SIZEOF

	STRUCTURE NewExamineThing,0
		UBYTE	NET_Type		type of examinething == 1
		UBYTE	NET_Pad1
		UWORD	NET_NumEntries
		ULONG	NET_NextKey		next key to return
		ULONG	NET_NextBlockNum	DirList block it's in
		ULONG	NET_BlockNum		Current dirlist block number
		APTR	NET_BlockPtr		Current dirlist block ptr
		APTR	NET_Entry		Current entry ptr
	LABEL NET_SIZEOF

fl_Examined EQU fl_SIZEOF+4		used for exnext kluge

buf.valid       EQU -1     // buffer valid in freeblock
buf.invalid     EQU 0    // buffer invalid....

grab            EQU -1     // readblock grab before write....
wait            EQU 0    // .... options

altered.lock    EQU 0
exclusive.lock  EQU -1
shared.lock     EQU -2

;   // file types

t.deleted       EQU 1
t.short         EQU 2
t.long          EQU 4
t.data          EQU 8
t.list          EQU 16

DCFS_REV	EQU 1
DIRLIST_KEY	EQU 32
DIRLIST_MASK	EQU $ffffffe0
t.dirlist	EQU (DIRLIST_KEY!DCFS_REV)


st.link		EQU -4		doesn't go on disk like this
st.file         EQU -3
st.root         EQU 1
st.userdir      EQU 2

;   // Protection bits

prot.read       EQU 8
prot.write      EQU 4
prot.execute    EQU 2
prot.delete     EQU 1

discstate.writeprotected   EQU 80  // disc is write protected
discstate.notvalidated     EQU 81  // disc is not validated
discstate.validated        EQU 82  // disc is validated
   
type.dos	EQU ('D'<<24)!('O'<<16)!('S'<<8)
type.kickstart	EQU ('K'<<24)!('I'<<16)!('C'<<8)!'K'
type.ndos	EQU ('N'<<24)!('D'<<16)!('O'<<8)!'S'
type.unreadable EQU ('B'<<24)!('A'<<16)!('D'<<8) 
type.busy	EQU ('B'<<24)!('U'<<16)!('S'<<8)!'Y'
type.rawcon	EQU ('R'<<24)!('A'<<16)!('W'<<8)
type.con	EQU ('C'<<24)!('O'<<16)!('N'<<8)
type.nodisk	EQU -1

motor.timeout   EQU  3       // 3 second timeout
comm.upb        EQU 20       // Upper bound of comment field in lwords

	STRUCTURE co_routine,0
		APTR	co_link		link in chain (must stay here)
		APTR	co_sp		stack pointer for this co-routine
		APTR	co_parent	parent co-routine (who called)
		APTR	co_func		initial function when starting up
		STRUCT	co_dsem,dsem_SIZEOF used for directory locks
	LABEL co_SIZEOF

Abort.Impossible      EQU 299
Abort.ActionNotKnown  EQU 298
Abort.KeyOutOfRange   EQU 297
Abort.DiscError       EQU 296
Abort.InvalidCheckSum EQU 293
Abort.LoadSegFailure  EQU 292
Abort.KeyAlreadyAllocated    EQU 290
Abort.KeyAlreadyFree  EQU 289
Abort.Busy            EQU 288
Abort.BitMapCorrupt   EQU 287


EndStreamch         EQU     -1
NotInUse            EQU     -1
BytesPerWord        EQU     4
BitsPerWord         EQU     32
BitsPerByte         EQU     8
MaxInt              EQU     $7FFFFFFF
MinInt              EQU     $80000000
TicksPerSecond      EQU     50

	STRUCTURE RecordLock,0
		STRUCT	rl_Timer,IOTV_SIZE	timer request for timeouts
		STRUCT	rl_FakePacket,dp_Arg2	dos packet for returning reqs
		APTR	rl_StartPacket		the initial lockrecord request
		ULONG	rl_ESecs		give up at or after this time
		ULONG	rl_EMicros
		ULONG	rl_Timeout		requested timeout period
		ULONG	rl_LowBound		first byte locked
		ULONG	rl_HighBound		last byte locked
		ULONG	rl_LockType		exclusive or shared record lock
		APTR	rl_Blocking		who we are blocking right now
		APTR	rl_BlockingMe		who is blocking us right now
		APTR	rl_rwddata		open we're associated with
	LABEL rl_SIZEOF

RL_EXCLUSIVE	EQU 0	an exclusive (write) record lock
RL_SHARED	EQU 1	a shared (read) record lock

RL_WRITE		EQU 0
RL_WRITE_IMMEDIATE	EQU 1
RL_READ			EQU 2
RL_READ_IMMEDIATE	EQU 3


; This is a notify entry that is added to the list for each NotifyRequest
; the list itself is hung off the appropriate volume node.  If any notify
; entries are still there when the disk is removed, the volume will not be
; zapped.  When a volume is re-activated, it is nescessary to make sure that
; the nr_Handler fields are pointing to the right handler (if a disk is
; inserted into a different drive).
	STRUCTURE NotifyEntry,0
		APTR	ne_Next		next entry on the list
		APTR	ne_NotifyReq	the actual notify request
		ULONG	ne_Orphaned	TRUE means file/dir doesn`t exist yet
		ULONG	ne_HeaderKey	which key we are watching for changes
		APTR	ne_Msg		an oustanding NotifyMessage
		STRUCT	ne_FullName,256	BSTR version of nr_FullName
	LABEL ne_SIZEOF


; softlinks actually take very little code in the FS.
SOFTLINKS_SUPPORTED	EQU	1
