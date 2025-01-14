/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*-----------------------------------------------------------------------*/
/* Command: Edit1.c                                                      */
/* Author:  John A. Toebes, VIII                                         */
/* Change History:                                                       */
/*  Date    Person        Action                                         */
/* -------  ------------- -----------------                              */
/* 25NOV90  John Toebes   Corrected crashing problems with buffer overrun*/
/* 03NOV89  John Toebes   Initial Creation                               */
/* 17APR91  John Toebes   Corrected Memory allocation failure problems   */
/* Notes:                                                                */
/*-----------------------------------------------------------------------*/
#include "edit.h"
#include "edit_rev.h"

#define TEMPLATE    "FROM/A,TO,WITH/K,VER/K,OPT/K,WIDTH/N,PREVIOUS/N" CMDREV
#define OPT_FROM   0
#define OPT_TO     1
#define OPT_WITH   2
#define OPT_VER    3
#define OPT_OPT    4
#define OPT_WIDTH  5
#define OPT_PREVIOUS 6
#define OPT_COUNT  7

int __stdargs _main()
{
   long opts[OPT_COUNT];
   int i;
   char *p;
   struct LINE *l, *n;
   struct FILESTRUC iovec[2];
   LEVELBUF level;

   BPTR oldoutput = Output();
   BPTR oldinput = Input();

   rc = 0;
   opened = FALSE;
   if (setjmp(level)) goto quitlab;
   quitlevel = &level;
   zerolevel = NULL;
   verout = oldoutput;
   edits  = oldinput;
   commlinel = 0;
   commpoint = 0;
   veclist = NULL;
   filelist = NULL;
   cfsp = 0;

   if (DOSBase->dl_lib.lib_Version < 36)
   {
      Write(Output(), "Edit requires Version 2.0 to run\n", 33);
      Result2(ERROR_INVALID_RESIDENT_LIBRARY);
      return(RETURN_FAIL);
   }
   memset((char *)opts, 0, sizeof(opts));
   rdargs = ReadArgs(TEMPLATE, opts, NULL);

   if (rdargs == NULL)
   {
      error(ERROR_ARG);
   }

   e_from    = (char *)opts[OPT_FROM];
   e_to      = (char *)opts[OPT_TO];
   e_work    = e_to;
   e_with    = (char *)opts[OPT_WITH];
   e_ver     = (char *)opts[OPT_VER];
   e_temp    = gettemp(e_to?e_to:e_from);
   i = strlen(e_temp);
   e_backup  = newvec(i+13);  /* Account for /EDIT-BACKUP\0 */
   memcpy(e_backup, e_temp, i);
   memcpy(e_backup+i, "/EDIT-BACKUP", 13);
   e_workin  = tempname("/E00-WK1");
   e_workout = tempname("/E00-WK2");

   if (e_to == NULL)
   {
      e_to = e_from;
      e_work = e_workin;
      e_workin = e_workout;
      e_workout = e_work;
   } 

   if (e_ver != NULL)
   {
      verout = Open(e_ver, MODE_NEWFILE);
      if (verout == NULL) error(ERROR_FFA, e_ver);
   }

   if (e_with != NULL)
   {
      edits = Open(e_with, MODE_OLDFILE);
      if (edits == NULL)  error(ERROR_FFA, e_with);
   }

   maxlinel  = LMAX;
   maxplines = PMAX;

   if ((p = (char *)opts[OPT_OPT]) != NULL)
   {
      while(*p)
      {
         switch(*p++)
         {
            case 'w':
            case 'W':  p = rdn(p, &maxlinel);  break;
            case 'p':
            case 'P':  p = rdn(p, &maxplines); break;
         }
      }
      if (maxlinel <= 0 || maxplines <= 0)
        error(ERROR_OPT);
   }

   freelines = newvec((sizeof(struct LINE)+(maxlinel<<1))*(maxplines+3));
   freelines->l_next = 0;

   for (i = 1, l = freelines; i < maxplines; i++, l=n)
   {
      n = (struct LINE *)((char *)l + (sizeof(struct LINE)+maxlinel<<1));
      l->l_next = n;
   }
   l->l_next = NULL;

   commbuf   = newvec(maxlinel+2);

   primaryoutput = &iovec[0];
   primaryinput  = &iovec[1];

   verifying  = IsInteractive(edits);
   SelectOutput(verout);
   trailing   = FALSE;
   str_comm   = C_NC;
   f_qual     = C_NC;
   str_qual   = C_NC;
   z_match[0] = 1;
   z_match[1] = 'Z';

   openstreams();
   if (verifying) {
      PutStr("Editor\n:");
      Flush(Output());
   }
   edit(0);

quitlab:
   if (verout != oldoutput)  closeout(verout);
   if (edits  != oldinput)   closein(edits);
   while (filelist != NULL)  losefilespec(filelist);
   while (veclist != NULL)   discardvec(veclist+1);

   if(rdargs) {
	FreeArgs(rdargs);
	rdargs=0;
   }

   return(rc);
}

/**
*
*  Routine:  rdn - read a number out of the opt string
*
*  Synopsis: p = rdn(p, &val)
*            char *p;    Parse pointer in opt string
*            long val;   Location to store the result
*
*  Notes:
*
**/
char *rdn(p, valp)
char *p;
long *valp;
{
   int c;
   *valp = 0;
   while(c = *p++)
   {
      /* Convert it to a real number and test to see that it was a valid */
      /* digit in the process.                                           */
      c -= '0';
      if (c < 0 || c > 9) break;
      *valp = (*valp * 10) + c;
   }
   return(p-1);  /* Remember we skipped one too many in the process */
}

/**
*
*  Routine: gettemp
*
*  Synopsis: name = gettemp(file)
*
*  Notes: This routine locates a :T directory on the target
*   	  of the volume where the output file is to be placed.
*
**/
char *gettemp(file)
char *file;
{
   BPTR lock, rootlock;
   char *p, *buf;
   int len, rc;
   struct FileInfoBlock __aligned fib;

   if ((lock = Lock(file, SHARED_LOCK)) != NULL)
   {
      /* We have a lock on it, now get a lock on the root */
      lock = CurrentDir(lock);
      rootlock = Lock(":", SHARED_LOCK);
      UnLock(CurrentDir(lock));
      if (rootlock != NULL)
      {
	 fib.fib_FileName[0] = 0;
	 rc = Examine(rootlock, &fib);
         UnLock(rootlock);
	 if (rc == DOSTRUE)
	 {
	    len = strlen(fib.fib_FileName);
	    buf = newvec(len+3);
	    memcpy(buf, fib.fib_FileName, len);
	    memcpy(buf+len, ":T", 3);
	    return(buf);
	 }
      }	
   }

   /* The file doesn't exist, look to see if it has a : in the name */
   for(p = file; *p && *p != ':'; p++);
   if (*p == ':')
   {
      len = (p - file);
      buf = newvec(len+3);  /* The name plus a :T\0 */
      memcpy(buf, file, len);
      memcpy(buf+len, ":T", 3);
      return(buf);	 
   }

   /* Just use the :T directory */
   return(":T");
}

/**
*
*  Routine:  tempname
*
*  Synopsis: name = tempname("/E00-WK1");
*
*  Notes:
*
**/
char *tempname(string)
char *string;
{
   int n;
   char *s;
   short tasknum;
   short tens;
   int len;

   len = strlen(e_temp);
   n = strlen(string);
   s = newvec(len+n+1);
   tasknum = ((struct Process *)FindTask(0))->pr_TaskNum;
   memcpy(s, e_temp, len);
   strcpy(s+len, string);
   while (tasknum > 100) tasknum -= 100;
   tens = tasknum / 10;
   tasknum -= tens*10;
   s[len+2] = tens + '0';
   s[len+3] = tasknum + '0';
   return(s);
}

void openstreams()
{
   BPTR lock;

   textin = Open(e_from, MODE_OLDFILE);
   if (textin == NULL) error(ERROR_FFA, e_from);
   textout = Open(e_work, MODE_NEWFILE);
   if (textout == DOSFALSE && IoErr() == ERROR_OBJECT_NOT_FOUND)
   {
      lock = CreateDir( e_temp );
      if (lock != NULL) UnLock(lock);
      textout = Open(e_work, MODE_NEWFILE);
   }
   if (textout == NULL)
   {
      closein(textin);
      error(ERROR_FFA, e_work);
   }
   primaryoutput->f_sp = textout;
   primaryinput->f_sp  = textin;
   primaryinput->f_lc  = 1;

   currentoutput = primaryoutput;
   currentinput  = primaryinput;
   currentline   = freelines;

   freelines     = currentline->l_next;
   oldestline    = currentline;
   currentline->l_next = 0;
   currentline->l_prev = 0;
   SelectInput( textin );
   readline();  /* Into currentline+l.buf */
   exhausted  = (cch == ENDSTREAMCH);
   currentinput->f_ex = exhausted;
   unchanged = TRUE;
   pointer = 0;
   current = 1;
   globcount = 0;
   ceiling  = MAXINT;
   opened  = TRUE;
}


void closestreams()
{
   opened = FALSE;
   while(oldestline != 0)
      writeline();
   if (currentoutput != primaryoutput)
      losefilespec(currentoutput);

   if (currentinput != primaryinput)
      losefilespec(currentinput);

   closeout(primaryoutput->f_sp);
   closein(primaryinput->f_sp);
}

void rewind()
{
   e_from = e_work;
   e_work = e_workin;
   e_workin = e_workout;
   e_workout = e_work;
}

void windup()
{
   int rc;

   if (e_work != e_to)
   {
      DeleteFile(e_backup);
      Rename(e_to, e_backup);
      rc = Rename(e_work, e_to);
      if (rc == DOSFALSE && IoErr() == ERROR_OBJECT_EXISTS)
      {
         DeleteFile(e_to);
         rc = Rename(e_work, e_to);
      }
      if (rc == DOSFALSE)
      {
         error(ERROR_RN, e_work, e_to);
      }
      DeleteFile(e_workin);
   }
}

void closeout(s)
BPTR s;
{
   if (s != NULL)
   {
      Close(s);
   }
}


void closein(s)
BPTR s;
{
   if (s != NULL)
   {
      Close(s);
   }
}

void *newvec(n)
long n;
{
   long *v;
 
   v = AllocVec((n+7)&~3, MEMF_CLEAR);
   if (v == NULL)
      error(ERROR_MEM);
   *v = (long)veclist;
   veclist = v;
   return(v+1);
}

void discardvec(v)
void *v;
{
   long *p, *t, *b;
   b = ((long *)v) -1;
   for(p = (long *)&veclist; p != NULL; p = t)
   {
      t = (long *)*p;
      if (t == b)
      {
         *p = *t;
         FreeVec(t);
         /* For some reason, the BCPL one kept trucking at this point */
         /* We better not find it on the list more than once          */
         return;
      }
   }
}

void __stdargs MemCleanup() {}
