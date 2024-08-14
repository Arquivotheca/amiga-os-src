/* globaldata.h
 *
 */

/*****************************************************************************/

#define ASM		__asm
#define REG(x)		register __ ## x
#define MEMORY_FOLLOWING(ptr)		((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)	((void *)( ((ULONG)ptr) + n ))
#define	MAKE_LONG(a,b,c,d) ((unsigned char)(a)<<24 | (unsigned char)(b)<<16 | (unsigned char)(c)<<8 | (unsigned char)(d))
#define	V(x)		((void *)(x))

/*****************************************************************************/

/*
 * exec.library  LVO $ff3a -198  AllocMem(byteSize,requirements)(d0/d1)
 * exec.library  LVO $ff34 -204  AllocAbs(byteSize,location)(d0/a1)
 * exec.library  LVO $ff2e -210  FreeMem(memoryBlock,byteSize)(a1,d0)
 * exec.library  LVO $ff28 -216  AvailMem(requirements)(d1)
 * exec.library  LVO $fd54 -684  AllocVec(byteSize,requirements)(d0/d1)
 * exec.library  LVO $fd4e -690  FreeVec(memoryBlock)(a1)
 *
 */

#define	LVO_ALLOCMEM	-198
#define	LVO_ALLOCABS	-204
#define	LVO_FREEMEM	-210
#define	LVO_AVAILMEM	-216
#define	LVO_ALLOCVEC	-684
#define	LVO_FREEVEC	-690

/*****************************************************************************/

#define	CHECK_FREE	(1L<<0)
#define	CHECK_ALLOC	(1L<<1)

/*****************************************************************************/

#define	ACALLER		0
#define	CCALLER		2

/*****************************************************************************/

#define  BIGBUFSZ  (20000L)
#define  SBUFSZ    (80L)

/*****************************************************************************/

#define	TEMPLATE	"CX_PRIORITY/K/N,CX_POPKEY/K,CX_POPUP/K,PUBSCREEN/K,STACKLINES/K/N,COMPAT/S"
#define OPT_PRIORITY    0
#define OPT_POPKEY      1
#define OPT_POPUP       2
#define	OPT_PUBSCREEN	3
#define	OPT_STACKLINES	4
#define	OPT_COMPAT	5
#define	NUM_OPTS	6

/*****************************************************************************/

#define	GID_CONTEXT	0
#define	GID_ANCHOR	1

#define	GID_SET_FREE	2
#define	GID_SET_ALLOC	3
#define	GID_SET_ALL	4

#define	GID_CHECK_FREE	5
#define	GID_CHECK_ALLOC	6
#define	GID_CHECK_ALL	7

#define	GID_MAX		8

/*****************************************************************************/

#define	MMC_HIDE	500
#define	MMC_QUIT	501

/*****************************************************************************/

struct GlobalData
{
    struct MemCheckInfo		 gd_MemCheckInfo;

    /* Shared Libraries */
    struct ExecBase		*gd_SysBase;
    struct Library		*gd_DOSBase;
    struct Library		*gd_UtilityBase;
    struct Library		*gd_IntuitionBase;
    struct Library		*gd_GadToolsBase;
    struct Library		*gd_CxBase;
    struct Library		*gd_LayersBase;
    ULONG			 gd_LStart;
    ULONG			 gd_LEnd;

    /* Control information */
    struct Process		*gd_Process;
    LONG			 gd_ErrorLevel;
    LONG			 gd_ErrorNumber;
    LONG			 gd_ErrorString;
    LONG			 gd_Going;
    ULONG			 gd_Flags;
    struct RDArgs		*gd_RDArgs;
    ULONG			 gd_Options[NUM_OPTS];

    /* Functions */
    VOID * (*ASM gd_AllocMem)(REG (d0) ULONG size, REG (d1) ULONG attributes, REG (a6) struct Library *);
    VOID * (*ASM gd_AllocAbs)(REG (d0) ULONG size, REG (a1) APTR location, REG (a6) struct Library *);
    VOID   (*ASM gd_FreeMem) (REG (a1) VOID * block, REG (d0) ULONG size, REG (a6) struct Library *);
    ULONG  (*ASM gd_AvailMem)(REG (d1) ULONG attributes, REG (a6) struct Library *);
    VOID * (*ASM gd_AllocVec)(REG (d0) ULONG size, REG (d1) ULONG attributes, REG (a6) struct Library *);
    VOID   (*ASM gd_FreeVec) (REG (a1) VOID * block, REG (a6) struct Library *);

    /* Memory tracking */
    struct MemHeader		*gd_MemHeader;

    /* Application data */
    struct MsgPort		*gd_MsgPort;
    UBYTE			 gd_WorkBuffer[BIGBUFSZ];
    UBYTE			 gd_ReadBuffer[SBUFSZ];

    /* GUI information */
    UBYTE			 gd_ScreenNameBuffer[MAXPUBSCREENNAME+1];
    STRPTR			 gd_ScreenName;
    struct Screen		*gd_Screen;
    struct DrawInfo		*gd_DrInfo;
    VOID			*gd_VI;
    struct Window		*gd_Window;
    struct Menu			*gd_Menu;
    struct Gadget		*gd_Gads[GID_MAX];
    UBYTE			 gd_WindowTitle[80];
    struct IBox			 gd_Memory;

    /* Commodity information */
    struct MsgPort		*gd_CxPort;
    CxObj			*gd_CxObj;
    CxObj			*gd_CxFilter;
    STRPTR			 gd_CxPopKey;
};

/*****************************************************************************/

#define	GDF_CLOSEWINDOW		(1L<<0)
    /* Indicate that the window should close */

#define	GDF_OPENWINDOW		(1L<<1)
    /* Indicate that the window should open */

#define	GDF_BUSY		(1L<<2)
    /* Show that we are busy doing something */

#define	GDF_CLOSING		(1L<<3)
    /* Show that we want to go home to bed */

#define	GDF_ACTIVE		(1L<<4)
    /* Are we active? */

#define	GDF_COMPATIBLE		(1L<<5)
    /* Supposed to be compatible with MungWall? */

/*****************************************************************************/

#define	SysBase			 gd->gd_SysBase
#define	DOSBase			 gd->gd_DOSBase
#define	UtilityBase		 gd->gd_UtilityBase
#define	IntuitionBase		 gd->gd_IntuitionBase
#define	GadToolsBase		 gd->gd_GadToolsBase
#define	CxBase			 gd->gd_CxBase
#define	LayersBase		 gd->gd_LayersBase
