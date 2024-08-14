/* ======================================================================== *
 *  Arrows.h -- a BOOPSI class implementing increment / decrement arrows
 *                             By Talin.
 *              Copyright © 1991 Sylvan Technical Arts
 * ======================================================================== */

#define ARROW_Min			(TAG_USER+1)
#define ARROW_Max			(TAG_USER+2)
#define ARROW_Current		(TAG_USER+3)
#define ARROW_IncreaseRate	(TAG_USER+4)
#define ARROW_MaxRate		(TAG_USER+5)

	/* output-only tags */

#define ARROW_State			(TAG_USER+7)

#define ARROWSTATE_NONE		0
#define ARROWSTATE_DEC		1
#define ARROWSTATE_INC		2
