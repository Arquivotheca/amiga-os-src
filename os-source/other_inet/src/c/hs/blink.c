/* -----------------------------------------------------------------------
 * blink.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: blink.c,v 1.1 91/05/09 16:11:20 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/blink.c,v 1.1 91/05/09 16:11:20 bj Exp $
 *
 * $Log:	blink.c,v $
 * Revision 1.1  91/05/09  16:11:20  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*

        BLINK.C -- AmigaVT blink task.

*/

#include "termall.h"

void  __saveds BlinkTask ()
  {
    register unsigned short int  color;
    register unsigned long  int  signals,
                                 mask;

    blink1_task_port = CreatePort ( /*"BLINK1 TASK PORT"*/ NULL, 0 );

    blink1_port = InitTimer ( &blink1_req, "Blink1 timer" );

    mask = ( 1L << blink1_task_port->mp_SigBit )
         | ( 1L << blink1_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit );

    for ( ;; )
      {
        color = GetRGB4 ( VPort->ColorMap, (long) vcb.bgcolor );

        blink1_req.tr_time.tv_secs  = 0;
        blink1_req.tr_time.tv_micro = 300000;
        blink1_req.tr_node.io_Command = TR_ADDREQUEST;
        SendIO ( (struct IORequest *) &blink1_req );
        signals = Wait ( mask );
        if ( signals & ( 1L << blink1_task_port->mp_SigBit ) )
            break;
        WaitIO ( (struct IORequest *) &blink1_req );

        if ( Screen->BitMap.Depth == 2 )
            SetRGB4 ( VPort, ( 1L << Screen->BitMap.Depth ) - 1,
                    (long)color >> 8,
                          ( color >> 4 ) & 0xfL,
                            color & 0xfL );
        color = GetRGB4 ( VPort->ColorMap, (long) vcb.fgcolor );

        blink1_req.tr_time.tv_secs  = 0;
        blink1_req.tr_time.tv_micro = 300000;
        blink1_req.tr_node.io_Command = TR_ADDREQUEST;
        SendIO ( (struct IORequest *) &blink1_req );
        signals = Wait ( mask );
        if ( signals & ( 1L << blink1_task_port->mp_SigBit ) )
            break;
        WaitIO ( (struct IORequest *) &blink1_req );

        if ( Screen->BitMap.Depth == 2 )
            SetRGB4 ( VPort, ( 1L << Screen->BitMap.Depth ) - 1,
                    (long) color >> 8,
                          ( color >> 4 ) & 0xfL,
                            color & 0xfL );
      }
    AbortIO ( (struct IORequest *) &blink1_req   );
    CloseDevice ( (struct IORequest *) &blink1_req   );
    DeletePort  ( blink1_port   );

    ReplyMsg ( (struct Message *) GetMsg ( blink1_task_port ) );

    DeletePort ( blink1_task_port );

    RemTask ( 0 );
  }

