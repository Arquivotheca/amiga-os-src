; /*
lc -O -d -j73 -o/obj/FileNote.o -i/include -v -csf FileNote
blink /obj/FileNote.o to /bin/FileNote sc sd nd
protect /bin/FileNote +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .   | || the authors:                            BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: FileNote                                                         */
/* Author:  Doug Walker                                                      */
/* Change History:                                                           */
/*  Date    Person        Action                                             */
/* -------  ------------- -----------------                                  */
/* 19MAR89  John Toebes   Initial Creation                                   */
/* 28MAR89  Doug Walker   Filled in body                                     */
/* 06NOV89  Doug Walker   Revved for dos.library 36.38                       */
/*   MAR90  Doug Walker   Added ALL option, wildcards                        */
/*   MAY90  Doug Walker   Added QUIET option                                 */
/* Notes:                                                                    */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "filenote_rev.h"

#define MSG_TRUNCATED "Note truncated to 79 characters\n"
#define MSG_NOTFOUND  "%s not found\n"
#define MSG_FAILED    "Note failed\n"
#define MSG_DONE      "...Done\n"
#define MSG_DIR       " (dir)"
#define MSG_BLANKS    "        "

#define TEMPLATE    "FILE/A,COMMENT,ALL/S,QUIET/S" CMDREV
#define OPT_FILE    0
#define OPT_COMMENT 1
#define OPT_ALL     2
#define OPT_QUIET   3
#define OPT_COUNT   4

#define NORMINDENT  3   
#define TABSIZE     5

/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(name, type) char a_##name[sizeof(type)+3]; \
			type *name = (type *)((LONG)(a_##name+3) & ~3);

struct uAnchorPath
{
   struct AnchorPath ap;
   char name[255];
};

int cmd_filenote(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int rc, rc2, rrc2, apinit, indent, tindent, isdir;
   char *msg;
   LONG opts[OPT_COUNT];
   struct RDargs *rdargs;
   BPTR lock;
   D_S(ap, struct uAnchorPath);

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset((char *)opts, 0, sizeof(opts));
      if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         apinit = 0;
         rc = RETURN_OK;
         msg = NULL;

         if(!opts[OPT_COMMENT]) {
            opts[OPT_COMMENT] = (LONG)"";
         }
         else if(strlen((char *)opts[OPT_COMMENT]) >= 80) {
            rc = RETURN_WARN;
            msg = MSG_TRUNCATED;
            ((char *)opts[OPT_COMMENT])[79] = '\0';
         }

         memset(ap, 0, sizeof(struct uAnchorPath));
         ap->ap.ap_Flags  = APF_DOWILD;
         ap->ap.ap_Strlen = 255;
         ap->ap.ap_BreakBits = SIGBREAKF_CTRL_C;

         if(MatchFirst((char *)opts[OPT_FILE], &ap->ap)) {
            rc = RETURN_FAIL;
            rc2 = IoErr();
            PrintFault(rc2, NULL);
            goto ERRORCASE;
         }

         apinit = 1;
         indent = 0;
         do {
            if(CheckSignal(SIGBREAKF_CTRL_C))
            {
               PrintFault(ERROR_BREAK, NULL);
               rc = RETURN_FAIL;
               rc2 = ERROR_BREAK;
               goto ERRORCASE;
            }

            if(ap->ap.ap_Flags & APF_DIDDIR) {
               /* Exiting a directory */
               ap->ap.ap_Flags &= ~APF_DIDDIR;
               indent--;
               continue;
            }

            isdir = (ap->ap.ap_Info.fib_DirEntryType >= 0);

            if(isdir && opts[OPT_ALL])
            {
               /* Step into the directory */
               ap->ap.ap_Flags |= APF_DODIR;
               indent++;
            }

            if(!opts[OPT_QUIET] && 
                ((ap->ap.ap_Flags & APF_ITSWILD) || opts[OPT_ALL]))
            {
               for(tindent = 0; tindent < indent; tindent++)
               {
                  WriteChars(MSG_BLANKS, TABSIZE);
               }
               if(!isdir) WriteChars(MSG_BLANKS, NORMINDENT);

               PutStr(ap->ap.ap_Info.fib_FileName);

               if(isdir) PutStr(MSG_DIR);
            }
            else
               tindent = -1;  /* We'll use tindent >= 0 below as a flag */

            lock = CurrentDir(ap->ap.ap_Current->an_Lock);
            if(!SetComment(ap->ap.ap_Info.fib_FileName, 
                           (char *)opts[OPT_COMMENT]))
            {
               rc = RETURN_FAIL;
               if((rc2=IoErr()) == ERROR_OBJECT_NOT_FOUND)
                  msg = MSG_NOTFOUND;
               else
                  msg = MSG_FAILED;
               opts[OPT_FILE] = (LONG)ap->ap.ap_Info.fib_FileName;
            }

            if(tindent >= 0)
            {
               /* If we put out the first part of the msg, put out the rest now */
               PutStr(MSG_DONE);
            }

            CurrentDir(lock);

         }
         while(!MatchNext(&ap->ap));

         if((rrc2 = IoErr()) && rrc2 != ERROR_NO_MORE_ENTRIES)
            rc2 = rrc2;
 
ERRORCASE:
         if (msg) VPrintf(msg, opts);

         if(apinit) MatchEnd(&ap->ap);

         FreeArgs(rdargs);

         SetIoErr(rc2);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      OPENFAIL;
   }
   return(rc);
}

#if 0
int aprintf( fmt, args )
char *fmt, *args;
{
struct Library *SysBase = (*((struct Library **) 4));
   struct Library *DOSBase; 
   DOSBase=OpenLibrary("dos.library",36);
   VFPrintf( Output(), fmt, (LONG *)&args );
   CloseLibrary(DOSBase);
   return(0);
}
#endif
