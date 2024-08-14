/*
 * $Id: LoadSym.c,v 40.2 93/02/25 15:26:11 mks Exp $
 *
 * $Log:	LoadSym.c,v $
 * Revision 40.2  93/02/25  15:26:11  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:39:30  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.4  91/11/07  15:44:38  mks
 * Added ugly kludge to skip the bogus symbols...
 *
 * Revision 1.3  91/10/17  18:16:17  mks
 * Forgot a typecast
 *
 * Revision 1.2  91/10/17  18:13:24  mks
 * Fixed output to not show the ".ld" and to check for very long file names
 *
 * Revision 1.1  91/10/17  15:45:07  mks
 * Initial revision
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

#include	"loadsym_rev.h"

/******************************************************************************/

#include	"sym.h"

#define	CHECKEOF(x)	if (((ULONG)&buf_from[x]) > ((ULONG)buf_end)) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
#define	GETLONG		getLong=*buf_from++; CHECKEOF(0)

/*
 * This command loads the symbol files generated from a build
 * into a special symbol memory area and
 */

/* This is the command template. */
#define TEMPLATE  "SYMBOL,REMOVE/S,SKIPBOGUS/S" VERSTAG

#define	OPT_SYMBOL	0
#define	OPT_REMOVE	1
#define	OPT_SKIPSYM	2
#define	OPT_COUNT	3

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
		struct	Sym_Sem	*sym;
			ULONG	tmp[2];

			Forbid();
			if (sym=(struct Sym_Sem *)FindSemaphore(SYM_NAME))
			{
				/*
				 * If the option to REMOVE the semaphore
				 * is set, we will remove it from the
				 * public list before we continue
				 * to optain it.  This makes sure that others
				 * will not try to obtain after we have...
				 */
				if (opts[OPT_REMOVE]) RemSemaphore((struct SignalSemaphore *)sym);
				ObtainSemaphore((struct SignalSemaphore *)sym);
			}

			/*
			 * If we are not removing the semaphore
			 * and we don't have one, we make one...
			 */
			if ((!opts[OPT_REMOVE]) && (!sym))
			{
				if (sym=AllocVec(sizeof(struct Sym_Sem),MEMF_PUBLIC|MEMF_CLEAR))
				{
					InitSemaphore((struct SignalSemaphore *)sym);
					strcpy(sym->SemName,SYM_NAME);
					sym->Sem.ss_Link.ln_Name=sym->SemName;
					sym->Sem.ss_Link.ln_Pri=-10;
					AddSemaphore((struct SignalSemaphore *)sym);
					ObtainSemaphore((struct SignalSemaphore *)sym);		/* Get it for us */
				}
			}
			Permit();

			if ((sym) && (opts[OPT_REMOVE]))
			{
				FreeVec(sym->Memory);
				FreeVec(sym);
				sym=NULL;
				rc=RETURN_OK;
			}
			else if ((sym) && (opts[OPT_SYMBOL]))
			{
			BPTR	map_file;

				/*
				 * Ok, we will load some symbols...
				 * First free the old ones.
				 */
				FreeVec(sym->Memory);
				sym->Memory=NULL;

				if (strlen((char *)opts[OPT_SYMBOL])<250)
				{
					strcpy(sym->Symbols,(char *)opts[OPT_SYMBOL]);
					strcat(sym->Symbols,".map");
					if (map_file=Open(sym->Symbols,MODE_OLDFILE))
					{
					BPTR	ld_file;

						strcpy(sym->Symbols,(char *)opts[OPT_SYMBOL]);
						strcat(sym->Symbols,".ld");
						if (ld_file=Open(sym->Symbols,MODE_OLDFILE))
						{
						ULONG	size;

							Seek(ld_file,0,OFFSET_END);
							size=Seek(ld_file,0,OFFSET_BEGINNING);
							if (sym->Memory=AllocVec(size,MEMF_PUBLIC))
							{
								PutStr(" Loading\r");
								if (size==Read(ld_file,sym->Memory,size))
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
									buf_from=sym->Memory;
									buf_to=sym->Memory;
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
													ULONG	*old_buf=buf_to;

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

														/*
														 * Some silly compilers put their local
														 * symbols as *GLOBAL* values!!!  Well,
														 * I will fix that here if the user
														 * has enabled it.
														 * These symbols are of the form "*_.*"
														 */
														if (opts[OPT_SKIPSYM])
														{
														char *chk;

															chk=(char *)&(old_buf[1]);
															while (*chk)
															{
																if ((chk[0]=='_') &&
																    (chk[1]=='.')    )
																{
																	/*
																	 * We found one, so
																	 * kill it.
																	 */
																	buf_to=old_buf;
																}
																chk++;
															}
														}
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
									 * Let the user know...
									 */
									strcpy(sym->Symbols,(char *)opts[OPT_SYMBOL]);
									tmp[0]=(ULONG)(sym->Symbols);
									tmp[1]=sym->Memory[-1];
									VPrintf("Symbols %s loaded: %ld bytes\n",tmp);

									/*
									 * Everything seemed to work so...
									 */
									rc=RETURN_OK;
								}
								else
								{
									rc=IoErr();

ReadError:								/*
									 * Drop into here on a read error
									 */
									PrintFault(rc,sym->Symbols);
									FreeVec(sym->Memory);
									sym->Memory=NULL;
									rc=RETURN_FAIL;
								}
							}
							else PrintFault(IoErr(),NULL);

							Close(ld_file);
						}
						else PrintFault(IoErr(),sym->Symbols);

						Close(map_file);
					}
					else PrintFault(IoErr(),sym->Symbols);
				}
				else PrintFault(ERROR_LINE_TOO_LONG,NULL);
			}
			else if (sym)
			{
				if (sym->Memory)
				{
					tmp[0]=(ULONG)(sym->Symbols);
					tmp[1]=sym->Memory[-1];
					VPrintf("Symbols %s loaded: %ld bytes\n",tmp);
				}
				else PutStr("No symbols loaded.\n");
				rc=RETURN_OK;
			}
			else if (!opts[OPT_REMOVE]) PutStr("Can not make symbol semaphore.\n");

			if (sym) ReleaseSemaphore((struct SignalSemaphore *)sym);
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
