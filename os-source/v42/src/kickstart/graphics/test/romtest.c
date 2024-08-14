#include <exec/types.h>
#include <exec/memory.h>

struct Library *IntuitionBase;

main()
{
	char *ptr1;
	ULONG newsecs,oldsecs,newmics,oldmics;
	int i;
	IntuitionBase=OpenLibrary("intuition.library",0);
	if (ptr1=AllocMem(524288,MEMF_CHIP))
	{
		CurrentTime(&oldsecs,&oldmics);
		for(i=0;i<100;i++)
			CopyMemQuick(0xf00000,ptr1,524288);
		CurrentTime(&newsecs,&newmics);
		FreeMem(ptr1,524288);
		printf("elapsed time=%ld\n",newsecs-oldsecs);
	}
}
	