#include	"wb2cli.h"
#include <stdio.h>

extern	struct	DosLibrary	*DOSBase;

void main(int argc,char *argv[])
{
struct	CommandLineInterface	*cli;

	cli=(void *)WB2CLI((struct WBStartup *)argv,4000,DOSBase);

	/*
	 * Note that we really should make sure we have valid
	 * pr_CIS and pr_COS since Workbench does not give us any.
	 */

	System("NewCLI",NULL);
}
