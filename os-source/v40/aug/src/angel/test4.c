#include <exec/types.h>
#include <exec/memory.h>

main()
{
	APTR block;

	block = (APTR)AllocMem(500,MEMF_CHIP);
	printf("block: %lx \n",block);
	FreeMem(block,500);
}


