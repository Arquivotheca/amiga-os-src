/*****************************************************************************
*                                                                            *
*****************************************************************************/


 /*

   *************************** BUGS **************************
 
   Examine Next dosen't work correctly in two cases:
      1) multiple users of ExNext
      2) ExNexts separated by create or delete. 
    
   The use of "", /  (trailing and freestanding), is not yet implimented.

   The PC's "." and ".." show up as dirs.  I was planning to play around
   with them but at this point it looks as if I will just filter them out...

   There is no performance optimization.  The buffers are small and there
   is needless copying taking place.
	
   Not Reentrant. Most of the Globals are going to go away (there are some
   read-only tables 'n' stuff that will remain) 

   I use Parameter memory rather than Buffer memory.      

   Incomplete.    
 */

#include <libraries/dos.h> 
#include <exec/types.h>
#include <exec/memory.h>
#include <janus/janus.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include "dev.h"

struct Process		*DosProc;	/* Our Process				    */
struct DeviceNode 	*DosNode;	/* Our DOS node.. created by DOS for us	    */
struct DeviceList	*DevList;	/* Device List structure for our volume node   */

struct JanusBase 	*JanusBase;	/* Declare library base pointer          */
void			*SysBase;	/* EXEC library base			*/
extern struct DosLibrary *DOSBase;	/* DOS library base for debug process	*/
struct MinList	FHBase;     /*	Open Files				*/
struct MinList	LCBase;     /*	Open Locks				*/

short AmigaFileError[20];
short PCFileError[20];

short error_trans[]=
			
		{
		0,
		ERROR_NOT_A_DOS_DISK,
		ERROR_OBJECT_NOT_FOUND,
		ERROR_OBJECT_NOT_FOUND,
		ERROR_NO_FREE_STORE,
		ERROR_OBJECT_IN_USE,
		ERROR_INVALID_LOCK,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NO_FREE_STORE,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NOT_A_DOS_DISK,
		0,
		ERROR_NOT_A_DOS_DISK,
		ERROR_OBJECT_IN_USE,
		ERROR_NOT_A_DOS_DISK,
		ERROR_NO_MORE_ENTRIES,
		};

char *st_ptr;
char np_buf[512];

struct List *LOCKlist;	/* this will end up as a non-global, but for now... */
struct List *PChandl;

long	TotalBytes;	/* total bytes of data in filesystem */
long    signum   ;      /* This will be the signal for MSDosServ */
long	sigmask  ;      /* Make a mask for the Wait function    */

struct ServiceData *sdata;    /* Janus ServiceData pointer              */
struct PCDosServ *PClink;
struct PCDosServ *PCByte;

char PCSLASH[]  = {	'\134','\0'};
char PCSPEC[] = {	'\134','*','.','*','\0'};


void
mane()
{
	register struct DosPacket *packet;
	register short   error;
	struct Message     *msg;
	UBYTE   notdone;
	char *str_pt;
	char Vol_name[20]; /* this is too short and should come from a big chunk I dynamically allocate */
	short Vol_size;
	short path_size;
	void    *tmp;
	short state;
	short sw_state;
	struct PClock *testlock;
	short ex_next_flag;
	short examine_root;
	char *read_wr_buffer;
	long read_wr_length;
	struct PChandle *hand;



    TotalBytes = 0;
 	JanusBase = 0;
	SysBase = AbsExecBase;
    DOSBase = OpenLibrary("dos.library",0); 
    DosProc = FindTask(NULL);
	ServInit(); 

    {


	LOCKlist =(struct List *)AllocMem(sizeof(struct List),MEMF_PUBLIC);
	PChandl =(struct List *)AllocMem(sizeof(struct List),MEMF_PUBLIC);
	
	/* will free when I redo cleanup... */
	NewList(LOCKlist);
	NewList(PChandl);

	WaitPort(&DosProc->pr_MsgPort); 	/*  Get Startup Packet	*/
	msg = GetMsg(&DosProc->pr_MsgPort);
	packet = (struct DosPacket *)msg->mn_Node.ln_Name;

	/*
	 *  Loading DosNode->dn_Task causes DOS *NOT* to startup a new
	 *  instance of the device driver for every reference.	E.G. if
	 *  you were writing a CON device you would want this field to
	 *  be NULL.
	 */

	if (DOSBase) {
	    struct DosInfo *di = BADDR(((struct RootNode *)DOSBase->dl_Root)->rn_Info);
	    register struct DeviceList *dl = dosalloc(sizeof(struct DeviceList));

	    DosNode = BADDR(packet->dp_Arg3);

	    /*
	     *	Create Volume node and add to the device list.	This will
	     *	cause the WORKBENCH to recognize us as a disk.	If we don't
	     *	create a Volume node, Wb will not recognize us.  However,
	     *	we are a RAM: disk, Volume node or not.
	     */

	    DevList = dl;
	    dl->dl_Type = DLT_VOLUME;
	    dl->dl_Task = &DosProc->pr_MsgPort;
	    dl->dl_DiskType = ID_DOS_DISK;
	    dl->dl_Name = (void *)DosNode->dn_Name;
	    dl->dl_Next = di->di_DevInfo;
	    di->di_DevInfo = (long)CTOB(dl);
	
	btos(dl->dl_Name,Vol_name);	 /* save volume name */
	Vol_size = *((char *)BADDR(dl->dl_Name)); /* sizeof string */ 
	Vol_name[Vol_size++] = ':';


	    /*
	     *	Set dn_Task field which tells DOS not to startup a new
	     *	process on every reference.
	     */

	    DosNode->dn_Task = &DosProc->pr_MsgPort;
	    packet->dp_Res1 = DOS_TRUE;
	    packet->dp_Res2 = 0;
	} else {			    /*	couldn't open dos.library   */
	    packet->dp_Res1 = DOS_FALSE;
	    returnpacket(packet);
	    return;			    /*	exit process		    */
	}
	returnpacket(packet);
    }

	NewList(&LCBase);

	examine_root = FALSE;

    /*
     *	Here begins the endless loop, waiting for requests over our
     *	message port and executing them.  Since requests are sent over
     *	our message port, this precludes being able to call DOS functions
     *	ourselves
     */

top:
for (notdone = 1; notdone;)
	{
	WaitPort(&DosProc->pr_MsgPort);
	while (msg = GetMsg(&DosProc->pr_MsgPort)) 
		{
		packet = (struct DosPacket *)msg->mn_Node.ln_Name;
		packet->dp_Res1 = DOS_TRUE;
		packet->dp_Res2 = 0;
		error = 0;
		/* kprintf("Packet: %3ld %08lx %08lx %08lx %10s ",
			packet->dp_Type,
			packet->dp_Arg1, packet->dp_Arg2,
			packet->dp_Arg3,
			typetostr(packet->dp_Type)
		  );
		*/
	for(state=0,sw_state = packet->dp_Type;state != DONE;sw_state=state)
	    {	
	    switch(sw_state) 
		{
		case ACTION_DIE:	    /*	attempt to die? */
			ex_next_flag = FALSE;
			state = DONE;
			notdone = 0;	    /*	try to die  */
			break;
	    	case ACTION_OPENNEW:    /*	FileHandle,Lock,Name	    Bool    */
        		ex_next_flag = FALSE;
		    case ACTION_OPENRW:     /*	FileHandle,Lock,Name	    Bool    */
		    case ACTION_OPENOLD:    /*	FileHandle,Lock,Name	    Bool    */
			{
			if(!(hand = dosalloc(sizeof(struct PChandle)))) 
				{
				error = ERROR_NO_FREE_STORE;
				state = DONE;
				break;
				}
			if(sw_state == ACTION_OPENNEW)
				PClink->function = PC_CREAT; 
			else
				PClink->function = PC_OPEN; 
			PClink->access = 0;  /* for now... */
			PCByte->name[0] = 0;
			np_buf[0] = 0;

			btos(packet->dp_Arg3,np_buf);  /*  get name */
			str_pt = pervert_path(np_buf,Vol_name,Vol_size);	
			strcpy(PCByte->name,GetLockPath(BADDR(packet->dp_Arg2),&path_size));  /* get parent */

			if(*str_pt == ':') /*IF not root , prepend PC type slash */
				{
				if(*(str_pt+1) != 0)
					{
					strcat(PCByte->name,":\134");
					str_pt++;
					}
				}
			else
				strcat(PCByte->name,PCSLASH);
			strcat(PCByte->name,str_pt); 
			testlock = GetPClock(PCByte->name);
			if(testlock) /* previous locks? */
				{
				state = DONE;
				if ((long) testlock == -1 || testlock->number_of_locks < 0 || ((testlock->number_of_locks > 0)  && sw_state != ACTION_OPENOLD))
					{
					error = ERROR_OBJECT_IN_USE;
					dosfree(hand);
					break;
					}
				hand->lock = (syslock(testlock, 0)); /* no modes yet... */
				state = GOT_OPEN_LOCK;
				}			
			else
				{
				if((*(PCByte->name)) && ((PCByte->name)[1] != 0)) /* null or root */
					state = SEND;
				else
					{
					state = DONE;
					error = ERROR_OBJECT_NOT_FOUND; /* Null Name */
					dosfree(hand);
					break;
					}
				}
			}
			break;

		case a_OPEN:	
			state = DONE;
			if(PClink->error == NONE)
				{
				testlock = MakeDirLock(PCByte->name,(PClink->filehandle) ? TYPE_FILE : TYPE_DIR); /* no filehandle + no error = directory */
				hand->lock = (syslock(testlock, 0)); /* no modes yet... */
				state = GOT_OPEN_LOCK;
				}
			else
				error = translate(PClink->error);
			break;

		case GOT_OPEN_LOCK:
			{
			register struct PClock *PL;
		
			PL= (struct PClock *)(((struct FileLock *)(hand->lock))->fl_Key);
			state = DONE;
			if (PL->type == TYPE_DIR)
				{
			    	error = ERROR_OBJECT_WRONG_TYPE;
				/* free lock now */
				/* free hand */
			   	break;
				}
			if(PL->HandleAvailable)
				{
				PL->HandleAvailable = FALSE;
				hand->filehandle = PL->filehandle0;
				((struct FileHandle *)BADDR(packet->dp_Arg1))->fh_Arg1 = (long)hand;
				}
			else
				{
				state = SEND;
				PClink->function = PC_OPEN_NXT; 
				}
			break;
			}

		case OPEN_GT_ONE:

			state = DONE;
			if(PClink->error == NONE)
				{
				hand->filehandle = PClink->filehandle;
	 			((struct FileHandle *)BADDR(packet->dp_Arg1))->fh_Arg1 = (long)hand;
				}
			else
				error = translate(PClink->error);
			break;

		case ACTION_READ:	    /*	 FHArg1,CPTRBuffer,Length   ActLength  */
			{
			state = SEND;
			PClink->filehandle = ((struct PChandle *)(packet->dp_Arg1))->filehandle;
			read_wr_buffer = packet->dp_Arg2;
			read_wr_length = packet->dp_Arg3;
			packet->dp_Res1 = 0;
			PClink->function = PC_READ; 
			PClink->access = (read_wr_length > RWBUFFSIZ) ? RWBUFFSIZ : read_wr_length ;
			break;
			} 
		
		case a_ACTION_READ:
			{
			if(PClink->error != NONE)
				{
				state = DONE;
				error = translate(PClink->error);
				break;
				}
			CopyMem(PCByte->name,read_wr_buffer,PClink->access);

			packet->dp_Res1 += PClink->access;
			read_wr_buffer += PClink->access;
			if((packet->dp_Res1 == read_wr_length) || (PClink->access != RWBUFFSIZ)) 
				{
				state = DONE;
				break;
				}
			state = SEND;
			PClink->function = PC_READ; 
			PClink->access = ((read_wr_length - packet->dp_Res1) > RWBUFFSIZ) ? RWBUFFSIZ : (read_wr_length - packet->dp_Res1) ;
			break;
			}



		case ACTION_WRITE:	    /*	 FHArg1,CPTRBuffer,Length   ActLength  */
			{
			state = SEND;
			PClink->filehandle = ((struct PChandle *)(packet->dp_Arg1))->filehandle;
			read_wr_buffer = packet->dp_Arg2;
			read_wr_length = packet->dp_Arg3;
			packet->dp_Res1 = 0;
			PClink->function = PC_WRITE; 
			PClink->access = (read_wr_length > RWBUFFSIZ) ? RWBUFFSIZ : read_wr_length ;
			CopyMem(read_wr_buffer,PCByte->name,PClink->access);
			}
			break;

		case a_ACTION_WRITE:
			{
			if(PClink->error != NONE)
				{
				state = DONE;
				error = translate(PClink->error);
				break;
				}
/*			strncpy(PCByte->name,read_wr_buffer,PClink->access); */
			packet->dp_Res1 += PClink->access;
			read_wr_buffer += PClink->access;

			if((packet->dp_Res1 == read_wr_length) || (PClink->access != RWBUFFSIZ)) 
				{
				state = DONE;
				break;
				}
			state = SEND;
			PClink->function = PC_WRITE; 
			PClink->access = ((read_wr_length - packet->dp_Res1) > RWBUFFSIZ) ? RWBUFFSIZ : (read_wr_length - packet->dp_Res1) ;
			CopyMem(read_wr_buffer,PCByte->name,PClink->access);
			break;
			}

		case ACTION_CLOSE:	    /*	 FHArg1 		    Bool:TRUE  */
		/*
		    seek back to zero but don't close file.  Only freesyslock can
		    close files 
		*/
			state = SEND;
			PClink->filehandle = ((struct PChandle *)(packet->dp_Arg1))->filehandle;
			PClink->function = PC_AJAR;   
			break;

		case a_ACTION_CLOSE:
			{
			register struct PClock *PL;
			register struct FileLock *FL;

			state = DONE;
			FL = (struct FileLock *) ((struct PChandle *)(packet->dp_Arg1))->lock;
			PL = (struct PClock *) FL->fl_Key;
			if(((struct PChandle *)packet->dp_Arg1)->filehandle == PL->filehandle0 ) /* first open of file ? */
				{
				PL->HandleAvailable = TRUE;
				}
			else
				{
				PClink->function = PC_CLOSE; 		
				PClink->filehandle = ((struct PChandle *)packet->dp_Arg1)->filehandle;
				CallService(sdata);
				Wait(sigmask);
				/* free up 2nd+ open */
				}
			freesyslock(FL);		
			}
		break;

		case ACTION_SEEK:	    /*	 FHArg1,Position,Mode	    OldPosition*/
			{
			state = SEND;
			PClink->filehandle = ((struct PChandle *)(packet->dp_Arg1))->filehandle;
			PClink->offset = packet->dp_Arg2;
			PClink->function = PC_SEEK; 
			PClink->access = packet->dp_Arg3;
			if(PClink->access == 0)
				PClink->access =  1;
			else
				if(PClink->access == 1)
					PClink->access = 2;
			if((PClink->access < 0) || (PClink->access > 2))
				PClink->access = 0;
			}
			break;

		case a_ACTION_SEEK:
			state = DONE;
			if(PClink->error)
				{
				error = ERROR_SEEK_ERROR;
				packet->dp_Res1 = -1;
				break;
				}			
			packet->dp_Res1 = PClink->offset;
			break;	


		break;

		case ACTION_EXAMINE_NEXT: /*   Lock,Fib		      Bool	 */
			{
	 		state = DONE;
			if (((struct PClock *) ((struct FileLock *) (BADDR(packet->dp_Arg1)))->fl_Key)->type == TYPE_FILE) 
				{
				error = ERROR_OBJECT_WRONG_TYPE;
				break;
				}
			if(ex_next_flag == TRUE)
				{
				PClink->function = PC_EXNEXT;
	 			state = SEND;
				}
			else
				{
				error =	ERROR_NO_MORE_ENTRIES;
				break; 
				}
			if(!examine_root)	/* root is a special case, we do the first real examine on first exnext */ 
				break;
			}
		/*  fall through    */
		case ACTION_EXAMINE_OBJECT: /*   Lock,Fib			Bool	   */
			{
		   	 register struct FileInfoBlock *fib;

			np_buf[0] = 0;
			fib = BADDR(packet->dp_Arg2);
			PClink->function = PC_EXAMINE; 
			PClink->access = 0;  /* for now... */
			ex_next_flag = TRUE;
			examine_root = FALSE;
			state = SEND;
			str_pt = GetLockPath(BADDR(packet->dp_Arg1),&path_size);
			if(str_pt)
				strcpy(np_buf,str_pt);
			str_pt = np_buf;
			fib->fib_Date.ds_Days = 10;
			fib->fib_Date.ds_Minute = 20;
			fib->fib_Date.ds_Tick = 30;
/*			fib->fib_Protection = 0; */

			if ((((struct PClock *) ((struct FileLock *) (BADDR(packet->dp_Arg1)))->fl_Key)->type == TYPE_DIR) && (sw_state == ACTION_EXAMINE_OBJECT)) 
				{
				state = DONE;
				for(str_pt += path_size; (str_pt != np_buf) && (*str_pt != '\134'); str_pt--);
				str_pt++; /* dump slash */
				path_size -= str_pt - np_buf;

				fib->fib_DiskKey = 0;
				fib->fib_DirEntryType = TYPE_DIR; 
				strcpy(&(fib->fib_FileName[1]),str_pt);
				fib->fib_FileName[0] = path_size; 
				fib->fib_EntryType = TYPE_DIR;
				fib->fib_Size = 0;
				fib->fib_NumBlocks = 0;
				fib->fib_Comment[0] = 0;
				examine_root = TRUE;
				break;
				}
						
			if (((struct PClock *) ((struct FileLock *) (BADDR(packet->dp_Arg1)))->fl_Key)->type == TYPE_DIR) 
				strcat(str_pt,PCSPEC); /* search all dir entries */
			strcpy(PCByte->name,str_pt); 
			}
			break;

		case a_ACTION_EXAMINE_OBJECT: 
			{
			register char *cpoint;
			register struct FileInfoBlock *fib= BADDR(packet->dp_Arg2);
			str_pt =PCByte->Xfer;
			str_pt +=30;

			if(PClink->error == NONE)
				{
				fib->fib_DiskKey = 0;
				fib->fib_DirEntryType = (*((PCByte->Xfer)+21) & 0x10) ?  TYPE_DIR : TYPE_FILE;
				strcpy(fib->fib_FileName+1,str_pt);
				fib->fib_FileName[0] = strlen(str_pt);
				fib->fib_EntryType = (*((PCByte->Xfer)+21) & 0x10) ?  TYPE_DIR : TYPE_FILE;
				cpoint = (PCByte->Xfer) + 26;
				fib->fib_Size = *cpoint + (256 * *(cpoint + 1)) +(65536 * *(cpoint + 2)) +(16777216 * *(cpoint + 3));  
				fib->fib_NumBlocks = (fib->fib_Size/512) + (fib->fib_Size % 512) ? 1 : 0;
				fib->fib_Comment[0] = 0;
				}
			else
				{
				error = translate(PClink->error);
				fib->fib_DiskKey = 1;
				}
			state = DONE;
			}
			break;
	
		case ACTION_INFO:	    /*	Lock, InfoData	  Bool:TRUE    */
			state = DONE;
			tmp = BADDR(packet->dp_Arg2);
			error = -1;
		/*  fall through    */
		case ACTION_DISK_INFO:  /*	InfoData	  Bool:TRUE    */
			{
		   	 register struct InfoData *id;
		/* This section is a complete rip off of MD's code and
		   as such does nothing useful in this program.  
		*/ 

		    /*
		     *	Note:	id_NumBlocks is never 0, but only to get
		     *	around a bug I found in my shell (where I divide
		     *	by id_NumBlocks).  Other programs probably break
		     *	as well.
		     */
			state = DONE;
			(error) ? (id = tmp) : (id = BADDR(packet->dp_Arg1));
			error = 0;
			id->id_DiskState = ID_VALIDATED;
			id->id_NumBlocks	 = (TotalBytes >> 9) + 1;
			id->id_NumBlocksUsed = (TotalBytes >> 9) + 1;
			id->id_BytesPerBlock = 512;
			id->id_DiskType = ID_DOS_DISK;
			id->id_VolumeNode = (long)CTOB(DosNode);
			id->id_InUse = 1234567L;
			}
			break;

		case ACTION_PARENT:     /*	 Lock			    ParentLock */
			{
			state = DONE;

			PClink->function = PC_PARENT; 
			PClink->access = 0;  /* for now... */
			np_buf[0] = 0;
			PCByte->name[0] = 0;
			strcpy((PCByte->name),GetLockPath(BADDR(packet->dp_Arg1),&path_size));  /* get parent */
			if (((*PCByte->name) == ':') && (PCByte->name[1] == 0)) /* at root ? */
					{
					error = ERROR_OBJECT_NOT_FOUND;
					break;
					}
			for(str_pt = (PCByte->name) + path_size; (str_pt != PCByte->name) && (*str_pt != '\134'); str_pt--)
				*str_pt = 0; 
			*str_pt = 0; /* get the slash... */
			testlock = GetPClock(PCByte->name);
			if(((*PCByte->name) == ':') && (PCByte->name[1] == 0)  && (!testlock)) /* need new root lock? */
				{
				testlock = MakeDirLock(PCByte->name,TYPE_DIR);
				packet->dp_Res1 =  (long)CTOB(syslock(testlock, 0));
				state = DONE;
				break;
				}
			if(testlock) /* previous locks? */
				{
				state = DONE;
				if ((long) testlock == -1 || testlock->number_of_locks < 0 || (testlock->type != TYPE_DIR))
					{
					error = ERROR_OBJECT_IN_USE;
					break;
					}
				packet->dp_Res1 = (long)CTOB(syslock(testlock, 0));
				}			
			else
				state = SEND;
			}
			break;

		case a_PARENT:	
			{
			if(PClink->error == NONE)
				{
				testlock = MakeDirLock(PCByte->name,TYPE_DIR); /* no filehandle + no error = directory */
				packet->dp_Res1 =  (long)CTOB(syslock(testlock, 0));
				}
			else
				error = translate(PClink->error);
			state = DONE;
			break;
			}


		case ACTION_DELETE_OBJECT: /*Lock,Name		    Bool       */
			ex_next_flag = FALSE;
			state = DONE;
			break;

		case ACTION_CREATE_DIR: /*	 Lock,Name		    Lock       */
			ex_next_flag = FALSE;
			state = DONE;
			break;

		case ACTION_LOCATE_OBJECT:	/*   Lock,Name,Mode		Lock	   */
			{
			PClink->function = PC_LOCK; 
			PClink->access = 0;  /* for now... */
			np_buf[0] = 0;
			PCByte->name[0] = 0;
			btos(packet->dp_Arg2,np_buf);  /*  get name */
			str_pt = pervert_path(np_buf,Vol_name,Vol_size);	
			strcpy((PCByte->name),GetLockPath(BADDR(packet->dp_Arg1),&path_size));  /* get parent */
			if(*str_pt == ':') /*IF not root , prepend PC type slash */
				{
				if(*(str_pt+1) != 0)
					{
					strcat(PCByte->name,":\134");
					str_pt++;
					}
				}
			else
				strcat(PCByte->name,PCSLASH);
			strcat(PCByte->name,str_pt); 
			testlock = GetPClock(PCByte->name);
			if(testlock) /* previous locks? */
				{
				state = DONE;
				if ((long) testlock == -1 || testlock->number_of_locks < 0 || ((testlock->number_of_locks > 0 ) && packet->dp_Arg3 == ACCESS_WRITE))
					{
					error = ERROR_OBJECT_IN_USE;
					break;
					}
				packet->dp_Res1 = (long)CTOB(syslock(testlock, packet->dp_Arg3));
				}			
			else
				{
				if((PCByte->name)[1]  == 0) /* bare ":" */
					{
					state = DONE;
					testlock = MakeDirLock(PCByte->name,TYPE_DIR);
					packet->dp_Res1 =  (long)CTOB(syslock(testlock, packet->dp_Arg3));
					}
				else 
					state = SEND;
				}
			}
			break;
    
		case a_ACTION_LOCATE_OBJECT:	
			{
			if(PClink->error == NONE)
				{
				testlock = MakeDirLock(PCByte->name,(PClink->filehandle) ? TYPE_FILE : TYPE_DIR); /* no filehandle + no error = directory */
				packet->dp_Res1 =  (long)CTOB(syslock(testlock, packet->dp_Arg3));
				}
			else
				error = translate(PClink->error);
			state = DONE;
			break;
			}
		
		case ACTION_COPY_DIR:   /*	 Lock,			    Lock       */
			{
			register struct FileLock *FL;

			FL = (struct FileLock *)(BADDR(packet->dp_Arg1)); 

			if(((struct PClock *)FL->fl_Key)->number_of_locks < 0) /* exclusive ? */
				error = ERROR_OBJECT_IN_USE;
			else
				packet->dp_Res1 = (long)CTOB(syslock(((struct PClock *)FL->fl_Key), FL->fl_Access));
			state = DONE;
			}
			break;

		case ACTION_FREE_LOCK:  /*	 Lock,			    Bool       */
			state = DONE;
			if (packet->dp_Arg1);
				freesyslock(BADDR(packet->dp_Arg1));
			notdone = 0;
			break;

		case ACTION_SET_PROTECT:/*	 -,Lock,Name,Mask	   Bool       */
			state = DONE;
			break;
		case ACTION_SET_COMMENT:/*	 -,Lock,Name,Comment	   Bool       */
			state = DONE;
			break;
		case ACTION_RENAME_OBJECT:/* SLock,SName,DLock,DName    Bool       */
			ex_next_flag =  FALSE;
			state = DONE;
			break;

	    /*
	     *	A few other packet types which we do not support
	     */
	    case ACTION_INHIBIT:    /*	 Bool			    Bool       */
		/*  Return success for the hell of it	*/
		state = DONE;
		break;
	    case ACTION_RENAME_DISK:/*	 BSTR:NewName		    Bool       */
	    case ACTION_MORECACHE:  /*	 #BufsToAdd		    Bool       */
	    case ACTION_WAIT_CHAR:  /*	 Timeout, ticks 	    Bool       */
	    case ACTION_FLUSH:	    /*	 writeout bufs, disk motor off	       */
	    case ACTION_RAWMODE:    /*	 Bool(-1:RAW 0:CON)	    OldState   */

		state = DONE;


	    default:
		error = ERROR_ACTION_NOT_KNOWN;
		state = DONE;

		break;
	    }

		if(state == SEND)
			{
	  		CallService(sdata);
			Wait(sigmask);
			state = PClink->function;
			continue;
			}
	
	   }

	    if (packet) {
		if (error) {
		    /* kprintf("!!!!!!!!!!!!!!!!!ERR=%ld\n", error); */
		    packet->dp_Res1 = DOS_FALSE;
		    packet->dp_Res2 = error;
		} else {
		    /* kprintf("RES=%06lx\n", packet->dp_Res1); */
		}
		returnpacket(packet);



	    }
	}
    }

/*    Forbid();
    if (packetsqueued(DosProc) || GetHead(&FHBase) || GetHead(&LCBase)
      || GetHead(&RFRoot.list))
	 {
	Permit();
 */
	goto top;		/*  sorry... can't exit    /
	   }
*/
    /*
     *	Causes a new process to be created on next reference
     */

    DosNode->dn_Task = FALSE;

    /*
     *	Remove Volume entry.  Since DOS uses singly linked lists, we
     *	must (ugg) search it manually to find the link before our
     *	Volume entry.
     */

    {
	struct DosInfo *di = BADDR(((struct RootNode *)DOSBase->dl_Root)->rn_Info);
	register struct DeviceList *dl;
	register void *dlp;

	dlp = &di->di_DevInfo;
	for (dl = BADDR(di->di_DevInfo); dl && dl != DevList; dl = BADDR(dl->dl_Next))
	    dlp = &dl->dl_Next;
	if (dl == DevList) {
	    *(BPTR *)dlp = dl->dl_Next;
	    dosfree(dl);
	} else {
	    /* kprintf("****PANIC: Unable to find volume node\n"); */
	}
    }



    CloseLibrary(DOSBase);
}


/*
 *  PACKET ROUTINES.	Dos Packets are in a rather strange format as you
 *  can see by this and how the PACKET structure is extracted in the
 *  GetMsg() of the main routine.
 */

void
returnpacket(packet)
register struct DosPacket *packet;
{
    register struct Message *mess;
    register struct MsgPort *replyport;

    replyport		     = packet->dp_Port;
    mess		     = packet->dp_Link;
    packet->dp_Port	     = &DosProc->pr_MsgPort;
    mess->mn_Node.ln_Name    = (char *)packet;
    mess->mn_Node.ln_Succ    = NULL;
    mess->mn_Node.ln_Pred    = NULL;
    PutMsg(replyport, mess);
}

/*
 *  Are there any packets queued to our device?
 */

packetsqueued()
{
    return ((void *)DosProc->pr_MsgPort.mp_MsgList.lh_Head !=
	    (void *)&DosProc->pr_MsgPort.mp_MsgList.lh_Tail);
}

/*
 *  DOS MEMORY ROUTINES
 *
 *  DOS makes certain assumptions about LOCKS.	A lock must minimally be
 *  a FileLock structure, with additional private information after the
 *  FileLock structure.  The longword before the beginning of the structure
 *  must contain the length of structure + 4.
 *
 *  NOTE!!!!! The workbench does not follow the rules and assumes it can
 *  copy lock structures.  This means that if you want to be workbench
 *  compatible, your lock structures must be EXACTLY sizeof(struct FileLock).
 */

void *
dosalloc(bytes)
register ULONG bytes;
{
    register ULONG *ptr;

    bytes += 4;
    ptr = AllocMem(bytes, MEMF_PUBLIC|MEMF_CLEAR);
    *ptr = bytes;
    return(ptr+1);
}

dosfree(ptr)
register ULONG *ptr;
{
    --ptr;
    FreeMem(ptr, *ptr);
}

/*
 *  Convert a BSTR into a normal string.. copying the string into buf.
 *  I use normal strings for internal storage, and convert back and forth
 *  when required.
 */

void
btos(bstr,buf)
UBYTE *bstr;
UBYTE *buf;
{
    bstr = BADDR(bstr);
    CopyMem(bstr+1,buf,*bstr);
    buf[*bstr] = 0;
}


/************************************************************************
 *                
 * ServInit -- Init services-related stuff
 *
 *************************************************************************/
ServInit()
	{
   /*
    * Open Janus.Library
    */

	if(! (JanusBase = (struct JanusBase *) OpenLibrary(JANUSNAME,0)))
		eexit();
 
   /*
    * Allocate a signal bit
    */
	signum   = -1;         
	sigmask  = 0;          
	if(signum = AllocSignal(-1))
		eexit();
   /*
    * Create a signal mask for Wait();
    */
	sigmask = 1 << signum;

   /*
    * Add new service;
    * allocate  Parameter Memory
    */


	if(JSERV_OK !=(AddService(&sdata,TIMESERV_APPLICATION_ID,TIMESERV_LOCAL_ID,
	sizeof(struct PCDosServ), MEMF_PARAMETER|MEM_WORDACCESS,signum,0)))
		eexit();

	PClink = (struct PCDosServ *)sdata->sd_AmigaMemPtr;
	PCByte = (struct PCDosServ *)MakeBytePtr(PClink); 
	PClink = (struct PCDosServ *)MakeWordPtr(PCByte);

	}


struct PClock *GetPClock(path)
	char *path;
	{
	return(FindName(LOCKlist,path));
	}
		


char *GetLockPath(lock_key, path_size)
	struct FileLock	 *lock_key;
	short *path_size;
	{
	struct PClock *key;
	if(lock_key)
		{
		key = (struct PClock *) lock_key->fl_Key;
		*path_size =  key->path_length;
		return(key->node.ln_Name);
		}
	else
		 return(NULL);	

	}

struct PClock *MakeDirLock(path,type)
	char *path;
	short type;
	{
	struct PClock *newlock;
	int pathsize;

	 
	pathsize = strlen(path);	
	if(newlock = dosalloc(sizeof(struct PClock)+pathsize+1, MEMF_PUBLIC|MEMF_CLEAR)) 
		{
		newlock->node.ln_Name = &newlock[1]; /* string space immediatly follows node struct */
		strcpy(newlock->node.ln_Name,path); /* get path */
		newlock->path_length = pathsize;
		newlock->type = type;
		newlock->number_of_locks = 0;
		newlock->filehandle0 = PClink->filehandle;
		newlock->HandleAvailable = TRUE;
		AddHead(LOCKlist,newlock); 
	
		return(newlock);
		}
	else
		return(TRUE);	/* failed */
	}

struct FileLock *syslock(peeceelock, mode)
struct PClock *peeceelock;
int mode;
{
    struct FileLock *lock;
    LOCKLINK *ln;
	ln = AllocMem(sizeof(LOCKLINK), MEMF_PUBLIC);
	lock =  dosalloc(sizeof(struct FileLock));

	if(ln && lock)
		{
		AddHead(&LCBase,ln);
		ln->lock = lock;
		lock->fl_Link= (long)ln;
		lock->fl_Key = (long)peeceelock;
		lock->fl_Access = mode;
		lock->fl_Task = &DosProc->pr_MsgPort;
		lock->fl_Volume = (BPTR)CTOB(DosNode);
		if (mode != ACCESS_WRITE)
			++peeceelock->number_of_locks;
		else
			peeceelock->number_of_locks = -1; /* exclusive */
		return(lock);
		}	
	else
		{
		if(ln)
			FreeMem(ln,sizeof(LOCKLINK));
		if(lock)
			dosfree(lock);
		return((struct FileLock *) -1);
		}
}

freesyslock(lock)
    	struct FileLock *lock;
	{
	register struct PClock *PL;
		
	PL= (struct PClock *)lock->fl_Key;
	PL->number_of_locks--;
	Remove(lock->fl_Link);			/* unlink from list */
	FreeMem(lock->fl_Link, sizeof(LOCKLINK));	/* free link node   */
	dosfree(lock);
	if(!(PL->number_of_locks > 0))  /* if < 0 (exclusive lock) or 0 (last lock freed) then free PClock */
		{
		PClink->function = PC_CLOSE; 		
		PClink->filehandle = PL->filehandle0;
		if(PL->filehandle0)	/* dir locks will have 0 filehandle which we don't want to free*/
			{
			CallService(sdata);
			Wait(sigmask);
			}
		Remove(PL);
		dosfree(PL);
		}


	}

short translate(error1)
	short error1;
	{
	return(error_trans[error1]);	
	}

/*
 *	eexit: cleanup -- This will go away in real version
 */
 eexit()
	{
	if(sdata)
		DeleteService(sdata);
	}
/*
 *
 * Convert  Amiga path/name to PC path/name
 * wildcards are not converted, only the "/" separator
 * Volume name or ":" is stripped out 
 * String converted to upper case
 *
 */

char *pervert_path(Amiga_path,volume_name,name_size)
	char *Amiga_path;
	char *volume_name;
	short name_size;
	{

	char *path;
	
	if(! *Amiga_path)	/* if Null string then run away */
		return(Amiga_path);
	if(!strnicmp(volume_name,Amiga_path,name_size)) /* volume name present? */
		Amiga_path += name_size-1; 
	path = strupr(Amiga_path);

	do
		{
		if(*Amiga_path == '/')
			*Amiga_path = '\134';
		}
	while(*Amiga_path++);
	return(path);
	}


/* Matt's debug stuff... 

char *
typetostr(ty)
{
    switch(ty) {
    case ACTION_DIE:		return("DIE");
    case ACTION_OPENRW: 	return("OPEN-RW");
    case ACTION_OPENOLD:	return("OPEN-OLD");
    case ACTION_OPENNEW:	return("OPEN-NEW");
    case ACTION_READ:		return("READ");
    case ACTION_WRITE:		return("WRITE");
    case ACTION_CLOSE:		return("CLOSE");
    case ACTION_SEEK:		return("SEEK");
    case ACTION_EXAMINE_NEXT:	return("EXAMINE NEXT");
    case ACTION_EXAMINE_OBJECT: return("EXAMINE OBJ");
    case ACTION_INFO:		return("INFO");
    case ACTION_DISK_INFO:	return("DISK INFO");
    case ACTION_PARENT: 	return("PARENTDIR");
    case ACTION_DELETE_OBJECT:	return("DELETE");
    case ACTION_CREATE_DIR:	return("CREATEDIR");
    case ACTION_LOCATE_OBJECT:	return("LOCK");
    case ACTION_COPY_DIR:	return("DUPLOCK");
    case ACTION_FREE_LOCK:	return("FREELOCK");
    case ACTION_SET_PROTECT:	return("SETPROTECT");
    case ACTION_SET_COMMENT:	return("SETCOMMENT");
    case ACTION_RENAME_OBJECT:	return("RENAME");
    case ACTION_INHIBIT:	return("INHIBIT");
    case ACTION_RENAME_DISK:	return("RENAME DISK");
    case ACTION_MORECACHE:	return("MORE CACHE"
    case ACTION_WAIT_CHAR:	return("WAIT FOR CHAR");
    case ACTION_FLUSH:		return("FLUSH");
    case ACTION_RAWMODE:	return("RAWMODE");
    default:			return("---------UNKNOWN-------");
    }
}

*/
