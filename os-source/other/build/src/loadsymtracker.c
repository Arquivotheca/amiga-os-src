/*
 * $Id: LoadSymTracker.c,v 40.2 93/03/03 09:07:32 mks Exp $
 *
 * $Log:	LoadSymTracker.c,v $
 * Revision 40.2  93/03/03  09:07:32  mks
 * Added CHIP keyword to load symbols into CHIP memory...
 * 
 * Revision 40.1  93/03/02  10:47:05  mks
 * First release
 *
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/tasks.h>
#include <exec/memory.h>
#include <exec/alerts.h>

#include <string.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include <utility/tagitem.h>
#include <clib/utility_protos.h>
#include <pragmas/utility_pragmas.h>

#include <dos/dosextens.h>
#include <dos/datetime.h>
#include <dos/dosasl.h>
#include <dos/rdargs.h>

#define DOSLIB	   "dos.library"
#define DOSVER	   37L

#define OPENFAIL { Result2(ERROR_INVALID_RESIDENT_LIBRARY); }
#define EXECBASE (*(struct ExecBase **)4)

#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))
#define Result2(x)  THISPROC->pr_Result2 = x

/******************************************************************************/

#include	<exec/ports.h>
#include	<exec/libraries.h>
#include	<exec/semaphores.h>
#include	<exec/resident.h>

#include	<dos/doshunks.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>

#include	"loadsymtracker_rev.h"

/******************************************************************************/

#include	"sym.h"

#define	CHECKEOF(x)	if (((ULONG)&buf_from[x]) > ((ULONG)buf_end)) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
#define	GETLONG		getLong=*buf_from++; CHECKEOF(0)

struct	SegNode
{
struct	MinNode		seg_Node;
	char		*seg_Name;
	ULONG		seg_Address;
	ULONG		seg_Size;
	ULONG		seg_NULL;
};

struct	SegSem
{
struct	SignalSemaphore	seg_Semaphore;
	ULONG		seg_PAD1;
struct	MinList		seg_List;
};

/*
 * This command loads the symbol files generated from a build
 * into a special symbol memory area and
 */

/* This is the command template. */
#define TEMPLATE  "SYMBOL/A,CHIP/S" VERSTAG

#define	OPT_SYMBOL	0
#define	OPT_CHIP	1
#define	OPT_COUNT	2

LONG cmd_loadsym(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
	LONG		rc;

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
	struct	RDArgs	*rdargs;
		LONG	opts[OPT_COUNT];

		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else
		{
		struct	SegSem	*sem;
			ULONG	tmp[2];
			ULONG	*Memory;
			char	Symbols[256];

			{
			BPTR	map_file;

				if (strlen((char *)opts[OPT_SYMBOL])<250)
				{
					strcpy(Symbols,(char *)opts[OPT_SYMBOL]);
					strcat(Symbols,".map");
					if (map_file=Open(Symbols,MODE_OLDFILE))
					{
					BPTR	ld_file;

						strcpy(Symbols,(char *)opts[OPT_SYMBOL]);
						strcat(Symbols,".ld");
						if (ld_file=Open(Symbols,MODE_OLDFILE))
						{
						ULONG	size;

							Seek(ld_file,0,OFFSET_END);
							size=Seek(ld_file,0,OFFSET_BEGINNING);
							if (Memory=AllocVec(size,MEMF_PUBLIC))
							{
								PutStr(" Loading\r");
								if (size==Read(ld_file,Memory,size))
								{
								ULONG	*buf_from;
								ULONG	*buf_to;
								ULONG	*buf_end;
								ULONG	firstHunk;
								ULONG	lastHunk;
								ULONG	hunk;
								ULONG	hunkType;
								ULONG	reloc;
								ULONG	getLong;
								BOOL	newhunk;

									/*
									 * Ok, we have now loaded the symbol file
									 * into memory.  Next comes adjusting it to
									 * match the ROM addresses for the hunks
									 */
									buf_from=Memory;
									buf_to=Memory;
									buf_end=(ULONG *)(((ULONG)buf_from)+size);


									GETLONG;	/* Check for hunk header... */
									if (getLong != HUNK_HEADER) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

									GETLONG;	/* Check for resident lib */
									if (getLong != 0) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

									GETLONG;	/* Get table size */
									CHECKEOF(2);
									firstHunk=buf_from[0];		/* Get First Hunk */
									lastHunk=buf_from[1];		/* Get Last Hunk  */
									buf_from=&buf_from[getLong+2];	/* Skip whole table */
									CHECKEOF(0);

									hunk=firstHunk;
									newhunk=TRUE;

									while (hunk<=lastHunk)
									{
										if (buf_from != buf_end)
										{
											if (newhunk)
											{
											char	MapLine[32];

												/*
												 * We are getting a new hunk so we need to read the next line
												 * from the map file
												 */
												if (FGets(map_file,MapLine,31))
												{
												char	*p;
												ULONG	h_num;
												ULONG	c;

													p=MapLine;

													/* Skip leading spaces */
													while (*p==' ') p++;

													/* Get hunk number  (decimal) */
													h_num=0;
													while (*p!=' ')
													{
														c=(ULONG)(*p-'0');
														if (c>9)
														{
															PutStr("Invalid .map file\n");
															rc=ERROR_OBJECT_WRONG_TYPE;
															goto ReadError;
														}
														else
														{
															h_num=(h_num*10) + c;
															p++;
														}
													}
													if (h_num!=hunk)
													{
														PutStr(".map file hunks do not match .ld file\n");
														rc=ERROR_OBJECT_WRONG_TYPE;
														goto ReadError;
													}
													/*
													 * Now get the relocation information...
													 */
													while (*p==' ') p++;
													reloc=0;
													while ((*p!=' ') && (*p))
													{
														c=(ULONG)(*p-'0');
														if (c>9)
														{
															c-=7;
															if (c<10) c=16;
														}
														if (c>16)
														{
															PutStr("Invalid .map file\n");
															rc=ERROR_OBJECT_WRONG_TYPE;
															goto ReadError;
														}
														else
														{
															reloc=(reloc<<4) + c;
															p++;
														}
													}
												}
												else
												{
													if (!(rc=IoErr())) rc=ERROR_OBJECT_WRONG_TYPE;
													goto ReadError;
												}
												newhunk=FALSE;
											}

											GETLONG;
											hunkType=getLong & 0x3FFFFFFF;

											switch (hunkType)
											{
											case	HUNK_SYMBOL:
												while (getLong)
												{
													GETLONG;
													if (getLong)
													{
														/*
														 * Now, copy this to the front, making sure
														 * we add the reloc value to the offset
														 */
														*buf_to++=getLong;	/* The size */
														CHECKEOF(getLong);
														while (getLong--) *buf_to++=*buf_from++;
														GETLONG;
														getLong+=reloc;
														*buf_to++=getLong;	/* Make sure getLong is non-null */
													}
												}
												break;

											case	HUNK_END:
												/*
												 * If we are making a symbol LD file, write
												 * out the hunkend as needed...
												 */
												hunk++;
												newhunk=TRUE;
												break;

											case	HUNK_CODE:
											case	HUNK_DATA:
											case	HUNK_DEBUG:
											case	HUNK_NAME:
												GETLONG;
												buf_from=&buf_from[getLong];
												CHECKEOF(0);
												break;

											case	HUNK_RELOC32:
												GETLONG;
												while (getLong)
												{
													buf_from=&buf_from[getLong+1];
													CHECKEOF(0);
													GETLONG;
												}
												break;

											default:	VPrintf("Death due to hunk %lx\n",&hunkType);
													rc=ERROR_OBJECT_WRONG_TYPE; goto ReadError;
											}
										}
									}

									/*
									 * Now, Make sure that the NULL end marker is visible...
									 */
									*buf_to++=NULL;
									*buf_to=NULL;

									/*
									 * Now install them over in the "other" place
									 */
									if (sem=(struct SegSem *)FindSemaphore("SegTracker"))
									{
										ULONG	addr;
										ULONG	*mem;
										ULONG	size=0;
										ULONG	size1=0;
									struct	SegNode	*new;
									struct	SegNode	*top;
									struct	SegNode	*last=NULL;
										char	*strings;
										char	*c;

										mem=Memory;

										while (*mem)
										{
											hunk=*mem++;

											size+=1+strlen((char *)mem);
											size1+=sizeof(struct SegNode);

											mem=&mem[hunk+1];
										}

										if (new=AllocVec(size1+size,MEMF_CLEAR|MEMF_PUBLIC|(opts[OPT_CHIP] ? MEMF_CHIP:MEMF_ANY)))
										{
											strings=(char *)(((ULONG)new)+size1);
											mem=Memory;
											while (hunk=*mem)
											{
												mem++;

												new->seg_Name=strings;
												new->seg_Address=addr=mem[hunk]-4;
												new->seg_Size=4;

												c=(char *)mem;
												while (*strings++=*c++);

												mem=&mem[hunk+1];

												Forbid();
												if (!last)
												{
													AddTail((struct List *)&(sem->seg_List),(struct Node *)new);
													last=top=new;
												}
												else
												{
													if (last->seg_Address > addr)
													{
														while (last != new)
														{
															if (last->seg_Address > addr)
															{
																last=(struct SegNode *)(last->seg_Node.mln_Succ);
																if (!last->seg_Node.mln_Succ)
																{
																	AddTail((struct List *)&(sem->seg_List),(struct Node *)new);
																	last=new;
																}
															}
															else
															{
																Insert((struct List *)&(sem->seg_List),
																	(struct Node *)new,
																	(struct Node *)(last->seg_Node.mln_Pred));
																last=new;
															}
														}
													}
													else
													{
														while (last!=new)
														{
															if (last->seg_Address > addr)
															{
																Insert((struct List *)&(sem->seg_List),
																	(struct Node *)new,
																	(struct Node *)last);
																last=new;
															}
															else
															{
																if (last==top)
																{
																	Insert((struct List *)&(sem->seg_List),
																		(struct Node *)new,
																		(struct Node *)(last->seg_Node.mln_Pred));
																	last=top=new;
																}
																else last=(struct SegNode *)(last->seg_Node.mln_Pred);
															}
														}
													}
												}
												Permit();

												new++;
											}

											while (last=(struct SegNode *)(top->seg_Node.mln_Succ))
											{
												if (last->seg_Node.mln_Succ) last->seg_Size=top->seg_Address-last->seg_Address+4;
												top=last;
											}

											/*
											 * Let the user know...
											 */
											strcpy(Symbols,(char *)opts[OPT_SYMBOL]);
											tmp[0]=(ULONG)(Symbols);
											tmp[1]=size+size1+4;
											VPrintf("Symbols %s loaded: %ld bytes\n",tmp);
											rc=RETURN_OK;
										}
										else PrintFault(IoErr(),NULL);
									}
									else PutStr("SegTracker not installed\n");
								}
								else
								{
									rc=IoErr();

ReadError:								/*
									 * Drop into here on a read error
									 */
									PrintFault(rc,Symbols);
									rc=RETURN_FAIL;
								}

								FreeVec(Memory);
							}
							else PrintFault(IoErr(),NULL);

							Close(ld_file);
						}
						else PrintFault(IoErr(),Symbols);

						Close(map_file);
					}
					else PrintFault(IoErr(),Symbols);
				}
				else PrintFault(ERROR_LINE_TOO_LONG,NULL);
			}

			FreeArgs(rdargs);
		}

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}
