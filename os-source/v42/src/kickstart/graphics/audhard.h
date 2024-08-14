/******************************************************************************
*
*	Source Control
*	--------------
*	$Id: audhard.h,v 37.0 91/03/01 12:04:44 chrisg Exp $
*
*	$Locker:  $
*
*	$Log:	audhard.h,v $
*    Revision 37.0  91/03/01  12:04:44  chrisg
*      this file mysteriously dissapeared from the server RCS ??????
*    
*   Revision 37.0  91/01/07  15:13:09  spence
*   initial switchover from V36
*   
*   Revision 33.3  90/07/27  16:25:47  bart
*   *** empty log message ***
*   
*   Revision 33.2  90/03/28  09:36:46  bart
*   *** empty log message ***
*   
*   Revision 33.1  89/05/02  09:28:42  bart
*   *** empty log message ***
*   
*   Revision 33.0  86/05/17  14:57:51  bart
*   added to rcs for updating
*   
*
******************************************************************************/


#ifndef _AUDHARD_DEF
#define _AUDHARD_DEF
struct AudHard {
    WORD *start;    /* ptr to start of waveform data */
    UWORD length;   /* length of waveform in words */
    UWORD period;   /* sample period */
    WORD volume;    /* volume */
    WORD sample;    /* sample pair */
    LONG dummy;     /* unused */
};
#endif
