/* -----------------------------------------------------------------------
 * transfer.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: transfer.c,v 1.1 91/05/09 16:33:02 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/transfer.c,v 1.1 91/05/09 16:33:02 bj Exp $
 *
 * $Log:	transfer.c,v $
 * Revision 1.1  91/05/09  16:33:02  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include "termall.h"

#define BLK_SZ              1536
#define WRITE_BUFFER_SIZE   8192

static unsigned char        write_buffer[ WRITE_BUFFER_SIZE ];
static unsigned char        *write_ptr;

void CrcTblI ()
  {
    unsigned short int poly;
    unsigned short int crc;
    unsigned short int i,j;

    poly = 0x1021;
    for ( i = 0; i < 256; i++ )
      {
        crc = i << 8;
        for ( j = 0; j < 8; j++ )
            if ( crc & 0x8000 )
                crc = ( crc << 1 ) ^ poly;
            else
                crc <<= 1;
        crctbl[ i ] = crc;
      }
  }

void ClrCRC ()
  {
    ftcb.high =
    ftcb.low  = 0;
  }
  
void UpdCRC ( bbyte, n )
unsigned char       *bbyte;
unsigned short int  n;
  {
    register unsigned short int icrc,
                                crc;
    
    for ( crc = 0, icrc = 0; icrc < n; icrc++ )
        crc = CRC( (char)bbyte[ icrc ], crc );
       
    ftcb.high = crc >> 8;
    ftcb.low  = crc & 0xff;
  }
  
void Binit ()
  {
    write_ptr = write_buffer;
    memset ( write_buffer, 0x00, WRITE_BUFFER_SIZE );
  }

int Bflush ()
  {
    int written,
        attempted;
    
    written = _dwrite ( ftcb.fh, write_buffer,
                        attempted = write_ptr - write_buffer );
    Binit ();
    return ( written == attempted );
  }

int Bwrite ( ptr, length )
unsigned char   *ptr;
short int       length;
  {
    short int section;

    while ( length > ( section = WRITE_BUFFER_SIZE - 
                                 ( write_ptr - write_buffer ) ) )
      {
          {
            memcpy ( write_ptr, ptr, section );
            ptr       += section;
            write_ptr += section;
            length    -= section;
            if ( !Bflush () )
               return ( -1 );
          }
      }
    memcpy ( write_ptr, ptr, length );
    write_ptr += length;
    return ( (int)length );
  }

unsigned char **FileList ( filespec )
unsigned char *filespec;
 {
    static unsigned char    names[3000],
                            *pointers[300];
    int                     count;
    
    count = getfnl ( filespec, names, sizeof(names), 0 );

    if ( !count )
        return ( NULL );

    if ( strbpl ( pointers, 300, names ) != count )
      {
        return  ( NULL );
      }
    
    strsrt ( pointers, count );

    return ( pointers );
  }
  
void FTCBInit ()
  {
    ftcb.size  = BLK_SZ;
    ftcb.block = (unsigned char *) HSAllocMem ( (unsigned long) ftcb.size,
                                              MEMF_CLEAR );
    if ( !ftcb.block )
      {
        DisplayAlert ( AG_NoMemory, MemMessage, 60L );
        ShutDown ();
      }
    ftcb.ptr   = ftcb.block;
    CrcTblI ();
  }

void FTCBClose ()
  {
    if ( ftcb.fh )
      {
        Bflush ();
        _dclose ( ftcb.fh );
        ftcb.fh = NULL;
      }
    if ( ftcb.block )
        HSFreeMem ( ftcb.block, BLK_SZ );
  }

int OpenTransferWindow ( title, xsize, ysize )
unsigned char       *title;
unsigned short int  xsize,
                    ysize;
 {
	unsigned short int color;
	
    NewWWindow.Width     = NewTWindow.Width     = xsize;
    NewWWindow.Height    = NewTWindow.Height    = ysize;
    NewWWindow.MinWidth  = NewTWindow.MinWidth  = xsize;
    NewWWindow.MinHeight = NewTWindow.MinHeight = ysize;
    NewWWindow.MaxWidth  = NewTWindow.MaxWidth  = xsize;
    NewWWindow.MaxHeight = NewTWindow.MaxHeight = ysize;
    NewWWindow.Title     = NewTWindow.Title     = title;
    NewTWindow.Screen = Screen;
    WWindow = (struct Window *) OpenWindow ( &NewWWindow );

    if ( WWindow == NULL )
      {
        Error ( "Insufficient memory to open Window" );
        return ( FALSE );
      }

	if ( Screen )
	  {
    	TWindow = (struct Window *) OpenWindow ( &NewTWindow );
    	if ( TWindow == NULL )
      	  {
        	Error ( "Insufficient memory to open Window" );
        	return ( FALSE );
      	  }
	  }
    
    if ( Screen && Screen->BitMap.Depth == 2 )
	  {
		StopBlinkTask ();

		color = nvmodes.color3;
       	SetRGB4 ( VPort, 3L,
           	(long)  color >>  8L,
           	(long)( color >>  4L ) & 0xfL,
           	(long)  color & 0xfL );
	  }
    memcpy ( Title_line + 35, "      ", 6 );
    ShowTitle ( Screen, TRUE );

    return  ( TRUE );
  }


void CloseTransferWindow ()
  {
    if ( WWindow )
        CloseWindow ( WWindow );
    WWindow = NULL;
    
    if ( TWindow )
        CloseWindow ( TWindow );
    TWindow = NULL;
	if ( Screen && Screen->BitMap.Depth == 2 )
		StartBlinkTask ();
  }

void SetSendProtocol ( string )
unsigned char *string;
  {
    SayTransfer ( 8, 60, "Block Verification: %-30s", string, -1L, -1L );
  }

void SetReceiveProtocol ( string )
unsigned char *string;
  {
    SayTransfer ( 192, 30, "Block Verification: %-12s", string, -1L, -1L );
  }

void SetMessage ( string )
unsigned char *string;
  {
    SayTransfer ( 8, 50, "Last error: %-46s", string, -1L, -1L );
  }

void SetErrorCounts ( block, total )
unsigned int   block,
               total;
  {
    SayTransfer ( 8, 40, "Errors this block: %2d, Total errors this file: %4d", 
              NULL, block, total );
  }

void SetSize ( bytes )
unsigned long int bytes;
  {
    SayTransfer ( 8,30, "%7ld bytes of %7ld sent", NULL, 0L, bytes );
  }
  
void SetReceived ( bytes )
unsigned long int bytes;
  {
    SayTransfer ( 8, 30, "%7ld bytes received", NULL, bytes, -1L );
  }
  
void SetBytes ( bytes )
unsigned long int bytes;
  {
    SayTransfer ( 8, 30, "%7ld", NULL, bytes, -1L );
  }

void SetBlock ( block )
unsigned int block;
  {
    SayTransfer ( 8, 30, "Block: %5d", NULL, block, -1L );
  }

void SetByteCount ( count )
unsigned long int   count;
  {
    SayTransfer ( 8, 30, "%7ld", NULL, count, -1L );
  }

void SetFile ( file_name )
unsigned char   *file_name;
  {
    SayTransfer ( 8, 20, "File: %-50s", file_name, -1L, -1L );
  }

int CheckForWindowAbort ()
  {
    int status = 0;

    if ( !TWindow )
       return ( 0 );
    while ( message = (struct IntuiMessage *) GetMsg ( TWindow->UserPort ) )
      {
        class   = message->Class;
        address = message->IAddress;
        ReplyMsg ( (struct Message *) message );
        if ( class == CLOSEWINDOW )
            status = 1;
      }
    return ( status );
  }

void TransferMessage ( string )
unsigned char   *string;
  {
    RemoveCursor ();
    while ( *string )
        VtDspCh ( *string++ );
    ScrFlush ();
  }

unsigned short int BeginASCIIReceive ( unsigned char *file )
  {
    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "Receive File Name", nvmodes.pc_file ) )
        return ( 0 );
        
    if ( -1 == ( ftcb.fh = _dcreat ( nvmodes.pc_file, 0 ) ) )
      {
        Error ( "File Open failed" );
        ftcb.fh = 0;
        return ( 0 );
      }

    Binit ();
    ftcb.ptr          = ftcb.block;
    ftcb.size         = BLK_SZ;
    ftcb.protocol     = XON_XOFF;
    ftcb.trans_act    = 1;
    nvmodes.direction = INCOMING;
    return ( 1 );
  }

void EndASCIIReceive ()
  {
    ftcb.trans_act  = 0;
    if ( ftcb.fh )
      {
        if ( ftcb.ptr != ftcb.block )
            if ( -1 == Bwrite ( (unsigned char *) ftcb.block,
                           (short int) ( ftcb.ptr - ftcb.block ) ) )
                Error ( "Disk Full" );
        Bflush ();
        _dclose ( ftcb.fh );
      }
    ftcb.fh = NULL;
  }

unsigned short int BeginASCIISend ( unsigned char *file )
  {
    int                 i;
    unsigned long int   file_size;
    unsigned char       file_name[67];

    if ( ftcb.trans_act )
        return ( 0 );

    if ( file )
        strcpy ( nvmodes.pc_file, file );
    else if ( !FileReq ( "File to Send", nvmodes.pc_file ) )
        return ( 0 );
    
    if ( -1 == ( ftcb.fh = _dopen ( nvmodes.pc_file, MODE_OLDFILE ) ) )
      {
        Error ( "File Open Failed" );
        ftcb.fh = 0;
        return ( 0 );
      }
    file_size = _dseek ( ftcb.fh, 0L, 2 );
    _dseek ( ftcb.fh, 0L, 0 );

    Binit ();
    ftcb.protocol  = XON_XOFF;
    nvmodes.direction = OUTGOING;
    ftcb.trans_act    = 1;

    OpenTransferWindow ( "ASCII Transmit", 500, 35 );
    stcgfn ( file_name, nvmodes.pc_file );
    SetFile ( file_name );
    SetSize ( file_size );
 
    while ( ftcb.size = _dread ( ftcb.fh, ftcb.block, BLK_SZ ) )
      {
        for ( i = 0; i < ftcb.size; i++ )
          {
            if ( ftcb.block[i] == '\n' )
                switch ( nvmodes.txnl )
                  {
                    case 0 :
                        if ( !Aputc ( '\r' ) )
                           goto ASCIISendAbort;
                        if ( !Aputc ( '\n' ) )
                           goto ASCIISendAbort;
                        break;
                    case 1 :
                        if ( !Aputc ( '\r' ) )
                           goto ASCIISendAbort;
                        break;
                    case 2 :
                        if ( !Aputc ( '\n' ) )
                           goto ASCIISendAbort;
                        break;
                    case 3 :
                        break;
                  }
            else if ( ftcb.block[i] == '\r' )
                switch ( nvmodes.txcr )
                  {
                    case 0 :
                        if ( !Aputc ( '\r' ) )
                           goto ASCIISendAbort;
                        if ( !Aputc ( '\n' ) )
                           goto ASCIISendAbort;
                        break;
                    case 1 :
                        if ( !Aputc ( '\r' ) )
                           goto ASCIISendAbort;
                        break;
                    case 2 :
                        if ( !Aputc ( '\n' ) )
                           goto ASCIISendAbort;
                        break;
                    case 3 :
                        break;
                  }
            else
                if ( !Aputc ( ftcb.block[i] ) )
                   goto ASCIISendAbort;
            if ( CheckForWindowAbort () )
                goto ASCIISendAbort;
          }
        SetByteCount ( _dseek ( ftcb.fh, 0L, 1 ) );  
      }

ASCIISendAbort:;

    
    CloseTransferWindow ();
    
    _dclose ( ftcb.fh );
    ftcb.fh = NULL;

    ftcb.trans_act = 0;

    return ( 1 );
  }

int Aputc ( c )
unsigned char c;
  {
    unsigned long int   signals,
                        timeoutmask,
                        serialmask;

    unsigned char       sent_ok;
    unsigned char       ch[2];

    *ch = c; /* Get character on an even boundary. */

    timeoutmask  = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    serialmask   = 1L << ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;

    ser_req.IOSer.io_Command = CMD_WRITE;
    ser_req.IOSer.io_Data    = (APTR) ch;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Length  = 1;
    SendIO ( (struct IORequest *) &ser_req );

    sent_ok = 0;
    do
      {
        StartTimer ( &timeout_req, 10 );

        signals = Wait ( timeoutmask | serialmask );

        if ( signals & serialmask )
         {
            if ( !( signals & timeoutmask ) )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( timeoutmask );
              }
            GetMsg  ( timeout_port );
            GetMsg  ( ser_port );;
            sent_ok = 1;

            if ( CheckForWindowAbort () )
                return ( 0 );
          }
        else
          {
            WaitIO ( (struct IORequest *) &timeout_req );
            if ( CheckForWindowAbort () )
              {
                AbortIO ( (struct IORequest *) &ser_req );
                Wait ( serialmask );
                GetMsg ( ser_port );
                return ( 0 );
              }
            HandleLockedLine ( &temp_ser_req );
          }
      } while ( !sent_ok );
   return ( 1 );
 }



void Capture ( ch )
unsigned char ch;
  {
    if ( ftcb.trans_act &&
         ftcb.protocol  == XON_XOFF &&
         nvmodes.direction == INCOMING )
      {
        if ( ch == '\n' )
            switch ( nvmodes.rxnl )
              {
                case 0 :
                    Fstore ( '\r' );
                    Fstore ( '\n' );
                    break;
                case 1 :
                    Fstore ( '\r' );
                    break;
                case 2 :
                    Fstore ( '\n' );
                    break;
                case 3 :
                    return;
              }
        else if ( ch == '\r' )
            switch ( nvmodes.rxcr )
              {
                case 0 :
                    Fstore ( '\r' );
                    Fstore ( '\n' );
                    break;
                case 1 :
                    Fstore ( '\r' );
                    break;
                case 2 :
                    Fstore ( '\n' );
                    break;
                case 3 :
                    return;
              }
        else
            Fstore ( ch );
      }
  }

void Fstore ( ch )
unsigned char ch;
  {
    *ftcb.ptr = ch;
    ++ftcb.ptr;
    if ( ftcb.ptr == ftcb.block + ftcb.size )
      {
        si2main_msg.opcode = FLUSH_CAPTURE_BUFFER;
        PutMsg ( main_task_port, (struct Message *) &si2main_msg );
        WaitPort ( si2main_msg.vt_msg.mn_ReplyPort );
        GetMsg ( si2main_msg.vt_msg.mn_ReplyPort );
      }
  }

void FlushCaptureBuffer ()
  {
    Bwrite ( ftcb.block, ftcb.size );
    ftcb.ptr = ftcb.block;
  }


void SetXmodemParameters ()
  {
    ftcb.sv_stpbits  = nvmodes.stpbits;
    ftcb.sv_bpc      = nvmodes.bpc;
    if ( nvmodes.direction ==  INCOMING &&
         ftcb.protocol  ==  XON_XOFF    &&
         ftcb.trans_act                 &&
         ftcb.fh )
      {
        Bflush ();
        ftcb.sv_fh       = ftcb.fh;
      }
    else
        ftcb.sv_fh       = NULL;
    nvmodes.stpbits   = 1;
    nvmodes.bpc       = 8;

    SetParams ( &ser_req, 0 );
  }

void RestoreParameters ()
  {
    nvmodes.stpbits  = ftcb.sv_stpbits;
    nvmodes.bpc      = ftcb.sv_bpc;
    if ( ftcb.sv_fh )
      {
        ftcb.fh           = ftcb.sv_fh;
        ftcb.trans_act    = 1;
        ftcb.protocol     = XON_XOFF;
        ftcb.size         = BLK_SZ;
        nvmodes.direction = INCOMING;
        ftcb.sv_fh        = NULL;
      }
    SetParams ( &ser_req, 1 );
  }

void CompleteBeep ()
  {
    MakeBeep (  500, 32 );
    MakeBeep (  750, 32 );
    MakeBeep ( 1250, 32 );
  }
  
void FailedBeep ()
  {
    MakeBeep ( 1250, 32 );
    MakeBeep (  750, 32 );
    MakeBeep (  500, 32 );
  }
  
void StartTimer ( timeout_req, seconds )
struct timerequest *timeout_req;
unsigned int    seconds;
  {
    timeout_req->tr_time.tv_secs    = seconds;
    timeout_req->tr_time.tv_micro   = 0;
    timeout_req->tr_node.io_Command = TR_ADDREQUEST;
    SendIO ( (struct IORequest *) timeout_req );
  }

void StartRead ( buffer, length )
unsigned char   *buffer;
unsigned int    length;
  {
    /*
    printf ( "Read of %4d bytes..", length );
    */
    ser_req.IOSer.io_Length  = length;
    ser_req.IOSer.io_Data    = (APTR) buffer;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_READ;
    SendIO ( (struct IORequest *) &ser_req );
    /*
    printf ( " started\n" );
    */
  }
  

void SendChar ( c )
unsigned char   c;
  {
    unsigned long int   chr;

    ( *(unsigned char *) &chr ) = c;
    
    /*
    printf ( "Byte %02x..", c );
    */

    ser_req.IOSer.io_Data    = (APTR) &chr;
    ser_req.IOSer.io_Length  = 1;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_WRITE;
    SendIO ( (struct IORequest *) &ser_req );
    Wait   ( 1L << ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit );
    GetMsg ( ser_req.IOSer.io_Message.mn_ReplyPort );
    /*
    DoIO ( (struct IORequest *) &ser_req );
    */
    
    /*
    printf ( " sent\n" );
    */
  }

unsigned short int ReceiveFile ( unsigned short int protocol,
                                 unsigned char *file )
  {
    unsigned short int status;

    if ( protocol != XON_XOFF )
        SleepTasks ();

    switch ( protocol )
      {
        case XON_XOFF:
            status = BeginASCIIReceive ( file );
        case XMODEM :
        case XMODEM_CRC :
        case YMODEM :
            status = BeginXmodemReceive ( protocol, file );
			break;
        case YMODEM_BATCH :
            status = BeginYmodemReceive ( protocol, file );
			break;
        case KERMIT :
            status = BeginKermitReceive ( 1, file );
			break;
        case KERMIT_7BIT :
            status = BeginKermitReceive ( 2, file );
			break;
        case KERMIT_TEXT :
            status = BeginKermitReceive ( 0, file );
			break;
        case XPROTOCOL :
            status = BeginXProtocolReceive ( file );
			break;
      }
    if ( protocol != XON_XOFF )
        WakeTasks ();

    return ( status );
  }

unsigned short int TransmitFile ( unsigned short int protocol,
                                  unsigned char *file )
  {
    unsigned short int status;

    SleepTasks ();

    switch ( protocol )
      {
        case XON_XOFF:
            status = BeginASCIISend ( file );
        case XMODEM:
        case XMODEM_CRC:
        case YMODEM:
            status = BeginXmodemSend ( file );
			break;
        case YMODEM_BATCH:
            status = BeginYmodemSend ( file );
			break;
        case KERMIT :
            status = BeginKermitSend ( 1, file );
			break;
        case KERMIT_7BIT :
            status = BeginKermitSend ( 2, file );
			break;
        case KERMIT_TEXT :
            status = BeginKermitSend ( 0, file );
			break;
        case XPROTOCOL :
            status = BeginXProtocolSend ( file );
			break;
      }
    
    WakeTasks ();

    return ( status );
  }
