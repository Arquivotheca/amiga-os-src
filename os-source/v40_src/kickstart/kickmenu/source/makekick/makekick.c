#include <exec/types.h>

#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <proto/all.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void kprintf(char *,...);
/* void sprintf(char *,char *,...); */
#define  D(x) x
#define  D1(x) ;
#define kprintf printf

char *DriveNames[]={
      "DF0:",
      "DF1:",
      "DF2:",
      "DF3:"
      };

#define DISK_BUF_SIZE   (0x40000)

#define HEADER_SIZE        (0x400)
#define K1_3_SIZE          (0x40000)
#define K1_4_SIZE          (0x80000)
/* #define BONUS13_MAX_SIZE   (0x10000) */
#define BONUS13_MAX_SIZE   (0x1ABFF)
#define BONUS14_MAX_SIZE   (0xFE0)

#define HEADER_OFFSET      (0x0)
#define K1_3_OFFSET        (0x400)
#define K1_4_OFFSET        (0x40400)
#define BONUS13_OFFSET     (0xC1400)
#define BONUS14_OFFSET     (0xC0400)

BOOL WriteFloppy(ULONG unit,ULONG offset,ULONG count,APTR address);
BOOL DiskInDrive(ULONG unit);
LONG DeviceNumber(char *s);
ULONG SumROM(ULONG *,ULONG);

main(int argc,char *argv[])
{
   char buf[1024];
   char *membuf;
   ULONG unit;
   FILE *infile;
   ULONG actual;
   LONG  Bonus13_size;
   LONG  Bonus14_size;
   LONG x;

   if(argc!=6)
   {
      printf("Usage: MakeKick dfx: 1.3 1.4 bonus13 bonus14\n");
      exit(0);
   }

   if((unit=DeviceNumber(argv[1]))==4)
   {
      printf("Invalid device %s\n",argv[1]);
      exit(0);
   }

   D1( printf("unit=%ld\n",unit); )

   membuf=AllocMem(DISK_BUF_SIZE,MEMF_CHIP|MEMF_CLEAR);
   if( ! membuf)
   {
      printf("Could not allocate memory for disk buffer\n");
      exit(0);
   }

   do {
      printf("Insert formatted Disk in drive %s and press RETURN\n",DriveNames[unit]);
      gets(buf);
   } while (!DiskInDrive(unit));


   /*##############################*/
   /*### read 1.3    file/write ###*/
   /*##############################*/
   if((infile=fopen(argv[2],"r"))==NULL)
   {
      printf("Error opening 1.3 Kickstart file %s\n",argv[2]);
      goto out;
   }
   if(fseek(infile,8L,0L))
   {
      printf("seek error on 1.3 Kickstart file %s\n",argv[2]);
      goto out;
   }
   printf("Reading 1.3 kickfile...");
   actual=fread(membuf,1,K1_3_SIZE,infile);
   if(actual!=K1_3_SIZE)
   {
      printf("\nError: wrong file size for 1.3 Kickstart file %s\n",argv[2]);
      goto out;
   }
   printf("done.\n");
   fclose(infile); infile=NULL;
   if((*((ULONG *)(&membuf[0]))) != 0x11114ef9)
   {
      printf("%s is not a properly formatted Kickstart file\n", argv[2]);
      goto out;
   }
   if(SumROM((ULONG *)membuf,256*1024))
   {
      printf("ERROR: Kickstart file corrupt.\n");
      goto out;
   }
   printf("Writing 1.3 kickfile...");
   if( ! WriteFloppy(unit,K1_3_OFFSET,K1_3_SIZE,(APTR)membuf))
   {
      printf("\nError writing 1.3 Kickstart file to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");


   /*##############################*/
   /*### read 1.4    file/write ###*/
   /*##############################*/
   if((infile=fopen(argv[3],"r"))==NULL)
   {
      printf("Error opening 1.4 Kickstart file %s\n",argv[3]);
      goto out;
   }
   if(fseek(infile,8L,0L))
   {
      printf("seek error on 1.3 Kickstart file %s\n",argv[3]);
      goto out;
   }
   printf("Reading 1.4 kickfile...");
   actual=fread(membuf,1,DISK_BUF_SIZE,infile);
   if(actual!=DISK_BUF_SIZE)
   {
      printf("Error: wrong file size for 1.4 Kickstart file %s\n",argv[3]);
      goto out;
   }
   if((*((ULONG *)(&membuf[0]))) != 0x11144ef9)
   {
      printf("\n%s is not a properly formatted Kickstart file\n", argv[3]);
      goto out;
   }
   printf("done.\n");
   printf("Writing 1.4 kickfile...");
   if( ! WriteFloppy(unit,K1_4_OFFSET,DISK_BUF_SIZE,(APTR)membuf))
   {
      printf("\nError writing 1.4 Kickstart file to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");
   printf("Reading 1.4 kickfile...");
   actual=fread(membuf,1,DISK_BUF_SIZE,infile);
   if(actual!=DISK_BUF_SIZE)
   {
      printf("Error: wrong file size for 1.4 Kickstart file %s\n",argv[3]);
      goto out;
   }
   printf("done.\n");
   printf("Writing 1.4 kickfile...");
   if( ! WriteFloppy(unit,0x80400,DISK_BUF_SIZE,(APTR)membuf))
   {
      printf("Error writing 1.4 Kickstart file to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");
   fclose(infile);


   /*###############################*/
   /*### read bonus13 file/write ###*/
   /*###############################*/
   printf("Clearing buffer..."); for(x=0;x<DISK_BUF_SIZE;x++) 
	membuf[x]=0; printf("done.\n");

   if((infile=fopen(argv[4],"r"))==NULL)
   {
      printf("Error opening bonus13 file %s\n",argv[4]);
      goto out;
   }
   printf("Reading bonus13 file...");
   actual=fread(membuf,1,BONUS13_MAX_SIZE,infile);
   if(actual==BONUS13_MAX_SIZE)
   {
      printf("\nError: bonus13 file %s is too long\n",argv[4]);
      goto out;
   }
   if((actual % 512)!=0)
      Bonus13_size=512*((actual+512)/512);
   else
      Bonus13_size=actual;
   printf("done. Size=0x%lx %ld\n",Bonus13_size,Bonus13_size);
   fclose(infile); infile=NULL;
   printf("Writing bonus13 file...");
   if( ! WriteFloppy(unit,BONUS13_OFFSET,Bonus13_size,(APTR)membuf))
   {
      printf("Error writing bonus13 file to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");
   /*###############################*/
   /*### read bonus14 file/write ###*/
   /*###############################*/
   printf("Clearing buffer..."); for(x=0;x<DISK_BUF_SIZE;x++) 
	membuf[x]=0; printf("done.\n");

   if((infile=fopen(argv[5],"r"))==NULL)
   {
      printf("Error opening bonus14 file %s\n",argv[5]);
      goto out;
   }
   printf("Reading bonus14 file...");
   actual=fread(membuf,1,BONUS14_MAX_SIZE,infile);
   if(actual==BONUS14_MAX_SIZE)
   {
      printf("\nError: bonus14 file %s is too long\n",argv[5]);
      goto out;
   }
   if((actual % 512)!=0)
      Bonus14_size=512*((actual+512)/512);
   else
      Bonus14_size=actual;
   printf("done. Size=0x%lx %ld\n",Bonus14_size,Bonus14_size);
   fclose(infile); infile=NULL;
   printf("Writing bonus14 file...");
   if( ! WriteFloppy(unit,BONUS14_OFFSET,Bonus14_size,(APTR)membuf))
   {
      printf("Error writing bonus14 file to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");

   /*#############################*/
   /*### make and write header ###*/
   /*#############################*/
  
	printf("Clearing buffer..."); 
	for(x=0;x<DISK_BUF_SIZE;x++) membuf[x]=0; 
	printf("done.\n"); 

   membuf[0]='K';
   membuf[1]='I';
   membuf[2]='C';
   membuf[3]='K';

   membuf[4]='S';
   membuf[5]='U';
   membuf[6]='P';
   membuf[7]='0';
   *((LONG *)(&membuf[8])) =Bonus13_size;
   *((LONG *)(&membuf[12]))=Bonus14_size;
   printf("Writing header...");
   if( ! WriteFloppy(unit,HEADER_OFFSET,HEADER_SIZE,(APTR)membuf))
   {
      printf("\nError writing Header to drive %s\n",DriveNames[unit]);
      goto out;
   }
   printf("done.\n");

out:
   if(infile) fclose(infile);
   if(membuf) FreeMem(membuf,DISK_BUF_SIZE);
}
BOOL WriteFloppy(ULONG unit,ULONG offset,ULONG count,APTR address)
{
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport;
   BOOL           retval    = FALSE;
   BOOL err=TRUE;

   D1( printf("init.c: ReadFloppy(offset=%ld,count=0x%lx,address=0x%lx) enter\n",offset,count,address); )

   if((diskport=CreatePort(NULL,NULL))==NULL)goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,unit,(struct IORequest *)diskreq,0L))goto out;

   /*** MOTOR ON ***/
   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);

   D1( printf("init.c: ReadFloppy() after motor on\n"); )

   /*** DO WRITE ***/
   diskreq->iotd_Req.io_Length =count;
   diskreq->iotd_Req.io_Data   =address;
   diskreq->iotd_Req.io_Command=CMD_WRITE;
   diskreq->iotd_Req.io_Offset =offset;
   DoIO((struct IORequest *)diskreq);
   if( ! diskreq->iotd_Req.io_Error)
   {
      retval=TRUE;
   }

   diskreq->iotd_Req.io_Command=CMD_UPDATE;
   DoIO((struct IORequest *)diskreq);
   diskreq->iotd_Req.io_Command= CMD_CLEAR;
   DoIO((struct IORequest *)diskreq);

   D1( printf("init.c: ReadFloppy() after read\n"); )

   /*** MOTOR OFF ***/
   diskreq->iotd_Req.io_Length=0;
   diskreq->iotd_Req.io_Command=TD_MOTOR;
   DoIO((struct IORequest *)diskreq);

   D1( printf("init.c: ReadFloppy() after motor off\n"); )

   CloseDevice((struct IORequest *)diskreq);
   err=0;

out:
   if(err)printf("ERROR in write operation\n");

   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
   return(retval);
}
BOOL DiskInDrive(ULONG unit)
{
   struct IOExtTD *diskreq  = NULL;
   struct MsgPort *diskport;
   BOOL           retval    = FALSE;

   D1( printf("init.c: DiskInDrive() enter\n"); )

   if((diskport=CreatePort(NULL,NULL))==NULL)
      goto out;

   if((diskreq=(struct IOExtTD *)CreateExtIO(diskport,sizeof(struct IOExtTD)))==NULL)
      goto out;

   if(OpenDevice(TD_NAME,unit,(struct IORequest *)diskreq,0L))
      goto out;

   diskreq->iotd_Req.io_Length=1;
   diskreq->iotd_Req.io_Command=TD_CHANGESTATE;
   /*DoIO((struct IORequest *)diskreq);*/
   SendIO((struct IORequest *)diskreq);
   WaitIO((struct IORequest *)diskreq);
   if(diskreq->iotd_Req.io_Actual==0)
   {
      retval=TRUE;
   } else {
      retval=FALSE;
   }

   CloseDevice((struct IORequest *)diskreq);

out:
   if(diskreq)  DeleteExtIO((struct IORequest *)diskreq);
   if(diskport) DeletePort(diskport);
   return(retval);
}
LONG DeviceNumber(char *s)
{
   D1( printf("DeviceNumber(%s)\n",s); )

   if(((s[0]!='d')&&(s[0]!='D'))||
      ((s[1]!='f')&&(s[1]!='F'))||
      ((s[3]!=':')&&(s[3]!=':')))
   {
      D1( printf("DeviceNumber first case fail\n"); )
      return(4);
   }
   if(s[2]=='0') return(0);
   if(s[2]=='1') return(1);
   if(s[2]=='2') return(2);
   if(s[2]=='3') return(3);
   D1( printf("DeviceNumber second case fail\n"); )
   return(4);
}
