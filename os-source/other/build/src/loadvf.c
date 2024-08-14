/*
 * $Id: LoadVF.c,v 40.2 93/02/25 15:26:18 mks Exp $
 *
 * $Log:	LoadVF.c,v $
 * Revision 40.2  93/02/25  15:26:18  mks
 * No longer uses the internal (private) include
 * 
 * Revision 40.1  93/02/18  15:39:38  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.15  92/04/29  19:33:24  mks
 * Changed the VBuf sizes a bit...
 *
 * Revision 1.14  92/04/29  19:17:40  mks
 * Added calls to SetVBuf() which should make builds a bit faster under
 * V39.
 *
 * Revision 1.13  91/12/06  08:44:29  mks
 * Removed dead assignment (don't like warnings)
 *
 * Revision 1.12  91/12/06  07:19:50  mks
 * Added support for CDTV_E
 *
 * Revision 1.11  91/11/14  19:48:42  mks
 * Now checks for control-C and strips local (.) labels that should
 * not be made global.  (Like in Intuition)
 *
 * Revision 1.10  91/10/21  10:35:33  mks
 * Added support for BSS hunks...  They now produce nasty warnings!
 *
 * Revision 1.9  91/10/18  15:36:42  mks
 * Added CDTV support and changed RELOC to be a HEX string
 *
 * Revision 1.8  91/10/17  10:21:13  mks
 * Cleaned up code formatting so that it will print nice
 *
 * Revision 1.7  91/10/16  13:13:58  mks
 * Added ROM-Tag search and module version/date display for the log
 * Cleaned up the kickety-split display
 *
 * Revision 1.6  91/10/15  10:19:51  mks
 * Added standard version string to the commands
 *
 * Revision 1.5  91/10/14  15:40:34  mks
 * Final kickitysplit version...
 *
 * Revision 1.4  91/10/11  20:48:29  mks
 * Added complete symbol file generation
 *
 * Revision 1.3  91/10/10  19:58:05  mks
 * Got the map file generation working...
 *
 * Revision 1.2  91/10/10  16:58:55  mks
 * Updated to use more memory but less time during the build.
 *
 * Revision 1.1  91/09/16  15:41:38  mks
 * Loads reloc table in one swallow - should help network speed a bit.
 *
 * Revision 1.0  91/09/16  14:55:20  mks
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
#include	<dos/stdio.h>

#include	"loadvf_rev.h"

/******************************************************************************/

#include	"vf.h"

#define	CHECKEOF(x)	if (((ULONG)&bufferpos[x]) > ((ULONG)bufferend)) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
#define	GETLONG	getLong=*bufferpos++; CHECKEOF(0)

#define	KICK_SPLIT	"   Kickety-Split ------- @ $FC0000-$FC0008 Size:%7ld bytes skipped\n"
#define	LOAD_STAT	"%16s %-7s @ $%06lx-$%06lx Size:%7ld  %s\n"

/*
 * This command loads an AmigaDOS load file into the VF space...
 */

/*
 * The .map file contains the global hunk number, address, and size...
 *
 * Output format is:  "%3ld %8lx %8lx\n",number,address,size
 */

/* This is the command template. */
#define TEMPLATE  "FILES/M/A,RELOC/K,SYMBOL/K,MAP/K,LOG/K,SPLIT/S,CDTV/S,CDTV_E/S" VERSTAG

#define	OPT_FILES	0
#define	OPT_RELOC	1
#define	OPT_SYMBOL	2
#define	OPT_MAP		3
#define	OPT_LOG		4
#define	OPT_SPLIT	5
#define	OPT_CDTV	6
#define	OPT_CDTV_E	7
#define	OPT_COUNT	8

LONG cmd_loadvf(void)
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
		struct	VF_Sem	*vf;
			ULONG	reloc_pos;
			ULONG	full_size;

			/*
			 * Set the default reloc
			 */
			full_size=FULL_SIZE;
			reloc_pos=0x00F80000;	/* Default relocation position */
			if (opts[OPT_CDTV])
			{
				reloc_pos=0x00F00000;
				full_size=(256*1024/4);	/* CDTV is only 256K */
			}
			if (opts[OPT_CDTV_E])
			{
				reloc_pos=0x00E00000;
				full_size=(256*1024/4);	/* CDTV is only 256K */
			}

			/*
			 * User selected a reloc position, so use it if it is valid...
			 */
			if (opts[OPT_RELOC])
			{
			ULONG	val;
			ULONG	c;
			char	*p;

				val=0;
				p=(char *)opts[OPT_RELOC];

				/*
				 * The relocation value is a HEX string...
				 * Convert it or return original number
				 * string is invalid.
				 */
				while (*p)
				{
					c=(ULONG)*p;
					if ((c>='a') && (c<='f')) c-=32;
					c-='0';
					if (c>9)
					{
						c-=7;
						if (c<10) c=16;
					}

					if (c<16)
					{
						val=(val << 4) + c;
						p++;
					}
					else
					{
						val=reloc_pos;
						p=&p[strlen(p)];
					}
				}
				reloc_pos=val;
			}

			/*
			 * We need to make sure that no one gets
			 * in here for a bit...
			 */
			Forbid();
			if (vf=(struct VF_Sem *)FindSemaphore(VF_NAME))
			{
				ObtainSemaphore((struct SignalSemaphore *)vf);
			}
			Permit();

			if (vf)
			{
			BPTR	sym_map=NULL;
			BPTR	sym_ld=NULL;
			BPTR	log_file=NULL;
			char	**files;

				/*
				 * Now check for map file options
				 */
				if (opts[OPT_MAP])
				{
					/* Now try to open/build the map file */
					if (sym_map=Open((char *)opts[OPT_MAP],MODE_READWRITE))
					{
						Seek(sym_map,0,OFFSET_END);
						/*
						 * Buffer for the symbol map file
						 */
						SetVBuf(sym_map,NULL,BUF_FULL,4096);
					}
					else PrintFault(IoErr(),"Could not open MAP file");
				}

				/*
				 * Now check for symbol file options
				 */
				if (opts[OPT_SYMBOL])
				{
					/* Now try to open/build the symbol file */
					if (sym_ld=Open((char *)opts[OPT_SYMBOL],MODE_READWRITE))
					{
						Seek(sym_ld,0,OFFSET_END);
						/*
						 * Make a reasonable buffer for writing symbol the file
						 */
						SetVBuf(sym_ld,NULL,BUF_FULL,8192);
					}
					else PrintFault(IoErr(),"Could not open SYMBOL file");
				}

				/*
				 * Now check for log file options
				 */
				if (opts[OPT_LOG])
				{
					/* Now try to open/build the log file */
					if (log_file=Open((char *)opts[OPT_LOG],MODE_READWRITE))
					{
						Seek(log_file,0,OFFSET_END);
					}
					else PrintFault(IoErr(),"Could not open LOG file");
				}

				/*
				 * Ok, so now we need to do the hard stuff
				 * for each file...
				 */
				rc=RETURN_OK;

				for (files=(char **)opts[OPT_FILES];(*files) && (!rc);files++)
				{
				BPTR	fh;
				char	module[256];	/* Module name buffer and symbol buffer */
				char	*file;
				ULONG	loop;
				BOOL	BSS_Error;

					/*
					 * Clear the BSS error flag...  (This is used to kludge
					 * around the BSS errors that CDTV has)
					 */
					BSS_Error=FALSE;

					/*
					 * Generate the module name
					 */
					file=module;
					strcpy(file,FilePart(*files));
					for (loop=0;file[loop];loop++) if (file[loop]=='.') file[loop]='\0';

					loop=strlen(module);
					module[loop]='_';
					file=&module[loop+1];

					if (fh=Open(*files,MODE_OLDFILE))
					{
					ULONG	getLong;
					ULONG	*buffer;
					ULONG	buffersize;

						Seek(fh,0,OFFSET_END);
						buffersize=Seek(fh,0,OFFSET_BEGINNING);
						if (buffer=AllocVec(buffersize,MEMF_PUBLIC))
						{
							if (buffersize==Read(fh,buffer,buffersize))
							{
							ULONG	*bufferpos;
							ULONG	*bufferend;
							ULONG	*table=NULL;
							ULONG	tableSize;
							ULONG	firstHunk;
							ULONG	lastHunk;
							ULONG	hunk;
							ULONG	total;
							ULONG	hunkType;
							ULONG	flag;
							ULONG	start_pos;
							ULONG	tmp[6];
							UWORD	*look;
							ULONG	count;

								bufferpos=buffer;
								bufferend=(ULONG *)(((ULONG)buffer)+buffersize);

								start_pos=((vf->LastPos)*4)+reloc_pos;

								GETLONG;	/* Check for hunk header... */
								if (getLong != HUNK_HEADER) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

								GETLONG;	/* Check for resident lib */
								if (getLong != 0) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }

								GETLONG;	/* Get table size */
								tableSize=getLong*4;
								if (!(table=AllocVec(tableSize,MEMF_CLEAR))) { rc=IoErr(); goto ReadError; }

								GETLONG;	/* Get first hunk slot */
								firstHunk=getLong;

								GETLONG;	/* Last hunk slot */
								lastHunk=getLong;

								total=0;
								for (hunk=firstHunk;hunk<=lastHunk;hunk++)
								{
									GETLONG;	/* Get next hunk size */
									getLong&=0x3FFFFFFF;

									/*
									 * Check for kickity-split option...
									 */
									if (opts[OPT_SPLIT])
									{
									ULONG	split;

										split=total+vf->LastPos;
										if (split<=65536L) if ((split+getLong)>65535L)
										{
										ULONG	*p;

											/*
											 * Ok, we need to split here...
											 * How much do we need to move the hunk?
											 */
											split=(65536L-split)+2L;
											total+=split;
											p=vf->Memory;
											p=(ULONG *)(((ULONG)p) + (4L * 65536L));
											p[0]=0x11114EF9;	/* fill...   JMP     */
											p[1]=0x00F80002;	/*           F80002  */

											tmp[0]=split*4;
											VPrintf(KICK_SPLIT,tmp);
											if (log_file) VFPrintf(log_file,KICK_SPLIT,tmp);
										}
									}

									table[hunk]=(ULONG)&(vf->Memory[total+vf->LastPos]);

									total+=getLong;
									if ((total+vf->LastPos)>(full_size-7))
									{
										rc=ERROR_OBJECT_TOO_LARGE;
										goto ReadError;
									}

									/*
									 * Check if we are outputing a symbol map file...
									 */
									if (sym_map)
									{
										tmp[0]=vf->LastHunk;
										tmp[1]=table[hunk]-((ULONG)(vf->Memory))+reloc_pos;
										tmp[2]=getLong << 2;

										VFPrintf(sym_map,"%3ld %8lx %8lx\n",tmp);
									}
									vf->LastHunk+=1;
								}

								hunk=firstHunk;
								while ((!rc)&&(hunk<=lastHunk))
								{
									if (bufferpos != bufferend)
									{
										GETLONG;
										hunkType=getLong & 0x3FFFFFFF;

										switch (hunkType)
										{
										case	HUNK_CODE:
										case	HUNK_DATA:
											/*
											 * If we are making a symbol LD file, write
											 * out an empty code hunk
											 */
											if (sym_ld)
											{
												tmp[0]=hunkType;
												tmp[1]=0;
												FWrite(sym_ld,tmp,8,1);
											}
											GETLONG;
											if (getLong)
											{
												CHECKEOF(getLong);
												CopyMemQuick((VOID *)bufferpos,(VOID *)(table[hunk]),getLong*4);
												bufferpos=&bufferpos[getLong];
											}
											break;

										case	HUNK_RELOC32:
											flag=TRUE;
											while (flag)
											{
											ULONG	relnum;
											ULONG	hunknum;
											ULONG	*p;
											ULONG	*rel;

												GETLONG;
												if (relnum=getLong)
												{
													GETLONG;
													hunknum=getLong;
													if (hunknum>lastHunk) { rc=ERROR_FILE_NOT_OBJECT; goto ReadError; }
													rel=bufferpos;
													bufferpos=&bufferpos[relnum];
													CHECKEOF(0);

													while (relnum--)
													{
														/* Relocate to the kickstart area... */
														p=(ULONG *)(table[hunk]+rel[0]);
														/*
														 * Check for trashed RELOC information
														 */
														if ( (((ULONG)p)<((ULONG)(vf->Memory))) ||
														     (((ULONG)p)>((ULONG)&(vf->Memory[full_size]))) )
														{
															rc=ERROR_FILE_NOT_OBJECT; goto ReadError;
														}
														*p+=table[hunknum]+reloc_pos-((ULONG)(vf->Memory));
														rel=&rel[1];
													}
												}
												else flag=FALSE;
											}
											break;

										case	HUNK_DEBUG:
										case	HUNK_NAME:
											GETLONG;
											bufferpos=&bufferpos[getLong];
											CHECKEOF(0);
											break;

										case	HUNK_SYMBOL:
											/*
											 * Check for symbol file, and if so, write
											 * the NULL word to end the table
											 */
											if (sym_ld)
											{
												tmp[0]=hunkType;
												FWrite(sym_ld,tmp,4,1);
											}
											while (getLong)
											{
												GETLONG;
												if (getLong)
												{
													/*
													 * Check for symbol file, and if so, write
													 * this symbol information...
													 */
													if (sym_ld)
													{
														/*
														 * First, we check if the symbol is
														 * a "." symbol.  All "." symbols are
														 * skipped.  (These are "hidden")
														 */
														if ((*((char *)bufferpos))!='.')
														{
															/*
															 * Here we kludge a bit...
															 * We assume that there is a NULL byte
															 * after the symbol name.  Usually there
															 * must be as it is padded to the longword.
															 * But, when there is not, we assume that
															 * the symbol value will be less than 16meg
															 * such that the high byte is 0 and thus
															 * giving us the NULL we need.
															 * Our output symbols will have the
															 * NULL counted such that we always have at
															 * least one NULL in the symbol name
															 */
															strcpy(file,(char *)bufferpos);
															loop=strlen(module);
															while ((loop&3)<3) module[loop++]='\0';
															module[loop++]='\0';
															tmp[0]=loop>>2;
															/*
															 * Now, write the data:
															 * The size, in long words,
															 * The symbol name,
															 * The symbol value
															 */
															FWrite(sym_ld,tmp,4,1);
															FWrite(sym_ld,module,loop,1);
															FWrite(sym_ld,&bufferpos[getLong],4,1);
														}
													}
													bufferpos=&bufferpos[getLong+1];
													CHECKEOF(0);
												}
											}
											/*
											 * Check for symbol file, and if so, write
											 * the NULL word to end the table
											 */
											if (sym_ld)
											{
												tmp[0]=0;
												FWrite(sym_ld,tmp,4,1);
											}
											break;

										case	HUNK_END:
											/*
											 * If we are making a symbol LD file, write
											 * out the hunkend as needed...
											 */
											if (sym_ld)
											{
												tmp[0]=hunkType;
												FWrite(sym_ld,tmp,4,1);
											}
											hunk++;
											break;

										case	HUNK_BSS:
											/*
											 * If we get a BSS hunk, we are not ROM code.
											 * However, due to a "bug" in the CDTV files,
											 * they have BSS hunks.  In order to keep them
											 * working, I will accept BSS hunks but will
											 * report a WARNING about BSS and ROM...
											 */
											bufferpos++; CHECKEOF(0);
											BSS_Error=TRUE;
											break;

										default:	VPrintf("Death due to hunk %lx\n",&hunkType);
												rc=ERROR_OBJECT_WRONG_TYPE; goto ReadError;
										}
									}
									if (SetSignal(0L,0L) & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)) goto ReadError;
								}

								/*
								 * Now put together the arguments for the log output
								 */
								tmp[0]=(ULONG)module;
								file[-1]='\0';		/* NULL terminal the module name */

								/*
								 * Now do ROM-Tag Search
								 * We will place the version information in the buffer right
								 * after the module name...
								 */
								strcpy(file,"-------");
								tmp[5]=tmp[1]=(ULONG)file;

								look=(UWORD *)&(vf->Memory[vf->LastPos]);
								for (count=0;count<(total*2);count++)
								{
									if (look[count]==RTC_MATCHWORD)
									{
									struct	Resident	*romtag;

										romtag=(struct Resident *)&look[count];
										if (((ULONG)(romtag->rt_MatchTag))==(((ULONG)romtag)+reloc_pos-((ULONG)(vf->Memory))))
										{
										char	*check;

											/*
											 * Ok, we found the ROM-Tag, so lets check the IdString
											 * and display the version and date from it if it is good
											 */
											check=(char *)(romtag->rt_IdString)-reloc_pos+((ULONG)(vf->Memory));
											while ((*check) && (*check!=' ')) check++;
											if (*check)
											{
												/*
												 * Check for a number...  Invalid if no number
												 */
												check++;
												if ((*check>='0') && (*check<='9'))
												{
													strcpy(file,check);

													/*
													 * Get rid of the CR/LF...
													 */
													file[strlen(file)-2]='\0';

													/*
													 * Now look for the date (if it is there)
													 */
													check=file;
													while ((*check) && (*check!=' ')) check++;
													if (*check) *check++='\0';
													tmp[5]=(ULONG)check;

													/*
													 * Exit the search loop
													 */
													count=total*2;
												}
											}
										}
									}
								}

								tmp[2]=start_pos;
								tmp[3]=start_pos+(tmp[4]=total*4);

								if (BSS_Error) tmp[5]=(ULONG)((char *)("** WARNING **\n!!! Module has BSS hunks !!!\n"));

								VPrintf(LOAD_STAT,tmp);
								if (log_file) VFPrintf(log_file,LOAD_STAT,tmp);

								/*
								 * Move the last position pointer in the structure
								 */
								vf->LastPos+=total;

ReadError:
								FreeVec(table);
							}
							else rc=IoErr();
							FreeVec(buffer);
						}
						else rc=IoErr();
						Close(fh);
					}
					else rc=IoErr();

					if (rc)
					{
						PrintFault(rc,NULL);
						rc=RETURN_FAIL;
					}
					else
					{
						if (SetSignal(0L,0L) & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D))
						{
							PutStr("***Break\n");
							rc=RETURN_FAIL;
						}
					}
				}

				ReleaseSemaphore((struct SignalSemaphore *)vf);

				if (log_file) Close(log_file);
				if (sym_ld) Close(sym_ld);
				if (sym_map) Close(sym_map);
			}
			else PutStr("Can not find VF Semaphore.\n");

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
