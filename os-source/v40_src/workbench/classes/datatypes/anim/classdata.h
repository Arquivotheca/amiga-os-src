
/*****************************************************************************/

struct StartupMsg
{
    struct Message		 sm_Message;		/* Embedded message */
    struct ClassBase		*sm_ClassBase;		/* Pointer to the library base */
    Class			*sm_Class;		/* Pointer to the class */
    Object			*sm_Object;		/* Pointer to the object */
};

/*****************************************************************************/

/* Simple InterProcess Communications message structure */
struct SIPCMsg
{
    struct Message		 sm_Message;		/* Embedded message */
    ULONG			 sm_Type;		/* Type of message */
    VOID			*sm_Data;		/* Data */
};

#define	SIPCT_COMMAND	1
#define	SIPCT_MESSAGE	2
#define	SIPCT_QUIT	3

/*****************************************************************************/

/* object data */
struct localData
{
    ULONG			 lod_ModeID;		/* Display mode id */
    struct BitMapHeader		 lod_BMHD;		/* BitMap header */
    struct BitMap		*lod_SourceBMap;	/* Source Bitmap */
    struct BitMap		*lod_SourceBMap1;	/* Source Bitmap (1) */
    struct BitMap		*lod_SourceBMap2;	/* Source Bitmap (2) */
    struct BitMap		*lod_BMap;		/* Destination Bitmap */
    struct ColorRegister	*lod_Colors;
    LONG			*lod_CRegs;
    LONG			*lod_GRegs;
    UBYTE			*lod_ColorTable;
    UBYTE			*lod_ColorTable2;
    UBYTE			*lod_Allocated;
    ULONG			*lod_Histogram;		/* Histogram */
    ULONG			*lod_Histogram2;	/* Sorted histogram */
    WORD			 lod_NumColors;
    WORD			 lod_NumAlloc;		/* Number of colors allocated */
    ULONG			 lod_HistoSize;		/* Size of histogram */
    struct ColorMap		*lod_ColorMap;		/* ColorMap that we attached to */
    ULONG			 lod_Flags;

    UWORD			 lod_NumFrames;		/* Number of frames in animation */
    UWORD			 lod_CurFrame;		/* Current frame */
    LONG			 lod_OTopVert;
    LONG			 lod_OTopHoriz;
    WORD			 lod_AX, lod_AY;	/* Anchor point */

    /*************************/
    /* Animation information */
    /*************************/

    LONG			*lod_DeltaBuffer;	/* Delta buffer */
    WORD			*lod_YTable;		/* Y table */
    struct MsgPort		*lod_TimerPort;		/* Timer port for the animation */
    struct timerequest		*lod_TimerIO;		/* IO request for the animation */
    struct timeval		 lod_TimeVal;		/* Used to request the time tick */

    /*******************************/
    /* Control process information */
    /*******************************/

    struct StartupMsg		 lod_Message;		/* Startup message */
    struct Process		*lod_Process;		/* This is the process that handles the object */
    struct MsgPort		*lod_SIPCPort;		/* Incoming message port */
    BOOL			 lod_Going;		/* Are we running? */

    /***********************/
    /* Display information */
    /***********************/

    struct Window		*lod_Window;		/* Cached window */
    struct Requester		*lod_Requester;		/* Cached requester */
    struct RastPort		 lod_RPort;		/* Embedded RastPort */
    struct RastPort		 lod_Render;
    struct RastPort		 lod_Clear;
};

/* private flags */
#define	LODF_RUNNING	(1L<<0)
#define	LODF_DISPOSE	(1L<<1)
#define	LODF_SELECTED	(1L<<2)
#define	LODF_REMAP	(1L<<3)
#define	LODF_ATTACHED	(1L<<4)

/*****************************************************************************/

#define	MAXCOLORS		256
#define	SQ(x)			((x)*(x))
#define	AVGC(i1,i2,c)		((ir->ir_GRegs[i1][c]>>1)+(ir->ir_GRegs[i2][c]>>1))
#define VANILLA_COPY		0xC0
#define NO_MASK			0xFF
#define	MAXSRCPLANES		24
#define	BPR(w)			((w) + 15 >> 4 << 1)
#define MaxPackedSize(rowSize)  ((rowSize) + (((rowSize)+127) >> 7 ))
#define	RowBytes(w)		((((w) + 15) >> 4) << 1)
#define	ChunkMoreBytes(cn)	(cn->cn_Size - cn->cn_Scan)
#define UGetByte()		(*source++)
#define UPutByte(c)		(*dest++ = (c))

/*****************************************************************************/

#define	PROC_NAME	"AnimationDaemon"
