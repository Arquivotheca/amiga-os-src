#ifndef IFFPRIVATE_H
#define IFFPRIVATE_H


/*****************************************************************************/


#ifndef LIBRARIES_IFFPARSE_H
#include "iffparse.h"
#endif

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif


/*****************************************************************************/


/* Structure associated with an active IFF file.
 * "iff_Stream" is a value used by the read/write functions -
 * it will not be accessed by the library itself and can have any value
 * (could even be a pointer or a BPTR).
 */
struct IFFHandleP
{
    struct IFFHandle iffp_IFFHandle;
    struct MinList   iffp_Stack;
    struct MinList   iffp_WriteBuffers;
    struct Hook	    *iffp_StreamHook;
};

#define iffp_Stream iffp_IFFHandle.iff_Stream
#define iffp_Flags  iffp_IFFHandle.iff_Flags
#define iffp_Depth  iffp_IFFHandle.iff_Depth


/* Additional bit masks for "iff_Flags" field for the library only.
 * These bits are "protected" by the IFFF_RESERVED mask.
 */
#define IFFFP_NEWIO (1L<<16)	/* newly opened file */
#define IFFFP_PAUSE (1L<<17)	/* paused at end of context */


/*****************************************************************************/


/* A node associated with a context on the iff_Stack. Each node
 * represents a chunk, the stack representing the current nesting
 * of chunks in the open IFF file. Each context node has associated
 * local context items in the LocalItems list. The ID, type, size and
 * scan values describe the chunk associated with this node.
 */
struct ContextNodeP
{
    struct ContextNode cnp_CNode;
    struct MinList     cnp_LocalItems;
    LONG	       cnp_Scan;	/* The *REAL* scan value */
};

#define cnp_Node	cnp_CNode.cn_Node
#define cnp_ID		cnp_CNode.cn_ID
#define cnp_Type	cnp_CNode.cn_Type
#define cnp_Size	cnp_CNode.cn_Size
#define	cnp_UserScan	cnp_CNode.cn_Scan


/*****************************************************************************/


/* Write buffer storage node. Buffers writes for non-seekable streams.
 * Nodes stored in the "WriteBuffers" list off the IFFHandle struct.
 */
struct WriteBuffer
{
    struct MinNode  wb_Node;
    LONG	    wb_Size;
    UBYTE	   *wb_Buf;
};


/*****************************************************************************/


/* Private parts of LocalContextItem, including the size of user data
 * and the purge vectors.
 */
struct LocalContextItemP
{
    struct LocalContextItem  lip_LCItem;
    struct Hook	            *lip_PurgeHook;
};

#define lip_Node  lip_LCItem.lci_Node
#define lip_Ident lip_LCItem.lci_Ident
#define lip_ID	  lip_LCItem.lci_ID
#define lip_Type  lip_LCItem.lci_Type


/*****************************************************************************/


/* ChunkHandler: a local context item which contains
 * a handler function to be applied when a chunk is pushed.
 */
struct ChunkHandler
{
    struct Hook	*ch_HandlerHook;
    APTR	 ch_Object;	 /* "Object" passed to call-back */
};


/*****************************************************************************/


/* Collection Property: a local context item that contains part of
 * the linked list of collection properties - the ones for this
 * context. Only the ones for this context get purged when this
 * item gets purged. First is the first element for this context
 * level, LastPtr is the value of the Next pointer for the last
 * element at this level.
 */
struct CollectionList
{
    struct CollectionItem *cl_First;
    struct CollectionItem *cl_LastPtr;
    struct ContextNode	  *cl_Context;
};


/*****************************************************************************/


#endif /* IFF_PRIVATE_H */
