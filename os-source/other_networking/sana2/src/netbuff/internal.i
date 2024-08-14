	optimon $ffffffff

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/ables.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"macros.i"
	INCLUDE	"debug.i"

	INT_ABLES

	INCLUDE	"netbuff.i"


 STRUCTURE NETBUFFLIBRARY,LIB_SIZE
	UWORD	NBL_PADDING

	APTR	NBL_SYSLIB
	APTR	NBL_SEGLIST

	STRUCT	NBL_FREEPOOL,MLH_SIZE
	APTR	NBL_DEFERED
	LABEL	NBL_SIZE
