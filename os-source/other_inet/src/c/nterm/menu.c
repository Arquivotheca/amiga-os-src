/* the menu defines for the terminal program */

#include "st.h"
#include "graphics/text.h"
#include "colors.h"

/*extern struct TextAttr MyFont;*/

/* array of menu text */
struct IntuiText line_wraptext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"On",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Off",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText ctrl_charstext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Executed",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Visible",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText belltext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Both",			/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Visible",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Audible",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText columntext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"80",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"132",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText rowtext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"24",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"49",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText delandbksptext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,		/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Normal",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,		/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Swapped",		/* IText */
	NULL			/* NextText */
	}
};

struct IntuiText projecttext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Save Settings",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Load Settings",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Reset Term",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Send Break",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"About",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Quit",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText terminaltext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"TTY",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"VT52",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"VT100",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	CHECKWIDTH, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"AMIGA",		/* IText */
	NULL			/* NextText */
	}
};
struct IntuiText setuptext[] = {
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Line Wrap",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"CTRL Chars",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Tabs",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Bell",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Del & Bksp",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Columns",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Rows",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Set Colors",		/* IText */
	NULL			/* NextText */
	},
	{
	COLOR0,COLOR1,JAM2,	/* FrontPen, BackPen, DrawMode */	
	0, 0,			/* LeftEdge, TopEdge */
	NULL,			/* ItextFont */
	"Func Keys",		/* IText */
	NULL			/* NextText */
	}

};


/* arrays of menu subitems */
struct MenuItem line_wrapitems[] = {
	{
	&line_wrapitems[1], /* next item */
	96,0,CHECKWIDTH+24,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT, /* flags */
	0xFFFF-1,	/* mutual exclude */
	&line_wraptext[0],	/* itemfill (Yes) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	96,8,CHECKWIDTH+24,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&line_wraptext[1],	/* itemfill (No) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};
struct MenuItem ctrl_charsitems[] = {
	{
	&ctrl_charsitems[1], /* next item */
	88,0,CHECKWIDTH+64,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&ctrl_charstext[0],	/* itemfill (Executed) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	88,8,CHECKWIDTH+64,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&ctrl_charstext[1],	/* itemfill (Visible) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};
struct MenuItem bellitems[] = {
	{
	&bellitems[1], /* next item */
	88,0,CHECKWIDTH+64,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&belltext[0],	/* itemfill (Audible) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&bellitems[2], /* next item */
	88,8,CHECKWIDTH+64,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&belltext[1],	/* itemfill (Silent) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	88,16,CHECKWIDTH+64,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-4,	/* mutual exclude */
	&belltext[2],	/* itemfill (No Flash) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};
struct MenuItem columnitems[] = {
	{
	&columnitems[1], /* next item */
	88,0,CHECKWIDTH+24,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&columntext[0],	/* itemfill (80) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	88,8,CHECKWIDTH+24,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&columntext[1],	/* itemfill (132) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};
struct MenuItem rowitems[] = {
	{
	&rowitems[1], /* next item */
	88,0,CHECKWIDTH+16,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&rowtext[0],	/* itemfill (24) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	88,8,CHECKWIDTH+16,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&rowtext[1],	/* itemfill (49) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};

struct MenuItem delandbkspitems[] = {
	{
	&delandbkspitems[1], /* next item */
	96,0,CHECKWIDTH+56,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&delandbksptext[0],	/* itemfill (Normal) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL, /* next item */
	96,8,CHECKWIDTH+56,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&delandbksptext[1],	/* itemfill (Swapped) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};

/* arrays of menu items */
struct MenuItem projectitems[] = {
	{
	&projectitems[1],	/* next item */
	0,0,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&projecttext[0],	/* itemfill (Save Settings) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&projectitems[2],	/* next item */
	0,10,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&projecttext[1],	/* itemfill (Load Settings) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&projectitems[3],	/* next item */
	0,20,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|COMMSEQ,	/* flags */
	0,	/* mutual exclude */
	&projecttext[2],	/* itemfill (Reset) */
	NULL,	/* selectfill */
	'.',	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&projectitems[4],	/* next item */
	0,30,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|COMMSEQ,	/* flags */
	0,	/* mutual exclude */
	&projecttext[3],	/* itemfill (Break) */
	NULL,	/* selectfill */
	'b',	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&projectitems[5],	/* next item */
	0,40,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&projecttext[4],	/* itemfill (Whodunit) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL,	/* next item */
	0,50,112+COMMWIDTH,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|COMMSEQ,	/* flags */
	0,	/* mutual exclude */
	&projecttext[5],	/* itemfill (Quit) */
	NULL,	/* selectfill */
	'q',	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};

struct MenuItem terminalitems[] = {
	{
	&terminalitems[1],	/* next item */
	0,0,CHECKWIDTH+40,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-1,	/* mutual exclude */
	&terminaltext[0],	/* itemfill (TTY) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&terminalitems[2],	/* next item */
	0,10,CHECKWIDTH+40,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-2,	/* mutual exclude */
	&terminaltext[1],	/* itemfill (VT52) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&terminalitems[3],	/* next item */
	0,20,CHECKWIDTH+40,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT|CHECKED,	/* flags */
	0xFFFF-4,	/* mutual exclude */
	&terminaltext[2],	/* itemfill (VT100) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL,	/* next item */
	0,30,CHECKWIDTH+40,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|CHECKIT,	/* flags */
	0xFFFF-8,	/* mutual exclude */
	&terminaltext[3],	/* itemfill (AMIGA) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}
};
struct MenuItem setupitems[] = {
	{
	&setupitems[1],	/* next item */
	0,0,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[0],	/* itemfill (Line Wrap) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&line_wrapitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[2],	/* next item */
	0,10,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[1],	/* itemfill (CTRL Chars) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&ctrl_charsitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[3],	/* next item */
	0,20,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[2],	/* itemfill (Tabs) */
	NULL,	/* selectfill */
	NULL,	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[4],	/* next item */
	0,30,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[3],	/* itemfill (Bell) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&bellitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[5],	/* next item */
	0,40,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[4],	/* itemfill (Del & Bksp) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&delandbkspitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[6],	/* next item */
	0,50,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[5],	/* itemfill (Columns) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&columnitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[7],	/* next item */
	0,60,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP,	/* flags */
	0,	/* mutual exclude */
	&setuptext[6],	/* itemfill (Rows) */
	NULL,	/* selectfill */
	NULL,	/* command */
	&rowitems[0],	/* subitem */
	NULL	/* nextselect */
	},
	{
	&setupitems[8],	/* next item */
	0,70,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|COMMSEQ,	/* flags */
	0,	/* mutual exclude */
	&setuptext[7],	/* itemfill (Colors) */
	NULL,	/* selectfill */
	'c',	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	},
	{
	NULL,	/* next item */
	0,80,120,8,	/* left, top, width, height */
	ITEMENABLED|ITEMTEXT|HIGHCOMP|COMMSEQ,	/* flags */
	0,	/* mutual exclude */
	&setuptext[8],	/* itemfill (Function Keys) */
	NULL,	/* selectfill */
	'f',	/* command */
	NULL,	/* subitem */
	NULL	/* nextselect */
	}

};

/* an array of the main menus */
struct Menu menus[] = {
	{
	&menus[1],			/* next menu */
	0, 0, 60, 0,	/* left, top, width, height */
	MENUENABLED,		/* flags */
	"Project",			/* name */
	&projectitems[0]		/* first item */
	},
	{
	&menus[2],			/* next menu */
	90, 0, 68, 0,	/* left, top, width, height */
	MENUENABLED,		/* flags */
	"Terminal",			/* name */
	&terminalitems[0]		/* first item */
	},
	{
	NULL,			/* next menu */
	180, 0, 44, 0,	/* left, top, width, height */
	MENUENABLED,		/* flags */
	"SetUp",			/* name */
	&setupitems[0]		/* first item */
	}
};
