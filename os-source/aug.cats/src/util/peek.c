/* peek.c - peek memory, compare for value */

#include "exec/types.h"
#include "libraries/dos.h"

UBYTE usage[]=
  "usage:  PEEK b|w|l [0x]ADDRESS [[0x]COMPVALUE] [[0x]MASK] (byte|word|long)\n";

main(argc,argv)
int argc;
UBYTE **argv;
   {
   ULONG address, value, mask, memval, *lp;
   UWORD *wp;
   UBYTE c, *bp, *fs;
   BOOL  compare = FALSE;

   if(argc < 3)  cleanexit(usage,RETURN_OK);

   address	= getval(argv[2]);
   mask		= argc > 4 ? getval(argv[4]) : 0xFFFFFFFF;
   compare	= argc > 3 ? TRUE : FALSE;
   value	= argc > 3 ? getval(argv[3]) & mask : 0;


   c = argv[1][0] | 0x20;

   switch(c)
      {
      case 'b':
        bp = (UBYTE *)address;
        memval = *bp & mask;
        fs = "0x%02lx\n";
        break;
      case 'w':
        if(address & 0x00000001) 
              cleanexit("Even address required for PEEK W\n",RETURN_FAIL);
        wp = (UWORD *)address;
        memval = *wp & mask;
        fs = "0x%04lx\n";
        break;
      case 'l':
        if(address & 0x00000001) 
              cleanexit("Even address required for PEEK L\n",RETURN_FAIL);
        lp = (ULONG *)address;
        memval = *lp & mask;
        fs = "0x%08lx\n";
        break;
      default:
        cleanexit(usage,RETURN_OK);
        break;
      }

   printf(fs,memval);
   if((!compare)||((compare)&&(memval==value))) cleanexit("",RETURN_OK);
   else cleanexit("",RETURN_WARN);
   }

cleanexit(s,e)
UBYTE *s;
int e;
   {
   if(*s) printf(s);
   exit(e);
   }


getval(s)
char *s;
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }






