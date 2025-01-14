/*
 * $Id: split.c,v 40.5 93/04/13 12:44:01 mks Exp $
 *
 * $Log:	split.c,v $
 * Revision 40.5  93/04/13  12:44:01  mks
 * updated to match real free ROM space...
 * 
 * Revision 40.4  93/03/30  11:33:38  mks
 * Added the SIZESONLY keyword for use in ROMSPACE Watchdog
 *
 * Revision 40.3  93/03/03  14:14:20  mks
 * Split now is a bit more flexible in building the
 * split files, which will be required for the larger
 * multi-512K kickstarts.
 *
 * Revision 40.2  93/02/25  15:26:50  mks
 * No longer uses the internal (private) include
 *
 * Revision 40.1  93/02/18  15:40:09  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.2  93/02/18  09:51:04  mks
 * Added the TO keyword for building new buildlists...
 *
 * Revision 1.1  93/02/17  17:46:52  mks
 * Initial revision
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
#include	<exec/lists.h>
#include	<exec/nodes.h>

#include	<dos/doshunks.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	<dos/stdio.h>

#include	<string.h>

#include	"split_rev.h"

/******************************************************************************/

#define NEWLIST(l) {((struct MinList *)l)->mlh_Head = (struct MinNode *)&(((struct MinList *)l)->mlh_Tail);\
                    ((struct MinList *)l)->mlh_Tail = NULL;\
                    ((struct MinList *)l)->mlh_TailPred = (struct MinNode *)l;}

#define	FILL_BUFFER(x,y)	{long z; strcpy(buffer,x); z=strlen(buffer); while(z<64) buffer[z++]=y; buffer[z]='\0';}

/*
 * Number of external loops
 */
#define	OUTSIDE		4

/*
 * Maximum energy loop
 */
#define	RETRY_LOOP	3

/* This is the command template. */
#define TEMPLATE  "BUILDLIST/A,DIR/K,EXTRA/K/N,SUPERKICK/S,TO/K,SPLIT/K/N,SIZE/K/N,ANYFIT/S,FIRSTHALF/K,SECONDHALF/K,SIZESONLY/S" VERSTAG

#define	OPT_BUILDLIST	0
#define	OPT_DIR		1
#define	OPT_EXTRA	2
#define	OPT_SUPERKICK	3
#define	OPT_TO		4
#define	OPT_SPLIT	5
#define	OPT_SIZE	6
#define	OPT_ANYFIT	7
#define	OPT_FIRSTHALF	8
#define	OPT_SECONDHALF	9
#define	OPT_SIZESONLY	10
#define	OPT_COUNT	11

struct	Module
{
struct	Node	m_Node;
	ULONG	m_Size;
	char	m_Name[1];
};

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
		BPTR	buildFile;
		char	*Directory="V:";
		ULONG	FullSize=(131072-10)*4;
		ULONG	Split=65536*4;

			if (opts[OPT_DIR]) Directory=(char *)opts[OPT_DIR];
			if (opts[OPT_SUPERKICK]) FullSize-=4056; /* 4096 - 40 */
			if (opts[OPT_EXTRA]) FullSize-=*(ULONG *)opts[OPT_EXTRA];

			/*
			 * These two options are for doing general
			 * splits such as the 1meg split...
			 */
			if (opts[OPT_SPLIT]) Split=*(ULONG *)opts[OPT_SPLIT];
			if (opts[OPT_SIZE]) FullSize=*(ULONG *)opts[OPT_SIZE];

			if (buildFile=Open((char *)opts[OPT_BUILDLIST],MODE_OLDFILE))
			{
			struct	List	list;
			struct	Module	*node=NULL;
			struct	Module	**nodes=NULL;
				BPTR	outFile=NULL;
				ULONG	modules=0;
				char	buffer[512];

				NEWLIST(&list);

				while (FGets(buildFile,buffer,255))
				{
				char	*name;
				char	*tmp;

					name=buffer;
					while ((*name==' ') || (*name=='\t')) name++;
					tmp=buffer;

					while (*tmp=*name)
					{
						switch (*name)
						{
						case ' ':
						case ';':
						case '\t':
						case '\n':	*name='\0';
								break;
						default:	name++;
								tmp++;
								break;
						}
					}

					if (buffer[0]) if (node=(struct Module *)AllocVec(sizeof(struct Module)+strlen(buffer),MEMF_PUBLIC|MEMF_CLEAR))
					{
						strcpy(node->m_Name,buffer);
						node->m_Node.ln_Name=node->m_Name;
						AddTail(&list,(struct Node *)node);
						modules++;
					}
				}

				if (IoErr()) PrintFault(IoErr(),NULL);
				else if (!node) PrintFault(ERROR_NO_FREE_STORE,NULL);
				else if (nodes=AllocVec(4*modules,MEMF_PUBLIC))
				{
				struct	Module	**array=nodes;

					rc=RETURN_OK;
					node=(struct Module *)list.lh_Head;
					while ((node->m_Node.ln_Succ) && (RETURN_OK==rc))
					{
					BPTR	ld_File;

						*array++=node;
						strcpy(buffer,Directory);
						strcat(buffer,node->m_Name);
						if (ld_File=Open(buffer,MODE_OLDFILE))
						{
						ULONG	tmp;
						ULONG	hunks;
						ULONG	lastHunk;

							rc=RETURN_FAIL;
							if (FRead(ld_File,&tmp,4,1)) if (tmp==HUNK_HEADER)
							{
								if (FRead(ld_File,&tmp,4,1)) if (!tmp)
								{
									if (FRead(ld_File,&tmp,4,1))
									{
										if (FRead(ld_File,&hunks,4,1))
										{
											if (FRead(ld_File,&lastHunk,4,1))
											{
												rc=RETURN_OK;
												while (hunks<=lastHunk)
												{
													hunks++;
													if (FRead(ld_File,&tmp,4,1)) node->m_Size+=((tmp & 0x3FFFFFFF)<<2);
													else rc=RETURN_FAIL;
												}
											}
										}
									}
								}
							}
							if (RETURN_FAIL==rc) PrintFault(IoErr(),NULL);

							Close(ld_File);
						}
						else
						{
							rc=RETURN_FAIL;
							PrintFault(IoErr(),buffer);
						}

						node=(struct Module *)(node->m_Node.ln_Succ);
					}
				}
				else PrintFault(IoErr(),NULL);

				Close(buildFile);

				if (RETURN_OK==rc)
				{
				struct	DateTime	ds;
					ULONG		loop;
					ULONG		splitSize;
					ULONG		splitPoint=(ULONG)~0;
					ULONG		size;
					ULONG		temp;
					LONG		free;
					LONG		diff;
					ULONG		tmp[2];
					LONG		retry;
					LONG		outside;
					ULONG		flip=0;

					/*
					 * Three tries, each time one order less
					 * energy in the system...
					 */
					for (outside=0;outside<OUTSIDE;outside++) for (retry=RETRY_LOOP;retry>=0;retry--)
					{
					BOOL	done=FALSE;
					ULONG	topLine=1;
					ULONG	startTop=1;

						while (!done)
						{
							size=0;

							/* We now have a list and array... */
							for (loop=0;loop<modules;loop++)
							{
								temp=size+nodes[loop]->m_Size;
								if (size<Split)
								{
									if (temp>=Split)
									{
										splitSize=Split-size;
										splitPoint=loop;
									}
								}
								size=temp;
							}
							free=FullSize-(size+splitSize+8);

							if (SetSignal(0,0) & SIGBREAKF_CTRL_C) done=TRUE;
							if (opts[OPT_ANYFIT]) if (free>0) done=TRUE;
							if (splitSize<5) done=TRUE;
							if (size+12>FullSize) done=TRUE;
							if (opts[OPT_SIZESONLY]) done=TRUE;

							if (!done)
							{
							ULONG	swapLine=0;

								if (topLine<splitPoint)
								{
								LONG	best=splitSize+(retry*(modules-startTop)*4);

									temp=nodes[topLine]->m_Size+splitSize-1;

									if (free<0)
									{
										best-=free;
										temp-=free;
									}

									for (loop=splitPoint;loop<splitPoint;loop++)
									{
										diff=temp-nodes[loop]->m_Size;
										if (diff>=0) if (diff<best)
										{
											swapLine=loop;
											best=diff;
											if (free>0) loop=splitPoint;
										}
									}
								}
								else if (topLine<modules)
								{
								LONG	best=splitSize+(retry*(modules-startTop)*4);

									temp=splitSize-1-nodes[topLine]->m_Size;

									if (free<0)
									{
										best-=free;
										temp-=free;
									}

									for (loop=1;(loop<splitPoint) && (!swapLine);loop++)
									{
										diff=temp+nodes[loop]->m_Size;
										if (diff>=0) if (diff<best)
										{
											swapLine=loop;
											best=diff;
											if (free>0) loop=splitPoint;
										}
									}
								}
								else
								{
									topLine=startTop;
									startTop++;
									if (startTop>modules) done=TRUE;
								}

								if (swapLine)
								{
									node=nodes[topLine];
									nodes[topLine]=nodes[swapLine];
									nodes[swapLine]=node;
								}
								topLine++;
							}
						}

						if (SetSignal(0,0) & SIGBREAKF_CTRL_C) retry=0;
						else if (size+12>FullSize) retry=0;
						else if ((free<0) && (retry<1))
						{
							/*
							 * If we still do not fit, play
							 * some simple games...
							 */
							flip++;
							if (flip<1) flip=1;
							if (flip>=modules) flip=1;

							if (flip<splitPoint)
							{
								node=nodes[flip];
								nodes[flip]=nodes[splitPoint];
								nodes[splitPoint]=node;

								node=nodes[modules-flip];
								nodes[modules-flip]=nodes[splitPoint-1];
								nodes[splitPoint-1]=node;
							}
							else
							{
								node=nodes[flip];
								nodes[flip]=nodes[splitPoint-1];
								nodes[splitPoint-1]=node;

								node=nodes[modules-flip];
								nodes[modules-flip]=nodes[splitPoint];
								nodes[splitPoint]=node;
							}
							retry=RETRY_LOOP;
						}
					}

					if (opts[OPT_TO])
					{
						if (!(outFile=Open((char *)opts[OPT_TO],MODE_NEWFILE)))
						{
							rc=RETURN_FAIL;
							PrintFault(IoErr(),(char *)opts[OPT_TO]);
						}
					}
					else if (opts[OPT_FIRSTHALF])
					{
						if (!(outFile=Open((char *)opts[OPT_FIRSTHALF],MODE_NEWFILE)))
						{
							rc=RETURN_FAIL;
							PrintFault(IoErr(),(char *)opts[OPT_FIRSTHALF]);
						}
					}

					if (RETURN_OK==rc)
					{
						if (!opts[OPT_SIZESONLY])
						{
							/*
							 * Output final order
							 */
							DateStamp(&(ds.dat_Stamp));
							ds.dat_Format=FORMAT_DOS;
							ds.dat_Flags=0;
							ds.dat_StrDay=NULL;
							ds.dat_StrDate=buffer;
							ds.dat_StrTime=&buffer[128];
							DateToStr(&ds);
							tmp[0]=(ULONG)buffer;
							tmp[1]=(ULONG)&buffer[128];
							if (outFile) VFPrintf(outFile,";\n; Split generated %s %s\n;\n",tmp);
							else VPrintf(";\n; Split generated %s %s\n;\n",tmp);
						}

						tmp[0]=(ULONG)buffer;
						for (loop=0;loop<modules;loop++)
						{
							if (!opts[OPT_SIZESONLY]) if (loop==splitPoint)
							{
								FILL_BUFFER("; Split ",'-');
								tmp[1]=splitSize+8;
								if (outFile) VFPrintf(outFile,"%s ;%7lu\n",tmp);
								else VPrintf("%s ;%7lu\n",tmp);
								if (opts[OPT_FIRSTHALF])
								{
									Close(outFile);
									outFile=NULL;
									if (opts[OPT_SECONDHALF])
									{
										if (outFile=Open((char *)opts[OPT_SECONDHALF],MODE_NEWFILE))
										{
											VFPrintf(outFile,"%s ;%7lu\n",tmp);
										}
										else
										{
											rc=RETURN_FAIL;
											PrintFault(IoErr(),(char *)opts[OPT_SECONDHALF]);
										}
									}
								}
							}

							FILL_BUFFER(nodes[loop]->m_Name,' ');
							tmp[1]=nodes[loop]->m_Size;
							if (outFile) VFPrintf(outFile,"%s ;%7lu\n",tmp);
							else VPrintf("%s ;%7lu\n",tmp);
						}


						if (!opts[OPT_SIZESONLY])
						{
							FILL_BUFFER("; ",'=');
							if (outFile) VFPrintf(outFile,"%s ;=======\n",tmp);
							else VPrintf("%s ;=======\n",tmp);


							FILL_BUFFER("; Bytes free ",((free<0) ? '!' : ' '));
							tmp[1]=free;
							if (outFile) VFPrintf(outFile,"%s ;%7ld\n",tmp);
							else VPrintf("%s ;%7ld\n",tmp);
						}
					}

					if (outFile) Close(outFile);
				}

				/*
				 * Clean up...
				 */
				while (node=(struct Module *)RemHead(&list)) FreeVec(node);
				FreeVec(nodes);
			}
			else PrintFault(IoErr(),(char *)opts[OPT_BUILDLIST]);

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
