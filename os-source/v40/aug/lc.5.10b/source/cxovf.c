#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <string.h>
void cxovf(void);
void _exit(int);

extern struct IntuitionBase *IntuitionBase;
extern char *_ProgramName;

static struct IntuiText Text2 = {-1,-1, /* pen numbers */
                             0,         /* draw mode */
                             14,14,     /* starting offsets */
                             NULL,      /* text attribute pointer */
                             NULL,
                             NULL };

static struct IntuiText BodyText = {-1,-1,      /* pen numbers */
                             0,         /* draw mode */
                             4,4,       /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "** Stack Overflow **",
                             &Text2 };

static struct IntuiText ResponseText = {-1,-1,  /* pen numbers */
                             0,         /* draw mode */
                             4,4,       /* starting offsets */
                             NULL,      /* text attribute pointer */
                             "EXIT",
                             NULL };
void cxovf()
{
char    temp[80];

if (IntuitionBase == NULL)
   IntuitionBase = (struct IntuitionBase *)
                   OpenLibrary("intuition.library",0);
memcpy(temp, _ProgramName, (long) _ProgramName[-1]);
temp[(int) _ProgramName[-1]] = '\0';
Text2.IText = temp;
AutoRequest(NULL,&BodyText,NULL,&ResponseText,0,0,250,40);
_exit(20);
}