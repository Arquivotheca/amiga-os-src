*******************************************************************************
*
* $Id: name.asm,v 39.4 93/08/11 16:10:50 vertex Exp $
*
* $Log:	name.asm,v $
* Revision 39.4  93/08/11  16:10:50  vertex
* Fixed bad type in autodoc
* 
* Revision 39.3  93/02/10  15:05:48  vertex
* Cleaned up docs
*
* Revision 39.2  92/04/09  14:58:29  mks
* Updated the autodocs...
*
* Revision 39.1  92/04/08  23:19:29  mks
* New NameSpace stuff added to utility
*
*******************************************************************************


	include	"exec/types.i"
	include "exec/lists.i"
	include "exec/memory.i"
	include "exec/ables.i"
	include	"exec/macros.i"
	include	"exec/semaphores.i"

	include "utility/tagitem.i"

	include	"utilitybase.i"
	include	"name.i"

CALLSYS		MACRO
		IFND	_LVO\1
		xref	_LVO\1		; Set the external reference
		ENDC
		jsr	_LVO\1(a6)
		ENDM

**********
* Namespace structures...  (private, very private)
**********

 STRUCTURE NamedObj,0
	STRUCT no_Node,LN_SIZE
	UWORD  no_UseCount	; Long word align the rest...
	APTR   no_object	; This is at 16...
	APTR   no_NameSpace
	APTR   no_RemoveMsg
	LABEL NamedObject_SIZEOF

*
* We use this structure when we don't need the node but
* need the fields...  (sick structure tricks)
*
 STRUCTURE NamedObj_Sub,-no_object
	STRUCT	nos_Node,LN_SIZE	; -16
	UWORD	nos_UseCount		; -2
	APTR	nos_Object		; 0...
	APTR	nos_NameSpace
	APTR	nos_RemoveMsg

*
* The real memory use is 16-bytes more to match starting address with
* nodes...  (sick tricks for Alloc/Free...)
*
 STRUCTURE NameSpace,0
	STRUCT ns_Entries,MLH_SIZE
	STRUCT ns_Semaphore,SS_SIZE
	ULONG  ns_Flags
	LABEL NameSpace_SIZEOF


******* utility.library/NamedObjectName ***************************************
*
*   NAME
*	NamedObjectName -- return the name of the object. (V39)
*
*   SYNOPSIS
*	name = NamedObjectName(object);
*	D0                     A0
*
*	STRPTR NamedObjectName(struct NamedObject *);
*
*   FUNCTION
*	Returns the name of the object passed in...
*	Note that the name string is passed back as just a pointer to
*	a read-only name. If the object goes away, so does the name.
*
*   INPUTS
*	object - the object, may be NULL in which case this function
*		 returns NULL.
*
*   RESULT
*	name - pointer to the name string, or NULL if 'object' is NULL.
*
*   SEE ALSO
*	FindNamedObject(), RemNamedObject()
*
*******************************************************************************
NamedObjectName:	xdef	NamedObjectName
		move.l	a0,d0		; Check for NULL object
		beq.s	non_Exit	; Exit if no object
		move.l	nos_Node+LN_NAME(a0),d0	; Get name pointer
non_Exit:	rts

******* utility.library/FindNamedObject ***************************************
*
*   NAME
*	FindNamedObject -- find the next object of a given name. (V39)
*
*   SYNOPSIS
*	object = FindNamedObject(nameSpace, name, lastObject);
*	D0                       A0         A1    A2
*
*	struct NamedObject *FindNamedObject(struct NamedObject *, STRPTR,
*					    struct NamedObject *);
*
*   FUNCTION
*	Finds an object and adds to the open count of the object. The
*	object is guaranteed not to be freed until ReleaseNamedObject() is
*	called. The name comparison is caseless, using the current
*	locale string comparison routines.
*
*	If name is NULL, then all objects will be matched.
*
*	If lastObject is non-NULL, it must be an object from the same
*	NameSpace found on a previous call to FindNamedObject(). It
*	will not be freed by this call. The search will start at the
*	node after lastobject, if non-NULL.
*
*	nameSpace is the name space from the named object given
*	or the root name space if NULL is given.
*
*   INPUTS
*	nameSpace - the name space to search
*	name - the name of the object to search for
*	lastObject - the starting point for the search or NULL
*
*   RESULT
*	object - the first match found, or NULL for no match
*
*   SEE ALSO
*	ReleaseNamedObject(), <utility/name.h>
*
*******************************************************************************
FindNamedObject:	xdef	FindNamedObject
		movem.l	d2/a2-a5,-(sp)
		move.l	ub_SysBase(a6),a5	; Get execbase into a5...
		bsr.s	getNameSpace		; Get namespace
		exg	a5,a6			; a5/a6 are swapped...
		beq.s	fno_Exit		; We have no name space, exit
*
		move.l	d0,a3			; Get namespace...
		move.l	a1,a4			; name
*
		; so no one modifies the namespace while we're scanning
		lea	ns_Semaphore(a3),a0
		CALLSYS	ObtainSemaphoreShared	; Obtain the semaphore...
*
		; find object to start search at
		move.l	a2,d0			; if lastobject
		beq.s	10$
		move.l	nos_Node+LN_SUCC(a2),a2
		bra.s	20$
10$		move.l	ns_Entries+LH_HEAD(a3),a2
20$
		; At this point a2 is a pointer to the real name object stuff
		; (That is, it no longer is the black-box nos object)
		; search the namespace (if needed) for an object
		bsr.s	search_ns		; a2/a3/a4/a6, returns a2!
		move.l	a2,d0			; object or NULL
		beq.s	release_sem

		; OK! we have a node to return
		; no race condition because this is 1 instruction!!!!
		; if this were more, a FORBID would be needed
		addq.w	#1,no_UseCount(a2)
		lea	no_object(a2),a2	; Move down to the object

release_sem:	lea	ns_Semaphore(a3),a0
		CALLSYS	ReleaseSemaphore
		move.l	a2,d0			; return object or NULL
*
fno_Exit:	exg	a5,a6			; Get utilitybase back
		movem.l	(sp)+,d2/a2-a5		; Restore...
		rts


;==============================================================================
; object = search_ns (initial_object, namespace, name, utilitybase, execbase)
;   a2		          a2             a3       a4      a5           a6
;
; Search a namespace for a name match.  return the node or NULL in a2!!!
; The search starts at initial_object.  If name is NULL returns the next
; object if any or NULL.  (trashes d2!!!)
;==============================================================================
search_ns:	move.l	a4,d0
		beq.s	match			; no name, match anything
*
*
		btst.b	#NSB_CASE,ns_Flags+3(a3)
		beq.s	object_loop		; Check if not case sensitive
*
* Simple findname...
*
		move.l	a4,a1
		move.l	no_Node+LN_PRED(a2),a0	; Get one before me...
		CALLSYS	FindName		; Find the next node...
		move.l	d0,a2			; Here it is...
		rts
*
* FindNameNoCase
*
no_match:	move.l	d2,a2			; Get next node...
object_loop:	move.l	no_Node+LN_SUCC(a2),d2	; save next node for later
		beq.s	not_found		; failed, exit

no_case:	; compare without case
		move.l	no_Node+LN_NAME(a2),a0
		move.l	a4,a1			; String to match...
		exg	a5,a6			; Get utilitybase...
		CALLSYS	Stricmp
		exg	a5,a6			; Restore execbase...
		tst.l	d0
		bne.s	no_match		; not the same, loop
match:		; the names match or no name was specified
		tst.l	no_Node+LN_SUCC(a2)	; is this a real node?
		bne.s	found			; yes, we found it.  return
*
not_found:	; Failure - return NULL
		sub.l	a2,a2
*
found:		rts				; a2 is return code!
*
*******************************************************************************
*
* Internal use:  Pass an object in a0, returns the namespace in d0
* If a0 is NULL, returns a root name space...
* Flags set on exit...
* a6 *must* be utilitybase
*
getNameSpace:	move.l	a0,d0		; Check for object...
		bne.s	gns_LiveOne	; We have a live one (non-NULL)
		move.l	ub_MasterSpace(a6),a0	; Root space
gns_LiveOne:	move.l	nos_NameSpace(a0),d0
		rts			; Return with namespace in d0
*
******* utility.library/AddNamedObject ****************************************
*
*   NAME
*	AddNamedObject -- add a named object to the given namespace. (V39)
*
*   SYNOPSIS
*	success = AddNamedObject(nameSpace, object);
*	D0			 A0         A1
*
*	BOOL AddNamedObject(struct NamedObject *, struct NamedObject *);
*
*   FUNCTION
*	Adds a new item to a NameSpace.  If the NameSpace doesn't support
*	duplicate names, a search for a duplicate will be made, and
*	0 (failure) will be returned.  Otherwise, the entry will be
*	Enqueue()ed to the NameSpace.
*
*   INPUTS
*	nameSpace - the name space to add to (NULL for root namespace)
*	object - the object to add  (If NULL, will return failure)
*
*   RESULT
*	success - whether the operation succeeded.  Check this always!
*
*   SEE ALSO
*	AttemptRemNamedObject(), RemNamedObject()
*
*******************************************************************************
AddNamedObject:	xdef	AddNamedObject
*
		movem.l	d2-d4/a2-a5,-(sp)
		bsr.s	getNameSpace		; Get the namespace
		beq.s	ano_Exit		; No namespace?
		move.l	d0,a3			; Get namespace...
		move.l	a1,d0			; Check object...
		beq.s	ano_Exit		; False if NULL object...
		lea	nos_Node(a1),a2		; object to add
		move.l	ub_SysBase(a6),a5	; Get execbase...
		exg	a5,a6
*
		lea	ns_Semaphore(a3),a0
		CALLSYS	ObtainSemaphore		; Get the semaphore...
*
		; do we need to check for dups?
		btst.b	#NSB_NODUPS,ns_Flags+3(a3)
		beq.s	enqueue_it

		; search for dups
		move.l	no_Node+LN_NAME(a2),a4	; name of object we're adding
		move.l	a2,d3			; save object ptr
		move.l	ns_Entries+LH_HEAD(a3),a2 ; first entry to search
		bsr.s	search_ns		; a2/a3/a4/a5, returns a2!!!!
		moveq	#0,d4			; default result = FALSE
		move.l	a2,d0			; Check failure...
		bne.s	cleanup			; cleanup & exit on failure
		move.l	d3,a2			; get object back
*
enqueue_it:	; enqueue the object into the namespace
		lea	ns_Entries(a3),a0	; Get entries list...
		move.l	a2,a1
		CALLSYS	Enqueue			; Enqueue onto list...
		move.l	a3,no_NameSpace(a2)	; remember which namespace
		moveq.l	#-1,d4			; success

cleanup:	lea	ns_Semaphore(a3),a0
		CALLSYS	ReleaseSemaphore	; Release our lock...
		move.l	d4,d0			; result
		exg	a5,a6			; Get utility base back...
ano_Exit:	movem.l	(sp)+,d2-d4/a2-a5
		rts


******* utility.library/AttemptRemNamedObject *********************************
*
*   NAME
*	AttemptRemNamedObject -- attempt to remove a named object. (V39)
*
*   SYNOPSIS
*	result = AttemptRemNamedObject(object);
*	D0                             A0
*
*	LONG AttemptRemNamedObject(struct NamedObject *);
*
*   FUNCTION
*	Attempts to remove an object from whatever NameSpace it's in.
*	You must have found the object first (in order to get a use count)
*	before trying to remove it.  If the object is in use or is
*	in the process of being removed, this function will return
*	a failure code.  If the object is fully removed, the object will
*	then be available to be FreeNamedObject().
*
*   INPUTS
*	object - the object to attempt to remove  The object must be valid
*
*   RESULT
*	success - FALSE if object is still in use (somewhere)
*	          TRUE if object was removed
*
*   SEE ALSO
*	RemNamedObject(), AddNamedObject(), ReleaseNamedObject()
*
*******************************************************************************
AttemptRemNamedObject:	xdef	AttemptRemNamedObject
			sub.l	a1,a1		; no message
			; fall through...

******* utility.library/RemNamedObject ****************************************
*
*   NAME
*	RemNamedObject -- remove a named object. (V39)
*
*   SYNOPSIS
*	RemNamedObject(object, message);
*	               A0      A1
*
*	VOID RemNamedObject(struct NamedObject *, struct Message *);
*
*   FUNCTION
*	This function will post a request to release the object
*	from whatever NameSpace it is in.  It will reply the message
*	when the object is fully removed.  The message.mn_Node.ln_Name
*	field will contain the object pointer or NULL if the object
*	was removed by another process.
*
*	This function will effectively do a ReleaseNamedObject()
*	thus you must have "found" the object first.
*
*   INPUTS
*	object - the object to remove: Must be a valid NamedObject.
*	message - message to ReplyMsg() (must be supplied)
*
*   RESULT
*	The message is replied with the ln_Name field either being
*	the object or NULL. If it contains the object, the object
*	is completely removed.
*
*   SEE ALSO
*	AttemptRemNamedObject(), AddNamedObject(), ReleaseNamedObject()
*
*******************************************************************************
RemNamedObject:	xdef	RemNamedObject
		movem.l	d2/a2-a4/a6,-(sp)	; Save
		move.l	ub_SysBase(a6),a6	; Get ExecBase
		FORBID				; Protect this...
		move.l	a1,a4			; Save message
		move.l	a0,a2			; Get node...
		move.l	nos_NameSpace(a2),d0	; Get NameSpace
		beq.s	rno_NameGone		; If null, NameSpace is gone...
		move.l	d0,a3			; Save namespace in a3
		move.l	a4,d0			; Check for message
		bne.s	rno_DoRemove		; Do the remove if message...
		cmp.w	#1,nos_UseCount(a2)	; Check if last user
		beq.s	rno_DoRemove		; If last one, we remove...
rno_NameGone:	move.l	a4,d0			; Check if message
		beq.s	rno_FailOut		; No message, just exit...
		clr.l	LN_NAME(a4)		; Clear LN_NAME
		move.l	a4,a1			; Get message into right reg
		CALLSYS	ReplyMsg		; Return it with NULL LN_NAME
rno_FailOut:	; Note that d0 is 0 if there is no message  ;^)
		CALLSYS	Permit			; Permit
		movem.l	(sp)+,d2/a2-a4/a6	; restore
		rts				; Return failure!
*
rno_DoRemove:	clr.l	nos_NameSpace(a2)	; Clear namespace backpointer
		lea	ns_Semaphore(a3),a0	; Get the semaphore
		CALLSYS	ObtainSemaphore		; (lock the namespace)
		lea	nos_Node(a2),a1		; Get node...
		CALLSYS	Remove			; Remove it...
		move.l	a4,nos_RemoveMsg(a2)	; Set and check message...
		beq.s	rno_NoMessage		; If none, we are almost done
		move.l	a2,LN_NAME(a4)		; Set LN_NAME of message
rno_NoMessage:	lea	ns_Semaphore(a3),a0	; Get semaphore
		CALLSYS	ReleaseSemaphore	; Let it go...
		CALLSYS	Permit			; System free...
		move.l	a2,a0			; Get object into a0
		movem.l	(sp)+,d2/a2-a4/a6	; Restore registers...
		; Drop into releasenamedobject...  (below)

******* utility.library/ReleaseNamedObject ************************************
*
*   NAME
*	ReleaseNamedObject -- free a named object. (V39)
*
*   SYNOPSIS
*	ReleaseNamedObject(object);
*			   A0
*
*	VOID ReleaseNamedObject(struct NamedObject *);
*
*   FUNCTION
*	Decrements the open count of the object. If the object has been
*	removed, and the count goes to 0, the remover will be notified
*	that the object is now free.
*
*   INPUTS
*	object - the object to release.  (No action if NULL)
*
*   SEE ALSO
*	FindNamedObject(), RemNamedObject()
*
*******************************************************************************
ReleaseNamedObject:	xdef	ReleaseNamedObject
		move.l	a6,-(sp)		; Save on stack
		move.l	ub_SysBase(a6),a6	; Get ExecBase
		FORBID
		move.l	a0,d0			; Check for silly case
		beq.s	rno_Exit		; (Silly... NULL)
*
		subq.w	#1,nos_UseCount(a0)
		bne.s	rno_Exit		; If not 0, exit...
*
		move.l	nos_RemoveMsg(a0),d0
		beq.s	rno_Exit		; Check for message
		move.l	d0,a1			; Ready for ReplyMsg
		CALLSYS	ReplyMsg		; Do it!
*
rno_Exit:	CALLSYS	Permit
		move.l	(sp)+,a6		; Restore utilitybase
		moveq.l	#1,d0			; Return TRUE for Attempt...
any_rts:	rts				; exit...

******* utility.library/AllocNamedObjectA *************************************
*
*   NAME
*	AllocNamedObjectA -- allocate a named object. (V39)
*
*   SYNOPSIS
*	object = AllocNamedObjectA(name, tagList);
*	D0                         A0    A1
*
*	struct NamedObject *AllocNamedObjectA(STRPTR, struct TagItem *);
*
*	object = AllocNamedObject(name, Tag1, ...);
*
*	struct NamedObject *AllocNamedObject(STRPTR, ULONG, ...);
*
*   FUNCTION
*	Allocates a NamedObject and initializes it as needed to the
*	name given. This object can then be used as an object in the
*	namespaces. Tags can be given to make an object contain a
*	namespace such that nested namespaces can be built.
*	When the object is allocated, it automatically has one use.
*	If you later wish to release this object such that others may
*	remove it from the namespace you must do a ReleaseNamedObject().
*
*   INPUTS
*	name - name for the object (must not be NULL)
*	tagList - tags with additional information for the allocation or NULL
*
*   TAGS
*	ANO_NameSpace - BOOL tag, default FALSE.  If this tag is
*	                TRUE, the named object will also have a
*	                name space attached to it.
*	ANO_UserSpace - ULONG tag, default 0.  If this tag is non-NULL
*	                it defines the size (in bytes) of the user
*	                space to be allocated with the named object
*	                and that will be pointed to by the no_Object
*	                pointer.  This memory is long-word aligned.
*	                If no space is defined, no_Object will be NULL.
*	ANO_Priority  - BYTE tag, default 0.  This tag lets you pick
*	                a priority for the named object for when it is
*	                placed into a name space.
*	ANO_Flags     - ULONG tag, default 0.  This tag lets you set
*	                the flags of the NameSpace (if you allocated
*	                one)  There currently are only TWO flags.
*	                Do *NOT* set *any* other bits as they are for
*	                future use!!!  (You can't read them anyway)
*	                The flags are:
*	                NSF_NODUPS	- Name space must be unique.
*	                NSF_CASE	- Name space is case sensitive
*
*   RESULT
*	object - the object allocated, or NULL for failure. The object
*	         is defined as a pointer to a pointer.  You can do what you
*	         wish with the pointer. (It may be NULL or contain a pointer
*	         to memory that you had asked for in the tags.)
*
*   SEE ALSO
*	FreeNamedObject(), <utility/name.h>
*
*******************************************************************************
AllocNamedObjectA:	xdef	AllocNamedObjectA
*
		move.l	a0,d0			; Check for NULL name
		beq.s	any_rts			; exit it NULL...
*
alloc_Go:	movem.l	d2-d3/a2-a3/a5,-(sp)	; Save as needed
		move.l	ub_SysBase(a6),a5	; Get execbase
		move.l	a0,a2			; Get string pointer...
		move.l	a1,a3			; Get tags...
*
		move.l	#ANO_NameSpace,d0	; Check for namespace
		moveq.l	#0,d1			; Default value
		move.l	a3,a0			; Taglist
		CALLSYS	GetTagData		; Get the data
		move.l	d0,d3			; Check if real...
		beq.s	alloc_nns		; If FALSE, skip...
		move.l	#NameSpace_SIZEOF,d3	; Get namespace size
*
alloc_nns:	move.l	#ANO_UserSpace,d0	; Check for user space
		moveq.l	#0,d1			; default none...
		move.l	a3,a0			; Tag list
		CALLSYS	GetTagData		; Get the data...
		move.l	d0,d2			; save it...
		beq.s	alloc_nud		; If none, we skip...
		addq.l	#3,d2			; Bump to long align it...
*
alloc_nud:	move.l	#NamedObject_SIZEOF,d0	; Root object size
		move.l	a2,a0			; Get string again...
alloc_Count:	addq.l	#1,d0			; Count one...
		tst.b	(a0)+			; Check next byte...
		bne.s	alloc_Count		; Count it (+1 for NULL)
;
		add.l	d3,d0			; Add in the namespace
		add.l	d2,d0			; Add in the user data
		move.l	#MEMF_PUBLIC|MEMF_CLEAR,d1
		exg	a6,a5			; Execbase now...
		CALLSYS	AllocVec		; Get the memory
		exg	a6,a5			; swap back...
		tst.l	d0			; Check for memory
		beq.s	alloc_exit		; Exit if no memory...
*
* Check for namespace...
*
		move.l	d0,a0			; Get pointer...
		moveq.l	#NamedObject_SIZEOF,d0	; Get size of root object
		add.l	a0,d0			; Point past root object
		tst.l	d3			; Check for name space
		beq.s	alloc_no_ns		; Skip if no name space
		move.l	d0,no_NameSpace(a0)	; Store it...
		add.l	d3,d0			; Get address of namespace
*
* Install and copy the name string
*
alloc_no_ns:	move.l	d0,no_Node+LN_NAME(a0)	; Store name...
		move.l	d0,a1			; Get string pointer...
alloc_strcpy:	move.b	(a2)+,(a1)+		; Copy the string...
		bne.s	alloc_strcpy
*
		lea	no_object(a0),a2	; Get return result...
		tst.l	d2			; Do we have user data?
		beq.s	alloc_no_user		; If not, skip...
*
		moveq.l	#3,d0			; Get mask...
		add.l	d0,a1			; bump up...
		not.l	d0			; Flip it
		move.l	a1,d1			; Into data reg...
		and.l	d1,d0			; (wish we could do AND A1,D0)
		move.l	d0,nos_Object(a2)	; Store object pointer...
*
* Now, if namespace, we will init it...
*
alloc_no_user:	move.l	a3,a0			; Get tag list
		moveq.l	#0,d1			; Get default flags
		move.l	#ANO_Flags,d0		; Get flags tag
		CALLSYS	GetTagData		; Get the data into d0
		move.l	nos_NameSpace(a2),d1	; Get namespace?
		beq.s	alloc_done		; Skip this if none...
		move.l	d1,a0			; Get pointer to object
		move.l	d0,ns_Flags(a0)		; Set the flags...
		NEWLIST	a0			; NameSpace LIST at top
		lea	ns_Semaphore(a0),a0	; Get semaphore
		exg	a5,a6			; Get execbase...
		CALLSYS	InitSemaphore		; initialize
		exg	a5,a6			; swap utilitybase back...
*
alloc_done:	move.l	a3,a0			; Get tag list
		moveq.l	#0,d1			; Default priority
		move.l	#ANO_Priority,d0	; Priority tag
		CALLSYS	GetTagData		; Get the data
		move.b	d0,nos_Node+LN_PRI(a2)	; Set the priority...
		addq.w	#1,nos_UseCount(a2)	; Bump use count...
		move.l	a2,d0			; Get return result...
*
alloc_exit:	movem.l	(sp)+,d2-d3/a2-a3/a5	; Restore
		rts				; exit...

******* utility.library/FreeNamedObject ***************************************
*
*   NAME
*	FreeNamedObject -- frees a name object. (V39)
*
*   SYNOPSIS
*	FreeNamedObject(object);
*			A0
*
*	VOID FreeNamedObject(struct NamedObject *);
*
*   FUNCTION
*	Free one of a number of structures used by utility.library.
*	The item must not be a member of any NameSpace, and no one
*	may have it open other than yourself. If the object also
*	contained a NameSpace, that namespace must be empty.
*	Any additional space allocated via the datasize parameter
*	for AllocNamedObject() is also released.
*
*   INPUTS
*	object - the object to be freed
*
*   SEE ALSO
*	AllocNamedObjectA()
*
*******************************************************************************
FreeNamedObject:	xdef	FreeNamedObject
		move.l	a6,-(sp)
		move.l	ub_SysBase(a6),a6
		move.l	a0,d0
		beq.s	10$
		lea	nos_Node(a0),a0		; Adjust to real start...
		CALLSYS	FreeVec
10$		move.l	(sp)+,a6
		rts
