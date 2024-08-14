/*
** popen()/pclose() for Amiga.  Relies on the existance of PIPE: device.
*/

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

static char template[] = "PIPE:popnXXXXXX";

FILE *
popen(program, type)
	register char	*program, *type;
{
	char	cbuf[64], temp[64], pname[sizeof(template)], *p, *q;
	int	result, pfd;
	FILE 	*fp;

	strcpy(pname, template);
	mktemp(pname);

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

pclose(fp)
	FILE	*fp;
{
	fclose(fp);
}
