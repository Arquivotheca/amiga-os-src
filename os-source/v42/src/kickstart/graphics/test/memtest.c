#include <exec/types.h>
#include <exec/memory.h>

struct Library *IntuitionBase;

main()
{
	char *ptr1;
	ULONG newsecs,oldsecs,newmics,oldmics;
	int i;
	IntuitionBase=OpenLibrary("intuition.library",0);
	ptr1=0x40200000;
	{
		CurrentTime(&oldsecs,&oldmics);
		for(i=0;i<100;i++)
			CopyMemQuick(ptr1+4,ptr1,500000);
		CurrentTime(&newsecs,&newmics);
		printf("elapsed time=%ld\n",newsecs-oldsecs);
	}
}
	