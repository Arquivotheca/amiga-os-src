/* ============================================================================ *
                 drum.c -- a simple drum machine using CAMD and RealTime
                                      By Talin.
                               ©1991 The Dreamers Guild
 * ============================================================================ */

#define MCLOCKS_PER_QNOTE	480

/* ============================================================================ *
                                        Notes
 * ============================================================================ */

#if 0
	What's next?
		-- program gadget for setup. Init strings? Standard init strings?
			-- a UI to compose a set of MIDI strings...?
		-- mute and solo gadgets. (Do as bevel boxes?)
		-- allow numpad keys to also work
		-- disallow key repeats.
		-- pattern length and resolution.
		-- do maestro calculations time correctly.
		-- solve maestro locating problem.
		-- seperate task to handle the actual timing chores
		-- recording as we go.
		-- add in toggle buttons (boopsi) for mute. (or just "roll-yer-own??)
		-- add in conductor setting and output link setting lists
				(use transport code...)
		-- hook up individual menu items.

	Menus:
		Project
			Load
			Save
		?	Save Settings?
			Quit
			About
		Edit
			?? Undo ??
			Erase Pattern
			Reverse Pattern
		Play
			Whole Song
			--------------
			Set Conductor...
			Set MIDI Output Port...
		Pattern
			Set Pattern Length...
			Set Drum Resolution >>  (Quarter note, etc...)

	Other ideas:
		-- a pattern arrangement dialogue...
		-- play backwards.
		-- ARexx port?

	WA_MenuHelp
	WA_NotifyDepth
#endif

/* ============================================================================ *
                                       Includes
 * ============================================================================ */

#include "std_headers.h"
#include <intuition/icclass.h>
#include <libraries/iffparse.h>
#include <libraries/asl.h>
#include <libraries/realtime.h>
#include <midi/mididefs.h>
#include <midi/camd.h>
#include <clib/asl_protos.h>
#include <clib/camd_protos.h>
#include <clib/realtime_protos.h>
#include <clib/alib_protos.h>
#include <pragmas/realtime_pragmas.h>
#include <pragmas/camd_pragmas.h>
#include <stdlib.h>
#include <math.h>

#include "arrows.h"								/* increment arrows				*/

/* ============================================================================ *
                                      Prototypes
 * ============================================================================ */

struct DrumColumn;
void draw_titlebar_scale( void );
void render_column( int column, int highlight );
void play_drum_note( struct DrumColumn *dc, BOOL note_on );
void preview_row( int row );
void step_play( int dir );

BOOL save_pattern( void );

void calc_tempo( void );
void calc_pattern_ticks( void );
void locate_to_tick ( long tick_count );

struct Gadget *make_arrow( struct NewGadget *ng, struct Gadget *prev, int min, int max, int current );
ULONG AddTailObject( Object *o, struct List *list);
void DisposeObjectList( struct List *l );
void shadow_drag( int col1, int col2 );

extern Class *initArrowButtonClass(void);
void freeArrowButtonClass(Class *sclass);

/* ============================================================================ *
                                       Defines
 * ============================================================================ */

#define clamp(a,b,c)	min( max( a, b ), c )	/* constrain b between a and c	*/

#define APPNAME			"Drum Machine"
#define DRUM_ROWS		10
#define ROW_HEIGHT		14
#define MAX_MATRIX_WIDTH 400
#define MAX_COLUMNS		64
#define MAX_PATTERNS	128
#define MATRIX_X		224

enum gadget_ids {
	ID_MUTE = 1,								/* mute button per row			*/
	ID_SOLO,									/* solo button per row			*/
	ID_PNAME,									/* name string per row			*/
	ID_KEY,										/* key to play -- per row		*/
	ID_KEYINC,									/* key increment arrows			*/
	ID_HIT,										/* hit drum now...				*/
	ID_FILE,									/* filename						*/
	ID_PLAY,									/* play button					*/
	ID_STOP,									/* stop button					*/
	ID_STEPFWD,									/* step fwd button				*/
	ID_STEPBACK,								/* step backwd button			*/
	ID_CHANNEL,									/* channel slider				*/
	ID_CHANNELINC,
	ID_TEMPO,									/* tempo slider					*/
	ID_VELOCITY,								/* velocity slider				*/
};

	/*	Normally, the columns of the drum grid are equal to a sixteenth note
		in duration, however this can be changed. Here are the values that
		the "drum resolution" value can take.
	*/

enum drum_res {
	DRES_NOTE1=0,								/* whole note					*/
	DRES_NOTE2,									/* half note					*/
	DRES_NOTE4,									/* quarter note					*/
	DRES_NOTE8,									/* eighth note					*/
	DRES_NOTE16,								/* etc. */
	DRES_NOTE32,
	DRES_NOTE64,
	DRES_NOTE128,
};

	/*	Defines a single column of the drum pattern */

struct DrumColumn {
	UBYTE				note[ DRUM_ROWS ];
};

	/*	Defines the setup information for the drum kit */

struct DrumSetEntry {
	char				name[20];
	UBYTE				key;
	UBYTE				flags;
};

typedef struct DrumSetEntry	DrumKit[ DRUM_ROWS ];

#define DRUMF_MUTE		(1<<0)

struct DrumPattern {
	UWORD				dpNumber;				/* pattern number				*/
	UWORD				dpColumns,				/* pattern columns (global now)	*/
						dpRows;					/* pattern rows (constant now)	*/
	DrumKit				dpSetup;				/* setup for pattern			*/
	struct DrumColumn	dpGrid[ MAX_COLUMNS ];	/* actual pattern data			*/
};

DrumKit					ReadKit;

/* ============================================================================ *
                                       Globals
 * ============================================================================ */

struct Library			*IntuitionBase,
						*GadToolsBase,
						*AslBase,
						*GfxBase,
						*UtilityBase,
						*CamdBase,
						*RealTimeBase,
						*IFFParseBase;

extern struct Library	*SysBase,
						*DOSBase;

struct Window			*window;
struct RastPort			*rp;
struct VisualInfo		*vi;
struct DrawInfo			*dri;
UWORD					*dri_pens;
struct FileRequester	*filereq;

BOOL					running = TRUE;

struct MidiNode			*midi;
struct MidiLink			*out_link;

struct Gadget			*context_gadget,
						*first_gadget,
						*last_gadget,
						*first_arrow,
						*arrow_gadget;

struct Gadget			*key_gadgets[ DRUM_ROWS ],
						*name_gadgets[ DRUM_ROWS ],
						*keyinc_gadgets[ DRUM_ROWS ],
						*channel_string,
						*channel_arrow;

BOOL					tempo_drag;

WORD					drag_state,
						drag_row,
						drag_column,
						drag_anchor_column;

#define DRAG_SET		1
#define DRAG_RESET		2

struct MinList			arrow_list;

struct TextAttr			topaz8 = { (STRPTR)"topaz.font", 8, FS_NORMAL, 0x0 };

struct BitMap			*off_bm;
struct RastPort			off_rp,
						off_shine_rp,
						off_shade_rp;

void					do_quit( struct IntuiMessage *msg );
void					do_about( struct IntuiMessage *msg );
void					do_savepattern( struct IntuiMessage *msg );
void					do_loadpattern( struct IntuiMessage *msg );
void					do_clear( struct IntuiMessage *imsg );

struct NewMenu menu_list[] = {
	{	NM_TITLE,	"Project",			NULL,			NULL,	0,	NULL },
	{	NM_ITEM,	"Load Pattern...",	(UBYTE *)"L",	NULL,	0,	do_loadpattern },
	{	NM_ITEM,	"Load Song...",		(UBYTE *)"O",	ITEMENABLED,	0,	NULL },
	{	NM_ITEM,	"Import from MIDI File...",NULL,	ITEMENABLED,0,NULL },
	{	NM_ITEM,	NM_BARLABEL,		NULL,			NULL,	0L },
	{	NM_ITEM,	"Save Pattern...",	(UBYTE *)"S",	NULL,	0,	do_savepattern },
	{	NM_ITEM,	"Save Song...",		(UBYTE *)"W",	ITEMENABLED,0,NULL },
	{	NM_ITEM,	"Export to MIDI File...",NULL,		ITEMENABLED,0,NULL },
	{	NM_ITEM,	NM_BARLABEL,		NULL,			NULL,	0L },
	{	NM_ITEM,	"About...",			(UBYTE *)"?",	NULL,	0,	do_about },
	{	NM_ITEM,	"Quit",				(UBYTE *)"Q",	NULL,	0,	do_quit },
	{	NM_TITLE,	"Edit",				NULL,			NULL,	0,	NULL },
	{	NM_ITEM,	"Copy Pattern",		(UBYTE *)"C",	ITEMENABLED,	0,	NULL },
	{	NM_ITEM,	"Paste Pattern",	(UBYTE *)"V",	ITEMENABLED,	0,	NULL },
	{	NM_ITEM,	"Erase Pattern",	(UBYTE *)"E",	NULL,	0,	do_clear },
	{	NM_ITEM,	NM_BARLABEL,		NULL,			NULL,	0L },
	{	NM_ITEM,	"Reverse Pattern",	(UBYTE *)"R",	ITEMENABLED,	0,	NULL },
	{	NM_TITLE,	"Playback",			NULL,			NULL,			0,	NULL },
	{	NM_ITEM,	"Play",				NULL,			ITEMENABLED,	0,	NULL },
	{	NM_SUB,		"Whole Song",		(UBYTE *)"H",	CHECKIT | CHECKED,	0, NULL },
	{	NM_SUB,		"Pattern",			(UBYTE *)"P",	CHECKIT,			0, NULL },
	{	NM_ITEM,	NM_BARLABEL,		NULL,			NULL,	0L },
	{	NM_ITEM,	"Set Conductor...",	(UBYTE *)"T",	ITEMENABLED,	0,	NULL },
	{	NM_ITEM,	"Set MIDI Output...",(UBYTE *)"M",	ITEMENABLED,	0,	NULL },
	{	NM_TITLE,	"Pattern",			NULL,			NULL,	0,	NULL },
	{	NM_ITEM,	"Set Pattern Length...",(UBYTE *)"G",ITEMENABLED,	0, NULL },
	{	NM_ITEM,	"Set Resolution...",NULL,			NULL,			0, NULL },
	{	NM_END,		NULL,				NULL,			0,		0,	NULL }
};

struct Menu				*menu_strip;

	/* drum matrix variables */

WORD					matrix_top,
						matrix_bottom,
						matrix_height,
						matrix_y_org;

WORD					total_columns = 32;
						column_width,
						matrix_width;

struct DrumPattern		*pattern_table[ MAX_PATTERNS ];
struct DrumPattern		base_pattern,
						*current_pattern = &base_pattern;

WORD					solo_drum = -1;

WORD					current_velocity = 64,
						current_tempo = 100,
						current_channel = 5;

Class					*aclass = NULL;

	/* playback variables */

struct Player			*drum_player;

WORD					playback_column = 0;

WORD					clock_state;
struct Task				*main_task;

	/*	Values which are set before playing begins */

enum drum_res			drum_resolution = DRES_NOTE16; /* size of each column		*/

UWORD					mclocks_per_column;		/* metric clocks per column		*/
LONG					pattern_mclocks;		/* metric clocks per pattern	*/

	/*	Values which can change during play */

UWORD					tempo_rate;				/* ratio of mtime / real time	*/

	/*	Values which are continuously incremented during play */

LONG					current_pattern_start,	/* start time of current pattern */
						current_pattern_end;	/* start time of next pattern	*/

LONG					clock_pos,				/* clock position				*/
						mclock_accumulator,		/* metric time accumulator		*/
						mclock_pos;				/* metric time position			*/

LONG					pattern_pos;			/* score position, in patterns	*/
UWORD					column_pos;				/* column position in pattern	*/


#define ASM __asm
#define REG(x) register __## x
        
extern ULONG ASM TimeServer (REG(a1)struct pmTime *msg, REG(a2)struct Player *pi);

struct Hook myHook = {
	{ NULL, NULL },
	TimeServer,
};

/* ============================================================================ *
                                    Glue Functions
 * ============================================================================ */

struct MidiNode *CreateMidi( Tag tag, ... )
{	return CreateMidiA( (struct TagItem *)&tag );
}

BOOL SetMidiAttrs( struct MidiNode *mi, Tag tag, ... )
{	return SetMidiAttrsA( mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink( struct MidiNode *mi, LONG type, Tag tag, ... )
{	return AddMidiLinkA( mi, type, (struct TagItem *)&tag );
}

BOOL SetMidiLinkAttrs( struct MidiLink *mi, Tag tag, ... )
{	return SetMidiLinkAttrsA( mi, (struct TagItem *)&tag );
}

struct Player *CreatePlayer( Tag tag, ... )
{	return CreatePlayerA( (struct TagItem *)&tag );
}

BOOL SetPlayerAttrs( struct Player *pi, Tag tag, ... )
{	return SetPlayerAttrsA( pi, (struct TagItem *)&tag );
}

/* ============================================================================ *
                                    Error Function
 * ============================================================================ */

LONG ErrorF( struct Window *w, UBYTE *TextFormat, ... )
{	struct EasyStruct es;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = APPNAME " Error Request";
	es.es_TextFormat = TextFormat;
	es.es_GadgetFormat = "Continue";

	return( EasyRequestArgs( w, &es, NULL, (APTR)( (&TextFormat) + 1) ));
}

struct Library *open_lib( char *name )
{	struct Library			*l;

	unless ( l = OpenLibrary( name, 0 ))
	{	ErrorF(	NULL, "Can't open library:\n%s", name);
	}
	return l;
}

/* ============================================================================ *
                                     main routine
 * ============================================================================ */

void main( int argc, char *argv[] )
{	struct NewGadget	ng;
	short				row, column;
	BOOL				gadgets_added = FALSE;
	WORD				last_qualifier = 0;

	main_task = FindTask( 0 );

	calc_pattern_ticks();
	calc_tempo();
	base_pattern.dpColumns = total_columns;
	base_pattern.dpRows = DRUM_ROWS;
	base_pattern.dpNumber = 0;

	NewList( (struct List *)&arrow_list );

		/* open libraries */

	unless (IntuitionBase = open_lib( "intuition.library" )) LEAVE;
	unless (GadToolsBase = open_lib( "gadtools.library" )) LEAVE;
	unless (AslBase = open_lib( "asl.library" )) LEAVE;
	unless (GfxBase = open_lib( "graphics.library" )) LEAVE;
	unless (UtilityBase = open_lib( "utility.library" )) LEAVE;
	unless (CamdBase = open_lib( "camd.library" )) LEAVE;
	unless (RealTimeBase = open_lib( "realtime.library" )) LEAVE;
	unless (IFFParseBase = OpenLibrary( "iffparse.library", 0L )) LEAVE;

	aclass = initArrowButtonClass();			/* arrow buttons                */

		/* create the MIDI stuff */

	unless (midi = CreateMidi(
		MIDI_Name, "VU Meters",
		MIDI_ErrFilter, CMEF_All,
		TAG_DONE ))
	{	ErrorF(	window, "Unable to create MIDI Node." );
		LEAVE;
	}

	unless (out_link = AddMidiLink ( midi, MLTYPE_Sender,
				MLINK_Location, "out.0",
				MLINK_Comment, "Drum Output",
				MLINK_Name, "Out",
				TAG_END ))
	{	ErrorF(	window, "Unable to create MIDI link." );
		LEAVE;
	}

	unless (drum_player = CreatePlayer(
				PLAYER_Name, APPNAME,
				PLAYER_Conductor, "Main",
				PLAYER_Hook, &myHook,
				PLAYER_Priority, 0,
				TAG_DONE ))
	{	ErrorF(	window, "Unable to create RealTime player." );
		LEAVE;
	}

		/* open the window */

	window = OpenWindowTags(
				NULL,
				WA_Width, 	640,
				WA_InnerHeight, 178,
				WA_IDCMP,	IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS |
							IDCMP_MOUSEMOVE | IDCMP_MENUPICK |
							IDCMP_GADGETUP | IDCMP_GADGETDOWN |
							IDCMP_RAWKEY | IDCMP_ACTIVEWINDOW |
							IDCMP_INACTIVEWINDOW | IDCMP_IDCMPUPDATE |
							SLIDERIDCMP | BUTTONIDCMP, /* help ? */
				WA_Flags,	WFLG_DRAGBAR | WFLG_DEPTHGADGET |
							WFLG_CLOSEGADGET | WFLG_REPORTMOUSE |
							WFLG_NOCAREREFRESH | WFLG_ACTIVATE |
							WFLG_SMART_REFRESH | WFLG_NEWLOOKMENUS,
				WA_Title,	"Drum Machine",
				WA_ScreenTitle,	"©1992 Sylvan Technical Arts" );

	unless (window)
	{	ErrorF(	window, "Unable to open window." );
		LEAVE;
	}

	unless (vi = GetVisualInfo( window->WScreen, TAG_DONE))
	{	ErrorF(	window, "Unable to get screen information." );
		LEAVE;
	}

	unless (dri = GetScreenDrawInfo( window->WScreen ))
	{	ErrorF(	window, "Unable to get screen information." );
		LEAVE;
	}

	rp = window->RPort;
	dri_pens = dri->dri_Pens;

		/* create off-screen bitmap */

	unless ( off_bm = AllocBitMap( 32, window->Height, rp->BitMap->Depth, 0, NULL ))
	{	ErrorF(	window, "Unable to create off-screen bitmap." );
		LEAVE;
	}

	InitRastPort( &off_rp );
	off_rp.BitMap = off_bm;
	off_shine_rp = off_rp; SetAPen( &off_shine_rp, dri_pens[ SHINEPEN ] );
	off_shade_rp = off_rp; SetAPen( &off_shade_rp, dri_pens[ SHADOWPEN ] );

		/* now, create the gadgets */

	unless (last_gadget = context_gadget = CreateContext( &first_gadget ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

    ng.ng_VisualInfo = vi;
    ng.ng_TextAttr = &topaz8;

	for (row = 0; row < DRUM_ROWS; row++)
	{
			/* initialize the drum kit while we're at it */

		current_pattern->dpSetup[ row ].name[0] = 0;
		current_pattern->dpSetup[ row ].key = 40 + row;
		current_pattern->dpSetup[ row ].flags = 0;

			/* general parameters for each row */

		ng.ng_TopEdge = window->BorderTop + 12 + row * ROW_HEIGHT;
		ng.ng_Height = ROW_HEIGHT - 1;
		ng.ng_UserData = (APTR)row;				/* user data is row #			*/

			/* Mute Button */

		ng.ng_LeftEdge = 7;
		ng.ng_Width = 16;
	    ng.ng_GadgetText = "M";
		ng.ng_Flags = PLACETEXT_IN;
		ng.ng_GadgetID = ID_MUTE;
		unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng,
				GA_Immediate, TRUE,
				TAG_DONE ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}

			/* solo button */

		ng.ng_LeftEdge = 24;
	    ng.ng_GadgetText = "S";
		ng.ng_GadgetID = ID_SOLO;
		unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng,
				GA_Immediate, TRUE,
				TAG_DONE ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}

			/* preview button */

		ng.ng_LeftEdge = 198;
	    ng.ng_GadgetText = "*";
		ng.ng_GadgetID = ID_HIT;
		unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng,
				GA_Immediate, TRUE,
				TAG_DONE ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}

			/* drum name */

		ng.ng_LeftEdge = 42;
		ng.ng_Width = 90;
	    if (row == 0) ng.ng_GadgetText = "Name"; else ng.ng_GadgetText = NULL;
		ng.ng_Flags = PLACETEXT_ABOVE;
		ng.ng_GadgetID = ID_PNAME;
		unless (last_gadget = CreateGadget( STRING_KIND, last_gadget, &ng,
				GTST_MaxChars, 24,
				TAG_DONE ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}
		name_gadgets[ row ] = last_gadget;

			/* key number */

		ng.ng_LeftEdge = 135;
		ng.ng_Width = 38;
	    if (row == 0) ng.ng_GadgetText = "   Note #";
		ng.ng_Flags = PLACETEXT_ABOVE;
		ng.ng_GadgetID = ID_KEY;
		unless (last_gadget = CreateGadget( INTEGER_KIND, last_gadget, &ng,
				GTIN_MaxChars, 3,
				GTIN_Number, current_pattern->dpSetup[ row ].key,
				TAG_DONE ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}
		key_gadgets[ row ] = last_gadget;

			/* key number increment */

		ng.ng_LeftEdge = 173;
		ng.ng_Width = 22;
	    ng.ng_GadgetText = NULL;
		ng.ng_GadgetID = ID_KEYINC + (row << 8);

		unless (arrow_gadget = make_arrow( &ng, arrow_gadget, 1, 127, current_pattern->dpSetup[ row ].key ))
		{	ErrorF(	window, "Unable to create gadgets." );
			LEAVE;
		}
		keyinc_gadgets[ row ] = arrow_gadget;

		if (first_arrow == NULL) first_arrow = arrow_gadget;
    }

		/* general parameters for bottom row */

	ng.ng_TopEdge = window->BorderTop + 22 + DRUM_ROWS * ROW_HEIGHT;
	ng.ng_Height = 14;
	ng.ng_UserData = NULL;

#if 0
		/* File Name indicator */

	ng.ng_LeftEdge = 8;
	ng.ng_Width = 147;
    ng.ng_GadgetText = NULL;
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_GadgetID = ID_FILE;
	unless (last_gadget = CreateGadget( TEXT_KIND, last_gadget, &ng,
			GTTX_Text,		"« No File »",
			GTTX_Border,	TRUE,
			GTTX_Clipped,	TRUE,
			TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}
#endif

		/* Play button */

	ng.ng_LeftEdge = 7 /* 162 */;
	ng.ng_Width = 31;
    ng.ng_GadgetText = ">";
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_GadgetID = ID_PLAY;
	unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng, TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

		/* Stop button */

	ng.ng_LeftEdge = 38 /* 193 */;
	ng.ng_Width = 31;
    ng.ng_GadgetText = "[]";
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_GadgetID = ID_STOP;
	unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng, TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

		/* Step Fwd button */

	ng.ng_LeftEdge = 73 /* 228 */;
	ng.ng_Width = 22;
    ng.ng_GadgetText = "«";
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_GadgetID = ID_STEPBACK;
	unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng,
		GA_Immediate, TRUE,
		TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

		/* Stop button */

	ng.ng_LeftEdge = 95 /* 250 */;
	ng.ng_Width = 22;
    ng.ng_GadgetText = "»";
	ng.ng_Flags = PLACETEXT_IN;
	ng.ng_GadgetID = ID_STEPFWD;
	unless (last_gadget = CreateGadget( BUTTON_KIND, last_gadget, &ng,
		GA_Immediate, TRUE,
		TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

		/* Tempo Slider */

	ng.ng_LeftEdge = 197;
	ng.ng_Width = 95;
    ng.ng_GadgetText = "Tempo:   ";
	ng.ng_Flags = PLACETEXT_LEFT;
	ng.ng_GadgetID = ID_TEMPO;
	unless (last_gadget = CreateGadget( SLIDER_KIND, last_gadget, &ng,
				GTSL_Min,			10,
				GTSL_Max,			300,
				GTSL_Level,			current_tempo,
				GTSL_LevelFormat,	"%3.3ld",
				GTSL_MaxLevelLen,	3,
				GA_Immediate,		TRUE,
				GA_RelVerify,		TRUE,
				GA_FollowMouse, 	TRUE,
				TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

		/* Channel Control */

	ng.ng_LeftEdge = 367 /* 370 */;
	ng.ng_Width = 38 /* 64 */;
    ng.ng_GadgetText = "Channel:";
	ng.ng_Flags = PLACETEXT_LEFT;
	ng.ng_GadgetID = ID_CHANNEL;
	unless (last_gadget = CreateGadget( INTEGER_KIND, last_gadget, &ng,
			GTIN_MaxChars, 2,
			GTIN_Number, current_channel + 1,
			TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}
	channel_string = last_gadget;

		/* Channel increment */

	ng.ng_LeftEdge += ng.ng_Width;
	ng.ng_Width = 22;
    ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = ID_CHANNELINC;

	unless (arrow_gadget = make_arrow( &ng, arrow_gadget, 0, 15, current_channel ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}
	channel_arrow = arrow_gadget;

		/* Velocity Slider */

	ng.ng_LeftEdge = 541 /* 545 */;
	ng.ng_Width = 88 /* 84 */;
    ng.ng_GadgetText = "Velocity:   ";
	ng.ng_Flags = PLACETEXT_LEFT;
	ng.ng_GadgetID = ID_VELOCITY;
	unless (last_gadget = CreateGadget( SLIDER_KIND, last_gadget, &ng,
				GTSL_Min,	1,
				GTSL_Max,	127,
				GTSL_Level,	current_velocity,
				GTSL_MaxLevelLen,	3,
				GTSL_LevelFormat,	"%3.3ld",
				GA_RelVerify,		TRUE,
				TAG_DONE ))
	{	ErrorF(	window, "Unable to create gadgets." );
		LEAVE;
	}

	AddGList( window, first_arrow, -1, -1, NULL );
	AddGList( window, first_gadget, -1, -1, NULL );

	gadgets_added = TRUE;
	GT_RefreshWindow( window, NULL );
	RefreshGList( first_arrow, window, NULL, -1 );

		/* render the non-gadget window contents */

	matrix_top = window->BorderTop + 2;
	matrix_bottom = window->BorderTop + 18 + row * ROW_HEIGHT;
	matrix_y_org = matrix_top + 16;
	matrix_height = matrix_bottom - matrix_top;

	column_width = clamp( 8, MAX_MATRIX_WIDTH / total_columns, 31 );
	matrix_width = column_width * total_columns;

	SetDrPt( rp, 0xaaaa );
	for (row = 0; row < DRUM_ROWS; row++)
	{	int				y = matrix_y_org + row * ROW_HEIGHT;

		SetAPen( rp, dri_pens[ SHADOWPEN ] );
		Move( rp, 214, y );
		Draw( rp, 635, y );

		SetAPen( rp, dri_pens[ SHINEPEN ] );
		Move( rp, 214, y + 1 );
		Draw( rp, 635, y + 1 );
	}
	SetDrPt( rp, -1 );

	for (column = 0; column <= total_columns; column++)
	{	render_column( column, TRUE );
	}
	draw_titlebar_scale( );

	SetAPen( rp, dri_pens[ SHADOWPEN ] );
	Move( rp, MATRIX_X, matrix_top - 1 );
	Draw( rp, MATRIX_X + total_columns * column_width, matrix_top - 1 );

	SetAPen( rp, dri_pens[ SHADOWPEN ] );
	Move( rp, window->BorderLeft, matrix_bottom );
	Draw( rp, 635, matrix_bottom );

	SetAPen( rp, dri_pens[ SHINEPEN ] );
	Move( rp, window->BorderLeft, matrix_bottom + 1 );
	Draw( rp, 635, matrix_bottom + 1 );

		/* Create menus */

	unless (menu_strip = CreateMenus( menu_list, GTMN_FrontPen, 0, TAG_END )) LEAVE;
	unless (LayoutMenus( menu_strip, vi, GTMN_NewLookMenus, TRUE, TAG_END ) ) LEAVE;
	SetMenuStrip( window, menu_strip );

		/*	Create ASL File requester */

	unless (filereq = AllocAslRequestTags( ASL_FileRequest,
			ASLFR_Window,		window,
			ASLFR_SleepWindow,	TRUE,
			ASLFR_RejectIcons,	TRUE,
			TAG_DONE ))
	{	ErrorF(	window, "Unable to create file requester." );
		LEAVE;
	}

	while (running)
	{	struct IntuiMessage	msg_copy;
		struct IntuiMessage *msg;
		struct Gadget		*g;
		struct TagItem		temp_tags[8],		/* hold tags after replymsg		*/
							*tag,
							*taglist;
		WORD				gadget_id,
							gadget_value;

		LONG signals;

		signals = Wait( (1 << window->UserPort->mp_SigBit)
						| SIGBREAKF_CTRL_F
						| SIGBREAKF_CTRL_E );

		if (signals & SIGBREAKF_CTRL_F)			/* clock signal					*/
		{
			column = playback_column;
			playback_column = column_pos;

			play_drum_note( &current_pattern->dpGrid[ playback_column ], TRUE );
			play_drum_note( &current_pattern->dpGrid[ playback_column ], FALSE );

			render_column( column, FALSE );
			render_column( playback_column, TRUE );
		}

		if (signals & SIGBREAKF_CTRL_E)			/* state change signal			*/
		{
			clock_state = drum_player->pl_Source->cdt_State;

			switch (clock_state) {

			case CONDSTATE_STOPPED:

				break;

	    	case CONDSTATE_PAUSED:

	    		break;

	    	case CONDSTATE_LOCATE:

				render_column( playback_column, FALSE );
				locate_to_tick ( drum_player->pl_Source->cdt_ClockTime );
	    		playback_column = column_pos;

				SetPlayerAttrs( drum_player, PLAYER_Ready, TRUE, TAG_END );
	    		break;

	    	case CONDSTATE_RUNNING:

				render_column( playback_column, FALSE );
				locate_to_tick ( drum_player->pl_Source->cdt_ClockTime );
	    		playback_column = column_pos;

				SetPlayerAttrs( drum_player, PLAYER_Ready, TRUE, TAG_END );

				Signal( main_task, SIGBREAKF_CTRL_F );
	    		break;
			}
		}

		while (msg = GT_GetIMsg( window->UserPort ))
		{	msg_copy = *msg;

			if (msg_copy.Class == IDCMPUPDATE)
				CopyMem( msg->IAddress, temp_tags, sizeof temp_tags );

			GT_ReplyIMsg( msg );

			switch ( msg_copy.Class ) {
			case IDCMP_CLOSEWINDOW: LEAVE;

			case IDCMP_ACTIVEWINDOW:
			case IDCMP_INACTIVEWINDOW:
				draw_titlebar_scale( );
				break;

			case IDCMP_MENUPICK:
				if (drag_state)
				{	shadow_drag( drag_column, drag_anchor_column );
					drag_state = 0;
				}

				while (msg_copy.Code != MENUNULL && msg_copy.IDCMPWindow)
				{	struct MenuItem	*menuitem;

					if (menuitem = ItemAddress( msg_copy.IDCMPWindow->MenuStrip,msg_copy.Code ))
					{	void (*userdata)( struct IntuiMessage * );

						if (userdata = GTMENUITEM_USERDATA(menuitem))
						{	(userdata)( &msg_copy );
						}
						msg_copy.Code = menuitem->NextSelect;
					}
					else break;
				}

				break;

			case IDCMP_IDCMPUPDATE:
				taglist = temp_tags;

				while (tag = NextTagItem( &taglist ))
				{
					switch (tag->ti_Tag) {
					case GA_ID: gadget_id = tag->ti_Data; break;
					}
				}

				switch ( gadget_id & 0xff ) {
				case ID_KEYINC:

					gadget_value = msg_copy.Code;
					row = gadget_id >> 8;
					current_pattern->dpSetup[ row ].key = gadget_value;
					GT_SetGadgetAttrs( key_gadgets[ row ], window, NULL,
							GTIN_Number, gadget_value,
							TAG_DONE );

					if (last_qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT))
						preview_row( row );

					break;

				case ID_CHANNELINC:
					gadget_value = msg_copy.Code;
					current_channel = clamp( 0, gadget_value, 15);
					GT_SetGadgetAttrs( channel_string, window, NULL,
							GTIN_Number, current_channel + 1,
							TAG_DONE );
					break;

				}
				break;

			case IDCMP_GADGETDOWN:
				g = (struct Gadget *)msg_copy.IAddress;
				row = (int)g->UserData;

				switch (g->GadgetID) {
				case ID_HIT:		preview_row( row ); break;
				case ID_STEPFWD:	step_play( 1 ); break;
				case ID_STEPBACK:	step_play( -1 ); break;
				case ID_TEMPO:		tempo_drag = TRUE; break;

				};
				break;

			case IDCMP_GADGETUP:
				g = (struct Gadget *)msg_copy.IAddress;
				row = (int)g->UserData;

				switch (g->GadgetID) {

				case ID_PLAY:
					SetConductorState( drum_player, CONDSTATE_LOCATE, 0 );
					break;

				case ID_STOP:
					SetConductorState( drum_player, CONDSTATE_STOPPED, 0 );
					break;

				case ID_KEY:
					current_pattern->dpSetup[ row ].key =
						((struct StringInfo *)g->SpecialInfo)->LongInt;

					SetGadgetAttrs( keyinc_gadgets[ row ], window, NULL,
						ARROW_Current, current_pattern->dpSetup[ row ].key,
						TAG_END );

					break;

				case ID_CHANNEL:

					current_channel =
						clamp(0, ((struct StringInfo *)g->SpecialInfo)->LongInt - 1, 15);

					GT_SetGadgetAttrs( key_gadgets[ row ], window, NULL,
							GTIN_Number, current_channel + 1,
							TAG_DONE );

					SetGadgetAttrs( channel_arrow, window, NULL,
						ARROW_Current, current_channel,
						TAG_END );

					break;

				case ID_PNAME:

					strncpy(current_pattern->dpSetup[ row ].name,
							((struct StringInfo *)g->SpecialInfo)->Buffer,
							sizeof current_pattern->dpSetup[ row ].name );
					break;

				case ID_TEMPO:
					current_tempo = msg_copy.Code;
					tempo_drag = FALSE;
					calc_tempo();
					break;

				case ID_VELOCITY:
					current_velocity = msg_copy.Code;
					break;

				};
				break;

			case IDCMP_MOUSEMOVE:

				if (tempo_drag)
				{	current_tempo = msg_copy.Code;
					calc_tempo();
				}
				else if (drag_state)
				{
					column = msg_copy.MouseX - MATRIX_X + column_width / 2;
					column /= column_width;

					if (column >= 0 && column < total_columns && column != drag_column )
					{	shadow_drag( drag_column, drag_anchor_column );
						drag_column = column;
						shadow_drag( drag_column, drag_anchor_column );
					}
				}
				break;

			case IDCMP_MOUSEBUTTONS:

				if (msg_copy.Code == SELECTDOWN)
				{
					row = msg_copy.MouseY - window->BorderTop - 12;
					column = msg_copy.MouseX - MATRIX_X + column_width / 2;

					if (row >= 0 && column >= 0)
					{	row /= ROW_HEIGHT;
						column /= column_width;

						if (column == total_columns) column = 0;

						if (row < DRUM_ROWS && column < total_columns)
						{	struct DrumColumn	*dc = &current_pattern->dpGrid[ column ];

							if (dc->note[ row ] > 0)
							{	dc->note[ row ] = 0;
								drag_state = DRAG_RESET;
							}
							else
							{	dc->note[ row ] = current_velocity;
								drag_state = DRAG_SET;
							}

							drag_column = drag_anchor_column = column;
							drag_row = row;

							render_column( column, TRUE );
							if (column == 0) render_column( total_columns, FALSE );

							if (dc->note[ row ] > 0 &&
								(msg_copy.Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)))
									preview_row( row );
						}
					}
				}
				else if (msg_copy.Code == SELECTUP)
				{
					if (drag_state && drag_column != drag_anchor_column)
					{	int	i;

						shadow_drag( drag_column, drag_anchor_column );

						if (drag_column < drag_anchor_column)
						{	i = drag_column;
							drag_column = drag_anchor_column;
							drag_anchor_column = i;
						}

						for (i=drag_anchor_column; i<=drag_column; i++)
						{	struct DrumColumn	*dc;

							column = (i == total_columns) ? 0 : i;

							dc = &current_pattern->dpGrid[ column ];

							if (drag_state == DRAG_RESET) dc->note[ row ] = 0;
							else dc->note[ row ] = current_velocity;

							render_column( column, TRUE );
							if (column == 0) render_column( total_columns, FALSE );
						}
					}
					drag_state = 0;
				}
				break;

			case IDCMP_RAWKEY:

				if (drag_state)
				{	shadow_drag( drag_column, drag_anchor_column );
					drag_state = 0;
				}

				last_qualifier = msg_copy.Qualifier;

				switch (msg_copy.Code) {
				case 0x4e:						/* cursor forward				*/
					step_play( 1 );
					break;

				case 0x4f:						/* cursor backwards				*/
					step_play( -1 );
					break;

				case 1:							/* number keys are drums...		*/
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					preview_row( msg_copy.Code - 1 );
					/* Holding control key enters or deleted from score at current
						playback position, minus a bit... */
					break;
				};
			};
		}
	}
exitit:
	if (drum_player) DeletePlayer( drum_player );
	if (midi) DeleteMidi( midi );

	FreeBitMap( off_bm );

	if (filereq) FreeAslRequest( filereq );

	if (window) { ClearMenuStrip( window ); CloseWindow( window ); }
	if (menu_strip) FreeMenus( menu_strip );

/*	if (gadgets_added) RemoveGList( window, first_gadget, -1 ); */
	FreeGadgets( first_gadget );
	DisposeObjectList( (struct List *)&arrow_list );
	FreeScreenDrawInfo( window->WScreen, dri );
	FreeVisualInfo( vi );

	freeArrowButtonClass(aclass);				/* delete boopsi classes        */

	CloseLibrary( IFFParseBase );
	CloseLibrary( RealTimeBase );
	CloseLibrary( CamdBase );
	CloseLibrary( GfxBase );
	CloseLibrary( UtilityBase );
	CloseLibrary( AslBase );
	CloseLibrary( GadToolsBase );
	CloseLibrary( IntuitionBase );
}

/* ============================================================================ *
                                   Editing Routines
 * ============================================================================ */

	/* add an object to a list */

ULONG AddTailObject( Object *o, struct List *list)
{	return DoMethod( o, OM_ADDTAIL, list );
}

void DisposeObjectList( struct List *l )
{	APTR				*obj_state = (APTR)l->lh_Head,
						*obj;

    while ( obj = NextObject( &obj_state ) )	/* iterate through list			*/
    {	DoMethod( (Object *)obj, OM_REMOVE );	/* remove from list				*/
  		DisposeObject( obj );
	}
}

struct TagItem arrow_map[] = {
	ARROW_Current,	ICSPECIAL_CODE,
	TAG_END,
};

struct Gadget *make_arrow( struct NewGadget *ng, struct Gadget *prev, int min, int max, int current )
{	struct Gadget		*g;

	g = (struct Gadget *)NewObject(aclass,NULL,
			GA_Left,		ng->ng_LeftEdge,
			GA_Top,  		ng->ng_TopEdge,
			GA_Width,		ng->ng_Width,
			GA_Height,		ng->ng_Height,
			GA_DrawInfo,	dri,
			GA_ID,			ng->ng_GadgetID,
			GA_UserData,	ng->ng_UserData,

			ARROW_Min,		min,
			ARROW_Max,		max,
			ARROW_Current,	current,
			ARROW_IncreaseRate, 5,
			ARROW_MaxRate,	20,

			ICA_TARGET,		ICTARGET_IDCMP,
			ICA_MAP,		arrow_map,

			prev ? GA_Previous : TAG_IGNORE, prev,
			TAG_END );

	if (g) AddTailObject( (Object *)g, (struct List *)&arrow_list );

	return g;
}

void draw_titlebar_scale( void )
{	int					column;
	int					y = window->BorderTop - 2;

	for (column = 0; column <= total_columns; column+= 2)
	{	int				x = MATRIX_X + column * column_width;
		int				h;

		if ((column & 0x07) == 0) h = 7;
		else if ((column & 0x03) == 0) h = 3;
		else h = 1;

		SetAPen( rp, dri_pens[ SHADOWPEN ] );
		Move( rp, x, y - h );
		Draw( rp, x, y );

		SetAPen( rp, dri_pens[ SHINEPEN ] );
		Move( rp, x + 1, y - h );
		Draw( rp, x + 1, y );
	}
}

void render_column( int column, int highlight )
{	int					side_width = column_width / 2;
	int					x = MATRIX_X + column * column_width;
	int					row;
	struct DrumColumn	*dc = &current_pattern->dpGrid[ column >= total_columns ? 0 : column ];

	if (column != playback_column) highlight = FALSE;

	SetAPen( &off_rp, 0 );
	RectFill( &off_rp, 0, 0, column_width, matrix_height );
	SetAPen( &off_rp, dri_pens[ highlight ? HIGHLIGHTTEXTPEN : TEXTPEN ] );

	SetDrPt( &off_shine_rp, (x & 1) ? 0xaaaa : 0x5555 );
	SetDrPt( &off_shade_rp, (x & 1) ? 0xaaaa : 0x5555 );

		/* draw the individual rows */

	for (row = 0; row < DRUM_ROWS; row++)
	{	int				y = 16 + row * ROW_HEIGHT;

		Move( &off_shade_rp, 0, y );
		Draw( &off_shade_rp, column_width, y );

		Move( &off_shine_rp, 0, y + 1 );
		Draw( &off_shine_rp, column_width, y + 1 );
	}

	SetDrPt( &off_shine_rp, -1 );
	SetDrPt( &off_shade_rp, -1 );

	if (highlight)
	{	Move( &off_shine_rp, side_width - 1, 0 );
		Draw( &off_shine_rp, side_width - 1, matrix_height );
	}
	else
	{	Move( &off_shade_rp, side_width - 1, 0 );
		Draw( &off_shade_rp, side_width - 1, matrix_height );
	}
	Move( &off_shine_rp, side_width, 0 );
	Draw( &off_shine_rp, side_width, matrix_height );

	for (row = 0; row < DRUM_ROWS; row++)
	{	int				y = 16 + row * ROW_HEIGHT;
		int				hit,
						dotsize;

		hit = dc->note[ row ];
		if (hit > 0)
		{	if (hit > 90) dotsize = 4;
			else if (hit > 60) dotsize = 3;
			else if (hit > 30) dotsize = 2;
			else dotsize = 1;

			RectFill( &off_rp,	side_width - dotsize - 1,
								y - dotsize,
								side_width + dotsize - 1,
								y + dotsize );
		}
	}

	BltBitMapRastPort(	off_bm, 0, 0,
						rp, x - side_width + 1, matrix_top,
						side_width + side_width, matrix_height,
						0xC0 );
}

void render_all( void )
{	int					column;

	for (column = 0; column <= total_columns; column++)
	{	render_column( column, TRUE );
	}

	/* also render gadgets */
}

void set_kit_gadgets( void )
{	int					row;

	for (row = 0; row < DRUM_ROWS; row++)
	{
		GT_SetGadgetAttrs( key_gadgets[ row ], window, NULL,
				GTIN_Number, current_pattern->dpSetup[ row ].key,
				TAG_DONE );

		SetGadgetAttrs( keyinc_gadgets[ row ], window, NULL,
			ARROW_Current, current_pattern->dpSetup[ row ].key,
			TAG_END );

		GT_SetGadgetAttrs( name_gadgets[ row ], window, NULL,
				GTST_String, current_pattern->dpSetup[ row ].name,
				TAG_DONE );
	}
}

void preview_row( int row )
{	struct DrumColumn	preview_col;

	memset( &preview_col, 0, sizeof preview_col );
	preview_col.note[ row ] = current_velocity;

	play_drum_note( &preview_col, TRUE );
	play_drum_note( &preview_col, FALSE );
}

void play_drum_note( struct DrumColumn *dc, BOOL note_on )
{	int					row;
	MidiMsg				mm;

	mm.mm_Status = MS_NoteOn | current_channel;

	for (row = 0; row < DRUM_ROWS; row++)
	{	if (dc->note[ row ] > 0 && !(current_pattern->dpSetup[ row ].flags & DRUMF_MUTE))
		{
			mm.mm_Data1 = current_pattern->dpSetup[ row ].key;
			mm.mm_Data2 = note_on ? dc->note[ row ] : 0;

			PutMidiMsg ( out_link, &mm );
		}
	}
}

void step_play( int dir )
{	int column = playback_column;

	if (dir > 0)
	{	playback_column++;
		if (playback_column >= total_columns) playback_column = 0;
	}
	else
	{	playback_column--;
		if (playback_column < 0) playback_column = total_columns - 1;
	}

	play_drum_note( &current_pattern->dpGrid[ playback_column ], TRUE );
	play_drum_note( &current_pattern->dpGrid[ playback_column ], FALSE );

	render_column( column, FALSE );
	render_column( playback_column, TRUE );
}

void shadow_drag( int col1, int col2 )
{	int					x1 = MATRIX_X + col1 * column_width,
						x2 = MATRIX_X + col2 * column_width;
	int					y = matrix_y_org + drag_row * ROW_HEIGHT;

	if (drag_column != drag_anchor_column)
	{	SetDrMd( rp, COMPLEMENT );
		Move( rp, x1, y );
		Draw( rp, x2, y );
		SetDrMd( rp, JAM2 );
	}
}

/* ============================================================================ *
                                 Save / Load routines
 * ============================================================================ */

	/* REM: Is the list of sounds local or global?
		Samples should be global,
		sounds should be local...
	*/

	/*	Print error message from IFFParse */

void IFFError(char *filename, int writing, int error)
{	char				*msg;

	switch ( error ) {
	case -100:					msg = "Can't open file."; break;
	case IFFERR_EOF:			return;
	case IFFERR_EOC:			msg = "File is Incomplete";
	case IFFERR_NOSCOPE:		return;

	case -101:
	case IFFERR_NOMEM:			msg = "Out of Memory"; break;

	case IFFERR_READ:			msg = "Read Error"; break;
	case IFFERR_WRITE:			msg = "Write Error"; break;
	case IFFERR_SEEK:	  		msg = "Read Error"; break;
	case IFFERR_MANGLED:		msg = "File is Corrupt"; break;
	case IFFERR_SYNTAX:			msg = "File is Incomplete"; break;
	case IFFERR_NOTIFF:			msg = "Not an IFF File"; break;

	case IFFERR_NOHOOK:
	case IFF_RETURN2CLIENT:
	default:					msg = "Internal Error"; break;
	}

	if (filename)
	{	ErrorF(
			window,
			writing ? "Error Writing File '%s':\n%s." : "Error Reading File '%s':\n%s.",
			filename,
			msg );
	}
	else
	{	ErrorF(
			window,
			writing ? "Error Writing to Clipboard:\n%s." : "Error Reading from Clipboard:\n%s",
			msg );
	}

}

WORD read_kit( struct IFFHandle *iff, struct DrumPattern *dp )
{	char				*tbuf;
	struct ContextNode	*cn;
	WORD				error = FALSE;

	unless (cn = CurrentChunk (iff)) return -102;		/* internal error */
	if (tbuf = AllocMem( cn->cn_Size, 0L ))
	{	if ((error = ReadChunkBytes( iff, tbuf, cn->cn_Size )) >= 0)
		{
			CopyMem( tbuf, &ReadKit, MIN( cn->cn_Size, sizeof ReadKit ) );
			CopyMem( &ReadKit, dp->dpSetup, sizeof dp->dpSetup ); /* temporary */
			error = 0;
		}
		FreeMem( tbuf, cn->cn_Size );
	}
	return error;
}

WORD read_pattern( struct IFFHandle *iff, struct DrumPattern *dp )
{	char				*tbuf;
	struct ContextNode	*cn;
	WORD				error = FALSE;

	unless (cn = CurrentChunk (iff)) return -102;		/* internal error */
	if (tbuf = AllocMem( cn->cn_Size, 0L ))
	{	if ((error = ReadChunkBytes( iff, tbuf, cn->cn_Size )) >= 0)
		{	int			i,j;
			struct DrumPattern	*pat;
			char		*array;
			WORD		array_bytes;

			pat = (struct DrumPattern *)tbuf;
		/*	dp->dpNumber = pat->dp_Number;	*/	/* wrong? */
			dp->dpColumns = MIN( pat->dpColumns, MAX_COLUMNS );
			dp->dpRows = DRUM_ROWS;

			array = tbuf + offsetof (struct DrumPattern, dpSetup);
			array_bytes = cn->cn_Size - offsetof( struct DrumPattern, dpSetup );

			memset( &dp->dpGrid, 0, sizeof dp->dpGrid );

			for ( i=0; i<dp->dpColumns; i++)
			{
				for (j=0; j<dp->dpRows; j++)
				{	WORD	val = 0;

					if (i < pat->dpColumns && j < pat->dpRows)
					{	val = array[j];
					}
					dp->dpGrid[i].note[j] = val;
				}
				array += dp->dpRows;
			}
			error = 0;
		}
		FreeMem( tbuf, cn->cn_Size );
	}
	return error;
}

WORD write_kit( struct IFFHandle *iff, struct DrumPattern *dp )
{	int					size = sizeof dp->dpSetup;
	WORD				error;

	if (error = PushChunk( iff, 0L, 'DKIT', size )) return error;
	if ((error = WriteChunkBytes (iff, (APTR) &dp->dpSetup, size)) != size) return error;
	if (error = PopChunk (iff)) return error;
	return 0;
}

WORD write_pattern( struct IFFHandle *iff, struct DrumPattern *dp )
{	int					size1 = offsetof( struct DrumPattern, dpSetup),
						size2 = sizeof (struct DrumColumn) * dp->dpColumns;
	WORD				error;

	if (error = PushChunk( iff, 0L, 'PTRN', size1 + size2 )) return error;
	if ((error = WriteChunkBytes( iff, (APTR) dp, size1 )) < 0) return error;
	if ((error = WriteChunkBytes( iff, (APTR) &dp->dpGrid, size2 )) < 0) return error;
	if (error = PopChunk (iff)) return error;
	return 0;
}

#if 0
BOOL read_song( void )
{	;
}

BOOL write_song( void )
{	;
}
#endif

static LONG			cmus_chunks[] = {
	'DRUM', 'DKIT',
	'DRUM', 'PTRN',
};

BOOL load_file( BOOL all )
{	struct IFFHandle	*iff = NULL;
	BOOL				error = 0;
	BPTR				fh = NULL,
						olddir,
						dlock = NULL;

		/*	Bring up the file requester */

	if (!AslRequestTags( (APTR)filereq,
					ASLFR_TitleText,	all ? "Load Song" : "Load Pattern",
					ASLFR_PositiveText,	"Load",
					TAG_DONE ))
	{	return 0;
    }

		/*	Lock the directory they typed */

    unless (dlock = Lock( filereq->rf_Dir, ACCESS_READ ))
	{	ErrorF( window, "Can't find directory: '%s'.", filereq->rf_Dir );
	}

		/*	CD to that directory and open the file */

	olddir = CurrentDir( dlock );
	fh = Open( filereq->rf_File, MODE_OLDFILE );
	CurrentDir( olddir );

		/*	Init IFF Stream */

	unless (fh) { error = -100; LEAVE; }
	unless (iff = AllocIFF()) { error = -101; LEAVE; }
	iff->iff_Stream = fh;
	InitIFFasDOS (iff);

		/*	Read the pattern */

	if (error = OpenIFF (iff, IFFF_WRITE)) LEAVE;
	if (error = StopChunks (iff, cmus_chunks, elementsof (cmus_chunks) / 2)) LEAVE;

	for (;;)
	{	struct ContextNode	*cn;

		if (error = ParseIFF (iff, IFFPARSE_SCAN))
		{	if (error == IFFERR_EOF) break;
			LEAVE;
		}

		unless (cn = CurrentChunk (iff))	/* shouldn't happen				*/
		{	error = IFFERR_EOC;
			LEAVE;
		}

		switch (cn->cn_ID) {
		case 'DKIT':
			if (error = read_kit( iff, current_pattern )) LEAVE;
			break;

		case 'PTRN':
			if (error = read_pattern( iff, current_pattern )) LEAVE;
			break;
		}
	}

exitit:

		/*	Clean up */

	if (iff)
	{	CloseIFF (iff);
		FreeIFF (iff);
	}
	if (fh) Close( fh );
	if (dlock) UnLock( dlock );

		/*	Report Error */

	if (error)
	{	IFFError( filereq->rf_File, FALSE, error);
	}

	render_all();
	set_kit_gadgets();

	return error;
}

BOOL save_file( BOOL all )
{	struct IFFHandle	*iff = NULL;
	BOOL				error = 0;
	BPTR				fh = NULL,
						olddir,
						dlock = NULL;

		/*	Bring up the file requester */

	if (!AslRequestTags( (APTR)filereq,
					ASLFR_TitleText,	all ? "Save Song" : "Save Pattern",
					ASLFR_PositiveText,	"Save",
					ASLFR_DoSaveMode,	TRUE,
					TAG_DONE ))
	{	return 0;
    }

		/*	Lock the directory they typed */

    unless (dlock = Lock( filereq->rf_Dir, ACCESS_READ ))
	{	ErrorF( window, "Can't find directory: '%s'.", filereq->rf_Dir );
	}

		/*	CD to that directory and open the file */

	olddir = CurrentDir( dlock );
	fh = Open( filereq->rf_File, MODE_NEWFILE );
	CurrentDir( olddir );

		/*	Init IFF Stream */

	unless (fh) { error = -100; LEAVE; }
	unless (iff = AllocIFF()) { error = -101; LEAVE; }
	iff->iff_Stream = fh;
	InitIFFasDOS (iff);

		/*	Write the pattern */

	if (error = OpenIFF (iff, IFFF_WRITE)) LEAVE;
	if (error = PushChunk (iff, 'DRUM', 'FORM', IFFSIZE_UNKNOWN)) LEAVE;
	if (error = write_kit( iff, current_pattern )) LEAVE;
	if (error = write_pattern( iff, current_pattern )) LEAVE;
	if (error = PopChunk (iff)) LEAVE;

exitit:

		/*	Clean up */

	if (iff)
	{	CloseIFF (iff);
		FreeIFF (iff);
	}
	if (fh) Close( fh );
	if (dlock) UnLock( dlock );

		/*	Report Error */

	if (error)
	{	IFFError( filereq->rf_File, TRUE, error);
	}
	return error;
}

/* ============================================================================ *
                                    Menu Functions
 * ============================================================================ */

void do_loadpattern( struct IntuiMessage *imsg )
{
	load_file( FALSE );
}

void do_savepattern( struct IntuiMessage *imsg )
{
	save_file( FALSE );
}

void do_about( struct IntuiMessage *imsg )
{	struct EasyStruct	es;
	APTR				dummyarg;

	es.es_StructSize = sizeof(struct EasyStruct);
	es.es_Flags = 0;
	es.es_Title = APPNAME " Program Information";
	es.es_TextFormat = "Drum Machine CAMD/RealTime Example\nBy Talin\n©1992 Sylvan Technical Arts";
	es.es_GadgetFormat = "Continue";

	EasyRequestArgs( window, &es, NULL, (APTR)(&dummyarg) );
}

void do_quit( struct IntuiMessage *imsg )
{
	running = FALSE;
	imsg->IDCMPWindow = NULL;				/* end NextSelect chain				*/
}

void do_clear( struct IntuiMessage *imsg )
{
	memset( &current_pattern->dpGrid, 0, sizeof current_pattern->dpGrid );
	render_all();
}

/* ============================================================================ *
                                  Playback routines
 * ============================================================================ */

	/*	This calculates how big, in clock terms, the columns and the overall
		pattern is.
	*/

void calc_pattern_ticks( void )
{
		/* calculate number of metric clocks per column */

	mclocks_per_column = ((MCLOCKS_PER_QNOTE * 4) >> drum_resolution);

		/* calculate the number of metric clocks per pattern */

	pattern_mclocks = total_columns * mclocks_per_column;
}

	/*	Calculate the tempo multiplication factor. The tempo is specified
		in quarter notes per minute; We need to convert that to metric
		clocks per real clock, plus a scaling factor of 256 (for 8-bit fixed
		point accuracy).
	*/

void calc_tempo( void )
{	tempo_rate = current_tempo * (MCLOCKS_PER_QNOTE << 8) / (TICK_FREQ * 60);
}

	/*	Locate to a particular metric time (in this case "locating" is fairly
		trivial, since all patterns are required to be the same length.
	*/

void locate_to_mclock( long mclocks )
{	WORD				column;

		/*	Unlike a normal division, we want numbers less than zero to
			round down, not up. So a seperate case is required for negative
			clock times.
		*/

	mclock_accumulator = mclocks << 8;

	clock_pos = (mclock_accumulator / tempo_rate);

	if (mclocks >= 0)
	{	pattern_pos = mclocks / pattern_mclocks;
	}
	else
	{	pattern_pos = (mclocks - pattern_mclocks + 1) / pattern_mclocks;
	}

		/*	Calculate the start and end of the current pattern in metric ticks */

	current_pattern_start = pattern_pos * pattern_mclocks;
	current_pattern_end   = current_pattern_start + pattern_mclocks;

		/*	Now, figure out which column we are on */

	column = (mclocks - current_pattern_start) / mclocks_per_column;

/*	if (REVERSE) column_pos = total_columns - (column + 1) */

	column_pos = column;
}

	/*	Locate to a particular real time. Since we assume that tempo is
		constant (no tempo changes) this is also trivial -- but needs to be
		done in extended precision if we are going to allow long songs
		without overflow.
	*/

void locate_to_tick ( long tick_count )
{
		/* REM: do extended multiply in assy... */

	locate_to_mclock( (tick_count * tempo_rate) >> 8 );
}

#if 0
	/*	Play to a particular clock time. This is different than the above
		routine in that it assumes that the current clock position is
		less than but near to the new clock time, and thus calculates
		using incremental addition rather than a full divide. If we were
		doing a full-on sequencer, we would exploit this philosophy even
		more fully.

		Also, it signals the tasks if the column position changed.
		Thus, the task only needs to wake up when a drum beat needs to
		actually be played.
	*/

void play_to_mclock( long new_mclocks )
{	WORD				column;

		/*	See if the clock counted past the end of the frame */

	while (new_mclocks >= current_pattern_end)
	{	pattern_pos++;
		current_pattern_start = current_pattern_end;
		current_pattern_end   = current_pattern_start + pattern_mclocks;
		/* mark for signal...? */
	}

	column = (clock_count - current_pattern_start) / mclocks_per_column;

/*	if (REVERSE) column = total_columns - (column + 1) */

	if (column != column_pos)
	{	column_pos = column;
	}
}
#endif
