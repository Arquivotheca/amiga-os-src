/* :ts=4
*
*	animation.h
*
*	William A. Ware						D309
*
*****************************************************************************
*   This information is CONFIDENTIAL and PROPRIETARY						*
*   Copyright 1993, Silent Software, Incorporated.							*
*   All Rights Reserved.													*
****************************************************************************/


#define ANIMMSG_SHUTDOWN    0                   /* Title booted, free animation   */
#define ANIMMSG_BOOTING     1                   /* Booting title, start boot anim */
#define ANIMMSG_HOLDOFFANIM 2                   /* Door closed, hold off anim     */
#define ANIMMSG_FALSEALARM  3                   /* No disk to boot, animate again */



void SendAnimMessage(int Msg);
int StartAnimation(void);
