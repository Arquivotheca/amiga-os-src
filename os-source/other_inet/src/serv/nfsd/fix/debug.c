#include<exec/types.h>
#include<dos/dos.h>

#define WINDOW "CON:0/0/630/200/ NFSD DEBUG /AUTO/WAIT/CLOSE"
BPTR debug_fh ;

void
debug(char *msg)
{
	if( debug_fh == NULL)
	{
		debug_fh = Open(WINDOW, MODE_NEWFILE) ;
	}
	if( debug_fh )
	{		
		FPuts(debug_fh, msg) ;
	}
}
				
void
debug_s(char *fmt, char *s)
{
	char buffer[256] ;

	if(fmt)
	{
		mysprintf(buffer, "%s", s) ;
		debug(buffer) ;
	}
}

void
debug_ss(char *fmt, char *s1, char *s2)
{
	char buffer[256] ;

	if(fmt)
	{
		mysprintf(buffer, "%s%s", s1, s2) ;
		debug(buffer) ;
	}
}
