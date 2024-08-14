#include <exec/types.h>
#include <dos/dosextens.h>

main()
{
	BPTR fin = Input() ;
	BPTR fout = Output() ;
	char buffer[256] ;
	long len ;
	
	printf("gAS  = %s\n", GetArgStr()) ;
	printf("Interactive: in %ld  out = %ld\n", IsInteractive(Input()), IsInteractive(Output())) ;
	printf("fin  = %08lx\n", fin) ;
	printf("fout = %08lx\n", fout) ;
	if(IsInteractive(Input()) == 0)
	{
		len = Read(Input(), buffer, 256) ; 
		while( len > 0)
		{
			Write(Output(), buffer, len) ;
			len = Read(Input(), buffer, 256) ; 
		}
		if( len < 0 )
		{
			printf("read error\n") ;
		}
	}
}