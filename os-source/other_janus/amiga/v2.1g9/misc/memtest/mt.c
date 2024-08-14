#include <janus/janus.h>
#include <stdio.h>

struct JanusBase *JanusBase =0;

void main();
void main()
{
   char buff[255];
   char *mp[4];
   int index;

   if((JanusBase=(struct JanusBase *)OpenLibrary("janus.library",0))==0)
   {
      printf("Unable to open Janus.Library!\n");
      return;
   }

   for(index=0;index<4;index++)
   {
      mp[index]=(char *)AllocJanusMem(500L,MEMF_PARAMETER | MEM_BYTEACCESS);
      if(mp[index]==0)
      {
         printf("Alloc of block %d failed\n",index);
      } else {
         printf("Memory block %d at 0x%x\n",index,mp[index]);
      }
   }

   if(mp[1]!=0)
   {
      printf("Return to free block 2\n");
      gets(buff);
      FreeJanusMem(mp[1],500L);
      mp[1]=0;
   }
   if(mp[2]!=0)
   {
      printf("Return to free block 3\n");
      gets(buff);
      FreeJanusMem(mp[2],500L);
      mp[2]=0;
   }
   if(mp[0]!=0)
   {
      printf("Return to free block 1\n");
      gets(buff);
      FreeJanusMem(mp[0],500L);
      mp[0]=0;
   }
   if(mp[3]!=0)
   {
      printf("Return to free block 4\n");
      gets(buff);
      FreeJanusMem(mp[3],500L);
      mp[3]=0;
   }

cleanup:
   for(index=0;index<4;index++)
   {
      if(mp[index]!=0)
         FreeJanusMem(mp[index],500L);
   }
   if(JanusBase) CloseLibrary(JanusBase);
   return;
}
