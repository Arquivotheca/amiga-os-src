#define P_EXIT 9997

SHORT PktBorderVectors1[] = {
	0,0,
	57,0,
	57,23,
	0,23,
	0,0
};
struct Border PktBorder1 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors1,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText1 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	4,7,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EXIT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget77 = {
	NULL,	/* next gadget */
	538,138,	/* origin XY of hit box relative to window TopLeft */
	54,22,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText1,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	P_EXIT,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors2[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder2 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors2,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText2 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget76 = {
	&PktGadget77,	/* next gadget */
	502,89,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder2,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText2,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors3[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder3 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors3,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText3 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget75 = {
	&PktGadget76,	/* next gadget */
	502,78,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder3,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText3,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors4[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder4 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors4,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText4 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget74 = {
	&PktGadget75,	/* next gadget */
	502,67,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder4,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText4,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors5[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder5 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors5,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText5 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget73 = {
	&PktGadget74,	/* next gadget */
	502,56,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder5,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText5,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors6[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder6 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors6,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText6 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget72 = {
	&PktGadget73,	/* next gadget */
	502,45,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder6,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText6,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors7[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder7 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors7,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText7 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget71 = {
	&PktGadget72,	/* next gadget */
	502,34,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder7,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText7,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors8[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder8 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors8,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText8 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget70 = {
	&PktGadget71,	/* next gadget */
	502,23,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder8,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText8,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors9[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder9 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors9,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText9 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget69 = {
	&PktGadget70,	/* next gadget */
	378,188,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder9,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText9,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors10[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder10 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors10,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText10 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget68 = {
	&PktGadget69,	/* next gadget */
	378,177,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder10,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText10,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors11[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder11 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors11,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText11 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget67 = {
	&PktGadget68,	/* next gadget */
	378,166,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder11,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText11,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors12[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder12 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors12,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText12 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget66 = {
	&PktGadget67,	/* next gadget */
	378,155,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder12,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText12,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors13[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder13 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors13,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText13 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget65 = {
	&PktGadget66,	/* next gadget */
	378,144,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder13,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText13,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors14[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder14 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors14,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText14 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget64 = {
	&PktGadget65,	/* next gadget */
	378,133,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder14,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText14,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors15[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder15 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors15,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText15 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget63 = {
	&PktGadget64,	/* next gadget */
	378,122,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder15,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText15,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors16[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder16 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors16,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText16 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget62 = {
	&PktGadget63,	/* next gadget */
	378,111,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder16,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText16,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors17[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder17 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors17,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText17 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget61 = {
	&PktGadget62,	/* next gadget */
	378,100,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder17,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText17,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors18[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder18 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors18,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText18 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget60 = {
	&PktGadget61,	/* next gadget */
	378,89,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder18,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText18,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors19[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder19 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors19,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText19 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget59 = {
	&PktGadget60,	/* next gadget */
	378,78,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder19,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText19,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors20[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder20 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors20,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText20 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget58 = {
	&PktGadget59,	/* next gadget */
	378,67,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder20,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText20,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors21[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder21 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors21,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText21 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" REMOVE NOTIFY",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget57 = {
	&PktGadget58,	/* next gadget */
	378,56,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder21,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText21,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	4098,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors22[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder22 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors22,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText22 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" ADD NOTIFY",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget56 = {
	&PktGadget57,	/* next gadget */
	378,45,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder22,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText22,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	4097,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors23[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder23 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors23,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText23 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FREE RECORD",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget55 = {
	&PktGadget56,	/* next gadget */
	378,34,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder23,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText23,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	2009,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors24[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder24 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors24,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText24 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EXAMINE ALL",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget54 = {
	&PktGadget55,	/* next gadget */
	254,188,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder24,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText24,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1033,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors25[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder25 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors25,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText25 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" PARENT FH",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget53 = {
	&PktGadget54,	/* next gadget */
	254,177,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder25,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText25,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1031,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors26[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder26 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors26,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText26 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" COPY DIR FH",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget52 = {
	&PktGadget53,	/* next gadget */
	254,166,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder26,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText26,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1030,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors27[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder27 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors27,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText27 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" LOCK RECORD",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget51 = {
	&PktGadget52,	/* next gadget */
	378,23,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder27,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText27,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	2008,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors28[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder28 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors28,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText28 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" CHANGE MODE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget50 = {
	&PktGadget51,	/* next gadget */
	254,155,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder28,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText28,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1028,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors29[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder29 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors29,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText29 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" IS FILESYSTEM",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget49 = {
	&PktGadget50,	/* next gadget */
	254,144,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder29,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText29,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1027,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors30[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder30 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors30,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText30 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FH FROM LOCK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget48 = {
	&PktGadget49,	/* next gadget */
	254,133,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder30,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText30,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1026,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors31[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder31 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors31,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText31 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" READ LINK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget47 = {
	&PktGadget48,	/* next gadget */
	254,122,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder31,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText31,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1024,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors32[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder32 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors32,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText32 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" MAKE LINK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget46 = {
	&PktGadget47,	/* next gadget */
	254,111,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder32,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText32,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1021,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors33[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder33 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors33,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText33 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FORMAT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget45 = {
	&PktGadget46,	/* next gadget */
	254,100,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder33,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText33,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1020,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors34[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder34 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors34,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText34 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SAME LOCK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget44 = {
	&PktGadget45,	/* next gadget */
	254,89,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder34,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText34,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	40,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors35[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder35 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors35,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText35 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" WRITE PROTECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget43 = {
	&PktGadget44,	/* next gadget */
	254,78,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder35,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText35,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1023,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors36[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder36 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors36,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText36 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SET FILE SIZE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget42 = {
	&PktGadget43,	/* next gadget */
	254,67,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder36,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText36,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1022,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors37[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder37 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors37,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText37 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" END",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget41 = {
	&PktGadget42,	/* next gadget */
	254,56,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder37,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText37,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1007,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors38[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder38 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors38,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText38 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FINDOUTPUT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget40 = {
	&PktGadget41,	/* next gadget */
	254,45,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder38,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText38,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1006,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors39[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder39 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors39,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText39 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FINDINPUT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget39 = {
	&PktGadget40,	/* next gadget */
	254,34,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder39,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText39,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1005,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors40[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder40 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors40,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText40 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" READ RETURN",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget38 = {
	&PktGadget39,	/* next gadget */
	130,177,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder40,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText40,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1001,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors41[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder41 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors41,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText41 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SCREEN MODE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget37 = {
	&PktGadget38,	/* next gadget */
	130,166,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder41,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText41,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	994,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors42[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder42 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors42,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText42 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SET DATE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget36 = {
	&PktGadget37,	/* next gadget */
	130,155,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder42,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText42,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	34,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors43[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder43 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors43,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText43 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FINDUPDATE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget35 = {
	&PktGadget36,	/* next gadget */
	254,23,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder43,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText43,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1004,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors44[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder44 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors44,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText44 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK CHANGE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget34 = {
	&PktGadget35,	/* next gadget */
	130,144,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder44,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText44,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	33,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors45[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder45 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors45,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText45 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK TYPE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget33 = {
	&PktGadget34,	/* next gadget */
	130,133,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder45,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText45,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	32,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors46[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder46 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors46,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText46 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" INHIBIT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget32 = {
	&PktGadget33,	/* next gadget */
	130,122,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder46,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText46,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	31,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors47[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder47 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors47,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText47 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" TIMER",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget31 = {
	&PktGadget32,	/* next gadget */
	130,111,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder47,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText47,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	30,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors48[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder48 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors48,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText48 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" PARENT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget30 = {
	&PktGadget31,	/* next gadget */
	130,100,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder48,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText48,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	29,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors49[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder49 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors49,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText49 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SET COMMENT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget29 = {
	&PktGadget30,	/* next gadget */
	130,89,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder49,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText49,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	28,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors50[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder50 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors50,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText50 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FLUSH",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget28 = {
	&PktGadget29,	/* next gadget */
	130,78,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder50,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText50,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	27,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors51[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder51 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors51,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText51 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" INFO",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget27 = {
	&PktGadget28,	/* next gadget */
	130,67,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder51,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText51,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	26,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors52[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder52 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors52,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText52 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DISK INFO",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget26 = {
	&PktGadget27,	/* next gadget */
	130,56,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder52,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText52,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	25,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors53[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder53 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors53,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText53 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EXAMINE NEXT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget25 = {
	&PktGadget26,	/* next gadget */
	130,45,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder53,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText53,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	24,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors54[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder54 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors54,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText54 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EXAMINE OBJECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget24 = {
	&PktGadget25,	/* next gadget */
	130,34,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder54,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText54,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	23,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors55[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder55 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors55,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText55 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" CREATE DIR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget23 = {
	&PktGadget24,	/* next gadget */
	130,23,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder55,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText55,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	22,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors56[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder56 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors56,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText56 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" WRITE RETURN",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget22 = {
	&PktGadget23,	/* next gadget */
	130,188,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder56,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText56,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1002,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors57[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder57 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors57,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText57 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" COPY DIR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget21 = {
	&PktGadget22,	/* next gadget */
	6,177,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder57,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText57,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	19,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors58[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder58 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors58,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText58 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" MORE CACHE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget20 = {
	&PktGadget21,	/* next gadget */
	6,166,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder58,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText58,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	18,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors59[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder59 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors59,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText59 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" RENAME OBJECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget19 = {
	&PktGadget20,	/* next gadget */
	6,155,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder59,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText59,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	9,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors60[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder60 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors60,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText60 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DELETE OBJECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget18 = {
	&PktGadget19,	/* next gadget */
	6,144,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder60,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText60,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	16,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors61[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder61 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors61,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText61 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" FREE LOCK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget17 = {
	&PktGadget18,	/* next gadget */
	6,133,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder61,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText61,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	15,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors62[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder62 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors62,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText62 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" READ",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget16 = {
	&PktGadget17,	/* next gadget */
	6,122,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder62,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText62,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	'R',	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors63[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder63 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors63,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText63 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" WRITE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget15 = {
	&PktGadget16,	/* next gadget */
	6,111,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder63,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText63,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	'W',	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors64[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder64 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors64,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText64 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" RENAME DISK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget14 = {
	&PktGadget15,	/* next gadget */
	6,100,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder64,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText64,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	9,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors65[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder65 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors65,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText65 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" LOCATE OBJECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget13 = {
	&PktGadget14,	/* next gadget */
	6,89,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder65,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText65,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	8,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors66[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder66 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors66,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText66 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" CURRENT VOLUME",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget12 = {
	&PktGadget13,	/* next gadget */
	6,78,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder66,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText66,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	7,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors67[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder67 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors67,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText67 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EVENT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget11 = {
	&PktGadget12,	/* next gadget */
	6,67,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder67,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText67,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	6,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors68[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder68 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors68,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText68 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" DIE",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget10 = {
	&PktGadget11,	/* next gadget */
	6,56,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder68,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText68,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	5,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors69[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder69 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors69,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText69 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SET MAP",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget9 = {
	&PktGadget10,	/* next gadget */
	6,45,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder69,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText69,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	4,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors70[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder70 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors70,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText70 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" GET BLOCK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget8 = {
	&PktGadget9,	/* next gadget */
	6,34,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder70,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText70,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	2,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors71[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder71 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors71,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText71 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" STARTUP",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget7 = {
	&PktGadget8,	/* next gadget */
	6,23,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder71,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText71,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	9998,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors72[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder72 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors72,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText72 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget6 = {
	&PktGadget7,	/* next gadget */
	502,12,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder72,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText72,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	NULL,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors73[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder73 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors73,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText73 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" EXAMINE FH",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget5 = {
	&PktGadget6,	/* next gadget */
	378,12,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder73,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText73,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1034,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors74[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder74 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors74,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText74 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SEEK",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget4 = {
	&PktGadget5,	/* next gadget */
	254,12,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder74,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText74,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1008,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors75[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder75 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors75,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText75 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" SET PROTECT",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget3 = {
	&PktGadget4,	/* next gadget */
	130,12,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder75,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText75,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	21,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors76[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder76 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors76,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText76 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" WAIT CHAR",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget2 = {
	&PktGadget3,	/* next gadget */
	6,188,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder76,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText76,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	20,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

SHORT PktBorderVectors77[] = {
	0,0,
	122,0,
	122,10,
	0,10,
	0,0
};
struct Border PktBorder77 = {
	-2,-1,	/* XY origin relative to container TopLeft */
	2,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	PktBorderVectors77,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText PktIText77 = {
	1,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-1,1,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	" NIL",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget PktGadget1 = {
	&PktGadget2,	/* next gadget */
	6,12,	/* origin XY of hit box relative to window TopLeft */
	119,9,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY+TOGGLESELECT,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&PktBorder77,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&PktIText77,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	9999,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

#define PktGadgetList1 PktGadget1

struct NewWindow nw3 = {
	0,0,	/* window XY origin relative to TopLeft of screen */
	626,200,	/* window width and height */
	0,1,	/* detail and block pens */
	GADGETUP,	/* IDCMP flags */
	NULL,	/* other window flags */
	&PktGadget1,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	NULL,	/* window title */
	NULL,	/* custom screen pointer */
	NULL,	/* custom bitmap */
	626,200,	/* minimum width and height */
	626,200,	/* maximum width and height */
	WBENCHSCREEN	/* destination screen type */
};


