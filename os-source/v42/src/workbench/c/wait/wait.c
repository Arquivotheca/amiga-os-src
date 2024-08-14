
#include "internal/commands.h"
#include "wait_rev.h"
#include <dos/notify.h>

#define MSG_BADHHMM  "Time should be HH:MM\n"
#define MSG_BADNUM   "Error in number\n"
#define MSG_ADJUSTED "Adjusted time\n"

#define TEMPLATE    "/N,SEC=SECS/S,MIN=MINS/S,UNTIL/K,FILE/K"  CMDREV
#define OPT_N       0             /* Numeric <n>         */
#define OPT_SECS    1             /* Keyword SEC or SECS */
#define OPT_MIN     2             /* Keyword MIN or MINS */
#define OPT_UNTIL   3             /* Keyword UNTIL       */
#define OPT_FILE    4			  /* wait for file (notify) */
#define OPT_COUNT   5

#define TPSEC  50                 /* Ticks per second    */
#define TPMIN  60 * 50            /* Ticks per minute    */
#define TPHOUR 60 * 60 * 50       /* Ticks per hour      */
#define TPDAY  24 * 60 * 60 * 50  /* Ticks per day       */

int cmd_wait(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   struct RDArgs     *rdargs;
   long   opts[OPT_COUNT];
   int    rc;
   char  *p;
   short  num;
   short  colon;
   long   hours;
   long   minutes;
   long   ticks;
   struct DateStamp   date;
   long   until_time, until_day;
   long   now_time, now_day;

   rc = RETURN_FAIL;

   if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER))
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
         DateStamp(&date);
         now_time = date.ds_Minute*TPMIN  + date.ds_Tick;
         now_day  = date.ds_Days;

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
               until_time = hours * TPHOUR + minutes * TPMIN;

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
            if (opts[OPT_MIN]) ticks = ticks * TPMIN;

            /* Wait <n> seconds  (default unit)  */
            else ticks = ticks * TPSEC;

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
            if (opts[OPT_FILE])
	    {
		struct NotifyRequest nr;
		ULONG                sigs;

		memset((char *)&nr, 0, sizeof(nr));
		nr.nr_Name                         = (STRPTR)opts[OPT_FILE];
		nr.nr_Flags                        = NRF_SEND_SIGNAL;
		nr.nr_stuff.nr_Signal.nr_Task      = FindTask(NULL);
		nr.nr_stuff.nr_Signal.nr_SignalNum = AllocSignal(-1);

		if (nr.nr_stuff.nr_Signal.nr_SignalNum != -1)
		{
		    if (StartNotify(&nr))
		    {
			sigs = Wait(SIGBREAKF_CTRL_C | (1 << nr.nr_stuff.nr_Signal.nr_SignalNum));

                        if (SIGBREAKF_CTRL_C & sigs)
                        {
                            PrintFault(ERROR_BREAK, NULL);
                            rc = RETURN_WARN;
                        }

			EndNotify(&nr);
		    }
		    else
                    {
                        PrintFault(IoErr(), NULL);
                        rc = RETURN_FAIL;
                    }
		    FreeSignal(nr.nr_stuff.nr_Signal.nr_SignalNum);
		}
      	    }
	    else
	    {
                while ((now_day < until_day) || (now_day == until_day && now_time < until_time))
                {
                    /* check for CTRL-C */
                    if (CheckSignal(SIGBREAKF_CTRL_C))
                    {
                       PrintFault(ERROR_BREAK, NULL);
                       rc = RETURN_WARN;
                       break;
                    }

                    /* Wait a second at a time */
                    Delay(TPSEC);

                    /* Readjust incase we are running at a low priority */
                    DateStamp(&date);
                    now_day  = date.ds_Days;
                    now_time = date.ds_Minute * TPMIN + date.ds_Tick;
                }
            }
         }
         FreeArgs(rdargs);
      }

      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      /* We couldn't open DOS or utility so let them know */
      if (DOSBase) CloseLibrary((struct Library *)DOSBase);
      OPENFAIL;
   }


   return(rc);
}
