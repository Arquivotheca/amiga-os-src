#include <exec/types.h>
#include <proto/all.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void kprintf(char *,...);
#define  D(x) ;
#define kprintf printf

main(int argc,char *argv[])
{
   ULONG *ptr = (ULONG *)0xf7ffe4;
   ULONG value=0;

   if(argc!=2)
   {
      printf("Usage: startloc startpos\n");
      exit(0);
   }

   value=atoi(argv[1]);

   printf("Poking 0x%lx\n",value);

   *ptr=value;
}
