
/*
** getcwd for Amiga.
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <string.h>
#include <dos.h>
#include <sys/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <sys/param.h>
#include <sys/errno.h>
extern struct ExecBase *SysBase;
extern struct Library *DOSBase;

#define NULLFIB	((struct FileInfoBlock *)0)
#define NULLCHAR ((char *)0)

char *getcwd(buf, size)
	char	*buf;
	int	size;
{
	struct FileInfoBlock *fib;
	long newcwd, cwd, len;
	register char *p;
	char nbuf[1024];

	Chk_Abort();

	if(size<=0) {
		errno = EINVAL;
		return(NULLCHAR);
	}
	
	if((fib = (struct FileInfoBlock *)AllocMem(sizeof(*fib), MEMF_PUBLIC))==NULLFIB)
		return(NULLCHAR);

	cwd = DupLock(((struct Process *)FindTask(0))->pr_CurrentDir);

	p = nbuf + sizeof(nbuf);
	for(; cwd != 0; cwd = newcwd){
		Examine(cwd, fib);
		newcwd = ParentDir(cwd);
		UnLock(cwd);
		len = strlen(fib->fib_FileName);
		if((p -= len) <= nbuf){
			p = NULLCHAR;
			break;
		}
		bcopy(fib->fib_FileName, p, len);
		*--p = '/';
	}
	FreeMem(fib, sizeof(*fib));

	if(*p == '/')
		p++;

	len = (nbuf + sizeof(nbuf)) - p;
	if(p == NULLCHAR || len > size) {
		errno = ERANGE;
		return(NULLCHAR);
	}

	/* if buf is NULL, we allocate 'size' bytes */
	if(buf==NULLCHAR) 
		if((buf=(char *)malloc(size))==NULL) 
			return(NULLCHAR);


	bcopy(p, buf, len);
	buf[len] = 0;

	/*
	** following turns path into amiga like path, e.g.:
	**
	**	volume/x/y/z
	**
	** becomes
	**
	**	volume:x/y/z
	*/
	for(p = buf; *p; p++){
		if(*p == '/'){
			*p = ':';
			break;
		}
	}
	if (*p != ':' )
	{
		/* volume only, no subdirs */
		*p++ = ':';
		*p = 0;
	}

	return(buf);
}

char *getwd(char *path)
{
	return getcwd(path, MAXPATHLEN);
}

