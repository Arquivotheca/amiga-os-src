/********************************************************
*							*
*  This module contains the actual functions to perform	*
*  all the different terminal emulation			*
*							*
********************************************************/

#include "st.h"
#include "term.h"
#include "colors.h"
#include "devices/conunit.h"
#include "ascii.h"
#include "window.h"
#include "charset.h"
#include "menu.h"
#include "serial.h"

/*#define DEBUG /* */
/* SD_BUXMAX * 2 (128) failed with tests by Hedley Davis as len equaled 130 */
/* SD_BUFMAX * 3 (196) failed with tests by Hedley Davis as len equaled 212 */
#define OUTPUTMAX (SD_BUFMAX + 512) /* max output string */

extern Msg();
extern unsigned char escape_buffer[], input_buffer[];
extern unsigned char *inputptr, *strptr,*escptr;
extern UBYTE terminal_mode, char_set, escape_flag, csi_flag, col_mode, row_mode;
extern UBYTE param_status, char_mask, done, init_state, line_wrap;
extern UBYTE escape_length;
extern struct Screen *s;
extern struct Window *w;
extern struct IOStdReq consoleIO;
extern unsigned char pbuf[];
extern struct MenuItem terminalitems[], line_wrapitems[];
extern struct MenuItem delandbkspitems[];
extern struct MenuItem columnitems[], rowitems[];
extern struct TextFont *tf_vt100_128;
extern ULONG FontBase, sbit;

unsigned char *strptr, string[OUTPUTMAX];
unsigned char huns, tens, ones;
UBYTE Pt, Pb, window_size, cursor_status, g0, g1, gd, lf_mode;
UBYTE absolute, str_start;
UBYTE numcolumns = MINCOLUMNS, numrows = MINROWS, screen_reverse = 0;
WORD row, column;

null()
{
}

enter_special_graphics_mode() /* vt52 */
{
	spc_gfx();
}

exit_special_graphics_mode() /* vt52 */
{
	us_set();
}

cursor_home() /* home cursor */
{
	*strptr++ = CSI;
	*strptr++ = 'H';
}

cursor_up() 
{
   *strptr++ = CSI;
   *strptr++ = 'A';
}

cursor_down()
{
   *strptr++ = CSI;
   *strptr++ = 'B';
}

cursor_right()
{
   *strptr++ = CSI;
   *strptr++ = 'C';
}

cursor_left()
{
   *strptr++ = CSI;
   *strptr++ = 'D';
}

reverse_linefeed() /* CTRL-K */
{
	*strptr++ = CSI;
	*strptr++ = '\013'; /* ctrl-K */
}

#ifdef DEBUG
static int count = 0, max = 0;
#endif

flush_string() /* flush output string to screen */
{
int len;
UBYTE i;

	len = strptr - string;
	if (len > str_start) { /* if any chars pending output */
		if (str_start != 0) { /* if want the cursor_off/cursor_on pair*/
			*strptr++ = CSI;
			*strptr++ = ' ';
			*strptr++ = 'p'; /* cursor on */
			len += 3;
		}
		if (len > OUTPUTMAX)	{
			sprintf(pbuf,"FATAL ERROR: output_buf=%ld",len);
			Msg(pbuf);
         CDPutChar(BELL); /* flash screen */
         VTBeep(TRUE); /* ring bell */
		}
		CDPutStrL(string,len); /* flush stored characters to screen */
		strptr = &string[str_start]; /* flag no chars */
	}

#ifdef DEBUG
	if (len > max) max = len;
	if ((count++ & 15) == 15) {
		count = 0;
		sprintf(pbuf, "output string current max = %ld", max);
		Msg(pbuf);
	}
#endif

}

read_cursor_posn() /* put cursor posn in row, column */
{
	flush_string(); /* flush string to screen */
	if (CheckIO(&consoleIO)==0) WaitIO(&consoleIO); /* wait for write to finish */
	row = ((struct ConUnit *)consoleIO.io_Unit)->cu_YCP + 1;
	column = ((struct ConUnit *)consoleIO.io_Unit)->cu_XCP + 1;
}

read_abs_cursor_posn() /* put abs cursor posn in row, column */
{
	read_cursor_posn(); /* get relative cursor posn */
	if (cursor_status != ABOVE_WINDOW) row = row + Pt - 1; /* compute absolute row */
}

set_cursor_posn(row,column) /* set cursor position */
UWORD row, column;
{
	digits(row); /* convert row to ascii digits */
	*strptr++ = CSI;
	*strptr++ = tens;
	*strptr++ = ones;
	digits(column); /* convert column to ascii digits */
	*strptr++ = ';';
	*strptr++ = huns;
	*strptr++ = tens;
	*strptr++ = ones;
	*strptr++ = 'H';
}

erase_to_end_of_screen()
{
UWORD save_row, save_column;
	read_abs_cursor_posn(); /* read abs cursor posn */
	save_row = row; /* save row */
	save_column = column; /* save column */
	set_window(save_row,numrows); /* open window from current row to bottom of screen */
	set_cursor_posn(1,save_column); /* restore column posn */
	*strptr++ = CSI;
	*strptr++ = 'J'; /* erase to end of screen */
	set_window(Pt,Pb); /* restore old window */
	set_abs_cursor_posn(save_row,save_column); /* restore old cursor posn */
}

erase_to_start_of_screen()
{
char c = 'J';
UBYTE xrow;
	read_abs_cursor_posn(); /* get cursor posn */
	if (row > 1) { /* if not on top row */
		xrow = row-1; /* default is row above cursor */
		if (row==2) { /* if on row 2 */
			c = 'K'; /* erase to end of line */
			xrow = row; /* can't open a window between 1 and 1 */
		}
		set_window(1,xrow); /* open up the window */
		*strptr++ = CSI;
		*strptr++ = c; /* erase to end of screen or end of line */
		set_window(Pt,Pb); /* restore org window */
		set_abs_cursor_posn(row,column); /* restore org cursor posn */
	}
	erase_to_start_of_line(); /* erase from cursor to the start of org line */
}

erase_entire_screen()
{
UWORD save_row, save_column;
	read_abs_cursor_posn(); /* get cursor posn */
/* new stuff */
	save_row = row; /* save row */
	save_column = column; /* save column */
	set_window(1,numrows); /* open window to full screen */
/* end new stuff */
	*strptr++ = CSI;
	*strptr++ = 'H';
	*strptr++ = CSI;
	*strptr++ = 'J'; /* move to home and erase to end of screen */
/* new stuff */
	set_window(Pt,Pb); /* restore old window */
	set_abs_cursor_posn(save_row,save_column); /* restore old cursor posn */
/* end new stuff */
}

erase_to_end_of_line()
{
	*strptr++ = CSI;
	*strptr++ = 'K'; /* erase to end of line */
}

erase_to_start_of_line()
{
UBYTE i;
	read_cursor_posn(); /* get cursor posn */
	set_cursor_posn(row,1); /* posn to first column */
	if (column==numcolumns) { /* if on last column */
		erase_to_end_of_line(); /* erase to end of line */
		set_cursor_posn(row,column); /* restore to original posn */
	}
	else { /* if not on last column */
		for (i=0; i<column; i++) *strptr++ = ' '; /* erase to org col */
		*strptr++ = BACKSPACE; /* restore to org col */
	}
}

erase_entire_line()
{
	read_cursor_posn(); /* get cursor position */
	set_cursor_posn(row,1); /* posn to start ofline */
	*strptr++ = CSI;
	*strptr++ = 'K'; /* erase to end of line */
	set_cursor_posn(row,column); /* restore to original posn */
}

direct_cursor_addressing()
{
	int i = escape_length;
	unsigned char *ptr = escape_buffer;

	*strptr++ = CSI;
	if (escape_length > MAX_ESCAPE) {
		sprintf(pbuf,"FATAL ERROR: escape_buf=%ld",escape_length); 
		Msg(pbuf);
	}
	do {
		*strptr++ = *ptr++;
	} while (--i); /* transfer parameters */
}

identify_terminal_type() /* VT52 */
{
	*inputptr++ = ESCAPE;
	*inputptr++ = '/';
	*inputptr++ = 'Z';
}

digits(x) /* convert x to ascii huns, tens and ones digit */
UBYTE x;
{
	huns = x / 100;
	x -= (huns * 100);
	tens = x / 10;
	x -= (tens * 10);
	ones = x + '0';
	tens += '0';
	huns += '0';
}

set_window(top,bottom) /* set new scroll area */
UBYTE top, bottom;
{
	*strptr++ = CSI;
	*strptr++ = 'H'; /* home cursor */
	digits(--top * 8); /* convert to pixels and then to ascii digits */
	*strptr++ = CSI;
	*strptr++ = huns;
	*strptr++ = tens;
	*strptr++ = ones;
	*strptr++ = 'y'; /* set top of window */
	digits(bottom-top); /* convert to ascii digits */
	*strptr++ = CSI;
	*strptr++ = tens;
	*strptr++ = ones;
	*strptr++ = 't'; /* set bottom of window */
}

UBYTE Back[3], Fore[3];

Screen_Colors(reg,r,g,b)
UBYTE reg, r, g, b;
{
	SetRGB4(&(s->ViewPort),reg,r,g,b);
}

Screen_Normal() /* COLOR1 chars on a COLOR0 background */
{
	SetRGB4(&(s->ViewPort),COLOR0,Back[0],Back[1],Back[2]); /* set background */
	SetRGB4(&(s->ViewPort),COLOR1,Fore[0],Fore[1],Fore[2]); /* set foreground */
	screen_reverse = 0;
}

Screen_Reverse() /* COLOR0 chars on a COLOR1 background */
{
	SetRGB4(&(s->ViewPort),COLOR0,Fore[0],Fore[1],Fore[2]); /* set background */
	SetRGB4(&(s->ViewPort),COLOR1,Back[0],Back[1],Back[2]); /* set foreground */
	screen_reverse = 1;
}

Set_Screen_Colors()
{
	if (screen_reverse) {
		Screen_Reverse();
	}
	else {
		Screen_Normal();
	}
}

Set_Terminal(type) /* set terminal type */
UBYTE type;
{
	BYTE err, i;
	UWORD save_row, save_column;

	/* common to all terminals */
	read_abs_cursor_posn();
	save_row = row;
	save_column = column;
	str_start = (type==TTY || type==AMIGA) ? 0 : 4;
	strptr = &string[str_start]; /* init output ptr */
	if (type==VT52 || type==VT100) {
		string[0] = CSI;		
		string[1] = '0';		
		string[2] = ' ';		
		string[3] = 'p'; /* cursor off */
	}

	for (i=0; i<NUMTERMINAL; i++)
		terminalitems[i].Flags &= ~CHECKED; /* uncheck menuitems */
	terminalitems[type].Flags |= CHECKED; /* check the new one */

	init_tabs(); /* init default tabs */
	Screen_Normal(); /* not reversed */
	inputptr = input_buffer; /* no chars in input buffer */
	terminal_mode = type; /* set terminal type flag */
	set_window(Pt=1, Pb=numrows); /* set full window */
	char_mask = 0x7f; /* dont want the high bit */
	escape_flag = csi_flag = FALSE; /* no active esc seq */

	Set_LF_Mode(NONEWLINE); /* lf means lf */

	Set_Line_Wrap(line_wrap); /* re-instate prev line wrap mode */
	for (i=0; i<NUMWRAP; i++)
		line_wrapitems[i].Flags &= ~CHECKED; /* uncheck menuitems */
	line_wrapitems[1].Flags |= CHECKED; /* check the new one */

	CDPutStr("\233m"); /* all attributes off */
	g0 = CS_US; /* g0 defaults to us char set selected */
	g1 = CS_NULL; /* NULL (US) character set for g1 */
	gd = 0; /* the g0 desig is active */

	Set_Columns(numcolumns==MINCOLUMNS ? COLUMN80 : COLUMN128);

/*
	for (i=0; i<NUMCOLUMN; i++)
		columnitems[i].Flags &= ~CHECKED; 
	columnitems[0].Flags |= CHECKED;
*/

	us_set(); /* select us char set */
	/* terminal specific stuff */
	switch (type) {
	case TTY:
		flush_string(); /* update changes to console */
		err = ttyNormal(); /* remap keyboard */
		break;
	case VT52 :
		escape_flag = csi_flag = param_status = escape_length = 0;
		escptr = escape_buffer;
		flush_string(); /* update changes to console */
		err = vt52Normal(); /* remap keyboard */
		break;
	case VT100:
		escape_flag = csi_flag = escape_length = 0;
		escptr = escape_buffer;
		window_size = Pb - Pt + 1;
		cursor_status = INSIDE_WINDOW;
		Set_Origin_Mode(ABSOLUTE); /* absolute cursor positioning */
		flush_string(); /* update changes to console */
		err = vt100Appl(); /* remap keyboard */
		break;
	case AMIGA:
		flush_string(); /* update changes to console */
		char_mask = 0xff;
		err = AmigaNormal(); /* remap keyboard */
		break;
	default: break;
	}

	for (i=0; i<NUMDEL; i++)
		delandbkspitems[i].Flags &= ~CHECKED; /* uncheck menuitems */
	delandbkspitems[0].Flags |= CHECKED; /* check the new one */

	set_abs_cursor_posn(save_row,save_column);
	flush_string();
	return(err);
}

Set_Origin_Mode(type)
UBYTE type;
{
	if (type==ABSOLUTE) { /* absolute */
		absolute = TRUE;
		set_window(1,Pb);
		if (Pt > 1) cursor_status = ABOVE_WINDOW;
	}
	else { /* relative */
		absolute = FALSE;
		set_window(Pt,Pb);
		cursor_status = INSIDE_WINDOW;
		window_size = Pb - Pt + 1;
	}		
}

Set_Line_Wrap(select)
UBYTE select;
{
BYTE i;
	for (i=0; i<NUMWRAP; i++)
		line_wrapitems[i].Flags &= ~CHECKED;
	line_wrapitems[select].Flags |= CHECKED;
	*strptr++ = CSI;
	*strptr++ = '?';
	*strptr++ = '7';
	*strptr++ = (select==SUBITEMLINEWRAPON) ? 'h' : 'l'; /* select on or off */
	line_wrap = select;
}

Set_LF_Mode(select)
UBYTE select;
{
	extern UBYTE lf_expand_in;

	if (select == NEWLINE) {
		set_newline_mode(); /* carriage-return sends cr/lf */
		lf_expand_in = TRUE; /* expand incoming lf to lf/cr */
	}
	else {
		set_linefeed_mode(); /* carriage-return sends cr */
		lf_expand_in = FALSE; /* expand incoming lf to just lf */
	}
	lf_mode = select; /* select NEWLINE or LINEFEED mode */
/*
	*strptr++ = CSI;
	*strptr++ = '2';
	*strptr++ = '0';
	*strptr++ = (select==NEWLINE) ? 'h' : 'l';
*/
}

display_control_char(c)
unsigned char c;
{
UBYTE style, reverse;
	flush_string(); /* update cursor */
	style = ((struct ConUnit *)consoleIO.io_Unit)->cu_AlgoStyle;
	reverse = ((struct ConUnit *)consoleIO.io_Unit)->cu_DrawMode;
	*strptr++ = CSI;
	*strptr++ = '0'; /* all attributes off */
	*strptr++ = ';';
	*strptr++ = '7'; /* reverse on */
	*strptr++ = 'm'; /* finish off the esc seq. */
	*strptr++ = c | 0x40; /* show char */
	*strptr++ = CSI;
	*strptr++ = '0'; /* all attributes off */
	if (style & FSF_BOLD) { /* bold on */
		*strptr++ = ';';
		*strptr++ = '1';
   }
   if (style & FSF_UNDERLINED) { /* underscore on */
      *strptr++ = ';';
      *strptr++ = '4';
   }
   if (reverse & INVERSVID) { /* inverse video on */
      *strptr++ = ';';
      *strptr++ = '7';
   }
   *strptr++ = 'm'; /* finish off the esc seq. */

	if (c==LINEFEED) { /* if lf */
		*strptr++ = c; /* LF */
		*strptr++ = RETURN; /* carriage_return */
	}
}

Set_Columns(select) /* set # of columns */
UBYTE select;
{
	extern UWORD scr_width;

	UBYTE i, reverse;

	col_mode = COLOK;

	for (i=0; i<NUMCOLUMN; i++) {
		columnitems[i].Flags &= ~CHECKED;
	}

	columnitems[select].Flags |= CHECKED;
	reverse = screen_reverse; /* save status */
	/* if desired and not already selected */
	if (select==COLUMN80 && numcolumns!=MINCOLUMNS) {
		numcolumns = MINCOLUMNS;
		scr_width = 640;
		bye(FALSE); /* partial shutdown */
		done = !init(0,0); /* set done flag */
		screen_reverse = reverse; /* restore status */
		Set_Screen_Colors();
		SetFont(w->RPort,FontBase);
		SetFont(&(w->WScreen->RastPort),FontBase);
		*strptr++ = ESCAPE;
		*strptr++ = 'c';
		Set_Line_Wrap(line_wrap);
		Set_LF_Mode(lf_mode);
		if (gd==0) si();
		else so();
	} else if (select==COLUMN128 && numcolumns!=MAXCOLUMNS) {
		if (tf_vt100_128) { /* if the font is available */
			numcolumns = MAXCOLUMNS;
			scr_width = 660;
			bye(FALSE); /* partial shutdown */
			done = !init(0,0); /* set done flag */
			screen_reverse = reverse; /* restore status */
			Set_Screen_Colors();
			SetFont(w->RPort,tf_vt100_128);
			SetFont(&(w->WScreen->RastPort),tf_vt100_128);
			*strptr++ = ESCAPE;
			*strptr++ = 'c';
			Set_Line_Wrap(line_wrap);
			Set_LF_Mode(lf_mode);
			if (gd==0) si();
			else so(); 
		}
		else {
			NoFont(1); /* 132 */
		}
	}
}

Set_Rows(select) /* set # of rows */
UBYTE select;
{
BYTE i;
	row_mode = ROWOK;
	for (i=0; i<NUMROW; i++) rowitems[i].Flags &= ~CHECKED; /* clear all */
	rowitems[select].Flags |= CHECKED; /* set selected one */

	if (select==ROW24 && numrows!=MINROWS) { /* if desired & not already set */
		numrows = MINROWS;
		bye(FALSE); /* partial shutdown */
		done = !init(0,0); /* set done flag */
		Set_Screen_Colors();
	} else if (select==ROW49 && numrows!=MAXROWS) { /* if desired & not already set */
		numrows = MAXROWS;
		bye(FALSE); /* partial shutdown */
		done = !init(0,0); /* set done flag */
		Set_Screen_Colors();
	}
	if (numcolumns==MINCOLUMNS) {
		SetFont(w->RPort,FontBase);
		SetFont(&(w->WScreen->RastPort),FontBase);
	} else {
		SetFont(w->RPort,tf_vt100_128);
		SetFont(&(w->WScreen->RastPort),tf_vt100_128);
	}

	*strptr++ = ESCAPE;
	*strptr++ = 'c';
	Set_Line_Wrap(line_wrap);
	Set_LF_Mode(lf_mode);
	if (gd==0) si();
	else so(); 

}
