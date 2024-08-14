;/* memlist.c - Execute me to compile me with SAS C 5.10
LC -b0 -cfistq -v -j73 memlist.c
Blink FROM LIB:rwstartup.obj,memlist.o TO memlist LIBRARY LIB:Amiga.lib,LIB:LC.lib
quit
*/

/* MemList.c - by  C. Scheppner  Copyright 1988 Commodore-Amiga, Inc.
 *
 *  July 88 revisions:  Now PURE
 *                      Fixed used-block-at-end-of-list-not-shown bug
 *                      Uses new RWstartup
 *  Aug 88 revisions:   Uses "*" for prompt and keyboard input
 *    1.32 revisions:   Add chunk sum, adjust line sizes
 *    1.33 revisions:   Add s option (sum only)
 *    37.1 revisions:   add version, fix memlist truncations
 *    37.2 revisions:   quit properly if too much for buf (would continue)
 *
 *  Based on frags.c
 *  Lists addresses and sizes of free chunks and used areas
 *  Prompts for rerun to avoid re-scatter-loading MemList program
 *  Outputs to stdout - redirect to a disk file permanent record
 *  (Press q<ret> to quit, anythingelse<ret> to re-execute)
 *
 *  Programming note:  The memlists are copied to an 8K buffer and not
 *   written to Output() until we have finished traversing the memlists
 *   and released our Forbid.  This is because the output functions
 *   call other functions which Wait, breaking our Forbid.  Thanks to
 *   Carl Sassenrath for pointing this out.
 *
 *  Linkage info:
 *   RWstartup.obj ... Amiga.lib, LC.lib
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/execbase.h> 
#include <libraries/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_stdio_protos.h>

#include <stdlib.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif


#define  DEFBUFSZ  (16000L)
#define  BIGBUFSZ  (32000L)
#define  SBUFSZ    (80L)

extern struct ExecBase *SysBase;

#define MINARGS 1

UBYTE *vers = "\0$VER: memlist 37.2";
UBYTE *Copyright = 
  "memlist v37.2\n(C) Copyright 1992-93 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: memlist [large] [sum] (larger buffer, sums only)";

/* for AWstartup */
extern UBYTE  NeedWStartup;
UBYTE  *HaveWStartup = &NeedWStartup;

char AppWindow[] =
  "CON:0/0/400/200/ MemList v37.2 Copyright Commodore-Amiga,Inc.";


void bye(UBYTE *s, int e);
void cleanup(void);



char line[] = {"----------------------------------------\n"};
char hash[] = {"########################################\n"};
char tally[]=   {"Total mh_Free %s RAM = $%lx (%ld)\n"};
char tallyc[] = {"  Sum mc_Byte %s RAM = $%lx (%ld)\n"};


void main(int argc,char **argv)
   {
   struct MemHeader *mem, *mymems, *memtmp;
   struct MemChunk *chunk, *mychunks, *chunktmp;
   struct ExecBase *eb = SysBase;
   ULONG  totalfree,totalfreec,chipfree,chipfreec,fastfree,fastfreec;
   ULONG  blockfree, chunkfree;
   ULONG  fstart,fsize,ustart,usize;
   LONG  out, star;
   char sbuf[SBUFSZ], *bufend, *memType, *wbuf, a1, a2;
   int k, bufsz;
   BOOL BufOK, SumOnly = FALSE;

   star = NULL;
   out = Output();

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	fprintf(out,"%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

   star = Open("*",MODE_OLDFILE);

   bufsz = DEFBUFSZ;
   a1 = NULL; a2 = NULL;
   if(argc > 1) a1 = (argv[1][0] | 0x20);
   if(argc > 2) a2 = (argv[2][0] | 0x20);
   if((a1=='l')||(a2=='l'))  bufsz = BIGBUFSZ;
   if((a1=='s')||(a2=='s'))  SumOnly = TRUE;

   if(!(wbuf = (char *)AllocMem(bufsz,MEMF_PUBLIC|MEMF_CLEAR))) goto Done;
   bufend = wbuf + bufsz - 80;

   Loop:

   BufOK = TRUE;
   totalfree = totalfreec = chipfree = chipfreec = fastfree = fastfreec = 0;

   Forbid();

   /* Copy the MemHeaders to my buffer, including tail node */
   mymems = (struct MemHeader *)wbuf;
   memtmp = mymems;
   for (mem = (struct MemHeader *)eb->MemList.lh_Head;
         mem;
           mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      *memtmp++ = *mem;
      }

   /* Copy MemChunks right after, put ptr to each list in mymem's Name */
   mychunks = (struct MemChunk *)memtmp;
   chunktmp = mychunks;
   memtmp = mymems;
   for (mem = (struct MemHeader *)eb->MemList.lh_Head;
      mem->mh_Node.ln_Succ && BufOK;
         mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      /* Use Name to point to my list, copy chunk list there */
      if(mem->mh_First)
         {
         memtmp->mh_Node.ln_Name = (char *)chunktmp;
         for (chunk = mem->mh_First; chunk && BufOK; chunk = chunk->mc_Next)
            {
            *chunktmp++ = *chunk;

            /* If in danger of overflowing our buffer */
            if((ULONG)chunktmp > (ULONG)bufend)
               {
               /* Cause copy and later printout to stop here */
               chunktmp->mc_Next = NULL;
               BufOK = FALSE;
               }
            }
         }
      else  memtmp->mh_Node.ln_Name = NULL;

      memtmp++;
      }

   Permit();

   /* Out of Forbid, now can output the lists */

   for (mem = mymems; mem->mh_Node.ln_Succ; mem++)
      {
      chunkfree = 0;

      if(mem->mh_Attributes & MEMF_FAST)  memType = "FAST";
      if(mem->mh_Attributes & MEMF_CHIP)  memType = "CHIP";

      fprintf(out,"\n%s",hash);
      fprintf(out,"               %s RAM\n",memType);
      fprintf(out,"From $%lx to $%lx, Attributes=$%lx\n",
          mem->mh_Lower,(ULONG)mem->mh_Upper - 1, mem->mh_Attributes);
      fprintf(out,line); 

      if(!SumOnly)
	{
        fprintf(out,"        FREE       |        USED\n");
        fprintf(out,"  $Start    $Size  |  $Start    $Size\n");
        fprintf(out,line);
        /* show used ram at start if any */
        if((ULONG)mem->mh_First != (ULONG)mem->mh_Lower)
         	{
         	fprintf(out,"                   |%8lx %8lx\n",
                 mem->mh_Lower, (ULONG)mem->mh_First - (ULONG)mem->mh_Lower);
         	}
	}

      /* Then show each free chunk, and used areas between them.
       *  Remember - my copy of first chunk in mem->mh_Node.ln_Name
       *             and Next chunk is at chunk++
       *
       * First get address of real first chunk
       */

      fstart = (ULONG)mem->mh_First;
      for (chunk = (struct MemChunk *)mem->mh_Node.ln_Name; chunk;
              chunk = (fstart = (ULONG)chunk->mc_Next) ? chunk + 1 : NULL)
         {
         fsize  = (ULONG)chunk->mc_Bytes;
         chunkfree += fsize;

         if(!SumOnly) fprintf(out,"%8lx %8lx  |",fstart,fsize);

         if(chunk->mc_Next)
            {
            ustart = fstart + fsize;
            usize = ((ULONG)chunk->mc_Next) - (fstart + fsize);
            if(!SumOnly) fprintf(out,"%8lx %8lx\n", ustart, usize);
            }
         /* Check for used chunk all the way at the top */
         else if((ustart=fstart+fsize) < (ULONG)mem->mh_Upper)
            {
            usize = (ULONG)mem->mh_Upper - ustart;
            if(!SumOnly) fprintf(out,"%8lx %8lx\n", ustart, usize);
            }
         else if(!SumOnly) fprintf(out,"\n");
         }

      if(!BufOK)
	{
	fprintf(out,"*** MEM BUFFER OVERFLOW - Try opt l ***\n");
	goto next;
	}

      blockfree = mem->mh_Free;
      totalfree += blockfree;
      totalfreec += chunkfree;
      if(*memType=='C') chipfree += blockfree, chipfreec += chunkfree;
      if(*memType=='F') fastfree += blockfree, fastfreec += chunkfree;

      fprintf(out,"%sThis %s Area mh_Free = $%lx (%ld)\n",
                 line, memType,blockfree, blockfree);
      fprintf(out,"     Chunk mc_Byte Sum = $%lx (%ld)\n%s",
                 chunkfree, chunkfree, line);
      if(!SumOnly) fprintf(out,line);
      }

   fprintf(out,hash);
   fprintf(out,tally,"CHIP",chipfree,chipfree);
   fprintf(out,tallyc,"CHIP",chipfreec,chipfreec);
   fprintf(out,tally,"FAST",fastfree,fastfree);
   fprintf(out,tallyc,"FAST",fastfreec,fastfreec);
   fprintf(out,tally," ALL",totalfree,totalfree);
   fprintf(out,tallyc," ALL",totalfreec,totalfreec);
   fprintf(out,hash);

next:

   if(!(IsInteractive(Input())))
      {
      fprintf(out,"\nDone. (Rerun prompt requires stdin)\n");
      goto Done;
      }

   fprintf(star,"\nq<RET>=quit, [text]<RET>=rerun: ");

   /* Get response in sbuf[] */
   for(k=0; ((sbuf[k]=fgetc(star))!='\n')&&(k<80); k++);
   k++;
   sbuf[k] = NULL;

   if ((sbuf[0] == 'q')&&(k==2))  goto Done;
   else fprintf(out,"\n== NEXT ==> %s",&sbuf[0]);  /* rerun comment */
   goto Loop;

   Done:
   if(wbuf)  FreeMem(wbuf,bufsz);
   if(star)  Close(star);
   }

void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {

    }
