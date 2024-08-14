/***********************************************************************
***
***  packets.h
***
***	CDFS Packet dispatch table.
***
***	This table is organized in a dispatch frequency order.
***
***	V24.1 -	ACTION_DIRECT_READ added
***		ARGS define now used (no longer NOARGS in these)
***
***********************************************************************/

IMPORT	ULONG	A_FHFromLock();
IMPORT	ULONG	A_IsFilesystem();
IMPORT	ULONG	A_DupLockFH();
IMPORT	ULONG	A_ParentOfFH();
IMPORT	ULONG	A_ExamineFH();
IMPORT	ULONG	A_LocateObj();
IMPORT	ULONG	A_ExamineNext();
IMPORT	ULONG	A_ExamineObj();
IMPORT	ULONG	A_Read();
IMPORT	ULONG	A_FreeLock();
IMPORT	ULONG	A_CurrentVol();
IMPORT	ULONG	A_OpenFile();
IMPORT	ULONG	A_End();
IMPORT	ULONG	A_Seek();
IMPORT	ULONG	A_Parent();
IMPORT	ULONG	A_CopyDir();
IMPORT	ULONG	A_DiskInfo();
IMPORT	ULONG	A_Info();
IMPORT	ULONG	A_Flush();
IMPORT	ULONG	A_Inhibit();
IMPORT	ULONG	A_MoreCache();
IMPORT	ULONG	A_FileDelError();
IMPORT	ULONG	A_DirectRead();
IMPORT	ULONG	NoWrite();

struct s_Actions
{
	UWORD	action;		/* action number		*/
	UWORD	argflags;	/* args to convert via shift	*/
	ULONG	(*func)();	/* action function pointer	*/
};

/*
 * argflags is a mask of which arguments are BPTRs.  For each 1 bit, that argument
 * is shifted left by 2.  NOARG and ARGS are both 0 (do nothing).  ARGS is 
 * used to differentiate between no args, and an fh_Arg1 in dp_Arg1.
 */

/* Want this to be a const, but MANX 3.6 cannot handle it. */
struct s_Actions d_Actions[] =
{
#asm
	cseg		; TABLE MUST BE IN ROM CODE SEGMENT
	public	_Actions
_Actions:
#endasm
	{ 'R',  ARGS,	A_Read		},
	{1005,  ARG123,	A_OpenFile	},
	{  24,  ARG12,	A_ExamineNext	},
	{   8,  ARG12,	A_LocateObj	},
	{  15,  ARG1,	A_FreeLock	},
	{1007,  ARGS,	A_End		},	/* Close file */
	{  23,  ARG12,	A_ExamineObj	},
	{1008,  ARGS,	A_Seek		},
	{  19,  ARG1,	A_CopyDir	},
	{  25,  ARG1,	A_DiskInfo	},
	{  29,  ARG1,	A_Parent	},
	{  26,  ARG12,	A_Info		},
	{   7,  ARG1,	A_CurrentVol	},
	{1900,  ARGS,	A_DirectRead	},	/* V24.1 added	*/
	{  27,  NOARG,	A_Flush		},
	{  31,  ARGS,	A_Inhibit	},
	{  18,  ARGS,	A_MoreCache	},
	{1026,  ARG12,	A_FHFromLock	},	/* OpenFileFromLock() */
	{1027,  NOARG,	A_IsFilesystem	},	/* IsFileSystem() */
	{1030,  ARGS,	A_DupLockFH	},	/* DupLockFH()    */
	{1031,  ARGS,	A_ParentOfFH	},	/* ParentOfFH()    */
/* 1033 examine all */
	{1034,  ARG2,	A_ExamineFH	},	/* ExamineFH()    */

	{1004,  ARG123,	A_OpenFile	},	/* FINDUPDATE	*/
	{1006,  ARG123,	A_OpenFile	},	/* FINDOUTPUT	*/
	{  16,  ARG12,	A_FileDelError	},	/* DELETE OBJECT */

	{   9,  NOARG,	NoWrite		},	/* RENAME DISK */
	{  17,  NOARG,	NoWrite		},	/* RENAME_OBJECT */
	{  21,  NOARG,	NoWrite		},	/* SET_PROTECT	*/
	{  22,  NOARG,	NoWrite		},	/* CREATE_DIR	*/
	{  28,  NOARG,	NoWrite		},	/* SET_COMMENT	*/
	{  34,  NOARG,	NoWrite		},	/* SET_DATE	*/
	{ 'W',  NOARG,	NoWrite		},	/* WRITE	*/
	0
};

extern struct s_Actions Actions[];
