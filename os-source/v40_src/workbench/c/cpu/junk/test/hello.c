#include <exec/types.h>
#include <libraries/dos.h>
#include <proto/dos.h>
#include <string.h>

long _stack = 4000;             /* a reasonable amount of stack space */
char *_procname = "my_program";
long _priority = 0;             /* run at standard priority */
long _BackGroundIO = 1;         /* perform background I/O */
extern BPTR _Backstdout;        /* file handle we will write to with */

#define MSG "Hello World!"

void _main(argc, argv)
int argc;
char *argv[];

{
	int i;

   if (argc  == 0)              /* called from Workbench, can't do anything */
        _exit(0L);

   if (_Backstdout)             /* called from CLI - print out the string */
     {
     Write(_Backstdout, MSG, strlen(MSG));
     Close(_Backstdout);
     }

	while(TRUE)
	{
		i++;
	}

}
