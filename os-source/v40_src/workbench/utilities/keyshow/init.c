
#include "keytoy.h"
#include "keyboard.h"

extern struct Window *window;
extern struct RastPort *rport;         /* pointer to clock-window RastPort */
extern struct Remember *RememberKey;
extern struct TextAttr Topaz;
extern struct TextAttr TopazI;
extern struct KeyTop Key[];
extern WORD BoardType;

extern struct Gadget LAlt;
extern struct Gadget LShift;

WORD Key1XY[12] = {	/* such as 'Q' key */
    0, 0,
    0, KEYHEIGHT,
    K1, KEYHEIGHT,
    K1, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key2XY[12] = {	/* such as a function key */
    0, 0,
    0, KEYHEIGHT,
    K2, KEYHEIGHT,
    K2, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key3XY[12] = {	/* such as european left-shift key */
    0, 0,
    0, KEYHEIGHT,
    K3, KEYHEIGHT,
    K3, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key4XY[12] = {	/* such as backspace key */
    0, 0,
    0, KEYHEIGHT,
    K4, KEYHEIGHT,
    K4, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key5XY[12] = {	/* such as right-shift key */
    0, 0,
    0, KEYHEIGHT,
    K5, KEYHEIGHT,
    K5, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key6XY[12] = {	/* space bar */
    0, 0,
    0, KEYHEIGHT,
    K6, KEYHEIGHT,
    K6, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD Key7XY[20] = {	/* USA return key */
    0, 0,
    0, ADDROW,
    -(K1-3), ADDROW,
    -(K1-3), KEYHEIGHT+ADDROW,
    K3-2, KEYHEIGHT+ADDROW,
    K3-2, 0,
    1, 0,
    1, ADDROW,
    1-(K1-3), ADDROW,
    1-(K1-3), KEYHEIGHT+ADDROW
};

WORD Key8XY[20] = {	/* European return key */
    0, 0,
    0, KEYHEIGHT,
    5, KEYHEIGHT,
    5, KEYHEIGHT+ADDROW,
    35, KEYHEIGHT+ADDROW,
    35, 0,
    1, 0,
    1, KEYHEIGHT,
    6, KEYHEIGHT,
    6, KEYHEIGHT+ADDROW
};

WORD Key9XY[12] = {	/* USA left shift */
    0, 0,
    0, KEYHEIGHT,
    K9, KEYHEIGHT,
    K9, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD KeyAXY[12] = {	/* Enter Key */
    0, 0,
    0, KEYHEIGHT+ADDROW,
    K1, KEYHEIGHT+ADDROW,
    K1, 0,
    1, 0,
    1, KEYHEIGHT+ADDROW
};

WORD KeyBXY[12] = {	/* Right Shift Key */
    0, 0,
    0, KEYHEIGHT,
    KB, KEYHEIGHT,
    KB, 0,
    1, 0,
    1, KEYHEIGHT
};

WORD KeyCXY[12] = {	/* Alts */
    0, 0,
    0, KEYHEIGHT,
    KC, KEYHEIGHT,
    KC, 0,
    1, 0,
    1, KEYHEIGHT
};

struct Image DelImage = {K46X+2, K46Y+1, 22, 9, 1, NULL, 0x02, 0x00,
    NULL};
struct Image HelpImage = {K5FX+2, K5FY+1, 22, 9, 1, NULL, 0x02, 0x00,
    &DelImage};
struct Image CapsImage = {K62X+2, K62Y+1, 22, 9, 1, NULL, 0x02, 0x00,
    &HelpImage};
struct Image LeftImage = {K4FX+2, K4FY+1, 22, 9, 1, NULL, 0x02, 0x00,
    &CapsImage};
struct Image RightImage = {K4EX+2, K4FY+1, 22, 9, 1, NULL, 0x02, 0x00,
    &LeftImage};
struct Image DownImage = {K4DX+2, K4DY+1, 22, 9, 1, NULL, 0x02, 0x00,
    &RightImage};
struct Image UpImage = {K4CX+2, K4CY+1, 22, 9, 1, NULL, 0x02, 0x00,
    &DownImage};

struct Image CtrlImage = {0, 0, 22, 9, 1, NULL, 0x1, 0x1, NULL};
struct Image AltImage = {0, 0, 30, 9, 1, NULL, 0x1, 0x1, NULL};
struct Image RShiftImage = {0, 0, 67, 9, 1, NULL, 0x1, 0x1, NULL};
struct Image ELShiftImage = {0, 0, 35, 9, 1, NULL, 0x1, 0x1, NULL};
struct Image ULShiftImage = {0, 0, 61, 9, 1, NULL, 0x1, 0x1, NULL};


struct Gadget RAlt = {
    NULL,		/* NextGadget */
    K65X+2, K65Y+1,	/* LeftEdge, TopEdge */
    KC-2, KEYHEIGHT-1,	/* Width, Height */
    GADGHCOMP|GADGIMAGE,		/* Flags */
    GADGIMMEDIATE|TOGGLESELECT,	/* Activation */
    BOOLGADGET,		/* GadgetType */
    &AltImage,		/* GadgetRender */
    NULL,		/* SelectRender */
    NULL,		/* GadgetText */
    0,			/* MutualExclude */
    NULL,		/* SpecialInfo */
    RALT,		/* GadgetID */
    &LAlt		/* UserData */
};

struct Gadget LAlt = {
    &RAlt,		/* NextGadget */
    K64X+2, K64Y+1,	/* LeftEdge, TopEdge */
    KC-2, KEYHEIGHT-1,	/* Width, Height */
    GADGHCOMP|GADGIMAGE,		/* Flags */
    GADGIMMEDIATE|TOGGLESELECT,	/* Activation */
    BOOLGADGET,		/* GadgetType */
    &AltImage,		/* GadgetRender */
    NULL,		/* SelectRender */
    NULL,		/* GadgetText */
    0,			/* MutualExclude */
    NULL,		/* SpecialInfo */
    LALT,		/* GadgetID */
    &RAlt		/* UserData */
};

struct Gadget Contr = {
    &LAlt,		/* NextGadget */
    K63X+2, K63Y+1,	/* LeftEdge, TopEdge */
    K1-2, KEYHEIGHT-1,	/* Width, Height */
    GADGHCOMP|GADGIMAGE,		/* Flags */
    GADGIMMEDIATE|TOGGLESELECT,	/* Activation */
    BOOLGADGET,		/* GadgetType */
    &CtrlImage,		/* GadgetRender */
    NULL,		/* SelectRender */
    NULL,		/* GadgetText */
    0,			/* MutualExclude */
    NULL,		/* SpecialInfo */
    CONTR,		/* GadgetID */
    NULL		/* UserData */
};

struct Gadget RShift = {
    &Contr,		/* NextGadget */
    K61X+2, K61Y+1,	/* LeftEdge, TopEdge */
    KB-2, KEYHEIGHT-1,	/* Width, Height */
    GADGHCOMP|GADGIMAGE,	/* Flags */
    GADGIMMEDIATE|TOGGLESELECT,	/* Activation */
    BOOLGADGET,		/* GadgetType */
    &RShiftImage,	/* GadgetRender */
    NULL,		/* SelectRender */
    NULL,		/* GadgetText */
    0,			/* MutualExclude */
    NULL,		/* SpecialInfo */
    RSHIFT,		/* GadgetID */
    &LShift		/* UserData */
};

struct Gadget LShift = {
    &RShift,		/* NextGadget */
    K60X+2, K60Y+1,	/* LeftEdge, TopEdge */
    K3-2, KEYHEIGHT-1,	/* Width, Height */
    GADGHCOMP|GADGIMAGE,		/* Flags */
    GADGIMMEDIATE|TOGGLESELECT,	/* Activation */
    BOOLGADGET,		/* GadgetType */
    &ELShiftImage,	/* GadgetRender */
    NULL,		/* SelectRender */
    NULL,		/* GadgetText */
    0,			/* MutualExclude */
    NULL,		/* SpecialInfo */
    LSHIFT,		/* GadgetID */
    &RShift		/* UserData */
};

WORD KeysLE[NUMKEYS]  =   { K00X, K01X, K02X, K03X, K04X, K05X, K06X, K07X,
    K08X, K09X, K0AX, K0BX, K0CX, K0DX, K0EX, K0FX, K10X, K11X, K12X, K13X,
    K14X, K15X, K16X, K17X, K18X, K19X, K1AX, K1BX, K1CX, K1DX, K1EX, K1FX,
    K20X, K21X, K22X, K23X, K24X, K25X, K26X, K27X, K28X, K29X, K2AX, K2BX,
    K2CX, K2DX, K2EX, K2FX, K30X, K31X, K32X, K33X, K34X, K35X, K36X, K37X,
    K38X, K39X, K3AX, K3BX, K3CX, K3DX, K3EX, K3FX, K40X, K41X, K42X, K43X,
    K44X, K45X, K46X, K47X, K48X, K49X, K4AX, K4BX, K4CX, K4DX, K4EX, K4FX,
    K50X, K51X, K52X, K53X, K54X, K55X, K56X, K57X, K58X, K59X, K5AX, K5BX,
    K5CX, K5DX, K5EX, K5FX, K60X, K61X, K62X, K63X, K64X, K65X, K66X, K67X
};


WORD KeysTE[NUMKEYS]  =   { K00Y, K01Y, K02Y, K03Y, K04Y, K05Y, K06Y, K07Y,
    K08Y, K09Y, K0AY, K0BY, K0CY, K0DY, K0EY, K0FY, K10Y, K11Y, K12Y, K13Y,
    K14Y, K15Y, K16Y, K17Y, K18Y, K19Y, K1AY, K1BY, K1CY, K1DY, K1EY, K1FY,
    K20Y, K21Y, K22Y, K23Y, K24Y, K25Y, K26Y, K27Y, K28Y, K29Y, K2AY, K2BY,
    K2CY, K2DY, K2EY, K2FY, K30Y, K31Y, K32Y, K33Y, K34Y, K35Y, K36Y, K37Y,
    K38Y, K39Y, K3AY, K3BY, K3CY, K3DY, K3EY, K3FY, K40Y, K41Y, K42Y, K43Y,
    K44Y, K45Y, K46Y, K47Y, K48Y, K49Y, K4AY, K4BY, K4CY, K4DY, K4EY, K4FY,
    K50Y, K51Y, K52Y, K53Y, K54Y, K55Y, K56Y, K57Y, K58Y, K59Y, K5AY, K5BY,
    K5CY, K5DY, K5EY, K5FY, K60Y, K61Y, K62Y, K63Y, K64Y, K65Y, K66Y, K67Y
};

WORD KeysFP[NUMKEYS] =   { 
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 00 - 09 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0A - 13 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 14 - 1D */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 1E - 27 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 28 - 31 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 32 - 3B */
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1,	/* 3C - 45 */
    1, 0, 0, 0, 1, 0, 1, 1, 1, 1,	/* 46 - 4F */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 50 - 59 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 5A - 63 */
    1, 1, 1, 1				/* 64 - 67 */
};

WORD KeysType[NUMKEYS] = { 1, 0, 0, 0, 0, 0, 0, 0, /* 0-7 */
    0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,	/* 8  - 19 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 20 - 31 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 32 - 43 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 44 - 55 */
    0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 3, 9, /* 56 - 67 */
    7, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 68 - 79 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, /* 80 - 91 */
    0, 0, 0, 2, 2, 10, 0, 0, 11, 11, 1, 1 /* 92 - 103 */
};


WORD KeysCount[12] = {6, 6, 6, 6, 6, 6, 10, 10, 6, 6, 6, 6};

WORD *KeysXY[12] = {Key1XY, Key2XY, Key3XY, Key4XY, Key5XY, Key6XY,
    Key7XY, Key8XY, Key9XY, KeyAXY, KeyBXY, KeyCXY};

WORD special[24] = {0x46, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
    0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5f, 0x62, 0x60, 0x61, 0x63,
    0x64, 0x65, 0x66, 0x67};
UBYTE sFlags[24] = {K_PREDEFINED, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED,
    K_PREDEFINED, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED,
    K_PREDEFINED, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED,
    K_PREDEFINED, K_PREDEFINED, K_BORDER, K_BORDER, K_BORDER, K_BORDER,
    K_BORDER, K_PREDEFINED, K_PREDEFINED, K_PREDEFINED};

struct IntuiText fakeIText;


MakeKeys()
{
    WORD i, j;
    static WORD NullKeys[] = {14, 28, 44, 59, 71, 72, 73, 75};
    struct Border *bp;
    struct IntuiText *tp;

    if((bp = (struct Border *)AllocRemember(&RememberKey, REALKEYS
	* sizeof(struct Border), 0)) == NULL)
	failNotice("Internal system error", "Not enough memory");
    if((tp = (struct IntuiText *)AllocRemember(&RememberKey, REALKEYS
	* sizeof(struct IntuiText), 0)) == NULL)
	failNotice("Internal system error", "Not enough memory");

    if(BoardType == USA)
    {
	KeysType[68] = 6;
	KeysType[96] = 8;
	LShift.Width = K9-2;
	LShift.GadgetRender = &ULShiftImage;
    }
    else
    {
	KeysType[68] = 7;
	KeysType[96] = 2;
	LShift.Width = K3-2;
	LShift.GadgetRender = &ELShiftImage;
    }

    for(i = 0, j = 0; i < NUMKEYS; i++)
    {
	if((i == NullKeys[j]) || 
	    ((BoardType == USA) && ((i == 48) || (i == 43))))
	{
	    Key[i].KBorder = NULL;
	    /* When 101 key keyboards came out, this line broke.  It
	       would set a NULL that was indirected off later.  I set
	       up a single fakeIText for deliberate trashing. */
	    Key[i].KText = &fakeIText;
	    if(i == NullKeys[j])
	    {
		Key[i].Flag = K_NADA;
		j++;
		if(j > 7)j = 0;
	    }
	}
	else
	{
	    Key[i].Flag = K_TEXT;

	    bp->LeftEdge = KeysLE[i];
	    bp->TopEdge = KeysTE[i];
	    bp->FrontPen = KeysFP[i];
	    bp->BackPen = NULL;
	    bp->DrawMode = JAM1;
	    bp->Count = KeysCount[KeysType[i]];
	    bp->XY = KeysXY[KeysType[i]];
	    bp->NextBorder = bp+1;

	    Key[i].KBorder = bp;

	    tp->FrontPen = BLACK;
	    tp->BackPen = WHITE;
	    tp->DrawMode = JAM2;
	    tp->LeftEdge = KeysLE[i];
	    tp->TopEdge = KeysTE[i];
	    tp->ITextFont = &Topaz;
	    tp->IText = (UBYTE *)Key[i].KCapTxt;
	    tp->NextText = tp+1;

	    Key[i].KText = tp;

	    bp++; tp++;
	}

    }

    Key[NUMKEYS - 1].KBorder->NextBorder = NULL;
    Key[NUMKEYS - 1].KText->NextText = NULL;

    for(i = 0; i < 24; i++)Key[special[i]].Flag = sFlags[i];

    Key[0x50].KText->IText = "F1";	/* F1 Function key */
    Key[0x51].KText->IText = "F2";	/* F2 Function key */
    Key[0x52].KText->IText = "F3";	/* F3 Function key */
    Key[0x53].KText->IText = "F4";	/* F4 Function key */
    Key[0x54].KText->IText = "F5";	/* F5 Function key */
    Key[0x55].KText->IText = "F6";	/* F6 Function key */
    Key[0x56].KText->IText = "F7";	/* F7 Function key */
    Key[0x57].KText->IText = "F8";	/* F8 Function key */
    Key[0x58].KText->IText = "F9";	/* F9 Function key */
    Key[0x59].KText->IText = "F10";	/* F10 Function key */
    for(i = 0x50; i < 0x5a; i++)
	Key[i].KText->FrontPen = BLUE;

    Key[0x66].KText->IText = "A";	/* Left-Amiga key */
    Key[0x67].KText->IText = "A";       /* Right-Amiga key */
    Key[0x66].KText->ITextFont = &TopazI;
    Key[0x66].KText->FrontPen = BLACK;	/* Really BLACK... */
    Key[0x67].KText->ITextFont = &TopazI;
    Key[0x67].KText->FrontPen = BLACK;	/* Really BLACK... */

    AddGList(window, &LShift, -1, -1, NULL);
    RefreshGList(&LShift, window, NULL, -1);
}

