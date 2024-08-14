/**************************************************************************
*
* janusvar.h -- the software data structures for the janus board.
*
*
* Copyright (c) 1986, Commodore Amiga Inc.  All rights reserved.
*
***************************************************************************/

/* all bytes described here are described in the byte order of the 8088.
 * Note that words and longwords in these structures will be accessed from
 * the word access space to preserve the byte order in a word -- the 8088
 * will access longwords by reversing the words: like a 68000 access to the
 * word access memory.
 */
/* JanusMemHead -- a data structure roughly analogous to an exec mem chunk.
 * It is used to keep track of memory used between the 8088 and the 68000.
 */

struct JanusMemHead {
    UBYTE   jmh_Lock;			/* lock byte between processors */
    UBYTE   jmh_pad0;
    APTR    jmh_68000Base;		/* rptr's are relative to this */
    UWORD   jmh_8088Segment;		/* segment base for 8088 */
    RPTR    jmh_First;			/* offset to first free chunk */
    RPTR    jmh_Max;			/* max allowable index */
    UWORD   jmh_Free;			/* total number of free bytes -1 */
};

/* JanusMemChunk -- keep track of individualy freed chunks of memory.
 * Memory Chunks are longword aligned in this memory.
 */

struct JanusMemChunk {
    RPTR    jmc_Next;			/* rptr to next free chunk */
    UWORD   jmc_Size;			/* size of chunk -1*/
};


#ifdef undef
this stuff is saved for future use, but is not yet thought out
/* JanusList -- an RPTR/Exec style list header.
 */
struct JanusList {
    RPTR    jl_Head;
    RPTR    jl_Tail;
    RPTR    jl_TailPred;
    UBYTE   jl_Lock;			/* lock byte between processors */
    UBYTE   jl_pad0;
};

/* JanusNode -- an RPTR/Exec style node.
 */
struct JanusReqList {
    RPTR    jn_Succ;
    RPTR    jn_Pred;
    RPTR    jn_Name;
    UWORD   jn_ReqIndex;		/* this' index into jb_CommRegs */
};

#endif undef

/* JanusBase -- the master data table for the janus project. It is located
 * at the bottom of parameter memory.
 */

struct JanusBase {
    UBYTE   jb_Lock;			/* lock byte between processors */
    UBYTE   jb_8088Go;
    struct  JanusMemHead jb_ParamMem;	/* free mem pool for param memory */
    struct  JanusMemHead jb_BufferMem;	/* free mem pool for buffer memory */
    RPTR    jb_Interrupts;		/* (UBYTE *) of request byte-pairs */
    RPTR    jb_Parameters;		/* array of ptrs to parameter areas  */
    UWORD   jb_NumInterrupts;		/* number of interrupts & parameters */
};


/* constant to set to indicate a pending software interrupt */
#define JSETINT     0x7f

