/**********************
* CIA timer functions *
**********************/

/* ---------------------
   Amiga System includes
   --------------------- */
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

/* ----------------
   LATTICE includes
   ---------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/all.h>

/* -------------
   ASPE includes
   ------------- */
#include "aspe.h"
#include "aspe_protos.h"

/* improves source code readability */
#define CLEAR       0

/* ----------------
   GLOBAL VARIABLES
   ---------------- */
extern struct CIA __far ciab;
extern struct CIA __far ciaa;

struct Library  *CIAResource = NULL; /* must be global */

char which_timer;     /* holds 'a' for ciaa or 'b' for ciab */
struct Task *thisTask;

/* -------------------------------------------------
   Statics: Only this file needs to know about these
   ------------------------------------------------- */

/* -----------------------------------------------
   OldCIAInterrupt must be non-null to ensure that
   EndTimer() only removes the interrupt if it was
   properly installed by BeginTimer(). 
   ----------------------------------------------- */
static struct Interrupt 
   TimerInterrupt,  /* The interrupt structure */
   *OldCIAInterrupt = (struct Interrupt *)-1;


/* ----------------------------------------
   global variables to be used in this file
   ---------------------------------------- */
LONG  TimerSigBit = -1;   /* allocated signal bit */
LONG  TimerSigMask;       /* TimerSigBit converted into a mask */
UWORD Timer;              /* CIA timer underflow clock */

/*******************************************************
* Function: Start cia timer, clear pending interrupts, *
*           and enable timer interrupts                *
*******************************************************/
void Start_cia(void)
{
#ifdef DEBUG
   printf("\n Start_cia(): which_timer = %c",which_timer);
#endif

   if (which_timer == 'a')
      {
      ciaa.ciacra &= ~CIACRAF_RUNMODE;   /* reload upon underflow */
      ciaa.ciacra |= CIACRAF_LOAD | CIACRAF_START;   /* Load and start */

      SetICR(CIAResource, CLEAR | CIAICRF_TA);
      AbleICR(CIAResource, CIAICRF_SETCLR | CIAICRF_TA);
      }
   else
      {
      ciab.ciacrb &= ~CIACRBF_RUNMODE;   /* reload upon underflow */
      ciab.ciacrb |= CIACRBF_LOAD | CIACRBF_START;   /* Load and start */

      SetICR(CIAResource, CLEAR | CIAICRF_TB);
      AbleICR(CIAResource, CIAICRF_SETCLR | CIAICRF_TB);
      }
}

/****************************************************
* Function: Disable timer interrupts and stop timer *
****************************************************/
void Stop_cia(void)
{
   if (which_timer == 'a')
      {
      AbleICR(CIAResource, CLEAR | CIAICRF_TA);
      ciaa.ciacra &= ~CIACRAF_START;
      }
   else
      {
      AbleICR(CIAResource, CLEAR | CIAICRF_TB);
      ciab.ciacrb &= ~CIACRBF_START;
      }
}

/*************************************************************
* Function: Set specific period between cia Timer increments *
*           in microseconds                                  *
*************************************************************/
void Set_cia(UWORD micros)
{
   if (which_timer == 'a')
      {
      ciaa.ciatalo = micros & 0xff;
      ciaa.ciatahi = micros >> 8;
      }
   else
      {
      ciab.ciatblo = micros & 0xff;
      ciab.ciatbhi = micros >> 8;
      }
}

/**************************************************************
* Function:  Initialize interrupt structure,                  *
*            allocate a signal and start cia timer.           *
* ----------------------------------------------------------- *
* Results:   If all goes well it returns TRUE, otherwise it   *
*            returns FALSE.                                   *
*                                                             *
* Note:      You must call End_cia() to clean up regardless   *
*            of the return value.                             *
**************************************************************/
BOOL Begin_cia(void)
{
   extern __saveds void TimeOut();

   thisTask = FindTask(NULL);

   /* Get a signal bit.*/
   if ((TimerSigBit = AllocSignal(-1L)) == -1) return(FALSE);

   TimerSigMask = 1L << TimerSigBit;

#ifdef DEBUG
   printf("\n Begin_cia(): which_timer = %c",which_timer);
#endif

   /* Open the CIA resource */
   if (which_timer == 'a')
      {
      if ((CIAResource = OpenResource(CIAANAME)) == NULL) return(FALSE);
      }
   else /* default to ciab */
      {
      if ((CIAResource = OpenResource(CIABNAME)) == NULL) return(FALSE);
      }

#ifdef DEBUG
   printf("\n Begin_cia(): Allocated cia%c resource",which_timer);
#endif

   /* Initialize the interrupt structure. */
   TimerInterrupt.is_Node.ln_Type = NT_INTERRUPT;
   TimerInterrupt.is_Node.ln_Pri = 0;
   TimerInterrupt.is_Code = TimeOut;

   /* Install the interrupt code. */
   if (which_timer == 'a')
      {
      if ((OldCIAInterrupt =
         AddICRVector(CIAResource, CIAICRB_TA, &TimerInterrupt)) != NULL)
         {
         return(FALSE);
         }
      }
   else /* default to ciab */
      {
      if ((OldCIAInterrupt =
         AddICRVector(CIAResource, CIAICRB_TB, &TimerInterrupt)) != NULL)
         {
         return(FALSE);
         }
      }

#ifdef DEBUG
   printf("\n Begin_cia(): Installed cia%c interrupt",which_timer);
#endif

   Start_cia();
   return(TRUE);
}

/************************************************************
* Function: Stop the timer and remove its interrupt vector. * 
************************************************************/
void End_cia(void)
{
   if (TimerSigBit != -1)
      {
      if (OldCIAInterrupt == NULL)
         {
         Stop_cia();

         if(which_timer=='a')
            RemICRVector(CIAResource, CIAICRB_TA, &TimerInterrupt);
         else 
            RemICRVector(CIAResource, CIAICRB_TB, &TimerInterrupt);
         }
      FreeSignal(TimerSigBit);
      }
}

/**********************************************
* Function: increment time variable interrupt * 
**********************************************/
__saveds void TimeOut()           /* save data segment */
{
   ++Timer;
   Signal(thisTask,TimerSigMask);
}

/* eof */