/* classdata.h
 * Animation class data
 *
 */

/*****************************************************************************/

/* Used to send DTM_TRIGGER messages */
struct TriggerMsg
{
    struct Message		 tm_Message;
    ULONG			 tm_Function;
    APTR			 tm_Data;
};

#define	ASTM_LAYOUT		0xFFFF

/*****************************************************************************/

struct StartupMsg
{
    struct Message		 sm_Message;		/* Embedded message */
    struct ClassBase		*sm_CB;			/* Pointer to the library base */
    Class			*sm_Class;		/* Pointer to the class */
    Object			*sm_Object;		/* Pointer to the object */
};

/*****************************************************************************/

struct FrameNode
{
    struct Node			 fn_Node;		/* Embedded node */
    struct adtFrame		 fn_Frame;		/* Embedded frame message */
};

/*****************************************************************************/

struct localData
{
    struct Process		*lod_DispProc;
    struct StartupMsg		 lod_DispMsg;

    struct Process		*lod_LoadProc;
    struct StartupMsg		 lod_LoadMsg;

    ULONG			 lod_Flags;

    /* RealTime information */
    struct ClassBase		*lod_CB;
    struct Hook			 lod_Hook;
    struct Player		*lod_Player;
    struct Conductor		*lod_Conductor;
    ULONG			 lod_TicksPerFrame;
    ULONG			 lod_Ticks;
    ULONG			 lod_Frame;
    ULONG			 lod_FPS;		/* Frames per second */
    ULONG			 lod_ClockTime;

    /* Controller information */
    struct Gadget		*lod_Active;		/* The active gadget */
    struct Gadget		*lod_TapeDeck;
    UWORD			 lod_TapeX, lod_TapeY;	/* Tapedeck positioning */

    /* Color & remap information */
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

    /* Animation information */
    struct BitMapHeader		 lod_BMHD;
    struct Window		*lod_Window;		/* The window that the object is attached to */

    ULONG			 lod_ModeID;
    ULONG			 lod_Width;
    ULONG			 lod_Height;
    ULONG			 lod_BltWidth;		/* Width */
    ULONG			 lod_BltHeight;		/* Blit height */
    UBYTE			 lod_Depth;
    struct BitMap		*lod_KeyFrame;		/* Key, or first, frame */
    struct BitMap		*lod_BMap;		/* Current bitmap */
    struct BitMap		*lod_BMap1;		/* First bitmap */
    struct BitMap		*lod_BMap2;		/* Second bitmap */
    struct BitMap		*lod_BMap3;		/* Third bitmap */
    ULONG			 lod_Which;		/* Which bitmap */
    ULONG			 lod_NumFrames;		/* Number of frames */
    ULONG			 lod_Adjust;		/* Amount to add for Fast Forward */
    LONG			 lod_OTopVert;
    LONG			 lod_OTopHoriz;
    WORD			 lod_AX, lod_AY;	/* Anchor point */

    /* Sound Information */
    Object			*lod_SndObj;		/* Sound object */
    UBYTE			*lod_Sample;
    ULONG			 lod_SampleLength;
    ULONG			 lod_Period;
    ULONG			 lod_Volume;

    /* Control information */
    struct MsgPort		*lod_MP;		/* Control message port */
    struct List			 lod_FrameList;		/* List of frames, sorted by timestamp */
    struct List			 lod_DiscardList;	/* List of frames to discard */
    ULONG			 lod_InList;
    ULONG			 lod_OutList;

    struct SignalSemaphore	 lod_FLock;		/* 46: Access lock */
    UWORD			 lod_Pad2;		/* ALIGNMENT */
    struct SignalSemaphore	 lod_DLock;		/* 46: Access lock */
    UWORD			 lod_Pad3;		/* ALIGNMENT */
};

/*****************************************************************************/

#define	LODF_DRUNNING	(1L<<0)
#define	LODF_LRUNNING	(1L<<1)
#define	LODF_DGOING	(1L<<2)
#define	LODF_LGOING	(1L<<3)
#define	LODF_REMAP	(1L<<4)
#define	LODF_LOADING	(1L<<5)
#define	LODF_ATTACHED	(1L<<6)
#define	LODF_CONTROLS	(1L<<7)
#define	LODF_AUTOPLAY	(1L<<8)

#define	LODF_PLAYING	(1L<<10)
#define	LODF_PAUSE	(1L<<11)
#define	LODF_STOPPED	(1L<<12)
#define	LODF_LOCATE	(1L<<13)
#define	LODF_SHUTTLE	(1L<<14)
#define	LODF_ALL	(LODF_PLAYING|LODF_PAUSE|LODF_STOPPED|LODF_LOCATE|LODF_SHUTTLE)
#define	LODF_STATE	(1L<<15)

/*****************************************************************************/

void __stdargs NewList (struct List *);
void __stdargs sprintf (void *, ...);

/*****************************************************************************/

#define	G(o)		((struct Gadget *)o)
#define	DISP_PROC_NAME	"Animation_ObjectHandler (Display)"
#define	LOAD_PROC_NAME	"Animation_ObjectHandler (Load)"

