/* Module for DEC VT100 terminal emulation.
	Currently, all	input is passed through here until
	the escape sequence is satisfied.
*/

#include "st.h"
#include "devices/console.h"
#include "devices/conunit.h"
#include "term.h"
#include "ascii.h"
#include "charset.h"
#include "window.h"
#include "menu.h"

/* #define DEBUG /* */
#define isdigit(c) (c>='0' && c<='9')

extern read_cursor_posn(), digits(), set_window(), si(), so();
extern AnsiKeypadAppl(), AnsiKeypadNumeric(), Set_Terminal();
extern puts();
extern int (*ctrl_functions[])();
extern unsigned char pbuf[];

extern WORD row, column;
extern UBYTE escape_flag, c, csi_flag, line_wrap, escape_length;
extern UBYTE Pt, Pb, g0, g1, gd, absolute, char_set, numcolumns;
extern UBYTE cursor_status, window_size, executed;
extern UBYTE numrows;
extern unsigned char *strptr, *escptr, escape_buffer[], huns, tens, ones;
extern unsigned char input_buffer[], *inputptr;
extern struct Window *w;
extern struct IOStdReq consoleIO;

/* must declare these external so that their addresses can be
	put in the function array */
extern end0(), end1(), end2(), end3(), end4(), end5(), end6(), end7();
extern end8(), end3d(), end3e(), endA(), endB(), endC(), endD(), endE();
extern endH(), endI(), endJ(), endK(), endM(), endZ(), end5b(), end5e();
extern endc(), endf(), endg(), endh(), endl(), endm(), endn();
extern endq(), endr(), endx(), endy(), null(), end3c();
extern endb(); /* private escape sequence */

/* array of ptrs to (vt100 escape sequence) functions */
int (*vt100_functions[])() = {
	end0, end1, end2, end3, end4, end5, end6, end7,    /* 0 1 2 3 4 5 6 7 */
	end8, null, null, null, end3c, end3d, end3e, null, /* 8 9 : ; < = > ? */
   null, endA, endB, endC, endD, endE, null, null,    /* @ A B C D E F G */
   endH, endI, endJ, endK, null, endM, null, null,    /* H I J K L M N O */
   null, null, null, null, null, null, null, null,    /* P Q R S T U V W */
   null, null, endZ, end5b, null, null, end5e, null,  /* X Y Z [ \ ] ^ _ */
   null, null, endb, endc, null, null, endf, endg,    /* ' a b c d e f g */
   endh, null, null, null, endl, endm, endn, null,    /* h i j k l m n o */
	null, endq, endr, null, null,	null, null, null,    /* p q r s t u v w */
	endx, endy, null, null, null, null, null, null     /* x y z { | } ~ DEL */
};

vt100parse() /* parse vt100 escape sequences */
{
int err;
	c &= 0x7f; /* ignore high bit */

	if (c < ' ') { /* if a ctrl char... */
		if (executed) (*ctrl_functions[c])(); /* execute control char */
		else display_control_char(c); /* display it */
	}
	else if (!escape_flag) { /* if not parsing an escape sequence */
		if (c!=DELETE) { /* ignore delete */
			if (c=='#' && char_set==CS_UK) c |= 0x80; /* change # to pound */
			*strptr++ = c; /* store vanilla char in buffer */
		}
	}
	else { /* escape flag set,  so we re parsing an escape sequence */
		*escptr++ = c; /* save char */
		escape_length++; /* count # of chars in esacpe buffer */
		if ((c >= '0') && (c < 0x80)) { /* if a possible  esc terminator */
			if (c < '@' && csi_flag) { /* cant start with CSI so ignore */
			}
			else { /* may start with CSI */
				if (escape_length >= MAX_ESCAPE) {
					sprintf(pbuf,"FATAL ERROR: escape_length=%ld",escape_length); 
					Msg(pbuf);
		         CDPutChar(BELL); /* flash screen */
		         VTBeep(TRUE); /* ring bell */
				}
				escape_flag = FALSE; /* flag that the esc seq is over */
				c -= '0'; /* normalize c */
				(*vt100_functions[c])(); /* execute proper escape sequence */
			}
		}
	}
}

check_si(new) /* check for shift-in */
UBYTE new;
{
	g0 = new; /* assign new */
	if (gd==0)
		si(); /* activate the new one */
}

check_so(new) /* check for shift-out */
UBYTE new;
{
	g1 = new; /* assign new */
	if (gd==1)
		so(); /* activate the new one */
}

/* special graphics characters and line drawing set, G0 or G1 designator */
end0() /* ESC ( 0 or ESC ) 0 */
{
	if (escape_length==2) { /* if right length for either G0 or G1 */
		if (*escape_buffer=='(') { /* if valid for G0 */
			check_si(CS_SPC_GFX); /* select spc gfx chars and line drawing */
		}
		else if (*escape_buffer==')') { /* if valid for G1 */
			check_so(CS_SPC_GFX); /* select spc gfx chars and line drawing */
		}
	}
}

/* alternate character rom, G0 or G1 designator */
end1() /* ESC ( 1 or ESC ) 1 */
{
	if (escape_length==2) { /* if right length for either G0 or G1 */
		if (*escape_buffer=='(') { /* if valid for G0 */
			check_si(CS_ALT_ROM); /* select alt char rom */
		}
		else if (*escape_buffer==')') { /* if valid for G1 */
			check_so(CS_ALT_ROM); /* select alt char rom */
		}
	}
}

/* alternate graphics rom, g0 or g1 designator */
end2() /* ESC ( 2 or ESC ) 2 */
{
	if (escape_length==2) { /* if a valid terminator */
		if (*escape_buffer=='(') {
			check_si(CS_ALT_ROM_SPC_GFX); /* select alt char rom spc gfx */
		}
		else if (*escape_buffer==')') {
			check_so(CS_ALT_ROM_SPC_GFX); /* select alt char rom spc gfx */
		}
	}
}

end3() /* double height top half (ESC # 3) */
{
	if (escape_length==2 && *escape_buffer=='#') /* if valid */
		; /* null */
}

end4() /* double height bottom half (ESC # 4) */
{
	if (escape_length==2 && *escape_buffer=='#') /* if valid */
		; /* null */
}

end5() /* single width single height (ESC # 5) */
{
	if (escape_length==2 && *escape_buffer=='#') /* if valid */
		; /* null */
}

end6() /* double width single height (ESC # 6) */
{
	if (escape_length==2 && *escape_buffer=='#') /* if valid */
		; /* null */
}

UBYTE save_row, save_column, save_style, save_reverse, save_g0, save_g1, save_gd;
end7() /* save cursor attributes (ESC 7) */
{
	if (escape_length==1) { /* if valid */
		read_cursor_posn(); /* get relative row */
		if (cursor_status!=ABOVE_WINDOW) row += (Pt-1); /* make it absolute */
		save_row = row;
		save_column = column;
		save_style = ((struct ConUnit *)consoleIO.io_Unit)->cu_AlgoStyle;
		save_reverse = ((struct ConUnit *)consoleIO.io_Unit)->cu_DrawMode;
		save_g0 = g0;
		save_g1 = g1;
		save_gd = gd;
	}
}

/* 132 Es for 'fill screen with Es' command */
static char Es[132]="EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";

end8() /* restore cursor attributes (ESC 8) or fill screen with Es (ESC # 8) */
{
	extern struct Window *w;
	extern struct TextFont *tf_vt100_128;
	extern UBYTE row_mode;

	struct RastPort *rp;
	int i, j, k;

	if (escape_length==1) { /* if restore cursor attributes */
		set_abs_cursor_posn(save_row,save_column); /* restore posn */
		*strptr++ = CSI;
		*strptr++ = '0'; /* all attributes off */
		if (save_style & FSF_BOLD) { /* bold on */
			*strptr++ = ';';
			*strptr++ = '1';
		}
		if (save_style & FSF_UNDERLINED) { /* underscore on */
			*strptr++ = ';';
			*strptr++ = '4';
		}
		if (save_reverse & INVERSVID) { /* inverse video on */
			*strptr++ = ';';
			*strptr++ = '7';
		}
		*strptr++ = 'm'; /* finish off the esc seq. */
		g0 = save_g0;
		g1 = save_g1;
		if ((gd = save_gd) == 1) {
			so();
		}
		else {
			si();
		}
	}
	/* fill screen with Es */
	else if (escape_length==2 && *escape_buffer=='#') {
		rp = w->RPort;
		k = (row_mode == ROW49) ? 49 : 24;
		j = (numcolumns == MAXCOLUMNS && tf_vt100_128) ?
			MAXCOLUMNS : MINCOLUMNS;
		for (i=0; i<k; i++) {
			Move(rp, 0, i*8+6);
			Text(rp, Es, j);
		}
	}
}

end3c() /* switch to vt100 mode from vt52 mode (ignore for vt100) */
{
}

end3d() /* select keypad application mode (ESC =) */
{
	if (escape_length==1) AnsiKeypadAppl();
}

end3e() /* select numeric application mode (ESC >) */
{
	if (escape_length==1) AnsiKeypadNumeric(0); /* vt100 mode */
}

endA() /* cursor up ( CSI Pn A ) or g0/g1 designator */
{
	char *ptr = &escape_buffer[1];

	if (csi_flag) { /* cursor up */
		read_cursor_posn(); /* get relative row */
		if ((row -= numparse(&ptr)) < 1) row = 1; /* bump at top(1) or Pt */
		if (cursor_status==BELOW_WINDOW && row<=window_size) {
			/* if entering window from the bottom... */
			set_window(Pt,Pb); /* re-establish the window */
			cursor_status = INSIDE_WINDOW; /* set new status */
		}
		set_cursor_posn(row,column); /* set new posn */
	}
	else if (escape_length==2) { /* g0 or g1 ? */
		if (*escape_buffer=='(') {
			check_si(CS_UK); /* select uk char set */
		}
		else if (*escape_buffer==')') {
			check_so(CS_UK); /* select uk char set */
		}
	}
}

endB() /* cursor down ( CSI Pn B ) or g0/g1 designator */
{
	char *ptr = &escape_buffer[1];
	UBYTE limit;

	if (csi_flag) { /* cursor down */
		read_cursor_posn(); /* get relative row */
		row += numparse(&ptr); /* add-in absolute magnitude */
		if (cursor_status==ABOVE_WINDOW && row>=Pt) {
			/* if entering window from the top... */
			set_window(Pt,Pb); /* re-establish the window */
			cursor_status = INSIDE_WINDOW; /* set new status */
			row -= (Pt-1); /* make row relative to Pt */
		}
		limit = cursor_status==BELOW_WINDOW ? numrows : window_size;
		if (row>limit) row = limit; /* bump at bottom or Pb */
		set_cursor_posn(row,column); /* set new posn */
	}
	if (escape_length==2) { /* go or g1 ? */
		if (*escape_buffer=='(') {
			check_si(CS_US); /* select us char set */
		}
		else if (*escape_buffer==')') {
			check_so(CS_US); /* select us char set */
		}
	}
}

endC() /* cursor right ( CSI Pn C ) */
{
	char *ptr = &escape_buffer[1];

	if (csi_flag) {
		read_cursor_posn(); /* read cursor posn */
		if ((column += numparse(&ptr)) > numcolumns) column = numcolumns;
		set_cursor_posn(row,column); /* set new posn */
	}
}

endD() /* cursor left (CSI Pn D) or index (ESC D) (cursor down with scroll) */
{
	char *ptr = &escape_buffer[1];

	if (escape_length==1) { /* if valid */
		if (cursor_status==ABOVE_WINDOW) { /* if currently above */
			read_cursor_posn(); /* get relative row */
			if (++row >= Pt) { /* if entering window from the top... */
				set_window(Pt,Pb); /* re-establish the window */
				cursor_status = INSIDE_WINDOW; /* set new status */
				set_cursor_posn(1,column); /* posn to proper column */
			}
			else to_string(); /* do index */
		}
		else to_string(); /* do index */
	}
	else if (csi_flag) { /* cursor left */
		read_cursor_posn(); /* read cursor posn */
		if ((column -= numparse(&ptr)) < 1) column = 1;
		set_cursor_posn(row,column); /* set new posn */
	}
}

endE() /* next line (ESC E) (move down and col to 1 with scroll) */
{
	if (escape_length==1) { /* if valid */
		if (cursor_status==ABOVE_WINDOW) {
			read_cursor_posn(); /* get relative row */
			if (++row >= Pt) { /* if entering window from the top... */
				set_window(Pt,Pb); /* re-establish the window */
				cursor_status = INSIDE_WINDOW; /* set new status */
			}
			else to_string(); /* do next line */
		}
		else to_string(); /* do next line */
	}
}

endH() /* direct cursor addressing or set tab at current column */
{
	if (escape_length==1) set_tab(); /* set tab */
	else endf(); /* else direct cursor addressing */
}

endI() /* Reply to VIDTEX identify sequence */
{
}

endJ() /* screen erasing */
{
	unsigned char c = escape_buffer[1];

	if (csi_flag) {
		if (escape_length==2)
			erase_to_end_of_screen();
		else if (escape_length==3) {
			if (c=='0')
				erase_to_end_of_screen();
			else if (c=='1')
				erase_to_start_of_screen();
			else if (c=='2')
				erase_entire_screen();
		}
	}
}

endK() /* line erasing */
{
	unsigned char c = escape_buffer[1];

	if (csi_flag) {
		if (escape_length==2)
			erase_to_end_of_line();
		else if (escape_length==3) {
			if (c=='0')
				erase_to_end_of_line();
			else if (c=='1')
				erase_to_start_of_line();
			else if (c=='2')
				erase_entire_line();
		}
	}
}

endM() /* reverse linefeed (move cursor up with scroll) */
{
	if (escape_length==1) { /* if valid */
		if (cursor_status==BELOW_WINDOW) { /* if window broken */
			read_cursor_posn(); /* get relative row */
			if (--row <= window_size) { /* if entering window from the bottom */
				set_window(Pt,Pb); /* re-establish the window */
				cursor_status = INSIDE_WINDOW; /* set new status */
				set_cursor_posn(window_size,column); /* set to bot of window */
			}
			else to_string(); /* perform normal reverse index */
		}
		else to_string(); /* perform normal reverse index */
	}
}

endZ() /* what are you (ESC Z) */
{
	if (escape_length==1)
		reply_to_what_are_you();
}

end5b() /* check if we just got a csi ( ESC [ ) */
{
	if (escape_length==1) { /* if just got csi */
		csi_flag = escape_flag = TRUE; /* set csi, fix escape, and exit */
	}
}

end5e() /* '^' cancel escape sequence and display '^' */
{
	*strptr++ = '^';
}

endb() /* switch numeric keypad keys 1-9 to rogue/vi keys (ESCb) */
{
	if (escape_length == 1) {
		RogueKeypad();
		sprintf(pbuf, "Rogue keypad invoked.  Enjoy!");
		Msg(pbuf);
	}
}

endc() /* what are you (CSI c or CSI 0 c) or reset (ESC c) */
{
	if (csi_flag) {
		if (escape_length==2)
			reply_to_what_are_you();
		else if ((escape_length==3) && (escape_buffer[1]=='0'))
			reply_to_what_are_you();
	}
	else if (escape_length==1) /* reset */
		Reset_Terminal();
}

set_abs_cursor_posn(y,x) /* set ABSOLUTE cursor posn */
UBYTE y,x;
{
	y = y>numrows ? numrows : y; /* numrows rows max */
	x = x>numcolumns ? numcolumns : x; /* 'numcolumns' columns max */
	if (absolute==TRUE) { /* if absolute addressing */
		if (y<Pt) { /* if want to go above window */
			set_window(1,Pb); /* open up the top */
			cursor_status = ABOVE_WINDOW; /* we are now above */
		}
		else if (y>Pb) { /* if want to go below window */
			set_window(Pt,numrows); /* open up the bottom */
			cursor_status = BELOW_WINDOW; /* we are now below */
			y -= (Pt-1); /* make row relative to the top of the window */
		}
		else { /* we want to be inside window, check if we were outside */
			if (cursor_status!=INSIDE_WINDOW) {
				set_window(Pt,Pb); /* re-establish window */
				cursor_status = INSIDE_WINDOW; /* we are now inside */
			}
			y -= (Pt-1); /* make row relative to the top of the window */
		}
	}
	set_cursor_posn(y,x); /* position cursor relatively */
}

endf() /* direct cursor addressing */
{
	char *ptr = &escape_buffer[1];
	WORD x, y;

	if (csi_flag) { /* if valid */
		if (escape_length==2) { /* no Pns */
			x = y = 1; /* home cursor */
		}
		else { /* parse Pns */
			/* get row arg */
			y = numparse(&ptr);
			if (*ptr==';') ptr++; /* skip ';' */
			/* get column arg */
			x = numparse(&ptr);
		}
		set_abs_cursor_posn(y,x); /* posn it */
	}
}

endg() /* clear tab stops (CSI g or CSI 0 g or CSI 1 g) */
{
	unsigned char c = escape_buffer[1];

	if (csi_flag) {
		if (escape_length==2)
			clear_tab();
		else if (escape_length==3) {
			if (c=='0')
				clear_tab();
			else if (c=='3')
				clear_all_tabs();
		}
	}
}

/*
	set mode sequences (CSI ? Pn h) or
	enter ANSI hex keypad mode (CSI > 3 h)
*/
endh()
{
	unsigned char c1 = escape_buffer[1];
	unsigned char c2 = escape_buffer[2];

	if (csi_flag && escape_length==4) {
		if (c1=='?') {
			switch (c2) {
			case '1': /* set appl cursor key mode */
				AnsiCursorAppl();
				break;
			case '3': /* set 132 (actually 128) column mode */
				Set_Columns(COLUMN128);
				break;
			case '4': /* set smooth scolling mode */
				break;
			case '5': /* set reverse screen mode */
				Screen_Reverse();
				break;
			case '6': /* set relative origin mode */
				Set_Origin_Mode(RELATIVE);
				break;
			case '7': /* wraparound mode on */
				Set_Line_Wrap(SUBITEMLINEWRAPON);
				break;
			case '8': /* auto-repeat mode on */
				break;
			case '9': /* interlace mode on */
				break;
			}
		}
		else if (c1=='1' && c2=='2') {
			/* set local echo mode (ie. half duplex) */
		}
		else if (c1=='2' && c2=='0') {
			Set_LF_Mode(NEWLINE);
		}
		else if (c1=='>' && c2=='3') {
			AnsiKeypadHex();
		}
	}
}

endl() /* reset mode sequences  (CSI ? Pn l) */
{
	unsigned char c1 = escape_buffer[1];
	unsigned char c2 = escape_buffer[2];

	if (csi_flag && escape_length==4) {
		if (c1=='?') {
			switch (c2) {
			case '1': /* set normal cursor key mode */
				AnsiCursorNormal();
				break;
			case '2': /* switch to vt52 mode */
				Set_Terminal(VT52);
				break;
			case '3': /* set 80 column mode */
				Set_Columns(COLUMN80);
				break;
			case '4': /* set jump scolling mode */
				break;
			case '5': /* set normal screen mode */
				Screen_Normal();
				break;
			case '6': /* set absolute origin mode */
				Set_Origin_Mode(ABSOLUTE);
				break;
			case '7': /* wraparound mode off */
				Set_Line_Wrap(SUBITEMLINEWRAPOFF);
				break;
			case '8': /* auto-repeat mode off */
				break;
			case '9': /* interlace mode off */
				break;
			}
		}
		else if (c1=='1' && c2=='2') {
			/* reset local echo mode (ie. full duplex) */
		}
		else if (c1=='2' && c2=='0') {
			Set_LF_Mode(NONEWLINE);
		}
		else if (c1=='>' && c2=='3') {
			AnsiKeypadNumeric(0); /* vt100 mode */
		}
	}
}

endm() /* set character attributes (CSI Ps;Ps;Ps...m) */
{
	if (csi_flag) to_string(); /* let the console device handle it */
}

endn() /* cursor position request or status report */
{
	unsigned char c = escape_buffer[1];

	if (csi_flag && escape_length==3) {
		if (c=='6') { /* cursor position request */
/*			to_string(); /* pass to console device */
			read_cursor_posn();
			*inputptr++ = ESCAPE;
			*inputptr++ = '[';
			if (row > 9) {
				*inputptr++ = row / 10 | '0';
			}
			*inputptr++ = row % 10 | '0';
			*inputptr++ = ';';
			if (column > 9) {
				*inputptr++ = column / 10 | '0';
			}
			*inputptr++ = column % 10 | '0';
			*inputptr++ = 'R';
		}
		else if (c=='5') { /* status report */
			*inputptr++ = ESCAPE;
			*inputptr++ = '[';
			*inputptr++ = '0';
			*inputptr++ = 'n';
		}
	}
}

endq() /* LED control (CSI Ps q) */
{
	if (csi_flag) /* if valid */
		; /* null */
}

endr() /* set scrolling region (ESC Pt ; Pb r) */
{
char *ptr;
UBYTE top, bot;
	if (csi_flag) { /* if valid */
		top = bot = 0; /* init to invalid parameters */
		if (escape_length==2) { /* if default (ESC [ r) */
			top = 1;
			bot = numrows;
		}
		else { /* parse values */
			ptr = &escape_buffer[1];
			/* get top line */
			top = numparse(&ptr);
			if (*ptr==';') { /* check for separator */
				ptr++; /* skip separator */
				/* get bottom line */
				bot = numparse(&ptr);
			}
		}
		if ((top<bot) && top && (bot<=numrows)) { /* if valid window parameters */
			set_window(Pt=top,Pb=bot); /* set the new window */
			window_size = Pb - Pt + 1; /* calc new size */
			set_abs_cursor_posn(1,1); /* home cursor */
		}
	}
}

endx() /* request terminal parameters (CSI <sol> x) */
{
	extern UBYTE parity, word_length;
	extern ULONG baud;

	char c;
	int n, huns, tens, ones;

	/* allow ESC[x or ESC[0x or ESC[1x */
	if (csi_flag && ((escape_length == 2) || (escape_length == 3 &&
		(escape_buffer[1]=='0' || escape_buffer[1]=='1')))) {
		*inputptr++ = ESCAPE;
		*inputptr++ = '[';
		if (escape_length == 2) {
			c = '2';
		}
		else {
			c = escape_buffer[1] + 2;
		}
		*inputptr++ = c;
		*inputptr++ = ';'; /* delimiter */
		if (parity == PARITY_NONE) {
			c = '1';
		}
		else if (parity == PARITY_ODD) {
			c = '4';
		}
		else {
			c = '5'; /* EVEN */
		}
		*inputptr++ = c; /* parity */
		*inputptr++ = ';'; /* delimiter */
		c = word_length == 8 ? '1' : '2';
		*inputptr++ = c; /* word length */
		*inputptr++ = ';'; /* delimiter */
		if (baud == 110) {
			n = 16;
		}
		else if (baud == 300) {
			n = 48;
		}
		else if (baud == 1200) {
			n = 64;
		}
		else if (baud == 2400) {
			n = 88;
		}
		else if (baud == 4800) {
			n = 104;
		}
		else if (baud == 9600) {
			n = 112;
		}
		else {
			n = 120;
		}
		if (huns = n / 100) {
			*inputptr++ = huns | '0';
		}
		if (tens = (n % 100) / 10) {
			*inputptr++ = tens | '0';
		}
		ones = n % 10;
		*inputptr++ = ones | '0'; /* x baud */
		*inputptr++ = ';'; /* delimiter */
		if (huns) {
			*inputptr++ = huns | '0';
		}
		if (tens) {
			*inputptr++ = tens | '0';
		}
		*inputptr++ = ones | '0'; /* r baud */
		*inputptr++ = ';'; /* delimiter */
		*inputptr++ = '1'; /* bit rate multiplier */
		*inputptr++ = ';'; /* delimiter */
		*inputptr++ = '0'; /* flags */
		*inputptr++ = 'x';
	}
}

endy() /* invoke test(s) (ESC [ 2 ; n y) */
{
	unsigned char c = escape_buffer[3];

	if (csi_flag && escape_length==5 && escape_buffer[1]=='2' && escape_buffer[2]==';') {
		if (c & '1') /* power-up self-test */
			; /* null */
		if (c & '2') /* data loop back */
			; /* null */
		if (c & '4') /* eia modem test */
			; /* null */
		if (c & '8') /* repeat test(s) indefinitely */
			; /* null */
	}
}

reply_to_what_are_you() /* VT100 with AVO (Advanced Video Option) */
{
	*inputptr++ = ESCAPE;
	*inputptr++ = '[';
	*inputptr++ = '?';
	*inputptr++ = '1';
	*inputptr++ = ';';
	*inputptr++ = '2';
	*inputptr++ = 'c';
}

to_string() /* pass escape sequence to console device string */
{
	unsigned char *ptr = escape_buffer;
	int i = escape_length;

	*strptr++ = ESCAPE; /* must start with an escape */
	do {
		*strptr++ = *ptr++; /* pass on to console device */
	} while (--i);
}

numparse(s) /* parse for a numeric value */
char **s; /* the address of the ptr to the string to parse */
{
WORD val = 0;
	while (isdigit(**s)) { /* while the next char is a number... */
		val *= 10;
		val += (**s & 0xf); /* add it to the total */
		*s += 1; /* point to next char */
	}
	return(val == 0 ? 1 : val);
}
