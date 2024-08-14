
/* *** selmatic.c ************************************************************
 * 
 * Selectomatic SELECT Generator input handler for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * History     Name        Description
 * ---------   ------      -------------------------------------------------
 * 1 Mar 86    =RJ Mical=  Created this file
 *
 * **************************************************************************/

#include "zaphod.h"					    

Selectomatica()
/* This is an input handler that sides beside the input stream upstream
 * from Intuition.  The purpose of this task is to create an artificial
 * mouse button re-selection of a gadget that the user has just selected,
 * if the user is still selecting that same gadget.  The handler 
 * monitors and records:
 *		- SELECT state
 *		- Time of last DOWN transition
 *		- Copy of last input event seen
 *
 * When this task gets a SelectomaticSignal from outside, it receives a
 * a message with:
 *		- Time of DOWN transition of interest
 *		- Select box of gadget
 *
 * If the SELECT state is DOWN and the time of the current DOWN transition
 * is equal to the SelectomaticMessage time, *and* the cursor is still
 * in a relative locale to the gadget's hit box, then we have achieved
 * Selectomatica.  This task then prepares two events, SELECT_UP followed
 * immediately by SELECT_DOWN, which will re-select the current gadget
 * before anything else happens.
 *
 * For this handler to be honest, it must put incrementally-increasing time
 * values in the two input events it creates.  This is no big deal.
 * The *big* deal comes from the fact that this routine must then verify
 * the next event it sees coming down the stream must have a time value
 * that's higher than the values it artificially creates.  The chances
 * that there will ever be a collision are *extremely* remote, but care
 * must be taken nevertheless.
 */
{
}

