/************************************************************************
 * Ex4.c -  Janus Services Programming Example #4
 *
 * 4-13-89  -  New Code -  Bill Koester
 *
 * Copyright (c) 1989, Commodore Amiga Inc., All rights reserved
 ************************************************************************/
#ifndef LINT_ARGS                /* Enable function type casting in Janus */
#define LINT_ARGS                /* Includes                              */
#endif
#ifndef PRAGMAS
#define PRAGMAS
#endif

#include <janus/janus.h>
#include <janus/timeserv.h>

struct JanusBase *JanusBase = 0; /* Declare library base pointer          */

void main();
void main()
{
   struct TimeServReq *bptr;
   SHORT  signum   = -1;         /* This will be the signal for TimeServ */
   ULONG  sigmask  = 0;          /* Make a mask for the Wait function    */
   struct ServiceData *sdata =0; /* Our ServiceData pointer              */
   int    error;

   /********************
    * Open Janus.Library
    ********************/
   if((JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME,33))==0)
   {
#ifdef DEBUG
      puts("Unable to open Janus.Library");
#endif
      goto cleanup;
   }

#ifdef DEBUG
   printf("JanusBase at %lx\n",JanusBase);
#endif

   /***********************
    * Allocate a signal bit
    ***********************/
   if((signum = AllocSignal(-1))== -1)
   {
#ifdef DEBUG
      puts("No free signal bits");
#endif
      goto cleanup;
   }

#ifdef DEBUG
   printf("Signum = %ld\n",signum);
#endif

   /**********************************
    * Create a signal mask for Wait();
    **********************************/
   sigmask = 1 << signum;

   error = GetService(&sdata,TIMESERV_APPLICATION_ID,TIMESERV_LOCAL_ID,
                      signum,GETS_WAIT_15|GETS_ALOAD_A);

   if(error!=JSERV_OK)
   {
#ifdef DEBUG
      printf("Error Getting service! Error = %ld\n",error);
#endif
      goto cleanup;
   }

#ifdef DEBUG
   printf("sdata = %lx\n",sdata);
#endif

   bptr = (struct TimeServReq *) sdata->sd_AmigaMemPtr;

   CallService(sdata);

   printf("Waiting for reply...");

   Wait(sigmask);

   printf("got it!\n");

cleanup:
   if(sdata) ReleaseService(sdata);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
