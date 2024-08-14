; /*
lc -O -d -j73 -o/obj/Makedir.o -i/include -v -csf Makedir
blink /obj/Makedir.o to /bin/Makedir sc sd nd
protect /bin/Makedir +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 382-8265    */
/* | o  | ||   Dave Baker      Alan Beale         Jim Cooper                 */
/* |  . |//    Jay Denebeim    Bruce Drake        Gordon Keener              */
/* ======      John Mainwaring Andy Mercier       Jack Rouse                 */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: Makedir                                                     */
/* Author:  Doug Walker                                                 */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 25APR89  Doug Walker   Filled in body                                */
/* 06NOV89  Doug Walker   Revved for dos.library 36.38                  */
/* 07JAN90  Doug Walker   Added support for multiargs                   */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "makedir_rev.h"

#define MSG_NONAME "No name given\n"
#define MSG_EXISTS "%s already exists\n"
#define MSG_IDUNNO "Can't create directory %s\n"

#define TEMPLATE    "NAME/M" CMDREV
#define OPT_NAME  0
#define OPT_COUNT 1

#define MSG_EXISTS "%s already exists\n"
#define MSG_IDUNNO "Can't create directory %s\n"

int cmd_makedir(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int rc=RETURN_FAIL, rc2=0;
   char *name;
   char **argptr;
   LONG opts[OPT_COUNT];
   BPTR lock;
   struct RDargs *rdargs;

   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))) {
      memset((char *)opts, 0, sizeof(opts));
      if(!(rdargs = ReadArgs(TEMPLATE, opts, NULL))) {
         PrintFault(IoErr(), NULL);
      }
      else {
         rc2 = 0;

         argptr = (char **)opts[OPT_NAME];
         if(argptr == NULL) {
            rc = RETURN_FAIL;
            VPrintf(MSG_NONAME, NULL);
         }
         else {
            rc = RETURN_OK;
            for(name=*argptr; name; name=*(++argptr)) {
               if(lock=Lock(name, SHARED_LOCK)) {
                  rc = RETURN_ERROR;
                  rc2 = 0;
                  VPrintf(MSG_EXISTS, (LONG *)&name);
               }
               else if(!(lock=CreateDir(name))) {
                  if(rc != RETURN_ERROR) {
                     rc2 = IoErr();
                     rc = RETURN_ERROR;
                  }
                  VPrintf(MSG_IDUNNO, (LONG *)&name);
               }
               if(lock)UnLock(lock);
            }
         }

         FreeArgs(rdargs);
         SetIoErr(rc2);
      }
      if(rc2)PrintFault(rc2, NULL);
      CloseLibrary((struct Library *)DOSBase);
   }
   else 
   {
      OPENFAIL;
      rc = RETURN_FAIL;
   }
   return(rc);
}
