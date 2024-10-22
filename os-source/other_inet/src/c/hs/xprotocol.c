/* -----------------------------------------------------------------------
 * xprotocol.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: xprotocol.c,v 1.1 91/05/09 16:26:27 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/xprotocol.c,v 1.1 91/05/09 16:26:27 bj Exp $
 *
 * $Log:	xprotocol.c,v $
 * Revision 1.1  91/05/09  16:26:27  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include "termall.h"
#include "xproto.h"

extern long __stdargs XProtocolSetup   ( struct XPR_IO * );
extern long __stdargs XProtocolSend    ( struct XPR_IO * );
extern long __stdargs XProtocolReceive ( struct XPR_IO * );
extern long __stdargs XProtocolCleanup ( struct XPR_IO * );
extern long __stdargs XProtocolHostMon ( struct XPR_IO *, char *, long, long );
extern long __stdargs XProtocolUserMon ( struct XPR_IO *, char *, long, long );

static struct XPR_IO xpr_io;
static long int      xprs;


void InitializeXProtocol ( void )
  {
    unsigned char xpr_init[80];
    unsigned char *initptr;

    /*
    * Open the External Protocol Library.
    */

    ShutXProtocol ();

    XProtocolBase = OpenLibrary ( nvmodes.xprotocol, 0L );
    if ( !XProtocolBase )
      {
        Error ( "External Protocol not found" );
        return;
      }

    /*
    * Get the Protocol Intialization string form the user.
    */
    stcgfn ( xpr_init, nvmodes.xprotocol );
    for ( initptr = xpr_init; *initptr != '.' && *initptr != '\0'; initptr++ );
    *initptr = '\0';

    if ( initptr = getenv ( xpr_init ) )
      {
        strcpy ( xpr_init, initptr );
        free ( initptr );
      }
    else
        *xpr_init = '\0';

    /*
    * Initialize the External protocol structure.
    */

    WWindow = NULL;
    TWindow = NULL;
    xpr_setup ( &xpr_io );        
    xpr_io.xpr_filename = xpr_init;
    xprs = XProtocolSetup ( &xpr_io );
    if ( xprs == XPRS_FAILURE )
      {
        ShutXProtocol ();
        CloseTransferWindow ();
        Error ( "External Protocol Initialization Failed" );
      }

  }

void XProtocolParams ()
  {
    if ( !XProtocolBase )
        return;

    xpr_io.xpr_filename = NULL;
    xprs = XProtocolSetup ( &xpr_io );
    if ( xprs == XPRS_FAILURE )
      {
        ShutXProtocol ();
        CloseTransferWindow ();
        Error ( "External Protocol Initialization Failed" );
      }
    
  }

void ShutXProtocol ()
  {
    if ( XProtocolBase )
      {
        XProtocolCleanup ( &xpr_io );
        CloseLibrary ( XProtocolBase );
        XProtocolBase = NULL;
        xprs = XPRS_FAILURE;
      }
  }

long XprHostMon ( char *buffer, int actual, int max )
  {
    if ( XProtocolBase && xprs & XPRS_HOSTMON )
        return ( XProtocolHostMon ( &xpr_io, buffer, actual, max ) );
    return ( actual );
  }

long XprUserMon ( char *buffer, int actual, int max )
  {
    if ( XProtocolBase && xprs & XPRS_USERMON )
        return ( XProtocolUserMon ( &xpr_io, buffer, actual, max ) );
    return ( actual );
  }

unsigned short int __stdargs BeginXProtocolSend ( unsigned char *source )
  {
    if ( !XProtocolBase )
        return ( 0 );

    /*
    * Get file name if necessary, and build file list.
    */
    if ( !( xprs & XPRS_NOSNDREQ ) )
      {
        if ( source )
            strcpy ( nvmodes.pc_file, source );
        else if ( !FileReq ( "File(s) To Send", nvmodes.pc_file ) )
          {
            CloseTransferWindow ();
            return ( 0 );
          }
      }
    
    /*
    * Send the file
    */
    OpenTransferWindow ( "External Protocol Transmit", 550, 152 );
    xpr_io.xpr_filename = nvmodes.pc_file;
    XProtocolSend ( &xpr_io );
    Delay ( 100 );
    CloseTransferWindow ();

    return ( 0 );
  }

/*
*
*
*
*/

unsigned short int __stdargs BeginXProtocolReceive ( unsigned char *destination )

  {
    if ( !XProtocolBase )
        return ( 0 );

    /*
    * Get file name if necessary, and build file list.
    */
    if ( !( xprs & XPRS_NORECREQ ) )
      {
        if ( destination )
            strcpy ( nvmodes.dest_path, destination );
        else if ( !FileReq ( "Specify Destination", nvmodes.dest_path ) )
          {
            CloseTransferWindow ();
            return ( 0 );
          }
      }
    
    /*
    * Receive the file
    */
    OpenTransferWindow ( "External Protocol Receive", 550, 152 );
    xpr_io.xpr_filename = nvmodes.dest_path;
    XProtocolReceive ( &xpr_io );
    Delay ( 100 );
    CloseTransferWindow ();

    return ( 0 );
  }

/*
*
*
*
*/

void BeginRead ( unsigned char *buffer, long int length )
  {
    ser_req.IOSer.io_Length  = length;
    ser_req.IOSer.io_Data    = (APTR) buffer;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_READ;
    SendIO ( (struct IORequest *) &ser_req );
  }
  

void BeginTimer ( unsigned long int micros )
  {
    timeout_req.tr_time.tv_secs    = micros / 1000000;
    timeout_req.tr_time.tv_micro   = micros % 1000000;
    timeout_req.tr_node.io_Command = TR_ADDREQUEST;
    SendIO ( (struct IORequest *) &timeout_req );
  }

/*
*
*
*
*/

void SayTransfer ( int x, int y, unsigned char *control,
                   unsigned char *strarg, long longarg1, long longarg2 )
  {
    unsigned char text[64];

    if ( strarg )
        sprintf ( text, control, strarg );
    else if ( longarg2 != -1 )
        sprintf ( text, control, longarg1, longarg2 );
    else if ( longarg1 != -1 )
        sprintf ( text, control, longarg1 );
    else
        strcpy  ( text, control );

    if ( TWindow )
      {
        Move ( TWindow->RPort, x, y );
        Text ( TWindow->RPort, text, strlen ( text ) );
      }

    if ( WWindow )
      {
        Move ( WWindow->RPort, x, y );
        Text ( WWindow->RPort, text, strlen ( text ) );
      }
  }


/*
*
* External Protocol call back functions
*
*/

static int fcount = 0;

long __asm __stdargs __saveds xpr_fopen ( register __a0 char *filename,
                                          register __a1 char *accessmode )
  {
    /*
    TransferMessage ( "XPR_fopen\r\n" );
    */
    fcount++;
    return ( (long) fopen ( filename, accessmode ) );
  }

long __asm __stdargs __saveds xpr_fclose ( register __a0 FILE *filepointer )
  {
    /*
    TransferMessage ( "XPR_close\r\n" );
    */
    if ( filepointer )
      {
        fflush ( filepointer );
        return ( (long) fclose ( filepointer ) );
      }
    return ( 0L );
  }

long __asm __stdargs __saveds xpr_fread ( register __a0 char *buffer,
                                          register __d0 long size,
                                          register __d1 long count,
                                          register __a1 FILE *filepointer )
  {
    /*
    TransferMessage ( "XPR_fread\r\n" );
    */
    return ( (long) fread ( buffer, size, count, filepointer ) );
  }

long __asm __stdargs __saveds xpr_fwrite ( register __a0 char *buffer,
                                           register __d0 long size,
                                           register __d1 long count,
                                           register __a1 FILE *filepointer )
  {
    /*
    TransferMessage ( "XPR_fwrite\r\n" );
    */
    return ( (long) fwrite ( buffer, size, count, filepointer ) );
  }

long __asm __stdargs __saveds xpr_sread ( register __a0 char *buffer,
                                          register __d0 long size,
                                          register __d1 long timeout )
  {
    unsigned long int   mask,
                        io_mask,
                        tr_mask,
                        signals;
    long int count = 0L;
    unsigned char inbuff[4];
    /*
    unsigned char message[80];

    sprintf ( message, "XPR_sread ( %08x, %5ld, %8ld ) ... ", buffer, size, timeout );
    TransferMessage ( message );
    */

    tr_mask = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    io_mask = 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;
    mask    = tr_mask | io_mask;

    if ( size == 0L )
        return ( 0L );

    ser_req.IOSer.io_Command = SDCMD_QUERY;
    DoIO ( (struct IORequest *) &ser_req );
    count = ser_req.IOSer.io_Actual;

    if ( count >= size )
      {
        count = ( count > size ) ? size : count;
        BeginRead ( buffer, count );
        WaitIO ( (struct IORequest *) &ser_req );
        /*
        Wait ( io_mask );
        GetMsg ( ser_port );
        */
        /*
        TransferMessage ( "out 1\n\r" );
        */
        return ( count );
      }

    if ( count )
      {
        BeginRead ( buffer, count );
        WaitIO ( (struct IORequest *) &ser_req );
        /*
        Wait ( io_mask );
        GetMsg ( ser_port );
        */
      }

    if ( timeout == 0L )
      {
        /*
        TransferMessage ( "out 2\n\r" );
        */
        return ( count );
      }

    BeginTimer ( timeout );
    do
      {
        BeginRead  ( inbuff, 1 );
        signals = Wait ( mask );
        if ( signals & io_mask )
          {
            WaitIO ( (struct IORequest *) &ser_req );
            /*
            GetMsg ( ser_port );
            */
            buffer[count++] = inbuff[0];
          }

        if ( signals & tr_mask )
            break;

      } while ( count < size );

    if ( !( signals & tr_mask ) )
      {
        AbortIO ( (struct IORequest *) &timeout_req );
        Wait ( tr_mask );
      }
    GetMsg ( timeout_port );

    if ( count != size )
      {
        if ( !( signals & io_mask ) )
          {
            AbortIO ( (struct IORequest *) &ser_req );
            WaitIO ( (struct IORequest *) &ser_req );
            /*
            Wait    ( io_mask  );
            GetMsg  ( ser_port );
            */
          }
      }

    /*
    TransferMessage ( "out 3\n\r" );
    */
    return ( count );
  }

long __asm __stdargs __saveds xpr_swrite ( register __a0 char *buffer,
                                           register __d0 long size )
  {
    /*
    TransferMessage ( "XPR_swrite\r\n" );
    */
    ser_req.IOSer.io_Length  = size;
    ser_req.IOSer.io_Data    = (APTR) buffer;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_WRITE;
    DoIO ( (struct IORequest *) &ser_req );
    return ( 0L );
  }

long __asm __stdargs __saveds xpr_sflush ( void )
  {
    /*
    TransferMessage ( "XPR_sflush\r\n" );
    */
    ser_req.IOSer.io_Command = CMD_FLUSH;
    SendIO ( (struct IORequest *) &ser_req );
    Wait ( 1L <<       ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit );
    GetMsg ( ser_port );
    return ( 0L );
  }

long __asm __stdargs __saveds xpr_update ( register __a0 struct XPR_UPDATE *upd )
  {
    /*
    TransferMessage ( "XPR_update\r\n" );
    */

    if ( upd->xpru_updatemask & XPRU_PROTOCOL &&
         upd->xpru_protocol )
        SayTransfer ( 8, 17, "Protocol: %-50s", upd->xpru_protocol, -1, -1 );

    if ( upd->xpru_updatemask & XPRU_FILENAME &&
         upd->xpru_filename )
        SayTransfer ( 8, 25, "    File: %-50s", upd->xpru_filename, -1, -1  );

    if ( upd->xpru_updatemask & XPRU_MSG &&
         upd->xpru_msg )
        SayTransfer ( 8, 33, " Message: %-50s", upd->xpru_msg, -1, -1 );

    if ( upd->xpru_updatemask & XPRU_ERRORMSG &&
         upd->xpru_errormsg )
        SayTransfer ( 8, 41, "   Error: %-50s", upd->xpru_errormsg, -1, -1 );

    if ( upd->xpru_updatemask & XPRU_BLOCKCHECK &&
         upd->xpru_blockcheck )
        SayTransfer ( 8, 49, "   Check: %-50s", upd->xpru_blockcheck, -1, -1 );

    if ( upd->xpru_updatemask & XPRU_EXPECTTIME &&
         upd->xpru_expecttime )
        SayTransfer ( 8, 57, "Transfer time: %-20s",upd->xpru_expecttime, -1, -1 );

    if ( upd->xpru_updatemask & XPRU_ELAPSEDTIME &&
         upd->xpru_elapsedtime )
        SayTransfer ( 8, 65, "Elapsed  time: %-20s", upd->xpru_elapsedtime, -1, -1 );
    
    if ( upd->xpru_updatemask & XPRU_FILESIZE &&
         upd->xpru_filesize != -1 )
        SayTransfer ( 8, 73, "   File Size: %d bytes      ", NULL, upd->xpru_filesize, -1 );
    
    if ( upd->xpru_updatemask & XPRU_BLOCKS &&
         upd->xpru_blocks != -1 )
        SayTransfer ( 8, 81, "      Blocks: %-d           ", NULL, upd->xpru_blocks, -1 );
    
    if ( upd->xpru_updatemask & XPRU_BLOCKSIZE &&
         upd->xpru_blocksize != -1 )
        SayTransfer ( 8, 89, "  Block Size: %-d bytes     ", NULL, upd->xpru_blocksize, -1 );
    
    if ( upd->xpru_updatemask & XPRU_BYTES &&
         upd->xpru_filesize != -1 )
        SayTransfer ( 8, 97, "       Bytes: %-d           ", NULL, upd->xpru_bytes, -1 );
    
    if ( upd->xpru_updatemask & XPRU_ERRORS &&
         upd->xpru_errors != -1 )
        SayTransfer ( 8, 105,"      Errors: %-d           ", NULL, upd->xpru_errors, -1 );
    
    if ( upd->xpru_updatemask & XPRU_TIMEOUTS &&
         upd->xpru_timeouts != -1 )
        SayTransfer ( 8, 113,"    Timeouts: %-d           ", NULL, upd->xpru_timeouts, -1 );
    
    if ( upd->xpru_updatemask & XPRU_PACKETTYPE &&
         upd->xpru_packettype != -1 )
        SayTransfer ( 8, 121," Packet Type: %-d           ", NULL, upd->xpru_packettype, -1 );
    
    if ( upd->xpru_updatemask & XPRU_PACKETDELAY &&
         upd->xpru_packetdelay != -1 )
        SayTransfer ( 8, 129,"Packet Delay: %-d           ", NULL, upd->xpru_packetdelay, -1 );
    
    if ( upd->xpru_updatemask & XPRU_CHARDELAY &&
         upd->xpru_chardelay != -1 )
        SayTransfer ( 8, 137," Char. Delay: %-d           ", NULL, upd->xpru_chardelay, -1 );
    
    if ( upd->xpru_updatemask & XPRU_DATARATE &&
         upd->xpru_datarate != -1 )
        SayTransfer ( 8, 145,"   Data Rate: %5d bytes per second", NULL, upd->xpru_datarate, -1 );
    
    return ( 0L );
  }

long __asm __stdargs __saveds xpr_chkabort ( void )
  {
    /*
    TransferMessage ( "XPR_chkabort\r\n" );
    */
    return ( CheckForWindowAbort () );
  }

long __asm __stdargs __saveds xpr_chkmisc ( void )
  {
    /*
    TransferMessage ( "XPR_chkmisc\r\n" );
    */
    return ( 0L );
  }

long __asm __stdargs __saveds xpr_gets ( register __a0 char *prompt,
                                         register __a1 char *buffer )
  {
    /*
    TransferMessage ( "XPR_gets\r\n" );
    */
    GetStringFromUser ( prompt, buffer, 256 );
    return ( 1L );
  }

long __asm __stdargs __saveds xpr_setserial ( register __d0 long newstatus )
  {
    long int    oldstatus;
    /*
    TransferMessage ( "XPR_setserial\r\n" );
    */
    ser_req.IOSer.io_Command = SDCMD_QUERY;
    DoIO ( (struct IORequest *) &ser_req );

    oldstatus  = ser_req.io_SerFlags;
    oldstatus |= ser_req.io_ExtFlags << 8;

    if ( ser_req.io_StopBits == 2 )
        oldstatus |= 0x00000400;
    if ( ser_req.io_ReadLen  == 7 )
        oldstatus |= 0x00000800;
    if ( ser_req.io_WriteLen == 7 )
        oldstatus |= 0x00001000;

    switch ( ser_req.io_Baud )
      {
        case 110:
            oldstatus |= 0 << 16;
            break;

        case 300:
            oldstatus |= 1 << 16;
            break;

        case 1200:
            oldstatus |= 2 << 16;
            break;

        case 2400:
            oldstatus |= 3 << 16;
            break;

        case 4800:
            oldstatus |= 4 << 16;
            break;

        case 9600:
            oldstatus |= 5 << 16;
            break;

        case 19200:
            oldstatus |= 6 << 16;
            break;

        case 32500:
            oldstatus |= 7 << 16;
            break;

        case 38400:
            oldstatus |= 8 << 16;
            break;

        case 57600:
            oldstatus |= 9 << 16;
            break;

        case 76800:
            oldstatus |= 10 << 16;
            break;

        case 115200:
            oldstatus |= 11 << 16;
            break;
      }

    if ( newstatus == -1L )
        return ( oldstatus );

    ser_req.io_SerFlags =   newstatus & 0x000000ff;
    ser_req.io_ExtFlags = ( newstatus & 0x00000300 ) >> 8;

    ser_req.io_StopBits = ( newstatus & 0x00000400 ) ? 2 : 1;
    ser_req.io_ReadLen  = ( newstatus & 0x00000800 ) ? 7 : 8;
    ser_req.io_WriteLen = ( newstatus & 0x00001000 ) ? 7 : 8;

    switch ( newstatus >> 16 )
      {
        case 0:
            ser_req.io_Baud = 110;
            break;

        case 1:
            ser_req.io_Baud = 300;
            break;

        case 2:
            ser_req.io_Baud = 1200;
            break;

        case 3:
            ser_req.io_Baud = 2400;
            break;

        case 4:
            ser_req.io_Baud = 4800;
            break;

        case 5:
            ser_req.io_Baud = 9600;
            break;

        case 6:
            ser_req.io_Baud = 19200;
            break;

        case 7:
            ser_req.io_Baud = 32500;
            break;

        case 8:
            ser_req.io_Baud = 38400;
            break;

        case 9:
            ser_req.io_Baud = 57600;
            break;

        case 10:
            ser_req.io_Baud = 76800;
            break;

        case 11:
            ser_req.io_Baud = 115200;
            break;
      }

    ser_req.IOSer.io_Command = SDCMD_SETPARAMS;
    DoIO ( (struct IORequest *) &ser_req );

    return ( oldstatus );
  }

static unsigned char    **files;
static unsigned long    file_idx = 0;

long __asm __stdargs __saveds xpr_ffirst ( register __a0 char *buffer,
                                           register __a1 char *pattern )
  {
    /*
    TransferMessage ( "XPR_ffirst\r\n" );
    */
    if ( !( files = FileList ( pattern ) ) )
        return ( 0L );

    file_idx = 0;
    strcpy ( buffer, files[ file_idx++ ] );

    return ( (long) file_idx );    
  }

long __asm __stdargs __saveds xpr_fnext ( register __d0 long oldstate,
                                          register __a0 char *buffer,
                                          register __a1 char *pattern )
  {
    /*
    TransferMessage ( "XPR_fnext\r\n" );
    */
    if ( *files [ oldstate ] )
      { 
        strcpy ( buffer, files [ oldstate++ ] );
        return ( oldstate );
      }

    return ( 0L );
  }

long __asm __stdargs __saveds xpr_finfo ( register __a0 char *filename,
                                          register __d0 long typeofinfo )

  {
    FILE *fp;
    long status;

    /*
    TransferMessage ( "XPR_finfo\r\n" );
    */
    if ( typeofinfo == 1 )
      {
        fp = fopen ( filename, "r" );
        if ( !fp )
            return ( 0L );

        fseek ( fp, 0L, 2 );
        status = ftell (fp);
        fclose ( fp );
      }
    else
        status = 1;

    return ( status );
  }

long __asm __stdargs __saveds xpr_fseek ( register __a0 FILE *filepointer,
                                          register __d0 long offset,
                                          register __d1 long origin )
  {
    /*
    TransferMessage ( "XPR_fseek\r\n" );
    */
    return ( fseek ( filepointer, offset, origin ) );
  }

long __asm __stdargs __saveds xpr_options ( register __d0 long n,
                                            register __a0 struct xpr_option *opt[] )
  {
    unsigned char   Header[51] = "";
    unsigned char   Title[81] = "";
    unsigned short  int i;

    for ( i = 0; i < n; i++ )
      {
        if ( opt[i]->xpro_type == XPRO_HEADER )
            strcpy ( Header, opt[i]->xpro_description );
        else
          {
            strcpy ( Title, Header );
            strcat ( Title, " " );
            strcat ( Title, opt[i]->xpro_description );
            GetStringFromUser ( Title, opt[i]->xpro_value, opt[i]->xpro_length );
          }
          
      }
    return ( 0x07ffffffL );
  }

long __asm __stdargs __saveds xpr_unlink ( register __a0 char *filename )
  {
    return ( unlink ( filename ) );
  }

long __asm __stdargs __saveds xpr_squery ( void )
  {
    ser_req.IOSer.io_Command = SDCMD_QUERY;
    DoIO ( (struct IORequest *) &ser_req );
    return ( (long int) ser_req.IOSer.io_Actual );
    
  }

long __asm __stdargs __saveds xpr_getptr ( register __d0 long type )
  {
    if ( type == 1L )
        return ( (long int) Screen );

    return ( -1L );
  }