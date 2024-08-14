/** sgad.c -- suck program gadget definitions and dispatch  **/

#include <exec/types.h>
#include <intuition/intuition.h>
#include "suck.h"

/* big box for bool gadgets */
struct Image	box1 = { 0,0, WIDTH1, HEIGHT1, 0, NULL, 0, 2, 0 };

/* border for string gadgets	*/
SHORT	gbpairs[] = {-1, -1,
		    STRGWIDTH, -1,
		    STRGWIDTH, STRGHEIGHT,
		    -1, STRGHEIGHT,
		    -1,-1};

struct Border	gadgetborder = { 0, 0, 2, 2, JAM1, 5, gbpairs, 0 };

/* string info stuff */
UBYTE cksizebuf[CKSIZEBUFLEN];
UBYTE ckfreebuf[CKFREEBUFLEN];

/* need LongInt initialized */
struct StringInfo	cksizesi = { cksizebuf,	NULL, 0, CKSIZEBUFLEN, 0};
struct StringInfo	ckfreesi = { ckfreebuf,	NULL, 0, CKFREEBUFLEN, 0};

struct Gadget cksizeg = {
    (struct Gadget *)	&ckfreeg, /* next */
    (SHORT) MIDLEFTBORD, (SHORT) ROW(0),
	(SHORT) STRGWIDTH, (SHORT) STRGHEIGHT,
    /* hit box */
    (USHORT)	GADGHCOMP,	/* Flags	*/
    (USHORT)	LONGINT,	/* Activation	*/
    (USHORT)	STRGADGET,	/* GadgetType	*/
    (APTR)	&gadgetborder,		/* GadgetRender	*/
    (APTR)	NULL,	/* SelectRender	*/
				/* GadgetText	*/
    (struct IntuiText *)	&cksizegt,
    (LONG) 	0,		/* MutualExclude*/
    (APTR)	&cksizesi,	/*  SpecialInfo	*/
    (USHORT) 	CKSIZEG,	/* GadgetID	*/
    (APTR)	NULL, 	/* UserData	*/
    };


struct Gadget ckfreeg = {
    (struct Gadget *)	&suckg,	/* next */
    (SHORT) MIDLEFTBORD, (SHORT) ROW(1),
	(SHORT) STRGWIDTH, (SHORT) STRGHEIGHT,
    /* hit box */
    (USHORT)	GADGHCOMP,	/* Flags	*/
    (USHORT)	LONGINT,	/* Activation	*/
    (USHORT)	STRGADGET,	/* GadgetType	*/
    (APTR)	&gadgetborder,		/* GadgetRender	*/
    (APTR)	NULL,	/* SelectRender	*/
				/* GadgetText	*/
    (struct IntuiText *)	&ckfreegt,
    (LONG) 	0,		/* MutualExclude*/
    (APTR)	&ckfreesi,	/*  SpecialInfo	*/
    (USHORT) 	CKFREEG,	/* GadgetID	*/
    (APTR)	NULL, 		/* UserData	*/
    };

struct Gadget suckg = {
    (struct Gadget *)	&freeg,	/* next */
    (SHORT) LEFTBORD, (SHORT) ROW(3), (SHORT) WIDTH1, (SHORT) HEIGHT1,
    /* hit box */
    (USHORT)	GADGHCOMP | GADGIMAGE,	/* Flags	*/
    (USHORT)	RELVERIFY,	/* Activation	*/
    (USHORT)	BOOLGADGET,	/* GadgetType	*/
    (APTR)	&box1,		/* GadgetRender	*/
    (APTR)	NULL,		/* SelectRender	*/
				/* GadgetText	*/
    (struct IntuiText *)	&suckgt,
    (LONG) 	0,		/* MutualExclude*/
    (APTR)	NULL,		/*  SpecialInfo	*/
    (USHORT) 	SUCKG,		/* GadgetID	*/
    (APTR)	NULL, 		/* UserData	*/
    };

struct Gadget freeg = {
    (struct Gadget *)	&availg,	/* next */
    (SHORT) MIDLEFTBORD, (SHORT) ROW(3), (SHORT) WIDTH1, (SHORT) HEIGHT1,
    /* hit box */
    (USHORT)	GADGHCOMP | GADGIMAGE,	/* Flags	*/
    (USHORT)	RELVERIFY,	/* Activation	*/
    (USHORT)	BOOLGADGET,	/* GadgetType	*/
    (APTR)	&box1,		/* GadgetRender	*/
    (APTR)	NULL,		/* SelectRender	*/
				/* GadgetText	*/
    (struct IntuiText *)	&freegt,
    (LONG) 	0,		/* MutualExclude*/
    (APTR)	NULL,	/*  SpecialInfo	*/
    (USHORT) 	FREEG,	/* GadgetID	*/
    (APTR)	NULL, 	/* UserData	*/
    };

struct Gadget availg = {
    (struct Gadget *)	NULL,	/* next */
    (SHORT) LEFTBORD, (SHORT) ROW(2), (SHORT) WIDTH1, (SHORT) HEIGHT1,
    /* hit box */
    (USHORT)	GADGHCOMP | GADGIMAGE,	/* Flags	*/
    (USHORT)	RELVERIFY,	/* Activation	*/
    (USHORT)	BOOLGADGET,	/* GadgetType	*/
    (APTR)	&box1,		/* GadgetRender	*/
    (APTR)	NULL,		/* SelectRender	*/
				/* GadgetText	*/
    (struct IntuiText *)	&availgt,
    (LONG) 	0,		/* MutualExclude*/
    (APTR)	NULL,	/*  SpecialInfo	*/
    (USHORT) 	AVAILG,	/* GadgetID	*/
    (APTR)	NULL, 	/* UserData	*/
    };

struct TextAttr SafeFont =
{ "topaz.font", 8, 0, 0, };


/* bool gadget text */
struct IntuiText suckgt =
	{1,0,JAM1, TLEFT1, TTOP1, &SafeFont, SUCKGT, NULL};
struct IntuiText freegt =
	{1,0,JAM1, TLEFT1, TTOP1, &SafeFont, FREEGT, NULL};
struct IntuiText availgt =
	{1,0,JAM1, TLEFT1, TTOP1, &SafeFont, AVAILGT, NULL};

/* string gadget (label) text */
struct IntuiText cksizegt =
	{1,0,JAM1, LEFTBORD - MIDLEFTBORD, TTOP2, &SafeFont, CKSIZEGT, NULL};
struct IntuiText ckfreegt =
	{1,0,JAM1, LEFTBORD - MIDLEFTBORD, TTOP2, &SafeFont, CKFREEGT, NULL};

/* other text */
UBYTE	ckmsgbuf[CKMSGLEN];
struct IntuiText ckmsgt =
	{1,0,JAM2, 0, 0, &SafeFont, ckmsgbuf, NULL};

UBYTE	suckmsgbuf[SUCKMSGLEN];
struct IntuiText suckmsgt =
	{1,0,JAM2, 0, 0, &SafeFont, suckmsgbuf, NULL};

UBYTE	availmsgbuf[AVAILMSGLEN];
struct IntuiText availmsgt =
	{1,0,JAM2, 0, 0, &SafeFont, availmsgbuf, NULL};

