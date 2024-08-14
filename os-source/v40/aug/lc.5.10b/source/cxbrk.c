#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <string.h>
int CXBRK(void);
static struct IntuiText Text2 = {-1,-1, /* pen numbers */
                             0,         /* draw mode */
                             14,14,     /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "** User Abort Requested **",
                             NULL };

static struct IntuiText BodyText = {-1,-1,      /* pen numbers */
                             0,         /* draw mode */
                             4,4,       /* starting offsets */
                             NULL,      /* text attribute pointer */
                             NULL,
                             &Text2 };

static struct IntuiText ContinueText = {-1,-1,  /* pen numbers */
                             0,         /* draw mode */
                             4,4,       /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "CONTINUE",
                             NULL };

static struct IntuiText AbortText = {-1,-1,     /* pen numbers */
                             0,         /* draw mode */
                             4,4,       /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "ABORT",
                             NULL };

extern char *_ProgramName;

/**
*
* name          CXBRK - check for abort
*
* synopsis      CXBRK();
*
* description   This function is the default control-b or control-c handler.
*
**/
int CXBRK()
   {
   char temp[81];
   int len;
   struct Process *us;
   BPTR fh;
   struct CommandLineInterface *cli;

   len = _ProgramName[-1];
   if (len > 79) len = 79;
   memcpy(temp, _ProgramName, len);
   temp[len] = '\0';

   /* Now see if we were invoked from CLI or from WorkBench */
   us = (struct Process *)FindTask(0L);
   if (us->pr_CLI != NULL)
      {
      cli = (struct CommandLineInterface *)(((long)(us->pr_CLI)) << 2);
      fh = cli->cli_StandardOutput;
      if (fh == NULL)
         fh = us->pr_COS;
      if (fh != NULL)
         {
         Write(fh, "*** Break: ", 11);
         temp[len++] = '\n';
         Write(fh, temp, len);
         return(-1);
	 }
      }

   if (IntuitionBase == NULL)
      IntuitionBase = (struct IntuitionBase *)
                       OpenLibrary("intuition.library",0);

   BodyText.IText = temp;
   if (AutoRequest(NULL,&BodyText,&ContinueText,&AbortText,0,0,250,60) != TRUE) return(-1);
   return(0);
}

