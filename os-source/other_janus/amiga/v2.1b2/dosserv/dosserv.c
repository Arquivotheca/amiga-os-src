/************************************************************************
 * DOSServ.c  - 
 *
 * 11-19-90  -  New Code -  Bill Koester
 *
 * Copyright (c) 1990, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#define LINT_ARGS
#define PRAGMAS
#define D(x) ;
#define printf kprintf

#include <janus/janus.h>
#include <libraries/dos.h>

#include "pat.h"

extern long SendPacket(struct MsgPort *pid, long action, long args[], long nargs);

struct JanusBase *JanusBase = 0;

struct FileInfoBlock FIB;

/**************/
/* File table */
/**************/
struct FileEntry {
   BPTR file;
   };

#define MAX_FILES 20
struct FileEntry FileList[MAX_FILES];
USHORT OpenFiles = 0;

/**************/
/* Lock table */
/**************/
struct LockEntry {
   BPTR lock;
   };
#define MAX_LOCKS 20
struct LockEntry LockList[MAX_LOCKS];
USHORT OpenLocks = 0;

/**********/
/* Buffer */
/**********/
#define BUFFER_SIZE (17*512)
UBYTE *Buffer;

/* running 2.0? */
int v37;

VOID DoFunction(struct ServiceData *sdb,struct ServiceData *sdw,
struct DOSServReq *dsrb,struct DOSServReq *dsrw);
void main();
void main()
{
   int    signum   = -1;
   int    sigmask  = 0;
   struct ServiceData *sdb, *sdw=0;
   struct DOSServReq *dsrb, *dsrw;
   int    error;
   struct JanusAmiga *jab,*jaw;
   USHORT x;

	v37 = FALSE;

   for(x=0;x<MAX_LOCKS;x++)
      LockList[x].lock=-1;

   for(x=0;x<MAX_FILES;x++)
      FileList[x].file=0;

   /********************
    * Open Janus.Library
    ********************/
   JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME,0);
   if(JanusBase==0)
   {
      D( printf("DS:Unable to open Janus.Library"); )
      goto cleanup;
   }
   D( kprintf("DS:JanusBase at %lx\n",JanusBase); )

   /***********************
    * Allocate a signal bit
    ***********************/
   signum = AllocSignal(-1);
   if(signum == -1)
   {
      D( printf("DS:No free signal bits"); )
      goto cleanup;
   }
   D( kprintf("DS:Signum = %ld\n",signum); )

   /**********************************
    * Create a signal mask for Wait();
    **********************************/
   sigmask = 1 << signum;

   /***************************************
    * Add our new service to the system and
    * allocate our Parameter Memory
    ***************************************/
   error = AddService(&sdw,DOSSERV_APPLICATION_ID,DOSSERV_LOCAL_ID,
                      sizeof(struct DOSServReq),
                      MEMF_PARAMETER|MEM_WORDACCESS,
                      (USHORT)signum,ADDS_FROMPC_ONLY|ADDS_TOPC_ONLY|ADDS_LOCKDATA);
   if(error!=JSERV_OK)
   {
      D( kprintf("DS:Error adding service!\n"); )
      goto cleanup;
   }
   D( kprintf("DS:sdw = %lx\n",sdw); )
   sdb = (struct ServiceData *)MakeBytePtr((APTR)sdw);
   D( kprintf("DS:sdb=0x%lx\n",sdb); )

   /**********************/
   /* Structure pointers */
   /**********************/
   dsrb = (struct DOSServReq *)sdw->sd_AmigaMemPtr;
   dsrb = (struct DOSServReq *)MakeBytePtr((APTR)dsrb);
   dsrw = (struct DOSServReq *)MakeWordPtr((APTR)dsrb);

   JanusInitLock(&dsrb->dsr_Lock);

   D( kprintf("DS:dsrb=0x%lx\n",dsrb); )
   D( kprintf("DS:dsrw=0x%lx\n",dsrw); )

   /*********************/
   /* Get buffer memory */
   /*********************/
   Buffer=AllocServiceMem(sdw,BUFFER_SIZE,MEMF_BUFFER|MEM_BYTEACCESS);
   if(Buffer==NULL)
   {
      D( kprintf("DS:Error Getting buffer mem!\n"); )
      goto cleanup;
   }
   D( kprintf("DS:Buffer=0x%lx\n",Buffer); )
   dsrw->dsr_Buffer_Off=JanusMemToOffset(Buffer);
   jab=(struct JanusAmiga *)MakeBytePtr((APTR)JanusBase->jb_ParamMem);
   jaw=(struct JanusAmiga *)MakeWordPtr((APTR)JanusBase->jb_ParamMem);
   dsrw->dsr_Buffer_Seg=jaw->ja_BufferMem.jmh_8088Segment;

   dsrw->dsr_Buffer_Size=BUFFER_SIZE;

   /***********************/
   /* Clear File pointers */
   /***********************/
   for(x=0;x<MAX_FILES;x++)
      FileList[x].file=0;

   /***********************/
   /* Clear Lock pointers */
   /***********************/
   for(x=0;x<MAX_LOCKS;x++)
      LockList[x].lock=-1;

   /*********************************************************
    * DOSServ mainloop. This will never exit! once added the
    * service will be available forever and ever Amen!
    *********************************************************/
	D( kprintf("ServiceDataLock=0x%lx\n",sdb->sd_ServiceDataLock); )
	UnlockServiceData(sdb);
more:
   Wait(sigmask); /* wait for someone to call us */
   D( kprintf("DS:DOSServ is being called!!!!!\n"); )



   if(sdw->sd_Flags & EXPUNGE_SERVICE)
      goto cleanup;

   DoFunction(sdb,sdw,dsrb,dsrw);

   /****************************************************************
    * When we are finished with a request we signal the caller(s) by
    * calling our own service.
    ****************************************************************/
/*   D( kprintf("DS:DOSServ is calling the PC!\n"); )
*/
   CallService(sdw);
   /* sdw->sd_Flags&=~SERVICE_PCWAIT; */
   D( kprintf("DS:DOSServ has called the PC!\n"); )

   goto more; 

   /***********************************************************
    * This is our cleanup routine if all steps up to
    * and including AddService are OK this will never be called
    ***********************************************************/
cleanup:
   if(sdw)         DeleteService(sdw);

   for(x=0;x<MAX_LOCKS;x++)
      if(LockList[x].lock != -1)
	     UnLock(LockList[x].lock);

   for(x=0;x<MAX_FILES;x++)
      if(FileList[x].file)
	     Close(FileList[x].file);

   if(signum!= -1) FreeSignal(signum);
   if(JanusBase)   CloseLibrary(JanusBase);
}

#define D(x) ;

void dumpbuf(UBYTE *ptr, int len)
{
	D(printf("%lx : ", ptr);)
	while (len--)
		D(printf("%lx ", *ptr++);)
	D(printf("\n");)
}


VOID DoFunction(struct ServiceData *sdb,struct ServiceData *sdw,
struct DOSServReq *dsrb,struct DOSServReq *dsrw)
{
   ULONG mode = 0;
   ULONG Arg1,Arg2,Arg3;
   ULONG x,t;
   ULONG eof;
   dsrw->dsr_Err=DSR_ERR_OK;

   switch(dsrw->dsr_Function)
   {
      case DSR_FUNC_OPEN_OLD:
	        D( kprintf("DSR_FUNC_OPEN_OLD: "); )
	        mode=MODE_OLDFILE;

      case DSR_FUNC_OPEN_NEW:
	        D( kprintf("DSR_FUNC_OPEN_NEW: "); )
	        if(mode==0)
			   mode=MODE_NEWFILE;

      case DSR_FUNC_OPEN_READ_WRITE:
	        D( kprintf("DSR_FUNC_OPEN_READ_WRITE: "); )
	        if(mode==0)
			   mode=MODE_READWRITE;
D(kprintf("file '%s', ", Buffer);)
			dsrw->dsr_Arg1_h=0;
			dsrw->dsr_Arg1_l=0;

            if(OpenFiles==MAX_FILES)
			{
			   dsrw->dsr_Err=DSR_ERR_TOO_MANY_FILES;
	           D( kprintf(" DSR_ERR_TOO_MANY_FILES "); )
			   break;
			}

			for(x=0;x<MAX_FILES;x++)
			{
			   if(FileList[x].file==0)
			   {

                  FileList[x].file=(BPTR)Open(Buffer,mode);
      			  if(FileList[x].file==NULL)
      			  {
      			     dsrw->dsr_Err=DSR_ERR_OPEN_ERROR;
	                 D( kprintf(" DSR_ERR_OPEN_ERROR "); )
      			     break;
      			  }
      			  OpenFiles++;
      			  dsrw->dsr_Arg1_h=(x>>16)&0xffff;
      			  dsrw->dsr_Arg1_l=x&0x0000ffff;
	              D( kprintf(" OpenFiles=0x%lx ",OpenFiles); )
	              D( kprintf(" Arg1=0x%lx ",(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l); )
				  break;
			   }
			}
	        break;


      case DSR_FUNC_CLOSE:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_CLOSE: file 0x%lx, ", x); )
            if((x>=OpenFiles)||(FileList[x].file==0))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   break;
			}
	        Close(FileList[x].file);
			FileList[x].file=0;
			OpenFiles--;
	        break;


      case DSR_FUNC_READ:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_READ: file = 0x%lx, length = 0x%lx, ",x , (dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l);)
            if((x>=OpenFiles)||(FileList[x].file==0))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   D( kprintf("\n## FILE_NOT_OPEN OpenFiles=0x%lx x=0x%lx file=0x%lx\n",OpenFiles,x,FileList[x].file); )
			   break;
			}
			t=Read(FileList[x].file,Buffer,
			        ((dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l));
	        D( kprintf("actual = 0x%lx ",t); )
			dsrw->dsr_Arg3_h=(t>>16)&0xffff;
			dsrw->dsr_Arg3_l=t&0x0000ffff;
			break;


      case DSR_FUNC_WRITE:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_WRITE: file = 0x%lx, length = 0x%lx, ",x , (dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l);)
            if((x>=OpenFiles)||(FileList[x].file==0))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   break;
			}
			t=Write(FileList[x].file,Buffer,
			        ((dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l));
	        D( kprintf("actual = 0x%lx ",t); )
			dsrw->dsr_Arg3_h=(t>>16)&0xffff;
			dsrw->dsr_Arg3_l=t&0x0000ffff;
			break;


      case DSR_FUNC_SEEK_BEGINING:
	        D( kprintf("DSR_FUNC_SEEK_BEGINING: "); )
	        mode=OFFSET_BEGINING;

      case DSR_FUNC_SEEK_CURRENT:
	        D( kprintf("DSR_FUNC_SEEK_CURRENT: "); )
	        if(mode==0)
			   mode=OFFSET_CURRENT;

      case DSR_FUNC_SEEK_END:
	        D( kprintf("DSR_FUNC_SEEK_END: "); )
	        if(mode==0)
			   mode=OFFSET_END;

	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("file = 0x%lx, offset = 0x%lx, ",x , (dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l);)
            if((x>=OpenFiles)||(FileList[x].file==0))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   break;
			}
            t=Seek(FileList[x].file,
			       ((dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l),mode);
            D( kprintf("result=0x%lx\n", t); )
			dsrw->dsr_Arg3_h=(t>>16)&0xffff;
			dsrw->dsr_Arg3_l=t&0x0000ffff;
			break;

      case DSR_FUNC_SEEK_EXTEND:
	        Arg1=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        Arg2=(dsrw->dsr_Arg2_h<<16)|dsrw->dsr_Arg2_l;
	        Arg3=(dsrw->dsr_Arg3_h<<16)|dsrw->dsr_Arg3_l;
	        D( kprintf("DSR_FUNC_SEEK_EXTEND: file = 0x%lx, seek = 0x%lx, extend = 0x%lx, ", Arg1, Arg2, Arg3); )
            if((Arg1>=OpenFiles)||(FileList[Arg1].file==0))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   break;
			}
			if(Seek(FileList[Arg1].file,0L,OFFSET_END)<0)
			{
			   dsrw->dsr_Err=DSR_ERR_SEEK_ERROR;
			   goto byebye;
			}
			if((eof=Seek(FileList[Arg1].file,0L,OFFSET_CURRENT))<0)
			{
			   dsrw->dsr_Err=DSR_ERR_SEEK_ERROR;
			   goto byebye;
			}
			if(Arg3>eof)
			{
			   for(x=0;x<((Arg3-eof)/512);x++)
			   {
			      if(Write(FileList[Arg1].file,Buffer,512L)<0)
				  {
			         dsrw->dsr_Err=DSR_ERR_SEEK_ERROR;
			         goto byebye;
				  }
			   }
			   if((Arg3-eof)%512L)
			   {
			      if(Write(FileList[Arg1].file,Buffer,(Arg3-eof)%512L)<0)
				  {
			         dsrw->dsr_Err=DSR_ERR_SEEK_ERROR;
			         goto byebye;
				  }
			   }
			}
			if(Seek(FileList[Arg1].file,Arg2,OFFSET_BEGINING)<0)
			{
			   dsrw->dsr_Err=DSR_ERR_SEEK_ERROR;
			   goto byebye;
			}
			break;

      case DSR_FUNC_CREATE_DIR:
	        D( kprintf("DSR_FUNC_CREATE_DIR: dir = '%s', ", Buffer); )

			dsrw->dsr_Arg1_h=0;
			dsrw->dsr_Arg1_l=0;

            if(OpenLocks==MAX_LOCKS)
			{
			   dsrw->dsr_Err=DSR_ERR_TOO_MANY_LOCKS;
	           D( kprintf(" DSR_ERR_TOO_MANY_LOCKS "); )
			   break;
			}

			for(x=0;x<MAX_LOCKS;x++)
			{
			   if(LockList[x].lock==-1)
			   {

                  LockList[x].lock=(BPTR)CreateDir(Buffer);
      			  if(LockList[x].lock==NULL)
      			  {
				LockList[x].lock = -1;
      			     dsrw->dsr_Err=DSR_ERR_OPEN_ERROR;
	                 D( kprintf(" DSR_ERR_OPEN_ERROR "); )
      			     break;
      			  }
      			  OpenLocks++;
      			  dsrw->dsr_Arg1_h=(x>>16)&0xffff;
      			  dsrw->dsr_Arg1_l=x&0x0000ffff;
	              D( kprintf(" result=0x%lx ",(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l); )
				  break;
			   }
			}
	        break;

      case DSR_FUNC_LOCK:
	        t=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
			dsrw->dsr_Arg1_h=0;
			dsrw->dsr_Arg1_l=0;

	        D( kprintf("DSR_FUNC_LOCK: file = '%s', mode = 0x%lx, ", Buffer, t); )

            if(OpenLocks==MAX_LOCKS)
			{
			   dsrw->dsr_Err=DSR_ERR_TOO_MANY_LOCKS;
	           D( kprintf(" DSR_ERR_TOO_MANY_LOCKS "); )
			   break;
			}

			for(x=0;x<MAX_LOCKS;x++)
			{
			   if(LockList[x].lock==-1)
			   {

                  LockList[x].lock=(BPTR)Lock(Buffer,t);
      			  if(LockList[x].lock==NULL)
      			  {
				LockList[x].lock = -1;
      			     dsrw->dsr_Err=DSR_ERR_OPEN_ERROR;
	                 D( kprintf(" DSR_ERR_OPEN_ERROR "); )
      			     break;
      			  }
      			  OpenLocks++;
      			  dsrw->dsr_Arg1_h=(x>>16)&0xffff;
      			  dsrw->dsr_Arg1_l=x&0x0000ffff;
	              D( kprintf(" lock=0x%lx ",(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l); )
				  break;
			   }
			}
	        break;

      case DSR_FUNC_UNLOCK:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_UNLOCK: lock = 0x%lx, ", x); )
            if((x>=OpenLocks)||(LockList[x].lock==-1))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   break;
			}
	        UnLock(LockList[x].lock);
			LockList[x].lock=-1;
			OpenLocks--;
	        break;

      case DSR_FUNC_EXAMINE:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_EXAMINE: lock = 0x%lx, ", x); )
            if((x>=OpenLocks)||(LockList[x].lock==-1))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   D( kprintf(
			      "\n## FILE_NOT_OPEN OpenFiles=0x%lx x=0x%lx file=0x%lx\n",
				  OpenFiles,x,FileList[x].file); )
			   break;
			}
			t=Examine(LockList[x].lock,&FIB);
			memcpy(Buffer,(char *)&FIB,sizeof(struct FileInfoBlock));
	        D( kprintf("t = 0x%lx ",t); )
			dsrw->dsr_Arg1_h=(t>>16)&0xffff;
			dsrw->dsr_Arg1_l=t&0x0000ffff;
			break;

	case DSR_FUNC_EXNEXT:
	        x=(dsrw->dsr_Arg1_h<<16)|dsrw->dsr_Arg1_l;
	        D( kprintf("DSR_FUNC_EXNEXT: lock = 0x%lx, ", x); )
            if((x>=OpenLocks)||(LockList[x].lock==-1))
			{
			   dsrw->dsr_Err=DSR_ERR_FILE_NOT_OPEN;
			   D( kprintf(
			      "\n## FILE_NOT_OPEN OpenFiles=0x%lx x=0x%lx file=0x%lx\n",
				  OpenFiles,x,FileList[x].file); )
			   break;
			}
			t=ExNext(LockList[x].lock,&FIB);
			memcpy(Buffer,(char *)&FIB,sizeof(struct FileInfoBlock));
	        D( kprintf("t = 0x%lx ",t); )
			dsrw->dsr_Arg1_h=(t>>16)&0xffff;
			dsrw->dsr_Arg1_l=t&0x0000ffff;
			break;

	case DSR_FUNC_GETCURRENTDIR:
		D(kprintf("DSR_FUNC_GETCURRENTDIR: ");)
		if (OpenLocks == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
		} else {
			for (x = 0; x < MAX_LOCKS; x++) {
				if (LockList[x].lock == -1) {
					Arg1 = CurrentDir(0);
					CurrentDir(Arg1);
					D(kprintf("lock = 0x%lx, ", x);)
					LockList[x].lock = Arg1;
					Arg1 = x;
					OpenLocks++;
					dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
					dsrw->dsr_Arg1_l = Arg1 & 0xffff;
					break;
				}
			}
			if (x == MAX_LOCKS) {
				dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			}
		}
		break;

	case DSR_FUNC_ENDCURRENTDIR:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
		D(kprintf("DSR_FUNC_ENDCURRENTDIR: lock = 0x%lx, ", Arg1);)
		if (LockList[Arg1].lock != -1) {
			CurrentDir(LockList[Arg1].lock);
			LockList[Arg1].lock = -1;
			OpenLocks--;
		}
		break;

	case DSR_FUNC_SETCURRENTDIR:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
		D(kprintf("DSR_FUNC_SETCURRENTDIR: lock = 0x%lx, ", Arg1);)
		if (Arg1 >= OpenLocks || LockList[Arg1].lock == -1) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			CurrentDir(LockList[Arg1].lock);
		}
		break;

	case DSR_FUNC_DELETEFILE:
		D(kprintf("DSR_FUNC_DELETEFILE: file = '%s', ", Buffer);)
		Arg1 = DeleteFile(Buffer);
		dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = Arg1 & 0xffff;
		break;

	case DSR_FUNC_DUPLOCK:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
		D(kprintf("DSR_FUNC_DUPLOCK: oldlock = 0x%lx, ", Arg1);)
		if (Arg1 >= OpenLocks || LockList[Arg1].lock == -1) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else if (OpenLocks == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
		} else {
			for (x = 0; x < MAX_LOCKS; x++) {
				if (LockList[x].lock == -1) {
					Arg1 = DupLock(LockList[Arg1].lock);
		D(kprintf("newlock = 0x%lx, ", x);)
					LockList[x].lock = Arg1;
					Arg1 = x;
					OpenLocks++;
					dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
					dsrw->dsr_Arg1_l = Arg1 & 0xffff;
					break;
				}
			}
			if (x == MAX_LOCKS) {
				dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			}
		}
		break;

	case DSR_FUNC_PARENTDIR:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
		D(kprintf("DSR_FUNC_PARENTDIR: oldlock = 0x%lx, ", Arg1);)
		if (Arg1 >= OpenLocks || LockList[Arg1].lock == -1) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else if (OpenLocks == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
		} else {
			for (x = 0; x < MAX_LOCKS; x++) {
				if (LockList[x].lock == -1) {
					Arg1 = ParentDir(LockList[Arg1].lock);
		D(kprintf("newlock = 0x%lx, ", x);)
					LockList[x].lock = Arg1;
					Arg1 = x;
					OpenLocks++;
					dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
					dsrw->dsr_Arg1_l = Arg1 & 0xffff;
					break;
				}
			}
			if (x == MAX_LOCKS) {
				dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			}
		}
		break;

	case DSR_FUNC_RENAME:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;		
		D(kprintf("DSR_FUNC_DELETEFILE: file = '%s', ", Buffer);)
		Arg1 = Rename(&Buffer[0], &Buffer[Arg1]);
		dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = Arg1 & 0xffff;
		break;

	case DSR_FUNC_SETPROTECTION:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
	D(kprintf("DSR_FUNC_SETPROTECTION: file = '%s', bits = 0x%lx, ", Buffer, Arg1);)
		Arg1 = SetProtection(Buffer, Arg1);
		dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = Arg1 & 0xffff;
		break;

	case DSR_FUNC_PARSEPATTERN:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
		Arg2 = (dsrw->dsr_Arg2_h << 16) | dsrw->dsr_Arg2_l;
D(kprintf("DSR_FUNC_PARSEPATTERN IN: [0] = '%s', inlen = %ld, outlen = %ld, ", &Buffer[0], Arg1, Arg2);)
D(dumpbuf(Buffer, 30);)
		if (v37) {
/*			x = ParsePattern(&Buffer[0], &Buffer[Arg1], Arg2);
*/
		} else {
			x = xParsePattern(&Buffer[0], &Buffer[Arg1], Arg2);
		}
D(dumpbuf(Buffer, 30);)
D(kprintf("DSR_FUNC_PARSEPATTERN OUT: [0] = '%s', [%ld] = '%s', len = %ld, wild = 0x%ld, ", &Buffer[0], Arg1, &Buffer[Arg1], Arg2, x);)
		dsrw->dsr_Arg1_h = (x >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = x & 0xffff;
		break;

	case DSR_FUNC_MATCHPATTERN:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
D(kprintf("DSR_FUNC_MATCHPATTERN IN: [0] = '%s', [%ld] = '%s', ", &Buffer[0], Arg1, &Buffer[Arg1]);)
D(dumpbuf(Buffer, 30);)
		if (v37) {
/*			x = MatchPattern(&Buffer[0], &Buffer[Arg1]);
*/
		} else {
			x = xMatchPattern(&Buffer[0], &Buffer[Arg1]);
		}
D(dumpbuf(Buffer, 30);)
D(kprintf("DSR_FUNC_MATCHPATTERN OUT: [0] = '%s', [%ld] = '%s', result = 0x%ld, ", &Buffer[0], Arg1, &Buffer[Arg1], x);)
		dsrw->dsr_Arg1_h = (x >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = x & 0xffff;
		break;

	case DSR_FUNC_SETFILEDATE:
		Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;

		if (v37) {
/*			x = SetFileDate(&Buffer[0], &Buffer[Arg1]);
*/
		} else {
			ULONG array[4];
			struct MsgPort *pid;
			char *s;
			struct DateStamp *d_s;
			int l;

			x = 0;

			l = strlen(&Buffer[0]);
			if (s = malloc(l + 2)) {
				s[0] = l;
				memcpy(&s[1], &Buffer[0], l + 1);
				if (d_s = malloc(sizeof(struct DateStamp))) {
					memcpy(d_s, &Buffer[Arg1], sizeof(struct DateStamp));
					if (pid = DeviceProc(&Buffer[0])) {
						array[1] = IoErr();
						array[2] = ((ULONG)s) >> 2;
						array[3] = d_s;
						x = SendPacket(pid, 34 /* ACTION_SET_DATE */, array, 4);
					}
					free(d_s);
				}
				free(s);
			}
		}

		dsrw->dsr_Arg1_h = (x >> 16) & 0xffff;
		dsrw->dsr_Arg1_l = x & 0xffff;
		break;

      default:
	        D( kprintf("DSR_UNKNOWN_FUNC: (%ld)", dsrw->dsr_Function); )
	        dsrw->dsr_Err=DSR_ERR_UNKNOWN_FUNCTION;
			break;
   }
byebye:
   D( kprintf(" dsr_Err=0x%lx\n",dsrw->dsr_Err); )
}
