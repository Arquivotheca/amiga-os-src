/************************************************************************
 * Ex7.c -  Janus Services Programming Example #7
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

#include "ex7.h"                       /* Service specific definitions  */

struct JanusBase *JanusBase = 0;       /* Declare library base pointer  */

void main();
void main()
{
   SHORT  signum   = -1;         /* signal number for service            */
   ULONG  sigmask  = 0;          /* Make a mask for the Wait function    */
   struct ServiceData *sdata =0; /* Our ServiceData pointer              */
   int    error;                 /* Generic error variable               */
   int    times = 0;             /* # of times we can be called before   */
                                 /* we die and DeleteService()           */
   APTR   ptr1,ptr2,ptr3;

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
   error = AddService(&sdata,EX7_APPLICATION_ID,EX7_LOCAL_ID,
                      0,0,signum,
                      ADDS_EXCLUSIVE|ADDS_TOPC_ONLY|ADDS_FROMPC_ONLY);

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

   printf("Press return to AllocServiceMem(ptr1)\n");
   { char buf[255]; gets(buf); }

   ptr1 = AllocServiceMem(sdata,100,MEMF_BUFFER|MEM_WORDACCESS);
   printf("ptr1 = %lx\n",ptr1);

   printf("Press return to AllocServiceMem(ptr2)\n");
   { char buf[255]; gets(buf); }

   ptr2 = AllocServiceMem(sdata,100,MEMF_BUFFER|MEM_BYTEACCESS);
   printf("ptr2 = %lx\n",ptr2);

   printf("Press return to AllocServiceMem(ptr3)\n");
   { char buf[255]; gets(buf); }

   ptr3 = AllocServiceMem(sdata,100,MEMF_BUFFER|MEM_BYTEACCESS);
   printf("ptr3 = %lx\n",ptr3);

   printf("Press return to FreeServiceMem(ptr2)\n");
   { char buf[255]; gets(buf); }

   FreeServiceMem(sdata,ptr2);

   printf("Press return to DeleteService()\n");
   { char buf[255]; gets(buf); }

   goto cleanup;
   /*********************************************************
    * Ex7 mainloop.
    * Put the GUTS to your service here.
    *********************************************************/
more:
   Wait(sigmask); /* wait for someone to call us */

   if(sdata->sd_Flags&EXPUNGE_SERVICE) /* Someone wants us to expunge   */
      goto cleanup;

#ifdef DEBUG
   printf("Ex7 is being called!!!!!\n");
   printf("On Entry:\n");
#endif

#ifdef DEBUG
   printf("Ex7 has been called!!!!!\n");
   printf("On Exit:\n");
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
