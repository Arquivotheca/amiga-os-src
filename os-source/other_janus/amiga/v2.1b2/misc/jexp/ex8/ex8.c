/************************************************************************
 * Ex8.c -  Janus Services Programming Example #8
 *
 *          Amiga provided service with no data memory.
 *
 *          This program adds an Amiga side service. The service, once
 *          added, waits until it has been called 5 times by the PC
 *          then exits freeing all associated resources.
 *
 * 4-13-89  -  New Code -  Bill Koester
 *
 * Copyright (c) 1989, Commodore Amiga Inc., All rights reserved
 ************************************************************************/
#ifndef LINT_ARGS                      /* Enable type casting in        */
#define LINT_ARGS                      /* Janus includes                */
#endif
#ifndef PRAGMAS                        /* Allow Lattice direct calling  */
#define PRAGMAS                        /* of library functions          */
#endif

#include <janus/janus.h>               /* Janus includes auto include   */
                                       /* any other needed includes     */
                                       /* such as types.h               */
#include <libraries/dos.h>

#include "ex8.h"                       /* Service specific definitions  */

struct JanusBase *JanusBase = 0;       /* Declare library base pointer  */

void main();
void main()
{
   SHORT  signum   = -1;               /* Signal number for Service     */
   ULONG  sigmask  = 0;                /* Make a mask for Wait function */
   struct ServiceData *sdata =0;       /* Our ServiceData pointer       */
   int    error;                       /* Generic error variable        */
   int    times=500;                   /* # of times we can be called   */
                                       /* before we die & DeleteService */
   int    times1=500;
   ULONG  ticks;
   struct DateStamp start,finish;

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

   /**********************************************************************
    * Add our new service to the system and don't allocate any
    * Memory
    * Note the flags, ADDS_EXCLUSIVE means that only one process can
    * GetService us. In addition we specify that we only want to send/get
    * signals to/from the PC, not the Amiga.
    **********************************************************************/
   error = AddService(&sdata,EX8_APPLICATION_ID,EX8_LOCAL_ID,0L,0L,
                      signum,ADDS_EXCLUSIVE|ADDS_TOPC_ONLY|ADDS_FROMPC_ONLY);

   if(error!=JSERV_OK)
   {
#ifdef DEBUG
      printf("Error adding service! Error = %ld\n",error);
#endif
      goto cleanup;
   }

#ifdef DEBUG
   printf("sdata = %lx\n",sdata);
#endif

   /*********************************************************
    * Ex8 mainloop.
    * Put the GUTS to your service here.
    *********************************************************/
   Wait(sigmask);                      /* wait for start signal */
   DateStamp(&start);
   CallService(sdata);
more:
   Wait(sigmask);                      /* wait for someone to call us   */

   if(sdata->sd_Flags&EXPUNGE_SERVICE) /* Someone wants us to expunge  */
      goto cleanup;

#ifdef DEBUG2
   printf("Ex8 is being called!!!!!\n");
#endif

   /* This is where you do your thing!! */

#ifdef DEBUG2
   printf("Ex8 has been called!!!!!\n");
#endif

   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service. This notifies all users execpt us.
    ****************************************************************/
   CallService(sdata);

   if((--times)>0)
      goto more;                       /* Repeat 5 times then die  */

   DateStamp(&finish);
   ticks = ((finish.ds_Minute-start.ds_Minute)*60*50)+
            (finish.ds_Tick-start.ds_Tick);
   printf("Time for %d call/respond cycles = %d ticks\n",
          times1,ticks);

   /****************************************************************
    * Note: By removing the 'if' statement above and leaving just
    *       the 'goto' the service will be a forever service which
    *       will never exit.
    *       By removing the 'if' and 'goto' the service will become
    *       a 'run once then die' service
    ****************************************************************/

cleanup:
   if(sdata) DeleteService(sdata);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
