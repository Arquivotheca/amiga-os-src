#include <libraries/dos.h>
#include <libraries/dosextens.h>

void main();

void main(argc,argv)
int argc;
char *argv[];
{
   struct FileHandle *file = 0;
   char *address;

   printf("Argc = %ld\n",argc);
   { int x;
      for(x=1;x<argc;x++)
         printf("argv[%ld]= %s\n",x,argv[x]);
   }

   printf("Enter Address in hex: ");
   scanf("%x\n",&address);
   printf("Address=%x\n",address);


   printf("Opening sys:sidecar/pc.boot\n");
   if((file=Open("sys:sidecar/pc.boot",MODE_OLDFILE))==0) {
      printf("Error opening file!\n");
      return;
   }


   if(Read(file,address,20000)<12228) {
      printf("Error Reading\n");
   }


   if(file) Close(file);
}
