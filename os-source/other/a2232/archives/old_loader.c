#include <exec/types.h>
#include <exec/exec.h>
#include <exec/io.h>
#include <libraries/configvars.h>
#include <libraries/configregs.h>
#include <stdio.h>		/* #include <lattice/stdio.h> */
#include <libraries/dos.h>

struct Library *ExpansionBase;
extern struct Library *OpenLibrary();
extern struct ConfigDev *FindConfigDev();
extern int Enable_Abort;


UBYTE getbyte(line)  /* Return the decimal value of the 2 digit */
			      /* HEX number poited to by 'line').        */
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


UWORD getword(line)
char **line;

{
  UWORD value;
  value = getbyte(line) * 256;
  value += getbyte(line);
  return (value);
}


void main(argc,argv)
	int argc;
	char *argv[];

{

struct ConfigDev *dev = NULL;
UWORD Manuf = 0X202;
UBYTE Product = 69, *adr, *base, bytecount;
char filestr[32], *filename, line[200], *lineptr;
FILE *fopen(), *fp;

Enable_Abort = 1;

ExpansionBase = OpenLibrary("expansion.library",33L);
if (ExpansionBase == NULL) exit(100);

  printf("Expansion library at %lx\n",ExpansionBase);

 if (dev = FindConfigDev(dev,Manuf,Product)) { /* Look for the board */
  base = (UBYTE *)(*dev).cd_BoardAddr;

  printf("Manufacturer %d,Product %d,Address $%lx\n",Manuf,Product,base);

  if (argc > 1)  /* Was the filename included on the command line? */
    filename = argv[1];
  else {   /* Nope, so ask for it */
    printf("\nFile name ?");
    scanf("%s",filestr);    /* bug-no length check */
    filename = filestr;
  } /* else */

  if ((fp = fopen(filename,"r")) == NULL) { /* Try to open the file */
    printf("Can't open file: %s\n",filename);
    exit(1);
  } /* if */
  else
    while (fgets(line, 100, fp) != NULL) { /* read each record */
      printf("%s",line);
      lineptr = line + 1;
      if ( (bytecount = getbyte(&lineptr) ) != 0) {
	adr = base + getword(&lineptr); /* Address for this record */
	for (;bytecount > 0; bytecount--) *adr++ = getbyte(&lineptr);
/*	for (;bytecount > 0; bytecount--) {
	  printf(" %x",getbyte(&lineptr));
	  adr++;
	} */ /* for */
      } /* if */
      else {
	printf ("\n%d records loaded.\n",getword(&lineptr));
	exit (0);
      } /* else */
  } /* while */

 } /* if */
 else printf("Serial Card not found!\n");

} /* main */


