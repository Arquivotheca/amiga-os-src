/*****************************************************************************/

#define	NAME	"« MemCheck »"

/*****************************************************************************/

struct MemCheck
{
    ULONG		 mc_Cookie;		/* Identifier (0xC0DEF00D) */
    ULONG		 mc_Size;		/* Size of allocation */
    ULONG		 mc_Hunk;		/* Hunk within executable where allocation was made */
    ULONG		 mc_Offset;		/* Offset within hunk where allocation was made */
    struct Task		*mc_Task;		/* Task address */
    ULONG		 mc_CheckSum;		/* Check sum of the cookie */
    struct DateStamp	 mc_DateStamp;		/* Allocation date stamp (3 LONG) */
    ULONG		 mc_Pad;		/* Alignment */
    UBYTE		 mc_Name[16];		/* Allocation task name */
    ULONG		 mc_StackPtr[16];	/* 16 long-words of stack at allocation time */
};

/*****************************************************************************/

struct MemCheckInfo
{
    struct DateStamp	 mci_DateStamp;		/* When MemCheck was activated */
    UWORD		 mci_Version;		/* Version of MemCheck */
    UWORD		 mci_Revision;		/* Revision of MemCheck */
    ULONG		 mci_Flags;		/* Additional flags */
    UWORD		 mci_Stack;		/* Number of lines of stack to show */
    UWORD		 mci_InfoSize;		/* Size of MemCheck structure */
    UWORD		 mci_PreSize;		/* Size of pre-wall */
    UWORD		 mci_PostSize;		/* Size of post-wall */
    UBYTE		 mci_FillChar;		/* Fill character for wall */
    UBYTE		 mci_Pad;		/* Word aligned */
};

/*****************************************************************************/

#define	INFOSIZE	sizeof (struct MemCheck)
#define	PRESIZE		32
#define	POSTSIZE	32
#define	FILLCHAR	0xBB
