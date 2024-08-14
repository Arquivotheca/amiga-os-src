#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/nodes.h>
#include <exec/interrupts.h>
#include <stdio.h>
#include "show.h"

/* Some external declarations. */

void GetCRP(), GetSRP();
ULONG GetCACR(), GetTC(), GetCPUType(), GetMMUType(), GetFPUType();
ULONG GetTT0(), GetTT1();
void ShowTT();

int Page,Ignore,Level[4];

int pow2(int x)
{
	int i,y;

	if (x==0) return(0);		/* not mathematically correct */
	y = 2;
	for (i=0; i<(x-1); i++) y=y*2;
	return(y);
}

void ShowRP(ULONG RP0, ULONG RP1)
{
	ULONG Index, Addr, DT;

	Index = (RP0>>16)&0x7FFF;
	if (RP0>>31)
		printf("Lower limit index: ");
	else
		printf("Upper limit index: ");
	printf("%lx\n",Index);

	DT = RP0&0x3;
	switch(DT)
	{
		case 0:	printf("Invalid: ");
			break;
		case 1: printf("Page descriptor: ");
			break;
		case 2: printf("Short format descriptor table at: ");
			break;
		case 3: printf("Long format descriptor table at: ");
			break;
	}
	Addr = RP1&0xFFFFFFF0;
	printf("%08lx \n",Addr);
}

void ShowTable(ULONG RP0, ULONG RP1)
{
    ULONG Type, Address, UpperLimit=0x7FFF, LowerLimit=0x0000, Index;

    Type = RP0&0x3;
    Address = RP1&0xFFFFFFF0;

	Index = (RP0>>16)&0x7FFF;
	if (RP0>>31)
		LowerLimit = Index;
	else
		UpperLimit = Index;

    ShowTableA(Address,Type,0,UpperLimit);
}

ShowTableA(ULONG *MMUtable, ULONG Type, int level, ULONG limit)
{
    ULONG DT, Pointer, Address, NewLimit=0x7FFF;
    int i,j;

    for (i=0; ( (i<Level[level]) && (i<limit) ); i++)
    {
	if (Type==2)
	{
		DT = MMUtable[i]&0x03;
		Pointer = MMUtable[i];
	}
	else
	{
		DT = MMUtable[(i*2)+1]&0x03;
		Pointer = MMUtable[i*2];
	}
	for (j=0; j<level; j++) printf("	");
	switch(DT)
	{
	    case 0: Address = Pointer&0xFFFFFFFF;
		    printf("Invalid: %08lx \n",Address);
		    break;
	    case 1: printf("Page descriptor: ");
		    switch(Type)
		    {
		      case 2:
			Address = Pointer&0xFFFFFF00;
			printf("%08lx  ",Address);
			if ((Pointer>>6)&0x1) printf("Cache_Inhibit ");
			if ((Pointer>>2)&0x1) printf("Write_protect ");
			break;
		      case 3:
			Address = MMUtable[(i*2)+1]&0xFFFFFF00;
			printf("%08lx  ",Address);
			if (Pointer>>31) printf("Upper limit index: ");
			else printf("Lower limit index: ");
			printf("%ld",(int)((Pointer>>16)&0x7FFF) );
			if ((Pointer>>8)&0x1) printf("Supervisor_Only ");
			if ((Pointer>>6)&0x1) printf("Cache_Inhibit ");
			if ((Pointer>>2)&0x1) printf("Write_protect ");
			break;
		    }
		    printf("\n");
		    break;
	    case 2: 
	    case 3: if (DT==2) printf("4-Byte table: ");
		    else printf("8-byte table: ");
		    switch(Type)
		    {
		      case 2:
			Address = Pointer&0xFFFFFFF0;
			printf("%08lx  ",Address);
			if ((Pointer>>6)&0x1) printf("Cache_Inhibit ");
			if ((Pointer>>2)&0x1) printf("Write_protect ");
			NewLimit=0x7FFF;
			break;
		      case 3:
			Address = MMUtable[(i*2)+1]&0xFFFFFFF0;
			printf("%08lx  ",Address);
			if (Pointer>>31)
			{
				NewLimit=(Pointer>>16)&0x7FFF;
				printf("Lower limit index: ");
			}
			else printf("Upper limit index: ");
			printf("%ld",(int)((Pointer>>16)&0x7FFF) );
			if ((Pointer>>8)&0x1) printf("Supervisor_Only ");
			if ((Pointer>>6)&0x1) printf("Cache_Inhibit ");
			if ((Pointer>>2)&0x1) printf("Write_protect ");
			break;
		    }
		    printf("\n");

		    ShowTableA(Address,DT,level+1,NewLimit);
		    break;
	}
    }
}

void ShowTT(ULONG TT)
{
	ULONG AddressBase, AddressMask, FCBase, FCMask;

	if ((TT>>15)&0x1)
	{
		AddressBase = TT&0xFF000000; AddressMask = (TT<<8)&0xFF000000;
		printf("Logical Address Base =    %08lx \n",AddressBase);
		printf("Bits (A31-A24) to ignore: %08lx \n",AddressMask);

		FCBase = (TT>>4)&0x7; FCMask = TT&0x7;
		printf("FC Base: %lx   FC Mask (ignore): %lx \n",FCBase,FCMask);

		if ((TT>>10)&0x1) printf("Cache inhibit\n");
		if ((TT>>9)&0x1) printf("Read accesses transparent\n");
		else printf("Write accesses transparent\n");
		if ((TT>>8)&0x1) printf("R/W field ignored\n");
		else printf("R/W field used\n");
	}
	else printf("Disabled\n");
}

main()
{
	ULONG myCRP[2], mySRP[2], myTC, myTT0, myTT1, *MMUTable;
	ULONG TCenable, SREenable, FCLenable, Addr;
	struct ExecBase *ExecBase;

	ExecBase = (struct DosLibrary *)OpenLibrary("exec.library",NULL);

	printf("ShowMMU 1.0 by Valentin Pepelea. Copyright 1990 Commodore-Amiga, Inc.\n\n");

	if (ExecBase->AttnFlags&AFF_68030)
	{
		myTT0 = GetTT0();
		myTT1 = GetTT1();

		printf("Transparent Translation Register 0\n");
		printf("----------------------------------\n");
		ShowTT(myTT0);
		printf("\n");
		printf("Transparent Translation Register 1\n");
		printf("----------------------------------\n");
		ShowTT(myTT1);
		printf("\n");
	}

	myTC = GetTC();
	GetCRP(myCRP);
	GetSRP(mySRP);
	MMUTable = (ULONG *)myCRP[1];

	printf("CRP\n");
	printf("----------------------------------\n");
	ShowRP(myCRP[0],myCRP[1]);
	printf("\n");

	SREenable = (myTC>>25)&0x1;
	FCLenable = (myTC>>24)&0x1;

	if (SREenable)
	{
		printf("SRP\n");
		printf("----------------------------------\n");
		ShowRP(mySRP[0],mySRP[1]);
		printf("\n");
	}

	printf("TC: %08lx \n",myTC);
	printf("----------------------------------\n");
	TCenable = myTC>>31;
	if (TCenable) printf("MMU enabled, ");
	else printf("MMU disabled, ");

	if (SREenable) printf("Supervisor root pointer enabled, ");
	else printf("Supervisor root pointer not enabled, ");
	if (FCLenable) printf("FCL signals enabled, ");
	else printf("FCL signals not enabled, ");
	printf("sir.\n");

	Page = pow2((myTC>>20)&0x000F);
	printf("Page size:	%ld \n",Page);

	Ignore = (myTC>>16)&0x000F;
	printf("Ignore:		%ld bits\n",Ignore);

	Level[0] = pow2((myTC>>12)&0x000F);
	Level[1] = pow2((myTC>>8)&0x000F);
	Level[2] = pow2((myTC>>4)&0x000F);
	Level[3] = pow2(myTC&0x000F);

	printf("Level A:	%ld pointers\nLevel B:	%ld pointers\n",Level[0],Level[1]);
	printf("Level C:	%ld pointers\nLevel D:	%ld pointers\n",Level[2],Level[3]);
	printf("\n");

	printf("CRP table\n");
	printf("----------------------------------\n");
	ShowTable(myCRP[0],myCRP[1]);
	if (SREenable)
	{
		printf("\n");
		printf("SRP table");
		printf("----------------------------------\n");
		ShowTable(mySRP[0],mySRP[1]);
	}
}

