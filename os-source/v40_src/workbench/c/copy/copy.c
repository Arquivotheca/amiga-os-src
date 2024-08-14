/* Notes:                                                                    *
 *                                                                           *
 * This program handles the following cases:                                 *
 *    1) Copy with wildcards                                                 *
 *    2) Copy file -> file                                                   *
 *    3) Copy file -> directory                                              *
 *    4) Copy directory -> directory                                         *
 *                                                                           *
 * In an attempt to make the program as small as possible, I have used       *
 * gotos and other nasty tricks.  Sorry about that, but hopefully there      *
 * are enough comments to enable the code to remain clear                    *
 *                                                                           *
 *---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "copy_rev.h"

#ifdef MWDEBUG
#include "memlib/memwatch.h"
#endif

#define TEMPLATE  "FROM/M,TO/A,ALL/S,QUIET/S,BUF=BUFFER/K/N,CLONE/S,DATES/S,NOPRO/S,COM/S,NOREQ/S" CMDREV
#define OPT_FROM     0
#define OPT_TO       1
#define OPT_ALL      2
#define OPT_QUIET    3
#define OPT_BUFS     4
#define OPT_CLONE    5
#define OPT_DATES    6
#define OPT_NOPRO    7
#define OPT_COM      8
#define OPT_NOREQ    9
#define OPT_COUNT    10

#define PARSE_KLUDGE
#define SEE_COLON_KLUDGE


/* need DOS V37 cause we now use ChangeMode() */
#define COPYDOSVER 37

/***********************************************************************
 *                        Messages used by this program                */

#define  MSG_DEST_WRONG_DIR   "Destination must be a directory.\n"
#define  MSG_DIR              "%s (Dir)"
#define  MSG_DIR_CREATE       "   [created]\n"
#define  MSG_COPIED           "copied.\n"
#define  MSG_TABIN            "        "	/* not strictly needed, but
						                       perhaps useful          */
#define  MSG_DELETED          "Destination file \"%s\" removed.\n"
#define  MSG_NOOPEN           "Can't open %s for "
#define  MSG_INOUT            "%sput - "
#define  MSG_IN               "in"
#define  MSG_OUT              "out"
#define  MSG_IOERROR          "Error in %sing - "
#define  MSG_READ             "read"
#define  MSG_WRITE            "writ"
#define  MSG_RECURS           "\nInfinite loop in \"%s\".\n"
#define  MSG_ERROR            "%s: "
#define  MSG_ERROR1           "%s - "
#define  MSG_TOWILD           "Wildcard destination invalid.\n"

/***********************************************************************/

#define  TABIN                8
#define  BUF_SAFE             100	/* To allow for extra memory
					                 * allocated by AllocVec      */
void Tab(int tabstops, struct DosLibrary * DOSBase);
ULONG IsFatal(LONG ErrNo);
LONG MatchNext2(struct AnchorPath *anchor, BPTR lock, struct DosLibrary * DOSBase);

/***********************************************************************
 *                                                                     *
 *    Copy program mainline.                                           *
 *                                                                     *
 * This routine is fairly optomized.  Since most of the code is error  *
 * handling, I've used GOTOs to collect the common areas.  It should   *
 * be clear anyway, in most cases clearer than it was before I         *
 * commonized the code sequences.  Its definately much smaller, and    *
 * has the advantage of putting all common error code in one place so  *
 * only one place needs changing if it needs modification.             *
 *                                                                     *
 * NOTE: This MUST be the first routine in the module                  *
 *                                                                     *
 ***********************************************************************/

#ifdef DEBUG1
int cmd_copy(void);
int main(void)
{
   return cmd_copy();
}

#endif

int cmd_copy(void)
{
   struct Library *SysBase;
   struct DosLibrary *DOSBase;
   int multisrc = FALSE, tabin = 0, isfile, created = 0;
   int cont, firsttime, inout;
   struct RDargs *rdargs = NULL;
   char *msgbuf1 = NULL, *msgbuf = NULL, *buf = NULL, *curarg=NULL, **argptr;
   long rc = RETURN_OK, res2 = 0, opts[OPT_COUNT];
   long bufsize, t=0, bs;
   BPTR outlock = NULL, temp, infh = NULL, outfh = NULL, temp1, temp2;
   BPTR destlock = NULL;
   struct FileInfoBlock *fib = NULL;
   APTR windptr;
   struct AnchorPath *ap = NULL;
   char	*fromless[] = {"",NULL};
#ifdef SEE_COLON_KLUDGE
   char *path, c;
#endif
   BPTR *prelocks = NULL;
   STRPTR *names = NULL;
   BPTR *arglock;
   BPTR oldCD;
   UWORD i,j;
   char oldCh;
   UWORD argcnt;
   BOOL argmatched;
   STRPTR argname;
   UWORD numsources = 0;
   BOOL ok;

/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(name, type) char a_##name[sizeof(type)+3]; \
			type *name = (type *)((LONG)(a_##name+3) & ~3);

	SysBase = (struct Library *)(*((struct Library **) 4));
	windptr = (APTR)-1;

   if ((DOSBase = (struct DosLibrary *) OpenLibrary(DOSNAME, COPYDOSVER))) {

/***********************************************************************
 *                                                                     *
 * Initialization section.  Allocate memory, initialize variables,     *
 *    etc.                                                             *
 *                                                                     *
 ***********************************************************************/

      if (CheckSignal(SIGBREAKF_CTRL_C)) {
	 PrintFault(ERROR_BREAK, NULL);
	 goto Done;
      }

/***********************************************************************
 *                                                                     *
 *    A common error routine lives near the bottom of this routine     *
 *    it is called ErrExit, and is where most fatal error cases end    *
 *    up.  It is gotten to with a goto rather than being a subroutine  *
 *                                                                     *
 ***********************************************************************/

      if ((msgbuf = AllocVec(MSGBUFSIZE, NULL)) == NULL ||
          (msgbuf1 = AllocVec(MSGBUFSIZE, NULL)) == NULL ||

	  (ap = AllocVec(sizeof(struct AnchorPath) + 256, MEMF_CLEAR)) == NULL ||

	  (fib = AllocVec(sizeof(struct FileInfoBlock), MEMF_PUBLIC)) == NULL) {

	 res2 = ERROR_NO_FREE_STORE;
	 PrintFault(ERROR_NO_FREE_STORE, NULL);

	 goto Done;
      }
      memset((char *) opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL) {
	 res2 = IoErr();
	 rc = RETURN_FAIL;
    t = 0;
	 goto ErrExit2;		/* <=== Bail out to common error routine       */
      } else {

/***********************************************************************
 *                                                                     *
 * We've got a good set of parameters.  Now we need to figure out the  *
 *    environment we're operating in.  The original BCPL copy routine  *
 *    had several special cases, each of which had a different output  *
 *    to the user.  Rather than duplicating the output, and having     *
 *    several special routines for each flavor of copy, this version   *
 *    is written to handle them all with one routine.                  *
 *                                                                     *
 * Here's a list of the possible variations to the input.  Originally  *
 *    multiargs were not supported, I will talk more about that later. *
 *                                                                     *
 *    1) One file to another file   - No output                        *
 *    2) One file to a directory    - No output                        *
 *          The difference of the two above are distinguished by the   *
 *          existance of the destination.  Its a file copy if the      *
 *          destination is a file, or does not exist, it is a copy     *
 *          to directory if it is a directory.                         *
 *    3) Directory to directory     - Prints each file                 *
 *    4) Wildcard to directory      - Prints each file                 *
 *    5) All parameter              - Prints each file                 *
 *                                                                     *
 * Delete printed nothing if multiple single files were given.  I      *
 * never liked that, I think whenever there are multiple things to do  *
 * some feedback needs to be given to the user.  This version of Copy  *
 * reflects that.  It always prints the file being worked on if there  *
 * is more than one file requested.                                    *
 *                                                                     *
 ***********************************************************************/

/***********************************************************************
 * If the user doesn't want requesters put on the screen, turn off     *
 * requesters for this process                                         *
 ***********************************************************************/

	 if (opts[OPT_NOREQ]) {
	    windptr = THISPROC->pr_WindowPtr;
	    THISPROC->pr_WindowPtr = (APTR) - 1;
	 }

/***********************************************************************\
 * I don't agree that this should be put in, however some have asked   *
 * for wildcards to be disallowed in the destination.                  *
\***********************************************************************/

#ifdef   PARSE_KLUDGE
   if((buf=AllocVec(500,0))==NULL || ParsePattern((char *)opts[OPT_TO], buf, 500))
#else
   if(ParsePattern((char *)opts[OPT_TO], NULL, 0))
#endif
   {
      PutStr(MSG_TOWILD);
		res2 = 0;
		rc = RETURN_ERROR;
		goto Done;
   }
#ifdef   PARSE_KLUDGE
   FreeVec(buf);
   buf = NULL;
#endif

/***********************************************************************\
 * Copy is documented as not requiring a FROM, I never realized this   *
 * but sure enough, even in the dos reference manual for 2.0 it says   *
 * this.  The behaviour is just like specifing a FROM "", so... we'll  *
 * fake it by providing the same structure that FROM "" would have     *
 * given.                                                              *
\***********************************************************************/
	if(opts[OPT_FROM] == NULL)
	{
		opts[OPT_FROM] = (long)&fromless;
	}

/***********************************************************************
 * We always may step into directories, so set the flag                *
 ***********************************************************************/

	 ap->ap_Flags = APF_DOWILD;
	 ap->ap_BreakBits = SIGBREAKF_CTRL_C;
	 ap->ap_Strlen = 256;

	 argptr = (char **) opts[OPT_FROM];
         numsources = 0;
         while (*argptr++)
             numsources++;

         if (!(prelocks = AllocVec((ULONG)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
         {
    	     res2 = ERROR_NO_FREE_STORE;
	     PrintFault(ERROR_NO_FREE_STORE, NULL);
	     goto Done;
         }

         if (!(names = AllocVec((ULONG)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
         {
    	     res2 = ERROR_NO_FREE_STORE;
	     PrintFault(ERROR_NO_FREE_STORE, NULL);
	     goto Done;
         }

	 argptr = (char **) opts[OPT_FROM];
	 argcnt = 0;
         while (argptr[argcnt])
         {
             argname = argptr[argcnt];
             i = 0;
             while ((argname[i] != ':') && (argname[i]))
                 i++;

             if (argname[i] == ':')
             {
                 i++;
		 oldCh = argname[i];
                 argname[i] = 0;

                 argmatched = FALSE;
                 j = 0;
                 while (j <= argcnt)
                 {
                     if ((names[j]) && (stricmp(argname,names[j]) == 0))
                     {
                         argmatched = TRUE;
                         break;
                     }
                     j++;
                 }

                 if (argmatched)
                 {
                     if (!(prelocks[argcnt] = DupLock(prelocks[j])))
		     {
                         res2 = IoErr();
	                 PrintFault(res2, NULL);
	                 goto Done;
	             }

                     argname[i] = oldCh;
                     if (oldCh)
                         strcpy(argname,&argname[i]);
                     else
                         argname[0] = 0;
                 }
                 else
                 {
                     if (!(prelocks[argcnt] = Lock(argname,ACCESS_READ)))
                     {
                         argname[i] = oldCh;
                         res2 = IoErr();
                         if (res2 != ERROR_ACTION_NOT_KNOWN)
                         {
                             t = (LONG)argname;
                             VPrintf(MSG_NOOPEN,&t);

                             t = (long)&MSG_IN;
                             VPrintf(MSG_INOUT, &t);
                             PrintFault(res2, NULL);
                             t = NULL;
                             goto Done;
                         }
                     }
                     else
                     {
                         if (!(names[argcnt] = AllocVec(strlen(argname)+1,MEMF_CLEAR|MEMF_PUBLIC)))
                         {
                             res2 = ERROR_NO_FREE_STORE;
                             PrintFault(ERROR_NO_FREE_STORE, NULL);
                             goto Done;
                         }
                         strcpy(names[argcnt],argname);

                         argname[i] = oldCh;
                         if (oldCh)
                             strcpy(argname,&argname[i]);
                         else
                             argname[0] = 0;
                     }
                 }
             }
             argcnt++;
         }

	 argptr = (char **) opts[OPT_FROM];
         arglock = &prelocks[0];

         t = 0;

         if (*arglock)
             oldCD = CurrentDir(*arglock);

	 if (MatchFirst(*argptr, ap)) {
	    if (*arglock)
                CurrentDir(oldCD);
	    res2 = IoErr();
		if( res2 == ERROR_OBJECT_NOT_FOUND ){

			t = opts[OPT_FROM];
			VPrintf(MSG_NOOPEN, (void *)opts[OPT_FROM]);

			t = (long)&MSG_IN;
			VPrintf(MSG_INOUT, &t);
			PrintFault(res2, NULL);
			t = NULL;
			rc = RETURN_FAIL;
#ifdef SEE_COLON_KLUDGE

/*
 * This code is to keep install scripts from breaking.  If
 * an attempt is made to copy a specific file from C: which is
 * not there, return warn rather than error.
 * (C= put a bunch of things resident into the shell
 * and removed it from C: in 2.0, not a good idea IMHO)
 */
                        path = PathPart(*(char **)opts[OPT_FROM]);
                        c = *path;
                        *path = '\0';
                        if (*arglock)
                           oldCD = CurrentDir(*arglock);
                        temp = Lock(*(char **)opts[OPT_FROM], ACCESS_READ);
                        temp1 = Lock("C:", ACCESS_READ);
                        if (*arglock)
                            CurrentDir(oldCD);

                        rc = (SameLock( temp, temp1 ) == LOCK_SAME )?
                                                        RETURN_WARN:RETURN_FAIL;
                        *path = c;
                        UnLock(temp);
                        UnLock(temp1);
                        temp = NULL;
                        temp1 = NULL;
#endif
			goto Done;
		}
	    goto ErrExit;
	 }

	 isfile = IsFileSystem((char *) opts[OPT_TO]);
	 if (!isfile && IoErr() && IoErr() != ERROR_ACTION_NOT_KNOWN) {
	    res2 = IoErr();

            if (*arglock)
  	        CurrentDir(oldCD);

	    goto ErrExit;
	 }

         if (*arglock)
  	     CurrentDir(oldCD);

/***********************************************************************
 *                                                                     *
 * If the first parameter is wild, or is a directory, or there are     *
 * more than one input files, then this program goes into 'multisrc'   *
 * mode.  This means the output must be a directory, and files will    *
 * be printed as output.                                               *
 *                                                                     *
 ***********************************************************************/

	 if (((ap->ap_Flags & APF_ITSWILD) != 0) ||
	     (ap->ap_Info.fib_DirEntryType > 0) ||
	     (argptr[1] != NULL)) {

	    multisrc = TRUE;

/***********************************************************************
 *                                                                     *
 * If the input parameter is not wild, and it is a directory, then     *
 * step into it.  Later code is not aware that we've steped into the   *
 * directory, so we have to be careful about not taking what           *
 * MatchFirst/Next as gosphal.                                         *
 *                                                                     *
 ***********************************************************************/

	    if ((ap->ap_Flags & APF_ITSWILD) == 0 &&
		(ap->ap_Info.fib_DirEntryType > 0)) {
	       ap->ap_Flags |= APF_DODIR;
	       if(MatchNext2(ap,*arglock,DOSBase)) {
		   res2 = IoErr();
	           if(res2 != ERROR_NO_MORE_ENTRIES)goto ErrExit; /* new test */
	       }
	    }
	 }
    t = opts[OPT_TO];
	 if (multisrc && isfile) {

/***********************************************************************
 *                                                                     *
 * Output MUST be a directory.  If it doesn't exist, create it.        *
 * If its not a directory, abort program.                              *
 *                                                                     *
 ***********************************************************************/
	    if ((outlock = Lock((char *) opts[OPT_TO], SHARED_LOCK))) {

	       if (Examine(outlock, fib)) {
		  if (fib->fib_DirEntryType < 0) {
		     PutStr(MSG_DEST_WRONG_DIR);
		     res2 = 0;
		     rc = RETURN_ERROR;
		     goto Done;
		  }
	       } else {
		  res2 = IoErr();
		  goto ErrExit;
	       }
	       created = FALSE;
	    } else {
	       if ((outlock = CreateDir((char *) opts[OPT_TO])) == NULL) {
		  res2 = IoErr();
		  goto ErrExit;
	       }
	       created = TRUE;
	       if (!opts[OPT_QUIET]) {
		  VPrintf("   %s" MSG_DIR_CREATE, &opts[OPT_TO]);
	       }
	       UnLock(outlock);
	       if ((outlock = Lock((char *) opts[OPT_TO], SHARED_LOCK)) == NULL)
	       {
		  res2 = IoErr();
		  goto ErrExit;
	       }
	    }
	    if (!(destlock = DupLock(outlock)))
	    {
                res2 = IoErr();
		goto ErrExit;
	    }
	 } else {

/***********************************************************************
 *                                                                     *
 * Not multiple source files, doesn't have to be a directory.          *
 * If its not a directory, unlock outlock.  Outlock is a lock on the   *
 * parent directory of the output file, not the output file itself.    *
 *                                                                     *
 ***********************************************************************/

	    if ((outlock = Lock((char *) opts[OPT_TO], ACCESS_READ)) == NULL) {
	       if (((res2 = IoErr()) != ERROR_OBJECT_NOT_FOUND) &&
		   (res2 != ERROR_ACTION_NOT_KNOWN))
		  goto ErrExit;
	    }
	    if (outlock) {
	       if (Examine(outlock, fib)) {
		  if (fib->fib_DirEntryType < 0) {
		     UnLock(outlock);
		     outlock = NULL;
		  }
	       } else {
		  res2 = IoErr();
		  goto ErrExit;
	       }
	    }
	 }

/***********************************************************************
 *                                                                     *
 * At this point, the following things are true:                       *
 *                                                                     *
 *          1) if dest is a dir we got a lock on it                    *
 *          2) if dest isn't a dir we don't                            *
 *          3) we know if the input is wild, or multiargs              *
 *          4) we should have a anchor on the first input file         *
 *                                                                     *
 *       we still need to:                                             *
 *                                                                     *
 *          1) allocate buffers                                        *
 *          2) turn off requesters if asked                            *
 *                                                                     *
 ***********************************************************************/

         if (*arglock)
  	     oldCD = CurrentDir(*arglock);

	 if (opts[OPT_BUFS])
	 {
	    bufsize = *(long *) opts[OPT_BUFS];

	    if (bufsize < 0 || !IsFileSystem(argptr[0]))
	       bufsize = 128;

/***********************************************************************
 * Doesn't make much sense to ask for negative buffers.  If the user   *
 * did, just quietly set the size to the default. 0 buffers is a       *
 * special case.  I'll go more into that when the code comes up        *
 ***********************************************************************/

	 }
	 else		/* No BUF argument, so.. */
	 {
             bufsize = 128;
	 }
	 bufsize *= 0x200;

         if (*arglock)
  	     CurrentDir(oldCD);


/***********************************************************************
 * If positive bufsize, allocate the buffer.                           *
 ***********************************************************************/

	 if (bufsize)
         {
             while (TRUE)
             {
                 if (buf = AllocVec(bufsize, 0))
                     break;

                 if (bufsize <= 1024)
                 {
                     res2 = ERROR_NO_FREE_STORE;
                     t = 0;
                     goto ErrExit;
                 }
                 bufsize -= 1024;
             }
	 }

/***********************************************************************
 *                                                                     *
 *    And now the moment we've all been waiting for.  The main loop of *
 *    this program.  The environment is set up, all the memory is      *
 *    allocated, its dark out, and we've got our sun glasses on.       *
 *    Let's rock.                                                      *
 *                                                                     *
 ***********************************************************************/

	 firsttime = TRUE;

/***********************************************************************
 * Step over multiargs                                                 *
 ***********************************************************************/

	 while (curarg = *argptr++) {
	    if (firsttime)
	       firsttime = FALSE;
	    else {

/***********************************************************************
 * Unless its the first time through the loop, do a little             *
 * initialization.  Make sure the file exists.  If the file is a dir   *
 * and is not wild, step into it.                                      *
 ***********************************************************************/

	       UnLock(outlock);
               if (!(outlock = DupLock(destlock)))
               {
		  res2 = IoErr();
		  goto ErrExit;
               }

	       arglock++;
               if (*arglock)
                   oldCD = CurrentDir(*arglock);
	       if (MatchFirst(curarg, ap)) {
	          if (*arglock)
	              CurrentDir(oldCD);
		  res2 = IoErr();
        	  t = (long)curarg;
			if( res2 == ERROR_OBJECT_NOT_FOUND ){

				t = opts[OPT_FROM];
				VPrintf(MSG_NOOPEN, (void *)&curarg);

				t = (long)&MSG_IN;
				VPrintf(MSG_INOUT, &t);
				PrintFault(res2, NULL);
				t = NULL;
				rc = RETURN_WARN;
				continue;
			}
		  goto ErrExit;
	       }

	       if (*arglock)
	           CurrentDir(oldCD);

	       if ((ap->ap_Flags & APF_ITSWILD) == 0 &&
		   (ap->ap_Info.fib_DirEntryType > 0)) {
		  ap->ap_Flags |= APF_DODIR;
		  if(MatchNext2(ap,*arglock,DOSBase)) {
		        res2 = IoErr(); /* new test */
	               if(res2 != ERROR_NO_MORE_ENTRIES)goto ErrExit;
		  }
	       }
	    }

/***********************************************************************
 *                                                                     *
 * Main copy loop, each argument gets copied here, one pass is made    *
 * through the loop for simple file to file copies, several times      *
 * through for wildcards or directories.                               *
 *                                                                     *
 ***********************************************************************/

	    do {

/***********************************************************************
 *                                                                     *
 * Using MatchFirst/Next stepping into directories list the directory  *
 * twice.  Once going in, and another time going out.  This is handy   *
 * because some programs, like delete are concerned with exiting the   *
 * the directory, others like this one want to know when a directory   *
 * is entered.                                                         *
 *                                                                     *
 * The following code takes care of walking back up the directory tree *
 * its at the beginning of the do loop rather than the end because we  *
 * need to ignore the second time the entry is hit, other than         *
 * popping out.                                                        *
 *                                                                     *
 ***********************************************************************/

	       while (ap->ap_Flags & APF_DIDDIR) {
		  tabin--;
			if(isfile){
			  temp = outlock;
			  outlock = ParentDir(outlock);
			  UnLock(temp);

			  if( outlock && (ap->ap_Info.fib_DirEntryType > 0) &&
			     (opts[OPT_CLONE] || opts[OPT_DATES])			){
				temp = CurrentDir(outlock);
				SetFileDate(ap->ap_Info.fib_FileName, &ap->ap_Info.fib_Date);
				CurrentDir(temp);
			  }
			}
		  /* tell Anchor to step out */
		  ap->ap_Flags &= ~APF_DIDDIR;

/***********************************************************************
 *                                                                     *
 * tabin in the following if statement is somewhat subtle.  Recall if  *
 * we're copying a directory, we do a DODIR, but don't create the      *
 * directory and step in.  MatchNext under that circumstance would     *
 * pass to this routine the original directory at the very end of the  *
 * copy.  We use the tabin variable to indicate this has occured, and  *
 * exit the loop even though MatchNext doesn't tell us to do this      *
 *                                                                     *
 ***********************************************************************/

		  if ((cont = MatchNext2(ap,*arglock,DOSBase)) || (tabin < 0)) {
		     goto ExitLoop;	/* Pardon the goto, trying for small */
		  }
	       }

	       if (CheckSignal(SIGBREAKF_CTRL_C)) {
		  PrintFault(ERROR_BREAK, NULL);
		  goto Done;
	       }
/***********************************************************************
 *                                                                     *
 * The following loop is the trickiest code in the program.  Its       *
 * purpose is to keep evaluating until we get to a file.  If the all   *
 * option isn't set, it will just go to the main copy loop.  If it is  *
 * this routine will keep creating directories and walking the dir     *
 * structure until a file is found.  In the event an empty directory   *
 * is found, it will drop back a directory, and continue the copy.     *
 *                                                                     *
 ***********************************************************************/

	       while (ap->ap_Info.fib_DirEntryType > 0) {

		  if (!opts[OPT_QUIET]) {
		     Tab(tabin + 1, DOSBase);
		     t = (LONG) ap->ap_Info.fib_FileName;
		     VPrintf(MSG_DIR, &t);
		  }
		  if (opts[OPT_ALL]) {
/***********************************************************************
 *                                                                     *
 * The previous line of code has already confused two people, myself,  *
 * and the person who do the walkthrough of the program.  We are       *
 * guaranteed to be multisource here because multisource is set if     *
 * we're copying a directory, or if there's a wildcard.  If we find    *
 * a directory under those circumstances, if ALL is set, we create     *
 * and lock the directory, if all is not set, we just print that we've *
 * hit the directory, and go to the next file.                         *
 *                                                                     *
 ***********************************************************************/

		     if (CheckSignal(SIGBREAKF_CTRL_C)) {
			PrintFault(ERROR_BREAK, NULL);
			goto Done;
		     }

/***********************************************************************\
 *                                                                     *
 * If a directory is linked to a subdirectory of itself, infinite      *
 * recursion can occur.  I can actually see someone doing this for a   *
 * legitimate reason, if that person had really nasty deep directory   *
 * structures.  Anyway, a check goes here and if it would cause        *
 * this problem, it will be fixed caught.                              *
 *                                                                     *
\***********************************************************************/

			/* Get a lock on the input directory */
			temp = CurrentDir(ap->ap_Current->an_Lock);
			temp1 = Lock(ap->ap_Info.fib_FileName, SHARED_LOCK);
			CurrentDir(temp);

			temp2 = DupLock(ap->ap_Current->an_Lock);

			while (temp2 != NULL){
				if (SameLock(temp1, temp2) == LOCK_SAME) {
					t = (LONG) ap->ap_Info.fib_FileName;
					VPrintf(MSG_RECURS, &t);
					res2 = 0;
					UnLock(temp1);
					UnLock(temp2);
					goto ErrExit;
				}
				temp = ParentDir(temp2);
				UnLock(temp2);
				temp2 = temp;
			};

			if(isfile){

				if (created) {
					temp2 = ParentDir(outlock);
				} else {
					temp2 = DupLock(outlock);
				}

				/* Walk up the chain of parents looking for the
					current input directory.  If its on the list
					infinite recursion will occure.              */

				while (temp2 != NULL){
					if (SameLock(temp1, temp2) == LOCK_SAME) {
						t = (LONG) ap->ap_Info.fib_FileName;
						VPrintf(MSG_RECURS, &t);
						res2 = 0;
						UnLock(temp1);
						UnLock(temp2);
						goto ErrExit;
					}
					temp = ParentDir(temp2);
					UnLock(temp2);
					temp2 = temp;
				};

				temp = CurrentDir(outlock);
				if ((outlock = Lock(ap->ap_Info.fib_FileName, SHARED_LOCK)) == NULL)
				{
					if ((outlock = CreateDir(ap->ap_Info.fib_FileName)) == NULL)
					{
						res2 = IoErr();
						outlock = CurrentDir(temp);
						if (!opts[OPT_QUIET])
							PutStr("\n");
						strcpy( msgbuf1, ap->ap_Info.fib_FileName);
						t = (long)msgbuf1;

						goto ErrExit;
					} else if (!opts[OPT_QUIET]) PutStr(MSG_DIR_CREATE);

				} else 		if (!opts[OPT_QUIET]) PutStr("\n");

				tabin++;
				temp = CurrentDir(temp);
				UnLock(temp);		/* release parent lock */
			} else 			if (!opts[OPT_QUIET]) PutStr("\n");
			UnLock(temp1);

		  } else {

/***********************************************************************
 *                                                                     *
 * This means we've hit a directory without option all set.  It will   *
 * only happen the first time through the while loop.  Since we're     *
 * not copying directories at this point, skip this dir and go to      *
 * the next file. (branches to the main copy loop)                     *
 *                                                                     *
 ***********************************************************************/

		     if (!opts[OPT_QUIET])
			PutStr("\n");
		     goto Loop;
		  }

/***********************************************************************
 * Okay, we've found a directory, and know we want to copy it, so now  *
 * Let's step into the directory.                                      *
 ***********************************************************************/

		  ap->ap_Flags |= APF_DODIR;
		  if(MatchNext2(ap,*arglock,DOSBase)) {
		   cont = IoErr();   /* new test */
	           goto ExitLoop;
		  }
		  if (ap->ap_Flags & APF_DIDDIR) {

/***********************************************************************
 * This means there's an empty directory, we've already created it,    *
 * but oh, well.. What should occur at this point, is we go to the     *
 * file in the parent directory following this directory without       *
 * copying                                                             *
 ***********************************************************************/

		     tabin--;
			 if(isfile){
			     temp = outlock;
			     outlock = ParentDir(outlock);
			     UnLock(temp);
				 if( outlock && (ap->ap_Info.fib_DirEntryType > 0)	&&
			     	 (opts[OPT_CLONE] || opts[OPT_DATES])			)
				 {
					temp = CurrentDir(outlock);
					SetFileDate(ap->ap_Info.fib_FileName, &ap->ap_Info.fib_Date);
					CurrentDir(temp);
				 }
			 }
		     ap->ap_Flags &= ~APF_DIDDIR;	/* tell Anchor to step out */
		     goto Loop;
		  }
	       }

/***********************************************************************
 *                                                                     *
 * Okay, here's the condition we're currently in...  We're pointing    *
 * to something that must be a file with the anchor, if we're copying  *
 * a file, outlock is NULL, otherwise outlock is a lock on the parent  *
 * directory of the soon-to-be file.                                   *
 *                                                                     *
 * Its now time to open the files.  Output file first, then input file *
 * Currently, if the open doesn't work, the whole program aborts, by   *
 * changing the common failure code, it would be easy to just loop     *
 * instead.                                                            *
 *                                                                     *
 * Quite a few bytes of code are saved by having common error handling *
 * for all three of the open statements.                               *
 *                                                                     *
 ***********************************************************************/

	       inout = TRUE;

	       temp = CurrentDir(ap->ap_Current->an_Lock);
	       if (!(infh = Open(ap->ap_Info.fib_FileName, MODE_OLDFILE))) {
		  inout = FALSE;
	       }
	       CurrentDir(temp);
	       if (infh){
			  if (outlock) {
			     temp = CurrentDir(outlock);
			     outfh = Open(ap->ap_Info.fib_FileName, MODE_NEWFILE);
		    	     CurrentDir(temp);
			  } else {
			     outfh = Open((char *) opts[OPT_TO], MODE_NEWFILE);
			  }
			}

	       if (!outfh || !infh) {
			res2 = IoErr();
			t = (int) ap->ap_Info.fib_FileName;
		  if(!outfh)
		  {
			if(!multisrc)
			{
				t=(int)opts[OPT_TO];
			}
			Close(infh);
			infh = 0;
			}

		  VPrintf(MSG_NOOPEN, &t);

		  t = (int) ((inout) ? &MSG_OUT : &MSG_IN);
		  VPrintf(MSG_INOUT, &t);
		  PrintFault(res2, NULL);
		  rc = RETURN_WARN;
		  if (IsFatal(res2)) {
		     MatchEnd(ap);
		     goto Done;
		  } else {
		     res2 = NULL;
		     goto Loop;
		  }
	       }
/***********************************************************************
 *                                                                     *
 * Hokay, now we've got both files open the input file is examined     *
 * into fib, so we print the name and copy the file.                   *
 *                                                                     *
 ***********************************************************************/

	       if (multisrc && !opts[OPT_QUIET]) {
		  Tab(tabin, DOSBase);
		  t = (LONG) ap->ap_Info.fib_FileName;
		  VPrintf("   %s..", &t);
		  Flush(Output());
	       }
/***********************************************************************
 *                                                                     *
 * Here is the promised new feature.  If BUFSIZE = 0 memory is         *
 * allocated and deleted for each file.  The buffer ends up being      *
 * exactly the right size for the file.  This gives the fast fileing   *
 * system the exact size of the file, hopefully it will find a space   *
 * on the disk the right size and will put the file contigious on the  *
 * disk.  This of course is a big win when it comes to running the     *
 * program, it will load a program stored on contigious sectors with   *
 * one file request, thereby loading even big programs very fast.      *
 *                                                                     *
 * I stole this idea from an early copy program for the amiga called   *
 * xcopy.                                                              *
 *                                                                     *
 ***********************************************************************/

	       if (bufsize == 0) {
		  bs = ap->ap_Info.fib_Size;

		  if (bs == 0)
		      bs = 8;

                  while (TRUE)
                  {
		      if (buf = AllocVec(bs, 0))
		          break;

		      if (bs <= 1024)
		      {
			  res2 = ERROR_NO_FREE_STORE;
         		  t = 0;
			  goto ErrExit;
	              }
	              bs -= 1024;
		  }
    	       }
	      else bs = bufsize;

/***********************************************************************
 * Here's the Copy (finally) lotta work for such a short thing, eh???  *
 ***********************************************************************/
	       while ((t = Read(infh, buf, bs)) > 0) {

		  if (CheckSignal(SIGBREAKF_CTRL_C)) {
		     t = 1;
		     PrintFault(ERROR_BREAK, NULL);
		     SetIoErr(0);
		     break;
		  }
		  if (Write(outfh, buf, t) != t) {
		     t = 1;
		     break;
		  }
	       }
/***********************************************************************
 * Collect the error reason (if any)                                   *
 * T is non-zero if any error occured                                  *
 ***********************************************************************/
	       if (t) {
		    res2 = IoErr();
	       }
/***********************************************************************
 * Get all the files closed before processing any error.               *
 ***********************************************************************/
	       Close(outfh);
	       outfh = NULL;

	       Close(infh);
	       infh = NULL;

	       if (outlock) {
		  temp = CurrentDir(outlock);
		  strcpy(msgbuf, ap->ap_Info.fib_FileName);
	       }
	       else {
                if( strlen((char *)opts[OPT_TO]) >= MSGBUFSIZE )
                {
                        FreeVec(msgbuf);
			/* Since strlen does not include the terminating NULL
			   we need strlen + 1 */
                        if((msgbuf = AllocVec(1 + strlen((char *)opts[OPT_TO]), NULL))
                                                                           == NULL)
                        {
                                res2 = ERROR_NO_FREE_STORE;
                                goto ErrExit;
                        }
                }
                strcpy( msgbuf, (char *)opts[OPT_TO]);

	       }

	       if (t) {
		  if (res2) {

/***********************************************************************
 * if res2 must have been some error, read returns < 0 if error.       *
 * Write is set above                                                  *
 ***********************************************************************/

		     t = (int) ((t < 0) ? &MSG_READ : &MSG_WRITE);
		     VPrintf(MSG_IOERROR, &t);
		     PrintFault(res2, NULL);
		     rc = RETURN_ERROR;
		     if ((t == (long) &MSG_READ) ||
			 (res2 != ERROR_WRITE_PROTECTED)) {
				if(isfile){
					VPrintf(MSG_DELETED, (int *) &msgbuf);
					DeleteFile(msgbuf);
				}
		     }
		     if ((LONG)temp != -1) CurrentDir(temp);
		     if (IsFatal(res2)) {
			MatchEnd(ap);
			goto Done;
		     }
		     else goto Loop;
		  }
		  else {

/***********************************************************************
 * musta hit ^C in the loop                                            *
 ***********************************************************************/

			if(isfile)
			{
				VPrintf(MSG_DELETED, (int *) &msgbuf);
				DeleteFile(msgbuf);
			}

		     if ((LONG)temp != -1) CurrentDir(temp);
	             t = 0;
		     goto ErrExit1;
		  }
	       }
	       if (bufsize == 0) {
		  FreeVec(buf);
		  buf = NULL;
	       }
	       if (multisrc && !opts[OPT_QUIET])
		  PutStr(MSG_COPIED);

/***********************************************************************
 * Set protection bits, dates, and comment if applicable               *
 ***********************************************************************/

	       if (isfile)
	       {
	           ok = TRUE;
		   if (!opts[OPT_NOPRO])
                       if (SetProtection(msgbuf,(ap->ap_Info.fib_Protection & ~FIBF_ARCHIVE)) == DOSFALSE)
                           if (IoErr() != ERROR_ACTION_NOT_KNOWN)
                               ok = FALSE;

                   if (ok)
                       if (opts[OPT_DATES] || opts[OPT_CLONE])
                           if (SetFileDate(msgbuf, &ap->ap_Info.fib_Date) == DOSFALSE)
                               if (IoErr() != ERROR_ACTION_NOT_KNOWN)
                                   ok = FALSE;

                   if (ok)
                       if (opts[OPT_CLONE] || opts[OPT_COM])
                           if (SetComment(msgbuf, ap->ap_Info.fib_Comment) == DOSFALSE)
                               if (IoErr() != ERROR_ACTION_NOT_KNOWN)
                                   ok = FALSE;

                   if (!ok)
                   {
                       PrintFault(IoErr(), msgbuf);
                       rc = RETURN_WARN;
                   }
	       }

	       if ((LONG)temp != -1)
		  CurrentDir(temp);

/***********************************************************************
 *                                                                     *
 * End of main copy loop.  Find next file to copy (if any)             *
 *                                                                     *
 ***********************************************************************/
	     Loop:;
	    } while ((cont = MatchNext2(ap,*arglock,DOSBase)) == NULL);

/***********************************************************************
 *                                                                     *
 * Error handling while in main copy loop.  When fatal errors occur in *
 * the loop control is passed to here.  Also, if MatchNext at the end  *
 * of the loop fails we execute this code.                             *
 *                                                                     *
 ***********************************************************************/

	  ExitLoop:
	    if (cont != ERROR_NO_MORE_ENTRIES && cont != NULL) {
	       res2 = IoErr();
	       goto ErrExit;
	    }
	    MatchEnd(ap);
	 }
      }
      goto Done;

/***********************************************************************
 *                                                                     *
 * main error code.  Fatal errors end up here generally.               *
 *                                                                     *
 ***********************************************************************/

    ErrExit:
      rc = RETURN_FAIL;
    ErrExit1:
      MatchEnd(ap);
    ErrExit2:
      if (res2 == ERROR_NO_MORE_ENTRIES) {
	 res2 = 0;
	 rc = RETURN_OK;
      }
      if (res2) {
         if(res2 != ERROR_BREAK){
            GetProgramName(msgbuf, MSGBUFSIZE);
            VPrintf(MSG_ERROR, (void *)&msgbuf);
            if(t){
               VPrintf(MSG_ERROR1, (void *)&t);
            }
	         PrintFault(res2, NULL);
         }
      }
/***********************************************************************
 * Clean up from whatever state we were in                             *
 ***********************************************************************/

    Done:
      if (prelocks)
      {
          i = 0;
          while (i < numsources)
              UnLock(prelocks[i++]);
          FreeVec(prelocks);
      }

      if (names)
      {
          i = 0;
          while (i < numsources)
              FreeVec(names[i++]);
          FreeVec(names);
      }

      FreeVec(msgbuf);
      FreeVec(msgbuf1);
      FreeVec(ap);
      FreeVec(buf);
      FreeVec(fib);

      if (rdargs)
          FreeArgs(rdargs);

      if (infh)
	 Close(infh);

      if (outfh)
	 Close(outfh);

      UnLock(outlock);
      UnLock(destlock);

      if (windptr != (void *) -1) {
	 THISPROC->pr_WindowPtr = windptr;
      }

      SetIoErr(res2);

      CloseLibrary((struct Library *) DOSBase);
   } else {
      OPENFAIL;
   }
   return (rc);
}


LONG MatchNext2(struct AnchorPath *anchor, BPTR lock, struct DosLibrary * DOSBase)
{
LONG result;
BPTR old;

    if (lock)
      old = CurrentDir(lock);

    result = MatchNext(anchor);

    if (lock)
      CurrentDir(old);

    return(result);
}


/***********************************************************************
 *                                                                     *
 * void  Tab( int, *dosbase ) : Prints 8 spaces for each tab stop      *
 *                              specified.                             *
 ***********************************************************************/
void Tab(int tabstops, struct DosLibrary * DOSBase)
{
   int i;
   for (i = 0; i < tabstops; i++) {
      PutStr(MSG_TABIN);
   }
}

/************************************************************************
 *                                                                      *
 * ULONG IsFatal( LONG ErrNo )   : Returns boolean indicating an abort  *
 *                                  of the program is in order.         *
 ************************************************************************/
ULONG IsFatal(LONG ErrNo)
{
#define CNV( errno ) (1<<(errno-201))
   ULONG fatals =
   CNV(ERROR_DISK_NOT_VALIDATED) |
   CNV(ERROR_DISK_WRITE_PROTECTED) |
   CNV(ERROR_DEVICE_NOT_MOUNTED) |
   CNV(ERROR_SEEK_ERROR) |
   CNV(ERROR_NOT_A_DOS_DISK) |
   CNV(ERROR_DISK_FULL) |
   CNV(ERROR_NO_DISK);

   return (ULONG) ((ErrNo > 200) &&
		   (ErrNo < 232) &&
		   ((fatals & CNV(ErrNo)) != 0));
}
