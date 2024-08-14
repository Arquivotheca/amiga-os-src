;/* report.c - Execute me to compile me with SAS C 6.x
SC report.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,report.o TO report LIBRARY LIB:SC.lib,LIB:Amiga.lib ND
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

#include "ReportCategories.h"

#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif

#define MINARGS 1

UBYTE *vers = "\0$VER: report 40.2";
UBYTE *Copyright = 
  "report v40.2\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: report [outfile] [help] (default outfile is ram:Report.txt)";

UBYTE *helplines[] = {
 "Generates Bug Reports, Compatibility Reports, and Enhancement Requests\n\n",
 "Default output filename is ram:Report.txt (or you may specify a filename).\n",
 "Default editor is your choice (specify with: setenv EDITOR editorname).\n",
 "If you have not setenv'd an EDITOR, Memacs will be used.\n",
 "Report creates/uses s:Report.sender,s:Report.config,s:Report.ks,Report.wb\n",
 "If your configuration is incorrect, delete s:Report.config and try again.\n",
 "\nPLEASE - DO NOT fill in the Subsystem field or any other restricted value\n",
 "fields by hand editing!  Only the REPORT program can validate these fields.\n",
  "\nReports may be submitted as follows (electronic submission preferred!):\n",
 "European ADSP users: Post in appropriate closed adsp bugs topic\n", 
 "BIX/CIX: Post bugs in the appropriate bugs topic of your closed conference.\n",
 "UUCP: to rutgers!cbmvax!bugs OR bugs@commodore.COM\n",
 "  (enhancement requests to suggestions instead of bugs)\n",
 "Mail: Mail individual bug reports in Report format, on disk if possible.\n" 
 "  European developers: Mail bug reports to your support manager\n",
 "  unless your support manager says to mail directly to West Chester.\n"
 "  U.S./others mail to: Amiga Software Engineering, ATTN: BUG REPORTS,\n",
 "  CBM, 1200 Wilson Drive, West Chester, PA., 19380, USA\n",
NULL };


/* prototypes */
int okA(UBYTE *buf, UBYTE *ma);
unsigned int doQ(UBYTE *buf, struct QA *qa);
unsigned int doPQA(UBYTE *buf, UBYTE *p, UBYTE *q, UBYTE *a);

void bye(UBYTE *s, int e);
void cleanup(void);
void showos(void);
void find_tasks(UBYTE **names, UBYTE *found);

#define ReportSenderFile	"s:Report.sender"
#define ReportConfigFile	"s:Report.config"
#define ReportKSFile		"s:Report.ks"
#define ReportWBFile		"s:Report.wb"
#define TmpEdFind		"t:Report.e"

#define defed		"Memacs"


UBYTE *bout = "ram:Report.txt";

/**
 **  The Question-Answer structure
 **/
struct QA {
	UBYTE *p;	/* prompt or instructions */
	UBYTE *q;	/* question */
	UBYTE *a;	/* optional string of allowable 1 char responses */
	UBYTE *o;	/* optional output formatting string */
	};

/**
 **  Q-A's for ReportSenderFile
 **/
#define BICNT 5
struct QA bi[] =
    {
	{ NULL, "Lastname,FirstName (no spaces) -> ",NULL, "# Refer: %s " },
	{ NULL, "Company Name (without Inc., Corp, etc.) -> ",NULL, "(%s " },
	{ NULL, "(AreaCode) Phone-Number -> ",NULL, ",phone %s)\n" },
	{ "Please enter your best electronic mail address.\n"
          "If UUCP, just enter the address,  for example: jsmith@endor.COM\n"
          "If other, enter address (SYSTEM), for example: jsmith (BIX)\n",
          "Enter electronic address (<RET> if none) -> ",NULL, "# Path: %s\n" },
	{ NULL, "Developer Number -> ",NULL, "# ReferID: %s\n" }
    };

/**
 **  Q-A's for ReportConfigFile
 **/
#define BCCNT 9
struct QA bc[] =
    {
	{ NULL, "Model (a500,a600,a1000,a1200,a2000,a2500,a3000,a4000,etc)-> ",NULL, "# Config: %s," },
	{ NULL, "Processor (68000,68020,68030,68040)-> ",NULL, "%s," },
	{ NULL, "Agnus/Alice type (AA,ECS,ECS-pal,old,old-pal) -> ",NULL, "A=%s," },
	{ NULL, "Denise/Lisa type (AA,ECS,old) -> ",NULL, "D=%s," },
	{ NULL, "Total Megs of CHIP ram -> ",NULL, "RAM=%smegC" },
	{ NULL, "Total Megs of FAST ram -> ",NULL, "/%smegF," },
	{ NULL, "Number of floppies -> ",NULL, "TD=%s," },
	{ NULL, "Hard disk controller (a2090,a2090a,a2091, etc) -> ",NULL, "HD=%s," },
	{ NULL, "Other expansion cards-> ",NULL, "%s\n" },
    };

/**
 **  Q-A's for Compat Reports Only
 **/
#define COCNT 3
struct QA co[] =
    {
	{ NULL, "\n3rd Party Product Name-> ",NULL, "# ProductName: %s" },
	{ NULL, "3rd Party Product Version-> ",NULL, " (vers=%s)\n" },
	{ NULL, "Manufacturer of Product (without Inc.,Corp., etc)-> ",NULL, "# ProductInfo: :%s:\n" },
    };

/**
 **  General Q-A's for Bug Report
 **/

#define BUGTYPE	 	(&qq[0])
#define SUMMARY  	(&qq[1])
#define SEVERITY	(&qq[2])
#define SEVERITYC 	(&qq[3])

struct QA qq[] =
    {
	/* 0 */
	{ "\nCBM Bug and Enhancement Report Generator\n"
	  "----------------------------------------\n"
	  " 1. System Software or Hardware Bug Report\n"
	  " 2. Third Party Product Compatibility\n"
	  " 3. Enhancement Request\n",
	  "Enter 1-3: ","123", NULL
	},
	/* 1 */
	{ NULL,"\nEnter a meaningful 1-line summary:\n",NULL,NULL },
	/* 2 */
	{ "\nClassification:\n"
	  " a. Crashes, Hangs, or Corrupts Data\n"
	  " b. Behaves incorrectly\n"
	  " c. Cosmetic problem\n",
	  "Enter appropriate letter: ","abc", NULL
	},
	/* 3 */
	{ "\nClassification:\n"
	  " a. Crashes, Hangs, or Corrupts Data\n"
	  " b. Behaves incorrectly\n"
	  " c. Cosmetic problem\n"
	  " w. Works correctly\n",
	  "Enter appropriate letter: ","abcw", NULL
	},

    };



#define IBUFSIZE 1024 
#define BUFSIZE  (IBUFSIZE << 4)

UBYTE *ibuf, *tbuf, *obuf, *cbuf;
BPTR ifile, ofile, bfile;

#define BUG	0
#define COMPAT	1
#define EREQ	2
ULONG btype = 0;
UBYTE *rtypes[] = {"bug","compat","enhancement"};
UBYTE *longrtypes[] = 
   {"System Bug Report","Compatibility Report","Enhancement Request"};

UBYTE *subsys1 = 
 "\n-------------------------- Valid Subsystems --------------------------------\n";
UBYTE *subsys2 = 
 "* NOTE * If you don't see specific command name, use cdos.command for 2.0 C:\n";
UBYTE *subsys3 = 
 "commands, util.command for Utilities/, system.command for System/, tools for\n";
UBYTE *subsys4 = 
 "Tools/, and just hit return if you are not sure of correct subsystem.\n";
UBYTE *subsys5 = 
 "Please categorize all include and autodoc bugs under their OS subsystem name.\n";
UBYTE *subsys6 = 
 "Use cdgs for CD32 HW bugs, AS225 for TCP/IP bugs.\n";
UBYTE *subsys7 = 
 "Use rkm for Rom Kernel Manual bugs, userdocs for user manual bugs.\n";
UBYTE *line = 
 "-----------------------------------------------------------------------------\n";

UBYTE *fsubsys = "%-15s%-16s%-16s%-15s%-15s\n";

/* The ReportCategories.h array is generated from the Engineering
 * ReportGen/Categories file using the Unix script BugCatToC
 *
 * Note - I remove the ?? category by hand, 
 * and I adjust fsubsys format string above to separate columns
 */

/* Task or CLI command names - not case sensitive (not using FindTask) */
UBYTE *TaskNames[] = { "« Enforcer »", "IO_Torture", NULL };
/* need as many elements as above */
UBYTE  TaskFound[] = { 0,              0,            0 };

UBYTE *SemaNames[] = { "SegTracker", NULL };
UBYTE *PortNames[] = { "_The Enforcer_", "Mungwall", "CpuBlit V1.00", NULL };



UBYTE Detected[512] = {0};

UBYTE ksver[40], wbver[40];


void main(int argc, char **argv)
    {
    LONG rlen;
    UBYTE *ss, *name;
    int k, i, l, b, perc, cols, maxi, offs, blocks, subcnt;

    UBYTE *subs[5];
    UBYTE ofnbuf[128];
    UBYTE edname[80];
    UBYTE severity;
    BOOL  DoneSubs, SubMatch;

    bfile = ifile = ofile = NULL;
    ibuf = NULL;
    ksver[0] = '\0';
    wbver[0] = '\0';

    if(((argc)&&(argc<MINARGS))||((argc==2)&&((argv[1][0]=='?'))))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}
    if((argc>1)&&(!strcmp(argv[argc-1],"help")))
	{
	printf("%s\n%s\n",Copyright,usage);
	for(k=0; helplines[k]; k++)	printf(helplines[k]);
	bye("",RETURN_OK);
	}

    if(!(ibuf = (UBYTE *)AllocMem(BUFSIZE, MEMF_CLEAR)))
	bye("Not enough mem\n",RETURN_FAIL);

    tbuf = ibuf + IBUFSIZE;
    cbuf = tbuf + IBUFSIZE;
    obuf = cbuf + IBUFSIZE;
    

    if(argc==2) bout = argv[1];

/**
 ** Detect other tools running
 **/
    i=0;
    Detected[0] = '\0';
    find_tasks(TaskNames,TaskFound);
    for(k=0; name=TaskNames[k]; k++)
	{
	if(TaskFound[k])   i+=sprintf(&Detected[i],"%s, ",name);
	}
    for(k=0; name=PortNames[k]; k++)
	{
	if(FindPort(name)) i+=sprintf(&Detected[i],"%s, ",name);
	}
    for(k=0; name=SemaNames[k]; k++)
	{
	if(FindSemaphore(name)) i+=sprintf(&Detected[i],"%s, ",name);
	}

/**
 **  Get editor name
 **/
    if(ifile = Open("ENV:Editor",MODE_OLDFILE))
	{
	rlen = Read(ifile,edname,80);
        edname[rlen] = '\0';
	Close(ifile);  ifile = NULL;
	}
    else strcpy(edname,defed);


/**
 **  Create bug report output file
 **/

GetBFile:
    while(!bout)
	{
	if(doPQA(ofnbuf,NULL,"Filename for output: ",NULL)) bout = ofnbuf;
	}
    if(bfile = Open(bout,MODE_OLDFILE))
	{
	Close(bfile); bfile = NULL;
	sprintf(tbuf,"Output file %s already exists.  Overwrite <y or n> ? ",bout);
	doPQA(ibuf,NULL,tbuf,"yn");
	if((ibuf[0] | 0x20) != 'y')
	    {
 	    bout = NULL;
	    goto GetBFile;
	    }
	}
    
    if(!(bfile = Open(bout,MODE_NEWFILE)))
	{
	printf("\nCan't open %s for output\n",bout);
	bout = NULL;
	goto GetBFile;
	}	


    /**
     **  Get report type, summary, date, OS versions, severity
     **/

    /* Type */
    doQ(ibuf,BUGTYPE);
    btype = atoi(ibuf) - 1;

    printf("%s selected.\n",longrtypes[btype]);


    /* Summary */
    doQ(ibuf,SUMMARY);
    sprintf(tbuf,"# Subject: %s\n# Type: %s\n",ibuf,rtypes[btype]);
    Write(bfile,tbuf,strlen(tbuf));

    /**
     **  Get additional info for Bug Reports and Enhancement requests
     **/
    if(btype != COMPAT)
	{
	for(k=0; subsys[k]; k++);
	subcnt = k;
	cols = 5;
SubLoop:
	perc = 16;
	maxi = (subcnt + (cols - 1 )) / cols;
	offs = 0;
	blocks = (subcnt + ((cols * perc)-1)) / (cols * perc);
    	DoneSubs = FALSE;
    	i = 0;
    	printf(subsys1);
	for(b = 0; (b < blocks)&&(!DoneSubs) ; b++)
	    {
	    offs = b * (perc * cols);
	    perc = MIN(perc,(maxi - i));
 	    for(l=0; (l<perc)&&(!DoneSubs); l++)
	    	{
	    	for(k=0; k<cols; k++)    
	    	    {
	    	    if(ss=subsys[l + offs + (perc * k)]) subs[k] = ss;
	    	    else subs[k] = "";
	    	    }
	    	printf(fsubsys,subs[0],subs[1],subs[2],subs[3],subs[4]);
	    	i++;		/* counts lines printed */
	    	if(i == maxi)	DoneSubs = TRUE;
		}
	    if(!DoneSubs)
		{
		printf("Press RETURN for More...");
		while(getchar() != '\n');
		}
	    }
	printf(line);
    	printf(subsys2);
    	printf(subsys3);
    	printf(subsys4);
    	printf(subsys5);
    	printf(subsys6);
    	printf(subsys7);
    	printf("%s\n",line);
    	ibuf[0] = '\0';
    	doPQA(ibuf,NULL,
	"Enter correct subsystem from list,\n   or <RET> if not sure, or ?<RET> to review list: ",NULL);

	if(ibuf[0]=='?') goto SubLoop;

    	SubMatch = ibuf[0] ? FALSE : TRUE;
    	if(ibuf[0])
	    {
	    for(k=0; subsys[k]; k++)
	    	{
	    	if(!(stricmp(ibuf,subsys[k]))) 
		    {
		    SubMatch = TRUE;
		    break;
		    }
	    	}
	    }
    	if(!SubMatch)
	    {
	    printf("%s is not a listed subsystem\n",ibuf);
	    goto SubLoop;
	    }
    	sprintf(tbuf,"# Subsystem: %s\n",ibuf);
    	Write(bfile,tbuf,strlen(tbuf));
	}

    /**
     **  Get additional info for Compatibility Reports
     **/
    if(btype == COMPAT)
	{
    	for(i=0, k=0; k < COCNT; k++)
	    {
	    doQ(ibuf,&co[k]);
	    i += sprintf(&tbuf[i],co[k].o,ibuf);
	    }
        Write(bfile,tbuf,i);
	}

    /* Severity */
    if(btype == EREQ) 	strcpy(ibuf,"e");
    else if(btype == COMPAT) doQ(ibuf,SEVERITYC);
    else doQ(ibuf,SEVERITY);
    severity = ibuf[0];
    if(severity == 'w') strcpy(ibuf,"ok");
    sprintf(tbuf,"# Severity: %s\n",ibuf);
    Write(bfile,tbuf,strlen(tbuf));

/**
 **  Get KS and WB vers
 **/

    if(btype != EREQ)
	{
    	if(ifile = Open(ReportKSFile,MODE_OLDFILE))
	    {
	    rlen = Read(ifile,ksver,36);
            ksver[rlen] = '\0';
	    Close(ifile);  ifile = NULL;
	    }
    	if(ifile = Open(ReportWBFile,MODE_OLDFILE))
	    {
	    rlen = Read(ifile,wbver,36);
            wbver[rlen] = '\0';
	    Close(ifile);  ifile = NULL;
	    }

	showos();	/* will fill in ksver and wbver if still unset */

    	i = 0;
	if(ksver[0])
	    sprintf(tbuf,"\nEnter Kickstart version.rev (default %s): ", ksver);
	else
	    sprintf(tbuf,"\nEnter Kickstart version.rev: ");
    	doPQA(ibuf,NULL,tbuf,NULL);
	if((ksver[0]) && (ibuf[0] == '\0')) strcpy(ibuf,ksver);
	else if(ofile = Open(ReportKSFile,MODE_NEWFILE))
	    {
	    Write(ofile,ibuf,strlen(ibuf));
	    Close(ofile);   ofile = NULL;
	    }
    	i += sprintf(obuf,"# Release: KS=%s,",ibuf);

	if(wbver[0])
	    sprintf(tbuf,"Enter Workbench version.rev (default %s): ", wbver);
	else
	    sprintf(tbuf,"Enter Workbench version.rev: ");
    	doPQA(ibuf,NULL,tbuf,NULL);
	if((wbver[0]) && (ibuf[0] == '\0')) strcpy(ibuf,wbver);
	else if(ofile = Open(ReportWBFile,MODE_NEWFILE))
	    {
	    Write(ofile,ibuf,strlen(ibuf));
	    Close(ofile);   ofile = NULL;
	    }
    	i += sprintf(&obuf[i],"WB=%s",ibuf);

	ibuf[0] = '\0';
	if(btype==BUG)
	    {
	    printf("\nIf bug location is a ROM library/device or 2.0 disk command/library/device,\n");
	    printf("use the VERSION command to find the program's internal version.\n");
	    printf("i.e. the output of VERSION commandname | name.library | name.device 0 0 unit\n");
	    printf("Or enter ?<RET> if you want to query the version now on this machine.\n");
GetVer:
    	    doPQA(ibuf,NULL,"Enter program's internal version.rev (or ?, or <RET> if n.a.): ",NULL);
	    if(!(strcmp(ibuf,"?")))
		{
		doPQA(ibuf,NULL,"Enter path/command, name.library, or name.device 0 0 unit: ",NULL);
		sprintf(tbuf,"version %s",ibuf);
		Execute(tbuf,0,0);
		goto GetVer;
		}		
	    }
	if(ibuf[0])  i += sprintf(&obuf[i],",Program=%s\n",ibuf);
	else i += sprintf(&obuf[i],"\n");
    	Write(bfile,obuf,strlen(obuf));
	}

    /* Date */
    Execute("date > t:bdate",0,0);
    if(ifile = Open("t:bdate",MODE_OLDFILE))
	{
	printf("inserting date...\n");
	rlen = Read(ifile,ibuf,IBUFSIZE);
	ibuf[rlen-1] = '\0';
	i = 0;
	i += sprintf(tbuf,"# Date: %s\n",ibuf);
	Write(bfile,tbuf,strlen(tbuf));
	Close(ifile);  ifile = NULL;
	}

    /**
     **  Get or create generic ReportSender file
     **/
    if(!(ifile = Open(ReportSenderFile,MODE_OLDFILE)))
	{
        if(!(ifile = Open(ReportSenderFile,MODE_NEWFILE)))
	    {
	    printf("\nCan't create %s\n",ReportSenderFile);
	    bye("",RETURN_FAIL);
	    }
        printf("\nCreating %s for use in reports\n",ReportSenderFile);
	for(i=0, k=0; k < BICNT; k++)
	    {
	    doQ(ibuf,&bi[k]);
	    i += sprintf(&tbuf[i],bi[k].o,ibuf);
	    }
        Write(ifile,tbuf,i);
        Close(ifile);
	ifile = Open(ReportSenderFile,MODE_OLDFILE);
        }

    /**
     **  Copy ReportSenderFile to bugfile
     **/
    printf("inserting %s...\n",ReportSenderFile);
    while(rlen = Read(ifile,ibuf,IBUFSIZE)) Write(bfile,ibuf,rlen);
    Close(ifile);  ifile = NULL;

    /**
     **  Get or create ReportConfigFile
     **/
    if(!(ifile = Open(ReportConfigFile,MODE_OLDFILE)))
	{
        if(!(ifile = Open(ReportConfigFile,MODE_NEWFILE)))
	    {
	    printf("\nCan't create %s\n",ReportConfigFile);
	    bye("",RETURN_FAIL);
	    }
        printf("\nCreating %s for use in reports\n",ReportConfigFile);
	for(i=0, k=0; k < BCCNT; k++)
	    {
	    doQ(ibuf,&bc[k]);
	    i += sprintf(&tbuf[i],bc[k].o,ibuf);
	    }
        Write(ifile,tbuf,i);
        Close(ifile);
	ifile = Open(ReportConfigFile,MODE_OLDFILE);
        }

    /**
     **  Copy ReportConfigFile to bugfile
     **/
    printf("inserting %s... (If incorrect, delete from s:)\n",ReportConfigFile);
    while(rlen = Read(ifile,ibuf,IBUFSIZE)) Write(bfile,ibuf,rlen);
    Close(ifile);  ifile = NULL;


    /**
     **  Add standard report lines
     **/
    i=0;

    if(btype == EREQ)
	{
	i += sprintf(&tbuf[i],
	  "\n### ENHANCEMENT REQUEST:\n\n\n");   
	}
    else
	{
	if(severity != 'w')
	    {
	    i += sprintf(&tbuf[i],
	      "\n### BRIEF BUG DESCRIPTION:\n\n\n");   
	    i += sprintf(&tbuf[i],
	      "\n### BUG GENERATION PROCEDURE OR EXAMPLE:\n\n\n");   
	    }


    	if(Detected[0])
	    {
    	    printf("\nDETECTED: %s\n",Detected);
    	    doPQA(ibuf,NULL,"These tools are detected. Correct for bug reports <y or n> ? ","yn");
	    if((ibuf[0] | 0x20) != 'y')
	    	{
	    	Detected[0] = '\0';
	    	}
	    }
    	if(Detected[0]=='\0')
	    {
    	    doPQA(ibuf,NULL,"Enter comma-separated names of debug tools and wedges you were using:\n",NULL);
	    strcpy(Detected,ibuf);
	    }

	i += sprintf(&tbuf[i],"\n### ALSO RUNNING:\n");
	i += sprintf(&tbuf[i],Detected);
	i += sprintf(&tbuf[i],"\n");

        i += sprintf(&tbuf[i],
	  "\n### IF THIS WORKS DIFFERENTLY ON OTHER VERSIONS OR HARDWARE, EXPLAIN:\n");
	}

    if((btype == COMPAT)&&(severity != 'w'))
	{
	i += sprintf(&tbuf[i],
	  "\n### WHAT DEVELOPER IS DOING IN THAT AREA OF SW OR HW:\n\n\n");   
	}
    if((btype != EREQ)&&(severity != 'w'))
	{
	i += sprintf(&tbuf[i],
	  "\n### RELATED PROBLEMS:\n\n\n");   
	}
    Write(bfile,tbuf,i);
    Close(bfile);   bfile=NULL;

    /**
     **  Let user edit file
     **/
FindEditor:
    sprintf(tbuf,"which >%s %s",TmpEdFind,edname);
    Execute(tbuf,NULL,NULL);
    if(ifile=Open(TmpEdFind,MODE_OLDFILE))
	{
	rlen = Read(ifile,ibuf,IBUFSIZE);
	Close(ifile); ifile = NULL;
	ibuf[rlen] = '\0';

	if((ibuf[0]=='\0')||((rlen>11)&&(!(strcmp(&ibuf[rlen-10],"not found\n")))))
	    {
	    printf("\nCan't find editor \"%s\"\n",edname);
	    printf("Add  SETENV EDITOR pathto/youreditor  to your user startup sequence\n");
	    doPQA(edname,NULL,"Meanwhile, enter the path/name of your editor: ",NULL);
	    if(edname[0]=='\0') strcpy(edname,defed);
	    goto FindEditor;
	    }
	}

    sprintf(tbuf,"%s %s",edname,bout);
    if(!(Execute(tbuf,NULL,NULL)))
	{
	}	
    printf("\nThank you.  Completed report is \"%s\"\n",bout);
    printf("Use \"report help\" for submission instructions\n");    
    bye("",0L);
    }



unsigned int doQ(UBYTE *buf, struct QA *qa)
    {
    return(doPQA(buf, qa->p, qa->q, qa->a));
    }


unsigned int doPQA(UBYTE *buf, UBYTE *p, UBYTE *q, UBYTE *a)
    {
    if(p) printf(p);

doQloop:
    if(SetSignal(0,0) & SIGBREAKF_CTRL_C) abort();
    if(q)
	{
	printf(q);
	gets(buf);
	if(!okA(buf,a)) goto doQloop;
	}
    if(SetSignal(0,0) & SIGBREAKF_CTRL_C) abort();
    return(strlen(buf));
    }



int okA(UBYTE *buf, UBYTE *a)
    {
    if(!a)  return(1);
    while(*a) if(*buf == *a++) return(1);
    return(0);
    }

	
void abort()
    {
    printf("abort...\n");
    bye("",RETURN_WARN);
    }

void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }


void cleanup()
    {
    if(ofile)	Close(ofile);
    if(ifile)	Close(ifile);
    if(bfile)	Close(bfile);
    DeleteFile("t:bdate");
    DeleteFile(TmpEdFind);
    if(ibuf)	FreeMem(ibuf, BUFSIZE);
    }


void showos()
    {
    extern struct Library *SysBase;
    struct Library *lib;
    ULONG  romstart;
    UWORD  ksv,ksr,wbv,wbr;
    BOOL   kickit;

    romstart = ((ULONG)(SysBase->lib_IdString)) & 0xFFFF0000;
    kickit = (romstart == 0x00200000) ? TRUE : FALSE;    

    ksv = *(UWORD *)(romstart + 0x000c);
    ksr = *(UWORD *)(romstart + 0x000e);

    if(lib = OpenLibrary("version.library",0L))
	{
	wbv = lib->lib_Version;
	wbr = lib->lib_Revision;
	CloseLibrary(lib);
	}
    else wbv = wbr = NULL;
    printf("\n(Note - This machine is running  KS=%d.%d  WB=%d.%d)\n",ksv,ksr,wbv,wbr);
    if(ksver[0]=='\0') sprintf(ksver,"%d.%d",ksv,ksr);
    if(wbver[0]=='\0') sprintf(wbver,"%d.%d",wbv,wbr);
    }





/* Passed a null terminated array of names, and an array of UBYTES
 * with one element per name, sets the UBYTE array to show which
 * names were found as command or task names
 */
void find_tasks(UBYTE **names, UBYTE *found)
    {
    extern struct ExecBase *SysBase;
    struct Process *proc;
    struct Task *task;
    struct CommandLineInterface *cli;
    UBYTE *name, *bname;
    int k;

    for(k=0; names[k]; k++) found[k] = 0;

    Disable();

    for ( task = (struct Task *)SysBase->TaskWait.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	for(k=0; names[k]; k++)
	    {
	    name = names[k];
	    if(!(stricmp(task->tc_Node.ln_Name,name)))
		{
		found[k] = 1;
		}
	    else if(task->tc_Node.ln_Type == NT_PROCESS)
		{
		proc = (struct Process *)task;
		if(proc->pr_CLI)
		    {
		    cli=(struct CommandLineInterface *)(BADDR(proc->pr_CLI));
		    if(cli->cli_CommandName)
		    	{
		    	bname = (UBYTE *)BADDR(cli->cli_CommandName);
		    	if(!(stricmp(&bname[1],name)))  found[k]=1;
		    	}
		    }
		}
	    }
        }


    for ( task = (struct Task *)SysBase->TaskReady.lh_Head;
          (NULL != task->tc_Node.ln_Succ);
          task = (struct Task *)task->tc_Node.ln_Succ)
        {
	for(k=0; names[k]; k++)
	    {
	    name = names[k];
	    if(!(stricmp(task->tc_Node.ln_Name,name)))
		{
		found[k] = 1;
		}
	    else if(task->tc_Node.ln_Type == NT_PROCESS)
		{
		proc = (struct Process *)task;
		if(proc->pr_CLI)
		    {
		    cli=(struct CommandLineInterface *)(BADDR(proc->pr_CLI));
		    if(cli->cli_CommandName)
		    	{
		    	bname = (UBYTE *)BADDR(cli->cli_CommandName);
		    	if(!(stricmp(&bname[1],name)))  found[k]=1;
		    	}
		    }
		}
	    }
        }


    Enable();
    }
