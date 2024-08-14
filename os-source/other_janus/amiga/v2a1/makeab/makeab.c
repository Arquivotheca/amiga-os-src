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
   int temp;
   struct AB_BLOCK *bp;
   int x;

   printf("Argc = %ld\n",argc);
   { int x;
      for(x=1;x<argc;x++)
         printf("argv[%ld]= %s\n",x,argv[x]);
   }
   if(argc!=2) {
      printf("Usage: MakeAB filename\n");
      return;
   }

   printf("Enter # of heads: ");
   scanf("%d\n",&Heads);
   printf("heads=%d\n",Heads);

   printf("Enter # of Sectors/Track: ");
   scanf("%d\n",&SecTrk);
   printf("sectors/track=%d\n",SecTrk);

   printf("Enter # of Cylinders: ");
   scanf("%d\n",&Cyl);
   printf("heads=%d\n",Cyl);

   printf("Fake drive will be %d bytes\n",(Heads*SecTrk*Cyl*512));
   printf("Enter 1 to accept\n");
   scanf("%d\n",&temp);
   if(temp!=1) goto Cleanup;

   printf("Creating %s\n",argv[1]);
   if((file=Open(argv[1],MODE_NEWFILE))==0) {
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
      printf("Error writing\n");
      goto Cleanup;
   }

   for(x=0;x<512;x++)
      buf[x]=0xf6;

   temp=Heads*SecTrk*Cyl;

   for(x=0;x<temp;x++)
   {
      if(Write(file,buf,512)!=512) {
         printf("Error writing\n");
         goto Cleanup;
      }
   }


Cleanup:
   if(file) Close(file);
}
