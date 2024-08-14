/* AMIGA terminal emulation */

#include "exec/types.h"
#include "ascii.h"

extern unsigned char *strptr, c, pbuf[];
extern UBYTE executed;
extern int (*amiga_functions[])();

AMIGAparse()
{
int err;

/* special compu_serve cmd check */
/*end of special compu_serve cmd check */

   if (c < ' ') { /* if a ctrl char... */
      if (executed) (*amiga_functions[c])(); /* execute control char */
      else display_control_char(c); /* display it */
   }
   else *strptr++ = c;
}
