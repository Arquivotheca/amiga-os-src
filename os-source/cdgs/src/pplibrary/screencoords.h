/* $Id: screencoords.h,v 1.4 93/04/07 16:51:50 peter Exp $
 *
 * Definitions of coordinates of buttons within the picture files.
 */


/* WARNING:  Due to late changes to the scrdata.pic screen, the contents
 * are not centered.  It was too late in development to recenter the
 * screen and change all the coordinates, not all of which have been
 * moved into this file.  Therefore, display.c knows a hard-coded offset
 * to account for the 32 pixels of blank space on the left.
 *
 * This note is here because if ever anyone fixes scrdata.pic, they'll
 * certainly have to modify this file.
 */
#define SCRDATA_LEFTOFFSET	32

/* Here are the coordinates of the cells that hold the button faces,
 * measured on scrdata.pic.  Note that for the right and bottom edges,
 * we store the coordinate plus one.
 */

#define CELL_REV	151,126,181,153
#define CELL_PLAY	187,126,235,153
#define CELL_FF		241,126,271,153
#define CELL_STOP	277,126,307,153

#define CELL_GOCDG	44,168,93,192
#define CELL_GOCDG_LEFT		44
#define CELL_GOCDG_TOP		168
#define CELL_GOCDG_WIDTH	49
#define CELL_GOCDG_HEIGHT	24
#define GOCDGSTATES CELL_GOCDG_HEIGHT
#define CELL_TIME	97,168,146,192
#define CELL_INTRO	151,164,199,191
#define CELL_SHUFF	205,164,253,191
#define CELL_REPEAT	259,164,307,191

#define CELL_MIN	63,131,92,149
#define CELL_SEC	103,131,132,149
#define CELL_NEG	52,139,60,142
#define CELL_CDG	56,152,133,159
#define CELL_COLON	97,134,99,147

/* Here are the upper-left coordinates of all the button states and
 * other things from scrbuttons.pic:
 */
#define BTN_REV_OFF		0,28
#define BTN_REV_ON		30,28

#define BTN_PLAY_OFF		0,55
#define BTN_PLAY_PAUSE		48,55
#define BTN_PLAY_PLAY		96,55

#define BTN_FF_OFF		60,28
#define BTN_FF_ON		90,28

#define BTN_STOP_OFF		0,128
#define BTN_STOP_ON		30,128

#define BTN_TIME_TRACK		156,52
#define BTN_TIME_TRACKTOGO	205,52
#define BTN_TIME_DISC		254,52
#define BTN_TIME_DISCTOGO	303,52

#define BTN_INTRO_OFF		0,182
#define BTN_INTRO_1		48,182
#define BTN_INTRO_2		96,182
#define BTN_INTRO_3		144,182
#define BTN_INTRO_4		192,182
#define BTN_INTRO_ON		240,182

#define BTN_SHUFF_OFF		0,101
#define BTN_SHUFF_ON		48,101

#define BTN_REPEAT_OFF		0,155
#define BTN_REPEAT_ON		48,155

#define BTN_NEG_OFF		154,9
#define BTN_NEG_ON		162,9

#define BTN_CDG_OFF		0,82
#define BTN_CDG_GFX		77,82
#define BTN_CDG_MIDI		0,89
#define BTN_CDG_BOTH		77,89

#define BTN_COLON_ON		154,12
#define BTN_COLON_OFF		156,12

#define BTN_GOCDG_OFF		190,158
#define BTN_GOCDG_ON		239,158

#define NUM_TRACK_XY		176,29
#define NUM_TRACK_DIMS		16,23

#define NUM_TIME_XY		0,9
#define NUM_TIME_DIMS		14,19

#define NUM_GRID_XY		0,0
#define NUM_GRID_DIMS		8,9
