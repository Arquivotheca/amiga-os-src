#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/text.h>

struct TextAttr CommonFont = {
	"topaz.font",			/* font we want */
	9,				/* YSize */
	FS_NORMAL,FPF_ROMFONT		/* style,flags */
};

struct IntuiText CancelText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	13,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intuitext */
	"CANCEL",			/* text string */
	0				/* next text */
};

struct IntuiText UseText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* draw mode */
	28,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intuitext */
	"USE",				/* text string */
	0				/* next text */
};

struct IntuiText SaveText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	23,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intuitext */
	"SAVE",				/* text string */
	0				/* next text */
};

struct IntuiText OffText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	145,3,				/* leftedge,topedge */
	&CommonFont,			/* font */
	"OFF",				/* text string */
	0				/* next text */
};

struct IntuiText OnText	= {
	1,2,				/* fontpen,backpen */
	JAM2,			/* drawmode */
	145,3,				/* leftedge,topedge */
	&CommonFont,
	"ON ",
	0
};

struct IntuiText MonoText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	10,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intutitext */
	"Mono video",			/* text string */
	0				/* next text */
};
		
struct IntuiText ColorText = {
	2,1,				/* frontpen,backpen */
	JAM2,
	10,3,				/* leftedge,topedge */
	&CommonFont,			/* font  */
	"Color video",			/* text string */
	0				/* next text */
};

#if 0
struct IntuiText SerialText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	10,3,				/* leftedge,topedge */
	&CommonFont,			/* font */
	"Serial port",			/* text string */
	0				/* next text */
};
#endif

struct IntuiText ShadowText = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	10,3,				/* leftedge,topedge */
	&CommonFont,			/* font */
	"Shadow Janus",			/* text string */
	0				/* next text */
};

struct IntuiText A000Text = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	4,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intuitext */
	"RAM=A000",			/* text string */
	0				/* next text */
};

struct IntuiText E000Text = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	4,3,				/* leftedge,topedge */
	&CommonFont,			/* font for all intuitext */
	"RAM=E000",			/* text string */
	0				/* next text */
};

struct IntuiText D000Text = {
	2,1,				/* frontpen,backpen */
	JAM2,			/* drawmode */
	4,3,				/* leftedge topedge */
	&CommonFont,			/* font for all intuitext */
	"RAM=D000",			/* text string */
	0				/* next text */
};

struct Image CommonImage = {
	0,0,90,13,			/* left,top,width,height */
	2,				/* depth */
	0,				/* no actual image */
	0,1,				/* planepick,planeoff */
	0				/* no next image */
};

struct Image WideImage = {
	0,0,180,13,			/* left,top,width,height */
	2,				/* depth */
	0,				/* no actual image */
	0,1,				/* planepick,planeoff */
	0				/* next image */
};


/* These gadgets can stay here in any kind of memory since they are always
 * present and contain no actual imagery that needs to be in chip memory.
 */
#if 0
struct Gadget SerialGadget = {
	0,				/* next gadget */
	112,66,180,13,			/* left,top,width,height */
	GADGIMAGE | GADGHCOMP,		/* flags */
	RELVERIFY | TOGGLESELECT,		/* activation flags */
	BOOLGADGET,			/* gadget type */
	&WideImage,
	0,				/* no alternate image */
	&SerialText,			/* text for this gadget */
	0,
	0,
	5,				/* gadgetID */
	0				/* user data field */
};
#endif

struct Gadget ShadowGadget = {
	0,				/* next gadget */
	112,66,180,13,			/* left,top,width,height */
	GADGIMAGE | GADGHCOMP,		/* flags */
	RELVERIFY | TOGGLESELECT,		/* activation flags */
	BOOLGADGET,			/* gadget type */
	&WideImage,
	0,				/* no alternate image */
	&ShadowText,			/* text for this gadget */
	0,
	0,
	5,				/* gadgetID */
	0				/* user data field */
};

struct Gadget ColorGadget = {
#if 0
	&SerialGadget,			/* next gadget */
#else
	&ShadowGadget,
#endif
	112,50,180,13,			/* left,top,width,height */
	GADGIMAGE | GADGHCOMP,		/* flags */
	RELVERIFY | TOGGLESELECT,		/* activation flags */
	BOOLGADGET,			/* gadgettype */
	&WideImage,			/* gadget imagery */
	0,				/* no alternate image */
	&ColorText,			/* text for this gadget */
	0,
	0,
	4,				/* gadget ID */
	0				/* user data field */
};

struct Gadget MonoGadget = {
	&ColorGadget,			/* next gadget */
	112,34,180,13,			/* left,top,width,height */
	GADGIMAGE | GADGHCOMP,		/* flags */
	RELVERIFY | TOGGLESELECT,		/* activation flags */
	BOOLGADGET,			/* gadget type */
	&WideImage,			/* imagery when unselected */
	0,				/* no alternate image */
	&MonoText,			/* text for this gadget */
	0,				/* mutual exclude */
	0,
	3,				/* gadgetID */
	0				/* user data field */
};

/* A large software fix is required to get BOOLGADGET gadgets working
 * with a complemented box around them.  Intuition has major problems when
 * trying to refresh these gadgets and often leaves bits of boxes lying
 * around the screen.  Mutualexclude and RefreshGadgets also appear to be
 * completely broken so I have done all the highlighting myself.
 */

struct Gadget E000Gadget = {
	&MonoGadget,			/* next gadget */
	8,66,90,13,			/* left,top,width,height */
	GADGIMAGE | GADGHNONE,		/* flags */
	GADGIMMEDIATE | TOGGLESELECT,		/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&E000Text,			/* text for this gadget */
	0,				/* mutual exclusion */
	0,
	3,				/* gadget ID (RAM bank) */
	0				/* user data field */
};

struct Gadget D000Gadget = {
	&E000Gadget,			/* next gadget */
	8,50,90,13,			/* left,top,width,height */
	GADGIMAGE | GADGHNONE,		/* flags */
	GADGIMMEDIATE | TOGGLESELECT,	/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&D000Text,			/* text for this gadget */
	0,				/* mutual exclusion */
	0,
	2,				/* gadget ID (RAM bank) */
	0				/* user data field */
};

struct Gadget A000Gadget = {
	&D000Gadget,			/* next gadget */
	8,34,90,13,			/* left top width height */
	GADGIMAGE | GADGHNONE,		/* flags */
	GADGIMMEDIATE | TOGGLESELECT,	/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&A000Text,			/* text for this gadget */
	0,				/* mutual exclusion */
	0,				/* specialinfo */
	1,				/* gadget ID (RAM bank) */
	0				/* user data field */
};

struct Gadget SaveGadget = {
	&A000Gadget,			/* next gadget */
	202,14,90,13,			/* left,top,width,height */
	GADGIMAGE | GADGHBOX,		/* flags */
	RELVERIFY,			/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&SaveText,			/* text for this gadget */
	0,
	0,
	2,				/* gadget ID */
	0				/* user data field */
};

struct Gadget UseGadget = {
	&SaveGadget,			/* next gadget */
	105,14,90,13,			/* left,top,width,height */
	GADGIMAGE | GADGHBOX,		/* flags */
	RELVERIFY,			/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&UseText,			/* text for this gadget */
	0,				/* mutualexclude */
	0,				/* specialinfo */
	1,				/* gadget ID */
	0				/* user data field */
};

struct Gadget CancelGadget = {
	&UseGadget,			/* next gadget */
	8,14,90,13,			/* left,top,width,height */
	GADGIMAGE | GADGHBOX,		/* flags */
	RELVERIFY,			/* activation flags */
	BOOLGADGET,			/* gadget type */
	&CommonImage,			/* imagery for this gadget */
	0,				/* no alternate image */
	&CancelText,			/* text for this gadget */
	0,				/* mutualexclude */
	0,				/* specialinfo */
	0,				/* Gadget ID */
	0				/* user data field */
};

