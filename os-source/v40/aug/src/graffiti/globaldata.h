/*****************************************************************************/

#define	TEMPLATE	"SERVER/K,REALM/K,PUBSCREEN/K,WIDTH/K/N,HEIGHT/K/N,DEPTH/K/N"
#define	OPT_SERVER	0
#define	OPT_REALM	1
#define	OPT_PUBSCREEN	2
#define	OPT_WIDTH	3
#define	OPT_HEIGHT	4
#define	OPT_DEPTH	5
#define	NUM_OPTS	6

/*****************************************************************************/

#define	GAD_CONTEXT	0
#define	GAD_ANCHOR	1
#define	GAD_PALETTE	2
#define	GAD_UNDO	3
#define	GAD_CLEAR	4
#define	GAD_FAST	5
#define	GAD_SLOW	6
#define	GAD_LINE	7
#define	GAD_ELLIPSE	8
#define	GAD_RECTANGLE	9
#define	GAD_TEXT	10
#define	GAD_BRUSH	11
#define	MAX_GAD		12

/*****************************************************************************/

#define	BUFFLEN		512

/*****************************************************************************/

struct GlobalData
{
    /* Shared Libraries */
    struct Library		*gd_SysBase;
    struct Library		*gd_AslBase;
    struct Library		*gd_DataTypesBase;
    struct Library		*gd_DOSBase;
    struct Library		*gd_GadToolsBase;
    struct Library		*gd_GfxBase;
    struct Library		*gd_IconBase;
    struct Library		*gd_IntuitionBase;
    struct Library		*gd_UtilityBase;
    struct Library		*gd_WorkbenchBase;
    struct Library		*gd_LayersBase;
    struct Library		*gd_NIPCBase;

    /* Control information */
    struct Process		*gd_Process;
    LONG			 gd_ErrorLevel;
    LONG			 gd_ErrorNumber;
    LONG			 gd_ErrorString;
    LONG			 gd_Going;
    struct RdArgs		*gd_RDArgs;
    ULONG			 gd_Options[NUM_OPTS];

    /* NIPC Information */
    struct Entity		*gd_SEntity;
    struct Entity		*gd_HEntity;
    ULONG			 gd_BitNumber;
    UBYTE			 gd_Message[512];
    UBYTE			 gd_Server[64];		/* Name of the server */
    UBYTE			 gd_ServerRealm[64];	/* Name of server realm */
    UBYTE			 gd_ServerHost[64];	/* Name of server host */
    UBYTE			 gd_User[64];		/* Name of user */
    UBYTE			 gd_Host[64];		/* Name of host */
    UBYTE			 gd_HostRealm[64];	/* Name of host realm */
    struct List			 gd_ClientList;		/* List of clients */
    ULONG			 gd_NumClients;		/* Number of clients */

    /* Talk information */
    struct MsgPort		*gd_CPort;		/* Console message port */
    BPTR			 gd_Console;		/* Console handle */
    struct Window		*gd_ConsoleWindow;	/* Console window pointer */
    struct IBox			 gd_ConsoleMemory;
    BPTR			 gd_Output;		/* Output console */
    struct Window		*gd_OutputWindow;	/* Output window pointer */
    struct IBox			 gd_OutputMemory;
    struct StandardPacket	*gd_CPacket;		/* DOS Packet */
    UBYTE			 gd_CPrompt[64];	/* Prompt */
    UBYTE			 gd_CBuff[BUFFLEN];	/* Input buffer */
    UBYTE			*gd_CSBuff;		/* Send buffer */
    ULONG			 gd_CPromptLen;
    BPTR			 gd_OCIS;
    BPTR			 gd_OCOS;
    APTR			 gd_ConsoleTask;
    ULONG			 gd_CStatus;
    BOOL			 gd_PacketOut;

    /* View Window information */
    UBYTE			 gd_ScreenNameBuffer[MAXPUBSCREENNAME+1];
    STRPTR			 gd_ScreenName;
    struct Screen		*gd_Screen;
    Class			*gd_WinClass;
    struct Window		*gd_Window;
    struct DrawInfo		*gd_DrInfo;
    VOID			*gd_VI;
    struct MsgPort		*gd_IDCMP;
    struct Menu			*gd_Menu;		/* The menus */
    struct Region		*gd_Region;		/* Region */
    struct Region		*gd_ORegion;		/* Previous region */

    /* Application data */
    struct MsgPort		*gd_WBPort;		/* Workbench-related message port */
    struct AppWindow		*gd_AppWindow;
    struct AppIcon		*gd_AppIcon;
    struct FileRequester	*gd_FR;			/* File requester */
    struct Gadget		*gd_Gadgets[MAX_GAD];	/* Our gadgets */
    ULONG			 gd_HTop;		/* Horizontal top of the data */
    ULONG			 gd_VTop;		/* Vertical top of the data */
    ULONG			 gd_InnerWidth;		/* Inside width of the window */
    ULONG			 gd_InnerHeight;	/* Inside height of the window */
    ULONG			 gd_Flags;

    /* Drawing information */
    ULONG			 gd_Width;		/* Width of the bitmap */
    ULONG			 gd_Height;		/* Height of the bitmap */
    ULONG			 gd_Depth;		/* Depth of the bitmap */
    struct BitMap		*gd_BitMap;		/* BitMap that we're going to scroll around */
    struct RastPort		 gd_RPort;		/* Our RastPort */
    struct AreaInfo		 gd_AreaInfo;
    struct TmpRas		 gd_TmpRas;
    WORD			*gd_AreaBuffer;

    /* Don't change... */
    UBYTE			 gd_FGPen;
    UBYTE			 gd_BGPen;
    UBYTE			 gd_DrMode;
    UBYTE			 gd_Tool;
    ULONG			 gd_NumPlot;
    struct Plot			 gd_Plot[1024];
};

/*****************************************************************************/

#define	GDT_SLOW	0
#define	GDT_FAST	1
#define	GDT_LINE	2
#define	GDT_ELLIPSE	3
#define	GDT_RECTANGLE	4
#define	GDT_FILL	5

/*****************************************************************************/

#define	AslBase		 gd->gd_AslBase
#define	DataTypesBase	 gd->gd_DataTypesBase
#define	DOSBase		 gd->gd_DOSBase
#define	GadToolsBase	 gd->gd_GadToolsBase
#define	GfxBase		 gd->gd_GfxBase
#define	IconBase	 gd->gd_IconBase
#define	IntuitionBase	 gd->gd_IntuitionBase
#define	LayersBase	 gd->gd_LayersBase
#define	UtilityBase	 gd->gd_UtilityBase
#define	WorkbenchBase	 gd->gd_WorkbenchBase
#define	NIPCBase	 gd->gd_NIPCBase

/*****************************************************************************/

#define	GDF_SERVER	(1L<<0)
    /* We are the server */

#define	GDF_DRAWING	(1L<<1)
    /* Show that we are drawing */

