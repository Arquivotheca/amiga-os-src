
/* ***************************************************************************
 * 
 * Color Window Data  --  Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 27 Feb 86	=RJ Mical=	Modified these routines for Zaphod
 * January 86	=RJ=		Modified the originals for Mandelbrot
 * Late 85	=RJ=		Created the color window for Graphicraft
 *
 * **************************************************************************/
									

#define EGLOBAL_CANCEL	      /* This prevents eglobal.c from being included */
#include "zaphod.h"

extern struct TextAttr SafeFont;


USHORT RGBData[] =
    {
    0xFC00,
    0x6600,
    0x6600,
    0x7C00,
    0x6C00,
    0x6600,
    0xE300,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x3C00,
    0x6600,
    0xC000,
    0xCE00,
    0xC600,
    0x6600,
    0x3E00,
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0xFC00,
    0x6600,
    0x6600,
    0x7C00,
    0x6600,
    0x6600,
    0xFC00,
    };


struct Image ColorRGBImage =
    {
    3, 1,
    8, 
    29,
    1,
    &RGBData[0],
    0x1, 0x0,
    NULL,
    };


SHORT ClusterBorderVectors[] =
    {
    -1, -1,
    -1, COLOR_CLUSTER_HEIGHT,
    COLOR_CLUSTER_WIDTH, COLOR_CLUSTER_HEIGHT,
    COLOR_CLUSTER_WIDTH, -1,
    -1, -1,
    };


struct Border ColorClusterBorder =
    {
    0, 0, 
    1, 0,
    JAM1,
    5,
    &ClusterBorderVectors[0],
    NULL,
    };

struct IntuiText ColorClusterText[4] =
    {
    /* "COPY" */
	{
	1, 0,
	JAM2,
	2 + CHARACTER_WIDTH, 1, 
	&SafeFont,
	"COPY",
	NULL,
	},

    /* "RANGE" */
	{
	1, 0, 
	JAM2,
	2 + (CHARACTER_WIDTH >> 1), 1, 
	&SafeFont,
	"RANGE",
	NULL,
	},

    /* "OK" */
	{
	1, 0, 
	JAM2,
	2 + (CHARACTER_WIDTH << 1), 1, 
	&SafeFont,
	"OK",
	NULL,
	},

    /* "CANCEL" */
	{
	1, 0, 
	JAM2,
	2, 1, 
	&SafeFont,
	"CANCEL",
	NULL,
	},

    };




/* ======================================================================== */
/* ======================================================================== */
/* ======================================================================== */

struct Image ColorPropsImages[3];


struct PropInfo ColorPropsInfos[3] = 
    {
	{
	/* COLOR_GREEN */
	AUTOKNOB | FREEHORIZ,
	0,
	0,
	COLOR_KNOB_BODY,
	0,
	0, 0, 0, 0, 0, 0,
	},

	{
	/* COLOR_RED */
	AUTOKNOB | FREEHORIZ,
	0,
	0,
	COLOR_KNOB_BODY,
	0,
	0, 0, 0, 0, 0, 0,
	},

	{
	/* COLOR_BLUE */
	AUTOKNOB | FREEHORIZ,
	0,
	0,
	COLOR_KNOB_BODY,
	0,
	0, 0, 0, 0, 0, 0,
	},

    };


struct Gadget ColorTemplateGadgets[COLOR_GADGETS_COUNT] =
    {
	{
	/* COLOR_COPY */
	NULL,
	COLOR_CLUSTER_LEFT,
	COLOR_CLUSTER_TOP + (00 * (COLOR_CLUSTER_HEIGHT + 3)),
	COLOR_CLUSTER_WIDTH,
	COLOR_CLUSTER_HEIGHT,
	GADGHCOMP,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ColorClusterBorder,
	NULL,
	&ColorClusterText[00],
	NULL,
	NULL,
	COLOR_COPY,
	NULL,
	},

	{
	/* COLOR_RANGE */
	&ColorTemplateGadgets[COLOR_COPY],
	COLOR_CLUSTER_LEFT,
	COLOR_CLUSTER_TOP + (01 * (COLOR_CLUSTER_HEIGHT + 3)),
	COLOR_CLUSTER_WIDTH,
	COLOR_CLUSTER_HEIGHT,
	GADGHCOMP,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ColorClusterBorder,
	NULL,
	&ColorClusterText[01],
	NULL,
	NULL,
	COLOR_RANGE,
	NULL,
	},

	{
	/* COLOR_OK */
	&ColorTemplateGadgets[COLOR_RANGE],
	COLOR_CLUSTER_LEFT,
	COLOR_CLUSTER_TOP + (02 * (COLOR_CLUSTER_HEIGHT + 3)),
	COLOR_CLUSTER_WIDTH,
	COLOR_CLUSTER_HEIGHT,
	GADGHCOMP,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ColorClusterBorder,
	NULL,
	&ColorClusterText[02],
	NULL,
	NULL,
	COLOR_OK,
	NULL,
	},

	{
	/* COLOR_CANCEL */
	&ColorTemplateGadgets[COLOR_OK],
	COLOR_CLUSTER_LEFT,
	COLOR_CLUSTER_TOP + (03 * (COLOR_CLUSTER_HEIGHT + 3)),
	COLOR_CLUSTER_WIDTH,
	COLOR_CLUSTER_HEIGHT,
	GADGHCOMP,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ColorClusterBorder,
	NULL,
	&ColorClusterText[03],
	NULL,
	NULL,
	COLOR_CANCEL,
	NULL,
	},

	{
	/* COLOR_GREEN */
	&ColorTemplateGadgets[COLOR_CANCEL],
	COLOR_PROP_LEFT,
	COLOR_PROP_TOP + (01 * (COLOR_PROP_HEIGHT + 1)),
	COLOR_PROP_WIDTH,
	COLOR_PROP_HEIGHT,
	GADGHCOMP | GADGIMAGE,
	FOLLOWMOUSE,
	PROPGADGET,
	(APTR)&ColorPropsImages[01],
	NULL,
	NULL,
	NULL,
	(APTR)&ColorPropsInfos[01],
	COLOR_GREEN,
	NULL,
	},

	{
	/* COLOR_RED */
	&ColorTemplateGadgets[COLOR_GREEN],
	COLOR_PROP_LEFT,
	COLOR_PROP_TOP + (00 * (COLOR_PROP_HEIGHT + 1)),
	COLOR_PROP_WIDTH,
	COLOR_PROP_HEIGHT,
	GADGHCOMP | GADGIMAGE,
	FOLLOWMOUSE,
	PROPGADGET,
	(APTR)&ColorPropsImages[00],
	NULL,
	NULL,
	NULL,
	(APTR)&ColorPropsInfos[00],
	COLOR_RED,
	NULL,
	},

	{
	/* COLOR_BLUE */
	&ColorTemplateGadgets[COLOR_RED],
	COLOR_PROP_LEFT,
	COLOR_PROP_TOP + (02 * (COLOR_PROP_HEIGHT + 1)),
	COLOR_PROP_WIDTH,
	COLOR_PROP_HEIGHT,
	GADGHCOMP | GADGIMAGE,
	FOLLOWMOUSE,
	PROPGADGET,
	(APTR)&ColorPropsImages[02],
	NULL,
	NULL,
	NULL,
	(APTR)&ColorPropsInfos[02],
	COLOR_BLUE,
	NULL,
	},

	{
	/* COLOR_HSL_RGB */
	&ColorTemplateGadgets[COLOR_BLUE],
	COLOR_HSL_LEFT,
	COLOR_HSL_TOP,
	CHARACTER_WIDTH + 5,
	COLOR_BOX_BOTTOM - COLOR_BOX_TOP + 1,
	GADGHIMAGE | GADGIMAGE | SELECTED,
	TOGGLESELECT,
	BOOLGADGET,
	(APTR)&ColorRGBImage,
	(APTR)&ColorRGBImage,
	NULL,
	NULL,
	NULL,
	COLOR_HSL_RGB,
	NULL,
	},
    };




