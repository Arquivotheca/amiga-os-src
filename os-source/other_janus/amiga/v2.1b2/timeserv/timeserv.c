/************************************************************************
 * TimeServ.c  -  System time and date service for the PC. This service
 *                allows the PC to set its time and date from the Amiga's
 *                RealTimeClock.
 *
 * 4-19-88  -  New Code -  Bill Koester
 *
 * Copyright (c) 1987, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#define LINT_ARGS
#define PRAGMAS
#define D(x) ;
#define printf kprintf

#include <janus/janus.h>

struct JanusBase *JanusBase = 0;

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
   JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME,0);
   if(JanusBase==0)
   {
      D( printf("TS:Unable to open Janus.Library"); )
      goto cleanup;
   }
   D( kprintf("TS:JanusBase at %lx\n",JanusBase); )

   /***********************
    * Allocate a signal bit
    ***********************/
   signum = AllocSignal(-1);
   if(signum == -1)
   {
      D( printf("TS:No free signal bits"); )
      goto cleanup;
   }
   D( kprintf("TS:Signum = %ld\n",signum); )

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
                      signum,ADDS_LOCKDATA);
   if(error!=JSERV_OK)
   {
      D( kprintf("TS:Error adding service!\n"); )
      goto cleanup;
   }
   D( kprintf("TS:sdata = %lx\n",sdata); )

   bptr = (struct TimeServReq *)sdata->sd_AmigaMemPtr;
   bptr = (struct TimeServReq *)MakeBytePtr((APTR)bptr);
   wptr = (struct TimeServReq *)MakeWordPtr((APTR)bptr);

   D( kprintf("TS:bptr=0x%lx\n",bptr); )
   D( kprintf("TS:wptr=0x%lx\n",wptr); )
   /*********************************************************
    * TimeServ mainloop. This will never exit! once added the
    * service will be available forever and ever Amen!
    *********************************************************/
	UnlockServiceData(sdata);
more:
   Wait(sigmask); /* wait for someone to call us */
   D( kprintf("TS:TimeServ is being called!!!!!\n"); )

   time(&t);                       /* Lattice time functions */
   s = (char *)ctime(&t);
   D( kprintf("TS:Current time is %s\n",ctime(&t)); )
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

D(
   printf("Year    = %ld\n",(long)wptr->tsr_Year);
   printf("Month   = %ld\n",(long)bptr->tsr_Month);
   printf("Day     = %ld\n",(long)bptr->tsr_Day);
   printf("Hour    = %ld\n",(long)bptr->tsr_Hour);
   printf("Minutes = %ld\n",(long)bptr->tsr_Minutes);
   printf("Seconds = %ld\n",(long)bptr->tsr_Seconds);
)


   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service.
    ****************************************************************/
   CallService(sdata);
/*   goto more; */    /* Repeat forever */

	while(sdata->sd_PCUserCount!=0);

   /***********************************************************
    * This is our cleanup routine if all steps up to
    * and including AddService are OK this will never be called
    ***********************************************************/
cleanup:
   D( kprintf("TS:Before DeleteService()\n"); )
   if(sdata) DeleteService(sdata);
   D( kprintf("TS:After DeleteService()\n"); )
   if(signum!= -1) FreeSignal(signum);
   if(JanusBase) CloseLibrary(JanusBase);
   D( kprintf("TS:Says later dude!\n"); )
}
/* function to convert the 3 letter ascii month into ant int for the PC */
unsigned char GetMonth(p)
char *p;
{
   int x;

   for(x=0;x<12;x++)
   if(stricmp(month[x],p)==0)
      return((unsigned char)(x+1));
}
