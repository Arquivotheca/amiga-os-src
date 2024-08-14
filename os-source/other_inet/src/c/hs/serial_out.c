/* -----------------------------------------------------------------------
 *  serial_out.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: serial_out.c,v 1.1 91/05/09 16:30:43 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/serial_out.c,v 1.1 91/05/09 16:30:43 bj Exp $
 *
 * $Log:	serial_out.c,v $
 * Revision 1.1  91/05/09  16:30:43  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Serial output subtask code.
*
***/

#include    "termall.h"

/***
*
* Serial Output Subtask
*
***/

void __saveds SerialOutput ()
  {
    register VT_message         *recvd_msg;

    register unsigned short int io_queued;

    register long int           mask,
                                outputmask = 0,
                                signals;

    /* 
    * Initialize task message port.
    */
    so_task_port = CreatePort ( /*"SO TASK PORT"*/ NULL, 0 );
    so2main_msg.vt_msg.mn_Node.ln_Type = NT_MESSAGE;
    so2main_msg.vt_msg.mn_Length       = sizeof ( unsigned short int );
    so2main_msg.vt_msg.mn_ReplyPort    = so_task_port;

    /*
    * Initialize devices.
    */
    ser_out_port = CreatePort ( /*"ser_out"*/ NULL, 0 );
    ser_out_req.IOSer.io_Message.mn_ReplyPort = ser_out_port;

    if ( Screen )
      {
        con_in_port  = InitConsoleIO ( &con_in_req,  "con_in"  );
        con_kb_port  = CreatePort ( /*"con_kb"*/ NULL, 0 );
        con_kb_req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
        con_kb_req.io_Message.mn_Node.ln_Pri  = 1;
        con_kb_req.io_Message.mn_ReplyPort = con_kb_port;
        con_kb_req.io_Device = con_in_req.io_Device;
        con_kb_req.io_Unit   = con_in_req.io_Unit;
      }

    so_timeout_port = InitTimer ( &so_timeout_req, "Output timer" );

    if ( Screen )
        outputmask   = 1L << con_in_req.io_Message.mn_ReplyPort->mp_SigBit;
    timeoutmask  = 1L << so_timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    serialmask   = 1L << ser_out_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask         = outputmask | ( 1L << so_task_port->mp_SigBit );


    if ( Screen )
      {
        SaveOriginalKeyMap ();
        SetKeyboard ();
      }

    so2main_msg.opcode = SO_SUBTASK_AWAKE;
    PutMsg ( main_task_port, (struct Message *) &so2main_msg );

    for ( io_queued = 0;;)
      {
        if ( Screen && !io_queued )
          {
            con_in_req.io_Command = CMD_READ;
            con_in_req.io_Length  = 1;
            con_in_req.io_Data    = (APTR) ser_out_buffer;
            SendIO ( (struct IORequest *) &con_in_req );
            io_queued = 1;
          }

        signals = Wait ( mask );

        if ( signals &  ( 1L << so_task_port->mp_SigBit ) )
          {
            recvd_msg = (VT_message *) GetMsg ( so_task_port );
            switch ( recvd_msg->opcode )
              {
                case SER_OUT:
                    ShutDownOutput ();
                    return;
                case SER_OUT_SLEEP:
                    So_asleep = 1;
                    recvd_msg->opcode = SO_SUBTASK_ASLEEP;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    WaitPort ( so_task_port );
                    recvd_msg = (VT_message *) GetMsg ( so_task_port );
                    if ( recvd_msg->opcode == SER_OUT )
                      {
                        ShutDownOutput ();
                        return;
                      }
                    recvd_msg->opcode = SO_SUBTASK_AWAKE;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    So_asleep = 0;
                    break;
                case SER_OUT_SET_KEYBOARD:
                    SetKeyboard ();
                    break;
                case SER_OUT_SWITCH_KEYBOARD:
                    InitKeyboard ();
                    recvd_msg->opcode = KEYBOARD_SWITCHED;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    break;
              }
          }

        if ( signals & outputmask )
          {
            if ( GetMsg ( con_in_req.io_Message.mn_ReplyPort )
                 && tmodes.kam == RESET
                 && !IDCMP_in_progress )
                Lputc ( *ser_out_buffer );
            io_queued = 0;
          }
      }
  }

/***
*
* Shutdown Output
*
***/
void ShutDownOutput ()
  {
    if ( Screen )
      {
        AbortIO ( (struct IORequest *) &con_in_req );
        WaitIO  ( (struct IORequest *) &con_in_req );
        CloseDevice ( (struct IORequest *) &con_in_req );
        DeletePort  ( con_in_port    );
        DeletePort  ( con_kb_port    );
        con_in_port = NULL;
        con_kb_port = NULL;
      }

    ser_out_req.IOSer.io_Message.mn_ReplyPort = NULL;
    DeletePort  ( ser_out_port   );
    ser_out_port = NULL;

    CloseDevice ( (struct IORequest *) &so_timeout_req );
    DeletePort  ( so_timeout_port );
    so_timeout_port = NULL;

    DeletePort  ( so_task_port    );
    so_task_port = NULL;
  }

/*----------------------------------------------------------------------------*/

/***
*
* Output a character to the serial port
*
***/
void Lputc ( c )
unsigned char   c;
  {
    register unsigned long int  signals;
    register unsigned char      sent_ok;
    static unsigned char        ch[256];
    long int                    length = 1;

    *ch = c; /* Get character on an even boundary. */

    /*
    length = XprUserMon ( ch, 1, 256 );
    */

    ser_out_req.IOSer.io_Command = CMD_WRITE;
    ser_out_req.IOSer.io_Data    = (APTR) ch;
    ser_out_req.IOSer.io_Flags   &= ~IOF_QUICK;
    ser_out_req.IOSer.io_Length  = length;
    SendIO ( (struct IORequest *) &ser_out_req );

    sent_ok = 0;
    do
      {
        StartTimer ( &so_timeout_req, 5 );

        signals = Wait ( timeoutmask | serialmask );

        if ( signals & serialmask )
          {
            if ( !( signals & timeoutmask ) )
              {
                AbortIO ( (struct IORequest *) &so_timeout_req );
                Wait    ( timeoutmask );
              }
            GetMsg  ( so_timeout_port );
            GetMsg  ( ser_out_port );
            sent_ok = 1;
          }
        else
          {
            So_asleep = 1;
            GetMsg  ( so_timeout_port );
            
            AbortIO ( (struct IORequest *) &ser_out_req );
            Wait    ( serialmask );
            GetMsg  ( ser_out_port );
            
            ser_out_req.IOSer.io_Command = CMD_FLUSH;
            ser_out_req.IOSer.io_Data    = NULL;
            ser_out_req.IOSer.io_Flags  &= ~IOF_QUICK;
            DoIO ( (struct IORequest *) &ser_out_req );
            
            ser_out_req.IOSer.io_Command = CMD_START;
            ser_out_req.IOSer.io_Data    = NULL;
            ser_out_req.IOSer.io_Flags  &= ~IOF_QUICK;
            DoIO ( (struct IORequest *) &ser_out_req );

            ser_out_req.IOSer.io_Command = CMD_WRITE;
            ser_out_req.IOSer.io_Data    = (APTR) ch;
            ser_out_req.IOSer.io_Flags  &= ~IOF_QUICK;
            ser_out_req.IOSer.io_Length  = 1;
            SendIO ( (struct IORequest *) &ser_out_req );
          }
      }
    while ( !sent_ok );

    So_asleep = 0;
    if ( nvmodes.echo == SET )
      {
        local_echo_char = *ch;
        so2main_msg.opcode = ECHO_CHARACTER;
        PutMsg ( main_task_port, (struct Message *) &so2main_msg );
        WaitPort ( so2main_msg.vt_msg.mn_ReplyPort );
        GetMsg ( so2main_msg.vt_msg.mn_ReplyPort );
      }
    if ( *ch == '\r' && nvmodes.lnm == SET )
        Lputc ( '\n' );
  }

/*----------------------------------------------------------------------------*/



