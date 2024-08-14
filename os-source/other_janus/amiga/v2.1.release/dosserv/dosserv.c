/************************************************************************
 * DOSServ.c  - 
 *
 * 11-19-90  -  New Code -  Bill Koester
 *
 * Copyright (c) 1990, Commodore Amiga Inc., All rights reserved
 ************************************************************************/

#define LINT_ARGS
#define PRAGMAS
#define D(x)
#define printf kprintf

#include <janus/janus.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

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

void main()
{
int signum = -1;
int sigmask = 0;
struct ServiceData *sdb, *sdw = 0;
struct DOSServReq *dsrb, *dsrw;
int error;
struct JanusAmiga *jab, *jaw;
USHORT x;

	v37 = FALSE;

	for (x = 0; x < MAX_LOCKS; x++)
		LockList[x].lock = -1;

	for (x = 0; x < MAX_FILES; x++)
		FileList[x].file = 0;

	/********************
	 * Open Janus.Library
	 ********************/
	JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME, 0);
	if (JanusBase == 0) {
		D( printf("DS:Unable to open Janus.Library"); )
		goto cleanup;
	}
	D( kprintf("DS:JanusBase at %lx\n",JanusBase); )

	/***********************
	 * Allocate a signal bit
	 ***********************/
	signum = AllocSignal(-1);
	if (signum == -1) {
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
	error = AddService(&sdw, DOSSERV_APPLICATION_ID, DOSSERV_LOCAL_ID,
			   sizeof(struct DOSServReq),
			   MEMF_PARAMETER | MEM_WORDACCESS, (USHORT)signum,
			   ADDS_FROMPC_ONLY | ADDS_TOPC_ONLY | ADDS_LOCKDATA);

	if (error != JSERV_OK) {
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
	Buffer = AllocServiceMem(sdw, BUFFER_SIZE, MEMF_BUFFER | MEM_BYTEACCESS);
	if (Buffer == NULL) {
		D( kprintf("DS:Error Getting buffer mem!\n"); )
		goto cleanup;
	}

	D( kprintf("DS:Buffer=0x%lx\n",Buffer); )
	dsrw->dsr_Buffer_Off = JanusMemToOffset(Buffer);
	jab = (struct JanusAmiga *)MakeBytePtr((APTR)JanusBase->jb_ParamMem);
	jaw = (struct JanusAmiga *)MakeWordPtr((APTR)JanusBase->jb_ParamMem);
	dsrw->dsr_Buffer_Seg = jaw->ja_BufferMem.jmh_8088Segment;

	dsrw->dsr_Buffer_Size = BUFFER_SIZE;

	/***********************/
	/* Clear File pointers */
	/***********************/
	for (x = 0; x < MAX_FILES; x++)
		FileList[x].file = 0;

	/***********************/
	/* Clear Lock pointers */
	/***********************/
	for (x = 0; x < MAX_LOCKS; x++)
		LockList[x].lock = -1;

	/*********************************************************
	 * DOSServ mainloop. This will never exit! once added the
	 * service will be available forever and ever Amen!
	 *********************************************************/
	D( kprintf("ServiceDataLock=0x%lx\n",sdb->sd_ServiceDataLock); )
	UnlockServiceData(sdb);
more:
	Wait(sigmask); /* wait for someone to call us */
	D( kprintf("DS:DOSServ is being called!!!!!\n"); )

	if (sdw->sd_Flags & EXPUNGE_SERVICE)
		goto cleanup;

	DoFunction(sdb,sdw,dsrb,dsrw);

	/****************************************************************
	 * When we are finished with a request we signal the caller(s) by
	 * calling our own service.
	 ****************************************************************/
/*	D( kprintf("DS:DOSServ is calling the PC!\n"); )
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
	if (sdw)			DeleteService(sdw);

	for (x = 0; x < MAX_LOCKS; x++)
		if (LockList[x].lock != -1)
			UnLock(LockList[x].lock);

	for (x = 0; x < MAX_FILES; x++)
		if (FileList[x].file)
			Close(FileList[x].file);

	if (signum != -1)
		FreeSignal(signum);

	if (JanusBase)
		CloseLibrary(JanusBase);
}

#define D(x)

D( void dumpbuf(UBYTE *ptr, int len) )
D( { )
D(	kprintf("%lx : ", ptr); )
D(	while (len--) )
D(		kprintf("%lx ", *ptr++); )
D(	kprintf("\n"); )
D( } )


D( void dumpfib(struct FileInfoBlock *fib) )
D( { )
D( 	kprintf("fib.diskkey = %ld\n", fib->fib_DiskKey); )
D( 	kprintf("fib.direntrytype = %ld\n", fib->fib_DirEntryType); )
D( 	kprintf("fib.filename = %s\n", fib->fib_FileName); )
D( 	kprintf("fib.protection = 0x%lx\n", fib->fib_Protection); )
D( 	kprintf("fib.entrytype = %ld\n", fib->fib_EntryType); )
D( 	kprintf("fib.size = %ld\n", fib->fib_Size); )
D( 	kprintf("fib.numblocks = %ld\n", fib->fib_NumBlocks); )
D( 	kprintf("fib.ds.days = %ld\n", fib->fib_Date.ds_Days); )
D( 	kprintf("fib.ds.mins = %ld\n", fib->fib_Date.ds_Minute); )
D( 	kprintf("fib.ds.ticks = %ld\n", fib->fib_Date.ds_Tick); )
D( 	kprintf("fib.comment = %s\n", fib->fib_Comment); )
D( } )


#if 0
char *funcnames[] = {
	"UNKNOWN (0)",
	"DSR_FUNC_OPEN_OLD",
	"DSR_FUNC_OPEN_NEW",
	"DSR_FUNC_OPEN_READ_WRITE",
	"DSR_FUNC_CLOSE",
	"DSR_FUNC_READ",
	"DSR_FUNC_WRITE",
	"DSR_FUNC_SEEK_BEGINING",
	"DSR_FUNC_SEEK_END",
	"DSR_FUNC_SEEK_CURRENT",
	"DSR_FUNC_SEEK_EXTEND",
	"DSR_FUNC_CREATE_DIR",
	"DSR_FUNC_LOCK",
	"DSR_FUNC_UNLOCK",
	"DSR_FUNC_EXAMINE",
	"DSR_FUNC_EXNEXT",
	"DSR_FUNC_GETCURRENTDIR",
	"DSR_FUNC_SETCURRENTDIR",
	"DSR_FUNC_DELETEFILE",
	"DSR_FUNC_DUPLOCK",
	"DSR_FUNC_PARENTDIR",
	"DSR_FUNC_RENAME",
	"DSR_FUNC_SETPROTECTION",
	"DSR_FUNC_PARSEPATTERN",
	"DSR_FUNC_MATCHPATTERN",
	"DSR_FUNC_ENDCURRENTDIR",
	"DSR_FUNC_SETFILEDATE"
};
#endif

VOID DoFunction(struct ServiceData *sdb,struct ServiceData *sdw,
		struct DOSServReq *dsrb,struct DOSServReq *dsrw)
{
ULONG mode = 0;
ULONG Arg1, Arg2, Arg3, err;
ULONG x;
ULONG eof;

	Arg1 = (dsrw->dsr_Arg1_h << 16) | dsrw->dsr_Arg1_l;
	Arg2 = (dsrw->dsr_Arg2_h << 16) | dsrw->dsr_Arg2_l;
	Arg3 = (dsrw->dsr_Arg3_h << 16) | dsrw->dsr_Arg3_l;

	err = 0;

	D( if (dsrw->dsr_Function >= sizeof(funcnames)/sizeof(char *)) )
	D(	kprintf("UNKNOWN (%ld): ", dsrw->dsr_Function); )
	D( else )
	D(	kprintf("%s: ", funcnames[dsrw->dsr_Function]); )
	D( kprintf("A1 = 0x%lx, A2 = 0x%lx, A3 = 0x%lx\n", Arg1, Arg2, Arg3); )

	dsrw->dsr_Err = DSR_ERR_OK;

	switch (dsrw->dsr_Function) {

	case DSR_FUNC_OPEN_OLD:
		mode = MODE_OLDFILE;
		D( kprintf("mode = MODE_OLDFILE, "); )

	case DSR_FUNC_OPEN_NEW:
		if (mode == 0) {
			mode = MODE_NEWFILE;
			D( kprintf("mode = MODE_NEWFILE, "); )
		}

	case DSR_FUNC_OPEN_READ_WRITE:
		if (mode == 0) {
			mode = MODE_READWRITE;
			D( kprintf("mode = MODE_READWRITE, "); )
		}

		D( kprintf("file = %s, ", Buffer); )

		for (x = 0; x < MAX_FILES; x++) {
			if (FileList[x].file == 0) {
				FileList[x].file = (BPTR)Open(Buffer,mode);
				err = IoErr();
				if (FileList[x].file == NULL) {
					dsrw->dsr_Err = DSR_ERR_OPEN_ERROR;
					break;
				}

				OpenFiles++;
				Arg1 = x;
				break;
			}
		}

		if (x == MAX_FILES) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_FILES;
			Arg1 = 0;
		}

		D( kprintf(" filenum = %ld\n", Arg1); )

		break;

	case DSR_FUNC_CLOSE:
		D( kprintf("filenum = %ld\n", Arg1); )

		if ((Arg1 >= MAX_FILES) || (FileList[Arg1].file == 0)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			Close(FileList[Arg1].file);
			err = IoErr();
			FileList[Arg1].file = 0;
			OpenFiles--;
		}

		break;

	case DSR_FUNC_READ:
		D( kprintf("filenum = %ld, buf = 0x%lx, len = %ld, ", Arg1, Buffer, Arg2); )

		if ((Arg1 >= MAX_FILES) || (FileList[Arg1].file == 0)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			Arg3 = Read(FileList[Arg1].file, Buffer, Arg2);
			err = IoErr();
		}

		D( kprintf("actual = %ld\n", Arg3); )

		break;

	case DSR_FUNC_WRITE:
		D( kprintf("filenum = %ld, buf = 0x%lx, len = %ld, ", Arg1, Buffer, Arg2); )

		if ((Arg1 >= MAX_FILES) || (FileList[Arg1].file == 0)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
			Arg3 = 0;
		} else {
			Arg3 = Write(FileList[Arg1].file, Buffer, Arg2);
			err = IoErr();
		}

		D( kprintf("actual = %ld\n", Arg3); )

		break;

	case DSR_FUNC_SEEK_BEGINING:
		mode = OFFSET_BEGINING;
		D( kprintf("mode = OFFSET_BEGINNING, "); )

	case DSR_FUNC_SEEK_CURRENT:
		if (mode == 0) {
			mode = OFFSET_CURRENT;
			D( kprintf("mode = OFFSET_CURRENT, "); )
		}

	case DSR_FUNC_SEEK_END:
		if (mode == 0) {
			mode = OFFSET_END;
			D( kprintf("mode = OFFSET_END, "); )
		}

		D( kprintf("file = %ld, offset = %ld, ", Arg1, Arg2); )

		if ((Arg1 >= MAX_FILES) || (FileList[Arg1].file == 0)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
			Arg3 = 0;
		} else {
			Arg3 = Seek(FileList[Arg1].file, Arg2, mode);
			err = IoErr();
		}

		D( kprintf("result = %ld\n", Arg3); )

		break;

	case DSR_FUNC_SEEK_EXTEND:
		D( kprintf("file = %ld, offset = %ld\n", Arg1, Arg2); )

		if ((Arg1 >= MAX_FILES) || (FileList[Arg1].file == 0)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			if (Seek(FileList[Arg1].file, 0L, OFFSET_END) < 0) {
				dsrw->dsr_Err = DSR_ERR_SEEK_ERROR;
			} else if ((eof = Seek(FileList[Arg1].file, 0L, OFFSET_CURRENT)) < 0) {
				dsrw->dsr_Err = DSR_ERR_SEEK_ERROR;
			} else if (Arg3 > eof) {
				for (x = 0; x < ((Arg3 - eof) / 512); x++) {
					if (Write(FileList[Arg1].file, Buffer, 512L) < 0) {
						dsrw->dsr_Err = DSR_ERR_SEEK_ERROR;
						break;
					}
				}

				if (dsrw->dsr_Err == DSR_ERR_OK) {
					if ((Arg3 - eof) % 512L) {
						if (Write(FileList[Arg1].file, Buffer, (Arg3 - eof) % 512L) < 0)
							dsrw->dsr_Err = DSR_ERR_SEEK_ERROR;
					}
				}
			}
		}

		if (dsrw->dsr_Err == DSR_ERR_OK) {
			if (Seek(FileList[Arg1].file, Arg2, OFFSET_BEGINING) < 0)
				dsrw->dsr_Err = DSR_ERR_SEEK_ERROR;
		}

		err = IoErr();
		break;

	case DSR_FUNC_CREATE_DIR:
		D( kprintf("dir = %s, ", Buffer); )

		for (x = 0; x < MAX_LOCKS; x++) {
			if (LockList[x].lock == -1) {
				LockList[x].lock = (BPTR)CreateDir(Buffer);
				err = IoErr();
				if (LockList[x].lock == NULL) {
					LockList[x].lock = -1;
					dsrw->dsr_Err = DSR_ERR_OPEN_ERROR;
					Arg1 = 0;
					break;
				}
				OpenLocks++;
				Arg1 = x;
				break;
			}
		}

		if (x == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			Arg1 = 0;
			break;
		}

		D( kprintf("result = %ld\n", Arg1); )

		break;

	case DSR_FUNC_LOCK:
		D( kprintf("file = %s, mode = 0x%lx, ", Buffer, Arg1); )

		for (x = 0; x < MAX_LOCKS; x++) {
			if (LockList[x].lock == -1) {
				LockList[x].lock = (BPTR)Lock(Buffer, Arg1);
				err = IoErr();
				if (LockList[x].lock == NULL) {
					LockList[x].lock = -1;
					dsrw->dsr_Err = DSR_ERR_OPEN_ERROR;
					Arg1 = 0;
					break;
				}
				OpenLocks++;
				Arg1 = x;
			  	break;
			}
		}

		if (x == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			Arg1 = 0;
		}

		D( kprintf("result = %ld\n", Arg1); )

		break;

	case DSR_FUNC_UNLOCK:
		D( kprintf("lock = %ld\n", Arg1); )

		if ((Arg1 >= MAX_LOCKS) || (LockList[Arg1].lock == -1)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			UnLock(LockList[Arg1].lock);
			err = IoErr();
			LockList[Arg1].lock = -1;
			OpenLocks--;
		}

		break;

	case DSR_FUNC_EXAMINE:
		D( kprintf("lock = %ld\n", Arg1); )

		if ((Arg1 >= MAX_LOCKS) || (LockList[Arg1].lock == -1)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
			Arg1 = 0;
		} else {
			Arg1 = Examine(LockList[Arg1].lock, &FIB);
			err = IoErr();
			memcpy(Buffer, (char *)&FIB, sizeof(struct FileInfoBlock));
			D( dumpfib(Buffer); )
		}

		break;

	case DSR_FUNC_EXNEXT:
		D( kprintf("lock = %ld\n", Arg1); )

		if ((Arg1 >= MAX_LOCKS) || (LockList[Arg1].lock == -1)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			D( ULONG *p; )
			D( ULONG a1; )

			D( dumpfib(Buffer); )
			D( p = ((UBYTE *)BADDR(LockList[Arg1].lock)) + sizeof(struct FileLock); )
			D( p = *p; )
			D( kprintf("ET_CurrentKey = 0x%lx, ", *p); )
			D( p = *p; )
			D( kprintf("*ET_CurrentKey = 0x%lx\n", *p); )
			D( a1 = Arg1; )

			D( kprintf("----\n"); )

			memcpy((char *)&FIB, Buffer, sizeof(struct FileInfoBlock));
			Arg1 = ExNext(LockList[Arg1].lock, &FIB);
			err = IoErr();
			memcpy(Buffer, (char *)&FIB, sizeof(struct FileInfoBlock));

			D( dumpfib(Buffer); )
			D( p = ((UBYTE *)BADDR(LockList[a1].lock)) + sizeof(struct FileLock); )
			D( p = *p; )
			D( kprintf("ET_CurrentKey = 0x%lx, ", *p); )
			D( p = *p; )
			D( kprintf("*ET_CurrentKey = 0x%lx\n", *p); )
		}

		break;

	case DSR_FUNC_GETCURRENTDIR:
		for (x = 0; x < MAX_LOCKS; x++) {
			if (LockList[x].lock == -1) {
				Arg1 = CurrentDir(0);
				CurrentDir(Arg1);
				err = IoErr();
				LockList[x].lock = Arg1;
				Arg1 = x;
				OpenLocks++;
				break;
			}
		}

		if (x == MAX_LOCKS) {
			dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			Arg1 = 0;
		}

		D( kprintf("result = %ld\n", Arg1); )

		break;

	case DSR_FUNC_ENDCURRENTDIR:
		D( kprintf("lock = %ld\n", Arg1); )

		if ((Arg1 >= MAX_LOCKS) || (LockList[Arg1].lock == -1)) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			CurrentDir(LockList[Arg1].lock);
			err = IoErr();
			LockList[Arg1].lock = -1;
			OpenLocks--;
		}

		break;

	case DSR_FUNC_SETCURRENTDIR:
		D( kprintf("lock = %ld\n", Arg1); )

		if (Arg1 >= MAX_LOCKS || LockList[Arg1].lock == -1) {
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			CurrentDir(LockList[Arg1].lock);
			err = IoErr();
		}
		break;

	case DSR_FUNC_DELETEFILE:
		D( kprintf("file = %s\n", Buffer); )

		Arg1 = DeleteFile(Buffer);
		err = IoErr();

		break;

	case DSR_FUNC_DUPLOCK:
		D( kprintf("oldlock = %ld, ", Arg1); )

		if (Arg1 >= MAX_LOCKS || LockList[Arg1].lock == -1) {
			Arg1 = 0;
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			for (x = 0; x < MAX_LOCKS; x++) {
				if (LockList[x].lock == -1) {
					Arg1 = DupLock(LockList[Arg1].lock);
					err = IoErr();
					LockList[x].lock = Arg1;
					Arg1 = x;
					OpenLocks++;
					break;
				}
			}

			if (x == MAX_LOCKS) {
				Arg1 = 0;
				dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			}
		}

		D( kprintf("newlock = %ld\n", Arg1); )

		break;

	case DSR_FUNC_PARENTDIR:
		D( kprintf("oldlock = %ld, ", Arg1); )

		if (Arg1 >= MAX_LOCKS || LockList[Arg1].lock == -1) {
			Arg1 = 0;
			dsrw->dsr_Err = DSR_ERR_FILE_NOT_OPEN;
		} else {
			for (x = 0; x < MAX_LOCKS; x++) {
				if (LockList[x].lock == -1) {
					Arg1 = ParentDir(LockList[Arg1].lock);
					err = IoErr();
					LockList[x].lock = Arg1;
					Arg1 = x;
					OpenLocks++;
					break;
				}
			}

			if (x == MAX_LOCKS) {
				Arg1 = 0;
				dsrw->dsr_Err = DSR_ERR_TOO_MANY_LOCKS;
			}
		}

		D( kprintf("newlock = %ld\n", Arg1); )

		break;

	case DSR_FUNC_RENAME:
		D( kprintf("file1 = %s, file2 = %s, ", &Buffer[0], &Buffer[Arg1]); )

		Arg1 = Rename(&Buffer[0], &Buffer[Arg1]);
		err = IoErr();

		D( kprintf("result = %ld\n", Arg1); )

		break;

	case DSR_FUNC_SETPROTECTION:
		D( kprintf("file = %s, bits = 0x%lx, ", Buffer, Arg1); )

		Arg1 = SetProtection(Buffer, Arg1);
		err = IoErr();

		D( kprintf("result = %ld\n", Arg1); )

		break;

	case DSR_FUNC_PARSEPATTERN:
		D( kprintf("pat = %s, ", &Buffer[0]); )

		if (v37) {
/*
			x = ParsePattern(&Buffer[0], &Buffer[Arg1], Arg2);
*/
		} else {
			x = xParsePattern(&Buffer[0], &Buffer[Arg1], Arg2);
		}

		D( kprintf("result = %ld, buf = %s\n", x, &Buffer[Arg1]); )

		Arg1 = x;

		break;

	case DSR_FUNC_MATCHPATTERN:
		D( kprintf("pat = %s, str = %s, ", &Buffer[0], &Buffer[Arg1]); )

		if (v37) {
/*
			x = MatchPattern(&Buffer[0], &Buffer[Arg1]);
*/
		} else {
			x = xMatchPattern(&Buffer[0], &Buffer[Arg1]);
		}

		D( kprintf("result = %ld\n", x); )

		Arg1 = x;

		break;

	case DSR_FUNC_SETFILEDATE:
		D( kprintf("file = %s, ds.days = %ld, ds.mins = %ld, ds.ticks = %ld\n", \
			    &Buffer[0], \
			    ((struct DateStamp *)(&Buffer[Arg1]))->ds_Days, \
			    ((struct DateStamp *)(&Buffer[Arg1]))->ds_Minute, \
			    ((struct DateStamp *)(&Buffer[Arg1]))->ds_Tick); )

		if (v37) {
/*
			Arg1 = SetFileDate(&Buffer[0], &Buffer[Arg1]);
*/
		} else {
			ULONG array[4];
			struct MsgPort *pid;
			char *s;
			struct DateStamp *d_s;
			int l;

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
						Arg1 = SendPacket(pid, 34 /* ACTION_SET_DATE */, array, 4);
					}
					free(d_s);
				}
				free(s);
			}
		}
		break;

	default:
		dsrw->dsr_Err = DSR_ERR_UNKNOWN_FUNCTION;
		break;
	}

	dsrw->dsr_Arg1_h = (Arg1 >> 16) & 0xffff;
	dsrw->dsr_Arg1_l = Arg1 & 0xffff;

	dsrw->dsr_Arg2_h = (Arg2 >> 16) & 0xffff;
	dsrw->dsr_Arg2_l = Arg2 & 0xffff;

	dsrw->dsr_Arg3_h = (Arg3 >> 16) & 0xffff;
	dsrw->dsr_Arg3_l = Arg3 & 0xffff;

	dsrw->dsr_Err_h = (err >> 16) & 0xffff;
	dsrw->dsr_Err_l = err & 0xffff;

	D( if (dsrw->dsr_Function >= sizeof(funcnames)/sizeof(char *)) )
	D(	kprintf("UNKNOWN (%ld): ", dsrw->dsr_Function); )
	D( else )
	D(	kprintf("%s: ", funcnames[dsrw->dsr_Function]); )
	D( kprintf("A1 = 0x%lx, A2 = 0x%lx, A3 = 0x%lx, ERR = %ld\n", Arg1, Arg2, Arg3, err); )
	D( if (dsrw->dsr_Err) )
	D(	kprintf(" *ERROR* %ld\n", dsrw->dsr_Err); )
	D( kprintf("\n"); )
}
