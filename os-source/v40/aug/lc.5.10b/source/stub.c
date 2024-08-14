#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <string.h>
#include <stdlib.h>

static struct IntuiText Text2 = {-1,-1, /* pen numbers */
                             0,         /* draw mode */
                             14,14,     /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "** Undefined Stub Routine Called **",
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
* name          stub - default routine for undefined routines
*
* synopsis      stub();
*
* description   This function is the default routine resolved by Blink for routines
*               not found in libraries.
*
**/
int stub()
   {
   char temp[80];

   if (IntuitionBase == NULL)
        IntuitionBase = (struct IntuitionBase *)
                        OpenLibrary("intuition.library",0);
   memcpy(temp, _ProgramName, (long) _ProgramName[-1]);
   temp[(int) _ProgramName[-1]] = '\0';
   BodyText.IText = temp;
   if (AutoRequest(NULL,&BodyText,&ContinueText,&AbortText,0,0,320,60) != TRUE)
      exit(1);
   return(0);
}
