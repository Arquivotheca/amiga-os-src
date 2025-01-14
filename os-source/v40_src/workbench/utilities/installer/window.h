/* ========================================================================= *
 * Window.h - creates the window for installer utility                       *
 * By Talin. (c) 1990 Sylvan Technical Arts                                  *
 * ========================================================================= */

#ifndef INSTALLER_WINDOW_H
#define INSTALLER_WINDOW_H

#ifndef EXEC_TYPES_H
#include <intuition/intuition.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <graphics/gfxbase.h>
#include <exec/memory.h>
#endif

/* ========================= Useful defines ============================== */

#define PROPFLAGS (FREEVERT | PROPBORDERLESS)

#define STD_FLAGS (MOUSEBUTTONS | MENUPICK | MOUSEMOVE | RAWKEY | GADGETDOWN | GADGETUP)

#define RAW_HELP		0x5f
#define RAW_ESC			0x45
#define RAW_UP			0x4c
#define RAW_DOWN		0x4d
#define RAW_LEFT		0x4f
#define RAW_RIGHT		0x4e

/* ========================= Gadgets ===================================== */

#define PUT_UPPER		0
#define PUT_LOWER		(1<<0)
#define PUT_STAY		(1<<1)
#define PUT_CENTER		0
#define PUT_LEFT		(1<<2)
#define PUT_RIGHT		(1<<3)

#define FULL_WIDTH		1
#define HALF_WIDTH		2
#define THIRD_WIDTH		3

#define TEXT_CENTER		(1<<0)
#define TEXT_BOLD		(1<<1)
#define TEXT_ITALICS	(1<<2)

#define MAX_SLIDERS		2
#define MAX_BUTTONS		21
#define MAX_CHECKMARKS	40
#define MAX_TEXTZONES	4
#define MAX_TZ_CHARS	512
#define IS_CHECKMARK	0
#define IS_RADIO		1
#define IS_TEXT			2

#define UP_ARROW		0
#define DN_ARROW		1

#define FIRSTRESV_ID	87
#define GAUGE_ID		87
#define COPYABORT_ID	88
#define CREATE_ID		89
#define PROCEED_ID		90
#define ABORT_ID		91
#define DRAWER_ID		92
#define DRIVES_ID		93
#define PARENT_ID		94
#define FILENAME_ID		95
#define LIST_ID			96
#define SLIDER_ID		97
#define UP_ID			98
#define DN_ID			99
#define HELP_ID			100

struct TextZone {
	struct Gadget		Gadget;
	struct StringInfo	Info;
	char				Buffer[MAX_TZ_CHARS];
};

struct Slider {
	struct Gadget	Gadget;
	struct PropInfo	Info;
	struct Image	Image;
};

struct ListControl {
	struct String	**Files;
	LONG			Count;
	LONG			Start;
	LONG			Visible;
	LONG			Hilite;
	struct TextZone	*FileZone;
	struct TextZone	*DrawerZone;
	BPTR			Lock;
	WORD			Mode;
	WORD			Match;
	BPTR			LastLock;
	LONG			OldStart;
};

struct Button {
	struct Gadget	Gadget;
	char			*Text;
	struct Border	Border[4];
	WORD			Vector[20];
};

	/* shift right 8 and mask by 0x0f */
#define GPUT_UPPER		0
#define GPUT_LOWER		(1<<8)
#define GPUT_STAY		(1<<9)
#define GPUT_CENTER		0
#define GPUT_LEFT		(1<<10)
#define GPUT_RIGHT		(1<<11)

#define GWIDTH_FULL		(1<<4)		/* not used */
#define GWIDTH_HALF		(2<<4)
#define GWIDTH_THIRD	(3<<4)		/* not used */
#define GWIDTH_SIDES	(4<<4)
#define GWIDTH_2		(5<<4)
#define GWIDTH_3		(6<<4)
#define GWIDTH_MASK		0x0070

	/* shift right by 12 and mask by 0x07 */
#define GTEXT_CENTER	(1<<12)
#define GTEXT_BOLD		(1<<13)

#define GFLAG_PTR		(1<<7)
#define GFLAG_LOCAL		(1<<14)
#define GFLAG_LAST		(1<<15)

#define GTYPE_NONE		0
#define GTYPE_TEXT		1
#define GTYPE_BUTTON	2
#define GTYPE_STRGAD	3
#define GTYPE_ARROW		4
#define GTYPE_SLIDER	5
#define GTYPE_LIST		6
#define GTYPE_RADIO		7
#define GTYPE_CHECKBOX	8
#define GTYPE_WRAP		9
#define GTYPE_ADJUST	10
#define GTYPE_INTGAD	11
#define GTYPE_BEVEL		12
#define GTYPE_GAUGE		13
#define GTYPE_MASK		0x000f

#define GADJUST_TOP		(1<<4)
#define GADJUST_BOTTOM	(2<<4)
#define GADJUST_CENTER	(3<<4)

#define GBEVEL_STD		(1<<4)
#define GBEVEL_SAVE		(2<<4)			/* also does other processing		*/
#define GBEVEL_SCALE	(3<<4)			/* scales bounds to old bevel, etc.	*/

#define GRADIO_SPECIAL1	(1<<4)			/* special code for get_options 	*/
#define GRADIO_SPECIAL2	(2<<4)
#define GCHECK_SPECIAL1	(3<<4)			/* ok to break-up (copyfiles)		*/

#define GSTRING_ACTIVE	(1<<4)			/* activate this string gadget		*/

#define GTEXT_PREWIPE	(1<<4)			/* prewipe area (GTYPE_TEXT only)	*/
#define GTEXT_SKIP		(2<<4)			/* skip, but act as printed			*/
#define GTEXT_PATH		(3<<4)			/* prewipe & strip path so it fits	*/

#define GSTART_LAYOUT	(1<<0)
#define GERASE_PAGE		(1<<1)
#define GSET_GADGETS	(1<<2)
#define GCLEAR_BUSY		(1<<3)

struct GadgetDef {
	void			*Text;
	UWORD			Flags;
	UWORD			ID;
};

struct MultiData {
	struct String	**Choices;
	LONG			NumChoices;
	LONG			DefChoice;
};

#endif
