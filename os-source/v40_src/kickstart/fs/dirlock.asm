		SECTION	filesystem,CODE

		NOLIST
		INCLUDE	"exec/types.i"
		INCLUDE	"exec/io.i"
		INCLUDE	"exec/lists.i"

		INCLUDE	"globals.i"
		INCLUDE	"private.i"
		INCLUDE	"printf.mac"
		LIST

;		DEBUGENABLE

		XREF	WaitCo,ResumeCo,CallCo
		XDEF	ObtainDir,ReleaseDir,ObtainRename,ReleaseRename

;==============================================================================
; ObtainDir (dir)
; 	     d0
;
; Waits for a directory to be available for modification.  Used to implement
; locking around file/dir creation and directory list updating, since we
; can have coroutine switches during those operations.  If someone else
; owns it, we'll queue behind them to get access.  When woken up, we own the
; directory.  We do our critical operations, then when done we call
; ReleaseDir, which swaps the next coroutine's dsem into the DirSemList,
; and CallCo's the next waiter.  This has the unexpected effect that they do
; their critical sections in order, but the following sections in reverse
; order (up to the next WaitCo).
; 
; This is wierd (kind of like QueueBehind in bufalloc.asm), but it works.
;
; The dsem structure is normally part of the coroutine structure, so we're
; guaranteed to have one for each coroutine.  Rename, however, requires two
; locks (source and destination).  This could lead to deadlock problems.  The
; best solution I can see is to remove it (with locking) from the source, then
; add it (with locking) to the destination.  This is effectively what happens
; now.  If the system dies inbetween, as now, the file will not be linked
; anywhere.  We'll do an "file already exists" check before unlinking, but
; the file could show up while we're unlinking.  We can try to relink it to
; the source, but that may fail also.  Ick.  I suppose we could modify the
; name algorithmically until we manage to add it back.  We could also put it
; somewhere similar to "lost+found" in Unix.
;
; You'd think that ordering of the locks would avoid the deadlock, but you
; don't know the directory block numbers until you find them, and finding the
; second could allow the first to be deleted, renamed, etc.
;
; We could also add a master "I'm renaming something" lock, and allow only
; one rename at a time.  That would remove any chance for deadlock, since only
; rename needs two locks.  In that case, we'll have an extra dsem for rename
; that's shared by all coroutines.  We can either use an in-use flag for the
; per-coroutine dsem, or we can use a flag passed in.  This sounds like the
; best solution.
;
; Where ObtainDir should be used:
;
; ObtainDir should be done anywhere modifications are being made to a
; directory or the directory hash chains, and there's a possibility of
; a WaitCo (such as when calling GrabBlock/WaitBlock).  This would include
; Create, Rename, and Scratch primarily.  Directory cache updates (UpdateList)
; and comment changes (DeleteList/InsertList) do not need this.  The exnext/all
; code can handle the changes, though which state it sees is indeterminate.
; Even SetComment will guarantee that the file is seen, though not which
; state (this is because of the way QueueBehind works - since setcomment
; writes the old block, and waits on the next block before anyone else
; can, it will get the next block first and can do its modifications (or
; the block after that, etc)).  One should also be careful of hardlinks on
; delete!
;
; (BTW, AllocKeys can't WaitCo, it uses special routines to read).
;
; In order to be certain that directories are not left locked because of
; a failure (read error, write-protected, etc), WorkDone/WorkFail will
; clean off any ObtainDir locks owned by the current coroutine.
; Normally the list should be empty or at most have one entry in it, so
; this shouldn't be a performance issue.  We use dsem_Block as a flag to
; whether it's in use or not.
;==============================================================================
ObtainDir   ; printf <'ObtainDir of %ld'>,d0
		move.l	CurrentCo(a5),a0
		lea	co_dsem(a0),a0
		tst.l	dsem_Block(a0)		are we already using this?
		beq.s	5$
	IFD DEBUG_CODE
	move.l	CurrentCo(a5),d1
	cmp.l	RenameCo(a5),d1
	beq.s	1$
	printf <'ERROR! ObtainDir of %ld when %ld already held!'>,d0,dsem_Block(a0)
1$
	ENDC
		lea	Rename_dsem(a5),a0	yes, must be rename
		move.l	CurrentCo(a5),dsem_OwnCo(a0) since ownership changes

5$		clr.l	dsem_NextCo(a0)		we'll be the end of the chain
		move.l	d0,dsem_Block(a0)	remember which directory
		lea	DirSemList(a5),a1	directory co-semaphore list
		move.l	MLH_HEAD(a1),a1

10$		move.l	MLN_SUCC(a1),d1
		beq.s	20$

		cmp.l	dsem_Block(a1),d0	is this block we care about?
		beq.s	found_dir
		move.l	d1,a1			no, loop
		bra.s	10$

20$		; not on the list - add it
		move.l	a0,a1			node to add
		lea	DirSemList(a5),a0	list to add it to
		ADDHEAD
	; printf <'  dir not owned'>
		rts				we own it now, do what we want
		
found_dir:	; we found the dsem (in a1, ours is in a0)
		; wait on it (add us to the end of the chain)
	printf <'  dir owned by $%lx, waiting'>,dsem_OwnCo(a1)
10$		move.l	dsem_NextCo(a1),d0	find last waiter
		beq.s	20$
		move.l	d0,a1
		bra.s	10$

20$		move.l	a0,dsem_NextCo(a1)
		bra	WaitCo			wait until we own it
		; when we get woken up, it will go to the caller
		; he owns the directory, and when he's done he calls
		; ReleaseDir.

;==============================================================================
; ReleaseDir (dsem)
; 	       a0
;
; We're done with a directory critical section, see if anyone else was
; waiting for it, and if so CallCo them. (see ObtainDir)  Before we CallCo
; them, swap their dsem entry onto the DirSemList with ours.  The last
; releaser (NextCo == NULL) removes his entry from the list.
;
; Note: if there were waiters, we don't return until they all have WaitCo'd.
; See ObtainDir.
;
; dsem is either &(CurrentCo->co_dsem), or &Rename_dsem (see ObtainDir)
;==============================================================================
ReleaseDir: ;printf <'$%lx releasing dirlock on %ld'>,dsem_OwnCo(a0),dsem_Block(a0)
		clr.l	dsem_Block(a0)		indicate not in use
		move.l	dsem_NextCo(a0),d0
		beq.s	release_done		no one else waiting
		move.l	MLN_SUCC(a0),a1		replace us with him on the
		move.l	d0,MLN_PRED(a1)		DirSemList
		move.l	MLN_PRED(a0),a1
		move.l	d0,MLN_SUCC(a1)
		move.l	d0,a1
		move.l	MLN_SUCC(a0),MLN_SUCC(a1)
		move.l	MLN_PRED(a0),MLN_PRED(a1)
		move.l	dsem_OwnCo(a1),a0	which coroutine owns waiter
		bra	CallCo			returns to caller of ReleaseDir

release_done:	; we're the only one using this directory (now)
		move.l	a0,a1
		REMOVE				remove it from DirSemList
		rts

;==============================================================================
; ObtainRename ()
;
; Make sure only one coroutine is renaming files at a time, so we can avoid
; any deadlocks due to needing two directory locks.  (See above.)
;==============================================================================
ObtainRename:	printf <'$%lx obtaining rename'>,CurrentCo(a5)
		move.l	RenameCo(a5),d0
		beq	rename_gotlock

		; someone is already renaming.  wait until they're done
		; (add us to the end of the chain)
		move.l	CurrentCo(a5),-(sp)	push coroutine addr
		pea	0			push null pointer on stack
		lea.l	RenameWaitCo(a5),a0	find last waiter
		printf <'  rename waiting for $%lx'>,RenameCo(a5)
20$		move.l	(a0),d0			get pointer to next waiter
		beq.s	30$
		move.l	d0,a0
		bra.s	20$

30$		move.l	sp,(a0)			add our stack entry into list
		printf <'$%lx asleep (sp = $%lx)'>,CurrentCo(a5),(a0)
		bsr	WaitCo			wait until we own it
		printf <'$%lx awake'>,CurrentCo(a5)
		move.l	(sp)+,RenameWaitCo(a5)	make next waiter head of list
		addq.w	#4,sp			drop CurrentCo off stack

		; we are the RenameCo now!
		printf <'  $%lx now owns rename'>,CurrentCo(a5)
rename_gotlock:	move.l	CurrentCo(a5),RenameCo(a5)
		rts

;==============================================================================
; ReleaseRename ()
;
; We're done with a rename, so wake up any other coroutines waiting to
; rename something.  The Rename_dsem must have already been freed!
;==============================================================================
ReleaseRename	printf <'$%lx releasing rename'>,CurrentCo(a5)
		clr.l	RenameCo(a5)		assume no waiters
		move.l	RenameWaitCo(a5),d0
		beq.s	10$			all done
		move.l	d0,a0			pointer to link to next
		move.l	4(a0),a0		coroutine addr above link
		printf <'waking up $%lx'>,a0
		bsr	CallCo			returns to our caller
10$		rts

	END
