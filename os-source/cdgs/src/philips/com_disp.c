/*  Project	:  Commodore
 * 	Copyright (c) 1992, Philips Consumer Electronics KMG Laser Optics ORD
 *
 * File name    : dispatch.c
 *
 * Description  : This file contains the dispather, which controls COMMO CD-drive.
 *              :
 *              :
 *
 * Decisions    :
 *              :
 *              :
 *
 * History      : Created by: J.J.M.M. Geelen  date: 09 - 03 - '93
 *              :
 *              :
 *
 * Version      : 0.1
 *
 * Status       : Creation phase
 *              :
 *              :
 *
 * Action list  :
 *              :
 *              :
 *
 *
 */

#include "gendef.h"
#include "commo.h"
#include "sts_q_id.h"
#include "cmd_hndl.h"

Byte    suspend_transmit = NO_UPDATE;	    /* save reg. for packets still to send  	  						*/

Byte	prev_cmd_hdler_rdy = TRUE;	    /* default 'previous command' was ready   							*/

Byte	report_for_free_intf_buf = FALSE;   /* 1 -> a new command reported to cmd handler 						*/

Byte	cmd_still_to_report = COMMO_NO_COMMAND;	 /* save reg. for receiving commands 								*/

Byte	acknowledge_update = FALSE;	     /* 1 -> Commo transmit accepted for packet from packet/q/id module.
													 *      Clear update status when status/q/id packet transmitted.
													 */
/*
 * Function name: Request_packet_transmit
 *              :
 *              :
 *
 * Abstract     : This function requests to transmit a packet over the commo interface.
 *		:
 *		:
 *
 * Input        : mode = { NO_UPDATE, STATUS_UPDATE, Q_READY, ID_READY, DISPATCH_STATUS }
 *		:
 *		:
 *
 * Output       :
 *		:
 *		:
 *
 * Return       : transmit accept status
 *		: FALSE -> transmit request not accepted
 *		: TRUE  -> transmit request accepted
 *
 * Pre          : Commo interface initialised
 *		:
 *		:
 *
 * Post         : transmit requested and transmit request status returned
 *		:
 *		:
 */

Byte Request_packet_transmit(Byte mode)
{
   
   switch (mode)
   {
      case NO_UPDATE:
      return (TRUE);

      case STATUS_UPDATE:       if (SEND_STRING(SEND_STRING_COMPLETE, Get_sts_q_id_ptr(), STATUS_PACKET_LENGTH) == COMMO_TRUE)
				{
				   acknowledge_update = TRUE;
				   return (TRUE);
				}
				else
				{
				   return (FALSE);
				}

      case Q_READY:		if (SEND_STRING(SEND_STRING_COMPLETE, Get_sts_q_id_ptr(), Q_PACKET_LENGTH) == COMMO_TRUE)
				{
				    acknowledge_update = TRUE;
				    return (TRUE);
				}
				else
				{
				   return (FALSE);
				}

      case ID_READY:		if (SEND_STRING(SEND_STRING_COMPLETE, Get_sts_q_id_ptr(), ID_PACKET_LENGTH) == COMMO_TRUE)
				{
				   acknowledge_update = TRUE;
				   return (TRUE);
				}
				else
				{
				   return (FALSE);
				}

      default:			return (FALSE);

   } /* End switch */
} /* End Request_packet_transmit */






/*
 * Function name: Dispatcher
 *              :
 *              :
 *
 * Abstract     : This function controls the COMMO drive. Its responsible for
 *		: detecting and reporting of commands, for all transmits over the commo interface
 *		:
 *
 * Input        :
 *		:
 *		:
 *
 * Output       :
 *		:
 *		:
 *
 * Return       :
 *		:
 *		:
 *              
 * Pre          :
 *   	        :
 *		:
 *
 * Post         :
 *		:
 *		:
 */


void Dispatcher(void)
{
   /* check first if a packet transmit is suspended */

   if (SEND_STRING_READY() <= COMMO_READY_WITH_ERROR)
   {
      /* previous transmit ready with or without error */

      if (acknowledge_update)
      {
	/* packet transmitted and came from status/q/id module => clear update */

	Clear_update();
	suspend_transmit = NO_UPDATE;
	acknowledge_update = FALSE;
      }
      else
      {
	 if (suspend_transmit != NO_UPDATE)
	 {
	    /* a packet transmit pending */

	    if (Request_packet_transmit(suspend_transmit) == TRUE)
	    {
	       /* packet transmit request accepted and 'acknowledge_update' will be
		* set if a packet will be send from status/q/id module
	       */
	    }
	 }
      }
   }

   /* check for command ready and status changes */

   if (	(suspend_transmit == NO_UPDATE) &&
	(Get_update_status() != NO_UPDATE)
      )
   {
      /* no packet transmits pending and there is a status update */

      suspend_transmit = Get_update_status();

   }

   /* check for release commo interface command buffer */

   if ( (report_for_free_intf_buf) && (Cmd_acception_status() == TRUE) )
   {
      /*  command reported and command handler can accept new comamnd */

      if (FREE_CMD_BUFFER() == COMMO_TRUE)
      {
	 /* release commo interface command buffer accepted */

	 report_for_free_intf_buf = FALSE;
      }
   }

   /* check for new command received from commo interface */

   if (cmd_still_to_report == COMMO_NO_COMMAND)
   {
      /* no command to report */

      cmd_still_to_report = NEW_CMD_RECEIVED();
   }

   switch (cmd_still_to_report)
   {
      case COMMO_NO_COMMAND:
      break;

      case COMMO_NEW_COMMAND:	/* report command to command handler */

				if (New_command() == TRUE)
				{
				   /* report of new command accepted */

				   prev_cmd_hdler_rdy = FALSE;					/* indicate command in execution 	   */
				   cmd_still_to_report = COMMO_NO_COMMAND;		/* no new-command to report 		   */
				   report_for_free_intf_buf = TRUE;			/* indicate that cmd has been reported */
				}
      break;

      case COMMO_SAME_COMMAND:	if (Get_update_status() == NO_UPDATE)
				{
				   /* its allowed to store in status/q/id packet buffer */
				   Store_command(GET_BUFFER(0));
				   Store_update_status(STATUS_UPDATE);
				   suspend_transmit = STATUS_UPDATE;
				   report_for_free_intf_buf = TRUE;		 /* commo interface cmd-buffer may released */
				   cmd_still_to_report = COMMO_NO_COMMAND;		 /* no new-command to report 		   */
				}
      break;

      case COMMO_CMD_ERROR:	if (Get_update_status() == NO_UPDATE)
				{
				   /* its allowed to store in status/q/id packet buffer */
				   Store_command(GET_BUFFER(0));
				   Store_error_condition(CHECKSUM_ERROR);
				   suspend_transmit = STATUS_UPDATE;
				   report_for_free_intf_buf = TRUE;		/* commo interface cmd-buffer may released */
				   cmd_still_to_report = COMMO_NO_COMMAND;		/* no new-command to report 	      */
				}
      break;

   } /* End switch */
} /* End Dispatcher */



