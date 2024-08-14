/************************************************************************
 * Ex2.c -  Janus Services Programming Example #2
 *
 *          Amiga provided service with data memory.
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

#include "ex2.h"                       /* Service specific definitions  */

struct JanusBase *JanusBase = 0;       /* Declare library base pointer  */

void main();
void main()
{
   SHORT  signum   = -1;         /* signal number for service            */
   ULONG  sigmask  = 0;          /* Make a mask for the Wait function    */
   struct ServiceData *sdata =0; /* Our ServiceData pointer              */
   struct Ex2Req *bptr, *wptr;   /* byte and word pointers to our data   */
   int    error;                 /* Generic error variable               */
   int    times = 0;             /* # of times we can be called before   */
                                 /* we die and DeleteService()           */

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
    * Add our new service to the system and allocate our Data Memory
    * Note the flags, ADDS_EXCLUSIVE means that only one process can
    * GetService us. In addition we specify that we only want to send/get
    * signals to/from the PC, not the Amiga.
    **********************************************************************/
   error = AddService(&sdata,EX2_APPLICATION_ID,EX2_LOCAL_ID,
                      sizeof(struct Ex2Req),MEMF_BUFFER|MEM_WORDACCESS,
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

   /* Need both word and byte access pointers so make them! */
   bptr = (struct Ex2Req *)sdata->sd_AmigaMemPtr;
   bptr = (struct Ex2Req *)MakeBytePtr((APTR)bptr);
   wptr = (struct Ex2Req *)MakeWordPtr((APTR)bptr);

   bptr->exr_Function = 0; /* Initialize our data structure */
   wptr->exr_Called   = 0;
   bptr->exr_Error    = 0;

   /*********************************************************
    * Ex2 mainloop.
    * Put the GUTS to your service here.
    *********************************************************/
more:
   Wait(sigmask); /* wait for someone to call us */

   if(sdata->sd_Flags&EXPUNGE_SERVICE) /* Someone wants us to expunge   */
      goto cleanup;

#ifdef DEBUG
   printf("Ex2 is being called!!!!!\n");
   printf("On Entry:\n");
   printf("Function = %lx\n",bptr->exr_Function);
   printf("Called   = %lx\n",wptr->exr_Called);  /* Note word access mode*/
   printf("Error    = %lx\n",bptr->exr_Error);
#endif

   switch(bptr->exr_Function)             /* This part of the service is */
   {                                      /* defined by the service.     */
      case  0  :                          /* In this case the service    */
            printf("Function 0\n");       /* supports different functions*/
            wptr->exr_Called +=1;         /* that the caller sets in     */
            bptr->exr_Error=Exr_ERR_OK;   /* exr_Function before calling */
            break;
      case  1  :
            printf("Function 1\n");
            wptr->exr_Called +=1;
            bptr->exr_Error=Exr_ERR_OK;
            break;
      default:
            printf("Function Unknown\n");
            bptr->exr_Error=Exr_ERR_FAILED; /* set error return       */
            break;
   }

#ifdef DEBUG
   printf("Ex2 has been called!!!!!\n");
   printf("On Exit:\n");
   printf("Function = %lx\n",bptr->exr_Function);
   printf("Called   = %lx\n",wptr->exr_Called);
   printf("Error    = %lx\n",bptr->exr_Error);
#endif

   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service. This notifies all users except us.
    ****************************************************************/
   CallService(sdata);

   if(times++ < 4 )
      goto more;                 /* Repeat five times then die     */

   /****************************************************************
    * Note: By removing the 'if' statement above and leaving just
    *       the 'goto' the service will be a 'forever' service which
    *       will never exit.
    *       By removing the 'if' and 'goto' the service will become
    *       a 'run once then die' service.
    ****************************************************************/

cleanup:
   if(sdata) DeleteService(sdata);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
