
/* BEFORE RELEASING THIS, CHECK THE BEHAVIOR OF SEARCH ON STANDARD INPUT! */


#include "internal/commands.h"
#include "asyncio.h"
#include "search_rev.h"

void aprintf();

#define MSG_FILE_ABANDONED	"** File abandoned\n"
#define MSG_SRCH_ARG_ERR	"** SEARCH string too long\n"

#define TEMPLATE    "FROM/M,SEARCH/A,ALL/S,NONUM/S,QUIET/S,QUICK/S,FILE/S,PATTERN/S,COLSTART/K/N,COLEND/K/N" CMDREV
#define TEMPLATE2   "SEARCH/A,NONUM/S,PATTERN/S,COLSTART/K/N,COLEND/K/N"

#define OPT_COUNT	10
#define SEARCH_DEPTH	25
#define BLANK_COUNT	(SEARCH_DEPTH + 1) * 5
#define LINESIZE	256
#define FPATHSIZE	256

#define FROM	  0
#define SEARCH	 FROM+1
#define ALL	 SEARCH+1
#define NONUM	 ALL+1
#define QUIET	 NONUM+1
#define QUICK	 QUIET+1
#define FILE	 QUICK+1
#define PATTERN  FILE+1
#define COLSTART PATTERN+1
#define COLEND   COLSTART+1

/* NOBLANKNAME controls whether to print a blank line in place of the	*/
/* file name for an unambiguous search path (filename) to completely 	*/
/* emulate Dos 1.3  (defined) or omit the blank line (undefined).	*/
/* Default is now defined - ie no blank line				*/

#define NOBLANKNAME

struct search_global {			/* sort of allocated statics	*/
   long opts[OPT_COUNT];
   long crout,				/* last line had CR, not '\n'   */
	pdirs;				/* print dirs (& indent etc)    */
   struct AsyncFile *curfile;
   long pattern;			/* searching contents of file	*/
   long name;		 		/* matching filenames		*/
   long foundmatch;			/* init false, controls rc	*/
   long filtering;			/* called with input redirected */
   long noterm;				/* line not terminated, LF or 0 */
   long lcount;				/* current line number in curfile */
   long colstart;
   long colend;
   char search[LINESIZE];		/* processed search template	*/
   char line[LINESIZE+4];		/* current line from file	*/
   char temp[LINESIZE+4];		/* work space for a line	*/
   char blankline[BLANK_COUNT+1];	/* blankline[BLANK_COUNT] == '\0'*/
   char fpath[FPATHSIZE+1];		/* full path to current file	*/
   };

void prepargs(struct DosLibrary *DOSBase, struct search_global *glob);
int dosearch(struct DosLibrary *DOSBase, struct search_global *glob);
void TFPN(struct DosLibrary *DOSBase, struct search_global *glob, BPTR lock);
unsigned long StrMovToUpper(char *mptr, char *uptr);
int filter(struct DosLibrary *DOSBase, struct search_global *glob);
int getline(struct DosLibrary *DOSBase, struct search_global *glob);

int cmd_search(void)
/*	------								*/
{
struct DosLibrary *DOSBase;
struct Library *SysBase = (*((struct Library **) 4));
struct RDArgs *rdargs;
struct CommandLineInterface *cli;
int rc, rc2;
struct search_global glob;

rc = RETURN_FAIL;
if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
   memset((char *)&glob, '\0', sizeof(glob));
   memset((char *)&glob.blankline[0], ' ', BLANK_COUNT);

   /* if input is redirected, function as a filter			*/
   cli = Cli();
   glob.filtering = ((cli->cli_StandardInput != Input()));
   /* on second thought, what if we're running under AREXX and we have	*/
   /* a modified file handle for the same console???			*/
   if (glob.filtering)
      if (IsInteractive(cli->cli_StandardInput)
       && IsInteractive(Input()))
         if (((struct FileHandle *)BADDR(cli->cli_StandardInput))->fh_Type
          == ((struct FileHandle *)BADDR(Input()))->fh_Type)
            glob.filtering = 0;

   if (glob.filtering) {
      rdargs = ReadArgs(TEMPLATE2, (long *)&glob.opts[0], NULL);
      /* rearrange results to line up with standard template		*/
      glob.opts[COLEND]   = glob.opts[4];
      glob.opts[COLSTART] = glob.opts[3];
      glob.opts[PATTERN]  = glob.opts[2];
      glob.opts[NONUM]    = glob.opts[1];
      glob.opts[SEARCH]   = glob.opts[0];

      glob.opts[FROM]     = 0;
      glob.opts[ALL]      = 0;
      glob.opts[QUIET]    = 0;
      glob.opts[QUICK]    = 0;
      glob.opts[FILE]     = 0;
      }
   else
      rdargs = ReadArgs(TEMPLATE, (long *)&glob.opts[0], NULL);
   if (rdargs == NULL) {
      rc2 = IoErr();
      PrintFault(rc2, NULL);
      }
   else {
      if (strlen((char *)glob.opts[SEARCH]) > 255)
         PutStr(MSG_SRCH_ARG_ERR);
      else {
         prepargs(DOSBase, &glob);

         if (glob.filtering)
            rc2 = filter(DOSBase, &glob);
         else {
            rc2 = dosearch(DOSBase, &glob);
            if (glob.crout)
            /* Erase last unmatched filename (from QUICK option)	*/
            PutStr("\r\033[K");
            }
         if (rc2) PrintFault(rc2, NULL);
         else	/* everything went OK					*/
            if (glob.foundmatch) rc = RETURN_OK;
            else rc = RETURN_WARN;
         }
      FreeArgs(rdargs);
      }
   CloseLibrary((struct Library *)DOSBase);
   }
else {OPENFAIL;}
return(rc);
}

void prepargs(struct DosLibrary *DOSBase, struct search_global *glob)
/*  --------								*/
/* check arguments in command line for valid input and put things into	*/
/* easier form for processing.						*/
{
char *optr, *sptr;
unsigned long l;

sptr = glob->search;
optr = (char *)glob->opts[SEARCH];
if (glob->opts[FILE])
   /* search arg will be used to match file names			*/
   glob->name = 1;
else
   {
   /* search arg will be used to match file contents			*/
   glob->pattern = 1;
   *sptr++ = P_ANY;	/* match anywhere within a line in the file	*/
   }
if (glob->opts[PATTERN] || glob->opts[FILE])
   {
   ParsePattern(optr, glob->temp, LINESIZE);
   optr = glob->temp;
   }
l = StrMovToUpper(optr, sptr);
if (!glob->opts[FILE])
   sptr[l] = P_ANY;

if (glob->opts[COLSTART])
    glob->colstart = *(LONG *)glob->opts[COLSTART];
else
    glob->colstart = 0;

if (glob->opts[COLEND])
    glob->colend = *(LONG *)glob->opts[COLSTART];
else
    glob->colend = 0x7ffffffff;

/* do not support QUICK or QUIET with FILE				*/
if (glob->opts[FILE])
   {
   glob->opts[QUICK] = 0;
   glob->opts[QUIET] = 0;
   }

/* control layouts depending on interaction of options			*/

if (glob->pattern && (glob->opts[QUICK] == 0) && (glob->opts[QUIET] == 0))
   /* use indented layout when searching contents unless QUICK or QUIET */
   glob->pdirs = 1;

if (glob->opts[FILE] && (glob->name == 0))
   /* also use indented layout for FILE with no NAME argument		*/
   glob->pdirs = 1;
}

int dosearch(struct DosLibrary *DOSBase, struct search_global *glob)
/*  --------								*/
{
struct Library *SysBase = (*((struct Library **) 4));
struct AnchorPath *anchor;
long pargs[3];
char **optptr;
char *fptr;
char *indent;
char *twodots = "..\n";
char *erase_eol = "\033[K\r";
char *newln = "\n";
char *dirstr = " (dir)\n";
char *alldir[2];
BPTR oldlock;
int frc, dothisfile, sres, depth, getpath = 0;
int justonearg = FALSE;
int justonefile = FALSE;

if ((anchor = AllocVec(sizeof(struct AnchorPath), MEMF_CLEAR)) == 0)
	/* plausible description of what happened			*/
   return ERROR_NO_FREE_STORE;

optptr = (char **)glob->opts[FROM];
/* if user did not specify FROM (path) argument default to current dir	*/
if (!optptr || (*optptr == 0))
{
   alldir[0] = "#?";
   alldir[1] = NULL;
   optptr = alldir;
   }
else
   /* only one path argument given by user				*/
   if (optptr[1] == 0) justonearg = TRUE;

while ((fptr = *optptr++) != 0) {

   pargs[1] = (long)&(anchor->ap_Info.fib_FileName);
   indent = (char *)&(glob->blankline[BLANK_COUNT-3]);
   depth = 0;
   /* if not standard indenting, need to initialize directory path	*/
   if (!glob->pdirs)getpath = 1;
   memset((char *)anchor, 0, sizeof(struct AnchorPath));
   anchor->ap_BreakBits = SIGBREAKF_CTRL_C;
   anchor->ap_Flags = APF_DOWILD;

   if (MatchFirst(fptr, anchor))
      frc = IoErr();
   else {
      frc = 0;
      justonefile =
         (justonearg &&
         (anchor->ap_Info.fib_DirEntryType < 0) &&
	 (!(anchor->ap_Flags & APF_ITSWILD))) ;
   }

   while (frc == 0) {
      if (anchor->ap_Info.fib_DirEntryType < 0) {        /*file*/
         /* If directory lock has changed and we are printing the path	*/
         /* for every file (QUICK or QUIET option) refresh the path.	*/
         /* Also need to do something for standard indented format, but	*/
         /* a bit too complicated to throw in at the last minute, and	*/
         /* in an case would like more support from MatchFirst/Next or	*/
         /* utility routines to do the genealogy of the directory lock	*/
         if (anchor->ap_Flags & APF_DirChanged) {
            if (!glob->pdirs) getpath = 1;
            }
	 if (glob->name) {
	    StrMovToUpper(anchor->ap_Info.fib_FileName, glob->temp);
	    dothisfile = MatchPattern(glob->search, glob->temp);
            if (dothisfile && !glob->pattern) glob->foundmatch = DOSTRUE;
	    }
         else
            /* process all files if matching contents instead of names	*/
            dothisfile = 1;
	 if (dothisfile) {
	    pargs[0] = (long)indent;
	    if (glob->pdirs) {    /* default indenting style              */
	       pargs[2] = (long)twodots;
	    }
	    else if (glob->opts[QUICK]) {
		  pargs[2] = (long)erase_eol; /* erase to EOL, no lf    */
		  glob->crout = 1;	  /* missing CR outstanding	*/
	    }
	    else {	/* QUIET or most instances of FILE		*/
		pargs[2] = (long)newln;
	    }

            /* print the filename now unless QUIET option was specified */
            /* by itself or the search path was a single file		*/
	    if ((glob->opts[QUIET] == 0 || glob->opts[QUICK] != 0)) {
               if (justonefile)
                  #ifndef NOBLANKNAME
                  PutStr("\n")
                  #endif
                  ;
               else {
	          /* only invoke NameFromLock() if we need to              */
	          if (getpath) {
		     TFPN(DOSBase, glob, anchor->ap_Current->an_Lock);
		     indent = glob->fpath;
		     pargs[0] = (long)indent;
		     getpath = 0;
		     }
	          VPrintf("%s%s%s", pargs);
	          }
	       }
            /* see if we're matching file contents instead of names	*/
	    if (glob->pattern) {
	       oldlock = CurrentDir(anchor->ap_Current->an_Lock);
	       glob->curfile =
		     OpenAsync(anchor->ap_Info.fib_FileName, NULL, MODE_READ, 4096, (struct Library *)DOSBase, SysBase);
	       if (glob->curfile == 0)
                  PrintFault(IoErr(), anchor->ap_Info.fib_FileName);
	       else {
		  sres = 0;
		  glob->lcount =  0;
		  glob->noterm =  0;
		  while (getline(DOSBase, glob)) {
		     if (MatchPattern(glob->search, glob->temp)) {
                        glob->foundmatch = DOSTRUE;
                        /* no need to half shift this line		*/
                        if (glob->noterm)
                           glob->noterm = -1;

			if (glob->opts[QUIET]) {
                        /* print name now that we have a contents match	*/
                           /* if QUICK also specified, name is already  */
                           /* printed but without a carriage return     */
                           if (glob->opts[QUICK])
                              PutStr("\n");
			   /* only invoke NameFromLock() if we need to  */
			   if (getpath) {
			      TFPN(DOSBase, glob, anchor->ap_Current->an_Lock);
			      indent = glob->fpath;
			      pargs[0] = (long)indent;
			      getpath = 0;
			      }
			   VPrintf("%s%s%s", pargs);
			   break;	/* one match is all we need	*/
			   }
			else {
			   if (glob->crout) {
			      PutStr("\n");       /* need a line feed	*/
			      glob->crout = 0;
			      }
			   if (glob->opts[NONUM]==0) {
                              /* print line number			*/
                              VPrintf("%6ld ", &glob->lcount);
                              }

			   PutStr(glob->line);
			   }
			}
		     if (sres=CheckSignal(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D))
			break;	  /* close this file, and maybe quit	*/
		     }
		  CloseAsync(glob->curfile);
		  if (sres & SIGBREAKF_CTRL_C) frc = ERROR_BREAK;
		  else if (sres & SIGBREAKF_CTRL_D)
		     PutStr(MSG_FILE_ABANDONED);
		  }
       	       CurrentDir(oldlock);
	       }
	    }
	 }
      else {	/*  dir  */
	 if (anchor->ap_Flags & APF_DIDDIR) {
	    /* exiting a directory, clean up				*/
	    if (glob->pdirs) indent = indent+5;
	    else getpath = 1;
	       /* signal need to restore previous directory path	*/
	    depth = depth-1;
	    anchor->ap_Flags &= ~APF_DIDDIR;
	    }
	 else {
	    /* first time we've seen this directory - print it or enter */
	    /* it or whatever...					*/
	    if (glob->pdirs) {
	       pargs[0] = (long)indent - 2;
	       pargs[2] = (long)dirstr;
               if (anchor->ap_Info.fib_FileName[0] != 0)
	          VPrintf("%s%s%s", pargs);
               else
                  /* suppress first level indenting for devices or	*/
                  /* blank file names, eg : or ""			*/
                  indent += 5;
	       }
	    if (glob->opts[ALL]
	       || ((depth == 0) && !(anchor->ap_Flags & APF_ITSWILD))) {
	       anchor->ap_Flags |= APF_DODIR;
	       depth = depth+1;
	       if (depth > SEARCH_DEPTH) frc = ERROR_BUFFER_OVERFLOW;
	       if (glob->pdirs) indent = indent-5;

	       else {
		  /* signal need to set up new directory path		*/
		  getpath = 1;
		  }
	       }
	    }
	 }
      /* as long as there's been no error, go around again              */
      if (frc == 0)
         if(MatchNext(anchor)) frc = IoErr();
      }
   if (frc == ERROR_NO_MORE_ENTRIES) frc = 0;
   MatchEnd(anchor);

   if (frc != 0)
      /* trouble somewhere, or maybe ^C - bail out			*/
      break;
   }

   FreeVec(anchor);
   return frc;	/* 0 means no problems in this directory		*/
}

void TFPN(struct DosLibrary *DOSBase, struct search_global *glob, BPTR lock)
/*   ----								*/
/*  put Terminated Full Path Name for lock in glob->fpath		*/
{
char *fnptr = glob->fpath;

NameFromLock(lock, fnptr, FPATHSIZE);
while (*fnptr++);
fnptr-=2;
if (*fnptr != ':')
   *++fnptr = '/';
*++fnptr = '\0';
}


unsigned long StrMovToUpper(char *mptr, char *uptr)
/*	      -------------						*/
{
unsigned long l = 0;
register unsigned char c, la = 'a', lz = 'z', cd = 'a'-'A';

while(c = *mptr)
   {
   if (c >= la)
      if (c <= lz)
         c -= cd;
   *uptr = c;

   uptr++;
   l++;
   mptr++;
   }
*uptr = 0;
return l;
}



int filter (struct DosLibrary *DOSBase, struct search_global *glob)
/*  ------								*/
/* try to match search string against lines read in from redirected	*/
/* standard input as a filter						*/
{
struct Library *SysBase = (*((struct Library **) 4));
int  sres;
sres = 0;		/* initialize return code			*/
glob->lcount = 0;	/* initialize line counter			*/
glob->curfile = OpenAsync(NULL,Input(),MODE_READ, 4096, (struct Library *)DOSBase, SysBase);
glob->noterm = 0;

   if (glob->curfile)
   {
while (getline(DOSBase,glob)) {
   if (MatchPattern(glob->search, glob->temp)) {
      glob->foundmatch = DOSTRUE;
      /* no need to half shift this line				*/
      if (glob->noterm)
         glob->noterm = -1;
      if (glob->opts[NONUM]==0) VPrintf("%6ld ", &glob->lcount);
      PutStr(glob->line);
      }
   if (sres=CheckSignal(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D))
      break;	  /* close this file, and maybe quit			*/
   }
   }

   CloseAsync(glob->curfile);

if (sres & SIGBREAKF_CTRL_C) return ERROR_BREAK;
else return 0;
}



int getline(struct DosLibrary *DOSBase, struct search_global *glob)
/*  -------								*/
/* Read a 'line' from glob->curfile into glob->line and glob->temp.	*/
/* Translate glob->temp to upper case for pattern matching.  Terminate	*/
/* glob->line with '/n' in case we need to print it.  If previous line	*/
/* was not terminated by '\n' or '\0', assume this is a binary file and	*/
/* step half a line at a time to ensure we will catch instances of the	*/
/* search pattern that happen to span a buffer.  Maintain line count in	*/
/* glob->lcount based on number of '\n' encountered.			*/
{
char *bufptr, *bufpt2;
struct AsyncFile *curfile;
long c, ccount;
long skip;

curfile = glob->curfile;

if (glob->noterm >0) { /* means previous 'line' was unterminated	*/
   /* move top half of line buffer to bottom and ignore old '\0'	*/
   memcpy(&glob->line[0], &glob->line[LINESIZE/2], LINESIZE/2);
   memcpy(&glob->temp[0], &glob->temp[LINESIZE/2], LINESIZE/2);

   bufptr = &glob->line[LINESIZE/2];
   bufpt2 = &glob->temp[LINESIZE/2];
   ccount = LINESIZE/2;
   }
else {
   /* previous buffer was terminated - start fresh			*/
   bufptr = glob->line;
   bufpt2 = glob->temp;
   ccount = LINESIZE;
   if (glob->noterm == 0) { /* terminated by '\n', not '\0'?		*/
      *bufpt2 = '\n'; /* put '\n' at start of line for pattern match	*/
      bufpt2++;
      }
   }

   skip = glob->colstart;
   while (skip--)
   {
       c = ReadCharAsync(curfile);
       if (c == -1)
           return 0;

       if (c == '\n')
           glob->lcount++;
   }

c = ReadCharAsync(curfile);
/* if we're at end of file now, return					*/
if (c == -1) return 0;

while ((c > 0) && (c != '\n')) {
   if ((c >= (long)'a') && (c <= (long)'z')) {
      /* store line as upper case for pattern matching			*/
      *bufpt2 = c + (long)('A' - 'a');
      }
   else {
      /* case conversion not required					*/
      *bufpt2 = c;
      }
   bufpt2 += 1;
   /* filter out NUL and control characters with or without high bit	*/
   if (((c + 1) & 0x7f) <= ' ')
      if (c != '\t') {
         /* placeholder for output					*/
         c = '.';
         }
   *bufptr = c;
   bufptr += 1;
   ccount -= 1;
   if (ccount <= 0) break;
   c = ReadCharAsync(curfile);
   }

/* assume unterminated line - otherwise will be changed below		*/
glob->noterm = 1;
/* always return null terminated strings with '\n' as well on line	*/
/* line (for possible printing purposes)				*/
*bufptr = '\n';
bufptr += 1;
*bufptr = '\0';
/* temp	(for matching against search pattern)				*/
if (c == '\n') {
   /* only criterion for incrementing line count			*/
   *bufpt2 = '\n';	/* need to include '\n' for pattern matching	*/
   bufpt2++;
   glob->lcount += 1;
   glob->noterm = 0;	/* indicate line terminated by '\n'		*/
   }
if (c == '\0') {
   /* must treat this as a terminator to find strings in binary files	*/
   glob->noterm = -1;	/* indicate line terminated by '\0'		*/
   }
*bufpt2 = '\0';
/* found something - return TRUE to process whether EOF or not.		*/
return 1;
}



#if 0
void aprintf( fmt, args )
char *fmt, *args;
{
   struct Library *DOSBase;
   struct Library *SysBase = (*((struct Library **) 4));

   if(DOSBase=OpenLibrary(DOSLIB,DOSVER)) {
       VFPrintf(Output(), fmt, (LONG *)&args );
       CloseLibrary(DOSBase);
   }
}
#endif

