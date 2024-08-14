/* FakeFastMem.c
 * Allocates some CHIP mem and adds a fake FAST memlist for it
 * C. Scheppner   CBM 06/89
 */
#include "exec/types.h"
#include "exec/memory.h"
#include "libraries/dos.h"

#define DEFAULT_SIZE 400

ULONG memblock = 0, memsize = DEFAULT_SIZE;
BOOL FromWb;

char *usage1  = "Usage: FakeFastMem KBYTES (1-time only, not reversable)\n";
char *usage2  = "Will convert up to 1/2 your chipmem to fake fastmem\n";
char *usage3  = "Example:  FakeFastMem [>NIL:] 300\n";

char *Copyright = "Copyright 1989 Commdore-Amiga, Inc.";

main(argc,argv)
int argc;
char **argv;
   {
   FromWb = argc ? FALSE : TRUE;
   
   if((argc==1)||((argc>1)&&(argv[1][0]=='?')))
      {
      printf(usage1);
      printf(usage2);
      cleanexit(usage3,RETURN_OK);
      }

   memsize = atoi(argv[1]);

   if(memsize==0)  cleanexit(usage1,RETURN_WARN);

   memsize = (memsize << 10);

   if(AvailMem(MEMF_FAST))
       cleanexit("FakeFast not allocated - FAST mem already present\n",RETURN_WARN);

   if(AvailMem(MEMF_CHIP) < (2 * memsize))
       cleanexit("FakeFast not allocated - request was > 1/2 avail CHIP mem\n", RETURN_WARN + 1);
 
   if(!(memblock=(ULONG)AllocMem(memsize,MEMF_CHIP)))
       cleanexit("Can't allocate memory block\n",RETURN_WARN + 2);

   AddMemList(memsize,MEMF_FAST|MEMF_PUBLIC,0,memblock,NULL);
   cleanexit("",RETURN_OK);
   }

cleanexit(s,n)
char *s;
int n;
   {   
   if((!FromWb)&&(*s)) printf(s);
   /* We never deallocate the memory */
   exit(n);
   }

atoi( s )
char *s;
   {
   int num = 0;
   int neg = 0;

   if( *s == '+' ) s++;
   else if( *s == '-' ) {
       neg = 1;
       s++;
   }

   while( *s >= '0' && *s <= '9' ) {
       num = num * 10 + *s++ - '0';
   }

   if( neg ) return( - num );
   return( num );
   }

