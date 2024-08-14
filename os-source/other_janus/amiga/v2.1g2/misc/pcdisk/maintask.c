/************************************************************************
 * Maintask.c - all PCDisk functions performed here.
 *
 * 02-02-88 Bill Koester
 ************************************************************************/
#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <janus/janus.h>
#include "pcddata.h"                /* private include file */

#define D(x) ;
#define printf kprintf

extern struct FileTable ft;
extern int open_files;
extern char *buffer;
extern char *rbuffer;

extern UBYTE Debug;
extern UBYTE DMA;

void MainProg();

void MainProg(sigmask,sigstruct,ReqPtr)
int sigmask;
struct SetupSig *sigstruct;
struct AmigaDiskReq *ReqPtr;
{
   short error;

   for(;;)
   {
      Wait(sigmask);
      if(Debug)
         dprintf("S->");
      switch(ReqPtr->adr_Fnctn)
      {
         case ADR_FNCTN_INIT:
               error=pcd_init(ReqPtr);
               break;
         case ADR_FNCTN_READ:
               error=pcd_read(ReqPtr);
               break;
         case ADR_FNCTN_WRITE:
               error=pcd_write(ReqPtr);
               break;
         case ADR_FNCTN_SEEK:
               error=pcd_seek(ReqPtr);
               break;
         case ADR_FNCTN_INFO:
               error=pcd_info(ReqPtr);
               break;
         case ADR_FNCTN_OPEN_OLD:
               error=pcd_open_old(ReqPtr);
               break;
         case ADR_FNCTN_OPEN_NEW:
               error=pcd_open_new(ReqPtr);
               break;
         case ADR_FNCTN_CLOSE:
               error=pcd_close(ReqPtr);
               break;
         case ADR_FNCTN_DELETE:
               error=pcd_delete(ReqPtr);
               break;
         default:
               error = ADR_ERR_FNCTN;
               break;
      }
      if(Debug)
         dprintf("Err=%ld->",error);
      ReqPtr->adr_Err = error;
      SendJanusInt(JSERV_PCDISK);
      if(Debug)
         dprintf("R\n");
   }
}
/************************************************************************
 * ADR_FNCTN_INIT: Close all open files
 ************************************************************************/
pcd_init(adr)
struct AmigaDiskReq *adr;
{
   int x;

   if(Debug)
      dprintf("Init: ");

   for(x=0;x<MAXFILES;x++)
      if(ft.File[x].fh)
      {
         Close(ft.File[x].fh);
         ft.File[x].fh=0;
         ft.File[x].cur_pos=0;
      }

   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_READ: reads bytes given file number, offset and count.
 *                 Returns count=number of bytes read and offset is
 *                 updated.
 ************************************************************************/
pcd_read(adr)
struct AmigaDiskReq *adr;
{
   int length;
   int error;

   if(Debug)
      dprintf("Rd: ");

   length = *((int *)&adr->adr_Count_h);

   if(Debug)
      dprintf("Cnt=%ld ",length);

   adr->adr_Count_l = (-1);
   error=pcd_realseek(adr);
   *((int *)&adr->adr_Count_h) = 0;
   if(error) return(error);

   if(Debug)
      dprintf("File=%d ",adr->adr_File);

   if( DMA )
   {
      error=Read(ft.File[adr->adr_File].fh,buffer,length);
   } else {
      if(Debug)
         dprintf("NODMA ");
      error=Read(ft.File[adr->adr_File].fh,rbuffer,length);
      CopyMem(rbuffer,buffer,length);
   }

   if(Debug)
      dprintf("Act=%ld ",error);

   *((int *)&adr->adr_Count_h) = error;

   if(error<0) return(ADR_ERR_READ);

   *((int *)&adr->adr_Offset_h) += error;
   ft.File[adr->adr_File].cur_pos += error;

   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_WRITE:
 ************************************************************************/
pcd_write(adr)
struct AmigaDiskReq *adr;
{
   int length;
   int error;

   if(Debug)
      dprintf("Wr: ");

   length = *((int *)&adr->adr_Count_h);

   if(Debug)
      dprintf("Cnt=%ld ",length);

   adr->adr_Count_l = (-1);
   error=pcd_realseek(adr);
   *((int *)&adr->adr_Count_h) = 0;
   if(error) return(error);

   if(Debug)
      dprintf("File=%d ",adr->adr_File);

   if( DMA )
   {
      error=Write(ft.File[adr->adr_File].fh,buffer,length);
   } else {
      if(Debug)
         dprintf("NODMA ");
      CopyMem(buffer,rbuffer,length);
      error=Write(ft.File[adr->adr_File].fh,rbuffer,length);
   }

   if(Debug)
      dprintf("Act=%ld ",error);

   *((int *)&adr->adr_Count_h) = error;

   if(error<0) return(ADR_ERR_WRITE);

   *((int *)&adr->adr_Offset_h) += error;
   ft.File[adr->adr_File].cur_pos += error;

   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_SEEK: Seeks to a byte given file, offset and count. w = seek
                   mode, -1 = offset begining, 0 = offset current, 1 =
                   offset end.
 ************************************************************************/
pcd_seek(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
      dprintf("Seek: ");
   return(pcd_realseek(adr));
}
/************************************************************************
 * realseek used by read/write & seek. Will pad extend a file if an attempt
 *          is made to seek past EOF.
 ************************************************************************/
pcd_realseek(adr)
struct AmigaDiskReq *adr;
{
   int result;
   int error;
   int pad;

   if(ft.File[adr->adr_File].fh==0)    /* No file! */
      return(ADR_ERR_FILE);

   if(Debug)
      dprintf("Off=%ld ",*((int *)&adr->adr_Offset_h));

   /* Already there ? */
   if( (*((int *)&adr->adr_Offset_h)) == ft.File[adr->adr_File].cur_pos)
      return(ADR_ERR_OK);

   /* nope seek there */
   result=Seek(ft.File[adr->adr_File].fh,*((int *)&adr->adr_Offset_h)
                                 ,(int)((short)adr->adr_Count_l));

   /* seek was in file */
   if(result >= 0 )
      goto noendseek;

   /* seek error so goto end of file */
   Seek(ft.File[adr->adr_File].fh,0,1); /* seek to end */

noendseek:

   /* get current pos */
   if((result=Seek(ft.File[adr->adr_File].fh,0,0)) < 0)
      return(ADR_ERR_SEEK);
   /* if thats what we want then out */
   if(result==(*((int *)&adr->adr_Offset_h)))
      goto ten;


   pad = *((int *)&adr->adr_Offset_h) - result;
   if(Debug)
      dprintf("Pad=%ld ",pad);
   while(pad>PCDISK_BUFFER_SIZE)
   {
      error=Write(ft.File[adr->adr_File].fh,buffer,PCDISK_BUFFER_SIZE);
      if(error<PCDISK_BUFFER_SIZE)
         return(ADR_ERR_SEEK);
      pad -= PCDISK_BUFFER_SIZE;
   }
   if(pad!=0)
   {
      error=Write(ft.File[adr->adr_File].fh,buffer,pad);
      if(error<pad)
         return(ADR_ERR_SEEK);
   }


ten:

   /* set current pos and exit */
   ft.File[adr->adr_File].cur_pos = result;
   if(Debug)
      dprintf("SeekEnd=%ld ",result);


   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_INFO: Dummy command, not used.
 ************************************************************************/
pcd_info(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
      dprintf("Info: ");
   return(ADR_ERR_FNCTN);
}
/************************************************************************
 * ADR_FNCTN_OPEN_OLD: returns adr_File = the array number of an opened
 *                                        old file.
 ************************************************************************/
pcd_open_old(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
      dprintf("OpenOld: ");
   return(pcd_real_open(adr,MODE_OLDFILE));
}
/************************************************************************
 * ADR_FNCTN_OPEN_NEW: returns adr_File = the array number of an opened
 *                                        new file.
 ************************************************************************/
pcd_open_new(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
      dprintf("OpenNew: ");
   return(pcd_real_open(adr,MODE_NEWFILE));
}
/************************************************************************
 * Internal open called by OpenOld & OpenNew
 ************************************************************************/
pcd_real_open(adr,mode)
struct AmigaDiskReq *adr;
int mode;
{
   int x;

   if(open_files==MAXFILES)
      return(ADR_ERR_FILE_COUNT);

   for(x=0;x<MAXFILES;x++)
      if(ft.File[x].fh == 0) break;

   if(Debug)
   {
      dprintf("Name=%s ",buffer);
      dprintf("File=%ld ",x);
   }
   if((ft.File[x].fh = (struct FileHandle *)Open(buffer,mode))==0)
   {
      if(IoErr()==ERROR_OBJECT_IN_USE)
      {
         return(ADR_ERR_LOCKED);
      }
      return(ADR_ERR_FILE);
   }
   open_files++;
   adr->adr_File=x;
   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_CLOSE: closes the file number given in adr_part
 ************************************************************************/
pcd_close(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
   {
      dprintf("Close: ");
      dprintf("File=%ld ",(int)adr->adr_File);
   }
   if(ft.File[adr->adr_File].fh==0)
      return(ADR_ERR_FILE);

   Close(ft.File[adr->adr_File].fh);
   ft.File[adr->adr_File].fh=0;
   ft.File[adr->adr_File].cur_pos=0;
   open_files--;
   return(ADR_ERR_OK);
}
/************************************************************************
 * ADR_FNCTN_DELETE: Deletes a file given the ASCIIZ filename in buffer
 ************************************************************************/
pcd_delete(adr)
struct AmigaDiskReq *adr;
{
   if(Debug)
   {
      dprintf("Delete: ");
      dprintf(" name=%s ",buffer);
   }

   if(DeleteFile(buffer)!=0)
      return(ADR_ERR_FILE);

   return(ADR_ERR_OK);
}

