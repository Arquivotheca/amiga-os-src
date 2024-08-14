
/*
** getcwd for Amiga.
*/

#ifdef AZTEC_C
#include <functions.h>
#endif

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <bstr.h>

#ifdef LATTICE
#include <proto/exec.h>
#endif


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
	if(p == NULLCHAR || len > size ||
			buf == NULLCHAR && (buf = malloc(size)) == NULLCHAR)
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

char *getwd(path)
	char	*path;
{
	return getcwd(path, 1024);
}

