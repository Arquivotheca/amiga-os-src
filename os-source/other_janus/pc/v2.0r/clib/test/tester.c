#include <janus/janus.h>
APTR AllocJanusMem(ULONG,ULONG);
VOID FreeJanusMem(APTR);
main()
{

	printf("Tesing GetBase.\n");
	{
		UBYTE error;
		UWORD	ParmSeg,ParmOff,BuffOff;

		error=GetBase(14,&ParmSeg,&ParmOff,&BuffOff);
		printf("Error from GetBase(PCDISK) = %x\n",(int)error);
		printf("ParmSeg = %x\n",ParmSeg);	
		printf("ParmOff = %x\n",ParmOff);	
		printf("BuffOff = %x\n",BuffOff);	
	}
	printf("\nTesing Alloc/Free-JanusMem.\n");
	{
		APTR	Ptr;

		Ptr=AllocJanusMem(0x00000100,(ULONG)MEMF_PARAMETER);
		printf("Ptr from AllocJanusMem(0x0100,MEMF_PARAMETER) = %lx\n",(ULONG)Ptr);
		
		FreeJanusMem(Ptr);
		printf("FreeJanusMem() returned\n");
		Ptr=AllocJanusMem(0x0100,MEMF_PARAMETER);
		printf("Ptr from AllocJanusMem(0x0100,MEMF_PARAMETER) = %lx\n",(ULONG)Ptr);
		
		FreeJanusMem(Ptr);
		printf("FreeJanusMem() returned\n");

		Ptr=AllocJanusMem(0x0100,MEMF_BUFFER);
		printf("Ptr from AllocJanusMem(0x0100,MEMF_BUFFER) = %lx\n",(ULONG)Ptr);
		
		FreeJanusMem(Ptr);
		printf("FreeJanusMem() returned\n");

		Ptr=AllocJanusMem(0x0100,MEMF_BUFFER);
		printf("Ptr from AllocJanusMem(0x0100,MEMF_BUFFER) = %lx\n",(ULONG)Ptr);
		
		FreeJanusMem(Ptr);
		printf("FreeJanusMem() returned\n");

	}
	printf("\nTesing SetParam.\n");
	{
		UBYTE error;
		UWORD	ParmOff;

		error=SetParam(20,0x300);
		printf("Error from SetParam(20,0x300) = %x\n",(int)error);
	}

	printf("Tester ending\n");

	test();
}
test()
{
	puts("hello");
}
