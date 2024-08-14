#define MANUAL_ENTRY		10001


struct TextAttr TOPAZ80 = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};

SHORT ErrBorderVectors0[] = {
	0,0,
	96,0,
	96,18,
	0,18,
	0,0
};

struct Border ErrBorder0 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors0,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText0 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	15,5,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"NO ERROR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget49 = {
	NULL,	/* next gadget */
	264,179,	/* origin XY of hit box relative to window TopLeft */
	93,19,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder0,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText0,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors1[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder1 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors1,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText1 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget48 = {
	&ErrGadget49,	/* next gadget */
	217,166,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText1,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors2[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder2 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors2,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText2 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget47 = {
	&ErrGadget48,	/* next gadget */
	428,166,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder2,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText2,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors3[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder3 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors3,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText3 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget46 = {
	&ErrGadget47,	/* next gadget */
	428,155,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder3,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText3,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors4[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder4 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors4,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText4 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget45 = {
	&ErrGadget46,	/* next gadget */
	428,144,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder4,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText4,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors5[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder5 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors5,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText5 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget44 = {
	&ErrGadget45,	/* next gadget */
	428,133,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder5,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText5,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors6[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder6 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors6,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText6 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget43 = {
	&ErrGadget44,	/* next gadget */
	428,122,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder6,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText6,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors7[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder7 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors7,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText7 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget42 = {
	&ErrGadget43,	/* next gadget */
	428,111,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder7,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText7,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors8[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder8 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors8,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText8 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" UNLOCK ERROR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget41 = {
	&ErrGadget42,	/* next gadget */
	428,100,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder8,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText8,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors9[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder9 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors9,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText9 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" LOCK TIMEOUT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget40 = {
	&ErrGadget41,	/* next gadget */
	428,89,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder9,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText9,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors10[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder10 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors10,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText10 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" LOCK COLLISION",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget39 = {
	&ErrGadget40,	/* next gadget */
	428,78,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder10,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText10,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors11[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder11 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors11,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText11 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" RECORD NOT LOCKED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget38 = {
	&ErrGadget39,	/* next gadget */
	428,67,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder11,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText11,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors12[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder12 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors12,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText12 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" BAD HUNK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget37 = {
	&ErrGadget38,	/* next gadget */
	428,56,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder12,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText12,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors13[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder13 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors13,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText13 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT LINKED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget36 = {
	&ErrGadget37,	/* next gadget */
	428,45,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder13,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText13,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors14[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder14 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors14,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText14 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" IS SOFT LINK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget35 = {
	&ErrGadget36,	/* next gadget */
	428,34,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder14,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText14,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors15[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder15 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors15,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText15 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" WRITE PROTECTED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget34 = {
	&ErrGadget35,	/* next gadget */
	217,155,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder15,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText15,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	223,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors16[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder16 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors16,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText16 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" TOO MANY LEVELS",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget33 = {
	&ErrGadget34,	/* next gadget */
	217,144,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder16,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText16,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	217,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors17[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder17 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors17,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText17 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" TASK TABLE FULL",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget32 = {
	&ErrGadget33,	/* next gadget */
	217,133,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder17,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText17,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	105,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors18[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder18 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors18,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText18 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SEEK ERROR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget31 = {
	&ErrGadget32,	/* next gadget */
	217,122,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder18,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText18,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	219,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors19[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder19 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors19,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText19 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" RENAME ACROSS DEVICES",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget30 = {
	&ErrGadget31,	/* next gadget */
	217,111,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder19,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText19,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	215,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors20[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder20 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors20,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText20 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" READ PROTECTED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget29 = {
	&ErrGadget30,	/* next gadget */
	217,100,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder20,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText20,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	224,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors21[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder21 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors21,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText21 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT WRONG TYPE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget28 = {
	&ErrGadget29,	/* next gadget */
	217,89,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder21,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText21,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	212,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors22[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder22 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors22,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText22 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT TOO LARGE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget27 = {
	&ErrGadget28,	/* next gadget */
	217,78,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder22,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText22,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	207,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors23[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder23 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors23,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText23 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT NOT FOUND",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget26 = {
	&ErrGadget27,	/* next gadget */
	217,67,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder23,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText23,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	205,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors24[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder24 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors24,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText24 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT IN USE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget25 = {
	&ErrGadget26,	/* next gadget */
	217,56,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder24,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText24,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	202,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors25[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder25 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors25,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText25 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" OBJECT EXISTS",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget24 = {
	&ErrGadget25,	/* next gadget */
	217,45,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder25,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText25,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	203,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors26[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder26 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors26,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText26 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NO MORE ENTRIES",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget23 = {
	&ErrGadget24,	/* next gadget */
	217,34,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder26,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText26,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	232,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors27[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder27 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors27,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText27 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget22 = {
	&ErrGadget23,	/* next gadget */
	428,23,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder27,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText27,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors28[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder28 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors28,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText28 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NO DISK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget21 = {
	&ErrGadget22,	/* next gadget */
	217,23,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder28,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText28,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	226,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors29[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder29 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors29,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText29 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DELETE PROTECTED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget20 = {
	&ErrGadget21,	/* next gadget */
	6,45,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder29,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText29,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	222,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors30[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder30 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors30,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText30 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" COMMENT TOO BIG",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget19 = {
	&ErrGadget20,	/* next gadget */
	6,34,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder30,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText30,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	220,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors31[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder31 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors31,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText31 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" BAD STREAM NAME",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget18 = {
	&ErrGadget19,	/* next gadget */
	6,23,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder31,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText31,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	206,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors32[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder32 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors32,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText32 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NOT A DOS DISK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget17 = {
	&ErrGadget18,	/* next gadget */
	217,12,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder32,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText32,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	225,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors33[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder33 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors33,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText33 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget16 = {
	&ErrGadget17,	/* next gadget */
	428,12,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder33,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText33,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

UBYTE ErrGadget15SIBuff[4] =
	"000";
struct StringInfo ErrGadget15SInfo = {
	ErrGadget15SIBuff,	/* buffer where text will be edited */
	NULL,	/* optional undo buffer */
	0,	/* character position in buffer */
	4,	/* maximum number of characters to allow */
	0,	/* first displayed character buffer position */
	0,0,0,0,0,	/* Intuition initialized and maintained variables */
	0,	/* Rastport of gadget */
	0,	/* initial value for integer gadgets */
	NULL	/* alternate keymap (fill in if you set the flag) */
};

SHORT ErrBorderVectors34[] = {
	0,0,
	46,0,
	46,9,
	0,9,
	0,0
};
struct Border ErrBorder34 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors34,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText34 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget15 = {
	&ErrGadget16,	/* next gadget */
	530,186,	/* origin XY of hit box relative to window TopLeft */
	43,8,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+STRINGCENTER,	/* activation flags */
	STRGADGET,	/* gadget type flags */
	(APTR)&ErrBorder34,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText34,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	(APTR)&ErrGadget15SInfo,	/* SpecialInfo structure */
	MANUAL_ENTRY,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors35[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder35 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors35,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText35 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NO FREE STORE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget14 = {
	&ErrGadget16,	/* next gadget */
	6,188,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder35,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText35,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	103,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors36[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder36 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors36,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText36 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NO DEFAULT DIR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget13 = {
	&ErrGadget14,	/* next gadget */
	6,177,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder36,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText36,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	201,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors37[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder37 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors37,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText37 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" LINE TOO LONG",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget12 = {
	&ErrGadget13,	/* next gadget */
	6,166,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder37,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText37,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	120,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors38[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder38 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors38,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText38 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" INVALID RESIDENT LIBRARY",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget11 = {
	&ErrGadget12,	/* next gadget */
	6,155,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder38,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText38,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	122,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors39[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder39 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors39,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText39 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" INVALID LOCK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget10 = {
	&ErrGadget11,	/* next gadget */
	6,144,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder39,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText39,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	211,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors40[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder40 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors40,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText40 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" INVALID COMPONENT NAME",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget9 = {
	&ErrGadget10,	/* next gadget */
	6,133,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder40,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText40,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	210,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors41[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder41 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors41,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText41 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FILE NOT OBJECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget8 = {
	&ErrGadget9,	/* next gadget */
	6,122,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder41,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText41,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	121,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors42[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder42 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors42,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText42 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK WRITE PROTECTED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget7 = {
	&ErrGadget8,	/* next gadget */
	6,111,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder42,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText42,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	214,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors43[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder43 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors43,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText43 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK NOT VALIDATED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget6 = {
	&ErrGadget7,	/* next gadget */
	6,100,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder43,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText43,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	213,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors44[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder44 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors44,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText44 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK FULL",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget5 = {
	&ErrGadget6,	/* next gadget */
	6,89,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder44,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText44,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	221,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors45[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder45 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors45,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText45 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DIRECTORY NOT EMPTY",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget4 = {
	&ErrGadget5,	/* next gadget */
	6,78,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder45,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText45,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	216,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors46[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder46 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors46,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText46 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DIR NOT FOUND",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget3 = {
	&ErrGadget4,	/* next gadget */
	6,67,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder46,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText46,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	204,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors47[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder47 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors47,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText47 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DEVICE NOT MOUNTED",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget2 = {
	&ErrGadget3,	/* next gadget */
	6,56,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder47,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText47,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	218,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT ErrBorderVectors48[] = {
	0,0,
	210,0,
	210,10,
	0,10,
	0,0
};
struct Border ErrBorder48 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	ErrBorderVectors48,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText ErrIText48 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" ACTION NOT KNOWN",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget ErrGadget1 = {
	&ErrGadget2,	/* next gadget */
	6,12,	/* origin XY of hit box relative to window TopLeft */
	207,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&ErrBorder48,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&ErrIText48,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	209,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

#define GadgetList2 ErrGadget1

struct IntuiText ErrIText49 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	358,188,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Other Error Code",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

#define IntuiTextList1 ErrIText49

struct NewWindow nw2 = {
	0,0,	/* window XY origin relative to TopLeft of screen */
	640,200,	/* window width and height */
	0,1,	/* detail and block pens */
	GADGETUP,	/* IDCMP flags */
	NULL,	/* other window flags */
	&ErrGadget1,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	NULL,	/* window title */
	NULL,	/* custom screen pointer */
	NULL,	/* custom bitmap */
	640,200,	/* minimum width and height */
	640,200,	/* maximum width and height */
	WBENCHSCREEN	/* destination screen type */
};
