/*
 *		action.c
 *
 * ========================================================================= 
 * Action.c - perform real actions for the installer                         
 * By Joe Pearce. (c) 1990 Sylvan Technical Arts                             
 * ========================================================================= 
 *
 *
 * Prototypes for functions defined in action.c
 *
 *	extern UWORD 			LogFileData			[208];
 *	extern struct Image 	LogFileImage;
 *	
 *	struct DiskObject * 	GetEmptyDiskObject	(void);
 *	void 					FreeEmptyDiskObject	(struct DiskObject * );
 *	void					write_log_file_icon	(unsigned char * );
 *	BOOL 					create_drawer_icon	(unsigned char * );
 *	void 					swap_image			(struct Image * );
 *	BOOL 					modify_icon			(unsigned char * , struct ToolType * , unsigned char * , int , WORD , WORD );
 *	BPTR 					LockDir				(unsigned char * , long );
 *	BOOL 					create_dir			(unsigned char * , BOOL );
 *
 *	extern BOOL 			did_tree_error;
 *
 *	WORD 					CopyFileWithStatus	(BPTR , BPTR , unsigned char * , BPTR , long );
 *	WORD 					CopyFile			(BPTR , BPTR );
 *	BPTR 					LockOrCreateDir		(unsigned char * );
 *	BPTR 					OpenNew				(unsigned char * , ULONG * );
 *	void 					mySetIoErr			(long );
 *	WORD 					copy_file			(unsigned char * , unsigned char * , unsigned char * , WORD );
 *	int 					copy_tree_file		(BPTR , BPTR , struct FileInfoBlock * );
 *	int 					copy_tree_dir		(BPTR , BPTR );
 *	int 					copy_tree			(BPTR , unsigned char * );
 *	int 					copy_object			(unsigned char * , unsigned char * , unsigned char * , WORD );
 *	BPTR 					set_execute_dir		(void);
 *	int 					execute_command		(unsigned char * );
 *
 *	extern struct Library * RexxSysBase;
 *
 *	int 					send_rexx			(unsigned char * );
 *	BOOL 					GetAssign			(unsigned char * , unsigned char * , LONG );
 *	BOOL 					add_assign			(struct DosInfo * , unsigned char * , BPTR );
 *	BOOL 					SetAssign			(unsigned char * , BPTR );
 *	BPTR 					lock_object			(struct DeviceNode * );
 *	BOOL 					FindAssign			(unsigned char * , long , BPTR * );
 *	long 					MatchAssign			(unsigned char * , unsigned char * , long );
 *	long 					VolumeMounted		(unsigned char * , long );
 *	BOOL 					FindDOSDevice		(unsigned char * , unsigned char * );
 *	long 					ChangeFileDate		(unsigned char * , struct DateStamp * );
 *	int 					RelabelDisk			(unsigned char * , unsigned char * );
 *
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 10/25/93:
 *		Changed copy_file to return 2 if the source file doesn't exist
 *		and this is allowed.  This lets copy_extra_file know that the
 *		file doesn't exist, so it doesn't claim that it copied it.
 *		Added the filename to many of the copy failure messages.
 */



#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <dos/dostags.h>
#include <workbench/workbench.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include "functions.h"

#define SystemTagList	System

#include <stddef.h>
#include <string.h>

#include "macros.h"
#include "xfunctions.h"

#include "installer.h"
#include "window.h"

extern char	*err_cannot_create,
			*err_cannot_copy,
			*err_overwrite,
			*prob_with_source,
			*err_execute_path,
			*err_no_nil,
			*err_no_command,
			*err_no_rexx,
			*err_cannot_duplock;

extern struct DosLibrary	*DOSBase;

struct RexxMsg *CreateRexxMsg(struct MsgPort *,char *,char *);
STRPTR CreateArgstring(char *,ULONG);
void DeleteRexxMsg(struct RexxMsg *);
void DeleteArgstring(STRPTR);

extern struct TreeNode *ehead;

/*========================== default drawer icon ===========================*/

static USHORT empty_Data[] = 
{
	/* plane 0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 
	0xf880, 0x001f, 0xffff, 0xffff, 0xffff, 0xe300, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0e80, 0x03ff, 0xffff, 0xffff, 0xffff, 0xcf00, 0x0380, 0x0000, 
	0x0000, 0x0001, 0xce80, 0x039f, 0xffff, 0xffff, 0xfff9, 0xcd00, 0x039f, 
	0xffff, 0xffff, 0xfff9, 0xce80, 0x039f, 0xfff1, 0xff8f, 0xfff9, 0xcd00, 
	0x039f, 0xffe0, 0x001f, 0xfff9, 0xca00, 0x039f, 0xffff, 0xffff, 0xfff9, 
	0xcc00, 0x039f, 0xffff, 0xffff, 0xfff9, 0xc800, 0x0380, 0x0000, 0x0000, 
	0x0001, 0xc000, 0x03ff, 0xffff, 0xffff, 0xffff, 0xc000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	/* plane 1 */
	0x0001, 0xffff, 0xffff, 0xffff, 0xffc0, 0x001e, 0x0000, 0x0000, 0x0000, 
	0x0740, 0x01e0, 0x0000, 0x0000, 0x0000, 0x1cc0, 0x0fff, 0xffff, 0xffff, 
	0xffff, 0xf140, 0x0c00, 0x0000, 0x0000, 0x0000, 0x30c0, 0x0c7f, 0xffff, 
	0xffff, 0xfffe, 0x3140, 0x0c60, 0x0000, 0x0000, 0x0006, 0x32c0, 0x0c60, 
	0x0000, 0x0000, 0x0006, 0x3140, 0x0c60, 0x000e, 0x0070, 0x0006, 0x32c0, 
	0x0c60, 0x001f, 0xffe0, 0x0006, 0x3580, 0x0c60, 0x0000, 0x0000, 0x0006, 
	0x3300, 0x0c60, 0x0000, 0x0000, 0x0006, 0x3600, 0x0c7f, 0xffff, 0xffff, 
	0xfffe, 0x3c00, 0x0c00, 0x0000, 0x0000, 0x0000, 0x3800, 0x0fff, 0xffff, 
	0xffff, 0xffff, 0xf000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

static struct Image empty = 
{
	0,0,									/* LeftEdge,TopEdge 	*/
	74,18,2,								/* Width,Height,Depth 	*/
	&empty_Data[0],							/* Address of data 		*/
	0x03,0x00,								/* PlanePick,PlaneOnOff */
	NULL };		 							/* NextImage 			*/

static USHORT empty2_Data[] = 
{
	/* plane 0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff, 
	0xf880, 0x001f, 0xffff, 0xffff, 0xffff, 0xe300, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0e80, 0x03ff, 0xffff, 0xffff, 0xffff, 0xcf00, 0x0380, 0x0000, 
	0x0000, 0x0001, 0xce80, 0x0200, 0x0000, 0x0000, 0x0001, 0xcd00, 0x0000, 
	0x0000, 0x0000, 0x0009, 0xce80, 0x0000, 0x0000, 0x0000, 0x0019, 0xcd00, 
	0x1fff, 0xffff, 0xffff, 0xff99, 0xca00, 0x1fff, 0xffff, 0xffff, 0xff99, 
	0xcc00, 0x1fff, 0xffff, 0xffff, 0xff99, 0xc800, 0x1fff, 0xfc7f, 0xe3ff, 
	0xff99, 0xc000, 0x1fff, 0xf800, 0x07ff, 0xff93, 0xc000, 0x1fff, 0xffff, 
	0xffff, 0xff80, 0x0000, 0x1fff, 0xffff, 0xffff, 0xff80, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	/* plane 1 */
	0x0001, 0xffff, 0xffff, 0xffff, 0xffc0, 0x001e, 0x0000, 0x0000, 0x0000, 
	0x0740, 0x01e0, 0x0000, 0x0000, 0x0000, 0x1cc0, 0x0fff, 0xffff, 0xffff, 
	0xffff, 0xf140, 0x0c00, 0x0000, 0x0000, 0x0000, 0x30c0, 0x0c7f, 0xffff, 
	0xffff, 0xfffe, 0x3140, 0x0dcd, 0x5555, 0x5555, 0x555e, 0x32c0, 0x1eac, 
	0xaaaa, 0xaaaa, 0xaab6, 0x3140, 0x7fff, 0xffff, 0xffff, 0xffe6, 0x32c0, 
	0x6000, 0x0000, 0x0000, 0x0066, 0x3580, 0x6000, 0x0000, 0x0000, 0x0066, 
	0x3300, 0x6000, 0x0000, 0x0000, 0x0066, 0x3600, 0x6000, 0x0380, 0x1c00, 
	0x0066, 0x3c00, 0x6000, 0x07ff, 0xf800, 0x006c, 0x3800, 0x6000, 0x0000, 
	0x0000, 0x007f, 0xf000, 0x6000, 0x0000, 0x0000, 0x0070, 0x0000, 0x7fff, 
	0xffff, 0xffff, 0xffe0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

static struct Image empty2 = 
{
	0,0,									/* LeftEdge,TopEdge 	*/
	74,18,2,								/* Width,Height,Depth 	*/
	&empty2_Data[0],						/* Address of data 		*/
	0x03,0x00,								/* PlanePick,PlaneOnOff */
	NULL };		 							/* NextImage 			*/

static struct DrawerData def_drawer_data = 
{
	{	
		100,50, 
		320,70, 
		(UBYTE)-1,(UBYTE)-1, 
		0,
		WINDOWSIZING | WINDOWCLOSE | WINDOWDEPTH | WINDOWDRAG |
			SIMPLE_REFRESH | SIZEBRIGHT | SIZEBBOTTOM | WBENCHWINDOW | 
			0x00040000,
		NULL, NULL, NULL, NULL, NULL,
		90,40, 
		(USHORT)-1,(USHORT)-1, 
		WBENCHSCREEN
	},
	0, 0
};

static struct DiskObject def_drawer = 
{
	WB_DISKMAGIC,
	WB_DISKVERSION,
	{
		NULL, 0,0, 74,18, GADGIMAGE | GADGHIMAGE, GADGIMMEDIATE | RELVERIFY,
		BOOLGADGET, (APTR)&empty, (APTR)&empty2,
	},
	WBDRAWER,
	NULL,
	NULL,
	NO_ICON_POSITION,
	NO_ICON_POSITION,
	&def_drawer_data,
	NULL,
	0
};	

/**************** LogFile Icon ****************/

UWORD LogFileData[] = 
{
	/* BitPlane 0 */
   0x0001,0xFFFC,0x0000,0x0000,
   0x0003,0xFFFE,0x0000,0x0020,
   0x0007,0x0007,0x0000,0x0020,
   0x0006,0x0003,0x0000,0x0420,
   0x1000,0x0083,0x0000,0x0220,
   0x3555,0x55F3,0x0000,0x0A20,
   0x7AAA,0xAAF3,0x000F,0xFA20,
   0x7000,0x00F3,0x000F,0xFA20,
   0x7000,0x00F3,0x001F,0xFA20,
   0x7000,0x00F3,0x0001,0x8220,
   0x7000,0x00F3,0x0021,0x8420,
   0x7000,0x00F3,0x801F,0xF820,
   0x7000,0x00F1,0xFFFF,0x0020,
   0x7000,0x00F0,0xFFFE,0x0020,
   0x7000,0x00F0,0x0000,0x0020,
   0x7FFF,0xFFF0,0x0000,0x0420,
   0x7FFF,0xFFF0,0x0000,0x0220,
   0x7FFF,0xFFF0,0x000C,0x3220,
   0x7C00,0x03F0,0x000C,0x3220,
   0x7CE0,0x03F0,0x000C,0x3220,
   0x7CE0,0x03F0,0x000C,0x3220,
   0x7CE0,0x03F0,0x0000,0x0220,
   0x7CE0,0x03F0,0x0020,0x0420,
   0x7CE0,0x03F0,0x001F,0xF820,
   0x7C00,0x03F0,0x0000,0x0020,
   0x7FFF,0xFFF0,0x03FF,0xFFE0,
	/* BitPlane 1 */
   0x0000,0x0000,0x07FF,0xFFC0,
   0x0000,0x0000,0x0400,0x0000,
   0x0000,0x0000,0x041F,0xF800,
   0x0000,0x0000,0x0420,0x0000,
   0x2FFF,0xFF60,0x0440,0x0000,
   0x6FFF,0xFF60,0x045F,0xF000,
   0xEFFF,0xFF60,0x045E,0x7000,
   0xEFFF,0xFF60,0x045E,0x7000,
   0xEFFF,0xFF60,0x0440,0x0000,
   0xEFFF,0xFF60,0x0440,0x0000,
   0xEFFF,0xFF60,0x0400,0x0000,
   0xEFFF,0xFF60,0x0000,0x0000,
   0xEFFF,0xFF60,0x0000,0x0000,
   0xEFFF,0xFF60,0x0000,0x0000,
   0xEFFF,0xFF60,0x041F,0xF800,
   0xF000,0x00E0,0x0420,0x0000,
   0xFFFF,0xFFE0,0x0440,0x0000,
   0xFC00,0x03E0,0x0440,0x0000,
   0xFBFF,0xFDE0,0x0442,0x0800,
   0xFB1F,0xFDE0,0x0442,0x0800,
   0xFB5F,0xFDE0,0x0442,0x0800,
   0xFB5F,0xFDE0,0x0446,0x1800,
   0xFB5F,0xFDE0,0x0400,0x0000,
   0xFB1F,0xFDE0,0x0400,0x0000,
   0xFBFF,0xFDE0,0x0400,0x0000,
   0x0000,0x0000,0x0000,0x0000
};

struct Image LogFileImage = 
{
	0, 0,									/* LeftEdge, TopEdge */
	59, 26, 2,								/* Width, Height, Depth */
	&LogFileData[0],						/* ImageData */
	0x03, 0x00,								/* PlanePick, PlaneOnOff */
	NULL									/* NextImage */
};

static struct DiskObject logfile_icon = 
{
	WB_DISKMAGIC,
	WB_DISKVERSION,
	{
		NULL, 0,0, 59,26, GADGIMAGE | GADGBACKFILL, GADGIMMEDIATE | RELVERIFY,
		BOOLGADGET, (APTR)&LogFileImage, NULL,
	},
	WBPROJECT,
	"SYS:utilities/more",
	NULL,
	NO_ICON_POSITION,
	NO_ICON_POSITION,
	NULL,
	NULL,
	0
};	

/*========================== icon functions ===============================*/

struct DiskObject *GetEmptyDiskObject(void)
{
	extern struct Library *IconBase;

	if (IconBase->lib_Version >= 36) 
		return GetDiskObject(NULL);
	return AllocMem(sizeof (struct DiskObject), MEMF_CLEAR|MEMF_PUBLIC);
}

void FreeEmptyDiskObject(struct DiskObject *dobj)
{
	extern struct Library *IconBase;

	if (IconBase->lib_Version >= 36) 
		FreeDiskObject(dobj);
	else 
		FreeMem(dobj,sizeof *dobj);
}

void write_log_file_icon(char *path)
{
	struct DiskObject	*dobj;

	if (dobj = GetEmptyDiskObject())
	{
		*dobj = logfile_icon;
		PutDiskObject(path,dobj);
		FreeEmptyDiskObject(dobj);
	}
}


BOOL create_drawer_icon(char *path)
{
	struct DiskObject	*dobj;
	LONG				result;
	WORD				i;
	static BOOL 		swapped = FALSE;
	static char 		*iconnames[4] = 
	{
                              "SYS:Empty",            /* not there under 2.0 */
                              "SYS:Expansion",
                              "SYS:System",
                              "SYS:Utilities"
	};

#ifdef DEBUG
	Puts("Creating icon for drawer.");
#endif

	if (dobj = GetDiskObject(path))
	{
			/* if already exists, just return TRUE */
		FreeDiskObject(dobj);
		return TRUE;
	}		

#ifndef ONLY2_0
	if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
	{
		if (dobj = GetDefDiskObject(WBDRAWER))
		{
			dobj->do_CurrentX = dobj->do_CurrentY = NO_ICON_POSITION;

			result = PutDiskObject(path,dobj);

			FreeDiskObject(dobj);

#ifdef DEBUG
			Puts("Used 2.0 default icon.");
#endif
			return ((BOOL)(result ? 1 : 0));
		}
	}

	for (i=0;i<4;i++)
	{
		if (dobj = GetDiskObject(iconnames[i]))
		{
			if (dobj->do_Type == WBDRAWER)
			{
				dobj->do_CurrentX = dobj->do_CurrentY = NO_ICON_POSITION;
				if (!PutDiskObject(path,dobj))
				{
					FreeDiskObject(dobj);
					return FALSE;
				}

				FreeDiskObject(dobj);
#ifdef DEBUG
				Printf("Used icon of \"%s\".\n",iconnames[i]);
#endif
				return TRUE;
			}				

			FreeDiskObject(dobj);
		}
	}

	if (swapped == FALSE)
	{
#ifndef ONLY2_0
		if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
		{
			LONG	size;

			size = sizeof empty_Data / 2;
			swap_mem(&empty_Data[0],&empty_Data[size / sizeof(USHORT)],size);
			swap_mem(&empty2_Data[0],&empty2_Data[size / sizeof(USHORT)],size);
			swapped = TRUE;
		}
	}

	if (dobj = GetEmptyDiskObject())
	{
		*dobj = def_drawer;
		result = PutDiskObject(path,dobj);
		FreeEmptyDiskObject(dobj);
		if (result == 0) 
			return FALSE;
	}

#ifdef DEBUG
	Puts("Made a new icon.");
#endif
	return TRUE;
}

#define MAX_TOOLSTRING		128

#ifndef ONLY2_0
void swap_image(struct Image *im)
{
	/*	Reverse the planes in a 1.3 icon image so it will look nice
	 *	under 2.0 and beyond.
	 */
	 
	LONG	size;

	if (im && im->Depth > 1)
	{	
		size = (im->Width + 15 >> 4) * im->Height;
		swap_mem(im->ImageData,&im->ImageData[size],2 * size);
	}
}
#endif


BOOL modify_icon(
	char 				*name,
	struct ToolType 	*ntt,
	char 				*deftool,
	int 				 stack,
	WORD 				 nopos,
	WORD 				 swap)
{
	struct DiskObject	*dobj;
	struct ToolType		*att;							/* curr tool ptr	*/

	char				*c,
						*ttbuf,							/* tool string storage */
					   **tt,
					   **ftt,							/* tool ptr array start */
						*tv;

	WORD				 i,
						 k,
						 count,
						 final;
	BOOL				 result = FALSE;

	if (dobj = GetDiskObject(name))
	{
		count = 1;										/* count tooltypes */
		if (tt = dobj->do_ToolTypes) 
			while (*tt++) 
				count++;								/* number preexisting */
		for (att = ntt; att; count++, att = att->next);	/* how many we have */

		if (ftt = MemAlloc(count * (MAX_TOOLSTRING + sizeof (char *)),MEMF_CLEAR))
		{
			ttbuf = (char *)(ftt + count);
			final = 0;

			/* step 1, copy all old entries */
			
			if (tt = dobj->do_ToolTypes)				/* if existing tools	*/
			{
				while (*tt)								/* while string	exists	*/
                {
                    i = strlen(*tt) + 1;				/* length of string		*/
                    if (i + ttbuf >= (MAX_TOOLSTRING * count) 
                    	  			  + (char *)(ftt + count))
                    {
                        /* ERROR - buffer will overflow! */
                    }
                    ftt[final++] = ttbuf;				/* load ptr to string	*/
                    strcpy(ttbuf,*tt++);				/* save the string		*/
                    ttbuf += i;							/* pass the string		*/
                }
            }

			/* step 2, add or delete entries */
			
			for (att=ntt; att; att = att->next)			/* while new requests	*/
			{
				if (c = (char *)FindToolType(ftt,STR_VAL(att->tooltype)))
				{
					for (k=0;ftt[k];k++)				/* set k to tool number	*/
						if (c >= ftt[k] && c < ftt[k] + strlen(ftt[k])) 
							break;

					if (att->toolval)					/* if we have a new val	*/
					{
change_it:				i = strlen(STR_VAL(att->tooltype)) 
						  + strlen(STR_VAL(att->toolval)) + 2;
						if (i + ttbuf >= (MAX_TOOLSTRING * count) 
							  				+ (char *)(ftt + count))
						{
							/* ERROR */
						}

						ftt[k] = ttbuf;
						strcpy(ttbuf,STR_VAL(att->tooltype));
						if ( strlen(tv = STR_VAL(att->toolval)) )
						{
							strcat(ttbuf,"=");
							strcat(ttbuf,tv);
						}
						ttbuf += i;
					}
					else								/* no new value, del old */
					{
						ftt[k] = ftt[--final];			/* put last here		*/
						ftt[final] = NULL;				/* and kill last		*/
					}
				}
				else 									/* no match to old tool	*/
				if (att->toolval)						/* have a value?		*/
				{
					k = final++;						/* save new value		*/
					goto change_it;
				}
			}

			tt = dobj->do_ToolTypes;
			c  = dobj->do_DefaultTool;
			if (deftool) 
				dobj->do_DefaultTool = deftool;
			dobj->do_ToolTypes = ftt;

			if (stack) 
				dobj->do_StackSize = stack;

			if (nopos) 
				dobj->do_CurrentX = dobj->do_CurrentY = NO_ICON_POSITION;

#ifndef ONLY2_0
			if (swap && IntuitionBase->LibNode.lib_Version < 36)
			{
				swap_image((struct Image *)dobj->do_Gadget.GadgetRender);
				swap_image((struct Image *)dobj->do_Gadget.SelectRender);
			}
#endif

			result = (PutDiskObject(name,dobj) != 0);

			MemFree(ftt);
		}

		FreeDiskObject(dobj);
	}

	return result;
}

/*==================== directory creation functions =========================*/

BPTR LockDir(char *name,long mode)
{
	BPTR					lock;
	struct FileInfoBlock	*fib;

	if (lock = Lock(name,mode))
	{
		unless (fib = MemAlloc (sizeof *fib, MEMF_PUBLIC|MEMF_CLEAR))
		{
			UnLock(lock);
			memerr();
			return NULL;
		}

		if (Examine(lock,fib) == 0 || fib->fib_DirEntryType < 0)
		{
			UnLock(lock);
			lock = NULL;
		}

		MemFree(fib);
	}

	return lock;
}

	/* new version of create_dir (re-arranged for memory), written by --DJ */

BOOL create_dir(char *path,BOOL icon)
{
	BPTR			lock;

	if ((lock = LockDir(path,ACCESS_READ))
	||  (lock = CreateDir(path)) )
	{
		UnLock(lock);
		if (icon == FALSE || create_drawer_icon(path)) 
			return TRUE;
		DeleteFile(path);
	}
	err_msg (ehead, ERR_DOS, FUNC_MAKEDIR, err_cannot_create, path);
	return FALSE;
}

/*==================== file & directory copying functions ===================*/

/* the routine requires that the source dir be the current dir */
/* this also needs to copy protection, filedate & filenote */

#define COPYBUFSIZE		(1024 * 8)

BOOL did_tree_error;

extern struct Window	*window;
extern struct RastPort	*rp;

extern WORD		left_edge,
				right_edge,
				top_edge,
				bottom_edge;

extern WORD		highlight_color;

extern char		format_string[];

extern BOOL		show_gauge;

WORD CopyFileWithStatus (
	BPTR  sfh,
	BPTR  dfh,
	char *sname,
	BPTR  dlock,
	long  size)								/* support for copy progress box */
{
	long			read = 0, readsize, bufsize;
	void			*buffer;
	BOOL			result = 0;
	struct Gadget	*fgad = NULL;
	int				i;

	bufsize = size / 10;
	if (bufsize < 256) 
		bufsize = 256;
	if (bufsize > COPYBUFSIZE) 
		bufsize = COPYBUFSIZE;

	unless (buffer = AllocMem (bufsize, MEMF_PUBLIC|MEMF_CLEAR)) 
		return -1;

	if (sname)
	{
		if (!ExpandPath((void *)dlock,format_string,MAX_STRING))
		{
			FreeMem(buffer,bufsize);
			return -1;
		}

		fgad = abortcopy_display(sname,format_string);
	}

	while (TRUE)
	{
		readsize = Read(sfh,buffer,bufsize);
		if (readsize < 0) 
			break;
		if (readsize > 0 && Write(dfh,buffer,readsize) != readsize) 
			break;
		if (readsize < bufsize) 
		{ 
			result = 1; 
			break; 
		}

		read += bufsize;
		if (sname && (i = ((int)fgad->Width * read) / size))
		{
			if (i == 0) 
				i = 1;					/* no negative rectfills, please */

			SetAPen(rp,highlight_color);
			RectFill(rp,
					 fgad->LeftEdge,
					 fgad->TopEdge,
					 fgad->LeftEdge + i - 1,
					 fgad->TopEdge+fgad->Height-1);

			if (check_escape() && aborterr(FUNC_COPYFILES))
			{
				did_tree_error = TRUE;
				break;
			}
		}
	}

	if (sname)
	{
		if (result == 1)
		{
			SetAPen(rp,highlight_color);
			RectFill(rp,
					 fgad->LeftEdge,
					 fgad->TopEdge,
					 fgad->LeftEdge+fgad->Width-1,
					 fgad->TopEdge+fgad->Height-1);
		}
		/* set_busy(); */		/* commented out to reduce flashies */
	}

	FreeMem(buffer,bufsize);

	return result;
}

WORD CopyFile(BPTR sfh,BPTR dfh)
{
	return CopyFileWithStatus(sfh,dfh,NULL,NULL,0L);
}

BPTR LockOrCreateDir(char *name)
{
	BPTR	lock;

	unless (lock = Lock(name,ACCESS_READ))
	{
		unless (lock = CreateDir(name)) 
			return NULL;
		UnLock(lock);		
		unless (lock = Lock(name,ACCESS_READ)) 
			return NULL;
	}
	return lock;
}

extern UBYTE	local_delopts;
extern UWORD	user_level;

BPTR OpenNew(char *name, ULONG *error)
{
	BPTR					lock;
	BPTR					file;
	struct FileInfoBlock	*fib;
	LONG					protection = 0;

	*error = 0;

	if (lock = Lock(name,ACCESS_READ))
	{
		if (fib = AllocMem (sizeof *fib, MEMF_PUBLIC|MEMF_CLEAR))
		{
			if (Examine(lock,fib))
			{
				protection =  fib->fib_Protection;
			}
			else
			{
				*error = IoErr();
				FreeMem(fib,sizeof *fib);
				UnLock(lock);
				return NULL;
			}

			FreeMem(fib,sizeof *fib);
			UnLock(lock);
		}
		else
		{
			*error = -1;
			UnLock(lock);
			return NULL;
		}
	}

	if (protection & (FIBF_WRITE | FIBF_DELETE))
	{
		*error = (protection & FIBF_DELETE ?
			ERROR_DELETE_PROTECTED : ERROR_WRITE_PROTECTED);

		if ( !(local_delopts & (DELOPT_ASKUSER | DELOPT_FORCE)) ) 
			return NULL;

		if (user_level == USER_NOVICE && !(local_delopts & DELOPT_FORCE))
			return NULL;

		*error = 0;

		if ((user_level != USER_NOVICE) && (local_delopts & DELOPT_ASKUSER) 
		&&	 !save_page (NULL, name, askdelete_page) )
		{
			return NULL;
		}

		SetProtection (name,0);	/* trying to delete, so other flags unimportant */
	}

	if (file = Open(name,MODE_NEWFILE)) 
		return file;
	*error = IoErr();
	return NULL;
}

#ifndef ONLY2_0

void mySetIoErr(long error)
{
	struct Process *pr = (struct Process *)FindTask(NULL);
	pr->pr_Result2 = error;
}

#endif

WORD copy_file(char *filename, char *dirname, char *newname, WORD nopos)
{
	struct FileInfoBlock *fib;
	BPTR				slock = NULL,
						dlock = NULL,
						olddir = NULL;
	BPTR				sfile = NULL,
						dfile = NULL;
	WORD				cp, 
						result = 0;
	char				ch, 
						*dot;
	WORD				counts[3];
	BOOL				ok;
	ULONG				error;

	did_tree_error = FALSE;

	if (newname == NULL) 
		newname = FileOnly(filename);

	unless (fib = AllocMem(sizeof *fib,MEMF_PUBLIC|MEMF_CLEAR))
	{
		memerr();
		goto err2;
	}

	slock = Lock(filename,ACCESS_READ);
	sfile = Open(filename,MODE_OLDFILE);
	if (sfile == NULL || slock == NULL)
	{
	 	if (local_delopts & DELOPT_OKNOSRC) 
	 	{ 
	 		result = 2; 
	 		goto err2; 
	 	}
		goto err1;
	}

	if (Examine(slock,fib) == 0) 
		goto err1;
	unless (dlock = LockOrCreateDir(dirname)) 
		goto err1;

	olddir = CurrentDir(dlock);

	unless (dfile = OpenNew(newname,&error))
	{
		if (error == 0) 
		{ 
			result = 1; 
			goto err2; 
		}
		
		if ((error == ERROR_DELETE_PROTECTED 
		 ||  error == ERROR_WRITE_PROTECTED 
		 ||  error == ERROR_DISK_WRITE_PROTECTED) 
		&& (local_delopts & DELOPT_OKNODELETE)) 
		{ 
			result = 1; 
			goto err2; 
		}
		
#ifdef ONLY2_0
		SetIoErr(error);
#else
		mySetIoErr(error);
#endif
		goto err1;
	}

		/* put up status box and copy file */

	ok = abortcopy_page(counts);
	cp = CopyFileWithStatus (sfile, 
							 dfile, 
							 show_gauge ? filename : NULL,
							 dlock, 
							 fib->fib_Size);
	if (ok) 
		abortcopy_clear(counts);

	if (cp == 0)
	{
		Close(dfile);
		dfile = NULL;
		DeleteFile(newname);
		goto err1;
	}
	if (cp == -1) 
	{ 
		memerr(); 
		goto err2; 
	}

	Close(dfile);
	dfile = NULL;

	SetProtection (newname,fib->fib_Protection & ~FIBF_ARCHIVE);
	if (fib->fib_Comment[0]) 
		SetComment (newname,fib->fib_Comment);
	ChangeFileDate (newname,&fib->fib_Date);

	if (nopos && (dot = strstr(newname,".info")))
	{
		ch = *dot;
		*dot = 0;
		modify_icon(newname,NULL,NULL,0,1,0);
		*dot = ch;
	}

	result = 1;

err1:
	if (!result && !(local_delopts & DELOPT_NOFAIL) && !did_tree_error)
	{
		err_msg(ehead,ERR_DOS,FUNC_COPYFILES,err_cannot_copy, newname);
	}

err2:
	if (fib) 	FreeMem(fib,sizeof *fib);
	if (sfile) 	Close(sfile);
	if (dfile) 	Close(dfile);
	if (olddir) CurrentDir(olddir);
	if (dlock) 	UnLock(dlock);
	if (slock) 	UnLock(slock);

	return result;
}

int copy_tree_file (BPTR src, BPTR dest, struct FileInfoBlock *fib)
{
	BPTR				olddir;
	BPTR				sfile,
						dfile = NULL;
	int					result = 0;
	WORD				cp;
	ULONG				error;

	if (!strcmp(fib->fib_FileName,".info")) 
		return 1;

	olddir = CurrentDir(src);

	unless (sfile = Open(fib->fib_FileName,MODE_OLDFILE)) 
		goto err;

	CurrentDir(dest);

	unless (dfile = OpenNew(fib->fib_FileName,&error))
	{
		if (error == 0) 
		{ 
			result = 1; 
			goto err2; 
		}
		if ((error == ERROR_DELETE_PROTECTED 
		 ||	 error == ERROR_WRITE_PROTECTED 
		 ||	 error == ERROR_DISK_WRITE_PROTECTED) 
		&&	(local_delopts & DELOPT_OKNODELETE)) 
		{ 
			result = 1; 
			goto err2; 
		}
		
#ifdef ONLY2_0
		SetIoErr(error);
#else
		mySetIoErr(error);
#endif
		goto err;
	}

	cp = CopyFileWithStatus (sfile, 
							 dfile, 
							 show_gauge ? fib->fib_FileName : NULL,
							 dest, 
							 fib->fib_Size);

	if (cp == 0)
	{
		Close(dfile);
		dfile = NULL;
		DeleteFile (fib->fib_FileName);
		goto err;
	}
	else 
	if (cp < 0)
	{
		memerr();
		did_tree_error = TRUE;
		goto err2;
	}

	Close(dfile);
	dfile = NULL;

	SetProtection (fib->fib_FileName,fib->fib_Protection & ~FIBF_ARCHIVE);
	if (fib->fib_Comment[0]) 
		SetComment (fib->fib_FileName,fib->fib_Comment);
	ChangeFileDate (fib->fib_FileName,&fib->fib_Date);

	result = 1;

err:
	if ( !result && !did_tree_error && !(local_delopts & DELOPT_NOFAIL) )
	{
		err_msg (ehead,ERR_DOS,FUNC_COPYFILES,
				 err_cannot_copy, fib->fib_FileName);
		did_tree_error = TRUE;
	}

err2:
	if (sfile) 	Close(sfile);
	if (dfile) 	Close(dfile);
	if (olddir) CurrentDir(olddir);

	return result;
}

int copy_tree_dir(BPTR src,BPTR dest)
{
	struct FileInfoBlock	*fib,
							*dfib = NULL;
	BPTR					slock = NULL,
							lock = NULL,
							olddir = NULL;
	int						result = 0;

	if (fib = AllocMem(sizeof *fib,MEMF_PUBLIC|MEMF_CLEAR))
	{
		if (Examine(src,fib))
		{
			while (ExNext(src,fib))
			{
				if (fib->fib_EntryType < 0) /* file */
				{
					if (!copy_tree_file(src,dest,fib)) 
						goto err;
				}
				else 
				if (fib->fib_EntryType > 0) /* dir */
				{
					olddir = CurrentDir(src);
					if (slock = Lock(fib->fib_FileName,ACCESS_READ))
					{
						CurrentDir(dest);
						lock = Lock(fib->fib_FileName,ACCESS_READ);
						if (lock != NULL)
						{
							/* is it a directory? */
							
							unless (dfib = AllocMem(sizeof *dfib,MEMF_PUBLIC|MEMF_CLEAR))
							{
								result = -1;
								goto err;
							}

							if (!Examine(lock,dfib)) goto err;

							if (dfib->fib_DirEntryType < 0)
							{
								if ( !(local_delopts & DELOPT_NOFAIL) )
								{
									err_msg(ehead,ERR_DATA,FUNC_COPYFILES,
										err_overwrite, fib->fib_FileName);
									did_tree_error = TRUE;
								}
								goto err;
							}

							FreeMem(dfib,sizeof *dfib);
							dfib = NULL;
						}
						else
						{
							unless (lock = LockOrCreateDir(fib->fib_FileName))
								goto err;
						}

						if (!copy_tree_dir(slock,lock)) 
							goto err;

						UnLock(lock);
						lock = NULL;

						SetProtection(fib->fib_FileName,
							fib->fib_Protection & ~FIBF_ARCHIVE);
						if (fib->fib_Comment[0])
							SetComment(fib->fib_FileName,fib->fib_Comment);
						ChangeFileDate(fib->fib_FileName,&fib->fib_Date);

						UnLock(slock);
						slock = NULL;
					}

					CurrentDir(olddir);
					olddir = NULL;
				}

				if (check_escape() && aborterr(FUNC_COPYFILES))
				{
					did_tree_error = TRUE;
					goto err;
				}
			}

			result = 1;
		}
	}
	else result = -1;

err:
	if (did_tree_error == FALSE)
	{
		if (!result)
		{
			if ( !(local_delopts & DELOPT_NOFAIL) )
			{
				err_msg(ehead,ERR_DOS,FUNC_COPYFILES,
						err_cannot_copy, fib->fib_FileName);
				did_tree_error = TRUE;
			}
		}
		else 
		if (result < 0)
		{
			memerr();
			result = 0;
			did_tree_error = TRUE;
		}
	}

	if (olddir) CurrentDir(olddir);
	if (lock) 	UnLock(lock);
	if (slock) 	UnLock(slock);
	if (dfib) 	FreeMem(dfib,sizeof *dfib);
	if (fib) 	FreeMem(fib,sizeof *fib);

	return result;
}

int copy_tree(BPTR src,char *dest)
{
	BPTR				lock;
	int					result;

	did_tree_error = FALSE;

	unless (lock = LockOrCreateDir(dest))
	{
		if ( !(local_delopts & DELOPT_NOFAIL) )
			err_msg(ehead,ERR_DOS,FUNC_COPYFILES,err_cannot_create, dest);

		return 0;
	}

	result = copy_tree_dir(src,lock);

	UnLock(lock);
	return result;
}

/* the routine requires that the source dir be the current dir */

int copy_object(char *src,char *dir,char *name,WORD nopos)
{
	BPTR					lock,
							dlock,
							olock;
	struct FileInfoBlock	*fib;
	int						result,
							what;

	fib = MemAlloc(sizeof *fib,MEMF_CLEAR|MEMF_PUBLIC);
	if (fib == NULL)
	{
		memerr();
		return 0;
	}

	lock = Lock(src,ACCESS_READ);
	if (lock == NULL) 		
	{
err:	if ( !(local_delopts & DELOPT_OKNOSRC) )
		{
			err_msg(ehead,ERR_DOS,FUNC_COPYFILES,prob_with_source, src);
		}

		MemFree(fib);
		return 0;
	}

	if (Examine(lock,fib) == 0)
	{
		UnLock(lock);
		goto err;
	}

	what = fib->fib_DirEntryType;
	MemFree(fib);

	if (what < 0)		/* file */
	{
		UnLock(lock);

		result = copy_file(src,dir,name,nopos);
		return result;
	}

	/* directory */

	did_tree_error = FALSE;

	unless (dlock = LockOrCreateDir(dir))
	{
		if ( !(local_delopts & DELOPT_NOFAIL) )
			err_msg(ehead,ERR_DOS,FUNC_COPYFILES,err_cannot_create, dir);

		UnLock(lock);
		return 0;	
	}

	if (name == NULL) 
		name = FileOnly(src);

	olock = CurrentDir(dlock);
	result = copy_tree(lock,name);
	CurrentDir(olock);
	UnLock(dlock);
	UnLock(lock);
	return result;
}

/*==================== external command execution functions =================*/

extern void	**execute_dir_name;

BPTR set_execute_dir(void)
{
	BPTR	lock;
	char	*str;

	if (*execute_dir_name == NULL) 
		return 0;

	str = STR_VAL(*execute_dir_name);
	if (*str == 0) 
		return 0;

	if ( lock = Lock(str,ACCESS_READ) )
		return CurrentDir(lock);					/* pass back current dir */

	err_msg(ehead,ERR_DOS,FUNC_EXECUTE,err_execute_path);
	return (BPTR)-1;
}

int execute_command(char *command)
{
	BPTR				lock = NULL,
						nil  = Open("NIL:",MODE_NEWFILE);
	int					result;

	if (nil == NULL)
	{
		err_msg(ehead,ERR_DOS,FUNC_EXECUTE,err_no_nil);
		return -1;
	}

	lock = set_execute_dir();
	if (lock == (BPTR)-1) 
	{ 
		Close(nil); 
		return -1; 
	}

#ifndef ONLY2_0
	if (IntuitionBase->LibNode.lib_Version >= 36)
#endif
	{
		struct TagItem tags[4];

		tags[0].ti_Tag 	= SYS_Input;
		tags[1].ti_Tag 	= SYS_Output;
		tags[0].ti_Data = tags[1].ti_Data = (long)nil;
		tags[2].ti_Tag 	= SYS_UserShell;
		tags[2].ti_Data = TRUE;
		tags[3].ti_Tag 	= TAG_DONE;

		if ((result = SystemTagList(command,tags)) == -1)
		{
			err_msg(ehead,ERR_DOS,FUNC_EXECUTE,err_no_command);
		}
	}
#ifndef ONLY2_0
	else
	{
		if (Execute(command,nil,nil) == 0)
		{
			err_msg(ehead,ERR_DOS,FUNC_EXECUTE,err_no_command);
			result = -1;
		}
		else
		{
			result = 0; /* ??? however you get the REAL result */
		}
	}
#endif

	Close(nil);
	if (lock) 
		UnLock(CurrentDir(lock));				/* restore current directory */

	return result;
}

struct Library		*RexxSysBase = NULL;

int send_rexx (char *str)
{	static char 		rexxname[] = "REXX";
	struct MsgPort		OutPort,
						*ServerPort;			/* ARexx server port */
	struct RexxMsg		*rxmsg = NULL;
	BPTR				nil;
	int					result = -1;
	BOOL				sent = FALSE;

	unless (RexxSysBase = OpenLibrary("rexxsyslib.library",0))
	{
		err_msg(ehead,ERR_DATA,FUNC_REXX,err_no_rexx);
		return -1;
	}

	unless (nil = Open("NIL:",MODE_NEWFILE))
	{
		err_msg (ehead, ERR_DOS, FUNC_REXX, err_no_nil);
		goto bye;
	}

	unless (ServerPort = FindPort(rexxname))		/* port there? */
	{
		int i = Execute("rexxmast",nil,nil);

		if (i == 0)		/* execute failed */
		{
			err_msg(ehead,ERR_SCRIPT,FUNC_REXX,err_no_rexx);
			goto bye;
		}
	}

	NewList(&OutPort.mp_MsgList);
	OutPort.mp_SigTask = FindTask(0);
	OutPort.mp_SigBit  = SIGBREAKB_CTRL_C;
	OutPort.mp_Flags   = PA_SIGNAL;

	rxmsg = CreateRexxMsg(&OutPort,rexxname,rexxname);
	if (rxmsg == NULL)
	{
		memerr();
		goto bye;
	}

	rxmsg->rm_Args[0] = CreateArgstring(str,strlen(str));
	if (rxmsg->rm_Args[0] == NULL)
	{
		memerr();
		goto bye;
	}

	rxmsg->rm_Action = RXCOMM;
	rxmsg->rm_Stdin  = rxmsg->rm_Stdout = (long)nil;

	Forbid();										/* disable tasking */
	if (ServerPort = FindPort(rexxname))
	{
		PutMsg(ServerPort,(struct Message *)rxmsg);
		sent = TRUE;
	}
	Permit();

	if (sent == FALSE)
	{
		err_msg(ehead,ERR_SCRIPT,FUNC_REXX,err_no_rexx);
		goto bye;
	}

	while (TRUE)
	{
		Wait(SIGBREAKF_CTRL_C);
		if (GetMsg(&OutPort)) break;
	}

	result = rxmsg->rm_Result1;

bye:
	if (rxmsg)
	{
		if (rxmsg->rm_Args[0]) 
			DeleteArgstring(rxmsg->rm_Args[0]);
		DeleteRexxMsg(rxmsg);
	}
	if (nil) 
		Close(nil);
	if (RexxSysBase) 
		CloseLibrary(RexxSysBase);

	return result;
}

/*====================== DOS list management functions ======================*/

BOOL GetAssign(char *name,char *buffer,LONG opts)
{
	BPTR			lock;
	BOOL			result = FALSE;

	if (FindAssign(name,opts,&lock))
	{
		if (lock == NULL && opts == LDF_DEVICES)
		{
			strcpy(buffer,name);
			result = TRUE;			
		}
		else 
		if (lock != NULL)
		{
			ExpandPath((void *)lock,buffer,MAX_STRING);
			UnLock(lock);
			result = TRUE;
		}
	}

	return result;
}

BOOL add_assign (struct DosInfo *dinfo, char *name, BPTR lock)
{
	long						length = strlen(name),
								*buf,
								*buf2;
								
	struct DeviceNode			*dnode;
	char						*str;

	buf = AllocMem(sizeof *dnode + sizeof(long), MEMF_PUBLIC|MEMF_CLEAR);
	if (buf == NULL)
	{
bad:	memerr();
		return FALSE;
	}

	*buf = (sizeof *dnode + sizeof(long) ) + 7 & ~7;

	buf2 = AllocMem(length + 5, MEMF_PUBLIC|MEMF_CLEAR);
	if (buf2 == NULL)
	{
		FreeMem(buf,*buf);
		goto bad;
	}

	*buf2 = (length + 5) + 7 & ~7;
	str   = (char *)(buf2 + 1);

	dnode = (struct DeviceNode *)(buf + 1);
	dnode->dn_Type = DLT_DIRECTORY;
	dnode->dn_Lock = (BPTR)lock;
	dnode->dn_Name = (BPTR)CpyCstrBstr(name,str);
	dnode->dn_Task = ((struct FileLock *)(lock << 2))->fl_Task;

	Forbid();
	dnode->dn_Next = dinfo->di_DevInfo;
	dinfo->di_DevInfo = (BPTR)((LONG)dnode >> 2);
	Permit();

	return TRUE;
}

BOOL SetAssign(char *name, BPTR flock)
{
	BPTR						lock = NULL;
	int							result;

	if (flock)
	{
		lock = DupLock(flock);
		if (lock == NULL)
		{
			err_msg (ehead, ERR_DOS, FUNC_MAKEASSIGN, err_cannot_duplock);
			return 0;
		}
	}

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		result = AssignLock((UBYTE *)name,(BPTR)lock);
		if (result == 0 && flock) 
			UnLock(lock);
		return ((BOOL)(result ? TRUE : FALSE));
	}
#ifndef ONLY2_0
	else
	{
		struct RootNode	  	  		*rnode; 
		struct DosInfo	  		  	*dinfo;
		struct DeviceNode			*dnode,**dlast;
		UWORD						length;
		char						*str;
		BPTR						olock = NULL;
		long						*buf;

		rnode = (struct RootNode *)DOSBase->dl_Root;
		dinfo = (struct DosInfo  *)BADDR(rnode->rn_Info);

		length = strlen(name);

		Forbid();

		for(dlast=(struct DeviceNode **)&dinfo->di_DevInfo,
				dnode=(struct DeviceNode *)BADDR(dinfo->di_DevInfo);
				
			dnode;
			
			dlast=(struct DeviceNode **)&dnode->dn_Next,
 				dnode=(struct DeviceNode *)BADDR(dnode->dn_Next))
		{
			if (dnode->dn_Type == DLT_DIRECTORY)
			{
				str = (char *)BADDR(dnode->dn_Name);
				if (str[0] == length && !strnicmp(&str[1],name,length))
				{
					olock = dnode->dn_Lock;
					if (flock != NULL)
					{
						dnode->dn_Lock = lock;
					}
					else
					{
						*dlast = (struct DeviceNode *)dnode->dn_Next;
						buf = (long *)BADDR(dnode->dn_Name) - 1;
						FreeMem(buf,*buf);
						buf = (long *)dnode - 1;
						FreeMem(buf,*buf);
					}
					break;
				}
    		}
		}

		Permit();

		if (olock) 
			UnLock(olock);

		if (dnode != NULL || (dnode == NULL && flock == NULL)) 
			return 1;

		if (!add_assign(dinfo,name,lock))
		{
			UnLock(lock);
			return 0;
		}

		return 1;
	}
#endif
}

/*
	This function returns a new lock on a device node. It either calls
	DupLock() or sends a packet to get a lock on the root of a volume or
	device.
*/

BPTR lock_object(struct DeviceNode *dnode)
{
	struct StandardPacket	*sp;
	char					*root;
	BPTR					lock;

	if (dnode->dn_Lock) 			/* case of NOT volume or device */
		return DupLock(dnode->dn_Lock);

		/* so, we have to send a packet to lock ":" to dnode->dn_Task */
	root = AllocMem (sizeof *sp + 4, MEMF_PUBLIC|MEMF_CLEAR);
	if (root == NULL) 
		return NULL;

	root[0] = 1;
	root[1] = ':';

	sp = (struct StandardPacket *)(root + 4);
	InitSimplePacket(ACTION_LOCATE_OBJECT,sp);

	sp->sp_Pkt.dp_Arg2 = (LONG)root >> 2;
	sp->sp_Pkt.dp_Arg3 = ACCESS_READ;

	SendPacket(dnode->dn_Task,sp);

	lock = (BPTR)sp->sp_Pkt.dp_Res1;

	FreeMem(root,sizeof *sp + 4);

	return lock;
}

BOOL FindAssign(char *old, long opts, BPTR *theLock)
{
	struct DeviceNode	  		*dnode;
	BOOL						result = FALSE;

	*theLock = NULL;

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)LockDosList(opts | LDF_READ))
		{
			if (dnode = (struct DeviceNode *)FindDosEntry((void *)dnode,(UBYTE *)old,opts))
			{
				result = TRUE;
				*theLock = lock_object(dnode);
			}

			UnLockDosList(opts | LDF_READ);
		}
	}
#ifndef ONLY2_0
	else
	{	struct DeviceNode	dcopy;

		if (dnode = (struct DeviceNode *)LockDevList())
		{
			if (dnode = (struct DeviceNode *)FindDevEntry((void *)dnode,(UBYTE *)old,opts))
			{
				dcopy = *dnode;
			}

			UnLockDevList();
		}

		if (dnode)
		{
			result = TRUE;
			*theLock = lock_object(&dcopy);
		}
	}
#endif

	return result;
}

long MatchAssign(char *new,char *old,long opts)
{
	BPTR						lock;
	long						result = FALSE;

	if (FindAssign(old,opts,&lock))
	{
#ifndef ONLY2_0
		if (DOSBase->dl_lib.lib_Version >= 36)
#endif
		{
			unless (result = AssignLock((UBYTE *)new,(BPTR)lock))
				UnLock(lock);
		}
#ifndef ONLY2_0
		else
		{
			struct RootNode	  	  		*rnode; 
			struct DosInfo	  		  	*dinfo;

			rnode = (struct RootNode *)DOSBase->dl_Root;
			dinfo = (struct DosInfo  *)BADDR(rnode->rn_Info);

			if (!add_assign(dinfo,new,lock)) 
				UnLock(lock);
			else 
				result = TRUE;
		}
#endif
	}

	return result;
}

long VolumeMounted(char *name, long types)
{
	struct DeviceNode	  		*dnode;
	long						result = FALSE;

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)LockDosList(types | LDF_READ))
		{
			if (dnode = (struct DeviceNode *)
							FindDosEntry((void *)dnode,(UBYTE *)name,types))
			{
				if (dnode->dn_Task != NULL) 
					result = TRUE;
			}

			UnLockDosList(types | LDF_READ);
		}
	}
#ifndef ONLY2_0
	else
	{
		if (dnode = (struct DeviceNode *)LockDevList())
		{
			if (dnode = (struct DeviceNode *)
							FindDevEntry((void *)dnode,(UBYTE *)name,types))
			{
				if (dnode->dn_Task != NULL) 
					result = TRUE;
			}

			UnLockDevList();
		}
	}
#endif

	return result;
}


BOOL FindDOSDevice(char *name, char *buffer)
{
	struct DeviceNode	  		*dnode;
	BOOL						result = FALSE;
	struct MsgPort				*proc;

	proc = DeviceProc(name);
	if (proc == NULL) 
		return FALSE;

#ifndef ONLY2_0
	if (DOSBase->dl_lib.lib_Version >= 36)
#endif
	{
		if (dnode = (struct DeviceNode *)LockDosList(LDF_DEVICES | LDF_READ))
		{
			while (dnode = (struct DeviceNode *)NextDosEntry((void *)dnode,LDF_DEVICES))
			{
				if (dnode->dn_Task == proc)
				{
					CpyBstrCstr((void *)dnode->dn_Name,buffer);
					result = TRUE;
					break;
				}
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
				if (dnode->dn_Task == proc)
				{
					CpyBstrCstr((void *)dnode->dn_Name,buffer);
					result = TRUE;
					break;
				}
			}

			UnLockDevList();
		}
	}
#endif

	return result;
}

/*========================== change file date ============================*/

#ifndef ACTION_SET_FILE_DATE
#define ACTION_SET_FILE_DATE 34L
#endif

#define	ALIGNL(a)	((long)(a) + 3 & ~3)

long ChangeFileDate (char *name,struct DateStamp *ds)
{
	struct StandardPacket 	*packet;
	BPTR					lock,
							parent = NULL;
	long					result = 0;
	char 					*fname,
							bname[34];
	struct DateStamp		curds;	
	void					*handler;

	if (handler = DeviceProc(name))
	{
		lock = Lock(name,ACCESS_READ);
		if (lock) 
		{ 
			parent = ParentDir(lock); 
			UnLock(lock); 
		}

		if (!MakeClearStruct(packet)) 
			goto bye;

		if (ds == NULL) 
			DateStamp((void *)(ds = &curds));

		fname = (char *)ALIGNL(&bname[0]);
   		strcpy(fname+1,FileOnly(name));
   		*fname = strlen(fname+1);    			/* make it a BSTR */

   		InitSimplePacket(ACTION_SET_FILE_DATE,packet);
		packet->sp_Pkt.dp_Arg1 = NULL;
		packet->sp_Pkt.dp_Arg2 = (long)parent;
		packet->sp_Pkt.dp_Arg3 = (long)fname >> 2;
		packet->sp_Pkt.dp_Arg4 = (long)ds;

   		SendPacket(handler,packet);
    	result = packet->sp_Pkt.dp_Res1;		/* set error code */
bye:
		if (parent) 
			UnLock(parent);						/* free lock on parent */
		UnMakeStruct(packet);					/* de-alloc packet */
	}

	return result;
}

int RelabelDisk(char *old,char *new)
{
	struct StandardPacket 	*packet;
	long					result = 0;
	char 					*fname,
							bname[34];
	void					*handler;

	if (handler = DeviceProc(old))
	{
		if (!MakeClearStruct(packet)) 
			goto bye;

		fname = (char *)ALIGNL(&bname[0]);
   		strcpy(fname+1,new);
   		*fname = strlen(fname+1);    					/* make it a BSTR	*/

   		InitSimplePacket(ACTION_RENAME_DISK,packet);	/* relabel disk		*/
		packet->sp_Pkt.dp_Arg1 = (long)fname >> 2;

   		SendPacket(handler,packet);
    	result = packet->sp_Pkt.dp_Res1;				/* set error code	*/

bye:	UnMakeStruct(packet);							/* de-alloc packet	*/
	}

	return result;
}
