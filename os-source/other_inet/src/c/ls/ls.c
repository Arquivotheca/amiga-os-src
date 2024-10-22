/* ---------------------------------------------------------------------
  LS.C -- Source code for the "improved" directory utility.

  Author: Justin V. McCormick

  V1.0    August 1986 Written from scratch, my first Amiga project.
  V2.0  November 1988 Revised for Lattice 5.0 and made 1.3 compatible.
  V2.1  December 1988 Minor size reduction, fixed a few bugs from 2.0.
  V2.2  December 1988 Fixed status return and multiple wildcard pathnames.
  V3.0  July 25, 1989 Instant sorting, best-fit output, new features.
  V3.1  July 29, 1989 Bug fixes, new output width and height options.

Notice:

  This program is placed in the public domain with the
understanding that the author makes no claims or guarantees with
regard to its suitability for any given application, and that
the author assumes no responsibility for damages incurred by its
usage.  Any resemblance of this program to any other program,
either living or dead, is purely coincidental.

  Feel free to borrow this code and make millions of dollars
from its sale or commercial use, but please give credit where
credit is due.

Synopsis:

  Features intelligent columnar listing, versatile sort options,
UNIX-style pattern matching, recursive subdirectory listing, and
more!

Usage:
        LS [options] [pattern1] [pattern2] ...

Bugs and Limitations:

  LS cannot pattern match devices (like "dh*:"), since this
would involve another level of recursion and parsing the Device
List.  LS shows signs of the same "creeping featurism" which
afflicted the UNIX command, evident from its bloated code size.

Changes From 1.0 to 2.0:

 o Source code prototyped, linted, traced, optimized, tweaked, etc.
 o Made resident ("pseudo-pure") by linking with cres.o from LC 5.0.
 o High-volume routines recoded in assembly (lssup.a).
 o Now handles multiple paths/files on a command line, up to 30.
 o New sort flags, including no sort.
 o Enhanced wildcards, understands complex *.?*.* expressions now.
 o More efficient ExNext() performance, less ram used for recursion.
 o SIGBREAKF_CTRL_C signal (Ctrl-C) cleanly aborts at any point now.
 o Command line parser handles quoted pathnames now (LC 5.0 benefit).
 o Short listing finally auto-adjusts to new console window sizes!
 o Pen color escape codes bypassed when redirecting long output.
 o Sorting by size or date is also subsorted alphabetically now.
 o Long listing shows new 1.3 file attributes, plus comment indicator.
 o File dates are now in international format, YY-MM-DD.
 o Fixed listings with files datestamped after 99-12-31 (overflow).
 o Fixed listings with files datestamped before 78-01-01 (time < 0).

Changes From 2.0 to 2.1

 o Fixed the show comment feature, a last minute bug in 2.0.
 o Fixed the "Unknown option ''" message problem.
 o Optimized the assembly branches, reduced code size another few bytes.

Changes From 2.1 to 2.2

 o Fixed erroneous Status returns.
 o Fixed bug with multiple wildcarded pathnames.
 o Compiled with LC 5.0 Patch 3 and CAPE 2.0, saved another 46 bytes.
 o Eliminated an extra stpcpy(), saved another few bytes.

Changes From 2.2 to 3.0

 o Added many new command line options!  See Usage descriptions and docs.
 o "Instant" output sorting, now using an on-the-fly insertion sort.
 o New Short listing technique is row-by-row, redirects to PRT: or file.
 o Better columnization of short listings, uses best-fit optimization.
 o Formatted output parsing system implemented for long listings.
 o Improved ^C handling, now breaks immediately in mid-output.
 o New command line parser, now handles multiple options mixed with names.
 o Added separate pattern matching for directories and files.
 o Now inhibits system requesters from popping up by default.
 o Added custom cres.o module for more size savings.
 o Now uses Exec List functions for smaller, faster code.
 o Fixed random cli_ReturnCode and pr_Result2 value CLI returns.
 o Rewrote many sections, further code cleaning/commenting.
 o Eliminated main(), LS handles the _main() function for smaller code.
 o Compiled with 5.03.90 (beta) Lattice compiler, saved a few bytes.

Changes From 3.0 to 3.1
 o Fixed random errors from uninitialized argument count in GetCLIArgs().
 o New -X and -Y options to specify short format columns and rows.

 --------------------------------------------------------------------- */

/* ---------------------- LS SOURCE ---------------------------------- */

/* Use Real strlen unless you comment out the following */
#define strlen strlen

/* Maximum number of command line arguments */
#define MAXARG 32
#define PADTABSIZE 100
#define WORKSIZE 300

#include "ls.h"
#include <exec/types.h>
#include <action.h>
#include <rpc/types.h>
#include <nfs/nfs.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>

long dos_packet(struct MsgPort *,long,long,long,long);

/* Extern CODE from lssup.a */
extern BYTE *aindex __ARGS((BYTE *, BYTE));
extern LONG  __stdargs asprintf __ARGS((BYTE *, BYTE *,...));
extern LONG CompareDateStamps __ARGS((struct DateStamp *, struct DateStamp *));
extern LONG CompFibs __ARGS((LONG, struct FileInfoBlock *, struct FileInfoBlock *));
extern LONG FillFibEntry __ARGS((struct List *, struct FileInfoBlock *));
extern LONG GetFileString __ARGS((BYTE *, BYTE *));
extern LONG GetPathString __ARGS((BYTE *, BYTE *));
extern LONG iswild __ARGS((BYTE *));
extern LONG wildmatch __ARGS((BYTE *, BYTE *));
extern VOID *myalloc __ARGS((LONG));
extern VOID FibFileDate __ARGS((struct DateStamp *, BYTE *, BYTE *));
extern VOID GetWinBounds __ARGS((LONG *, LONG *));
extern VOID InsertFibNode __ARGS((struct List *, struct FibEntry *));
extern VOID MakePathString __ARGS((struct FileLock *, BYTE *));
extern VOID myfree __ARGS((VOID *));
extern VOID NoFileExit __ARGS((BYTE *));
extern VOID NoMemExit __ARGS((VOID));

/* Local DATA */
struct DateStamp theolddate;     /* Show files older than this date */
struct DateStamp thenewdate;     /* Show files newer than this date */
struct FileHandle *ConOut;       /* Standard output */
struct FileHandle *ConIn;        /* Standard input */
struct FileLock *CurFLock;       /* Global Directory lock */
struct FileInfoBlock *GFibp;     /* Global FIB for Examine/ExNext */
APTR OldWindowPtr;               /* Copy of what was in pr_WindowPtr */

/* User flags, See ls.h for bit definitions */
ULONG LSFlags = SHOWDIRS | SHOWFILES;

LONG gdircount;                 /* Grand total # of dirs for Recursive */
LONG gfilecount;                /* Grand total # of files found */
LONG gtotblocks;                /* Grand total # of blocks */
LONG gtotbytes;                 /* Grand total # of bytes */

LONG dircount;                  /* Number of directories found */
LONG filecount;                 /* Number of files found */
LONG totblocks;                 /* total # of blocks */
LONG totbytes;                  /* total # of bytes */
LONG maxnamlen;                 /* Longest name found */
LONG sortkey;                   /* 0=alpha, 1=size, 2=date */

BYTE padtab[PADTABSIZE];        /* Column table tab amounts */
BYTE thePath[WORKSIZE];         /* Current filename space */
BYTE theDirPat[WORKSIZE];       /* Dirname pattern workspace */
BYTE theFilePat[WORKSIZE];      /* Filename pattern workspace */
BYTE workstr[WORKSIZE+64];      /* Temp string space */
BYTE UidStr[7];
BYTE GidStr[7];

BYTE theblksstr[20];
BYTE thedatestr[12];
BYTE theprotstr[12];
BYTE thesizestr[20];
BYTE thetimestr[12];

LONG CurWinRows;                /* Window bounds, determined at runtime */
LONG CurWinCols;

/* Initialized Strings */
BYTE Author[]  = "\23333mLS 3.1N\2330m Based On PD Freeware by Justin V. McCormick";
BYTE usage[]   = "\n  Usage: LS [-options <args>] [path1] [path2] ...\n";
BYTE usage0[]  = "\tc  Show filenotes           D  Show dirs last\n";
BYTE usage1[]  = "\td  Dirs only                H  No headings and subtotals\n";
BYTE usage2[]  = "\tf  Files only               I  No page prompts\n";
BYTE usage3[]  = "\tk  No ANSI display codes    M  Mixed output files and dirs\n";
BYTE usage4[]  = "\tl  Long listing             N <name> Show newer than entry\n";
BYTE usage5[]  = "\tn  No sort                  O <name> Show older than entry\n";
BYTE usage6[]  = "\tp  Permit system requests   P  Show full pathnames\n";
BYTE usage7[]  = "\tr  Reverse sort             R  Recursive listing\n";
BYTE usage8[]  = "\ts  Sort by size             T  Total all subtotals\n";
BYTE usage9[]  = "\tt  Sort by date             X <wide> Set output columns\n";
BYTE usage10[] = "\tv <pattern> Avoid pattern   Y <high> Set output rows\n";
BYTE usage11[] = "\tF <format> Format output (default: \042";
BYTE usage12[] = "\042)\n";

/* Date and output format strings */
BYTE baddatestr[]     = "00-00-00";
BYTE badtimestr[]     = "00:00:00";
BYTE datepat[]        = "%02ld-%02ld-%02ld";
BYTE timepat[]        = "%02ld:%02ld:%02ld";
BYTE dayspermonth[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
BYTE deffmtstr[40]    = "%p %u %g %d %t %4b %8s %n\\n";
BYTE LongFmtStr[]     = "%ld";
BYTE totalfmtstr[]    = "Dirs:%-4ld Files:%-4ld Blocks:%-5ld Bytes:%-8ld\n";
BYTE TotHeaderMsg[]   = "Totals:\n";
BYTE ColonStr[]       = ":";
BYTE SlashStr[]       = "/";
BYTE RamNameStr[]     = "RAM:";

/* ANSI color control strings */
BYTE penstr0[]        = "\2330m";       /* Reset CON:   */
BYTE penstr1[]        = "\2330 p";      /* Cursor off   */
BYTE penstr2[]        = "\233 p";       /* Cursor on    */
BYTE penstr3[]        = "\23333m";      /* Color 3 NORM */
BYTE penstr4[]        = "\2331;33;40m"; /* Color 3 BOLD */
BYTE penstr5[]        = "\2331;31;40m"; /* Color 1 BOLD */

/* CON: command sequence for window bounds report */
BYTE gwbrstr[4] = {  0x9b, '0', ' ', 'q' };

/* Newline and "" source */
BYTE NLine[4] = { 10, 0, 0, 0 };

/* Error messages */
BYTE BadOptFmtStr[]   = "Unknown option: -%s\n";
BYTE ErrFmtStr[]      = "LS: %s, Error #%ld\n";
BYTE NoExamFmtStr[]   = "Cannot examine file or directory, Error #%ld\n";
BYTE NoFindFmtStr[]   = "'%s' not found";
BYTE NoRAMMsg[]       = "No RAM";
BYTE NoWildPathMsg[]  = "Unable to pattern match paths";
BYTE VolEmptyMsg[]    = "Volume or directory is empty.\n";
BYTE NoMatchMsg[]     = "No match.\n";

/* Pointers to usage strings for quick easy output */
BYTE *usagestrs[] =
{
  Author, usage, usage0, usage1, usage2, usage3, usage4, usage5,
  usage6, usage7, usage8, usage9, usage10, usage11, deffmtstr, usage12, 0L
};

BYTE *thefmtstr = deffmtstr;    /* Format string pointer for long output */
BYTE *curpath;
BYTE *theAntiPat;		/* Avoid pattern string */

/* Local CODE */
BYTE *GetDecNum __ARGS((BYTE *, LONG *));
LONG CmpDateBounds __ARGS((struct DateStamp *));
LONG GetFileDate __ARGS((BYTE *, struct DateStamp *));
LONG ParseCmdOptions __ARGS((LONG, LONG, BYTE **));
struct FibEntry *AllocFib __ARGS((VOID));
struct FibEntry *ModNextFib __ARGS((struct FibEntry *, LONG));
struct List *GetDir __ARGS((struct FileLock *, struct FileInfoBlock *));
VOID CleanUp __ARGS((BYTE *, LONG, LONG));
VOID DirIt __ARGS((struct FileLock *, BYTE *));
VOID FreeAllFibs __ARGS((struct List *));
VOID GetCLIArgs __ARGS((BYTE *, LONG *, BYTE **));
VOID LListDir __ARGS((struct List *));
VOID LListEntry __ARGS((struct FileInfoBlock *));
VOID LongList __ARGS((LONG *, LONG *, struct List *));
VOID PagePrompt __ARGS((LONG, LONG));
VOID ParseFormatOpts __ARGS((struct FileInfoBlock *));
VOID SetConPen __ARGS((BYTE *));
VOID SListDir __ARGS((struct List *));
VOID TestBreak __ARGS((VOID));
VOID Usage __ARGS((VOID));
VOID WCHR __ARGS((BYTE *));
VOID WSTR __ARGS((BYTE *));
VOID _main __ARGS((BYTE *));


/* --------------------------------------------------------------------
 * Allocate a FibEntry structure and associated FileInfoBlock.
 * -------------------------------------------------------------------- */
struct FibEntry *AllocFib ()
{
  struct FibEntry *tfibp;

  if ((tfibp = myalloc ((LONG)(sizeof (struct FibEntry) + sizeof (struct FileInfoBlock)))) != 0)
  {
    tfibp->fe_Fib = (struct FileInfoBlock *)((ULONG)tfibp + sizeof (struct FibEntry));
  }
  else
    LSFlags |= BREAKFLAG;
  return (tfibp);
}

/* --------------------------------------------------------------------
 * Use AmigaDOS to output a string to the stdout channel
 * -------------------------------------------------------------------- */
VOID WSTR (tstring)
  BYTE *tstring;
{
  LONG i;

  i = strlen (tstring);
  if (i > 0)
  {
    (VOID) Write ((BPTR)ConOut, tstring, i);
  }
}

/* --------------------------------------------------------------------
 * Use AmigaDOS to put a character to the stdout
 * -------------------------------------------------------------------- */
VOID WCHR (ch)
  BYTE *ch;
{
  (VOID) Write ((BPTR)ConOut, ch, 1L);
}

/* --------------------------------------------------------------------
 * Check to see if the user hit ^C
 * -------------------------------------------------------------------- */
VOID TestBreak ()
{
  ULONG oldsig;

  oldsig = SetSignal (0L, (LONG)SIGBREAKF_CTRL_C);
  if ((oldsig & SIGBREAKF_CTRL_C) != 0)
  {
    WSTR ("\2330m\233 p**BREAK\n");
    LSFlags |= BREAKFLAG;
  }
}

/* --------------------------------------------------------------------
 * Prompt the user to hit return, wait till return is hit
 * -------------------------------------------------------------------- */
VOID PagePrompt (page, maxpage)
  LONG page, maxpage;
{
  if (CurWinCols > maxnamlen && page > 1 && (LSFlags & NOINTERACT) == 0)
  {
    WSTR ("\2337m -- MORE -- ");
    if (CurWinCols >= 27)
      WSTR ("Press Return: ");
    WSTR ("\2330m\233 p");
    (VOID) Read ((BPTR)ConIn, workstr, 1L);
    WSTR ("\2330 p\233F\233K");
    TestBreak ();
  }

  if ((LSFlags & (NOHEADERS|BREAKFLAG)) == 0)
  {
    if ((LSFlags & CONSOLE) != 0)
    {
      WSTR ("\2330 p\2334;33m");
    }
    (VOID) asprintf (workstr, "Page %ld of %ld:", page, maxpage);
    WSTR (workstr);
    SetConPen (penstr0);
    WSTR (NLine);
  }
}

/* -------------------------------------------------------------------- */
struct FibEntry *ModNextFib(tfibp, rows)
  struct FibEntry *tfibp;
  LONG rows;
{
  LONG i;

  for (i = 0; i < rows && tfibp->fe_Node.mln_Succ != 0; i++)
  {
    tfibp = (struct FibEntry *)tfibp->fe_Node.mln_Succ;
  }
  return(tfibp);
}

/* --------------------------------------------------------------------
 * set CON: character color to default Pen1 colors 
 * -------------------------------------------------------------------- */
VOID SetConPen (penstr)
  BYTE *penstr;
{
  if ((LSFlags & CONSOLE) != 0)
    WSTR (penstr);
}

/* #define DEBUGSLD 1 */
/* --------------------------------------------------------------------
 * List a FibEntry list in a compact fashion
 * -------------------------------------------------------------------- */
VOID SListDir (fibheadp)
  struct List *fibheadp;
{
  LONG avglen;
  LONG colcnt;
  LONG currow;
  LONG dfcount;
  LONG i, j, wlen;
  LONG maxcol;
  LONG maxpage;
  LONG maxrow;
  LONG maxwinrow;
  LONG pagecnt;
  LONG rowcnt;
  LONG tlen;
  LONG totlen;
  struct FibEntry *hfibp, *tfibp;

  SetConPen (penstr1);        /* Turn the cursor off since it will blink anyway */
  GetWinBounds (&colcnt, &currow); /* Get current window size */

  if (CurWinCols == 0)
    CurWinCols = colcnt;
  if (CurWinRows == 0)
    CurWinRows = currow;

/* Make a average-case WxH estimate for # of display columns */
  for (totlen = dfcount = 0, hfibp = (struct FibEntry *)fibheadp->lh_Head;
       hfibp->fe_Node.mln_Succ != 0;
       hfibp = (struct FibEntry *)hfibp->fe_Node.mln_Succ)
  {
    if (hfibp->fe_Fib->fib_DirEntryType > 0 && (LSFlags & CONSOLE) == 0)
      (VOID) strcat (&hfibp->fe_Fib->fib_FileName[1], SlashStr);
    totlen += hfibp->fe_Fib->fib_FileName[0];
    dfcount++;
  }

/* Calc average length of all entries */
  avglen = totlen / dfcount;            /* Avg name length = totlen/numentries */
  if ((totlen % dfcount) != 0)
    avglen++;

  if ((CurWinCols) <= maxnamlen)        /* Longest name wider than window? */
    maxcol = 1;                         /* Yep, just print one column */
  else
  {
  /* Else maxcols = winwidth/namewidth */
    for (maxcol = 0, colcnt = CurWinCols; colcnt >= avglen; maxcol++)
    {
      colcnt -= avglen + 2;
    }
  }
#ifdef DEBUGSLD
  asprintf(workstr, "avg:%ld max:%ld\n", avglen, maxnamlen); WSTR(workstr);
#endif

/* Dry run output avg-case WxH table to see if it needs adjusting */
  for (;;)
  {
  /* Clear out previous padtab */
    memset(padtab, 0, PADTABSIZE);

  /* Number of rows = total entries / entries per row */
    maxrow = dfcount / maxcol;
    if ((dfcount % maxcol) != 0)        /* Round up if non-integral */
      maxrow++;
#ifdef DEBUGSLD
    asprintf(workstr, "avg: %ld rows by %ld cols\n", maxrow, maxcol); WSTR(workstr);
#endif

    for (rowcnt = 0, hfibp = (struct FibEntry *)fibheadp->lh_Head;
         rowcnt < maxrow && hfibp->fe_Node.mln_Succ != 0;
         rowcnt++, hfibp = (struct FibEntry *)hfibp->fe_Node.mln_Succ)
    {
      for (colcnt = 0, tfibp = hfibp;
           colcnt < maxcol && tfibp->fe_Node.mln_Succ != 0;
           colcnt++, tfibp = ModNextFib (tfibp, maxrow))
      {
        tlen = tfibp->fe_Fib->fib_FileName[0];
        if (tlen > padtab[colcnt])
          padtab[colcnt] = tlen;
      }

    /* If this is the first row, calc actual maxcol/maxrow for this dfcount */
      if (rowcnt == 0)
      {
        maxcol = colcnt;
        maxrow = dfcount / maxcol;
        if ((dfcount % maxcol) != 0)    /* Round up if non-integral */
          maxrow++;
      }
    }

  /* Calculate actual total width by adding up width of all columns */
    for (colcnt = totlen = 0; (colcnt + 1) < maxcol; colcnt++)
    {
      totlen += (LONG)padtab[colcnt] + 2;
#ifdef DEBUGSLD
      asprintf(workstr, "padtab[%ld]=%ld\n", colcnt, (LONG)padtab[colcnt]); WSTR(workstr);
#endif
    }
    totlen += (LONG)padtab[colcnt];
#ifdef DEBUGSLD
    asprintf(workstr, "padtab[%ld]=%ld\n", colcnt, (LONG)padtab[colcnt]); WSTR(workstr);
    asprintf(workstr, "totlen %ld\n", totlen); WSTR(workstr);
#endif

  /* if More than one column and 
   * total width of all columns is greater > our window width,
   * then decrease number of display columns
   */
    if (maxcol > 1 && totlen > CurWinCols)
    {
      maxcol--;
#ifdef DEBUGSLD
      asprintf(workstr, "new maxcol:%ld\n", maxcol); WSTR(workstr);
#endif
    }
    else
      break;
  }
#ifdef DEBUGSLD
  asprintf(workstr, "adjusted: maxrow:%ld maxcol:%ld (winwidth:%ld totlen:%ld)\n", maxrow, maxcol, CurWinCols, totlen); WSTR(workstr);
#endif

/* Calc number of pages */
  maxwinrow = CurWinRows - 3;
  if (maxwinrow <= 0)
    maxwinrow = 1;
  pagecnt = 1;
  maxpage = maxrow / maxwinrow;
  if ((maxrow % maxwinrow) != 0)
    maxpage++;

/* Do actual output scan */
  for (rowcnt = 0, currow = maxwinrow, hfibp = (struct FibEntry *)fibheadp->lh_Head;
       (LSFlags & BREAKFLAG) == 0 && rowcnt < maxrow && hfibp->fe_Node.mln_Succ != 0;
       TestBreak(), rowcnt++, currow++, hfibp = (struct FibEntry *)hfibp->fe_Node.mln_Succ)
  {
    if (maxpage > 1 && currow == maxwinrow)
    {
      currow = 0;
      if ((LSFlags & CONSOLE) != 0)
      {
        PagePrompt (pagecnt, maxpage);
        if ((LSFlags & BREAKFLAG) != 0)
          break;
      }
      pagecnt++;
    }

    for (colcnt = 0, tfibp = hfibp;
         colcnt < maxcol && tfibp->fe_Node.mln_Succ != 0;
         colcnt++)
    {
      if (tfibp->fe_Fib->fib_DirEntryType < 0)
      {
      /* Print a file entry */
        (VOID) stpcpy (workstr, &tfibp->fe_Fib->fib_FileName[1]);
        wlen = tfibp->fe_Fib->fib_FileName[0];
      }
      else
      {
      /* Print a directory entry */
        workstr[0] = 0;
        wlen = tfibp->fe_Fib->fib_FileName[0];
        if ((LSFlags & CONSOLE) != 0)
          (VOID) strcat (workstr, penstr3);
        (VOID) strcat (workstr, &tfibp->fe_Fib->fib_FileName[1]);
        if ((LSFlags & CONSOLE) != 0)
          (VOID) strcat (workstr, penstr0);
      }

    /* Move along list to the next entry, mod maxcol, print this entry */
      tfibp = ModNextFib (tfibp, maxrow);

    /* If this is not the last column, pad with spaces till we get to next column */
      if ((colcnt + 1) < maxcol && tfibp->fe_Node.mln_Succ != 0)
      {
        for (i = (LONG)padtab[colcnt] + 1, j = strlen (workstr); i >= wlen; i--, j++)
          workstr[j] = ' ';
        workstr[j] = 0;
      }
    /* Output the final entry */
      WSTR (workstr);
    }

  /* Filled this row, start next down */
    WCHR (NLine);
  }

  SetConPen (penstr2);        /* Turn cursor back on */
}

/* -------------------------------------------------------------------- */
BYTE *GetDecNum (cp, spccnt)
  BYTE *cp;
  LONG *spccnt;
{
  for (*spccnt = 0; *cp >= '0' && *cp <= '9'; cp++)
  {
    *spccnt = *spccnt * 10 + (LONG)*cp - '0';
  }
  return (cp);
}

/* -------------------------------------------------------------------- */
VOID ParseFormatOpts (fib)
  struct FileInfoBlock *fib;
{
  BYTE *cp1, *cp2, *reps;
  BYTE *pathend, *thenamestr;
  LONG i, spccnt;

  i = strlen (curpath);
  pathend = curpath + i;
  if (i > 1 && *(curpath + i - 1) != ':')
  {
    *(curpath + i) = '/';
    i++;
    *(curpath + i) = 0;
  }
  thenamestr = curpath + strlen(curpath);
  cp2 = thenamestr;
  if (fib->fib_DirEntryType > 0 && (LSFlags & CONSOLE) != 0)
    cp2 = stpcpy (cp2, penstr3);
  cp2 = stpcpy (cp2, &fib->fib_FileName[1]);
  if (fib->fib_DirEntryType > 0 && (LSFlags & CONSOLE) != 0)
    (VOID) stpcpy (cp2, penstr0);

  for (cp1 = workstr, cp2 = thefmtstr; *cp2 != 0; cp2++)
  {
    if (*cp2 != '%' && *cp2 != '\\')
      *cp1++ = *cp2;
    else
    {
      if (*cp2 == '%')
      {
        cp2++;
        cp2 = GetDecNum (cp2, &spccnt);
        if (spccnt > 99)
        {
          spccnt = 99;
        }
          
        switch (*cp2)
        {
          case 'u':
            reps = UidStr;
            break;
          case 'g':
            reps = GidStr;
            break;
          case 'p':
            reps = theprotstr;
            break;
          case 'd':
            reps = thedatestr;
            break;
          case 't':
            reps = thetimestr;
            break;
          case 'b':
            reps = theblksstr;
            break;
          case 's':
            reps = thesizestr;
            break;
          case 'n':
            if (fib->fib_DirEntryType > 0 && (LSFlags & CONSOLE) != 0 && spccnt > 0)
              spccnt += 7;
            if ((LSFlags & FULLPATHNAMES) != 0)
              reps = curpath;
            else
              reps = thenamestr;
            break;
          case '%':
            *cp1++ = '%';
            *cp1++ = 0;
          default:
            reps = &NLine[1];
            break;
        }
        for (i = strlen(reps); i < spccnt; i++)
          *cp1++ = ' ';
        cp1 = stpcpy (cp1, reps);
      }
      else
      {
        cp2++;
        switch (*cp2)
        {
          case 'n':
            *cp1++ = '\n';
            break;
          case 't':
            *cp1++ = '\t';
            break;
          case '\\':
            *cp1++ = '\\';
            break;
          default:
            break;
        }
        *cp1 = 0;
      }
    }
  }
  WSTR (workstr);

  if ((LSFlags & NOTEFLAG) != 0 && fib->fib_Comment[0] != 0)
  {
    SetConPen(penstr3);
    fib->fib_Comment[(int)(*fib->fib_Comment+1)]='\0';
    (VOID)asprintf(workstr, "/* %s */\n", &fib->fib_Comment[1]);
    WSTR (workstr);
    SetConPen(penstr0);
  }
  *pathend = 0;
}

/* --------------------------------------------------------------------
 * Verbosely list a particular FibEntry
 * -------------------------------------------------------------------- */
VOID LListEntry (fib)
  struct FileInfoBlock *fib;
{

  LONG i, p;
  struct MyInfoBlock *mfib;
  char *cp;

  mfib = (struct MyInfoBlock *)fib;


  FibFileDate (&fib->fib_Date, thedatestr, thetimestr);

  if (mfib->is_remote) { 
    cp = theprotstr;
    p = mfib->mode;
    switch (p & NFSMASK){
	case NFSMODE_LNK:
		*cp++ = 'l';	
		break;
	case NFSMODE_DIR:
		*cp++ = 'd';	
		break;	
	case NFSMODE_CHR:
		*cp++ = 'c';	
		break;
	case NFSMODE_BLK:
		*cp++ = 'b';	
		break;
	case NFSMODE_SOCK:
		*cp++ = 's';	
		break;
	default:
		*cp++ = '-';	
	}

	*cp++ = (p & NFS_OWN_RDPERM) ? 'r' : '-';
	*cp++ = (p & NFS_OWN_WRPERM) ? 'w' : '-';

	*cp = (p & NFS_OWN_EXPERM) ? 'x' : '-';
	if (p & NFS_SETUID){
		*cp = 's';	
	}

	cp++;
	*cp++ = (p & NFS_GID_RDPERM) ? 'r' : '-';
	*cp++ = (p & NFS_GID_WRPERM) ? 'w' : '-';

	*cp = (p & NFS_GID_EXPERM) ? 'x' : '-';
	if (p & NFS_SETGID){
		*cp = 's';
	}

	cp++;
	*cp++ = (p & NFS_PUB_RDPERM) ? 'r' : '-';
	*cp++ = (p & NFS_PUB_WRPERM) ? 'w' : '-';
	*cp++ = (p & NFS_PUB_EXPERM) ? 'x' : '-';
	*cp = '\0';

	(VOID)asprintf(UidStr,"%-5ld ",mfib->uid);
	(VOID)asprintf(GidStr,"%-5ld ",mfib->gid);
  } else {
  	p = fib->fib_Protection & 0xff;
  	(VOID)stpcpy (theprotstr, "chsparwed");
    if (fib->fib_Comment[0] == 0)
        theprotstr[0] = '-';

  	for (i = 3; i >=  0; i--) {
    	if ((p & (1 << i)) != 0)
      		theprotstr[8 - i] = '-';
    	if ((p & (1 << (i + 4))) == 0)
      		theprotstr[4 - i] = '-';
  	}

  }
  if (fib->fib_DirEntryType > 0) {
   	(VOID)stpcpy (theblksstr, "0");
   	(VOID)stpcpy (thesizestr, "Dir");
  } else {
   	(VOID) asprintf (theblksstr, LongFmtStr, fib->fib_NumBlocks);
   	(VOID) asprintf (thesizestr, LongFmtStr, fib->fib_Size);
  }
  	
  ParseFormatOpts (fib);
}

/* --------------------------------------------------------------------
 * List a directory in a verbose informative manner
 * -------------------------------------------------------------------- */
VOID LListDir (fibheadp)
  struct List *fibheadp;
{
  struct FibEntry *tfibp;

  SetConPen(penstr1);

  totblocks = totbytes = 0;
  for (tfibp = (struct FibEntry *)fibheadp->lh_Head;
       tfibp->fe_Node.mln_Succ != 0;
       tfibp = (struct FibEntry *)tfibp->fe_Node.mln_Succ)
  {
    TestBreak();
    if ((LSFlags & BREAKFLAG) != 0)
      return;
    LListEntry (tfibp->fe_Fib);
    if (tfibp->fe_Fib->fib_DirEntryType < 0)
    {
      totblocks += tfibp->fe_Fib->fib_NumBlocks;
      totbytes += tfibp->fe_Fib->fib_Size;
    }
  }

  if ((LSFlags & (BREAKFLAG | NOHEADERS)) == 0)
  {
    (VOID) asprintf (workstr, totalfmtstr, dircount, filecount, totblocks, totbytes);
    WSTR (workstr);
  }

  SetConPen(penstr2);
}

/* -------------------------------------------------------------------- */
LONG CmpDateBounds (tdate)
  struct DateStamp *tdate;
{
  if ((LSFlags & SHOWNEWERTHAN) != 0)
  {
    if (CompareDateStamps(&thenewdate, tdate) >= 0)
      return (0L);
  }
  if ((LSFlags & SHOWOLDERTHAN) != 0)
  {
    if (CompareDateStamps(tdate, &theolddate) >= 0)
      return (0L);
  }
  return (1L);
}

/* --------------------------------------------------------------------
 * Free up memory allocated to a linked list of FibEntrys
 * -------------------------------------------------------------------- */
VOID FreeAllFibs (fibheadp)
  struct List *fibheadp;
{
  struct FibEntry *tfibp;

  if (fibheadp != 0)
  {
    while (fibheadp->lh_Head->ln_Succ != 0)
    {
      tfibp = (struct FibEntry *)RemTail(fibheadp);
      myfree (tfibp);
    }

  /* Now free the MinList itself */
    myfree (fibheadp);
  }
}

/* --------------------------------------------------------------------
 * Allocate and fill a linked list of FileInfoBlocks
 * -------------------------------------------------------------------- */
struct List *GetDir (lockp, fibp)
  struct FileLock *lockp;
  struct FileInfoBlock *fibp;
{
  BYTE *thepat;                /* Pattern pointer to dir or file pattern */
  LONG matchstat;              /* Result of wildmatch() */
  LONG nextstat;               /* Status of ExNext() */
  LONG tempnamlen;             /* Compare length for strings */
  LONG dfcount;
  struct List *fibhead;        /* Temp list of Fibs created */
  struct List *dirhead;        /* Temp list of dirs if SHOWDIRS == 0 */

  maxnamlen = dfcount = dircount = filecount = 0L;

/* Initialize an exec list of nodes, zero entries */
  if ((fibhead = myalloc ((LONG)sizeof(struct MinList))) == 0)
    return (0L);
  NewList (fibhead);

/* Allocate an separate list for directories that don't match specs */
  if ((dirhead = myalloc((LONG)sizeof(struct MinList))) == 0)
    goto BADALLOC;
  NewList (dirhead);

  do
  {
    TestBreak ();
    if ((LSFlags & BREAKFLAG) != 0)
      goto GOODRET;

  /* If we got something */
    if ((nextstat = GetFib ((BPTR)lockp, fibp, ACTION_EX_NEXT)) !=0)
    {
      dfcount++;

    /* If the entry is wanted bump count of files or directories */
      if (CmpDateBounds (&fibp->fib_Date) != 0)
      {
        if (fibp->fib_DirEntryType > 0) /* It's a directory */
          thepat = theDirPat;
        else
          thepat = theFilePat;

      /* See if this is the longest filename for later use in listing */
        tempnamlen=fibp->fib_FileName[0];
        if (tempnamlen > maxnamlen)
          maxnamlen = tempnamlen;

      /* NULL terminate BSTR */
        fibp->fib_FileName[tempnamlen+1]='\0';


        matchstat = wildmatch (&fibp->fib_FileName[1], thepat);
        if ((LSFlags & ANTIMATCH) != 0 && matchstat != 0)
          matchstat = wildmatch (&fibp->fib_FileName[1], theAntiPat) ^ 1;

	if (matchstat == 0)	/* No match? Then move on */
	  continue;

        if (fibp->fib_DirEntryType > 0) /* It's a directory */
        {
          if (FillFibEntry (dirhead, fibp) == 0)
            goto BADALLOC;
          if ((LSFlags & SHOWDIRS) != 0)
          {
            dircount++;
            gdircount++;
          }
          else
            continue;
        }
        else                            /* It's a file entry */
        {
          if ((LSFlags & SHOWFILES) != 0)
          {
            filecount++;
            gfilecount++;
            gtotblocks += fibp->fib_NumBlocks;
            gtotbytes += fibp->fib_Size;
          }
          else                /* Don't want this file, move on to next entry */
            continue;
        }

      /* Allocate another FibEntry, put the info inside */
        if (FillFibEntry (fibhead, fibp) == 0)
          goto BADALLOC;
      }
    }
  } while (nextstat != 0);

/* No entries found? print message and return FALSE */
  if ((dircount + filecount) == 0)
  {
    if ((LSFlags & NOHEADERS) == 0)
    {
      if (dfcount == 0)
        WSTR (VolEmptyMsg);
      else
        WSTR (NoMatchMsg);
    }
  }
  else
  {
    if ((LSFlags & (SHOWDIRS|SHOWFILES)) != 0)
    {
      if ((LSFlags & LONGLIST) == 0)  /* Short listing wanted */
        SListDir (fibhead);
      else                            /* Full listing */
        LListDir (fibhead);
    }
  }

GOODRET:
  FreeAllFibs (fibhead);
  return (dirhead);

BADALLOC:
  FreeAllFibs (fibhead);
  FreeAllFibs (dirhead);
  return (0L);
}

/* --------------------------------------------------------------------
 * Given a directory name and a lock on that directory, create a list
 * of entries.  Recursively decends into subdirectories if flaged.
 * -------------------------------------------------------------------- */
VOID DirIt (curlock, dirname)
  struct FileLock *curlock;
  BYTE *dirname;
{
  BYTE *subdir;
  LONG dnamlen;
  struct FibEntry *tfibp;
  struct FileLock *sublock;
  struct List *fibheadp;

/* Try to fill FileInfoBlock, bomb if not readable for some reason */

  if (GetFib ((BPTR)curlock, GFibp,ACTION_EX_OBJECT) == 0)
  {
    (VOID) asprintf (workstr, NoExamFmtStr, IoErr());
    WSTR (workstr);
    return;
  }

/* Put directory header for each listing, if we know the name && recursive */
  if (dirname[0] != 0 && (LSFlags & LISTALL) != 0 && (LSFlags & NOHEADERS) == 0)
  {
    if ((LSFlags & CONSOLE) != 0)
     (VOID)asprintf (workstr, "\23330;41m %s \2330m\n", dirname);
    else
     (VOID)asprintf (workstr, "%s\n", dirname);
    WSTR (workstr);
  }

/* If this is a single file list it verbosely */
  if (GFibp->fib_DirEntryType < 0)
  {
    if ((LSFlags & SHOWFILES) != 0)
    {
      (VOID) GetPathString (thePath, thePath);
      LListEntry (GFibp);
    }
  }
  else /* It is a directory entry */
  {
  /* Allocate, fill, and display a dir of entries, check for no ram or abort */
    if ((fibheadp = GetDir (curlock, GFibp)) != 0)
    {
    /* Recursively descend any subdirs in this list, if wanted */
      if ((LSFlags & LISTALL) != 0)
      {
        for (tfibp = (struct FibEntry *)fibheadp->lh_Head;
              tfibp->fe_Node.mln_Succ != 0;
              tfibp = (struct FibEntry *)tfibp->fe_Node.mln_Succ)
        {
          TestBreak ();
          if ((LSFlags & BREAKFLAG) != 0)
            break;

          if (tfibp->fe_Fib->fib_DirEntryType > 0)
          {
          /* Alloc length of path + 1 + newdir name +1 + length of maxname + 1 for NULL
           * + 1 for null + 3 for rounding.
           */
            dnamlen = (LONG)(strlen (dirname) + tfibp->fe_Fib->fib_FileName[0] + 36);
            if ((subdir = myalloc ((LONG)dnamlen)) != 0)
            {
              if (dirname[0] != 0)
              {
                (VOID) stpcpy (subdir, dirname);
                dnamlen = strlen (dirname) - 1;
                if (dirname[dnamlen] != ':' && dirname[dnamlen] != '/')
                {
                  (VOID) strcat (subdir, SlashStr);
                }
              }
              (VOID) strcat (subdir, &tfibp->fe_Fib->fib_FileName[1]);

              if ((sublock = (struct FileLock *)Lock (subdir, (LONG)ACCESS_READ)) != 0)
              {
                if ((LSFlags & NOHEADERS) == 0)
                  WCHR (NLine);                 /* Put a blank line between directories */
		
		curpath = subdir;

                DirIt (sublock, subdir); /* Recurse into this subdirectory */

                UnLock ((BPTR)sublock);	 /* Unlock our sublock */
              }
              myfree (subdir);      	 /* Free the current namespace */
            }
          }
        }
      }
    /* Free up this fib list */
      FreeAllFibs (fibheadp);
    }
  }
}

/* -------------------------------------------------------------------- */
VOID GetCLIArgs (line, argc, argv)
  BYTE *line;
  LONG *argc;
  BYTE **argv;
{
  BYTE **pargv, *qarg;

  *argc = 0;
  while (*argc < MAXARG)
  {
    while (*line == ' ' || *line == '\t' || *line == '\n')
      line++;
    if (*line == 0)
      break;
    pargv = &argv[*argc];
    *argc += 1;

    if (*line == '"')
    {
      qarg = line;
      line += 1;
      *pargv = line;                 /* ptr inside quoted string */
      while (*line != 0 && *line != '"')
        line++;
      if (*line == 0)                /* Hit end of string without quote! */
      {
        *pargv = qarg;               /* Must be okay */
        break;
      }
      else
        *line++ = 0;                 /* terminate arg ontopof quote */
    }
    else                             /* non-quoted arg */
    {
      *pargv = line;
      while (*line != 0 && !(*line == ' ' || *line == '\t' || *line == '\n'))
        line++;
      if (*line == 0)
        break;
      else
        *line++ = 0;                 /* terminate arg */
    }
  }                                  /* while */
}

/* -------------------------------------------------------------------- */
LONG GetFileDate(name, ptime)
  char *name;
  struct DateStamp *ptime;
{
  LONG status;
  struct FileLock *flock;
  struct FileInfoBlock *fib;

  status = 0;
  if ((fib = myalloc ((LONG)sizeof(struct FileInfoBlock))) != 0)
  {
    if ((flock = (struct FileLock *)Lock (name, (LONG)ACCESS_READ)) != 0)
    {
      if (Examine ((BPTR)flock, fib) != 0)
      {
        *ptime = fib->fib_Date; 	/* Copy the Date structure */
        status = 1;
      }
      UnLock ((BPTR)flock);
    }
    myfree (fib);
  }
  return (status);
}

/* --------------------------------------------------------------------
 * Deallocate and close everything
 * -------------------------------------------------------------------- */
VOID CleanUp (exit_msg, exit_status, result2)
  BYTE *exit_msg;
  LONG exit_status, result2;
{
  BYTE workstr[WORKSIZE];
  struct Process *procp;

/* Make sure we unlock any locks we created! */
  if (CurFLock != 0 && (LSFlags & PATHNAMED) != 0)
    UnLock ((BPTR)CurFLock);

/* Free our fib */
  myfree (GFibp);

  if (*exit_msg != 0)
  {
    (VOID) asprintf (workstr, ErrFmtStr, exit_msg, result2);
    WSTR (workstr);
  }

/* Put windowptr back */
  procp = (struct Process *) FindTask (0L);
  procp->pr_WindowPtr = OldWindowPtr;

/* Set return status, exit */
  procp->pr_Result2 = result2;
  exit ((int)exit_status);
}

/* --------------------------------------------------------------------
 * Explain how to use.
 * -------------------------------------------------------------------- */
VOID Usage ()
{
  LONG i;

  for (i = 0; usagestrs[i] != 0; i++)
    WSTR (usagestrs[i]);
  CleanUp (&NLine[1], 20L, 120L);
}

/* -------------------------------------------------------------------- */
LONG ParseCmdOptions (ncnt, argc, argv)
  LONG ncnt, argc;
  BYTE **argv;
{
  BYTE tmpmsg[4];
  LONG i, cnt, len;
  struct Process *procp;

  cnt = ncnt;      /* Current arg number that contains the options */
  ncnt += 1;       /* Advance to next arg */

  for (i = 1, len = strlen (argv[cnt]); i < len; i++)
  {
    switch (argv[cnt][i])
    {
      case '?':
      case 'h':
        Usage ();
        break;
      case 'D':
        LSFlags |= FILESFIRST;
        break;
      case 'F':
        if (argc < (ncnt + 1))     /* Missing required format string */
          Usage ();
        thefmtstr = argv[ncnt];    /* Else our format string is the next arg */
        ncnt += 1;                 /* Bump arg counter */
        LSFlags |= LONGLIST;
        break;
      case 'H':
        LSFlags |= NOHEADERS;
        break;
      case 'I':
        LSFlags |= NOINTERACT;
        break;
      case 'M':
        LSFlags |= MIXFILESDIRS;
        break;
      case 'N':
        if (argc < (ncnt + 1))     /* Missing required name string */
          Usage ();
        else
        {
          if (GetFileDate (argv[ncnt], &thenewdate) == 0)
            NoFileExit (argv[ncnt]);
          ncnt += 1;                 /* Bump arg counter */
          LSFlags |= SHOWNEWERTHAN;
        }
        break;
      case 'O':
        if (argc < (ncnt + 1))     /* Missing required name string */
        {
          Usage ();
        }
        else
        {
          if (GetFileDate (argv[ncnt], &theolddate) == 0)
            NoFileExit (argv[ncnt]);
          ncnt += 1;                 /* Bump arg counter */
          LSFlags |= SHOWOLDERTHAN;
        }
        break;
      case 'P':
        LSFlags |= (FULLPATHNAMES | LONGLIST);
        break;
      case 'R':
        LSFlags |= LISTALL;
        break;
      case 'T':
        LSFlags |= TOTALIZE;
        break;
      case 'X':
        if (argc < (ncnt + 1))     /* Missing required width number */
        {
          Usage ();
        }
        else
        {
          (VOID) GetDecNum (argv[ncnt], &CurWinCols);
          ncnt += 1;                 /* Bump arg counter */
        }
        break;
      case 'Y':
        if (argc < (ncnt + 1))     /* Missing required height number */
        {
          Usage ();
        }
        else
        {
          (VOID) GetDecNum (argv[ncnt], &CurWinRows);
          ncnt += 1;                 /* Bump arg counter */
        }
        break;
      case 'c':
        LSFlags |= LONGLIST | NOTEFLAG;
        break;
      case 'd':
        LSFlags &= ~SHOWFILES;
        break;
      case 'f':
        LSFlags &= ~SHOWDIRS;
        break;
      case 'k':
        LSFlags &= ~CONSOLE;
        break;
      case 'l':
        LSFlags |= LONGLIST;
        break;
      case 'n':
        LSFlags |= NOSORTFLAG;
        break;
      case 'p':
        procp = (struct Process *) FindTask (0L);
        procp->pr_WindowPtr = OldWindowPtr;
        break;
      case 'r':
        LSFlags |= REVFLAG;
        break;
      case 's':
        sortkey = 1;
        break;
      case 't':
        sortkey = 2;
        break;
      case 'v':
        if (argc < (ncnt + 1))     	/* Missing required pattern string */
          Usage ();
        theAntiPat = argv[ncnt];	/* Point to pattern area */
        ncnt += 1;			/* Skip over arg */
        LSFlags |= ANTIMATCH;		/* Set flag to use antipat */
        break;
      default:
        tmpmsg[0] = argv[cnt][i];
        tmpmsg[1] = 0;
        (VOID) asprintf (workstr, BadOptFmtStr, tmpmsg);
        WSTR (workstr);
        Usage ();
        break;
    }
  }
  return(ncnt);
}

/* -------------------------------------------------------------------- */
VOID _main (line)
  BYTE *line;
{
  BYTE *argv[MAXARG];             /* arg pointers */
  LONG argc;                      /* arg count */
  LONG cnt;
  struct Process *procp;

/* Prevent system request from occuring by default */
  procp = (struct Process *) FindTask (0L);
  OldWindowPtr = procp->pr_WindowPtr;
  procp->pr_WindowPtr = (APTR)-1L;
  
/* Construct list of args */
  GetCLIArgs (line, &argc, argv);

/* Grab FileHandles for input and output to console (or redirection file) */
  ConIn = (struct FileHandle *)Input ();
  ConOut = (struct FileHandle *)Output ();

/* Is this a CLI? Set a flag */
  if (IsInteractive ((BPTR)ConOut) != 0 && IsInteractive ((BPTR)ConIn) != 0)
    LSFlags |= CONSOLE;

/* Allocate a global FileInfoBlock for ExNext() */
  if ((GFibp = myalloc ((LONG) sizeof (struct FileInfoBlock))) == 0)
    NoMemExit ();

/* Initialize arg count, zero grand totals */
  cnt = 1;
  gtotblocks = gtotbytes = gdircount = gfilecount = 0;

/* Parse command line arguments, if any */
  do
  {
    if (cnt < argc && argv[cnt][0] == '-')
    {
    /* Reset for next arg */ 
      LSFlags &= CONSOLE;
      LSFlags |= SHOWDIRS | SHOWFILES;
      thefmtstr = deffmtstr;

      do
      {
        cnt = ParseCmdOptions(cnt, argc, argv);
      } while (cnt < argc && argv[cnt][0] == '-');
    }

  /* Is there an named path arg to do? */
    if (cnt < argc)
    {
      LSFlags |= PATHNAMED;                  /* Flag that we have a name */
      theFilePat[0] = 0;                     /* Terminate pattern strings */
      theDirPat[0] = 0;
      (VOID) stpcpy (thePath, argv[cnt]);    /* Copy this arg to work space */
      cnt++;                                 /* Advance arg counter */

    /* Wild pathname? Separate into components until we find a non-wild path */
      if (iswild (thePath) != 0)
      {
        (VOID) GetFileString (theFilePat, thePath);
        (VOID) stpcpy (theDirPat, theFilePat);
        (VOID) GetPathString (thePath, thePath);

      /* Still wild?  First part must be wild filename to match */
        if (iswild (thePath) != 0)
        {
          (VOID) GetFileString (theDirPat, thePath);
          (VOID) GetPathString (thePath, thePath);
        }
      }
#ifdef DEBUGIT
      asprintf(workstr, "path: %s\n", thePath); WSTR (workstr);
      asprintf(workstr, " dir: %s\n", theDirPat); WSTR (workstr);
      asprintf(workstr, "file: %s\n", theFilePat); WSTR (workstr);
#endif

    /* No wildcards allowed in the final pathname! */
      if (iswild (thePath) != 0)
        CleanUp (NoWildPathMsg, 20L, 120L);

    /* Now try to lock the dir (or file) */
      if ((CurFLock = (struct FileLock *)Lock (thePath, (LONG)ACCESS_READ)) == 0)
      {
        NoFileExit (thePath);
      }
    }
    else
    {
    /*
     * If no filename was specified, steal Lock on current directory from
     * CLI process task info.  We durn well better get something useful back;
     * since we don't do any error checking on the "borrowed" Lock.
     */
      CurFLock = (struct FileLock *)procp->pr_CurrentDir;
    }

  /* Make a full pathname string from given CurFLock if no colon in path */
    if (aindex(thePath, ':') == 0)
      MakePathString (CurFLock, thePath);
    curpath = thePath;

#ifdef DEBUGIT
    asprintf(workstr, "Final path: %s\n", thePath); WSTR (workstr);
#endif

  /* If there isn't a dir pattern or file pattern specified, match everything */
    if (theDirPat[0] == 0)
    {
      theDirPat[0] = '*';               /* "*" default matchall dir pattern */
      theDirPat[1] = 0;                 /* Null terminate string */
    }

    if (theFilePat[0] == 0)
    {
      theFilePat[0] = '*';
      theFilePat[1] = 0;
    }

  /* Get the directory for this path, display it */
    DirIt (CurFLock, thePath);

  /* Release the lock if we locked it */
    if (CurFLock != 0 && (LSFlags & PATHNAMED) != 0)
    {
      UnLock ((BPTR)CurFLock);
    }
    CurFLock = 0;

  /* Still more args? Put a linefeed between listing outputs
    if (cnt < argc && (LSFlags & NOHEADERS) == 0)
      WCHR (NLine);
   */
    TestBreak();
    if ((LSFlags & TOTALIZE) != 0)
    {
      SetConPen (penstr4);
      WSTR (TotHeaderMsg);
      SetConPen (penstr5);
      (VOID) asprintf (workstr, totalfmtstr, gdircount, gfilecount, gtotblocks, gtotbytes);
      WSTR (workstr);
      SetConPen (penstr0);
    }
  } while (cnt < argc && (LSFlags & BREAKFLAG) == 0);

/* All done! Clean exit */
  CleanUp (&NLine[1], 0L, 0L);
}


LONG GetFib (lock, inf, action)
BPTR lock;
struct FileInfoBlock *inf;
int action;
{
	struct fattr fa;
	struct MsgPort *dev;
	ULONG result;
	struct MyInfoBlock *minf;

	minf = (struct MyInfoBlock *)inf;
	if (lock != 0) {
		dev = btod(lock,struct FileLock *)->fl_Task;
	} else {
		dev = (struct MsgPort *)(((struct Process *)FindTask(0))->pr_FileSystemTask);
	}
		
	result = dos_packet(dev, action, lock, dtob(inf), &fa);
	
	if (!result) {
		action = (action==ACTION_EX_OBJECT) ? ACTION_EXAMINE_OBJECT : 
			ACTION_EXAMINE_NEXT;
		result = dos_packet(dev, action, lock, dtob(inf),0);
		if(!result) {
			return 0;
		}
		minf->is_remote = 0; 
	}  else {
		minf->is_remote = 1; 
	} 
			
	if (minf->is_remote) {	
		minf->uid = fa.uid;
		minf->gid = fa.gid;
		minf->mode = fa.mode;
	} 
	return 1;
}

long
dos_packet(port, type, arg1, arg2, arg3)
struct MsgPort *port;
long type, arg1, arg2, arg3;
{
	register struct StandardPacket *sp;
	register struct MsgPort *rp;
	long ret;
	
	if ((rp = CreatePort(0L, 0L)) == 0)
		return(0);
	if ((sp = AllocMem((long)sizeof(*sp), MEMF_PUBLIC|MEMF_CLEAR)) == 0) {
		DeletePort(rp);
		return(0);
	}
	sp->sp_Msg.mn_Node.ln_Name = (char *)&sp->sp_Pkt;
	sp->sp_Pkt.dp_Link = &sp->sp_Msg;
	sp->sp_Pkt.dp_Port = rp;
	sp->sp_Pkt.dp_Type = type;
	sp->sp_Pkt.dp_Arg1 = arg1;
	sp->sp_Pkt.dp_Arg2 = arg2;
	sp->sp_Pkt.dp_Arg3 = arg3;
	sp->sp_Pkt.dp_Arg4 = 0;
	sp->sp_Pkt.dp_Arg5 = 0;
	sp->sp_Pkt.dp_Arg6 = 0;
	sp->sp_Pkt.dp_Arg7 = 0;
	PutMsg(port, &sp->sp_Msg);
	WaitPort(rp);
	GetMsg(rp);
	ret = sp->sp_Pkt.dp_Res1;
	FreeMem(sp, (long)sizeof(*sp));
	DeletePort(rp);
	return(ret);
}
