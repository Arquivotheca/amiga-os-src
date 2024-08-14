/**********************************************\
* 															  *
*  Control Character Jump Table and Routines   *
* 															  *
\**********************************************/

#include "exec/types.h"
#include "devices/conunit.h"
#include "st.h"
#include "term.h"
#include "charset.h"
#include "window.h"
#include "ascii.h"
#include "menu.h"

#define PASS *strptr++ = c; /* pass ctrl-char on to console device */

extern struct IOStdReq consoleIO;
extern struct TextFont *tf_gfx, *tf_spcgfx, *tf_vt100_80;
extern struct TextFont *tf_gfx_128, *tf_spcgfx_128, *tf_vt100_128;
extern struct Window *w;
extern int do_tab(), ignore();
extern int read_cursor_posn(), set_cursor_posn(), set_window();
extern unsigned char c, *strptr, *inputptr, *escptr, escape_buffer[];
extern ULONG FontBase;
extern UBYTE escape_flag, escape_length, csi_flag;
extern UBYTE g0, g1, gd, char_set, numcolumns;
extern UBYTE cursor_status, row, Pt, Pb, beep;
extern WORD column;

UBYTE cr_expand_in = FALSE, lf_expand_in = FALSE;

uk_set() /* switch to uk character set */
{
	us_set(); /* same as uk */
	if (char_set==CS_US) char_set = CS_UK; /* flag that its the active char set */
}

us_set() /* switch to us character set */
{
	if (numcolumns==MINCOLUMNS) { /* if selected */
		*strptr++ = '\017'; /* select lower half char set */
		flush_string(); /* insure that the above goes into effect */
		((struct ConUnit *)consoleIO.io_Unit)->cu_Font = (struct TextFont *)FontBase;
		char_set = CS_US; /* flag that its the active char set */
	}
	else if (numcolumns==MAXCOLUMNS) {
		if (tf_vt100_128) { /* if open */
			*strptr++ = '\017'; /* select lower half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_vt100_128;
			char_set = CS_US; /* flag that its the active char set */
		}
		else {
			NoFont(1); /* 132 */
		}
	}
}

spc_gfx() /* switch to special graphics and line drawing */
{
	if (numcolumns==MINCOLUMNS) {
		if (tf_gfx) { /* if the font is open */
			*strptr++ = '\017'; /* select lower half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_gfx; /* set gfx */
			char_set = CS_SPC_GFX; /* flag that its the active char set */
		}
		else {
			NoFont(4); /* gfx */
		}
	}
	else if (numcolumns==MAXCOLUMNS) {
		if (tf_gfx_128) { /* if the font is open */
			*strptr++ = '\017'; /* select lower half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_gfx_128; /* set gfx */
			char_set = CS_SPC_GFX; /* flag that its the active char set */
		}
		else {
			NoFont(5); /* gfx 132 */
		}
	}
}

alt_rom() /* switch to alternate char rom */
{
	if (numcolumns==MINCOLUMNS) {
		if (tf_vt100_80) { /* if the font is open */
			*strptr++ = '\016'; /* select upper half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_vt100_80;
			char_set = CS_ALT_ROM; /* flag that its the active char set */
		}
		else {
			NoFont(2); /* alt */
		}
	}
	else if (numcolumns==MAXCOLUMNS) {
		if (tf_vt100_128) { /* if the font is open */
			*strptr++ = '\016'; /* select upper half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_vt100_128;
			char_set = CS_ALT_ROM; /* flag that its the active char set */
		}
		else {
			NoFont(3); /* alt 132 */
		}
	}
}

alt_rom_spc_gfx() /* switch to alternate character rom, special graphics */
{
	if (numcolumns==MINCOLUMNS) {
		if (tf_spcgfx) { /* if the font is open */
			*strptr++ = '\017'; /* select lower half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_spcgfx; /* set spc */
			char_set = CS_ALT_ROM_SPC_GFX; /* flag that its the active char set */
		}
		else {
			NoFont(6); /* spcgfx */
		}
	}
	else if (numcolumns==MAXCOLUMNS) {
		if (tf_spcgfx_128) { /* if the font is open */
			*strptr++ = '\017'; /* select lower half char set */
			flush_string(); /* insure that the above goes into effect */
			((struct ConUnit *)consoleIO.io_Unit)->cu_Font = tf_spcgfx_128; /* set spc */
			char_set = CS_ALT_ROM_SPC_GFX; /* flag that its the active char set */
		}
		else {
			NoFont(7); /* spcgfx 132 */
		}
	}
}

/* array of ptrs to character set designators */
int (*g_designators[])() = {
	uk_set, us_set,
	spc_gfx, alt_rom, alt_rom_spc_gfx,
	us_set
	};

nul() /* @ null */
{
/* ignored, not stored in input buffer */
}

soh() /* A */
{
	PASS /* pass through */
}

stx() /* B */
{
	PASS /* pass through */
}

etx() /* C */
{
	PASS /* pass through */
}

eot() /* D */
{
	PASS /* pass through */
}

enq() /* E enquiry */
{
	PASS /* pass through */
}

ack() /* F acknowledge */
{
	PASS /* pass through */
}

bell() /* G sound bell */
{
	if (beep!=NOFLASH) *strptr++ = BELL; /* flash screen */
	VTBeep(beep!=SILENT); /* ring bell (maybe) */
}

bs() /* H backspace */
{
	PASS /* console device does this */
}

ht() /* I horizontal tab */
{
	do_tab(); /* advance to next tab position or right margin if none */
}

lf() /* J linefeed (cursor down with scroll) */
{
	if (cursor_status==ABOVE_WINDOW) { /* if currently above */
		read_cursor_posn(); /* get relative row */
		if (++row >= Pt) { /* if entering window from the top... */
			set_window(Pt,Pb); /* re-establish the window */
			cursor_status = INSIDE_WINDOW; /* set new status */
			set_cursor_posn(1,column); /* posn to proper column */
		}
		else goto lf_local;
	}
	else {
lf_local:
		/*
			The console device expands a linefeed to
			carriage-return / linefeed.  So I do a cursor
			down here which is equivalent to a linefeed.
		*/
		*strptr++ = ESCAPE;
		*strptr++ = 'D'; /* move cursor down one line */
		if (lf_expand_in) { /* if expand incomming lf to lf/cr */
			c = RETURN;
			PASS;
		}
	}
}

vt() /* K vertical tab */
{
	c = LINEFEED; /* interpret as linefeed */
	lf();
}

ff() /* L formfeed */
{
	c = LINEFEED; /* interpret as linefeed */
	lf();
}

cr() /* M carriage return */
{
	PASS /* console device does this */
	if (cr_expand_in) { /* if expand incomming cr to cr/lf */
		c = LINEFEED;
		lf();
	}
}

so() /* N shift out (change to G1 designator) */
{
	gd = 1; /* g1 is active */
	(*g_designators[g1])();
}

si() /* O shift in (change to G0 designator) */
{
	gd = 0; /* g0 is active */
	(*g_designators[g0])();
}

dle() /* P */
{
	PASS /* pass through */
}

xon() /* Q (or dc1) causes terminal to resume transmission */
{
	PASS /* console device does this */
}

dc2() /* R */
{
	PASS /* pass through */
}

xoff() /* S (or dc3) causes terminal to stop transmission */
{
	PASS /* console device does this */
}

dc4() /* T */
{
	PASS /* pass through */
}

nak() /* U */
{
	PASS /* pass through */
}

syn() /* V */
{
	PASS /* pass through */
}

etb() /* W */
{
	PASS /* pass through */
}

can() /* X cancel escape sequence and display error character */
{
	escape_flag = csi_flag = FALSE; /* cancel escape sequence */
	c = DELETE; /* delete is the error character */
	PASS /* pass through */
}

em() /* Y */
{
	PASS /* pass through */
}

sub() /* Z interpreted as can */
{
	can();
}

esc() /* [ the escape character himself! */
{
	escape_flag = TRUE;
	escape_length = 0;
	escptr = escape_buffer;
	csi_flag = FALSE;
}

fs() /* \ */
{
	PASS /* pass through */
}

gs() /* ] */
{
	PASS /* pass through */
}

rs() /* ~ */
{
	PASS /* pass through */
}

us() /* ? */
{
	PASS /* pass through */
}

/* array of ptrs to ctrl-char functions */
int (*ctrl_functions[])() = {
	nul, soh, stx, etx, 	/* @ A B C */
	eot, enq, ack, bell,	/* D E F G */
	bs, ht, lf, vt,		/* H I J K */
	ff, cr, so, si,		/* L M N O */
	dle, xon, dc2, xoff,	/* P Q R S */
	dc4, nak, syn, etb,	/* T U V W */
	can, em, sub, esc,	/* X Y Z [ */
	fs, gs, rs, us			/* \ ] ~ ? */
};

NoFont(code)
int code;
{
	extern unsigned char pbuf[];
/*
	0 - N/A
	1 - 132 column
	2 - alt
	3 - alt 132
	4 - gfx
	5 - gfx 132
	6 - spcgfx
	7 - spcgfx 132
*/

static char *ErrColumns[2] = {
	"80 column"
	"132 column",
};

static char *ErrFont[4] = {
	"",
	"alternate",
	"graphics",
	"special graphics",
};

	sprintf(pbuf, "WARNING: Cannot select %s %s font!",
		ErrColumns[code & 1], ErrFont[code >> 1]);
	Msg(pbuf);
}
