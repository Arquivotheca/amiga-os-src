/*************************************************************************

  This program downloads code from the specified file into the RAM on
  the A2232 multiport serial card.  The file should contain MOS hex
  format.

	  LOADER <filename> <S>

		filename : file (in MOSHEX) to download
		S	 : start the 6502 after loading the file

    70

*************************************************************************/

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
int memtestl ();

/***************************************************************************/

void main(argc,argv)

	int argc;
	char *argv[];

{

struct ConfigDev *dev = NULL;
UWORD Manuf = 0X202;
int Product;
UBYTE  *adr, *base, bytecount;
char filestr[25], *filename, line[200], *lineptr;
FILE *fopen(), *fp;

ExpansionBase = OpenLibrary("expansion.library",33);
if (ExpansionBase == NULL) exit(100);

  printf("Enter product number (decimal) :");
  scanf("%d",&Product);
  if (dev = FindConfigDev(dev,Manuf,(UBYTE)Product)) { /* Look for the board */
    base = (UBYTE *)(*dev).cd_BoardAddr;

    if (argc > 1)  /* Was the filename included on the command line? */
      filename = argv[1];
    else {   /* Nope, so ask for it */
      printf("\nFile name ?");
      scanf("%s",filestr);
      filename = filestr;
    } /* else */

    if ((fp = fopen(filename,"r")) == NULL) { /* Try to open the file */
      printf("Can't open file: %s\n",filename);
      exit(1);
    } /* if */
    else printf("\nDownloading code to board ... ");

    *(base + 0x8000) = 0; /* Pull the 6502's RESET line low */

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

    if (argc > 2) { /* Should the 6502 be allowed to execute? */
      *(base+0xc000) = 0; /* Start the 6502 */
      printf("6502 started.\n");
    }

  } /* if */

  else printf("A2232 Serial Card not found!\n");

   } /* main */


/*************************************************************************/

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

/*************************************************************************/

UWORD getword(line)
char **line;

{
  UWORD value;
  value = getbyte(line) * 256;
  value += getbyte(line);
  return (value);
}

/*************************************************************************/

