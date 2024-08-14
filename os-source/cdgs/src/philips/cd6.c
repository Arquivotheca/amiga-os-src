/***************************************************************************
*
* Project:	Commodore
* File:		cd6.c
* Date:		06 July 1993
* Version:	0.1
* Status:	Draft
*
* Description:	this module initialises the CD-6 EFM-decoder and reads the subcode
*
* Interface functions: void cd6_init(void)
*		       bit status_cd6(byte)
*                      void start_subcode_reading(void)
*                      void stop_subcode_reading(void)
*                      bit is_subcode(byte mode)
*                      void move_abstime(struct time *p)
*                      void subcode_module(void)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 17 November 1992	creation H.v.K.
* 22 March 1993		statement IE0=FALSE added in function: start_subcode_reading
* 23 March 1993		new public variable "hex_abs_min" for function: time_to_brake
* 23 April 1993		function "is_subcode" changed for multi-session discs.
* 26 April 1993		function "move_abstime" added.
* 11 May 1993           check on start time is FF:FF:FF for next possible
* 			program area.
* 02 June 1993		flag "i_can_read_subcode" is set when valid subcode
*			is read.
* 11 June 1993		function "status_cd6" moved from servo.c to cd6.c.
* 30 June 1993		dummy status return for motor overflow in function
*			"status_cd6" is deleted.
*
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

extern void cd6_wr(byte);
extern bit cd6_status_pin(void);
extern bit cd6_read_subcode(void);
extern byte bcd_to_hex(byte);
extern void bcd_to_hex_time(struct time *, struct time *);


struct subcode_frame	Q_buffer;	/* array for storing subcode data */
byte   idat		peak_level_low;
byte   idat		peak_level_high;
bit			subcode_reading;
static bit		new_subcode_request;
bit			i_can_read_subcode; /* set when valid subcode is read
					       reset in servo_monitor_state and timer re-started */
byte			hex_abs_min;

extern bit		cd_disc, scor_edge;


/***************************************************************************
* Function:	cd6_init
* Input: 	-
* Output        -
* Abstract: 	Initialize the cd6 circuit and related variables that should have
*		a well defined value before the execution starts
* Decisions:	-
*/
void cd6_init(void)
{
   cd6_wr(SPEED_CONTROL_N1);   /* default settings for n=1 */
   cd6_wr(DAC_OUTPUT_MODE);
   cd6_wr(MOT_OUTPUT_MODE);
   cd6_wr(EBU_OUTPUT_MODE);
   cd6_wr(MOT_GAIN_12CM_N1);
   cd6_wr(MOT_BANDWIDTH);
}



/***************************************************************************
* Function:	start_subcode_reading
* Input: 	-
* Output        -
* Abstract: 	enables subcode module for subcode reading
* Decisions:	-
*/
void start_subcode_reading(void)
{
   subcode_reading = TRUE;
   new_subcode_request = TRUE;
   scor_edge=FALSE;
}

/***************************************************************************
* Function:	give_peak_level_low
* Input: 	-
* Output        -
* Abstract: 	return measured value low
* Decisions:	-
*/
byte give_peak_level_low(void)
{
   return peak_level_low;
}

/***************************************************************************
* Function:	give_peak_level_high
* Input: 	-
* Output        -
* Abstract: 	return measured value high
* Decisions:	-
*/
byte give_peak_level_high(void)
{
   return peak_level_high;
}

/***************************************************************************
* Function:	stop_subcode_reading
* Input: 	-
* Output        -
* Abstract: 	disables subcode module for subcode reading
* Decisions:	-
*/
void stop_subcode_reading(void)
{
   subcode_reading = FALSE;
}


/***************************************************************************
* Function:	is_subcode
* Input: 	mode -> 0=all_subcodes, 1=abs_time, 2=catalog_nr,
*			3=isrc_nr, 4=first_leadin_area, 5=leadin_area,
*                       6=program_area, 7=leadout_area
* Output        CY=0 -> not requested subcode in Q_buffer
* 		CY=1 -> requested subcode in Q_buffer
* Abstract: 	this function checks if the subcode stored in Q_buffer is
*		the requested subcode
* Decisions:	-
*/
bit is_subcode(byte mode)
{
   if (new_subcode_request) return FALSE;
   switch (mode)
   {
   case ALL_SUBCODES:      return TRUE;
   case ABS_TIME:          if ((byte)(Q_buffer.conad & 0x0F)==0x01)
			   {
			      if (Q_buffer.tno!=0) return TRUE;
			      else
			      {
				 if (Q_buffer.r_time.min > 90 || cd_disc)
				    return FALSE;
				 else
				    return TRUE;
			      }
			   }
			   return FALSE;
   case CATALOG_NR:        if ((byte)(Q_buffer.conad & 0x0F)==0x02)
			      return TRUE;
			   else
			      return FALSE;
   case ISRC_NR:           if ((byte)(Q_buffer.conad & 0x0F)==0x03)
			      return TRUE;
			   else
			      return FALSE;
   case FIRST_LEADIN_AREA: if ((byte)(Q_buffer.conad & 0x0F)==0x01 && Q_buffer.tno==0)
			   {
			      if (Q_buffer.r_time.min > 90 || cd_disc)
				 return TRUE;
			   }
			   return FALSE;
   case LEADIN_AREA:       if ((byte)(Q_buffer.conad & 0x03)==0x01 && Q_buffer.tno==0)
			      /* address=1 or address=5 */
			      return TRUE;
			   else
			      return FALSE;
   case PROGRAM_AREA:      if ((byte)(Q_buffer.conad & 0x0F)==0x01 && Q_buffer.tno!=0 && Q_buffer.tno!=0xAA)
			      return TRUE;
			   else
			      return FALSE;
   case LEADOUT_AREA:      if ((byte)(Q_buffer.conad & 0x0F)==0x01 && Q_buffer.tno==0xAA)
			      return TRUE;
			   else
			      return FALSE;
   }
}


/***************************************************************************
* Function:	move_abs_time
* Input: 	p = pointer to struct time where the absolute time
*                   should be copied to
* Output        -
* Abstract: 	this function copies the current absolute time to
*               the requested location
* Decisions:	before this function is called a check should be done if the
*               current subcode buffer contains absolute time information
*               (with function: is_subcode(ABS_TIME) )
*/
void move_abstime(struct time *p)
{
   if (Q_buffer.tno != 0)
   {
      p->min = Q_buffer.a_time.min;
      p->sec = Q_buffer.a_time.sec;
      p->frm = Q_buffer.a_time.frm;
   }
   else
   {
      p->min = Q_buffer.r_time.min;
      p->sec = Q_buffer.r_time.sec;
      p->frm = Q_buffer.r_time.frm;
   }
}


/***************************************************************************
* Function:	subcode_module
* Input: 	-
* Output        Q_buffer[10]
* Abstract: 	this function checks if CD-6 has new subcode, if so then
*		this subcode is read, if allowed
* Decisions:	-
*/
void subcode_module(void)
{
   if (subcode_reading)
   {
      if (cd6_read_subcode())
      {  /* new subcode available */
	 new_subcode_request = FALSE;
	 i_can_read_subcode = TRUE;

	 /* check which bcd values have to be converted to hex values */
	 if ((byte)(Q_buffer.conad & 0x0F) == 0x01)
	 {  /* address = 1 */
	    if (Q_buffer.tno != 0xAA) Q_buffer.tno = bcd_to_hex(Q_buffer.tno);
	    bcd_to_hex_time(&Q_buffer.r_time,&Q_buffer.r_time);
	    if (Q_buffer.index == 0xA0 || Q_buffer.index == 0xA1)
	       Q_buffer.a_time.min = bcd_to_hex(Q_buffer.a_time.min);
	    else
	    {
	       bcd_to_hex_time(&Q_buffer.a_time,&Q_buffer.a_time);
	       if (Q_buffer.index != 0xA2)
		  Q_buffer.index = bcd_to_hex(Q_buffer.index);
	    }
	    /* update last read absolute time minutes for time_to_brake table */
	    if (Q_buffer.tno != 0x00)
	       hex_abs_min = Q_buffer.a_time.min;
	    else
	       hex_abs_min = 0;
	 }
	 else if ((byte)(Q_buffer.conad & 0x0F) == 0x05)
	 {  /* address = 5 */
	    if (Q_buffer.tno==0 && Q_buffer.index==0xB0 && Q_buffer.r_time.min!=0xFF)
	       /* convert starttime next program area to hex values */
	       bcd_to_hex_time(&Q_buffer.r_time,&Q_buffer.r_time);
	 }
      }
   }
}
