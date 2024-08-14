/*  :ts=8 bk=0
 *
 * joymouse.h:	public definition for joymouse routines
 *
 * William A. Ware					B215
 ***************************************************************************
 *	This information is CONFIDENTIAL and PROPRIETARY		   *
 *	Copyright 1991, Silent Software Incorporated.			   *
 *	All Rights Reserved.						   *
 ***************************************************************************
 */

/*
 * The name of the joymouse's port.  FindPort() this port and post
 * messages to it.
 */
#define	JOYMOUSEPORTNAME	"CDTV JoyMouse"

/*
 * The commands you can send to the JoyMouse.  These commands are
 * placed in a standard Message structure in the mn_Length field.  The
 * result is returned in the same place.
 */
#define	JOYMSE_DIE	0	/*  Kill off JoyMouse task.		*/
#define	JOYMSE_OFF	1	/*  Deactivate the JoyMouse for a time  */
#define	JOYMSE_ON	2	/*  Reactivate the JoyMosue		*/
