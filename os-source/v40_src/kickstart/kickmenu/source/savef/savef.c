#include <exec/types.h>

#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <proto/all.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void kprintf(char *,...);
/* void sprintf(char *,char *,...); */
#define  D(x) x
#define  D1(x) ;
#define kprintf printf

ULONG *startloc = (ULONG *)0xf7ffe4;

main(int argc,char *argv[])
{
   FILE *outfile;
   ULONG actual;
   ULONG offset;
   ULONG count;

   if(argc!=3)
   {
      printf("Usage: savef file offset\n");
      exit(0);
   }

   if((outfile=fopen(argv[1],"w"))==NULL)
   {
      printf("Error opening output file %s\n",argv[1]);
      goto out;
   }
   offset=atoi(argv[2]);
   count =(*startloc)-(0xf00000L+offset);
   printf("savef: saving 0x%lx bytes at offset=0x%lx to file %s\n",count,offset,argv[1]);
   actual=fwrite((char *)(0xf00000L + offset),1,count,outfile);
   if(actual!=count)
   {
      printf("Error: Error: write length =  %ld\n",actual);
      goto out;
   }

out:
   if(outfile) fclose(outfile);
}
