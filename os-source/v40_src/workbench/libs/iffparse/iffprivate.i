	IFND	IFFPRIVATE_I
IFFPRIVATE_I	SET	1

;---------------------------------------------------------------------------

	IFND LIBRARIES_IFFPARSE_I
	INCLUDE "iffparse.i"
	ENDC

	IFND UTILITY_HOOKS_H
	INCLUDE "utility/hooks.i"
	ENDC

;---------------------------------------------------------------------------

; Structure associated with an active IFF file.
; "iff_Stream" is a value used by the read/write functions -
; it will not be accessed by the library itself and can have any value
; (could even be a pointer or a BPTR).
;
   STRUCTURE IFFHandleP,iff_SIZEOF		; iffp_IFFHandle
	STRUCT	iffp_Stack,MLH_SIZE
	STRUCT	iffp_WriteBuffers,MLH_SIZE
	APTR	iffp_StreamHook
   LABEL iffp_SIZEOF

iffp_Stream	EQU	iff_Stream
iffp_Flags	EQU	iff_Flags
iffp_Depth	EQU	iff_Depth

; Additional bit masks for "iff_Flags" field for the library only.
; These bits are "protected" by the IFFF_RESERVED mask.
IFFFP_NEWIO	EQU	1<<16	; newly opened file
IFFFP_PAUSE	EQU	1<<17	; paused at end of context

;---------------------------------------------------------------------------

; A node associated with a context on the iff_Stack.  Each node
; represents a chunk, the stack representing the current nesting
; of chunks in the open IFF file.  Each context node has associated
; local context items in the LocalItems list.  The ID, type, size and
; scan values describe the chunk associated with this node.

   STRUCTURE ContextNodeP,cn_SIZEOF	; cnp_CNode
	STRUCT	cnp_LocalItems,MLH_SIZE
	LONG	cnp_Scan	; The *REAL* scan value
   LABEL cnp_SIZEOF

cnp_ID		EQU	cn_ID
cnp_Type	EQU	cn_Type
cnp_Size	EQU	cn_Size
cnp_UserScan	EQU	cn_Scan

;---------------------------------------------------------------------------

; Write buffer storage node.  Buffers writes for non-seekable streams.
; Nodes stored in the "WriteBuffers" list off the IFFHandle struct.

   STRUCTURE WriteBuffer,MLN_SIZE		; wb_Node
	LONG wb_Size
	APTR wb_Buf
   LABEL wb_SIZEOF

;---------------------------------------------------------------------------

; Private parts of LocalContextItem, including the size of user data
; and the purge vectors.
;
   STRUCTURE LocalContextItemP,lci_SIZEOF	; lip_LCItem
	LONG lip_UserSize
	APTR lip_PurgeHook
   LABEL lib_SIZEOF

lip_Ident	EQU	lci_Ident
lip_ID		EQU	lci_ID
lip_Type	EQU	lci_Type

;---------------------------------------------------------------------------

; ChunkHandler: a local context item which contains
; a handler function to be applied when a chunk is pushed.
;
   STRUCTURE ChunkHandler,0
	APTR ch_HandlerHook
	APTR ch_Object
   LABEL ch_SIZEOF

;---------------------------------------------------------------------------

; Collection Property: a local context item that contains part of
; the linked list of collection properties - the ones for this
; context.  Only the ones for this context get purged when this
; item gets purged.  First is the first element for this context
; level, LastPtr is the value of the Next pointer for the last
; element at this level.
;
   STRUCTURE CollectionList,0
	APTR cl_First
	APTR cl_LastPtr
	APTR cl_Context
   LABEL cl_SIZEOF

;---------------------------------------------------------------------------

	ENDC	; IFFPRIVATE_I
