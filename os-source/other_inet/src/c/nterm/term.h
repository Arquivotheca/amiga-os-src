/* header file for the terminal program */

#include "intuition/intuition.h"

#define MAX_ESCAPE 128 /* maximum length of an incoming escape sequence */

#define HALF 0 /* half duplex */
#define FULL 1 /* full duplex */

#define ABSOLUTE 0
#define RELATIVE 1

#define TTY 0 /* a tty terminal */
#define VT52 1 /* vt52 terminal */
#define VT100 2 /* vt100 terminal */
#define AMIGA 3 /* std amiga */

#define IDCMP_FLAGS (MENUPICK|GADGETUP|REQSET|GADGETDOWN|ACTIVEWINDOW|INACTIVEWINDOW)

#define MINCOLUMNS 80
#define MAXCOLUMNS 132
#define MINROWS 24
#define MAXROWS 49

#define NEWLINE 0
#define NONEWLINE 1
