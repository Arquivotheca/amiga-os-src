/* From joe Thu Oct 26 09:48:17 1989

	       AMIGA BUG REPORT   CBM SOFTWARE NEW PRODUCT ASSURANCE

   Name:    C. Perry Weinthal		       Date:  10/25/89

   Product: A2232 - Serial.Device

   Bug Location: Hardware      [ ]	Software (A2232 - serial.device) [x]
		 Documentation [ ]	Other  (specify below)           [ ]

   Configuration: WorkBench	   1.3
		  Number of Disks  2	Revs:	34.5 Kickstart
		  Chip Memory	   1MB		34.21  Workbench
		  Fast Memory	   2MB (A2052)        CLI
				   2MB (A2630)
		  Other Equipment: A2630, A2090A, A1084S, two Rodime RD3055s


   Brief Bug Description:

     Prelinary Hardware Test Code (PHTC) will output properly
     on serial port # 1 (mother board).   If any of the A2232 ports are
     used, it will only output one or two bytes, if the test code is called
     repeatedly it will output successive bytes of the original string.
     If a null string is included the previous string will be used- valid only
     for the A2232 port.

   Bug Generation Procedure:

    Run at cli	PHTC [unit] [baud] "string up to 80 char"
    unit = 1 thru 8 ( 1 = mother board, 2 thru 8 = A2232 units )
    baud = 110, 300, ...... 9600 ( all valid options )

   Related Bugs:

    None identified at this time.

   Additional notes:

    Source code follows.

 */
/*
 *   Serial
 *
 *   DESCRIPTION:
 *   This file contains the serial i/o routines
 *	OpenSerial
 *	CloseSerial
 *	SerialWrite
 *	SerialRead
 *	AbortSerialRead
 *
 *   Programmer      Date	  Action
 *   Linda	     28-Aug-1989  Initial Coding.
 *
 */

#include <exec/types.h>
#include <devices/serial.h>

#include <proto/exec.h>

/*#include "ads.h" */

static struct IOExtSer *in    = NULL;
static struct IOExtSer *out   = NULL;
static struct MsgPort  *pin   = NULL;
static struct MsgPort  *pout  = NULL;

BOOL OpenSerial (device, unit, baud)

char	   *device;
ULONG	    unit, baud;

{

  if ((pin = CreatePort (NULL, 0)) == NULL)
    return FALSE;

  if ((in = (struct IOExtSer *)
    CreateExtIO (pin, sizeof (struct IOExtSer))) == NULL) {
    DeletePort (pin);
    return FALSE;
  }

  if (OpenDevice (device, unit, (struct IORequest *) in, 0) != 0) {
    DeleteExtIO ((struct IORequest *) in);
    DeletePort (pin);
    return FALSE;
  }

  in->IOSer.io_Command	  = SDCMD_SETPARAMS;
  in->io_Baud		  = baud;
  in->io_ReadLen	  = 8;
  in->io_WriteLen	  = 8;
  in->io_StopBits	  = 1;
  in->io_SerFlags	  = 0;
  in->io_RBufLen	  = 5000;
  in->IOSer.io_Flags	  = 0;
  in->io_ExtFlags	  = 0;
  in->io_SerFlags	  = 0;

  DoIO ((struct IORequest *) in);

  if ((pout = CreatePort (NULL, 0)) == NULL) {
    DeletePort (pin);
    DeleteExtIO ((struct IORequest *) in);
    return FALSE;
  }

  if ((out = (struct IOExtSer *)
    CreateExtIO (pout, sizeof (struct IOExtSer))) == NULL) {
    DeletePort (pin);
    DeletePort (pout);
    DeleteExtIO ((struct IORequest *) in);
    return FALSE;
  }

  *out = *in;
  out->IOSer.io_Message.mn_ReplyPort = pout;

  return TRUE;

}

void CloseSerial ()

{

  if (in) {
    if (in->IOSer.io_Device)
      CloseDevice ((struct IORequest *) in);
    DeleteExtIO ((struct IORequest *) in);
  }

  if (out)
    DeleteExtIO ((struct IORequest *) out);

  if (pin)
    DeletePort (pin);

  if (pout)
    DeletePort (pout);

  return;

}

ULONG SerialRead (data, len)

void	  *data;
USHORT	   len;

{

  in->IOSer.io_Command	= CMD_READ;
  in->IOSer.io_Data	= data;
  in->IOSer.io_Length	= len;

  SendIO ((struct IORequest *) in);

  return (ULONG) (1 << pin->mp_SigBit);

}

void AbortSerialRead ()

{

  if (!CheckIO ((struct IORequest *) in)) {
    AbortIO ((struct IORequest *) in);
    Wait (1 << pin->mp_SigBit);
    GetMsg (pin);
  }

  return;

}

void SerialWrite (data, len)

void	  *data;
USHORT	   len;

{

  out->IOSer.io_Command  = CMD_WRITE;
  out->IOSer.io_Data	 = data;
  out->IOSer.io_Length	 = len;

  DoIO ((struct IORequest *) out);

  return;

}

main (argc, argv)

int argc;
char *argv[];

{

  int unit, baud;
  char crlf[2];
   crlf[0]=10;
   crlf[1]=13;

  sscanf (argv [1], "%d", &unit);
  sscanf (argv [2], "%d", &baud);

  if (!OpenSerial ("serial.device", unit, baud)) {
    printf ("can't open serial\n");
    exit (20);
  }

  SerialWrite (argv[3],(strlen(argv[3])+1));
  SerialWrite (crlf,3);

  CloseSerial ();

}

