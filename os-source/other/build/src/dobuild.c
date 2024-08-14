/*
 * $Id: dobuild.c,v 40.5 93/10/12 10:59:26 darren Exp $
 *
 * $Log:	dobuild.c,v $
 * Revision 40.5  93/10/12  10:59:26  darren
 * NOCHECKSUM switch added - used for building a resolved ROM image
 * which has no exec, and no need for checksum info.
 * 
 * Revision 40.4  93/03/10  10:46:58  mks
 * Added INTERLEAVE option to Word Interleave the kickstart
 * for the strange 16-bit in 32-bit system ROM hardware
 * 
 * Revision 40.3  93/03/04  11:52:41  mks
 * Added code needed to deal with 1Meg builds...
 * Requires the up-to-date build tools.
 *
 * Revision 40.2  93/02/25  15:26:57  mks
 * No longer uses the internal (private) include
 *
 * Revision 40.1  93/02/18  15:40:15  mks
 * CleanUp for SAS/C 6 compile
 *
 * Revision 1.20  92/06/06  11:34:18  mks
 * Added REKICK option and changed the way REKICK/FKICK work...
 *
 * Revision 1.19  92/03/05  10:58:58  mks
 * Fixed FKICK keyword
 *
 * Revision 1.18  92/02/28  15:24:00  mks
 * Added the BUMPREV option for AUTOREV
 *
 * Revision 1.17  92/02/28  11:27:39  mks
 * Added the AUTOREV and FKICK options...
 *
 * Revision 1.16  92/02/04  12:33:14  mks
 * Cleaned up the output from the QUICK option
 *
 * Revision 1.15  91/12/06  08:41:04  mks
 * Fixed typecast error.
 *
 * Revision 1.14  91/12/06  07:18:58  mks
 * Added CDTV_E and DIRECT support
 *
 * Revision 1.13  91/11/15  10:42:52  mks
 * Removed the ADD/REPLACE/REMOVE keywords and changed that to the
 * multi-arg CHANGE keyword
 *
 * Revision 1.12  91/11/14  19:38:13  mks
 * Now does revision 0 and CDTV:
 *
 * Revision 1.11  91/10/28  12:19:42  mks
 * Added the REMOVE and ADD options and change the REPLACE option
 *
 * Revision 1.10  91/10/25  18:48:56  mks
 * Added comment features to build lists and the ability to
 * offer replacement .ld files.  (Only replacements)
 *
 * Revision 1.9  91/10/18  18:40:00  mks
 * Added KickIt support
 *
 * Revision 1.8  91/10/18  17:16:42  mks
 * Added the CDTV: part of the CDTV switch.
 * Added the DIR switch
 * Added the TO switch
 *
 * Revision 1.7  91/10/18  15:36:12  mks
 * Added CDTV option and RELOC option
 *
 * Revision 1.6  91/10/17  10:23:56  mks
 * Cleaned up formatting so it will print nice
 *
 * Revision 1.5  91/10/15  10:19:08  mks
 * Added standard version string to the commands
 *
 * Revision 1.4  91/10/15  09:45:22  mks
 * Made the script generated with the SCRIPT keyword a bit better
 *
 * Revision 1.3  91/10/15  09:21:11  mks
 * Added the script file creation feature
 *
 * Revision 1.2  91/10/14  15:22:35  mks
 * Fixed log file output...
 *
 * Revision 1.1  91/10/14  14:29:58  mks
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

#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	<dos/datetime.h>
#include	<dos/dostags.h>

#include	<string.h>

#include	"dobuild_rev.h"

/******************************************************************************/

/*
 * This command runs the build process...
 *
 * The command line will take a BUILDLIST file which contains
 * the files to be build in the order that they need to be built
 * it will also take major and minor version which will be
 * used to generate the path to the .ld files.  (V37:, V39:, etc)
 * For CDTV, the directory is CDTV:  (No version, they did not want it)
 *
 * The build list file name will also be used to generate the
 * sub-names for the kickstart and symbol files.
 *
 * This assumes that the build commands are available in the path.
 *
 * The names of the files:
 *
 * <BUILDLIST>			This is a simple file containing the complete
 *				kickstart build list
 *
 * kick.<build>.<rev>		This is the kickstart file produced
 *
 * log.<build>.<rev>		This is the log file produced
 *
 * sym.<build>.<rev>.ld		This is the .ld symbol file produced
 *
 * sym.<build>.<rev>.map	This is the .map sumbol file produced
 */

/* This is the command template. */
#define TEMPLATE  "BUILDLIST/A,VERSION/N/A,REVISION/N/A,AUTOREV/K,BUMPREV/S,RELOC/K,SPLIT/S,1MEG/S,SUPERKICK/K,REKICK/S,FKICK/S,INTERLEAVE/S,QUICK/S,DIRECT/S,SCRIPT/K,CDTV/S,CDTV_E/S,DIR/K,TO/K,CHANGE/K/M,NOCHECKSUM/S" VERSTAG

#define	OPT_BUILDLIST	0
#define	OPT_VERSION	1
#define	OPT_REVISION	2
#define	OPT_AUTOREV	3
#define	OPT_BUMPREV	4
#define	OPT_RELOC	5
#define	OPT_SPLIT	6
#define	OPT_1MEG	7
#define	OPT_SUPERKICK	8
#define	OPT_REKICK	9
#define	OPT_FKICK	10
#define	OPT_INTERLEAVE	11
#define	OPT_QUICK	12
#define	OPT_DIRECT	13
#define	OPT_SCRIPT	14
#define	OPT_CDTV	15
#define	OPT_CDTV_E	16
#define	OPT_DIR		17
#define	OPT_TO		18
#define	OPT_CHANGE	19
#define OPT_NOCHECKSUM	20
#define	OPT_COUNT	21

struct	BuildStrings
{
	char	*reloc;
	char	version[16];
	char	revision[16];
	char	kick[256];
	char	log[256];
	char	ld[256];
	char	map[256];
};

void long_to_string(long,char *);
void SetUp_Load(char *,struct BuildStrings *,LONG *);

#define DO_COMMAND	if (script) VFPrintf(script,"%s\n",(VOID *)&vbuf); else if (System(vbuf,tags)) rc=RETURN_FAIL

LONG cmd_dobuild(void)
{
struct	Library		*SysBase = (*((struct Library **) 4));
struct	DosLibrary	*DOSBase;
struct	RDArgs		*rdargs;
struct	BuildStrings	*strings;
	LONG		opts[OPT_COUNT];
	LONG		rc;
	char		*tmp;
	char		buf[1024];
struct	TagItem		tags[] =
			{
				{SYS_Asynch, FALSE},
				{SYS_UserShell, TRUE},
				{TAG_DONE, NULL}
			};

	rc=RETURN_FAIL;
	if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
	{
		memset((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts, NULL);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(),NULL);
		}
		else if (strings=AllocVec(sizeof(struct BuildStrings),MEMF_PUBLIC|MEMF_CLEAR))
		{
		BPTR	build_file;
		BPTR	script=NULL;
		ULONG	revision;
		char	*name;

			/*
			 * Set "Default" reloc
			 */
			strings->reloc=NULL;

			/*
			 * Check for CDTV switch and set
			 * the name to CDTV ROM from kickstart...
			 */
			name="kickstart";
			if (opts[OPT_CDTV] || opts[OPT_CDTV_E]) name="CDTV ROM";

			/*
			 * If FKick, we need to relocate to 00F00000
			 */
			if (opts[OPT_FKICK]) strings->reloc="00F00000";

			/*
			 * If ReKick, we need to relocate to 00200000
			 */
			if (opts[OPT_REKICK]) strings->reloc="00200000";

			/*
			 * Check if we have a relocation key word
			 */
			if (opts[OPT_RELOC]) strings->reloc=(char *)opts[OPT_RELOC];

			/*
			 * Check for the script keyword
			 */
			if (opts[OPT_SCRIPT]) script=Open((char *)opts[OPT_SCRIPT],MODE_NEWFILE);

			/*
			 * Now check for auto AutoRev feature...
			 */
			revision=*(ULONG *)opts[OPT_REVISION];	/* Get command line revision */

			if (opts[OPT_AUTOREV])
			{
			BPTR	rev_file;

				if (rev_file=Open((char *)opts[OPT_AUTOREV],MODE_OLDFILE))
				{
				char	num[32];

					Read(rev_file,num,30);
					Close(rev_file);

					StrToLong(num,&revision);
					if (opts[OPT_BUMPREV]) revision++;
				}

				if (rev_file=Open((char *)opts[OPT_AUTOREV],MODE_NEWFILE))
				{
					VFPrintf(rev_file,"%ld\n",&revision);
					Close(rev_file);
				}
			}

			/*
			 * Build the version and revision strings...
			 */
			long_to_string(*(ULONG *)opts[OPT_VERSION],strings->version);
			long_to_string(revision,strings->revision);

			tmp=(char *)opts[OPT_BUILDLIST];
			if (build_file=Open(tmp,MODE_OLDFILE))
			{
				tmp=FilePart(tmp);
				if (opts[OPT_DIR]) if (strlen((char *)opts[OPT_DIR])>32) tmp=(char *)opts[OPT_DIR];
				if (opts[OPT_TO]) if (strlen((char *)opts[OPT_TO])>32) tmp=(char *)opts[OPT_TO];
				if (strlen(tmp)<32)
				{
				struct	DateTime	ds;
					char		*vbuf=buf;
					VOID		*args[6];	/* For VPrintf() */
					BPTR		tmp_fh=NULL;

					rc=RETURN_OK;

					strcpy(buf,".");
					strcat(buf,tmp);
					strcat(buf,".");
					strcat(buf,strings->version);
					strcat(buf,".");
					strcat(buf,strings->revision);

					/* Now build the names... */
					if (opts[OPT_TO])
					{
						/*
						 * We have a destination drawer
						 */
						strcpy(strings->kick,(char *)opts[OPT_TO]);
						strcpy(strings->log,(char *)opts[OPT_TO]);
						strcpy(strings->ld,(char *)opts[OPT_TO]);
					}
					if (opts[OPT_CDTV] || opts[OPT_CDTV_E]) strcat(strings->kick,"cdtv");
					else strcat(strings->kick,"kick");
					strcat(strings->kick,buf);

					strcat(strings->log,"log");
					strcat(strings->log,buf);

					strcat(strings->ld,"sym");
					strcat(strings->ld,buf);
					strcpy(strings->map,strings->ld);
					strcat(strings->ld,".ld");
					strcat(strings->map,".map");

					/*
					 * Now, make sure that the symbol files are empty
					 */
					if (script)
					{
						args[0]=strings->ld;
						VFPrintf(script,"Delete >NIL: %s\n",(VOID *)args);
						args[0]=strings->map;
						VFPrintf(script,"Delete >NIL: %s\n",(VOID *)args);
						args[0]=strings->kick;
						VFPrintf(script,"Delete >NIL: %s\n",(VOID *)args);
						args[0]=strings->log;
						VFPrintf(script,"Delete >NIL: %s\n",(VOID *)args);
					}
					else
					{
						DeleteFile(strings->ld);
						DeleteFile(strings->map);
						DeleteFile(strings->kick);
						DeleteFile(strings->log);
					}

					/*
					 * Now write the log file header
					 */
					if ((!script) && (!opts[OPT_QUICK])) tmp_fh=Open(strings->log,MODE_NEWFILE);

					DateStamp(&(ds.dat_Stamp));
					ds.dat_Format=FORMAT_DOS;
					ds.dat_Flags=0;
					ds.dat_StrDay=NULL;
					ds.dat_StrDate=buf;
					ds.dat_StrTime=&buf[32];
					DateToStr(&ds);

					args[0]=name;
					args[1]=strings->version;
					args[2]=strings->revision;
					args[3]=FilePart((char *)opts[OPT_BUILDLIST]);
					args[4]=buf;
					args[5]=&buf[32];
					if (script)
					{
						if (!opts[OPT_QUICK])
						{
							tmp=(char *)opts[OPT_SCRIPT];
							args[3]=tmp;
							VPrintf("Building a BUILD script in %s\n",(VOID *)&tmp);

							VFPrintf(script,"Echo \"*n",NULL);
							VFPrintf(script,"Building %s %s.%s from script <%s>",(VOID *)args);
							VFPrintf(script,"*n\"\n",NULL);

							tmp=strings->log;
							VFPrintf(script,"Echo >%s \"",(VOID *)&tmp);
							VFPrintf(script,"Building %s %s.%s from script <%s>",(VOID *)args);
							VFPrintf(script," : \" NOLINE\nDate >>%s\n",(VOID *)&tmp);
							VFPrintf(script,"Echo >>%s \" \"\n",(VOID *)&tmp);
						}
					}
					else VPrintf("Building %s %s.%s from BuildList <%s> : %s %s\n\n",(VOID *)args);

					if (tmp_fh)
					{
						VFPrintf(tmp_fh,"Building %s %s.%s from BuildList <%s> : %s %s\n\n",(VOID *)args);
						Close(tmp_fh);
					}

					/*
					 * If we have a 1MEG build, we will need to pre-split the build list
					 * into two 512K parts...  This is not put into the build script.
					 */
					if (opts[OPT_1MEG])
					{
					char	*p;

						Close(build_file);
						build_file=NULL;

						strcpy(vbuf,"Split DIR ");
						if (opts[OPT_DIR]) strcat(buf,(char *)opts[OPT_DIR]);
						else
						{
							strcat(buf,"V");
							strcat(buf,strings->version);
							strcat(buf,":");
						}							/* The 512K line, but we adjust it */
						p=&vbuf[strlen(vbuf)];					/* to fit in the extra stuff...    */
													/*   SPLIT 524288 SIZE 1048576     */
						strcat(vbuf," FIRSTHALF T:BuildPart0 SECONDHALF T:BuildPart1 SPLIT 524200 SIZE 1048000 ANYFIT BUILDLIST ");
						strcat(vbuf,(char *)opts[OPT_BUILDLIST]);

						if (System(vbuf,tags)) rc=RETURN_FAIL;
						else if (opts[OPT_SPLIT])
						{
							strcpy(p," BUILDLIST T:BuildPart0 TO T:BuildPart0 ANYFIT");
							if (System(vbuf,tags)) rc=RETURN_FAIL;
						}
					}

					/*
					 * Now, we need to start doing the build...
					 */
					strcpy(vbuf,"MakeVF CLEAR");
					if (opts[OPT_DIRECT]) strcat(vbuf," DIRECT");
					if (opts[OPT_1MEG]) strcat(vbuf," TAG");
					DO_COMMAND;

					if (rc==RETURN_OK)
					{
					char	*newstuff;
					ULONG	buildpart=1;
					ULONG	split_Remember=opts[OPT_SPLIT];

						vbuf=buf;
						strcpy(buf,"LoadVF ");

						if (opts[OPT_DIR]) strcat(buf,(char *)opts[OPT_DIR]);
						else
						{
							/*
							 * If CDTV is set, we should use CDTVxx:
							 */
							if (opts[OPT_CDTV] || opts[OPT_CDTV_E]) strcat(buf,"CDTV");
							else
							{
								strcat(buf,"V");
								strcat(buf,strings->version);
							}
							strcat(buf,":");
						}
						newstuff=&buf[strlen(buf)];

						if (opts[OPT_1MEG]) buildpart=2;

						while (buildpart--)
						{
							if (rc==RETURN_OK)
							{
								if (opts[OPT_1MEG])
								{
								char	*p;

									vbuf=newstuff;

									if (build_file) Close(build_file);

									if (buildpart==1)
									{
										p="T:BuildPart1";
										strings->reloc="00E00000";
										opts[OPT_SPLIT]=FALSE;
									}
									else if (buildpart==0)
									{
										p="T:BuildPart0";
										strings->reloc=NULL;
										opts[OPT_SPLIT]=split_Remember;

										/*
										 * Now, we need to save this part and
										 * build the next part...
										 */

										if(opts[OPT_NOCHECKSUM]==0L)
										{
											strcpy(vbuf,"CheckSumVF VERSION ");
											strcat(vbuf,strings->version);
											strcat(vbuf," REVISION ");
											strcat(vbuf,strings->revision);
											DO_COMMAND;
										}

										strcpy(vbuf,"SaveVF ");
										strcat(vbuf,strings->kick);
										DO_COMMAND;

										strcpy(vbuf,"MakeVF APPEND");
										DO_COMMAND;
									}

									if (!(build_file=Open(p,MODE_OLDFILE))) rc=RETURN_FAIL;

									vbuf=buf;
								}
							}

							while ((rc==RETURN_OK) && (tmp=FGets(build_file,newstuff,256)))
							{
							char	*mystuff;
							char	*t;

								t=newstuff;
								mystuff=newstuff;
								while (*mystuff==' ' || *mystuff=='\t') mystuff++;

								while (*t=*mystuff)
								{
									switch (*mystuff)
									{
									case ' ':
									case ';':
									case '\t':
									case '\n':	*mystuff='\0';
											break;
									default:	mystuff++;
											t++;
											break;
									}
								}

								if (newstuff != &buf[strlen(buf)])
								{
								char	**rep;
								char	*new;

									/*
									 * Ok, so we have a command...  We really should check to see if
									 * it is one we will replace...
									 */
									mystuff=FilePart(newstuff);
									if (rep=(char **)opts[OPT_CHANGE]) while ((new=*rep) && (mystuff) && (vbuf))
									{
										if (*new=='-')
										{
											/*
											 * We have a delete match
											 */
											new++;
											if (!strcmp(FilePart(new),mystuff)) vbuf=NULL;
										}
										else if (*new!='+')
										{
											if (!strcmp(FilePart(new),mystuff))
											{
												/*
												 * We have a replacement match!!!
												 */
												vbuf=newstuff;
												strcpy(vbuf,"LoadVF ");
												strcat(vbuf,new);
												mystuff=NULL;
											}
										}
										rep++;
									}

									if (vbuf)
									{
										SetUp_Load(vbuf,strings,opts);
										DO_COMMAND;
									}

									vbuf=buf;
								}
							}

							if (!tmp) if (IoErr())
							{
								rc=RETURN_FAIL;
								PrintFault(IoErr(),NULL);
							}
						}

						if (rc==RETURN_OK)
						{
						char	**rep;
						char	*new;

							/*
							 * Now, process all of the ADD commands...
							 */
							if (rep=(char **)opts[OPT_CHANGE]) while ((new=*rep) && (rc==RETURN_OK))
							{
								if (*new++=='+')
								{
									vbuf=newstuff;
									strcpy(vbuf,"LoadVF ");
									strcat(vbuf,new);

									SetUp_Load(vbuf,strings,opts);

									DO_COMMAND;
									vbuf=buf;
								}
								rep++;
							}
						}

						if (rc==RETURN_OK)
						{
							/* Now checksum the kickstart */
							if(opts[OPT_NOCHECKSUM]==0L)
							{
								strcpy(vbuf,"CheckSumVF VERSION ");
								strcat(vbuf,strings->version);
								strcat(vbuf," REVISION ");
								strcat(vbuf,strings->revision);
								if (opts[OPT_CDTV] || opts[OPT_CDTV_E]) strcat(vbuf," CDTV");
								if (opts[OPT_SUPERKICK]) strcat(vbuf," SUPERKICK");

								DO_COMMAND;
							}
						}

						if (rc==RETURN_OK)
						{
							/* Now save the kickstart */
							strcpy(vbuf,"SaveVF ");
							strcat(vbuf,strings->kick);

							if (opts[OPT_1MEG]) strcat(vbuf," APPEND");

							if (opts[OPT_SUPERKICK])
							{
								strcat(vbuf," SUPERKICK ");
								strcat(vbuf,(char *)opts[OPT_SUPERKICK]);
							}

							if (opts[OPT_REKICK]) strcat(vbuf," REKICK");
							if (opts[OPT_FKICK]) strcat(vbuf," FKICK");

							if (!opts[OPT_QUICK])
							{
								strcat(vbuf," SYMBOL ");
								strcat(vbuf,strings->ld);
							}

							DO_COMMAND;
						}

						strcpy(vbuf,"MakeVF REMOVE");
						DO_COMMAND;

						if (opts[OPT_INTERLEAVE])
						{
							strcpy(vbuf,"KickWordMix ");
							strcat(vbuf,strings->kick);
							DO_COMMAND;
						}

						if ((!script) && (!opts[OPT_QUICK])) if (tmp_fh=Open(strings->log,MODE_OLDFILE))
						{
							Seek(tmp_fh,0,OFFSET_END);
						}

						DateStamp(&(ds.dat_Stamp));
						ds.dat_Format=FORMAT_DOS;
						ds.dat_Flags=0;
						ds.dat_StrDay=NULL;
						ds.dat_StrDate=buf;
						ds.dat_StrTime=&buf[32];
						DateToStr(&ds);

						args[0]=name;
						args[1]=strings->version;
						args[2]=strings->revision;
						args[3]="completed";	if (rc!=RETURN_OK) args[3]="failed";
						args[4]=buf;
						args[5]=&buf[32];
						if (script)
						{
							if (!opts[OPT_QUICK])
							{
								tmp=(char *)opts[OPT_SCRIPT];
								args[3]=tmp;
								VPrintf("BUILD script %s is complete.\n",(VOID *)&tmp);
								tmp=strings->log;
								VFPrintf(script,"Echo >>%s \"*n",(VOID *)&tmp);
								VFPrintf(script,"Built %s %s.%s from script <%s> : ",(VOID *)args);
								VFPrintf(script,"\" NOLINE\nDate >>%s\n",(VOID *)&tmp);
								VFPrintf(script,"Echo \"*nBuild from script done.\"\n",NULL);
							}
						}
						else VPrintf("\nBuild of %s %s.%s %s at : %s %s\n",(VOID *)args);

						if (tmp_fh)
						{
							VFPrintf(tmp_fh,"\nBuild of %s %s.%s %s at : %s %s\n",(VOID *)args);
							Close(tmp_fh);
						}
					}
				}
				else PrintFault(ERROR_INVALID_COMPONENT_NAME,NULL);

				if (build_file) Close(build_file);
			}
			else PrintFault(IoErr(),"Can not open BUILDLIST");

			/*
			 * Clean up the temp build list splits...
			 */
			if (opts[OPT_1MEG])
			{
				DeleteFile("T:BuildPart1");
				DeleteFile("T:BuildPart0");
			}

			if (script) Close(script);
			FreeVec(strings);
		}

		if (rdargs) FreeArgs(rdargs);

		CloseLibrary((struct Library *)DOSBase);
	}
	else
	{
		OPENFAIL;
	}
	return(rc);
}

void long_to_string(long num,char *buf)
{
char	tmp[16];
char	*p;

	p=&tmp[15];
	*p='\0';

	if (!num)
	{
		p--;
		*p='0';
	}
	else while (num)
	{
		p--;
		*p='0'+(num%10);
		num=num/10;
	}
	strcpy(buf,p);
}

void SetUp_Load(char *vbuf,struct BuildStrings *strings,LONG *opts)
{
	if (!opts[OPT_QUICK])
	{
		strcat(vbuf," SYMBOL ");
		strcat(vbuf,strings->ld);
		strcat(vbuf," MAP ");
		strcat(vbuf,strings->map);
		strcat(vbuf," LOG ");
		strcat(vbuf,strings->log);
	}

	if (strings->reloc)
	{
		strcat(vbuf," RELOC ");
		strcat(vbuf,strings->reloc);
	}

	if (opts[OPT_SPLIT]) strcat(vbuf," SPLIT");
	if (opts[OPT_CDTV]) strcat(vbuf," CDTV");
	if (opts[OPT_CDTV_E]) strcat(vbuf," CDTV_E");
}
