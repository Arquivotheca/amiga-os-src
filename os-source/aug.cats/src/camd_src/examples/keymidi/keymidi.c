/*	keymidi.c

	Copyright 1990 Parsec Soft Systems
	Created May 1, 1990 by Joe Pearce

	Tables:
		what each key does when pressed (note off when released)
		two right mouse button modes:
			1) menus
			2) aftertouch
			3) pitch bend
*/

#if 0
#include <intuition/intuition.h>
/* #include <libraries/arpbase.h> */
#include <exec/memory.h>
#include <xlib.h>
#include <functions.h>
#endif

/* #include <xfunctions.h> */
#include "keyconvert.h"

#include "midi/camd.h"
#include "midi/camdtime.h"
#include "midi/mididefs.h"
#include "clib/camd_protos.h"
#include "pragmas/camd_pragmas.h"

#include "rawcode.h"

/*================= prototypes =================*/

void *MemAlloc(long,long);
void MemFree(void *);

handle_key(struct MidiLink *mi,UWORD code,UWORD qual);
void SelectGadget(struct Gadget *gad, struct Window *win,
	struct Requester *req, USHORT state);
void UpdateIntegerGadget(struct Window *w,struct Gadget *gad,long value);
void copy_2_chip(void *source,void *desta,long size);
LONG key_data_size(UBYTE *data);
LONG DoFileRequest(char *dir,char *file,char *msg);
open_edit_req(WORD key);

void assign_colors(struct Window *window,UWORD *colors);

/*================= system interface =================*/

struct Library			*GfxBase,*IntuitionBase,*CamdBase,*AslBase;
/* struct Library		*ArpBase; */

struct MidiNode *mi;
struct MidiLink *mlink;

struct TextAttr	topaz8 = { (STRPTR)"topaz.font", 8, FS_NORMAL, 0 };
struct TextFont *topazfont;

UWORD colors[4];

#define HILITE_HUE	colors[3]
#define SHADOW_HUE	colors[1]
#define TEXT_HUE	colors[2]
#define LIGHT_HUE	colors[0]

/*================= user interface =================*/

struct Image	radio_on = { 0,0, 17,9,2, NULL, 3,0, NULL },
				radio_off = { 0,0, 17,9,2, NULL, 3,0, NULL };

struct IntuiText EditText = {
	0,1,JAM2, 19,0, &topaz8, (UBYTE *)"Edit",
};

struct MenuItem EditItem = {
	NULL, 0,9, 130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP+CHECKIT,
	0x01, (APTR)&EditText, NULL, 'E',
};

struct IntuiText PlayText = {
	0,1,JAM2, 19,0, &topaz8, (UBYTE *)"Play",
};

struct MenuItem PlayItem = {
	&EditItem, 0,0,	130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP+CHECKIT+CHECKED,
	0x02, (APTR)&PlayText, NULL, 'P',
};

struct Menu KeyboardMenu = {
	NULL, 100,0, 95,0, MENUENABLED,	(BYTE *)"Keyboard", &PlayItem
};

struct IntuiText SaveText = {
	0,1,JAM2, 0,0, &topaz8, (UBYTE *)"Save Keys",
};

struct MenuItem SaveItem = {
	NULL, 0,18, 130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP, 0,
	(APTR)&SaveText, NULL, 'S',
};

struct IntuiText LoadText = {
	0,1,JAM2, 0,0, &topaz8, (UBYTE *)"Load Keys",
};

struct MenuItem LoadItem = {
	&SaveItem, 0,9, 130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP, 0,
	(APTR)&LoadText, NULL, 'L',
};

struct IntuiText QuitText = {
	0,1,JAM2, 0,0, &topaz8, (UBYTE *)"Quit",
};

struct MenuItem QuitItem = {
	&LoadItem, 0,27, 130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP, 0,
	(APTR)&QuitText, NULL, 'Q',
};

struct IntuiText AboutText = {
	0,1,JAM2, 0,0, &topaz8, (UBYTE *)"About",
};

struct MenuItem AboutItem = {
	&QuitItem, 0,0,	130,9, ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP, 0,
	(APTR)&AboutText, NULL, '?',
};

struct Menu ProjectMenu = {
	&KeyboardMenu,	0,0, 63,0, MENUENABLED,	(BYTE *)"Project", &AboutItem
};

#define KEYIDCMP	( CLOSEWINDOW | GADGETDOWN | GADGETUP | RAWKEY | MOUSEBUTTONS | MENUPICK )

struct NewWindow new_window = {
	0,20, 640,100, 0,1, KEYIDCMP,
	NOCAREREFRESH | ACTIVATE | SMART_REFRESH | WINDOWDRAG | WINDOWCLOSE | WINDOWDEPTH,
	NULL,NULL,(UBYTE *)"KeyMIDI",NULL,NULL,10,10,-1,-1,WBENCHSCREEN,
};

struct MinGad {
	WORD	LeftEdge,TopEdge;
	WORD	Width,Height;
	UWORD	GadgetID;
} mingads[] = {
	{ 	0,12, 31,11, RKEY_ACCENT_GRAVE },
	{	32,12, 25,11, RKEY_1 },
	{	58,12, 25,11, RKEY_2 },
	{	84,12, 25,11, RKEY_3 },
	{	110,12, 25,11, RKEY_4 },
	{	136,12, 25,11, RKEY_5 },
	{	162,12, 25,11, RKEY_6 },
	{	188,12, 25,11, RKEY_7 },
	{	214,12, 25,11, RKEY_8 },
	{	240,12, 25,11, RKEY_9 },
	{	266,12, 25,11, RKEY_0 },
	{	292,12, 25,11, RKEY_HYPHEN },
	{	318,12, 25,11, RKEY_EQUALS },
	{	344,12, 25,11, RKEY_BACKSLASH },
	{	500,60, 51,11, RKEY_NUM_0 },
	{	47,24, 25,11, RKEY_Q },
	{	73,24, 25,11, RKEY_W },
	{	99,24, 25,11, RKEY_E },
	{	125,24, 25,11, RKEY_R },
	{	151,24, 25,11, RKEY_T },
	{	177,24, 25,11, RKEY_Y },
	{	203,24, 25,11, RKEY_U },
	{	229,24, 25,11, RKEY_I },
	{	255,24, 25,11, RKEY_O },
	{	281,24, 25,11, RKEY_P },
	{	307,24, 25,11, RKEY_LBRACKET },
	{	333,24, 25,11, RKEY_RBRACKET },
	{	500,48, 25,11, RKEY_NUM_1 },
	{	526,48, 25,11, RKEY_NUM_2 },
	{	552,48, 25,11, RKEY_NUM_3 },
	{	58,36, 25,11, RKEY_A },
	{	84,36, 25,11, RKEY_S },
	{	110,36, 25,11, RKEY_D },
	{	136,36, 25,11, RKEY_F },
	{	162,36, 25,11, RKEY_G },
	{	188,36, 25,11, RKEY_H },
	{	214,36, 25,11, RKEY_J },
	{	240,36, 25,11, RKEY_K },
	{	266,36, 25,11, RKEY_L },
	{	292,36, 25,11, RKEY_SEMICOLON },
	{	318,36, 25,11, RKEY_QUOTE },
	{	500,36, 25,11, RKEY_NUM_4 },
	{	526,36, 25,11, RKEY_NUM_5 },
	{	552,36, 25,11, RKEY_NUM_6 },
	{	68,48, 25,11, RKEY_Z },
	{	94,48, 25,11, RKEY_X },
	{	120,48, 25,11, RKEY_C },
	{	146,48, 25,11, RKEY_V },
	{	172,48, 25,11, RKEY_B },
	{	198,48, 25,11, RKEY_N },
	{	224,48, 25,11, RKEY_M },
	{	250,48, 25,11, RKEY_COMMA },
	{	276,48, 25,11, RKEY_PERIOD },
	{	302,48, 25,11, RKEY_SLASH },
	{	552,60, 25,11, RKEY_NUM_PERIOD },
	{	500,24, 25,11, RKEY_NUM_7 },
	{	526,24, 25,11, RKEY_NUM_8 },
	{	552,24, 25,11, RKEY_NUM_9 },
	{	82,60, 233,11, RKEY_SPACE },
	{	370,12, 25,11, RKEY_BS },
	{	0,24, 46,11, RKEY_TAB },
	{	577,48, 25,23, RKEY_ENTER },
	{	0,0, 25,11, RKEY_ESC },
	{	408,12, 38,11, RKEY_DEL },
	{	578,24, 25,11, RKEY_NUM_HYPHEN },
	{	434,36, 25,11, RKEY_UP },
	{	408,48, 25,11, RKEY_LEFT },
	{	434,48, 25,11, RKEY_DOWN },
	{	460,48, 25,11, RKEY_RIGHT },
	{	32,0, 31,11, RKEY_F1 },
	{	64,0, 31,11, RKEY_F2 },
	{	96,0, 31,11, RKEY_F3 },
	{	128,0, 31,11, RKEY_F4 },
	{	160,0, 31,11, RKEY_F5 },
	{	210,0, 31,11, RKEY_F6 },
	{	242,0, 31,11, RKEY_F7 },
	{	274,0, 31,11, RKEY_F8 },
	{	306,0, 31,11, RKEY_F9 },
	{	338,0, 31,11, RKEY_F10 },
	{	500,12, 25,11, RKEY_NUM_LPAREN },
	{	526,12, 25,11, RKEY_NUM_RPAREN },
	{	552,12, 25,11, RKEY_NUM_SLASH },
	{	578,12, 25,11, RKEY_NUM_ASTERISK },
	{	578,36, 25,11, RKEY_NUM_PLUS },
	{	447,12, 38,11, RKEY_HELP },
	{	0,48, 67,11, RKEY_LSHIFT },
	{	328,48, 67,11, RKEY_RSHIFT },
	{	32,36, 25,11, RKEY_CAPSLOCK },
	{	0,36, 31,11, RKEY_CTRL },
	{	18,60, 31,11, RKEY_LALT },
	{	347,60, 31,11, RKEY_RALT },
	{	50,60, 31,11, RKEY_LAMIGA },
	{	315,60, 31,11, RKEY_RAMIGA }
};

struct MinGad foreigngads[2] = {
	{	344,36, 25,11, RKEY_FOREIGN1 },
	{	42,48, 25,11, RKEY_FOREIGN2 }
};

#define NUMKGADS	(sizeof(mingads)/sizeof(struct MinGad))

struct BoolInfo retkeyinfo;

struct Gadget	gadkeys[NUMKGADS+2],
				gadretkey,
				*lastgad;

UWORD *retkeymask;

struct Gadget radiogads[8];

UBYTE textbufs[3][4],undobuf[4];

struct StringInfo textinfos[3] = {
	{	&textbufs[0][0], undobuf, 0,4, },
	{	&textbufs[1][0], undobuf, 0,4, },
	{	&textbufs[2][0], undobuf, 0,4, }
};

struct Gadget textgads[3] = {
	{	NULL, 0,0, 32,8, GADGHCOMP, STRINGRIGHT|LONGINT, STRGADGET,
		NULL,NULL, NULL, 0, (APTR)&textinfos[0], },
	{	NULL, 0,0, 32,8, GADGHCOMP, STRINGRIGHT|LONGINT, STRGADGET,
		NULL,NULL, NULL, 0, (APTR)&textinfos[1], },
	{	NULL, 0,0, 32,8, GADGHCOMP, STRINGRIGHT|LONGINT, STRGADGET,
		NULL,NULL, NULL, 0, (APTR)&textinfos[2], }
};

struct Gadget
	cancelgad = { NULL, 324,46, 67,11, GADGHCOMP, RELVERIFY, BOOLGADGET,
		NULL, NULL, NULL, 0, NULL, 101, },
	okgad = { &cancelgad, 324,22, 67,11, GADGHCOMP, RELVERIFY, BOOLGADGET,
		NULL, NULL, NULL, 0, NULL, 100, };

struct Requester edit_req = { NULL, 115,18, 401,74, };	

#define GetIMsg(a)		(struct IntuiMessage *)GetMsg(a)
#define ReplyIMsg(a)	ReplyMsg(&(a)->ExecMessage)

struct Window			*window;
struct Screen			*screen;
struct RastPort			*rp;

/*================= keypress info =================*/

/*
	All events are stored as either 4 bytes (command,byte1,byte2,nul) or
	as SYSEX (figure out length by looking for EOX).
*/

enum {
	KEYDEF_ENDS=0,
	KEYDEF_LSHIFT,
	KEYDEF_RSHIFT,
	KEYDEF_LALT,
	KEYDEF_RALT,
	KEYDEF_CTRL
};

UBYTE		keypressed[128];

UBYTE		*keyevent[128],
			*defevent[128] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	(UBYTE *)"\x97\x42\x40",		/* notes */
	(UBYTE *)"\x97\x43\x40",
	(UBYTE *)"\x97\x44\x40",
	(UBYTE *)"\x97\x45\x40",
	(UBYTE *)"\x97\x46\x40",
	(UBYTE *)"\x97\x47\x40",
	(UBYTE *)"\x97\x48\x40",
	(UBYTE *)"\x97\x49\x40",

	(UBYTE *)"\x97\x4a\x40",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL,
	(UBYTE *)"\xe7\x00\x60",		/* pitch bend */
	(UBYTE *)"\xe7\x00\x20",

	(UBYTE *)"\xc7\x00\x00",		/* program change */
	(UBYTE *)"\xc7\x01\x00",
	(UBYTE *)"\xc7\x02\x00",
	(UBYTE *)"\xc7\x03\x00",
	(UBYTE *)"\xc7\x04\x00",
	(UBYTE *)"\xc7\x05\x00",
	(UBYTE *)"\xc7\x06\x00",
	(UBYTE *)"\xc7\x07\x00",

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/*================= imagery - copy to CHIP ram =================*/

UWORD SymbolsData[105] = {
   0xF9E7,0xBF01,0x81F0,0x0060,0x3007,0x1800,0x01E0,0x003C,0x0600,0xFCFB,0x0CDF,0x61F0,0x3006,0x000C,0x0000,
   0xC30C,0x0C01,0x8303,0x0060,0xCC0D,0x9980,0x07E0,0x00D4,0x1E00,0xC6C3,0x0CD8,0x6198,0xFC06,0x000F,0x0000,
   0xF9CC,0x0C79,0xF307,0xCF63,0x0318,0xDBE0,0x1EE0,0x0354,0x7FFE,0xC6FB,0x0FDF,0x61F3,0xFF06,0x0FFF,0xC000,
   0xC06C,0x0CCD,0x9B03,0x186F,0xCFDF,0xD980,0x78E0,0x0D14,0x1E00,0xC6C3,0x0CD8,0x6180,0x307F,0xE00F,0x0000,
   0xFBC7,0x8C6D,0xF1F1,0xD860,0xCC18,0xD8E1,0xFFE0,0x37F4,0x0600,0xFCFB,0xECDF,0x7D80,0x301F,0x800C,0x0000,
   0x0000,0x0000,0x0000,0x0000,0xFC00,0x0007,0x80E0,0xD014,0x0000,0x0000,0x0000,0x0000,0x3006,0x0000,0x0000,
   0x0000,0x0000,0x0000,0x0000,0x0000,0x001F,0xC3FB,0xF87F,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
};

UWORD *SymData;

#define SYM_MOD		(2 * (105 / 7))

/*================= code =================*/

#define SafeCloseLibrary(a)	if (a) CloseLibrary(a)

cleanup(char *msg,int errnum)
{	if (msg) puts(msg);

	if (window)
	{	if (window->MenuStrip) ClearMenuStrip(window);
		CloseWindow(window);
	}
    if (mi) DeleteMidi (mi);
	if (topazfont) CloseFont(topazfont);
	/* SafeCloseLibrary(ArpBase); */
	SafeCloseLibrary(AslBase);
	SafeCloseLibrary(CamdBase);
	SafeCloseLibrary(IntuitionBase);
	SafeCloseLibrary(GfxBase);
	exit(errnum);
}

struct MidiNode *CreateMidi(Tag tag, ...)
{	return CreateMidiA((struct TagItem *)&tag );
}

BOOL SetMidiAttrs(struct MidiNode *mi, Tag tag, ...)
{	return SetMidiAttrsA(mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type, Tag tag, ...)
{	return AddMidiLinkA(mi, type, (struct TagItem *)&tag );
}

BOOL SetMidiLinkAttrs(struct MidiLink *mi, Tag tag, ...)
{	return SetMidiLinkAttrsA(mi, (struct TagItem *)&tag );
}

long clamp(long min,long val,long max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void Textf(struct RastPort *rp,char *str,...)
{	char	buffer[80];

	vsprintf(buffer,str,&str + 1);
	Text(rp,buffer,strlen(buffer));
}

void main(int argc,char *argv[])
{
	BOOL	done = 0;
	WORD	i;
	LONG	size,thruport;
	char	*linkname = "out.0";

	if (argc > 1) linkname = argv[1];

	GfxBase = OpenLibrary("graphics.library",33L);
	if (GfxBase == NULL) cleanup("Can't open graphics library",10);

	IntuitionBase = OpenLibrary("intuition.library",33L);
	if (IntuitionBase == NULL) cleanup("Can't open intuition library",10);

	CamdBase = OpenLibrary("camd.library", 0L);
	if (CamdBase == NULL) cleanup("Can't open CAMD",10);

#if 0
	if (IntuitionBase->lib_Version < 36L)
	{
		ArpBase = OpenLibrary("arp.library", 39L);
		if (ArpBase == NULL) cleanup("Can't open ARP",10);
	}
	else
#endif
	{
		AslBase = OpenLibrary("asl.library", 36L);
		if (AslBase == NULL) cleanup("Can't open ASL library",10);
	}

	topazfont = OpenFont(&topaz8);

	window = OpenWindow(&new_window);
	if (window == NULL) cleanup("can't open window",10);

	screen = window->WScreen;
	rp = window->RPort;

	assign_colors(window,colors);
	make_images();

	copy_2_chip(SymbolsData,&SymData,sizeof SymbolsData);

	if (!build_keygads()) cleanup("Can't open ConsoleDevice",10);
	build_retkeygad();

	AddGList(window,&gadretkey,-1,-1,NULL);

	mi = CreateMidi(
		MIDI_Name, "KeyMIDI",
		MIDI_RecvSignal, SIGBREAKB_CTRL_E,
		MIDI_MsgQueue,   100,
		MIDI_ErrFilter, CMEF_All,
		TAG_DONE);
	if (mi == NULL) cleanup("Can't create MidiNode",10);

	mlink = AddMidiLink(mi, MLTYPE_Sender,
		MLINK_Name, "KeyMIDI Link",
		MLINK_Location,	linkname,
		MLINK_Comment,	"KeyMIDI [Output]",
		TAG_DONE);
	if (mlink == NULL) cleanup("Can't create MidiLink",10);

	for (i=0;i<128;i++) if (defevent[i])
	{
		size = key_data_size(defevent[i]);
		keyevent[i] = MemAlloc(size,0L);
		if (keyevent[i] == NULL) cleanup("Out of memory",10);

		CopyMem(defevent[i],keyevent[i],size);
	}

	SetMenuStrip(window,&ProjectMenu);

	while (!done)
	{	struct IntuiMessage *imsg;
		ULONG				class;
		UWORD				code,qual;
		struct Gadget		*gad;
		struct MenuItem		*item;

		Wait(1 << window->UserPort->mp_SigBit);

		while (imsg = GetIMsg(window->UserPort))
		{	class = imsg->Class;
			code = imsg->Code;
			qual = imsg->Qualifier;
			gad = (struct Gadget *)imsg->IAddress;

			ReplyIMsg(imsg);

			switch (class)
			{	case CLOSEWINDOW:
				done = 1;
				break;

				case MENUPICK:
				while (code != MENUNULL)
				{	item = ItemAddress(&ProjectMenu,code);

					if (item == &QuitItem) { done = 1; break; }

					if (item == &AboutItem)
					{	/* show about box */
					}

					if (item == &SaveItem) save_keys();
					if (item == &LoadItem) load_keys();

					code = item->NextSelect;
				}
				break;

				case INACTIVEWINDOW:
				all_off();
				break;

				case MOUSEBUTTONS:
				if (code == SELECTUP && lastgad != NULL)
				{	if (lastgad->GadgetID < 0x0080)
						handle_key(mlink,lastgad->GadgetID|0x0080,qual);
					lastgad = NULL;
				}
				break;

				case GADGETDOWN:
				if (PlayItem.Flags & CHECKED)
				{	lastgad = gad;
					if (gad->GadgetID < 0x0080) handle_key(mlink,gad->GadgetID,qual);
				}
				break;

				case GADGETUP:
				if (EditItem.Flags & CHECKED)
				{
					if (gad->GadgetID < 0x0080)
					{
						all_off();
						open_edit_req(gad->GadgetID);
					}
				}
				if (PlayItem.Flags & CHECKED)
				{	if (lastgad != NULL && gad->GadgetID < 0x0080)
						handle_key(mlink,gad->GadgetID|0x0080,qual);
				}
				lastgad = NULL;
				break;

				case RAWKEY:
				if ( !(qual & IEQUALIFIER_REPEAT) ) handle_key(mlink,code,qual);
				break;
			}
		}
	}

	all_off();

	cleanup(NULL,0);
}

handle_key(struct MidiLink *mi,UWORD code,UWORD qual)
{	UBYTE	*command;
	UWORD	key;
	ULONG	t;
	MidiMsg msg;

	/* Printf("Code = %02.2lx\n",(ULONG)code); */

	key = code & 0x7f;
	msg.mm_Status = 0;

	if (command = keyevent[key])
	{	if (code & 0x80)
		{	switch (*command & 0xf0)
			{	case 0x90:						/* note off */
				msg.mm_Status = command[0];
				msg.mm_Data1 = command[1];
				msg.mm_Data2 = 0x00;
				break;

				case 0xe0:						/* pitchbend off */
				msg.mm_Status = command[0];
				msg.mm_Data1 = 0x00;
				msg.mm_Data2 = 0x40;
				break;
			}
			keypressed[key] = 0;
		}
		else
		{	switch (*command & 0xf0)
			{	case 0x90:						/* note on */
				msg.mm_Status = command[0];
				msg.mm_Data1 = command[1];
				t = (ULONG)(screen->Height - screen->MouseY) * 127L / (ULONG)screen->Height;
				msg.mm_Data2 = clamp(1L,t,127L);
				break;

				case 0xc0:						/* program change */
				msg.mm_Status = command[0];
				msg.mm_Data1 = command[1];
				if (qual & IEQUALIFIER_LSHIFT) msg.mm_Data1 += 0x08;
				if (qual & IEQUALIFIER_RSHIFT) msg.mm_Data1 += 0x10;
				if (qual & IEQUALIFIER_LALT) msg.mm_Data1 += 0x18;
				if (qual & IEQUALIFIER_RALT) msg.mm_Data1 += 0x20;
				if (qual & IEQUALIFIER_CONTROL) msg.mm_Data1 += 0x28;
				break;

				case 0xe0:						/* pitchbend */
				msg.mm_Status = command[0];
				msg.mm_Data1 = command[1];
				msg.mm_Data2 = command[2];
				break;
			}
			keypressed[key] = 1;
		}

		if (msg.mm_Status) PutMidiMsg (mlink,&msg);
	}
}

LONG key_data_size(UBYTE *data)
{	LONG count = 4;

	if (data == NULL) return 0;

	while (TRUE)
	{	if (data[0] == (UBYTE)0xf7)
		{
			/* SYSEX (not available yet) */
		}
		else if (data[3] == 0) break;
		else
		{	count += 4;
			data += 4;
		}
	}

	return count;
}

all_off()
{	UWORD	i;
	UBYTE	*command;
	MidiMsg msg;

	for (i=0;i<128;i++) if (keypressed[i])
	{	command = keyevent[i];

		if ( (*command & 0xf0) == 0x90 )	/* note on */
		{	msg.mm_Status = command[0];
			msg.mm_Data1 = command[1];
			msg.mm_Data2 = 0x00;

			PutMidiMsg (mlink,&msg);
		}

		keypressed[i] = 0;
	}
}

void draw_bevel(struct RastPort *rp,WORD *dim)
{	WORD	vecs[10];

	SetAPen(rp,HILITE_HUE);
	vecs[0] = dim[0] + dim[2] - 2;
	vecs[2] = vecs[4] = dim[0];
	vecs[1] = vecs[3] = dim[1];
	vecs[5] = dim[1] + dim[3] - 1;
	vecs[6] = vecs[8] = vecs[2] + 1;
	vecs[9] = vecs[1] + 1;
	vecs[7] = vecs[5] - 1;
	Move(rp,vecs[0],vecs[1]);
	PolyDraw(rp,4L,&vecs[2]);

	SetAPen(rp,SHADOW_HUE);
	vecs[0] = dim[0] + 1;
	vecs[7] = dim[1] + 1;
	vecs[1] = vecs[3] = dim[1] + dim[3] - 1;
	vecs[2] = vecs[4] = dim[0] + dim[2] - 1;
	vecs[5] = dim[1];
	vecs[6] = vecs[8] = vecs[2] - 1;
	vecs[9] = vecs[1] - 1;
	Move(rp,vecs[0],vecs[1]);
	PolyDraw(rp,4L,&vecs[2]);
}

void draw_text(struct RastPort *rp,char *str,WORD *dim)
{	WORD	len,pixels;

	SetAPen(rp,SHADOW_HUE);
	SetFont(rp,topazfont);
	pixels = TextLength(rp,str,len = strlen(str));
	Move(rp,(dim[2]-pixels)/2+dim[0],(dim[3]-7)/2+6+dim[1]);
	Text(rp,str,len);
}

#define KEY_XOFF	20
#define KEY_YOFF	20

struct IOStdReq	conReq;

build_keygads()
{	struct InputEvent imsg;
	struct Gadget *gad;
	struct MinGad *mgad;
	UBYTE	buffer[2];
	WORD	i,vecs[10];
	struct Library *ConsoleDevice;

	if (OpenDevice("console.device", -1, (struct IORequest *)&conReq, 0))
		return 0;

	ConsoleDevice = (struct Library *)conReq.io_Device;

	imsg.ie_NextEvent = 0;
	imsg.ie_Class = IECLASS_RAWKEY;
	imsg.ie_SubClass = 0;
	imsg.ie_Code = 0;
	imsg.ie_Qualifier = 0;
	imsg.ie_EventAddress = 0;
	imsg.ie_TimeStamp.tv_secs = 0;
	imsg.ie_TimeStamp.tv_micro = 0;

	SetFont(rp,topazfont);

	for (i=0,gad=gadkeys,mgad=mingads;i<NUMKGADS;i++,gad++,mgad++)
	{
		gad->Flags = GADGHCOMP;
		gad->Activation = GADGIMMEDIATE | RELVERIFY;
		gad->GadgetType = BOOLGADGET;
		gad->LeftEdge = mgad->LeftEdge + KEY_XOFF;
		gad->TopEdge = mgad->TopEdge + KEY_YOFF;
		gad->Width = mgad->Width;
		gad->Height = mgad->Height;
		gad->GadgetID = mgad->GadgetID;

		draw_bevel(rp,&gad->LeftEdge);

		imsg.ie_Code = mgad->GadgetID;
		if (mgad->GadgetID == RKEY_ENTER)
		{
			BltTemplate((void *)SymData,129,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+4,14,5);
			RectFill(rp,gad->LeftEdge+17,gad->TopEdge+3,
				gad->LeftEdge+18,gad->TopEdge+6);
		}
		else if (RawKeyConvert(&imsg,buffer,2,NULL) == 1)
		{
			switch (buffer[0])
			{
				case '\t':
				BltTemplate((void *)SymData,18,SYM_MOD,rp,gad->LeftEdge+4,
					gad->TopEdge+2,19,5);
				break;

				case '\b':
				BltTemplate((void *)SymData,129,SYM_MOD,rp,gad->LeftEdge+4,
					gad->TopEdge+2,14,5);
				break;

				case 0x1b:
				BltTemplate((void *)SymData,0,SYM_MOD,rp,gad->LeftEdge+4,
					gad->TopEdge+2,17,5);
				break;

				case 0x7f:
				BltTemplate((void *)SymData,144,SYM_MOD,rp,gad->LeftEdge+4,
					gad->TopEdge+2,19,5);
				break;

				default:
				buffer[1] = 0;
				strupr((char *)buffer);
				Move(rp,gad->LeftEdge+4,gad->TopEdge+8);
				Text(rp,(char *)buffer,1);
				break;
			}
		}
		else if (imsg.ie_Code >= (UBYTE)RKEY_F1 && imsg.ie_Code <= (UBYTE)RKEY_F10)
		{
			Move(rp,gad->LeftEdge+4,gad->TopEdge+8);
			Textf(rp,"F%ld",(ULONG)(imsg.ie_Code-(UBYTE)RKEY_F1+1));
		}
		else switch (imsg.ie_Code)
		{	case RKEY_UP:
			BltTemplate((void *)SymData,190,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,10,6);
			break;

			case RKEY_DOWN:
			BltTemplate((void *)SymData,201,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,10,6);
			break;

			case RKEY_RIGHT:
			BltTemplate((void *)SymData,212,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,14,5);
			break;

			case RKEY_LEFT:
			BltTemplate((void *)SymData,129,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,14,5);
			break;

			case RKEY_HELP:
			BltTemplate((void *)SymData,164,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,25,5);
			break;

			case RKEY_LSHIFT:
			case RKEY_RSHIFT:
			BltTemplate((void *)SymData,60,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,14,6);
			break;

			case RKEY_CAPSLOCK:
			RectFill(rp,gad->LeftEdge+13,gad->TopEdge+3,
				gad->LeftEdge+20,gad->TopEdge+4);
			RectFill(rp,gad->LeftEdge+14,gad->TopEdge+2,
				gad->LeftEdge+19,gad->TopEdge+5);
			SetAPen(rp,3);
			RectFill(rp,gad->LeftEdge+14,gad->TopEdge+3,
				gad->LeftEdge+19,gad->TopEdge+4);
			break;

			case RKEY_CTRL:
			BltTemplate((void *)SymData,38,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,21,5);
			break;

			case RKEY_LALT:
			case RKEY_RALT:
			BltTemplate((void *)SymData,75,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,16,5);
			break;

			case RKEY_LAMIGA:
			BltTemplate((void *)SymData,91,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,18,7);
			break;

			case RKEY_RAMIGA:
			BltTemplate((void *)SymData,110,SYM_MOD,rp,gad->LeftEdge+4,
				gad->TopEdge+2,18,7);
			break;
		}

		if (i != NUMKGADS - 1) gad->NextGadget = gad + 1;
	}

	CloseDevice((struct IORequest *)&conReq);
	return 1;
}

build_retkeygad()
{	WORD	i;
	UWORD	*start;

	start = retkeymask = MemAlloc(8*23,MEMF_CHIP|MEMF_CLEAR);

	if (start == NULL) cleanup("Out of memory",10);

	for (i=0;i<23;i++)
	{	if (i <= 11)
		{	*start++ = 0x0001;
			*start++ = 0xffff;
			*start++ = 0xffff;
			*start++ = 0xe000;
		}
		else
		{	*start++ = 0xffff;
			*start++ = 0xffff;
			*start++ = 0xffff;
			*start++ = 0xe000;
		}
	}

	gadretkey.Flags = GADGHCOMP;
	gadretkey.Activation = GADGIMMEDIATE | RELVERIFY | BOOLEXTEND;
	gadretkey.GadgetType = BOOLGADGET;
	gadretkey.LeftEdge = 344 + KEY_XOFF;
	gadretkey.TopEdge = 24 + KEY_YOFF;
	gadretkey.Width = 50;
	gadretkey.Height = 23;
	gadretkey.GadgetID = RKEY_RETURN;
	gadretkey.SpecialInfo = (APTR)&retkeyinfo;
	retkeyinfo.Flags = BOOLMASK;
	retkeyinfo.Mask = retkeymask;
	gadretkey.NextGadget = gadkeys;

	SetAPen(rp,HILITE_HUE);
	Move(rp,393+KEY_XOFF,24+KEY_YOFF);
	Draw(rp,359+KEY_XOFF,24+KEY_YOFF);
	Draw(rp,359+KEY_XOFF,36+KEY_YOFF);
	Draw(rp,360+KEY_XOFF,35+KEY_YOFF);
	Draw(rp,360+KEY_XOFF,25+KEY_YOFF);
	Move(rp,358+KEY_XOFF,36+KEY_YOFF);
	Draw(rp,344+KEY_XOFF,36+KEY_YOFF);
	Draw(rp,344+KEY_XOFF,46+KEY_YOFF);
	Draw(rp,345+KEY_XOFF,45+KEY_YOFF);
	Draw(rp,345+KEY_XOFF,37+KEY_YOFF);

	SetAPen(rp,SHADOW_HUE);
	Move(rp,394+KEY_XOFF,24+KEY_YOFF);
	Draw(rp,394+KEY_XOFF,46+KEY_YOFF);
	Draw(rp,345+KEY_XOFF,46+KEY_YOFF);
	Move(rp,393+KEY_XOFF,25+KEY_YOFF);
	Draw(rp,393+KEY_XOFF,45+KEY_YOFF);

	BltTemplate((void *)SymData,129,SYM_MOD,rp,gadretkey.LeftEdge+14,
		gadretkey.TopEdge+15,14,5);
	RectFill(rp,gadretkey.LeftEdge+32,gadretkey.TopEdge+7,
		gadretkey.LeftEdge+33,gadretkey.TopEdge+17);
	Move(rp,gadretkey.LeftEdge+26,gadretkey.TopEdge+17);
	Draw(rp,gadretkey.LeftEdge+31,gadretkey.TopEdge+17);
}

LONG find_keygad_func(UWORD *id,struct Gadget *gad)
{
	if (gad->GadgetID == *id) return 0;
	if (gad->GadgetID < *id) return 1;
	return -1;
}

struct Gadget *find_keygad(UWORD id)
{
	return (struct Gadget *)bsearch((void *)&id,gadkeys,NUMKGADS,
		sizeof(struct Gadget),find_keygad_func);
}

char *edit_text[8] = {
	"UNDEFINED",
	"NOTE",
	"PROGRAM CHANGE",
	"CONTROL CHANGE",
	"CHANNEL PRESSURE",
	"POLYKEY PRESSURE",
	"PITCH BEND",
	"SYSTEM EXCLUSIVE"
};

open_edit_req(WORD key)
{	WORD	i,
			done = 0;
	struct RastPort *rrp;
	struct Gadget	*sel,*isel;
	WORD	dim[4];

	okgad.LeftEdge = cancelgad.LeftEdge = 324;
	okgad.TopEdge = 41;
	cancelgad.TopEdge = 57;
	cancelgad.NextGadget = radiogads;

	for (i=0;i<8;i++)
	{	radiogads[i].Flags = GADGHIMAGE | GADGIMAGE;
		radiogads[i].Activation = GADGIMMEDIATE;
		radiogads[i].GadgetType = BOOLGADGET | REQGADGET;
		radiogads[i].TopEdge = (i % 4) * 10 + 6;
		radiogads[i].LeftEdge = (i / 4) * 149 + 10;
		radiogads[i].Width = 17;
		radiogads[i].Height = 9;
		radiogads[i].GadgetID = i;
		radiogads[i].GadgetRender = (APTR)&radio_off;
		radiogads[i].SelectRender = (APTR)&radio_on;
		radiogads[i].NextGadget = &radiogads[i+1];
	}

	if (keyevent[key] == NULL) sel = &radiogads[0];
	else switch (*keyevent[key] & (UBYTE)0xf0)
	{	case (UBYTE)0x90: sel = &radiogads[1]; break;
		case (UBYTE)0xC0: sel = &radiogads[2]; break;
		case (UBYTE)0xB0: sel = &radiogads[3]; break;
		case (UBYTE)0xD0: sel = &radiogads[4]; break;
		case (UBYTE)0xA0: sel = &radiogads[5]; break;
		case (UBYTE)0xE0: sel = &radiogads[6]; break;
		case (UBYTE)0xF0: sel = &radiogads[7]; break;
		default: sel = &radiogads[0]; break;
	}

	isel = sel;
	sel->Flags |= SELECTED;
	radiogads[7].Flags = GADGHIMAGE | GADGIMAGE | GADGDISABLED;
	radiogads[7].NextGadget = &textgads[0];

	textgads[0].LeftEdge = 341;
	textgads[0].TopEdge = 21;
	textgads[0].NextGadget = &textgads[1];
	UpdateIntegerGadget(NULL,&textgads[0],
		(keyevent[key] ? *keyevent[key] & 0x0f : 0) + 1L);

	textgads[1].LeftEdge = 121;
	textgads[1].TopEdge = 49;
	textgads[1].NextGadget = &textgads[2];
	UpdateIntegerGadget(NULL,&textgads[1],
		keyevent[key] ? (ULONG)keyevent[key][1] : 0L);
	
	textgads[2].LeftEdge = 224;
	textgads[2].TopEdge = 49;
	textgads[2].NextGadget = NULL;
	UpdateIntegerGadget(NULL,&textgads[2],
		keyevent[key] ? (ULONG)keyevent[key][2] : 0L);
	
	edit_req.ReqGadget = &okgad;

	if (Request(&edit_req,window))
	{	rrp = edit_req.ReqLayer->rp;

		dim[0] = dim[1] = 0;
		dim[2] = edit_req.Width;
		dim[3] = edit_req.Height;
		draw_bevel(rrp,dim);
		draw_bevel(rrp,&okgad.LeftEdge);
		draw_bevel(rrp,&cancelgad.LeftEdge);

		dim[2] = 36;
		dim[3] = 10;
		dim[0] = 339;
		dim[1] = 20;
		draw_bevel(rrp,dim);
		dim[0] = 119;
		dim[1] = 48;
		draw_bevel(rrp,dim);
		dim[0] = 222;
		dim[1] = 48;
		draw_bevel(rrp,dim);

		draw_text(rrp,"OK",&okgad.LeftEdge);
		draw_text(rrp,"CANCEL",&cancelgad.LeftEdge);

		SetAPen(rrp,SHADOW_HUE);
		Move(rrp,329,17);
		Textf(rrp,"CHANNEL");
		Move(rrp,65,55);
		Textf(rrp,"BYTE1:",6L);
		Move(rrp,168,55);
		Textf(rrp,"BYTE2:",6L);

		SetFont(rrp,topazfont);
		SetAPen(rrp,SHADOW_HUE);
		for (i=0;i<8;i++)
		{	Move(rrp,radiogads[i].LeftEdge+20,radiogads[i].TopEdge+7);
			Textf(rrp,edit_text[i]);
		}

		while (!done)
		{	struct IntuiMessage *imsg;
			ULONG				class;
			struct Gadget		*gad;

			Wait(1 << window->UserPort->mp_SigBit);

			while (imsg = GetIMsg(window->UserPort))
			{	class = imsg->Class;
				gad = (struct Gadget *)imsg->IAddress;

				ReplyIMsg(imsg);

				switch (class)
				{
					case GADGETDOWN:
					if (gad->GadgetID < 8)
					{	if (gad != sel)
						{	SelectGadget(sel,window,&edit_req,0);
							sel = gad;
							SelectGadget(sel,window,&edit_req,SELECTED);
						}
					}
					break;

					case GADGETUP:
					done = gad->GadgetID;
					break;
				}
			}
		}

		EndRequest(&edit_req,window);

		if (done == cancelgad.GadgetID) return;

		if (isel != sel)
		{	if (sel->GadgetID == 0)
			{		/* undefine the key */
				MemFree(keyevent[key]);
				keyevent[key] = NULL;
			}
			else
			{	static UBYTE com[7] = { 0x90,0xc0,0xb0,0xd0,0xa0,0xe0,0xf0 };

				if (isel->GadgetID == 0)
				{		/* need memory */
					keyevent[key] = MemAlloc(4L,MEMF_CLEAR);
				}

				keyevent[key][0] =
					com[sel->GadgetID - 1] |
					clamp(1,textinfos[0].LongInt,16) - 1;
				keyevent[key][1] = textinfos[1].LongInt & 0x7f;
				keyevent[key][2] = textinfos[2].LongInt & 0x7f;
			}
		}
		else if (isel->GadgetID != 0)
		{
			keyevent[key][0] = 
				(keyevent[key][0] & 0xf0) |
				clamp(1,textinfos[0].LongInt,16) - 1;
			keyevent[key][1] = textinfos[1].LongInt & 0x7f;
			keyevent[key][2] = textinfos[2].LongInt & 0x7f;
		}
	}
}

char filename[32 + 1] = "keymidi.dat";
char directory[(32 * 10) + 32 + 1] = "";

save_keys()
{	BPTR lock,olddir;
	BPTR file;
	WORD	i;

	if (DoFileRequest(directory,filename,"Save Keys"))
	{	if (lock = Lock(directory,ACCESS_READ))
		{	olddir = CurrentDir(lock);
			if (file = Open(filename,MODE_NEWFILE))
			{
				if (Write(file,"KEYM",4) != 4L)		/* magic header */
					goto fail;

				for (i=0;i<128;i++)
				{	if (keyevent[i])
					{
						if (Write(file,keyevent[i],4L) != 4L) goto fail;
					}
					else if (Write(file,(void *)&keyevent[i],4L) != 4L)
						goto fail;
				}

fail:			Close(file);
			}
			CurrentDir(olddir);
			UnLock(lock);
		}
	}
}

load_keys()
{	BPTR lock,olddir;
	BPTR file;
	WORD	i;
	LONG	magic;
	char	buf[4];

	if (DoFileRequest(directory,filename,"Load Keys"))
	{	if (lock = Lock(directory,ACCESS_READ))
		{	olddir = CurrentDir(lock);
			if (file = Open(filename,MODE_OLDFILE))
			{
				Read(file,&magic,4);		/* magic header */
				if (magic != 'KEYM') goto fail;

				for (i=0;i<128;i++)
				{	MemFree(keyevent[i]);
					keyevent[i] = NULL;

					if (Read(file,buf,4L) != 4L) goto fail;
					if (buf[0] != 0)
					{
						keyevent[i] = MemAlloc(4L,0L);
						CopyMem(buf,keyevent[i],4L);
					}
				}

fail:			Close(file);
			}
			CurrentDir(olddir);
			UnLock(lock);
		}
	}
}

void UpdateIntegerGadget(struct Window *w,struct Gadget *gad,long value)
{	WORD	t;

	if (w) t = RemoveGadget(w,gad);

	((struct StringInfo *)gad->SpecialInfo)->LongInt = value;
	sprintf(((struct StringInfo *)gad->SpecialInfo)->Buffer,"%ld",value);

	if (w)
	{	AddGadget(w,gad,t);
		RefreshGList(gad,w,NULL,1L);
	}
}

void SelectGadget(struct Gadget *gad, struct Window *win,
	struct Requester *req, USHORT state)
{	long p;

	if ((gad->Flags & GADGHIGHBITS) == GADGHCOMP)
	{	p = RemoveGadget(win,gad);
		AddGList(win,gad,p,1,req);
		RefreshGList(gad,win,req,1);
	}

	p = RemoveGadget(win,gad);
	gad->Flags = (gad->Flags & ~SELECTED) | state;
	AddGList(win,gad,p,1,req);
	RefreshGList(gad,win,req,1);
}

#if 0
LONG FileRequest(struct FileRequester *);
#pragma amicall(ArpBase, 0x126, FileRequest(a0))

void *AllocFileRequest(void);
void FreeAslRequest(void *);
LONG AslRequest(void *,void *);
#pragma amicall(AslBase, 0x1e, AllocFileRequest())
#pragma amicall(AslBase, 0x24, FreeAslRequest(a0))
#pragma amicall(AslBase, 0x3c, AslRequest(a0,a1))

struct AslFileRequester	{
	APTR	Reserved1;
	BYTE	*rf_File;	/* Filename pointer		*/
	BYTE	*rf_Dir;	/* Directory name pointer	*/
	APTR	Reserved2;
	WORD	Reserved3;
	LONG	Reserved4;
	LONG	Reserved5;
	LONG	Reserved6;
	WORD	Reserved7;
	struct MinList	rf_SelectList;	/* List returned for multiselect */
};

struct MeTags {
	LONG	tag;
	VOID	*tagitem;
	};

#define TAG_DONE	(0L)
#define TAG_USER	(1<<31L)

enum {
	ASL_Dummy = TAG_USER + 0x80000,
	ASL_Hail,		/* Hailing text follows			*/
	ASL_Window,		/* Parent window for IDCMP and screen	*/
	ASL_LeftEdge,		/* Initialize LeftEdge			*/
	ASL_TopEdge,		/* Initialize TopEdge			*/
	ASL_Width,
	ASL_Height,
	ASL_HookFunc,		/* Hook function pointer		*/

/* Tags specific to file request					*/
	ASL_File,		/* Initial name of file follows		*/
	ASL_Dir,		/* Initial string for filerequest dir	*/

/* Tags specific to font request					*/
	ASL_FontName,		/* Initial font name			*/
	ASL_FontHeight,		/* Initial font height			*/
	ASL_FontStyles,		/* Initial font styles			*/
	ASL_FontFlags,		/* Initial font flags for textattr	*/
	ASL_FrontPen,		/* Initial frontpen color		*/
	ASL_BackPen,		/* Initial backpen color		*/
	ASL_MinHeight,		/* Minimum font height to display	*/
	ASL_MaxHeight,		/* Max font height to display		*/

	ASL_OKText,		/* Text displayed in OK gadget		*/
	ASL_CancelText,		/* Text displayed in CANCEL gadget	*/
	ASL_FuncFlags,		/* Function flags, depend on request	*/

	ASL_ModeList,		/* Substitute list for font drawmodes	*/		
};

#define	FILB_SAVE	5L /* For a SAVE operation, set this bit	*/

#define	FILF_SAVE	(1L << FILB_SAVE)
#endif

struct TagItem FileTags[] = {
	ASL_Hail, NULL,
	ASL_File, NULL,
	ASL_Dir, NULL,
/*	ASL_FuncFlags,(void *)FILF_SAVE, */
	TAG_DONE, NULL
};

/* struct FileRequester FR; */

#if 0
#asm

FileTags
		dc.l	$80080001			; ASL_Hail
		dc.l	0
		dc.l	$80080008			; ASL_File
		dc.l	0
		dc.l	$80080009			; ASL_Dir
		dc.l	0
		dc.l	$80080002			; ASL_Window
		dc.l	0
		dc.l	0					; TAG_DONE

_DoAslFR				; hail text poiner in a0, window ptr in a1
						; standard Amiga register use
	movem.l	d0/d1/a2/a6,-(sp)
	lea		FileTags,a2
	move.l	a0,4(a2)				; set hail
	move.l	d1,a6
	move.l	a6,12(a2)				; set file
	move.l	d0,a6
	move.l	a6,20(a2)				; set dir
	move.l	a1,28(a2)				; set window

	move.l	_AslBase,a6
	jsr		-$1e(a6)				; AllocFileRequest
	tst.l	d0
	beq.s	99$

	move.l	d0,a2					; save request ptr
	lea		FileTags,a1
	move.l	a2,a0
	jsr		-$3c(a6)				; AslRequest
	tst.l	d0
	beq.s	98$

	move.l	(sp),a1
	move.l	8(a2),a0
1$	move.b	(a0)+,(a1)+
	bne.s	1$

	move.l	4(sp),a1
	move.l	4(a2),a0
2$	move.b	(a0)+,(a1)+
	bne.s	2$

98$	exg		d0,a2					; save result, get request ptr

	move.l	d0,a0
	jsr		-$24(a6)				; FreeAslRequest

	move.l	a2,d0

99$	addq.w	#8,sp
	movem.l	(sp)+,a2/a6
	rts								; success value in d0

#endasm

long DoAslFR(char *msg,struct Window *back,char *dir,char *file);
#pragma regcall( DoAslFR(a0,a1,d0,d1) )
#endif

LONG DoFileRequest(char *dir,char *file,char *msg)
{	LONG	result;
	struct FileRequester *afr;

	ClearMenuStrip(window);
	ModifyIDCMP(window,MENUPICK);
	window->Flags |= RMBTRAP;

#if 0
	if (ArpBase)
	{	FR.fr_Hail = msg;
		FR.fr_File = file;
		FR.fr_Dir = dir;
		FR.fr_Window = window;

		result = FileRequest(&FR);
	}
	else DoAslFR(msg,window,dir,file);
#endif

	if (afr = AllocFileRequest())
	{
		FileTags[0].ti_Data = (ULONG)msg;
		FileTags[1].ti_Data = (ULONG)file;
		FileTags[2].ti_Data = (ULONG)dir;
		if (result = AslRequest( afr, FileTags ))
		{
			strcpy(dir,afr->rf_Dir);
			strcpy(file,afr->rf_File);
		}
		FreeAslRequest(afr);
	}

	window->Flags &= ~RMBTRAP;
	ModifyIDCMP(window,KEYIDCMP);
	SetMenuStrip(window,&ProjectMenu);

	return result;
}

struct RadioShape {
	UWORD	data[18];
};

struct RadioImage
{	struct RadioShape	plane1,
						plane2;
};

struct RadioShape RadOnData = {
   0x0000,0x0000,
   0x0000,0x0000,
   0x03E0,0x0000,
   0x0FF8,0x0000,
   0x0FF8,0x0000,
   0x0FF8,0x0000,
   0x03E0,0x0000,
   0x0000,0x0000,
   0x0000,0x0000
};

struct RadioShape RadHiData = {
   0x07F0,0x0000,
   0x3C1E,0x0000,
   0x6000,0x0000,
   0xC000,0x0000,
   0xC000,0x0000,
   0xC000,0x0000,
   0x6000,0x0000,
   0x0000,0x0000,
   0x0000,0x0000
};

struct RadioShape RadLoData = {
   0x0000,0x0000,
   0x0000,0x0000,
   0x0003,0x0000,
   0x0001,0x8000,
   0x0001,0x8000,
   0x0001,0x8000,
   0x0003,0x0000,
   0x3C1E,0x0000,
   0x07F0,0x0000
};

struct RadioImage *on_image,*off_image;

void copy_2_chip(void *source,void *desta,long size)
{	char **dest = (char **)desta;

	if (TypeOfMem(source) != MEMF_CHIP)
	{
		*dest = MemAlloc(size,MEMF_CHIP);
		if (*dest == NULL) cleanup("Out of memory",10);
		CopyMem(source,*dest,size);
	}
	else *dest = source;
}

make_images()
{
	on_image = MemAlloc(sizeof *on_image,MEMF_CHIP|MEMF_CLEAR);
	off_image = MemAlloc(sizeof *off_image,MEMF_CHIP|MEMF_CLEAR);

	radio_on.ImageData = (USHORT *)on_image;
	radio_off.ImageData = (USHORT *)off_image;

	if (LIGHT_HUE & 1)
	{	or_mem(&RadHiData,&off_image->plane1,sizeof (struct RadioShape));
		or_mem(&RadLoData,&on_image->plane1,sizeof (struct RadioShape));
	}
	if (LIGHT_HUE & 2)
	{	or_mem(&RadHiData,&off_image->plane2,sizeof (struct RadioShape));
		or_mem(&RadLoData,&on_image->plane2,sizeof (struct RadioShape));
	}
	if (SHADOW_HUE & 1)
	{	or_mem(&RadLoData,&off_image->plane1,sizeof (struct RadioShape));
		or_mem(&RadHiData,&on_image->plane1,sizeof (struct RadioShape));
	}
	if (SHADOW_HUE & 2)
	{	or_mem(&RadLoData,&off_image->plane2,sizeof (struct RadioShape));
		or_mem(&RadHiData,&on_image->plane2,sizeof (struct RadioShape));
	}
	if (HILITE_HUE & 1)
	{	or_mem(&RadOnData,&on_image->plane1,sizeof (struct RadioShape));
	}
	if (HILITE_HUE & 2)
	{	or_mem(&RadOnData,&on_image->plane2,sizeof (struct RadioShape));
	}
}

#asm

_or_mem:
			move.l		4(sp),a0
			move.l		8(sp),a1
			move.l		12(sp),d0

			bra.s		2$
1$			move.b		(a0)+,d1
			or.b		d1,(a1)+
2$			dbra		d0,1$

			rts

#endasm
