#include <exec/types.h>

#include "protos.h"

#define TOUPPER(c)   ((c) & 0x5f)

VOID strcpy(UBYTE *d, UBYTE *s)
{
   while (*d++ = *s++);
}

ULONG strlen(UBYTE *s)
{
   ULONG i = 0;

   while (*s++) {
      i++;
   }
   return(i);
}

VOID strcat(UBYTE *to, UBYTE *from)
{
   while(*to) {
      to++;
   }
   strcpy(to, from);
}

ULONG stricmp(UBYTE *s, UBYTE *t)
{
   char c1, c2;

   do {
      c1 = *s++;
      if (c1 >= 'a' && c1 <= 'z') {
         c1 = TOUPPER(c1);
      }
      c2 = *t++;
      if (c2 >= 'a' && c2 <= 'z') {
         c2 = TOUPPER(c2);
      }
      if (c1 == '\0' || c2 == '\0') {
         break;
      }
   } while (c1 == c2);

   if (c1 == '\0') {
      if (c2 == '\0')return(0);
      return(1);
   }
   return(-1);
}

ULONG stricmpn(UBYTE *s, UBYTE *t, ULONG n)
{
   char c1, c2;
   LONG i=0;

   do {
      c1 = *s++;
      if (c1 >= 'a' && c1 <= 'z') {
         c1 = TOUPPER(c1);
      }
      c2 = *t++;
      if (c2 >= 'a' && c2 <= 'z') {
         c2 = TOUPPER(c2);
      }
      if (c1 == '\0' || c2 == '\0') {
         break;
      }
   } while ((c1 == c2) && (i++ < n));

   if(i==n)return(0);

   if (c1 == '\0') {
      if (c2 == '\0')return(0);
      return(1);
   }
   return(-1);
}

VOID movmem(UBYTE *s, UBYTE *d, ULONG len)
{
   while (len--) {
      *d++ = *s++;
   }
}

VOID setmem(UBYTE *s, ULONG len, BYTE val)
{
   while (len--) {
      *s++ = val;
   }
}

VOID basecopy(UBYTE *dest,UBYTE *source)
{
   while ((*source != ':')&&( *dest++ = *source++ ));
   *dest=0;
}

VOID restcat(UBYTE *to, UBYTE *from)
{
   while(*to) to++;
   while((*from != ':')&&(*from))from++;
   strcpy(to, from);
}

VOID strzap(UBYTE *to)
{
   while(*to) to++;
   to--;
   *to=0;
}
