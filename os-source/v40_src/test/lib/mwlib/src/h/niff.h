/*****************************************************************************
    niff.h

    Mitchell/Ware Systems           Version 1.02            18-Feb-89

    New Iff Reader/Writer structs

    Philosophy:
        The first time I wrote an IFF reader, I tried to make it too general.
        As a result, it became unwildly and unmanagable. Besides, I was new
        to the IFF concept at the time. Now, with NIFF, things will be simple.

        1) The NIFF routines only provides the 'skeletal' structure (FORM,
           PROP, CAT, LIST) for the iff format. For the application-specific
           chunks, an array of custom ids and functions will exist.

        2) The NIFF writer will only write out the 'skeletal' structures and
           the custom chunks that are handed to it. The NIFF system will
           take care of offsets and chunk sizes and padding automatically.

        3) UNDER NO CIRCUMSTANCES will NIFF pay special attention to custom
           chunks. That is the province of the specific routine to handle it.

        4) The custom routines will provide custom stacks, if nesacary,
           to handle the custom chunks. NIFF will maintain a simple
           stack of its own along with a simple stack counter. The stack
           counter can be used by the custom routines to index custom
           properties, etc. An entry of FORM, CAT, LIST, or PROP into the
           custom array will be called upon reaching the beginning and end
           of scope of those group chunks.

        Now with that in mind, let's hop to it!
*****************************************************************************/

#ifndef _NIFF_
#define _NIFF_ 1

/* ---------- ID -------------------------------------------------------*/

#define ID long   // An ID is four printable ASCII chars but
                  //    stored as a LONG for efficient copy & compare.

typedef struct IDC { UBYTE c[4]; } IDC;

/* Four-character IDentifier builder.*/
#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )

/* Standard group IDs.  A chunk with one of these IDs contains a
   SubTypeID followed by zero or more chunks.*/
#define FORM MakeID('F','O','R','M')
#define PROP MakeID('P','R','O','P')
#define LIST MakeID('L','I','S','T')
#define CAT  MakeID('C','A','T',' ')
#define FILLER MakeID(' ',' ',' ',' ')

/* The IDs "FOR1".."FOR9", "LIS1".."LIS9", & "CAT1".."CAT9" are reserved
 * for future standardization.*/

/* ---------- Chunk ----------------------------------------------------*/

/* All chunks start with a type ID and a count of the data bytes that
   follow--the chunk's "logical size" or "data size". If that number is odd,
   a 0 pad byte is written, too. */

typedef struct ChunkHeader {
    ID   ckID;
    LONG  ckSize;
    } ChunkHeader;

typedef struct Chunk {
    ID   ckID;
    LONG  ckSize;
    UBYTE ckData[ 1 /*REALLY: ckSize*/ ];
    } Chunk;

/* Need to know whether a value is odd so can word-align.
 */
#define IS_ODD(a)   ((a) & 1)

/* This macro rounds up to an even number.
 */
#define WordAlign(size)   ((size+1)&~1)

/* ALL CHUNKS MUST BE PADDED TO EVEN NUMBER OF BYTES.
 * ChunkPSize computes the total "physical size" of a padded chunk from
 * its "data size" or "logical size".
 */
#define ChunkPSize(dataSize)  (WordAlign(dataSize) + sizeof(ChunkHeader))

/* The Grouping chunks (LIST, FORM, PROP, & CAT) contain concatenations of
 * chunks after a subtype ID that identifies the content chunks.
 * "FORM type XXXX", "LIST of FORM type XXXX", "PROPerties associated
 * with FORM type XXXX", or "conCATenation of XXXX".
 */
typedef struct GroupHeader {
    ID   ckID;
    LONG  ckSize;  /* this ckSize includes "grpSubID".*/
    ID    grpSubID;
    } GroupHeader;

typedef struct GroupChunk{
    ID   ckID;
    LONG  ckSize;
    ID    grpSubID;
    UBYTE grpData[ 1 /*REALLY: ckSize-sizeof(grpSubID)*/ ];
    } GroupChunk;

/*****************************************************************************

    niff_reader

*****************************************************************************/

typedef struct NiffStack {  /* Stack for the Niff reader */
    struct NiffStack *Next;     /* next one in the stack */
    GroupHeader Group;
    short StackCnt;

    /* for the writer routines
    */
    long Start;     /* start of group */
    } NiffStack;

typedef struct NiffCustom { /* An entry into the array of routines */
    ID id;              /* custom id */
    BOOL (*Custom)();   /* routine to be called */
    } NiffCustom;

typedef struct NiffCustomIni { /* This form is for initialization to allow double quotes! */
    IDC id;             /* custom id */
    BOOL (*Custom)();   /* routine to be called */
    } NiffCustomIni;

typedef struct NiffSize {   /* trackers of chunk sizes to be written out
                               upon closing */
    struct NiffSize *Next, *Prev;   /* doubly-linked! */
    long Where, Size;       /* Where to seek to, and what Size to write out! */
    } NiffSize;

typedef struct Niff {   /* Structure for Niff */
    int fd;
    NiffStack *stack;
    NiffCustom *array;
    NiffSize *nsb, *nse; /* doubly-linked chain of sizes to be resolved - writer only */
    short StackCnt;      /* bumped for every group structure encountered */
    BOOL ok;             /* if FALSE, shutdown! */
    void *user_env;      /* User's environment area */
    ID subID;            /* subID of a FORM, CAT, or PROP. Valid only during niff_push. */
    } Niff;

typedef enum NiffCommand {  /* commands to custom routines */
    niff_push,
    niff_pop,
    niff_process
    } NiffCommand;

#endif
