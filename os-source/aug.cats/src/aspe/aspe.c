/****************************************************************
* ASPE (Amiga Serial Performance Evaluator) (c)1990 by CBM Inc. *
* starting date: 7/11/90, written by: Darius Taghavy            *
****************************************************************/

/* Amiga includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <exec/tasks.h>
#include <resources/misc.h>
#include <resources/cia.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/cia.h>
#include <devices/serial.h>
#include <libraries/dos.h>

/* LATTICE includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/all.h>

/* ASPE includes */
#include "aspe.h"
#include "aspe_protos.h"

/* defined in amiga.lib */
extern struct Custom __far custom;

/**********
* GLOBALS * ----------------------------------------------------------------
**********/
/* library base pointers */
extern struct Library *TimerBase; /* defined in stop_watch.c */
extern struct ExecBase *SysBase;  /* defined in amiga.lib */

/* other base vectors */
struct MiscResource *MiscBase=NULL;

/* global variables shared between aspe.c, rbf.asm and trans.asm */
ULONG receive_count;  /* number of received bytes in rbf handler */

/* globals to be shared between aspe.c and parser.c */
LONG size;	          /* size of data array */
UWORD baud_rate;      /* serial baud rate */
ULONG num;            /* total number of bytes per test iteration */
ULONG max_tests;      /* number of test iterations */

extern char which_timer;     /* holds 'a' for ciaa or 'b' for ciab */
extern LONG  TimerSigBit;        /* allocated signal bit */
extern LONG  TimerSigMask;       /* TimerSigBit converted into a mask */
extern UWORD Timer;              /* CIA timer underflow clock */

BOOL repeat_aspe = TRUE;
char* error_text;

/******************************************
* Trap (Disable) Lattice CNTRL/C handling * ------------------------------
******************************************/
int CXBRK(void)
{
extern BOOL repeat_aspe;

repeat_aspe = FALSE;
return(0);
}

/********************
*   MAIN Program    * ----------------------------------------------------
*********************/
void main(int argc,char* argv[])
{
   struct Device *dev;
   char *data;		  /* pointer to data array */
   UWORD time_slice;     /* ciab interrupt every time_slice microseconds. */
   ULONG max_interrupts; /* run each iteration of test for ..*/
   ULONG dropped,        /* number of dropped bytes per test iteration */
         test,           /* number of tests */
         ave_dropped;    /* average number of bytes dropped */

   register struct timerequest *StopWatch;

   register LONG i;                /* loop counter variable */
   struct Interrupt *RBFintr,      /* aspe's receive buffer interrupt */
                    *oldRBFintr;   /* previous receive buffer interrupt */

   UBYTE *MyName="A.S.P.E. (c)1990 CBM Inc.",
         *user;

   BOOL GotPort=FALSE, /* flags for successful allocations */
        GotBits=FALSE,
        PriorEnable=FALSE;


   /* --------------- BEGINNING OF CODE SECTION ----------------------- */

   printf("\x9b\x30\x20\x70");  /* tell console to make cursor invisible */
   printf("\x9b\x48\x9b\x4a");  /* home and clear to end of window */

   printf("\n");
   printf(" ======================================================================\n");
   printf(" AMIGA SERIAL PERFORMANCE EVALUATOR by Darius Taghavy, (c)1990 CBM Inc.\n");
   printf(" =v1.33================================================================\n");

   /* Set variables to default values */
   baud_rate = ASPE_BAUDRATE;     /* serial baud rate (10 bits per byte) */
   size = (LONG) ASPE_PACKETSIZE; /* size of data to be sent per interrupt */
   num = (LONG) ASPE_TOTALBYTES;  /* total number of bytes to be sent */
   max_tests = ASPE_MAX_TESTS;    /* number of test iterations */
   which_timer = 'b';             /* CIA timer b */

   /* if aspe was envoked with command line options options do ... */
   if (argc >= 2)
      {
      /* Parse argument options (detected as those beginning with '-') */
      for (i = 1; i < argc && *argv[i] == '-'; ++i) argparse(argv[i]);
      if (*argv[i] == '?')help();
      }

   if (data = init_data(size))    /* initialize data array */
      {
      time_slice = (UWORD) ((10 * 1000000/baud_rate)*size);
      max_interrupts = (num/size);

      /* attempt to get the serial hardware resources */
      if(!(NULL==(MiscBase=(struct MiscResource*)OpenResource("misc.resource"))))
         {
         GotPort = ((user=GetMiscResource(MR_SERIALPORT,MyName))==NULL);
         if (user)
            {
            /* if the last user was serial device flush it */
            if (!(strcmp(user,"serial.device")))
               {
               Forbid(); /* need to lock access to DeviceList */
               dev=(struct Device *)
                  FindName(&SysBase->DeviceList,"serial.device");

               if(dev) RemDevice(dev);
               Permit();
               GotPort=((GetMiscResource(MR_SERIALPORT,MyName))==NULL);
               }
            }

         GotBits=((GetMiscResource(MR_SERIALBITS,MyName))==NULL);
         if(GotPort&&GotBits)
            {
            /* Allocate Interrupt Node structure */
            if(RBFintr=
               AllocMem((LONG)sizeof(struct Interrupt),MEMF_PUBLIC|MEMF_CLEAR))
               {
               /* Initialize the Receive Interrupt node */
               RBFintr->is_Node.ln_Type = NT_INTERRUPT;  /* node type */
               RBFintr->is_Node.ln_Pri  = 0;            /* priority */
               RBFintr->is_Node.ln_Name = MyName;
               RBFintr->is_Code         = RBFHand;	/* pointer to code */
               RBFintr->is_Data         = NULL;

               /* Save state of RBF interrupt and disable it */
               PriorEnable=custom.intenar&INTF_RBF?TRUE:FALSE;
               custom.intena=INTF_RBF;

               /* install the new RBF handler */
               oldRBFintr = SetIntVector( INTB_RBF, RBFintr );

               /* enable aspe's receive interrupt */
               custom.intena=INTF_SETCLR|INTF_RBF;

               /******************************
               * if START gadget is pressed *
               *****************************/
               if (StopWatch=Init_Timer())
                  {
                  Set_cia(time_slice);  /* Tell timer the size of a time unit */
                  set_baud(baud_rate);  /* set serial baud rate */

                  printf("\n Use the system to see how it affects serial performance!");
                  printf("\n Hit <control-c> to exit. \n");

                  printf("\n Baudrate --------------> %ld",baud_rate);
                  printf("\n CIA timer -------------> %c",which_timer);
                  printf("\n Timer resolution ------> %ld micro seconds",time_slice);
                  printf("\n # of tests requested --> %ld",max_tests);
                  printf("\n\n --------------------------- CURRENT TEST -----------------------------\n\n");

                  /* ------------ MAIN EVENT LOOP --------------- */
                  test = 0L;
                  ave_dropped = 0L;

                  /* repeat until user hits control-c or max_tests */
                  while((repeat_aspe == TRUE) && (test <= (max_tests-1)))
                     {
                     Timer = 0;         /* Initilialize global Timer variable */
                     receive_count=0L;  /* Initialize receive byte counter */
                     dropped = 0L;      /* number of dropped bytes */

                     Timer_Start(StopWatch);  /* start stopwatch */
                     if (Begin_cia())         /* if Start timer successful */
                        {
                        while (Timer < max_interrupts)
                           {
                           Wait(TimerSigMask);   /* Wait for timer interrupt signal */
                           trans((char*)data,(LONG)size);     /* send data */
                           }
                        End_cia();                  /* Stop ciab timer */
                        }
                     Timer_Stop(StopWatch);      /* stop stopwatch */

                     /* ---------- PRINT OUT RESULTS --------- */
                     printf(" Test iteration number --------> %ld\n",test+1);
                     printf(" Transmit interrupt executed --> %ld times.\n",(ULONG)Timer);
                     printf(" Bytes sent per interrupt -----> %ld \n",size);
                     printf(" Total bytes sent -------------> %ld\n",(size*(ULONG)Timer));
                     printf(" Total bytes received ---------> %ld\n",receive_count);
                     printf(" Time elapsed -----------------> %lu.%06lu seconds\n",
                              StopWatch->tr_time.tv_secs,
                              StopWatch->tr_time.tv_micro);

                     dropped = (ULONG) 100*((size*Timer)-receive_count); /* scaled up by a factor of 100 */
                     printf(" Dropped bytes ----------------> %ld ",dropped/100);

                     printf("\x0d\x9b\x36\x41"); /* move to beginning of the line */
                                              /* and up by 6 (ASCII 36) lines */
                     fflush(stdout);

                     test++;
                     ave_dropped = ((ave_dropped*(test-1))+dropped)/test;
                     }
                  /* ------------- END OF MAIN EVENT LOOP ---------- */

                  Free_Timer(StopWatch); /* Free timer structure */

                  printf("\n\n\n\n\n\n\n");

                  printf("\n -------------------------- STATISTICS --------------------------------\n");
                  printf("\n # of tests conducted ---> %ld",test);
                  printf("\n Ave. # bytes sent ------> %ld",(size*(ULONG)Timer));

                  /* we're trough with test variable, so we can reuse it ... */
                  test = (ave_dropped*100)/(size*(ULONG)Timer);

                  printf("\n Ave. # bytes dropped ---> %ld.%ld (%ld.%ld \%)\n",
                      ave_dropped/100,ave_dropped-((ave_dropped/100)*100),
                      test/100,test-((test/100)*100) );

                  printf("\n ======================================================================");
                  }
               else error_text = ("\n Couldn't open timer device!\n");

               /* -------- CLOSE DOWN ----------------------- */

               /* disable A.S.P.E.'s RBF interrupt */
               custom.intena=INTF_RBF;

               /* restore prior RBF handler interrupt */
               SetIntVector( INTB_RBF, oldRBFintr);

               /* restore state of prior RBF */
               if(PriorEnable)custom.intena=INTF_SETCLR|INTF_RBF;
               /* and free the memory of MIDIMETER's interrupt */
               FreeMem( RBFintr, (LONG)sizeof(struct Interrupt));
               }
            else error_text= "\n Not enough memory !\n";
            }
         else error_text = "\n Serial resources allocated by another process !\n";
         if(GotBits) FreeMiscResource(MR_SERIALBITS);
         if(GotPort)FreeMiscResource(MR_SERIALPORT);
         }
      else error_text = "\n Couldn't open misc resource! \n";
      FreeMem(data,size);   /* deallocate memory block to exec */
      }
      else error_text= "\n Not enough memory !\n";

      /* ---------- EXIT back to SHELL ------------------- */
      printf("\x9b\x20\x70"); /* make cursor visible again */
      if (error_text)
         {
         puts(error_text);
         exit(RETURN_FAIL);    /* return to DOS */
         }
      else
         {
         puts("\n");
         exit(RETURN_OK);    /* return to DOS */
         }
}
/* eo main() */

/*****************************************************************
* Function: initialize data array                                *
* -------------------------------------------------------------- *
* Arguments: "size" is the size of the array to be allocated     *
*             it could come from a string gadget or tool types   *
* Result:   pointer to beginning of data array                   *
*           or NULL if no sufficient memory                      *
*****************************************************************/
char *init_data(LONG size)
{
   char *data;         /* pointer to data array */
   register LONG i;    /* local loop counter */

   /* allocate memory block from memory free list */
   if(!(data=AllocMem(size,MEMF_PUBLIC))) return(NULL);

   /* initialize array */
   for(i=0L;i<size;i++) data[i]=0xf8; /* MIDI System realtime timing clock */

   return(data);       /* return pointer to data array */
}

/****************************
* Function: Set baud rate   *
* ------------------------- *
* Argument: UWORD baudrate  *
****************************/
#include <exec/execbase.h>

#define NTSCFREQ 3579545
#define PALFREQ 3546895

void set_baud(UWORD baudrate)
{
extern struct ExecBase *SysBase;
UWORD *serper; /* PAULA serper register */
UWORD magicvalue;

   serper = (WORD *)0xdff032; /* absolute address of PAULA */

   if((SysBase->VBlankFrequency)==(UBYTE)50)           /* PAL */
      magicvalue=((PALFREQ+baudrate/2)/baudrate)-1;

   else magicvalue=((NTSCFREQ+baudrate/2)/baudrate)-1; /* NTSC */

   *serper=magicvalue;
}

/* eof ASPE.c  */
