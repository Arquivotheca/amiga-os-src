/*
   Status/Message routines.
*/

#include "extern.h"

#include "protos.h"

VOID Status(struct Window *w,LONG Selected, LONG Required, LONG sw)
{
   static char buf[58] = \
 "  File Blocks To Delete[     ]    Still Needed[     ]    ";
/*012345678901234567890123456789012345678901234567890123456*/
   static char abuf[58] = \
 "  File Blocks Selected To Install[     ]                 ";
/*012345678901234567890123456789012345678901234567890123456*/
    if(sw) {
   sprintf(&buf[24], "%5ld", Selected);
   buf[29] = ']'; /* overwrite NULL */
   sprintf(&buf[47], "%5ld", Required);
   buf[52] = ']'; /* overwrite null */
        SetWindowTitles(w, buf,(char *) -1);
   InformMsg(Required);
    }
    else {
   sprintf(&abuf[34], "%5ld", Selected);
   abuf[39] = ']'; /* overwrite NULL */
        SetWindowTitles(w, abuf,(char *) -1);
    }
}


VOID PrintMsg(UBYTE *msg)
{
#define PM_BUFSIZE   80

   char buf[PM_BUFSIZE];

   strcpy(buf, msg);
   setmem(&buf[strlen(buf)], PM_BUFSIZE - 1 - strlen(buf), ' ');
   SetAPen(rp,3);
   Move(rp, 4, 18);
   Text(rp, buf, PM_BUFSIZE - 1);
   SetAPen(rp,1);
}

VOID ClearMsg(VOID)
{
   PrintMsg("");
}
