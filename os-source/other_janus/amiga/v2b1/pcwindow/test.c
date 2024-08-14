
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

USHORT BorderVectors1[] = {
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
	3,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-204,-10,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"BETWEEN TRANSMISSION OF KEY EVENTS:",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct IntuiText IText1 = {
	3,0,JAM2,	/* front and back text pens, drawmode and fill byte */
	-151,-19,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"ENTER NUMBER OF MICROSECONDS",	/* pointer to text */
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
	75,85,	/* window XY origin relative to TopLeft of screen */
	294,44,	/* window width and height */
	0,1,	/* detail and block pens */
	NULL,	/* IDCMP flags */
	NULL,	/* other window flags */
	&AdjustMicros,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	"Your new window",	/* window title */
	NULL,	/* custom screen pointer */
	NULL,	/* custom bitmap */
	5,5,	/* minimum width and height */
	640,200,	/* maximum width and height */
	WBENCHSCREEN	/* destination screen type */
};

/* end of PowerWindows source generation */
