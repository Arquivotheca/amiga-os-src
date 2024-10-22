/*  :ts=8 bk=0
 *
 * keyclick.h:	Public definitions for the key click module.
 *
 * Leo L. Schwab
 ***************************************************************************
 *	This information is CONFIDENTIAL and PROPRIETARY		   *
 *	Copyright 1991, Silent Software Incorporated.			   *
 *	All Rights Reserved.						   *
 ***************************************************************************
 */
/*
 * The message port to which key click commands are sent.
 */
#define	CLICKPORTNAME	"CDTV Key Clicker"

/*
 * These are the commands you can feed to the key click task.  These are
 * placed in the io_Command field of an IOStdReq.
 *
 * Details of command operation are found in the autodoc
 * playerprefs.library/KeyClickCommand.  The parameters map to the IOStdReq
 * fields as follows:
 *
 * cmd		io_Command
 * sample	io_Data
 * length	io_Length
 * rate		io_Actual
 * ncycles	io_Offset
 * volume	io_Flags	; Yeah, I know...
 */
#define	CLKCMD_DIE		0	/*  Terminate key click task.	*/
#define	CLKCMD_DISABLE		1	/*  Disable/enable all clicking.*/
#define	CLKCMD_SETREPEAT	2	/*  Turn repeat clicks on/off.	*/
#define	CLKCMD_SETJOYBEEP	3	/*  Enable/disable joybeep.	*/
#define	CLKCMD_SETVOLUME	4	/*  Set volume of current sound.*/
#define	CLKCMD_SETRATE		5	/*  Set sampling rate of sound.	*/
#define	CLKCMD_CLICK		6	/*  Produce a click now.	*/
#define	CLKCMD_NEWSOUND		7	/*  Replace click sound sample.	*/
#define	CLKCMD_ISDEFAULT	8	/*  Is current sound default?	*/
