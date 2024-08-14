/***************************************************************************
*
* Project:	BASIC PLAYER MODULE
* File:		shock.c
* Date:		17 May 1993
* Version:	0.1
* Status:	Draft
*
* Description:	shock detector module, this module has the following tasks:
*               - monitor subcode during tracking mode
*               - take action on disc errors (scratches)
*
* Interface functions: byte shock_detector_off(void)
*		       byte shock_detector_on(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 17 May 1992		creation H.v.K.
* 21 June 1993		finally made it to a real function PvdZ
***************************************************************************/


#include "defs.h"

#define		PROGRESS_N1	40	/* time slice: N=1 25 frames*/
#define		PROGRESS_N2	20      /* time slice: N=2*/

bit				shock_phase0;
static bit			shock_phase1;
bit		      		shock_recovery_active;
static struct time   idat		last_absolute_time;

extern byte			progress_timer;
extern bit			n1_speed;
extern union {
	     struct {
		    struct subcode_frame	subframe;
		    byte		     	left;
		    byte		     	right;
			} play_subcode;
	     struct {
		    struct time 		low_time;
		    struct time 		high_time;
		    struct time 		target_time;
		    struct time 		tmp_time;
		    } play_times;
	     } store;


extern bit 		is_subcode(byte);
extern byte 		compare_time(struct time *, struct time *);
extern void 		add_time(struct time *, struct time *, struct time *);
extern void 		subtract_time(struct time *, struct time *, struct time *);
extern byte		mute_on(void);
extern byte		mute_off(void);
extern byte		jump_time(struct time *);
extern void 		move_abstime(struct time *);
extern struct time	*alloc_struct_time(void);
extern void		release_struct_time(void);
extern void		start_subcode_reading(void);

/***********************************************************************************************
 * Function  : init_shock_registers
 * Input     : -
 * Output    : -
 * Abstract  : load registers progress timer and last absolute time
 * Decisions :
*/
void init_shock_registers()
{
   if (n1_speed)
      progress_timer=PROGRESS_N1;
   else
      progress_timer=PROGRESS_N2;
      /* copy absolute time (.r_time in case of second and next leadin otherwise .a_time)
       * to last absolute time
      */
      move_abstime(&last_absolute_time);
}


/***********************************************************************************************
 * Function  : shock_detector_off
 * Input     : -
 * Output    : READY
 * Abstract  : switch shock_detector off by resetting shock_recovery_active bit
 * Decisions :
*/

byte shock_detector_off(void)
{
   shock_recovery_active=FALSE;
   return READY;
}


/***********************************************************************************************
 * Function  : shock_detector_on
 * Input     : -
 * Output    : READY
 * Abstract  : switch shock_detector on by setting shock_recovery_active bit
 * Decisions :
*/
byte shock_detector_on(void)
{
   if (n1_speed)
      progress_timer=PROGRESS_N1;
   else
      progress_timer=PROGRESS_N2;

   /* Start shock recovery at target of play_track or jump_to_address*/
   last_absolute_time.min=store.play_times.target_time.min;
   last_absolute_time.sec=store.play_times.target_time.sec;
   last_absolute_time.frm=store.play_times.target_time.frm;
   shock_recovery_active=TRUE;
   shock_phase0=0; shock_phase1=0;
   return READY;
}

/***********************************************************************************************
 * Function  : shock_recover
 * Input     : -
 * Output    : -
 * Abstract  : check subcode continuity as function of time
 *
 *                         < delta       >
 *            ==============================================
 *            --------------|--------|--------|-----------|----->
 *                         O        B        N           U      time
 *
 *                       O = last_absolute_time  N = Q_buffer.a_time
 *                       B = O + 00:00:15  (minimal progress in delta time)
 *                       U = O + 00:01:00  (upper border)
 *
 *			 Demand: B < N < U
 *
 * Decisions :
*/
void shock_recover(void)
{
   struct time		*position;
   struct time		*window;
   byte			status;

   position = alloc_struct_time();
   window = alloc_struct_time();

   if (shock_recovery_active)
   {
      /* Monitor continuity */
      if (!shock_phase0)
      {
	 /* shock_phase = 0
	  * wait till progrssion timer runs out
	 */
	 if (progress_timer == 0)
	 {
	    /* Minimal fiveteen subcode frames should have passed during
	     * decreasing of timer!!
	    */
	    if ( is_subcode(ABS_TIME) )
	    {
	       /* In program area or second or next leadin area
		* fill *alloc_struct_time() with absolute time
		* check minimal progress (about 10 frames) ==>
		* *alloc_struct_time() > last_absolute_time + 00:00:15 (= progress)
	       */
	       move_abstime(position);
	       window->min = 0x00;
	       window->sec = 0x00;
	       window->frm = 0x0F;
	       add_time(&last_absolute_time,window,window);
	       if (compare_time(position,window) == BIGGER)
	       {
		  /* Progress OK!!
		   * *position > last_absolute_time + 00:00:15
		   * Test if *position < last_absolute_time + 00:01:00
		  */
		  window->min = 0x00;
		  window->sec = 0x01;
		  window->frm = 0x00;
		  add_time(&last_absolute_time,window,window);
		  if (compare_time(window,position) == SMALLER)
		  {
		     /* *position > last_absolute_time + 00:01:00
		      * repair in phase 1  
		     */
		     shock_phase0 = 1;
		     mute_on();

		  }
		  else
		  {
		     /* *position > last_absolute_time + 00:00:15
		      * *position < last_absolute_time + 00:01:00
		     */
		     init_shock_registers();
		  }
	       }
	       else
	       {
		  /* Progress not ok: goto phase2 !! 
		   * going to phase 2
		   * adapt last_absolute_time: could hang in a disc defect
		   * add 40 frames
		  */
		  shock_phase0 = 1;
		  mute_on();

	       }
	    }
	    else if ( is_subcode(FIRST_LEADIN_AREA) )
	    {
		  shock_phase0 = 1;
		  mute_on();

	    }
	 }
      }

      if (shock_phase0)
      {
	 /*shock_phase = 1*/
	 window->min = 0x00;
	 window->sec = 0x00;
	 window->frm = 0x28;
	 add_time(&last_absolute_time,window,window);
	 status=jump_time(window);
	 if (status == READY)
	 {
	    shock_phase0 = 0;              /* goto phase 0*/
	    init_shock_registers();
	    mute_off();
	 }
	 else if (status == ERROR)
	 {
	    /*Could not reach position: try a neighbour position*/
	     last_absolute_time.min = window->min;
	     last_absolute_time.sec = window->sec;
	     last_absolute_time.frm = window->frm;
	 }
      }
   }
   release_struct_time();
   release_struct_time();
}