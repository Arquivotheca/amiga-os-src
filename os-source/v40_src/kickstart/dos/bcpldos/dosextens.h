#ifndef LIBRARIES_DOSEXTENS_H
#define LIBRARIES_DOSEXTENS_H
/*
**	$Filename: libraries/dosextens.h $
**	$Release: 1.3 $
**
**	DOS structures not needed for the casual AmigaDOS user 
**
**	(C) Copyright 1985,1986,1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif
#ifndef EXEC_TASKS_H
#include "exec/tasks.h"
#endif
#ifndef EXEC_PORTS_H
#include "exec/ports.h"
#endif
#ifndef EXEC_LIBRARIES_H
#include "exec/libraries.h"
#endif

#ifndef LIBRARIES_DOS_H
#include "libraries/dos.h"
#endif

/* All DOS processes have this structure */
/* Create and Device Proc returns pointer to the MsgPort in this structure */
/* dev_proc = (struct Process *) (DeviceProc(..) - sizeof(struct Task)); */

struct Process {
    struct  Task    pr_Task;	     
    struct  MsgPort pr_MsgPort; /* This is BPTR address from DOS functions  */
    WORD    pr_Pad;		/* Remaining variables on 4 byte boundaries */
    BPTR    pr_SegList;		/* Array of seg lists used by this process  */
    LONG    pr_StackSize;	/* Size of process stack in bytes	    */
    APTR    pr_GlobVec;		/* Global vector for this process (BCPL)    */
    LONG    pr_TaskNum;		/* CLI task number of zero if not a CLI	    */
    BPTR    pr_StackBase;	/* Ptr to high memory end of process stack  */
    LONG    pr_Result2;		/* Value of secondary result from last call */
    BPTR    pr_CurrentDir;	/* Lock associated with current directory   */
    BPTR    pr_CIS;		/* Current CLI Input Stream		    */
    BPTR    pr_COS;		/* Current CLI Output Stream		    */
    APTR    pr_ConsoleTask;	/* Console handler process for current window*/
    APTR    pr_FileSystemTask;	/* File handler process for current drive   */
    BPTR    pr_CLI;		/* pointer to ConsoleLineInterpreter	    */
    APTR    pr_ReturnAddr;	/* pointer to previous stack frame	    */
    APTR    pr_PktWait;		/* Function to be called when awaiting msg  */
    APTR    pr_WindowPtr;	/* Window for error printing */
};  /* Process */

/* The long word address (BPTR) of this structure is returned by
 * Open() and other routines that return a file.  You need only worry
 * about this struct to do async io's via PutMsg() instead of
 * standard file system calls */

struct FileHandle {
   struct Message *fh_Link;	 /* EXEC message	      */   
   struct MsgPort *fh_Port;	 /* Reply port for the packet */
   struct MsgPort *fh_Type;	 /* Port to do PutMsg() to  
				  * Address is negative if a plain file */
   LONG fh_Buf;
   LONG fh_Pos;
   LONG fh_End;
   LONG fh_Funcs;
#define fh_Func1 fh_Funcs
   LONG fh_Func2;
   LONG fh_Func3;
   LONG fh_Args;
#define fh_Arg1 fh_Args
   LONG fh_Arg2;
}; /* FileHandle */

/* This is the extension to EXEC Messages used by DOS */

struct DosPacket {
   struct Message *dp_Link;	 /* EXEC message	      */
   struct MsgPort *dp_Port;	 /* Reply port for the packet */
				 /* Must be filled in each send. */
   LONG dp_Type;		 /* See ACTION_... below and 
				  * 'R' means Read, 'W' means Write to the
				  * file system */
   LONG dp_Res1;		 /* For file system calls this is the result
				  * that would have been returned by the
				  * function, e.g. Write ('W') returns actual
				  * length written */
   LONG dp_Res2;		 /* For file system calls this is what would
				  * have been returned by IoErr() */
/*  Device packets common equivalents */
#define dp_Action  dp_Type
#define dp_Status  dp_Res1
#define dp_Status2 dp_Res2
#define dp_BufAddr dp_Arg1
   LONG dp_Arg1;		    
   LONG dp_Arg2;
   LONG dp_Arg3;
   LONG dp_Arg4;
   LONG dp_Arg5;
   LONG dp_Arg6;
   LONG dp_Arg7;
}; /* DosPacket */

/* A Packet does not require the Message to be before it in memory, but
 * for convenience it is useful to associate the two. 
 * Also see the function init_std_pkt for initializing this structure */

struct StandardPacket {
   struct Message   sp_Msg;
   struct DosPacket sp_Pkt;
}; /* StandardPacket */

/* Packet types */
#define ACTION_NIL		0
#define ACTION_GET_BLOCK	2	/* OBSOLETE */
#define ACTION_SET_MAP		4
#define ACTION_DIE		5
#define ACTION_EVENT		6
#define ACTION_CURRENT_VOLUME	7
#define ACTION_LOCATE_OBJECT	8
#define ACTION_RENAME_DISK	9
#define ACTION_WRITE		'W'
#define ACTION_READ		'R'
#define ACTION_FREE_LOCK	15
#define ACTION_DELETE_OBJECT	16
#define ACTION_RENAME_OBJECT	17
#define ACTION_MORE_CACHE	18
#define ACTION_COPY_DIR		19
#define ACTION_WAIT_CHAR	20
#define ACTION_SET_PROTECT	21
#define ACTION_CREATE_DIR	22
#define ACTION_EXAMINE_OBJECT	23
#define ACTION_EXAMINE_NEXT	24
#define ACTION_DISK_INFO	25
#define ACTION_INFO		26
#define ACTION_FLUSH		27
#define ACTION_SET_COMMENT	28  
#define ACTION_PARENT		29
#define ACTION_TIMER		30
#define ACTION_INHIBIT		31
#define ACTION_DISK_TYPE	32
#define ACTION_DISK_CHANGE	33
#define ACTION_SET_DATE		34

#define ACTION_SCREEN_MODE	994

#define ACTION_READ_RETURN	1001
#define ACTION_WRITE_RETURN	1002
#define ACTION_SEEK		1008
#define ACTION_FINDUPDATE	1004
#define ACTION_FINDINPUT	1005
#define ACTION_FINDOUTPUT	1006
#define ACTION_END		1007
#define ACTION_TRUNCATE		1022	/* fast file system only */
#define ACTION_WRITE_PROTECT	1023	/* fast file system only */

/* DOS library node structure.
 * This is the data at positive offsets from the library node.
 * Negative offsets from the node is the jump table to DOS functions  
 * node = (struct DosLibrary *) OpenLibrary( "dos.library" .. )	     */
   
struct DosLibrary {
    struct  Library dl_lib;
    APTR    dl_Root;	      /* Pointer to RootNode, described below */
    APTR    dl_GV;	      /* Pointer to BCPL global vector	      */
    LONG    dl_A2;	      /* Private register dump of DOS	      */
    LONG    dl_A5;
    LONG    dl_A6;
};  /*	DosLibrary */

/*			       */

struct RootNode {
    BPTR    rn_TaskArray;	     /* [0] is max number of CLI's
				      * [1] is APTR to process id of CLI 1
				      * [n] is APTR to process id of CLI n */
    BPTR    rn_ConsoleSegment; /* SegList for the CLI			   */ 
    struct  DateStamp rn_Time; /* Current time				   */
    LONG    rn_RestartSeg;     /* SegList for the disk validator process   */
    BPTR    rn_Info;	       /* Pointer ot the Info structure		   */
    BPTR    rn_FileHandlerSegment; /* segment for a file handler	   */
};  /* RootNode */

struct DosInfo {
    BPTR    di_McName;	       /* Network name of this machine; currently 0 */
    BPTR    di_DevInfo;	       /* Device List				    */
    BPTR    di_Devices;	       /* Currently zero			    */
    BPTR    di_Handlers;       /* Currently zero			    */
    APTR    di_NetHand;	       /* Network handler processid; currently zero */
};  /* DosInfo */

/* DOS Processes started from the CLI via RUN or NEWCLI have this additional
 * set to data associated with them */

struct CommandLineInterface {
    LONG   cli_Result2;	       /* Value of IoErr from last command	  */  
    BSTR   cli_SetName;	       /* Name of current directory		  */
    BPTR   cli_CommandDir;     /* Lock associated with command directory  */
    LONG   cli_ReturnCode;     /* Return code from last command		  */
    BSTR   cli_CommandName;    /* Name of current command		  */
    LONG   cli_FailLevel;      /* Fail level (set by FAILAT)		  */
    BSTR   cli_Prompt;	       /* Current prompt (set by PROMPT)	  */
    BPTR   cli_StandardInput;  /* Default (terminal) CLI input		  */
    BPTR   cli_CurrentInput;   /* Current CLI input			  */
    BSTR   cli_CommandFile;    /* Name of EXECUTE command file		  */
    LONG   cli_Interactive;    /* Boolean; True if prompts required	  */
    LONG   cli_Background;     /* Boolean; True if CLI created by RUN	  */
    BPTR   cli_CurrentOutput;  /* Current CLI output			  */
    LONG   cli_DefaultStack;   /* Stack size to be obtained in long words */
    BPTR   cli_StandardOutput; /* Default (terminal) CLI output		  */
    BPTR   cli_Module;	       /* SegList of currently loaded command	  */
};  /* CommandLineInterface */

/* This structure can take on different values depending on whether it is
 * a device, an assigned directory, or a volume.  Below is the structure
 * reflecting volumes only.  Following that is the structure representing
 * only devices. Following that is the unioned structure representing all 
 * the values
 */

/* structure representing a volume */

struct DeviceList {
    BPTR		dl_Next;	/* bptr to next device list */
    LONG		dl_Type;	/* see DLT below */
    struct MsgPort *	dl_Task;	/* ptr to handler task */
    BPTR		dl_Lock;	/* not for volumes */
    struct DateStamp	dl_VolumeDate;	/* creation date */
    BPTR		dl_LockList;	/* outstanding locks */
    LONG		dl_DiskType;	/* 'DOS', etc */
    LONG		dl_unused;
    BSTR *		dl_Name;	/* bptr to bcpl name */
};

/* device structure (same as the DeviceNode structure in filehandler.h) */

struct	      DevInfo {
    BPTR  dvi_Next;
    LONG  dvi_Type;
    APTR  dvi_Task;
    BPTR  dvi_Lock;
    BSTR  dvi_Handler;
    LONG  dvi_StackSize;
    LONG  dvi_Priority;
    LONG  dvi_Startup;
    BPTR  dvi_SegList;
    BPTR  dvi_GlobVec;
    BSTR  dvi_Name;
};

/* combined structure for devices, assigned directories, volumes */

struct DosList {
    BPTR		dol_Next;	 /* bptr to next device on list */
    LONG		dol_Type;	 /* see DLT below */
    struct MsgPort     *dol_Task;	 /* ptr to handler task */
    BPTR		dol_Lock;
    union {
	struct {
	BSTR	dol_Handler;	/* file name to load if seglist is null */
	LONG	dol_StackSize;	/* stacksize to use when starting process */
	LONG	dol_Priority;	/* task priority when starting process */
	ULONG	dol_Startup;	/* startup msg: FileSysStartupMsg for disks */
	BPTR	dol_SegList;	/* already loaded code for new task */
	BPTR	dol_GlobVec;	/* BCPL global vector to use when starting
				 * a process. -1 indicates a C/Assembler
				 * program. */
	} dol_handler;

	struct {
	struct DateStamp	dol_VolumeDate;	 /* creation date */
	BPTR			dol_LockList;	 /* outstanding locks */
	LONG			dol_DiskType;	 /* 'DOS', etc */
	} dol_volume;

    } dol_misc;

    BSTR		dol_Name;	 /* bptr to bcpl name */
    };
    

/* definitions for dl_Type */
#define DLT_DEVICE	0
#define DLT_DIRECTORY	1
#define DLT_VOLUME	2

/* a lock structure, as returned by Lock() or DupLock() */
struct FileLock {
    BPTR		fl_Link;	/* bcpl pointer to next lock */
    LONG		fl_Key;		/* disk block number */
    LONG		fl_Access;	/* exclusive or shared */
    struct MsgPort *	fl_Task;	/* handler task's port */
    BPTR		fl_Volume;	/* bptr to a DeviceList */
};

#endif	/* LIBRARIES_DOSEXTENS_H */
