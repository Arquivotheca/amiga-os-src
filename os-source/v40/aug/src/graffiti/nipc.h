/* nipc.h
 *
 */

/*****************************************************************************/

struct Client
{
    struct Node c_Node;
    UBYTE c_Unique[16];
    UBYTE c_Name[128];		/* Name of the host */
    struct Entity *c_Entity;
};

/*****************************************************************************/

#define	CMD_LOGIN	0
#define	CMD_LOGOUT	1
#define	CMD_MESSAGE	2
#define	CMD_BITMAP	3
#define	CMD_PLOTPOINTS	4
#define	CMD_CLEAR	5
#define	CMD_TALK	6

/*****************************************************************************/

struct Dimensions
{
    ULONG	d_Width;
    ULONG	d_Height;
};

/*****************************************************************************/

struct PlotPoints
{
    UBYTE		 pp_FGPen;
    UBYTE		 pp_BGPen;
    UBYTE		 pp_DrMode;
    UBYTE		 pp_Tool;
    ULONG		 pp_NumPlot;
    struct Plot		 pp_Plot[1024];
};

/*****************************************************************************/

struct PlotPointsHeader
{
    UBYTE		 pp_FGPen;
    UBYTE		 pp_BGPen;
    UBYTE		 pp_DrMode;
    UBYTE		 pp_Tool;
    ULONG		 pp_NumPlot;
};

