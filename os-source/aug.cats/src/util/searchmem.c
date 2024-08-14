/* searchmem.c --- searches memory range for hex value */

#include "exec/types.h"
#include "libraries/dos.h"

extern UBYTE ctoh();
char *clearline = "\033[F\033[B\033[J";

main(argc,argv)
int argc;
UBYTE **argv;
   {
   UBYTE *start, *end, *s;
   int l, ih, il, ib;
   ULONG signals, temp;
   BOOL  Ascii, Hex;
   UBYTE tb;

   if(argc != 4)  usageExit();

   Ascii = (argv[3][0] == '/') ? TRUE : FALSE;
   Hex   = ((!Ascii)&&(argv[3][1] | 0x20 == 'x')) ? TRUE : FALSE;
   if((!Ascii)&&(!Hex))  usageExit();

   start = (UBYTE *)getval(argv[1]);
   end   = (UBYTE *)getval(argv[2]) + 1;
   s     = (UBYTE *)argv[3];

   l = strlen(s) - 2;
   if((Hex)&&(l & 0x01))
      {
      printf("Need even number of nibbles for hex search\n");
      exit(0);
      }

   s++;  /* Skip initial '/' or '0' */

   if(Ascii)  
      {
      s[l] = NULL;    /* Null final '/' */
      }

   if(Hex)
      {
      s++;         /* Skip 'x' */

      /* start ih = hi nib last hex pair, il = lo nib, ib = byte dest */
      for(ih = l-2, ib = l-1; ih>=0; ih -= 2, ib--)
         {
         il = ih + 1;
         tb = ctoh(s[ih]) << 4;
         tb |= ctoh(s[il]);
         s[ib] = tb;
         }
       l = l >> 1;
       s = &s[l];
       }

   printf("\033[0 p"); /* cursor off */

   for(signals=0 ; (start<end)&&(!signals); start++)
      {
      if(match(start,s,l))
         {
         if((Hex)||((*(start-1)!='/')&&(start[l] != '/')))
            {
            printf("%s--> $%lx\n",clearline,start);
            }
         }

      temp = (ULONG)start & 0x0FFF;
      if(!temp)
         {
         signals = (SetSignal(0,0) & SIGBREAKF_CTRL_C);
         printf("%s$%lx",clearline,start);
         }
      }
   printf("%s\033[1 p\n",clearline);  /* cursor on and newline */
   if(signals) printf("Terminated at $%lx\n",start);
   }

getval(s)
char *s;
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }

usageExit()
   {
   printf("Usage: searchmem [0x]startaddr [0x]endaddr [ 0xhex | /ascii/ ]\n");
   exit(0);
   }

UBYTE ctoh(c)
UBYTE c;
   {
   if(c > 0x39)  c -= 7;
   c &= 0x0f;
   return(c);
   }

strlen(s)
char *s;
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }


match(a,b,l)
UBYTE *a, *b;
int l;
   {
   int k;
   for(k=0; k<l; k++,a++,b++)
      {
      if(*a != *b) return(0);
      }
   return(1);
   }

