/* agdata.h - data required for AmigaGuide
 *
 *
 */

/*****************************************************************************/

#define	MAX_FONTS	30
#define	MAX_TABS	40

/*****************************************************************************/

#define	MASTER_HELP	"sys/amigaguide.guide"
#define	MASTER_HELPNODE	"sys/amigaguide.guide/Main"
#define	MAIN_NODE	"Main"

/*****************************************************************************/

/* Text attributes */
#define	TDTA_Buffer		(DTA_Dummy + 300)
#define	TDTA_BufferLen		(DTA_Dummy + 301)
#define	TDTA_LineList		(DTA_Dummy + 302)
#define	TDTA_WordSelect		(DTA_Dummy + 303)
#define	TDTA_WordDelim		(DTA_Dummy + 304)
#define	TDTA_WordWrap		(DTA_Dummy + 305)
     /* Boolean. Should the text be word wrapped.  Defaults to false. */

/*****************************************************************************/

/* There is one Line structure for every line of text in our fictional document. */
struct Line
{
    struct MinNode	 ln_Link;	/* to link the lines together */
    STRPTR		 ln_Text;	/* pointer to the text for this line */
    ULONG		 ln_TextLen;	/* the length of the text for this line */
    ULONG		 ln_Flags;	/* Control flags */
    struct IBox		 ln_Box;	/* Where the text is */
    ULONG		 ln_Style;	/* Text style */
    UBYTE		 ln_FgPen;	/* Foreground pen */
    UBYTE		 ln_BgPen;	/* Background pen */
    UWORD		 ln_Baseline;	/* The baseline for this line */
    struct TextFont	*ln_Font;	/* The font to use */
    APTR		 ln_Data;	/* Link data */
    LONG		 ln_LinkNum;	/* Link number */
};

#define	LNF_LF		0x00000001
    /* Line feed */

#define	LNF_LINK	0x00000002
    /* Segment is a link */

#define	LNF_OBJECT	0x00000004
    /* ln_Data is a pointer to a DataTypes object */

#define	LNF_SELECTED	0x00000008
    /* Object is selected */

#define	LNF_IMAGE	0x00000010
    /* ln_Text points to a struct Image */

#define	LNF_TEMPLATE	0x00000020
    /* ln_Text points to a struct Template */

#define	LNF_FANCY	0x00000040
    /* ln_Text points to char *, but gets drawn drop-shadowed */

#define	LNF_SINGLEBAR	0x00010000
#define	LNF_DBLBAR	0x00020000

/*****************************************************************************/

struct Keyword
{
    STRPTR	word;
    LONG	id;
};

#define NOMATCH         -1

/*****************************************************************************/

/* IFF types that may be text */
#define	ID_FTXT		MAKE_ID('F','T','X','T')
#define	ID_CHRS		MAKE_ID('C','H','R','S')

/*****************************************************************************/

struct StartupMsg
{
    struct Message	 sm_Message;		/* Embedded message */
    struct AGLib	*sm_AG;			/* Pointer to the library base */
    Class		*sm_Class;		/* Pointer to the class */
    Object		*sm_Object;		/* Pointer to the object */
    ULONG		 sm_Return;		/* Return value */
};

/*****************************************************************************/

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

struct History
{
    struct Node			 hs_Node;		/* Node in the history list */
    STRPTR			 hs_Name;		/* Complete node name */
    Object			*hs_Object;		/* The node */
    LONG			 hs_CurLine;		/* Current line */
    LONG			 hs_CurColumn;		/* Current column */
    LONG			 hs_ActiveKey;		/* Ordinal number of active link */
    ULONG			 hs_ActiveLine;		/* Active line */
    ULONG			 hs_ActiveColumn;	/* Active column */
};

/*****************************************************************************/

struct Controls
{
    UWORD			 c_Left;		/* Left edge of button */
    UWORD			 c_TLeft;		/* Offset for text */
    UWORD			 c_Top;			/* Top of button */
    UWORD			 c_ID;			/* ID of button */
    STRPTR			 c_Label;		/* Label */
    ULONG			 c_LabelLen;
    ULONG			 c_Flags;		/* Disabled, etc. */
};

#define	C_CONTENTS	0
#define	C_INDEX		1
#define	C_HELP		2
#define	C_RETRACE	3
#define	C_BROWSE_PREV	4
#define	C_BROWSE_NEXT	5
#define	MAX_CONTROLS	6

#define	CF_DISABLED	0x0001

#define	STM_HELP	0xBAD

/*****************************************************************************/

struct ARexxContext
{
    struct MsgPort	*ARexxPort;			/* The port messages come in at... */
    LONG		 Outstanding;			/* The count of outstanding ARexx messages... */
    char		 PortName[24];			/* The port name goes here... */
    char		 ErrorName[28];			/* The name of the <base>.LASTERROR... */
    char		 Extension[8];			/* Default file name extension... */
};

#define	AREXXCONTEXT		struct ARexxContext *
#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*****************************************************************************/

#define	DBS_DOWN	0
#define	DBS_MOVE	1
#define	DBS_SCROLL	2
#define	DBS_UP		3
#define	DBS_RENDER	4

/*****************************************************************************/

/* Client information */
struct ClientData
{
    Object			*cd_Database;		/* Database object */
    Object			*cd_Glue;		/* Glue it all together */
    Object			*cd_Active;		/* Active object */
    struct Locale		*cd_Locale;		/* Locale used for this client */
    struct MinList		 cd_HistoryList;	/* Retrace list */
    struct MinList		 cd_XRefList;		/* Cross reference list */
    ULONG			 cd_Flags;		/* Access flags */
    ULONG			 cd_ErrorLevel;
    ULONG			 cd_ErrorNumber;
    ULONG			 cd_ErrorString;

    /*******************************/
    /* Control process information */
    /*******************************/

    struct StartupMsg		 cd_Message;		/* Startup message */
    struct Process		*cd_Process;		/* This is the process that handles the
							 * AmigaGuide object.  We have to do disk
							 * access, ARexx and system calls. */
    struct MsgPort		*cd_SIPCPort;		/* Incoming message port */
    BOOL			 cd_Going;		/* Are we running? */
    struct ARexxContext		*cd_ARexxContext;

    /****************************/
    /* Current node information */
    /****************************/

    Object			*cd_CurrentDatabase;	/* Current Database */
    Object			*cd_CurrentNode;	/* Current Node */
    UBYTE			 cd_WorkName[512];
    UBYTE			 cd_WorkPath[256];
    UBYTE			 cd_WorkNode[128];

    /* Misc fields */
    BOOL			 cd_InSequence;
    BYTE			 cd_CharsInBuffer;
    BYTE			 cd_NumGoodChars;
    STRPTR			 cd_ARexxPortName;
    BYTE			 cd_ScanStep;
    SHORT			 cd_WhichANSI;

    /* Current information */
    LONG			 cd_TopVert;
    LONG			 cd_VisVert;
    LONG			 cd_TotVert;
    LONG			 cd_OTopVert;
    LONG			 cd_TopHoriz;
    LONG			 cd_VisHoriz;
    LONG			 cd_TotHoriz;
    LONG			 cd_OTopHoriz;

    /* Display information */
    Object			*cd_Frame;		/* Frame image */
    struct TextAttr		*cd_TextAttr;
    struct TextFont		*cd_Font;
    void			*cd_Pool;		/* Line pool */
    struct MinList		 cd_LineList;
    struct Line			*cd_CurLine;
    struct Line			*cd_ActiveLine;		/* Pointer to the active link */
    LONG			 cd_ActiveNum;		/* Number of the active link */
    struct IBox			 cd_CurBox;
    UWORD			 cd_CurX, cd_CurY;
    ULONG			 cd_UsefulHeight;
    ULONG			 cd_LineHeight;
    ULONG			 cd_ControlHeight;
    ULONG			 cd_ClientData;
    ULONG			 cd_HelpGroup;
    struct Window		*cd_Window;		/* Cached window */
    struct Requester		*cd_Requester;		/* Cached requester */
    struct GadgetInfo		 cd_GInfo;		/* Fake GadgetInfo */
    struct RastPort		 cd_RPort;		/* Embedded RastPort */
    struct RastPort		 cd_Control;		/* Control panel RastPort */
    struct RastPort		 cd_Render;		/* Rastport for text */
    struct RastPort		 cd_Clear;		/* Rastport for clearing */
    struct RastPort		 cd_Highlight;		/* Rastport for highlighting */

    struct TmpRas		 cd_TmpRas;
    PLANEPTR			 cd_TmpPlane;
    UWORD			 cd_TmpWidth, cd_TmpHeight;
    struct Region		*cd_Region;

    /******************/
    /* Word selection */
    /******************/
    UBYTE			*cd_WordDelim;		/* Word deliminators */
    LONG			 cd_SSec, cd_SMic;	/* Previous stamp */
    WORD			 cd_WX, cd_WY;		/* Current X, Y */
    WORD			 cd_OX, cd_OY;		/* Previous X, Y */
    ULONG			 cd_SWord;		/* Previous word */
    ULONG			 cd_Word;		/* Pointer to the word */
    UBYTE			 cd_WordBuffer[128];	/* Buffer containing the word that was selected */
    ULONG			 cd_AWord;		/* Anchor buffer pointer */
    ULONG			 cd_EWord;		/* End buffer pointer */
    struct IBox			 cd_Select;		/* Current select box */
    BYTE			*cd_Selected;

    /*****************/
    /* Control Panel */
    /*****************/
    ULONG			 cd_BoxWidth;		/* Width of menu strip buttons */
    ULONG			 cd_Top;		/* Top of document view area */
    ULONG			 cd_Height;		/* Height of document view area */
    struct Controls		 cd_Controls[MAX_CONTROLS];
};

#define	cd_DrawInfo	cd_GInfo.gi_DrInfo

#define	AGF_RETRACE	(1L<<0)
#define	AGF_RUNNING	(1L<<1)
#define	AGF_DISPOSE	(1L<<2)
#define	AGF_RENDER	(1L<<3)
#define	AGF_SYNC	(1L<<4)
#define	AGF_LAYOUT	(1L<<5)
#define	AGF_LAYOUTEMBED	(1L<<6)

#define	AGF_HIGHLIGHT	(1L<<7)
#define	AGF_WORDSELECT	(1L<<8)
#define	AGF_CHANGED	(1L<<9)

#define	AGF_SELECTED	(1L<<10)
#define	AGF_CONTROL	(1L<<11)

#define	AGF_NEWSIZE	(1L<<12)
#define	AGF_BUSY	(1L<<13)

#define	AGF_COERCED	(1L<<14)

#define	AGF_PRINTING	(1L<<15)
#define	AGF_SAVING	(1L<<16)
#define	AGF_CONTROLS	(1L<<17)

#define	PROC_NAME	"AmigaGuide® (amigaguide.datatype)"


/*****************************************************************************/

struct Embed
{
    struct Node			 e_Node;		/* Label is in e_Node.ln_Name */
    Object			*e_Object;		/* The object */
    struct IBox			 e_Domain;		/* Where to place the object */
    LONG			 e_Line;		/* Line embedded at */
};

/*****************************************************************************/

/* Node information */
struct NodeData
{
    Object			*nd_Database;		/* Database object */
    Object			*nd_Parent;		/* Parent HNData */
    Object			*nd_Object;		/* The node itself */
    struct SignalSemaphore	 nd_Lock;		/* Exclusive access */
#if 1
#define	ND_STRINGS	8
    STRPTR			 nd_Strings[ND_STRINGS];
#define	nd_Name		nd_Strings[0]
#define	nd_Title	nd_Strings[1]
#define	nd_TOC		nd_Strings[2]
#define	nd_Prev		nd_Strings[3]
#define	nd_Next		nd_Strings[4]
#define	nd_Keywords	nd_Strings[5]
#define	nd_OnOpen	nd_Strings[6]
#define	nd_OnClose	nd_Strings[7]
#else
    STRPTR			 nd_Name;		/* Name of node */
    STRPTR			 nd_Title;		/* Title for help node */
    STRPTR			 nd_TOC;		/* Name of Table of Contents */
    STRPTR			 nd_Prev;		/* Browse backwards node */
    STRPTR			 nd_Next;		/* Browse forwards node */
    STRPTR			 nd_Keywords;		/* Keywords */
    STRPTR			 nd_OnOpen;		/* On open command */
    STRPTR			 nd_OnClose;		/* On close command */
#endif

    BPTR			 nd_FileHandle;		/* File handle */
    LONG			 nd_Offset;		/* Node offset in file */
    LONG			 nd_Length;		/* Length of node */
    ULONG			 nd_Flags;		/* Node flags */
    ULONG			 nd_UseCnt;		/* Number of users in node */
    ULONG			 nd_TotVert;		/* Number of lines */
    STRPTR			 nd_Buffer;		/* Raw node data */
    ULONG			 nd_BufferLen;		/* Length of raw data */
    struct opNodeIO		 nd_ONM;		/* Host IO message */

    /* Layout information */
    UBYTE			 nd_FontName[34];	/* Font name */
    struct TextAttr		 nd_TextAttr;
    struct TextFont		*nd_Font;
    LONG			 nd_DefFont;		/* Default font number */
    UWORD			 nd_Tabs[MAX_TABS];	/* Tab table */
    LONG			 nd_DefTabs;		/* Default tab spacing */
    LONG			 nd_MinWidth;		/* Minimum width in characters */
};

/*****************************************************************************/

#define	HNF_WRAP		(1L << 1)
#define	HNF_PROPORTIONAL	(1L << 2)
#define	HNF_FREEBUFFER		(1L << 4)
#define	HNF_SMARTWRAP		(1L << 5)

/*****************************************************************************/

#define	HNA_Dummy		(TAG_USER + 500)
#define	HNA_Database		(HNA_Dummy + 1)
#define	HNA_Parent		(HNA_Dummy + 2)
#define	HNA_FileHandle		(HNA_Dummy + 3)
#define	HNA_Offset		(HNA_Dummy + 4)
#define	HNA_Length		(HNA_Dummy + 5)
#define	HNA_Object		(HNA_Dummy + 6)
#define	HNA_EmbedName		(HNA_Dummy + 7)
#define	HNA_EmbedLine		(HNA_Dummy + 8)
#define	HNA_EmbedID		(HNA_Dummy + 9)
#define	HNA_HostHandle		(HNA_Dummy + 10)
#define	HNA_SIPCPort		(HNA_Dummy + 11)

#define	HNA_Name		(HNA_Dummy + 20)
#define	HNA_Title		(HNA_Dummy + 21)
#define	HNA_TOC			(HNA_Dummy + 22)
#define	HNA_Previous		(HNA_Dummy + 23)
#define	HNA_Next		(HNA_Dummy + 24)
#define	HNA_Keywords		(HNA_Dummy + 25)
#define	HNA_OnOpen		(HNA_Dummy + 26)
#define	HNA_OnClose		(HNA_Dummy + 27)

/*****************************************************************************/

#define	ATTR_UNDERLINE	1
#define	ATTR_HIGHLIGHT	2
#define ATTR_BLINK	4
#define	ATTR_INVERSE	8

/*****************************************************************************/

#define	BACKSLASH	'\\'
#define	SCURLY		'{'
#define	ECURLY		'}'
#define	QUOTE		34
#define	SQUOTE		39
#define isspace(c)	((c==' ')||(c=='\t')||(c=='\n')||(c==','))
#define	EVEN(a)		((a%2)?0:1)

/*****************************************************************************/
#if 0
/* Callback handle */
struct AmigaGuideHost
{
    struct Hook			 agh_Dispatcher;	/* Dispatcher */
    ULONG			 agh_Reserved;		/* Must be 0 */
    ULONG			 agh_Flags;
    ULONG			 agh_UseCnt;		/* Number of open nodes */
    APTR			 agh_SystemData;	/* Reserved for system use */
    APTR			 agh_UserData;		/* Anything you want... */
};
#endif
/*****************************************************************************/

/* Database information */
struct DatabaseData
{
    struct SignalSemaphore	 dbd_Lock;		/* Exclusive access */
    BPTR			 dbd_FileLock;		/* Lock on the file */
    BPTR			 dbd_FileHandle;	/* File handle */

#if 1
#define	DBD_STRINGS	5
    STRPTR			 dbd_Strings[DBD_STRINGS];
#define	dbd_Name	dbd_Strings[0]
#define	dbd_Index	dbd_Strings[1]
#define	dbd_Help	dbd_Strings[2]
#define	dbd_OnOpen	dbd_Strings[3]
#define	dbd_OnClose	dbd_Strings[4]
#else
    STRPTR			 dbd_Name;		/* Name of the database */
    STRPTR			 dbd_Index;		/* Name of index document/node */
    STRPTR			 dbd_Help;		/* Name of help document/node */
    STRPTR			 dbd_OnOpen;		/* On open command */
    STRPTR			 dbd_OnClose;		/* On close command */
#endif

    ULONG			 dbd_Flags;
    ULONG			 dbd_UseCnt;		/* Usage count */
    struct MinList		 dbd_NodeList;		/* List of nodes */
    struct MinList		 dbd_XRefList;		/* XRef list */
    ULONG			 dbd_NumNodes;		/* Number of nodes in database */
    ULONG			 dbd_NumEndNodes;	/* Number of endnodes in database */
    ULONG			 dbd_NumLines;		/* Number of lines in database */
    UWORD			 dbd_Width;		/* Width in characters */
    UWORD			 dbd_Height;		/* Height in characters */
    struct HostHandle		*dbd_HH;		/* Pointer to AmigaGuideHost */
    ULONG			 dbd_Offset;		/* Working offset */
    Object			*dbd_CurrentNode;	/* Working node */

    /* Debugging information */
    UBYTE			 dbd_FullName[300];	/* Complete name of the file */
    UBYTE			 dbd_ErrorName[300];	/* Where to put the error message(s) */
    UBYTE			 dbd_ErrorLine[512];	/* Where to put the error message(s) */
    BPTR			 dbd_ErrorFH;		/* Error file handle */
    ULONG			 dbd_Errors;		/* Number of errors */
    ULONG			 dbd_Warnings;		/* Number of warnings */

    /* Layout information */
    UBYTE			 dbd_FontName[34];	/* Font name */
    struct TextAttr		 dbd_TextAttr;
    struct TextFont		*dbd_Font;
    struct TextFont		*dbd_Fonts[MAX_FONTS];	/* Font table */
    struct TextAttr		 dbd_TAttr[MAX_FONTS];	/* TextAttr's for font table */
    LONG			 dbd_DefFont;		/* Default font number */
    UWORD			 dbd_Tabs[MAX_TABS];	/* Tab table */
    LONG			 dbd_DefTabs;		/* Default tab spacing */
    LONG			 dbd_MinWidth;
    struct MinList		 dbd_MacroList;		/* Macro list */
};

/* This flag is set if the object is an AmigaGuide object */
#define	DBDF_AMIGAGUIDE		(1L<<0)

/* They can handle word wrapping */
#define	DBDF_WRAP		(1L<<1)

/* They can handle a proportional font */
#define	DBDF_PROPORTIONAL	(1L<<2)

/* They need a layer */
#define	DBDF_LAYER		(1L<<3)

/* This flag is set if the object is an Node Host */
#define	DBDF_NODEHOST		(1L<<4)

/* This flag is set if the master is newer than the guide file */
#define	DBDF_STALE		(1L<<5)

/* This flag is set if debugging is turned on */
#define	DBDF_DEBUG		(1L<<6)

/* They can handle word wrapping */
#define	DBDF_SMARTWRAP		(1L<<7)

/*****************************************************************************/

#define	DBA_Dummy		(TAG_USER + 600)
#define	DBA_FileHandle		(DBA_Dummy + 1)
#define	DBA_HostHandle		(DBA_Dummy + 3)
#define	DBA_ClientData		(DBA_Dummy + 4)
#define	DBA_Class		(DBA_Dummy + 5)
#define	DBA_Object		(DBA_Dummy + 6)
#define	DBA_EmbeddedObject	(DBA_Dummy + 7)

#define	DBA_XRef		(DBA_Dummy + 19)
#define	DBA_Name		(DBA_Dummy + 20)
#define	DBA_Index		(DBA_Dummy + 21)
#define	DBA_Help		(DBA_Dummy + 22)
#define	DBA_OnOpen		(DBA_Dummy + 23)
#define	DBA_OnClose		(DBA_Dummy + 24)

/*****************************************************************************/

#define	MAXARG		64

/*****************************************************************************/

struct MacroNode
{
    struct Node		 mn_Node;
    STRPTR		 mn_Macro;
};

/*****************************************************************************/

struct WorkLayout
{
    struct IBox		 wl_View;		/* Current domain to layout to */
    struct TextFont	*wl_Font;		/* Current font */
    ULONG		 wl_Style;		/* Current style */
    UBYTE		 wl_FgPen;		/* Current foreground pen */
    UBYTE		 wl_BgPen;		/* Current background pen */
    UBYTE		 wl_Justify;		/* Current justification */
    UBYTE		 wl_Pad1;		/* Padding */

    BOOL		 wl_LineFeed;
    BOOL		 wl_NewSeg;
    BOOL		 wl_InAttr;
    BOOL		 wl_InCmnd;
    BOOL		 wl_InQuote;		/* Are we inside quotes? */
    BOOL		 wl_InLink;		/* Are we in a link command */
    BOOL		 wl_DefWrap;
    BOOL		 wl_DefSmartWrap;	/* Default smart wrap */
    BOOL		 wl_Wrap;		/* Old style word wrap */
    BOOL		 wl_SmartWrap;		/* New style word wrap */
    BOOL		 wl_Text;

    ULONG		 wl_XOffset;
    ULONG		 wl_YOffset;
    ULONG		 wl_ParXAdjust;		/* Paragraph adjust */
    ULONG		 wl_XAdjust;
    ULONG		 wl_YAdjust;

    LONG		 wl_PicLeft;
    LONG		 wl_PicRight;
    LONG		 wl_PicHeight;

    ULONG		 wl_NumLines;
    ULONG		 wl_NumColumns;
    ULONG		 wl_Baseline;
    ULONG		 wl_BelowBase;
    ULONG		 wl_LineHeight;
    ULONG		 wl_Num;
    ULONG		 wl_NumTabs;

    LONG		 wl_LeftMargin;
    LONG		 wl_RightMargin;
    LONG		 wl_ParIndent;
    LONG		 wl_LinkNum;

    ULONG		 wl_TabSpace;
    ULONG		 wl_SpaceWidth;

    struct Line		*wl_AnchorLine;		/* Anchor line */
    struct Line		*wl_Line;

    /* Can't use these yet... */
    struct Line		*wl_SpaceLine;		/* Space line */
    LONG		 wl_LastSpace;		/* Last space */

    struct RastPort	 wl_RPort;		/* Temporary RastPort */
    UBYTE		 wl_CmdBuffer[300];
    UBYTE		 wl_ArgBuffer[300];
};

/*****************************************************************************/

#define	WPI_LEFT	0
#define	WPI_LINE	1
#define	WPI_RIGHT	2

/*****************************************************************************/

#define	JLEFT		0
#define	JCENTER		1
#define	JRIGHT		2

