/* prefs_lib.h
 * preference library header
 *
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/notify.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <iff/ilbm.h>

/* GetPrefsDrawer() flags */
#define	PREFS_GLOBAL_GPDF	(1L<<0)
#define	PREFS_READONLY_GPDF	(1L<<1)
#define	PREFS_FALLBACK_GPDF	(1L<<2)

/* GetPref() tags */
/* fix these ... */
#define	PREFS_DEFAULT_A		(TAG_USER + 1L)
#define	PREFS_QUICKFAIL_A	(TAG_USER + 2L)
#define	PREFS_NAME_A		(TAG_USER + 3L)
#define	PREFS_NOTIFY_A		(TAG_USER + 4L)
#define	PREFS_CALLBACK_A	(TAG_USER + 5L)
#define	PREFS_FRESHEN_A		(TAG_USER + 6L)
#define	PREFS_LIST_A		(TAG_USER + 7L)
#define	PREFS_PRIVATE_1		(TAG_USER + 8L)
#define	PREFS_DEPTH		(TAG_USER + 9L)

/* Preference kinds */
#define PREFS_SCREENMODE	0
#define PREFS_PALETTE		1
#define PREFS_POINTER		2
#define PREFS_BUSYPOINTER	3
#define PREFS_WBCONFIG		4
#define PREFS_FONT		5
#define	PREFS_KINDS		6

/* Common header for each preference file */
struct Prefs
{
    struct Node p_Node;		/* Node in the list of preferences */
    LONG p_Kind;		/* Kind of preference file */
    ULONG p_Flags;		/* Flags */
    struct NotifyRequest *p_NR;	/* Notification request to use for file */
    VOID (*p_Func)(VOID *, ...);/* Function to call when notification occurs */
};

/* Prefs flags */
#define	PREFS_INTERNAL_F	(1L<<0)
#define	PREFS_CLOSEALL_F	(1L<<1)
#define	PREFS_GLOBAL_F		(1L<<2)	/* Pref record came from global dir */
#define	PREFS_SYSTEM_F		(1L<<3)	/* Pref record came from system dir */
#define	PREFS_DEFAULT_F		(1L<<4)	/* Pref record is the default */
#define	PREFS_WHERE_F		(PREFS_GLOBAL_F | PREFS_SYSTEM_F | PREFS_DEFAULT_F)

/* Screen mode preference record */
struct ScreenModePref
{
    struct Prefs smp_Header;	/* Header */
    ULONG smp_Reserved[4];
    ULONG smp_ModeID;		/* Display mode ID */
    UWORD smp_Width;		/* Width of screen, 65535 indicates DEFAULT */
    UWORD smp_Height;		/* Height of screen, 65535 indicates DEFAULT */
    UWORD smp_Depth;		/* Depth of screen */
    UWORD smp_AutoScroll;	/* Use autoscroll? */
};

#define	MAXSRCPLANES	8
#define	MAXCOLORS	256

#define	BPR(w)			((w) + 15 >> 4 << 1)
#define MaxPackedSize(rowSize)  ((rowSize) + (((rowSize)+127) >> 7 ))
#define	RowBytes(w)		((((w) + 15) >> 4) << 1)
#define	ChunkMoreBytes(cn)	(cn->cn_Size - cn->cn_Scan)
#define UGetByte()		(*source++)
#define UPutByte(c)		(*dest++ = (c))

#define VANILLA_COPY	0xC0
#define NO_MASK		0xFF

#define	MAXPENS		12

/* Palette preference record */
struct PalettePref
{
    struct Prefs pp_Header;	/* Header */
    WORD pp_CRegs[MAXCOLORS];	/* Color table used by LoadRGB4() */
    SHORT pp_NumColors;		/* Number of entries in the table */
    UWORD pp_Pens[MAXPENS];	/* Pen spec to use with this color palette */
};

/* Pointer preference record */
struct PointerPref
{
    struct Prefs pp_Header;	/* Preference header */
    UWORD *pp_PData;		/* Pointer data */
    WORD pp_Height;		/* Height of pointer */
    WORD pp_Width;		/* Width of pointer */
    WORD pp_XOffset;		/* X offset of hotspot */
    WORD pp_YOffset;		/* Y offset of hotspot */
    WORD pp_CRegs[3];		/* Color spec */
    LONG pp_DSize;		/* Size of data */
};

/* Work structure */
typedef struct ILBMRec
{
    ULONG ir_ModeID;		/* Display mode id */
    UWORD ir_Width;		/* Width of image */
    UWORD ir_Height;		/* Height of image */
    UWORD ir_Depth;		/* Depth of image */
    struct BitMap ir_BMap;	/* Bitmap */
    struct RastPort ir_RPort;	/* RastPort */
    struct Point2D ir_Grab;	/* Grab coordinates */
    WORD ir_CRegs[MAXCOLORS];	/* Color table used by LoadRGB4() */
    WORD ir_NumColors;		/* Number of colors in color table */
} ILBM;

#ifndef PREFS_INTERNAL
BPTR GetPrefsDrawer (STRPTR basename, ULONG flags);
VOID *GetPref (LONG kind, BPTR drawer, struct TagItem * attrs);
BOOL SetPref (LONG kind, BPTR drawer, VOID *pref, struct TagItem * attrs);
VOID FreePref (VOID *pref);
VOID FreePrefList (struct List *list);
VOID *GetPrefRecord (struct List *list, LONG kind);
#endif /* ifndef PREFS_INTERNAL */
