/* ========================================================================= *
 * Files.c - directory searching code for installer utility                  *
 * By Joe Pearce. (c) 1990 Sylvan Technical Arts                             *
 * ========================================================================= */

/*	NOTES:

	The pattern matching code is still not the same as that supported
	by KS 2.0. If such code is ever put in here, it would be best that
	the calling interace should be the same as the SAS/C function astcsma().
*/

#ifndef EXEC_TYPES_H

#include <intuition/intuition.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <exec/memory.h>

/* #define __NO_PRAGMAS */
#include "functions.h"
#endif

#include <string.h>

#include "macros.h"
#include "xfunctions.h"

#include "installer.h"
#include "wild.h"
#include "text.h"

extern struct DosLibrary *DOSBase;

extern char *dot_info, *dot_font;

struct StringNode {			/* structure to store pathnames & dos types */
	struct MinNode	node;
	LONG			size;
	LONG			dostype;
	WORD			type;
	struct String	str;
};

#define STR_OFFSET (sizeof(struct StringNode) - sizeof(struct String))

WORD get_entry_type(struct String *str)
{	struct StringNode *n = (struct StringNode *)((char *)str - STR_OFFSET);

	return n->type;
}

LONG get_dos_type(struct String *str)
{	struct StringNode *n = (struct StringNode *)((char *)str - STR_OFFSET);

	return n->dostype;
}

LONG get_entry_size(struct String *str)
{	struct StringNode *n = (struct StringNode *)((char *)str - STR_OFFSET);

	return n->size;
}

LONG astcsma(char *str, char *pat)
{	LONG PatternParse(char *,char *,LONG);
	LONG PatternMatch(char *,char *);
#ifdef ONLY2_0
	LONG	length = 2 * strlen(pat) + 8;
#else
	LONG	length = 5 * strlen(pat) + 8;
#endif
	LONG	result = 0;
	char	*pattern_buf;

	if (pattern_buf = AllocMem(length,0L))
	{
#ifdef ONLY2_0
		if (ParsePatternNoCase(pat,pattern_buf,length - 4) != -1)
		{
			if (MatchPatternNoCase(pattern_buf,str)) result = strlen(str);
		}
#else
		if (PatternParse(pat,pattern_buf,length - 4) != -1)
		{
			if (PatternMatch(pattern_buf,str)) result = strlen(str);
		}
#endif

		FreeMem(pattern_buf,length);
	}

	return result;
}

	/* pattern MUST be file only, no path info */

struct String **match_files(BPTR lock,char *pat,WORD mode,LONG *total)
{	struct FileInfoBlock *fib;
	struct String **str;
	struct MinList list;
	struct StringNode *n,*n2;
	char	*name;
	LONG	err,i,j,k,rcount,count = 0;
	WORD	no_infos = mode & UNMATCH_INFO_FILES,
			free_infos = mode & MATCH_FREE_INFOS,
			no_fonts = mode & UNMATCH_FONT_FILES;

	mode &= ~(UNMATCH_FONT_FILES|UNMATCH_INFO_FILES|MATCH_FREE_INFOS);

		/* initialize list */
	NewList((struct List *)&list);

		/* allocate a FileInfoBlock */
	fib = MemAlloc(sizeof *fib,MEMF_PUBLIC|MEMF_CLEAR);
	if (fib == NULL) return NULL;

		/* find any entries that match */
	if (Examine(lock,fib))
	{
		while (ExNext(lock,fib))
		{	LONG	len;

			if (fib->fib_EntryType < 0 && mode == MATCH_DIRS_ONLY) continue;
			if (fib->fib_EntryType > 0 && mode == MATCH_FILES_ONLY) continue;

				/* what about ".info/.icon" files */
			if (!stricmp(fib->fib_FileName,dot_info)) continue;

			len = strlen(fib->fib_FileName);

			if (no_infos != 0 &&
				astcsma(fib->fib_FileName,"#?.info") == len) continue;
			if (no_fonts != 0 &&
				astcsma(fib->fib_FileName,"#?.font") == len) continue;

				/* see if its a match */
			err = (astcsma(fib->fib_FileName,pat) == len ? MATCH_FOUND : MATCH_FAILED);

			if (err == MATCH_FOUND)
			{	n = MemAlloc(sizeof *n + 32,MEMF_CLEAR);
				if (n == NULL) goto fail;

				strcpy((char *)(n + 1),fib->fib_FileName);
				n->dostype = fib->fib_EntryType;
				n->type = (fib->fib_EntryType < 0 ? IS_FILE : IS_DRAWER);
				n->size = fib->fib_Size;
				n->str.length = strlen(fib->fib_FileName);
				AddTail((struct List *)&list,(struct Node *)n);
				count++;
			}
		}
	}

	rcount = count;

	/*	if MATCH_FREE_INFOS set, remove any #?.info file matching
		a non-.info file...											*/

	if (free_infos)
	{
		for (i=0,n=GetHead(&list);i<count;i++,n=NextNode(n))
		{
			name = (char *)(n+1);
			if (astcsma(name,"#?.info") == strlen(name))
			{
				for (j=0,n2=GetHead(&list);j<count;j++,n2=NextNode(n2))
				{
					if (n->str.length == n2->str.length + 5 &&
						!strnicmp(name,(char *)(n2+1),n2->str.length))
					{
						n->str.length = 0;		/* invalidate node */
						rcount--;
						break;
					}
				}
			}
		}
	}

	/*	if MATCH_FILES_ONLY , remove any #?.info file matching
		a directory...												*/

	if (mode == MATCH_FILES_ONLY)
	{
		for (i=0,n=GetHead(&list);i<count;i++,n=NextNode(n))
			if (n->str.length != 0)
		{
			name = (char *)(n+1);
			if (astcsma(name,"#?.info") == strlen(name))
			{	char	*dot;

				if (dot = strrchr(name,'.'))
				{	BPTR dlock, olddir;

					olddir = CurrentDir(lock);
					*dot = 0;
					if (dlock = Lock(name,ACCESS_READ))
					{
						if (Examine(dlock,fib) && fib->fib_EntryType < 0)
						{
							n->str.length = 0;		/* invalidate node */
							rcount--;
						}
						UnLock(dlock);
					}
					*dot = '.';
					CurrentDir(olddir);
				}
			}
		}
	}

	MemFree(fib);

		/* allocate one larger so last entry in NULL */
	str = MemAlloc(4 * (rcount+1),MEMF_CLEAR);
	if (str == NULL) goto fail;

		/* now, do an insertion sort */
	for (i=0,n=GetHead(&list);n;n=NextNode(n))
	{	if (n->str.length)
		{
			for (j=0;j<i;j++)
			{
				k = get_entry_type(str[j]);
				if (k != n->type)
				{	if (k == IS_DRAWER) goto sort;
					continue;
				}

				if ( stricmp(STR_VAL(str[j]),STR_VAL(&n->str)) > 0 )
				{
sort:				for (k=i;k>j;k--) str[k] = str[k-1];
					break;
				}
			}
			str[j] = &n->str;
			i++;
		}
	}

	*total = rcount;
	return str;

fail:
	while (n = (struct StringNode *)RemHead((struct List *)&list)) MemFree(n);
	return NULL;
}

/* free an array of StringNode structures */

void free_matches(struct String **str)
{	struct String **hold = str;

	if (str)
	{	while (*str)
		{	MemFree((char *)(*str) - STR_OFFSET);
			str++;
		}
		MemFree(hold);
	}
}

/* functions to (somewhat) duplicate various 2.0 DosList calls */

#ifndef ONLY2_0
struct DosList *LockDevList(void)
{	struct RootNode	  	  		*rnode; 
	struct DosInfo	  		  	*dinfo;

	rnode = (struct RootNode *)DOSBase->dl_Root;
	dinfo = (struct DosInfo*)BADDR(rnode->rn_Info);
 
	Forbid();

	return (struct DosList *)&dinfo->di_DevInfo;
}

struct DosList *FindDevEntry(struct DosList *dnode, char *name, LONG type)
{	char	*str;
	WORD	length = strlen(name);

	for (dnode = (struct DosList *)BADDR(dnode->dol_Next);dnode;
 		dnode=(struct DosList *)BADDR(dnode->dol_Next))
	{
		if (((type & LDF_ASSIGNS) && dnode->dol_Type == DLT_DIRECTORY) ||
			((type & LDF_DEVICES) && dnode->dol_Type == DLT_DEVICE) ||
			((type & LDF_VOLUMES) && dnode->dol_Type == DLT_VOLUME))
		{
			str = (char *)BADDR(dnode->dol_Name);
			if (str[0] == length && !strnicmp(&str[1],name,length))
			{
				return dnode;
			}
		}
	}

	return NULL;
}

struct DosList *NextDevEntry(struct DosList *dnode, LONG type)
{
	for (dnode = (struct DosList *)BADDR(dnode->dol_Next);dnode;
 		dnode=(struct DosList *)BADDR(dnode->dol_Next))
	{
		if (((type & LDF_ASSIGNS) && dnode->dol_Type == DLT_DIRECTORY) ||
			((type & LDF_DEVICES) && dnode->dol_Type == DLT_DEVICE) ||
			((type & LDF_VOLUMES) && dnode->dol_Type == DLT_VOLUME))
		{
			return dnode;
		}
	}

	return NULL;
}

void UnLockDevList(void)
{
	Permit();
}
#endif

/* is a device a file system with disk inserted? */

BOOL is_disk(struct DeviceNode *dnode)
{	long	type = DiskType(dnode);

	return (type != ID_NO_DISK_PRESENT &&
			type != ID_UNREADABLE_DISK &&
			type != ID_NOT_REALLY_DOS &&
			type != ID_KICKSTART_DISK &&
			type != 0);
}

/* check a dos node to see if its a valid DEVICE or ASSIGN */

BOOL check_node(struct MinList *list,struct DeviceNode *dnode,LONG *count)
{	struct StringNode	*n;
	char				*name;

	if (dnode->dn_Type == DLT_DEVICE && is_disk(dnode))
	{
		n = MemAlloc(sizeof *n + 64,MEMF_CLEAR);
		if (n == NULL) return FALSE;

		name = (char *)(n+1);
		CpyBstrCstr((void *)dnode->dn_Name,name);
		strcat(name,":");
		n->type = IS_DEVICE;
		n->size = 0;
		AddTail((struct List *)list,(struct Node *)n);
		(*count)++;
	}
	else if (dnode->dn_Type == DLT_DIRECTORY)
	{
		n = MemAlloc(sizeof *n + 32,MEMF_CLEAR);
		if (n == NULL) return FALSE;

		name = (char *)(n+1);
		CpyBstrCstr((void *)dnode->dn_Name,name);
		strcat(name,":");
		n->type = IS_ASSIGN;
		n->size = 0;
		AddTail((struct List *)list,(struct Node *)n);
		(*count)++;
	}

	return TRUE;
}

/* make a list of drives */

struct String **get_drives(LONG *total)
{	extern struct DosLibrary *DOSBase;
	struct DeviceNode	*dnode;
	struct String		**str;
	BPTR				lock;
	struct MinList		list;
	struct StringNode	*n;
	LONG				i,j,k,count = 0;
	char				*name;

	NewList((struct List *)&list);

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)
			LockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_READ))
		{
			while (dnode = (struct DeviceNode *)
				NextDosEntry((void *)dnode,LDF_DEVICES | LDF_ASSIGNS))
			{
				if (!check_node(&list,dnode,&count))
				{
					UnLockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_READ);
					goto fail;
				}
			}
	
			UnLockDosList(LDF_DEVICES | LDF_ASSIGNS | LDF_READ);
		}
	}
#ifndef ONLY2_0
	else
	{
		if (dnode = (struct DeviceNode *)LockDevList())
		{
			while (dnode = (struct DeviceNode *)NextDevEntry((void *)dnode,LDF_DEVICES | LDF_ASSIGNS))
			{
				if (!check_node(&list,dnode,&count))
				{
					UnLockDevList();
					goto fail;
				}
	  		}

			UnLockDevList(); 
		}
	}
#endif

		/* allocate one larger so last entry is NULL */
	str = MemAlloc(4 * (count+1),MEMF_CLEAR);
	if (str == NULL) goto fail;

		/* now, do an insertion sort */
	for (i=0,n=GetHead(&list);i<count;i++,n=NextNode(n))
	{
		for (j=0;j<i;j++)
		{
			k = get_entry_type(str[j]);
			if (k != n->type)
			{	if (k == IS_ASSIGN) goto sort;
				continue;
			}

			if ( stricmp(STR_VAL(str[j]),STR_VAL(&n->str)) > 0 )
			{
sort:			for (k=i;k>j;k--) str[k] = str[k-1];
				break;
			}
		}
		str[j] = &n->str;

		if (n->type == IS_DEVICE)
		{	name = STR_VAL(&n->str);
			name += strlen(name) + 1;

			if (lock = Lock(STR_VAL(&n->str),ACCESS_READ))
			{	ExpandPath((void *)lock,name,32L);
				UnLock(lock);
			}
			else *name = 0;
		}
	}

	*total = count;
	return str;

fail:
	while (n = (struct StringNode *)RemHead((struct List *)&list)) MemFree(n);
	return NULL;
}

long GetDiskSpace(char *name)
{	struct InfoData	*id;
	BPTR			flock;
	struct Process	*pr;
	APTR			prwin;
	int				size = 0;

	if (id = AllocMem(sizeof *id,MEMF_CLEAR|MEMF_PUBLIC))
	{
		pr = (struct Process *)FindTask(NULL);
		prwin = pr->pr_WindowPtr;
		pr->pr_WindowPtr = (APTR)-1;

		if (flock = Lock(name,ACCESS_READ))
		{
			if (Info(flock, id))
			{
				size = (id->id_NumBlocks - id->id_NumBlocksUsed) *
					id->id_BytesPerBlock;   
			}
			else size = -1;

			UnLock(flock);
		}

		FreeMem(id,sizeof *id);

		pr->pr_WindowPtr = prwin;
	}
	else memerr();

	return size;
}

/*	The following code trys to find the largest partition (with one
	exception) as the default place to install an application.		*/

#define MAX_DEVICES			64

struct DevData {					/* for keeping a copy of a dos node */
	struct DeviceNode	dnode;
	char				name[32];
};

struct String *find_largest_partition(void)
{	extern struct DosLibrary	*DOSBase;
	struct InfoData				*id;
	struct DevData				*dd_list,
								*dd_node;
	struct DeviceNode			*dnode;
	int							dn_count = 0,
								i;
	int							bestsize = 0;
	char						*bname = NULL;
	struct String				*str;

	unless (id = AllocMem(sizeof *id + MAX_DEVICES * sizeof *dd_list,
		MEMF_CLEAR)) return NULL;

	dd_list = (struct DevData *)(id + 1);

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)LockDosList(LDF_DEVICES | LDF_READ))
		{	
			while ((dnode = (struct DeviceNode *)NextDosEntry((void *)dnode,LDF_DEVICES)) &&
				dn_count < MAX_DEVICES)
			{
				dd_list[dn_count].dnode = *dnode;
				CpyBstrCstr((char *)dnode->dn_Name,dd_list[dn_count++].name);
			}
	
			UnLockDosList(LDF_DEVICES | LDF_READ);
		}
	}
#ifndef ONLY2_0
	else
	{
		if (dnode = (struct DeviceNode *)LockDevList())
		{
			while ( dn_count < MAX_DEVICES &&
				(dnode = (struct DeviceNode *)NextDevEntry((void *)dnode,LDF_DEVICES)) )
			{
				dd_list[dn_count].dnode = *dnode;
				CpyBstrCstr((char *)dnode->dn_Name,dd_list[dn_count++].name);
			}

			UnLockDevList();
		}
	}
#endif

		/* look for all valid devices and find best one to use */ 

	for (i=0; i<dn_count; i++)
	{	dd_node = dd_list + i;

		if ( is_disk(&dd_node->dnode) )
		{	BPTR			flock;
			int				size = 1;

			strcat(dd_node->name,":");

				/* if name is WORK:, give it precedence */
			if (!stricmp(dd_node->name,"Work:")) size = 2;

				/* now, start all over again to generate path name */

			if (flock = Lock(dd_node->name,ACCESS_READ))
			{
				ExpandPath((void *)flock,dd_node->name,32+10) + 1;
				if (!stricmp(dd_node->name,"Work:")) size = 2;

				if (Info(flock, id))
				{
					size *= id->id_NumBlocks - id->id_NumBlocksUsed;

					if (size > bestsize)
					{	bestsize = size;
						bname = dd_node->name;
					}
				}
				UnLock(flock);
			}
		}
	}

		/* return our best choice or SYS: if could find none */

	if (bname) str = MakeString(bname,strlen(bname));
	else str = MakeString("SYS:",4);

	FreeMem(id,sizeof *id + MAX_DEVICES * sizeof *dd_list);
	return str;
}

/*	The routine returns a copy of the dos node corresponding to a
	particular DOS handler task.									*/

struct DeviceNode *find_device_for_task(struct MsgPort *mp)
{	extern struct DosLibrary	*DOSBase;
	struct DeviceNode	  		*dnode;
	/* struct FileSysStartupMsg	*fssm; */
	static struct DeviceNode	found;


#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)LockDosList(LDF_DEVICES | LDF_READ))
		{
			while (dnode = (struct DeviceNode *)NextDosEntry((void *)dnode,LDF_DEVICES))
			{
				if (dnode->dn_Task == mp) { found = *dnode; break; }
			}
	
			UnLockDosList(LDF_DEVICES | LDF_READ);
		}
	}
#ifndef ONLY2_0
	else
	{
		if (dnode = (struct DeviceNode *)LockDevList())
		{
			while (dnode = (struct DeviceNode *)NextDevEntry((void *)dnode,LDF_DEVICES))
			{
				if (dnode->dn_Task == mp) { found = *dnode; break; }
			}
	
			UnLockDevList();
		}
	}
#endif

	return (dnode ? &found : NULL);
}

/*	This routine attempts to find the system's boot volume. It neccessary,
	it will ask the user to insert his boot disk (if on a floppy-based
	system).															*/

BPTR boot_volume(void)
{	APTR				prwin;
	struct DeviceNode	*bdn;
	struct FileSysStartupMsg *fssm;
	BPTR				olddir,
						lock,
						block = NULL;
	struct FileLock		*c_lock;
	struct Process		*pr = (struct Process *)FindTask(NULL);
	char				buffer[256];

	prwin = pr->pr_WindowPtr;
	pr->pr_WindowPtr = (APTR)-1;

	olddir = CurrentDir(NULL);

	if ( (lock = Lock("",ACCESS_READ)) == NULL )
	{
			/*  this means the drive was empty */
askdisk:
		while (report_message(0,GetLocalText(MSG_BOOTDISK)))
		{
			if (lock = Lock("",ACCESS_READ))
			{	block = lock;
				lock = NULL;
				break;
			}
		}
	}
	else
	{
		c_lock = (struct FileLock *)BADDR(lock);

		bdn = find_device_for_task(c_lock->fl_Task);
		if (bdn == NULL)
		{
				/* this is a fatal error */
			/* Puts("Can't find device belonging to lock!"); */
		}
		else
		{
		/*	CpyBstrCstr((void *)bdn->dn_Name,buffer);
			Printf("Device name = %s\n",buffer); */

			if (bdn->dn_Startup == NULL)
			{
					/* accept as unremovable media */
				block = lock;
				lock = NULL;
				/* Puts("Volume node has NULL startup pointer!"); */
			}
			else
			{
				fssm = (struct FileSysStartupMsg *)BADDR(bdn->dn_Startup);

				if (fssm->fssm_Device == NULL)
				{
						/* accept as unremovable media */
					block = lock;
					lock = NULL;
					/* Puts("Startup node has NULL device name pointer"); */
				}
				else
				{
					CpyBstrCstr((void *)fssm->fssm_Device,buffer);

					if (!strcmp(buffer,"trackdisk.device")) goto askdisk;

						/* accept as unremovable media */
					block = lock;
					lock = NULL;
					/* Printf("Driver name = %s\n",buffer); */
				}
			}
		}
	}

bye:
	if (olddir) CurrentDir(olddir);
	if (lock) UnLock(lock);
	pr->pr_WindowPtr = prwin;

	return block;
}
