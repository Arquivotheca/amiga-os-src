/************************************************************************
 * LockTest.c  -  System Locking test program for testing locking protocols
 *
 * 10-09-90  -  New Code -  Bill Koester
 *
 * Copyright (c) 1990, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <janus/janus.h>
#include <time.h>
#include <stdio.h>

#include "lockserv.h"

#ifndef LINT_ARGS
#define LINT_ARGS
#endif

#define D(x) x
#define kprintf printf

struct JanusAmiga *JanusBase = 0;

void main(int argc,char *argv[]);

void main(int argc,char *argv[])
{
   int    signum   = -1;
   int    sigmask  = 0;
   struct ServiceData *sdata;
   struct LockServReq *bptr, *wptr;
   int    error;
   char buf[50];
   unsigned long start[3];
   unsigned long end[3];
   unsigned long count;
   unsigned long x;
   unsigned long d,m,t;
   long tm;

   if(argc!=2)
   {
      printf("Usage: Lockserv [count]\n");
	  printf("if no count given, infinite loop with time printing\n");
	  return;
   }

   count=0;
   if(argc==2)
      count=(unsigned long)atoi(argv[1]);
   printf("count=%lu\n",count);

   /********************
    * Open Janus.Library
    ********************/
   JanusBase = (struct JanusAmiga *) OpenLibrary(JANUSNAME,0);
   if(JanusBase==0)
   {
      D( puts("Unable to open Janus.Library"); )
      goto cleanup;
   }
   D( kprintf("JanusBase at %lx\n",JanusBase); )

   /***********************
    * Allocate a signal bit
    ***********************/
   signum = AllocSignal(-1);
   if(signum == -1)
   {
      D( puts("No free signal bits"); )
      goto cleanup;
   }
   D( kprintf("Signum = %ld\n",signum); )

   /**********************************
    * Create a signal mask for Wait();
    **********************************/
   sigmask = 1 << signum;

   /***************************************
    * Add our new service to the system and
    * allocate our Parameter Memory
    ***************************************/
   error = AddService(&sdata,LOCKTEST_APPLICATION_ID,LOCKTEST_LOCAL_ID,
                      sizeof(struct LockServReq),
                      MEMF_PARAMETER|MEM_BYTEACCESS,
                      signum,0);
   if(error!=JSERV_OK)
   {
      D( kprintf("Error adding service!\n"); )
      goto cleanup;
   }
   D( kprintf("sdata = %lx\n",sdata); )


   bptr = (struct LockServReq *)sdata->sd_AmigaMemPtr;
   bptr = (struct LockServReq *)MakeBytePtr(bptr);
   wptr = (struct LockServReq *)MakeWordPtr(bptr);

   JanusInitLock(&bptr->lsr_b1);


   if(count==0)
   {
      printf("Press return to begin locking:");
      gets(buf);
	  time(&tm);
	  printf("Locking begins!\n");
	  printf("Current time = %.24s\n",ctime(&tm));
	  printf("Current Time = %.24s\015",ctime(&tm));
      JanusLock(&bptr->lsr_b1);
      for(;;)
	  {
	     for(x=0;x<100000;x++)
		 {
            JanusUnlock(&bptr->lsr_b1);
            JanusLock(&bptr->lsr_b1);
		 }
	     time(&tm);
	     printf("Current Time = %.24s\015",ctime(&tm));
	  }
   } else {
      while(1)
	  {
         printf("Press return to begin locking:");
         gets(buf);
         Forbid();
         Disable();
         DateStamp(&start);
         for(x=0;x<count;x++)
         {
            JanusLock(&bptr->lsr_b1);
            JanusUnlock(&bptr->lsr_b1);
         }
         DateStamp(&end);
         Enable();
         Permit();
         d=end[0]-start[0];
         m=end[1]-start[1];
         if(m)
            end[2]+=(m*50*60);
         t=end[2]-start[2];
         printf("Ticks=%lu\n",t);
      }
   }


   /*********************************************************
    * LockServ mainloop. This will never exit! once added the
    * service will be available forever and ever Amen!
    *********************************************************/
#if 0
more:
   Wait(sigmask); /* wait for someone to call us */
   D( kprintf("TimeServ is being called!!!!!\n"); )

   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service.
    ****************************************************************/
   CallService(sdata);
/*   goto more; */    /* Repeat forever */
#endif


   /***********************************************************
    * This is our cleanup routine if all steps up to
    * and including AddService are OK this will never be called
    ***********************************************************/
cleanup:
   if(sdata) DeleteService(sdata);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
