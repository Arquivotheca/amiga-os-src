/*  :ts=8 bk=0
 *
 * screensaver.h:	Public definitions for the screen saver.
 *
 * Leo L. Schwab					9101.26
 ***************************************************************************
 *	This information is CONFIDENTIAL and PROPRIETARY		   *
 *	Copyright 1991, Silent Software Incorporated.			   *
 *	All Rights Reserved.						   *
 ***************************************************************************
 */

/*
 * The name of the screen saver's port.  FindPort() this port and post
 * messages to it.
 */
#define	SAVEPORTNAME	"CDTV Screen Saver"

/*
 * The commands you can send to the screen saver.  These commands are
 * placed in a standard Message structure in the mn_Length field.  The
 * result is returned in the same place.
 */
#define	SCRSAV_DIE	0	/*  Kill off screen saver task.		*/
#define	SCRSAV_SAVE	1	/*  Start screen saver now.		*/
#define	SCRSAV_UNSAVE	2	/*  Turn off screen saver now.		*/
#define	SCRSAV_FAKEIT	3	/*  Don't actually blank the screen.	*/
#define	SCRSAV_UNFAKEIT	4	/*  Behave normally.			*/
#define	SCRSAV_ISACTIVE	5	/*  Is the saver active?		*/

/*
 * If this bit is set in mn_Length, then the lower 15 bits of mn_Length
 * are interpreted as the new time delay until the screen saver kicks in,
 * expressed in seconds.  If this bit is clear, then mn_Length is
 * interpreted as above.
 */
#define	SCRSAV_TIMECMD	0x8000
