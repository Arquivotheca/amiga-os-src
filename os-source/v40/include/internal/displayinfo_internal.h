#ifndef	GRAPHICS_RAWDISPLAYINFO_H
#define	GRAPHICS_RAWDISPLAYINFO_H
/*
**	$Filename: displayinfo_internal.h $
**	$Release: 1.4 $
**	$Revision: 39.10 $
**	$Date: 93/02/16 13:39:05 $
**
**	include define file for displayinfo database
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

/* the "compressed" form of a DisplayInfoRecord */

struct RomCompressedNode
{
    BYTE            cno; 
    UBYTE           tag; 
    UWORD           cid;
};

/* the "public" part of a DisplayInfoRecord */

struct RecordNode 
{
    struct RecordNode *rcn_Succ;    /* next sibling */
    struct RecordNode *rcn_Pred;    /* previous sibling */
    struct RecordNode *rcn_Child;   /* subtype of record */
    struct RecordNode *rcn_Parent;  /* supertype of record */
};

/* some useful macro definitions */

#define SubRecord(r) (r)->rec_Node.rcn_Child
#define SupRecord(r) (r)->rec_Node.rcn_Parent
#define SibRecord(r) (r)->rec_Node.rcn_Succ

/* the "private" internal definition of a DisplayInfoRecord */

struct DisplayInfoRecord 
{
    struct RecordNode  rec_Node;
    UWORD	       rec_MajorKey;
    UWORD	       rec_MinorKey;
    struct TagItem     rec_Tag;     /* { TAG_DONE, 0 } or { TAG_MORE, &data } */
    ULONG	       rec_Control;
    ULONG              (*rec_get_data)();
    ULONG              (*rec_set_data)();
    struct Rectangle   rec_ClipOScan;
    ULONG	       reserved[2];
};

/* the "private" internal definition of datachunk types */

struct RawDisplayInfo
{
	struct	QueryHeader Header;
	UWORD	NotAvailable;
	ULONG	PropertyFlags;
	Point	Resolution;
	UWORD	PixelSpeed;
	UWORD	NumStdSprites;
	UWORD	PaletteRange;
	Point	SpriteResolution;
	ULONG	ModeID;
	UBYTE	RedBits;
	UBYTE	GreenBits;
	UBYTE	BlueBits;
	UBYTE	pad2[5];
	ULONG	reserved[2];    /* tag end substitute */
};

struct RawNameInfo
{
	struct	QueryHeader Header;
	UBYTE	Name[DISPLAYNAMELEN];
	ULONG	reserved[2];    /* tag end substitute */
};

struct RawDimensionInfo
{
	struct	QueryHeader Header;
	UWORD	MaxDepth;
        UWORD   MinRasterWidth;
        UWORD   MinRasterHeight;
        UWORD   MaxRasterWidth;
        UWORD   MaxRasterHeight;
	struct  Rect32	Nominal;
	struct  Rect32	MaxOScan;
	struct  Rect32	VideoOScan;
	UBYTE	HWMaxDepth;
	UBYTE   pad[5];
	ULONG	reserved[2];    /* tag end substitute */
};

struct RawMonitorInfo
{
	struct	QueryHeader   Header;
	struct	MonitorSpec  *Mspc;
        Point   ViewPosition;
        Point   ViewResolution;
        struct  Rectangle ViewPositionRange;
        UWORD   TotalRows;
        UWORD   TotalColorClocks;
        UWORD   MinRow;
	WORD	Compatibility;
	struct  Rect32	TxtOScan;
	struct  Rect32	StdOScan;
	Point	MouseTicks;
	Point	DefaultViewPosition;
	ULONG	PreferredModeID;
	ULONG	reserved[2];    /* tag end substitute */
};

struct RawVecInfo
{
	struct	QueryHeader Header;
	APTR	Vec;
	APTR	Data;
	UWORD	Type;
	UWORD	pad[3];
	ULONG	reserved[2];
};

#define DEFAULT_ENTRY (DEFAULT_MONITOR_ID >> 16)
#define NTSC_ENTRY (NTSC_MONITOR_ID >> 16)
#define PAL_ENTRY (PAL_MONITOR_ID >> 16)
#define VGA_ENTRY (VGA_MONITOR_ID >> 16)
#define ROM_MONITOR_ENTRIES 2		/* entry 2 = last entry in the ROM - rest are diskbased */


/* Some new Squeezed stuff to get those last few bytes.
 * (I'm glad I have a sense of humour)
 */

typedef struct tbPoint
{
   UBYTE x, y;
} bPoint;


struct Rect8
{
    UBYTE   MinX,MinY;
    UBYTE   MaxX,MaxY;
};

struct SqueezedVecInfo	/* unused */
{
	UWORD	Type;
};

struct SqueezedDisplayInfo
{
	ULONG	PropertyFlags;
	bPoint	Resolution;
	UBYTE	PixelSpeed;
	UWORD	PaletteRange;
	bPoint	SpriteResolution;
	ULONG	ModeID;
};

#define DIMS_RANGE_LORES 0
#define DIMS_RANGE_HIRES 1
#define DIMS_RANGE_SHIRES 2
#define DIMS_RANGE_A2024NTSC  3
#define DIMS_RANGE_A2024PAL  4
#define DIMS_DEPTH_HW6 128

struct SqueezedDimensionInfo
{
	UBYTE	MaxDepth;
	UBYTE	DimsRange;
	struct  Rectangle	Nominal;
	struct  Rectangle	MaxOScan;
	struct  Rectangle	VideoOScan;
};

/* These are used in the "(un)cooking" functions. */

#define GET_DISPLAYINFODATA 0
#define SET_DISPLAYINFODATA 1
#define NEW_DISPLAYINFODATA 2


#endif	/* GRAPHICS_RAWDISPLAYINFO_H */
