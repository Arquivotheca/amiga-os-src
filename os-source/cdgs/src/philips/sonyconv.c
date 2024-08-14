/***************************************************************************
*
* Project:	Commodore
* File:		sonyconv.c
* Date:		30 March 1993
* Status:	Draft
*
* Description:	this module converts CD-6 driver functions into
*               CXD2500 driver functions
*
* Interface functions: static byte time_to_brake(void)
*                      bit status_cd6(byte status_type)
*                      void cd6_wr(byte mode)
*
* Decisions:	-
*
* HISTORY         	AUTHOR COMMENTS
* 04 March 1993		creation H.v.K.
*
***************************************************************************/
#include "defs.h"
#include "serv_def.h"

#define NORMAL_MODE			0x00
#define LEVEL_MODE			0x04
#define PEAK_MODE                       0x08

byte				scor_counter;		/* presetted it counts down falling edges of scor*/
byte  idat			audio_cntrl;		/* shadow register of cxd2500 audio control register*/

extern void cxd2500_wr(byte);
extern void audio_cxd2500(byte);
extern bit simulate_subcode_ready(void);

extern bit 		mute_pin;
extern bit		n1_speed;
extern byte		hex_abs_min; /* for brake table */
extern byte		simulation_timer; /* used to simulate mot_strt_1 and mot_stop signals */


/***************************************************************************/
static byte time_to_brake(void)
{
static byte const brake_table[] = {19,18,18,17,17,16,16,15,14,14,13,13,12,11,11,10,10,9,9,8};

   if (n1_speed) return brake_table[hex_abs_min>>2];
   else return brake_table[hex_abs_min>>2]*2;
}

/***************************************************************************/
byte get_area(void)
{
   if (hex_abs_min < 16) return 1;
   if (hex_abs_min > 32) return 3;
   return 2;
}


/***************************************************************************
 * Function  : init_scor_counter
 * Input     : value to be decremented
 * Output    : 
 * Abstract  : init settings for scor counter interrupt
 * Decisions :
*/
void init_scor_counter(byte count)
{
	IT0=TRUE;
	scor_counter=count;
	scor_counter--;
}
/***************************************************************************
 * Function  : zero_scor_counter
 * Input     : -
 * Output    : 
 * Abstract  : scor_counter == 0 ?
 * Decisions :
*/
bit zero_scor_counter(void)
{
	return scor_counter == 0;
}

/***************************************************************************
 * Function  : increment_scor_counter
 * Input     : -
 * Output    : 
 * Abstract  : scor_counter++
 * Decisions :
*/
void increment_scor_counter(void)
{
	scor_counter++;
}

/***************************************************************************
 * Function  : enable_scor_counter
 * Input     : 
 * Output    : 
 * Abstract  : init settings for scor counter interrupt
 * Decisions :
*/
void enable_scor_counter(void)
{
	EX0=TRUE;
}


/***************************************************************************
 * Function  : status_cd6
 * Input     : status_type -> mot_strt_1, motor_overflow, mot_stop, subcode_ready
 * Output    : CY = 1: status line=1
 *	       CY = 0: status_line=0
 * Abstract  : select requested status cd6; read status pin
 * Decisions :
*/
bit status_cd6(byte status_type)
{
   switch (status_type)
   {
   case MOTOR_OVERFLOW: return 0;
   case MOT_STRT_2:     return 0;
   case MOT_STRT_1:
   case MOT_STOP:       return (simulation_timer==0);
   case SUBCODE_READY:  return simulate_subcode_ready();
   }
}

/***************************************************************************
 * Function  : set_level_meter_mode
 * Input     : mode
 * Output    :
 *
 * Abstract  : set peak level mode in audio register
 * Decisions :
*/
void set_level_meter_mode(byte mode)
{
   audio_cntrl &= 0xF0;
   switch (mode)
   {
   case 0:	audio_cntrl |= NORMAL_MODE;
   break;

   case 1:	audio_cntrl |= LEVEL_MODE;
   break;

   case 2:	audio_cntrl |= PEAK_MODE;
   break;
   }
}


/***************************************************************************/
void cd6_wr(byte mode)
{
   switch (mode)
   {
   case	MOT_OFF_ACTIVE:        cxd2500_wr(0xE0);
			       break;
   case	MOT_BRM1_ACTIVE:       cxd2500_wr(0xEA); /* simulate with brake mode */
			       break;
   case	MOT_BRM2_ACTIVE:       simulation_timer = time_to_brake();
			       cxd2500_wr(0xEA); /* simulate with brake mode */
			       break;
   case	MOT_STRTM1_ACTIVE:     cxd2500_wr(0xE8); /* simulate with kick mode */
			       hex_abs_min = 0; /* clear brake table index */
			       break;
   case	MOT_STRTM2_ACTIVE:     simulation_timer = 600/8; /* simulate time to startup is 600 msec */
			       cxd2500_wr(0xEE); /* simulate with rough servo mode */
			       break;
   case	MOT_JMPM_ACTIVE:
   case	MOT_JMPM1_ACTIVE:      
			       cxd2500_wr(0xEE);  /*simulate with rough servo mode */
			       break;

   case	MOT_PLAYM_ACTIVE:      cxd2500_wr(0xE6);
			       break;
   case	SPEED_CONTROL_N1:      cxd2500_wr(0x99);
			       break;
   case	SPEED_CONTROL_N2:      cxd2500_wr(0x9D);
			       break;
   case	MOT_GAIN_8CM_N1:
   case	MOT_GAIN_12CM_N1:      cxd2500_wr(0xC1);
			       break;
   case	MOT_GAIN_8CM_N2:
   case	MOT_GAIN_12CM_N2:      cxd2500_wr(0xC6);
			       break;

   case	DAC_OUTPUT_MODE:       cxd2500_wr(0x89); /* audio mode=0x81, cd-rom mode=0x89 */
			       break;
   case	MOT_OUTPUT_MODE:       cxd2500_wr(0xD0);
			       break;

   case	EBU_OUTPUT_MODE:       break;


   case MUTE:     	       audio_cntrl |= 0x20;
			       audio_cxd2500(audio_cntrl);
			       mute_pin=1;
			       break;

   case FULL_SCALE:            if ( ( (byte)(audio_cntrl) & 0x20 ) == 0x20)
			       {
				  /*MUTED*/
				  audio_cntrl &= 0xCF;
				  audio_cxd2500(audio_cntrl);
				  mute_pin=0;
			       }
			       break;

   case ATTENUATE:	       audio_cntrl &= 0xCF;
			       audio_cntrl |= 0x10;
			       mute_pin=0;
			       audio_cxd2500(audio_cntrl);
			       break;

   default:                    cxd2500_wr(mode);
   }
}
