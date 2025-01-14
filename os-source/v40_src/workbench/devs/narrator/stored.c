/*
 *
 *  Reads a phonetically encoded file specified as an arg to the
 *  program call and computes the f1-f2-f3 matrix.
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <functions.h>
#include <ctype.h>
#include <exec/exec.h>
#undef NULL
#include <stdio.h>
#include "narrator.h"
#include "private.h"

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

#define INTUITION_REV 1L
#define GRAPHICS_REV 1L


struct MsgPort *talkport = NULL;
struct narrator_rb *iorb = NULL;

char line[400];
char filename[30];
UWORD space[31][27][21];


extern struct IORequest *CreateExtIO();

#define VOICED	0x80	/* flag-byte bits */
#define FRIC	0x40
#define ASPIR	0x20
#define NASAL	0x10
#define	WORDBND	2
#define	PHONBND	1

main(argc,argv)
char **argv;
{
extern APTR Talk();

float f;
WORD f1,f2,f3,a1,a2,a3,f0;
UBYTE ff;			/* flags byte in coef		      */
LONG i,j,k;			/* loop counters		      */
LONG count,total;
UBYTE *coefbfr;			/* returned addr of the coef buffer   */
UBYTE *bytptr;
ULONG coeflen;			/* length of the coef buffer returned */
FILE *fd;
char *flags,*p,c;

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
iorb->flags |= (NDF_NEWIORB | NDF_KEEPSYNTH | NDF_KEEPCOEF);


if((fd = fopen(argv[1],"r")) == NULL)
{   printf("Can't open %s\n",argv[1]);
    goto clean2;
}
total = 0;
for (i=0; i<31; ++i)
  for (j=0; j<27; ++j)
    for (k=0; k<21; ++k) space[i][j][k] = 0;

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
     coefbfr = iorb->coefbfr;
     coeflen = iorb->coefsiz;

    if((coefbfr < 0L) || (coeflen < 0L))
    {   printf("Narrator error = %ld\n",coefbfr);
        goto clean3;
    }

printf("%s\n",line);

/* Print out the coef buffer */
flags = "        ";
bytptr = coefbfr;
j = 0;

while(*bytptr != 0xff) {   
    ff = *(bytptr+6);				/* Get flags byte */

    if (!(ff & (NASAL|ASPIR|FRIC)) && *(bytptr+3)) {		/* Not nasal	  */
        f1 = (WORD)*bytptr;			/* Get state space indices */
        f2 = (WORD)*(bytptr+1);
        f3 = (WORD)*(bytptr+2);

        space[f1][f2][f3] += 1;			/* Incr state space entry */
	total++;
    }

    bytptr += 8;

}


    if (coefbfr) FreeMem(coefbfr,coeflen);  	    /* give back the coef bfr */
    coefbfr = NULL;
    if (iorb->synthbfr) FreeMem(iorb->synthbfr, iorb->synthsiz);

    p = line;	/* reset line pointer */
}
else *p++ = c;

}   /* end while() */

printf("total increments=%ld\n\r",total);

count = 0;
for (i=0; i<31; ++i)
  for (j=0; j<27; ++j)
    for (k=0; k<21; ++k) {
	if (space[i][j][k] != 0) {
		count += 1;
		printf("state %2ld,%2ld,%2ld = %3ld\n\r",i,j,k,space[i][j][k]);
	}
    }

printf("total non-zero states=%ld\n\r",count);



printf("End of file\n");

clean3: fclose(fd);
clean2: CloseDevice(iorb);
	DeleteExtIO(iorb,(LONG)sizeof(struct narrator_rb));
clean1: DeletePort(talkport);

exit(TRUE);
} /* end main()		=== END OF PROGRAM ===  */
