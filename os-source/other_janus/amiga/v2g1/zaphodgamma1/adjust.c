
/* **************************************************************************
 * 
 * Your Text Here
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 16 Dec 86  =RJ Mical=   Created this file
 *
 * *************************************************************************/

#include "zaphod.h"
#include <prosuite/reqsupp.h>


extern LONG KeyDelaySeconds, KeyDelayMicros;

#define ADJUST_MICROS	100




UBYTE SIBuffer1[8];
struct StringInfo GadgetSI1 = {
	SIBuffer1,	/* buffer where text will be edited */
	NULL,	/* optional undo buffer */
	0,	/* character position in buffer */
	8,	/* maximum number of characters to allow */
	0,	/* first displayed character buffer position */
	0,0,0,0,0,	/* Intuition initialized and maintained variables */
	0,	/* Rastport of gadget */
	0,	/* initial value for integer gadgets */
	NULL	/* alternate keymap (fill in if you set the flag) */
};

SHORT BorderVectors1[] = {
	0,0,
	73,0,
	73,10,
	0,10,
	0,0
};
struct Border Border1 = {
	-2,-1,	/* Border XY origin relative to container TopLeft */
	1,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	BorderVectors1,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText IText2 = {
	1,2,JAM1,	/* front and back text pens, drawmode and fill byte */
	-204,-10,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	(UBYTE *)"BETWEEN TRANSMISSION OF KEY EVENTS:",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct IntuiText IText1 = {
	1,2,JAM1,	/* front and back text pens, drawmode and fill byte */
	-151,-19,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	(UBYTE *)"ENTER NUMBER OF MICROSECONDS",	/* pointer to text */
	&IText2	/* next IntuiText structure */
};

struct Gadget AdjustMicros = {
	NULL,	/* next gadget */
	211,26,	/* origin XY of hit box relative to window TopLeft */
	70,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+ENDGADGET+STRINGRIGHT+LONGINT,	/* activation flags */
	STRGADGET,	/* gadget type flags */
	(APTR)&Border1,	/* gadget border or image to be rendered */NULL,	/* alternate imagery for selection */
	&IText1,	/* first IntuiText structure */
	0,	/* gadget mutual-exclude long word */
	(APTR)&GadgetSI1,	/* SpecialInfo structure */
	ADJUST_MICROS,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

/* Gadget list */

struct NewWindow NewWindowStructure1 = {
	10, 12,	/* window XY origin relative to TopLeft of screen */
	298,48,	/* window width and height */
	0,1,	/* detail and block pens */
	NULL,	/* IDCMP flags */
	SIMPLE_REFRESH | ACTIVATE | NOCAREREFRESH,	/* other window flags */
	NULL,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	NULL,	/* window title */
	NULL,	/* custom screen pointer */
	NULL,	/* custom bitmap */
	5,5,	/* minimum width and height */
	640,200,	/* maximum width and height */
	WBENCHSCREEN	/* destination screen type */
};

/* end of PowerWindows source generation */



#define AdjustSInfo		GadgetSI1
#define AdjustNewWindow	NewWindowStructure1

/* === Main Data ======================================================== */
struct ReqSupport AdjustSupport = 
	{

	/* struct Requester Requester; */
		{
		/*	struct Requester *OlderRequest;*/
		NULL,

		/*	SHORT LeftEdge, TopEdge;*/
		2, 2,

		/*	SHORT Width, Height;*/
		294, 44, 

		/*	SHORT RelLeft, RelTop;*/
		0, 0,

		/*	struct Gadget *ReqGadget;*/
		&AdjustMicros,
	
		/*	struct Border *ReqBorder;*/
		NULL,
		
		/*	struct IntuiText *ReqText;*/
		NULL,
	
		/*	USHORT Flags;*/
		NOISYREQ,

		/*	UBYTE BackFill;*/
		2,

		/*	struct ClipRect ReqCRect;*/
		{ NULL },

		/*	struct BitMap *ImageBMap;*/
		NULL,

		/*	struct BitMap ReqBMap;*/
		{ NULL },

		},	/* end of Requester structure */

	/* struct Window *Window; */
	NULL,

	/* LONG (*StartRequest)(); */
	NULL,

	/* LONG (*GadgetHandler)(); */
	NULL,

	/* LONG (*NewDiskHandler)(); */
	NULL,

	/* LONG (*KeyHandler)(); */
	NULL,

	/* LONG (*MouseMoveHandler)(); */
	NULL,

	/* SHORT SelectedGadgetID; */
	0,
};

struct Window *AdjustWindow;



AdjustStart()
{
	ActivateGadget(&AdjustMicros, AdjustWindow, &AdjustSupport.Requester);
}



AdjustKeyTiming(display)
struct Display *display;
{
	if (FlagIsSet(display->Modes, OPEN_SCREEN))
		{
		NewWindowStructure1.Type = CUSTOMSCREEN;
		NewWindowStructure1.Screen 
				= display->FirstWindow->DisplayScreen;
		}
	else
		NewWindowStructure1.Type = WBENCHSCREEN;
		
	AdjustWindow = OpenWindow(&AdjustNewWindow);
	if (AdjustWindow)
		{
		AdjustSupport.StartRequest = AdjustStart;
		AdjustSInfo.LongInt = KeyDelayMicros;
		sprintf(AdjustSInfo.Buffer, "%ld", KeyDelayMicros);
		AdjustSupport.Window = AdjustWindow;
		DoRequest(&AdjustSupport);
		KeyDelayMicros = ABS(AdjustSInfo.LongInt);
		if (KeyDelayMicros == 0) KeyDelayMicros = 1;
		KeyDelaySeconds = KeyDelayMicros / MILLION;
		KeyDelayMicros = KeyDelayMicros % MILLION;
		CloseWindow(AdjustWindow);
		}
}



