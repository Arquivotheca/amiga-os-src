/*
	KickFile V1.1
	by Copyright 1989 by Dave Haynie

	MAIN PROGRAM FILE
*/

#define KICKFILE

#include "setcpu.h"

/* This is the main routine */

void main(argc,argv)
int argc;
char *argv[];
{
   struct systag *tag;
   BPTR outfile;
   char *rom;
   static ULONG space[2] = { 0,0 };
   
  
   printf("\23333mKickFile V%1.2f Copyright 1989 by Dave Haynie\2330m\n",
   	  ((float)PROGRAM_VERSION/100.0));

   if (argc != 3) {
      printf("Usage: KickFile DEV: pathname\n");
      exit(0);
   }

   if (!(tag = AllocKSImage(argv[1]))) {
      printf("Error: Can't read KickStart Disk in %s:\n",argv[1]);
      exit(10);
   }
   if (outfile = Open(argv[2],MODE_NEWFILE)) {
      printf("Writing \"%s\"\n",argv[2]);
      space[1] = tag->romused;
      Write(outfile,space,8L);
      rom = ((char *)tag->romimage) + (tag->romsize - tag->romused);
      Write(outfile,rom,tag->romused);
      Close(outfile);
      printf("Done!\n");
   } else
      printf("Error: Can't open file \"%s\"\n",argv[2]);

   FreeMem(tag->romimage,tag->romsize);
   FreeMem(tag,(ULONG)sizeof(struct systag));
}
