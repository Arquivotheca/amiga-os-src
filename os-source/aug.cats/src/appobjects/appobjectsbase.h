/* appobjectsbase.h
 *
 * $Id
 *
 * $Log
 *
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/semaphores.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/cghooks.h>
#include <intuition/sghooks.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/classusr.h>
#include <intuition/classes.h>
#include <intuition/icclass.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/layers.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <utility/hooks.h>
#include <string.h>
#include <ctype.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/console_protos.h>
#include <clib/utility_protos.h>
#include <clib/layers_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/layers_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#include "appobjects_lib.h"
#include "appobjects_pragmas.h"

/* Locks that we use */
#define	CGTL_GLOBAL	0
#define	CGTL_CLIP	1

/* Room for growth */
#define	CGT_MAX_LOCKS	5

/* Classes that we use */
#define	CGTC_AFRAMEI	0
#define	CGTC_MTEXTG	1
#define	CGTC_MTMODEL	2
#define	CGTC_MTGROUP	3
#define	CGTC_ACTION	4
#define	CGTC_LGROUPG	5
#define	CGTC_SLIDER	6
#define	CGTC_GADGET	7
#define	CGTC_TEXTI	8
#define CGTC_LABELG	9
#define	CGTC_APPSYSI	10
#define	CGTC_SCROLLER	11
#define	CGTC_MUTEX	12
#define	CGTC_COLUMNI	13
#define	CGTC_VIEWG	14
#define	CGTC_LISTVIEWG	15

/* Room for growth */
#define	CGT_MAX_CLASSES	20

#define SCALE(a,b,c)	( ((b) * (c)) / (a) )

/* This is the good scale */
#define	OSCALE(a,b,c)	( (((a) / 2) + ((b) * (c))) / (a) )

#define	DEF_WIDTH 8L
#define	DEF_HEIGHT 8L
#define	DEF_BASE 6L

/* global variables required for library */
struct LibBase
{
    /* Exclusive access */
    struct SignalSemaphore lb_Lock[CGT_MAX_LOCKS];

    /* put any other globals here */
    Class *lb_Classes[CGT_MAX_CLASSES];

    ULONG lb_Pad;
};

/* Temporary work data, cut back on stack useage */
struct WorkData
{
    struct ObjectInfo *pwd_OI;	/* Current ObjectInfo */
    struct ObjectNode *pwd_CON;	/* current object node */

    /* private GadTools work area structure pointers */
    struct Gadget *pwd_Gad;	/* gadget strip */
    struct NewGadget pwd_NewGadget;	/* NewGadget structure */

    /* private work area structure pointers */
    struct TagItem *pwd_CTI;	/* current tagitem */
    struct TagItem *pwd_NTI;	/* next tagitem */
    struct TagItem pwd_Inserts[20];	/* used for inserting tags */

    /* private work area embedded structures */
    struct IBox pwd_Work1;	/* work area for window size */
    struct IBox pwd_Work2;	/* work area for inner bounderies */
    struct ObjectNode pwd_EON;	/* object node */

    /* private work area variables */
    LONG pwd_swidth;		/* source font width */
    LONG pwd_sheight;		/* source font height */
    LONG pwd_sbase;		/* source font baseline */
    LONG pwd_fwidth;		/* Font width */
    LONG pwd_fheight;		/* Font height */
    LONG pwd_fbase;		/* Font baseline */

    /* Window domain */
    struct IBox pwd_MinWin;	/* Minimum window size */
    struct IBox pwd_NomWin;	/* Nominal window size */
    struct IBox pwd_MaxWin;	/* Maximum window size */

    /* Group smarts */
    struct ObjectNode *pwd_MO;	/* Mutual exclusion object */
    struct ObjectNode *pwd_LV;	/* ListView header object */
    Object *pwd_Header;		/* Header object */
    struct List pwd_GroupList;	/* List of groups */
    WORD pwd_Level;		/* Maximum acceptable group level */
    WORD pwd_Groups;		/* Number of groups */
    WORD pwd_MaxLevel;		/* Maximum level used in object array */

    WORD pwd_WBorLeft;
    WORD pwd_WBorTop;
    WORD pwd_WBorRight;
    WORD pwd_WBorBottom;
};

struct Group
{
    struct Node gn_Node;	/* embedded node structure for group list */
    UBYTE gn_Name[16];		/* Name of group sprintf("%d",group) */
    struct Object *gn_Header;	/* Group header */
    struct IBox gn_Domain;	/* Group domain */
    WORD gn_Flags;		/* Flags */
    WORD gn_Level;		/* Maximum acceptable level */
    WORD gn_Members;		/* Number of members in group */
    WORD gn_MaxWidth;		/* Maximum member width */
    WORD gn_MaxHeight;		/* Maximum member height */
    WORD gn_VFill;		/* Vertical fill */
    WORD gn_HFill;		/* Horizontal fill */
    WORD gn_VMargin;		/* Vertical margin */
    WORD gn_HMargin;		/* Horizontal margin */
};

/* Use minimum size */
#define	GF_MINIMUM	(1<<0)

/* Shared libraries that are needed */
extern struct Library *SysBase;
extern struct Library *AppObjectsBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;
extern struct Library *KeymapBase;
extern struct Library *UtilityBase;
extern struct Library *LayersBase;
extern struct Library *GadToolsBase;
extern struct Library *IFFParseBase;

struct ExtLibrary
{
    struct Library el_Lib;
    LONG el_SegList;
    struct LibBase *el_Base;
};

/* classface.asm prototypes */
ULONG DoMethod (Object * o, ULONG methodID,...);
ULONG DoSuperMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CoerceMethod (Class * cl, Object * o, ULONG methodID,...);
ULONG CM (Class * cl, Object * o, Msg msg);
ULONG DM (Object * o, Msg msg);
ULONG DSM (Class * cl, Object * o, Msg msg);
ULONG SetSuperAttrs (Class * cl, Object * o, ULONG data,...);
ULONG notifyAttrChanges (Object * o, VOID * ginfo, ULONG flags, ULONG tag1,...);

/* misc.c */
UWORD GetLabelKeystroke ( STRPTR label );
ULONG notifyAttrChanges ( Object *o , VOID *ginfo , ULONG flags , ULONG tag1 , ...);
ULONG PointInBox ( struct IBox *point , struct IBox *box );
BOOL hitGadget ( struct Gadget *g , struct GadgetInfo *gi );
LONG GetVisualState ( struct Gadget *g , struct GadgetInfo *gi );
WORD aTextExtent ( struct RastPort *rp , STRPTR string , LONG count , struct TextExtent *te );
UWORD aText ( struct RastPort *rp , STRPTR label , WORD left , WORD top );
UWORD printIText ( struct RastPort *rp , struct IntuiText *itext , WORD left , WORD top , WORD pens );
VOID gadgetBox ( struct Gadget *g , struct IBox *domain , struct IBox *box , struct IBox *cbox , ULONG flags );
VOID computeDomain ( Class *cl , Object *o , struct gpRender *gpr , struct IBox *box , struct IBox *cbox , ULONG flags );
WORD clamp ( WORD min , WORD cur , WORD max );
VOID drawImage ( struct RastPort *rp , struct Image *im , WORD left , WORD top , LONG state , struct DrawInfo *drinfo );
VOID offsetRect ( struct Rectangle *r , int dx , int dy );
VOID rectHull ( struct Rectangle *big , struct Rectangle *r2 );
struct IBox *rectToBox ( struct IBox *rect , struct IBox *box );
struct TextFont *ISetFont ( struct RastPort *rp , struct TextAttr *font );
VOID ICloseFont ( struct TextFont *fp );
int RastPortITextExtent ( struct RastPort *rp , struct IntuiText *itext , struct TextExtent *te );
int ITextLayout ( struct RastPort *rp , struct IntuiText *it , int numitext , struct IBox *box , BOOL do_layout , int xoffset , int yoffset );
VOID FreeExecList ( struct List *list , struct Hook *hook );
VOID PrepTag ( struct TagItem *tag , ULONG label , LONG data );
struct TagItem *MakeNewTagList ( ULONG data , ...);
LONG BreakString(STRPTR text, STRPTR *array, LONG maxstrings);
struct GadgetInfo *GetGadgetInfo (VOID *msg);
BOOL compareRect (struct IBox * b1, struct IBox * b2);
VOID GhostRectangle (Object *o, struct gpRender *gpr, struct IBox *b);
BOOL CompareRect (struct Rectangle *r1, struct Rectangle *r2);

