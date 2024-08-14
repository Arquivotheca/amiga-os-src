; /*
lc -d -j73 -O -o/obj/Type.o -i/include -v -csf Type
blink /obj/Type.o to /bin/Type sc sd nd
protect /bin/Type +p
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

/*----------------------------------------------------------------------*/
/* Command: Type                                                        */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 19MAR89  John Toebes   Initial Creation                              */
/* 21MAR89  John Toebes   Major rewrite                                 */
/* 25OCT89  John Toebes   Revamp for DOS36.21                           */
/* 02DEC89  John Toebes   Final rewrite after code review               */
/* 23MAY90  John Toebes   Corrected handling of RC                      */
/* 26AUG90  John Toebes   Corrected BUG8731 - files without CR at end   */
/* 19JAN91  John Toebes   Corrected B12043 - Bogus error messages       */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/

#include "internal/commands.h"
#include "type_rev.h"

#define MSG_OPTIGN   "Option '%lc' ignored\n"
#define MSG_CANTOPEN "TYPE can't open %s\n"
#define MSG_EXISTS   "%s already exists\n"
#define MSG_NOBOTH   "Type can't do both HEX and NUMBER\n"
#define MSG_NO_FILES "No files to type\n"

#define TEMPLATE    "FROM/A/M,TO/K,OPT/K,HEX/S,NUMBER/S" CMDREV
#define OPT_FROM   0
#define OPT_TO     1
#define OPT_OPT    2
#define OPT_HEX    3
#define OPT_NUMBER 4
#define OPT_COUNT  5

/*---------------------------------------------------------------------------*/
/* Structure used by the pattern matching routines.                          */
/*---------------------------------------------------------------------------*/
struct uAnchor {
  struct AnchorPath uA_AP;
  BYTE uA_Path[256];
};


int typefile(int state, BPTR fh, struct DosLibrary *DOSBase);

/*----------------------------------------------------------------------*/
/* These are the states for the type routine.  They are order dependent */
/* In particular, STATE_NUM is one less than STATE_NONUM to allow for   */
/* a convenient check of valid arguments by subtracting one from the    */
/* state when NUMBER is specified.  If HEX were also specified then it  */
/* will set the state to an invalid type.                               */
/*----------------------------------------------------------------------*/
#define STATE_HEX     0 /* File is being typed in HEX                   */
#define STATE_NUM     1 /* File is being typed with NUMBER              */
#define STATE_NONUM   2 /* File is being typed with default             */
#define STATE_NUMCONT 3 /* File is being typed with NUMBER but the      */
                        /* line overflowed the buffer so we need to     */
                        /* Supress the line number the next time        */

#define HEXASC  39

#define HEXSIZE 61
#define OUTSIZE 255

int cmd_type(void)
{
   struct Library *SysBase = (*((struct Library **) 4));
   struct DosLibrary *DOSBase;
   int rc;
   char *msg;
   int c;
   int state;
   long opts[OPT_COUNT];
   long args[1];
   BPTR infile, outfile , oldfile;
   BPTR lock;
   char *curarg, **argptr;
   struct RDargs *rdargs;
   struct uAnchor *ua;

   rc = RETURN_FAIL;
   if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
      memset(opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else {
	oldfile=Output();

         /*-------------------------------------------------------------*/
         /* Allocate memory to use with the pattern match stuff.        */
         /*-------------------------------------------------------------*/
         if ((ua = AllocVec(sizeof(struct uAnchor),MEMF_CLEAR)) == NULL)
         {
            PrintFault(IoErr(),NULL);
         }

         else {
            infile = outfile = NULL;

            /*----------------------------------------------------------*/
            /* Now we have to set up the pattern match structure.       */
            /*                                                          */
            /* Tell the pattern routines that we want them to return up */
            /* to 255 chars of the full path to any matches.            */
            /*----------------------------------------------------------*/
            ua->uA_AP.ap_Strlen = 255;
            /*----------------------------------------------------------*/
            /* Set the flag which tells the pattern matcher to decode   */
            /* wild cards.                                              */
            /*----------------------------------------------------------*/
            /* ua->uA_AP.ap_Flags = 0; */       /* It is zeroed memory!!*/
            /*----------------------------------------------------------*/
            /* Tell the matcher to stop if the user presses Ctrl-C.     */
            /*----------------------------------------------------------*/
            ua->uA_AP.ap_BreakBits = SIGBREAKF_CTRL_C;

            /*----------------------------------------------------------*/
            /* Parse out the options for typing the file.  We can have  */
            /* HEX, NUMBERS, or nothing.                                */
            /* The options can come from the keywords and the OPT string*/
            /*----------------------------------------------------------*/

            /*----------------------------------------------------------*/
            /* Handle OPT H and OPT h as HEX; Likewise for OPT n        */
            /*----------------------------------------------------------*/
            if (msg = (char *)opts[OPT_OPT])
            {
               while (c = *msg++)
               {
                  /*----------------------------------------------------*/
                  /* Take the option char and convert it to lower case  */
                  /*----------------------------------------------------*/
                  c |= 0x20;
 
                  if (c == 'h')
                     opts[OPT_HEX]    = 1;  /* Any value will do        */
                  else if (c == 'n')
                     opts[OPT_NUMBER] = 1;  /* Any value will do        */
                  else
                  {
                     /*-------------------------------------------------*/
                     /* Something we don't like.  Tell them about it    */
                     /*-------------------------------------------------*/
                     opts[OPT_OPT] = msg[-1];
                     VPrintf(MSG_OPTIGN, opts+OPT_OPT);
                  }
               }
            }

            /*----------------------------------------------------------*/
            /* Choose a default mode to operate in                      */
            /*----------------------------------------------------------*/
            state = STATE_NONUM;
            if (opts[OPT_HEX])    state = STATE_HEX;

            /*----------------------------------------------------------*/
            /* Convert STATE_NONUM to STATE_NUM - STATE_HEX goes to -1  */
            /*----------------------------------------------------------*/
            if (opts[OPT_NUMBER]) state--; 

            /*----------------------------------------------------------*/
            /* See if they ask us to do both HEX and NUMBER             */
            /*----------------------------------------------------------*/
            if (state < 0)
            {
               msg = MSG_NOBOTH;
               rc = RETURN_WARN;
            }

            else {
               msg = MSG_NO_FILES;
               /*-------------------------------------------------------*/
               /* Now we loop through the input files typing them out   */
               /* The first loop is for the multi-args followed by the  */
               /* by the loop over the wild cards.                      */
               /*-------------------------------------------------------*/
               argptr = (char **)opts[OPT_FROM];
	       rc = 0;
               while ((rc == 0) && (curarg = *argptr++)) {
                  msg = NULL;
                  /*----------------------------------------------------*/
                  /* Next we process the wild cards                     */
                  /*----------------------------------------------------*/
                  if (MatchFirst(curarg, ua)) {     /* = not ==     */
                     /*-------------------------------------------------*/
                     /* Set up a default message for when we can't find */
                     /* either the input or the output file             */
                     /*-------------------------------------------------*/
		     rc=IoErr();
		     if (rc == ERROR_NO_MORE_ENTRIES) rc = 0;
                     args[0] = (long)curarg;
                     VPrintf(MSG_CANTOPEN, args);
                     break;
                  }
                  rc = 0;
                  while(rc == 0) {
                     /*-------------------------------------------------*/
                     /* Locate and open the input source file.          */
                     /*-------------------------------------------------*/
                     lock = CurrentDir(ua->uA_AP.ap_Current->an_Lock);
                     infile = Open(ua->uA_AP.ap_Info.fib_FileName,
                                   MODE_OLDFILE);
		     rc = IoErr();
                     CurrentDir(lock);

                     /*-------------------------------------------------*/
                     /* Check to make sure that we could open the input */
                     /* file.                                           */
                     /*-------------------------------------------------*/
                     if (infile == NULL) {
                        args[0] = (long)ua->uA_AP.ap_Info.fib_FileName;
                        /*----------------------------------------------*/
                        /* No, give them a message and fall through     */
                        /*----------------------------------------------*/
                        VPrintf(MSG_CANTOPEN, args);
                        break;
                     }
                     /*-------------------------------------------------*/
                     /* If this is the first time through, we may need  */
                     /* to open up the output file also.                */
                     /*-------------------------------------------------*/
                     if (opts[OPT_TO] && (outfile == NULL)) {
                        if (!(outfile =
                                Open((char *)opts[OPT_TO], MODE_NEWFILE)))
                        {
			   rc=IoErr();
                           args[0] = opts[OPT_TO];
                           VPrintf(MSG_CANTOPEN, args);
                           break;
                        }
                     }

                     /*-------------------------------------------------*/
                     /* By some miracle we have succeeded,              */
                     /* Type the files to the buffered output.          */
                     /*-------------------------------------------------*/
                     if (outfile)SelectOutput(outfile);
                     rc = typefile(state, infile, DOSBase);
                     if(outfile)SelectOutput(oldfile);
                     Close(infile);
		     infile = NULL;
                     if (rc) break;

                     /*-------------------------------------------------*/
                     /* Advance to the next file that might match       */
                     /*-------------------------------------------------*/
                     if (MatchNext(ua)) {
			rc = IoErr();
			if (rc == ERROR_NO_MORE_ENTRIES) rc = 0;
			break;
		     }
                  }
                  MatchEnd(ua);
               }
            }
            /*----------------------------------------------------------*/
            /* Now that we are done, check for any failures             */
            /*----------------------------------------------------------*/
            if (msg) {
		VPrintf(msg, opts);
	    }
            else if (rc) {
               /*-------------------------------------------------------*/
               /* Not a ^C, how about a normal exit of no more entries  */
               /*-------------------------------------------------------*/
               if(rc>=0)PrintFault(rc,NULL);
	       if (rc == ERROR_BREAK)rc = RETURN_WARN;
	       else rc = RETURN_ERROR;
            }

            /*----------------------------------------------------------*/
            /* Cleanup, closing anything we have left open.             */
            /*----------------------------------------------------------*/
            if (infile)  Close(infile);
            if (outfile) Close(outfile);
            FreeVec(ua);
         }
         FreeArgs(rdargs);
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else {OPENFAIL;}
   return(rc);
}

/***
*
* This subroutine types a file from the buffered input to the buffered
* output.  It takes a state flag which indicates the mode of typing
* to be done.
*
***/
int typefile(int state, BPTR fh, struct DosLibrary *DOSBase)
{
   int counter;
   int cyc;
   int c;
   long opts[10];
   char outbuf[OUTSIZE];
   char *msg;
   int i;

   msg = NULL;
   /*-------------------------------------------------------------*/
   /* Success, we have the arguments and the open file.  Now just */
   /* run until we hit the EOF or they send us a ^C               */
   /*-------------------------------------------------------------*/
   counter = cyc = 0;

   /*-------------------------------------------------------------*/
   /* Read the next load of input and process a char at a time    */
   /*-------------------------------------------------------------*/
   while((c = FGetC(fh)) != -1) {
      if (state == STATE_HEX) {
         /*-------------------------------------------------------*/
         /* HEX formatting comes here                             */
         /*-------------------------------------------------------*/
         if (!cyc) {
            /* Time to init the output buffer */
            memset(outbuf, ' ', HEXSIZE);
            outbuf[HEXSIZE-1] = 0;
            opts[0] = counter;
            opts[1] = (long)outbuf;
         }
         counter++;
         /*-------------------------------------------------*/
         /* Now fill in the current character slot          */
         /*-------------------------------------------------*/
         msg = outbuf+(cyc<<1)+(cyc>>2);
         msg[0] = "0123456789ABCDEF"[c>>4];
         msg[1] = "0123456789ABCDEF"[c&15];

         /*-------------------------------------------------*/
         /* Translate all non printing characters to a '.'  */
         /* This includes the range 0-1F, 7F, 80-9F, FF     */
         /*-------------------------------------------------*/
         if (((c+1)&0x7f) <= ' ') c = '.';

         msg = "%04lx: %s\n";
         outbuf[HEXASC+cyc] = c;
         cyc++;

         /*-------------------------------------------------------*/
         /* If we haven't filled the buffer then skip the print   */
         /*-------------------------------------------------------*/
         if (cyc != 16) continue;
      }
      else {
         /*-------------------------------------------------------*/
         /* NON-HEX goes here.                                    */
         /*-------------------------------------------------------*/
         msg = "%5ld %s";
         if (state == STATE_NUM) {
            opts[0] = counter+1;
            opts[1] = (long)outbuf;
         }
         else {
            msg += 5;  /* skip the %5ld */
            opts[0] = (long)outbuf;
         }
         outbuf[cyc++] = c;
         if (cyc == OUTSIZE-1) {
            if (state == STATE_NUM)
               state = STATE_NUMCONT;
         }
         else if (c == '\n') {
            if (state == STATE_NUMCONT)
               state = STATE_NUM;
            counter++;
         }
         else
            /* No reason to print so skip the print code */
            continue;
         outbuf[cyc] = 0;  /* Make sure it is null terminated */
      }

      /*-------------------------------------------------------*/
      /* When a mode decides it is time to print it just falls */
      /* through to here by not doing a continue               */
      /*-------------------------------------------------------*/
      cyc = 0;
      i=VPrintf(msg, opts);
      if(i < 0)return(IoErr());
      /*---------------------------------------*/
      /* Has the user aborted the operation ?  */
      /*---------------------------------------*/
            
      if (CheckSignal(SIGBREAKF_CTRL_C))return(ERROR_BREAK);

      /*-----------------------------------------------*/
      /* Reset so we assume there is no more to output */
      /*-----------------------------------------------*/
      msg = NULL;
   }

   /*-------------------------------------------------------*/
   /* If we have any trailing chars we need to output them  */
   /*-------------------------------------------------------*/
   if (msg != NULL) {
      if ((state != STATE_HEX) && cyc && outbuf[cyc-1] != '\n') {
         outbuf[cyc++] = '\n';
         outbuf[cyc++] = 0;
      }
      i=VPrintf(msg, opts);
      if(i < 0)return(IoErr());
   }
   return(0);
}
