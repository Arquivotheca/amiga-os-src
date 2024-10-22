/* sendreport.c
 *
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/

#include "Categories.h"
#include "ModuleOwners.h"

/*****************************************************************************/

#include "sendreport_rev.h"
UBYTE *vers = VERSTAG;

/*****************************************************************************/

int CXBRK (void)
{
    return (0);
}
int chkabort (void)
{
    return (0);
}

/*****************************************************************************/

int okA (UBYTE * buf, UBYTE * ma);
int doQ (UBYTE * buf, struct QA * qa);
int doPQA (UBYTE * buf, UBYTE * p, UBYTE * q, UBYTE * a);

void bye (UBYTE * s, int e);
void cleanup (void);
void showos (void);

/*****************************************************************************/

#define defed			"TURBOTEXT:TTX WAIT"
#define defmail			"inet:smtp/sendmail -h cbmvax"
#define	defuser			"unknown"

/*****************************************************************************/

#define ReportSenderFile	"s:Report.sender"
#define ReportConfigFile	"s:Report.config"
#define ReportKSFile		"s:Report.ks"
#define ReportWBFile		"s:Report.wb"
#define TmpEdFind		"t:Report.e"
#define	ReportLog		"s:SendReport.log"

/*****************************************************************************/

UBYTE *bout = "ram:Report.txt";

/*****************************************************************************/

#define MINARGS 1

/*****************************************************************************/

UBYTE *Copyright =
VERSTAG \
"\nCopyright (c) 1992 Commodore-Amiga, Inc.\nAll Rights Reserved\n";

/*****************************************************************************/

UBYTE *usage = "Usage: sendreport [outfile] [help] (default outfile is ram:Report.txt)";

/*****************************************************************************/

UBYTE *helplines[] =
{
    "Generates Bug Reports, Compatibility Reports, and Enhancement Requests\n\n",

    "Report creates/uses s:Report.sender,s:Report.config,s:Report.ks,Report.wb\n",
    "\nPLEASE - DO NOT fill in the Subsystem field or any other restricted value\n",
    "fields by hand!  Only the REPORT program can validate these fields.\n\n",

    "Default output filename is ram:Report.txt (or you may specify a filename).\n\n",

    "Completed bug reports will be mailed to bugs@cbmvax, module owners and\n",
    "cc'ed to you.  To always cc others, use setenv CCBUGS <names>.  Note that\n",
    "the names must be separated by spaces, not commas.\n\n",

    "Default editor is TurboText.  You may specify a different editor using\n",
    "setenv EDITOR <editorname>.  Note that the editor command must NOT return\n",
    "until the document is saved (or canceled).\n\n",

    "Default mailer is Willey Langeveld's sendmail, using\n",
    "\"inet:smtp/sendmail -h cbmvax\".  You may specify a different mailer\n",
    "using setenv SENDMAIL <command>.\n",

    NULL
};

/*****************************************************************************/

/**
 **  The Question-Answer structure
 **/
struct QA
{
    UBYTE *p;			/* prompt or instructions */
    UBYTE *q;			/* question */
    UBYTE *a;			/* optional string of allowable 1 char responses */
    UBYTE *o;			/* optional output formatting string */
};

/*****************************************************************************/

/**
 **  Q-A's for ReportSenderFile
 **/
#define BICNT 5
struct QA bi[] =
{
    {NULL, "Lastname,FirstName (no spaces) -> ", NULL, "# Refer: %s "},
    {NULL, "Company Name (without Inc., Corp, etc.) -> ", NULL, "(%s "},
    {NULL, "(AreaCode) Phone-Number -> ", NULL, ",phone %s)\n"},
    {"Please enter your best electronic mail address.\n"
     "If UUCP, just enter the address,  for example: jsmith@endor.COM\n"
     "If other, enter address (SYSTEM), for example: jsmith (BIX)\n",
     "Enter electronic address (<RET> if none) -> ", NULL, "# Path: %s\n"},
    {NULL, "Developer Number -> ", NULL, "# ReferID: %s\n"}
};

/*****************************************************************************/

/**
 **  Q-A's for ReportConfigFile
 **/
#define BCCNT 9
struct QA bc[] =
{
    {NULL, "Model (a500,a1000,a2000,a2500,a3000)-> ", NULL, "# Config: %s,"},
    {NULL, "Processor (68000,68020,68030,68040)-> ", NULL, "%s,"},
    {NULL, "Agnus type (ECS,ECS-pal,old,old-pal) -> ", NULL, "A=%s,"},
    {NULL, "Denise type (ECS,old) -> ", NULL, "D=%s,"},
    {NULL, "Total Megs of CHIP ram -> ", NULL, "RAM=%smegC"},
    {NULL, "Total Megs of FAST ram -> ", NULL, "/%smegF,"},
    {NULL, "Number of floppies -> ", NULL, "TD=%s,"},
    {NULL, "Hard disk controller (a2090,a2090a,a2091, etc) -> ", NULL, "HD=%s,"},
    {NULL, "Other expansion cards-> ", NULL, "%s\n"},
};

/*****************************************************************************/

/**
 **  Q-A's for Compat Reports Only
 **/
#define COCNT 3
struct QA co[] =
{
    {NULL, "\n3rd Party Product Name-> ", NULL, "# ProductName: %s"},
    {NULL, "3rd Party Product Version-> ", NULL, " (vers=%s)\n"},
    {NULL, "Manufacturer of Product (without Inc.,Corp., etc)-> ", NULL, "# ProductInfo: %s\n"},
};

/*****************************************************************************/

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
    {"\nCBM Bug and Enhancement Report Generator\n"
     "----------------------------------------\n"
     " 1. System Software or Hardware Bug Report\n"
     " 2. Third Party Product Compatibility\n"
     " 3. Enhancement Request\n",
     "Enter 1-3: ", "123", NULL
    },
 /* 1 */
    {NULL, "\nEnter a meaningful 1-line summary:\n", NULL, NULL},
 /* 2 */
    {"\nClassification:\n"
     " a. Crashes, Hangs, or Corrupts Data\n"
     " b. Behaves incorrectly\n"
     " c. Cosmetic problem\n",
     "Enter appropriate letter: ", "abc", NULL
    },
 /* 3 */
    {"\nClassification:\n"
     " a. Crashes, Hangs, or Corrupts Data\n"
     " b. Behaves incorrectly\n"
     " c. Cosmetic problem\n"
     " w. Works correctly\n",
     "Enter appropriate letter: ", "abcw", NULL
    },

};

/*****************************************************************************/

#define IBUFSIZE 1024
#define BUFSIZE  (IBUFSIZE << 4)

/*****************************************************************************/

UBYTE *ibuf, *tbuf, *obuf, *cbuf;
BPTR ifile, ofile, bfile, lfile;

#define BUG	0
#define COMPAT	1
#define EREQ	2

ULONG btype = 0;
UBYTE *rtypes[] =
{"bug", "compat", "enhancement"};
UBYTE *longrtypes[] =
{"System Bug Report", "Compatibility Report", "Enhancement Request"};

UBYTE *subsys1 =
"\n-------------------------- Valid Subsystems --------------------------------\n";
UBYTE *subsys2 =
"* NOTE * If you don't see specific command name, use cdos.command for 2.0 C:\n";
UBYTE *subsys3 =
"commands, util.command for Utilities/, sys.command for System/, tools for\n";
UBYTE *subsys4 =
"Tools/, and just hit return if you are not sure of correct subsystem.\n";
UBYTE *subsys5 =
"Please categorize all include and autodoc bugs under their OS subsystem name.\n";
UBYTE *line =
"-----------------------------------------------------------------------------\n";

UBYTE *fsubsys = "%-16s%-16s%-15s%-15s%-15s\n";

/*****************************************************************************/

UBYTE ksver[40], wbver[40];

/*****************************************************************************/

struct ReportForm
{
    struct FileInfoBlock rf_FIB;
    BPTR rf_Lock;
    struct DateStamp rf_ODate;
    struct DateStamp rf_NDate;

    /* Mail header */
    UBYTE rf_Mailer[300];
    UBYTE rf_From[60];
    UBYTE rf_Subject[80];
    UBYTE rf_To[400];
    UBYTE rf_Cc[400];

    /* Bug report */
    UBYTE rf_SubSystem[128];
    UBYTE rf_Type[80];
    UBYTE rf_Severity[80];
    UBYTE rf_Version[80];
    UBYTE rf_Date[256];
};

/*****************************************************************************/

struct ReportForm *rf;
struct ModuleOwners *cmo;

extern struct Library *DOSBase, *SysBase;
struct Library *IntuitionBase;

/*****************************************************************************/

void main (int argc, char **argv)
{
    int k, i, l, b, perc, cols, maxi, offs, blocks, subcnt;
    LONG rlen;
    UBYTE *ss;
    UBYTE *subs[5];
    UBYTE ofnbuf[128];
    UBYTE edname[80];
    UBYTE severity;
    BOOL DoneSubs, SubMatch;
    struct TagItem tagitem[10] =
    {
	{SYS_Input, NULL},
	{SYS_Output, NULL},
	{SYS_Asynch, FALSE},
	{SYS_UserShell, TRUE},
	{NP_Priority, 0},
	{NP_StackSize, 4096},
	{NP_CloseInput, FALSE},
	{NP_CloseOutput, FALSE},
	{TAG_DONE, NULL},
    };

    if (IntuitionBase = OpenLibrary ("intuition.library", 37))
    {
	bfile = ifile = ofile = NULL;
	ibuf = NULL;
	ksver[0] = '\0';
	wbver[0] = '\0';

	if (((argc) && (argc < MINARGS)) || ((argc == 2) && ((argv[1][0] == '?'))))
	{
	    printf ("%s\n%s\n", Copyright, usage);
	    bye ("", RETURN_OK);
	}
	if ((argc > 1) && (!strcmp (argv[argc - 1], "help")))
	{
	    printf ("%s\n%s\n", Copyright, usage);
	    for (k = 0; helplines[k]; k++)
		printf (helplines[k]);
	    bye ("", RETURN_OK);
	}

	if (!(ibuf = (UBYTE *) AllocMem (BUFSIZE, MEMF_CLEAR)))
	    bye ("Not enough memory\n", RETURN_FAIL);

	if (!(rf = AllocMem (sizeof (struct ReportForm), MEMF_CLEAR)))
	    bye ("Not enough memory\n", RETURN_FAIL);

	tbuf = ibuf + IBUFSIZE;
	cbuf = tbuf + IBUFSIZE;
	obuf = cbuf + IBUFSIZE;


	if (argc == 2)
	    bout = argv[1];

	/****************/
	/* Get who from */
	/****************/
	if (ifile = Open ("ENV:user", MODE_OLDFILE))
	{
	    rlen = Read (ifile, rf->rf_From, sizeof (rf->rf_From));
	    rf->rf_From[rlen] = '\0';
	    Close (ifile);
	    ifile = NULL;
	}
	else
	    strcpy (rf->rf_From, defuser);
	strcpy (rf->rf_Cc, rf->rf_From);

	/* Default who to */
	strcpy (rf->rf_To, "bugs@cbmvax");

	/********************/
	/* Get who to cc to */
	/********************/
	if (ifile = Open ("ENV:CCBUGS", MODE_OLDFILE))
	{
	    rlen = Read (ifile, tbuf, 200);

	    /* Close the file */
	    Close (ifile);
	    ifile = NULL;

	    /* Tack the names onto the Cc list */
	    tbuf[rlen] = 0;
	    strcat (rf->rf_Cc, " ");
	    strcat (rf->rf_Cc, tbuf);
	}

	/******************/
	/* Get the mailer */
	/******************/
	if (ifile = Open ("ENV:sendmail", MODE_OLDFILE))
	{
	    rlen = Read (ifile, rf->rf_Mailer, sizeof (rf->rf_Mailer));
	    rf->rf_Mailer[rlen] = '\0';
	    Close (ifile);
	    ifile = NULL;
	}
	else
	{
	    strcpy (rf->rf_Mailer, defmail);
	}

	/***********************/
	/* Get the editor name */
	/***********************/
	if (ifile = Open ("ENV:Editor", MODE_OLDFILE))
	{
	    rlen = Read (ifile, edname, sizeof (edname));
	    edname[rlen] = '\0';
	    Close (ifile);
	    ifile = NULL;
	}
	else
	{
	    strcpy (edname, defed);
	}

/**
 **  Create bug report output file
 **/

      GetBFile:
	while (!bout)
	{
	    if (doPQA (ofnbuf, NULL, "Filename for output: ", NULL))
		bout = ofnbuf;
	}
	if (bfile = Open (bout, MODE_OLDFILE))
	{
	    Close (bfile);
	    bfile = NULL;
	    sprintf (tbuf, "Output file %s already exists.  Overwrite <y or n> ? ", bout);
	    doPQA (ibuf, NULL, tbuf, "yn");
	    if ((ibuf[0] | 0x20) != 'y')
	    {
		bout = NULL;
		goto GetBFile;
	    }
	}

	if (!(bfile = Open (bout, MODE_NEWFILE)))
	{
	    printf ("\nCan't open %s for output\n", bout);
	    bout = NULL;
	    goto GetBFile;
	}


	/**
         **  Get report type, summary, date, OS versions, severity
         **/

	/* Type */
	doQ (ibuf, BUGTYPE);
	btype = atoi (ibuf) - 1;

	printf ("%s selected.\n", longrtypes[btype]);


	/* Summary */
	doQ (ibuf, SUMMARY);
	strcpy (rf->rf_Subject, ibuf);
	strcpy (rf->rf_Type, rtypes[btype]);

	/**
         **  Get additional info for Bug Reports and Enhancement requests
         **/
	if (btype != COMPAT)
	{
	    for (k = 0; Categories[k]; k++) ;
	    subcnt = k;
	    cols = 5;

	    ibuf[0] = '\0';
	    doPQA (ibuf, NULL,
		   "Enter subsystem, or <RET> if not sure, or ?<RET> to see list: ", NULL);

	    goto getSubSystem;

	  SubLoop:
	    perc = 16;
	    maxi = (subcnt + (cols - 1)) / cols;
	    offs = 0;
	    blocks = (subcnt + ((cols * perc) - 1)) / (cols * perc);
	    DoneSubs = FALSE;
	    i = 0;
	    printf (subsys1);
	    for (b = 0; (b < blocks) && (!DoneSubs); b++)
	    {
		offs = b * (perc * cols);
		perc = MIN (perc, (maxi - i));
		for (l = 0; (l < perc) && (!DoneSubs); l++)
		{
		    for (k = 0; k < cols; k++)
		    {
			if (ss = Categories[l + offs + (perc * k)])
			    subs[k] = ss;
			else
			    subs[k] = "";
		    }
		    printf (fsubsys, subs[0], subs[1], subs[2], subs[3], subs[4]);
		    i++;	/* counts lines printed */
		    if (i == maxi)
			DoneSubs = TRUE;
		}
		if (!DoneSubs)
		{
		    printf ("Press RETURN for More...");
		    while (getchar () != '\n') ;
		}
	    }
	    printf (line);
	    printf (subsys2);
	    printf (subsys3);
	    printf (subsys4);
	    printf (subsys5);
	    printf ("%s\n", line);

	    ibuf[0] = '\0';
	    doPQA (ibuf, NULL,
		   "Enter correct subsystem from list,\n   or <RET> if not sure, or ?<RET> to review list: ", NULL);

	  getSubSystem:

	    if (ibuf[0] == '?')
		goto SubLoop;

	    SubMatch = ibuf[0] ? FALSE : TRUE;
	    if (ibuf[0])
	    {
		for (k = 0; Categories[k]; k++)
		{
		    if (!(stricmp (ibuf, Categories[k])))
		    {
			SubMatch = TRUE;
			break;
		    }
		}
	    }
	    if (!SubMatch)
	    {
		printf ("%s is not a listed subsystem\n", ibuf);
		goto SubLoop;
	    }
	    strcpy (rf->rf_SubSystem, ibuf);
	    for (i = 0, cmo = NULL; mo[i].mo_Module && (cmo == NULL); i++)
	    {
		if (!(stricmp (ibuf, mo[i].mo_Module)))
		{
		    cmo = &mo[i];
		    sprintf (rf->rf_To, "bugs@cbmvax %s", cmo->mo_Owners);
		}
	    }
	}

	/**
         **  Get additional info for Compatibility Reports
         **/
	if (btype == COMPAT)
	{
	    /* Who from */
	    sprintf (tbuf, "From: %s\n", rf->rf_From);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "Subject: %s\n", rf->rf_Subject);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "To: %s\nCc: %s\n", rf->rf_To, rf->rf_Cc);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "X-Mailer: Report [version %d.%d (%s)]\n\n", VERSION, REVISION, DATE);
	    Write (bfile, tbuf, strlen (tbuf));

	    /* Write the subject line */
	    sprintf (tbuf, "# Subject: %s\n# Type: %s\n", rf->rf_Subject, rf->rf_Type);
	    Write (bfile, tbuf, strlen (tbuf));

	    for (i = 0, k = 0; k < COCNT; k++)
	    {
		doQ (ibuf, &co[k]);
		i += sprintf (&tbuf[i], co[k].o, ibuf);
	    }
	    Write (bfile, tbuf, i);
	}

	/* Severity */
	if (btype == EREQ)
	    strcpy (ibuf, "e");
	else if (btype == COMPAT)
	    doQ (ibuf, SEVERITYC);
	else
	    doQ (ibuf, SEVERITY);
	severity = ibuf[0];
	if (severity == 'w')
	    strcpy (ibuf, "ok");
	strcpy (rf->rf_Severity, ibuf);

/**
 **  Get KS and WB vers
 **/

	if (btype != EREQ)
	{
	    if (ifile = Open (ReportKSFile, MODE_OLDFILE))
	    {
		rlen = Read (ifile, ksver, 36);
		ksver[rlen] = '\0';
		Close (ifile);
		ifile = NULL;
	    }
	    if (ifile = Open (ReportWBFile, MODE_OLDFILE))
	    {
		rlen = Read (ifile, wbver, 36);
		wbver[rlen] = '\0';
		Close (ifile);
		ifile = NULL;
	    }

	    showos ();		/* will fill in ksver and wbver if still unset */

	    i = 0;
	    if (ksver[0])
		sprintf (tbuf, "\nEnter Kickstart version.rev (default %s): ", ksver);
	    else
		sprintf (tbuf, "\nEnter Kickstart version.rev: ");
	    doPQA (ibuf, NULL, tbuf, NULL);
	    if ((ksver[0]) && (ibuf[0] == '\0'))
		strcpy (ibuf, ksver);
	    else if (ofile = Open (ReportKSFile, MODE_NEWFILE))
	    {
		Write (ofile, ibuf, strlen (ibuf));
		Close (ofile);
		ofile = NULL;
	    }
	    i += sprintf (obuf, "# Release: KS=%s,", ibuf);

	    if (wbver[0])
		sprintf (tbuf, "Enter Workbench version.rev (default %s): ", wbver);
	    else
		sprintf (tbuf, "Enter Workbench version.rev: ");
	    doPQA (ibuf, NULL, tbuf, NULL);
	    if ((wbver[0]) && (ibuf[0] == '\0'))
		strcpy (ibuf, wbver);
	    else if (ofile = Open (ReportWBFile, MODE_NEWFILE))
	    {
		Write (ofile, ibuf, strlen (ibuf));
		Close (ofile);
		ofile = NULL;
	    }
	    i += sprintf (&obuf[i], "WB=%s", ibuf);

	    ibuf[0] = '\0';
	    if (btype == BUG)
	    {
		printf ("\nIf bug location is a ROM library/device or 2.0 disk command/library/device,\n");
		printf ("use the VERSION command to find the program's internal version.\n");
		printf ("i.e. the output of VERSION commandname | name.library | name.device 0 0 unit\n");
		printf ("Or enter ?<RET> if you want to query the version now on this machine.\n");
	      GetVer:
		doPQA (ibuf, NULL, "Enter program's internal version.rev (or ?, or <RET> if n.a.): ", NULL);
		if (!(strcmp (ibuf, "?")))
		{
		    doPQA (ibuf, NULL, "Enter path/command, name.library, or name.device 0 0 unit: ", NULL);
		    sprintf (tbuf, "version %s", ibuf);
		    Execute (tbuf, 0, 0);
		    goto GetVer;
		}
	    }
	    if (ibuf[0])
		i += sprintf (&obuf[i], ",Program=%s\n", ibuf);
	    else
		i += sprintf (&obuf[i], "\n");
	    strcpy (rf->rf_Version, obuf);
	}

	/* Date */
	Execute ("date > ram:bdate", 0, 0);
	if (ifile = Open ("ram:bdate", MODE_OLDFILE))
	{
	    printf ("inserting date...\n");

	    rlen = Read (ifile, rf->rf_Date, sizeof (rf->rf_Date));
	    Close (ifile);
	    ifile = NULL;

	    rf->rf_Date[(rlen-1)] = 0;
	}

	/*********************/
	/* Output the report */
	/*********************/
	if (btype != COMPAT)
	{
	    /* Who from */
	    sprintf (tbuf, "From: %s\n", rf->rf_From);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "Subject: %s\n", rf->rf_Subject);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "To: %s\nCc: %s\n", rf->rf_To, rf->rf_Cc);
	    Write (bfile, tbuf, strlen (tbuf));

	    sprintf (tbuf, "X-Mailer: Report [version %d.%d (%s)]\n\n", VERSION, REVISION, DATE);
	    Write (bfile, tbuf, strlen (tbuf));

	    /* Write the subject line */
	    sprintf (tbuf, "# Subject: %s\n# Type: %s\n", rf->rf_Subject, rf->rf_Type);
	    Write (bfile, tbuf, strlen (tbuf));

	    /* Write the subsystem line */
	    sprintf (tbuf, "# Subsystem: %s\n", rf->rf_SubSystem);
	    Write (bfile, tbuf, strlen (tbuf));

	    /* Write the severity */
	    sprintf (tbuf, "# Severity: %s\n", rf->rf_Severity);
	    Write (bfile, tbuf, strlen (tbuf));
	}
	if (btype != EREQ)
	{
	    Write (bfile, rf->rf_Version, strlen (rf->rf_Version));
	}

	sprintf (tbuf, "# Date: %s\n", rf->rf_Date);
	Write (bfile, tbuf, strlen (tbuf));

	/**
         **  Get or create generic ReportSender file
         **/
	if (!(ifile = Open (ReportSenderFile, MODE_OLDFILE)))
	{
	    if (!(ifile = Open (ReportSenderFile, MODE_NEWFILE)))
	    {
		printf ("\nCan't create %s\n", ReportSenderFile);
		bye ("", RETURN_FAIL);
	    }
	    printf ("\nCreating %s for use in reports\n", ReportSenderFile);
	    for (i = 0, k = 0; k < BICNT; k++)
	    {
		doQ (ibuf, &bi[k]);
		i += sprintf (&tbuf[i], bi[k].o, ibuf);
	    }
	    Write (ifile, tbuf, i);
	    Close (ifile);
	    ifile = Open (ReportSenderFile, MODE_OLDFILE);
	}

	/**
         **  Copy ReportSenderFile to bugfile
         **/
	printf ("inserting %s...\n", ReportSenderFile);
	while (rlen = Read (ifile, ibuf, IBUFSIZE))
	    Write (bfile, ibuf, rlen);
	Close (ifile);
	ifile = NULL;

	/**
         **  Get or create ReportConfigFile
         **/
	if (!(ifile = Open (ReportConfigFile, MODE_OLDFILE)))
	{
	    if (!(ifile = Open (ReportConfigFile, MODE_NEWFILE)))
	    {
		printf ("\nCan't create %s\n", ReportConfigFile);
		bye ("", RETURN_FAIL);
	    }
	    printf ("\nCreating %s for use in reports\n", ReportConfigFile);
	    for (i = 0, k = 0; k < BCCNT; k++)
	    {
		doQ (ibuf, &bc[k]);
		i += sprintf (&tbuf[i], bc[k].o, ibuf);
	    }
	    Write (ifile, tbuf, i);
	    Close (ifile);
	    ifile = Open (ReportConfigFile, MODE_OLDFILE);
	}

	/**
         **  Copy ReportConfigFile to bugfile
         **/
	printf ("inserting %s...\n", ReportConfigFile);
	while (rlen = Read (ifile, ibuf, IBUFSIZE))
	    Write (bfile, ibuf, rlen);
	Close (ifile);
	ifile = NULL;


	/**
         **  Add standard report lines
         **/
	i = 0;

	if (btype == EREQ)
	{
	    i += sprintf (&tbuf[i],
			  "\n### ENHANCEMENT REQUEST:\n\n\n");
	}
	else
	{
	    if (severity != 'w')
	    {
		i += sprintf (&tbuf[i],
			      "\n### BRIEF BUG DESCRIPTION:\n\n\n");
		i += sprintf (&tbuf[i],
			      "\n### BUG GENERATION PROCEDURE OR EXAMPLE:\n\n\n");
	    }
	    i += sprintf (&tbuf[i],
			  "\n### IF THIS WORKS DIFFERENTLY ON OTHER VERSIONS OR HARDWARE, EXPLAIN:\n");
	}

	if ((btype == COMPAT) && (severity != 'w'))
	{
	    i += sprintf (&tbuf[i],
			  "\n### WHAT DEVELOPER IS DOING IN THAT AREA OF SW OR HW:\n\n\n");
	}
	if ((btype != EREQ) && (severity != 'w'))
	{
	    i += sprintf (&tbuf[i],
			  "\n### RELATED PROBLEMS:\n\n\n");
	}
	Write (bfile, tbuf, i);
	Close (bfile);
	bfile = NULL;

	/**
         **  Let user edit file
         **/
      FindEditor:
	sprintf (tbuf, "which >%s %s", TmpEdFind, edname);
	Execute (tbuf, NULL, NULL);
	if (ifile = Open (TmpEdFind, MODE_OLDFILE))
	{
	    rlen = Read (ifile, ibuf, IBUFSIZE);
	    Close (ifile);
	    ifile = NULL;
	    ibuf[rlen] = '\0';

	    if ((ibuf[0] == '\0') || ((rlen > 11) && (!(strcmp (&ibuf[rlen - 10], "not found\n")))))
	    {
		printf ("\nCan't find editor \"%s\"\n", edname);
		printf ("Add  SETENV EDITOR pathto/youreditor  to your user startup sequence\n");
		doPQA (edname, NULL, "Meanwhile, enter the path/name of your editor: ", NULL);
		if (edname[0] == '\0')
		    strcpy (edname, defed);
		goto FindEditor;
	    }
	}

	if (rf->rf_Lock = Lock (bout, ACCESS_READ))
	{
	    if (Examine (rf->rf_Lock, &rf->rf_FIB))
		CopyMem (&rf->rf_FIB.fib_Date, &rf->rf_ODate, sizeof (struct DateStamp));
	    UnLock (rf->rf_Lock);
	}
	else
	{
	    printf ("unable to examine %s!!!\n", bout);
	}

	sprintf (tbuf, "%s %s", edname, bout);
	tagitem[0].ti_Data = Input ();
	tagitem[1].ti_Data = Output ();
	if (!System (tbuf, tagitem))
	{
	    if (rf->rf_Lock = Lock (bout, ACCESS_READ))
	    {
		if (Examine (rf->rf_Lock, &rf->rf_FIB))
		    CopyMem (&rf->rf_FIB.fib_Date, &rf->rf_NDate, sizeof (struct DateStamp));
		UnLock (rf->rf_Lock);
	    }
	    else
	    {
		printf ("unable to examine %s!!!\n", bout);
	    }

	    /* Successful */
	    if (CompareDates (&rf->rf_ODate, &rf->rf_NDate) > 0)
	    {
		sprintf (tbuf, "%s %s %s %s", rf->rf_Mailer, bout, rf->rf_To, rf->rf_Cc);
		if (!System (tbuf, tagitem))
		{
		    if (lfile = Open (ReportLog, MODE_READWRITE))
		    {
			Seek (lfile, 0, OFFSET_END);

			sprintf (tbuf, "%s\n\n", rf->rf_Date);
			Write (lfile, tbuf, strlen (tbuf));

			sprintf (tbuf, " o %s : %s\n\n", rf->rf_SubSystem, rf->rf_Subject);
			Write (lfile, tbuf, strlen (tbuf));

			Close (lfile);
		    }

		    printf ("\nThank you.  Completed report is \"%s\"\n", bout);
		    printf ("and has been sent to %s and %s\n", rf->rf_To, rf->rf_Cc);
		}
		else
		{
		    printf ("unable to send mail\n");
		}
	    }
	    else
	    {
		printf ("no change to %s\n", bout);
	    }
	}

	bye ("", 0L);
    }
    else
	bye ("Requires 2.x or beyond\n", RETURN_FAIL);
}

/*****************************************************************************/

int doQ (UBYTE * buf, struct QA * qa)
{

    return (doPQA (buf, qa->p, qa->q, qa->a));
}

/*****************************************************************************/

int doPQA (UBYTE * buf, UBYTE * p, UBYTE * q, UBYTE * a)
{

    if (p)
	printf (p);

  doQloop:
    if (SetSignal (0, 0) & SIGBREAKF_CTRL_C)
	abort ();
    if (q)
    {
	printf (q);
	gets (buf);
	if (!okA (buf, a))
	    goto doQloop;
    }
    if (SetSignal (0, 0) & SIGBREAKF_CTRL_C)
	abort ();
    return (strlen (buf));
}

/*****************************************************************************/

int okA (UBYTE * buf, UBYTE * a)
{

    if (!a)
	return (1);
    while (*a)
	if (*buf == *a++)
	    return (1);
    return (0);
}

/*****************************************************************************/

void abort ()
{

    printf ("abort...\n");
    bye ("", RETURN_WARN);
}

/*****************************************************************************/

void bye (UBYTE * s, int e)
{

    cleanup ();
    exit (e);
}

/*****************************************************************************/

void cleanup ()
{

    if (ofile)
	Close (ofile);
    if (ifile)
	Close (ifile);
    if (bfile)
	Close (bfile);
    if (ibuf)
	FreeMem (ibuf, BUFSIZE);
    if (rf)
	FreeMem (rf, sizeof (struct ReportForm));
}

/*****************************************************************************/

void showos ()
{
    extern struct Library *SysBase;
    struct Library *lib;
    ULONG romstart;
    UWORD ksv, ksr, wbv, wbr;
    BOOL kickit;

    romstart = ((ULONG) (SysBase->lib_IdString)) & 0xFFFF0000;
    kickit = (romstart == 0x00200000) ? TRUE : FALSE;

    ksv = *(UWORD *) (romstart + 0x000c);
    ksr = *(UWORD *) (romstart + 0x000e);

    if (lib = OpenLibrary ("version.library", 0L))
    {
	wbv = lib->lib_Version;
	wbr = lib->lib_Revision;
	CloseLibrary (lib);
    }
    else
	wbv = wbr = NULL;
    printf ("\n(Note - This machine is running  KS=%d.%d  WB=%d.%d)\n", ksv, ksr, wbv, wbr);
    if (ksver[0] == '\0')
	sprintf (ksver, "%d.%d", ksv, ksr);
    if (wbver[0] == '\0')
	sprintf (wbver, "%d.%d", wbv, wbr);
}
