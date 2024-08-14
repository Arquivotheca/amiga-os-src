/* 		            Commodore
 *			    --------
 * 
 * Copyright Philips Consumer Electronics / Key Modules Group - hORDe
 * All rights  are reserved. Reproduction in whole or part is prohibited 
 * without the prior written consent of the copyright owner
 *
 * Company Confidential
 *
 * File		: sts_q_id.c
 *
 *
 * Rev		Date			Author Comments
 * 01	 	930311 PCM vd Zande.    Creation
*/
#include "gendef.h"
#include "sts_q_id.h"

static idat Byte		update_status;
static idat Byte		last_update_status;
static idat Byte		status_q_id_array[STATUS_Q_ID_ARRAY_SIZE];




/***********************************************************************************************
 * Function  : Store_command
 * Input     : command
 * Output    : TRUE FALSE
 *
 * Abstract  : try to store command in status_q_id_array[0]
 *
 *
 * Decisions :
*/
Byte  Store_command(Byte command)
{
   if (update_status == NO_UPDATE)
   {
      status_q_id_array[0]=command;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Store_status
 * Input     : status
 * Output    : TRUE FALSE
 *
 * Abstract  : try to store status in status_q_id_array[1]
 *
 *
 * Decisions :
*/
Byte  Store_status(Byte status)
{
   if (update_status == NO_UPDATE)
   {
      status_q_id_array[1]=status;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Store_q_id
 * Input     : position; value
 * Output    : TRUE FALSE
 *
 * Abstract  : try to store value in status_q_id_array[position]
 *
 *
 * Decisions :
*/
Byte  Store_q_id(Byte position, Byte value)
{
   if (update_status == NO_UPDATE)
   {
      status_q_id_array[position]=value;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Get_command
 * Input     : -
 * Output    : status_q_id_array[0]
 *
 * Abstract  : return value of status_q_id_array[0]
 *
 *
 * Decisions :
*/
Byte Get_command(void)
{
   return status_q_id_array[0];
}

/***********************************************************************************************
 * Function  : Get_status
 * Input     : -
 * Output    : status_q_id_array[1]
 *
 * Abstract  : return value of status_q_id_array[1]
 *
 *
 * Decisions :
*/
Byte Get_status(void)
{
   return status_q_id_array[1];
}

/***********************************************************************************************
 * Function  : Get_q_id
 * Input     : position
 * Output    : status_q_id_array[position]
 *
 * Abstract  : return value of status_q_id_array[position]
 *
 *
 * Decisions :
*/
Byte Get_q_id(Byte position)
{
   return status_q_id_array[position];
}

/***********************************************************************************************
 * Function  : Clear_update
 * Input     : -
 * Output    : -
 *
 * Abstract  : update_status = set to NO_UPDATE
 *
 *
 * Decisions :
*/
void Clear_update(void)
{
   update_status=NO_UPDATE;
}

/***********************************************************************************************
 * Function  : Get_update_status
 * Input     : -
 * Output    : update_status
 *
 * Abstract  : return present value of update_status
 *
 *
 * Decisions :
*/
Byte Get_update_status(void)
{
   return update_status;
}

/***********************************************************************************************
 * Function  : Get_last_status_update
 * Input     : -
 * Output    : value of last update (STATUS_UPDATE Q_READY, ID_READY)p
 *
 * Abstract  : return present value of last_update_status
 *
 *
 * Decisions :
*/
Byte Get_last_status_update(void)
{
   return update_status;
}
/***********************************************************************************************
 * Function  : Get_sts_q_id_ptr
 * Input     : -
 * Output    : pointer pointing to status_q_id_array
 *
 * Abstract  : return that pointer
 *
 *
 * Decisions :
*/
Byte *Get_sts_q_id_ptr(void)
{
   return status_q_id_array;
}

/***********************************************************************************************
 * Function  : Store_update_status
 * Input     : status (NO_UPDATE , Q_READY, STATUS_UPDATE, ID_READY)
 * Output    : TRUE FALSE
 *
 * Abstract  : update update_status
 *
 *
 * Decisions :
*/
Byte Store_update_status(Byte status)
{
   if (update_status == NO_UPDATE)
   {
      update_status=status;
      if (status != NO_UPDATE)
	 last_update_status=status;
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Store_cmd_progress
 * Input     : prog e.g . SPINNING_UP
 * Output    : TRUE FALSE
 *
 * Abstract  : store progr in progrss part of status Byte of status_q_id_array
 *
 *
 * Decisions :
*/
Byte Store_cmd_progress(Byte prog)
{
   if (update_status == NO_UPDATE)
   {
      STORE_CMD_PROGRESS(status_q_id_array[1], prog);
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Store_error_condition
 * Input     : err e.g . FOCUS_ERROR
 * Output    : TRUE FALSE
 *
 * Abstract  : store err in error part of status Byte of status_q_id_array
 *
 *
 * Decisions :
*/
Byte Store_error_condition(Byte err)
{
   if (update_status == NO_UPDATE)
   {
      STORE_ERROR_COND(status_q_id_array[1], err);
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Store_drive_status
 * Input     : status e.g . PLAY
 * Output    : TRUE FALSE
 *
 * Abstract  : store err in error part of status Byte of status_q_id_array
 *
 *
 * Decisions :
*/
Byte Store_drive_status(Byte stat)
{
   if (update_status == NO_UPDATE)
   {
      STORE_DRV_STATUS(status_q_id_array[1], stat);
      return TRUE;
   }
   return FALSE;
}

/***********************************************************************************************
 * Function  : Init_sts_q_id
 * Input     : -
 * Output    : -
 *
 * Abstract  : clear status_q_id_array from 0 till STATUS_Q_ID_ARRAY_SIZE
 *
 *
 * Decisions :
*/
void Init_sts_q_id(void)
{
   Byte 	teller;

   teller=0;
   while (teller < STATUS_Q_ID_ARRAY_SIZE)
   {
      status_q_id_array[teller]=0;
      teller++;
   }
}
