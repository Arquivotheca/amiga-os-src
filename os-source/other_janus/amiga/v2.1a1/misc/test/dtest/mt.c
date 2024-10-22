#include <janus/janus.h>
#include <exec/memory.h>
#include <stdio.h>

struct JanusBase *JanusBase =0;

void main();
void main()
{
   unsigned char *p;
   unsigned char *buf; 
   char buff[50];
   long x;
   FILE *f=NULL;
   long estart;

   if((JanusBase=(struct JanusBase *)OpenLibrary("janus.library",0))==0)
   {
      printf("Unable to open Janus.Library!\n");
      return;
   }
   printf("JanusBase @ 0x%lx\n",JanusBase);

   buf=(unsigned char *)AllocMem(0x10000,MEMF_CLEAR);
   if(!buf)
   {
      printf("No memory for buffer\n");
	  goto cleanup;
   }
   printf("Snapshot buffer @ 0x%lx\n",buf);

   p=(unsigned char *)GetJanusStart();
   printf("Janus Start @ 0x%lx\n",p);

   f=fopen("sys:pc/system/pc.boot","r");
   if(!f)
   {
      printf("Could not open pc.boot\n");
	  goto cleanup;
   }

   x=fread(&buf[0],0x2000,1,f);
   if(x!=1)
   {
      printf("read error\n");
	  goto cleanup;
   }

/*
   printf("Type return to snapshot\n");
   gets(buff);

   for(x=0;x<0x10000;x++)
      buf[x]=p[x];
*/

   printf("Type return for list of differences\n");
   gets(buff);

   estart=0x4000;
   p+=estart;
   for(x=0;x<0x2000;x++)
   {
      if(p[x]!=buf[x])
	  {
	     printf("Offset 0x%lx changed from 0x%2x to 0x%2x\n",x,
		        (unsigned char)buf[x],(unsigned char)p[x]);
	  }
   }


cleanup:
   if(f) fclose(f);
   if(buf) FreeMem(buf,0x10000);
   if(JanusBase) CloseLibrary(JanusBase);
   return;
}
