/* tester */

#include <exec/lists.h>
#include <proto/exec.h>
#include "OwnDevUnit.h"
#include <dos/dosextens.h>
#include <stdio.h>
#include <stdlib.h>

struct Library *OwnDevUnitBase;

main(int argc, char *argv[])
{
	UBYTE *RetVal;
	ULONG NotifyBit;
	ULONG Signal;

	if ((NotifyBit = AllocSignal(-1)) == -1) {
		printf("couldn't get signal bit\n");
		exit(0);
	}

	if (!(OwnDevUnitBase = OpenLibrary(ODU_NAME, 3))) {
		printf("couldn't open ODU.\n");
		exit(0);
	}

	printf("OwnerDevUnit() returned \"%s\"\n", OwnerDevUnit(argv[1], atol(argv[2])));

	RetVal = AttemptDevUnit(argv[1], atol(argv[2]), "Tester", NotifyBit);

	if (RetVal)
		printf("returned \"%s\"\n", RetVal);
	else
		printf("obtained the lock!\n");

	if (!RetVal)
		for (;;) {
			Signal = Wait(SIGBREAKF_CTRL_F | (1L << NotifyBit));

			if (Signal & (1L << NotifyBit))
				printf("NotifyBit triggered\n");

			if (Signal & SIGBREAKF_CTRL_F)
				break;
		}

	if (!RetVal)
		FreeDevUnit(argv[1], atol(argv[2]));

	FreeSignal(NotifyBit);
	CloseLibrary(OwnDevUnitBase);
}
