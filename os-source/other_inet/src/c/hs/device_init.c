/* -----------------------------------------------------------------------
 * device_init.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: device_init.c,v 1.1 91/05/09 15:37:47 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/device_init.c,v 1.1 91/05/09 15:37:47 bj Exp $
 *
 * $Log:	device_init.c,v $
 * Revision 1.1  91/05/09  15:37:47  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Device initialization subroutines
*
***/

#include    "termall.h"
extern BPTR               _Backstdout;

/***
*
* Initialize the Serial Device
*
***/
struct MsgPort  *InitSerialIO ( ser_req, string )
struct IOExtSer *ser_req;
unsigned char   *string;
  {
    struct MsgPort  *temp_ser_port;
    int             error;

    /*
    * Make sure that a driver has been specified.
    */
    if ( !*nvmodes.serial_driver )
        strcpy ( nvmodes.serial_driver, SERIALNAME );
    
    /*
    if ( _Backstdout )
        Write ( _Backstdout,
                nvmodes.serial_driver,
                (long)strlen ( nvmodes.serial_driver) );
    */
      
    /*
    * Create a reply port for the serial device.
    */
    temp_ser_port = CreatePort ( /*string*/ NULL, 0 );
    ser_req->IOSer.io_Message.mn_ReplyPort = temp_ser_port;
    
    /*
    * Open the serial device.
    */
    ser_req->io_SerFlags = SERF_SHARED;

    if ( nvmodes.seven_wire )
       ser_req->io_SerFlags |= SERF_7WIRE;
    else
       ser_req->io_SerFlags &= ~SERF_7WIRE;
    
    error = OpenDevice ( nvmodes.serial_driver,
                         (long)nvmodes.serial_port,
                         (struct IORequest *) ser_req,
                         0 );
    if ( error )
        return ( 0 );

    return ( temp_ser_port );
  }

/*----------------------------------------------------------------------------*/

/***
*
* Initialize the Console Device.
*
***/
struct MsgPort *InitConsoleIO ( con_req, string )
struct IOStdReq *con_req;
unsigned char   *string;
  {
    struct MsgPort  *temp_con_port;
    int             error;

    if ( !Screen )
        return ( NULL );

    /*
    * Open the console output device.
    */
    con_req->io_Data = (APTR) Window;
    error = OpenDevice ( "console.device", 0L, (struct IORequest *) con_req, 0L );
    if ( error )
        return ( 0 );

    /*
    * Create a reply port for the console output device.
    */
    temp_con_port = CreatePort ( /*string*/ NULL, 0 );
    con_req->io_Message.mn_ReplyPort = temp_con_port;
    return ( temp_con_port );
  }

/*----------------------------------------------------------------------------*/

/***
*
* Initialize the Timer Device.
*
***/
struct MsgPort *InitTimer ( tim_req, string )
struct timerequest *tim_req;
unsigned char   *string;
  {
    struct MsgPort  *temp_tim_port;
    int             error;

    /*
    * Open the timer device.
    */
    error = OpenDevice ( "timer.device", UNIT_VBLANK,
                         (struct IORequest *) tim_req, 0 );
    if ( error )
        return ( 0 );

    /*
    * Create a reply port for the console output device.
    */
    temp_tim_port = CreatePort ( /*string*/ NULL, 0 );
    tim_req->tr_node.io_Message.mn_ReplyPort = temp_tim_port;
    return ( temp_tim_port );
  }

/*----------------------------------------------------------------------------*/

/***
*
* Set serial port parameters
*
***/
int SetParams ( ser_req, flow )
struct IOExtSer     *ser_req;
unsigned short int  flow;
{
    int error;
    
    if ( flow )
      {
        ser_req->IOSer.io_Flags  &= ~IOF_QUICK;
        ser_req->IOSer.io_Command = CMD_STOP;
        DoIO ( (struct IORequest *) ser_req);
      }
    ser_req->IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req->IOSer.io_Command = CMD_CLEAR;
    DoIO ( (struct IORequest *) ser_req);

    ser_req->IOSer.io_Flags          = 0x00;
    ser_req->io_CtlChar              = 0x11130000;
    ser_req->io_BrkTime              = 250000;
    ser_req->io_Baud                 = nvmodes.lspeed;
    ser_req->io_ReadLen              = nvmodes.bpc;
    ser_req->io_WriteLen             = nvmodes.bpc;
    ser_req->io_StopBits             = nvmodes.stpbits;
    ser_req->io_RBufLen              = 1536;
    ser_req->io_TermArray.TermArray0 = 0x51040303;
    ser_req->io_TermArray.TermArray1 = 0x03030303;
    ser_req->io_SerFlags             = SERF_SHARED;
    ser_req->IOSer.io_Flags         &= ~IOF_QUICK;
    ser_req->IOSer.io_Command        = SDCMD_SETPARAMS;
    ser_req->io_ExtFlags             = 0;
    
    switch ( nvmodes.parity )
      {
        case 0: /* No Parity */
            ser_req->io_SerFlags &= ~SERF_PARTY_ON;
            ser_req->io_ExtFlags  =  0;
            break;
        case 1: /* Odd Parity */
            ser_req->io_SerFlags |=  SERF_PARTY_ON | SERF_PARTY_ODD;
            ser_req->io_ExtFlags  =  0;
            break;
        case 2: /* Even Parity */
            ser_req->io_SerFlags |=  SERF_PARTY_ON;
            ser_req->io_SerFlags &= ~SERF_PARTY_ODD;
            ser_req->io_ExtFlags  =  0;
            break;
        case 3: /* Mark Parity */
            ser_req->io_ExtFlags |=  SEXTF_MSPON | SEXTF_MARK;
            break;
        case 4: /* Space Parity */
            ser_req->io_ExtFlags |=  SEXTF_MSPON;
            ser_req->io_ExtFlags &= ~SEXTF_MARK;
            break;
      }
    
    if ( flow && (nvmodes.axonxoff == 1) )
      {
            ser_req->io_SerFlags &= ~SERF_XDISABLED;
            ser_req->io_SerFlags &= ~SERF_RAD_BOOGIE;
      }
        else
      {
            ser_req->io_SerFlags |=  SERF_XDISABLED;
            ser_req->io_SerFlags |=  SERF_RAD_BOOGIE;
      }
    
    if ( nvmodes.seven_wire )
        ser_req->io_SerFlags |=  SERF_7WIRE;
    else
        ser_req->io_SerFlags &= ~SERF_7WIRE;
    
    error = DoIO ( (struct IORequest *) ser_req);

    if ( flow )
      {
        ser_req->IOSer.io_Flags  &= ~IOF_QUICK;
        ser_req->IOSer.io_Command = CMD_START;
        error = DoIO ( (struct IORequest *) ser_req);
      }

    return (error);
}
