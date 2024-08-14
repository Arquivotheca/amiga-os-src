        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

	INCLUDE	"exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "utilitybase.i"
	INCLUDE "tagitem.i"

	LIST

;---------------------------------------------------------------------------

	XREF	_LVOAllocVec
	XREF	_LVOFreeVec

;---------------------------------------------------------------------------

	XDEF	NextTagItem
	XDEF	FindTagItem
	XDEF	GetTagData
	XDEF	PackBoolTags
	XDEF	FilterTagChanges
	XDEF	MapTags
	XDEF	AllocateTagItems
	XDEF	RefreshTagItemClones
	XDEF	CloneTagItems
	XDEF	FreeTagItems
	XDEF	TagInArray
	XDEF	FilterTagItems
	XDEF	ApplyTagChanges

;---------------------------------------------------------------------------

;	next_tag = NextTagItem( tagItemPtr )
;	D0			A0
;
;	struct TagItem *NextTagItem( struct TagItem **tagItemPtr );

NextTagItem:
	move.l	(a0),d0		; load address of first tag
	beq.s   NTIExit		; if 0, then we didn't find anything this time

NTILoop:
	move.l	d0,a1		; load address of current tag
	move.l	ti_Tag(a1),d1   ; load the tag type
	bmi.s	NTISuccess	; if bit 31 is set then this is a user tag, so we got our man!
	beq.s	NTIFailure	; if it is 0, then its TAG_DONE so exit

	cmp.l	#TAG_SKIP,d1	; second-level check
	bhi.s	NTISuccess	; if greater than TAG_SKIP, then its a user tag

	add.b	d1,d1           ; calculate index into jump table
	jmp	NTIJumpTable(pc,d1.b)	; evil ASM jump table!

NTIJumpTable:
	bra.s	NTIFailure	; never called, but used to keep the jump table happy
	bra.s	NTITagIgnore
	bra.s	NTITagMore
;	bra.s	NTITagSkip	; ** sneaky, last entry in jump table is the actual code!

NTITagSkip:
	move.l	ti_Data(a1),d1		; # of tags to skip
	asl.l	#3,d1			; convert # of tags into total byte size
	add.l	d1,d0			; skip all these bytes

NTITagIgnore:
	addq.l	#ti_SIZEOF,d0		; load address of next tag
	bra.s	NTILoop			; try again

NTITagMore:
	move.l	ti_Data(a1),d0		; go to a new tag list
	bne.s	NTILoop			; if more tags, loop back
	move.l	d0,(a0)			; no more tags, so leave
	rts

NTISuccess:
	addq.l	#ti_SIZEOF,a1		; address of next tag
	move.l	a1,(a0)			; store it in the "next tag" pointer
	rts

NTIFailure:
	moveq.l	#0,d0			; nothing left in life...
	move.l	d0,(a0)			; store it in the "next tag" pointer
NTIExit:
	rts

;---------------------------------------------------------------------------

;	tag = FindTagItem( tagVal, tagList)
;	D0		   D0	   A0
;
;	struct TagItem *FindTagItem( Tag tagVal, struct TagItem *tagList );

FindTagItem:
	move.l	d2,-(sp)
	move.l	d0,d2		; desired tag value in d2
	move.l	a0,-(sp)	; save list pointer on stack

FTILoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	FTIExit		; no more tags so split
	move.l	d0,a0           ; tag pointer in a0
	cmp.l	ti_Tag(a0),d2	; is this what we want?
	bne.s	FTILoop		; nope, so look some more

FTIExit:
	addq.l	#4,sp		; remove list pointer from stack
	move.l	(sp)+,d2	; good to see you back!
	rts

;---------------------------------------------------------------------------

;	BOOL TagInArray( tag, tagArray )
;	D0	         D0   A0
;
;	BOOL TagInArray( Tag tag, Tag *tagArray);

TagInArray:
	move.l	(a0)+,d1	; get a tag value from the array
	beq.s	TIAFailure	; if 0 (TAG_END), then exit
	cmp.l	d0,d1		; does this match what we want?
	bne.s	TagInArray	; not what we want, look again
	moveq.l	#1,d0		; found it, set to TRUE
        rts			; bye

TIAFailure:
	moveq.l	#0,d0		; not found, set to FALSE
        rts			; bye

;---------------------------------------------------------------------------

;	value = GetTagData(tagVal, default, tagList)
;	D0		   D0	   D1	    A0
;
;	ULONG GetTagData(Tag TagVal, ULONG Default, struct TagItem *TagList)

GetTagData:
	move.l	d1,-(sp)		; save default value
	CALL	FindTagItem		; find the tag we want
	tst.l	d0			; did we get a tag back?
	beq.s   GTDDefault		; if no tag, go return default
	move.l	d0,a0			; tag pointer in a0
	move.l  ti_Data(a0),d0		; got a tag, return its data
	addq.l	#4,sp			; remove default value from stack
	rts				; bye

GTDDefault:
	move.l	(sp)+,d0		; remove default value from stack
	rts				; bye

;---------------------------------------------------------------------------

;	tagList = AllocateTagItems( numItems )
;	D0		            D0
;
;	struct TagItem *AllocateTagItems( ULONG numItems);

AllocateTagItems:
	asl.l	#3,d0	                   ; numitems * sizeof(struct TagItem)
	move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1 ; memory requirements
	move.l	a6,-(sp)		   ; save UtilityBase
	move.l	ub_SysBase(a6),a6	   ; load ExecBase
	CALL	AllocVec		   ; allocate the memory
	move.l	(sp)+,a6		   ; restore UtilityBase
	rts				   ; bye

;---------------------------------------------------------------------------

;	FreeTagItems( tagList )
;			A0
;
;	void FreeTagItems( struct TagItem *tagList );

FreeTagItems:
	move.l	a0,a1			; tag list in a1
	move.l	a6,-(sp)		; save UtilityBase
	move.l	ub_SysBase(a6),a6	; load ExecBase
	CALL	FreeVec			; free the memory
	move.l	(sp)+,a6		; restore UtilityBase
	rts

;---------------------------------------------------------------------------

;	RefreshTagItemClones( cloneTagItems, originalTagItems )
;			      A0	     A1
;
;	void RefreshTagItemClones( struct TagItem *cloneTagItems,
;				  struct TagItem *originalTagItems );

RefreshTagItemClones:
	move.l	a2,-(sp)	; save this guy
	move.l	a0,a2		; point to first tag in clone's list
	move.l	a1,-(sp)	; save original's list pointer on stack

RTICLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	RTICExit	; no more tags so split
	move.l	d0,a0           ; tag pointer in a0
	move.l	(a0)+,(a2)+	; copy the tag!
	move.l	(a0),(a2)+	; copy the data!
	bra.s	RTICLoop	; do next one

RTICExit:
	clr.l	(a2)		; set last tag's ti_Tag to TAG_DONE
	addq.l	#4,sp		; remove list pointer from stack
	move.l	(sp)+,a2	; hi guy
	rts

;---------------------------------------------------------------------------

;	newTagList = CloneTagItems( tagList )
;	D0			    A0
;
;	struct TagItem *CloneTagItems( struct TagItem *tagList );

CloneTagItems:

	move.l	d2,-(sp)	; save this guy
	move.l	a0,-(sp)	; save list pointer on stack
	move.l	a0,-(sp)	; save list pointer on stack
	move.l	#0,d2		; initialize counter

CTILoop:
	addq.l	#1,d2		; count an item
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	bne.s	CTILoop		; if so, keep looping

	move.l	d2,d0		; number of items to allocate
	CALL	AllocateTagItems
	tst.l	d0		; did it work?
	beq.s	CTIExit
	move.l	d0,d2		; new tag list in d2
	move.l	d0,a0		; new tag list in a0
	move.l	4(sp),a1	; old tag list in a1
	CALL	RefreshTagItemClones
	move.l	d2,d0		; return new tag list

CTIExit:
	addq.l	#8,sp		; remove list pointers from stack
	move.l	(sp)+,d2	; good to see you back!
	rts

;---------------------------------------------------------------------------

;	boolflags = PackBoolTags( initialFlags, tagList, boolMap )
;	D0			  D0	        A0       A1
;
;	ULONG PackBoolTags( ULONG initialFlags, struct TagItem *tagList,
;                           struct TagItem *boolMap);

PackBoolTags:
	movem.l	d2/a2/a3,-(sp)
	move.l	a0,-(sp)	; save tag list pointer on stack
	move.l	d0,d2		; d2 has current flags
	move.l	a1,a2		; a2 has boolMap

PBTLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	PBTExit		; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in boolMap
	moveq.l	#0,d1		; default will be 0
	move.l	a2,a0		; load boolMap address
	CALL	GetTagData	; find the tag in boolMap
	tst.l	ti_Data(a3)	; if data is 0...
	beq.s	1$
	or.l	d0,d2		; or in tag data with current flags
	bra.s	PBTLoop		; do next item

1$:	not.l   d0
	and.l   d0,d2		; remove tag data bits from current flags
	bra.s	PBTLoop		; do next one

PBTExit:
	move.l	d2,d0		; return flags
	addq.l	#4,sp		; remove list pointer
	movem.l	(sp)+,d2/a2/a3
	rts

;---------------------------------------------------------------------------

;	nvalid = FilterTagItems(tagList, tagArray, logic)
;	D0		       A0	A1	  D0
;
;	ULONG FilterTagItems(struct TagItem *tagList, Tag *tagArray,
;			     LONG logic);

FilterTagItems:
	movem.l	d2/a2/a3,-(sp)
	move.l	a0,-(sp)	; save tag list pointer on stack
	moveq.l	#0,d2		; d2 has item counter
	move.l	a1,a2		; a2 has tagArray

	tst.b	d0		; what filtering is needed?
	bne.s	FNLoop

FALoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	FLTIExit	; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in tagArray
	move.l	a2,a0		; tagArray in a0
	CALL	TagInArray	; find the tag in tagArray
	tst.w	d0		; if the tag is in the array
	bne.s	1$
	moveq	#TAG_IGNORE,d0
	move.l	d0,ti_Tag(a3)
	bra.s	FALoop

1$:	addq.l	#1,d2		; increment item counter
	bra.s	FALoop

FNLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	FLTIExit	; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in tagArray
	move.l	a2,a0		; tagArray in a0
	CALL	TagInArray	; find the tag in tagArray
	tst.w	d0		; if the tag is in the array
	beq.s	1$
	moveq.l	#TAG_IGNORE,d0
	move.l	d0,ti_Tag(a3)
	bra.s	FNLoop

1$:	addq.l	#1,d2		; increment item counter
	bra.s	FNLoop

FLTIExit:
	move.l	d2,d0		; return counter
	addq.l	#4,sp		; remove list pointer from stack
	movem.l	(sp)+,d2/a2/a3
	rts

;---------------------------------------------------------------------------

;	FilterTagChanges( changeList, oldValues, apply)
;			  A0	      A1	 D0
;
;	void FilterTagChanges( struct TagItem *changeList,
;			       struct TagItem *oldValues, LONG apply );

FilterTagChanges:
	movem.l	d2/a2/a3,-(sp)
	move.l	a0,-(sp)	; save tag list pointer on stack
	move.l	d0,d2		; d2 has "apply" flag
	move.l	a1,a2		; a2 has oldValues

FTCLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	FTCExit		; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in oldValues
	move.l	a2,a0		; oldValues in a0
	CALL	FindTagItem	; find the tag in oldValue
	tst.l	d0		; if the tag is in the array
	beq.s	FTCLoop         ; not in oldValue, get next tag
	move.l	d0,a0		; oldValue tag in a0
	move.l	ti_Data(a0),d0	; oldValue tag data in d0
	cmp.l	ti_Data(a3),d0	; does it match?
	bne.s	1$		; no match, so do other part
	moveq	#TAG_IGNORE,d0
	move.l	d0,ti_Tag(a3)
	bra.s	FTCLoop

1$:	tst.l	d2
	beq.s	FTCLoop
	move.l	ti_Data(a3),ti_Data(a0)
	bra.s	FTCLoop

FTCExit:
	addq.l	#4,sp			; remove list pointer from stack
	movem.l	(sp)+,d2/a2/a3
	rts

;---------------------------------------------------------------------------

;	MapTags(tagList, mapList, includeMiss)
;	        A0	    A1       D0
;
;	VOID MapTags(struct TagItem *tagList, struct TagItem mapList,
;		     LONG includeMiss);

MapTags:
	movem.l	d2/a2/a3,-(sp)
	move.l	a0,-(sp)	; save tag list pointer on stack
	move.l	d0,d2		; d2 has "incluseMiss" flag
	move.l	a1,a2		; a2 has map

MTLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	MTExit		; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in map
	move.l	a2,a0		; map in a0
	CALL	FindTagItem	; find the tag in map
	tst.l	d0		; if the tag is in the array
	beq.s	1$ 	        ; not in map
	move.l	d0,a0		; mapping in a0
	move.l	ti_Data(a0),ti_Tag(a3)
	bne.s	MTLoop
	moveq	#TAG_IGNORE,d0
	move.l	d0,ti_Tag(a3)
	bra.s	MTLoop

1$:	tst.l	d2		; do we include the non-mapping tags?
	bne.s	MTLoop
	moveq.l	#TAG_IGNORE,d0
	move.l	d0,ti_Tag(a3)	; don't want to see this one
	bra.s	MTLoop          ; all done, loop back

MTExit:
	addq.l	#4,sp			; remove list pointer from stack
	movem.l	(sp)+,d2/a2/a3
	rts

;---------------------------------------------------------------------------

;	ApplyTagChanges(list,changeList);
;			A0   A1
;
;	VOID ApplyTagChanges(struct TagItem *,struct TagItem *);

ApplyTagChanges:
	move.l	a2,-(sp)
	move.l	a3,-(sp)
	move.l	a0,-(sp)	; save tag list pointer on stack
	move.l	a1,a2		; a2 has changeList

ATCLoop:
	move.l	sp,a0		; where the list pointer is
	CALL	NextTagItem	; get next tag available
	tst.l	d0		; did we get something?
	beq.s	ATCExit		; no more tags so split
	move.l	d0,a3           ; tag pointer in a3
	move.l	ti_Tag(a3),d0   ; tag value to look for in changeList
	move.l	a2,a0		; changeList in a0
	CALL	FindTagItem	; find the tag in changeList
	tst.l	d0		; if the tag is in the array
	beq.s	ATCLoop         ; not in changeList, get next tag
	move.l	d0,a0		; changeList tag in a0
	move.l	ti_Data(a0),ti_Data(a3)	; update original tag
	bra.s	ATCLoop

ATCExit:
	addq.l	#4,sp		; remove list pointer from stack
	move.l	(sp)+,a3
	move.l	(sp)+,a2
	rts
