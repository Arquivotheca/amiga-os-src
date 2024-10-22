/* -----------------------------------------------------------------------
 * xmodem.c		handshake_src 
 *
 * $Locker:  $
 *
 * $Id: xmodem.c,v 1.1 91/05/09 15:34:47 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/xmodem.c,v 1.1 91/05/09 15:34:47 bj Exp $
 *
 * $Log:	xmodem.c,v $
 * Revision 1.1  91/05/09  15:34:47  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include "termall.h"

#define BLK_SZ              1536
#define WRITE_BUFFER_SIZE   8192

static unsigned short int   transfer_aborted;
void SendCANs ()
  {
    SendChar ( CAN );
    SendChar ( CAN );
    SendChar ( CAN );
    SendChar ( CAN );
    SendChar ( CAN );
  }

unsigned short int __stdargs BeginXmodemSend ( unsigned char *file )
  {
    static unsigned char        message_buffer[81];
    unsigned char               GetOneByteFromSet ();
    unsigned long int           file_size;
    unsigned short int          transfer_minutes,
                                transfer_seconds;
    register unsigned char      resp_char;
    register unsigned short int transfer_complete = 0;

    if ( ftcb.trans_act                &&
         ftcb.protocol     != XON_XOFF &&
         nvmodes.direction != INCOMING )
        return ( 0 );
    
    SetXmodemParameters ();

    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "File To Send", nvmodes.pc_file ) )
      {
        RestoreParameters ();
        return ( 0 );
      }
    
    if ( -1 == ( ftcb.fh = _dopen ( nvmodes.pc_file, MODE_OLDFILE ) ) )
      {
        Error ( "File Open Failed" );
        ftcb.fh = 0;
        RestoreParameters ();
        return ( 0 );
      }


    file_size = _dseek ( ftcb.fh, 0L, 2 );
    _dseek ( ftcb.fh, 0L, 0 );
    
    Binit ();
    ftcb.protocol  = nvmodes.protocol;
    if ( ftcb.protocol == XMODEM_CRC )
       ftcb.protocol = XMODEM;
       
    nvmodes.direction = OUTGOING;
    ftcb.trans_act    = 1;

    ftcb.hdrsize   = XMODEM_HEADER_SIZE;
    ftcb.blocksize = ( ftcb.protocol == YMODEM ) ? 1024 : XMODEM_BLOCK_SIZE;
    ftcb.checksize = ( ftcb.protocol == YMODEM ) ? XMODEM_CRC_SIZE
                                                 : XMODEM_CHECK_SIZE;

/*-----*/

    /*
    ShowAbortRequester ( "File transfer in progress" );
    */
    if ( ftcb.protocol == YMODEM )
      {
        if ( !OpenTransferWindow ( "YMODEM Single File Transmit", 500, 65 ) )
            goto xmodem_transfer_aborted;
      }
    else
      {
        if ( !OpenTransferWindow ( "XMODEM Transmit", 500, 65 ) )
            goto xmodem_transfer_aborted;
      }
    
    transfer_seconds  = file_size / ( nvmodes.lspeed / 10 );
    transfer_seconds += transfer_seconds / 2;
    transfer_minutes  = transfer_seconds / 60;
    transfer_seconds %= 60;
    
    SetFile ( nvmodes.pc_file );
    sprintf ( message_buffer, "Block: %5ld of %5ld, approximate transfer time: %d:%02d",
              0L, (long)(file_size / ftcb.blocksize + 1),
              transfer_minutes, transfer_seconds );
    if ( TWindow )
      {
        Move ( TWindow->RPort, 8, 30 );
        Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
      }

    if ( WWindow )
      {
        Move ( WWindow->RPort, 8, 30 );
        Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
      }

    SetErrorCounts ( 0, 0 );
    SetMessage ( "" );
    SetSendProtocol ( "" );

    transfer_aborted  = 0;
    ftcb.retries      = 0;
    ftcb.total_errors = 0;
    ftcb.blockno      = 1;
    memset ( ftcb.block, 0, ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );
    ftcb.size = _dread ( ftcb.fh, ftcb.block + ftcb.hdrsize, ftcb.blocksize );

    while ( (!transfer_complete) && ( ftcb.retries < MAX_RETRIES ) )
      {
        if ( ftcb.blockno == 1 )
            resp_char = GetOneByteFromSet ( "\x04\x06\x15\x18\x43" );
        else
            resp_char = GetOneByteFromSet ( "\x04\x06\x15\x18" );

        switch ( resp_char )
          {
            case ACK :
                if ( ftcb.size == 0 )
                  {
                    transfer_complete = 1;
                    break;
                  }
                SetBlock ( ftcb.blockno );
                if ( ftcb.retries != 0 )
                  {
                    ftcb.retries = 0;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                  }
                ftcb.blockno++;
                memset ( ftcb.block, 0,
                         ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );
                ftcb.size = _dread ( ftcb.fh, ftcb.block + ftcb.hdrsize,
                                   ftcb.blocksize );
                SendBlock ();
                break;
             case CRC_CHAR:
                if ( transfer_aborted )
                    goto xmodem_transfer_aborted;

                if ( ftcb.protocol != YMODEM )
                  {
                    ftcb.protocol    = XMODEM_CRC;
                    ftcb.checksize   = XMODEM_CRC_SIZE;
                  }
             case NAK :
                if ( transfer_aborted )
                    goto xmodem_transfer_aborted;

                if ( ftcb.blockno != 1 )
                  {
                    ftcb.retries++;
                    ftcb.total_errors++;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                    SetMessage ( "Block verification error" );
                  }
                else
                  {
                    if ( ftcb.protocol == XMODEM_CRC
                      || ftcb.protocol == YMODEM )
                      {
                        SetSendProtocol ( "CRC" );
                        ftcb.crcmode = 1;
                      }
                    else
                      {
                        ftcb.protocol  = XMODEM;
                        ftcb.checksize = XMODEM_CHECK_SIZE;
                        SetSendProtocol ( "Checksum" );
                        ftcb.crcmode = 0;
                      }
                  }
                SendBlock ();
                break;
            case CAN :
                goto xmodem_transfer_aborted;
            default:
                if ( transfer_aborted )
                    goto xmodem_transfer_aborted;

                ftcb.retries++;
                ftcb.total_errors++;
                SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                SetMessage ( "Timeout waiting for ACK or NAK" );
                break;
          }
        if ( transfer_aborted )
            break;
      }

xmodem_transfer_aborted:;

    _dclose ( ftcb.fh );
    ftcb.fh = NULL;

    RestoreParameters ();

    CloseTransferWindow ();

    if ( transfer_complete )
      {
        TransferMessage ( "\r\n\nTransmit Successful\n\n\r" );
        CompleteBeep ();
      }
    else if ( ftcb.retries == MAX_RETRIES )
      {
        TransferMessage ( "\r\n\nToo Many retries\n\n\r" );
        FailedBeep ();
      }
    else
      {
        TransferMessage ( "\r\n\nTransmission aborted by user\n\n\r" );
        SendCANs ();
        FailedBeep ();
      }

/*-----*/

    ftcb.trans_act = 0;

    return ( transfer_complete );

  }

unsigned short int __stdargs BeginYmodemSend ( unsigned char * file )
  {
    unsigned char               GetOneByteFromSet ();
    
    static unsigned char        message_buffer[81];
    unsigned char               **files;
    unsigned long int           file_size,
                                bytes_transfered;
    unsigned short int          transfer_minutes,
                                transfer_seconds,
                                file_idx,
                                can_count = 0;
    register unsigned char      resp_char;
    register unsigned short int transfer_complete = 0;

    if ( ftcb.trans_act                &&
         ftcb.protocol     != XON_XOFF &&
         nvmodes.direction != INCOMING )
         return ( 0 );
    
    SetXmodemParameters ();

    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "File(s) To Send", nvmodes.pc_file ) )
      {
        RestoreParameters ();
        return ( 0 );
      }
    
    if ( !(files = FileList ( nvmodes.pc_file ) ) )
      {
        Error ( "Could not build file list" );
        RestoreParameters ();
        return ( 0 );
      }
    
    Binit ();
    ftcb.protocol     = YMODEM_BATCH;
    nvmodes.direction = OUTGOING;
    ftcb.trans_act    = 1;

    transfer_aborted  = 0;
    ftcb.hdrsize      = XMODEM_HEADER_SIZE;
    ftcb.checksize    = XMODEM_CHECK_SIZE;

/*-----*/

    if ( !OpenTransferWindow ( "YMODEM BATCH Transmit", 500, 65 ) )
       goto ymodem_transfer_aborted;
    
    file_idx = 0;
    do
      {
        SetFile ( files[ file_idx] );
        
        if ( *files[ file_idx ] )
          {
            if ( -1 == ( ftcb.fh = _dopen ( files[ file_idx ], MODE_OLDFILE ) ) )
              {
                Error ( "File Open Failed" );
                ftcb.fh = 0;
                goto ymodem_transfer_aborted;
              }

            file_size = _dseek ( ftcb.fh, 0L, 2 );
            _dseek ( ftcb.fh, 0L, 0 );
            ftcb.size      = XMODEM_BLOCK_SIZE;
          }
        else
          {
            file_size = 0;
            ftcb.size = 0;
          }
    
        transfer_seconds  = file_size / ( nvmodes.lspeed / 10 );
        transfer_seconds += transfer_seconds / 2;
        transfer_minutes  = transfer_seconds / 60;
        transfer_seconds %= 60;
    
        bytes_transfered = 0L;
        sprintf ( message_buffer, "%7ld bytes of %7ld, approximate transfer time: %d:%02d",
              bytes_transfered, file_size, transfer_minutes, transfer_seconds );
        if ( TWindow )
          {
            Move ( TWindow->RPort, 8, 30 );
            Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
          }

        if ( WWindow )
          {
            Move ( WWindow->RPort, 8, 30 );
            Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
          }
 
        SetErrorCounts ( 0, 0 );
        SetMessage ( "" );
        SetSendProtocol ( "" );

        transfer_complete = 0;
        transfer_aborted  = 0;
        ftcb.retries      = 0;
        ftcb.total_errors = 0;
        ftcb.blockno      = 0;
        memset ( ftcb.block, 0, ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );
        strcpy ( ftcb.block+ftcb.hdrsize, nvmodes.pc_file );
        sprintf ( ftcb.block+ftcb.hdrsize+strlen(nvmodes.pc_file),"%ld", file_size );
    
        ftcb.blocksize = XMODEM_BLOCK_SIZE;

        while ( (!transfer_complete) && ( ftcb.retries < MAX_RETRIES ) )
          {
            if ( ftcb.blockno <= 1 )
                resp_char = GetOneByteFromSet ( "\x04\x06\x15\x18\x43" );
            else
                resp_char = GetOneByteFromSet ( "\x04\x06\x15\x18" );

            switch ( resp_char )
              {
                case ACK :
                    can_count = 0;
                    
                    if ( transfer_aborted )
                        goto ymodem_transfer_aborted;
                    if ( ftcb.size == 0 )
                      {
                        transfer_complete = 1;
                        break;
                      }
                      
                    if ( ftcb.blockno )
                      {
                        bytes_transfered += ftcb.size;
                        SetBytes ( bytes_transfered );
                      }
                    else
                        ftcb.blocksize = 1024;
                        
                    /*
                    if ( file_size - bytes_transfered <= 768L )
                       ftcb.blocksize = 128;
                    else
                       ftcb.blocksize = 1024;
                   
                    if ( ftcb.total_errors > 10 )
                       ftcb.blocksize = 128;
                    */
                       
                    if ( ftcb.retries != 0 )
                      {
                        ftcb.retries = 0;
                        SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                      }
                    ftcb.blockno++;
                    
                    if ( ftcb.blockno == 1 )
                        break;
                    
                    memset ( ftcb.block, 0,
                             ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );
                    ftcb.size = _dread ( ftcb.fh, ftcb.block + ftcb.hdrsize,
                                   ftcb.blocksize );
                
                    SendBlock ();
                    break;
                 case CRC_CHAR:
                    can_count = 0;
                    
                    if ( transfer_aborted )
                        goto ymodem_transfer_aborted;
                    if ( ftcb.blockno == 0 )
                      {
                        ftcb.protocol  = YMODEM_BATCH;
                        ftcb.checksize = XMODEM_CRC_SIZE;
                        SetSendProtocol ( "CRC" );
                        ftcb.crcmode = 1;
                        SendHeaderBlock ( files[ file_idx ], file_size );
                      }
                    else if ( ftcb.blockno == 1 )
                      {
                        memset ( ftcb.block, 0,
                                 ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );
                        ftcb.size = _dread ( ftcb.fh, ftcb.block + ftcb.hdrsize,
                                            ftcb.blocksize );
                        SendBlock ();
                      }
                    else
                      {
                        ftcb.retries++;
                        ftcb.total_errors++;
                        SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                        SetMessage ( "CRC specifier invalid for this block" );
                      }
                    break;
                 case NAK :
                    can_count = 0;
                    
                    if ( transfer_aborted )
                        goto ymodem_transfer_aborted;
                    if ( ftcb.blockno == 0 )
                       break;
                    ftcb.retries++;
                    ftcb.total_errors++;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                    SetMessage ( "Block verification error" );
                    SendBlock ();
                    break;
                case CAN :
                    if ( ++can_count == 3 )
                        goto ymodem_transfer_aborted;
                    break;
                default:
                    if ( transfer_aborted )
                        goto ymodem_transfer_aborted;

                    ftcb.retries++;
                    ftcb.total_errors++;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                    SetMessage ( "Timeout waiting for ACK or NAK" );
                    break;
              }
            if ( transfer_aborted )
                break;
          }

        ymodem_transfer_aborted:;
        
        if ( ftcb.fh )
          {
            _dclose ( ftcb.fh );
            ftcb.fh = NULL;
          }
        
        if ( *files[ file_idx ] )
          {
            TransferMessage ( files[ file_idx ] );
            TransferMessage ( " : " );

            if ( transfer_complete )
                TransferMessage ( "Transmit Successful\n\r" );
            else if ( ftcb.retries == MAX_RETRIES )
              {
                TransferMessage ( "Too Many retries\n\r" );
                FailedBeep ();
                break;
              }
            else
              {
                TransferMessage ( "Transmission aborted by user\n\r" );
                SendCANs ();
                FailedBeep ();
                break;
              }
          }
        
        file_idx++;
    } while ( *files[ file_idx - 1 ] );

/*-----*/

    RestoreParameters ();

    CloseTransferWindow ();

    ftcb.trans_act = 0;

    if ( transfer_complete )
        CompleteBeep ();
    
    return ( transfer_complete );

  }

unsigned short int __stdargs BeginXmodemReceive ( unsigned short int protocol, unsigned char *file )
  {
    unsigned char               GetOneByteFromSet ();
    register unsigned char      first_byte;
    register unsigned short int transfer_complete = 0;

    if ( ftcb.trans_act                &&
         ftcb.protocol  == XON_XOFF &&
         nvmodes.direction != INCOMING )
         return ( 0 );

    SetXmodemParameters ();

    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "Receive File Name", nvmodes.pc_file ) )
      {
        RestoreParameters ();
        return ( 0 );
      }
    
    if ( -1 == ( ftcb.fh = _dcreat ( nvmodes.pc_file, 0 ) ) )
      {
        Error ( "File Open Failed" );
        ftcb.fh = 0;
        RestoreParameters ();
        return ( 0 );
      }

    ftcb.protocol = protocol;
    if ( protocol == XMODEM_CRC || ftcb.protocol == YMODEM )
      {
        ftcb.crcmode   = 1;
        ftcb.checksize = XMODEM_CRC_SIZE;
      }
    else
      {
        ftcb.crcmode   = 0;
        ftcb.checksize = XMODEM_CHECK_SIZE;
      }
    
    Binit ();
    nvmodes.direction = INCOMING;
    ftcb.trans_act    = 1;

    ftcb.blocksize = ( ftcb.protocol == YMODEM ) ? 1024 : XMODEM_BLOCK_SIZE;
    ftcb.hdrsize   = XMODEM_HEADER_SIZE;
    
/*-----*/

    if ( ftcb.protocol == YMODEM )
      {
        if ( !OpenTransferWindow ( "YMODEM Single File Receive", 500, 55 ) )
            goto xmodem_receive_aborted;
      }
    else
      {
        if ( !OpenTransferWindow ( "XMODEM Receive", 500, 55 ) )
            goto xmodem_receive_aborted;
      }
    
    transfer_aborted  = 0;
    ftcb.retries      = 0;
    ftcb.total_errors = 0;
    ftcb.blockno      = 1;
    memset ( ftcb.block, 0, ftcb.hdrsize + ftcb.blocksize + ftcb.checksize);

    SetFile ( nvmodes.pc_file );
    SetBlock ( 0 );
    if ( protocol == XMODEM )
        SetReceiveProtocol ( "Checksum" );
    else
        SetReceiveProtocol ( "CRC" );
    SetErrorCounts ( 0, 0 );
    SetMessage ( "" );

    DrainLine ();
    if ( ftcb.crcmode )
        SendChar ( CRC_CHAR );
    else
        SendChar ( NAK );

    while ( (!transfer_complete) && ( ftcb.retries < MAX_RETRIES ) )
      {
        if ( ftcb.protocol == YMODEM )
            first_byte = GetOneByteFromSet ( "\x01\x02\x04\x18" );
        else
            first_byte = GetOneByteFromSet ( "\x01\x04\x18" );

        switch ( first_byte )
          {
            case SOH :
            case STX :
                if ( ftcb.protocol == YMODEM )
                    ftcb.blocksize = ( first_byte == SOH ) ? XMODEM_BLOCK_SIZE
                                                           : 1024;
                if ( GetBlock () )
                  {
                    SetBlock ( ftcb.blockno );
                    if ( ftcb.retries != 0 )
                      {
                        ftcb.retries = 0;
                        SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                      }
                    if ( ftcb.block[0] == (unsigned char) ftcb.blockno )
                      {
                        if ( Bwrite ( ftcb.block + ftcb.hdrsize - 1,
                                      ftcb.blocksize )
                             != ftcb.blocksize )
                          {
                            SetMessage ( "Disk Full" );
                            TransferMessage ( "Disk Full\n\r" );
                            transfer_aborted = 1;
                            goto xmodem_receive_aborted;
                          }
                        ftcb.blockno++;
                      }
                    SendChar ( ACK );
                  }
                else
                  {
                    ftcb.retries++;
                    ftcb.total_errors++;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                    DrainLine ();
                    SendChar ( NAK );
                  }
                if ( transfer_aborted )
                    goto xmodem_receive_aborted;
                break;
            case CAN :
                goto xmodem_receive_aborted;
            case EOT :
                SendChar ( ACK );
                transfer_complete = 1;
                if ( transfer_aborted )
                    goto xmodem_receive_aborted;
                break;
            default  :
                if ( transfer_aborted )
                    goto xmodem_receive_aborted;

                SetMessage ( "Timeout waiting for header" );
                ftcb.retries ++;
                if ( ftcb.protocol == XMODEM_CRC
                  && ftcb.blockno  == 1
                  && ftcb.retries  == 5 )
                  {
                    ftcb.crcmode = 0;
                    ftcb.checksize = XMODEM_CHECK_SIZE;
                    ftcb.protocol  = XMODEM;
                    SetMessage ( "Switching to checksum mode" );
                    SetReceiveProtocol ( "Checksum" );
                  }
                ftcb.total_errors++;
                SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                DrainLine ();
                if ( ftcb.crcmode && ftcb.blockno == 1 )
                    SendChar ( CRC_CHAR );
                else
                    SendChar ( NAK );
                break;
          }

        if ( transfer_aborted )
            break;
      }

xmodem_receive_aborted:;

    Bflush ();
    _dclose ( ftcb.fh );
    ftcb.fh = NULL;

    RestoreParameters ();

    CloseTransferWindow ();

    if ( transfer_complete )
      {
        TransferMessage ( "\r\n\nReceive Successful\n\n\r" );
        CompleteBeep ();
      }
    else if ( ftcb.retries == MAX_RETRIES )
      {
        TransferMessage ( "\r\n\nToo Many retries\n\n\r" );
        FailedBeep ();
      }
    else
      {
        TransferMessage ( "\r\n\nTransmission aborted by user\n\n\r" );
        SendCANs ();
        FailedBeep ();
      }

/*-----*/

    ftcb.trans_act = 0;

    return ( transfer_complete );
  }

unsigned short int __stdargs BeginYmodemReceive ( unsigned short int protocol, unsigned char *path )
  {
    unsigned char               GetOneByteFromSet ();
    
    static unsigned char        file_name[129],
                                message_buffer[81];
    unsigned char               *name_ptr;
    long int                    file_size,
                                bytes_received;
    register unsigned char      first_byte;
    register unsigned short int transfer_complete,
                                transfer_minutes,
                                transfer_seconds;
    unsigned char               more_files = 1;


    if ( ftcb.trans_act             &&
         ftcb.protocol  == XON_XOFF &&
         nvmodes.direction != INCOMING )
         return ( 0 );

    SetXmodemParameters ();

    if ( path )
        strcpy ( nvmodes.dest_path, path );
    else if ( !FileReq ( "Destination Path", nvmodes.dest_path ) )
      {
        RestoreParameters ();
        return ( 0 );
      }
    
    if ( nvmodes.dest_path [ strlen ( nvmodes.dest_path ) - 1 ] != '/' &&
         nvmodes.dest_path [ strlen ( nvmodes.dest_path ) - 1 ] != ':' )
        strcat ( nvmodes.dest_path, "/" );
    
    ftcb.protocol  = YMODEM_BATCH;
    ftcb.crcmode   = 1;
    ftcb.checksize = XMODEM_CRC_SIZE;
    
    Binit ();
    nvmodes.direction = INCOMING;
    ftcb.trans_act    = 1;

    ftcb.blocksize = 1024;
    ftcb.hdrsize   = XMODEM_HEADER_SIZE;
    
    if ( !OpenTransferWindow ( "YMODEM BATCH Receive", 500, 65 ) )
      {
        more_files = 0;
        goto ymodem_receive_aborted;
      }
    
    DrainLine ();

/*-----*/

    while ( more_files )
      {
        transfer_complete = 0;
        transfer_aborted  = 0;
        ftcb.retries      = 0;
        ftcb.total_errors = 0;
        ftcb.blockno      = 0;
        bytes_received    = 0L;
        memset ( ftcb.block, 0, ftcb.hdrsize + ftcb.blocksize + ftcb.checksize );

        SetFile ( "" );
        sprintf ( message_buffer, "%7ld bytes of %7d", 0L, 0L );
        if ( TWindow )
          {
            Move ( TWindow->RPort, 8, 30 );
            Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
          }

        if ( WWindow )
          {
            Move ( WWindow->RPort, 8, 30 );
            Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
          }

        SetSendProtocol ( "CRC" );
        SetErrorCounts ( 0, 0  );
        SetMessage ( "" );

        SendChar ( CRC_CHAR );

        while ( (!transfer_complete) && ( ftcb.retries < MAX_RETRIES ) )
          {
            first_byte = GetOneByteFromSet ( "\x01\x02\x04\x18" );

            switch ( first_byte )
              {
                case SOH :
                case STX :
                    if ( transfer_aborted )
                      {
                        more_files = 0;
                        goto ymodem_receive_aborted;
                      }
                    ftcb.blocksize = ( first_byte == SOH ) ? 128 : 1024;
                    if ( GetBlock () )
                      {
                        if ( ftcb.retries != 0 )
                          {
                            ftcb.retries = 0;
                            SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                          }
                        if ( ftcb.blockno == 0 )
                          {
                            name_ptr = ftcb.block + ftcb.hdrsize - 1;
                            if ( !*name_ptr )
                              {
                                SendChar ( ACK );
                                TransferMessage ( "All Finished\n\r" );
                                more_files = 0;
                                transfer_complete = 1;
                                goto ymodem_receive_aborted;
                              }
                            strcpy ( file_name, nvmodes.dest_path );
                            stcgfn ( file_name + strlen ( file_name ),
                                     name_ptr );
                            SetFile ( file_name );
                            if ( -1 == ( ftcb.fh = _dcreat ( file_name,0 ) ) )
                              {
                                Error ( "File Open Failed" );
                                ftcb.fh = 0;
                                transfer_aborted = 1;
                                goto ymodem_receive_aborted;
                              }

                            if ( stcd_l ( name_ptr + strlen( name_ptr ) + 1,
                                 &file_size )
                               )
                              {
                                transfer_seconds  = file_size /
                                                    ( nvmodes.lspeed / 10 );
                                transfer_seconds += transfer_seconds / 2;
                                transfer_minutes  = transfer_seconds / 60;
                                transfer_seconds %= 60;
                                sprintf ( message_buffer,
                                          "%7ld bytes of %7ld, approximate transfer time: %d:%02d",
                                          bytes_received, file_size,
                                          transfer_minutes,
                                          transfer_seconds );
                                if ( TWindow )
                                  {
                                    Move ( TWindow->RPort, 8, 30 );
                                    Text ( TWindow->RPort, message_buffer,
                                        (long) strlen ( message_buffer ) );
                                  }

                                if ( WWindow )
                                  {
                                    Move ( WWindow->RPort, 8, 30 );
                                    Text ( WWindow->RPort, message_buffer,
                                           (long) strlen ( message_buffer ) );
                                  }
                              }
                            else
                              {
                                file_size = -1L;
                              }
                            ftcb.blockno++;
                            SendChar ( ACK );
                            SendChar (CRC_CHAR );
                          }
                        else
                          {
                            if ( file_size != -1L )
                              {
                                if ( file_size > ftcb.blocksize )
                                    file_size -= ftcb.blocksize;
                                else
                                  ftcb.blocksize = file_size;
                              }
                            if ( ftcb.block[0]
                                 == (unsigned char) ftcb.blockno )
                              {
                                if ( Bwrite ( ftcb.block + 2, ftcb.blocksize )
                                     != ftcb.blocksize )
                                  {
                                    SetMessage ( "Disk Full" );
                                    transfer_aborted = 1;
                                    more_files = 0;
                                    goto ymodem_receive_aborted;
                                  }
                                bytes_received += ftcb.blocksize;
                                SetBytes ( bytes_received );
                                ftcb.blockno++;
                              }
                            ftcb.blocksize = ( first_byte == SOH ) ? 128 : 1024;
                            SendChar ( ACK );
                          }
                      }
                    else
                      {
                        ftcb.retries++;
                        ftcb.total_errors++;
                        SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                        DrainLine ();
                        if ( ftcb.blockno != 0 )
                            SendChar ( NAK );
                        else
                            SendChar ( CRC_CHAR );
                      }
                    break;
                case CAN :
                    goto ymodem_receive_aborted;
                case EOT :
                    SendChar ( ACK );
                    transfer_complete = 1;
                    if ( transfer_aborted )
                      {
                        more_files = 0;
                        goto ymodem_receive_aborted;
                      }
                    break;
                default  :
                    if ( transfer_aborted )
                      {
                        more_files = 0;
                        goto ymodem_receive_aborted;
                      }

                    SetMessage ( "Timeout waiting for header" );
                    ftcb.retries ++;
                    ftcb.total_errors++;
                    SetErrorCounts ( ftcb.retries, ftcb.total_errors );
                    DrainLine ();
                    if ( ftcb.crcmode && ftcb.blockno == 0 )
                        SendChar ( CRC_CHAR );
                    else
                        SendChar ( NAK );
                    break;
              }

            if ( transfer_aborted )
                break;
          }

        ymodem_receive_aborted:;

        if ( ftcb.fh )
          {
            Bflush ();
            _dclose ( ftcb.fh );
            ftcb.fh = NULL;
          }

        if ( *name_ptr )
          {
            TransferMessage ( file_name );
            TransferMessage ( " : " );
          }

        if ( transfer_complete )
            TransferMessage ( "Receive Successful\n\r" );
        else if ( ftcb.retries == MAX_RETRIES )
          {
            TransferMessage ( "Too Many retries\n\r" );
            FailedBeep ();
            break;
          }
        else
          {
            TransferMessage ( "Transmission aborted by user\n\r" );
            SendCANs ();
            FailedBeep ();
            break;
          }
    }

/*-----*/

    ftcb.trans_act = 0;

    RestoreParameters ();

    CloseTransferWindow ();

    if ( transfer_complete )
        CompleteBeep ();
    
    return ( transfer_complete );
  }

unsigned char GetOneByteFromSet ( set )
char    *set;
  {
    unsigned short int  i;
    unsigned long int   inchar;
    unsigned char       *sptr;
    unsigned long int   mask,
                        io_mask,
                        tr_mask,
                        signals;

    tr_mask = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    io_mask = 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask    = tr_mask | io_mask;

    StartRead ( (unsigned char *) &inchar, 1 );
    for ( i = 0; i < 10; i ++ )
      {
        StartTimer ( &timeout_req, 1 );
        
        for ( ;; )
          {
            /*
            printf ( "Waiting for Single byte.." );
            */
            signals = Wait ( mask );

            if ( signals & io_mask )
              {
                /*
                printf ( " IO completed.." );
                */
                GetMsg ( ser_port );
                if ( CheckForWindowAbort () )
                  {
                    transfer_aborted = 1;
                    if ( !( signals & tr_mask ) )
                      {
                        AbortIO ( (struct IORequest *) &timeout_req );
                        Wait    ( tr_mask  );
                      }
                    GetMsg  ( timeout_port );
                    /*
                    printf ( " User Abort\n" );
                    */
                    return ( 0 );
                  }
                sptr = set;
                while ( *sptr )
                    if ( ( *(unsigned char *) &inchar ) == *sptr++ )
                      {
                        if ( !( signals & tr_mask ) )
                          {
                            AbortIO ( (struct IORequest *) &timeout_req );
                            Wait    ( tr_mask );
                          }
                        GetMsg  ( timeout_port );
                        /*
                        printf ( " Got a Match %02x\n", *(unsigned char *) &inchar );
                        */
                        return  ( *(unsigned char *) &inchar );
                      }
                StartRead ( (unsigned char *) &inchar, 1 );
                /*
                printf ( " Match failed on %02x\n", inchar );
                */
              }
            if ( signals & tr_mask )
              {
                /*
                printf ( " Timeout\n" );
                */
                GetMsg ( timeout_port );
                if ( CheckForWindowAbort () )
                  {
                    transfer_aborted = 1;
                    goto send_aborted;
                  }
                break;
              }
          }
      }

send_aborted:;

    if ( !( signals & io_mask ) )
      {
        AbortIO ( (struct IORequest *) &ser_req );
        Wait    ( io_mask  );
      }
    GetMsg  ( ser_port );
    return  ( 0 );
  }

void SendHeaderBlock ( filename, filesize )
unsigned char       *filename;
unsigned long int   filesize;
  {
    unsigned char       *blk_ptr;

    
    memset ( ftcb.block + ftcb.hdrsize, 0, ftcb.blocksize + ftcb.checksize );
    
    ftcb.block[0] =  SOH;
    ftcb.block[1] =  (unsigned char) ftcb.blockno;
    ftcb.block[2] = ~(unsigned char) ftcb.blockno;
    

    blk_ptr = ftcb.block + ftcb.hdrsize;
    stcgfn ( blk_ptr, filename );
    if ( filesize ) 
        sprintf ( blk_ptr + strlen ( blk_ptr ) + 1,
                  "%ld", filesize );
    
    ClrCRC ();
    UpdCRC ( ftcb.block + ftcb.hdrsize, ftcb.blocksize );
    ftcb.block[ftcb.hdrsize + ftcb.blocksize + 0] = ftcb.high;
    ftcb.block[ftcb.hdrsize + ftcb.blocksize + 1] = ftcb.low;

    ser_req.IOSer.io_Length  =
        ftcb.hdrsize + ftcb.blocksize + ftcb.checksize;
    ser_req.IOSer.io_Data    = (APTR) ftcb.block;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_WRITE;
    DoIO ( (struct IORequest *) &ser_req );
  }

void SendBlock ()
  {
    unsigned short int  i;
    unsigned char  check_sum = 0;

    /*
    printf ( "Sending block of %d.. ", ftcb.blocksize );
    */
    
    if ( ftcb.size == 0 )
      {
        SendChar ( EOT );
      }
    else
      {
        
        ftcb.block[0] =  (ftcb.blocksize == 128 ) ? SOH : STX;
        ftcb.block[1] =  (unsigned char) ftcb.blockno;
        ftcb.block[2] = ~(unsigned char) ftcb.blockno;

        if ( ftcb.crcmode )
          {
            ClrCRC ();
            UpdCRC ( ftcb.block + ftcb.hdrsize, ftcb.blocksize );
            ftcb.block[ftcb.hdrsize + ftcb.blocksize + 0] = ftcb.high;
            ftcb.block[ftcb.hdrsize + ftcb.blocksize + 1] = ftcb.low;
          }
        else
          {
            for ( i = 0; i < ftcb.blocksize; i++ )
                check_sum += ftcb.block[i + ftcb.hdrsize];
            ftcb.block[ftcb.hdrsize+ftcb.blocksize] = check_sum;
          }

        ser_req.IOSer.io_Length  =
            ftcb.hdrsize + ftcb.blocksize + ftcb.checksize;
        ser_req.IOSer.io_Data    = (APTR) ftcb.block;
        ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
        ser_req.IOSer.io_Command = CMD_WRITE;
        DoIO ( (struct IORequest *) &ser_req );
      }
    /*
    printf ( " IO completed\n" );
    */
  }

int GetBlock ()
  {
    unsigned long int   mask,
                        io_mask,
                        tr_mask,
                        signals;
    unsigned short int  i;

    tr_mask = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    io_mask = 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask    = tr_mask | io_mask;

    StartRead ( ftcb.block,
                 ftcb.hdrsize - 1 + ftcb.blocksize + ftcb.checksize );
    /*
    printf ( "Waiting for block.." );
    */
    for ( i = 0; i < 9 + (ftcb.blocksize/(nvmodes.lspeed/10)); i++ )
      {
        StartTimer ( &timeout_req, 1 );

        signals = Wait ( mask );

        if ( signals & io_mask )
          {
            /*
            printf ( " IO completed.." );
            */
            GetMsg  ( ser_port );
            if ( !( signals & tr_mask ) )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( tr_mask );
              }
            GetMsg  ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                /*
                printf ( " User Abort\n" );
                */
                return ( 0 );
              }
            /*
            printf ( " Verifying block\n" );
            */
            return ( VerifyBlock () );
          }
        else
          {
            /*
            printf ( " timeout.." );
            */
            GetMsg ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                /*
                printf ( " User Abort2\n" );
                */
                goto get_aborted;
              }
            /*
            printf ( " Waiting again\n" );
            */
          }
      }
    SetMessage ( "Timeout waiting for block" );

get_aborted:;

    if ( !( signals & io_mask ) )
      {
        AbortIO ( (struct IORequest *) &ser_req );
        Wait    ( io_mask  );
      }
    GetMsg  ( ser_port );
    return  ( 0 );
  }

int VerifyBlock ()
  {
    register unsigned short int     i;
    unsigned char                   buffer[80];

    if ( ftcb.block[0] != ( ( ~ftcb.block[1] ) & 0xff ) )
      {
        sprintf ( buffer, "Invalid Header, lock bytes are <%02x>, <%02x>",
                  ftcb.block[0], ftcb.block[1] );
        SetMessage ( buffer );
        return ( 0 );
      }

    if ( ftcb.block[0] != (unsigned char) ftcb.blockno )
      {
        if ( ftcb.block[0] == (unsigned char) ( ftcb.blockno - 1 ) )
            return ( -1 );
        SetMessage ( "Block out of sequence" );
        return ( 0 );
      }
      
    if ( !ftcb.crcmode )
      {
        for ( ftcb.chksum = i = 0; i < ftcb.blocksize;
              ftcb.chksum += ftcb.block[ ftcb.hdrsize - 1 + i++ ] );
        if ( ftcb.chksum != ftcb.block[ ftcb.hdrsize - 1 + ftcb.blocksize  ] )
          {
            SetMessage ( "Checksum verification error" );
            return ( 0 );
          }
      }
    else
      {
        ClrCRC ();
        UpdCRC ( (unsigned char *) ftcb.block+2,
                 (unsigned short int) (ftcb.blocksize + ftcb.checksize ) );
        if ( ftcb.low != 0 || ftcb.high != 0 )
          {
            SetMessage ( "CRC verification error" );
            return ( 0 );
          }
      }

    return ( 1 );
  }

void DrainLine ()
  {
    unsigned char       ser_buffer[1536];
    unsigned long int   mask,
                        io_mask,
                        tr_mask,
                        signals;

    tr_mask = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    io_mask = 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask    = tr_mask | io_mask;

    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = SDCMD_QUERY;
    DoIO ( (struct IORequest *) &ser_req );
    ser_req.IOSer.io_Length  = ser_req.IOSer.io_Actual ?
                               ser_req.IOSer.io_Actual : 1;
    StartTimer ( &timeout_req, 1 );
    StartRead ( ser_buffer, (unsigned int) ser_req.IOSer.io_Length );

    for ( ;; )
      {
        /*
        printf ( "Draining port.." );
        */
        signals = Wait ( mask );

        if ( signals & io_mask )
          {
            /*
            printf ( " IO completed.." );
            */
            GetMsg  ( ser_port );
            if ( !( signals & tr_mask ) )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( tr_mask      );
              }
            GetMsg  ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                return;
              }
              
            ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
            ser_req.IOSer.io_Command = SDCMD_QUERY;
            DoIO ( (struct IORequest *) &ser_req );
            ser_req.IOSer.io_Length  = ser_req.IOSer.io_Actual ?
                                       ser_req.IOSer.io_Actual : 1;
            StartTimer ( &timeout_req, 1 );
            StartRead ( ser_buffer, (short int) ser_req.IOSer.io_Length );
          }
        else
          {
            /*
            printf ( " Timeout\n" );
            */
            GetMsg  ( timeout_port );
            if ( CheckForWindowAbort () )
              {
                transfer_aborted = 1;
                if ( !( signals & io_mask ) )
                  {
                    AbortIO ( (struct IORequest *) &ser_req );
                    Wait    ( io_mask  );
                  }
                GetMsg  ( ser_port );
                return;
              }
            break;
          }
      }
    if ( !( signals & io_mask ) )
      {
        AbortIO ( (struct IORequest *) &ser_req );
        Wait    ( io_mask  );
      }
    GetMsg  ( ser_port );
    return;
  }
