; /* 
lc -d -j73 -O -o/obj/Wait.o -i/include -v -rr -csf Wait
blink /obj/Wait.o to /bin/Wait sc sd nd lib lib:lcr.lib
protect /bin/Wait +p
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

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*------------------------------------------------------------------------*/
/* Command: wait                                                          */
/* Author:  Mary Ellen Toebes                                             */
/* Change History:                                                        */
/*  Date    Person        Action                                          */
/* -------  ------------- -----------------                               */
/*                                                                        */
/* 20Oct89  Mary Ellen Toebes Initial Creation                            */
/* 20May90  Mary Ellen Toebes Readjust what time every sec incase Low pri */
/* 20May90  Mary Ellen Toebes Have checked for bad RC related bugs        */
/* 27dec90  Mary Ellen Toebes Work with no arguments                      */
/* 03apr91  Mary Ellen Toebes wait no args should wait 1 sec              */
/*                                                                        */
/* Format: Wait<n>[Sec|Secs][Min|Mins][Until<HH:MM>]                      */
/*                                                                        */
/*------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "wait_rev.h"

#define MSG_BADHHMM  "Time should be HH:MM\n"
#define MSG_BADNUM   "Error in number\n"
#define MSG_ADJUSTED "Adjusted time\n"

#define TEMPLATE    "/N,SEC=SECS/S,MIN=MINS/S,UNTIL/K"  CMDREV
#define OPT_N       0             /* Numeric <n>         */
#define OPT_SECS    1             /* Keyword SEC or SECS */
#define OPT_MIN     2             /* Keyword MIN or MINS */
#define OPT_UNTIL   3             /* Keyword UNTIL       */
#define OPT_COUNT   4 

#define TPSEC  50                 /* Ticks per second    */
#define TPMIN  60 * 50            /* Ticks per minute    */                
#define TPHOUR 60 * 60 * 50       /* Ticks per hour      */
#define TPDAY  24 * 60 * 60 * 50  /* Ticks per day       */

int cmd_wait(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct Library    *UtilityBase;
   struct RDargs     *rdargs;
   long   opts[OPT_COUNT];
   int    rc;
   char  *p;
   short  num;
   short  colon;
   long   hours;
   long   minutes;
   long   ticks;
   long   v[3];
   long   until_time, until_day;
   long   now_time, now_day;

   rc = RETURN_FAIL;

   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)) &&
       (UtilityBase = OpenLibrary("utility.library", 0))) 
   {
      memset((char *)opts, 0, OPT_COUNT * sizeof(long));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         rc = RETURN_OK;

         /* Get the current time */
         DateStamp(v);
         now_time = (SMult32(v[1],TPMIN)  + v[2]);
         now_day  = v[0];
         
         /* Wait until HH:MM */
         if (opts[OPT_UNTIL]) 
         {
            /* Read Args does not strip leading blanks for /K */
            for (p = (char *)opts[OPT_UNTIL]; (*p == ' ');  p++);

            /* Parse HH:MM string */
            colon = num = 0;
            while(*p && rc == RETURN_OK)
            {
               /* Colon found. Assign accumulated number to hours */
               if (*p == ':')
               {
                  if (colon++ > 1)    /* What ? more than 1 colon */
                     rc = RETURN_FAIL;
                  hours = (long)num;
                  num = 0;
               }
 
               /* Digit found. Add it to the number being ccumulated */
               else if (*p < '0' || *p > '9')
               {
                  rc = RETURN_FAIL;
               }
               else
               {
                  num *= 10;
                  num += (*p - '0');
               }
               p++;
            }
            minutes = (long)num;

            if((colon != 1) || 
               (rc == RETURN_FAIL) ||
               (hours < 0) || (hours > 23) || 
               (minutes < 0) || (minutes > 59))
            {
               PutStr(MSG_BADHHMM);
               rc = RETURN_FAIL;
            }
            else 
            {
               until_day  = now_day;
               until_time = SMult32(hours,TPHOUR) + SMult32(minutes,TPMIN);

               /* has until_time already passed? Then wait until same time*/
               /* next day.                                               */  
               if (now_time > until_time) until_day++;
            }
         }
         else
         {
            ticks = 1;
	    if (opts[OPT_N])
               ticks = *((long *)opts[OPT_N]);

            /* wait <n> minutes */
            if (opts[OPT_MIN]) ticks = SMult32(ticks,TPMIN);

            /* Wait <n> seconds  (default unit)  */
            else ticks = SMult32(ticks,TPSEC);

            until_day  = now_day;
            until_time = ticks + now_time;
            if (until_time >= TPDAY)
            {
               until_time -= TPDAY;
               until_day++;
            }
                        
            /* Check sign of tick number */
            if (ticks < 0)
            {
               PutStr(MSG_BADNUM);
               rc = RETURN_FAIL;
            }
         }

         if (rc == RETURN_OK)
         {
            while ((now_day < until_day) ||
                   (now_day == until_day && now_time < until_time))
            {
               /* check for CNTRL-C */
               if (CheckSignal(SIGBREAKF_CTRL_C))
               {
                  PrintFault(ERROR_BREAK, NULL);
                  rc = RETURN_WARN;
                  break;
               } 

               /* Wait a second at a time */
               Delay(TPSEC);

               /* Readjust incase we are running at a lo]w priority */
               DateStamp(v);
               now_day  = v[0];
               now_time = SMult32(v[1],TPMIN) + v[2];
            }
         }
         FreeArgs(rdargs);
      }

      CloseLibrary((struct Library *)DOSBase);
      CloseLibrary(UtilityBase);
   }
   else
   {
      /* We couldn't open DOS or utility so let them know */
      if (DOSBase) CloseLibrary((struct Library *)DOSBase);
      OPENFAIL;
   }


   return(rc);
}
