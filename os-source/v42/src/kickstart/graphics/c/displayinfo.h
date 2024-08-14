#ifndef	GRAPHICS_DISPLAYINFO_H
#define	GRAPHICS_DISPLAYINFO_H
/*
**	$Id: displayinfo.h,v 37.5 91/04/26 16:33:27 spence Exp $
**
**	include define file for displayinfo database
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif /* GRAPHICS_GFX_H */

#ifndef GRAPHICS_MONITOR_H
#include <graphics/monitor.h>
#endif /* GRAPHICS_MONITOR_H */

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif /* UTILITY_TAGITEM_H */

/* the "public" handle to a DisplayInfoRecord */

typedef APTR DisplayInfoHandle;	

/* datachunk type identifiers */

#define DTAG_DISP               0x80000000
#define DTAG_DIMS               0x80001000
#define DTAG_MNTR               0x80002000
#define DTAG_NAME               0x80003000

struct QueryHeader
{
	ULONG	StructID;	/* datachunk type identifier */
        ULONG   DisplayID;      /* copy of display record key   */
	ULONG	SkipID;		/* TAG_SKIP -- see tagitems.h */
	ULONG	Length;		/* length of local data in double-longwords */
};

struct DisplayInfo
{
        struct  QueryHeader Header;
        UWORD   NotAvailable;   /* if NULL available, else see defines */
        ULONG   PropertyFlags;  /* Properties of this mode see defines */
        Point   Resolution;     /* ticks-per-pixel X/Y                 */
        UWORD   PixelSpeed;     /* aproximation in nanoseconds         */
        UWORD   NumStdSprites;  /* number of standard amiga sprites    */
        UWORD   PaletteRange;   /* distinguishable shades available    */
        Point   SpriteResolution; /* std sprite ticks-per-pixel X/Y    */
        UBYTE   pad[4];
        ULONG   reserved[2];    /* terminator */
};

/* availability */ 

#define DI_AVAIL_NOCHIPS        0x0001
#define DI_AVAIL_NOMONITOR      0x0002
#define DI_AVAIL_NOTWITHGENLOCK 0x0004

/* mode properties */ 

#define DIPF_IS_LACE            0x00000001
#define DIPF_IS_DUALPF          0x00000002
#define DIPF_IS_PF2PRI          0x00000004
#define DIPF_IS_HAM             0x00000008

#define DIPF_IS_ECS             0x00000010	/*	note: ECS modes (SHIRES, VGA, and **
											**	PRODUCTIVITY) do not support      **
											**	attached sprites.                 **
											*/
#define DIPF_IS_PAL             0x00000020
#define DIPF_IS_SPRITES         0x00000040
#define DIPF_IS_GENLOCK         0x00000080

#define DIPF_IS_WB              0x00000100
#define DIPF_IS_DRAGGABLE       0x00000200
#define DIPF_IS_PANELLED        0x00000400
#define DIPF_IS_BEAMSYNC        0x00000800

#define DIPF_IS_EXTRAHALFBRITE	0x00001000

struct DimensionInfo
{
        struct  QueryHeader Header;
        UWORD   MaxDepth;             /* log2( max number of colors ) */
        UWORD   MinRasterWidth;       /* minimum width in pixels      */
        UWORD   MinRasterHeight;      /* minimum height in pixels     */
        UWORD   MaxRasterWidth;       /* maximum width in pixels      */
        UWORD   MaxRasterHeight;      /* maximum height in pixels     */
        struct  Rectangle   Nominal;  /* "standard" dimensions        */
        struct  Rectangle   MaxOScan; /* fixed, hardware dependant    */
        struct  Rectangle VideoOScan; /* fixed, hardware dependant    */
        struct  Rectangle   TxtOScan; /* editable via preferences     */
        struct  Rectangle   StdOScan; /* editable via preferences     */
        UBYTE   pad[14];
        ULONG   reserved[2];          /* terminator */
};

struct MonitorInfo
{
        struct  QueryHeader Header;
        struct  MonitorSpec  *Mspc;   /* pointer to monitor specification  */
        Point   ViewPosition;         /* editable via preferences          */
        Point   ViewResolution;       /* standard monitor ticks-per-pixel  */
        struct  Rectangle ViewPositionRange;  /* fixed, hardware dependant */
        UWORD   TotalRows;            /* display height in scanlines       */
        UWORD   TotalColorClocks;     /* scanline width in 280 ns units    */
        UWORD   MinRow;               /* absolute minimum active scanline  */
        WORD    Compatibility;        /* how this coexists with others     */
        UBYTE   pad[36];
		Point	DefaultViewPosition;  /* original, never changes */
		ULONG	PreferredModeID;      /* for Preferences */
        ULONG   reserved[2];          /* terminator */
};

/* monitor compatibility */

#define MCOMPAT_MIXED   0       /* can share display with other MCOMPAT_MIXED */
#define MCOMPAT_SELF    1       /* can share only within same monitor */
#define MCOMPAT_NOBODY -1       /* only one viewport at a time */

#define DISPLAYNAMELEN 32

struct NameInfo
{
        struct  QueryHeader Header;
        UBYTE   Name[DISPLAYNAMELEN];
        ULONG   reserved[2];          /* terminator */
};

/* DisplayInfoRecord identifiers */

#define INVALID_ID              ~0

/* normal identifiers */

#define MONITOR_ID_MASK      		0xFFFF1000 

#define DEFAULT_MONITOR_ID      	0x00000000 
#define NTSC_MONITOR_ID         	0x00011000 
#define PAL_MONITOR_ID          	0x00021000 

/* the following 20 composite keys are for Modes on the default Monitor */
/* ntsc & pal "flavors" of these particular keys may be made by or'ing  */
/* the ntsc or pal MONITOR_ID with the desired MODE_KEY... */

#define LORES_KEY           		0x00000000 
#define HIRES_KEY           		0x00008000 
#define SUPER_KEY           		0x00008020 
#define HAM_KEY             		0x00000800 
#define LORESLACE_KEY       		0x00000004 
#define HIRESLACE_KEY       		0x00008004 
#define SUPERLACE_KEY       		0x00008024 
#define HAMLACE_KEY         		0x00000804 
#define LORESDPF_KEY        		0x00000400 
#define HIRESDPF_KEY        		0x00008400 
#define SUPERDPF_KEY        		0x00008420 
#define LORESLACEDPF_KEY    		0x00000404 
#define HIRESLACEDPF_KEY    		0x00008404 
#define SUPERLACEDPF_KEY    		0x00008424 
#define LORESDPF2_KEY       		0x00000440 
#define HIRESDPF2_KEY       		0x00008440 
#define SUPERDPF2_KEY       		0x00008460 
#define LORESLACEDPF2_KEY   		0x00000444 
#define HIRESLACEDPF2_KEY   		0x00008444 
#define SUPERLACEDPF2_KEY   		0x00008464 
#define EXTRAHALFBRITE_KEY           	0x00000080 
#define EXTRAHALFBRITELACE_KEY       	0x00000084 

/* vga identifiers */

#define VGA_MONITOR_ID          	0x00031000 

#define VGAEXTRALORES_KEY         	0x00031004 
#define VGALORES_KEY          		0x00039004 
#define VGAPRODUCT_KEY           	0x00039024 
#define VGAHAM_KEY            		0x00031804 
#define VGAEXTRALORESLACE_KEY     	0x00031005 
#define VGALORESLACE_KEY      		0x00039005 
#define VGAPRODUCTLACE_KEY       	0x00039025 
#define VGAHAMLACE_KEY        		0x00031805 
#define VGAEXTRALORESDPF_KEY      	0x00031404 
#define VGALORESDPF_KEY       		0x00039404 
#define VGAPRODUCTDPF_KEY        	0x00039424 
#define VGAEXTRALORESLACEDPF_KEY  	0x00031405 
#define VGALORESLACEDPF_KEY   		0x00039405 
#define VGAPRODUCTLACEDPF_KEY    	0x00039425 
#define VGAEXTRALORESDPF2_KEY     	0x00031444 
#define VGALORESDPF2_KEY      		0x00039444 
#define VGAPRODUCTDPF2_KEY       	0x00039464 
#define VGAEXTRALORESLACEDPF2_KEY 	0x00031445 
#define VGALORESLACEDPF2_KEY  		0x00039445 
#define VGAPRODUCTLACEDPF2_KEY   	0x00039465 
#define VGAEXTRAHALFBRITE_KEY   	0x00031084 
#define VGAEXTRAHALFBRITELACE_KEY   	0x00031085 

/* vga70 identifiers */

#define VGA70_MONITOR_ID 			0x00061000

#define VGA70EXTRALORES_KEY         0x00061004
#define VGA70LORES_KEY          	0x00069004
#define VGA70PRODUCT_KEY	        0x00069024
#define VGA70HAM_KEY            	0x00061804
#define VGA70EXTRALORESLACE_KEY     0x00061005
#define VGA70LORESLACE_KEY      	0x00069005
#define VGA70PRODUCTLACE_KEY	    0x00069025
#define VGA70HAMLACE_KEY        	0x00061805
#define VGA70EXTRALORESDPF_KEY      0x00061404
#define VGA70LORESDPF_KEY       	0x00069404
#define VGA70PRODUCTDPF_KEY     	0x00069424
#define VGA70EXTRALORESLACEDPF_KEY  0x00061405
#define VGA70LORESLACEDPF_KEY   	0x00069405
#define VGA70PRODUCTLACEDPF_KEY    	0x00069425
#define VGA70EXTRALORESDPF2_KEY     0x00061444
#define VGA70LORESDPF2_KEY      	0x00069444
#define VGA70PRODUCTDPF2_KEY       	0x00069464
#define VGA70EXTRALORESLACEDPF2_KEY 0x00061445
#define VGA70LORESLACEDPF2_KEY  	0x00069445
#define VGA70PRODUCTLACEDPF2_KEY   	0x00069465
#define VGA70EXTRAHALFBRITE_KEY   	0x00061084
#define VGA70EXTRAHALFBRITELACE_KEY 0x00061085

/* a2024 identifiers */

#define A2024_MONITOR_ID        	0x00041000 

#define A2024TENHERTZ_KEY       	0x00041000 
#define A2024FIFTEENHERTZ_KEY   	0x00049000 

/* prototype identifiers */

#define PROTO_MONITOR_ID        	0x00051000 

#endif	/* GRAPHICS_DISPLAYINFO_H */
