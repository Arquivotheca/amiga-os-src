/************************************************************************
 * TimeServ.c  -  System time and date service for the PC. This service
 *                allows the PC to set its time and date from the Amiga's
 *                RealTimeClock.
 *
 * 4-19-88  -  New Code -  Bill Koester
 *
 * Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#define TINY

#include <exec/types.h>
#include <exec/memory.h>
#include <janus/janus.h>

#ifndef LINT_ARGS
#define LINT_ARGS
#endif

struct JanusAmiga *JanusBase = 0;

void main();
unsigned char GetMonth();

char   *month[12] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG",
                     "SEP","OCT","NOV","DEC"};

void main()
{
   int    signum   = -1;
   int    sigmask  = 0;
   struct ServiceData *sdata;
   LONG   t;
   char   *s;
   struct TimeServReq *bptr, *wptr;
   int    error;

   /********************
    * Open Janus.Library
    ********************/
   JanusBase = (struct JanusAmiga *) OpenLibrary(JANUSNAME,0);
   if(JanusBase==0)
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
   signum = AllocSignal(-1);
   if(signum == -1)
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

   /***************************************
    * Add our new service to the system and
    * allocate our Parameter Memory
    ***************************************/
   error = AddService(&sdata,TIMESERV_APPLICATION_ID,TIMESERV_LOCAL_ID,
                      sizeof(struct TimeServReq),
                      MEMF_PARAMETER|MEM_WORDACCESS,
                      signum,0);
   if(error!=JSERV_OK)
   {
#ifdef DEBUG
      printf("Error adding service!\n");
#endif
      goto cleanup;
   }
#ifdef DEBUG
   printf("sdata = %lx\n",sdata);
#endif


   bptr = (struct TimeServReq *)sdata->sd_AmigaMemPtr;
   bptr = MakeBytePtr(bptr);
   wptr = MakeWordPtr(bptr);

   /*********************************************************
    * TimeServ mainloop. This will never exit! once added the
    * service will be available forever and ever Amen!
    *********************************************************/
more:
   Wait(sigmask); /* wait for someone to call us */
#ifdef DEBUG
   printf("TimeServ is being called!!!!!\n");
#endif

   time(&t);                       /* Lattice time functions */
   s = ctime(&t);
#ifdef DEBUG
   printf("Current time is %s\n",ctime(&t));
#endif
   strcpy(bptr->tsr_String,s);      /* We get a pointer to a string */
   bptr->tsr_String[3]=0;           /* containing the date and time */
   bptr->tsr_String[7]=0;           /* null terminate the various   */
   bptr->tsr_String[10]=0;          /* fields so we can use atoi()  */
   bptr->tsr_String[13]=0;
   bptr->tsr_String[16]=0;
   bptr->tsr_String[19]=0;
   bptr->tsr_String[24]=0;

   wptr->tsr_Year    = (unsigned short)atoi(&bptr->tsr_String[20]);
   bptr->tsr_Month   = GetMonth(&bptr->tsr_String[4]);
   bptr->tsr_Day     = (unsigned char)atoi(&bptr->tsr_String[8]);
   bptr->tsr_Hour    = (unsigned char)atoi(&bptr->tsr_String[11]);
   bptr->tsr_Minutes = (unsigned char)atoi(&bptr->tsr_String[14]);
   bptr->tsr_Seconds = (unsigned char)atoi(&bptr->tsr_String[17]);

   strcpy(bptr->tsr_String,s);   /* put string into PC format for printing */
   bptr->tsr_String[24]=13;      /* by int 21h */
   bptr->tsr_String[25]=10;
   bptr->tsr_String[26]='$';

#ifdef DEBUG
   printf("Year    = %d\n",wptr->tsr_Year);
   printf("Month   = %d\n",bptr->tsr_Month);
   printf("Day     = %d\n",bptr->tsr_Day);
   printf("Hour    = %d\n",bptr->tsr_Hour);
   printf("Minutes = %d\n",bptr->tsr_Minutes);
   printf("Seconds = %d\n",bptr->tsr_Seconds);
#endif


   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service.
    ****************************************************************/
   CallService(sdata);
   goto more;     /* Repeat forever */



   /***********************************************************
    * This is our cleanup routine if all steps up to
    * and including AddService are OK this will never be called
    ***********************************************************/
cleanup:
   if(sdata) DeleteService(sdata);
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
}
/* function to convert the 3 letter ascii month into ant int for the PC */
unsigned char GetMonth(p)
char *p;
{
   int x;

   for(x=0;x<12;x++)
   if(stricmp(month[x],p)==0)
      return(x+1);
}
