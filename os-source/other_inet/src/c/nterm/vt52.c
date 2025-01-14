/* Module for DEC VT52 terminal emulation.
	Currently, all
	input is passed through here until
	the escape sequence is satisfied.
*/

#include "exec/types.h"
#include "term.h"
#include "ascii.h"
#include "menu.h"

UBYTE param_status; /* parameter parsing status */

extern int (*ctrl_functions[])();
extern UBYTE escape_flag, csi_flag, c, escape_length, executed, got_dle;
extern unsigned char *strptr, escape_buffer[];
extern unsigned char pbuf[];

/* all terminal functions are external to the specific
	emulator so that separate emulators can share
	similar functions */
extern int null(), Set_Terminal(), vt52Alt(), vt52Normal();
extern int cursor_up(), cursor_down(), cursor_right();
extern int cursor_left(), enter_special_graphics_mode();
extern int exit_special_graphics_mode(), cursor_home();
extern int reverse_linefeed(), erase_to_end_of_screen();
extern int erase_to_end_of_line(), direct_cursor_addressing();
extern int identify_terminal_type();

extern int local_cursor_addressing();

/* array of ptrs to (vt52 escape sequence) functions */
int (*function[])() = {
	null,	/* reserved for parameter parsing direction */
	Set_Terminal, vt52Alt,		/* < = */
	vt52Normal, null, null,		/* > ? @ */
	cursor_up, cursor_down,		/* A B */
	cursor_right, cursor_left,	/* C D */
	null, enter_special_graphics_mode,	/* E F */
	exit_special_graphics_mode, cursor_home,	/* G H */
	reverse_linefeed, erase_to_end_of_screen,	/* I J */
	erase_to_end_of_line, null, null, null,	/* K L M N */
	null, null, null, null, null,	/* O P Q R S */
	null, null, null, null, null,	/* T U V W X */
	local_cursor_addressing,	/* Y */
	identify_terminal_type		/* Z */
	};

int local_cursor_addressing() /* requires parameters */
{
	unsigned char *ptr = escape_buffer;

	switch (param_status) {
	case 0: /* just got Y */
		param_status = 1; /* bump state */
		escape_flag = TRUE; /* escape still valid */
		function[0] = local_cursor_addressing;
		*ptr++ = '0';
		*ptr++ = '0';
		*ptr++ = ';';
		*ptr++ = '0';
		*ptr++ = '0';
		*ptr++ = 'H';
		break;
	case 1: /* got row in c (range space to 7) */
		param_status = 2; /* bump state */
		if (c > '7') c = '7'; /* prevent out of range */
		c -= 31; /* normalize */
		while (c > 9) { /* compute tens digit of row */
			escape_buffer[0]++;
			c -= 10;
		}
		escape_buffer[1] += c; /* save ones digit of row */
		break;
	case 2: /* got column in c (range space to o) */
		param_status = 0; /* reset */
		if (c > 'o') c = 'o'; /* prevent out of range */
		c -= 31; /* normalize */
		while (c > 9) { /* compute tens digit of column */
			escape_buffer[3]++;
			c -= 10;
		}
		escape_buffer[4] += c; /* save ones digit of column */
		escape_length = 5;
		direct_cursor_addressing();
		escape_flag = FALSE;
		break;
	}
}

vt52parse()
{
int err;
	c &= 0x7f; /* ignore high bit */

/* special compu_serve cmd check */
/*end of special compu_serve cmd check */

	if (c < ' ') { /* if a ctrl char... */
		if (executed) (*ctrl_functions[c])(); /* execute control char */
		else display_control_char(c); /* display it */
	}
   else if (!escape_flag) { /* if not parsing an escape sequence */
      if (c!=DELETE) { /* ignore delete */
	      *strptr++ = c; /* store vanilla char in buffer */
      }
   }
	else { /* parsing an escape sequence */
		if (param_status > 0) /* if in the middle of a param esc sequence */
			(*function[0])(); /* do parameter parsing */
		else { /* execute esc seq. */
			escape_flag = FALSE; /* flag that the sequence was handled */
			if (c >= '<' && c <= 'Z') { /* if valid terminator */
				if (escape_length >= MAX_ESCAPE) {
					sprintf(pbuf,"FATAL ERROR: escape_buf=%ld",escape_length);
					Msg(pbuf);
                 CDPutChar(BELL); /* flash screen */
                 VTBeep(TRUE); /* ring bell */
				}
				c -= ';'; /* normalize c */
				if (c==1) (*function[c])(VT100); /* select VT100 mode */
				else (*function[c])(); /* execute proper escape sequence */
			} /* if */
		} /* else */
	} /* else */
}
