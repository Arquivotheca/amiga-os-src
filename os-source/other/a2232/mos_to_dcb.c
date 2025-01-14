/*

    Convert an mos hex file into "dc.b" instructions palletable
    to an assembler.  Modifed from a Kludge by Greg Berlin

    Sunday 07-May-89 21:57:06 -Bryce Nesbitt

*/
#include <stdio.h>



 /* Return the decimal value of the 2 digit */
/* HEX number poited to by 'line').        */
unsigned char getbyte(line)
char **line;
{
  unsigned char value=0;
  short mul;

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


unsigned short getword(line)
char **line;

{
  unsigned short value;

  value = getbyte(line) * 256;
  value += getbyte(line);
  return (value);
}




void main(argc,argv)
int argc;
char *argv[];
{
#define 	BUFFER_SIZE 200
char		*filename, line[BUFFER_SIZE], *lineptr;
unsigned char	bytecount,savedcount;
unsigned short	adr;
FILE		*fopen(), *fp;

  if (argc > 1)  /* Was the filename included on the command line? */
    filename = argv[1];
  else
    {	/* Nope, so ask for it */
    printf("Convert mos hex file into dc.b assembler statements\n");
    printf("%s <file name>\n",argv[0]);
    exit(5);
    } /* else */

  if ((fp = fopen(filename,"r")) == NULL)
    { /* Try to open the file */
    printf("Can't open mos hex file %s\n",filename);
    exit(1);
    } /* if */
  else
    {
    printf(";This is the 6502 code to run the multiport board\n");
    printf(";it was converted from the MOS records by mos_to_dcb\n");
    printf("; <hi offset>,<low offset>,<num bytes>,<data...>\n");
    printf("    SECTION main,CODE\n");
    printf("    XDEF server_object\n");
    printf("server_object:\n");

    while (fgets(line, BUFFER_SIZE-1, fp) != NULL)
      { /* read each record */
      if (argc > 3) printf("%s",line);
	lineptr = line + 1;
	if ( (bytecount = getbyte(&lineptr) ) != 0)
	  {
	  adr = getword(&lineptr); /* Address for this record */
	  savedcount=bytecount;
	  printf("\n dc.b $%x,$%x,$%x",(adr >> 8),(adr & 0xff),savedcount-1);
	  for (;bytecount > 0; bytecount--)
	    {
	    printf(",$%x", getbyte(&lineptr) );
	    adr++;
	    }
	  } /* if */
      else
	{
	printf("\n dc.b $0,$0\n\n");
	printf("\n ds.w 0\n\n");
	printf ("\n;%d records loaded.\n",getword(&lineptr));
	printf("  END\n");
	exit(0);
	} /* else */
      } /* while */
    } /* else */

} /* main */
