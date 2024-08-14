/* poke.c - poke memory */

#include "exec/types.h"
#include "libraries/dos.h"

UBYTE usage[]=
  "usage:  POKE b|w|l [0x]ADDRESS [0x]VALUE [0xMASK] (b|w|l is byte|word|long)\n";

main(argc,argv)
int argc;
UBYTE **argv;
   {
   ULONG address, value, mask, memval, *lp;
   UWORD *wp;
   UBYTE c, *bp;

   if(argc < 3)  cleanexit(usage,RETURN_OK);

   address	= getval(argv[2]);
   mask		= argc > 4 ? getval(argv[4]) : 0xFFFFFFFF;
   value	= argc > 3 ? getval(argv[3]) & mask : 0;


   c = argv[1][0] | 0x20;

   switch(c)
      {
      case 'b':
        bp = (UBYTE *)address;

        memval = *bp & (~mask);	/* knock out the bits we are changing */
        *bp = memval | value;   /* then | in our already-masked value */
        break;
      case 'w':
        if(address & 0x00000001) 
              cleanexit("Even address required for POKE W\n",RETURN_FAIL);
        wp = (UWORD *)address;
        memval = *wp & (~mask);
        *wp = memval | value;
        break;
      case 'l':
        if(address & 0x00000001) 
              cleanexit("Even address required for POKE L\n",RETURN_FAIL);
        lp = (ULONG *)address;
        memval = *lp & (~mask);
        *lp = memval | value;
        break;
      default:
        cleanexit(usage,RETURN_OK);
        break;
      }
   cleanexit("",RETURN_OK);
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






