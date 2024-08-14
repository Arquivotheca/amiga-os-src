/*
    LoadF - LoadSeg system modules into $F00000 memory.  See doc file
	    for more info.

    Created Friday 01-Sep-89 12:40:33	Bryce Nesbitt
    Updated Sunday 13-Nov-89 03:40:33	Bryce Nesbitt
    Updated Thursday 15-Mar-90 07:07:55 Bryce Nesbitt
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>

#define EOF (-1)
#define STARTFMEM   0xf00000
#define ENDFMEM     0xf80000

extern struct ExecBase *SysBase;

unsigned long PrivateLoadSeg();

main(argc,argv)
int argc;
char *argv[];
{
ULONG *MagicStorageAddress=(ULONG *)(ENDFMEM-28); /* High water mark      */
ULONG LoadFromAddress;				 /* Current load address */
unsigned long * volatile RamTest;		/* RAM test pointer	*/
unsigned long Temp;			       /* general purpose long */
unsigned long Temp2;			       /* general purpose long */
int argindex;				      /* Agument processing   */


	/* Get high water mark (end of last module) */
	LoadFromAddress=*MagicStorageAddress;
	if (LoadFromAddress < STARTFMEM || LoadFromAddress > ENDFMEM)
	    LoadFromAddress=STARTFMEM;


	printf("LoadF 2.0d - Load \".ld\" modules into $f00000 memory [$%lx].\n",
		LoadFromAddress);


	if (argc == 1 || (argc==2 && *argv[1]=='?'))
	    {
	    printf("USAGE: LoadF [module 1] [module 2]\n");
	    printf("NOTE : It's getting beta all the time!\n");
	    XCEXIT(5);
	    }


	/**** Cheesey test for write-protected board ****/
	RamTest=(unsigned long *)STARTFMEM;
	Temp=*RamTest;
	*RamTest^=0xffffffff;

	if (SysBase->LibNode.lib_Version >= 36)
	    CacheClearU(-1);

	if( (~Temp) != (*RamTest) )
	    {
	    printf("ERROR:  $F00000 memory not found\n");
	    XCEXIT(20);
	    }
	*RamTest=Temp;


	/**** First pass - test that all files exist ****/
	for( argindex=1; argindex < argc; argindex++)
	    {
	    if( Temp=Lock(argv[argindex],MODE_OLDFILE) )
		UnLock(Temp);
	    else
	       {
	       printf("File %s does not exist!\n",argv[argindex]);
	       XCEXIT(10);
	       }
	    } /* check all files */


	/***** Second pass - load all files *****/
	for( argindex=1; argindex < argc; argindex++)
	    {
	    printf("Loading file %s\nStart \t= $%lx\t",
		argv[argindex],LoadFromAddress);
	    Temp=PrivateLoadSeg(argv[argindex],LoadFromAddress);

	   if(LoadFromAddress==Temp)
		{
		printf("ERROR: LoadSeg of %s failed!\n",argv[argindex]);
		XCEXIT(10);
		}
	    else
		{
		printf("End \t= $%lx\tSize \t= $%lx / %ld\n",
		    Temp,
		    Temp-LoadFromAddress,
		    Temp-LoadFromAddress);
		LoadFromAddress=Temp;
		}
	    } /* load all files */

    /* If Exec was loaded, adjust magic constant */
    /* (We patch the ROM ID code so V1.3 ROMs can find us) */
    if( (*((ULONG *)STARTFMEM) & 0xfff0ffff) == 0x11104ef9L )
	 *(ULONG *)STARTFMEM = 0x11114ef9L;

    /* Set new high water mark (last module loaded into billboard) */
    *MagicStorageAddress=LoadFromAddress;
}
