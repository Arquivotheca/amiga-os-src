#include <libraries/dos.h>
#include <libraries/dosextens.h>

struct AB_BLOCK {
      char name[8];
      short Heads;
      short SecTrk;
      short Cyl;
      };

void main();

void main(argc,argv)
int argc;
char *argv[];
{
   struct FileHandle *file = 0;
   int   Heads;
   int   SecTrk;
   int   Cyl;
   char  buf[512];
   char  abuf[512];
   int temp;
   struct AB_BLOCK *bp;
   int x;

/*
   printf("Argc = %ld\n",argc);
   { int x;
      for(x=1;x<argc;x++)
         printf("argv[%ld]= %s\n",x,argv[x]);
   }
*/

   if(argc!=2) {
      printf("Usage: MakeAB filename\n");
      return;
   }

moreh:
   printf("Enter # of heads 1-16: ");
   scanf("%d\n",&Heads);
   if(Heads>16 || Heads<1) goto moreh;
/*
   printf("heads=%d\n",Heads);
*/

moresecs:
   printf("Enter # of Sectors/Track 1-64: ");
   scanf("%d\n",&SecTrk);
   if(SecTrk>64 || SecTrk<1) goto moresecs;
/*
   printf("sectors/track=%d\n",SecTrk);
*/

morecyls:
   printf("Enter # of Cylinders 1-1024: ");
   scanf("%d\n",&Cyl);
   if(Cyl>1024 || Cyl<1) goto morecyls;
/*
   printf("heads=%d\n",Cyl);
*/

   printf("\nParameters selected:\n");
   printf("Heads             = %d\n",Heads);
   printf("Sectors per Track = %d\n",SecTrk);
   printf("Cylinders         = %d\n",Cyl);
   printf("Fake drive will be %d bytes\n",(Heads*SecTrk*Cyl*512));
   printf("Total file size will be %d bytes\n",(Heads*SecTrk*Cyl*512)+512);
   printf("\nEnter Y to accept N to quit: ");
   scanf("%s\n",abuf);
   if(abuf[0]!='Y'&&abuf[0]!='y') goto Cleanup;

   printf("Creating %s\n",argv[1]);
   if((file=(struct FileHandle *)Open(argv[1],MODE_NEWFILE))==0) {
      printf("Error opening file!\n");
      return;
   }

   for(x=0;x<512;x++)
      buf[x]=0xf6;

   bp=(struct AB_BLOCK *)buf;
   bp->name[0]='A';
   bp->name[1]='B';
   bp->name[2]='O';
   bp->name[3]='O';
   bp->name[4]='T';
   bp->name[5]=0;
   bp->name[6]=0;
   bp->name[7]=0;
   bp->Heads=Heads;
   bp->SecTrk=SecTrk;
   bp->Cyl=Cyl;

   if(Write(file,buf,512)!=512) {
      printf("Error writing, output file deleted!\n");
      if(file) Close(file);
      DeleteFile(argv[1]);
      return;
   }

   for(x=0;x<512;x++)
      buf[x]=0xf6;

   temp=Heads*SecTrk*Cyl;

   for(x=0;x<temp;x++)
   {
      if(Write(file,buf,512)!=512) {
         printf("Error writing, output file deleted!\n");
         if(file) Close(file);
         DeleteFile(argv[1]);
         return;
      }
   }


Cleanup:
   if(file) Close(file);
}
