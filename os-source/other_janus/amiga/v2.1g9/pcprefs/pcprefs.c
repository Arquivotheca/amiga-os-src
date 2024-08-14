#include <stdio.h>

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuitionbase.h>
#include <libraries/dos.h>
#include <janus/janus.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>

#define USE_ACTIVE 0
#define GHOST_FLIPPER 0

/*
          Janus Handler Load Segment  |?|    A000    |

              Janus Handler Shadowed  |?|     Yes    |

Monochrome Display Adapter Emulation  |?|   Enabled  |

    Color Graphics Adapter Emulation  |?|  Disabled  |

    Default Bridgeboard Display Mode  |?| Monochrome |

                 Bridgeboard Speaker  |?|     Off    |

                Bridgeboard Drive A:  |?|  External  |

                Bridgeboard Drive B:  |?|   Shared   |

                  Shared Amiga Drive  |?|    DF0:    |

          Type Of Shared Amiga Drive  |?|    880K    |


| Save |              | Use |               | Cancel |

*/

/*
	000000000000000000000
	000001111111000000100
	000011000001100000100
	000011000001100000100
	000011000111111000100
	000011000011110000100
	000011000001100000100
	000011000000000000100
	000011000001100000100
	000001111111000000100
	000000000000000000100
	000000000000000000000

	000000000000000000000
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000010
	000000000000000000000
	000000000000000000000

	0000 0000 0000 0000 0000 0000 0000 0000
	0000 0111 1111 0000 0010 0000 0000 0000
	0000 1100 0001 1000 0010 0000 0000 0000
	0000 1100 0001 1000 0010 0000 0000 0000
	0000 1100 0111 1110 0010 0000 0000 0000
	0000 1100 0011 1100 0010 0000 0000 0000
	0000 1100 0001 1000 0010 0000 0000 0000
	0000 1100 0000 0000 0010 0000 0000 0000
	0000 1100 0001 1000 0010 0000 0000 0000
	0000 0111 1111 0000 0010 0000 0000 0000
	0000 0000 0000 0000 0010 0000 0000 0000
	0000 0000 0000 0000 0000 0000 0000 0000

	0000 0000 0000 0000 0000 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0001 0000 0000 0000
	0000 0000 0000 0000 0000 0000 0000 0000

*/

#define PREFFILE "sys:pc/system/2500Prefs"

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *ExpansionBase;
struct JanusBase *JanusBase;

struct NewWindow prefwindow = {
	0,0,456,200,				/* left,top,width,height */
	-1,-1,					/* detailpen,blockpen */
	REFRESHWINDOW | GADGETUP | GADGETDOWN,	/* idcmp */
	WINDOWDRAG | WINDOWDEPTH | SMART_REFRESH,	/* flags */
	0,				/* first gadget */
	0,					/* checkmark */
	"PC Configuration V3.0",		/* title *not relocatable* */
	0,					/* screen */
	0,					/* bitmap */
	0,0,0,0,				/* min/max width/height */
	WBENCHSCREEN				/* window type */
};

struct TextAttr common_font = {
	"topaz.font",			/* font we want */
	8,				/* YSize */
	FS_NORMAL,FPF_ROMFONT		/* style,flags */
};


USHORT __chip change_image_data[] = {
	0x0000, 0x0000,
	0x07f0, 0x2000,
	0x0c18, 0x2000,
	0x0c18, 0x2000,
	0x0c7e, 0x2000,
	0x0c3c, 0x2000,
	0x0c18, 0x2000,
	0x0c00, 0x2000,
	0x0c18, 0x2000,
	0x07f0, 0x2000,
	0x0000, 0x2000,
	0x0000, 0x0000,

	0x0000, 0x0000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x1000,
	0x0000, 0x0000
};

struct Image change_box_image = {
	0, 0,			/* upper left corner */
	21, 12, 2,		/* width, height, depth */
	&change_image_data[0],	/* data ptr */
	0x3, 0x0,		/* planepick, planeonoff */
	NULL			/* nextimage */
};

struct IntuiText misc_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	0, 0,				/* leftedge,topedge */
	&common_font,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText change_line_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	4, 3,				/* leftedge,topedge */
	&common_font,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText change_box_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	25, 3,				/* leftedge,topedge */
	&common_font,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct IntuiText button_text = {
	1, 0,				/* frontpen,backpen */
	JAM2,				/* drawmode */
	4, 3,				/* leftedge,topedge */
	&common_font,			/* font for all intuitext */
	"",				/* text string */
	0				/* next text */
};

struct change_line {
	struct Node node;		/* linkage */
	struct Gadget gad;		/* the gadget struct */
	int change_text_x;		/* x posn of label */
	int change_text_y;		/* y posn of label */
	int change_text_width;		/* width of label area */
	int change_box_x;		/* x posn of box */
	int change_box_y;		/* y posn of box */
	int change_box_width;		/* width of box */
	int change_box_height;		/* height of box */
	int num;			/* which number */
	char *text;			/* text to left of changebox */
	char **values;			/* array of value text */
	int value;			/* current value # */
	int old_value;			/* original value */
	void (*next_value)();		/* returns next value */
};

struct button {
	struct Gadget gad;
	int on;
	int x, y, w, h;
	char *text;
};

struct change_line janus_segment, janus_shadowed_cl, mda_emul_cl, cga_emul_cl,
		   default_vid_mode_cl, speaker_cl, drive_a_cl, drive_b_cl,
		   shared_drive_cl, shared_type_cl;

struct button cancel_button, use_button, save_button;

struct RastPort *rp;			/* our window's rastport */

struct Screen wbscr;

struct Window *w;

int wbscr_barheight;
int wbscr_width;
int wbscr_height;

struct List change_list;		/* list of lines */

#define A1060		0
#define A2088		1
#define A2088T		2
#define A2286		3
#define A2386SX		4
int bbtype;
char *bbnames[] = { "A1060", "A2088", "A2088T", "A2286", "A2386SX" };

UBYTE volatile *janus_config;
UBYTE volatile *a2386_config;
UBYTE prefs[2];

ULONG changes;

#define GENERAL_CHANGED			(1 << 0)
#define JANUS_SEGMENT_CHANGED		(1 << 1)
#define JANUS_SHADOWED_CHANGED		(1 << 2)
#define MDA_EMUL_CHANGED		(1 << 3)
#define CGA_EMUL_CHANGED		(1 << 4)
#define DEFAULT_VID_MODE_CHANGED	(1 << 5)
#define SPEAKER_CHANGED			(1 << 6)
#define DRIVE_A_CHANGED			(1 << 7)
#define DRIVE_B_CHANGED			(1 << 8)
#define SHARED_DRIVE_CHANGED		(1 << 9)
#define SHARED_TYPE_CHANGED		(1 << 10)

#define CHANGE_CABLING	(SHARED_DRIVE_CHANGED)

#define REBOOT_AMIGA	(JANUS_SEGMENT_CHANGED | JANUS_SHADOWED_CHANGED)

#define REBOOT_PC	(MDA_EMUL_CHANGED | CGA_EMUL_CHANGED \
			 | DEFAULT_VID_MODE_CHANGED | DRIVE_A_CHANGED \
			 | DRIVE_B_CHANGED)

#define SAVE_OK		(JANUS_SEGMENT_CHANGED | JANUS_SHADOWED_CHANGED \
			 | MDA_EMUL_CHANGED | CGA_EMUL_CHANGED \
			 | DEFAULT_VID_MODE_CHANGED | SPEAKER_CHANGED \
			 | DRIVE_A_CHANGED | DRIVE_B_CHANGED \
			 | SHARED_DRIVE_CHANGED | SHARED_TYPE_CHANGED)

#define USE_OK		(REBOOT_PC | SPEAKER_CHANGED)

#define USE_BAD		(REBOOT_AMIGA)

#define FLIPPER_CHANGED	(DRIVE_A_CHANGED | DRIVE_B_CHANGED \
			 | SHARED_DRIVE_CHANGED | SHARED_TYPE_CHANGED)


#define START_FLIPPER	"run <nil: >nil: sys:pc/flipper <nil: >nil: auto window=450,0"
#define STOP_FLIPPER	"sys:pc/flipper <nil: >nil: quit"

#define CHANGELINE_GADGET 0
#define CANCEL_GADGET 1
#define USE_GADGET 2
#define SAVE_GADGET 3

#define JANUS_SEGMENT_NONE	0
#define JANUS_SEGMENT_A000	1
#define JANUS_SEGMENT_D000	2
#define JANUS_SEGMENT_E000	3

#define JANUS_SHADOWED_NO	0
#define JANUS_SHADOWED_YES	1

#define MDA_EMUL_NO		0
#define MDA_EMUL_YES		1

#define CGA_EMUL_NO		0
#define CGA_EMUL_YES		1

#define DEFAULT_VID_MODE_MDA	0
#define DEFAULT_VID_MODE_CGA	1

#define SPEAKER_OFF		0
#define SPEAKER_ON		1

#define DRIVE_A_INTERNAL	0
#define DRIVE_A_EXTERNAL	1
#define DRIVE_A_SHARED		2

#define DRIVE_B_INTERNAL	0
#define DRIVE_B_EXTERNAL	1
#define DRIVE_B_SHARED		2

#define SHARED_DRIVE_DF0	0
#define SHARED_DRIVE_DF1	1

#define SHARED_TYPE_NORMAL	0
#define SHARED_TYPE_DUALSPEED	1


struct change_line janus_segment_cl, janus_shadowed_cl, mda_emul_cl, cga_emul_cl,
		   default_vid_mode_cl, speaker_cl, drive_a_cl, drive_b_cl,
		   shared_drive_cl, shared_type_cl;

char *janus_segment_values[] = {
	"Don't Load", "A000", "D000", "E000"
};

char *janus_shadowed_values[] = {
	"No", "Yes"
};

char *mda_emul_values[] = {
	"Disabled", "Enabled"
};

char *cga_emul_values[] = {
	"Disabled", "Enabled"
};

char *default_vid_mode_values[] = {
	"Monochrome", "Color"
};

char *speaker_values[] = {
	"Off", "On"
};

char *drive_a_values[] = {
	"Internal", "External", "Shared"
};

char *drive_b_values[] = {
	"Internal", "External", "Shared"
};

char *shared_drive_values[] = {
	"DF0:", "DF1:"
};

char *shared_type_values[] = {
	"Standard", "Dual Speed"
};

struct IntuiText req_body_text2 = {
	0, 1, JAM2, 6, 3+12,
	&common_font, 0, 0
};

struct IntuiText req_body_text1 = {
	0, 1, JAM2, 6, 3,
	&common_font, 0, &req_body_text2
};

struct IntuiText req_pos_text = {
	0, 1, JAM2, 6, 3,
	&common_font, 0, 0
};

struct IntuiText req_neg_text = {
	0, 1, JAM2, 6, 3,
	&common_font, 0, 0
};

int error(char *t1, char *t2, char *pt, char *nt)
{
	req_body_text1.IText = t1;
	req_body_text2.IText = t2;
	req_pos_text.IText = pt;
	req_neg_text.IText = nt;

	return AutoRequest(w, &req_body_text1, &req_pos_text, &req_neg_text,
			   0, 0, 320, 60);
}

void off_gadget(struct Gadget *g)
{
	OffGadget(g, w, 0);
}

void on_gadget(struct Gadget *g)
{
	OnGadget(g, w, 0);
}

void next_janus_segment_value()
{
	do {
		janus_segment_cl.value++;
		janus_segment_cl.value %= (sizeof(janus_segment_values) / sizeof(char *));
	} while (janus_segment_cl.value == JANUS_SEGMENT_E000  &&
		 (bbtype == A2286 || bbtype == A2386SX));

	if (janus_segment_cl.value == janus_segment_cl.old_value)
		changes &= ~JANUS_SEGMENT_CHANGED;
	else
		changes |= JANUS_SEGMENT_CHANGED;
}

void next_janus_shadowed_value()
{
	janus_shadowed_cl.value++;
	janus_shadowed_cl.value %= (sizeof(janus_shadowed_values) / sizeof(char *));

	if (janus_shadowed_cl.value == janus_shadowed_cl.old_value)
		changes &= ~JANUS_SHADOWED_CHANGED;
	else
		changes |= JANUS_SHADOWED_CHANGED;
}

void next_mda_emul_value()
{
	mda_emul_cl.value++;
	mda_emul_cl.value %= (sizeof(mda_emul_values) / sizeof(char *));

	if (mda_emul_cl.value == mda_emul_cl.old_value)
		changes &= ~MDA_EMUL_CHANGED;
	else
		changes |= MDA_EMUL_CHANGED;
}

void next_cga_emul_value()
{
	cga_emul_cl.value++;
	cga_emul_cl.value %= (sizeof(cga_emul_values) / sizeof(char *));

	if (cga_emul_cl.value == cga_emul_cl.old_value)
		changes &= ~CGA_EMUL_CHANGED;
	else
		changes |= CGA_EMUL_CHANGED;
}

void next_default_vid_mode_value()
{
	default_vid_mode_cl.value++;
	default_vid_mode_cl.value %= (sizeof(default_vid_mode_values) / sizeof(char *));

	if (default_vid_mode_cl.value == default_vid_mode_cl.old_value)
		changes &= ~DEFAULT_VID_MODE_CHANGED;
	else
		changes |= DEFAULT_VID_MODE_CHANGED;
}

void next_speaker_value()
{
	speaker_cl.value++;
	speaker_cl.value %= (sizeof(speaker_values) / sizeof(char *));

	if (speaker_cl.value == speaker_cl.old_value)
		changes &= ~SPEAKER_CHANGED;
	else
		changes |= SPEAKER_CHANGED;
}

extern void flipper_gads_onoff();

void next_drive_a_value()
{
int old;

	old = (drive_a_cl.value == DRIVE_A_SHARED);

	do {
		drive_a_cl.value++;
		drive_a_cl.value %= (sizeof(drive_a_values) / sizeof(char *));
	} while ((drive_a_cl.value == DRIVE_A_SHARED && drive_b_cl.value == DRIVE_B_SHARED)
	      || (drive_a_cl.value == DRIVE_A_EXTERNAL && drive_b_cl.value == DRIVE_B_EXTERNAL));

	if (drive_a_cl.value == drive_a_cl.old_value)
		changes &= ~DRIVE_A_CHANGED;
	else
		changes |= DRIVE_A_CHANGED;

	if (old != (drive_a_cl.value == DRIVE_A_SHARED))
		flipper_gads_onoff();
}

void next_drive_b_value()
{
int old;

	old = (drive_b_cl.value == DRIVE_B_SHARED);

	do {
		drive_b_cl.value++;
		drive_b_cl.value %= (sizeof(drive_b_values) / sizeof(char *));
	} while ((drive_b_cl.value == DRIVE_B_SHARED && drive_a_cl.value == DRIVE_A_SHARED)
	      || (drive_b_cl.value == DRIVE_B_EXTERNAL && drive_a_cl.value == DRIVE_A_EXTERNAL));

	if (drive_b_cl.value == drive_b_cl.old_value)
		changes &= ~DRIVE_B_CHANGED;
	else
		changes |= DRIVE_B_CHANGED;

	if (old != (drive_b_cl.value == DRIVE_B_SHARED))
		flipper_gads_onoff();
}

void next_shared_drive_value()
{
	shared_drive_cl.value++;
	shared_drive_cl.value %= (sizeof(shared_drive_values) / sizeof(char *));

	if (shared_drive_cl.value == shared_drive_cl.old_value)
		changes &= ~SHARED_DRIVE_CHANGED;
	else
		changes |= SHARED_DRIVE_CHANGED;
}

void next_shared_type_value()
{
	shared_type_cl.value++;
	shared_type_cl.value %= (sizeof(shared_type_values) / sizeof(char *));

	if (shared_type_cl.value == shared_type_cl.old_value)
		changes &= ~SHARED_TYPE_CHANGED;
	else
		changes |= SHARED_TYPE_CHANGED;
}

void DrawLine(struct RastPort *rp, int x1, int y1, int x2, int y2)
{
	Move(rp, x1, y1);
	Draw(rp, x2, y2);
}

void draw_change_text(int x, int y, int w, char *text)
{
int width;
int h = 8;

	SetDrMd(rp, JAM1);

	/* clear it */
	SetAPen(rp, 0);
	RectFill(rp, x, y, x+w-1, y+h-1);

	/* fill in text assuming 8 bit high font */
	change_line_text.IText = text;
	width = w - 2 * 4;
	PrintIText(rp, &change_line_text,
		x + width - IntuiTextLength(&change_line_text),
		y);
}

void draw_change_box(int x, int y, int w, int h, char *text)
{
int width;

	SetDrMd(rp, JAM1);

	/* clear it */
	SetAPen(rp, 0);
	RectFill(rp, x, y, x+w-1, y+h-1);

	/* draw border */
	SetAPen(rp, 2);
	DrawLine(rp, x, y, x+w-1, y);	/* white line across top */
	DrawLine(rp, x, y, x, y+h-1);	/* 1st white line down left */
	DrawLine(rp, x+1, y+1, x+1, y+h-2);	/* 2nd white line down left */
	SetAPen(rp, 1);
	DrawLine(rp, x+1, y+h-1, x+w-1, y+h-1);	/* black line along bottom */
	DrawLine(rp, x+w-1, y, x+w-1, y+h-1);	/* 1st black line down right */
	DrawLine(rp, x+w-2, y+1, x+w-2, y+h-1);	/* 2nd black line down right */

	/* draw select thingy */
	DrawImage(rp, &change_box_image, 2 + x, 1 + y);

	/* fill in text assuming 8 bit high font */
	change_box_text.IText = text;
	width = w - change_box_text.LeftEdge - 4;
	PrintIText(rp, &change_box_text,
		(width - IntuiTextLength(&change_box_text)) / 2 + x,
		y);
}

void draw_change_gadget(struct change_line *cl)
{
	RemoveGadget(w, &cl->gad);

	draw_change_box(cl->change_box_x, cl->change_box_y,
			cl->change_box_width, cl->change_box_height,
			cl->values[cl->value]);

	AddGadget(w, &cl->gad, 0);
}

void flipper_gads_onoff()
{
#if GHOST_FLIPPER
	if (bbtype != A2386SX)
		return;

	if (drive_a_cl.value == DRIVE_A_SHARED || drive_b_cl.value == DRIVE_B_SHARED) {
		on_gadget(&shared_drive_cl.gad);
		draw_change_gadget(&shared_drive_cl);

		on_gadget(&shared_type_cl.gad);
		draw_change_gadget(&shared_type_cl);
	} else {
		off_gadget(&shared_drive_cl.gad);
		off_gadget(&shared_type_cl.gad);
	}
#endif
}

void draw_change_lines()
{
struct change_line *cl, *ncl;

	for (cl = (struct change_line *)change_list.lh_Head;
	     ncl = (struct change_line *)cl->node.ln_Succ;
	     cl = ncl) {
		draw_change_text(cl->change_text_x, cl->change_text_y,
				 cl->change_text_width, cl->text);
		draw_change_gadget(cl);
	}
}

void draw_button(struct button *b)
{
int x, y, wid, h, width;

	RemoveGadget(w, &b->gad);

	x = b->x;
	y = b->y;
	wid = b->w;
	h = b->h;

	SetDrMd(rp, JAM1);

	/* clear it */
	SetAPen(rp, 0);
	RectFill(rp, x, y, x+wid-1, y+h-1);

	/* draw border */
	SetAPen(rp, 2);
	DrawLine(rp, x, y, x+wid-1, y);	/* white line across top */
	DrawLine(rp, x, y, x, y+h-1);	/* 1st white line down left */
	DrawLine(rp, x+1, y+1, x+1, y+h-2);	/* 2nd white line down left */
	SetAPen(rp, 1);
	DrawLine(rp, x+1, y+h-1, x+wid-1, y+h-1);	/* black line along bottom */
	DrawLine(rp, x+wid-1, y, x+wid-1, y+h-1);	/* 1st black line down right */
	DrawLine(rp, x+wid-2, y+1, x+wid-2, y+h-1);	/* 2nd black line down right */

	/* fill in text assuming 8 bit high font */
	button_text.IText = b->text;
	width = wid - 2 * button_text.LeftEdge;
	PrintIText(rp, &button_text,
		(width - IntuiTextLength(&button_text)) / 2 + x,
		y);

	AddGadget(w, &b->gad, 0);
}

void draw_buttons()
{
	draw_button(&cancel_button);
#if USE_ACTIVE
	draw_button(&use_button);
#endif
	draw_button(&save_button);
}

void init_change_line(struct change_line *cl, char *ltext, char **vtext,
		      void (*next_value)(), int num)
{
int change_line_height = 14;	/* number of pixels occupied vertically */
int change_line_y = 16;		/* offset from top of window */

int change_text_x = 15;		/* offset from left of window for label*/
int change_text_width = 300;	/* width of label area */

int change_box_x = change_text_x + change_text_width + 5; /* offset from left of window box */
int change_box_width = 120;	/* width of box */
int change_box_height = 1 + 2 + 8 + 2 + 1;	/* height of box */

/* note:
 *
 * we don't initialize the NextGadget or node pointers
 */

	/* init misc junk */
	cl->num = num;
	cl->text = ltext;
	cl->values = vtext;
	cl->old_value = cl->value;
	cl->next_value = next_value;

	/* figure out where the label goes */
	cl->change_text_x = change_text_x;
	cl->change_text_y = wbscr_barheight + change_line_y + change_line_height * cl->num;
	cl->change_text_width = change_text_width;

	/* and the box */
	cl->change_box_x = change_box_x;
	cl->change_box_y = wbscr_barheight + change_line_y + change_line_height * cl->num;
	cl->change_box_width = change_box_width;
	cl->change_box_height = change_box_height;

	/* init gadget */
	cl->gad.NextGadget = 0;
	cl->gad.Flags = GADGHCOMP;
	cl->gad.Activation = RELVERIFY;
	cl->gad.GadgetType = BOOLGADGET;
	cl->gad.GadgetRender = 0;
	cl->gad.SelectRender = 0;
	cl->gad.GadgetText = 0;
	cl->gad.MutualExclude = 0;
	cl->gad.SpecialInfo = 0;
	cl->gad.GadgetID = CHANGELINE_GADGET;
	cl->gad.UserData = (APTR)cl;

	/* gadget's select box is the box */
	cl->gad.LeftEdge = cl->change_box_x;
	cl->gad.TopEdge = cl->change_box_y;
	cl->gad.Width = cl->change_box_width;
	cl->gad.Height = cl->change_box_height;

	/* link it */
	memset(&cl->node, 0, sizeof(struct Node));
	AddTail(&change_list, &cl->node);

	prefwindow.Height = cl->change_box_y + cl->change_box_height + 34;
}

void init_change_lines()
{
int i;

	i = 0;

	NewList(&change_list);

	init_change_line(&janus_segment_cl, "Janus Handler Load Segment:",
			 janus_segment_values, next_janus_segment_value, i++);

	if (bbtype == A2386SX) {
		init_change_line(&janus_shadowed_cl, "Janus Handler Shadowed:",
				 janus_shadowed_values, next_janus_shadowed_value, i++);
	}

	init_change_line(&mda_emul_cl, "Monochrome Display Adapter Emulation:",
			 mda_emul_values, next_mda_emul_value, i++);

	init_change_line(&cga_emul_cl, "Color Graphics Adapter Emulation:",
			 cga_emul_values, next_cga_emul_value, i++);

	if (bbtype == A2386SX) {
		init_change_line(&default_vid_mode_cl, "Default Bridgeboard Video Mode:",
				 default_vid_mode_values, next_default_vid_mode_value, i++);

		init_change_line(&speaker_cl, "Bridgeboard Speaker:",
				 speaker_values, next_speaker_value, i++);

		init_change_line(&drive_a_cl, "Drive A:",
				 drive_a_values, next_drive_a_value, i++);

		init_change_line(&drive_b_cl, "Drive B:",
				 drive_b_values, next_drive_b_value, i++);

		init_change_line(&shared_drive_cl, "Shared Amiga Drive:",
				 shared_drive_values, next_shared_drive_value, i++);

		init_change_line(&shared_type_cl, "Shared Amiga Drive Type:",
				 shared_type_values, next_shared_type_value, i++);
	}
}

void init_button(struct button *b, int gadid, char *text, int x, int y)
{
	/* init button junk */
	b->x = x;
	b->y = y;
	b->w = 60;
	b->h = 1 + 2 + 8 + 2 + 1;
	b->text = text;

	/* init gadget */
	b->gad.NextGadget = 0;
	b->gad.Flags = GADGHCOMP;
	b->gad.Activation = RELVERIFY;
	b->gad.GadgetType = BOOLGADGET;
	b->gad.GadgetRender = 0;
	b->gad.SelectRender = 0;
	b->gad.GadgetText = 0;
	b->gad.MutualExclude = 0;
	b->gad.SpecialInfo = 0;
	b->gad.GadgetID = gadid;
	b->gad.UserData = (APTR)b;

	/* gadget's select box is the box */
	b->gad.LeftEdge = b->x;
	b->gad.TopEdge = b->y;
	b->gad.Width = b->w;
	b->gad.Height = b->h;
}

void init_buttons()
{
int x, y;
int xi;

#if USE_ACTIVE
	xi = prefwindow.Width / 3;
#else
	xi = prefwindow.Width / 2;
#endif
	y = prefwindow.Height - 25;
	x = xi/2 - 60/2;

	init_button(&save_button, SAVE_GADGET, "Save", x, y);
	x += xi;

#if USE_ACTIVE
	init_button(&use_button, USE_GADGET, "Use", x, y);
	x += xi;
#endif

	init_button(&cancel_button, CANCEL_GADGET, "Cancel", x, y);
}

void link_change_lines()
{
struct change_line *cl, *ncl;

	for (cl = (struct change_line *)change_list.lh_Head;
	     ncl = (struct change_line *)cl->node.ln_Succ;
	     cl = ncl) {
		AddGadget(w, &cl->gad, 0);
	}
}

void link_buttons()
{
	AddGadget(w, &cancel_button.gad, 0);

#if USE_ACTIVE
	AddGadget(w, &use_button.gad, 0);
#endif

	AddGadget(w, &save_button.gad, 0);
}

void init_gadgets()
{
	init_change_lines();

	init_buttons();
}

void link_gadgets()
{
	link_change_lines();

	link_buttons();
}

void draw_header()
{
char s[80];
int x;

	strcpy(s, bbnames[bbtype]);

	if (bbtype == A1060)
		strcat(s, " Sidecar");
	else
		strcat(s, " Bridgeboard");

	misc_text.IText = s;

	x = (prefwindow.Width - IntuiTextLength(&misc_text)) / 2;

	PrintIText(rp, &misc_text, x, wbscr_barheight + 4);
}

void draw_screen()
{
	draw_header();

	draw_change_lines();

	draw_buttons();
}

int set_bbtype()
{
int retval;
UBYTE t;

	/* gimme configuration register */
	janus_config = ((UBYTE *)JanusBase->jb_IoBase) + jio_PCconfiguration;
	a2386_config = ((UBYTE *)JanusBase->jb_IoBase) + 0x1f9f;

	/* assume ok */
	retval = 1;

	/* get expansion */
	if (ExpansionBase = (struct Library *) OpenLibrary("expansion.library",0)) {

		/* is this mfg 513, product 1? */
		if (FindConfigDev(0, 513, 1)) {

			/* yes - 2088/sidecar or 2286? (XT bit set?) */
			if (*janus_config & 0x80) {

				/* 2088/sidecar.  fiddle with KBENB bit.
				 * if we can change the bit, it's a 2088.
				 * the sidecar has a dip switch here.
				 */
				t = *janus_config;
				*janus_config = t ^ 0x04;
				if (*janus_config == (t ^ 0x04)) {

					/* we changed it - 2088 */
					bbtype = A2088;
				} else {

					/* sticky bits - sidecar */
					bbtype = A1060;
				}

				/* fix our meddling */
				*janus_config = t;

			} else {

				/* 2286. */
				bbtype = A2286;
			}

		/* hmm, how about a 2386? */
		} else if (FindConfigDev(0, 513, 103)) {

			/* yes. */
			bbtype = A2386SX;

#if A2088T_DEFINED
		/* in the future, there may be a 2088T */
		} else if (FindConfigDev(0, ???, ???)) {
			bbtype = A2088T;
#endif
		} else {

			/* none of the above. */
			retval = 0;
		}

		/* no more expansion. */
		CloseLibrary(ExpansionBase);

	} else {

		/* oops. */
		retval = 0;
	}

	return retval;
}

void read_stuff()
{
int n;
BPTR f;

	/* set default values */
	switch (bbtype) {
	case A1060:
	case A2088:
	case A2088T:
		janus_segment_cl.value = JANUS_SEGMENT_E000;
		janus_shadowed_cl.value = JANUS_SHADOWED_NO;
		mda_emul_cl.value = MDA_EMUL_YES;
		cga_emul_cl.value = MDA_EMUL_YES;
		default_vid_mode_cl.value = DEFAULT_VID_MODE_MDA;
		speaker_cl.value = SPEAKER_ON;
		drive_a_cl.value = DRIVE_A_INTERNAL;
		drive_b_cl.value = DRIVE_B_INTERNAL;
		shared_drive_cl.value = SHARED_DRIVE_DF0;
		shared_type_cl.value = SHARED_TYPE_NORMAL;
		break;

	case A2286:
		janus_segment_cl.value = JANUS_SEGMENT_D000;
		janus_shadowed_cl.value = JANUS_SHADOWED_NO;
		mda_emul_cl.value = MDA_EMUL_YES;
		cga_emul_cl.value = MDA_EMUL_YES;
		default_vid_mode_cl.value = DEFAULT_VID_MODE_MDA;
		speaker_cl.value = SPEAKER_ON;
		drive_a_cl.value = DRIVE_A_INTERNAL;
		drive_b_cl.value = DRIVE_B_INTERNAL;
		shared_drive_cl.value = SHARED_DRIVE_DF0;
		shared_type_cl.value = SHARED_TYPE_NORMAL;
		break;

	case A2386SX:
		janus_segment_cl.value = JANUS_SEGMENT_D000;
		janus_shadowed_cl.value = JANUS_SHADOWED_YES;
		mda_emul_cl.value = MDA_EMUL_YES;
		cga_emul_cl.value = MDA_EMUL_YES;
		default_vid_mode_cl.value = DEFAULT_VID_MODE_MDA;
		speaker_cl.value = SPEAKER_ON;
		drive_a_cl.value = DRIVE_A_INTERNAL;
		drive_b_cl.value = DRIVE_B_INTERNAL;
		shared_drive_cl.value = SHARED_DRIVE_DF0;
		shared_type_cl.value = SHARED_TYPE_NORMAL;
		break;
	}

	if (bbtype == A1060) {
		prefs[0] = *janus_config;
		prefs[1] = 0;
		n = 2;
	} else {
		n = 0;
		if (f = Open(PREFFILE, MODE_OLDFILE)) {
			n = Read(f, &prefs[0], sizeof(prefs));
			Close(f);
		}
	}

	prefs[0] |= 0x87;

	if (n > 0) {

		/* do segment */
		switch (prefs[0] & 0x60) {
		case 0x00:
			janus_segment_cl.value = JANUS_SEGMENT_NONE;
			break;
		case 0x20:
			janus_segment_cl.value = JANUS_SEGMENT_A000;
			break;
		case 0x40:
			janus_segment_cl.value = JANUS_SEGMENT_D000;
			break;
		case 0x60:
			if (bbtype == A1060 || bbtype == A2088 || bbtype == A2088T) {
				janus_segment_cl.value = JANUS_SEGMENT_E000;
			}
			break;
		}

		/* do CGA */
		switch (prefs[0] & 0x10) {
		case 0x00:
			cga_emul_cl.value = CGA_EMUL_NO;
			break;
		case 0x10:
			cga_emul_cl.value = CGA_EMUL_YES;
			break;
		}

		/* do MDA */
		switch (prefs[0] & 0x08) {
		case 0x00:
			mda_emul_cl.value = MDA_EMUL_NO;
			break;
		case 0x08:
			mda_emul_cl.value = MDA_EMUL_YES;
			break;
		}

	}

	if (n > 1) {

		/* do speaker */
		switch (prefs[1] & 0x80) {
		case 0x80:
			speaker_cl.value = SPEAKER_OFF;
			break;
		case 0x00:
			speaker_cl.value = SPEAKER_ON;
			break;
		}

		/* do video mode */
		switch (prefs[1] & 0x40) {
		case 0x00:
			default_vid_mode_cl.value = DEFAULT_VID_MODE_MDA;
			break;
		case 0x40:
			default_vid_mode_cl.value = DEFAULT_VID_MODE_CGA;
			break;
		}

		/* do a/b drive config */
		switch (prefs[1] & 0x38) {
		case 0x00:
			drive_a_cl.value = DRIVE_A_INTERNAL;
			drive_b_cl.value = DRIVE_B_INTERNAL;
			break;
		case 0x08:
			drive_a_cl.value = DRIVE_A_INTERNAL;
			drive_b_cl.value = DRIVE_B_SHARED;
			break;
		case 0x10:
			drive_a_cl.value = DRIVE_A_INTERNAL;
			drive_b_cl.value = DRIVE_B_EXTERNAL;
			break;
		case 0x18:
			drive_a_cl.value = DRIVE_A_EXTERNAL;
			drive_b_cl.value = DRIVE_B_INTERNAL;
			break;
		case 0x20:
			drive_a_cl.value = DRIVE_A_SHARED;
			drive_b_cl.value = DRIVE_B_INTERNAL;
			break;
		case 0x28:
			drive_a_cl.value = DRIVE_A_EXTERNAL;
			drive_b_cl.value = DRIVE_B_SHARED;
			break;
		case 0x30:
			drive_a_cl.value = DRIVE_A_SHARED;
			drive_b_cl.value = DRIVE_B_EXTERNAL;
			break;
		case 0x38:
			/* illegal */
			break;
		}

		/* do shared drive */
		switch (prefs[1] & 0x04) {
		case 0x00:
			shared_drive_cl.value = SHARED_DRIVE_DF0;
			break;
		case 0x04:
			shared_drive_cl.value = SHARED_DRIVE_DF1;
			break;
		}

		/* do shared drive type */
		switch (prefs[1] & 0x02) {
		case 0x00:
			shared_type_cl.value = SHARED_TYPE_NORMAL;
			break;
		case 0x02:
			shared_type_cl.value = SHARED_TYPE_DUALSPEED;
			break;
		}

		/* do shadowed janus */
		switch (prefs[1] & 0x01) {
		case 0x00:
			janus_shadowed_cl.value = JANUS_SHADOWED_NO;
			break;
		case 0x01:
			janus_shadowed_cl.value = JANUS_SHADOWED_YES;
			break;
		}
	}
}

void build_prefs(UBYTE *p)
{

	p[0] = 0x87;	/* bit 7 on, bit 2 on, bit 1 on, bit 0 on */
	p[1] = 0;

	/* do segment */
	switch (janus_segment_cl.value) {
	case JANUS_SEGMENT_NONE:
		p[0] |= 0x00;
		break;
	case JANUS_SEGMENT_A000:
		p[0] |= 0x20;
		break;
	case JANUS_SEGMENT_D000:
		p[0] |= 0x40;
		break;
	case JANUS_SEGMENT_E000:
		p[0] |= 0x60;
		break;
	}

	/* do CGA */
	switch (cga_emul_cl.value) {
	case CGA_EMUL_NO:
		p[0] |= 0x00;
		break;
	case CGA_EMUL_YES:
		p[0] |= 0x10;
		break;
	}

	/* do MDA */
	switch (mda_emul_cl.value) {
	case MDA_EMUL_NO:
		p[0] |= 0x00;
		break;
	case MDA_EMUL_YES:
		p[0] |= 0x08;
		break;
	}

	/* do speaker */
	switch (speaker_cl.value) {
	case SPEAKER_OFF:
		p[1] |= 0x80;
		break;
	case SPEAKER_ON:
		p[1] |= 0x00;
		break;
	}

	/* do video mode */
	switch (default_vid_mode_cl.value) {
	case DEFAULT_VID_MODE_MDA:
		p[1] |= 0;
		break;
	case DEFAULT_VID_MODE_CGA:
		p[1] |= 0x40;
		break;
	}

	/* do a/b drive config */
	if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_INTERNAL)
		p[1] |= 0x00;
	else if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_SHARED)
		p[1] |= 0x08;
	else if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_EXTERNAL)
		p[1] |= 0x10;
	else if (drive_a_cl.value == DRIVE_A_EXTERNAL && drive_b_cl.value == DRIVE_B_INTERNAL)
		p[1] |= 0x18;
	else if (drive_a_cl.value == DRIVE_A_SHARED && drive_b_cl.value == DRIVE_B_INTERNAL)
		p[1] |= 0x20;
	else if (drive_a_cl.value == DRIVE_A_EXTERNAL && drive_b_cl.value == DRIVE_B_SHARED)
		p[1] |= 0x28;
	else if (drive_a_cl.value == DRIVE_A_SHARED && drive_b_cl.value == DRIVE_B_EXTERNAL)
		p[1] |= 0x30;

	/* do shared drive */
	switch (shared_drive_cl.value) {
	case SHARED_DRIVE_DF0:
		p[1] |= 0x00;
		break;
	case SHARED_DRIVE_DF1:
		p[1] |= 0x04;
		break;
	}

	/* do shared drive type */
	switch (shared_type_cl.value) {
	case SHARED_TYPE_NORMAL:
		p[1] |= 0x00;
		break;
	case SHARED_TYPE_DUALSPEED:
		p[1] |= 0x02;
		break;
	}

	/* do shadowed janus */
	switch (janus_shadowed_cl.value) {
	case JANUS_SHADOWED_NO:
		p[1] |= 0x00;
		break;
	case JANUS_SHADOWED_YES:
		p[1] |= 0x01;
		break;
	}
}

void update_stuff()
{
UBYTE p[2];
struct FileHandle *nilfh;

	build_prefs(p);

	/* update 2088/2286 stuff (only mono/color enables) */
	*janus_config = (p[0] & 0x18) | (prefs[0] & ~0x18);

	/* update 2386 stuff */
	if (bbtype == A2386SX) {
		if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_INTERNAL) {
			*a2386_config = 0x00;
			*a2386_config = 0x02;
			*a2386_config = 0x04;
		} else if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_SHARED) {
			*a2386_config = 0x00;
			*a2386_config = 0x03;
			*a2386_config = 0x04;
		} else if (drive_a_cl.value == DRIVE_A_INTERNAL && drive_b_cl.value == DRIVE_B_EXTERNAL) {
			*a2386_config = 0x01;
			*a2386_config = 0x03;
			*a2386_config = 0x04;
		} else if (drive_a_cl.value == DRIVE_A_EXTERNAL && drive_b_cl.value == DRIVE_B_INTERNAL) {
			*a2386_config = 0x00;
			*a2386_config = 0x02;
			*a2386_config = 0x05;
		} else if (drive_a_cl.value == DRIVE_A_SHARED && drive_b_cl.value == DRIVE_B_INTERNAL) {
			*a2386_config = 0x01;
			*a2386_config = 0x02;
			*a2386_config = 0x05;
		} else if (drive_a_cl.value == DRIVE_A_EXTERNAL && drive_b_cl.value == DRIVE_B_SHARED) {
			*a2386_config = 0x01;
			*a2386_config = 0x02;
			*a2386_config = 0x04;
		} else if (drive_a_cl.value == DRIVE_A_SHARED && drive_b_cl.value == DRIVE_B_EXTERNAL) {
			*a2386_config = 0x01;
			*a2386_config = 0x03;
			*a2386_config = 0x05;
		}

		if (speaker_cl.value == SPEAKER_ON) {
			*a2386_config = 0x07;
		} else {
			*a2386_config = 0x06;
		}

		if (default_vid_mode_cl.value == DEFAULT_VID_MODE_MDA) {
			*a2386_config = 0x09;
		} else {
			*a2386_config = 0x08;
		}

		if (changes & FLIPPER_CHANGED) {
			if (nilfh = Open("NIL:", MODE_NEWFILE)) {
				Execute(STOP_FLIPPER, 0, nilfh);
				Execute(START_FLIPPER, 0, nilfh);
				Close(nilfh);
			}
		}
	}
}

int save_stuff()
{
UBYTE p[2];
BPTR f;

	build_prefs(p);

	if (f = Open(PREFFILE, MODE_NEWFILE)) {
		Write(f, &p[0], sizeof(p));
		Close(f);

		return 1;
	} else {
		error("Could not create 2500Prefs file.", 0, "OK", 0);
		return 0;
	}
}

int warnings()
{
	if (changes & CHANGE_CABLING)
		return error("You must change cabling/jumpers",
			     "for the change(s) to take effect.",
			     "OK", "Cancel");

	if (changes & REBOOT_PC)
		return error("You must reboot the PC side",
			     "for the change(s) to take effect.",
			     "OK", "Cancel");

	if (changes & REBOOT_AMIGA)
		return error("You must reboot the Amiga",
			     "for the change(s) to take effect.",
			     "OK", "Cancel");

	return 1;
}

void changeline(struct change_line *cl)
{
int use, save;

	use = !!((changes & USE_OK) && !(changes & USE_BAD));
	save = !!(changes & SAVE_OK);

	(cl->next_value)();

	draw_change_gadget(cl);

#if USE_ACTIVE
	if (use != !!((changes & USE_OK) && !(changes & USE_BAD)) {
		if ((changes & USE_OK) && !(changes & USE_BAD)) {
			on_gadget(&use_button.gad);
			draw_button(&use_button);
		} else {
			off_gadget(&use_button.gad);
		}
	}
#endif

	if (save != !!(changes & SAVE_OK)) {
		if (changes & SAVE_OK) {
			on_gadget(&save_button.gad);
			draw_button(&save_button);
		} else {
			off_gadget(&save_button.gad);
		}
	}
}

int gdown(struct Gadget *g)
{
	return 0;
}

int gup(struct Gadget *g)
{
	switch (g->GadgetID) {

	case CHANGELINE_GADGET:
		changeline((struct change_line *)g->UserData);
		break;

	case CANCEL_GADGET:
		return 1;
		break;

#if USE_ACTIVE
	case USE_GADGET:
		if (warnings()) {
			update_stuff();
			return 1;
		}
		break;
#endif

	case SAVE_GADGET:
		if (warnings()) {
			if (save_stuff()) {
				update_stuff();
				return 1;
			}
		}
		break;
	}

	return 0;
}

void change_stuff()
{
struct IntuiMessage *msg;
int done = 0;

	while (!done) {

		WaitPort(w->UserPort);
		while (msg = (struct IntuiMessage *)GetMsg(w->UserPort)) {

			switch (msg->Class) {
			case GADGETDOWN:
				done = gdown((struct Gadget *)msg->IAddress);
				break;

			case GADGETUP:
				done = gup((struct Gadget *)msg->IAddress);
				break;
			}

			ReplyMsg(msg);
		}
	}
}

void main()
{
	if (!(JanusBase = (struct JanusBase *)OpenLibrary("janus.library", 0)))
		goto exit1;

	if (!set_bbtype())
		goto exit2;

	if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)))
		goto exit2;

	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0)))
		goto exit3;

	if (GetScreenData((APTR)&wbscr, sizeof(struct Screen), WBENCHSCREEN, NULL)) {
		wbscr_barheight = wbscr.BarHeight;
		wbscr_height = wbscr.Height;
		wbscr_width = wbscr.Width;
	} else {
		wbscr_barheight = 10;
		wbscr_height = 200;
		wbscr_width = 640;
	}

	read_stuff();

	init_gadgets();

	prefwindow.LeftEdge = (wbscr_width - prefwindow.Width) / 2;
	prefwindow.TopEdge = (wbscr_height - prefwindow.Height) / 2;

	if (w = (struct Window *)OpenWindow(&prefwindow)) {

		rp = w->RPort;

		link_gadgets();

		draw_screen();

		if (bbtype == A1060) {
			off_gadget(&janus_segment_cl.gad);
			off_gadget(&mda_emul_cl.gad);
			off_gadget(&cga_emul_cl.gad);
		}

#if USE_ACTIVE
		off_gadget(&use_button.gad);
#endif
		off_gadget(&save_button.gad);

		flipper_gads_onoff();

		change_stuff();

		CloseWindow(w);

	}

	CloseLibrary(IntuitionBase);

exit3:
	CloseLibrary(GfxBase);

exit2:
	CloseLibrary(JanusBase);

exit1:
	exit(0);
}
