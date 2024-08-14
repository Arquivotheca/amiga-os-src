;/* mergemem.c - Execute me to compile me with SAS C 5.10
LC -b0 -cfistq -v -y -j73 mergemem.c
Blink FROM LIB:awstartup.obj,mergemem.o TO mergemem LIBRARY LIB:Amiga.lib,LIB:LC.lib ND
quit
*/



/* MergeMem.c v2.2 - by  Carolyn Scheppner  CBM  02/87
 *    modified:  07/87 to move printf's outside of Forbid
 *               (printf eventually Waits which breaks a Forbid)
 *               Now responsive from WB, linkage with TWstartup.obj
 *    modified:  05/88 - only merge nodes of type NT_MEMORY
 *               (experimental nodes may be a different size)
 *    modified:  05/89 - sort mem array, merge sorted array
 *
 *    modified:  03/93 to add version string
 *
 *  Attempts to merge the memlists of sequentially configged ram boards
 *  which have the same Attributes (for contiguous expansion ram)
 *
 *  Note:  This program has been tested with an Alegra plugged into
 *         a Microbotics Starboard' pass-thru, and with an A2000
 *         with multiple 2-meg ram boards.  This program makes
 *         the assumption that sequentially configged ram boards
 *         have sequential entries in the MemHeader list.
 *         If ram boards are not installed largest to smallest,
 *         this may not be true and this program will not be able
 *         to merge them.
 *
 *  Alink with AWStartup.obj ... Amiga.lib, LC.lib
 *
 */


#include <exec/types.h>
#include <exec/memory.h>
#include "exec/execbase.h" 
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>

#include <stdlib.h>
#include <string.h>


#define MINARGS 1

UBYTE *vers = "\0$VER: mergemem 39.1";
UBYTE *Copyright = 
  "mergemem v39.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: mergemem";


extern struct ExecBase *SysBase;
extern LONG   stdout;

extern UBYTE  NeedWStartup;
UBYTE  *HaveWStartup = &NeedWStartup;
char AppWindow[] = {"CON:50/30/540/140/ MergeMem v2.2"};
char auth[]   = {"cas/cbm"};


void main(int argc, char **argv)
    {
   struct MemChunk *chunk;
   struct MemHeader *mem, *firstmem, *prevmem = 0;
   struct ExecBase *eb = SysBase;
   ULONG  memsize;
   ULONG  nods[32], mems[32], ends[32], atts[32], prev[32], mrgs[32];
   ULONG  sort[32];
   ULONG  mhSize, memCnt = 0, mrgCnt = 0;
   int    k,c,p;
   BOOL   FromWb;
   /* Temps */
   struct MemChunk *oldFirst;
   APTR   oldLower, oldUpper;
   ULONG  oldFree, tempu;

   FromWb = (argc==0) ? TRUE : FALSE;
   if((argc==2)&&(argv[1][0]=='?'))
      {
      printf("%s\n%s\n",Copyright,usage);
      printf("Attempts to merge sequentially addressed ram boards\n");
      exit(0);
      }

   mhSize = sizeof(struct MemHeader);

   Forbid();
   firstmem = (struct MemHeader *)eb->MemList.lh_Head;

   /* Go to end of MemHeader list */
   for (mem = firstmem;
           mem->mh_Node.ln_Succ;
              mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      nods[memCnt] = (ULONG)mem->mh_Node.ln_Type;
      mems[memCnt] = (ULONG)mem;
      ends[memCnt] = (ULONG)mem->mh_Upper;
      atts[memCnt] = (ULONG)mem->mh_Attributes;

      c = memCnt;
      p = c-1;
      sort[c] = mems[c];
      if(c>0)
         {
         while(c > 0)
            {
            if(sort[c] < sort[p])
               {
               tempu = sort[c];
               sort[c] = sort[p];
               sort[p] = tempu;
               }
            c--;
            p--;
            }
         }

      memCnt++;
      }


   /* Start with highest address sorted mem */
   c = memCnt - 1;
   p = c - 1;
   mem = (struct MemHeader *)(sort[c]);

   /* Backwards, for each except first */
   for( ; c > 0; mem = prevmem)
      {
      prevmem = (struct MemHeader *)(sort[p]);

      /* If prev MemHeader describes neighboring ram of same Attributes */
      if(((ULONG)prevmem->mh_Upper == (ULONG)mem->mh_Lower - mhSize)&&
           (prevmem->mh_Attributes == mem->mh_Attributes)&&
           (prevmem->mh_Node.ln_Type == NT_MEMORY)&&
           (mem->mh_Node.ln_Type == NT_MEMORY))
         {
         prev[mrgCnt] = (ULONG)prevmem;
         mrgs[mrgCnt] = (ULONG)mem;
         mrgCnt++;

         /* Save needed stuff from MemHeader before Remove()ing it */
         oldFirst = mem->mh_First;
         oldLower = mem->mh_Lower;
         oldUpper = mem->mh_Upper;
         oldFree  = mem->mh_Free;
         Remove(mem);

         /* Adjust Upper and Free in prev MemHeader to include this mem */
         memsize = (ULONG)oldUpper - (ULONG)oldLower + mhSize;
         prevmem->mh_Upper = (APTR)((ULONG)prevmem->mh_Upper + memsize);
         prevmem->mh_Free += oldFree;

         /* Link last free chunk of prevmem to first free of mem */
         for (chunk = prevmem->mh_First;
                  chunk->mc_Next; chunk = chunk->mc_Next);
         chunk->mc_Next = oldFirst;

         /* Now FreeMem() the old MemHeader as a MemHeader size chunk */
         FreeMem(mem,mhSize);
         }
      c--;
      p--;
      }
   Permit();

/* TEST */
   for(k=0; k<memCnt; k++) printf("sort[%ld] = 0x%lx\n",k,sort[k]);
   

   if(stdout > 0)
      {
      printf("RAM configuration:\n");
      for(k=0; k<memCnt; k++)
         {
         printf("   Memory node type $%lx, attribute $%lx, from $%lx to $%lx\n",
            nods[k], atts[k], mems[k], (ends[k]) - 1);
         }
     
      if(!mrgCnt)  printf("No merging possible.\n");
      else
         {
         printf("Merged:\n");
         for(k=0; k<mrgCnt; k++)
            {
            printf("   $%lx with $%lx\n",mrgs[k], prev[k]);
           }
         }
      if(FromWb)
         {
         printf("\nPress RETURN to exit: ");
         while(getchar() != '\n');
         }
      }
   }


