;/* searchfile.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 searchfile.c
Blink FROM LIB:c.o,searchfile.o TO searchfile LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

/* 37_3 mods - fix detection at wrap of buffer
 * 37_4 mods - error out if file smaller than search thing
 */

#include "exec/types.h"
#include "libraries/dos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>


#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

extern UBYTE ctoh();
char *clearline = "\033[F\033[B\033[J";


UBYTE *vers = "\0$VER: searchfile 37.4";
UBYTE *Copyright = 
  "searchfile v37.4\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";

#define BUFSZ 4096
UBYTE buf[BUFSZ];

void dumphex(UBYTE *values, int count);
int getval(char *s);
void usageExit(void);
int match(UBYTE *a, UBYTE *b, int l);
UBYTE ctoh(UBYTE c);

void main(int argc, char **argv)
   {
   UBYTE *start, *end, *s;
   LONG  ih, il, ib;
   ULONG signals;
   LONG  l, rlen, offs, left, bump;
   BOOL  Ascii, Hex;
   ULONG file;
   UBYTE tb;

   if(argc != 3)  usageExit();

   Ascii = (argv[2][0] == '/') ? TRUE : FALSE;
   Hex   = ((!Ascii)&&(argv[2][1] | 0x20 == 'x')) ? TRUE : FALSE;
   if((!Ascii)&&(!Hex))  usageExit();

   s     = (UBYTE *)argv[2];

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

   if(!(file=Open(argv[1],MODE_OLDFILE)))
      {
      printf("Can't open %s\n",argv[0]);
      exit(RETURN_FAIL);
      }

   rlen = 1;
   signals = 0;
   offs = 0;
   left = 0;
   while((!signals)&&(((rlen = Read(file,&buf[left],BUFSZ-left)) > 0)||(left)))
      {
      start = buf;
      if(!left)
	{
	if(rlen < l)
	    {
	    printf("Can't search - search string (size %ld) is larger than file (%ld)\n",l,rlen);
	    }
        left = (l > 16) ? (l-1) : 15;
	if(left > rlen) left=(rlen-1);
        end = buf + (rlen - left);
	bump = rlen - left;
	}
      else
	{
        end = buf + rlen;
	bump = rlen;
	}


/* DEBUGGING
      printf("read %ld at position %ld, end=$%lx (search %ld spots) \n",
		rlen,left,end,end-buf+1);
      printf("buffer start: "); dumphex(buf,16);
      printf("buffer +16:   "); dumphex(buf+16,16);
*/

      printf("%s$%lx  ",clearline,offs+((ULONG)start-(ULONG)buf));

      for( ; (start<end)&&(!signals); start++)
         {
         if(match(start,s,l))
            {
            if((Hex)||((*(start-1)!='/')&&(start[l] != '/')))
               	{
               	printf("%s--> $%lx  \t",
			clearline,offs+((ULONG)start-(ULONG)buf));
	       	dumphex(start,16);
               	}
            } 
 
         signals = (SetSignal(0,0) & SIGBREAKF_CTRL_C);
         }

      if(rlen > 0)
	{
        offs += bump;
      	CopyMem(end,buf,left);

/* DEBUGGING
      	printf("offs now %ld, copied %ld bytes of: ",offs, left); dumphex(buf,16);
*/
	}
      else left=0;	/* done */
      }
   printf("%s\033[1 p\n",clearline);  /* cursor on and newline */
   if(signals) printf("Break at $%lx\n",offs+((ULONG)start-(ULONG)buf));

   Close(file);
   exit(RETURN_OK);
   }

int getval(char *s)
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }

void usageExit()
   {
   printf("Usage: searchfile file [ 0xhex | /ascii/ ]\n");
   exit(0);
   }

UBYTE ctoh(UBYTE c)
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


int match(UBYTE *a, UBYTE *b, int l)
   {
   int k;
   for(k=0; k<l; k++,a++,b++)
      {
      if(*a != *b) return(0);
      }
   return(1);
   }



void dumphex(UBYTE *values, int count)
    {
    ULONG  ul[33];
    UBYTE  *ub1, *ub2, j;
    int lcnt;

    if(count > 64) count = 64;

    count = (count + 3) & 0xFFFFFFFC;
    lcnt = count >> 2;

    ub1 = values;
    ub2 = (UBYTE *)(&ul[0]);

    /* Copy 16 bytes pointed at to align */
    for(j=0; j<count; j++)  ub2[j] = ub1[j];
    /* Output as hex ulong dump */
    for(j=0; j<lcnt; j++)  printf("%08lx ",ul[j]);
    printf("  ");
    /* Convert ul[] bytes to ASCII representation */
    for(j=0; j<count; j++)
        {
        if((ub2[j] < 0x20)||(ub2[j] > 0x7e))  ub2[j] = '.';
        }
    ub2[j] = NULL;
    /* Output ASCII representation */
    printf("%s\n",(char *)ul);
    }
