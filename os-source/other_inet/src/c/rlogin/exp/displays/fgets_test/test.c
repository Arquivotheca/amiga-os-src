#include <exec/types.h>
#include <dos/dosextens.h>

int myexit(void) ;

int chkabort()
{
	printf("chkabort() called\n") ;
	return(0) ;
}

main()
{
	BPTR fin, fout ;
	STRPTR buf[256] ;
    
    fin = Input() ;
    fout = Output() ;

	printf("onbreak = %ld\n", (long)onbreak(&myexit)) ;

	do
	{
		FPuts(fout, "Prompt> ") ;
		Flush(fout) ;
		FGets(fin, buf, 256L) ;
		if(strchr(buf, 3))
			printf("got control-c\n") ;
		if(strchr(buf,'Q'))
			break ;
		FPuts(fout, buf) ;
		Flush(fout) ;
	} 
	while(1 == 1) ;
}

int
myexit()
{
	printf("myexit\n") ;
	exit(1000) ;
}
		