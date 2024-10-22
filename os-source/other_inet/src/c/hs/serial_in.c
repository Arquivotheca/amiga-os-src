/* -----------------------------------------------------------------------
 * serial_in.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: serial_in.c,v 1.1 91/05/09 16:31:38 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/serial_in.c,v 1.1 91/05/09 16:31:38 bj Exp $
 *
 * $Log:	serial_in.c,v $
 * Revision 1.1  91/05/09  16:31:38  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Serial input subtask
*
***/

#include    "termall.h"
#include    "rexx/storage.h"

static struct GetSMsg             *RexxGetStrMsg    = NULL;
static struct timerequest          RexxTimeout_req;
static struct MsgPort             *RexxTimeoutPort  = NULL;
static unsigned char               MatchBuffer[41];
static unsigned short int          MatchIDX         = 0;

/***
*
* Serial input subtask code.
*
***/

void __saveds SerialInput ()
  {
    unsigned char           *lgets();
    register unsigned char  *str;

    /*
    * Initialize task message port.
    */
    si_task_port = CreatePort ( /*"SI TASK PORT"*/ NULL, 0 );
    si2main_msg.vt_msg.mn_Node.ln_Type = NT_MESSAGE;
    si2main_msg.vt_msg.mn_Length       = sizeof ( unsigned short int );
    si2main_msg.vt_msg.mn_ReplyPort    = si_task_port;

    /*
    * Initialize devices.
    */
    ser_in_port = CreatePort ( /*"ser_in"*/ NULL, 0 );
    ser_in_req.IOSer.io_Message.mn_ReplyPort = ser_in_port;
    
    ser_rp_port = CreatePort ( /*"ser_rp"*/ NULL, 0 );
    ser_rp_req.IOSer.io_Message.mn_ReplyPort = ser_rp_port;

    inputmask = 1L << ser_in_port->mp_SigBit;
    IDCMPmask = 1L << si_task_port->mp_SigBit;

    mask      = IDCMPmask | inputmask;

    si2main_msg.opcode = SI_SUBTASK_AWAKE;
    PutMsg ( main_task_port, (struct Message *) &si2main_msg );

    for (;;)
      {
        str = Lgets ();
        while ( *str ) VtDspCh ( *str++ );
      }
  }

/*----------------------------------------------------------------------------*/
/***
*
* Put a byte to the console.
*
***/
void Vputc ( c )
register unsigned char   c;
  {
   VsetCAttr ( c );
   scr_out_buffer[scr_out_count++] = c;
    if ( scr_out_count == scr_out_limit )
        ScrFlush ();
  }

/*----------------------------------------------------------------------------*/
/***
*
* Flush the screen output buffer
*
***/

void ScrFlush ()
  {
    if ( scr_out_count )
      {
        Text ( RPort, scr_out_buffer, (unsigned long) scr_out_count );
        scr_out_count = 0;
      }
  }
/*----------------------------------------------------------------------------*/

/***
*
* Get a byte from the serial port.
*
***/
unsigned char Lgetc ()
  {
    static unsigned short int   io_queued = 0;
    static unsigned char        ser_buffer [1536];
    static unsigned char        *chr_ptr = ser_buffer,
                                *end_ptr = ser_buffer;
    register unsigned char      lechr;
    register unsigned long int  signals;
    register VT_message         *recvd_msg;
    register struct IOStdReq    *ser_in_req_ptr = &ser_in_req.IOSer;
    register unsigned short int home_cursor_copy;
    unsigned long int           error;

    for ( ;; )
      {
        if ( !Si_asleep )
          {
            if ( chr_ptr < end_ptr )
              {
                lechr = *chr_ptr++;
                if ( nvmodes.decanm <= VT100 )
                    lechr &= '\x7f';
                Capture ( lechr );
                if ( RexxGetStrMsg )
                    RexxMatch ( (char)(lechr /* & '\x7f' */) );
                return  ( lechr );
              }
          }

        if ( !io_queued )
          {
            do
              {
                ser_in_req_ptr->io_Command = SDCMD_QUERY;
                if ( error = DoIO ( (struct IORequest *) &ser_in_req )
                     || ser_in_req_ptr->io_Error )
                  {
                    if ( ser_in_req_ptr->io_Error )
                        error = ser_in_req_ptr->io_Error;

		    MakeBeep ( 1000, 64 );
                    /*
                    CloseDevice ( (struct IORequest *) &ser_in_req   );
                    DeletePort  ( ser_in_port   );
                    ser_in_port = InitSerialIO ( &ser_in_req, "ser_in" );
                    */
                    /*???*/
                  }
              } while ( error );

            ser_in_req_ptr->io_Length  =
                                ser_in_req_ptr->io_Actual ?
                                ser_in_req_ptr->io_Actual :
                                1;
            ser_in_req_ptr->io_Data    = (APTR) ser_buffer;
            ser_in_req_ptr->io_Command = CMD_READ;
            ser_in_req_ptr->io_Flags   &= ~IOF_QUICK ;
            SendIO ( (struct IORequest *) &ser_in_req );
            end_ptr = chr_ptr = ser_buffer;
            io_queued = 1;
          }

        ScrFlush ();
        PlaceCursor ();
        
        signals = Wait ( mask );
        
        RemoveCursor ();

        if ( signals & inputmask )
          {
            GetMsg ( ser_in_port );
            io_queued = 0;
            /*
            ser_in_req_ptr->io_Actual = XprHostMon ( ser_buffer,
                                                     ser_in_req_ptr->io_Actual,
                                                     1536 );
            */
            if ( ser_in_req_ptr->io_Actual
              == ser_in_req_ptr->io_Length
              && !ser_in_req_ptr->io_Error )
                end_ptr = ser_buffer + ser_in_req_ptr->io_Actual;
            else
              {
                /*
                CloseDevice ( (struct IORequest *) &ser_in_req   );
                DeletePort  ( ser_in_port   );
                ser_in_port = InitSerialIO ( &ser_in_req, "ser_in" );
                */
		MakeBeep ( 1000, 64 );

                /*???*/
              }
          }
        
        if ( signals & IDCMPmask )
          {
            recvd_msg = (VT_message *) GetMsg ( si_task_port );
            switch ( recvd_msg->opcode )
              {
                case SER_IN:
                    if ( io_queued )
                      {
                        if ( !( signals & inputmask ) )
                          {
                            AbortIO ( (struct IORequest *) &ser_in_req );
                            Wait ( inputmask );
                          }
                        GetMsg  ( ser_in_port );
                        io_queued = 0;
                      }
    
                    ExitLabel:

                    ser_in_req.IOSer.io_Message.mn_ReplyPort = NULL;
                    ser_rp_req.IOSer.io_Message.mn_ReplyPort = NULL;
                    DeletePort  ( ser_in_port   );
                    DeletePort  ( ser_rp_port   );
                    ser_in_port = NULL;
                    ser_rp_port = NULL;

                    DeletePort  ( si_task_port );
                    si_task_port = NULL;

                    RemTask ( 0 );
                    break;
                case SER_IN_SLEEP:
                    Si_asleep = 1;
                    if ( io_queued )
                      {
                        if ( !( signals & inputmask ) )
                          {
                            AbortIO ( (struct IORequest *) &ser_in_req );
                            Wait ( inputmask );
                          }
                        GetMsg  ( ser_in_port );
                        io_queued = 0;
                      }
                    home_cursor_copy = home_cursor;
                    ScrFlush ();
                    recvd_msg->opcode = SI_SUBTASK_ASLEEP;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    WaitPort ( si_task_port );
                    recvd_msg = (VT_message *) GetMsg ( si_task_port );
                    if ( recvd_msg->opcode == SER_IN )
                        goto ExitLabel;

                    if ( home_cursor_copy )
                      {
                        Vcls ( 1, vcb.screenlen );
                        Vlocate (1, 1);
                      }
                    recvd_msg->opcode = SI_SUBTASK_AWAKE;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    scr_out_limit = nvmodes.lspeed / 120;
                    Si_asleep = 0;
                    break;
                case SER_IN_ECHO:
                    lechr = (tmodes.controls == 7 ) ? local_echo_char & '\x7f' :
                                                      local_echo_char;
                    recvd_msg->opcode = ECHO_COMPLETE;
                    ReplyMsg ( (struct Message *) recvd_msg );
                    if ( ! Si_asleep )
                      {
                        if ( nvmodes.decanm <= VT100 )
                            lechr &= '\x7f';
                        Capture ( lechr );
                        if ( RexxGetStrMsg )
                            RexxMatch ( (char)(lechr & '\x7f') );
                            
                        return  ( lechr );
                      }
                    break;
                case REXX_GETSTR:
                    RexxGetStrMsg   = (struct GetSMsg *)recvd_msg;
                    RexxTimeoutPort = InitTimer ( &RexxTimeout_req,
                                       "Rexx timer" );
                    mask           |= 1L << RexxTimeoutPort->mp_SigBit;
                    StartTimer ( &RexxTimeout_req, RexxGetStrMsg->timeout );
                    memset ( MatchBuffer, '\x00', 40 );
                    MatchIDX = 0;
                    break;
              }
          }
        if ( RexxGetStrMsg &&
             signals & ( 1L << RexxTimeoutPort->mp_SigBit ) )
          {
            mask &= ~(1L << RexxTimeoutPort->mp_SigBit);
            WaitIO ( (struct IORequest *) &RexxTimeout_req );
            CloseDevice ( (struct IORequest *) &RexxTimeout_req );
            DeletePort  ( RexxTimeoutPort );
            RexxGetStrMsg->timeout = 0;
            RexxGetStrMsg->rmsg = NULL;
            ReplyMsg ( (struct Message *) RexxGetStrMsg );
            RexxTimeoutPort = NULL;
            RexxGetStrMsg   = NULL;
            memset ( MatchBuffer, '\x00', 40 );
            MatchIDX = 0;
          }
      }
    return ( (unsigned char) '\0' );
  }

/*----------------------------------------------------------------------------*/

void RexxMatch (  char inchar )
  {
    unsigned short int count;
    unsigned short int length;
    static short int   fillcount = 0;
    unsigned char temp[2];
    
    if ( fillcount )
      {
	temp[0] = inchar;
	temp[1] = '\0';
	strcat ( MatchBuffer, temp );
	fillcount--;
	if ( !fillcount )
	  {
            DeletePort  ( RexxTimeoutPort );
            ReplyMsg ( (struct Message *) RexxGetStrMsg );
            RexxTimeoutPort = NULL;
            RexxGetStrMsg   = NULL;
            MatchIDX = 0;
          }
        return;
      }

    if ( MatchIDX < 40 )
        MatchBuffer[ MatchIDX++ ] = inchar;
    else
      {
        memcpy ( MatchBuffer, MatchBuffer + 1, 40 );
        MatchBuffer[ 40 - 1 ] = inchar;
      }
      
    for ( count = 1;
          count <= (RexxGetStrMsg->rmsg->rm_Action & 0xff);
          count++ )
          {
            length = strlen ( RexxGetStrMsg->rmsg->rm_Args[count] );    
            if ( MatchIDX >= length &&
                strcmp ( RexxGetStrMsg->rmsg->rm_Args[count],
                        MatchBuffer + MatchIDX - length ) == 0 )
              { 
                AbortIO ( (struct IORequest *) &RexxTimeout_req );
                WaitIO  ( (struct IORequest *) &RexxTimeout_req );
                mask &= ~(1L << RexxTimeoutPort->mp_SigBit);
                CloseDevice ( (struct IORequest *) &RexxTimeout_req );
                strcpy ( MatchBuffer, (char *)
                    (struct RexxMsg *)RexxGetStrMsg->rmsg->rm_Args[count] );
                RexxGetStrMsg->rmsg = (struct RexxMsg *)MatchBuffer;
	        if ( GetLength )
              {
		        fillcount = GetLength;
		        break;
	          }
	        else
	          {
                DeletePort  ( RexxTimeoutPort );
                ReplyMsg ( (struct Message *) RexxGetStrMsg );
                RexxTimeoutPort = NULL;
                RexxGetStrMsg   = NULL;
                MatchIDX = 0;
                return;
	          }
          }
      }
  }

/*----------------------------------------------------------------------------*/

/***
*
* Get a string from the serial device.
*
***/

unsigned char *Lgets ()
  {
    unsigned char           Lgetc ();
    static unsigned char    s_buffer [ MAXESCSEQ + 1 ];
    register unsigned char  *end_ptr;
    register unsigned char  *str = s_buffer;    
    register unsigned char  c;

    *str = c = Lgetc ();

    *( str + 1 ) = '\0';

    if( c == (unsigned char) ESC ||
        c == (unsigned char) CSI ||
        c == (unsigned char) DCS ||
        c == (unsigned char) SS2 ||
        c == (unsigned char) SS3 )
      {
        ScrFlush ();
        end_ptr = str;
        if ( c == (unsigned char) CSI )
          {
            *end_ptr = (unsigned char) ESC;
            *++end_ptr = (unsigned char) '[';
          }
        else if ( c == (unsigned char) DCS )
          {
            *end_ptr = (unsigned char) ESC;
            *++end_ptr = (unsigned char) 'P';
          }
        else if ( c == (unsigned char) SS2 )
          {
            *end_ptr = (unsigned char) ESC;
            *++end_ptr = (unsigned char) 'N';
          }
        else if ( c == (unsigned char) SS3 )
          {
            *end_ptr = (unsigned char) ESC;
            *++end_ptr = (unsigned char) 'O';
          }
        
        do
          {
            while ( ( c = Lgetc () ) < ESC )
                VtDspCh ( c );
            *++end_ptr   = c;
            *(end_ptr+1) = '\0';
          } while ( !ESCOver (str, end_ptr ) );
          
        if ( s_buffer[1] == 'P' && nvmodes.decanm > VT100 )
          {
            while ( ( c = Lgetc () ) <= 0x7f  && c != ESC )
                if ( c  >= 0x20 )
                    *++end_ptr = c;
                /*
                else
                    VtDspCh ( c );
                */
                    
            if ( c == ST || ( c == ESC && ( c = Lgetc () ) == '\\' ) )
              {
                *(end_ptr+1) = '\0';
                Dcs ( s_buffer );
              }
            else MakeBeep ( 3000L, 64L );
          }
          
        else if ( end_ptr - s_buffer < MAXESCSEQ )
         Escape ( s_buffer );
        *s_buffer = '\0';
      }
    else if ( c == ENQ )
        Respond ( nvmodes.answrbck );

    return ( s_buffer );
  }

