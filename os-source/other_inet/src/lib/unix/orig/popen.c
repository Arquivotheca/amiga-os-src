/*
** popen()/pclose() for Amiga.  Relies on the existance of PIPE: device.
*/

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
extern struct Library *DOSBase;
extern struct ExecBase *SysBase;


FILE *
popen(program, type)
	char	*program, *type;
{
	char	cbuf[64], pname[32], *p;
	int	result;
	FILE 	*fp;


	strcpy(pname,"pipe:popen");
	sprintf(cbuf,"%ld",(ULONG)FindTask(0));
	strcat(pname,cbuf);


	/*
	** tricky here.  We must add a redirection ( < or > ) after the
	** command part of the string but before any arguments.
	*/
	strcpy(cbuf, "c:run >nil: ");
	for(p = cbuf; *p; p++){
		/* skip to EOS */
	}
	while(*program){
		if(isspace(*program)){
			break;
		}

		*p++ = *program++;
	}
	*p = '\0';
	strcat(p, *type=='r'? " >":" <");
	strcat(p, pname);
	strcat(p, program);

    	if(!(fp = fopen(pname, type))){
		perror(pname);
		return 0;
	}

	result = Execute(cbuf, 0L, 0L);

	if(result){
		return fp;
	} else {
		fclose(fp);
		return 0;
	}
}

void pclose(FILE *fp)
{
	fclose(fp);
}
