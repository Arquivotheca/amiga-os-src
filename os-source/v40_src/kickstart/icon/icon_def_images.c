/*
 * $Id: icon_def_images.c,v 38.1 91/06/24 19:01:36 mks Exp $
 *
 * $Log:	icon_def_images.c,v $
 * Revision 38.1  91/06/24  19:01:36  mks
 * Changed to V38 source tree - Trimmed Log
 * 
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>

USHORT diskGadData1[] =
{
/* Plane 0 */
 0x7F80, 0x001F, 0x8000,
 0xFF80, 0x071F, 0xE000,
 0xFF80, 0x071F, 0xE000,
 0xFF80, 0x071F, 0xE000,
 0xFF80, 0x071F, 0xE000,
 0xFFC0, 0x003F, 0xE000,
 0xFFFF, 0xFFFF, 0xE000,
 0xFFFF, 0xFFFF, 0xE000,
 0xF800, 0x0003, 0xE000,
 0xF000, 0x0001, 0xE000,
 0xF000, 0x06C1, 0xE000,
 0xF000, 0x0D81, 0xE000,
 0xF000, 0x1B01, 0xE000,
 0xF01B, 0x3601, 0xE000,
 0xF00D, 0xEC01, 0xE000,
 0x9006, 0xD801, 0xE000,
 0xF000, 0x0001, 0xE000,
/* Plane 1 */
 0x007F, 0xFFE0, 0x0000,
 0x007F, 0xF8E0, 0x0000,
 0x007F, 0xF8E0, 0x0000,
 0x007F, 0xF8E0, 0x0000,
 0x007F, 0xF8E0, 0x0000,
 0x003F, 0xFFC0, 0x0000,
 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000,
 0x07FF, 0xFFFC, 0x0000,
 0x0FFF, 0xFFFE, 0x0000,
 0x0FFF, 0xF93E, 0x0000,
 0x0FFF, 0xF27E, 0x0000,
 0x0FFF, 0xE4FE, 0x0000,
 0x0FE4, 0xC9FE, 0x0000,
 0x0FF2, 0x13FE, 0x0000,
 0x0FF9, 0x27FE, 0x0000,
 0x0FFF, 0xFFFE, 0x0000
};


struct	Image	diskGadI1 = {
	0, 0,				/* Top Corner */
	35, 17, 2,			/* Width, Height, Depth */
	&diskGadData1[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

struct	DrawerData	diskdr = {
	  50, 50, 400, 100,		/* Size */
	  255, 255,			/* DetailPen, BlockPen */
	  NULL,				/* IDCMP Flags */
	  0x240027f,			/* Flags */
	  NULL,NULL,NULL,NULL,NULL,
	  90, 40,			/* Min Size */
	  65535, 65535,			/* Max Size */
	  WBENCHSCREEN,			/* Type */
	  0,0				/* Origin */
};

struct	DiskObject	disk = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,35,18,			/* Left,Top,Width,Height */
	  GADGIMAGE|GADGHCOMP,		/* Flags */
	  0x0003,			/* Activation Flags */
	  BOOLGADGET,			/* Gadget Type */
	  (APTR)&diskGadI1,		/* Render Image */
	  NULL,				/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBDISK,				/* Icon Type */
	(char *)"SYS:System/DiskCopy",	/* Default Tool */
	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */
	&diskdr,			/* Drawer Structure */
	NULL,				/* Tool Window */
	0				/* Stack Size */
};

#ifdef	OLD_DRAWER
#define	DRAWER_WIDTH	80
#define	DRAWER_HEIGHT	16
#else
#define	DRAWER_WIDTH	57
#define	DRAWER_HEIGHT	14
#endif

USHORT  drawerGadData1[] = {
#ifdef	OLD_DRAWER
/* Plane 0 */
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0600, 0x0000, 0x0000, 0x0000, 0x0020,
 0x0400, 0x0000, 0x0000, 0x0000, 0x0120,
 0x0400, 0x0008, 0x0000, 0x8000, 0x0120,
 0x0400, 0x0009, 0xFFFC, 0x8000, 0x0120,
 0x0400, 0x0009, 0xD554, 0x8000, 0x0120,
 0x0400, 0x0008, 0x0000, 0x8000, 0x0120,
 0x0400, 0x0007, 0xFFFF, 0x0000, 0x0120,
 0x0400, 0x0000, 0x0000, 0x0000, 0x0120,
 0x04FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0020,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFE0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/* Plane 1 */
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x19FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1BFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFEC0,
 0x1BFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF6, 0x0003, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF6, 0x2AAB, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFEC0,
 0x1BFF, 0xFFF8, 0x0000, 0xFFFF, 0xFEC0,
 0x1BFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFEC0,
 0x1B00, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
#else
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0000,0x0FFF,0xFFFF,0xFFFF,0xE400,
    0x0C00,0x0000,0x0000,0x0400,0x0800,0x0000,0x0000,0x2400,
    0x0800,0x09FF,0x2000,0x2400,0x0800,0x09D5,0x2000,0x2400,
    0x0800,0x0800,0x2000,0x2400,0x0800,0x07FF,0xC000,0x2400,
    0x0800,0x0000,0x0000,0x2400,0x09FF,0xFFFF,0xFFFF,0xE400,
    0x0000,0x0000,0x0000,0x0400,0x0FFF,0xFFFF,0xFFFF,0xFC00,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
/* Plane 1 */
    0x3FFF,0xFFFF,0xFFFF,0xF800,0x3000,0x0000,0x0000,0x1800,
    0x33FF,0xFFFF,0xFFFF,0xF800,0x37FF,0xFFFF,0xFFFF,0xD800,
    0x37FF,0xF600,0xDFFF,0xD800,0x37FF,0xF62A,0xDFFF,0xD800,
    0x37FF,0xF7FF,0xDFFF,0xD800,0x37FF,0xF800,0x3FFF,0xD800,
    0x37FF,0xFFFF,0xFFFF,0xD800,0x3600,0x0000,0x0000,0x1800,
    0x3FFF,0xFFFF,0xFFFF,0xF800,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
#endif	/* OLD_DRAWER */
};

struct	Image	drawerGadI1 = {
	0, 0,				/* Top Corner */
	DRAWER_WIDTH, DRAWER_HEIGHT, 2,	/* Width, Height, Depth */
	&drawerGadData1[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

USHORT  drawerGadData2[] = {
#ifdef	OLD_DRAWER
/* Plane 0 */
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF20,
 0x0C00, 0x0000, 0x0000, 0x0000, 0x00A0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x0060,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFE0,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0030,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0034,
 0x2000, 0x0008, 0x0000, 0x8000, 0x0034,
 0x2000, 0x0009, 0xFFFC, 0x8000, 0x0035,
 0x2000, 0x0009, 0xD554, 0x8000, 0x0035,
 0x2000, 0x0008, 0x0000, 0x8000, 0x0035,
 0x2000, 0x0007, 0xFFFF, 0x0000, 0x0035,
 0x2000, 0x0000, 0x0000, 0x0000, 0x0035,
 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFF5,
 0x0055, 0x5555, 0x5555, 0x5555, 0x5555,
 0x0015, 0x5555, 0x5555, 0x5555, 0x5555,
/* Plane 1 */
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1800, 0x0000, 0x0000, 0x0000, 0x00C0,
 0x13FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF40,
 0x07FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF80,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF6, 0x0003, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF6, 0x2AAB, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF7, 0xFFFF, 0x7FFF, 0xFFC0,
 0x1FFF, 0xFFF8, 0x0000, 0xFFFF, 0xFFC0,
 0x1FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFC0,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
#else
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0000,0x0FFF,0xFFFF,0xFFFF,0xE400,
    0x1AAA,0xAAAA,0xAAAA,0xB400,0x3555,0x5555,0x5555,0x5C00,
    0x7FFF,0xFFFF,0xFFFF,0xFC00,0x4000,0x0000,0x0000,0x0400,
    0x4000,0x0000,0x0000,0x0600,0x4000,0x09FF,0x2000,0x0600,
    0x4000,0x09D5,0x2000,0x0680,0x4000,0x0800,0x2000,0x0680,
    0x4000,0x07FF,0xC000,0x0680,0x4000,0x0000,0x0000,0x0680,
    0x7FFF,0xFFFF,0xFFFF,0xFE80,0x02AA,0xAAAA,0xAAAA,0xAA80,
/* Plane 1 */
    0x3FFF,0xFFFF,0xFFFF,0xF800,0x3000,0x0000,0x0000,0x1800,
    0x22AA,0xAAAA,0xAAAA,0xA800,0x0555,0x5555,0x5555,0x5000,
    0x0000,0x0000,0x0000,0x0000,0x3FFF,0xFFFF,0xFFFF,0xF800,
    0x3FFF,0xFFFF,0xFFFF,0xF800,0x3FFF,0xF600,0xDFFF,0xF800,
    0x3FFF,0xF62A,0xDFFF,0xF800,0x3FFF,0xF7FF,0xDFFF,0xF800,
    0x3FFF,0xF800,0x3FFF,0xF800,0x3FFF,0xFFFF,0xFFFF,0xF800,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
#endif	/* OLD_DRAWER */
};

struct	Image	drawerGadI2 = {
	0, 0,				/* Top Corner */
	DRAWER_WIDTH, DRAWER_HEIGHT, 2,	/* Width, Height, Depth */
	&drawerGadData2[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

struct	DiskObject	drawer = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,DRAWER_WIDTH,DRAWER_HEIGHT, /* Left,Top,Width,Height */
	  0x0006,			/* Flags */
	  0x0003,			/* Activation Flags */
	  BOOLGADGET,			/* Gadget Type */
	  (APTR)&drawerGadI1,		/* Render Image */
	  (APTR)&drawerGadI2,		/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBDRAWER,			/* Icon Type */
	NULL,				/* Default Tool */
	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */
	&diskdr,			/* Drawer Structure */
	NULL,				/* Tool Window */
	0				/* Stack Size */
};

USHORT  toolGadData1[] = {
 /* Plane 0 */
 0x0000, 0x0000, 0x0000, 0x0400,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x03F0, 0x0FFF, 0xE000, 0x0C00,
 0x0208, 0x3000, 0x1C00, 0x0C00,
 0x0207, 0xC000, 0x0380, 0x0C00,
 0x0200, 0x0000, 0x0060, 0x0C00,
 0x0200, 0x0000, 0x0010, 0x0C00,
 0x0200, 0x0000, 0x0008, 0x0C00,
 0x0207, 0xC000, 0x1FC4, 0x0C00,
 0x0208, 0x2000, 0x2032, 0x0C00,
 0x03F0, 0x1800, 0xC00D, 0x0C00,
 0x0000, 0x0603, 0x0003, 0x0C00,
 0x0000, 0x0202, 0x0000, 0x0C00,
 0x0000, 0x0202, 0x0000, 0x0C00,
 0x0000, 0x0202, 0x0000, 0x0C00,
 0x0000, 0x03FE, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x7FFF, 0xFFFF, 0xFFFF, 0xFC00,
 /* Plane 1 */
 0xFFFF, 0xFFFF, 0xFFFF, 0xF800,
 0xD555, 0x5555, 0x5555, 0x5000,
 0xD555, 0x5555, 0x5555, 0x5000,
 0xD555, 0x5555, 0x5555, 0x5000,
 0xD555, 0x5555, 0x5555, 0x5000,
 0xD555, 0x5555, 0x5555, 0x5000,
 0xD405, 0x5000, 0x1555, 0x5000,
 0xD405, 0x4000, 0x0155, 0x5000,
 0xD400, 0x0000, 0x0055, 0x5000,
 0xD400, 0x0000, 0x0015, 0x5000,
 0xD400, 0x0000, 0x0005, 0x5000,
 0xD400, 0x0000, 0x0005, 0x5000,
 0xD400, 0x0000, 0x0001, 0x5000,
 0xD405, 0x4000, 0x1541, 0x5000,
 0xD405, 0x4000, 0x1550, 0x5000,
 0xD555, 0x5000, 0x5554, 0x5000,
 0xD555, 0x5401, 0x5555, 0x5000,
 0xD555, 0x5401, 0x5555, 0x5000,
 0xD555, 0x5401, 0x5555, 0x5000,
 0xD555, 0x5401, 0x5555, 0x5000,
 0xD555, 0x5555, 0x5555, 0x5000,
 0x8000, 0x0000, 0x0000, 0x0000,
 };

struct	Image	toolGadI1 = {
	0, 0,				/* Top Corner */
	54, 22, 2,			/* Width, Height, Depth */
	&toolGadData1[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

struct	DiskObject	tool = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,54,23,			/* Left,Top,Width,Height */
	  0x4,				/* Flags */
	  0x1,				/* Activation Flags */
	  0x1,				/* Gadget Type */
	  (APTR)&toolGadI1,		/* Render Image */
	  NULL,				/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBTOOL,				/* Icon Type */
	NULL,				/* Default Tool */
	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */
	NULL,				/* Drawer Structure */
	NULL,				/* Tool Window */
	0				/* Stack Size */
};

USHORT projectGadData1[] = {
/* Plane 0 */
 0x0000, 0x0000, 0x0004, 0x0000,
 0x0000, 0x0000, 0x0001, 0x0000,
 0x0000, 0x0000, 0x0000, 0x4000,
 0x0000, 0x0000, 0x0000, 0x1000,
 0x0000, 0x0000, 0x0000, 0x0800,
 0x0000, 0xE000, 0x0000, 0x0C00,
 0x0001, 0xF000, 0x0000, 0x0C00,
 0x0003, 0xB800, 0x0000, 0x0C00,
 0x0007, 0x1C00, 0x0000, 0x0C00,
 0x000E, 0x0E00, 0x0000, 0x0C00,
 0x001C, 0x0700, 0x0000, 0x0C00,
 0x003F, 0xFF80, 0x7FF0, 0x0C00,
 0x001F, 0xFF80, 0x0000, 0x0C00,
 0x0000, 0x0000, 0x7E00, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x001F, 0xFFFC, 0x0C00,
 0x0000, 0x0000, 0x0000, 0x0C00,
 0x0000, 0x001F, 0xFFC0, 0x0C00,
 0x4000, 0x0000, 0x0000, 0x0C00,
 0x1000, 0x0000, 0x0000, 0x0C00,
 0x0400, 0x0000, 0x0000, 0x0C00,
 0x01FF, 0xFFFF, 0xFFFF, 0xFC00,
/* Plane 1 */
 0xFFFF, 0xFFFF, 0xFFF8, 0x0000,
 0xD555, 0x5555, 0x5556, 0x0000,
 0xD555, 0x5555, 0x5555, 0x8000,
 0xDFFF, 0xFFFF, 0x5555, 0x6000,
 0xD000, 0x0001, 0x5555, 0x5000,
 0xD000, 0xC001, 0x5555, 0x5000,
 0xD001, 0xE001, 0x5555, 0x5000,
 0xD003, 0x3001, 0x5555, 0x5000,
 0xD006, 0x1801, 0x7FFF, 0x5000,
 0xD00C, 0x0C01, 0x7FFF, 0x5000,
 0xD018, 0x0601, 0x7FFF, 0x5000,
 0xD03F, 0xFF01, 0x000F, 0x5000,
 0xD000, 0x0001, 0x7FFF, 0x5000,
 0xD000, 0x0001, 0x01FF, 0x5000,
 0xDFFF, 0xFFFF, 0x7FFF, 0x5000,
 0xD555, 0x55E0, 0x0003, 0x5000,
 0xD555, 0x55FF, 0xFFFF, 0x5000,
 0xD555, 0x55E0, 0x003F, 0x5000,
 0x3555, 0x55FF, 0xFFFF, 0x5000,
 0x0D55, 0x55FF, 0xFFFF, 0x5000,
 0x0355, 0x5555, 0x5555, 0x5000,
 0x0000, 0x0000, 0x0000, 0x0000,
};

struct	Image	projectGadI1 = {
	0, 0,				/* Top Corner */
	54, 22, 2,			/* Width, Height, Depth */
	&projectGadData1[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

struct	DiskObject	project = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,54,23,			/* Left,Top,Width,Height */
	  0x5,				/* Flags */
	  0x1,				/* Activation Flags */
	  0x1,				/* Gadget Type */
	  (APTR)&projectGadI1,		/* Render Image */
	  NULL,				/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBPROJECT,			/* Icon Type */
	NULL,				/* Default Tool */
	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */
	NULL,				/* Drawer Structure */
	NULL,				/* Tool Window */
	0				/* Stack Size */
};

USHORT  trashcanGadData1[] = {
 /* Plane 0 */
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x3FE0, 0x0000, 0x0000,
 0x0000, 0xE038, 0x0000, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x1FFF, 0xFFFF, 0xFFF0, 0x0000,
 0x1FFF, 0xFFFF, 0xFFF0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x07FF, 0xFFFF, 0xFFE0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFFF8, 0x0000,
 0x03FF, 0xFFFF, 0xFFFF, 0xE000,
 0x01FF, 0xFFFF, 0xFFFF, 0xE000,
 0x000F, 0xFFFF, 0xFFFC, 0x0000,
 /* Plane 1 */
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x03FF, 0xFFF5, 0x5500, 0x0000,
 0x07FF, 0xFFFF, 0xAA80, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x03FF, 0xFF55, 0x5400, 0x0000,
 0x03FF, 0xFFFF, 0xAA80, 0x0000,
 0x038F, 0xFC7D, 0x4200, 0x0000,
 0x0317, 0xF8BE, 0x8880, 0x0000,
 0x0337, 0xF9BF, 0x9100, 0x0000,
 0x0313, 0xF9BF, 0x1000, 0x0000,
 0x019B, 0xF9BE, 0x9100, 0x0000,
 0x019B, 0xF93F, 0x1200, 0x0000,
 0x019B, 0xF9BE, 0x9100, 0x0000,
 0x019B, 0xF93F, 0x2200, 0x0000,
 0x019B, 0xF9BE, 0x2500, 0x0000,
 0x0189, 0xF93F, 0x2200, 0x0000,
 0x01CD, 0xF9BE, 0x2400, 0x0000,
 0x00C9, 0xF93D, 0x2A00, 0x0000,
 0x00CD, 0xF9BE, 0x4400, 0x0000,
 0x00C7, 0xF9FE, 0x4A00, 0x0000,
 0x00E3, 0xFE75, 0x1400, 0x0000,
 0x007F, 0xFFEA, 0xA800, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 };

struct	Image	trashcanGadI1 = {
	0, 0,				/* Top Corner */
	51, 31, 2,			/* Width, Height, Depth */
	&trashcanGadData1[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

USHORT  trashcanGadData2[] = {
 /* Plane 0 */
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0003, 0xFFFF, 0x8000, 0x0000,
 0x00FF, 0xFFFF, 0xFE00, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x1FFF, 0xFFFF, 0xFFF0, 0x0000,
 0x1FFF, 0xFFFF, 0xFFF0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x00FF, 0xFFFF, 0xFE00, 0x0000,
 0x0003, 0xFFFF, 0x8000, 0x0000,
 0x1FFF, 0xFFFF, 0xFFF0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x0FFF, 0xFFFF, 0xFFE0, 0x0000,
 0x07FF, 0xFFFF, 0xFFE0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x07FF, 0xFFFF, 0xFFC0, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFF80, 0x0000,
 0x03FF, 0xFFFF, 0xFFF8, 0x0000,
 0x03FF, 0xFFFF, 0xFFFF, 0xE000,
 0x01FF, 0xFFFF, 0xFFFF, 0xE000,
 0x000F, 0xFFFF, 0xFFFC, 0x0000,
 /* Plane 1 */
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0003, 0xFFFF, 0x8000, 0x0000,
 0x00FC, 0x0000, 0x7E00, 0x0000,
 0x0702, 0xAAAA, 0x81C0, 0x0000,
 0x0855, 0x6AB5, 0x5420, 0x0000,
 0x02AA, 0xAAAA, 0xABC0, 0x0000,
 0x00FD, 0x5555, 0x7E00, 0x0000,
 0x0003, 0xFFFF, 0x8000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x03FF, 0xFF55, 0x5400, 0x0000,
 0x03FF, 0xFFFF, 0xAA80, 0x0000,
 0x038F, 0xFC7D, 0x4200, 0x0000,
 0x0317, 0xF8BE, 0x8880, 0x0000,
 0x0337, 0xF9BF, 0x9100, 0x0000,
 0x0313, 0xF9BF, 0x1000, 0x0000,
 0x019B, 0xF9BE, 0x9100, 0x0000,
 0x019B, 0xF93F, 0x1200, 0x0000,
 0x019B, 0xF9BE, 0x9100, 0x0000,
 0x019B, 0xF93F, 0x2200, 0x0000,
 0x019B, 0xF9BE, 0x2500, 0x0000,
 0x0189, 0xF93F, 0x2200, 0x0000,
 0x01CD, 0xF9BE, 0x2400, 0x0000,
 0x00C9, 0xF93D, 0x2A00, 0x0000,
 0x00CD, 0xF9BE, 0x4400, 0x0000,
 0x00C7, 0xF9FE, 0x4A00, 0x0000,
 0x00E3, 0xFE75, 0x1400, 0x0000,
 0x007F, 0xFFEA, 0xA800, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 0x0000, 0x0000, 0x0000, 0x0000,
 };

struct	Image	trashcanGadI2 = {
	0, 0,				/* Top Corner */
	51, 31, 2,			/* Width, Height, Depth */
	&trashcanGadData2[0],		/* Image Data */
	0x003, 0x000,			/* PlanePick,PlaneOnOff */
	NULL				/* Next Image */
};

struct	DiskObject	trashcan = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,51,31,			/* Left,Top,Width,Height */
	  0x0006,			/* Flags */
	  0x0003,			/* Activation Flags */
	  BOOLGADGET,			/* Gadget Type */
	  (APTR)&trashcanGadI1,		/* Render Image */
	  (APTR)&trashcanGadI2,		/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBGARBAGE,			/* Icon Type */
	NULL,				/* Default Tool */
	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */
	&diskdr,			/* Drawer Structure */
	NULL,				/* Tool Window */
	0				/* Stack Size */
};

/* no DrawerData required for kickstart disk */

struct	DiskObject	kick = {
	WB_DISKMAGIC,			/* Magic Number */
	WB_DISKVERSION,			/* Version */
	{				/* Embedded Gadget Structure */
	  NULL,				/* Next Gadget Pointer */
	  0,0,35,18,			/* Left,Top,Width,Height */
	  GADGIMAGE|GADGHCOMP,		/* Flags */
	  0x0003,			/* Activation Flags */
	  BOOLGADGET,			/* Gadget Type */
	  (APTR)&diskGadI1,		/* Render Image */
	  NULL,				/* Select Image */
	  NULL,				/* Gadget Text */
	  NULL,				/* Mutual Exclude */
	  NULL,				/* Special Info */
	  0,				/* Gadget ID */
	  NULL				/* User Data */
	},
	WBKICK,				/* Icon Type */

	/* no default tool required for kickstart disk */
	NULL,				/* Default Tool */

	NULL,				/* Tool Type Array */
	NO_ICON_POSITION,		/* Current X */
	NO_ICON_POSITION,		/* Current Y */

	/* no ptr to DrawerData required for kickstart disk */
	NULL,				/* Drawer Structure */

	NULL,				/* Tool Window */
	0				/* Stack Size */
};
