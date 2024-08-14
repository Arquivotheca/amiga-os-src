/*
 *		layout.c
 *
 * ========================================================================= 
 * Layout.c - creates and manages gadgets for installer utility              
 * By Talin & Joe Pearce. (c) 1990 Sylvan Technical Arts                     
 * ========================================================================= 
 *	
 *	
 * Prototypes for functions defined in layout.c
 *
 *	extern unsigned char 		UndoBuf[512];
 *	extern struct TextZone 		textzones[4];
 *	extern struct TextZone *	tz_activate;
 *	extern WORD 				textzone_count;
 *	extern struct Gadget 		checkmarks[40];
 *	extern WORD 				checkmark_count;
 *	extern struct Slider 		sliders[2];
 *	extern WORD 				slider_count;
 *	extern struct Button 		buttons[21];
 *	extern WORD 				button_count;
 *	extern WORD 				prop_flags;
 *	extern UWORD * 				RtArrow;
 *	extern UWORD 				RtArrowData[7];
 *	extern UWORD * 				InsertMask;
 *	extern UWORD 				InsertMaskData[63];
 *	extern UWORD * 				InsertText;
 *	extern UWORD 				InsertTextData[63];
 *	extern struct ArrowShape 	UpArrowShape;
 *	extern struct ArrowShape 	DnArrowShape;
 *	extern struct ArrowShape * 	uparrow_image;
 *	extern struct ArrowShape * 	dnarrow_image;
 *	extern struct RadioShape 	top_shape;
 *	extern struct RadioShape 	bottom_shape;
 *	extern struct RadioShape 	dot_shape;
 *	extern struct RadioShape 	check_shape;
 *	extern struct RadioImage * 	on_image;
 *	extern struct RadioImage * 	off_image;
 *	extern struct RadioImage * 	check_image;
 *	extern struct Image 		radio_on;
 *	extern struct Image 		radio_off;
 *	extern struct Image 		check_on;
 *	extern struct Image 		check_off;
 *	extern USHORT * 			dots;
 *
 *	void 						make_images			(void);
 *	void 						DrawBevel			(struct RastPort * , WORD , WORD , WORD , WORD , WORD );
 *	void 						DrawBevelRect		(struct RastPort * , struct Rectangle * , WORD );
 *
 *	extern WORD 				place_top;
 *	extern WORD 				place_bottom;
 *
 *	void 						restart_layout		(void);
 *	void 						start_layout		(void);
 *	void 						center_layout		(WORD );
 *	void 						erase_page			(void);
 *	WORD 						TextPixels			(unsigned char * );
 *	void 						write_text			(struct RastPort * , struct Gadget * , unsigned char * );
 *	void 						new_border_button	(struct Button * , UWORD , unsigned char * , struct Gadget ** );
 *	void 						arrow_gadget		(struct Gadget ** , UWORD , WORD , WORD , WORD );
 *	void 						slider_gadget		(struct Gadget ** , UWORD , WORD , WORD , WORD , WORD );
 *	void 						list_gadget			(struct Gadget ** , UWORD );
 *	void 						layout_button		(struct Gadget ** , unsigned char * , UWORD , WORD , WORD );
 *	void 						layout_2_buttons	(struct Gadget ** , unsigned char * , UWORD , unsigned char * , UWORD , WORD );
 *	void 						layout_3_buttons	(struct Gadget ** , unsigned char * , UWORD , unsigned char * , UWORD , unsigned char * , UWORD , WORD );
 *	void 						layout_side_buttons	(struct Gadget ** , unsigned char * , UWORD , unsigned char * , UWORD , WORD );
 *
 *	extern struct StringExtend 	strext;
 *
 *	void 						finish_textzone		(struct TextZone * , unsigned char * , UWORD );
 *	void 						std_text_gad		(struct TextZone * , WORD );
 *	void 						layout_gauge		(struct Gadget ** , UWORD );
 *	void 						layout_strgad		(struct Gadget ** , unsigned char * , UWORD , WORD );
 *	void 						layout_intgad		(struct Gadget ** , long , UWORD , WORD );
 *	void 						init_wrap			(void);
 *	WORD 						find_wrap			(unsigned char ** , WORD );
 *	void						layout_text_length	(unsigned char * , WORD , WORD , WORD , ULONG );
 *	void 						layout_text			(unsigned char * , WORD , WORD , UWORD );
 *	void 						layout_wrap_text	(unsigned char * , WORD , WORD );
 *	void						build_checkmark		(struct Gadget * , UWORD );
 *	void 						build_radio			(struct Gadget * , UWORD );
 *  WORD                        layout_box_gads     (struct Gadget ** , struct Rectangle * , struct String ** , WORD , WORD , WORD );
 *	void 						ghost_gadget		(UWORD );
 *	void 						enable_gadget		(UWORD );
 *	void 						disable_radio		(struct Gadget * , WORD , WORD );
 *	void 						default_radio		(struct Gadget * , WORD , WORD );
 *	WORD 						final_radio			(struct Gadget * , WORD );
 *	void 						init_checkboxes		(struct Gadget * , LONG , WORD );
 *	LONG 						final_checkboxes	(struct Gadget * , WORD );
 *	void 						set_slider			(struct Gadget * , struct ListControl * );
 *	void 						hilite_file			(struct Gadget * , struct ListControl * );
 *	void 						show_files			(struct Gadget * , struct ListControl * );
 *	LONG 						which_file			(struct Gadget * , struct ListControl * , WORD );
 *	void 						set_integer_gadget	(struct Gadget * , LONG );
 *	void 						set_zone			(struct TextZone * , unsigned char * );
 *	void 						expand_zone			(struct TextZone * , BPTR );
 *	void 						init_text			(struct Gadget * , struct ListControl * );
 *
 *	extern UWORD * 				RtArrow;
 *	extern UWORD * 				InsertMask;
 *	extern UWORD * 				InsertText;
 *
 *	void 						show_text			(struct Gadget * , struct ListControl * , WORD );
 *
 *	extern UWORD * 				RtArrow;
 *	extern UWORD * 				InsertMask;
 *	extern UWORD * 				InsertText;
 *
 *	void 						erase_text			(struct Gadget * , struct ListControl * );
 *	LONG 						which_line			(struct Gadget * , struct ListControl * , WORD );
 *	void 						radio_space_check	(WORD );
 *	struct Gadget * 			find_key_gadget		(UWORD );
 *
 *	extern unsigned char * 		gir_prompt;
 *	extern unsigned char * 		gir_default;
 *	extern unsigned char * 		gir_help;
 *	extern unsigned char * 		gir_deftool;
 *
 *	unsigned char * 			make_gad_text		(struct GadgetDef * );
 *	LONG 						make_gadgets		(struct Gadget ** , struct GadgetDef * , struct InstallationRecord * , UWORD );
 *	void 						clip_off			(struct RastPort * );
 *	void 						clip_on				(struct RastPort * , WORD , WORD , WORD , WORD );
 *	
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 10/24/93:
 *		Improved layout routines for strings so that they cannot overwrite
 *		the window borders or write off the bottom or top of the window area.
 */



#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/sghooks.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>

#include "functions.h"

#include "xfunctions.h"
#include <string.h>

#include "installer.h"
#include "wild.h"
#include "window.h"

extern int tolower(int);


/* ======================== external references ============================ */

extern struct TextFont		*UFont;
extern struct Window		*window;
extern struct RastPort		*rp;
extern struct Gadget		*first_gadget;

extern struct IntuitionBase	*IntuitionBase;
extern struct Library		*LayersBase;
extern struct GfxBase		*GfxBase;

extern WORD					window_width,
							window_height,
							xsize;

extern WORD					darkest_color,
							lightest_color,
							text_color,
							highlight_color;
	
extern WORD					left_edge,
							right_edge,
							top_edge,
							bottom_edge;


/* ========================= Gadgets ===================================== */

char UndoBuf[MAX_TZ_CHARS];

struct TextZone textzones[MAX_TEXTZONES],
				*tz_activate;
WORD textzone_count;

struct Gadget checkmarks[MAX_CHECKMARKS];
WORD checkmark_count;

struct Slider sliders[MAX_SLIDERS];
WORD slider_count;

struct Button buttons[MAX_BUTTONS];
WORD button_count;

WORD prop_flags;


/* ========================= Images ====================================== */

UWORD *RtArrow;

UWORD RtArrowData[7] = 
{
   0xC000,
   0xF000,
   0xFC00,
   0xFE00,
   0xFC00,
   0xF000,
   0xC000
};

UWORD *InsertMask;

UWORD InsertMaskData[63] = 
{
   0x00FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x03FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x0FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x3FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x3FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x0FFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x03FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x00FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000
};

UWORD *InsertText;

UWORD InsertTextData[63] = 
{
   0x00FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000,
   0x0300,0x0000,0x0000,0x0000,0x0000,0x0000,0x6000,
   0x0C06,0xE67C,0xFEFC,0xFF00,0xC6FE,0xFCFE,0x6000,
   0x3006,0xF6C0,0xC0C6,0x1800,0xC6C0,0xC6C0,0x6000,
   0xC006,0xDE7C,0xF8FC,0x1800,0xFEF8,0xFCF8,0x6000,
   0x3006,0xCE06,0xC0D8,0x1800,0xC6C0,0xD8C0,0x6000,
   0x0C06,0xC67C,0xFEC6,0x1800,0xC6FE,0xC6FE,0x6000,
   0x0300,0x0000,0x0000,0x0000,0x0000,0x0000,0x6000,
   0x00FF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xE000
};

struct ArrowShape 
{
	UWORD	data[5];
};

struct ArrowShape UpArrowShape = 
{
   0x0C00, 0x1E00, 0x3F00, 0x7F80, 0xFFC0
};

struct ArrowShape DnArrowShape = 
{
   0xFFC0, 0x7F80, 0x3F00, 0x1E00, 0x0C00
};

#if 0
struct ArrowImage
{	struct ArrowShape	plane1,
						plane2;
};

struct ArrowImage *uparrow_image,*dnarrow_image;
#else
struct ArrowShape *uparrow_image,*dnarrow_image;
#endif

struct RadioShape 
{
	UWORD	data[9];
};

struct RadioShape top_shape = 
{
	0x3FFC,0x6000,0xC000,0xC000,0xC000,0xC000,0xC000,0x6000,0x0000
};

struct RadioShape bottom_shape = 
{
	0x0000,0x0006,0x0003,0x0003,0x0003,0x0003,0x0003,0x0006,0x3FFC
};

struct RadioShape dot_shape = 
{
	0x0000,0x0000,0x07E0,0x0FF0,0x0FF0,0x0FF0,0x07E0,0x0000,0x0000
};

struct RadioShape check_shape = 
{
	0x0000,0x0038,0x0060,0x00C0,0xE180,0x7300,0x3E00,0x1C00,0x0000
};

struct RadioImage
{	struct RadioShape	plane1,
						plane2;
};

struct RadioImage		*on_image,
						*off_image,
						*check_image;

struct Image			radio_on = { 0,0, 16,9,2, NULL, 3,0, NULL },
						radio_off = { 0,0, 16,9,2, NULL, 3,0, NULL },
#if 0
						arrow_up = { 7,3, 10,5,2, NULL, 3,0, NULL },
						arrow_dn = { 7,3, 10,5,2, NULL, 3,0, NULL },
#endif
						check_on = { 4,1, 13,9,2, NULL, 3,0, NULL },
						check_off = { 4,1, 13,9,0, NULL, 0,0, NULL };

USHORT					*dots;

	/* the routine remaps images so they look right given workbench colors */

void make_images(void)
{
	WORD	line_height = clamp(11,UFont->tf_YSize,100);

	on_image 	= MemAlloc(sizeof *on_image,MEMF_CHIP|MEMF_CLEAR);
	off_image 	= MemAlloc(sizeof *off_image,MEMF_CHIP|MEMF_CLEAR);
	check_image = MemAlloc(sizeof *off_image,MEMF_CHIP|MEMF_CLEAR);
	dots 		= MemAlloc(4,MEMF_CHIP);
	dots[0] 	= 0xcccc;
	dots[1] 	= 0x3333;
	uparrow_image = MemAlloc(sizeof *uparrow_image,MEMF_CHIP|MEMF_CLEAR);
	dnarrow_image = MemAlloc(sizeof *dnarrow_image,MEMF_CHIP|MEMF_CLEAR);

	RtArrow = MemAlloc(sizeof RtArrowData,MEMF_CHIP);
	CopyMem((char *)RtArrowData,(char *)RtArrow,sizeof RtArrowData);
	InsertMask = MemAlloc(sizeof InsertMaskData,MEMF_CHIP);
	CopyMem((char *)InsertMaskData,(char *)InsertMask,sizeof InsertMaskData);
	InsertText = MemAlloc(sizeof InsertTextData,MEMF_CHIP);
	CopyMem((char *)InsertTextData,(char *)InsertText,sizeof InsertTextData);

	radio_on.ImageData = (USHORT *)on_image;
	radio_off.ImageData = (USHORT *)off_image;
	check_on.ImageData = (USHORT *)check_image;
#if 0
	arrow_up.ImageData = (USHORT *)uparrow_image;
	arrow_dn.ImageData = (USHORT *)dnarrow_image;
#endif

	if (lightest_color & 1)
	{	
		or_mem(&top_shape,&off_image->plane1,sizeof (struct RadioShape));
		or_mem(&bottom_shape,&on_image->plane1,sizeof (struct RadioShape));
	}
	if (lightest_color & 2)
	{	
		or_mem(&top_shape,&off_image->plane2,sizeof (struct RadioShape));
		or_mem(&bottom_shape,&on_image->plane2,sizeof (struct RadioShape));
	}
	if (darkest_color & 1)
	{	
		or_mem(&bottom_shape,&off_image->plane1,sizeof (struct RadioShape));
		or_mem(&top_shape,&on_image->plane1,sizeof (struct RadioShape));
		or_mem(&check_shape,&check_image->plane1,sizeof (struct RadioShape));
#if 0
		or_mem(&UpArrowShape,&uparrow_image->plane1,sizeof (struct ArrowShape));
		or_mem(&DnArrowShape,&dnarrow_image->plane1,sizeof (struct ArrowShape));
#endif
	}
	if (darkest_color & 2)
	{	
		or_mem(&bottom_shape,&off_image->plane2,sizeof (struct RadioShape));
		or_mem(&top_shape,&on_image->plane2,sizeof (struct RadioShape));
		or_mem(&check_shape,&check_image->plane2,sizeof (struct RadioShape));
#if 0
		or_mem(&UpArrowShape,&uparrow_image->plane2,sizeof (struct ArrowShape));
		or_mem(&DnArrowShape,&dnarrow_image->plane2,sizeof (struct ArrowShape));
#endif
	}
	if (highlight_color & 1)
	{	
		or_mem(&dot_shape,&on_image->plane1,sizeof (struct RadioShape));
	}
	if (highlight_color & 2)
	{	
		or_mem(&dot_shape,&on_image->plane2,sizeof (struct RadioShape));
	}

#if 1
	CopyMem(&UpArrowShape,uparrow_image,sizeof (struct ArrowShape));
	CopyMem(&DnArrowShape,dnarrow_image,sizeof (struct ArrowShape));
#endif

	check_on.TopEdge = check_off.TopEdge = (line_height - 9) / 2;
}



/* ========================= drawing functions =========================== */

void DrawBevel(struct RastPort *rp,WORD x,WORD y,WORD w,WORD h,WORD in)
{
	w--; h--;
	if (w > 0 && h > 0)
	{	
		SetAPen(rp,in ? darkest_color : lightest_color);
		RectFill(rp,x,y,x+1,y+h);

		SetAPen(rp,in ? lightest_color : darkest_color);
		RectFill(rp,x+w-1,y,x+w,y+h);
		Move(rp,x+1,y+h);
		Draw(rp,x+w,y+h);

		SetAPen(rp,in ? darkest_color : lightest_color);
		Move(rp,x,y);
		Draw(rp,x+w-1,y);
	}
}



void DrawBevelRect(struct RastPort *rp,struct Rectangle *box,WORD in)
{
	DrawBevel(rp,box->MinX,box->MinY,box->MaxX-box->MinX,box->MaxY-box->MinY,0);
}



/* ========================= Screen Layout functions ===================== */

WORD					place_top,				/* where to place next upper control */
						place_bottom;			/* where to place next lowe control */

void restart_layout(void)
{
	place_top = top_edge;
	place_bottom = window_height - bottom_edge + 1;
	tz_activate = NULL;
}


void start_layout(void)
{
	restart_layout();
	button_count = textzone_count = checkmark_count = slider_count = 0;
	first_gadget = NULL;
}


void center_layout(WORD a)
{	
	WORD t = a * (rp->TxHeight + 5);

	place_top =	(place_top + place_bottom - t)/2;
	place_bottom = place_top + t/2;
}


void erase_page(void)
{
	SetAPen(rp,0L);
	RectFill(rp,left_edge,top_edge,window_width-right_edge-1,
			 window_height-bottom_edge-1);
}



/* ======================== gadget creation functions ===================== */

WORD TextPixels(char *text)
{
	return TextLength(rp,text,strlen(text));
}

#ifndef ONLY2_0

void write_text (struct RastPort *rp, struct Gadget *gad, char *text)
{
	WORD		text_width,
				text_offset,
				text_length,
				i, x, y;
	char		*underscore;

	text_length = strlen(text);
	underscore = strchr(text,'_');

	text_width = TextLength(rp,text,text_length);
	if (underscore != NULL) 
		text_width -= TextLength(rp,"_",1);
	text_offset = (gad->Width - text_width) / 2;

	y = gad->TopEdge + (gad->Height - rp->TxHeight)/2 + rp->TxBaseline + 1;
	SetAPen(rp,text_color);
	Move(rp, gad->LeftEdge + text_offset, y);
	if (underscore != NULL)
	{
		if (underscore > text) 
			Text(rp,text,underscore - text);
		x = rp->cp_x;
		i = text_length - (++underscore - text);
		Text(rp,underscore,i);
		text_width = TextLength(rp,underscore,1);

		Move(rp,x-1,y+2);
		Draw(rp,x+text_width-1,y+2);
	}
	else 
	Text(rp,text,text_length);
}

#else

extern struct Hook ButtonHook;
extern struct Hook ArrowHook;

#endif

void new_border_button (
	struct Button  *gad,
	UWORD 			id,
	char		   *text,
	struct Gadget **list)
{
	struct Border	*b;
	WORD			*v;

	gad->Gadget.Flags = GADGHIMAGE;
	gad->Gadget.Activation = RELVERIFY;
#ifdef ONLY2_0
	gad->Gadget.GadgetType = CUSTOMGADGET;
	gad->Gadget.SpecialInfo = (APTR)text;
	gad->Gadget.MutualExclude = (LONG)&ButtonHook;
#else
	gad->Gadget.GadgetType = BOOLGADGET;
#endif
	gad->Text = text;
	gad->Gadget.GadgetID = id;
	b = gad->Border;
	gad->Gadget.GadgetRender = (APTR)b;
	gad->Gadget.SelectRender = (APTR)(b + 2);
	v = gad->Vector;

	b->NextBorder = b + 1;
	b->FrontPen = lightest_color;
	b->DrawMode = JAM1;
	b->Count = 5;
	b->XY = v;
	v[0] = gad->Gadget.Width - 2;
	v[5] = gad->Gadget.Height - 1;
	v[6] = v[8] = v[9] = 1;
	v[7] = v[5] - 1;
	v += 10;
	b[2] = *b;
	b++;
	b->FrontPen = darkest_color;
	b->DrawMode = JAM1;
	b->Count = 5;
	b->XY = v;
	v[0] = v[7] = 1;
	v[1] = v[3] = gad->Gadget.Height - 1;
	v[2] = v[4] = gad->Gadget.Width - 1;
	v[6] = v[8] = v[4] - 1;
	v[9] = v[1] - 1;
	b[2] = *b;
	b++;
	b->NextBorder = b + 1;
	b->FrontPen = darkest_color;
	b++;
	b->FrontPen = lightest_color;

#ifndef ONLY2_0
	if (text) 
		write_text(rp,(struct Gadget *)gad,text);
#endif

	gad->Gadget.NextGadget = *list;
	*list = (struct Gadget *)gad;
}

void arrow_gadget(struct Gadget **list, UWORD id, WORD x, WORD y, WORD type)
{	
	struct Button		*gad;

	gad = &buttons[button_count++];
	zero(gad,sizeof *gad);

	gad->Gadget.LeftEdge 	= x;
	gad->Gadget.TopEdge 	= y;
	gad->Gadget.Width 		= 24;
	gad->Gadget.Height 		= 11;

	new_border_button(gad,id,NULL,list);	/* fills in rest of structure */
	gad->Gadget.Activation |= GADGIMMEDIATE;
	
#ifdef ONLY2_0
	gad->Gadget.MutualExclude = (LONG)&ArrowHook;
	gad->Gadget.SpecialInfo =
		(type == UP_ARROW ? (APTR)uparrow_image : (APTR)dnarrow_image);
#else
#if 1
	SetAPen(rp,darkest_color);
	SetDrMd(rp,JAM1);
	BltTemplate(type == UP_ARROW ? (void *)uparrow_image : (void *)dnarrow_image,
		0,2,rp,x+7,y+3,10,5);
#else
	DrawImage(rp,type == UP_ARROW ? &arrow_up : &arrow_dn,x,y);
#endif
#endif

}

void slider_gadget (
	struct Gadget **list, 
	UWORD 			id, 
	WORD 			x, 
	WORD 			y,
	WORD 			width, 
	WORD 			height)
{	
	struct Slider		*gad;

	gad = &sliders[slider_count++];
	zero(gad,sizeof *gad);

	gad->Gadget.LeftEdge 	 = x + 4;
	gad->Gadget.TopEdge 	 = y + 2;
	gad->Gadget.Width 		 = width - 8;
	gad->Gadget.Height 		 = height - 4;

	gad->Gadget.Flags 		 = GADGHCOMP|GADGIMAGE;
	gad->Gadget.Activation 	 = RELVERIFY|FOLLOWMOUSE;
	gad->Gadget.GadgetType 	 = PROPGADGET;
	gad->Gadget.GadgetID 	 = id;
	gad->Gadget.GadgetRender = (APTR)&gad->Image;
	gad->Gadget.SpecialInfo  = (APTR)&gad->Info;

#ifndef ONLY2_0
	if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
	{
		prop_flags = PROPFLAGS | AUTOKNOB;
	}
#ifndef ONLY2_0
	else
	{
		prop_flags = PROPFLAGS;
		gad->Image.Width = gad->Gadget.Width;
		gad->Image.Height = gad->Gadget.Height;
		gad->Image.PlaneOnOff = darkest_color;
	}
#endif

	gad->Info.Flags = prop_flags;
	gad->Info.HorizBody = gad->Info.VertBody = MAXBODY;

	DrawBevel(rp,x,y,width,height,0);

	gad->Gadget.NextGadget = *list;
	*list = (struct Gadget *)gad;
}

void list_gadget (struct Gadget **list, UWORD id)
{
	struct Button		*gad;
	WORD				r = right_edge + 28;

	gad = &buttons[button_count++];
	zero(gad,sizeof *gad);

	gad->Gadget.LeftEdge 	= left_edge + 2;
	gad->Gadget.TopEdge 	= place_top + 1;
	gad->Gadget.Width 		= window_width - r - left_edge - 4;
	gad->Gadget.Height 		= place_bottom - place_top - 4;
	gad->Gadget.Flags 		= GADGHNONE;
	gad->Gadget.Activation 	= GADGIMMEDIATE|RELVERIFY;
	gad->Gadget.GadgetType 	= BOOLGADGET;
	gad->Gadget.GadgetID 	= id;

	DrawBevel  (rp,
				left_edge,
				place_top,
				window_width-r-left_edge,
				place_bottom-place_top-2,
				0);

	gad->Gadget.NextGadget = *list;
	*list = (struct Gadget *)gad;
}

void layout_button (
	struct Gadget **list, 
   	char 		   *text, 
   	UWORD 			id,
	WORD 			lower_flag, 
	WORD 			width_type)
{
	struct Button		*gad;
	WORD				temp;

	gad = &buttons[button_count++];
	zero(gad,sizeof *gad);

	/* first, calc height of gadget... */

	gad->Gadget.Width =
		(window_width - left_edge - right_edge) / width_type - 2;

	if (lower_flag & PUT_LEFT) 
		gad->Gadget.LeftEdge = left_edge;
	else 
	if (lower_flag & PUT_RIGHT)
		gad->Gadget.LeftEdge = window_width - right_edge - gad->Gadget.Width;
	else 
		gad->Gadget.LeftEdge = (window_width - gad->Gadget.Width) / 2;

	gad->Gadget.Height = rp->TxHeight + 5;	/* was 3 */

	if (lower_flag & PUT_LOWER)
	{
		temp = place_bottom - gad->Gadget.Height - 1;
		gad->Gadget.TopEdge = temp;
		if ( !(lower_flag & PUT_STAY) ) 
			place_bottom = temp;
	}
	else
	{
		gad->Gadget.TopEdge = place_top;
		if ( !(lower_flag & PUT_STAY) ) 
			place_top += gad->Gadget.Height + 1;
	}

	new_border_button(gad,id,text,list);
}

void layout_2_buttons (
	struct Gadget **list, 
   	char		   *text1, 
   	UWORD 			id1,
	char		   *text2, 
	UWORD 			id2, 
	WORD 			lower_flag)
{
	layout_button(list,text1,id1,lower_flag|PUT_LEFT|PUT_STAY,HALF_WIDTH);
	layout_button(list,text2,id2,lower_flag|PUT_RIGHT,HALF_WIDTH);
}

void layout_3_buttons (
	struct Gadget **list, 
	char		   *text1, 
	UWORD 			id1,
	char		   *text2, 
	UWORD 			id2, 
	char		   *text3, 
	UWORD 			id3, 
	WORD 			lower_flag)
{
	layout_button(list,text1,id1,lower_flag|PUT_LEFT|PUT_STAY,THIRD_WIDTH);
	layout_button(list,text2,id2,lower_flag|PUT_STAY,THIRD_WIDTH);
	layout_button(list,text3,id3,lower_flag|PUT_RIGHT,THIRD_WIDTH);
}

void layout_side_buttons (
	struct Gadget 	  **list, 
	char 		       *text1, 
	UWORD 			    id1,
	char 		       *text2, 
	UWORD 			    id2, 
	WORD 			    lower_flag)
{
	struct Button		*gad1,*gad2;
	WORD				temp;

	gad1 = &buttons[button_count];
	button_count += 2;
	gad2 = gad1 + 1;
	zero(gad1,2 * sizeof *gad1);

	/* first, calc height of gadget... */

	gad1->Gadget.Width = (window_width - left_edge - right_edge) / 6 - 2;

	gad2->Gadget.Width = gad1->Gadget.Width;
	gad1->Gadget.LeftEdge = left_edge;
	gad2->Gadget.LeftEdge = window_width - right_edge - gad2->Gadget.Width;

	gad1->Gadget.Height = gad2->Gadget.Height = rp->TxHeight + 5; /* was 3 */

	if (lower_flag & PUT_LOWER)
	{	
		temp = place_bottom - gad1->Gadget.Height - 1;
		gad1->Gadget.TopEdge = gad2->Gadget.TopEdge = temp;
		if ( !(lower_flag & PUT_STAY) ) 
			place_bottom = temp;
	}
	else
	{	
		gad1->Gadget.TopEdge = gad2->Gadget.TopEdge = place_top;
		if ( !(lower_flag & PUT_STAY) ) 
			place_top += gad1->Gadget.Height + 1;
	}

	/* center the IntuiText... */
	/* Adjust the border.. */	

	new_border_button(gad1,id1,text1,list);		/* fills in rest of structure */
	new_border_button(gad2,id2,text2,list);		/* fills in rest of structure */
}

struct StringExtend strext;

void finish_textzone (struct TextZone *gad, char *text, UWORD id)
{
	gad->Gadget.Flags 		= GADGHCOMP;
	gad->Gadget.Activation 	= RELVERIFY|STRINGCENTER;
	gad->Gadget.GadgetType 	= STRGADGET;
	gad->Gadget.GadgetID 	= id;
	gad->Gadget.SpecialInfo = (APTR)&gad->Info;
	gad->Info.Buffer 		= (UBYTE *)gad->Buffer;
	gad->Info.UndoBuffer 	= (UBYTE *)UndoBuf;
	gad->Info.MaxChars 		= MAX_TZ_CHARS;

#ifndef ONLY2_0
	if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
	{
		strext.Font = UFont;
		strext.Pens[0] = strext.ActivePens[0] = text_color;
		if (IntuitionBase->LibNode.lib_Version == 36)
			gad->Gadget.Activation |= GACT_STRINGEXTEND;	/* want to work on v36 */
		else 
			gad->Gadget.Flags |= GFLG_STRINGEXTEND;
		gad->Info.Extension = &strext;
	}

	strcpy(gad->Buffer,text);
}


void std_text_gad (struct TextZone *gad, WORD lower_flag)
{
	zero(gad,sizeof *gad);

	/* first, calc height of gadget... */

	gad->Gadget.Width = window_width - left_edge - right_edge - 8;
	gad->Gadget.LeftEdge = left_edge + 4;

#ifndef ONLY2_0
	if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
	{
		gad->Gadget.Height = rp->TxHeight + 1;
	}
#ifndef ONLY2_0
	else
	{
		gad->Gadget.Height = window->WScreen->RastPort.TxHeight + 1;
	}
#endif

	if (lower_flag & PUT_LOWER)
	{	
		place_bottom -= gad->Gadget.Height + 5;
		gad->Gadget.TopEdge = place_bottom + 2;
	}
	else
	{	
		gad->Gadget.TopEdge = place_top + 2;
		place_top += gad->Gadget.Height + 5;
	}

	DrawBevel  (rp,
				gad->Gadget.LeftEdge-4,
				gad->Gadget.TopEdge-2,
				gad->Gadget.Width+8,
				gad->Gadget.Height+4,
				0);
				
	DrawBevel  (rp,
				gad->Gadget.LeftEdge-2,
				gad->Gadget.TopEdge-1,
				gad->Gadget.Width+4,
				gad->Gadget.Height+2,
				1);
}

void layout_gauge (struct Gadget **list, UWORD id)
{
	struct TextZone		*gad = &textzones[textzone_count++];

	zero(gad,sizeof *gad);

	gad->Gadget.Width = window_width - 3 * left_edge - 3 * right_edge - 8;
	gad->Gadget.LeftEdge = 3 * left_edge + 4;
	gad->Gadget.Height = rp->TxHeight + 1;

	place_bottom -= gad->Gadget.Height + 5;
	gad->Gadget.TopEdge = place_bottom + 2;

	DrawBevel  (rp,
				gad->Gadget.LeftEdge-2,
				gad->Gadget.TopEdge-1,
				gad->Gadget.Width+4,
				gad->Gadget.Height+2,
				1);

	gad->Gadget.Flags 		= GADGHNONE;
	gad->Gadget.Activation 	= 0;
	gad->Gadget.GadgetType 	= BOOLGADGET;
	gad->Gadget.GadgetID 	= id;
	gad->Gadget.NextGadget 	= *list;

	*list = (struct Gadget *)gad;
}

void layout_strgad (struct Gadget **list, char *text, UWORD id, WORD lower_flag)
{
	struct TextZone		*gad;

	gad = &textzones[textzone_count++];
	std_text_gad(gad,lower_flag);
	finish_textzone(gad,text,id);		/* fills in rest of structure */

	gad->Gadget.NextGadget = *list;
	*list = (struct Gadget *)gad;
}

void layout_intgad (struct Gadget **list, long val, UWORD id, WORD lower_flag)
{
	struct TextZone		*gad;
	char				 str[12];

	gad = &textzones[textzone_count++];
	std_text_gad(gad,lower_flag);
	Sprintf(str,"%ld",val);
	finish_textzone(gad,str,id);		/* fills in rest of structure */

	gad->Gadget.NextGadget = *list;
	gad->Gadget.Activation |= LONGINT;
	gad->Info.LongInt = val;
	*list = (struct Gadget *)gad;
}

	/* this code handles word wrapping  */

static UBYTE	wrap_flags,
				last_quote;

#define WRAP_INQUOTE	(1<<0)
#define WRAP_NOQUOTE	(1<<1)
#define WRAP_HYPHENATE	(1<<2)

void init_wrap(void)
{
	wrap_flags = 0;
}

WORD find_wrap (char **textptr, WORD pixels)
{
	/*	This procedure proports to find wrap points in the text so that
	 *	it will all fit within the borders of our window.  The major
	 *	obvious problem is that we don't check the HEIGHT of the window,
	 *	just the width, so we will happily write off the bottom of the
	 *	window!  
	 */
	 	
	UBYTE	*text  = (UBYTE *)*textptr;
	WORD	*space = (WORD *)(UFont->tf_CharSpace);		/* space table	*/
	WORD	*kern  = (WORD *)(UFont->tf_CharKern);		/* kern table	*/

	WORD	pixel_length = 0,
			last_size 	 = 0, 
			current_size = 0;

	UBYTE	*last_text   = text;
	WORD	c, i;
	UBYTE	squote, 
			quote;


	/* reset WRAP_HYPHENATE and WRAP_NOQUOTE to start. */

	wrap_flags &= ~(WRAP_HYPHENATE | WRAP_NOQUOTE);

	/* setup internal quote state. 
	 *
	 *	For the uninitiated, this boils down to the following:
	 *
	 *	If we were called before, and we ended up having to break the
	 *	line in the middle of a quoted string, then last_quote holds
	 *	the value of that quote character (" or ') and WRAP_INQUOTE
	 *	will be true.  In this case, we init the value of quote to the
	 *	saved value (thus indicating that we are inside a quoted string).
	 *	We also init squote, but I don't know what that means yet.
	 */

	quote = squote = (wrap_flags & WRAP_INQUOTE ? last_quote : 0);

	last_quote = 0;									/* no dangling quotes yet */

	for (;;)										/* loop thru characters	*/
	{
		if (pixel_length > pixels)					/* too long already?	*/
		{
			if (last_size == 0)						/* seen a space yet?	*/
			{
				/* no break... were we left inside a quoted string? */

				if (quote != 0 						/* we saw a left quote	*/
				&& !(wrap_flags & WRAP_NOQUOTE))	/* not breaking in strings */
				{
					/* redo whole thing ignoring quoted strings */

					text 		 = last_text = *textptr;
					current_size = pixel_length = 0;
					wrap_flags 	|= WRAP_NOQUOTE;	/* can break in strings	*/
					
					/*	Set quote for start of input string.  */
					
					quote = wrap_flags & WRAP_INQUOTE ? last_quote : 0;
					
					continue;						/* restart the loop		*/
				}

				/* what a big word (or pathname) you have there... */
				/* just break it up in mid word */

				wrap_flags |= WRAP_HYPHENATE;

				/* go back two characters */

				for (i=0;i<2;i++)
				{
					--text;				
					c = *text;

					/* get quoting right */

					if (quote == 0 && (c == '"' /* || c == '\'' */)) 
						quote = c;					/* back inside a string	*/
					else 
					if (quote == c) 				/* inside a string now?	*/
						quote = 0;					/* not any more!		*/
				}

				/* return this state */

				*textptr = text;					/* new text starts here	*/
				
				/*	Shouldn't this be -2?  */
				
				--current_size;						/* slightly smaller		*/
				break;								/* exit the loop		*/
			}

			/* we're done... */

			*textptr = (char *)last_text;			/* just after last space */
			current_size = last_size;				/* size after last space */
			quote = squote;							/* saved quote state	 */
			break;									/* exit the loop		 */
		}

		c = *text++;

		if (c == '\n')
		{
			/*	If the user wants this to be the end of the line we really
			 *	should stop right here!
			 */
			 
			*textptr = (char *)text;
			break;
		}
		if (c == ' ' && (quote == 0 || (wrap_flags & WRAP_NOQUOTE) ))
		{
			/*	If this is a space and we haven't seen a quote, or we
			 *	aren't breaking in quoted strings, then this is a reasonably
			 *	good place to break the string if we need to.  Remember it.
			 */
			 
			last_size = current_size;
			last_text = text;
			squote = quote;
		}
		if (c == '\0')
		{
			/*	We have reached the end of the world.  
			 *	Jump before we fall off.
			 */
			
			*textptr = NULL;
			break;
		}
		if (quote == 0 && (c == '"' /* || c == '\'' */)) 
			quote = c;							/* flag starting quote seen	*/
		else 
		if (quote == c) 						/* matching ending quote?	*/
			quote = 0;							/* yep, not in string now	*/

		/* keep this code, even though Proportional fonts not used now... */

		if ((UFont->tf_Flags & FPF_PROPORTIONAL) 
		&&  (c <= UFont->tf_HiChar 
		&&   c >= UFont->tf_LoChar) )
		{	
			c -= UFont->tf_LoChar;
			pixel_length += (space[c] + kern[c]);	/* add up text size		*/
		}
		else 
			pixel_length += xsize;					/* add fixed text size	*/

		current_size++;								/* count another char	*/
	}												/* end of while() loop	*/

	if (last_quote = quote) 						/* remember if in string */
		wrap_flags |= WRAP_INQUOTE;					/* yep, in a string.	*/
	else 
		wrap_flags = 0;								/* nope, no string		*/
	return current_size;							/* number of chars		*/
}

	/* this define is here so we could one day add bold/italic formatting */

#define	render_text(a,b,c)	Text(a,b,c)


#ifdef ONLY2_0

#define mySetSoftStyle	SetSoftStyle

#endif



void layout_text_length (
	char *text, 
	WORD  length, 
	WORD  lower_flag,
	WORD  style_type, 
	ULONG special)
{
	/*	This procedure attempts to determine the length of the text to 
	 *	be written into our window, and also ATTEMPTS to make the text
	 *	fit into the window if it doesn't.  However, it has at least two
	 *	major failiings in this last regard:
	 *
	 *	1) 	It does not take into account text widening that can happen
	 *	   	from bold text.
	 *	2) 	It cuts down text as small as it can conveniently get it, but 
	 *		if that is wider than the window, that is just too darn bad.
	 *
	 *	ER, UM, dept:
	 *		This procedure ALSO actually lays out the text, DESPITE the
	 *		name suggesting it merely calculates the size of the text!
	 */
	 
	WORD				text_width,
						text_offset,
						text_top,
						text_space = window_width - left_edge - right_edge,
						
						elipsln = TextLength (rp, "...", 3),
						dashln	= TextLength (rp, "-",   1);
						
	char			   *rtext	 = text;
	
	BOOL				ellipsis = FALSE,
						hyphen   = FALSE;


	if (lower_flag)									/* stick at window bottom */
	{
		if (place_bottom <= place_top)				/* off the window top?	  */
			return;									/* yes, forget this line! */
		place_bottom -= rp->TxHeight + 1;
		text_top = place_bottom;
	}
	else											/* stick at window top	  */
	{
		text_top = place_top;
		if (place_top >= place_bottom)				/* off the window bottom? */
			return;									/* yes, forget this line! */
		place_top += rp->TxHeight + 1;
	}

	text_width = TextLength (rp, rtext, length);

	if (special == GTEXT_PATH)
	{
		/* strip off path components until it fits after adding "..." */

		while ( text_width > text_space 
		&&		(strchr(rtext,':') || strchr(rtext,'/')) )
		{
			while (*rtext != ':' && *rtext != '/')
			{
				++rtext;
				--length;
			}

			text_width = TextLength(rp,++rtext,--length) + elipsln;
			ellipsis = TRUE;
		}

		while (text_width > text_space)
		{
			text_width = TextLength(rp,++rtext,--length);
			ellipsis = FALSE;
		}
	}

	if (special & (WRAP_HYPHENATE << 16))
	{
		if (text_width + dashln <= text_space)
		{
			text_width += dashln;
			hyphen = TRUE;
		}
	}
	
	/*	If the text is still too wide for the window, lop off the trailing
	 *	characters until it DOES fit.  Bad grammar is better than overwriting
	 *	the window borders and crashing the system!
	 */
	
	while (text_width > text_space)
		text_width = TextLength (rp, rtext, --length);

	if (style_type & TEXT_CENTER) 
		text_offset = (window_width - text_width) / 2;
	else 
		text_offset = left_edge + 3;

	if (special != GTEXT_SKIP)						/* SKIP only spaces vert */
	{
		if (special == GTEXT_PREWIPE || special == GTEXT_PATH)
		{
			SetAPen	   (rp, 0);						/* clear the line		*/
			RectFill   (rp,
						left_edge,
						text_top,
						window_width - right_edge,
						text_top + rp->TxHeight);
		}

		/* Set Style based on flags... */
		
		if (style_type & TEXT_BOLD) 
			mySetSoftStyle (rp, FSF_BOLD, FSF_BOLD);

		SetAPen (rp, text_color);
		
		Move (rp, text_offset, text_top + rp->TxBaseline);
		
		if (ellipsis) 
			render_text(rp,"...",3);
			
		render_text(rp,rtext,length);
		
		if (hyphen) 
			render_text(rp,"-",1);

		if (style_type & TEXT_BOLD) 
			mySetSoftStyle(rp,0,FSF_BOLD);
	}
}

void layout_text (char *text, WORD lower_flag, WORD style_type, UWORD special)
{
	layout_text_length (text,
						strlen(text),
						lower_flag,
						style_type,
						special);
}

void layout_wrap_text (char *text, WORD lower_flag, WORD style_type)
{
	char 				*textptr;
	WORD				length;

	init_wrap();
	while (text)
	{
		textptr = text;
		length = find_wrap(&textptr, window_width - right_edge - left_edge - 8);
		layout_text_length (text,
							length,
							lower_flag,
							style_type,
							(wrap_flags & WRAP_HYPHENATE) << 16);
		text = textptr;
	}
}

#define CHECK_SIZE			24
#define MIN_SPACING			8

/* note that this has to be able to lay out both Checkmarks and Radio Buttons. */
/* Also note that there is a minimum size for checkmarks (can be bigger) and an
	absolute size for radio buttons.
*/

void build_checkmark (struct Gadget *gad, UWORD id)
{
	gad->Flags 			= GADGHIMAGE|GADGIMAGE|SELECTED;
	gad->Activation 	= GADGIMMEDIATE|TOGGLESELECT;
	gad->GadgetType 	= BOOLGADGET;
	gad->GadgetID 		= id;
	gad->GadgetRender 	= (APTR)&check_off;
	gad->SelectRender 	= (APTR)&check_on;
}

void build_radio(struct Gadget *gad,UWORD id)
{
	gad->Flags 			= GADGHIMAGE|GADGIMAGE;
	gad->Activation 	= GADGIMMEDIATE;
	gad->GadgetType 	= BOOLGADGET;
	gad->GadgetID 		= id;
	gad->GadgetRender 	= (APTR)&radio_off;
	gad->SelectRender 	= (APTR)&radio_on;
}

WORD layout_box_gads (
	struct Gadget 	  **list, 
	struct Rectangle   *box,
	struct String  	  **text, 
	WORD 				count, 
	WORD 				id, 
	WORD 				type)
{
	/*	This procedure is used to do the layout pages for radio buttons
	 *	and checkmark gadgets.  These are mutual exclusion and multiselect
	 *	gadgets that differ in imagry of the box/button, but are otherwise
	 *	similar.
	 */
	 
	WORD				gap = 1;
	WORD				line_height = clamp((type & 0x7f) != IS_CHECKMARK 
								? 9 : 11,
								rp->TxHeight,
								100);
	WORD				max_pixels = box->MaxX - box->MinX - 8;
	WORD				max_lines =
							(box->MaxY - box->MinY - 4) / (line_height + 1);
	WORD				lines, columns;
	WORD				line, col;
	WORD				wraplines = 0;
	WORD				total_width;
	WORD				last_good_lines, last_good_cols, last_good_width;
	WORD				gadgets_displayed;
	WORD				spacing;
	WORD				top_position;
	WORD				special = type & 0x0080;
	WORD				length;
	char				*str, 
						*temp,
						c;
	struct Gadget		*start = NULL;
	struct TextFont		*ofont,
						*efont = NULL;

	type &= 0x7f;

again:					/* in case a second (or third) run though is needed */
	last_good_lines = last_good_cols = 0;

	/*	"count" is the number of gadgets we have to render.  Knowing how many
	 *	vertical lines we have available in this window, determine how many
	 *	columns we will need to render all of the gadgets.  Note that we may
	 *	not be able to use anywhere near this many columns, since the labels
	 *	on the gadgets will likely reduce the number of available columns
	 *	by a good bit.
	 */
	 
	columns = (count + max_lines - 1) / max_lines;
	lines = max_lines;

	for (;;)
	{
		total_width = 0;

		/* calculate the total width of all columns */

		for (col = 0; col < columns; col++)		/* for each column			*/
		{
			WORD	col_width = 0;

			for (line = 0; line < lines; line++) /* for each line in col	*/
			{
				WORD index = line + col*lines;
				WORD pixels;

				if (index < count)
				{
					pixels = TextPixels((char *)(text[index] + 1)) +
						MIN_SPACING	+ (type == IS_TEXT ? 0 : CHECK_SIZE);
					if (col_width < pixels) 
						col_width = pixels;		/* increase column width 	*/
				}
			}
			total_width += col_width;			/* calc total window width 	*/
		}

		/* if it's too big, reduce number of columns by 1 and try again 	*/

		if (total_width > max_pixels)			/* too big for window?		*/
		{
			if (last_good_lines)				/* did  something fit once?	*/ 
				break;							/*  go with it 				*/
			if (columns == 1)					/* if we can't reduce anymore */
			{
try_topaz8:		/* not so fast... lets try it with topaz 8 first */
				
				if (efont == NULL)				/* looked for Topaz8 yet?	*/
				{
					extern struct TextAttr Topaz8;

					if ( !(efont = OpenFont(&Topaz8)) )
					{
						memerr();				/* where did Topaz 8 go??   */
						return 0;				/* bail out now				*/
					}

					ofont = rp->Font;			/* remember curr font		*/
					SetFont(rp,efont);			/* now topaz 8				*/
					UFont = efont;				/* set this too!			*/
					xsize = TextLength(rp,"MEi",3) / 3;

					/*	Reset the window size parameters and try again		*/
					
					line_height =
						clamp(type != IS_CHECKMARK ? 9 : 11,rp->TxHeight,100);
					max_lines = (box->MaxY - box->MinY - 4) 
						/ (line_height + gap);
					goto again;
				}

				/*	Well, that's a fine kettle of fish...
				 *
				 *	It seems that no matter what we do we can't fit the
				 *	box and the longest text into the window in even a
				 *	single column.  We can't easily make the box any bigger,
				 *	so what we should do here is use two lines for the gadget
				 *	and text and just warp the text.  This will perhaps do
				 *	odd things to how we normally expect gadgets to be layed
				 *	out, but I think it should still work...
				 *
				 *	Note that even then there is the pathalogical case of so
				 *	much button text on one button that it takes up more than
				 *	an entire window.  In that case, I think we are justified
				 *	in truncating the bottom part of the button text!
				 */

				last_good_lines = lines;		/* then go with that...		*/
				last_good_cols  = 1;
				last_good_width = max_pixels;
				break;
			}
			columns--;							/* one less column			*/
			continue;							/* try again				*/
		}

		gadgets_displayed = columns * lines;	/* max gadgets displayable	*/

		/*  If we can't display all the gadgets, we should give up now. 
		 *	But note that we actually don't!
		 */
		
		last_good_lines = lines;
		last_good_cols  = columns;
		last_good_width = total_width;

		if (gadgets_displayed < count)
		{
			if (special) 
				break;

			/* ok, lets try a smaller font... */

			if (efont == NULL) 
				goto try_topaz8;

			/* well, lets reduce the spacing */

			if (gap == 0) 
				break;

			gap = 0;
			max_lines = (box->MaxY - box->MinY - 4) / line_height;
			goto again;
		}

		/* 	at this point we know that the gadgets will fit on the screen.
		 * 	if columns are too uneven, then give it another try with 
		 *	less lines.
		 *
		 *	Of course, the above statement might be a lie.  It is entirely
		 *	possible to get here when the text line WON'T fit in the window.
		 */

		if (gadgets_displayed - count < columns) 
			break;
		lines--;
	}

	if (last_good_lines)
	{
		lines 		= last_good_lines;
		columns 	= last_good_cols;
		total_width = last_good_width;
	}

	/* calculate the total width of all columns */

	gadgets_displayed = 0;

	total_width -= MIN_SPACING * columns;

	spacing = (max_pixels - total_width) / (columns + 1);
	total_width = (window_width - box->MaxX) + spacing + 4;

	top_position = (box->MinY + box->MaxY - lines * (line_height+gap))/2;

	for (col=0; col<columns; col++)
	{
		WORD	col_width = 0;

		for (line = 0; line < lines; line++)
		{
			WORD index = line + col*lines - wraplines;	/* index to gadget */
			WORD pixels;
			WORD x = total_width;

			if (index < count)					/* if we have this many gadgets */
			{
				WORD y = top_position + line * (line_height + gap);
				struct Gadget *gad;

				/*	Before we put anything on this line, make sure that
				 *	the string text will fit on the line, or if it won't,
				 *	make sure there is one more line in this (single)
				 *	column for the continuation of the text.
				 */
				 
				str    = (char *)(text[index] + 1);
				temp   = str;
				init_wrap();					/* init wrap settup			*/ 
				length = find_wrap (&temp, 
					max_pixels - (type == IS_TEXT ? 0 : CHECK_SIZE) - 4);
				if (temp && line+1 >= lines)	/* room for wrap line?		*/
						continue;				/* no, don't show it		*/
						
				if (type != IS_TEXT)
				{
					gad = &checkmarks[checkmark_count++];
					zero (gad, sizeof *gad);
					if (start == NULL) 
						start = gad;
				}

				if (type == IS_CHECKMARK)
					DrawBevel(rp,x,y,21,line_height,0);

				SetAPen(rp,text_color);
				Move   (rp,
						x + (type == IS_TEXT ? 0 : CHECK_SIZE),
						y + (rp->TxBaseline + line_height - 1) / 2);

				Text (rp, str, length);			/* format first line		*/

				c = str[length];
				str[length] = '\0';
				pixels = TextPixels(str) + (type == IS_TEXT ? 0 : CHECK_SIZE);
				str[length] = c;
				
				if (col_width < pixels) 
					col_width = pixels;
				
				if (temp)						/* have overflow text?		*/
				{
					str = temp;					/* text start pointer		*/
					length = find_wrap (&temp, 	/* truncate text?			*/
						max_pixels - (type == IS_TEXT ? 0 : CHECK_SIZE) - 4);
					str[length] = '\0';
					Move   (rp,
							x + (type == IS_TEXT ? 0 : CHECK_SIZE),
							y + (rp->TxBaseline + line_height - 1) / 2
							  + line_height + gap);
					Text (rp, str, length);		/* format second line		*/
                    pixels = TextPixels(str) 
                    		 + (type == IS_TEXT ? 0 : CHECK_SIZE);
                    if (col_width < pixels) 
                        col_width = pixels;
                    wraplines++;				/* wrapped another line		*/
                    line++;						/* used another line!		*/
				}

				if (type != IS_TEXT)
				{
					gad->LeftEdge = x;
					gad->TopEdge =
						(type == IS_CHECKMARK ? y : y + (line_height - 9)/2);
					gad->Width = (type == IS_CHECKMARK ? 21 : 16);
					gad->Height = (type == IS_CHECKMARK ? line_height : 9);
					gad->UserData = (APTR)start;

					if (type == IS_CHECKMARK) 
						build_checkmark(gad,id++);
					else 
						build_radio(gad,id++);

					gad->NextGadget = *list;
					*list = gad;
				}

				gadgets_displayed++;
			}
		}
		total_width += col_width + spacing;
	}

	if (efont)	/* if needed emergency font, reset old font and close efont */
	{
		SetFont(rp,ofont);
		UFont = ofont;							/* reset this too			*/
		CloseFont(efont);
		xsize = TextLength(rp,"MEi",3) / 3;		/* restore running value 	*/
	}

	return gadgets_displayed;
}

/* ====================== gadget management functions ===================== */

void ghost_gadget(UWORD id)
{	struct Gadget *gad = FindGadget(window,id);

	if (gad)
	{
		if (gad->Activation & (GADGIMMEDIATE|RELVERIFY))
			gad->Flags |= GADGDISABLED;
		RefreshGList(gad,window,NULL,1);
	}
}

void enable_gadget(UWORD id)
{	struct Gadget *gad = FindGadget(window,id);

	if (gad)
	{
		if (gad->Activation & (GADGIMMEDIATE|RELVERIFY))
			gad->Flags &= ~GADGDISABLED;
		SetAPen(rp,0L);
		RectFill(rp,
				 gad->LeftEdge,
				 gad->TopEdge,
				 gad->LeftEdge+gad->Width-1,
				 gad->TopEdge+gad->Height-1);
		RefreshGList(gad,window,NULL,1);
	}
}

void disable_radio(struct Gadget *gad,WORD sel,WORD total)
{
	UWORD	b;

	while (--total != sel) 
		gad = gad->NextGadget;
	gad->Flags |= GADGDISABLED;

	b = window->BlockPen;
	window->BlockPen = darkest_color;
	RefreshGList(gad,window,NULL,1);
	window->BlockPen = b;
}

void default_radio (struct Gadget *gad, WORD sel, WORD total)
{
	while (--total != sel) 
		gad = gad->NextGadget;
	gad->Flags |= SELECTED;
}

WORD final_radio (struct Gadget *gad, WORD total)
{
	while (total--)
	{
		if (gad->Flags & SELECTED) 
			return total;
		gad = gad->NextGadget;
	}
	return -1;
}

void init_checkboxes (struct Gadget *gad, LONG mask, WORD total)
{
	ULONG	bits = 1 << (total-1);

	while (total--)
	{
		if (bits & mask) 
			gad->Flags |= SELECTED;
		else 
			gad->Flags &= ~SELECTED;
		gad = gad->NextGadget;
		bits >>= 1;
	}
}

ULONG final_checkboxes (struct Gadget *gad, WORD total)
{
	ULONG	mask = 0,
			bits = 1 << (total-1);

	while (total--)
	{
		if (gad->Flags & SELECTED) 
			mask |= bits;
		gad = gad->NextGadget;
		bits >>= 1;
	}

	return mask;
}

void set_slider(struct Gadget *gad, struct ListControl *ctrl)
{
	ULONG			bod, pot;
#ifndef ONLY2_0
	struct Slider	*sl = (struct Slider *)gad;
#endif

	if (ctrl->Count <= ctrl->Visible)
	{
		bod = MAXBODY;
		pot = 0;

#ifndef ONLY2_0
		if (IntuitionBase->LibNode.lib_Version < 36)
		{
			sl->Image.Height = gad->Height;
		}
#endif
	}
	else
	{	
		bod = ((UWORD)MAXBODY * ctrl->Visible) / (ULONG)ctrl->Count;
		pot = ((UWORD)MAXBODY * ctrl->Start) / (ULONG)(ctrl->Count - ctrl->Visible);

#ifndef ONLY2_0
		if (IntuitionBase->LibNode.lib_Version < 36)
		{
			sl->Image.Height =
				((UWORD)gad->Height * (ULONG)ctrl->Visible) / (ULONG)ctrl->Count;
		}
#endif
	}

	/* replace pot value */

	NewModifyProp(gad,window,NULL,prop_flags,0,pot,0xffff,bod,1);
}

void hilite_file(struct Gadget *gad, struct ListControl *ctrl)
{
	WORD	i,j,
			ty = rp->TxHeight + 1,
			h = gad->Height,
			lines = h / ty,
			off = (h - ty * lines) / 2;

	i = ctrl->Hilite - ctrl->Start;
	j = gad->TopEdge + off + i * ty;

	SetDrMd(rp,COMPLEMENT);
	RectFill(rp,
			 gad->LeftEdge+4,
			 j,
			 gad->LeftEdge+gad->Width-5,
			 j + rp->TxHeight - 1);

	Delay(5);

	RectFill(rp,
			 gad->LeftEdge+4,
			 j,
			 gad->LeftEdge+gad->Width-5,
			 j + rp->TxHeight - 1);

	SetDrMd(rp,JAM2);
}

void show_files(struct Gadget *gad, struct ListControl *ctrl)
{
	WORD	i,j,len,
			ty = rp->TxHeight + 1,
			h = gad->Height,
			lines = h / ty,
			off = (h - ty * lines) / 2;
	char	*str;
	LONG	start = ctrl->Start,
			delta = start - ctrl->OldStart;
	BOOL	scroll = (ctrl->OldStart != -1);

	ctrl->Visible = lines;
	i = 0;

	if (scroll && (delta >= lines || delta <= -lines)) 
		scroll = FALSE;

	if (scroll == FALSE)
	{
		SetAPen(rp,0L);
		RectFill(rp,
				 gad->LeftEdge,
				 gad->TopEdge,
				 gad->LeftEdge+gad->Width-1,
				 gad->TopEdge+gad->Height-1);
	}
	else
	{
		if (delta < 0) 
			lines = -delta;
		if (delta > 0)
		{
			i = lines - delta;
			start += i;
		}

		SetDrMd(rp,JAM1);
		SetBPen(rp,0L);
		ScrollRaster(rp, 
					 0, 
					 delta * ty, 
					 gad->LeftEdge, 
					 gad->TopEdge + off,
					 gad->LeftEdge+gad->Width-1, 
					 gad->TopEdge+off+ctrl->Visible*ty-1);
	}

	for (;start < ctrl->Count && i < lines;i++,start++)
	{
		j = gad->TopEdge + off + i * ty;
		Move(rp,gad->LeftEdge + 4,j + rp->TxBaseline);

		j = get_entry_type(ctrl->Files[start]);
		if (j == IS_ASSIGN || j == IS_DRAWER)
		{	
			SetDrMd(rp,JAM2);
			SetAPen(rp,0);
			SetBPen(rp,lightest_color);
			Text(rp,
				 GetLocalText(j == IS_DRAWER ? TX_DRAWER_LABEL : TX_ASSIGN_LABEL),
				 5);
			SetDrMd(rp,JAM1);
			Text(rp," ",1);
		}

		SetDrMd(rp,JAM2);
		SetAPen(rp,text_color);
		SetBPen(rp,0);
		str = STR_VAL(ctrl->Files[start]);
		if (j == IS_DEVICE)
		{
			len = strlen(str);
			Text(rp,str,len);
			Text(rp,"  ",2);
			str += len + 1;
		}
		Text(rp,str,strlen(str));
	}

	ctrl->OldStart = -1;
}

LONG which_file(struct Gadget *gad, struct ListControl *list, WORD my)
{
	LONG	j;
	WORD	ty = rp->TxHeight + 1,
			off = (gad->Height - ty * list->Visible) / 2;

	j = (my - (gad->TopEdge + off)) / ty;
	if (j >= list->Visible || j < 0) 
		return list->Count;
 	j += list->Start;
	if (j >= list->Count) 
		return list->Count;
	return j;
}

void set_integer_gadget(struct Gadget *gad, LONG val)
{
	WORD	i = RemoveGadget(window,gad);

	Sprintf(((struct StringInfo *)gad->SpecialInfo)->Buffer,"%ld",val);
	((struct StringInfo *)gad->SpecialInfo)->LongInt = val;
	AddGadget(window,gad,i);
	RefreshGList(gad,window,NULL,1L);
}

void set_zone(struct TextZone *gad,char *text)
{
	WORD i = RemoveGadget(window,(struct Gadget *)gad);

	strcpy(gad->Buffer,text);
	AddGadget(window,(struct Gadget *)gad,i);
	RefreshGList((struct Gadget *)gad,window,NULL,1L);
}

void expand_zone(struct TextZone *gad,BPTR lock)
{
	WORD	i;

	if (lock)
	{
		i = RemoveGadget(window,(struct Gadget *)gad);
		ExpandPath((void *)lock,gad->Buffer,gad->Info.MaxChars);
		AddGadget(window,(struct Gadget *)gad,i);
		RefreshGList((struct Gadget *)gad,window,NULL,1L);
	}
}

	/* fill in to do change-startup */
extern char				*textbuf;
extern int				textloc;

void init_text(struct Gadget *gad,struct ListControl *ctrl)
{
	char	*at;

		/* initialize some fields */
	ctrl->Visible = gad->Height / (rp->TxHeight + 1);
	ctrl->Files = NULL;
	ctrl->Count = ctrl->Hilite = 0;
	ctrl->OldStart = -1;

		/* convert tabs to spaces */
	at = textbuf;
	while (*at) 
	{ 
		if (*at == '\t') 
			*at = ' '; 
		at++; 
	}

		/* count the number of lines and find hilite point */
	at = textbuf;
	while (at = strchr(at,'\n'))
	{
		at++;
		ctrl->Count++;
		if (textloc == at - textbuf) 
			ctrl->Hilite = ctrl->Count;
	}

		/* determine which is the first displayed line */
	ctrl->Start = ctrl->Hilite - ctrl->Visible / 2;
	if (ctrl->Start + ctrl->Visible > ctrl->Count)
		ctrl->Start = ctrl->Count - ctrl->Visible;
	if (ctrl->Start < 0) 
		ctrl->Start = 0;
}


extern UWORD *RtArrow,*InsertMask,*InsertText;

void show_text (
	struct Gadget 		*gad, 
	struct ListControl 	*ctrl,
	WORD  				 mode)
{
	char	*loc,
			*at = textbuf;
	WORD	i,j,
			ty = rp->TxHeight + 1,
			h = gad->Height,
			lines = h / ty,
			off = (h - ty * lines) / 2;
	LONG	start = ctrl->Start;

	for (i=0;i<start; i++) 
		at = strchr(at,'\n') + 1;

	if (mode == 0)
	{
		SetAPen(rp,0L);
		RectFill(rp,
				 gad->LeftEdge,
				 gad->TopEdge,
				 gad->LeftEdge+gad->Width-1,
				 gad->TopEdge+gad->Height-1);
	}

	SetDrMd(rp,JAM1);
	SetAPen(rp,text_color);
	clip_on(rp,gad->LeftEdge+2,gad->TopEdge+1,
		gad->LeftEdge+gad->Width-3,gad->TopEdge+gad->Height-2);

	for (i=0;start < ctrl->Count && i < lines;i++,start++)
	{
		j = gad->TopEdge + off + i * ty;
		Move(rp,gad->LeftEdge + 11,j + rp->TxBaseline);

		loc = strchr(at,'\n');
		if (loc == NULL) 
			loc = at + strlen(at);
		j = loc - at;
		Text(rp,at,j);
		at = loc + 1;
	}

	j = gad->TopEdge + off + (ctrl->Hilite - ctrl->Start) * ty - 1;
	SetAPen(rp,highlight_color);
	Move(rp,gad->LeftEdge + 4,j);
	Draw(rp,gad->LeftEdge + gad->Width - 5,j);
	BltTemplate((void *)RtArrow,0,2,rp,gad->LeftEdge + 4,j-3,7,7);
	SetAPen(rp,0);
	BltTemplate((void *)InsertMask,
				0,
				14,
				rp,
				gad->LeftEdge + gad->Width - 102,
				j-4,
				99,
				9);
	SetAPen(rp,highlight_color);
	BltTemplate((void *)InsertText,
				0,
				14,
				rp,
				gad->LeftEdge + gad->Width - 102,
				j-4,
				99,
				9);

	clip_off(rp);
	SetDrMd(rp,JAM2);

	ctrl->OldStart = -1;
}


extern UWORD *RtArrow,*InsertMask,*InsertText;

void erase_text(struct Gadget *gad, struct ListControl *ctrl)
{
	WORD	j,
			ty = rp->TxHeight + 1,
			h = gad->Height,
			lines = h / ty,
			off = (h - ty * lines) / 2;

	clip_on(rp,gad->LeftEdge+2,gad->TopEdge+1,
		gad->LeftEdge+gad->Width-3,gad->TopEdge+gad->Height-2);

	SetDrMd(rp,JAM1);
	SetAPen(rp,0);
	j = gad->TopEdge + off + (ctrl->Hilite - ctrl->Start) * ty - 1;
	Move(rp,gad->LeftEdge + 4,j);
	Draw(rp,gad->LeftEdge + gad->Width - 5,j);
	BltTemplate((void *)RtArrow,0,2,rp,gad->LeftEdge + 4,j-3,7,7);
	BltTemplate((void *)InsertMask,0,14,rp,gad->LeftEdge + gad->Width - 102,j-4,99,9);
	SetAPen(rp,highlight_color);

	clip_off(rp);
	SetDrMd(rp,JAM2);
}

LONG which_line(struct Gadget *gad, struct ListControl *list, WORD my)
{
	LONG	j;
	WORD	ty = rp->TxHeight + 1,
			off = (gad->Height - ty * list->Visible) / 2;

	j = clamp(0,(my - (gad->TopEdge + off) + ty / 2) / ty,list->Visible);
	return clamp(0,j + list->Start,list->Count);
}

#if 0
	/*	Only used in formatted listbox in startup-sequence editing.	*/
	
LONG final_text(struct Gadget *gad, struct ListControl *ctrl)
{
	char	*at = textbuf;
	WORD	i;
	LONG	hi = ctrl->Hilite;

	for (i=0;i<hi;i++) 
		at = strchr(at,'\n') + 1;
	return at - textbuf;
}
#endif

void radio_space_check(WORD count)
{
	WORD	h = clamp(9,rp->TxHeight,100),
			m = (place_bottom - place_top - 4 - (rp->TxHeight + 1)) / (h + 1);

	if (m < count)
	{
		h = count * (h + 1) + 6 + (rp->TxHeight + 1) -
			(place_bottom - place_top);

		place_top -= h / 2;
		place_bottom += h / 2;
	}
}

struct Gadget *find_key_gadget(UWORD code)
{
	struct Gadget	*gad = window->FirstGadget;
	char			*ch;

	while (gad)
	{
		if (gad >= (struct Gadget *)buttons &&
			gad < (struct Gadget *)&buttons[MAX_BUTTONS])
		{
			if (ch = strchr(((struct Button *)gad)->Text,'_'))
			{
				if (tolower(ch[1]) == code) return gad;
			}
		}

		gad = gad->NextGadget;
	}

	return NULL;
}

/* ==================== master gadget creation function ==================== */

char *gir_prompt, *gir_default, *gir_help, *gir_deftool;

char *make_gad_text(struct GadgetDef *def)
{
	char	*text = def->Text;

	if (text != NULL && def->Flags & GFLAG_PTR) 
		text = *(char **)text;
	if (def->Flags & GFLAG_LOCAL) 
		text = GetLocalText((long)text);

	return text;
}

LONG make_gadgets (
	struct Gadget 			 ** gad  ,
	struct GadgetDef 		  * def  ,
	struct InstallationRecord * ir   ,
	UWORD					    mode )
{
	static LONG			u;
	struct MultiData	*data;
	struct Rectangle 	box;
	char				*text, 
						*text1, 
						*text2;
	UWORD				b, 
						type;
	WORD				dx,
						dy;
	LONG				longint,
						val 	= 0;
	BOOL				radio_special;

	if (mode & GCLEAR_BUSY) 
		clear_busy();
	if (gad == NULL)
	{
		if (mode & GSTART_LAYOUT) 
			start_layout();
	}
	else 
	if (*gad == NULL)
	{
		if (mode & GSTART_LAYOUT) 
			start_layout();
		else 
			restart_layout();
	}

	if (mode & GERASE_PAGE) 
		erase_page();

	do {
		if ((def->Flags & GTYPE_MASK) != GTYPE_INTGAD)
		{
			if (ir != NULL)
			{
				if (ir->prompt_val.type == TYPE_STRING)
					gir_prompt = STR_VAL(ir->prompt_val.v);

				if (ir->default_val.type == TYPE_STRING)
					gir_default = STR_VAL(ir->default_val.v);

				if (ir->help_val.type == TYPE_STRING)
					gir_help = STR_VAL(ir->help_val.v);

				if (ir->deftool_val.type == TYPE_STRING)
					gir_deftool = STR_VAL(ir->deftool_val.v);
			}
		}
		else										/* GTYPE_INTGAD */
		{
			if (ir != NULL) 
                longint = (LONG)ir->deftool_val.v;
		}

		text = make_gad_text(def);

		/*****************************************************************/
		/*****************************************************************/
		
		switch (type = def->Flags & GTYPE_MASK)
		{
		case GTYPE_GAUGE:
			layout_gauge(gad,GAUGE_ID);
			break;			


		case GTYPE_ADJUST:
			switch (def->Flags & GWIDTH_MASK)
			{
			case GADJUST_TOP: 	 place_top += def->ID; 		break;

			case GADJUST_BOTTOM: place_bottom -= def->ID; 	break;

			case GADJUST_CENTER: center_layout(def->ID); 	break;
			}
			break;
	

		case GTYPE_LIST:
			list_gadget( gad, LIST_ID);
			
			DrawBevel 	   (rp,
							left_edge,
							place_top,
							window_width-right_edge-28-left_edge,
							place_bottom-place_top-2,
							0);
			break;			
	

		case GTYPE_SLIDER:		
			slider_gadget  (gad,
							SLIDER_ID,
							window_width-right_edge-24,
							place_top,
							24,
							place_bottom-24-place_top);
			break;
	

		case GTYPE_ARROW:		
			arrow_gadget   (gad,
							UP_ID,
							window_width-right_edge-24,
							place_bottom-24,
							UP_ARROW);
							
			arrow_gadget   (gad,
							DN_ID,
							window_width-right_edge-24,
							place_bottom-13,
							DN_ARROW);
			break;


		case GTYPE_TEXT:
			if (text)
				layout_text(text,
							def->Flags >> 8  & 0x0f,
							def->Flags >> 12 & 0x07,
							def->Flags & GWIDTH_MASK);
			break;


		case GTYPE_WRAP:
			if (text)
				layout_wrap_text(text,
								 def->Flags >> 8  & 0x0f,
								 def->Flags >> 12 & 0x07);
			break;

	
		case GTYPE_STRGAD:		
			layout_strgad  (gad,
							text,
							def->ID,
							def->Flags >> 8 & 0x0f);
							
			if ((def->Flags & GWIDTH_MASK) == GSTRING_ACTIVE)
				tz_activate = (struct TextZone *)*gad;
				
			break;


		case GTYPE_INTGAD:
			layout_intgad  (gad,
							longint,
							def->ID,
							def->Flags >> 8 & 0x0f);
							
			if ((def->Flags & GWIDTH_MASK) == GSTRING_ACTIVE)
				tz_activate = (struct TextZone *)*gad;
			break;


		case GTYPE_RADIO:
		case GTYPE_CHECKBOX:
		
			box.MinX = left_edge;
			box.MaxX = window_width - right_edge;

			radio_special = 0;

			if ((def->Flags & GWIDTH_MASK) == GRADIO_SPECIAL1)
			{
				u = (place_bottom - place_top)/8;		/* static */
				box.MinY = place_bottom - 4 * u - 1;
				box.MaxY = place_bottom - u - 1;

				DrawBevelRect(rp,&box,0);
				radio_special = 1;
			}
			else 
			if ((def->Flags & GWIDTH_MASK) == GRADIO_SPECIAL2)
			{
				box.MinY = place_top + 1 * u + 1;
				box.MaxY = place_top + 3 * u + 1;

				DrawBevelRect(rp,&box,0);
				radio_special = 1;
			}
			else
			{
				box.MinY = place_top;
				box.MaxY = place_bottom;
			}

			data = (struct MultiData *)text;

			val = layout_box_gads
					   (gad,
						&box,
						data->Choices,
						data->NumChoices,
						def->ID,
						(type == GTYPE_RADIO ? IS_RADIO : IS_CHECKMARK) |
							((def->Flags & GWIDTH_MASK) == GCHECK_SPECIAL1 
								? 0x80 
								: 0)
						);

			if (type == GTYPE_RADIO)
				default_radio  (*gad,
								data->DefChoice,
								data->NumChoices);
			else 
			if (data->DefChoice != -1)
				init_checkboxes(*gad,
								data->DefChoice,
								data->NumChoices);
			if (radio_special) 
				place_bottom = box.MinY;
			break;


		case GTYPE_BEVEL:
		
			dx = 2 * xsize;
			dy = (UFont->tf_YSize + 1) / 2;
			
			switch (def->Flags & GWIDTH_MASK)
			{
			case GBEVEL_STD:
			
				DrawBevel  (rp,
							left_edge,
							place_top,
							window_width-right_edge-left_edge,
							place_bottom-place_top-2,
							def->ID);
				break;

			case GBEVEL_SAVE:
				left_edge 	+= dx;
				right_edge 	+= dx;
				top_edge 	+= dy;
				bottom_edge += dy;

				erase_page();
				DrawBevel  (rp,
							left_edge,
							top_edge,
							window_width-right_edge-left_edge,
							window_height-bottom_edge-top_edge,
							def->ID);

				left_edge 	+= window->BorderLeft;
				right_edge 	+= window->BorderRight;
				top_edge 	+= window->BorderBottom;
				bottom_edge += window->BorderBottom;

				restart_layout();
				break;

			case GBEVEL_SCALE:
				left_edge 	+= window->BorderLeft + dx;
				right_edge 	+= window->BorderRight + dx;
				top_edge 	+= window->BorderBottom + dy;
				bottom_edge += window->BorderBottom + dy;
				place_top  	 = top_edge;
				place_bottom = window_height - bottom_edge + 1;
#if 0
				SetAPen(rp,0);
				RectFill(rp,left_edge,top_edge,
					window_width-right_edge-left_edge,def->ID);
#endif
				break;
			}
			break;


		case GTYPE_BUTTON:
			if (text)
			{ 
                switch (def->Flags & GWIDTH_MASK)
                {
                case GWIDTH_HALF:
                    layout_button  (gad,
                    				text,
                    				def->ID,
                    				def->Flags >> 8 & 0x0f,
                    				HALF_WIDTH);
                    break;

                case GWIDTH_SIDES:
                    text1 = make_gad_text(&def[1]);
                    layout_side_buttons(gad,
                    					text,
                    					def->ID,
                        				text1,
                        				def[1].ID,
                        				def->Flags >> 8 & 0x0f);
                    def++;
                    break;

                case GWIDTH_2:
                    text1 = make_gad_text(&def[1]);
                    layout_2_buttons   (gad,
                    					text,
                    					def->ID,
                        				text1,
                        				def[1].ID,
                        				def->Flags >> 8 & 0x0f);
                    def++;
                    break;

                case GWIDTH_3:
                    text1 = make_gad_text(&def[1]);
                    text2 = make_gad_text(&def[2]);
                    layout_3_buttons   (gad,
                    					text,
                    					def->ID,
                    					text1,
                    					def[1].ID,
                        				text2,
                        				def[2].ID,
                        				def->Flags >> 8 & 0x0f);
                    def += 2;
                    break;
                }
			}
			break;
		}
	} while ( !((def++)->Flags & GFLAG_LAST) );

	if (mode & GSET_GADGETS)
	{
		b = window->BlockPen;
		window->BlockPen = darkest_color;
		set_gadgets(*gad);
		window->BlockPen = b;
	}

	return val;
}

/* ====================== clipping region management ======================= */

static struct Rectangle crect;
static struct Region *clip_reg=NULL, *old_reg=NULL;

void clip_off(struct RastPort *rp)
{
	if (clip_reg)
	{
		InstallClipRegion(rp->Layer,old_reg);
		DisposeRegion(clip_reg);
		clip_reg = NULL;
	}
}

void clip_on(
	struct RastPort *rp,
	WORD x1,
	WORD y1,
	WORD x2,
	WORD y2)
{
	crect.MinX = x1;
	crect.MinY = y1;									 /* set the clip coords */
	crect.MaxX = x2;
	crect.MaxY = y2;
	clip_off(rp);										 /* remove previous clip */
	if (clip_reg = NewRegion())
	{
		OrRectRegion(clip_reg,&crect);					 /* add in the clip rect */
		old_reg = InstallClipRegion(rp->Layer,clip_reg); /* install into rastport */
	}
}
