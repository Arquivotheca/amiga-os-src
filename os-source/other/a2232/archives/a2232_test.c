/***************************************************************************

   This program is used to test the A2232 multiport serial card.  The
   following tests are performed:

	- The 16k bytes of static RAM on the A2232 board is checked by
	  the Amiga's CPU.
	- Software for the 6502 is downloaded to the A2232's RAM.  This
	  software is in the file called '6502test.obj', and is in MOS
	  hex format. The Amiga also verifies that it was written OK.
	- The 6502 is started, allowing the code just downloaded to
	  execute.  This code has the 6502 test most of the 16k bytes
	  of RAM from its 'point of view'.  It also reads and writes
	  to the CONTROL register of each of the ACIAs to check their
	  operation, and watches for spurious IRQ's.  The Amiga tells
	  the 6502 how many passes to do, and then watches the count
	  value in the shared RAM, waiting for the 6502 to finish.
	  The Amiga then reads the error count values to see if there
	  were any errors.
	- The Amiga then changes the RESET vector for the 6502 to point
	  to another section of the code that was loaded.  This code
	  checks out the operation of one of the serial lines.	This
	  requires that a special cable be plugged into the port
	  that is being checked, configured as folows:

	   SIGNAL     25 PIN   8 PIN	      SIGNAL	  25 PIN   8 PIN
	    NAME      D CONN	DIN	       NAME	  D CONN    DIN
	   -------------------------------------------------------------
	   RxD(in)      2        1            TxD(out)      3        2
	   RTS(out)     4        3            DCD(in)       8        7
	   DTR(out)    20        8            DSR,CTS(in)   6,5      5,4

	 The Amiga 'pokes' the number of the channel to be checked
	 into a variable in the shared RAM, and then resets the 6502.
	 This is done for each of the 7 ports (0 thru 6).
	 The errors reported are:

	     $01 : Transmitter not sending 1st byte.
	     $02 : Transmitter not sending 2nd byte.
	     $04 : Receiver didn't get the byte that was sent.
	     $06 : Byte received is incorrect.
	     $08 : CTS did not stop transmitter.
	     $0A : Control line error (RTS=0,DTR=0)
	     $0C : Control line error (RTS=0,DTR=1)
	     $0E : Control line error (RTS=1,DTR=1)
	     $10 : Control line error (RTS=1,DTR=0)

**************************************************************************/

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/io.h>
#include <libraries/configvars.h>
#include <libraries/configregs.h>
#include <lattice/stdio.h>
#include <libraries/dos.h>

struct Library *ExpansionBase;
extern struct Library *OpenLibrary();
extern struct ConfigDev *FindConfigDev();

UBYTE getbyte();
UWORD getword();
int memtestl();

/* ----------------------------------------------------------------------- */

void main(argc,argv)

	int argc;
	char *argv[];

{

struct ConfigDev *dev = NULL;
UWORD Manuf = 0X202;
UBYTE Product = 70, *adr, *base, bytecount;
UBYTE STARTFLAG=0,PASSES=1,MEMFLAG=4,ACIAFLAG=5,IRQFLAG=12;
USHORT ERRORS=0x1001,NUMERRORS=0x1000,CHANNEL=0x3e00,FINISHFLAG=0x3e01;
USHORT ACK=0x3e02,CHECKACIAS=0x3e03;
char  *filename={"6502test.obj"}, line[200], *lineptr;
FILE *fopen(), *fp;
int i,j,bad;

ExpansionBase = OpenLibrary("expansion.library",33);
if (ExpansionBase == NULL) exit(100);

printf("\nA2232 Test : V1.1\n");

if (dev = FindConfigDev(dev,Manuf,Product)) { /* Look for the board */
  base = (UBYTE *)(*dev).cd_BoardAddr;

  printf("\nA2232 serial card found at %x\n",base);

  /* Test the memory from the Amiga's side. */
  *(base+0X8000) = 0; /* Make sure the 6502 is stopped. */
  printf("Testing memory from the Amiga's side ... ");
  if ( memtestl( (ULONG *)base, (ULONG *)(base+0x4000-4),10 ) ) {

    printf("Fail!\n");
    exit(1);
  }
  printf("OK\n");

  if ((fp = fopen(filename,"r")) == NULL) { /* Try to open the file */
    printf("Can't open file: %s !\n",filename);
    exit(1);
  } /* if */
  else printf("Downloading code to board ... ");

  /* Copy the file to the RAM on the A2232 board. */
  while (fgets(line, 100, fp) != NULL) { /* read each record */
    /* printf("%s",line); */
    lineptr = line + 1;
    if ( (bytecount = getbyte(&lineptr) ) != 0) {
      adr = base + getword(&lineptr); /* Address for this record */
      for (;bytecount > 0; bytecount--) *adr++ = getbyte(&lineptr); /* store each byte */
    } /* if */
  } /* while */

  /* Verify that the file was written correctly */
  lseek(fp,0L,0); /* rewind the file */
  while (fgets(line, 100, fp) != NULL) { /* read each record */
    lineptr = line + 1;
    if ( (bytecount = getbyte(&lineptr) ) != 0) {
      adr = base + getword(&lineptr); /* Address for this record */
      for (;bytecount > 0; bytecount--) {
	if ( *adr++ != getbyte(&lineptr) ) { /* verify each byte */
	  printf("Verify error!\n");
	  exit(1);
	} /* if */
      } /* for */
    } /* if */
  } /* while */

  printf ("loaded.\n",getword(&lineptr));

  *(base+PASSES)=10;
  *(base+0XC000) = 0; /* Start the 6502. */
  printf("Testing memory from the 6502's side ... ");
  Delay(10); /* Give the 6502 a chance to reset the 'IBESTARTED' byte ... */
  if ( *(base+STARTFLAG) != 0 ) {
    printf("6502 did not start!\n");
    exit(1);
  }

  while ( *(base+PASSES) != 0XFF )  /* Wait for the 6502 to finish its test */
    /* printf("\n%d passes left",*(base+PASSES) ) */ ;

  /* Send IRQ's to 6502 to test this feature */
  for (i=1;i<=100;i++){
    *(base+0xA000)=0;  /* Set IRQ low */
    Delay(1);  /* Give the 6502 time to respond and reset it */
  }

  /* Check for any errors */
  bad=0;
  if ( *(base+MEMFLAG) != 0 ) {
    printf("  \nMemory error! ");
    bad |= 1;
  }
  for (i=0;i<=6;i++)
    if ( *(base+ACIAFLAG+i) != 0 ) {
      printf("\n  ACIA #%d register error! ",i);
      bad |= 2;
    }
  if ( *(base+IRQFLAG) != 100 ) {
    printf("\n  IRQ error! Count was %d, should be 100 ",*(base+IRQFLAG));
    bad |= 4;
  }

  if ( bad ) {
    printf("\n");
    exit(bad);
  }
  else
    printf ("OK\n");


/* Check each of the ACIA's shifters and handshake lines */

  *(base+0x8000)=0;     /* stop the 6502 */
  *(base+0x3ffc)=(UBYTE)(CHECKACIAS & 0xff); /* Set RESET vector to other code. */
  *(base+0x3ffd)=(UBYTE)( (CHECKACIAS>>8) & 0xff);

  for (i=0;i<=6;i++) { /* Check each of the 7 serial ports */
    printf("%cChannel %d ... ",13,i);
    *(base+0x8000)=0;   /* stop the 6502 */
    *(base+CHANNEL)=(UBYTE)i;   /* Put the channel # to be tested in RAM */
    *(base+ACK)=0xff;   /* Reset the acknowledge byte */
    *(base+FINISHFLAG)=0xff; /* Reset the FINISHFLAG byte */
    *(base+0xc000)=0;   /* start the 6502 */
    while( *(base+FINISHFLAG) );  /* Wait for 6502 to start */
    *(base+ACK)=0;      /* Signal 6502 that we acknowledge that it started */
    while( *(base+FINISHFLAG) == 0); /* Wait for the 6502 to finish */
    if (*(base+NUMERRORS) != 0)printf("Error(s) =");
    for (j=1;j<=*(base+NUMERRORS);j++) /* List the errors */
      printf(" %2x",*(base+ERRORS+j-1));
    if (*(base+NUMERRORS) != 0)printf("\n");
  } /* for */
  printf("\n");


} /* if */

else printf("\nA2232 Serial Card not found!\n");

} /* main */

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

UBYTE getbyte(line)  /* Return the decimal value of the 2 digit */
			      /* HEX number pointed to by 'line').        */
char **line;

{
  UBYTE value=0;
  SHORT mul;

  for (mul=16; mul > 0; mul -= 15)  {
    if ( (**line >= '0') && (**line <= '9') )
      value = value + ((**line - '0') * mul);
    else if ( (**line >= 'A') && (**line <= 'Z') )
      value = value + ((**line - 'A' + 10) * mul);
    else if ( (**line >= 'a') && (**line <= 'z') )
      value = value + ((**line - 'a' + 10) * mul);
    *line += 1;
  } /* for */
  return(value);
}

/* ------------------------------------------------------------------------ */

UWORD getword(line)
char **line;

{
  UWORD value;
  value = getbyte(line) * 256;
  value += getbyte(line);
  return (value);
}

/* ------------------------------------------------------------------------ */

int memtestl(start,stop,passes) /* Test memory using longwords */

   ULONG *start, *stop;
   int passes;
{
   ULONG *tp,q;
   ULONG values[8];
   int v;

   values[0] = 0x00000000;
   values[1] = 0xaaaaaaaa;
   values[2] = 0x55555555;
   values[3] = 0x0000ffff;
   values[4] = 0xffff0000;
   values[5] = 0x0f0f0f0f;
   values[6] = 0xf0f0f0f0;
   values[7] = 0xffffffff;


for (;passes>0;passes--) /* repeat the test for the # of passes specified */

   for (v=0;v<=7;v++) { /* Check memory using each of the test values */

      for (tp=start; tp <= stop; tp++) { /* Fill the memory */
	 *tp = values[v];
      }

      for (tp=start; tp <= stop; tp++) { /* Now check if memory is still OK */
	 q = *tp;
	 if (q != values[v]) {
	    printf("\nwrote %4x @ %6x, read %4x\n",values[v],tp,q);
	    return (1);
	 }
      }

   }

return (0);

}

/* ----------------------------------------------------------------------- */


