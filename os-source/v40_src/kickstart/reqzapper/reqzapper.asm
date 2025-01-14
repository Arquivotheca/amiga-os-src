        OPTIMON

;---------------------------------------------------------------------------

; This code is a small module which gets called late in the boot process
; after DOS has been started. It goes through the list of all processes in the
; system and disables system requesters for them by poking pr_WindowPtr to
; -1. This prevents DOS requesters from appearing in the boot process of a
; game system.
;
; In addition, this code also installs a patch to AddTask() which simply clears
; out the TC_TRAPCODE field of the task structure whenever a process is
; created. This has the effect of disabling the DOS trap code which is
; installed in every process created by DOS. The DOS trap code is the one
; responsible for bringing up DOS software failure requesters. Removing the
; trap code prevents these requesters from coming up, and instead jumps
; directly to the system alert code. It just so happens that on a game
; system, the alert code is disabled and instead causes an instant reboot.

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        INCLUDE "exec/memory.i"
        INCLUDE "dos/dosextens.i"

        INCLUDE "reqzapper_rev.i"

        LIST

;---------------------------------------------------------------------------

	XREF	_LVODisable
	XREF	_LVOEnable
	XREF	_LVOSetFunction
	XREF	_LVOAllocMem
	XREF	_LVOCopyMem
	XREF	_LVOAddTask

;---------------------------------------------------------------------------

REQZAPPERTAG:
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    REQZAPPERTAG               ; APTR  RT_MATCHTAG
        DC.L    ENDCODE                    ; APTR  RT_ENDSKIP
        DC.B    RTF_AFTERDOS		   ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_UNKNOWN                 ; UBYTE RT_TYPE
        DC.B    -50                        ; BYTE  RT_PRI
        DC.L    ReqZapperName              ; APTR  RT_NAME
        DC.L    ReqZapperID                ; APTR  RT_IDSTRING
        DC.L    ReqZapperCode	           ; APTR  RT_INIT

ReqZapperName DC.B 'reqzapper',0
ReqZapperID   VSTRING

	CNOP	0,2

;-----------------------------------------------------------------------

AddTaskPatch:
	cmp.b	#NT_PROCESS,LN_TYPE(a1)	 ; is a process being added?
	beq.s	1$			 ; if not, then ignore...
	clr.l	TC_TRAPCODE(a1)		 ; if so, then clear the trap code
1$	move.l	oldAddTask(pc),a0	 ; get the original vector
	jmp	(a0)			 ; go do the real AddTask()

oldAddTask:
	DC.L	0

AddTaskPatchEnd

patchSize equ (AddTaskPatchEnd-AddTaskPatch)

;-----------------------------------------------------------------------

ReqZapperCode:
	jsr	_LVODisable(a6)		 ; prevent the task lists from changing

DoReadyTasks:
	move.l	TaskReady+LH_HEAD(a6),a0 ; first node
1$:	move.l	LN_SUCC(a0),d0		 ; was this the last node?
	beq.s	DoWaitingTasks		 ; if so, do next list
	cmp.b	#NT_PROCESS,LN_TYPE(a0)  ; do we have a process?
	bne.s	2$			 ; if not, do next node
	move.l	#-1,pr_WindowPtr(a0)     ; kill requesters for this process
	move.l	TaskTrapCode(a6),TC_TRAPCODE(a0)
2$:	move.l	d0,a0			 ; load next node
	bra.s	1$			 ; go process that node...

DoWaitingTasks:
	move.l	TaskWait+LH_HEAD(a6),a0  ; first node
1$:	move.l	LN_SUCC(a0),d0		 ; was this the last node?
	beq.s	DoThisTask		 ; if so, do the current task
	cmp.b	#NT_PROCESS,LN_TYPE(a0)	 ; do we have a process?
	bne.s	2$			 ; if not, do next node
	move.l	#-1,pr_WindowPtr(a0)	 ; kill requesters for this process
	move.l	TaskTrapCode(a6),TC_TRAPCODE(a0)
2$:	move.l	d0,a0			 ; load next node
	bra.s	1$			 ; go process that node

DoThisTask:
	move.l	ThisTask(a6),a0		 ; hey, that's us!
	move.l	#-1,pr_WindowPtr(a0)	 ; no more requesters for us either
	move.l	TaskTrapCode(a6),TC_TRAPCODE(a0)

	move.l	#patchSize,d0		 ; size of AddTask patch code
	move.l	#MEMF_ANY,d1		 ; where to stick the patch
	jsr	_LVOAllocMem(a6)	 ; allocate space for the patch
	move.l	d0,-(sp)		 ; save the new patch pointer
	beq.s	Exit			 ; no memory, so leave...

	lea	AddTaskPatch(pc),a0 	 ; the patch code
	move.l	d0,a1			 ; the allocate patch memory
	move.l	#patchSize,d0		 ; the size of the patch
	jsr	_LVOCopyMem(a6)		 ; copy the code over

	move.l	a6,a1			 ; library to patch, execbase
	move.l	#_LVOAddTask,a0		 ; vector to replace
	move.l	(sp),d0			 ; patch code
	jsr	_LVOSetFunction(a6)	 ; replace the code

	move.l	(sp),a0			 ; re-obtain patch pointer
	move.l	d0,(oldAddTask-AddTaskPatch)(a0)  ; store pointer to old AddTask()

Exit:	addq	#4,sp			 ; clean up stack frame

	jmp	_LVOEnable(a6)		 ; allow multitasking, and quit

;-----------------------------------------------------------------------

ENDCODE:

;-----------------------------------------------------------------------

        END
