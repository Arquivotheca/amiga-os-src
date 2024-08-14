/* TTY terminal emulation */

#include "exec/types.h"
#include "ascii.h"

extern unsigned char *strptr, c, pbuf[];
extern UBYTE executed;
extern int (*ctrl_functions[])();

ttyparse()
{
int err;
	c &= 0x7f; /* ignore high bit */

/* special compu_serve cmd check */
/*end of special compu_serve cmd check */

   if (c < ' ') { /* if a ctrl char... */
      if (executed) (*ctrl_functions[c])(); /* execute control char */
      else display_control_char(c); /* display it */
   }
	else *strptr++ = c; /* vanilla char */
}
