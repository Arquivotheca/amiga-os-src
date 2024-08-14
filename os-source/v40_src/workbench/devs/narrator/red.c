/*****************************************************************************
*
*  Reads a phonetically encoded file specified as an arg to the
*  program call.  eg: red text
*
*  Written (commenced) 4/88 by Mark Barton
*
*  There are no guarantees in life.
*
*  This version calls a special version of Narrator and uses the coef
*  buffer thus produced as input to the new algorithm.
*
*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <functions.h>
#include <ctype.h>
#include <exec/exec.h>
#undef NULL
#include <stdio.h>
#include "narrator.h"

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define NEWIORB		0x01
#define LPFCUTOFF	0x02
#define KEEPCOEF	0x04		/* iorb flags */
#define KEEPSYNTH	0x08
#define DEBUGMODE	0x10

#define INTUITION_REV 1L
#define GRAPHICS_REV 1L


struct MsgPort *talkport = NULL;
struct narrator_rb *iorb = NULL;

char line[400];
char filename[30];

extern struct IORequest *CreateExtIO();

main(argc,argv)
char **argv;
{
extern APTR Talk();

LONG i,j,k;			/* loop counters		      */
UBYTE *coefbfr;			/* returned addr of the coef buffer   */
UBYTE *bytptr;
ULONG coeflen;			/* length of the coef buffer returned */
FILE *fd;
char *p,c;

LONG	f1adj, f2adj, f3adj;
LONG	a1adj, a2adj, a3adj;
LONG	f0enth;

BYTE chans[4];
APTR coef;
LONG error;

chans[0] = 1;
chans[1] = 2;
chans[2] = 4;
chans[3] = 8;



/* Open Intuition library */
IntuitionBase = (struct IntuitionBase *)
		OpenLibrary("intuition.library",INTUITION_REV);
if(IntuitionBase == NULL) exit(FALSE);


talkport = CreatePort(NULL,0);
if(talkport == NULL)
{   printf("CreatePort failed\n");
    exit(FALSE);
}


iorb = (struct narrator_rb *)CreateExtIO(talkport,
	(LONG)sizeof(struct narrator_rb));
if(iorb == NULL)
{   printf("Iorb could not be allocated\n");
    goto clean1;
}



iorb->ch_masks = chans;
iorb->nm_masks = 4;
iorb->message.io_Command = CMD_WRITE;
iorb->flags = NDF_NEWIORB;

error = OpenDevice("narrator.device",0L,iorb,0L);
if(error != 0L)
{   printf("Error opening narrator.  Return code = %ld\n",error);
    return(0L);
}

iorb->sex = MALE;
iorb->pitch = DEFPITCH;
iorb->volume = 64;
iorb->mode = NATURALF0;

/*
printf("enter f1, f2, and f3 adjustments ");
scanf("%ld%ld%ld",&f1adj, &f2adj, &f3adj);
iorb->F1adj = f1adj;
iorb->F2adj = f2adj;
iorb->F3adj = f3adj;

printf("enter a1, a2, and a3 adjustments ");
scanf("%ld%ld%ld",&a1adj, &a2adj, &a3adj);
iorb->A1adj = a1adj;
iorb->A2adj = a2adj;
iorb->A3adj = a3adj;

printf("enter f0 enthusiasm ");
scanf("%ld", &f0enth);
iorb->F0enthusiasm = f0enth;

*/
if((fd = fopen(argv[1],"r")) == NULL)
{   printf("Can't open %s\n",argv[1]);
    goto clean2;
}


p = line;	/* point to start of line */

/* Read the input file */
while((c = getc(fd)) != EOF)
{
if(isspace(c))  *p++ = ' ';
else if((c == '.') || (c == '?'))
{   *p++ = c;
    *p = '\0';

     iorb->message.io_Data = line;
     iorb->message.io_Length = strlen(line);
     error = DoIO(iorb);
     if(error != 0L)
     {   printf("Speech error %ld\n",error);
          CloseDevice(iorb);
    	  exit(error);
     }
     printf("Talked\n");

/*
    if (iorb->flags & KEEPCOEF)
      {
	coefbfr = iorb->coefbfr;
	coeflen = iorb->coefsiz;
	printf("coef buffer ptr=%lx    len=%ld\n",coefbfr, coeflen);


	if((coefbfr > 0L) && (coeflen > 0L))
	  {
	    bytptr = coefbfr;			
	    for(j=0; j<(coeflen >> 3); j++)
	      {   printf("\n%ld    ",j+1L);
	          for(k=0; k<8; k++)  printf("%4d  ",*bytptr++);
	      }
	    printf("\n");

	    FreeMem(coefbfr,coeflen); 		
	  }
	else printf("KEEPCOEF flag set but coef bfr not allocated\n");
      }

    if (iorb->flags & KEEPSYNTH)
      {
	printf("returning synth buffer:  addr=%lx    len=%ld\n",
		iorb->synthbfr, iorb->synthsiz);
	FreeMem(iorb->synthbfr, iorb->synthsiz);
      }

    p = line;	
}
*/
*p++ = c;
}
}   /* end while() */



printf("End of file\n");

clean3: fclose(fd);
clean2: CloseDevice(iorb);
	DeleteExtIO(iorb,(LONG)sizeof(struct narrator_rb));
clean1: DeletePort(talkport);

exit(TRUE);
} /* end main()		=== END OF PROGRAM ===  */
