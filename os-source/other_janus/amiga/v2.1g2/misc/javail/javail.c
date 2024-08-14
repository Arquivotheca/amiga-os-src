#include <janus/janus.h>
#include <stdio.h>

struct JanusBase *JanusBase =0;

void PrintMemHeader();
void main();
void main()
{
   struct JanusAmiga *japtr;
   long  pfree,bfree,tfree;
   long  pmax,bmax,tmax;
   long  puse,buse,tuse;
   long  plarge,blarge;
   struct JanusMemChunk *mc;

   if((JanusBase=(struct JanusBase *)OpenLibrary("janus.library",33))==0)
   {
      printf("Unable to open Janus.Library!\n");
      return;
   }
   japtr=(struct JanusAmiga *)JanusBase->jb_ParamMem;
   japtr=(struct JanusAmiga *)MakeWordPtr(japtr);

/*
   printf("jb_ATFlag = %x\n",JanusBase->jb_ATFlag);
   printf("Parameter\n");
   PrintMemHeader(&(japtr->ja_ParamMem));
   printf("Buffer\n");
   PrintMemHeader(&(japtr->ja_BufferMem));
*/

   /*##### SHOULD LOCK MEMORY HERE #####*/
   JanusLock(MakeBytePtr(&(japtr->ja_ParamMem.jmh_Lock)));
   JanusLock(MakeBytePtr(&(japtr->ja_BufferMem.jmh_Lock)));

   mc = (struct JanusMemChunk *)((ULONG)japtr->ja_ParamMem.jmh_68000Base +
                                 (ULONG)japtr->ja_ParamMem.jmh_First);
   mc=(struct JanusMemChunk *)MakeWordPtr(mc);

   plarge=mc->jmc_Size + 1;

   for(;;)
   {
/*
      printf("Next = %x Size = %x\n",(ULONG)mc->jmc_Next,(ULONG)mc->jmc_Size);
*/
      if((mc->jmc_Size+1) > plarge)
      {
         plarge=mc->jmc_Size + 1;
      }
      if(mc->jmc_Next==-1) break;
      mc = (struct JanusMemChunk *)((ULONG)japtr->ja_ParamMem.jmh_68000Base +
                                    (ULONG)mc->jmc_Next);
      mc=(struct JanusMemChunk *)MakeWordPtr(mc);
/*
      printf("in loop mc = %x\n",mc);
*/
   }

   mc = (struct JanusMemChunk *)((ULONG)japtr->ja_BufferMem.jmh_68000Base +
                                 (ULONG)japtr->ja_BufferMem.jmh_First);
   mc=(struct JanusMemChunk *)MakeWordPtr(mc);

/*
   printf("mc = %x\n",mc);
*/

   blarge=mc->jmc_Size + 1;

   for(;;)
   {
/*
      printf("Next = %x Size = %x\n",(ULONG)mc->jmc_Next,(ULONG)mc->jmc_Size);
*/
      if((mc->jmc_Size+1) > blarge)
      {
         blarge=mc->jmc_Size + 1;
      }
      if(mc->jmc_Next==-1) break;
      mc = (struct JanusMemChunk *)((ULONG)japtr->ja_BufferMem.jmh_68000Base +
                                    (ULONG)mc->jmc_Next);
      mc=(struct JanusMemChunk *)MakeWordPtr(mc);
/*
      printf("in loop mc = %x\n",mc);
*/
   }

   pfree=japtr->ja_ParamMem.jmh_Free + 1;
   bfree=japtr->ja_BufferMem.jmh_Free + 1;
   tfree=pfree+bfree;

   pmax=japtr->ja_ParamMem.jmh_Max + 1;
   bmax=japtr->ja_BufferMem.jmh_Max + 1;
   tmax=pmax+bmax;

   /*##### UNLOCK MEMORY HERE #####*/

   JanusUnlock(MakeBytePtr(&(japtr->ja_ParamMem.jmh_Lock)));
   JanusUnlock(MakeBytePtr(&(japtr->ja_BufferMem.jmh_Lock)));

   puse=pmax-pfree;
   buse=bmax-bfree;
   tuse=tmax-tfree;

   printf("Type  Available    In-Use   Maximum   Largest\n");
   printf("parm  %9d    %6d   %7d   %7d\n",pfree,puse,pmax,plarge);
   printf("buff  %9d    %6d   %7d   %7d\n",bfree,buse,bmax,blarge);
   printf("total %9d    %6d   %7d   %7d\n",tfree,tuse,tmax,plarge>blarge?plarge:blarge);

cleanup:
   if(JanusBase) CloseLibrary(JanusBase);
   return;
}
void PrintMemHeader(mh)
struct JanusMemHead *mh;
{
   struct JanusMemHead *mb,*mw;

   mb=(struct JanusMemHead *)MakeBytePtr(mh);
   mw=(struct JanusMemHead *)MakeWordPtr(mh);

   printf("jmh_Lock        = %x\n",mb->jmh_Lock);
   printf("jmh_pad0        = %x\n",mb->jmh_pad0);
   printf("jmh_68000Base   = %x\n",mw->jmh_68000Base);
   printf("jmh_8088Segment = %x\n",mw->jmh_8088Segment);
   printf("jmh_First       = %x\n",mw->jmh_First);
   printf("jmh_Max         = %d\n",mw->jmh_Max);
   printf("jmh_Free        = %d\n",mw->jmh_Free);
   return;
}
