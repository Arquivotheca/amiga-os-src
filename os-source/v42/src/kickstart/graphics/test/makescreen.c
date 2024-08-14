#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <stdio.h>

struct IntuitionBase *IntuitionBase;

main()
{
	IntuitionBase=OpenLibrary("intuition.library",0);
	for(;;)
	{
		getchar();
		MakeScreen(IntuitionBase->ActiveScreen);
		RethinkDisplay();
		printf("remade\n");
	}
}
