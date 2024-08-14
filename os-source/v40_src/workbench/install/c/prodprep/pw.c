/* header for powerwindows file - pw.h */
/* no particular reason for this second line */

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>

#include "/refresh.h"

struct TextAttr TOPAZ80 = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};
struct IntuiText IText1 = {
	1,0,JAM1,
	70,4,
	&TOPAZ80,
	"Exit",
	NULL
};

struct Gadget ExitGad = {
	NULL,
	206,159,
	175,16,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText1,
	NULL,
	NULL,
	BORDER|FANCY,
	NULL
};

struct IntuiText IText2 = {
	1,0,JAM1,
	37,9,
	&TOPAZ80,
	"Setup Hard Drive",
	NULL
};

struct Gadget Setup = {
	&ExitGad,
	192,57,
	197,25,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&IText2,
	NULL,
	NULL,
	BORDER|DROPSHADOW,
	NULL
};

#define GadgetList1 Setup

struct NewWindow NewWindowStructure1 = {
	0,0,
	640,200,
	0,1,
	REFRESHWINDOW+GADGETDOWN+GADGETUP+REQSET+REQCLEAR,
	WINDOWDRAG+WINDOWDEPTH+SIMPLE_REFRESH+ACTIVATE,
	&Setup,
	NULL,
	"A590 production program",
	NULL,
	NULL,
	40,30,
	-1,-1,
	WBENCHSCREEN
};

struct IntuiText IText3 = {
	2,0,JAM1,
	7,24,
	&TOPAZ80,
	"Formatting drive, please wait",
	NULL
};

#define IntuiTextList2 IText3

struct Requester RequesterStructure2 = {
	NULL,
	174,70,
	252,51,
	0,0,
	NULL,
	NULL,
	&IntuiTextList2,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};

SHORT BorderVectors1[] = {
	0,0,
	80,0,
	80,14,
	0,14,
	0,0
};
struct Border Border1 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors1,
	NULL
};

struct IntuiText IText4 = {
	2,0,JAM1,
	14,3,
	&TOPAZ80,
	"Cancel",
	NULL
};

struct Gadget SureCancelGad = {
	NULL,
	-100,-20,
	77,13,
	GRELBOTTOM+GRELRIGHT,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border1,
	NULL,
	&IText4,
	NULL,
	NULL,
	NULL,
	NULL
};

SHORT BorderVectors2[] = {
	0,0,
	80,0,
	80,14,
	0,14,
	0,0
};
struct Border Border2 = {
	-2,-1,
	2,0,JAM1,
	5,
	BorderVectors2,
	NULL
};

struct IntuiText IText5 = {
	2,0,JAM1,
	6,3,
	&TOPAZ80,
	"Continue",
	NULL
};

struct Gadget SureContinueGad = {
	&SureCancelGad,
	13,-19,
	77,13,
	GRELBOTTOM,
	RELVERIFY,
	BOOLGADGET+REQGADGET,
	(APTR)&Border2,
	NULL,
	&IText5,
	NULL,
	NULL,
	NULL,
	NULL
};

#define GadgetList3 SureContinueGad

struct Requester RequesterStructure8 = {
	NULL,
	152,45,
	306,94,
	0,0,
	&GadgetList3,
	NULL,
	NULL,
	NULL,
	1,
	NULL,
	NULL,
	NULL
};
