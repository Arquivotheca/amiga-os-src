/* simple terminal program */

#include "st.h"
#include "term.h"
#include "ascii.h"
#include "charset.h"
#include "serial.h"
#include "gadgets.h"
#include "menu.h"
#include "window.h"

#define INPUTMAX 128

extern int vt100parse();

struct Screen *s;
struct Window *w;
UBYTE duplex = FULL, capture = FALSE, parity = PARITY_NONE;
UBYTE local = FALSE, line_delays = 0, executed = TRUE, beep = AUDIBLE;
UBYTE first_time = TRUE, terminal_mode = VT100;
UBYTE word_length = 8, stop_bits = 1, serial_open = FALSE;
UBYTE line_wrap = SUBITEMLINEWRAPOFF;
UWORD baud = 1200, scr_width = 640;
UBYTE escape_flag, csi_flag, char_mask, char_set;
UBYTE escape_length = 0, param_status;
unsigned char c, escape_buffer[MAX_ESCAPE], input_buffer[INPUTMAX];
unsigned char *escptr = escape_buffer, *inputptr = input_buffer;
unsigned char pbuf[80];
int (*parsers[])() = {vt100parse};
unsigned char read_buf[SD_BUFMAX];

struct IntuiMessage * GetIntuition(w) /* get an intuition message (maybe) */
struct Window *w;
{
	return (struct IntuiMessage *)(GetMsg(w->UserPort));
}

HandleIntuition()
{
	struct IntuiMessage *msg;
	struct Gadget *address;
	UWORD id;
	UBYTE i;

	while (msg = (struct IntuiMessage *)GetIntuition(w)) { /* get all messages */
		switch (msg->Class) {
		case MENUPICK: /* some menu thing happended */
			DoMenus(msg); /* handle it */
			break;
		case ACTIVEWINDOW: /* my window was made active */
			Requests_to_Me();
			break;
		case INACTIVEWINDOW: /* my window was made inactive */
			Requests_to_System();
			break;
		default: break;
		}
		ReplyMsg(msg); /* reply to the message */
	}
	flush_string(); /* send string to screen (maybe) */
}

VTBeep()
{
/*	c = '\007';
	vt100parse();
*/
}

BreakSerial()
{
}
