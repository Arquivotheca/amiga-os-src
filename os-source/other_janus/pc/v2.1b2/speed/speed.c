
#include	<stdio.h>
#include	<conio.h>
#include	<dos.h>

#define TRUE 1
#define FALSE 0

#define slow 0x01
#define turbo 0x41
#define double 0x81
#define port63 0x0063
#define equip_flag 0x0410

unsigned int *pointer;

/***********************************************************************
 * ncp() returns numeric-co-processor bit of the equipment flag        *
 ***********************************************************************/
ncp()
{
pointer = (unsigned int *) equip_flag;
return(*pointer & 0x02);
}

/***********************************************************************
 * setdouble() sets 2010a to 9.54 Mhz                                  *
 ***********************************************************************/
setdouble()
{
outp(port63,(ncp()|double));
}
		
/***********************************************************************
 * setturbo() sets 2010a to 7.16 Mhz                                   *
 ***********************************************************************/
setturbo()
{
outp(port63,(ncp()|turbo));
}

/***********************************************************************
 * setslow() sets 2010a to 4.77 Mhz                                    *
 ***********************************************************************/
setslow()
{
outp(port63,(ncp()|slow));
}

/***********************************************************************/
usage(error_string)
char	error_string[];
{			  
printf("Commodore A2088T Speed Utility V1.0\n");
if (strlen(error_string) >= 3)
	{
	printf("error: %s",error_string);
	}
printf("usage: SPEED -[std]");
exit(1);
}

/***********************************************************************/

main(argc, argv)
int argc;
unsigned char *argv[];

{
char	string[100];
unsigned char	c;

if (argc<2)
	{
	usage("");
	}

if (sscanf(argv[1],"%s",string)>1)
	{
	usage("invalid parameters\n");
	}

if (string[0] != '-')
	{
	usage("invalid switch character\n");
	}

if (strlen(string)!=2)
	{
	usage("invalid number of parameters\n");
	}

c=string[1];

switch(c)
	{
	case's':
	case'S':
		setslow();
		break;
	case't':
	case'T':
		setturbo();
		break;
	case'd':
	case'D':
		setdouble();
		break;
	default:
		usage("invalid command switch\n");
	}
exit(0);
}
			
			


